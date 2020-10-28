#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef enum {
	NALU_TYPE_SLICE    = 1,
	NALU_TYPE_DPA      = 2,
	NALU_TYPE_DPB      = 3,
	NALU_TYPE_DPC      = 4,
	NALU_TYPE_IDR      = 5,
	NALU_TYPE_SEI      = 6,
	NALU_TYPE_SPS      = 7,
	NALU_TYPE_PPS      = 8,
	NALU_TYPE_AUD      = 9,
	NALU_TYPE_EOSEQ    = 10,
	NALU_TYPE_EOSTREAM = 11,
	NALU_TYPE_FILL     = 12,
} NaluType;

typedef enum{
	NALU_PRIORITY_DISPOSABLE=0,
	NALU_PRIRITY_LOW        =1,
	NALU_PRIORITY_HIGH      =2,
	NALU_PRIORITY_HIGHEST   =3
}NaluPriority;


typedef struct
{
	int startcodeprefix_len;     
	//4 for parameter sets and first slice in picture,
	//3 for everything else (suggested)
	unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
	unsigned max_size;            //! Nal Unit Buffer size
	int forbidden_bit;            //! should be always FALSE
	int nal_reference_idc;        //! NALU_PRIORITY_xxxx
	int nal_unit_type;            //! NALU_TYPE_xxxx    
	char *buf;                    //! contains the first byte followed by the EBSP
} NALU_t;

FILE *h264bitstream=NULL;
int info2=0,info3=0;

static int FindStartCode2(unsigned char* Buf){
	if(Buf[0]!=0||Buf[1]!=0||Buf[2]!=1)return 0;//00 00 01
	else return 1;
}

static int FindStartCode3(unsigned char *Buf){
	if(Buf[0]!=0||Buf[1]!=0||Buf[2]!=0||Buf[3]!=1)return 0;//00 00 00 01
	else return 1;
}

int GetAnnexbNALU(NALU_t *nalu){
	int pos=0;
	int StartCodeFound,rewind;
	unsigned char* Buf;
	if((Buf=(unsigned char *)calloc(nalu->max_size,sizeof(char)))==NULL){
		//calloc申请的内存空间自动初始化为0
		printf("GetAnnexbNALU:cannot allocate Buf\n");
	}
	nalu->startcodeprefix_len=3;
	//先读三个字节，判断
	if(3!=fread(Buf,1,3,h264bitstream)){
		free(Buf);
		return 0;
	}
	info2=FindStartCode2(Buf);
	if(info2!=1){
		//再读一个字节，判断
		if(1!=fread(Buf+3,1,1,h264bitstream)){
			free(Buf);
			return 0;
		}
		info3=FindStartCode3(Buf);
		if(info3!=1){
			free(Buf);
			return -1;
		}else {
			pos=4;
			nalu->startcodeprefix_len=4;
		}
	}else {
		nalu->startcodeprefix_len=3;
		pos=3;
	}
	//已设置pos和startcodeprefix_len
	StartCodeFound=0;
	info2=0;
	info3=0;
	while(!StartCodeFound){//未找到下一个
		if(feof(h264bitstream)){//到文件尾还未出现startcode,即最后一段NALU
			nalu->len=(pos-1)-nalu->startcodeprefix_len;//pos-1是因为最后一个FF读入到buf中了
			memcpy(nalu->buf,&Buf[nalu->startcodeprefix_len],nalu->len);
			//数据段第一个字节
			nalu->forbidden_bit=nalu->buf[0]&0x80;//1 bit
			nalu->nal_reference_idc=nalu->buf[0]&0x60;//2 bit
			nalu->nal_unit_type=(nalu->buf[0])&0x1f;//5 bit
			free(Buf);
			return pos-1;
		}
		Buf[pos++]=fgetc(h264bitstream);
		info3=FindStartCode3(&Buf[pos-4]);//每次读入一个字节都需要判断是否构成startcode
		if(info3!=1){
			info2=FindStartCode2(&Buf[pos-3]);
		}
		StartCodeFound=(info2==1||info3==1);
	}
	//多读入的3/4字节删掉
	rewind=(info3==1)?-4:-3;
	if(0!=fseek(h264bitstream,rewind,SEEK_CUR)){
		free(Buf);
		printf("cannot fseek in the bit stream file\n");
	}
	//Here the Start code, the complete NALU, and the next start code is in the Buf.  
	// The size of Buf is pos, pos+rewind are the number of bytes excluding the next
	// start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code
	nalu->len=(pos+rewind)-nalu->startcodeprefix_len;
	memcpy(nalu->buf,&Buf[nalu->startcodeprefix_len],nalu->len);
	nalu->forbidden_bit=nalu->buf[0]&0x80;
	nalu->nal_reference_idc=nalu->buf[0]&0x60;
	nalu->nal_unit_type=(nalu->buf[0])&0x1f;
	free(Buf);
	return (pos+rewind);
}

int simple_h264_parser(char *url){
	NALU_t *n;
	int buffersize=100000;
	FILE *myout=stdout;
	h264bitstream=fopen(url,"rb+");
	if(h264bitstream==NULL){
		printf("open file failed\n");
		return 0;
	}
	n=(NALU_t*)calloc(1,sizeof(NALU_t));
	if(n==NULL){
		printf("Alloc NALU failed\n");
		return 0;
	}
	n->max_size=buffersize;//最大值
	n->buf=(char*)calloc(buffersize,sizeof(char));//max_size memory
	if(n->buf==NULL){
		free(n);
		printf("there is no enough memory\n");
		return 0;
	}
	int data_offset=0;
	int nal_num=0;
	printf("-----+-------- NALU Table ------+---------+\n");
	printf(" NUM |    POS  |    IDC |  TYPE |   LEN   |\n");
	printf("-----+---------+--------+-------+---------+\n");
	while(!feof(h264bitstream)){
		int data_length;
		data_length=GetAnnexbNALU(n);
		char type_str[0]={0};
		switch(n->nal_unit_type){
			case NALU_TYPE_SLICE:sprintf(type_str,"SLICE");break;
			case NALU_TYPE_DPA:sprintf(type_str,"DPA");break;
			case NALU_TYPE_DPB:sprintf(type_str,"DPB");break;
			case NALU_TYPE_DPC:sprintf(type_str,"DPC");break;
			case NALU_TYPE_IDR:sprintf(type_str,"IDR");break;
			case NALU_TYPE_SEI:sprintf(type_str,"SEI");break;
			case NALU_TYPE_SPS:sprintf(type_str,"SPS");break;
			case NALU_TYPE_PPS:sprintf(type_str,"PPS");break;
			case NALU_TYPE_AUD:sprintf(type_str,"AUD");break;
			case NALU_TYPE_EOSEQ:sprintf(type_str,"EOSEQ");break;
			case NALU_TYPE_EOSTREAM:sprintf(type_str,"EOSTREAM");break;
			case NALU_TYPE_FILL:sprintf(type_str,"FILL");break;
		}
		char idc_str[20]={0};
		switch(n->nal_reference_idc>>5){
			case NALU_PRIORITY_DISPOSABLE:sprintf(idc_str,"DISPOS");break;
			case NALU_PRIRITY_LOW:sprintf(idc_str,"LOW");break;
			case NALU_PRIORITY_HIGH:sprintf(idc_str,"HIGH");break;
			case NALU_PRIORITY_HIGHEST:sprintf(idc_str,"HIGHEST");break;
		}
		fprintf(myout,"%5d| %8d| %7s| %6s| %8d|\n",nal_num,data_offset,idc_str,type_str,n->len);
		data_offset=data_offset+data_length;
		nal_num++;
	}
	if(n){
		if(n->buf){
			free(n->buf);
			n->buf=NULL;
		}
		free(n);
	}
	return 0;
}

			
int main(int argc,char* argv[]){
	if(argc<2){
		printf("input parameter is false\n");
		return -1;
	}
	simple_h264_parser(argv[1]);
	return 0;
}



