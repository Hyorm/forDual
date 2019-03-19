#include <stdio.h>
#include <string.h>
#include <errno.h>

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
		int readInt = 0;
		printf("> ");
		scanf("%d",&readInt);
		printf(">>");
		for(int i = 0; i<readInt; i++)
			scanf(" %c", &str[i]);
		str[readInt]='\0';
		sprintf(str, "%s\n\r", str);
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
