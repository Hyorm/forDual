// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "base/basic_functions.h"
#include "libcrown/bin_expression_writer.h"

#include <string.h>

#define size_MAX 1024

namespace crown {

BinExprWriter::BinExprWriter(ops::binary_op_t op, SymbolicExprWriter *l, SymbolicExprWriter *r,
		size_t s, Value_t v)
	: SymbolicExprWriter(s,v), binary_op_(op), left_(l), right_(r) {
	}

BinExprWriter::~BinExprWriter() {
	delete left_;
	delete right_;
}

BinExprWriter* BinExprWriter::Clone() const {
	return new BinExprWriter(binary_op_, left_->Clone(), right_->Clone(),
			size(), value());
}
void BinExprWriter::AppendToString(string *s) const {
	s->append("(");
	s->append(kBinaryOpStr[binary_op_]);
	s->append(" ");
	left_->AppendToString(s);
	s->append(" ");
	right_->AppendToString(s);
	s->append(")");
}

//#define DEBUG

void BinExprWriter::Serialize(ostream &os) const {
	SymbolicExprWriter::Serialize(os,kBinaryNodeTag);
	os.write((char *)&binary_op_, sizeof(char));send_server((char *)&binary_op_,"char");
	left_->Serialize(os);
	right_->Serialize(os);
}

void BinExprWriter::send_server(char* message, char* type) const{

	//printf("send_server_Bin message: .%s. type: .%s.\n", message, type);

        char cmd[size_MAX];

        memset(cmd, 0x00, size_MAX);

	sprintf(cmd,"%s%s%s%s%s","tcp_client 2 \"",message,"\" \"", type,"\"");

	system(cmd);
}

}  // namespace crown
