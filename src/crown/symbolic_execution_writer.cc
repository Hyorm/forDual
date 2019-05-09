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

#include "libcrown/symbolic_execution_writer.h"

#define size_MAX 1024

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

	os.write((char*)&len, sizeof(len)); send_server_exec((char*)&len, "size_t");

	for (VarIt i = vars_.begin(); i != vars_.end(); ++i) {
        //i->first means index of variable.
        //i->second means type of variable.
		name_len = var_names_[i->first].length();
		os.write((char *)&name_len, sizeof(name_len)); send_server_exec((char*)&name_len, "size_t");

		memset(nbr, 0x00, size_MAX);
		sprintf(nbr, "%d", name_len);
		os.write((char*)(var_names_[i->first].c_str()), name_len); send_server_exec((char*)(var_names_[i->first].c_str()), nbr);



		os.write((char*) &(values_[i->first]), sizeof(long long)); send_server_exec((char*) &(values_[i->first]), "long long");
		os.write((char*) &(indexSize_[i->first]), sizeof(char)); send_server_exec((char*) &(indexSize_[i->first]), "char");
		os.write((char*) &(h_[i->first]), sizeof(char));  send_server_exec((char*) &(h_[i->first]), "char");
		os.write((char*) &(l_[i->first]), sizeof(char)); send_server_exec((char*) &(l_[i->first]), "char");

		exprs_[i->first]->Serialize(os);
		os.write((char*)&(locations_[i->first].lineno), sizeof(int)); send_server_exec((char*)&(locations_[i->first].lineno), "int");

    name_len = (locations_[i->first].fname).length();
		os.write((char*)&name_len, sizeof(name_len)); send_server_exec((char*)&name_len, "size_t");

		memset(nbr, 0x00, size_MAX);
		sprintf(nbr, "%d", name_len);
    os.write((char*)((locations_[i->first].fname).c_str()), name_len);send_server_exec((char*)((locations_[i->first].fname).c_str()), nbr);

		char ch = static_cast<char>(i->second);
		os.write(&ch, sizeof(char)); send_server_exec(&ch, "char");

		if(i->second == types::FLOAT){
			float tmp = (float)inputs_[i->first].floating;

			memset(nbr, 0x00, size_MAX);
			sprintf(nbr, "%d", kSizeOfType[i->second]);
			os.write((char*)&tmp, kSizeOfType[i->second]);send_server_exec((char*)&tmp, nbr);
		}else if(i->second == types::DOUBLE){
			memset(nbr, 0x00, size_MAX);
			sprintf(nbr, "%d", kSizeOfType[i->second]);
			os.write((char*)&inputs_[i->first].floating, kSizeOfType[i->second]); send_server_exec((char*)&inputs_[i->first].floating, nbr);
		}else{
			os.write((char*)&inputs_[i->first].integral, sizeof(value_t));send_server_exec((char*)&inputs_[i->first].integral, "value_t");
		}

	}
	// Write symbolic objects from snapshotManager_
	tracker->Serialize(os);	

	// Write the path.
	path_.Serialize(os);


}

void SymbolicExecutionWriter::send_server_exec(char* message, char* type) const{

	printf("send_server_exec\n");

	char cmd[size_MAX];

	memset(cmd, 0x00, size_MAX);

	//printf("message: .%s., type: .%s.\n", message, type);
	sprintf(cmd,"%s%s%s%s%s","tcp_client 2 \"",message,"\" \"", type,"\"");

	system(cmd);
}

}  // namespace crown
