#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <crown.h>

#include "wiringPi.h"
#include "wiringSerial.h"

void uart_ch(char ch);
void uart_str(char *str);

int main(void)
{
	char str[100] = {0,};
	char str1[100] = {0,};

	if(wiringPiSetup () == -1)
	{
		fprintf(stdout, "Unable to start wiringPi: %s\n, strerror(errno)");
		return 1;
	}

	while(1)
	{
		int crown_int = 0;//
		SYM_int(crown_int);//
		crown_int = crown_int%100;//
		for(int i = 0; i < crown_int ; i++)//
		{//
			char crown_ch;//
			//scanf("%s", str1);
			SYM_char(crown_ch);//
			str[i] = crown_ch;//
		}//
		//getchar();
		sprintf(str, "%s\n\r", str1);
		uart_str(str);
		
	}

	return 0;
}

void uart_ch(char ch)
{
	int fd;

	//TODO:change sereal name  automatically

	if((fd = serialOpen("/dev/ttyUSB0",115200)) < 0)
	{
		fprintf(stderr, "Unable to open serial device:%s\n", strerror(errno));
		return;
	}

	serialPutchar(fd, ch);
	close(fd);
}

void uart_str(char *str)
{
	while(*str) uart_ch(*str++);
}
