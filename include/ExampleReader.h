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

Jan 2007, M. Spiekermann. New result tokens file\_platform, bug and 
crashes added.
Moreover the parser reports more kind of errors.

Jan 2012, T. Achmann. New result token Sequential added. This allows
Testrunner to perform the test in the .exmples files to be processed
sequentially( e.g if some tests need to be performed before others)

Jan 2012, T.Achmann. Split ExampleReader class into two classes:
ExampleReader - parses .example files for TestRunner
ExampleWriter - creates .example files in secondo/bin/tmp dir

1 Overview

This file contains datastructures to store data from .example files
of an algebra

2 Includes and defines

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


/*
3 struct ExampleInfo

This datastructure stores
information provided in an example
of an Algebra

*/  


struct ExampleInfo {

  typedef enum { List, Atom, 
                 File, PlatformFile, Bug, Crash, Unpredictable, Invalid} Type;

  std::string opName;
  std::string aliasName;
  int number;
  int lineNo;
  std::string signature;
  std::string example;
  std::string result;
  std::string remark;
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

  void print(std::ostream& os) const
  {
    os << "Operator : " << opName << std::endl;
    os << "Number   : " << number << std::endl;
    os << "Signature: " << signature << std::endl;
    os << "Example  : ";
    if (example.find("query ")==std::string::npos)
      os << "query ";
    os << example << std::endl;
    os << "Result   : " << result << std::endl;
    if(tolerance>0){
       os << "Tolerance:" << tolerance << std::endl;
       os << "relative :" <<(relativeTolerance?"true":"false") << std::endl;
    }
  } 
}; 


/*
4 class ExampleReader

This storage class is reading the Data
from the .examples file of an Algebra.
It is used in the TestRunner application

*/  

class ExampleReader {

  typedef enum { Error, Sequential, Database, Restore, Operator, 
                 Number, Signature, Example, Result, Remark,Tolerance} Token; 

  bool debug;
  int lineCtr;
  std::string line;
  std::string lineRest;
  std::string algName;
  std::string fileName;
  std::string database;
  bool restore;

  size_t pos;
  Token expected;
  std::map<Token, std::string> tokendef; 

  bool useSequentialOrder;

public:
  typedef std::list<ExampleInfo*> ExampleList; 

protected:
  typedef enum { EX_NONE, EX_READER, EX_WRITER } EType;
  EType m_type;
  typedef std::map<std::string, ExampleList> ExampleMap; 

  ExampleMap examples;

  // no default nor copy
  // using auto-generated operator= !
  ExampleReader() { assert(0); m_type = EX_NONE; }
  ExampleReader(const ExampleReader&) {assert(0); m_type = EX_NONE; }


private:

  typedef ExampleMap::const_iterator iterator;

  iterator begin() const { return examples.begin(); }
  iterator end()   const { return examples.end();   }

  iterator scan;

  void nextLine(CFile& stream, std::string& line) {
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
     
     std::string token = tokendef[t];
     pos = line.find(token); 
     if ( pos == std::string::npos ) {
       if (showErr) {
       std::cerr << errMsg() << "expecting token '" 
            << token << "'! But got '" << line << "'"
            << std::endl;
       } 
       return false;
     }
     pos = pos + token.size();
     size_t pos2 = pos;
     if (!match_Astar_B_Cstar(' ',':',' '))
     {
       if (showErr) {
       std::cerr << errMsg() << "expecting regexp (' '*':' '*)" 
            << lineCtr << "! But got '" << line.substr(pos2) << "'" 
            << std::endl;
       }
       return false;
     }

     lineRest = trim( line.substr(pos) );
     return true;
  }


  std::string errMsg() {
 
     std::stringstream s;
     s << "Parse error in line " << lineCtr << ": ";
     return s.str();
  }

  public:

  ExampleReader(const std::string& file, const std::string& algebra="") 
    : expected(Sequential) 
  {
    
    lineCtr = 0;
    line="";
    lineRest="";
    fileName = file;
    algName=algebra;
    database="";
    restore=false;

    debug = RTFlag::isActive("SI:ExampleParser:Debug");
    tokendef[Error]  = "Error";
    tokendef[Sequential]  = "Sequential";
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

    m_type = EX_READER;
    useSequentialOrder = false;
  }

 /*
  ExampleReader& operator=(const ExampleReader& src){
    expected =  src.expected;
    lineCtr = src.lineCtr;
    line = src.line;
    lineRest= src.lineRest;
    fileName = src.fileName;;
    algName= src.algName;
    database= src.database;
    restore= src.restore;

    debug  = src.debug;;
    tokendef = src.tokendef;
    pos = src.pos;
    useSequentialOrder = src.useSequentialOrder;
    examples = src.examples;
    scan = examples.begin();

    m_type = src.m_type;
    return *this;
  }
 */

  ~ExampleReader() {

    assert(m_type != EX_NONE);
    ExampleMap::iterator it = examples.begin();
    while (it != examples.end() ) 
    {
      ExampleList& list = it->second;
      ExampleList::iterator it2 = list.begin();
      while (it2 != list.end() ) {
        if(*it2){
           delete *it2;
        }
        it2++;
      }
      it++;
    } 
  }

  // get-er
  const std::string& getAlgName() const { return algName; }
  const std::string& getFileName() const { return fileName; }
  EType getType() const { return m_type; }

  // methods
  bool parse() { 

    assert(m_type != EX_NONE);

    CFile stream(fileName);
    if (!stream.open()) {;
      std::cerr << "Error: Could not open file!" << std::endl;
      return false;
    }  

    ExampleInfo* info = 0;
    std::string key="";
    
    if (useSequentialOrder)
      key = "seq";

    while (!stream.eof() && !stream.fail()) {

      // read next input std::string;
      line = "";
      lineRest ="";
      pos=0;
      nextLine(stream, line);
      while ( isSpaceStr(line) && !stream.eof()) {
        line="";
        nextLine(stream, line);
      }
      if (debug)
        cout << lineCtr << ": " << line << std::endl;
      if (debug)
        cout << tokendef[expected] << std::endl;

      bool switchAgain = true; 
      while(switchAgain) {
        switchAgain=false;
      switch (expected) {
      case Sequential:
        {
          if (!match(Sequential, false))
            {
              // no 'Sequential' token 
              switchAgain = true;
            }
          else if (getType() == EX_READER)
            {

              if (hasSuffix(lineRest, "Yes") ||
                  hasSuffix(lineRest, "YES") ||
                  hasSuffix(lineRest, "yes") )
                {
                  useSequentialOrder = true; 
                }
            }
          
          expected = Database;
          break;
        }
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

          if ( pos != std::string::npos )
            info->aliasName = trim( lineRest.substr(pos+6) );
          else 
            pos = lineRest.length();
          
          const std::string op_name = trim( lineRest.substr(0,pos) );

          if (!useSequentialOrder)
            key = op_name;

          info->opName = op_name;
          examples[key].push_back(info);
          break;
       }

       case Number: { 
          if (!match(Number))
            return false;
           expected = Signature;
           info->number = ::parse<int>(lineRest);
           if (!uniqueNumbers(key)) {
            std::cerr << errMsg() << "The numbers for operator " << key 
                 << " are not unique! Number " << info->number
                 << " is used more than one times. " << std::endl;
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
             std::cerr << "Result [" << lineRest << "] is invalid" << std::endl;
             info->result = "";
           } 
          break;
       }

      case Remark: {
           if (!match(Remark, false)) {
            if ( (info->resultType == ExampleInfo::Bug) || 
                 (info->resultType == ExampleInfo::Unpredictable)) {
              std::cerr << errMsg()
                   << "Expecting a remark field which describes "
                   << "the specified bug/ why the result is unpredictable!"
                   << std::endl;
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
             std::string tol = lineRest;
             if(tol.size()<1){
                 std::cerr << errMsg()
                      << "invalid value for tolerance"
                      << std::endl;
                 return false;
             }
             if(tol[0]=='%'){
                info->relativeTolerance=true;
                tol = tol.substr(1);
             } else {
                info->relativeTolerance=false;
             }

             // double value = parse<double>(tol);
             std::istringstream is(tol);
             double value;
             is >> value;
             if(info->relativeTolerance){
                  value = value / 100;
             }
             if(value<0){
               std::cerr << errMsg() 
                    << "negative tolerance not allowed"
                    << std::endl;
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
         std::cerr << errMsg() << std::endl; 
      }
      }

      if (debug) {
        cout << lineCtr << ": " << lineRest << std::endl;
      }  
    }
    return true;
  }

/*
Get first example of the example list

*/

  std::string getDB() { return database; } 

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
 
  inline bool uniqueNumbers(const std::string& key) {
    
    std::set<int> usedNumbers;
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


  ExampleInfo::Type resultType(const std::string& result)
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

    if (result=="unpredictable")
      return ExampleInfo::Unpredictable;

    // otherwise a list atom
    if (result=="TRUE" || result=="FALSE")
      return ExampleInfo::Atom; 
    
    char c = result[0];
    const std::string validChars="+-.0123456789\"'<";
    if ( validChars.find(c) != std::string::npos )
      return ExampleInfo::Atom; 

    return ExampleInfo::Invalid; 
  } 

};

/*
5 class ExampleWriter

This storage class is used to create an example 
of an Algebra by reading the data from the y
SecondoCatalog and then writing it into the 
secondo/bin/tmp directory

*/  
class ExampleWriter : public ExampleReader
{ 
private:
  // no default nor copy and paste
  ExampleWriter(const ExampleWriter& ew):ExampleReader(ew) 
        { assert(0); }

public:
  ExampleWriter(const std::string& file, const std::string& algebra="") 
    : ExampleReader(file, algebra) 
  {
    m_type = EX_WRITER;
  }
 
  virtual ~ExampleWriter() {}

  ExampleWriter &operator= (const ExampleWriter& ew) 
  { ExampleReader::operator=(ew); return *this; }

    bool find(const std::string& op, ExampleInfo& ex) const
  {
    ExampleReader::ExampleMap::const_iterator it = examples.find(op);
     if (it == examples.end())
       return false;

     ex = *it->second.front();
     return true;
  } 

  const ExampleList& find(const std::string& op) const
  {
     ExampleReader::ExampleMap::const_iterator it = examples.find(op);
     assert( it != examples.end() );
     return it->second;
  } 

  void add(const std::string& op, const int nr, ExampleInfo& ex)
  {
     std::stringstream key;
     key << op << getAlgName() << nr;
     ExampleInfo* info = new ExampleInfo(ex);
     examples[key.str()].push_back(info);
  } 

  bool write() {

    CFile out(getFileName());
    if ( !out.overwrite() ) {
      std::cerr << "Opening " << out.fileName << " failed!" << std::endl;
      return false;
    } 

    out.ios() << "Database: berlintest" << std::endl
              << "Restore : No" << std::endl << std::endl;

    ExampleInfo ex; 
    std::ostream& os = out.ios();
    initScan();
    while(next(ex)) {

      ex.print(os);
      os << std::endl << std::endl;  
    } 

    return out.close();
  } 

};

#endif
