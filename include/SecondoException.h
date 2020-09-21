/*
---- 
This file is part of SECONDO.

Copyright (C) 2015, University in Hagen, Department of Computer Science, 
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

A Base class for Exceptions. All specific exception classes should be inherit
this class.

*/

#ifndef SECONDOEXCEPTION_H
#define SECONDOEXCEPTION_H

#include <exception>
#include <string>


class SecondoException : public std::exception {

  public:
  
  SecondoException() : msgStr("Secondo-Exception: Unknown Error") {
  }

  SecondoException(const std::string& Msg) : 
   exception(), msgStr("Secondo-Exception: " + Msg) {
  }

  SecondoException(const SecondoException& rhs) : 
    std::exception(), msgStr("Secondo-Exception: " + rhs.msgStr) {
  }

  virtual ~SecondoException() throw() {
  }

  virtual const char* what() const throw() {
    return msgStr.c_str();
  }

  const std::string msg() { 
   return msgStr; 
  }
  
  protected:
    std::string msgStr;
};

#endif


