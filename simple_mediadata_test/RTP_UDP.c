#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>

//#pragma comment(lib,"ws2_32.lib")//表示链接这个库
#pragma pack(1)

typedef struct RTP_FIXED_HEADER{
	unsigned char csrc_len:4;
	unsigned char extension:1;
	unsigned char padding:1;
	unsigned char version:2;
	unsigned char payload:7;

