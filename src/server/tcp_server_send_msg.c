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

void cnct_client(int port);

void send_msg(int port, char* message);

int svr_sock;
int clt_sock;

int main(int argc, char** argv){

	if(argc <1){
                 printf("Usage : %s [port: number] [file name]\n", argv[0]);
                 exit(1);
        }

	send_msg(atoi(argv[1]), argv[2]);

}
void error_handling(char *message) {
        fputs(message, stderr);
        fputc('\n', stderr);
        exit(1);
}
  


void cnct_client(int port){

        int clnt_addr_size;

        struct sockaddr_in serv_addr;
        struct sockaddr_in clnt_addr;

        svr_sock=socket(PF_INET, SOCK_STREAM, 0);

        if(svr_sock == -1)
                error_handling("socket() error");

        memset(&serv_addr, 0, sizeof(serv_addr));

        serv_addr.sin_family=AF_INET;
        serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
        serv_addr.sin_port=htons(port);

        if(bind(svr_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
                error_handling("bind() error");

        if(listen(svr_sock, 5)==-1)
                error_handling("listen() error");

        clnt_addr_size=sizeof(clnt_addr);

        while(1){
                clt_sock=accept(svr_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
                printf("not yet...\n");
                if(clt_sock !=-1)break;

        }
                //error_handling("accept() error");
}
void send_msg(int port, char* message){

        cnct_client(port);

        char buf[MAX];

        send(clt_sock, message,strlen(message),0);

        memset(buf, 0x00, MAX);

        read(clt_sock, buf, MAX);

        close(clt_sock);
        close(svr_sock);
}
