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
#define crown_tcp_port 3030

void error_handling(char *message);

int cnct_server(int port);

void recv_file(int port);
void recv_msg(int port);
void send_file(int port, char* file_name);
void send_msg(int port, char* message);

char buf_msg[MAX];
char cur_pwd[MAX];

int main(int argc, char** argv){

	char cmd[MAX];
	//char* home = getenv("HOME");

	if(argc <3){
		 printf("Usage : %s [file name:*.c] [iteration: number] [method: -dfs /-cfg/etc] [port: number]\n", argv[0]);
		 exit(1);
	}

	//memset(cur_pwd,0x00, MAX);
        //sprintf(cur_pwd,"%s%s",home,argv[5]);
	

	//<1 TODO: Crownc Target File
	
	memset(cmd, 0x00, MAX);
	sprintf(cmd,"%s%s%s","crownc ",argv[1],".c 2>hide.txt");
	system(cmd);

	//1>	


	//<2 TODO: Send Option

	send_msg(atoi(argv[4]), argv[1]);
	send_msg(atoi(argv[4]), argv[2]);
	send_msg(atoi(argv[4]), argv[3]);	

	//2>
	

	//<3 TODO: Send Target file
	memset(cmd, 0x00, MAX);
	sprintf(cmd,"%s%s",argv[1],".c");
	send_file(atoi(argv[4]), cmd);

	//3>
	
	//<4 TODO: Wait for Server Execute Cil
	while(1){
		recv_msg(crown_tcp_port);
	
		if(strcmp(buf_msg, "exec cil")==0){

			recv_file(crown_tcp_port);
			//printf("input done!\n");
			memset(cmd, 0x00, MAX);
			sprintf(cmd,"%s%s","./",argv[1]);
			system(cmd);
		
		}
		else if(strcmp(buf_msg, "recv input")==0){
		
			recv_file(crown_tcp_port);
	
		}
		else if(strcmp(buf_msg,"test test done")==0){
			//printf("done!\n");
			break;	
	
		}else{
			printf("hi");
		}
	
	}
	//4>
	//recv result
	recv_file(atoi(argv[4]));
	
	//print result
	//printf("test done\n");
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
		printf("Wait...\n");
	}

	return sock;
}

void recv_file(int port){
	
	int file_bin, file_size, left_size;

	struct stat file_info;

	char buf[MAX];
	char recv_file_name[MAX];
	char recv_name[MAX];

	int sock = cnct_server(port);

	memset(recv_file_name, 0x00, MAX);
	read(sock, recv_file_name, MAX);

	//memset(recv_file_name, 0x00,MAX);
	//sprintf(recv_file_name, "%s%s",cur_pwd,recv_name);

	file_bin = open(recv_file_name, O_WRONLY | O_CREAT |O_TRUNC, 0700);
                
        if(!file_bin) {
		perror("Error: FILE Open Error");
		exit(1);
	}
                
	send(sock, recv_file_name, strlen(recv_file_name), 0);//file name
	
	memset(buf,0x00,MAX);
                
	read(sock, buf,MAX);//file size
                
	file_size = atoi(buf);
	//printf("%d\n",file_size);

	send(sock, "ok",strlen("ok"),0);

	if(file_size != 0){
		while(1){

			memset(buf, 0x00, MAX);

			left_size = read(sock, buf, MAX);//file content

			write(file_bin, buf, left_size);
			printf("%s\n", buf);	
               
			stat(recv_file_name, &file_info);

			if(file_info.st_size >= file_size){
				break;
			}
		}
	}
	//printf("hello\n");
	send(sock, "done", strlen("done"),0);
	
	close(file_bin);
	close(sock);
	//printf("file done!\n");

}

void recv_msg(int port){

	int sock = cnct_server(port);

	memset(buf_msg,0x00,MAX);
	read(sock, buf_msg,MAX);

	//printf("buf_msg: .%s.\n", buf_msg);
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
	//printf("recv file_name\n", file_name);

	memset(buf, 0x00, MAX);
	sprintf(buf,"%ld", file_info.st_size );

	send(sock, buf,strlen(buf),0);//file size

	memset(buf, 0x00, MAX);

	recv(sock, buf,MAX,0);//ok

	while(1) {
		memset(buf, 0x00, MAX);
		left_size = read(file_bin, buf, MAX);
		
		write(sock, buf, strlen(buf));//file content
		if(left_size == EOF | left_size == 0) {
			//printf("done\n");
			break;
		}
	}
	memset(buf, 0x00, MAX);

	recv(sock, buf, MAX,0);
	//printf("after buf: %s\n");
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
