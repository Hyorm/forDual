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
#define crown_ip "192.168.0.150"
#define crown_port 2024

void error_handling(char *message);

int main(int argc, char** argv){

	int sock;
	int file_bin;
	int size_file;
	int left_size;

	struct stat file_info;

	struct sockaddr_in serv_addr;
	
	char buf[MAX];

	//<1 DONE: Connection Server Socket
	//
	if(argc <3){
		printf("Usage : %s option []\noption - 1: receive file, [file name(input or result)]\n         2: send piece of file, [message:char*] [type:char*]\n         3: send file, [file name:*.c]\n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);

	if(sock == -1)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(crown_ip);
	serv_addr.sin_port=htons(crown_port);

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");

	//<2 TODO: Recevie file
	if(atoi(argv[1])==1){

		file_bin = open(argv[3], O_WRONLY | O_CREAT |O_TRUNC, 0700);
		
		if(!file_bin) {
			perror("Error: FILE Open Error");
			return 1;
		}
		
		send(sock, argv[2], strlen(argv[2]), 0);//file name

		memset(buf,0x00,MAX);
		
		read(sock, buf,MAX);//file size
		
		size_file = atoi(buf);

		send(sock, "ok",strlen("ok"),0);

		while(1) {

			memset(buf, 0x00, MAX);

			left_size = read(sock, buf, MAX);//file content

			stat(argv[3],&file_info);
		
			if(file_info.st_size >= size_file){
				send(sock, "done", strlen("done"),0);
				break;
			}

			send(file_bin, buf, left_size,0);
		}
		close(file_bin);
	
	}
	//2>
	//<3 TODO: Send piece of file
	else if(atoi(argv[1])==2){
		printf("argv[2]: %s\n",argv[2]);
		printf("argv[3]: %s\n",argv[3]);
		
		if(strcmp(argv[2],"")==0){
			char* crown_NULL = "crown_NULL";
			send(sock, crown_NULL, strlen(crown_NULL), 0);
		}
		else{
			send(sock, argv[2], strlen(argv[2]), 0);
		}
		memset(buf,0x00,MAX);

		read(sock, buf,MAX);

		send(sock, argv[3], strlen(argv[3]), 0);

		memset(buf,0x00,MAX);

		read(sock, buf,MAX);

	}
	//3>
	//<4 TODO: Send file
	else if(atoi(argv[1])==3){

		file_bin = open(argv[2], O_RDONLY);

		stat(argv[2], &file_info);

		if(!file_bin) {

			perror("Error : ");
			return 1;
		}

		send(sock, argv[3], strlen(argv[3]), 0);//file name

		memset(buf, 0x00, MAX);

		recv(sock, buf, MAX,0);

		memset(buf,0x00,MAX);

		sprintf(buf,"%ld",file_info.st_size);

		send(sock, buf,strlen(buf),0);//file size

		memset(buf, 0x00, MAX);

		recv(sock, buf,MAX,0);

		while(1) {
			memset(buf, 0x00, MAX);
			
			left_size = read(file_bin, buf, MAX);

			send(sock, buf, left_size,0);//file content

			if(left_size == EOF | left_size == 0) {
				break;
			}
		}
		memset(buf, 0x00, MAX);

		recv(sock, buf, MAX,0);

		close(file_bin);
	}
	//4>
	else error_handling("Wrong Option!");

	close(sock);
	return 0;//1>

}

void error_handling(char* message){

	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);

}
