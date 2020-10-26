#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<libavutil/log.h>


//将Y,U,V三个分量分离开并保存成三个文件
//planar格式
//高和宽为h和w，一帧YUV420p像素数据一共占用w*h*3/2的数据；
int simple_yuv420_split(char* url,int w,int h,int num){
	FILE *fp=fopen(url,"rb+");
	FILE *fp1=fopen("output_420_y.y","wb+");
	FILE *fp2=fopen("output_420_u.y","wb+");
	FILE *fp3=fopen("output_420_v.y","wb+");

	unsigned char *pic=(unsigned char*)malloc(w*h*3/2);
	unsigned char *rest=(unsigned char*)malloc(w*h/2);
	memset(rest,128,w*h/2);//可以通过修改UV的信息来控制生成图的颜色；
	//128为灰度图；
	for(int i=0;i<num;i++){
		//for 420P:
		fread(pic,1,w*h*3/2,fp);
		fwrite(pic,1,w*h,fp1);//w*h存Y
		fwrite(rest,1,w*h/2,fp1);
		fwrite(pic+w*h,1,w*h/4,fp2);//w*h/4存储U
		fwrite(pic+w*h*5/4,1,w*h/4,fp3);//w*h/4存储V
		//for 444P:
		/*pic=(unsigned char*)malloc(w*h*3);
		fwrite(pic,1,w*h*3,fp);
		fwrite(pic,1,w*h,fp1);
		fwrite(pic+w*h,1,w*h,fp2);
		fwrite(pic+w*h*2,w*h,fp3);*/

	}
	free(pic);
	fclose(fp);//原分辨率为256x256
	fclose(fp1);//256x256的纯Y数据
	fclose(fp2);//128x128的纯U数据
	fclose(fp3);//128x128的纯V数据
	return 0;
}

int simple_yuv420_halfl(char* url,int w,int h,int num){
	FILE *fp=fopen(url,"rb+");
	FILE *fp1=fopen("output_half.yuv","wb+");
	unsigned char *pic=(unsigned char*)malloc(w*h*3/2);
	for(int i=0;i<num;i++){
		fread(pic,1,w*h*3/2,fp);
		for(int j=0;j<w*h;j++){
			unsigned char temp=pic[j]/2;
			pic[j]=temp;
		}
		fwrite(pic,1,w*h*3/2,fp1);
	}
	free(pic);
	fclose(fp);
	fclose(fp1);
	return 0;
}
 
//(640,360,0,255,10,"graybar_640x360.yuv");
int simple_yuv420_graybar(int width,int height,int ymin,int ymax,int barnum,char *url_out){
	int barwidth;
	float lum_inc;
	unsigned char lum_temp;
	int uv_width,uv_height;
	FILE *fp=NULL;
	unsigned char *data_y=NULL;
	unsigned char *data_u=NULL;
	unsigned char *data_v=NULL;
	int t=0,i=0,j=0;
	barwidth=width/barnum;//每条的宽度
	lum_inc=((float)(ymax-ymin))/((float)(barnum-1));//bar之间的差值
	uv_width=width/2;
	uv_height=height/2;
	data_y=(unsigned char *)malloc(width*height);
	data_u=(unsigned char *)malloc(uv_width*uv_height);
	data_v=(unsigned char *)malloc(uv_width*uv_height);
	if((fp=fopen(url_out,"wb+"))==NULL){
		printf("ERROR: cannot creat file\n");
		return -1;
	}
	for(t=0;t<(width/barwidth);t++){
		lum_temp=ymin+(char)(t*lum_inc);
		printf("%3d,128,128\n",lum_temp);
	}
	for(j=0;j<height;j++){
		for(i=0;i<width;i++){
			t=i/barwidth;
			lum_temp=ymin+(char)(t*lum_inc);
			data_y[j*width+i]=lum_temp;
			}
	}
	
	for(j=0;j<uv_height;j++){
		for(i=0;i<uv_width;i++){
			data_u[j*uv_width+i]=64;
		}
	}
	for(j=0;j<uv_height;j++){
		for(i=0;i<uv_width;i++){
			data_v[j*uv_width+i]=64;
		}
	}
	fwrite(data_y,width*height,1,fp);
	fwrite(data_u,uv_width*uv_height,1,fp);
	fwrite(data_v,uv_height*uv_width,1,fp);
	fclose(fp);
	free(data_u);
	free(data_y);
	free(data_v);
	return 0;
}

int main(int argc,char* argv[]){
	av_log_set_level(AV_LOG_DEBUG);
	//av_log(NULL,AV_LOG_INFO,"%s\n",op);
		
	if(argc<2){
		av_log(NULL,AV_LOG_DEBUG,"wrong input format\n");
		return -1;
	}
	//simple_yuv420_split(argv[1],2000,2000,1);
	//simple_yuv420_halfl(argv[1],2000,2000,1);
	simple_yuv420_graybar(640,360,0,255,10,"graybar_640x360");
	return 0;
}
