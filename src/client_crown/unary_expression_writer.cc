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

	//printf("send_server_Unary message: .%s. type: .%s.\n", message, type);

        char cmd[size_MAX];

        memset(cmd, 0x00, size_MAX);

	message= replace_all(message, "\'", "\\\'");
        message= replace_all(message, "\"", "\\\"");
        message = replace_all(message, "\`", "\\\`");

        sprintf(cmd,"%s%s%s%s%s","tcp_client 2 \"",message,"\" \"", type,"\"");

        system(cmd);
}

char* UnaryExprWriter::replace_all(char* s, const char *olds, const char*news)const{
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
