#include<stdlib.h>
#include<stdio.h>
#include<string.h>

unsigned char clip_value(unsigned char x,unsigned char min_val,unsigned char max_val){
	if(x>max_val){
		return max_val;
	}else if( x<min_val){
		return min_val;
	}else {
		return x;
	}
}

bool  RGB24_TO_YUV420P(unsigned char *RgbBuf,int w,int h,unsigned char *yuvBuf){
	unsigned char* ptrY,*ptrU,*ptrV,*ptrRGB;
	memset(yuvBuf,0,w*h*3/2);
	ptrY=yuvBuf;
	ptrU=yuvBuf+w*h;
	ptrV=yuvBuf+w*h*5/4;
	unsigned char y,u,v,r,g,b;
	for(int j=0;j<h;j++){
		ptrRGB=RgbBuf+w*j*3;
		for(int i=0;i<w;i++){
			r=*(ptrRGB++);
			g=*(ptrRGB++);
			b=*(ptrRGB++);
			y=(unsigned char)( (66*r+129*g+25*b+128)>>8)+16;
			u=(unsigned char)( (-38*r-74*g+112*b+128)>>8)+128;
			v=(unsigned char)( (12*r-94*g-18*b+128)>>8)+128;
			*(ptrY++)=clip_value(y,0,255);
			if(j%2==0&&i%2==0){
				*(ptrU++)=clip_value(u,0,255);
			}else{
				if(i%2==0){
					*(ptrV++)=clip_value(v,0,255);
				}
			}
		}
	}
	return true;
}

int simple_rgb24_to_yuv420(const char *url_in,int w,int h,int num,\
			const char* url_out){
	FILE *fp=fopen(url_in,"rb+");
	FILE *fp1=fopen(url_out,"wb+");
	if(fp==NULL||fp1==NULL){
		printf("open file failed\n");
		return 0;
	}
	unsigned char* pic_rgb24=(unsigned char*)malloc(w*h*3);
	unsigned char* pic_yuv420=(unsigned char*)malloc(w*h*3/2);
	for(int i=0;i<num;i++){
		fread(pic_rgb24,1,w*h*3,fp);
		RGB24_TO_YUV420P(pic_rgb24,w,h,pic_yuv420);
		fwrite(pic_yuv420,1,w*h*3/2,fp1);
	}
	free(pic_yuv420);
	free(pic_rgb24);
	fclose(fp);
	fclose(fp1);
	return 0;
}


int main(){
	const char* name1="chongyang.rgb";
	const char* name2="newchongyang.yuv";
	simple_rgb24_to_yuv420(name1,2000,2000,1,name2);
	return 0;
}
