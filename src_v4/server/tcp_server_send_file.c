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

void cnct_server(int port);
void cnct_client(int port);

void send_file(int port, char* file_name);

int svr_sock;
int clt_sock;

int main(int argc, char** argv){

	if(argc <1){
                 printf("Usage : %s [port: number] [file name]\n", argv[0]);
                 exit(1);
        }

	cnct_server(atoi(argv[1]));

	send_file(atoi(argv[1]), argv[2]);

	close(svr_sock);
		

}
void error_handling(char *message) {
        fputs(message, stderr);
        fputc('\n', stderr);
        exit(1);
}
  


void cnct_server(int port){

        struct sockaddr_in serv_addr;

        svr_sock=socket(PF_INET, SOCK_STREAM, 0);

        if(svr_sock == -1)
                error_handling("socket() error");

        memset(&serv_addr, 0, sizeof(serv_addr));

        serv_addr.sin_family=AF_INET;
        serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
        serv_addr.sin_port=htons(port);

	const int on = 1;
        if (setsockopt(svr_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
        
		error_handling("bind() error");
	}

	if(bind(svr_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
		error_handling("bind() error");


        if(listen(svr_sock, 5)==-1)
                error_handling("listen() error");
}
void cnct_client(int port){

	int clnt_addr_size;
	struct sockaddr_in clnt_addr;

        clnt_addr_size=sizeof(clnt_addr);

        while(1){
                clt_sock=accept(svr_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
                //printf("not yet...\n");
                if(clt_sock !=-1)break;

        }
                //error_handling("accept() error");
}

void send_file(int port, char* file_name){

        int file_bin, file_size, left_size;

        char buf[MAX];

        struct stat file_info;

        cnct_client(port);
	//printf("file name:.%s.\n", file_name);

        file_bin = open(file_name, O_RDONLY);

        if(!file_bin) {
                perror("Error : ");
                exit(1);
        }
	stat(file_name, &file_info);

        send(clt_sock, file_name,strlen(file_name),0);

        memset(buf, 0x00, MAX);

        recv(clt_sock, buf,MAX,0);

	file_size = file_info.st_size;

        sprintf(buf,"%ld",file_info.st_size);

	//printf("buf(file_size) %d, %d\n", atoi(buf), file_size);
        send(clt_sock, buf,strlen(buf),0);

        memset(buf, 0x00, MAX);

        recv(clt_sock, buf,MAX,0);

	if(file_size != 0){
	
        	while(1) {
        	        memset(buf, 0x00, MAX);
        	        left_size = read(file_bin, buf, MAX);
			//printf("ho\n");
        	        send(clt_sock, buf, left_size,0);//file content
	
	                if(left_size == EOF | left_size == 0) {
	                        break;
	                }
	        }
	}
        memset(buf, 0x00, MAX);

        recv(clt_sock, buf,MAX,0);

        close(file_bin);

        close(clt_sock);
	

        
}
