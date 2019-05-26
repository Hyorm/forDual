// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.
 
#include <assert.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>

#include "libcrown/symbolic_expression_writer.h"
#include "libcrown/unary_expression_writer.h"
#include "libcrown/bin_expression_writer.h"
#include "libcrown/pred_expression_writer.h"
#include "libcrown/atomic_expression_writer.h"
#include "libcrown/symbolic_object_writer.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef DEBUG
#define IFDEBUG(x) x
#else
#define IFDEBUG(x)
#endif

#define size_MAX 1024
#define ipAddress "192.168.0.150"
#define portNum "3030"

namespace crown {

typedef map<var_t,value_t>::iterator It;
typedef map<var_t,value_t>::const_iterator ConstIt;

size_t SymbolicExprWriter::next = 0;

SymbolicExprWriter::~SymbolicExprWriter() { }

SymbolicExprWriter* SymbolicExprWriter::Clone() const {
	return new SymbolicExprWriter(size_, value_);
}

void SymbolicExprWriter::AppendToString(string* s) const {
	assert(IsConcrete());

	char buff[92];
	if(value().type == types::FLOAT ||value().type ==types::DOUBLE){
		std::ostringstream fp_out;
		fp_out<<value().floating;
		strcpy(buff, fp_out.str().c_str());
		s->append(buff);
	}else{
		sprintf(buff, "%lld", value().integral);
		s->append(buff);
	}
}


void SymbolicExprWriter::Serialize(ostream &os) const {
	SymbolicExprWriter::Serialize(os, kConstNodeTag);
}

void SymbolicExprWriter::Serialize(ostream &os, char c) const {

	int hyorm = 0;
	char nbr[size_MAX]; memset(nbr, 0x00, size_MAX);

	//printf("expr %d\n",hyorm++);
	//os.write((char*)&value_, sizeof(Value_t)); 

	os.write((char*)&(value_.integral),sizeof(value_t));	
	send_server_expr( "Value_t",strlen("Value_t"));
	
	send_server_expr((char*)&(value_.integral),sizeof(value_t));
	
	memset(nbr, 0x00, size_MAX);
	sprintf(nbr,"%lf", value_.floating);

	os.write((char*)&(value_.floating),sizeof(fp_value_t));
	send_server_expr(nbr,strlen(nbr));
	//send_server_expr((char*)&(value_.floating),sizeof(fp_value_t));
	sprintf(nbr, "%d", sizeof(type_t));
	send_server_expr(nbr, strlen(nbr)); 

	os.write((char*)&(value_.type),sizeof(type_t));
	memset(nbr, 0x00, size_MAX);
	os.write(nbr,8-sizeof(type_t));
	send_server_expr((char*)&(value_.type),sizeof(type_t));

	//printf("expr %d\n",hyorm++);
	os.write((char*)&size_, sizeof(size_t)); send_server_expr( "size_t", strlen("size_t"));send_server_expr((char*)&size_,sizeof(size_t));
	//printf("expr %d\n",hyorm++);
	os.write((char*)&unique_id_, sizeof(size_t)); send_server_expr( "size_t", strlen("size_t"));send_server_expr((char*)&unique_id_, sizeof(size_t));

	IFDEBUG(std::cerr<<"SerializeInSymExpr: "<<value_.type<<" "<<value_.integral<<" "<<value_.floating<<" size:"<<(size_t)size_<<" nodeTy:"<<(int)c<<" id:"<<unique_id_<<std::endl);
	
	//printf("expr %d\n",hyorm++);
	os.write(&c, sizeof(char)); send_server_expr( "char", strlen("char")); send_server_expr(&c, sizeof(char)+1); 
}

void SymbolicExprWriter::send_server_expr(char* message, int cap) const {

	//printf("send_server_expr message: \n");
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
