/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[TOC] [\tableofcontents]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[<] [$<$]
//[>] [$>$]

[10] Header and implementation file of all implemented Types and Operators
from the ~Secondo Standard-C++ Algebra~

2016/2017 H.Brieschke created the new CompiledExpression Algebra


[TOC]


1 Overview 

In this file are declares and implemented all ~Secondo Types~ and ~Secondo Operators~
from the ~Secondo Algebra Standard-C++~, which are currently supported in the functionality
of code generation by the ~Compiled Expressions Algebra~.

These are currently the following ~Secondo Types~:

  * ~int~
  
  * ~real~
  
  * ~bool~
  
  * ~string~
  
  * ~longint~
  
  * ~rational~
  
The following ~Secondo Operators~ are also supported:

  * ~+ (plus)~
  
  * ~- (minus)~
  
  * ~[*] (product)~
  
  * ~/ (division)~
  
  * ~[<] (less)~
  
  * ~[<]= (less equal)~
  
  * ~= (equal)~
  
  * ~[>]= (greater equal)~
  
  * ~[>] (greater)~
  
  * ~mod~
  
  * ~randint~
  
  * ~not~
  
  * ~and~
  
  * ~ands~
  
  * ~or~
  
  * ~ors~

  
For this purpose, the following classes ~CECGImplStandardType[<]typename[>]~
and ~CECGImplStandardOperator[<]operatorname[>]~, which are subclasses
of ~CECGImplSecondoType~ and ~CECGImplSecondoOperator~, are declared and implemented
in this file.

Furthermore, the implemented ~CCECGImplStandardType~ and ~CECGImplStandardOperator~ in
this file are assigned to the class ~CECGImplSecondoAlgebraStandard~, which is a subclass
of ~CECGImplSecondoAlgebra~. Enabling the implementation of this forest is then done in a
second step in the file ~CECodeStore.cpp~ in the root folder from the ~Compiled Expressions Algebra~.
For further information on how to integrate the individual ~ceAlgebraXXX.h~-files, see also the
documentation in the file ~CECodeStore.cpp~.


2 Defines, includes, and constants

*/
#ifndef _COMPILED_EXPRESSIONS_INCLALG_STANDARD_H_
#define _COMPILED_EXPRESSIONS_INCLALG_STANDARD_H_

//define macro TRACE_ON if trace outputs are needed
//#define TRACE_ON
#undef TRACE_ON

#include <set>

#include "StandardTypes.h"
#include "Algebras/Standard-C++/LongInt.h"
#include "Algebras/Standard-C++/RationalAttr.h"

using namespace CompiledExpressions;
namespace CompiledExpressions {

/*
3 Class ~CECGImplStandardTypeINT~

The class ~CECGImplStandardTypeINT~, a subclass from the ~CECGImplSecondoType~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Type int~.

*/
  class CECGImplStandardTypeINT : public CECGImplSecondoType {
  public:
/*
3.1 The Constructor.

*/
    CECGImplStandardTypeINT() {}
    
/*
3.2 The Destructor.

*/
    ~CECGImplStandardTypeINT() {}
    
/*
3.3 Function ~getTypeName~

A ~get~-function returned a string with the ~Secondo Typename~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeName() {
      return CcInt::BasicType();
    }
    
/*
3.4 Function ~getTypeClassName~

A ~get~-function returned a string with the classname of the ~Secondo Type~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeClassName() {
      return "CcInt";
    }
    
/*
3.5 Function ~isCPPType~

A ~get~-function returned ~TRUE~, which means that the 
corresponding ~Secondo Type~ has a correspondence as ~C++ Type~.


This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline bool isCPPType() {
      return true;
    }
  
/*
3.6 Function ~getCPPTypeName~

A ~get~-function returned a string with the ~C++ Type Name~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getCPPTypeName() {
      return "int";
    }
    
/*
3.7 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
  };
  


/*
4 Class ~CECGImplStandardTypeREAL~

The class ~CECGImplStandardTypeREAL~, a subclass from the ~CECGImplSecondoType~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Type real~.

*/
  class CECGImplStandardTypeREAL : public CECGImplSecondoType {
  public:
/*
4.1 The Constructor.

*/
    CECGImplStandardTypeREAL() {}
    
/*
4.2 The Destructor.

*/
    ~CECGImplStandardTypeREAL() {}
    
/*
4.3 Function ~getTypeName~

A ~get~-function returned a string with the ~Secondo Typename~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeName() {
      return CcReal::BasicType();
    }
    
/*
4.4 Function ~getTypeClassName~

A ~get~-function returned a string with the classname of the ~Secondo Type~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeClassName() {
      return "CcReal";
    }
    
/*
4.5 Function ~isCPPType~

A ~get~-function returned ~TRUE~, which means that the 
corresponding ~Secondo Type~ has a correspondence as ~C++ Type~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline bool isCPPType() {
      return true;
    }
  
/*
4.6 Function ~getCPPTypeName~

A ~get~-function returned a string with the ~C++ Type Name~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getCPPTypeName() {
      return "real";
    }
    
/*
4.7 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
  };
  


/*
5 Class ~CECGImplStandardTypeBOOL~

The class ~CECGImplStandardTypeBOOL~, a subclass from the ~CECGImplSecondoType~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Type bool~.

*/
  class CECGImplStandardTypeBOOL : public CECGImplSecondoType {
  public:
/*
5.1 The Constructor.

*/
    CECGImplStandardTypeBOOL() {}
    
/*
5.2 The Destructor.

*/
    ~CECGImplStandardTypeBOOL() {}
    
/*
5.3 Function ~getTypeName~

A ~get~-function returned a string with the ~Secondo Typename~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeName() {
      return CcBool::BasicType();
    }
    
/*
5.4 Function ~getTypeClassName~

A ~get~-function returned a string with the classname of the ~Secondo Type~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeClassName() {
      return "CcBool";
    }
    
/*
5.5 Function ~isCPPType~

A ~get~-function returned ~TRUE~, which means that the 
corresponding ~Secondo Type~ has a correspondence as ~C++ Type~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline bool isCPPType() {
      return true;
    }
  
/*
5.6 Function ~getCPPTypeName~

A ~get~-function returned a string with the ~C++ Type Name~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getCPPTypeName() {
      return "bool";
    }
    
/*
5.7 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
  };
  


/*
6 Class ~CECGImplStandardTypeSTRING~

The class ~CECGImplStandardTypeSTRING~, a subclass from the ~CECGImplSecondoType~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Type string~.

*/
  class CECGImplStandardTypeSTRING : public CECGImplSecondoType {
  public:
/*
6.1 The Constructor.

*/
    CECGImplStandardTypeSTRING() {}
    
/*
6.2 The Destructor.

*/
    ~CECGImplStandardTypeSTRING() {}
    
/*
6.3 Function ~getTypeName~

A ~get~-function returned a string with the ~Secondo Typename~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeName() {
      return CcString::BasicType();
    }
    
/*
6.4 Function ~getTypeClassName~

A ~get~-function returned a string with the classname of the ~Secondo Type~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeClassName() {
      return "CcString";
    }
    
/*
6.5 Function ~isCPPType~

A ~get~-function returned ~TRUE~, which means that the 
corresponding ~Secondo Type~ has a correspondence as ~C++ Type~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline bool isCPPType() {
      return true;
    }
  
/*
6.6 Function ~getCPPTypeName~

A ~get~-function returned a string with the ~C++ Type Name~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getCPPTypeName() {
      return "std::string";
    }
    
/*
6.7 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
  };
  


/*
7 Class ~CECGImplStandardTypeLONGINT~

The class ~CECGImplStandardTypeLONGINT~, a subclass from the ~CECGImplSecondoType~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Type longint~.

*/
  class CECGImplStandardTypeLONGINT : public CECGImplSecondoType {
  public:
/*
7.1 The Constructor.

*/
    CECGImplStandardTypeLONGINT() {}
    
/*
7.2 The Destructor.

*/
    ~CECGImplStandardTypeLONGINT() {}
    
/*
7.3 Function ~getTypeName~

A ~get~-function returned a string with the ~Secondo Typename~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeName() {
      return LongInt::BasicType();
    }
    
/*
7.4 Function ~getTypeClassName~

A ~get~-function returned a string with the classname of the ~Secondo Type~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeClassName() {
      return "LongInt";
    }
    
/*
7.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_REST].insert("../../Standard-C++/LongInt.h");
    }
  };
  


/*
8 Class ~CECGImplStandardTypeRATIONAL~

The class ~CECGImplStandardTypeRATIONAL~, a subclass from the ~CECGImplSecondoType~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Type rational~.

*/
  class CECGImplStandardTypeRATIONAL : public CECGImplSecondoType {
  public:
/*
8.1 The Constructor.

*/
    CECGImplStandardTypeRATIONAL() {}
    
/*
8.2 The Destructor.

*/
    ~CECGImplStandardTypeRATIONAL() {}
    
/*
8.3 Function ~getTypeName~

A ~get~-function returned a string with the ~Secondo Typename~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeName() {
      return Rational::BasicType();
    }
    
/*
8.4 Function ~getTypeClassName~

A ~get~-function returned a string with the classname of the ~Secondo Type~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeClassName() {
      return "Rational";
    }
    
/*
8.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_REST].insert("../../Standard-C++/RationalAttr.h");
    }
  };
  


/*
9 Class ~CECGImplStandardOperatorPLUS~

The class ~CECGImplStandardOperatorPLUS~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator + (plus)~.

*/
  class CECGImplStandardOperatorPLUS : public CECGImplSecondoOperator {
  public:
/*
9.1 The Constructor.

*/
    CECGImplStandardOperatorPLUS() {}
    
/*
9.2 The Destructor.

*/
    ~CECGImplStandardOperatorPLUS() {}
    
/*
9.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
9.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "+";
    }
    
/*
9.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
9.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
      (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 3)
        throw CECRuntimeError("Signature must contain 3 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append(signature[2]->getTypeClassName());
        code.append("* localArg_1 = arg_1->eval();\n");
        code.append("if (localArg_0->IsDefined()");
        code.append("&& localArg_1->IsDefined()) {\n");

        if (signature[1]->getTypeName() == "int"
          && signature[2]->getTypeName() == "int") {
          code.append("resultStorage->Set(true, localArg_0->GetIntval()");
          code.append(" + localArg_1->GetIntval());\n");
        
        } else if (signature[1]->getTypeName() == "real"
          && signature[2]->getTypeName() == "int") {
          code.append("resultStorage->Set(true, localArg_0->GetRealval()");
          code.append(" + localArg_1->GetIntval());\n");
        
        } else if (signature[1]->getTypeName() == "int"
          && signature[2]->getTypeName() == "real") {
          code.append("resultStorage->Set(true, localArg_0->GetIntval()");
          code.append(" + localArg_1->GetRealval());\n");
        
        } else if (signature[1]->getTypeName() == "real"
          && signature[2]->getTypeName() == "real") {
          code.append("resultStorage->Set(true, localArg_0->GetRealval()");
          code.append(" + localArg_1->GetRealval());\n");
        
        } else if (signature[1]->getTypeName() == "string"
          && signature[2]->getTypeName() == "string") {
          code.append("std::string str1 = reinterpret_cast<const char*>");
          code.append("(localArg_0->GetStringval());\n");
          code.append("std::string str2 = reinterpret_cast<const char*>");
          code.append("(localArg_1->GetStringval());\n");        
          code.append("resultStorage->Set(true, (STRING_T*)(str1 + str2)");
          code.append(".substr(0, MAX_STRINGSIZE).c_str());\n");


        } else if (signature[1]->getTypeName() == "int" 
          && signature[2]->getTypeName() == "longint") {
          code.append("resultStorage->Set(true, localArg_0->GetIntval()");
          code.append(" + localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "longint"
          && signature[2]->getTypeName() == "int") {
          code.append("resultStorage->Set(true, localArg_0->GetValue()");
          code.append(" + localArg_1->GetIntval());\n");
        
        } else if (signature[1]->getTypeName() == "longint"
          && signature[2]->getTypeName() == "longint") {
          code.append("resultStorage->Set(true, localArg_0->GetValue()");
          code.append(" + localArg_1->GetValue());\n");
        
        
        } else if (signature[1]->getTypeName() == "rational"
          && signature[2]->getTypeName() == "int") {
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" + localArg_1->GetIntval());\n");
        
        } else if (signature[1]->getTypeName() == "rational"
          && signature[2]->getTypeName() == "longint") {
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" + localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "int"
          && signature[2]->getTypeName() == "rational") {
          code.append("resultStorage->Set(localArg_0->GetIntval()");
          code.append(" + localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "longint"
          && signature[2]->getTypeName() == "rational") {
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" + localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "rational"
          && signature[2]->getTypeName() == "rational") {
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" + localArg_1->GetValue());\n");
        }
        
        code.append("} else {\n");
        if (signature[0]->getTypeName() == "int")
          code.append("resultStorage->Set(false, 0);\n");
        else if (signature[0]->getTypeName() == "real")
          code.append("resultStorage->Set(false, 0.0);\n");
        else if (signature[0]->getTypeName() == "string")
          code.append("resultStorage->Set(false, \"\");\n");
        else if (signature[0]->getTypeName() == "longint")
          code.append("resultStorage->Set(false, 0);\n");
        else if (signature[0]->getTypeName() == "rational")
          code.append("resultStorage->Set(false);\n");

        code.append("}\n");
      }
      return code;
    }
  };
  

  
/*
10 Class ~CECGImplStandardOperatorMINUS~

The class ~CECGImplStandardOperatorMINUS~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator - (minus)~.

*/
  class CECGImplStandardOperatorMINUS : public CECGImplSecondoOperator {
  public:
/*
10.1 The Constructor.

*/
    CECGImplStandardOperatorMINUS() {}
    
/*
10.2 The Destructor.

*/
    ~CECGImplStandardOperatorMINUS() {}
    
/*
10.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
10.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "-";
    }
    
/*
10.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
10.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 3)
        throw CECRuntimeError("Signature must contain 3 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append(signature[2]->getTypeClassName());
        code.append("* localArg_1 = arg_1->eval();\n");
        code.append("if (localArg_0->IsDefined()");
        code.append("&& localArg_1->IsDefined()) {\n");
        
        if (signature[1]->getTypeName() == "int"
          && signature[2]->getTypeName() == "int") {
          code.append("int a = localArg_0->GetIntval();\n");
          code.append("int b = localArg_1->GetIntval();\n");
          code.append("int diff = a-b;\n");
          code.append("if( ((b>0) && (diff>a) ) || ((b<0) && (diff<a)))\n");
          code.append("resultStorage->Set(false, 0);\n");
          code.append("else\n");
          code.append("resultStorage->Set(true, diff);\n");
        
        } else if (signature[1]->getTypeName() == "real"
          && signature[2]->getTypeName() == "int") {
          code.append("resultStorage->Set(true, localArg_0->GetRealval()");
          code.append(" - localArg_1->GetIntval());\n");
        
        } else if (signature[1]->getTypeName() == "int"
          && signature[2]->getTypeName() == "real") {
          code.append("resultStorage->Set(true, localArg_0->GetIntval()");
          code.append(" - localArg_1->GetRealval());\n");
        
        } else if (signature[1]->getTypeName() == "real"
          && signature[2]->getTypeName() == "real") {
          code.append("resultStorage->Set(true, localArg_0->GetRealval()");
          code.append(" - localArg_1->GetRealval());\n");
        
        
        } else if (signature[1]->getTypeName() == "int"
          && signature[2]->getTypeName() == "longint") {
          code.append("resultStorage->Set(true, localArg_0->GetIntval()");
          code.append(" - localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "longint"
          && signature[2]->getTypeName() == "int") {
          code.append("resultStorage->Set(true, localArg_0->GetValue()");
          code.append(" - localArg_1->GetIntval());\n");
        
        } else if (signature[1]->getTypeName() == "longint"
          && signature[2]->getTypeName() == "longint") {
          code.append("resultStorage->Set(true, localArg_0->GetValue()");
          code.append(" - localArg_1->GetValue());\n");
        
        
        } else if (signature[1]->getTypeName() == "rational"
          && signature[2]->getTypeName() == "int") {
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" - localArg_1->GetIntval());\n");
        
        } else if (signature[1]->getTypeName() == "rational"
          && signature[2]->getTypeName() == "longint") {
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" - localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "int"
          && signature[2]->getTypeName() == "rational") {
          code.append("resultStorage->Set(localArg_0->GetIntval()");
          code.append(" - localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "longint"
          && signature[2]->getTypeName() == "rational") {
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" - localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "rational"
          && signature[2]->getTypeName() == "rational") {
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" - localArg_1->GetValue());\n");
        }
        
        code.append("} else {\n");
        if (signature[0]->getTypeName() == "int")
          code.append("resultStorage->Set(false, 0);\n");
        else if (signature[0]->getTypeName() == "real")
          code.append("resultStorage->Set(false, 0.0);\n");
        else if (signature[0]->getTypeName() == "string")
          code.append("resultStorage->Set(false, \"\");\n");
        else if (signature[0]->getTypeName() == "longint")
          code.append("resultStorage->Set(false, 0);\n");
        else if (signature[0]->getTypeName() == "rational")
          code.append("resultStorage->Set(false);\n");

        code.append("}\n");
      }
      return code;
    }
  };
  


/*
11 Class ~CECGImplStandardOperatorPRODUCT~

The class ~CECGImplStandardOperatorPRODUCT~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator * (product)~.

*/
  class CECGImplStandardOperatorPRODUCT : public CECGImplSecondoOperator {
  public:
/*
11.1 The Constructor.

*/
    CECGImplStandardOperatorPRODUCT() {}
    
/*
11.2 The Destructor.

*/
    ~CECGImplStandardOperatorPRODUCT() {}
    
/*
11.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
11.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "*";
    }
    
/*
11.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
11.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 3)
        throw CECRuntimeError("Signature must contain 3 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append(signature[2]->getTypeClassName());
        code.append("* localArg_1 = arg_1->eval();\n");
        code.append("if (localArg_0->IsDefined()");
        code.append(" && localArg_1->IsDefined()) {\n");
        
        if (signature[1]->getTypeName() == "int"
          && signature[2]->getTypeName() == "int") {
          code.append("int a = localArg_0->GetIntval();\n");
          code.append("int b = localArg_1->GetIntval();\n");
          code.append("int prod = a*b;\n");
          code.append("if((b!=0) && ((prod/b)!=a))\n");
          code.append("resultStorage->Set(false, 0);\n");
          code.append("else\n");
          code.append("resultStorage->Set(true, prod);\n");
        
        } else if (signature[1]->getTypeName() == "real"
          && signature[2]->getTypeName() == "int") {
          code.append("resultStorage->Set(true, localArg_0->GetRealval()");
          code.append(" * localArg_1->GetIntval());\n");
        
        } else if (signature[1]->getTypeName() == "int"
          && signature[2]->getTypeName() == "real") {
          code.append("resultStorage->Set(true, localArg_0->GetIntval()");
          code.append(" * localArg_1->GetRealval());\n");
        
        } else if (signature[1]->getTypeName() == "real"
          && signature[2]->getTypeName() == "real") {
          code.append("resultStorage->Set(true, localArg_0->GetRealval()");
          code.append(" * localArg_1->GetRealval());\n");
        
        
        } else if (signature[1]->getTypeName() == "int"
          && signature[2]->getTypeName() == "longint") {
          code.append("resultStorage->Set(true, localArg_0->GetIntval()");
          code.append(" * localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "longint"
          && signature[2]->getTypeName() == "int") {
          code.append("resultStorage->Set(true, localArg_0->GetValue()");
          code.append(" * localArg_1->GetIntval());\n");
        
        } else if (signature[1]->getTypeName() == "longint"
          && signature[2]->getTypeName() == "longint") {
          code.append("resultStorage->Set(true, localArg_0->GetValue()");
          code.append(" * localArg_1->GetValue());\n");
        
        
        } else if (signature[1]->getTypeName() == "rational"
          && signature[2]->getTypeName() == "int") {
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" * localArg_1->GetIntval());\n");
        
        } else if (signature[1]->getTypeName() == "rational"
          && signature[2]->getTypeName() == "longint") {
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" * localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "int" 
          && signature[2]->getTypeName() == "rational") {
          code.append("resultStorage->Set(localArg_0->GetIntval()");
          code.append(" * localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "longint"
          && signature[2]->getTypeName() == "rational") {
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" * localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "rational"
          && signature[2]->getTypeName() == "rational") {
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" * localArg_1->GetValue());\n");
        }
        
        code.append("} else {\n");
        if (signature[0]->getTypeName() == "int")
          code.append("resultStorage->Set(false, 0);\n");
        else if (signature[0]->getTypeName() == "real")
          code.append("resultStorage->Set(false, 0.0);\n");
        else if (signature[0]->getTypeName() == "string")
          code.append("resultStorage->Set(false, \"\");\n");
        else if (signature[0]->getTypeName() == "longint")
          code.append("resultStorage->Set(false, 0);\n");
        else if (signature[0]->getTypeName() == "rational")
          code.append("resultStorage->Set(false);\n");

        code.append("}\n");
      }
      return code;
    }
  };
  


/*
12 Class ~CECGImplStandardOperatorDIVISION~

The class ~CECGImplStandardOperatorDIVISION~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator / (division)~.

*/
  class CECGImplStandardOperatorDIVISION : public CECGImplSecondoOperator {
  public:
/*
12.1 The Constructor.

*/
    CECGImplStandardOperatorDIVISION() {}
    
/*
12.2 The Destructor.

*/
    ~CECGImplStandardOperatorDIVISION() {}
    
/*
12.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
12.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "/";
    }
    
/*
12.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
12.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 3)
        throw CECRuntimeError("Signature must contain 3 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append(signature[2]->getTypeClassName());
        code.append("* localArg_1 = arg_1->eval();\n");
        code.append("if (localArg_0->IsDefined()");
        code.append(" && localArg_1->IsDefined()");
        if (signature[1]->getTypeName() == "int"
          && signature[2]->getTypeName() == "int") {
          code.append(" && localArg_1->GetIntval()) {\n");
          code.append("resultStorage->Set(true, localArg_0->GetIntval()");
          code.append(" / localArg_1->GetIntval());\n");
        
        } else if (signature[1]->getTypeName() == "real"
          && signature[2]->getTypeName() == "int") {
          code.append(" && localArg_1->GetIntval()) {\n");
          code.append("resultStorage->Set(true, localArg_0->GetRealval()");
          code.append(" / localArg_1->GetIntval());\n");
        
        } else if (signature[1]->getTypeName() == "int"
          && signature[2]->getTypeName() == "real") {
          code.append(" && localArg_1->GetRealval()) {\n");
          code.append("resultStorage->Set(true, localArg_0->GetIntval()");
          code.append(" / localArg_1->GetRealval());\n");
        
        } else if (signature[1]->getTypeName() == "real"
          && signature[2]->getTypeName() == "real"){
          code.append(" && localArg_1->GetRealval()) {\n");
          code.append("resultStorage->Set(true, localArg_0->GetRealval()");
          code.append(" / localArg_1->GetRealval());\n");
        
        
        } else if (signature[1]->getTypeName() == "int"
          && signature[2]->getTypeName() == "longint") {
          code.append(" && localArg_1->GetValue()) {\n");
          code.append("resultStorage->Set(true, localArg_0->GetIntval()");
          code.append(" / localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "longint"
          && signature[2]->getTypeName() == "int") {
          code.append(" && localArg_1->GetIntval()) {\n");
          code.append("resultStorage->Set(true, localArg_0->GetValue()");
          code.append(" / localArg_1->GetIntval());\n");
        
        } else if (signature[1]->getTypeName() == "longint"
          && signature[2]->getTypeName() == "longint") {
          code.append(" && localArg_1->GetValue()) {\n");
          code.append("resultStorage->Set(true, localArg_0->GetValue()");
          code.append(" / localArg_1->GetValue());\n");
        
        
        } else if (signature[1]->getTypeName() == "rational"
          && signature[2]->getTypeName() == "int") {
          code.append(" && localArg_1->GetIntval()) {\n");
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" / localArg_1->GetIntval());\n");
        
        } else if (signature[1]->getTypeName() == "rational"
          && signature[2]->getTypeName() == "longint") {
          code.append(" && localArg_1->GetValue()) {\n");
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" / localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "int"
          && signature[2]->getTypeName() == "rational") {
          code.append(" && localArg_1->GetValue()) {\n");
          code.append("resultStorage->Set(localArg_0->GetIntval()");
          code.append(" / localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "longint"
          && signature[2]->getTypeName() == "rational") {
          code.append(" && localArg_1->GetValue()) {\n");
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" / localArg_1->GetValue());\n");
        
        } else if (signature[1]->getTypeName() == "rational"
          && signature[2]->getTypeName() == "rational") {
          code.append(" && localArg_1->GetValue()) {\n");
          code.append("resultStorage->Set(localArg_0->GetValue()");
          code.append(" / localArg_1->GetValue());\n");
        }
        
        
        code.append("} else {\n");
        if (signature[0]->getTypeName() == "int")
          code.append("resultStorage->Set(false, 0);\n");
        else if (signature[0]->getTypeName() == "real")
          code.append("resultStorage->Set(false, 0.0);\n");
        else if (signature[0]->getTypeName() == "string")
          code.append("resultStorage->Set(false, \"\");\n");
        else if (signature[0]->getTypeName() == "longint")
          code.append("resultStorage->Set(false, 0);\n");
        else if (signature[0]->getTypeName() == "rational")
          code.append("resultStorage->Set(false);\n");

        code.append("}\n");
      }
      return code;
    }
  };
  

  
/*
13 Class ~CECGImplStandardOperatorLESS~

The class ~CECGImplStandardOperatorLESS~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator < (less)~.

*/
  class CECGImplStandardOperatorLESS : public CECGImplSecondoOperator {
  public:
/*
13.1 The Constructor.

*/
    CECGImplStandardOperatorLESS() {}
    
/*
13.2 The Destructor.

*/
    ~CECGImplStandardOperatorLESS() {}
    
/*
13.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
13.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "<";
    }
    
/*
13.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
13.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 3)
        throw CECRuntimeError("Signature must contain 3 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append(signature[2]->getTypeClassName());
        code.append("* localArg_1 = arg_1->eval();\n");
        code.append("if (localArg_0->IsDefined()");
        code.append(" && localArg_1->IsDefined()) {\n");
        
        code.append("resultStorage->Set(true,");
        code.append(" localArg_0->Compare(localArg_1) == -1);\n");
        
        code.append("} else {\n");
        code.append("resultStorage->Set(false, false);\n");
        code.append("}\n");
      }
      return code;
    }
  };
  


/*
14 Class ~CECGImplStandardOperatorLESSEQUAL~

The class ~CECGImplStandardOperatorLESSEQUAL~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator <= (less equal)~.

*/
  class CECGImplStandardOperatorLESSEQUAL : public CECGImplSecondoOperator {
  public:
/*
14.1 The Constructor.

*/
    CECGImplStandardOperatorLESSEQUAL() {}
    
/*
14.2 The Destructor.

*/
    ~CECGImplStandardOperatorLESSEQUAL() {}
    
/*
14.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
14.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "<=";
    }
    
/*
14.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
14.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 3)
        throw CECRuntimeError("Signature must contain 3 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append(signature[2]->getTypeClassName());
        code.append("* localArg_1 = arg_1->eval();\n");
        code.append("if (localArg_0->IsDefined()");
        code.append(" && localArg_1->IsDefined()) {\n");
        
        code.append("resultStorage->Set(true,");
        code.append(" localArg_0->Compare(localArg_1) <= 0);\n");
        
        code.append("} else {\n");
        code.append("resultStorage->Set(false, false);\n");
        code.append("}\n");
      }
      return code;
    }
  };
  


/*
15 Class ~CECGImplStandardOperatorEQUAL~

The class ~CECGImplStandardOperatorEQUAL~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator = (equal)~.

*/
  class CECGImplStandardOperatorEQUAL : public CECGImplSecondoOperator {
  public:
/*
15.1 The Constructor.

*/
    CECGImplStandardOperatorEQUAL() {}
    
/*
15.2 The Destructor.

*/
    ~CECGImplStandardOperatorEQUAL() {}
    
/*
15.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
15.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "=";
    }
    
/*
15.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
15.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 3)
        throw CECRuntimeError("Signature must contain 3 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append(signature[2]->getTypeClassName());
        code.append("* localArg_1 = arg_1->eval();\n");
        code.append("if (localArg_0->IsDefined()");
        code.append(" && localArg_1->IsDefined()) {\n");
        
        code.append("resultStorage->Set(true,");
        code.append(" localArg_0->Compare(localArg_1) == 0);\n");
        
        code.append("} else {\n");
        code.append("resultStorage->Set(false, false);\n");
        code.append("}\n");
      }
      return code;
    }
  };
  


/*
16 Class ~CECGImplStandardOperatorGREATEREQUAL~

The class ~CECGImplStandardOperatorGREATEREQUAL~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator >= (greater equal)~.

*/
  class CECGImplStandardOperatorGREATEREQUAL : public CECGImplSecondoOperator {
  public:
/*
16.1 The Constructor.

*/
    CECGImplStandardOperatorGREATEREQUAL() {}
    
/*
16.2 The Destructor.

*/
    ~CECGImplStandardOperatorGREATEREQUAL() {}
    
/*
16.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
16.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return ">=";
    }
    
/*
16.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
16.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 3)
        throw CECRuntimeError("Signature must contain 3 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append(signature[2]->getTypeClassName());
        code.append("* localArg_1 = arg_1->eval();\n");
        code.append("if (localArg_0->IsDefined()");
        code.append(" && localArg_1->IsDefined()) {\n");
        
        code.append("resultStorage->Set(true,");
        code.append(" localArg_0->Compare(localArg_1) >= 0);\n");
        
        code.append("} else {\n");
        code.append("resultStorage->Set(false, false);\n");
        code.append("}\n");
      }
      return code;
    }
  };
  


/*
17 Class ~CECGImplStandardOperatorGREATER~

The class ~CECGImplStandardOperatorGREATER~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator > (greater)~.

*/
  class CECGImplStandardOperatorGREATER : public CECGImplSecondoOperator {
  public:
/*
17.1 The Constructor.

*/
    CECGImplStandardOperatorGREATER() {}
    
/*
17.2 The Destructor.

*/
    ~CECGImplStandardOperatorGREATER() {}
    
/*
17.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
17.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return ">";
    }
    
/*
17.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
17.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 3)
        throw CECRuntimeError("Signature must contain 3 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append(signature[2]->getTypeClassName());
        code.append("* localArg_1 = arg_1->eval();\n");
        code.append("if (localArg_0->IsDefined()");
        code.append(" && localArg_1->IsDefined()) {\n");
        
        code.append("resultStorage->Set(true,");
        code.append(" localArg_0->Compare(localArg_1) == 1);\n");
        
        code.append("} else {\n");
        code.append("resultStorage->Set(false, false);\n");
        code.append("}\n");
      }
      return code;
    }
  };
  


/*
18 Class ~CECGImplStandardOperatorMOD~

The class ~CECGImplStandardOperatorMOD~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator mod~.

*/
  class CECGImplStandardOperatorMOD : public CECGImplSecondoOperator {
  public:
/*
18.1 The Constructor.

*/
    CECGImplStandardOperatorMOD() {}
    
/*
18.2 The Destructor.

*/
    ~CECGImplStandardOperatorMOD() {}
    
/*
18.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
18.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "mod";
    }
    
/*
18.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
18.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 3)
        throw CECRuntimeError("Signature must contain 3 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append(signature[2]->getTypeClassName());
        code.append("* localArg_1 = arg_1->eval();\n");
        code.append("if (localArg_0->IsDefined()");
        code.append(" && localArg_1->IsDefined()) {\n");
        
        code.append("resultStorage->Set(true,");
        code.append(" localArg_0->GetIntval() % localArg_1->GetIntval());\n");
        
        code.append("} else {\n");
        code.append("resultStorage->Set(false, 0);\n");
        code.append("}\n");
      }
      return code;
    }
  };
  
/*
19 Class ~CECGImplStandardOperatorRANDINT~

The class ~CECGImplStandardOperatorRANDINT~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator randint~.

*/
  class CECGImplStandardOperatorRANDINT : public CECGImplSecondoOperator {
  public:
/*
19.1 The Constructor.

*/
    CECGImplStandardOperatorRANDINT() {}
    
/*
19.2 The Destructor.

*/
    ~CECGImplStandardOperatorRANDINT() {}
    
/*
19.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
19.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "randint";
    }
    
/*
19.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_CPP].insert("time.h");
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
19.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 2)
        throw CECRuntimeError("Signature must contain 3 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append("if (localArg_0->IsDefined()) {\n");
        code.append("int u = localArg_0->GetIntval();\n");
        code.append("if (u < 2) {u=2; srand(time(NULL));}\n");
        code.append("resultStorage->Set(true,");
        code.append(" (int)((float)u * rand()/(RAND_MAX+1.0)));\n");
        
        code.append("} else {\n");
        code.append("resultStorage->Set(false, 0);\n");
        code.append("}\n");
      }
      return code;
    }
  };
  


/*
20 Class ~CECGImplStandardOperatorNOT~

The class ~CECGImplStandardOperatorNOT~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator not~.

*/
  class CECGImplStandardOperatorNOT : public CECGImplSecondoOperator {
  public:
/*
20.1 The Constructor.

*/
    CECGImplStandardOperatorNOT() {}
    
/*
20.2 The Destructor.

*/
    ~CECGImplStandardOperatorNOT() {}
    
/*
20.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
20.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "not";
    }
    
/*
20.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
20.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 2)
        throw CECRuntimeError("Signature must contain 2 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append("if (localArg_0->IsDefined())\n");
        code.append("resultStorage->Set(true, !localArg_0->GetBoolval());\n");
        code.append("else\n");
        code.append("resultStorage->Set(false, false);\n");
      }
      return code;
    }
  };
  


/*
21 Class ~CECGImplStandardOperatorAND~

The class ~CECGImplStandardOperatorAND~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator and~.

*/
  class CECGImplStandardOperatorAND : public CECGImplSecondoOperator {
  public:
/*
21.1 The Constructor.

*/
    CECGImplStandardOperatorAND() {}
    
/*
21.2 The Destructor.

*/
    ~CECGImplStandardOperatorAND() {}
    
/*
21.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
21.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "and";
    }
    
/*
21.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
21.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 3)
        throw CECRuntimeError("Signature must contain 3 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append(signature[2]->getTypeClassName());
        code.append("* localArg_1 = arg_1->eval();\n");
        code.append("if (!localArg_0->IsDefined())\n");
        code.append("resultStorage->Set(true, false);\n");
        code.append("else if (!localArg_1->IsDefined())\n");
        code.append("resultStorage->Set(true, false);\n");
        code.append("else \n");
        code.append("resultStorage->Set(true, localArg_0->GetBoolval()");
        code.append(" && localArg_1->GetBoolval());\n");
      }
      return code;
    }
  };
  
  
/*
22 Class ~CECGImplStandardOperatorANDS~

The class ~CECGImplStandardOperatorANDS~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator ands~.

*/
  class CECGImplStandardOperatorANDS : public CECGImplSecondoOperator {
  public:
/*
22.1 The Constructor.

*/
    CECGImplStandardOperatorANDS() {}
    
/*
22.2 The Destructor.

*/
    ~CECGImplStandardOperatorANDS() {}
    
/*
22.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
22.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "ands";
    }
    
/*
22.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
22.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 3)
        throw CECRuntimeError("Signature must contain 3 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append(signature[2]->getTypeClassName());
        code.append("* localArg_1 = arg_1->eval();\n");
        code.append("if (!localArg_0->IsDefined()");
        code.append(" || !localArg_1->IsDefined())\n");
        
        code.append("resultStorage->Set(false, false);\n");
        code.append("else \n");
        code.append("resultStorage->Set(true, localArg_0->GetBoolval()");
        code.append(" && localArg_1->GetBoolval());\n");
      }
      return code;
    }
  };
  


/*
23 Class ~CECGImplStandardOperatorOR~

The class ~CECGImplStandardOperatorOR~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator or~.

*/
  class CECGImplStandardOperatorOR : public CECGImplSecondoOperator {
  public:
/*
23.1 The Constructor.

*/
    CECGImplStandardOperatorOR() {}
    
/*
23.2 The Destructor.

*/
    ~CECGImplStandardOperatorOR() {}
    
/*
23.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
23.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "or";
    }
    
/*
23.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
23.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 3)
        throw CECRuntimeError("Signature must contain 3 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append(signature[2]->getTypeClassName());
        code.append("* localArg_1 = arg_1->eval();\n");
        code.append("if (localArg_0->IsDefined()");
        code.append(" && localArg_0->GetBoolval())\n");
        
        code.append("resultStorage->Set(true, true);\n");
        code.append("else if (localArg_1->IsDefined()");
        code.append(" && localArg_1->GetBoolval())\n");
        code.append("resultStorage->Set(true, true);\n");
        
        code.append("else \n");
        code.append("resultStorage->Set(true, false);\n");
      }
      return code;
    }
  };
  


/*
24 Class ~CECGImplStandardOperatorORS~

The class ~CECGImplStandardOperatorORS~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator ors~.

*/
  class CECGImplStandardOperatorORS : public CECGImplSecondoOperator {
  public:
/*
24.1 The Constructor.

*/
    CECGImplStandardOperatorORS() {}
    
/*
24.2 The Destructor.

*/
    ~CECGImplStandardOperatorORS() {}
    
/*
24.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
24.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "ors";
    }
    
/*
24.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_SECONDO].insert("StandardTypes.h");
    }
    
/*
24.6 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 3)
        throw CECRuntimeError("Signature must contain 3 elements.");
      else {
        code.append(signature[1]->getTypeClassName());
        code.append("* localArg_0 = arg_0->eval();\n");
        code.append(signature[2]->getTypeClassName());
        code.append("* localArg_1 = arg_1->eval();\n");
        code.append("if (!localArg_0->IsDefined()");
        code.append(" || !localArg_1->IsDefined())\n");
        
        code.append("resultStorage->Set(false, false);\n");
        code.append("else \n");
        code.append("resultStorage->Set(true, localArg_0->GetBoolval()");
        code.append(" || localArg_1->GetBoolval());\n");
      }
      return code;
    }
  };
  


/*
25 Class ~CECGImplSecondoAlgebraStandard~

The class ~CECGImplSecondoAlgebraStandard~, a subclass from the ~CECGImplSecondoAlgebra~,
implements a data structure to save all implemented ~Types~ and ~Operators~ into this file.

*/
  class CECGImplSecondoAlgebra_Standard : public CECGImplSecondoAlgebra {
  public:
/*
25.1 The Constructor.

*/
    CECGImplSecondoAlgebra_Standard()
    : CECGImplSecondoAlgebra(std::string("StandardAlgebra")) {}
    
/*
25.2 The Destructor.

*/
    virtual ~CECGImplSecondoAlgebra_Standard() {}

/*
25.3 Function ~loadCECGImplSecondoTypes~

This function loads the implemented types into the code store of the ~CECompiler~.
In this function created for all ~CECGImplStandardType~-classes an object and
call the ~loadCECGImplSecondoType~-function from ~CECodeGenerator~ to load
this ~CECGImplStandardType~-object. The call has the following syntax:

----

ceCG->loadCECGImplSecondoType(new CECGImplStandardTypeXXX());

----


This function overrides the function of the base class ~CECGImplSecondoAlgebra~.

*/
    void loadCECGImplSecondoTypes(CECodeGenerator* ceCG) {
      //list all types
      ceCG->loadCECGImplSecondoType(new CECGImplStandardTypeINT());
      ceCG->loadCECGImplSecondoType(new CECGImplStandardTypeREAL());
      ceCG->loadCECGImplSecondoType(new CECGImplStandardTypeBOOL());
      ceCG->loadCECGImplSecondoType(new CECGImplStandardTypeSTRING());
      ceCG->loadCECGImplSecondoType(new CECGImplStandardTypeLONGINT());
      ceCG->loadCECGImplSecondoType(new CECGImplStandardTypeRATIONAL());
      
    }
    
/*
25.4 Function ~loadCECGImplSecondoOperators~

This function loads the implemented operators into the code store of the ~CECompiler~.
In this function created for all ~CECGImplStandardOperator~-classes an object and
call the ~loadCECGImplSecondoOperator~-function from ~CECodeGenerator~ to load
this ~CECGImplStandardOperator~-object. The call has the following syntax:

----

ceCG->loadCECGImplSecondoOperator
        (algName, new CECGImplStandardOperatorXXX());

----


This function overrides the function of the base class ~CECGImplSecondoAlgebra~.

*/
    void loadCECGImplSecondoOperators(CECodeGenerator* ceCG) {
      //list all operators
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorPLUS());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorMINUS());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorPRODUCT());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorDIVISION());
      
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorLESS());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorLESSEQUAL());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorEQUAL());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorGREATEREQUAL());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorGREATER());
      
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorMOD());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorRANDINT());

      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorNOT());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorAND());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorANDS());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorOR());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplStandardOperatorORS());

    }
  private:
  };
} // end of namespace CompiledExpressions
#endif // _COMPILED_EXPRESSIONS_INCLALG_STANDARD_H_
