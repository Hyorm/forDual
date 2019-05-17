// Copyright (c) 2009, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <cstring>
#include <assert.h>
#include <iostream>
#include <string>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "base/basic_functions.h"
#include "libcrown/object_tracker_writer.h"
#include "libcrown/symbolic_object_writer.h"

#define size_MAX 1024
#define ipAddress "192.168.0.150"
#define portNum "3030"

using std::map;

namespace crown {

typedef map<addr_t,SymbolicObjectWriter*>::iterator EntryIt;
typedef map<addr_t,SymbolicObjectWriter*>::const_iterator ConstEntryIt;
typedef map<addr_t,bool>::iterator EntryItDer;
typedef map<addr_t,bool>::const_iterator ConstEntryItDer;

ObjectTrackerWriter::~ObjectTrackerWriter() {
	for (EntryIt i = memRegions_.begin(); i != memRegions_.end(); ++i) {
		//delete i->second;
	}
	//FIXME: add snapshotManager
}

void ObjectTrackerWriter::addRegion(addr_t addr, size_t size) {
	//Add for blocking alloc after the end of program
	if(addRegionBarrior == 1) return;

	snapshotManager_.push_back(new SnapshotVector());
	int managerIdx = snapshotManager_.size() - 1;
	//objs in manager do not free, so the objects which hava same addr can exist, so use snapshotIdx instead of addr.

	memRegions_[addr + size] = new SymbolicObjectWriter(addr, size, managerIdx);
	isRegionDereferred_[addr + size] = true;
}

SymbolicObjectWriter* ObjectTrackerWriter::storeAndGetNewObj(
					SymbolicObjectWriter &obj, addr_t addr){
	SymbolicObjectWriter* newObj = new SymbolicObjectWriter(obj);
	size_t idx = obj.managerIdx();
	//Store the old obj to snapshotManager
	snapshotManager_[idx]->push_back(&obj);

	assert(memRegions_.upper_bound(addr)->second->managerIdx() == idx);

	//Tracking obj is changed to newObj	
	removeTrackingObj(addr);
	memRegions_[newObj->end()] = newObj;	

	updateDereferredStateOfRegion(*newObj, addr, false);

	return newObj;
}

void ObjectTrackerWriter::storeAllObjAndRemove(){
	// Before end of program, store all the valid region to snapshot
	addRegionBarrior = 1;
	for(EntryIt i = memRegions_.begin(); i != memRegions_.end(); i++){
		size_t idx = i->second->managerIdx();
		if(!(0 <= idx && idx < snapshotManager_.size())){
			continue;
		}
		if(snapshotManager_[idx]->size()==0) continue;
		snapshotManager_[idx]->push_back(i->second);
		removeTrackingObj(i->second->start());
	}
}

void ObjectTrackerWriter::storeObj(addr_t addr){
	// Stre the region to snapshot
	EntryIt i = memRegions_.upper_bound(addr);

	if (i == memRegions_.end())
		return;

	if (i->second->start() != addr)
		return;
	size_t idx = i->second->managerIdx();
	snapshotManager_[idx]->push_back(i->second);
}

void ObjectTrackerWriter::updateDereferredStateOfRegion(
			SymbolicObjectWriter &obj, addr_t addr, bool truth) {
	//Update the state of valid region (dereferenced or not)
	EntryItDer i = isRegionDereferred_.upper_bound(addr);

	if (i == isRegionDereferred_.end())
		return;

	isRegionDereferred_.erase(i);
	isRegionDereferred_[obj.end()] = truth;
}


void ObjectTrackerWriter::removeTrackingObj(addr_t addr) {
	EntryIt i = memRegions_.upper_bound(addr);

	if (i == memRegions_.end())
		return;

	if (i->second->start() != addr)
		return;

	memRegions_.erase(i->second->end());
}

bool ObjectTrackerWriter::getDereferredStateOfRegion(addr_t addr) const {
	ConstEntryItDer i = isRegionDereferred_.upper_bound(addr);

	if (i == isRegionDereferred_.end())
		return NULL;

	return i->second;
}


SymbolicObjectWriter* ObjectTrackerWriter::find(addr_t addr) const {
	ConstEntryIt i = memRegions_.upper_bound(addr);

	if (i == memRegions_.end())
		return NULL;

	if (i->second->start() <= addr)
		return i->second;

	return NULL;
}

void ObjectTrackerWriter::Serialize(ostream &os) const {
	size_t managerSize = snapshotManager_.size();
	os.write((char*)&managerSize, sizeof(size_t)); send_server_tracker((char*)&managerSize); send_server_tracker("size_t");
	for(size_t i = 0; i < managerSize; i++){
		size_t objsSize = snapshotManager_[i]->size();

		os.write((char*)&objsSize, sizeof(size_t)); send_server_tracker((char*)&objsSize); send_server_tracker( "size_t");
		for(size_t j = 0; j < objsSize; j++){
			snapshotManager_[i]->at(j)->Serialize(os);
		}
	}

}

void ObjectTrackerWriter::send_server_tracker(char* message) const {

	printf("send_server_tracker message: .%s.\n", message);
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
void ObjectTrackerWriter::Dump() const {
	for (ConstEntryIt i = memRegions_.begin(); i != memRegions_.end(); ++i) {
		fprintf(stderr, "Object [%lu,%lu] --\n", i->second->start(), i->first);
		i->second->Dump();
	}
}


}  // namespace crown

