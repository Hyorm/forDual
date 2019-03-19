#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <crown.h>

#include "lib/wiringPi.h"
#include "lib/wiringSerial.h"

void uart_ch(char ch);
void uart_str(char *str);
char str[100] = {0,};

int main(void)
{

	if(wiringPiSetup () == -1)
	{
		fprintf(stdout, "Unable to start wiringPi: %s\n, strerror(errno)");
		return 1;
	}

	//while(1)
	//{
		int readInt;
		printf("> ");
		
		SYM_int(readInt);
		printf("readInt: %d\n",readInt);
		//readInt = 5;//readInt%99;
		
		//scanf("%d",&readInt);
	
		printf("%d \n>> ",readInt);
		for(int i = 0; i<readInt; i++)
		{
			SYM_unsigned_char(str[i]);
			//scanf(" %c", &str[i]);
		}
		str[readInt]='\0';
		printf("%s\n",str);
		sprintf(str, "%s\n\r", str);
		uart_str(str);	
	//}

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
