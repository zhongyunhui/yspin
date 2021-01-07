/* gcc -g -o flvtoyuv flvtoyuv.c `pkg-config --libs libavformat libavutil libavcodec libswscale`  */

#include <asm-generic/errno-base.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavutil/pixfmt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

/*
bool saveAsBitmap(AVFrame *RGB,int width, int height, int iframe){
	FILE *pf=NULL;
	BITM
*/


int main(int argc, char * argv[]){
	AVFormatContext *context=avformat_alloc_context();
	if(argc<3){
		printf("input lack of parameter\n");
		return -1;
	}
	const char *infile =argv[1];
	const char *outfile=argv[2];
	int ret=avformat_open_input(&context, infile, NULL, NULL);
	if(ret<0){
		printf("open input file failed\n");
		return -1;
	}
	ret=avformat_find_stream_info(context,NULL);
	if(ret<0){
		printf("find stream info failed\n");
		return -1;
	}
	ret=av_find_best_stream(context, AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
	if(ret<0){
		printf("cannot find video stream\n");
		return 0;
	}
	int stream_index=ret;
	AVStream *stm=context->streams[ret];
	AVCodec *codec=avcodec_find_decoder(stm->codecpar->codec_id);
	AVCodecContext *codecCtx=avcodec_alloc_context3(codec);
	if(!codecCtx){
		printf("cannot get the codeccontext\n");
		return AVERROR(EINVAL);
	}
	if(avcodec_open2(codecCtx, codec, NULL)){
		printf("cannot open the codec\n");
		return -1;
	}
	int nGot=0;
	AVFrame *frame=av_frame_alloc();
	AVFrame *RGB=av_frame_alloc();
	AVPacket *packet=av_packet_alloc();
	printf("width: %d, Height: %d\n",codecCtx->width, codecCtx->height);
	int picSize=avpicture_get_size(AV_PIX_FMT_YUV420P, codecCtx->width, codecCtx->height);
	if(picSize<0){
		printf("Error cannot get picture size \n");
		return -1;
	}
	uint8_t *buf=(uint8_t *)av_malloc(picSize);
	avpicture_fill((AVPicture*)RGB, buf, AV_PIX_FMT_YUV420P, codecCtx->width, codecCtx->height);
	FILE *fp;
	printf("Log: before open\n");
	if((fp=fopen(outfile,"wb+"))==NULL){
		printf("Open source file failed\n");
		return -1;
	}
	struct SwsContext *sws=sws_getContext(codecCtx->width, codecCtx->height, codecCtx->pix_fmt, codecCtx->width, codecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL,NULL,NULL);
	if(sws==NULL){
		printf("Sws_getContext failed\n");
		return -1;
	}
	printf("come to while RGB size %lu",sizeof(RGB));
	while(1){
		if(av_read_frame(context,packet)>=0){
			if(packet->stream_index==stream_index){
				static int count=0;
				count++;
				int len=avcodec_send_packet(codecCtx, packet);
				if(len<0){
					printf("Error cannot send_packet %d",count);
					return -1;
				}
				while(1){
					int got_frame=avcodec_receive_frame(codecCtx, frame);
					if(got_frame==AVERROR_EOF||got_frame==AVERROR(EAGAIN)){
						return -1;
					}else{
						fflush(stdout);
						int nRet=sws_scale(sws, frame->data,frame->linesize,\
							codecCtx->height,codecCtx->width,RGB->data,RGB->linesize);
						fwrite(RGB->data[0],(codecCtx->width)*codecCtx->height,1,fp);
						fwrite(RGB->data[1],codecCtx->width*codecCtx->height/4,1,fp);
						fwrite(RGB->data[2],codecCtx->width*codecCtx->height/4,1,fp);
			
					}
				}
			}
		}else{
			break;
		}
	}
	fclose(fp);
	sws_freeContext(sws);
	av_free(frame);
	av_free(RGB);
	av_free(buf);
	av_free(packet);
	avcodec_close(codecCtx);
	avformat_close_input(&context);
	return 0;
}


