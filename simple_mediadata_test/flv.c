#include<stdio.h>
#include<string.h>
#include<stdlib.h>
//设置结构体边界对齐为一个字节，就是让编译器将结构体数据强制连续排列，
#pragma pack(1)//what?

typedef unsigned char byte;
typedef unsigned int uint;

typedef struct{
	byte Signature[3];
	byte Version;
	byte flags;
	uint DataOffset;
}FLV_HEADER;
typedef struct{
	byte TagType;
	byte DataSize[3];
	byte Timestamp[3];
	uint Reserved;
}TAG_HEADER;
//bigEndian to LittleEndian
uint reverse_bytes(byte *p,char c){
	int r=0;
	int i;
	for(i=0;i<c;i++){
		r|=(*(p+i)<<(((c-1)*8)-8*i));
	}
	return r;
}






int main(){
	return 0;
}

