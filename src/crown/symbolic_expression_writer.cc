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

#ifdef DEBUG
#define IFDEBUG(x) x
#else
#define IFDEBUG(x)
#endif

#define size_MAX 1024

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

	os.write((char*)&value_, sizeof(Value_t)); send_server_expr((char*)&value_, "Value_t");
	os.write((char*)&size_, sizeof(size_t)); send_server_expr((char*)&size_, "size_t");
	os.write((char*)&unique_id_, sizeof(size_t)); send_server_expr((char*)&unique_id_, "size_t");

	IFDEBUG(std::cerr<<"SerializeInSymExpr: "<<value_.type<<" "<<value_.integral<<" "<<value_.floating<<" size:"<<(size_t)size_<<" nodeTy:"<<(int)c<<" id:"<<unique_id_<<std::endl);
	
	os.write(&c, sizeof(char)); send_server_expr(&c, "char");
}

void SymbolicExprWriter::send_server_expr(char* message, char* type) const {

	//printf("send_server_expr message: .%s. type: .%s.\n", message, type);

        char cmd[size_MAX];

        memset(cmd, 0x00, size_MAX);
	
	message= replace_all(message, "\'", "\\\'");
	message= replace_all(message, "\"", "\\\"");
	message = replace_all(message, "\`", "\\\`");
	//printf("message: %s\n", message);	
	sprintf(cmd,"%s%s%s%s%s","tcp_client 2 \"",message,"\" \"", type,"\"");
        system(cmd);
}

char* SymbolicExprWriter::replace_all(char* s, const char *olds, const char*news)const{
	char *result, *sr;
	size_t i , count = 0;
	size_t old_len = strlen(olds);
	size_t new_len = strlen(news);

	if(old_len<1) return s;
	
	if(new_len != old_len){
	
		for(i = 0; s[i]!='\0';){
			
			if(memcmp(&s[i], olds, old_len)==0){
			       	count++; 
				i+= old_len;
			}else i++;
		}
	}else i = strlen(s);

	result = (char*)malloc(i+1 + count*(new_len - old_len));

	if(result == NULL) return NULL;

	sr = result;

	while(*s){
		if(memcmp(s, olds, old_len)==0){
			memcpy(sr, news, new_len);
			sr += new_len;
			s += old_len;
		}else *sr++ = *s++;
	}

	*sr = '\0';
	return result;
}

}  // namespace crown
