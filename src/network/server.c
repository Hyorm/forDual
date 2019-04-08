#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#define MAX 1024

void error_handling(char *message);
void send_file(int file_bin, int clt_sock);

int main(int argc, char ** argv){

	int svr_sock, clt_sock;
	int file_bin;
	int left_size;
	int clnt_addr_size;
	int size_file;
	struct stat file_info;

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;

	char buf[MAX];
	char message[MAX];
	char* message_cat;
	char name[MAX];

	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	//<1 DONE: Bind Server Socket
	svr_sock=socket(PF_INET, SOCK_STREAM, 0);

	if(svr_sock == -1)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));

	if(bind(svr_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
		error_handling("bind() error");

	if(listen(svr_sock, 5)==-1)
		error_handling("listen() error");
	

	//<2 DONE: Accept Client Socket
	clnt_addr_size=sizeof(clnt_addr);
	clt_sock=accept(svr_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);

	if(clt_sock==-1)
		error_handling("accept() error");

	sleep(5);

	//<3 DONE: Receive File and Message - Execute run_crown
	while(1){
		memset(buf, 0x00, MAX);
		read(clt_sock, buf, MAX);
		strcpy(message, buf);
		//printf("%s\n",message);
		if(strcmp(message, "finish")==0)
			break;

		if(strstr(message, ".c")!=NULL)sprintf(name, "%s%s%s","crownc ",message," 2>crown_result > crown_result2");
		system("rm *.cil.c");
		file_bin = open(message, O_WRONLY | O_CREAT |O_TRUNC, 0700);
		if(!file_bin) {
			perror("file open error : ");
			exit(1);
		}else{
			write(clt_sock, "start", strlen("start"));
		}

		memset(buf,0x00,MAX);
		read(clt_sock,buf,MAX);
		size_file = atoi(buf);
		write(clt_sock,"ok",strlen("ok"));

		while(1) {
			memset(buf, 0x00, MAX);
			left_size = read(clt_sock, buf, MAX);
			write(file_bin, buf, left_size);
			stat(message,&file_info);
			if(file_info.st_size >=size_file){
				write(clt_sock, "good", strlen("good"));
				break;
			}
		}
		close(file_bin);
	}
	printf("%s\n",name);
	system(name);
	write(clt_sock, "1", strlen("1")); //server <2>

	memset(buf, 0x00, MAX);
	left_size = read(clt_sock, buf, MAX);
	//printf("buf:%s\n",buf);
	if(left_size > 0) {
		strcpy(message, buf);
		printf("%s > %s\n", inet_ntoa(clnt_addr.sin_addr), message);
	}
	else{
		write(clt_sock, "-1", strlen("-1"));
		close(clt_sock);
	}
	write(clt_sock, "1", strlen("1"));

	//<4 DONE: Execute Run_Crown
	memset(buf, 0x00, MAX);
	sprintf(buf, "%s%s","run_crown ./", message);
	printf("%s\n",buf);
	system(buf);//4>//3>


	//<5 DONE: Send Result File

	/*
	printf("input for gcov send\n");

	send(clt_sock, "input_for_gcov", strlen("input_for_gcov"), 0);

	file_bin = open("input_for_gcov", O_RDONLY);

	if(!file_bin){
		perror("file open error : ");
		exit(1);
	}

	memset(buf, 0x00, MAX);
	read(clt_sock, buf, MAX);

	send_file(file_bin, clt_sock);

	memset(buf, 0x00, MAX);
	read(clt_sock, buf, MAX);
	*/
	/*
	printf("result from crown send\n");

	send(clt_sock, "result_from_crown", strlen("result_from_crown"), 0);

	file_bin = open("result_from_crown", O_RDONLY);

	if(!file_bin){
		perror("file open error : ");
		exit(1);
	}

	memset(buf, 0x00, MAX);
	read(clt_sock, buf, MAX);

	send_file(file_bin, clt_sock);
	
	memset(buf, 0x00, MAX);
	read(clt_sock, buf, MAX);

	close(file_bin);
	*/
	close(clt_sock);//5>//2>

	close(svr_sock);
	return 0;//1>
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void send_file(int file_bin, int clt_sock) {

	char buf[MAX];
	int file_size;
	while(1) {
		memset(buf, 0x00, MAX);
		file_size = read(file_bin, buf, MAX);
		printf("\nread : %s",buf);
		write(clt_sock, buf, file_size);
		if(file_size == EOF | file_size == 0) {
			printf("finish file\n");
			break;
		}
	}
}
