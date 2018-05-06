#include "lib.h"
#include "types.h"
#include<stdarg.h>
/*
 * io lib here
 * 库函数写在这
 */
#define MAX_STACK_SIZE 1024 * 4
#define MAX_PCB_NUM 2 // father process , child processh

#define RUNABLED 0
#define BLOCKED 1
#define DEAD 2
//extern struct ProcessTable PCB[2];
//extern struct ProcessTable *current;
char * temp;
int32_t syscall(int num, uint32_t a1,uint32_t a2,
		uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret = 0;
	asm volatile("nop":"=a"(ret):"a"(num),"b"(a1),"c"(a2),"d"(a3),"D"(a4),"S"(a5));
	/* 内嵌汇编 保存 num, a1, a2, a3, a4, a5 至通用寄存器*/

	asm volatile("int $0x80");
		
	return ret;
}

int fork()
{
	return syscall(SYS_fork,0,0,0,0,0);
}


int sleep(uint32_t time)
{
	return syscall(SYS_sleep,0,0,time,0,0);
}

int exit()
{
	return syscall(SYS_exit,0,0,0,0,0);
}





char  *convert(unsigned int data, int num)
{
	static char num_list[20] = "0123456789abcdef";
	static char str2[20];
	str2[19] = '\0';
	 temp = &str2[19];
//	while(data != 0)
	do{
		temp--;
		*temp = num_list[data % num];
		data = data / num;
		
	}while(data != 0);
//	if(temp != &str2[18])
//		temp++;
	return temp;
}

void strcpy(char *dest, char *src)
{
	while(*src != '\0')
	{
		*dest = *src;
		src++;
		dest++;
	}

}

int strlen(char *str)
{
	int count = 0;
	while(*str != '\0')
	{
		str++;
		count++;
	}
	return count;

}
void printf(const char *format,...){
	const char *str = format;
	va_list ap;
	char buf[256];
	va_start(ap,format);
	uint32_t len = 0;
	char *s;
	for(; *str != '\0';str++)
	{
		if(*str == '%')  // fetch arguments
		{
			str++;
			if(*str == 'c')
			{
				buf[len] = va_arg(ap,int);
				len++;
			}
			else if(*str == 'd')
			{
				int n = va_arg(ap,int);
				if(n < 0)
				{	n = -n;
				buf[len] = '-';
				len++;
				}
				s = convert(n,10);
				strcpy(buf + len,s);
				len += strlen(s);

			}
			else if(*str == 'x')
			{
				
				int n = va_arg(ap,unsigned int);
		//		if(n < 0)
		//			n = -n;
		//		buf[len] = '-';
		//		len++;
				s = convert(n,16);
				strcpy(buf + len,s);
				len += strlen(s);
			}
			else if(*str == 's')
			{
				s = va_arg(ap,char *);
				strcpy(buf + len ,s);
				len += strlen(s);
			}
			else ;
		//	str++;


		}
		else {
			buf[len] = *str;
			len ++;

	}
	}
	va_end(ap);
	buf[len] = '\0';
	asm volatile ("int $0x80": :"a"(4),"b"(1),"c"(buf),"d"(len));
	
}
