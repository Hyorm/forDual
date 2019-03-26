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

void sendFile(int file_bin, int clt_sock);
void waitFile(int clt_sock, int file_bin);
 
int main(int argc, char **argv) {
	int svr_sock, clt_sock;
	int client_len;
	char buf[MAX];
	char message[MAX];
	int file_bin;
	int getset;
	int bind_check;
	int left_size;	
	struct sockaddr_in serv_addr;
        struct sockaddr_in clnt_addr;
        int clnt_addr_size;

	if(argc!=2){
        	printf("Usage : %s <port>\n", argv[0]);
                exit(1);
        }

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

        clnt_addr_size=sizeof(clnt_addr);
        clt_sock=accept(svr_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);

        if(clt_sock==-1)
                error_handling("accept() error");

	sleep(5);

	printf("222\n");

	
	memset(buf, 0x00, MAX);

	/*receive_file name*/
	left_size = read(clt_sock, buf, MAX);
        if(left_size > 0) {
		strcpy(message, buf);
		printf("%s > %s\n", inet_ntoa(clnt_addr.sin_addr), message);
	}else{
                close(clt_sock);
        }       

	if(atoi(message)==2){
		/*input file send*/
		printf("send\n");
		file_bin = open("input", O_RDONLY); 
		if(!file_bin) {
			perror("file open error : ");
		}
		printf("send input\n");
		sendFile(file_bin, clt_sock);
	}
	else{
		printf("recv\n");
        	
		/* create file */
        	while(1){
        		file_bin = open(message, O_WRONLY | O_CREAT, 0700);
        	 	if(!file_bin) {
        	                perror("file open error : ");
        	                exit(1);
                	}else{
                        	break;
                	}
		}
                waitFile(clt_sock, file_bin);

	}
	close(clt_sock);
	close(file_bin);

	close(svr_sock);
	return 0;
}

void sendFile(int file_bin, int clt_sock){
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

void waitFile(int clt_sock, int file_bin){

	char buf[MAX];
	int left_size;
	write(clt_sock, "start", MAX);
	/* file save */
        while(1) {
       		memset(buf, 0x00, MAX);
                left_size = read(clt_sock, buf, MAX);
                write(file_bin, buf, left_size);

                if(left_size == EOF | left_size == 0) {
                	printf("finish file\n"); break;
                }
	}

}

void error_handling(char *message) {
        fputs(message, stderr);
        fputc('\n', stderr);
        exit(1);
}
