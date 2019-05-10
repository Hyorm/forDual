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

void error_handling(char *message);

int cnct_server(int port);

void recv_file(int port);
void recv_msg(int port);
void send_file(int port, char* file_name);
void send_msg(int port, char* message);

char buf_msg[MAX];

int main(int argc, char** argv){

	char cmd[MAX];

	if(argc <3){
		 printf("Usage : %s [file name:*.c] [iteration: number] [method: -dfs /-cfg/etc] [port: number]\n", argv[0]);
		 exit(1);
	}

	//<1 TODO: Crownc Target File
	
	memset(cmd, 0x00, MAX);
	sprintf(cmd,"%s%s%s","crownc ",argv[1],".c ");
	system(cmd);

	//1>	


	//<2 TODO: Send Option

	send_msg(atoi(argv[4]), argv[1]);
	send_msg(atoi(argv[4]), argv[2]);
	send_msg(atoi(argv[4]), argv[3]);	

	//2>
	

	//<3 TODO: Send Target file
	memset(cmd, 0x00, MAX);
	sprintf(cmd,"%s%s",argv[1],".c ");
	send_file(atoi(argv[4]), cmd);

	//3>
	
	//<4 TODO: Wait for Server Execute Cil
	while(1){
		
		recv_msg(atoi(argv[4]));
	
		if(strcmp(buf_msg, "exec cil")==0){
	
			memset(cmd, 0x00, MAX);
			sprintf(cmd,"%s%s","./",argv[1]);
		
		}
		if(strcmp(buf_msg, "recv input")==0){
		
			recv_file(atoi(argv[4]));
	
		}
		else{
			//recv result
		
			//print result
	
		}
	
	}
	//4>
}

void error_handling(char* message){

        fputs(message, stderr);
        fputc('\n', stderr);
        exit(1);

}

int cnct_server(int port){

	int sock;

        struct sockaddr_in serv_addr;

	sock = socket(PF_INET, SOCK_STREAM, 0);

	if(sock == -1)
                error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_addr.s_addr=inet_addr(crown_ip);
        serv_addr.sin_port=htons(port);

	while(1){
		if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) !=-1)break;
		printf("not yet...\n");
	}

	return sock;
}

void recv_file(int port){
	
	int file_bin, file_size, left_size;

	struct stat file_info;

	char buf[MAX];
	char recv_file_name[MAX];

	int sock = cnct_server(port);

	memset(recv_file_name, 0x00, MAX);
	read(sock, recv_file_name, MAX);

	file_bin = open(recv_file_name, O_WRONLY | O_CREAT |O_TRUNC, 0700);
                
        if(!file_bin) {
		perror("Error: FILE Open Error");
		exit(1);
	}
                
	send(sock, recv_file_name, strlen(recv_file_name), 0);//file name
	
	memset(buf,0x00,MAX);
                
	read(sock, buf,MAX);//file size
                
	file_size = atoi(buf);

	send(sock, "ok",strlen("ok"),0);

	while(1){

		memset(buf, 0x00, MAX);

		left_size = read(sock, buf, MAX);//file content

		stat(recv_file_name,&file_info);
                
		if(file_info.st_size >= file_size){
			send(sock, "done", strlen("done"),0);
			break;
		}

		send(file_bin, buf, left_size,0);
	}
	close(file_bin);
	close(sock);

}

void recv_msg(int port){

	int sock = cnct_server(port);

	memset(buf_msg,0x00,MAX);
	read(sock, buf_msg,MAX);

	send(sock, "buf_msg",strlen("buf_msg"),0);

	close(sock);
}

void send_file(int port, char* file_name){

	int file_bin, file_size, left_size;

        char buf[MAX];

	int sock = cnct_server(port);

	struct stat file_info;

	file_bin = open(file_name, O_RDONLY);
	
	stat(file_name, &file_info);

	if(!file_bin) {
		perror("Error : ");
		exit(1);
	}

	send(sock, file_name,strlen(file_name),0);//file name

	memset(buf, 0x00, MAX);

	recv(sock, buf,MAX,0);

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

	close(sock);
}
void send_msg(int port, char* message){

	int sock = cnct_server(port);

	char buf[MAX];

        send(sock, message,strlen(message),0);

	memset(buf,0x00,MAX);
        read(sock, buf,MAX);

	close(sock);
}
