// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include "libcrown/symbolic_path_writer.h"
#include<assert.h>
#include <string.h>
#include<iostream>

#define size_MAX 1024

namespace crown {

SymbolicPathWriter::SymbolicPathWriter() { }

SymbolicPathWriter::SymbolicPathWriter(bool pre_allocate) {
	if (pre_allocate) {
		// To cut down on re-allocation.
		branches_.reserve(4000000);
		constraints_idx_.reserve(50000);
		constraints_.reserve(50000);
	}
}

SymbolicPathWriter::SymbolicPathWriter(const SymbolicPathWriter &p)
	: branches_(p.branches_),
	constraints_idx_(p.constraints_idx_),
	constraints_(p.constraints_) {
		for(size_t i = 0; i < p.constraints_.size(); i++)
			constraints_[i] = p.constraints_[i]->Clone();
	}


SymbolicPathWriter::~SymbolicPathWriter() {
	for (size_t i = 0; i < constraints_.size(); i++)
		delete constraints_[i];
}

void SymbolicPathWriter::Swap(SymbolicPathWriter& sp) {
	branches_.swap(sp.branches_);
	constraints_idx_.swap(sp.constraints_idx_);
	constraints_.swap(sp.constraints_);
}

void SymbolicPathWriter::Push(branch_id_t bid) {
	branches_.push_back(bid);
}

/*
 * comments written by Hyunwoo Kim (17.07.13)
 * line number and filename of branch are added as parameters.
 * received information is saved in variable line_ and fname_.
 */

void SymbolicPathWriter::Push(branch_id_t bid, SymbolicExprWriter* constraint,
		unsigned int lineno, const char *filename) {
	bool flag = true;
	if (constraint) {
//FIXME
#if 0
		for(size_t i =0; i < constraints_.size(); i++)
			if(constraints_[i]->Equals(*constraint))
				flag = false;
#endif
		if(flag) {
            mutable_locations()->push_back(Loc_t(std::string(filename),lineno));

			constraints_.push_back(constraint);
			constraints_idx_.push_back(branches_.size());
		}
	}
	branches_.push_back(bid);
}

void SymbolicPathWriter::Push(branch_id_t bid, SymbolicExprWriter* constraint, bool pred_value,
		unsigned int lineno, const char *filename, const char *exp){
	Push(bid, constraint, lineno, filename);
	//branch_info_.push_back(branch_info_t(pred_value, lineno, filename, exp));
}


void SymbolicPathWriter::Serialize(ostream &os) const{

	char nbr[size_MAX];

	typedef vector<SymbolicExprWriter*>::const_iterator ConIt;

	// Write the path.
	size_t len = branches_.size();

	memset(nbr, 0x00, size_MAX);
	sprintf(nbr, "%d%s", branches_.size(),"*int");
	os.write((char*)&len, sizeof(len)); send_server_path((char*)&len, "size_t"); 
	os.write((char*)&branches_.front(), branches_.size() * sizeof(branch_id_t)); send_server_path((char*)&branches_.front(), nbr);

	// Write the path constraints.
	len = constraints_.size();
	os.write((char*)&len, sizeof(len)); send_server_path((char*)&len, "size_t");

	memset(nbr, 0x00, size_MAX);
	sprintf(nbr, "%d%s", constraints_.size(),"*size_t");

	os.write((char*)&constraints_idx_.front(), constraints_.size() * sizeof(size_t)); send_server_path((char*)&constraints_idx_.front(), nbr);
    
    /*
     * comments written by Hyunwoo Kim (17.07.13)
     * line number and filename of branch are added saved in file <ostream &os>.
     */

    vector<Loc_t>::const_iterator j = locations_.begin();
	for (ConIt i = constraints_.begin(); i != constraints_.end(); ++i, ++j) {
		os.write((char*)&(j->lineno) , sizeof(int));send_server_path((char*)&(j->lineno), "int");
		len = (j->fname).length();
		os.write((char*)&len, sizeof(len));send_server_path((char*)&len, "size_t");


		memset(nbr, 0x00, size_MAX);
		sprintf(nbr, "%d", len);

		os.write((char*)((j->fname).c_str()), len);send_server_path((char*)((j->fname).c_str()), nbr);

		(*i)->Serialize(os);
	}
}

void SymbolicPathWriter::send_server_path(char* message, char* type) const {

	//printf("send_server_path\n");

        char cmd[size_MAX];

        memset(cmd, 0x00, size_MAX);

	sprintf(cmd,"%s%s%s%s%s","tcp_client 2 \"",message,"\" \"", type,"\"");	

        system(cmd);
}

}  // namespace crown
