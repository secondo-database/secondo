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
 MessageWrapper::MessageWrapper() : header() {}

 MessageWrapper::MessageWrapper(MessageWrapper::Header header, Tuple *body)
  : body(body), header(header) {}

 MessageWrapper::Header MessageWrapper::Header::read(char *buffer) {
  char *offset = buffer;
  int destination=-1;
  MessageType type=MessageType::UNKNOWN;
  unsigned long length = 0;
  int round = 0;

  memcpy(&destination, offset, sizeof(int));
  offset += sizeof(int);
  memcpy(&type, offset, sizeof(MessageType));
  offset += sizeof(MessageType);
  memcpy(&length, offset, sizeof(unsigned long));
  offset += sizeof(unsigned long);
  memcpy(&round, offset, sizeof(int));

  Header header(destination, type, length, round);
//  BOOST_LOG_TRIVIAL(debug) << "Header read: " << header;
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

 char *MessageWrapper::Header::write(char *buffer) const {
//  BOOST_LOG_TRIVIAL(debug) << "Serializing header: " << *this;
  char *offset = buffer;
  memcpy(offset, &recipient, sizeof(int));
  offset += sizeof(int);
  memcpy(offset, &type, sizeof(MessageType));
  offset += sizeof(MessageType);
  memcpy(offset, &length, sizeof(unsigned long));
  offset += sizeof(unsigned long);
  memcpy(offset, &superstep, sizeof(int));
  offset += sizeof(int);

  return offset;
 }

 char *MessageWrapper::Header::write() const {
  char *buffer = new char[HEADER_SIZE];
  write(buffer);
  return buffer;
 }

 Tuple *MessageWrapper::getBody() const {
  return body;
 }

 void MessageWrapper::setBody(Tuple *body) {
  MessageWrapper::body = body;
 }

 unsigned long MessageWrapper::serialize(char *&buffer) {
  std::string bodyAsBin;
  unsigned long size = 0;

  if (header.type == DATA) {
   bodyAsBin = body->WriteToBinStr();
   unsigned long bodySize = bodyAsBin.size();
   header.length = bodySize;
   size = HEADER_SIZE + bodySize;
  }

  buffer = new char[size];

  char *offset = header.write(buffer);

  if (header.type == DATA) {
   strcpy(offset, bodyAsBin.c_str());
  }

//  BOOST_LOG_TRIVIAL(debug) << "message serialized (" << size << "B)";
  return size;
 }

 MessageWrapper *MessageWrapper::deserialize(char *buffer, Header header) {
  char *offset = buffer;

  TupleType* tupleType = PregelContext::get().getTupleType();
  Tuple *tuple = new Tuple(tupleType);

  std::string read(offset, header.length);

  tuple->ReadFromBinStr(0, read);

  return new MessageWrapper(header, tuple);
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
  ListExpr toList = wrapper.getBody()->SaveToList(messageType);
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

 char *MessageWrapper::serialize(unsigned long &size) {
  std::string bodyAsBin;
  unsigned long bodySize = 0;

  if (header.type == DATA) {
   bodyAsBin = body->WriteToBinStr();
   bodySize = bodyAsBin.size();
   header.length = bodySize;
  }

  size = HEADER_SIZE + bodySize;

  char *buffer = new char[size];
  char *offset = header.write(buffer);
//   BOOST_LOG_TRIVIAL(debug) << "Serialized header";

  if (header.type == DATA) {
   memcpy(offset, bodyAsBin.c_str(), bodySize);
//    BOOST_LOG_TRIVIAL(debug) << "Serialized body";
  }

//   BOOST_LOG_TRIVIAL(debug) << "message serialized (" << size << "B)";
  return buffer;
 }

 MessageWrapper *MessageWrapper::fromTuple(Tuple *tuple, int round) {
  Header header = Header::fromTuple(tuple, round);
  return new MessageWrapper(header, tuple);
 }
}
