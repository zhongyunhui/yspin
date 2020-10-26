#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//将RGB数据前面加上头文件
//将RGB数据中每个像素的‘B'和'R'的位置互换；
//BMP文件由BitmapFileHeader，BitmapInfoHeader,RGB像素数据三部分构成
typedef  struct  tagBITMAPFILEHEADER
{ 
	unsigned short int  bfType;       //位图文件的类型，必须为BM 
	unsigned long       bfSize;       //文件大小，以字节为单位
	unsigned short int  bfReserverd1; //位图文件保留字，必须为0 
	unsigned short int  bfReserverd2; //位图文件保留字，必须为0 
	unsigned long       bfbfOffBits;  //位图文件头到数据的偏移量，以字节为单位
}BITMAPFILEHEADER; 
typedef  struct  tagBITMAPINFOHEADER 
{ 
	long biSize;                        //该结构大小，字节为单位
	long  biWidth;                     //图形宽度以象素为单位
	long  biHeight;                     //图形高度以象素为单位
	short int  biPlanes;               //目标设备的级别，必须为1 
	short int  biBitcount;             //颜色深度，每个象素所需要的位数
	short int  biCompression;        //位图的压缩类型
	long  biSizeImage;              //位图的大小，以字节为单位
	long  biXPelsPermeter;       //位图水平分辨率，每米像素数
	long  biYPelsPermeter;       //位图垂直分辨率，每米像素数
	long  biClrUsed;            //位图实际使用的颜色表中的颜色数
	long  biClrImportant;       //位图显示过程中重要的颜色数
}BITMAPINFOHEADER;
int simple_rgb24_to_bmp(const char *rgb24path,int width,int height,const char *bmppath){
	typedef struct{
		long imageSize;
		long blank;
		long startPosition;
	}BmpHead;
	typedef struct{
		long length;
		long width;
		long height;
		unsigned short colorPlane;
		unsigned short bitColor;
		long zipFormat;
		long realSize;
		long xPels;
		long yPels;
		long colorUse;
		long colorImportant;
	}InfoHead;

	int i=0,j=0;
	BmpHead m_BMPHeader={0};
	InfoHead m_BMPInfoHeader={0};
	char bfType[2]={'B','M'};
	int header_size=sizeof(bfType)+sizeof(BmpHead)+sizeof(InfoHead);
	unsigned char *rgb24_buffer=NULL;
	FILE *fp_rgb24=NULL,*fp_bmp=NULL;
	if((fp_rgb24=fopen(rgb24path,"rb"))==NULL){//无+意思是只读
		printf("cannot open rgb file\n");
		return -1;
	}
	if((fp_bmp=fopen(bmppath,"wb"))==NULL){
		printf("Error: connot open output file\n");
		return -1;
	}
	rgb24_buffer=(unsigned char *)malloc(width*height*3);
	fread(rgb24_buffer,1,width*height*3,fp_rgb24);//read frome source
	m_BMPHeader.imageSize=3*width*height+header_size;
	m_BMPHeader.startPosition=header_size;
	m_BMPInfoHeader.length=sizeof(InfoHead);
	m_BMPInfoHeader.width=width;
	m_BMPInfoHeader.height=-height;
	m_BMPInfoHeader.colorPlane=1;
	m_BMPInfoHeader.bitColor=24;
	m_BMPInfoHeader.realSize=3*width*height;
	fwrite(bfType,1,sizeof(bfType),fp_bmp);//写入bfType
	fwrite(&m_BMPHeader,1,sizeof(m_BMPHeader),fp_bmp);//m_BMPHeader
	fwrite(&m_BMPInfoHeader,1,sizeof(m_BMPInfoHeader),fp_bmp);//m_BMPInfoHeader;

	for(j=0;j<height;j++){
		for(i=0;i<width;i++){
			char temp=rgb24_buffer[(j*width+i)*3+2];
			rgb24_buffer[(j*width+i)*3+2]=rgb24_buffer[(j*width+i)*3];
			rgb24_buffer[(j*width+i)*3]=temp;
		}
	}
	fwrite(rgb24_buffer,3*width*height,1,fp_bmp);//先写
	fclose(fp_rgb24);
	fclose(fp_bmp);
	free(rgb24_buffer);
	return 0;
}


int simple_rgb24_split(char *url,int w,int h,int num){
	FILE *fp=fopen(url,"rb+");
	FILE *fp1=fopen("output_r.y","wb+");
	FILE *fp2=fopen("output_g.y","wb+");
	FILE *fp3=fopen("output_b.y","wb+");
	unsigned char *pic=(unsigned char *)malloc(w*h*3);
	for(int i=0;i<num;i++){
		fread(pic,1,w*h*3,fp);
		for(int j=0;j<w*h*3;j=j+3){
			fwrite(pic+j,1,1,fp1);//R
			fwrite("128",1,1,fp1);
			fwrite("128",1,1,fp1);
			fwrite(pic+j+1,1,1,fp2);//G
			fwrite(pic+j+2,1,1,fp3);//B
		}
	}
	free(pic);
	fclose(fp);
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	return 0;
}

int main(int argc,char *argv[]){
	if(argc<2){
		printf("wrong input format\n");
		return -1;
	}
	simple_rgb24_split(argv[1],2000,2000,1);
	return 0;
}

