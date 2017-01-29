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
from the ~Secondo ExtRelation-C++ Algebra~

2016/2017 H.Brieschke created the new CompiledExpression Algebra


[TOC]


1 Overview 

In this file are declares and implemented all ~Secondo Types~ and ~Secondo Operators~
from the ~Secondo Algebra ExtRelation-C++~, which are currently supported in the functionality
of code generation by the ~Compiled Expressions Algebra~.

These are currently the following ~Secondo Types~:

  * ~none~
  
The following ~Secondo Operators~ are also supported:

  * ~extend~
  
For this purpose, the following classes ~CECGImplExtRelationType[<]typename[>]~
and ~CECGImplExtRelationOperator[<]operatorname[>]~, which are subclasses
of ~CECGImplSecondoType~ and ~CECGImplSecondoOperator~, are declared and implemented
in this file.

Furthermore, the implemented ~CECGImplExtRelationType~ and ~CECGImplExtRelationOperator~ in
this file are assigned to the class ~CECGImplSecondoAlgebraExtRelation~, which is a subclass
of ~CECGImplSecondoAlgebra~. Enabling the implementation of this forest is then done in a
second step in the file ~CECodeStore.cpp~ in the root folder from the ~Compiled Expressions Algebra~.
For further information on how to integrate the individual ~ceAlgebraXXX.h~-files, see also the
documentation in the file ~CECodeStore.cpp~.


2 Defines, includes, and constants

*/
#ifndef _COMPILED_EXPRESSIONS_INCLALG_EXTRELATION_H_
#define _COMPILED_EXPRESSIONS_INCLALG_EXTRELATION_H_

//define macro TRACE_ON if trace outputs are needed
//#define TRACE_ON
#undef TRACE_ON

#include <set>

//using namespace CompiledExpressions;
namespace CompiledExpressions {

/*
3 Class ~CECGImplExtRelationOperatorEXTEND~

The class ~CECGImplExtRelationOperatorEXTEND~, a subclass from the ~CECGImplSecondoOperator~,
implements all the functions required for code generation within the ~Compiled Expressions Algebra~
for the ~Secondo Operator extend~.

*/
  class CECGImplExtRelationOperatorEXTEND : public CECGImplSecondoOperator {
  public:
/*
3.1 The Constructor.

*/
    CECGImplExtRelationOperatorEXTEND() {}
    
/*
3.2 The Destructor.

*/
    ~CECGImplExtRelationOperatorEXTEND() {}
    
/*
3.3 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getOperatorName() {
      return "extend";
    }
    
/*
3.4 Function ~isStreamOperator~

A ~get~-function returned ~TRUE~, which means that the 
corresponding ~Secondo Operator~ is a stream operator.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline bool isStreamOperator() {
      return true;
    }
    
/*
3.5 Function ~getHeaderFilesEntry~

This function writes all the names of the headerfiles required for the type class
into the ~Set Array hf~ so that they can be included in the c++-code to be generated.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {
      hf[HF_IDX_REST].insert("../../Relation-C++/RelationAlgebra.h");
    }

/*
3.6 Function ~getCodeOperatorAdditionalVariablesDefinition~

A ~get~-function returned a string with the code to defintion additional variables.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorAdditionalVariablesDefinition() {
      std::string code = "";
      code.append("Tuple* tup;\n");
      code.append("TupleType* resultTupleType;\n");
      return code;
    }

/*
3.7 Function ~getCodeOperatorAdditionalVariablesInitialisation~

A ~get~-function returned a string with the code to initialisation additional variables.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorAdditionalVariablesInitialisation() {
      std::string code = "";
      code.append("tup = 0;\n");
      code.append("resultTupleType = 0;\n");
      return code;
    }

/*
3.8 Function ~getCodeOperatorEvalOpenBody~

A ~get~-function returned a string with the code from the body of the ~eval open~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalOpenBody() {
      std::string code = "";
      code.append("resultTupleType = new TupleType( nl->Second(");
      code.append("CodeCompiledExpressions::getInstance()");
      code.append("->getPtrCEQP()->GetTupleResultType(ceNodeKey,");
      code.append("CodeCompiledExpressions::getInstance()");
      code.append("->getPtrCEQY())));\n");
      
      code.append("if (resultTupleType)\n");
      code.append("return true;\n");
      code.append("else\n");
      code.append("return false;\n");
      return code;
    }

/*
3.9 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
  inline std::string getCodeOperatorEvalRequestBody(
    std::vector<CECGImplSecondoType*>(&signature))
    throw (CECRuntimeError) {
      std::string code = "";
      if (signature.size() != 2) //Insert the number of parameter.
        throw CECRuntimeError("Signature must contain 2 elements.");
      else {
        //Here insert code from operator.
        code.append("resultStorage = 0;\n");
        code.append("int nooffun = arg_1->getNumOfSons();\n");
        code.append("if ((tup = arg_0->eval()) != 0) {\n");
        code.append("Tuple *newTuple = new Tuple(resultTupleType);\n");
        code.append("for(int i = 0; i < tup->GetNoAttributes(); i++ ) {\n");
        code.append("newTuple->CopyAttribute( i, tup, i );\n");
        code.append("}\n");
        code.append("for (int i=0; i < nooffun; i++) {\n");
        
        code.append("(dynamic_cast<CECNodeAbstraction*>");
        code.append("(arg_1->getSon(i)->getSon(1)))");
        code.append("->setMapType(SetWord(tup), 1);\n");
        
        code.append("newTuple->PutAttribute( tup->GetNoAttributes()+i,\n");
        
        code.append("((Attribute*)");
        code.append("arg_1->getSon(i)->getSon(1)->evalDefault().addr)");
        code.append("->Clone() );\n");
        
        code.append("}\n");
        code.append("tup->DeleteIfAllowed();\n");
        code.append("resultStorage = newTuple;\n");
        code.append("}\n");
      }
      return code;
    }
    
/*
3.10 Function ~getCodeOperatorEvalCloseBody~

A ~get~-function returned a string with the code from the body of the ~eval close~-function.

This function overrides the function of the base class ~CECGImplSecondoOperator~.

*/
    inline std::string getCodeOperatorEvalCloseBody() {
      std::string code = "";
      code.append("tup = 0;\n");
      code.append("resultTupleType->DeleteIfAllowed();\n");
      code.append("resultTupleType = 0;\n");
      return code;
    }
  };
  



/*
4 Class ~CECGImplSecondoAlgebraExtRelation~

The class ~CECGImplSecondoAlgebraExtRelation~, a subclass from the ~CECGImplSecondoAlgebra~,
implements a data structure to save all implemented ~Types~ and ~Operators~ into this file.

*/
  class CECGImplSecondoAlgebra_ExtRelation : public CECGImplSecondoAlgebra {
  public:
/*
4.1 The Constructor.

*/
    CECGImplSecondoAlgebra_ExtRelation()
    : CECGImplSecondoAlgebra(std::string("ExtRelationAlgebra")) {}

/*
4.2 The Destructor.

*/
    virtual ~CECGImplSecondoAlgebra_ExtRelation() {}
    
/*
4.3 Function ~loadCECGImplSecondoTypes~

This function loads the implemented types into the code store of the ~CECompiler~.
In this function created for all ~CECGImplExtRelationType~-classes an object and
call the ~loadCECGImplSecondoType~-function from ~CECodeGenerator~ to load
this ~CECGImplExtRelationType~-object. The call has the following syntax:

----

ceCG->loadCECGImplSecondoType(new CECGImplExtRelationTypeXXX());

----

This function overrides the function of the base class ~CECGImplSecondoAlgebra~.

*/
    void loadCECGImplSecondoTypes(CECodeGenerator* ceCG) {
      
    }
    
/*
4.4 Function ~loadCECGImplSecondoOperators~

This function loads the implemented operators into the code store of the ~CECompiler~.

In this function created for all ~CECGImplExtRelationOperator~-classes an object and
call the ~loadCECGImplSecondoOperator~-function from ~CECodeGenerator~ to load
this ~CECGImplExtRelationOperator~-object. The call has the following syntax:

----

ceCG->loadCECGImplSecondoOperator
        (algName, new CECGImplExtRelationOperatorXXX());

----

This function overrides the function of the base class ~CECGImplSecondoAlgebra~.

*/
    void loadCECGImplSecondoOperators(CECodeGenerator* ceCG) {
      ceCG->loadCECGImplSecondoOperator
              (algName, new CECGImplExtRelationOperatorEXTEND());

    }
    
  private:
  };
  
  
} // end of namespace CompiledExpressions
#endif // _COMPILED_EXPRESSIONS_INCLALG_EXTRELATION_H_