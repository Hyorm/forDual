// Copyright (c) 2009, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <algorithm>
#include <utility>
#include <assert.h>
#include <cstdio>
#include <string.h>
#include "libcrown/symbolic_memory_writer.h"
#include "libcrown/symbolic_expression_writer.h"
#include "libcrown/symbolic_expression_factory.h"
#include <cstring>
#include <iostream>
#include <string>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define size_MAX 1024
#define ipAddress "192.168.0.150"
#define portNum "3030"

using std::make_pair;
using std::min;
using __gnu_cxx::hash_map;

namespace crown {

const size_t SymbolicMemoryWriter::MemElem::kMemElemCapacity;
const size_t SymbolicMemoryWriter::MemElem::kOffsetMask;
const size_t SymbolicMemoryWriter::MemElem::kAddrMask;

SymbolicMemoryWriter::MemElem::MemElem() {
	for (size_t i = 0; i < kMemElemCapacity; i++) {
		slots_[i] = NULL;
	}
}


SymbolicMemoryWriter::MemElem::MemElem(const MemElem& elem) {
	for (size_t i = 0; i < kMemElemCapacity; i++) {
		if (elem.slots_[i] != NULL) {
			slots_[i] = elem.slots_[i]->Clone();
		} else {
			slots_[i] = NULL;
		}
	}
}

SymbolicMemoryWriter::MemElem::MemElem(SymbolicExprWriter** slots) {
	for(size_t i = 0; i < kMemElemCapacity; i++)
		slots_[i] = slots[i];
}

SymbolicMemoryWriter::MemElem::~MemElem() {
	for (size_t i = 0; i < kMemElemCapacity; i++) {
		delete slots_[i];
	}
}


SymbolicExprWriter* SymbolicMemoryWriter::MemElem::read(addr_t addr,
		size_t n,
		Value_t val, 
        const MemElem *next_elem) const {
	size_t i = addr & kOffsetMask;

	{
		size_t j = i;
		while ((j != 0) && (slots_[j] == NULL)) {
			j = j & (j - 1);  
		}

		if ((slots_[j] != NULL)                // found an expression,
				&& (slots_[j]->size() > n)         // the expression is larger,
				&& (j + slots_[j]->size() > i)) {  // and our read overlaps

				return SymbolicExprWriterFactory::ExtractBytes(slots_[j]->Clone(), i - j, n);
		}
	}
	{
		SymbolicExprWriter* ret = NULL;

		size_t j = 0;
		do {
			if (i + j < kMemElemCapacity){
				if (slots_[i + j] != NULL) {
					if (ret == NULL) {
						ret = slots_[i + j]->Clone();
					} else {
						ret = SymbolicExprWriterFactory::Concatenate(ret, slots_[i + j]->Clone());
					}

				} else {
					while ((++j < n) && (slots_[i + j] == NULL)) ;
					if (ret == NULL) {
						if (j == n) return NULL;  // Whole read is concrete.
						ret = SymbolicExprWriterFactory::ExtractBytes(n, val, 0, j);
					} else {
						SymbolicExprWriter* tmp =
							SymbolicExprWriterFactory::ExtractBytes(n, val, ret->size(), j - ret->size());
						ret = SymbolicExprWriterFactory::Concatenate(ret, tmp);
					}
				}

				j = ret->size();
			}else{
				if (next_elem->slots_[i + j - kMemElemCapacity] != NULL){
					if (ret == NULL){
						ret = next_elem->slots_[i + j - kMemElemCapacity]->Clone();
					}else{
						ret = SymbolicExprWriterFactory::Concatenate(ret, 
								next_elem->slots_[i + j - kMemElemCapacity]->Clone());               }
				}else{
					while ((++j < n) && (next_elem->slots_[i + j - kMemElemCapacity] == NULL));
					if (ret == NULL){
						if (j == n) return NULL;
						ret = SymbolicExprWriterFactory::ExtractBytes(n, val, 0, j);
					} else {
						SymbolicExprWriter* tmp = 
							SymbolicExprWriterFactory::ExtractBytes(n, val, ret->size(), j - ret->size());
						ret = SymbolicExprWriterFactory::Concatenate(ret, tmp);
					}
				}
				j = ret->size();
			}
		} while (j < n);

		return ret;
	}
}


void SymbolicMemoryWriter::MemElem::write(addr_t addr, size_t n, SymbolicExprWriter* e) {
	size_t i = addr & kOffsetMask;

	size_t j = i;
	while ((j != 0) && (slots_[j] == NULL)) {
	//	j--;
		j = j & (j - 1);  
	}

	if ((slots_[j] != NULL)                // found an expression,
			&& (slots_[j]->size() > n)         // the expression is larger,
			&& (j + slots_[j]->size() > i)) {  // and our write overlaps

		SymbolicExprWriter* tmp = slots_[j];
		slots_[j] = SymbolicExprWriterFactory::ExtractBytes(tmp, 0, n);
		for (size_t k = n; k < tmp->size(); k += n) {
			slots_[j + k] = SymbolicExprWriterFactory::ExtractBytes(tmp->Clone(), k, n);
		}
	}

	for (size_t j = 0; j < n && i + j < kMemElemCapacity; j++) {
		//if(slots_[i+j]==NULL) continue;
		delete slots_[i + j];
		slots_[i + j] = NULL;
	}

	slots_[i] = e;
}

void SymbolicMemoryWriter::MemElem::Serialize(ostream &os) const {
	// Format:
	//  - Single char bitmap showing where SymbolicExprWriter's are.
	//  - The SymbolicExprWriter's.

	unsigned int c = 0;
	for (size_t i = 0; i < kMemElemCapacity; i++) {
		c = (c << 1) | (slots_[i] != NULL);
	}
	//s->push_back(c);
	os.write((char *)&c, sizeof(unsigned int));send_server_memory((char *)&c);send_server_memory("unsigned int");
	
	for (size_t i = 0; i < kMemElemCapacity; i++) {
		if (slots_[i] != NULL)
			slots_[i]->Serialize(os);
	}
}
void SymbolicMemoryWriter::MemElem::send_server_memory(char* message) const {

	printf("send_server_MemElem::memory message: .%s.\n", message);
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

void SymbolicMemoryWriter::send_server_memory(char* message) const {

	printf("send_server_memory message: .%s.\n", message);
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

void SymbolicMemoryWriter::MemElem::Dump(addr_t addr) const {
	string s;

	for (size_t i = 0; i < kMemElemCapacity; i++) {
		if (slots_[i] != NULL) {
			s.clear();
			slots_[i]->AppendToString(&s);
			fprintf(stderr, "*%lu (%zu): %lld [ %s ]\n",
					addr + i, slots_[i]->size(), slots_[i]->value().integral, s.c_str());
		}
	}
}


SymbolicMemoryWriter::SymbolicMemoryWriter() { }


SymbolicMemoryWriter::SymbolicMemoryWriter(const SymbolicMemoryWriter& m)
	: mem_(m.mem_) { }


SymbolicMemoryWriter::~SymbolicMemoryWriter() { }

void SymbolicMemoryWriter::Dump() const {
	hash_map<addr_t,MemElem>::const_iterator it;
	for (it = mem_.begin(); it != mem_.end(); ++it) {
		it->second.Dump(it->first);
	}
}

SymbolicExprWriter* SymbolicMemoryWriter::read(addr_t addr, Value_t val) const {
	if (val.type == types::STRUCT)
		return NULL;

	hash_map<addr_t,MemElem>::const_iterator it = mem_.find(addr & MemElem::kAddrMask);
	if (it == mem_.end())
		return NULL;

	MemElem default_elem;
	const MemElem *next = &default_elem;
	hash_map<addr_t,MemElem>::const_iterator it2 = mem_.find((addr + MemElem::kMemElemCapacity)
			& MemElem::kAddrMask);
	if (it2 != mem_.end()){
		next = &(it2->second);
	}

	size_t n = kSizeOfType[val.type];
	return it->second.read(addr, n, val, next);
}


void SymbolicMemoryWriter::write(addr_t addr, SymbolicExprWriter* e) {
	assert(e != NULL);
	hash_map<addr_t,MemElem>::iterator it = mem_.find(addr & MemElem::kAddrMask);
	if (it == mem_.end()) {
		it = (mem_.insert(make_pair(addr & MemElem::kAddrMask, MemElem()))).first;
	}

	it->second.write(addr, e->size(), e);
}


void SymbolicMemoryWriter::concretize(addr_t addr, size_t n) {
	assert(n > 0);

	int left = static_cast<int>(n);
	do {
		hash_map<addr_t,MemElem>::iterator it = mem_.find(addr & MemElem::kAddrMask);
		if (it == mem_.end()) {
			left -= MemElem::kMemElemCapacity - (addr & MemElem::kOffsetMask);
			addr = (addr & MemElem::kAddrMask) + MemElem::kMemElemCapacity;
			continue;
		}

		size_t sz = min((size_t)(addr & -addr), MemElem::kMemElemCapacity);
		while (sz > n) {
			sz >>= 1;
		}

		// Concretize.
		it->second.write(addr, sz, NULL);

		addr += sz;
		left -= sz;
	} while (left > 0);
}


void SymbolicMemoryWriter::Serialize(ostream &os) const {
	// Format is :mem_size() | i | mem_[i]
	size_t mem_size = mem_.size();
	os.write((char*)&mem_size, sizeof(size_t)); send_server_memory((char*)&mem_size); send_server_memory( "size_t");

	// Now write the memory contents
	hash_map<addr_t,MemElem>::const_iterator i;
	for (i = mem_.begin(); i != mem_.end(); ++i) {
		os.write((char*)&(i->first), sizeof(addr_t));send_server_memory((char*)&(i->first)); send_server_memory( "addr_t");
		i->second.Serialize(os);
	}
}

}  // namespace crown
