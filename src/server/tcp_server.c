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
#define type_num 8

void error_handling(char *message);

int main(int argc, char ** argv){

	int svr_sock, clt_sock;
	int clnt_addr_size;
	int file_bin;

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;

	char buf[MAX];
	char type[MAX];

	char* type_list[type_num] = {"size_t","long long", "char", "int", "value_t", "Value_t","addr_t", "unsigned int"};
	int type_size[type_num] = {sizeof(size_t),sizeof(long long),sizeof(char),sizeof(int),sizeof(long long int), 24,sizeof(unsigned long int), sizeof(unsigned int)};

	if(argc<1){

		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	file_bin = open("szd_execution", O_WRONLY | O_CREAT|O_TRUNC , 0700);
	close(file_bin);

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
//while(1){	
	while(1){
		clt_sock=accept(svr_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
		//printf("New Client Connect : %s\n", inet_ntoa(clnt_addr.sin_addr)); 
	
		if(clt_sock !=-1){
			
			file_bin = open("szd_execution", O_WRONLY | O_CREAT|O_APPEND , 0700);		
			memset(buf, 0x00, MAX);

			read(clt_sock, buf, MAX);
			//printf("buf: %s\n",buf);
		
			if(strcmp(buf,"szd_done")==0){
				close(file_bin);
				close(clt_sock);
				close(svr_sock);
				break;
			}

			send(clt_sock, buf, strlen(buf), 0);

			if(strcmp(buf,"crown_NULL")==0) memset(buf, 0x00, MAX);

			memset(type, 0x00, MAX);

			read(clt_sock, type, MAX);
			//printf("type: %s\n",type);

			int idx = -1;

			for(int i = 0; i < type_num; i++){
				if(strcmp(type,type_list[i])==0){
					write(file_bin, buf, type_size[i]);
					idx = i;	
				}
			}
			if(idx==-1){
				if(strchr(type,'*')!=NULL){
					char* type_tok;
					char* re_tok;
					type_tok = strtok(type,"*");
					re_tok = strtok(NULL,"*");
					
					//printf("type_tok:%s, re_tok:%s\n",type_tok, re_tok);	

					for(int i = 0; i < type_num; i++){
						if(strcmp(re_tok,type_list[i])==0){
							write(file_bin, buf, type_size[i] * atoi(type_tok));
						}
					}
				}
				else write(file_bin, buf, atoi(type));
			}
			send(clt_sock, type, strlen(type), 0);
			
			close(file_bin);
		}
		close(clt_sock);

	}
/*	printf("Quit? (y/n) >");
	char qq;
	scanf("%s", &qq);
	if(qq == 'y'){
		close(svr_sock);
		return 0;
	}
}*/
	close(svr_sock);
	return 0;

}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
