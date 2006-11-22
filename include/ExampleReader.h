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
#include <map>

#include "CharTransform.h"


using namespace std;


struct ExampleInfo {

  string opName;
  int number;
  string signature;
  string example;
  string result;

  ExampleInfo() {
    opName="";
    number=0;
    signature="";
    example="";
    result="";
  } 
}; 



class ExampleReader {

  typedef enum {Operator, Number, Signature, Example, Result} Token; 

  bool debug;
  int lineCtr;
  string line;
  string lineRest;
  string algName;
  string fileName;
  size_t pos;
  Token expected;
  map<Token, string> tokendef; 

  typedef map<string, ExampleInfo> ExampleMap; 
  ExampleMap examples;

  void nextLine(istream& stream, string& line) {
    getline(stream, line);
    lineCtr++;
  }

/*
The function below matches regular expressions of the pattern
a*bc* with single characters a,b and c.

*/   

  bool match_Astar_B_Cstar(char A, char B, char C)
  {
    const char a=A, b=B, c=C;
    int state = 0;
    
    bool ok=false;
    bool end=false;

    while (!end) {
    char n=line[pos];
    pos++;
    switch (state) {
 
      case 0: { 
                if (n==a)
                  state=1;
                else if (n==b)
                  state=2;
                else 
                  end=true;
                break;
      }
      case 1: {  
                if (n==a)
                  state=1;
                else if (n==b)
                  state=2;
                else 
                  end=true;
                break;
      }
      case 2: { 
                ok=true;
                if (n==c)
                  state=2;
                else
                  end=true;
                break;
      }
      default: { exit(1); } // should never be reached

    }   
    }
    pos--;
    return ok;
  } 

  bool match(Token t) {
     
     string token = tokendef[t];
     pos = line.find(token); 
     if ( pos == string::npos ) {
       cerr << errMsg() << "expecting token '" 
            << token << "'! But got '" << line << "'"
            << endl;
       return false;
     }
     pos = pos + token.size();
     size_t pos2 = pos;
     if (!match_Astar_B_Cstar(' ',':',' '))
     {
       cerr << errMsg() << "expecting regexp (' '*':' '*)" 
            << lineCtr << "! But got '" << line.substr(pos2) << "'" 
            << endl;
       return false;
     }

     lineRest = trim( line.substr(pos) );
     return true;
  }


  string errMsg() {
 
     stringstream s;
     s << "Parse error in line " << lineCtr << ": ";
     return s.str();
  }

  public:
  ExampleReader(const string& file) : expected(Operator) 
  {
    
    lineCtr = 0;
    line="";
    lineRest="";
    fileName=file;

    
    debug=true;
    tokendef[Operator]  = "Operator";
    tokendef[Number]    = "Number";
    tokendef[Signature] = "Signature";
    tokendef[Example]   = "Example";
    tokendef[Result]    = "Result";
  }
  ~ExampleReader() {}


  bool parse() { 

    size_t p = fileName.find(".examples");
    if (p == string::npos) {
      cerr << "Warning: file '" << fileName 
           << "'does not end with '.examples'" << endl;
      algName=fileName; 
    }   
    algName=fileName.substr(0,p);
    cout << "Processing examples for algebra '" << algName << "'" << endl;

    ifstream stream;
    stream.open(fileName.c_str());
    if (!stream.good()) {
      cerr << "Error: Could not open file!" << endl;
      return false;
    }  

    while (!stream.eof() && !stream.fail()) {

      // read next input string;
      line = "";
      lineRest ="";
      pos=0;
      nextLine(stream, line);
      while ( isSpaceStr(line)) {
        nextLine(stream,line);
      }
      if (debug)
        cout << lineCtr << ": " << line << endl;

      ExampleInfo info;

      switch (expected) {
  
       case Operator: { 
         
          if (!match(Operator))
            return false;
          expected = Number;
          info.opName = lineRest;
          examples[info.opName+algName] = info;
          break;
       }

       case Number: { 
          if (!match(Number))
            return false;
           expected = Signature;
           info.number = ::parse<int>(lineRest);
          break;
       }

      case Signature: { 
          if (!match(Signature))
            return false;
           expected = Example;
           info.signature = lineRest;
          break;
       }

       case Example: { 
          if (!match(Example))
            return false;
           expected = Result;
           info.example = lineRest;
          break;
       }

       case Result: {
          if (!match(Result))
            return false;
           expected = Operator;
           info.result = lineRest;
          break;
       }

       default: // never reached
         cerr << errMsg() << endl; 
      }

      if (debug) {
        cout << lineCtr << ": " << lineRest << endl;
      }  
    }
    return true;
  }

  bool find(const string& Op, const string& Alg, ExampleInfo& ex) const
  {
     ExampleMap::const_iterator it = examples.find(Op+Alg+"1");
     if (it == examples.end())
       return false;

     ex = it->second;
     return true;
  } 


  void add(const string& op, ExampleInfo& ex)
  {
     examples[op+algName+"1"] = ex; 
  } 

  bool write() {

    string fileName = "tmp/"+algName+".examples";
    ofstream file;
    file.open(fileName.c_str());

    ExampleMap::const_iterator it = examples.begin();
    while(it != examples.end() && file.good()) {

      file << "test" << endl;  
      it++;
    } 

    if (!file.good())
      return false;
    file.close();

    return true;
  } 

};



#endif
