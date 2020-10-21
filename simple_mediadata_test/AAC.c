#include<stdio.h>
#include<stdlib.h>
#include<string.h>
//data为AAC frame;
int getADTSframe(unsigned char* buffer,int buf_size,unsigned char* data,int* data_size){
	int size=0;
	if(!buffer||!data||!data_size){
		return -1;
	}
	while(1){
		if(buf_size<7){
			return -1;
		}
		//size 是32位，低13位并
		if((buf_size[0]==0xff)&&((buffer[1]&0xf0)==0xf0)){
			size|=((buffer[3]&0x03)<<11);//11是因为后面部分加起来11
			size|=buffer[4]<<3;
			size|=((buffer[5]&0xe0)>>5);
			break;
		}
		--buf_size;
		++buffer;//去除头部同步字前无用的信息；
	}
	if(buf_size<size){
		return 1;
	}
	memcpy(data,buffer,size);
	*data_size=size;
	return 0;
}

int simple_aac_parser(char *url){
	int data_size=0;
	int size=0;
	int cnt=0;
	int offset=0;
	FILE *myout=stdout;
	unsigned char* aacframe=(unsigned char*)malloc(1024*5);
	unsigned char* aacbuffer=(unsigned char*)malloc(1024*1024);
	FILE *ifile=fopen(url,"rb");
	if(!ifile){
		printf("Open file failed");
		return -1;
	}
	while(!feof(ifile)){  //feof检测流上的文件结束符的函数，文件未结束则返回0
		data_size=fread(aacbuffer+offset,1,1024*1024-offset,ifile);//从ifile读取信息；
			
		unsigned char* input_data=aacbuffer;
		while(1){
			int ret=getADTSframe(input_data,data_size,aacframe,&size);
			if(ret==-1){
				break;
			}else if(ret==1){
				memcpy(aacbuffer,input_data,data_size);
				offset=data_size;
				break;
			}
			char profile_str[10]={0};
			char frequence_str[10]={0};
			unsigned char profile=aacframe[2]&0xc0;//高两位
			profile=profile>>6;
			switch(profile){
				case 0:sprintf(profile_str, "Main");break;
				case 1:sprintf(profile_str,"LC");break;
				case 2:sprintf(profile_str,"SSR");break;
				default:sprintf(profile_str,"unknown");break;
			}
			unsigned char sampling_frequency_index=aacframe[2]&0x3c;//中间四位，判断帧数
			sampling_frequency_index=sampling_frequency_index>>2;
			switch(sampling_frequency_index){
				case 0:sprintf(frequence_str,"96000Hz");break;
				case 1:sprintf(frequence_str,"88200Hz");break;
				case 2:sprintf(frequence_str,"64000Hz");break;
				case 3:sprintf(frequence_str,"48000Hz");break;
				case 4:sprintf(frequence_str,"44100Hz");break;
				case 5:sprintf(frequence_str,"32000Hz");break;
				case 6:sprintf(frequence_str,"24000Hz");break;
				case 7:sprintf(frequence_str,"22050Hz");break;
				case 8:sprintf(frequence_str,"16000Hz");break;
					   default:sprintf(frequence_str,"unknown");break;;
			}
			fprintf(myout,"%5d| %8s| %8s| %5d|\n",cnt,profile_str,frequence_str,size);
			data_size-=size;
			input_data+=size;
			cnt++;
		}
	}
	fclose(ifile);
	free(aacbuffer);
	free(aacframe);
	return 0;
}
