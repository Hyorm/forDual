//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

/***
 * Author: Sudeep juvekar (sjuvekar@eecs.berkeley.edu)
 * 4/17/09
 */

#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <cstring>

#include "base/basic_functions.h"
#include "libcrown/deref_expression_writer.h"
#include "libcrown/symbolic_object_writer.h"
#include "libcrown/symbolic_expression_factory.h"

#define size_MAX 1024

namespace crown {

DerefExprWriter::DerefExprWriter(SymbolicExprWriter *c, SymbolicObjectWriter *o,
                     size_t s, Value_t v)
  : SymbolicExprWriter(s,v), managerIdx_(o->managerIdx()), 
	snapshotIdx_(o->snapshotIdx()), object_(o), addr_(c){ }


DerefExprWriter::DerefExprWriter(const DerefExprWriter& de)
  : SymbolicExprWriter(de.size(), de.value()),
	managerIdx_(de.managerIdx_), snapshotIdx_(de.snapshotIdx_),
    object_(de.object_), addr_(de.addr_->Clone()) { }

DerefExprWriter::~DerefExprWriter() {
//  delete object_;
	delete addr_;
}

DerefExprWriter* DerefExprWriter::Clone() const {
  return new DerefExprWriter(*this);
}

void DerefExprWriter::AppendToString(string *s) const {
  s->append(" (*?)");
}

void DerefExprWriter::Serialize(ostream &os) const {
	//printf("serialize Deref\n");
	SymbolicExprWriter::Serialize(os, kDerefNodeTag);

	assert(object_->managerIdx() == managerIdx_);
	assert(object_->snapshotIdx() == snapshotIdx_ && "There are some bugs for assigning snapshotIdx");

	/*
	for(size_t i = 0; i < object_->writes().size(); i++){
		SymbolicExprWriter *exp = object_->writes()[i].second;
	}
	*/

	//Store the indexes to find object on run_crown
	size_t managerIdx = object_->managerIdx();
	size_t snapshotIdx = object_->snapshotIdx();
	os.write((char*)&managerIdx, sizeof(size_t));send_server((char*)&managerIdx,"size_t");
	os.write((char*)&snapshotIdx, sizeof(size_t));send_server((char*)&snapshotIdx, "size_t");

//	object_->Serialize(os);
	addr_->Serialize(os);
}

void DerefExprWriter::send_server(char* message, char* type) const{

        //printf("send_server_Deref message: .%s. type: .%s.\n", message, type);

        char cmd[size_MAX];

        memset(cmd, 0x00, size_MAX);

	message= replace_all(message, "\'", "\\\'");
        message= replace_all(message, "\"", "\\\"");
        message = replace_all(message, "\`", "\\\`");

        sprintf(cmd,"%s%s%s%s%s","tcp_client 2 \"",message,"\" \"", type,"\"");

        system(cmd);
}

char* DerefExprWriter::replace_all(char* s, const char *olds, const char*news)const{
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

