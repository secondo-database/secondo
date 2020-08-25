/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File of the class ~PregelAlgebra~

November 2018, J. Mende


[TOC]

1 Overview

This file defines the members of class MessageWrapper

*/

#include <cstring>
#include <StandardTypes.h>
#include "MessageWrapper.h"
#include "../PregelContext.h"

namespace pregel {

 constexpr size_t MessageWrapper::HEADER_SIZE;

 MessageWrapper::MessageWrapper() : body(nullptr),header() {}

 MessageWrapper::~MessageWrapper() {
   if(body != nullptr){
     body->DeleteIfAllowed();
   }
 }

 MessageWrapper::MessageWrapper(MessageWrapper::Header header, Tuple *body)
  : body(body), header(header) {
   if(body != nullptr){
     body->bringToMemory(); // to overcome end of query and
                            //messages having native flobs 
   }
 }

 template<typename T>
 char* readBin(char* buffer, T& v){
   memcpy(&v, buffer,sizeof(T));
   return buffer + sizeof(T);
 }
 MessageWrapper::Header MessageWrapper::Header::read(char *buffer) {
   Header header;
   char* nb = buffer;
   nb = readBin(nb, header.recipient);
   nb = readBin(nb, header.type);
   nb = readBin(nb, header.length);
   nb = readBin(nb, header.superstep);
   return header;
 }

 MessageWrapper::Header::Header() {}

 MessageWrapper::Header::Header(int destination,
                                MessageType messageType,
                                unsigned long messageLength,
                                int round)
  : recipient(destination),
    type(messageType),
    length(messageLength),
    superstep(round) {
 }

 MessageWrapper::Header
 MessageWrapper::Header::fromTuple(Tuple *body, const int round) {
  int index = PregelContext::get().getAddressIndex();
  auto attribute = (CcInt *) body->GetAttribute(index);
  const int destination = attribute->GetValue();
  return {destination, DATA, 0, round};
 }

 template<typename T>
 char* writeBin(char* buffer, T v){
   memcpy(buffer, &v, sizeof(T));
   return buffer + sizeof(T);
 }

 char *MessageWrapper::Header::write(char *buffer) const {
  char* nb = buffer;
  nb = writeBin(nb, recipient);
  nb = writeBin(nb, type);
  nb = writeBin(nb, length);
  nb = writeBin(nb, superstep);
  return nb;
 }

 char *MessageWrapper::Header::write() const {
  char *buffer = new char[HEADER_SIZE];
  write(buffer);
  return buffer;
 }

 bool MessageWrapper::Header::operator==(
         const MessageWrapper::Header& rhs) const{
  return     (recipient == rhs.recipient)
         &&  type == rhs.type
         && superstep == rhs.superstep;
       // && length = rhs.length    // only equal affer serialization
 }

 bool equalTuples(Tuple* t1, Tuple* t2){
    if(t1==nullptr && t2==nullptr){
      return true;
    }
    if(t1==nullptr || t2==nullptr){
       return false;
    }
    if(t1->GetNoAttributes() != t2->GetNoAttributes()){
      return false;
    }
    for(int i=0;i<t1->GetNoAttributes(); i++){
      if(t1->GetAttribute(i)->Compare(t2->GetAttribute(i)) != 0){
         return false;
      }
    }
    return true;
 }


  bool MessageWrapper::operator==(const MessageWrapper& rhs) const{
    return    header==rhs.header
           && equalTuples(body, rhs.body);
  }


 Tuple *MessageWrapper::getBody1() const {
  if(body){
     body->IncReference();
  }
  return body;
 }

 void MessageWrapper::setBody1(Tuple *body) {
  if(this->body){
    this->body->DeleteIfAllowed();
  }
  body->IncReference();
  this->body = body;
  this->body->bringToMemory();
 }


 char* tuple2Bin(Tuple* tuple, size_t& buffersize){
   size_t coreSize;
   size_t extensionSize;
   size_t flobSize;
   size_t blocksize = tuple->GetBlockSize(coreSize, extensionSize,
                                         flobSize);
   // allocate buffer and write flob into it
   char* buffer = new char[blocksize];
   tuple->WriteToBin(buffer, coreSize, extensionSize, flobSize);
   uint32_t tsize = blocksize;
   buffersize = tsize;
   return buffer;
 }


 unsigned long MessageWrapper::serialize(char *&buffer) {
  unsigned long size = HEADER_SIZE;
  size_t bodySize = 0;
  char* bodyBuffer = 0;

  if (header.type == DATA) {
    bodyBuffer = tuple2Bin(body,bodySize);
    header.length = bodySize;
    size += bodySize; 
  }

  buffer = new char[size];

  char *offset = header.write(buffer);

  if (header.type == DATA) {
     memcpy(offset,bodyBuffer, bodySize);
     delete[] bodyBuffer;
  }
  return size;
 }

 std::shared_ptr<MessageWrapper> 
 MessageWrapper::deserialize(char *buffer, Header header) {
  char *offset = buffer;

  TupleType* tupleType = PregelContext::get().getTupleType();
  Tuple *tuple = new Tuple(tupleType);
  tuple->ReadFromBin(0, offset );
  return std::make_shared<MessageWrapper>(header, tuple);
 }

 int MessageWrapper::getDestination() const {
  return header.recipient;
 }

 void MessageWrapper::setDestination(int destination) {
  header.recipient = destination;
 }

 std::ostream &operator<<(std::ostream &os, const MessageWrapper &wrapper) {
  if (wrapper.getType() != MessageWrapper::MessageType::DATA) {
   os << "non data message:" << " header: " << wrapper.header;
   return os;
  }
  ListExpr messageType;
  nl->ReadFromString(PregelContext::get().getMessageType(), messageType);
  Tuple* tuple = wrapper.getBody1();
  ListExpr toList = tuple->SaveToList(messageType);
  tuple->DeleteIfAllowed();
  os << "data message:" << " body: " << nl->ToString(toList) << " header: "
     << wrapper.header;
  return os;
 }

 inline std::ostream &
 operator<<(std::ostream &os, MessageWrapper::MessageType value) {
  switch (value) {
   case MessageWrapper::MessageType::DATA :
    return os << "DATA";
   case MessageWrapper::MessageType::EMPTY :
    return os << "EMPTY";
   case MessageWrapper::MessageType::FINISH :
    return os << "FINISH";
   default:
    return os << (int) value;
  }
 }

 std::ostream &
 operator<<(std::ostream &os, const MessageWrapper::Header &header) {
  os << "recipient: " << header.recipient << " type: " << header.type
     << " length: "
     << header.length << " superstep: " << header.superstep;
  return os;
 }


 std::shared_ptr<MessageWrapper> 
 MessageWrapper::fromTuple(Tuple *tuple, int round) {
  Header header = Header::fromTuple(tuple, round);
  return std::make_shared<MessageWrapper>(header, tuple);
 }
}
