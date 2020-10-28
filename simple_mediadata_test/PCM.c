#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int simple_pcm16le_split(char *url){
	FILE *fp=fopen(url,"rb+");
	FILE *fp1=fopen("out_putl.pcm","wb+");
	FILE *fp2=fopen("out_putr.pcm","wb+");
	unsigned char* sample=(unsigned char*)malloc(4);
	//PCM16LE双声道中左声道和右声道采样值是间隔存储的，
	//每个采样值占用2Byte空间
	while(!feof(fp)){
		fread(sample,1,4,fp);
		short *samplenum=NULL;
		samplenum=(short*)sample;
		//*(samplenum)/=2;可将左声道音量减半，左声道数据在前面
		//
		fwrite(sample,1,2,fp1);//count个size大小
		fwrite(sample+2,1,2,fp2);
	}
	//可以将数据速度提高一倍的方法：
	//只采样每个声道奇数点的样值。
	/*
	int cnt=0;
	while(!feof(fp)){
		fread(sample,1,4,fp);
		if(cnt%2!=0){
			fwrite(sample,1,2,fp1);
			fwrite(sample+2,1,2,fp1);
		}
		cnt++;
	}
	*/   
	free(sample);
	fclose(fp);
	fclose(fp1);
	fclose(fp2);
	return 0;
}

/**
 * Convert PCM16LE raw data to WAVE format
 * @param pcmpath      Input PCM file.
 * @param channels     Channel number of PCM file.
 * @param sample_rate  Sample rate of PCM file.
 * @param wavepath     Output WAVE file.
 */
// WAVE_HEADER,WAVE_FMT,WAVE_DATA,采样数据四部分构成；
// WAVE_HEADER和WAVE_DATA包含了文件长度信息的dwsize字段，需要写完采样数据后获得
// 这两个结构体最后写入；
int simplest_pcm16le_to_wave(const char *pcmpath,int channels,int sample_rate,const char *wavepath)
{
	//将unsigned long改为了unsigned int;
	typedef struct WAVE_HEADER{
		char         fccID[4];
		unsigned   int    dwSize;
		char         fccType[4];
	}WAVE_HEADER;

	typedef struct WAVE_FMT{
		char         fccID[4];
		unsigned   int       dwSize;
		unsigned   short     wFormatTag;
		unsigned   short     wChannels;
		unsigned   int       dwSamplesPerSec;
		unsigned   int       dwAvgBytesPerSec;
		unsigned   short     wBlockAlign;
		unsigned   short     uiBitsPerSample;
	}WAVE_FMT;

	typedef struct WAVE_DATA{
		char       fccID[4];
		unsigned int dwSize;
	}WAVE_DATA;


	if(channels==0||sample_rate==0){
    channels = 2;
    sample_rate = 44100;
	}
	int bits = 16;

    WAVE_HEADER   pcmHEADER;
    WAVE_FMT   pcmFMT;
    WAVE_DATA   pcmDATA;

    unsigned   short   m_pcmData;
    FILE   *fp,*fpout;

	fp=fopen(pcmpath, "rb");
    if(fp == NULL) {
        printf("open pcm file error\n");
        return -1;
    }
	fpout=fopen(wavepath,   "wb+");
    if(fpout == NULL) {
        printf("create wav file error\n");
        return -1;
    }
	//WAVE_HEADER
    memcpy(pcmHEADER.fccID,"RIFF",strlen("RIFF"));
    memcpy(pcmHEADER.fccType,"WAVE",strlen("WAVE"));
    fseek(fpout,sizeof(WAVE_HEADER),1);
	//WAVE_FMT
    pcmFMT.dwSamplesPerSec=sample_rate;
    pcmFMT.dwAvgBytesPerSec=pcmFMT.dwSamplesPerSec*sizeof(m_pcmData);
    pcmFMT.uiBitsPerSample=bits;
    memcpy(pcmFMT.fccID,"fmt ",strlen("fmt "));
    pcmFMT.dwSize=16;
    pcmFMT.wBlockAlign=2;
    pcmFMT.wChannels=channels;
    pcmFMT.wFormatTag=1;

    fwrite(&pcmFMT,sizeof(WAVE_FMT),1,fpout);

    //WAVE_DATA;
    memcpy(pcmDATA.fccID,"data",strlen("data"));
    pcmDATA.dwSize=0;
    fseek(fpout,sizeof(WAVE_DATA),SEEK_CUR);

    fread(&m_pcmData,sizeof(unsigned short),1,fp);
    while(!feof(fp)){//注意feof用法
        pcmDATA.dwSize+=2;
        fwrite(&m_pcmData,sizeof(unsigned short),1,fpout);
        fread(&m_pcmData,sizeof(unsigned short),1,fp);
    }

    pcmHEADER.dwSize=44+pcmDATA.dwSize;

    rewind(fpout);
    fwrite(&pcmHEADER,sizeof(WAVE_HEADER),1,fpout);
    fseek(fpout,sizeof(WAVE_FMT),SEEK_CUR);
    fwrite(&pcmDATA,sizeof(WAVE_DATA),1,fpout);

	fclose(fp);
    fclose(fpout);

    return 0;
}

//16bit采样位数转换为8bit
int simple_pcm16le_to_pcm8(char *url){
	FILE *fp=fopen(url,"rb+");
	FILE *fp1=fopen("output_8.pcm","wb+");
	int cnt=0;
	unsigned char* sample=(unsigned char*)malloc(4);
	while(!feof(fp)){
		short *samplenum16=NULL;
		char samplenum8=0;
		unsigned char samplenum8_u=0;
		fread(sample,1,4,fp);
		//PCM16L格式的采样数据取值范围是-32768-32767，
		//PCM8格式取值范围是0-255
		//先取两个字节，舍弃低位，加上128；
		samplenum16=(short *)sample;
		samplenum8=(*samplenum16)>>8;
		samplenum8_u=samplenum8+128;
		fwrite(&samplenum8_u,1,1,fp1);
		samplenum16=(short*)(sample+2);
		samplenum8=(*samplenum16)>>8;
		samplenum8_u=samplenum8+128;
		fwrite(&samplenum8_u,1,1,fp1);
		cnt++;
	}
	printf("Sample cnt:%d\n",cnt);
	free(sample);
	fclose(fp);
	fclose(fp1);
	return 0;
}
			

int main(){
	return 0;
}

