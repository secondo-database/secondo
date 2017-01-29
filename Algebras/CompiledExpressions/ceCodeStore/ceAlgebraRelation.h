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
from the ~Secondo Relation-C++ Algebra~

2016/2017 H.Brieschke created the new CompiledExpression Algebra


[TOC]


1 Overview 

In this file are declares and implemented all ~Secondo Types~ and ~Secondo Operators~
from the ~Secondo Algebra Relation-C++~, which are currently supported in the functionality
of code generation by the ~Compiled Expressions Algebra~.

These are currently the following ~Secondo Types~:

  * ~Tuple~
  
  * ~Relation~
  
The following ~Secondo Operators~ are also supported:

  * ~attr~
  
  * ~feed~
  
  * ~filter~
  
  * ~consume~
  
  * ~count~
  
For this purpose, the following classes ~CECGImplRelationType[<]typename[>]~
and ~CECGImplRelationOperator[<]operatorname[>]~, which are subclasses
of ~CECGImplSecondoType~ and ~CECGImplSecondoOperator~, are declared and implemented
in this file.

Furthermore, the implemented ~CECGImplRelationType~ and ~CECGImplRelationOperator~ in
this file are assigned to the class ~CECGImplSecondoAlgebraRelation~, which is a subclass
of ~CECGImplSecondoAlgebra~. Enabling the implementation of this forest is then done in a
second step in the file ~CECodeStore.cpp~ in the root folder from the ~Compiled Expressions Algebra~.
For further information on how to integrate the individual ~ceAlgebraXXX.h~-files, see also the
documentation in the file ~CECodeStore.cpp~.


2 Defines, includes, and constants

*/
#ifndef _COMPILED_EXPRESSIONS_INCLALG_RELATION_H_
#define _COMPILED_EXPRESSIONS_INCLALG_RELATION_H_

//define macro TRACE_ON if trace outputs are needed
//#define TRACE_ON
#undef TRACE_ON

#include <set>

#include "../Relation-C++/RelationAlgebra.h"

using namespace CompiledExpressions;
namespace CompiledExpressions {

/*
3 Class ~CECGImplRelationTypeTUPLE~

The class ~CECGImplRelationTypeTUPLE~, a subclass from the ~CECGImplSecondoType~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Type tuple~.

*/
  class CECGImplRelationTypeTUPLE : public CECGImplSecondoType {
  public:
/*
3.1 The Constructor.

*/
    CECGImplRelationTypeTUPLE() {}

/*
3.2 The Destructor.

*/
    ~CECGImplRelationTypeTUPLE() {}
    
/*
3.3 Function ~getTypeName~

A ~get~-function returned a string with the ~Secondo Typename~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeName() {
      return Tuple::BasicType();
    }
    
/*
3.4 Function ~getTypeClassName~

A ~get~-function returned a string with the classname of the ~Secondo Type~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeClassName() {
      return "Tuple";
    }
    
/*
3.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_REST].insert("../../Relation-C++/RelationAlgebra.h");
    }
  };

/*
4 Class ~CECGImplRelationTypeRELATION~

The class ~CECGImplRelationTypeRELATION~, a subclass from the ~CECGImplSecondoType~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Type rel~.

*/
  class CECGImplRelationTypeRELATION : public CECGImplSecondoType {
  public:
/*
4.1 The Constructor.

*/
    CECGImplRelationTypeRELATION() {}
    
/*
4.2 The Destructor.

*/
    ~CECGImplRelationTypeRELATION() {}
    
/*
4.3 Function ~getTypeName~

A ~get~-function returned a string with the ~Secondo Typename~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeName() {
      return Relation::BasicType();
    }
    
/*
4.4 Function ~getTypeClassName~

A ~get~-function returned a string with the classname of the ~Secondo Type~.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline std::string getTypeClassName() {
      return "GenericRelation";
    }
    
/*
4.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoType~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_REST].insert("../../Relation-C++/RelationAlgebra.h");
    }
  };



/*
5 Class ~CECGImplRelationOperatorATTR~

The class ~CECGImplRelationOperatorATTR~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator attr~.

*/
  class CECGImplRelationOperatorATTR : public CECGImplSecondoOperator {
  public:
/*
5.1 The Constructor.

*/
    CECGImplRelationOperatorATTR() {}
    
/*
5.2 The Destructor.

*/
    ~CECGImplRelationOperatorATTR() {}
    
/*
5.3 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~, which means that the operator creates a new object
in his ~ResultStorage~ and returned a pointer of this object as result of the ~eval~-funktion.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isGenerateNewObject() {
      return true;
    }
    
/*
5.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "attr";
    }
    
/*
5.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_REST].insert("../../Relation-C++/RelationAlgebra.h");
    }

/*
5.6 Function ~getCodeOperatorEvalRequestBody~

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
        code.append("* localArg_2 = arg_2->eval();\n");
        code.append("int index = localArg_2->GetIntval();\n");
        code.append("assert( 1 <= index");
        code.append("&& index <= localArg_0->GetNoAttributes() );\n");
        code.append("resultStorage->CopyFrom((");
        code.append(signature[0]->getTypeClassName());
        code.append("*) localArg_0->GetAttribute(index - 1));\n");
      }
      return code;
    }
  };


/*
6 Class ~CECGImplRelationOperatorFEED~

The class ~CECGImplRelationOperatorFEED~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator feed~.

*/
  class CECGImplRelationOperatorFEED : public CECGImplSecondoOperator {
  public:
/*
6.1 The Constructor.

*/
    CECGImplRelationOperatorFEED() {}
    
/*
6.2 The Destructor.

*/
    ~CECGImplRelationOperatorFEED() {}
    
/*
6.3 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "feed";
    }
    
/*
6.4 Function ~isStreamOperator~

A ~get~-function returned ~TRUE~, which means that the 
corresponding ~Secondo Operator~ is a stream operator.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isStreamOperator() {
      return true;
    }
    
/*
6.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_REST].insert("../../Relation-C++/RelationAlgebra.h");
    }

/*
6.6 Function ~getCodeOperatorAdditionalVariablesDefinition~

A ~get~-function returned a string with the code to defintion additional variables.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorAdditionalVariablesDefinition() {
      std::string code = "";
      code.append("GenericRelation* rel;\n");
      code.append("GenericRelationIterator* itRel;\n");
      return code;
    }

/*
6.7 Function ~getCodeOperatorAdditionalVariablesInitialisation~

A ~get~-function returned a string with the code to initialisation additional variables.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorAdditionalVariablesInitialisation() {
      std::string code = "";
      code.append("rel = 0;\n");
      code.append("itRel = 0;\n");
      return code;
    }

/*
6.8 Function ~getCodeOperatorEvalOpenBody~

A ~get~-function returned a string with the code from the body of the ~eval open~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalOpenBody() {
      std::string code = "";
      code.append("rel = (GenericRelation*)arg_0->eval();\n");
      code.append("itRel = rel->MakeScan();\n");
      code.append("return true;\n");
      return code;
    }

/*
6.9 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
  inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 2) //Insert the number of parameter.
        throw CECRuntimeError("Signature must contain 2 elements.");
      else {
        //Here insert code from operator.
        code.append("resultStorage = itRel->GetNextTuple();\n");
      }
      return code;
    }
    
/*
6.10 Function ~getCodeOperatorEvalCloseBody~

A ~get~-function returned a string with the code from the body of the ~eval close~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalCloseBody() {
      std::string code = "";
      code.append("if (itRel)\n");
      code.append("delete itRel;\n");
      code.append("itRel = 0;\n");
      code.append("rel = 0;\n");
      return code;
    }
  };
  
/*
7 Class ~CECGImplRelationOperatorFILTER~

The class ~CECGImplRelationOperatorFILTER~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator filter~.

*/
  class CECGImplRelationOperatorFILTER : public CECGImplSecondoOperator {
  public:
/*
7.1 The Constructor.

*/
    CECGImplRelationOperatorFILTER() {}
    
/*
7.2 The Destructor.

*/
    ~CECGImplRelationOperatorFILTER() {}
    
/*
7.3 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "filter";
    }
    
/*
7.4 Function ~isStreamOperator~

A ~get~-function returned ~TRUE~, which means that the 
corresponding ~Secondo Operator~ is a stream operator.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isStreamOperator() {
      return true;
    }
    
/*
7.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_REST].insert("../../Relation-C++/RelationAlgebra.h");
    }

/*
7.6 Function ~getCodeOperatorAdditionalVariablesDefinition~

A ~get~-function returned a string with the code to defintion additional variables.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorAdditionalVariablesDefinition() {
      std::string code = "";
      code.append("Tuple* tup;\n");
      return code;
    }

/*
7.7 Function ~getCodeOperatorAdditionalVariablesInitialisation~

A ~get~-function returned a string with the code to initialisation additional variables.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorAdditionalVariablesInitialisation() {
      std::string code = "";
      code.append("tup = 0;\n");
      return code;
    }

/*
7.8 Function ~getCodeOperatorEvalOpenBody~

A ~get~-function returned a string with the code from the body of the ~eval open~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalOpenBody() {
      std::string code = "";
      code.append("return true;\n");
      return code;
    }

/*
7.9 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
  inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 2) //Insert the number of parameter.
        throw CECRuntimeError("Signature must contain 2 elements.");
      else {
        //Here insert code from operator.
        code.append("bool found = false;\n");
        code.append("while (!found && (tup = arg_0->eval()) != 0) {\n");
        code.append("arg_1->setMapType_1(tup);\n");
        code.append("CcBool* argResult = arg_1->eval();\n");
        code.append("if (argResult->IsDefined())\n");
        code.append("found = argResult->GetBoolval();\n");
        code.append("if (!found)\n");
        code.append("tup->DeleteIfAllowed();\n");
        code.append("}\n");
        code.append("if (found)\n");
        code.append("resultStorage = tup;\n");
        code.append("else\n");
        code.append("resultStorage = 0;\n");
      }
      return code;
    }
    
/*
7.10 Function ~getCodeOperatorEvalCloseBody~

A ~get~-function returned a string with the code from the body of the ~eval close~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalCloseBody() {
      std::string code = "";
      code.append("tup = 0;\n");
      return code;
    }
  };
  


/*
8 Class ~CECGImplRelationOperatorCONSUME~

The class ~CECGImplRelationOperatorCONSUME~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator consume~.

*/
  class CECGImplRelationOperatorCONSUME : public CECGImplSecondoOperator {
  public:
/*
8.1 The Constructor.

*/
    CECGImplRelationOperatorCONSUME() {}
    
/*
8.2 The Destructor.

*/
    ~CECGImplRelationOperatorCONSUME() {}
    
/*
8.3 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "consume";
    }
    
/*
8.4 Function ~isStreamOperator~

A ~get~-function returned ~TRUE~, which means that the 
corresponding ~Secondo Operator~ is a stream operator.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isStreamOperator() {
      return true;
    }
    
/*
8.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_REST].insert("../../Relation-C++/RelationAlgebra.h");
    }

/*
8.6 Function ~getCodeOperatorAdditionalVariablesDefinition~

A ~get~-function returned a string with the code to defintion additional variables.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorAdditionalVariablesDefinition() {
      std::string code = "";
      code.append("GenericRelation* rel;\n");
      code.append("Tuple* tup;\n");
      return code;
    }

/*
8.7 Function ~getCodeOperatorAdditionalVariablesInitialisation~

A ~get~-function returned a string with the code to initialisation additional variables.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorAdditionalVariablesInitialisation() {
      std::string code = "";
      code.append("rel = 0;\n");
      code.append("tup = 0;\n");
      return code;
    }

/*
8.8 Function ~getCodeOperatorEvalOpenBody~

A ~get~-function returned a string with the code from the body of the ~eval open~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalOpenBody() {
      std::string code = "";
      code.append("return true;\n");
      return code;
    }

/*
8.9 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
  inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 2) //Insert the number of parameter.
        throw CECRuntimeError("Signature must contain 2 elements.");
      else {
        //Here insert code from operator.
        code.append("rel = (GenericRelation*)");
        code.append("(CodeCompiledExpressions::getInstance()");
        code.append("->getPtrCEQP()->qpResultStorage(sup)).addr;\n");

        code.append("if (rel->GetNoTuples() > 0)\n");
        code.append("rel->Clear();\n");
        
        code.append("while ((tup = arg_0->eval()) != 0) {\n");
        code.append("rel->AppendTuple(tup);\n");
        code.append("tup->DeleteIfAllowed();\n");
        code.append("}\n");
        code.append("resultStorage = rel;\n");
      }
      return code;
    }
    
/*
8.10 Function ~getCodeOperatorEvalCloseBody~

A ~get~-function returned a string with the code from the body of the ~eval close~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalCloseBody() {
      std::string code = "";
      code.append("tup = 0;\n");
      code.append("rel = 0;\n");
      return code;
    }
  };
  


/*
9 Class ~CECGImplRelationOperatorCOUNT~

The class ~CECGImplRelationOperatorCOUNT~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator count~.

*/
  class CECGImplRelationOperatorCOUNT : public CECGImplSecondoOperator {
  public:
/*
9.1 The Constructor.

*/
    CECGImplRelationOperatorCOUNT() {}
    
/*
9.2 The Destructor.

*/
    ~CECGImplRelationOperatorCOUNT() {}
    
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
      return "count";
    }
    
/*
9.5 Function ~isStreamOperator~

A ~get~-function returned ~TRUE~, which means that the 
corresponding ~Secondo Operator~ is a stream operator.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isStreamOperator() {
      return true;
    }
    
/*
9.6 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_REST].insert("../../Relation-C++/RelationAlgebra.h");
    }

/*
9.7 Function ~getCodeOperatorAdditionalVariablesDefinition~

A ~get~-function returned a string with the code to defintion additional variables.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorAdditionalVariablesDefinition() {
      std::string code = "";
      code.append("Tuple* tup;\n");
      code.append("int count;\n");
      return code;
    }

/*
9.8 Function ~getCodeOperatorAdditionalVariablesInitialisation~

A ~get~-function returned a string with the code to initialisation additional variables.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorAdditionalVariablesInitialisation() {
      std::string code = "";
      code.append("tup = 0;\n");
      code.append("count = 0;\n");
      return code;
    }

/*
9.9 Function ~getCodeOperatorEvalOpenBody~

A ~get~-function returned a string with the code from the body of the ~eval open~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalOpenBody() {
      std::string code = "";
      code.append("return true;\n");
      return code;
    }

/*
9.10 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
  inline std::string getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 2) //Insert the number of parameter.
        throw CECRuntimeError("Signature must contain 2 elements.");
      else {
        //Here insert code from operator.
        code.append("while ((tup = arg_0->eval()) != 0) {\n");
        code.append("count++;\n");
        code.append("tup->DeleteIfAllowed();\n");
        code.append("}\n");
        code.append("resultStorage->Set(true, count);\n");
      }
      return code;
    }
    
/*
9.11 Function ~getCodeOperatorEvalCloseBody~

A ~get~-function returned a string with the code from the body of the ~eval close~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalCloseBody() {
      std::string code = "";
      code.append("tup = 0;\n");
      code.append("count = 0;\n");
      return code;
    }
  };
  

/*
10 Class ~CECGImplSecondoAlgebraRelation~

The class ~CECGImplSecondoAlgebraRelation~, a subclass from the ~CECGImplSecondoAlgebra~,
implements a data structure to save all implemented ~Types~ and ~Operators~ into this file.

*/
  class CECGImplSecondoAlgebra_Relation : public CECGImplSecondoAlgebra {
  public:
/*
10.1 The Constructor.

*/
    CECGImplSecondoAlgebra_Relation()
    : CECGImplSecondoAlgebra(std::string("RelationAlgebra")) {}

/*
10.2 The Destructor.

*/
    virtual ~CECGImplSecondoAlgebra_Relation() {}
    
/*
10.3 Function ~loadCECGImplSecondoTypes~

This function loads the implemented types into the code store of the ~CECompiler~.
In this function created for all ~CECGImplRelationType~-classes an object and
call the ~loadCECGImplSecondoType~-function from ~CECodeGenerator~ to load
this ~CECGImplRelationType~-object. The call has the following syntax:

----

ceCG->loadCECGImplSecondoType(new CECGImplRelationTypeXXX());

----


This function overrides the function of the base class ~CECGImplSecondoAlgebra~.

*/
    void loadCECGImplSecondoTypes(CECodeGenerator* ceCG) {
      ceCG->loadCECGImplSecondoType(new CECGImplRelationTypeTUPLE());
      ceCG->loadCECGImplSecondoType(new CECGImplRelationTypeRELATION());
      
    }
    
/*
10.4 Function ~loadCECGImplSecondoOperators~

This function loads the implemented operators into the code store of the ~CECompiler~.
In this function created for all ~CECGImplRelationOperator~-classes an object and
call the ~loadCECGImplSecondoOperator~-function from ~CECodeGenerator~ to load
this ~CECGImplRelationOperator~-object. The call has the following syntax:

----

ceCG->loadCECGImplSecondoOperator
        (algName, new CECGImplRelationOperatorXXX());

----


This function overrides the function of the base class ~CECGImplSecondoAlgebra~.

*/
    void loadCECGImplSecondoOperators(CECodeGenerator* ceCG) {
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplRelationOperatorATTR());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplRelationOperatorFEED());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplRelationOperatorFILTER());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplRelationOperatorCONSUME());
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplRelationOperatorCOUNT());

    }
    
  private:
  };
  
  
} // end of namespace CompiledExpressions
#endif // _COMPILED_EXPRESSIONS_INCLALG_RELATION_H_