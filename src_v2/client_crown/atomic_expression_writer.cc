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

	os.write((char*)&var_, sizeof(var_t));send_server((char*)&var_,"unsigned int");
}

void AtomicExprWriter::send_server(char* message, char* type) const{

	printf("send_server_Atomic message: .%s. type: .%s.\n", message, type);

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

    char buf[size_MAX];
    memset(buf, 0x00, size_MAX);


    //if(strlen(message) == 0)
    if(message==NULL)
            auto bytes_sent = send(sockFD, "crown_NULL", strlen("crown_NULL"), 0);
    else
        auto bytes_sent = send(sockFD, message, strlen(message), 0);

    memset(buf, 0x00, size_MAX);
    auto bytes_recv = recv(sockFD, buf, size_MAX, 0);
    if (bytes_recv == -1) {
        std::cerr << "Error while receiving bytes\n";
        exit(1);
    }
    auto bytes_sent = send(sockFD, type, strlen(type), 0);

    memset(buf, 0x00, size_MAX);
    bytes_recv = recv(sockFD, buf, size_MAX, 0);
    if (bytes_recv == -1) {
        std::cerr << "Error while receiving bytes\n";
        exit(1);
    }

    close(sockFD);
    freeaddrinfo(p);
}


}  // namespace crown
