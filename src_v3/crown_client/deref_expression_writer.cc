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
	os.write((char*)&managerIdx, sizeof(size_t));send_server_deref("size_t", strlen("size_t"));send_server_deref((char*)&managerIdx, sizeof(size_t));
	os.write((char*)&snapshotIdx, sizeof(size_t));send_server_deref( "size_t", strlen("size_t"));send_server_deref((char*)&snapshotIdx,sizeof(size_t));

//	object_->Serialize(os);
	addr_->Serialize(os);
}

void DerefExprWriter::send_server_deref(char* message, int cap) const{

    //printf("send_server_Deref message: \n");

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
	len = cap ;

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

