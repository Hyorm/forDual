// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <utility>
#include <iostream>
#include <iomanip>
#include <string.h>
#include <climits>
#include <assert.h>
#include <cstring>
#include <string>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "libcrown/symbolic_execution_writer.h"

#define size_MAX 1024
#define ipAddress "192.168.0.150"
#define portNum "3030"

namespace crown {

SymbolicExecutionWriter::SymbolicExecutionWriter() { }

SymbolicExecutionWriter::SymbolicExecutionWriter(bool pre_allocate)
  : path_(pre_allocate) { }

SymbolicExecutionWriter::~SymbolicExecutionWriter() { }

void SymbolicExecutionWriter::Swap(SymbolicExecutionWriter& se) {
	vars_.swap(se.vars_);
	values_.swap(se.values_);
	indexSize_.swap(se.indexSize_);
	h_.swap(se.h_);
	l_.swap(se.l_);
    exprs_.swap(se.exprs_);
	inputs_.swap(se.inputs_);
	path_.Swap(se.path_);
}

void SymbolicExecutionWriter::Serialize(ostream &os, const ObjectTrackerWriter* tracker) const{
	typedef map<var_t,type_t>::const_iterator VarIt;
	// Write the inputs.
    /* comments written by Hyunwoo Kim (17.07.13)
     * variable name_len was added to temporally save the length
     * of symbolic variable name.
     * 
     * Additionally, the length of symbolic variable,
     * the name of symbolic variable,
     * linu of symbolic declaration,
     * the name of file and its length are written in <ostream &os>.
     *
     * This information is used in print_execution.
     */
	size_t len = vars_.size();
	size_t name_len;

	char nbr[size_MAX]; 

	//int hyorm = 0;
	//printf("exec %d\n",hyorm++);
	os.write((char*)&len, sizeof(len)); send_server_exec("size_t", strlen("size_t"));send_server_exec((char*)&len, sizeof(len));

	for (VarIt i = vars_.begin(); i != vars_.end(); ++i) {
        //i->first means index of variable.
        //i->second means type of variable.
		name_len = var_names_[i->first].length();
		//printf("exec %d\n",hyorm++);
		os.write((char *)&name_len, sizeof(name_len));send_server_exec("size_t", strlen("size_t"));send_server_exec((char*)&name_len, sizeof(name_len));

		memset(nbr, 0x00, size_MAX);
		sprintf(nbr, "%d", name_len);
		//printf("exec %d\n",hyorm++);
		os.write((char*)(var_names_[i->first].c_str()), name_len);send_server_exec(nbr, strlen(nbr));send_server_exec((char*)(var_names_[i->first].c_str()), name_len);



		//printf("exec %d\n",hyorm++);
		os.write((char*) &(values_[i->first]), sizeof(long long)); send_server_exec("long long", strlen("long long"));send_server_exec((char*) &(values_[i->first]),sizeof(long long));
		//printf("exec %d\n",hyorm++);
		os.write((char*) &(indexSize_[i->first]), sizeof(char)); send_server_exec("char", strlen("char"));send_server_exec((char*) &(indexSize_[i->first]), sizeof(char)+1);
		//printf("exec %d\n",hyorm++);
		os.write((char*) &(h_[i->first]), sizeof(char)); send_server_exec("char", strlen("char"));send_server_exec((char*) &(h_[i->first]),  sizeof(char)+1);
		//printf("exec %d\n",hyorm++);
		os.write((char*) &(l_[i->first]), sizeof(char)); send_server_exec("char", strlen("char"));send_server_exec((char*) &(l_[i->first]), sizeof(char)+1);

		exprs_[i->first]->Serialize(os);
		//printf("exec %d\n",hyorm++);
		os.write((char*)&(locations_[i->first].lineno), sizeof(int)); send_server_exec("int", strlen("int"));send_server_exec((char*)&(locations_[i->first].lineno), sizeof(int));

    name_len = (locations_[i->first].fname).length();
	        //printf("exec %d\n",hyorm++);
    		os.write((char*)&name_len, sizeof(name_len)); send_server_exec( "size_t", strlen("size_t"));send_server_exec((char*)&name_len, sizeof(name_len));

		memset(nbr, 0x00, size_MAX);
		sprintf(nbr, "%d", name_len);
		//printf("exec %d\n",hyorm++);
    os.write((char*)((locations_[i->first].fname).c_str()), name_len);send_server_exec( nbr, strlen(nbr));send_server_exec((char*)((locations_[i->first].fname).c_str()), name_len);

		char ch = static_cast<char>(i->second);
		//printf("data &ch:  .%c.\n", &ch);
		//printf("exec %d\n",hyorm++);
		os.write(&ch, sizeof(char)); send_server_exec( "char", strlen("char"));send_server_exec(&ch, sizeof(char)+1);

		if(i->second == types::FLOAT){
			float tmp = (float)inputs_[i->first].floating;

			memset(nbr, 0x00, size_MAX);
			sprintf(nbr, "%d", kSizeOfType[i->second]);
			//printf("exec %d\n",hyorm++);
			os.write((char*)&tmp, kSizeOfType[i->second]);send_server_exec( nbr, strlen(nbr));send_server_exec((char*)&tmp, kSizeOfType[i->second]);
		}else if(i->second == types::DOUBLE){
			memset(nbr, 0x00, size_MAX);
			sprintf(nbr, "%d", kSizeOfType[i->second]);
			//printf("exec %d\n",hyorm++);
			os.write((char*)&inputs_[i->first].floating, kSizeOfType[i->second]); send_server_exec( nbr,strlen(nbr));send_server_exec((char*)&inputs_[i->first].floating, kSizeOfType[i->second]);
		}else{
			//printf("exec %d\n",hyorm++);
			os.write((char*)&inputs_[i->first].integral, sizeof(value_t));send_server_exec( "value_t", strlen("value_t"));send_server_exec((char*)&inputs_[i->first].integral, sizeof(value_t));
		}

	}
	// Write symbolic objects from snapshotManager_
	tracker->Serialize(os);	

	// Write the path.
	path_.Serialize(os);


}

void SymbolicExecutionWriter::send_server_exec(char* message, int cap) const{

	//if(message == NULL)printf("hehe\n");
	//printf("send_server_exec message: \n");
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
