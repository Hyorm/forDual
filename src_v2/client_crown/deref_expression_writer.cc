//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

/***
 * Author: Sudeep juvekar (sjuvekar@eecs.berkeley.edu)
 * 4/17/09
 */

#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <cstring>

#include <string>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "base/basic_functions.h"
#include "libcrown/deref_expression_writer.h"
#include "libcrown/symbolic_object_writer.h"
#include "libcrown/symbolic_expression_factory.h"

#define size_MAX 1024
#define ipAddress "192.168.0.150"
#define portNum "3030"

namespace crown {

DerefExprWriter::DerefExprWriter(SymbolicExprWriter *c, SymbolicObjectWriter *o,
                     size_t s, Value_t v)
  : SymbolicExprWriter(s,v), managerIdx_(o->managerIdx()), 
	snapshotIdx_(o->snapshotIdx()), object_(o), addr_(c){ }


DerefExprWriter::DerefExprWriter(const DerefExprWriter& de)
  : SymbolicExprWriter(de.size(), de.value()),
	managerIdx_(de.managerIdx_), snapshotIdx_(de.snapshotIdx_),
    object_(de.object_), addr_(de.addr_->Clone()) { }

DerefExprWriter::~DerefExprWriter() {
//  delete object_;
	delete addr_;
}

DerefExprWriter* DerefExprWriter::Clone() const {
  return new DerefExprWriter(*this);
}

void DerefExprWriter::AppendToString(string *s) const {
  s->append(" (*?)");
}

void DerefExprWriter::Serialize(ostream &os) const {
	//printf("serialize Deref\n");
	SymbolicExprWriter::Serialize(os, kDerefNodeTag);

	assert(object_->managerIdx() == managerIdx_);
	assert(object_->snapshotIdx() == snapshotIdx_ && "There are some bugs for assigning snapshotIdx");

	/*
	for(size_t i = 0; i < object_->writes().size(); i++){
		SymbolicExprWriter *exp = object_->writes()[i].second;
	}
	*/

	//Store the indexes to find object on run_crown
	size_t managerIdx = object_->managerIdx();
	size_t snapshotIdx = object_->snapshotIdx();
	os.write((char*)&managerIdx, sizeof(size_t));send_server((char*)&managerIdx,"size_t");
	os.write((char*)&snapshotIdx, sizeof(size_t));send_server((char*)&snapshotIdx, "size_t");

//	object_->Serialize(os);
	addr_->Serialize(os);
}

void DerefExprWriter::send_server(char* message, char* type) const{

    printf("send_server_Deref message: .%s. type: .%s.\n", message, type);

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
    auto ytes_sent = send(sockFD, type, strlen(type), 0);

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

