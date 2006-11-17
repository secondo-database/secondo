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

November 2006, M. Spiekermann.

*/

#ifndef SECONDO_EXAMPLE_READER_H
#define SECONDO_EXAMPLE_READER_H

#include <iostream>
#include "Chartransform.h"

using namespace std;

class ExampleReader {

  enum {Operator, Example, Result} Token; 

  int line = 0;
  istream& stream;
  Token expected;
  map<Token, string> tokendef; 

  ExampleReader(const istream& is) : stream(is), expected(Operator) 
  {
    tokendef[Operator] = "Operator";
    tokendef[Example]  = "Example";
    tokendef[Result]   = "Result";
  }
  ~ExampleReader() {}

  void nextLine(string& line) {
    getline(is, line);
    line++;
  }

  bool expect(const string& s, Token t) {
     
     string token = tokendef[t];
     if ( s.find(token) == 0 ) {
       cerr << "Parse error: expecting " 
            << token << "in line " << line << endl;
       return false;
     }
     
     return true;
  }

  bool parse() { 

    while (!is.eof() && !cin.fail()) {

      // read next input string;
      string line = "";
      nextLine(line);
      while ( isSpaceStr(line)) {
        nextLine(line);
      }
      cout << "line: " << line << endl;

      // extract the first word of the line
      string token = parse<string>(line);
      size_t pos = line.find(token) + command.size();
      string restOfLine = trim( line.substr(pos) );

      switch (expected) {
  
       case Operator: { 
         
          if (!expect(token,Operator))
            return false;
          expected = Example;
          break;
       }

       case Example: { 
          if (!expect(token,Example))
            return false;
           expected = Result;
          break;
       }

       case Result: {
          if (!expect(token,Result))
            return false;
           expected = Operator;
          break;
       }

       default: // never reached
         cerr << "Parse error!" 
      }
    }
    return true;
  }

};



#endif
