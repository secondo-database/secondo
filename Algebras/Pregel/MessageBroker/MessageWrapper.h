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

This header file defines the class MessageWrapper

2 Defines and includes

*/

#ifndef SECONDO_MESSAGEWRAPPER_H
#define SECONDO_MESSAGEWRAPPER_H


#include <string>
#include <ostream>
#include "../../Relation-C++/RelationAlgebra.h"
#include "../Helpers/Logging.h"

namespace pregel {
 class MessageWrapper {
 public:
  friend std::ostream &
  operator<<(std::ostream &os, const MessageWrapper &wrapper);

 public:
  enum MessageType {
   DATA,
   EMPTY,
   FINISH,
   INIT_DONE
  };

  static std::string typToString(MessageType value) {
   switch (value) {
    case DATA:
     return "DATA";
    case EMPTY:
     return "EMPTY";
    case FINISH:
     return "FINISH";
    case INIT_DONE:
     return "INIT_DONE";
    default:
     return std::to_string(value);
   }
  }

  struct Header {
   Header(int destination, MessageType type, unsigned long length, int round);

   Header();

   static Header fromTuple(Tuple *body, int round);

   static Header read(char *buffer);

   char *write(char *buffer) const;

   char *write() const;

   friend std::ostream &operator<<(std::ostream &os, const Header &header);

   int recipient = 0;
   MessageType type = DATA;
   unsigned long length = 0;
   int superstep = 0;
  };

  constexpr static size_t HEADER_SIZE = sizeof(int) + sizeof(MessageType) +
                                        sizeof(unsigned long) + sizeof(int);

  static MessageWrapper *
  constructControlMessage(MessageType type, int destination, int superstep) {
   MessageWrapper *message = new MessageWrapper(
    Header(destination, type, 0, superstep), nullptr);
   return message;
  }

  static MessageWrapper *constructEmptyMessage(int destination, int superstep) {
   return constructControlMessage(EMPTY, destination, superstep);
  }

  static MessageWrapper *
  constructFinishMessage(int destination, int superstep) {
   return constructControlMessage(FINISH, destination, superstep);
  }

  static MessageWrapper *
  constructInitDoneMessage(int destination, int superstep) {
   return constructControlMessage(INIT_DONE, destination, superstep);
  }

  MessageWrapper();

  static MessageWrapper *fromTuple(Tuple *tuple, int round);

  MessageWrapper(Header header, Tuple *body);

  unsigned long serialize(char *&buffer);

  char *serialize(unsigned long &size);

  static MessageWrapper *deserialize(char *buffer, Header header);

  Tuple *getBody() const;

  void setBody(Tuple *body);

  void setDestination(int destination);

  int getDestination() const;

  inline void setType(MessageType type) {
   header.type = type;
  }

  inline MessageType getType() const {
   return header.type;
  }

  inline void setLength(unsigned long size) {
   header.length = size;
  }

  inline unsigned long getLength() const {
   return header.length;
  }

  inline int getRound() const {
   return header.superstep;
  }

  inline void setRound(const int round) {
   header.superstep = round;
  }

  char *writeHeader() const {
   return header.write();
  }

 private:
  Tuple *body;
  Header header;
 };
}


#endif //SECONDO_MESSAGEWRAPPER_H
