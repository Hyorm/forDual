// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <assert.h>
#include "base/basic_functions.h"
#include "libcrown/pred_expression_writer.h"
#include <cstring>
#include <string>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <string.h>

#define ipAddress "192.168.0.150"
#define portNum "3030"
#define size_MAX 1024

namespace crown {

PredExprWriter::PredExprWriter(ops::compare_op_t op, SymbolicExprWriter *l, SymbolicExprWriter *r, size_t s, Value_t v)
	: SymbolicExprWriter(s, v), compare_op_(op), left_(l), right_(r) {
//	std::cerr<<"new PredExprWriter "<<op<<" "<<v.floating<<"\n";
}

PredExprWriter::~PredExprWriter() {
	delete left_;
	delete right_;
}

PredExprWriter* PredExprWriter::Clone() const {
	return new PredExprWriter(compare_op_, left_->Clone(), right_->Clone(),
			size(), value());
}
#if 0
void PredExprWriter::AppendVars(set<var_t>* vars) const {
	left_->AppendVars(vars);
	right_->AppendVars(vars);
}

bool PredExprWriter::DependsOn(const map<var_t,type_t>& vars) const {
	return left_->DependsOn(vars) || right_->DependsOn(vars);
}
#endif
void PredExprWriter::AppendToString(string *s) const {
	s->append("(");
	s->append(kCompareOpStr[compare_op_]);
	s->append(" ");
	left_->AppendToString(s);
	s->append(" ");
	right_->AppendToString(s);
	s->append(")");
}


void PredExprWriter::Serialize(ostream &os) const {

	SymbolicExprWriter::Serialize(os, kCompareNodeTag);
	os.write((char*)&compare_op_, sizeof(char)); send_server((char*)&compare_op_); send_server("char");
	left_->Serialize(os);
	right_->Serialize(os);
}

void PredExprWriter::send_server(char* message) const{

printf("send_server_Pred message: .%s.\n", message);
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
	len = strlen(message) ;

	s = 0 ;
	while (len > 0 && (s = send(sockFD, data, len, 0)) > 0) {
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
