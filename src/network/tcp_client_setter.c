#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <fcntl.h>
#include <sys/types.h> 
#include <errno.h> 
#include <sys/stat.h>
#include <unistd.h> 
#define MAX 1024 

void error_handling(char *message);

int main(int argc, char** argv) { 
	int sock; 
	struct sockaddr_in serv_addr;
	

	char buf[MAX]; 
	int readbytes; 
	int i, len; 
	int file_bin;  
	int left_size; 
	int totaln = 0; 
	printf("111\n");
	/*if(argc!=2){
                printf("Usage : %s <IP> <port> <file name>\n", argv[0]);
                exit(1);
        }
	*/
        sock=socket(PF_INET, SOCK_STREAM, 0);

        if(sock == -1)
                error_handling("socket() error");
	printf("222\n");
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
        serv_addr.sin_port=htons(atoi(argv[2]));

        if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
                error_handling("connect() error!");//connect
	printf("333\n");
	/*Recive input*/
	if(argv[3]==NULL){	
		send(sock, "2", strlen("2"), 0); 
		file_bin = open("input", O_WRONLY | O_CREAT, 0700); 
		if(!file_bin) { 
			perror("Error: FILE Open Error"); 
			return 1; 
		} 
		if(errno == EEXIST) { 
			perror("Error: FILE Already Exist "); 
			close(file_bin); 
			return 1; 
		} 
		printf("Done\n"); 
	
		while(1) {
			memset(buf, 0x00, MAX); 
			left_size = read(sock, buf, MAX); 
			printf("size: %d\n", left_size);
			write(file_bin, buf, left_size); 
			if(left_size == EOF | left_size == 0) { 
				printf("finish file\n"); 
				break; 
			}

		} 
		close(file_bin); 
	}/*Send File*/
	else{
		memset(buf, 0x00, MAX); 
		send(sock, argv[3], strlen(argv[3]), 0); 
		file_bin = open(argv[3], O_RDONLY); 
		if(!file_bin) { 
			perror("Error : "); 
			return 1; 
		}
		memset(buf, 0x00, MAX);
		read(sock, buf, MAX);
		printf("%s\n", buf); 
		while(1) { 
			memset(buf, 0x00, MAX); 
			left_size = read(file_bin, buf, MAX); 
			printf("\nread : %s",buf); 
			write(sock, buf, left_size);
			if(left_size == EOF | left_size == 0) { 
				printf("finish file\n"); 
				break; 
			}
		}
		close(file_bin);
	}
	close(sock); 
	return 0; 
}
void error_handling(char *message) {
        fputs(message, stderr);
        fputc('\n', stderr);
        exit(1);
}
