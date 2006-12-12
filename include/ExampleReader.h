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
#include <sstream>
#include <map>

#include "CharTransform.h"
#include "LogMsg.h"

using namespace std;


struct ExampleInfo {

  string opName;
  int number;
  int lineNo;
  string signature;
  string example;
  string result;

  ExampleInfo() {
    opName="";
    number=0;
    lineNo=0;
    signature="";
    example="";
    result="";
  } 

  void print(ostream& os) const
  {
    os << "Operator : " << opName << endl;
    os << "Number   : " << number << endl;
    os << "Signature: " << signature << endl;
    os << "Example  : ";
    if (example.find("query ")==string::npos)
      os << "query ";
    os << example << endl;
    os << "Result   : " << result << endl;
  } 
}; 



class ExampleReader {

  typedef enum { Database, Restore, Operator, 
                 Number, Signature, Example, Result} Token; 

  bool debug;
  int lineCtr;
  string line;
  string lineRest;
  string algName;
  string fileName;
  string database;
  bool restore;

  size_t pos;
  Token expected;
  map<Token, string> tokendef; 

  //typedef list<ExampleInfo> ExampleList; 
  typedef map<string, ExampleInfo> ExampleMap; 
  ExampleMap examples;

  typedef ExampleMap::iterator iterator;

  iterator begin() { return examples.begin(); }
  iterator end()   { return examples.end();   }

  iterator scan;

  void nextLine(CFile& stream, string& line) {
    getline(stream.ios(), line);
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
  ExampleReader(const string& file, const string& algebra="") 
    : expected(Database) 
  {
    
    lineCtr = 0;
    line="";
    lineRest="";
    fileName = WinUnix::MakePath(file);
    algName=algebra;
    database="";
    restore=false;

    debug = RTFlag::isActive("SI:ExampleParser:Debug");
    tokendef[Database]  = "Database";
    tokendef[Restore]   = "Restore";
    tokendef[Operator]  = "Operator";
    tokendef[Number]    = "Number";
    tokendef[Signature] = "Signature";
    tokendef[Example]   = "Example";
    tokendef[Result]    = "Result";
  }
  ~ExampleReader() {}

  


  bool parse() { 


    CFile stream(fileName);
    if (!stream.open()) {;
      cerr << "Error: Could not open file!" << endl;
      return false;
    }  

    ExampleInfo info;
    string key="";
    
    while (!stream.eof() && !stream.fail()) {

      // read next input string;
      line = "";
      lineRest ="";
      pos=0;
      nextLine(stream, line);
      while ( isSpaceStr(line) && !stream.eof()) {
	line="";
        nextLine(stream, line);
      }
      if (debug)
        cout << lineCtr << ": " << line << endl;
      if (debug)
        cout << tokendef[expected] << endl;

      switch (expected) {
  
      case Database: { 
        
          if (!match(Database))
            return false;
          expected = Restore;
          database = lineRest;
          break;
       }

      case Restore: { 
        
          if (!match(Restore))
            return false;
          expected = Operator;
          restore = hasSuffix(lineRest, "Yes");
          restore = restore || hasSuffix(lineRest, "YES");
          restore = restore || hasSuffix(lineRest, "yes");
          break;
       }


       case Operator: { 
        
          if (stream.eof())
	    return true;
		       
          if (!match(Operator))
            return false;
          expected = Number;
          info.opName = lineRest;
	  key = info.opName;
          //examples[key].push_back(info);
          examples[key] = info;
          break;
       }

       case Number: { 
          if (!match(Number))
            return false;
           expected = Signature;
           examples[key].number = ::parse<int>(lineRest);
          break;
       }

      case Signature: { 
          if (!match(Signature))
            return false;
           expected = Example;
           examples[key].signature = lineRest;
          break;
       }

       case Example: { 
          if (!match(Example))
            return false;
           expected = Result;
           examples[key].example = lineRest;
           examples[key].lineNo = lineCtr;
          break;
       }

       case Result: {
          if (!match(Result))
            return false;
           expected = Operator;
           examples[key].result = lineRest;
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

  bool find(const string& Op, ExampleInfo& ex) const
  {
     ExampleMap::const_iterator it = examples.find(Op);
     if (it == examples.end())
       return false;

     ex = it->second;
     return true;
  } 


  void add(const string& op, const int nr, ExampleInfo& ex)
  {
     stringstream key;
     key << op << algName << nr;
     examples[key.str()] = ex; 
  } 

  bool write() {

    CFile out(fileName);
    if ( !out.overwrite() ) {
      cerr << "Opening " << fileName << " failed!" << endl;
      return false;
    } 
    
    out.ios() << "Database: berlintest" << endl
              << "Restore : No" << endl << endl;

    ExampleMap::const_iterator it = examples.begin();
    while(it != examples.end()) {

      it->second.print(out.ios());
      out.ios() << endl << endl;  
      it++;
    } 

    return out.close();
  } 

  string getDB() { return database; } 

  bool getRestoreFlag() { return restore; } 

  void initScan() { scan=begin();}
  bool next(ExampleInfo& ex) {
     scan++;
     if (scan != end())
     { 
       ex = scan->second;
       return true;
     }
     return false;       
  } 

};



#endif
