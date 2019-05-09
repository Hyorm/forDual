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

#define size_MAX 1024

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

	//printf("send_server_Atomic message: .%s. type: .%s.\n", message, type);

        char cmd[size_MAX];

        memset(cmd, 0x00, size_MAX);

        sprintf(cmd,"%s%s%s%s%s","tcp_client 2 \"",message,"\" \"", type,"\"");

        system(cmd);
}

}  // namespace crown
