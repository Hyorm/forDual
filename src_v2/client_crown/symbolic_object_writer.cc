// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <assert.h>
#include <z3.h>
#include <string.h>
#include "libcrown/symbolic_object_writer.h"
#include "libcrown/symbolic_expression_factory.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <string>

#define size_MAX 1024
#define OPT2METHOD
#define ipAddress "192.168.0.150"
#define portNum "3030"

using namespace std;

namespace crown {

SymbolicObjectWriter::SymbolicObjectWriter(addr_t start, size_t size, size_t managerIdx)
	: start_(start), size_(size), managerIdx_(managerIdx),
		snapshotIdx_(0),  writes_() {  }

SymbolicObjectWriter::SymbolicObjectWriter(const SymbolicObjectWriter &obj)
	: start_(obj.start_), size_(obj.size_), managerIdx_(obj.managerIdx_),
#ifdef OPT2METHOD
	snapshotIdx_(obj.snapshotIdx_+1), mem_(obj.mem_), writes_(){
#else
	snapshotIdx_(obj.snapshotIdx_+1), mem_(obj.mem_), writes_(obj.writes_){
	for (vector<Write>::iterator it = writes_.begin(); 
									it != writes_.end(); ++it) {
		it->first = it->first->Clone();
		it->second = it->second->Clone();
	}
#endif
}


SymbolicObjectWriter::~SymbolicObjectWriter() {
	for (vector<Write>::iterator it = writes_.begin(); 
									it != writes_.end(); ++it) {
	//	delete it->first;
	//	delete it->second;
	}
}


SymbolicExprWriter* SymbolicObjectWriter::read(addr_t addr, Value_t val) {
	if (writes_.size() == 0) {
		// No symbolic writes yes, so normal read.
		return mem_.read(addr, val);
	} else {
		// There have been symbolic writes, so return a deref.
		return SymbolicExprWriterFactory::NewConstDerefExprWriter(val, *this, addr);
	}
}


void SymbolicObjectWriter::write(SymbolicExprWriter* sym_addr, addr_t addr,
		SymbolicExprWriter* e) {
#if 0
	if ((writes_.size() == 0) && ((sym_addr == NULL) ||
		 sym_addr->IsConcrete())) {
		// Normal write.
		mem_.write(addr, e);
	} else {
#endif

	std::vector<Write>::iterator it = find_if(writes_.begin(), writes_.end(), WriteAddrIs(addr));
	// There have been symbolic writes, so record this write.
	if (sym_addr == NULL) {
		sym_addr = SymbolicExprWriterFactory::NewConcreteExpr(Value_t(addr, 0, types::U_LONG));
	}
/*
	if(it != writes_.end()){
		//replace it second element (assigned symbolic expr)
		it->first = sym_addr;
		it->second = e;
	}else{
*/
		//Insert new expr which the addr does not used yet.
		writes_.push_back(make_pair(sym_addr, e));
//	}
}

#if 0
void SymbolicObjectWriter::concretize(SymbolicExprWriter* sym_addr, addr_t addr, size_t n) {
	if ((writes_.size() == 0) && ((sym_addr == NULL) || sym_addr->IsConcrete())) {
		// Normal write.
		mem_.concretize(addr, n);
	} else {
		// There have been symbolic writes, so record this write.
		// TODO: Don't know how to do this yet.
		assert(false);
		//mem_.concretize(addr, n);
	}
}
#endif


void SymbolicObjectWriter::Serialize(ostream &os) const {
	// Format is:
	// start_ | size_ | mem_ | writes_.size() | writes[0] | ... |writes[n-1]
	// writes[n] == (SymExpr1, SymExpr2)
#ifdef DEBUG
	printf("Serialize obj: start %d, size %d, writes_size %d\n",start_, size_, writes_.size());
#endif
	// Not keeping tracks of symbolic writes
	//assert(writes_.size() == 0);

	os.write((char*)&start_, sizeof(start_)); send_server_object((char*)&start_, "addr_t");
	os.write((char*)&size_, sizeof(size_));send_server_object((char*)&size_, "size_t");
	os.write((char*)&managerIdx_, sizeof(managerIdx_));send_server_object((char*)&managerIdx_, "size_t");
	os.write((char*)&snapshotIdx_, sizeof(snapshotIdx_));send_server_object((char*)&snapshotIdx_, "size_t");
	mem_.Serialize(os);
	size_t size = writes_.size();
	os.write((char*)&size, sizeof(size)); send_server_object((char*)&size, "size_t");
	for (size_t i=0; i<writes_.size(); i++){
		writes_[i].first->Serialize(os);
		writes_[i].second->Serialize(os);
	}

}

void SymbolicObjectWriter::send_server_object(char* message, char* type) const {

	printf("send_server_object message: .%s. type: .%s.\n", message, type);

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

void SymbolicObjectWriter::Dump() const {
	mem_.Dump();
}

}  // namespace crown

