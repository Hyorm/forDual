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

	message= replace_all(message, "\'", "\\\'");
        message= replace_all(message, "\"", "\\\"");
        message = replace_all(message, "\`", "\\\`");
	sprintf(cmd,"%s%s%s%s%s","tcp_client 2 \"",message,"\" \"", type,"\"");

	system(cmd);
}

char* BinExprWriter::replace_all(char* s, const char *olds, const char*news)const{
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
