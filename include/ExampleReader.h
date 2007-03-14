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

November 2006, M. Spiekermann. Start of implementation.

Dec 2006, M. Spiekermann. Implementation Finished. The example reader can now
handle multiple examples per operator and supports alias names.

Jan 2007, M. Spiekermann. New result tokens file_platform, bug and crashes added.
Moreover the parser reports more kind of errors. 

*/

#ifndef SECONDO_EXAMPLE_READER_H
#define SECONDO_EXAMPLE_READER_H

#include <iostream>
#include <sstream>
#include <map>
#include <list>
#include <set>

#include "CharTransform.h"
#include "LogMsg.h"

using namespace std;


struct ExampleInfo {

  typedef enum { List, Atom, 
                 File, PlatformFile, Bug, Crash, Invalid} Type;

  string opName;
  string aliasName;
  int number;
  int lineNo;
  string signature;
  string example;
  string result;
  string remark;
  double tolerance;
  bool   relativeTolerance;
  Type resultType;
  

  ExampleInfo() {
    reset();
  } 

  void reset() {
    opName="";
    aliasName="";
    number=0;
    lineNo=0;
    signature="";
    example="";
    result="";
    remark="";
    resultType=List;
    setDefaultTolerance();
  }

  void setDefaultTolerance(){
     tolerance = 0;
     relativeTolerance = false;
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
    if(tolerance>0){
       os << "Tolerance:" << tolerance << endl;
       os << "relative :" <<(relativeTolerance?"true":"false") << endl;
    }
  } 
}; 



class ExampleReader {

  typedef enum { Database, Restore, Operator, 
                 Number, Signature, Example, Result, Remark,Tolerance} Token; 

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

  public:
  typedef list<ExampleInfo*> ExampleList; 

  private:
  typedef map<string, ExampleList> ExampleMap; 
  ExampleMap examples;

  typedef ExampleMap::const_iterator iterator;

  iterator begin() const { return examples.begin(); }
  iterator end()   const { return examples.end();   }

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

  bool match(Token t, bool showErr = true) {
     
     string token = tokendef[t];
     pos = line.find(token); 
     if ( pos == string::npos ) {
       if (showErr) {
       cerr << errMsg() << "expecting token '" 
            << token << "'! But got '" << line << "'"
            << endl;
       } 
       return false;
     }
     pos = pos + token.size();
     size_t pos2 = pos;
     if (!match_Astar_B_Cstar(' ',':',' '))
     {
       if (showErr) {
       cerr << errMsg() << "expecting regexp (' '*':' '*)" 
            << lineCtr << "! But got '" << line.substr(pos2) << "'" 
            << endl;
       }
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
    fileName = file;
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
    tokendef[Remark]    = "Remark";
    tokendef[Tolerance] = "Tolerance";

    examples.clear();
    scan=begin();
  }
  ~ExampleReader() {

    ExampleMap::iterator it = examples.begin();
    while (it != examples.end() ) 
    {
      ExampleList& list = it->second;
      ExampleList::iterator it2 = list.begin();
      while (it2 != list.end() ) {
        //delete *it2;
        it2++;
      }
      it++;
    } 
  }

  


  bool parse() { 


    CFile stream(fileName);
    if (!stream.open()) {;
      cerr << "Error: Could not open file!" << endl;
      return false;
    }  

    ExampleInfo* info = 0;
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

      bool switchAgain = true; 
      while(switchAgain) {
        switchAgain=false;
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

          info = new ExampleInfo;
          size_t pos = lineRest.find(" alias");

          if ( pos != string::npos )
            info->aliasName = trim( lineRest.substr(pos+6) );
          else 
            pos = lineRest.length(); 	  

          key = trim( lineRest.substr(0,pos) );
          info->opName = key;
          examples[key].push_back(info);
          break;
       }

       case Number: { 
          if (!match(Number))
            return false;
           expected = Signature;
           info->number = ::parse<int>(lineRest);
           if (!uniqueNumbers(key)) {
            cerr << errMsg() << "The numbers for operator " << key 
                 << " are not unique! Number " << info->number
                 << " is used more than one times. " << endl;
            return false;
           }
          break;
       }

      case Signature: { 
          if (!match(Signature))
            return false;
           expected = Example;
           info->signature = lineRest;
          break;
       }

       case Example: { 
          if (!match(Example))
            return false;
           expected = Result;
           info->example = lineRest;
           info->lineNo = lineCtr;
          break;
       }

       case Result: {
          if (!match(Result))
            return false;
           expected = Remark;
           info->result = lineRest;
           info->resultType = resultType(lineRest); 
           if (info->resultType == ExampleInfo::Invalid) {
             cerr << "Result [" << lineRest << "] is invalid" << endl;
             info->result = "";
           } 
          break;
       }

      case Remark: {
           if (!match(Remark, false)) {
            if (info->resultType == ExampleInfo::Bug) {
              cerr << errMsg()
                   << "Expecting a remark field which describes "
                   << "the specified bug!"
                   << endl;
              info->remark = "";
              return false;
            }
            else {
              switchAgain = true;
              info->remark = lineRest;
            }
           }
           expected = Tolerance;
          break;
       }

      case Tolerance: {
          if(match(Tolerance,false)){
             string tol = lineRest;
             if(tol.size()<1){
                 cerr << errMsg()
                      << "invalid value for tolerance"
                      << endl;
                 return false;
             }
             if(tol[0]=='%'){
                info->relativeTolerance=true;
                tol = tol.substr(1);
             } else {
                info->relativeTolerance=false;
             }

             // double value = parse<double>(tol);
             istringstream is(tol);
             double value;
             is >> value;
             if(info->relativeTolerance){
                  value = value / 100;
             }
             if(value<0){
               cerr << errMsg() 
                    << "negative tolerance not allowed"
                    << endl;
               return false;
             }
             info->tolerance = value; 

          } else {
              switchAgain = true;
              info->setDefaultTolerance();

          }
          expected = Operator;
          break;

      }

       default: // never reached
         cerr << errMsg() << endl; 
      }
      }

      if (debug) {
        cout << lineCtr << ": " << lineRest << endl;
      }  
    }
    return true;
  }

/*
Get first example of the example list

*/
  bool find(const string& op, ExampleInfo& ex) const
  {
     ExampleMap::const_iterator it = examples.find(op);
     if (it == examples.end())
       return false;

     ex = *it->second.front();
     return true;
  } 

  const ExampleList& find(const string& op) const
  {
     ExampleMap::const_iterator it = examples.find(op);
     assert( it != examples.end() );
     return it->second;
  } 

  void add(const string& op, const int nr, ExampleInfo& ex)
  {
     stringstream key;
     key << op << algName << nr;
     ExampleInfo* info = new ExampleInfo(ex);
     examples[key.str()].push_back(info);
  } 

  bool write() {

    CFile out(fileName);
    if ( !out.overwrite() ) {
      cerr << "Opening " << out.fileName << " failed!" << endl;
      return false;
    } 
    
    out.ios() << "Database: berlintest" << endl
              << "Restore : No" << endl << endl;

    ExampleInfo ex; 
    ostream& os = out.ios();
    initScan();
    while(next(ex)) {

      ex.print(os);
      os << endl << endl;  
    } 

    return out.close();
  } 

  string getDB() { return database; } 

  bool getRestoreFlag() { return restore; } 

/*
The functions below are provided for iterating
over all examples

*/


  inline void initScan() { scan=begin();}

  inline bool next(ExampleInfo& ex) {
     bool end = endOfScan();
     if (!end) {
       ex = *scan->second.front();
       nextOfScan();
     }
     return !end;       
  }

  inline void nextOfScan() {
     scan++;
  }

  inline bool endOfScan() const {
     return ( scan == end() );
  }

  inline const ExampleList& getCurrentList() const
  {
    return scan->second;
  }
 
  inline bool uniqueNumbers(const string& key) {
    
    set<int> usedNumbers;
    ExampleList& list = examples[key];
    ExampleList::const_iterator it = list.begin();

    for (it = list.begin(); it != list.end(); it++)
    {
      if ( !usedNumbers.insert( (*it)->number ).second ) {
        return false;
      }
    }
 
    return true;
  }


  ExampleInfo::Type resultType(const string& result)
  {
    if (  (result[0] == '(') ) 
      return ExampleInfo::List;
       
    // special result token
    if (result=="file")
      return ExampleInfo::File; 
    if (result=="file_platform")
      return ExampleInfo::PlatformFile; 
    if (result=="bug")
      return ExampleInfo::Bug; 
    if (result=="crashes")
      return ExampleInfo::Crash;

    // otherwise a list atom
    if (result=="TRUE" || result=="FALSE")
      return ExampleInfo::Atom; 
    
    char c = result[0];
    const string validChars="+-.0123456789\"'<";
    if ( validChars.find(c) != string::npos )
      return ExampleInfo::Atom; 

    return ExampleInfo::Invalid; 
  } 

};



#endif
