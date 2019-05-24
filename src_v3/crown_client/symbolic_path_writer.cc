// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include "libcrown/symbolic_path_writer.h"
#include<assert.h>
#include <string.h>
#include<iostream>
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

namespace crown {

SymbolicPathWriter::SymbolicPathWriter() { }

SymbolicPathWriter::SymbolicPathWriter(bool pre_allocate) {
	if (pre_allocate) {
		// To cut down on re-allocation.
		branches_.reserve(4000000);
		constraints_idx_.reserve(50000);
		constraints_.reserve(50000);
	}
}

SymbolicPathWriter::SymbolicPathWriter(const SymbolicPathWriter &p)
	: branches_(p.branches_),
	constraints_idx_(p.constraints_idx_),
	constraints_(p.constraints_) {
		for(size_t i = 0; i < p.constraints_.size(); i++)
			constraints_[i] = p.constraints_[i]->Clone();
	}


SymbolicPathWriter::~SymbolicPathWriter() {
	for (size_t i = 0; i < constraints_.size(); i++)
		delete constraints_[i];
}

void SymbolicPathWriter::Swap(SymbolicPathWriter& sp) {
	branches_.swap(sp.branches_);
	constraints_idx_.swap(sp.constraints_idx_);
	constraints_.swap(sp.constraints_);
}

void SymbolicPathWriter::Push(branch_id_t bid) {
	branches_.push_back(bid);
}

/*
 * comments written by Hyunwoo Kim (17.07.13)
 * line number and filename of branch are added as parameters.
 * received information is saved in variable line_ and fname_.
 */

void SymbolicPathWriter::Push(branch_id_t bid, SymbolicExprWriter* constraint,
		unsigned int lineno, const char *filename) {
	bool flag = true;
	if (constraint) {
//FIXME
#if 0
		for(size_t i =0; i < constraints_.size(); i++)
			if(constraints_[i]->Equals(*constraint))
				flag = false;
#endif
		if(flag) {
            mutable_locations()->push_back(Loc_t(std::string(filename),lineno));

			constraints_.push_back(constraint);
			constraints_idx_.push_back(branches_.size());
		}
	}
	branches_.push_back(bid);
}

void SymbolicPathWriter::Push(branch_id_t bid, SymbolicExprWriter* constraint, bool pred_value,
		unsigned int lineno, const char *filename, const char *exp){
	Push(bid, constraint, lineno, filename);
	//branch_info_.push_back(branch_info_t(pred_value, lineno, filename, exp));
}


void SymbolicPathWriter::Serialize(ostream &os) const{

	char nbr[size_MAX];

	typedef vector<SymbolicExprWriter*>::const_iterator ConIt;

	// Write the path.
	size_t len = branches_.size();

	os.write((char*)&len, sizeof(len)); send_server_path( "size_t", strlen("size_t")); send_server_path((char*)&len, sizeof(len));
	memset(nbr, 0x00, size_MAX);
	sprintf(nbr, "%d%s", branches_.size(),"*int");
	os.write((char*)&branches_.front(), branches_.size() * sizeof(branch_id_t)); send_server_path( nbr, strlen(nbr));send_server_path((char*)&branches_.front(), branches_.size() * sizeof(branch_id_t));

	// Write the path constraints.
	len = constraints_.size();
	os.write((char*)&len, sizeof(len)); send_server_path( "size_t", strlen("size_t"));send_server_path((char*)&len, sizeof(len));

	memset(nbr, 0x00, size_MAX);
	sprintf(nbr, "%d%s", constraints_.size(),"*size_t");

	os.write((char*)&constraints_idx_.front(), constraints_.size() * sizeof(size_t)); send_server_path( nbr, strlen(nbr));send_server_path((char*)&constraints_idx_.front(), constraints_.size() * sizeof(size_t));
    
    /*
     * comments written by Hyunwoo Kim (17.07.13)
     * line number and filename of branch are added saved in file <ostream &os>.
     */

    vector<Loc_t>::const_iterator j = locations_.begin();
	for (ConIt i = constraints_.begin(); i != constraints_.end(); ++i, ++j) {
		os.write((char*)&(j->lineno) , sizeof(int));send_server_path("int", strlen("int"));send_server_path((char*)&(j->lineno), sizeof(int));
		len = (j->fname).length();
		os.write((char*)&len, sizeof(len));send_server_path("size_t", strlen("size_t"));send_server_path((char*)&len, sizeof(len));


		memset(nbr, 0x00, size_MAX);
		sprintf(nbr, "%d", len);

		os.write((char*)((j->fname).c_str()), len);send_server_path( nbr, strlen(nbr));send_server_path((char*)((j->fname).c_str()), len);

		(*i)->Serialize(os);
	}
}

void SymbolicPathWriter::send_server_path(char* message, int cap) const {

	//printf("send_server_path message: \n");
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
