#include <cstring>    // sizeof()
#include <iostream>
#include <string>   
#include <sstream>

#include <fstream>
// headers for socket(), getaddrinfo() and friends
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <unistd.h>    // close()
#define MAX 1024
#define type_num 8

int main(int argc, char *argv[])
{
	const char* type_list[type_num] = {"size_t","long long", "char", "int", "value_t", "Value_t","addr_t", "unsigned int"};
        int type_size[type_num] = {sizeof(size_t),sizeof(long long),sizeof(char),sizeof(int),sizeof(long long int), 24,sizeof(unsigned long int), sizeof(unsigned int)};
    if (argc != 2) {
        std::cerr << "Run program as 'program <port>'\n";
        return -1;
    }
  std::ofstream out("szd_execution", std::ios::out | std::ios::binary); 
    auto &portNum = argv[1];
    const unsigned int backLog = 8;  // number of connections allowed on the incoming queue


    addrinfo hints, *res, *p;    // we need 2 pointers, res to hold and p to iterate over
    memset(&hints, 0, sizeof(hints));

    hints.ai_family   = AF_UNSPEC;    // don't specify which IP version to use yet
    hints.ai_socktype = SOCK_STREAM;  // SOCK_STREAM refers to TCP, SOCK_DGRAM will be?
    hints.ai_flags    = AI_PASSIVE;


    int gAddRes = getaddrinfo(NULL, portNum, &hints, &res);
    if (gAddRes != 0) {
        std::cerr << gai_strerror(gAddRes) << "\n";
        return -2;
    }

    unsigned int numOfAddr = 0;
    char ipStr[INET6_ADDRSTRLEN];    // ipv6 length makes sure both ipv4/6 addresses can be stored in this variable


    for (p = res; p != NULL; p = p->ai_next) {
        void *addr;
        std::string ipVer;
        if (p->ai_family == AF_INET) {
            ipVer             = "IPv4";
            sockaddr_in *ipv4 = reinterpret_cast<sockaddr_in *>(p->ai_addr);
            addr              = &(ipv4->sin_addr);
            ++numOfAddr;
            inet_ntop(p->ai_family, addr, ipStr, sizeof(ipStr));
		break;
	}
    }

    if (!numOfAddr) {
        std::cerr << "Found no host address to use\n";
        return -3;
    }

    unsigned int choice = 1;
    bool madeChoice     = true;

    p = res;

    int sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockFD == -1) {
        std::cerr << "Error while creating socket\n";
        freeaddrinfo(res);
        return -4;
    }

    const int on = 1;
        if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {

		fprintf(stderr, "socket error: %s\n", strerror(errno));

	}

    int bindR = bind(sockFD, p->ai_addr, p->ai_addrlen);
    if (bindR == -1) {
        std::cerr << "Error while binding socket\n";
        
        close(sockFD);
        freeaddrinfo(res);
        return -5;
    }


    char buf[MAX];
    char type[MAX];
	int numnum = 1;
	char * data = 0x0, * tdata = 0x0 ;
   	int s ; 
	int len = 0 ;
	int inin;
   while (1) {
	int listenR = listen(sockFD, backLog);
	if (listenR == -1) {
	        std::cerr << "Error while Listening on socket\n";
	        close(sockFD);
	        freeaddrinfo(res);
	        return -6;
	    }

	sockaddr_storage client_addr;
	socklen_t client_addr_size = sizeof(client_addr);

        int newFD = accept(sockFD, (sockaddr *) &client_addr, &client_addr_size);
        if (newFD == -1) {
            std::cerr << "Error while Accepting on socket\n";
            continue;
        }
	if(numnum == 1){

		data = 0x0;
		len = 0;
		memset(buf, 0x00, MAX);
	inin = 0;
	while ( (s = recv(newFD, buf, MAX-1, 0)) > 0 ) {
		buf[s] = 0x0 ;
		inin = 1;
		if (data == 0x0) {
			data = strdup(buf) ;
			len = s ;
		}
		else {
			data = (char*)realloc(data, len + s + 1) ;
			strncpy(data + len, buf, s) ;
			data[len + s] = 0x0 ;
			len += s ;
		}
	}

	len = strlen("hoho");
	while (len > 0 && (s = send(newFD, "hoho",len, 0)) > 0) {
		//data += s ;
		len -= s ;
	}
	shutdown(newFD, SHUT_WR) ;



	numnum = 0;

	if(data == NULL)printf("hehe\n");

	if(inin==1){
		sprintf(buf, "%s", data);
		if(strcmp(buf, "szd_done")==0){
			close(newFD);
			break;
		}
	}
	}
	else{
	numnum = 1;

	len = 0 ;
	memset(type, 0x00, MAX);
	tdata = 0x0 ;

	while ( (s = recv(newFD, type, MAX-1, 0)) > 0 ) {
		type[s] = 0x0 ;
		if (tdata == 0x0) {
			tdata = strdup(type) ;
			len = s ;
		}
		else {
			tdata = (char*)realloc(tdata, len + s + 1) ;
			strncpy(tdata + len, type, s) ;
			tdata[len + s] = 0x0 ;
			len += s ;
		}
	}
	printf("type: %s\n", tdata) ;

	int idx = -1;

                        for(int i = 0; i < type_num; i++){
                                if(strcmp(tdata,type_list[i])==0){
					printf("write\n");
                                        out.write(data, type_size[i]);
					printf("w\n");
                                        idx = i;
                                }
                        }
                        if(idx==-1){
                                if(strchr(tdata,'*')!=NULL){
                                        char* type_tok;
                                        char* re_tok;
                                        type_tok = strtok(tdata,"*");
                                        re_tok = strtok(NULL,"*");

                                        //printf("type_tok:%s, re_tok:%s\n",type_tok, re_tok);

                                        for(int i = 0; i < type_num; i++){
                                                if(strcmp(re_tok,type_list[i])==0){
							printf("write\n");
                                                        out.write(data, type_size[i] * atoi(type_tok));
                                                	printf("w\n");
						}
                                        }
                                }
                                else {printf("write\n");out.write(data, atoi(tdata));printf("w\n");}
                        }

	len = strlen(tdata);
	while (len > 0 && (s = send(newFD, tdata, len, 0)) > 0) {
		tdata += s ;
		len -= s ;
	}
	shutdown(newFD, SHUT_WR) ;
	}
    }

   out.close();
    close(sockFD);
    freeaddrinfo(res);

    return 0;
}
