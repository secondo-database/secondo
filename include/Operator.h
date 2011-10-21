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

April 2006, M. Spiekermann. The file Algebra.h need to be divided into Operators.h. TypeConstructors.h, 
AlgebraClassDef.h and AlgebraInit.h   

*/

#ifndef SEC_OPERATOR_H
#define SEC_OPERATOR_H

#include <string>
#include <vector>

#include "NestedList.h"
#include "NList.h"
#include "AlgebraTypes.h"

//extern NestedList* nl;

using namespace std;

/*
1 Class ~Operator~

An operator instance consists of

  * a name

  * at least one value mapping function, sometimes called evaluation function

  * a type Mapping function, returned the operator's result type with respect
    to input parameters type

  * a selection function calculating the index of a value mapping function
    with respect to input parameter types

  * a boolean telling whether this operator supports progress queries, default is false.

  * a boolean indicating whether this operator does not want automatic evaluation of its arguments (example: ifthenelse operator), default is false.

  * a boolean indicating whether this operator needs arguments to be passed to type mapping functions, default is false.

  * a boolean indicating whether this operator uses (large) main memory buffers, default is false.

All properties of operators are set in the constructor. Only the value mapping
functions have to be registered later on since their number is arbitrary. This
number is set in the constructor (~noF~).

*/

struct OperatorInfo {

  string name;
  string signature;
  string syntax;
  string meaning;
  string example;
  string remark;
  bool supportsProgress;
  bool requestsArgs;
  bool usesArgsInTypeMapping;
  bool usesMemory;
   
  OperatorInfo() :
    name(""),  
    signature(""),  
    syntax(""),
    meaning(""),
    example(""),
    remark(""),
    supportsProgress(false),	
    requestsArgs(false),
    usesArgsInTypeMapping(false),
    usesMemory(false)	
  {}

  OperatorInfo(const OperatorInfo& o) :
    name(o.name),  
    signature(o.signature),  
    syntax(o.syntax),
    meaning(o.meaning),
    example(o.example),
    remark(o.remark),
    supportsProgress(o.supportsProgress),	
    requestsArgs(o.requestsArgs),
    usesArgsInTypeMapping(o.usesArgsInTypeMapping),
    usesMemory(o.usesMemory)	
  {}

  OperatorInfo( const string& _name,
                const string& _signature, 
                const string& _syntax, 
                const string& _meaning, 
                const string& _example )
 { 
   name = _name;
   signature = _signature;
   syntax = _syntax;
   meaning = _meaning;
   example = _example;
   remark="";
   supportsProgress = false;
   requestsArgs = false;
   usesArgsInTypeMapping = false;
   usesMemory = false;
 } 

 OperatorInfo( const string& opName, const string& specStr);

 const string str() const;
 const ListExpr list() const;

 void appendSignature(const string& sig);


 ostream& Print(ostream& o) const{
   o << "OperatorInfo[ " 
	   <<  name << ", " 
     <<  signature <<", "
     <<  syntax << ", "
     <<  meaning << ", "
     <<  example << ", "
     <<  remark << ", "
     << "supportsProgress = " << supportsProgress << ", "
     << "requestsArgs = " <<  requestsArgs << ", "
     << "usesArgsInTypeMapping = " <<  usesArgsInTypeMapping << ", "
     << "usesMemory = " <<  usesMemory << "]";
   return o;
 }
 
};

ostream& operator<<(ostream& o, const OperatorInfo& oi);

class Operator
{

 public:
  Operator( const string& nm,
            const string& spec,
            const int noF,
            ValueMapping vms[],
            SelectFunction sf,
            TypeMapping tm );
/*
Constructs an operator with ~noF~ overloaded evaluation functions.

*/
  Operator( const string& nm,
            const string& spec,
            ValueMapping vm,
            SelectFunction sf,
            TypeMapping tm );

/*
Constructs an operator with *one* evaluation functions.

*/

  Operator( const OperatorInfo& oi,
            ValueMapping vm,
            TypeMapping tm );

  Operator( const OperatorInfo& oi,
            ValueMapping vms[],
            SelectFunction sf,
            TypeMapping tm );

/*
Versions using ~OperatorInfo~.

*/

  

  virtual ~Operator()
  {
    delete[] valueMap;
    delete[] calls;
  }
/*
Destroys an operator instance.

*/
/*
Returns the operator specification as a string.

*/
  inline int Select( ListExpr argtypes ) const 
  {
    return ((*selectFunc)( argtypes ));
  }
/*
Returns the index of the overloaded evaluation function depending on
the argument types ~argtypes~.

*/
  inline int CallValueMapping( const int index,
                               ArgVector args,
                               Word& result,
                               int message,
                               Word& local,
                               Supplier s ) const
  {
    assert((0 <= index) && (index < numOfFunctions));
    calls[index]++;
    return (*valueMap[index])( args, result, message, local, s );
  }

  ValueMapping GetValueMapping( const int index ) const
  {
    assert((0 <= index) && (index < numOfFunctions));
    return valueMap[index];
  }


  
/*
Calls the value mapping function of the operator.

*/
  inline ListExpr CallTypeMapping( ListExpr argList ) const
  {
    return ((*typeMap)( argList ));
  }
/*
Calls the type mapping function of the operator.

*/
  inline static int SimpleSelect( ListExpr ) { return 0; }
/*
Defines a simple selection function for operators.

*/
  inline const string& GetName() const { return name; }
/*
Returns the name of the operator.

*/
  const ListExpr GetSpecList()   const { return spec.list(); }
  const string&  GetSpecString() const { return specString; }
/*
Returns the specification of operator ~operatorId~ of algebra ~algebraId~
as a nested list expression or as a string.

*/

  OperatorInfo  GetOpInfo() const 
  { 
    return OperatorInfo(name, specString); 
  }
  
  void SetOpInfo(const OperatorInfo& oi) 
  { 
     spec = oi;
     specString = oi.str(); 
  }

/*
Returns the specification string of the operator.

*/


  void EnableProgress() { supportsProgress = true; }
/*
Sets the ~supportsProgress~ field.

*/

  bool SupportsProgress() { return supportsProgress; }
/*
Checks the ~supportsProgress~ field.

*/


  void SetRequestsArguments() { requestsArgs = true; }
/*
Sets the ~requestsArgs~ field.

*/


  bool RequestsArguments() { return requestsArgs; }
/*
Checks the ~requestsArgs~ field.

*/


  void SetUsesArgsInTypeMapping() { usesArgsInTypeMapping = true; }
/*
Sets the ~ usesArgsInTypeMapping ~ field.

*/


  bool UsesArgsInTypeMapping() { return usesArgsInTypeMapping; }
/*
Checks the ~ usesArgsInTypeMapping ~ field.

*/


  void SetUsesMemory() { usesMemory = true; }
/*
Sets the ~ usesMemory ~ field.

*/


  bool UsesMemory() { return usesMemory; }
/*
Checks the ~ usesMemory ~ field.

*/



  inline int GetNumOfFun(){
     return numOfFunctions;
  }
/*
Returns the number of contained value mappings.

*/

  inline int GetCalls(int index){
      return calls[index];
  }


  inline void incCalls(int index){
     assert(index>=0 && index < numOfFunctions);
     calls[index]++;
  }
/*
Increments the number of calls;

*/


/*
Returns the number calls of the specified value mapping.

*/
  int GetCalls(){
    int sum = 0;
    for(int i=0;i<numOfFunctions;i++){
      sum += calls[i];
    }
    return sum;
  }

/*
Returns the number of calls for this operator.

*/

ostream& Print(ostream& o) const;


    private:

    bool AddValueMapping( const int index, ValueMapping f );
/*
Adds a value mapping function to the list of overloaded operator functions.

*/
    string         name;           // Name of operator
    string         specString;     // Specification
    OperatorInfo   spec;
    int            numOfFunctions; // No. overloaded functions
    SelectFunction selectFunc;
    ValueMapping*  valueMap;       // Array of size numOfFunctions
    unsigned int*  calls;          // counter for each overloaded version
    TypeMapping    typeMap;
    bool           supportsProgress;  //Operator supports progress queries.
    bool           requestsArgs;	//operator explicitly asks for 
						//evaluation of its arguments
    bool           usesArgsInTypeMapping;  // Operator needs arguments
	                    // to be passed to its type mapping 
						// function 
    bool           usesMemory;     // Operator uses a large memory buffer
                                   // like a tuple buffer	
};


ostream& operator<<(ostream& o, const Operator& op);

#endif
