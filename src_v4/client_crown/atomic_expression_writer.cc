// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include "libcrown/atomic_expression_writer.h"
#include "base/basic_types.h"
#include <iostream>
#include <cstdio>

#include <string.h>

#include <cstring>
#include <iostream>
#include <string>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define size_MAX 1024
#define ipAddress "192.168.0.150"
#define portNum "3030"

typedef unsigned int var_t;
namespace crown {

AtomicExprWriter::AtomicExprWriter(size_t size, Value_t val, var_t var)
	: SymbolicExprWriter(size, val), var_(var) { }

AtomicExprWriter* AtomicExprWriter::Clone() const {
	return new AtomicExprWriter(size(), value(), var_);
}

void AtomicExprWriter::AppendToString(string* s) const {
	char buff[32];
	sprintf(buff, "x%u", var_);
	s->append(buff);
}

void AtomicExprWriter::Serialize(ostream &os) const {

	//printf("Atomic\n");

	SymbolicExprWriter::Serialize(os, kBasicNodeTag);
	//int hyorm = 0;
	//printf("atomic %d\n",hyorm++);
	os.write((char*)&var_, sizeof(var_t));send_server_atomic("unsigned int", strlen("unsigned int"));send_server_atomic((char*)&var_, sizeof(var_t));
}

void AtomicExprWriter::send_server_atomic(char* message, int cap) const{

	//printf("send_server_Atomic message: \n");

	    addrinfo hints, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    int gAddRes = getaddrinfo(ipAddress, portNum, &hints, &p);
    if (gAddRes != 0) {
        std::cerr << gai_strerror(gAddRes) << "\n";
        exit(1);
    }

    if (p == NULL) {
        std::cerr << "No addresses found\n";
        exit(1);
    }

    int sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockFD == -1) {
        std::cerr << "Error while creating socket\n";
        exit(1);
    }

    int connectR = connect(sockFD, p->ai_addr, p->ai_addrlen);
    if (connectR == -1) {
        close(sockFD);
        std::cerr << "Error while connecting socket\n";
        exit(1);
    }

    	int s, len ;
	char * data ;

	char buf[size_MAX];
memset(buf, 0x00, size_MAX);
        data = message ;
	len = cap;

	s = 0 ;
	while (len > 0 && (s = send(sockFD, data, len, 0)) > 0) {
		//printf("data: .%s.\n",data);// len: .%d.\n", data, len);
		data += s ;
		len -= s ;
	}

	shutdown(sockFD, SHUT_WR) ;

	data = 0x0 ;
	len = 0 ;

	while ( (s = recv(sockFD, buf, size_MAX-1, 0)) > 0 ) {
		buf[s] = 0x0 ;
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

    close(sockFD);
    freeaddrinfo(p);
}


}  // namespace crown
