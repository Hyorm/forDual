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
#define crown_port 8000

void error_handling(char *message);

int main(int argc, char** argv){

	int sock;
	int crown_sock;
	int readbytes;
	int file_bin;
	int left_size;

	FILE *fp;

	struct sockaddr_in serv_addr;
	struct sockaddr_in serv_crown_addr;
	
	char buf[MAX];
	char message[MAX];
	char sysCrown[MAX];

	char* message_p;
	system("rm -rf c1/*");

	if(argc !=6){
		printf("Usage : %s <IP> <port> <file name> <iter number> <-method>\n", argv[0]);
		exit(1);
	}

	//<1 DONE: Connection Server Socket
	sock = socket(PF_INET, SOCK_STREAM, 0);

	if(sock == -1)
		error_handling("socket() error");
		
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");

	//<2 DONE: Execute CROWNC - Send All File
	sprintf(sysCrown,"%s%s%s","crownc ",argv[3],".c\0");
	printf("%s\n", sysCrown);
	
	system(sysCrown);
	system("cp * c2");
	system("diff c1 c2|sed 's/Only in c2: //g' >diff");
	system("sed '/a.out/d' diff >diff_result");
	fp = fopen("diff_result", "r");
	if(fp != NULL){
		while(!feof(fp)){
			memset(buf, 0x00, MAX);
			message_p = fgets(buf, MAX, fp);
			if((message_p = fgets(buf, MAX, fp))==NULL)break;

			send(sock, message_p, strlen(message_p)-1, 0);
			message_p[strlen(message_p)-1] = '\0';
			printf("%s\n",message_p);
			file_bin = open(message_p, O_RDONLY);

			if(!file_bin) {
				perror("Error : ");
				return 1;
			}

			memset(buf, 0x00, MAX);
			read(sock, buf, MAX);//start
			
			while(1) {
				memset(buf, 0x00, MAX);
				left_size = read(file_bin, buf, MAX);
				write(sock, buf, left_size);
				if(left_size == EOF | left_size == 0) {
					break;
				}
			}
			memset(buf, 0x00, MAX);
			read(sock, buf, MAX);//good

			close(file_bin);
		}
		printf("finish\n");
		send(sock, "finish", strlen("finish"), 0);
	}

	memset(buf, 0x00, MAX);
	read(sock, buf, MAX);//2>
	printf("%s\n",buf);
	//<3 DONE: Send Message - Call server run_crown
	memset(message, 0x00, MAX);
	sprintf(message,"%s%s%s%s%s",argv[3]," ",argv[4]," ",argv[5]);
	write(sock, message, strlen(message));

	// if send success 1
	memset(buf, 0x00, MAX);
	read(sock, buf, MAX);//3>

	//<4 TODO: Receive Input File
	//<5 TODO: Connect Server-Crown Socket
	int iter_num = 0;
	while(iter_num < atoi(argv[4])){
		iter_num++;
		crown_sock = socket(PF_INET, SOCK_STREAM, 0);

		if(crown_sock == -1)
			error_handling("socket() error");
		while(1){
			memset(&serv_crown_addr, 0, sizeof(serv_crown_addr));
			serv_crown_addr.sin_family=AF_INET;
			serv_crown_addr.sin_addr.s_addr=inet_addr(argv[1]);
			serv_crown_addr.sin_port=htons(crown_port);

			if(connect(crown_sock, (struct sockaddr*)&serv_crown_addr, sizeof(serv_crown_addr))==-1)
				continue;
			else break;
		}
		memset(buf, 0x00, MAX);
		read(crown_sock, buf, MAX);
		memset(message, 0x00, MAX);
		strcpy(message, buf);

		file_bin = open(message, O_WRONLY | O_CREAT |O_TRUNC, 0700);

		if(!file_bin) {
			perror("Error: FILE Open Error");
			return 1;
		}
		
		send(crown_sock, "ok", strlen("ok"), 0);
		
		while(1) {
			
			memset(buf, 0x00, MAX);
			left_size = read(crown_sock, buf, MAX);
			if(strstr(buf,"NULL")==NULL){
				printf("finish file\n");
				write(crown_sock, "done", strlen("done"));
				break;
			}
			printf(">>%s\n",buf);

			if(left_size == EOF | left_size == 0) {
				printf("finish file\n");
				write(crown_sock, "done", strlen("done"));
				break;
			}

			if(left_size < MAX){
				printf("finish file\n");
				write(crown_sock, "done", strlen("done"));
				break;
			}
			write(file_bin, buf, left_size);
		}
		close(file_bin);

		//<6 DONE: Execute Cil
		memset(buf, 0x00, MAX);
		read(crown_sock, buf, MAX);//good
		system("cp * c1");
		memset(message, 0x00, MAX);
		sprintf(message,"%s%s","./",argv[3]);
		system(message);//6>
		printf("cil exe\n");
		send(crown_sock, "send", strlen("send"), 0);
		//<7 TODO:Send Diff Files
		system("cp * c2");
		system("diff c1 c2|sed 's/Only in c2: //g' >diff");
		system("egrep -o 'szd_execution|coverage|input' diff |uniq >diff_result");
		
		system("mv c2 c1");
		system("rm -rf c2");
		system("mkdir c2");


		fp = fopen("diff_result", "r");
		printf("hey1\n");	
		if(fp != NULL){
			while(!feof(fp)){
				memset(buf, 0x00, MAX);
				printf("hey2\n");
				if((message_p = fgets(buf, MAX, fp))==NULL){
					printf("nothing\n");
					send(crown_sock, "finish", strlen("finish"), 0);
					break;
				}
				printf("hey3\n");
				send(crown_sock, message_p, strlen(message_p)-1, 0);
				message_p[strlen(message_p)-1] = '\0';
				printf("%s\n",message_p);
				
				file_bin = open(message_p, O_RDONLY);

				if(!file_bin) {
					perror("Error : ");
					exit(1);
				}
				memset(buf, 0x00, MAX);
				read(crown_sock, buf, MAX);

				while(1) {
					memset(buf, 0x00, MAX);
					left_size = read(file_bin, buf, MAX);
					printf("read: %s\n",buf);
					write(crown_sock, buf, left_size);
					if(left_size == EOF | left_size == 0) {
						printf("finish file\n");
						break;
					}
				}
				memset(buf, 0x00, MAX);
				read(crown_sock, buf, MAX);//done
				close(file_bin);
			}
			send(crown_sock, "finish", strlen("finish"), 0);
			close(file_bin);
		}//7>
		close(crown_sock);

	}//5>
	//4>
	// TODO: Get Result
	/*
	memset(buf, 0x00, MAX);
	read(sock, buf, MAX);

	file_bin = open(buf, O_WRONLY | O_CREAT |O_TRUNC, 0700);

	if(!file_bin) {
		perror("Error: FILE Open Error");
		return 1;
	}

	send(sock, "ok", strlen("ok"), 0);

	while(1) {
		memset(buf, 0x00, MAX);
		left_size = read(sock, buf, MAX);
		write(file_bin, buf, left_size);
	
		if(left_size == EOF | left_size == 0) {
			printf("finish file\n");
			break;
		}
	}
	close(file_bin);
	*/
	close(sock);
	return 0;//1>
}

void error_handling(char* message){

	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);

}
