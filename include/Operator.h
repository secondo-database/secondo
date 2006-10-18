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
#include "AlgebraTypes.h"

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
   
  OperatorInfo() :
    name(""),  
    signature(""),  
    syntax(""),
    meaning(""),
    example("")
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
 } 

 const string str() const { 
 
   const string S("<text>"); 
   const string E("</text--->"); 
   const string headStr = "(\"Signature\" \"Syntax\" \"Meaning\" \"Example\")";

   string spec = "(" + headStr + "(" 
                 + S + signature + E 
                 + S + syntax + E
                 + S + meaning + E
                 + S + example + E + "))";
   
   return spec; 
 }

 void appendSignature(const string& sig) {
 
   signature += ", " + sig;
 }  
 
};



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

  Operator( const OperatorInfo& oi,
            ValueMapping vm,
            TypeMapping tm );

  Operator( const OperatorInfo& oi,
            ValueMapping vms[],
            SelectFunction sf,
            TypeMapping tm );

  
/*
Constructs an operator with *one* evaluation functions.

*/
  virtual ~Operator()
  {
    delete[] valueMap;
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
    return (*valueMap[index])( args, result, message, local, s );
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
  inline const string& GetSpecString() const { return specString; }
  inline const string& Specification() const { return specString; }

/*
Returns the specification string of the operator.

*/

    private:

    bool AddValueMapping( const int index, ValueMapping f );
/*
Adds a value mapping function to the list of overloaded operator functions.

*/
    string         name;           // Name of operator
    string         specString;     // Specification
    int            numOfFunctions; // No. overloaded functions
    SelectFunction selectFunc;
    ValueMapping*  valueMap; // Array of size numOfFunctions
    TypeMapping    typeMap;
};

#endif
