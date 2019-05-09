// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <string.h>
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "libcrown/unary_expression_writer.h"

#define size_MAX 1024

#ifdef DEBUG
#define IFDEBUG(x) x
#else
#define IFDEBUG(x)
#endif

namespace crown {

UnaryExprWriter::UnaryExprWriter(ops::unary_op_t op, SymbolicExprWriter *c, size_t s, Value_t v)
  : SymbolicExprWriter(s, v), child_(c), unary_op_(op) { }

UnaryExprWriter::~UnaryExprWriter() {
	delete child_;
}

UnaryExprWriter* UnaryExprWriter::Clone() const {
	return new UnaryExprWriter(unary_op_, child_->Clone(), size(), value());
}
void UnaryExprWriter::AppendToString(string *s) const {
	s->append("(");
	s->append(kUnaryOpStr[unary_op_]);
	if (unary_op_ == ops::SIGNED_CAST || unary_op_ == ops::UNSIGNED_CAST){
		char buff[32];
		sprintf(buff, "[%d]", size()*8);
		s->append(buff);
	}
	s->append(" ");
	child_->AppendToString(s);
	s->append(")");
}

void UnaryExprWriter::Serialize(ostream &os) const {
	SymbolicExprWriter::Serialize(os, kUnaryNodeTag);
	os.write((char*)&unary_op_, sizeof(char));send_server((char*)&unary_op_,"char");
	child_->Serialize(os);
}

void UnaryExprWriter::send_server(char* message, char* type) const{

	printf("send_server_Unary message: .%s. type: .%s.\n", message, type);

        char cmd[size_MAX];

        memset(cmd, 0x00, size_MAX);

        sprintf(cmd,"%s%s%s%s%s","tcp_client 2 \"",message,"\" \"", type,"\"");

        system(cmd);
}

}  // namespace crown
