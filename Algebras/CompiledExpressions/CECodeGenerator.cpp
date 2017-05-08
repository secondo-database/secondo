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

[10] Implementation file of the CECodeGenerator

2016/2017 H.Brieschke created the new CompiledExpression Algebra


[TOC]


1 Overview 

In this file the classes needed for the ~CECodeGenerator~ are implemented. These are

  * ~CECGImplSecondoType~
  
  * ~CECGImplSecondoOperator~
  
  * ~CECGImplSecondoAlgebra~
  
  * ~CECodeGenerator~
  
  * ~CECOpTreeVisitorGenerateCode~

  
The 3 classes ~CECGImplSecondoAlgebra~, ~CECGImplSecondoType~ and ~CECGImplSecondoOperator~ are
the base classes of the ~ceCodeStore~.

The class ~CECodeGenerator~ implements the code generator.

The class ~CECOpTreeVisitorGenerateCode~, a subclass from the ~CECOpTreeVisitor~, searches for the
code parts in the operator tree and provides it to the ~CECodeGenerator~.


2 Defines, includes, and constants

*/
#include <iostream>
#include <sstream>
#include <string>


#include "Symbols.h"
#include "Trace.h"
#include "TypeConstructor.h"
#include "StandardTypes.h"
#include "FileSystem.h"

#include "./CECodeGenerator.h"
#include "./CECodeStore.cpp"

extern AlgebraManager *am;

using namespace CompiledExpressions;
namespace CompiledExpressions {

/*
3 Class ~CECGImplSecondoType~

This class is the base class of all ~ceCodeStore~-classes to implements
the code generation of a ~Secondo Type~.


3.1 The Constructor.

*/
  CECGImplSecondoType::CECGImplSecondoType()
  : algID(0),
    typeID(0),
    isActiv(false) {}

/*
3.2 Function ~initType~

A ~set~-function that initializes the class objects with the current values
of the ~Secondo Algebra Manager~ at runtime.

*/
  void
  CECGImplSecondoType::initType(int algID,
                                int typeID,
                                bool isActiv) {
    (*this).algID = algID;
    (*this).typeID = typeID;
    (*this).isActiv = isActiv;
  }

/*
3.3 Function ~isActivType~

A ~get~-function returned ~TRUE~ or ~FALSE~ if the 
corresponding ~Secondo Type~ activ or not.

*/
  bool
  CECGImplSecondoType::isActivType() {
    return isActiv;
  }
  
/*
3.4 Function ~isCPPType~

A ~get~-function returned ~TRUE~ or ~FALSE~ whether the 
corresponding ~Secondo Type~ has a correspondence as ~C++ Type~.

*/
  bool
  CECGImplSecondoType::isCPPType() {
    return false;
  }
  
/*
3.5 Function ~getTypeName~

A ~get~-function returned a string with the ~Secondo Type Name~.

*/
  std::string
  CECGImplSecondoType::getTypeName() {
    return "";
  }
  
/*
3.6 Function ~getTypeClassName~

A ~get~-function returned a string with the ~Secondo Type Class Name~.

*/
  std::string
  CECGImplSecondoType::getTypeClassName() {
    return "";
  }
  
/*
3.7 Function ~getCPPTypeName~

A ~get~-function returned a string with the ~C++ Type Name~.

*/
  std::string
  CECGImplSecondoType::getCPPTypeName() {
    return "";
  }
  
/*
4 Class ~CECGImplSecondoOperator~

This class is the base class of all ~ceCodeStore~-classes to implements
the code generation of a ~Secondo Operator~.


4.1 The Constructor.

*/
  CECGImplSecondoOperator::CECGImplSecondoOperator() 
  : algID(0),
    opID(0),
    isActiv(false) {}
    
/*
4.2 Function ~initOperator~

A ~set~-function that initializes the class objects with the current values
of the ~Secondo Algebra Manager~ at runtime.

*/
  void
  CECGImplSecondoOperator::initOperator(int algID,
                                        int opID,
                                        bool isActiv) {
    (*this).algID = algID;
    (*this).opID = opID;
    (*this).isActiv = isActiv;
  }

/*
4.3 Function ~isActivOperator~

A ~get~-function returned ~TRUE~ or ~FALSE~ if the 
corresponding ~Secondo Operator~ activ or not.

*/
  bool
  CECGImplSecondoOperator::isActivOperator() {
    return isActiv;
  }
  
/*
4.4 Function ~getOperatorName~

A ~get~-function returned a string with the ~Secondo Operator Name~.

*/
  std::string
  CECGImplSecondoOperator::getOperatorName() {
    return "";
  }
  
/*
4.5 Function ~isGenerateNewObject~

A ~get~-function returned ~TRUE~ or ~FALSE~ if the 
corresponding ~Secondo Operator~ in his ~ResultStorage~ created new object or not.

*/
  bool
  CECGImplSecondoOperator::isGenerateNewObject() {
    return false;
  }
  
/*
4.6 Function ~isStreamOperator~

A ~get~-function returned ~TRUE~ or ~FALSE~ if the 
corresponding ~Secondo Operator~ a stream operator or not.

*/
  bool
  CECGImplSecondoOperator::isStreamOperator() {
    return false;
  }

/*
4.7 Function ~getCodeOperatorAdditionalVariablesDefinition~

A ~get~-function returned a string with the code to defintion additional variables.

*/
  std::string
  CECGImplSecondoOperator::getCodeOperatorAdditionalVariablesDefinition() {
    return "";
  }

/*
4.8 Function ~getCodeOperatorAdditionalVariablesInitialisation~

A ~get~-function returned a string with the code to initialisation additional variables.

*/
  std::string
  CECGImplSecondoOperator::getCodeOperatorAdditionalVariablesInitialisation() {
    return "";
  }

/*
4.9 Function ~getCodeOperatorEvalOpenBody~

A ~get~-function returned a string with the code from the body of the ~eval open~-function.

*/
  std::string
  CECGImplSecondoOperator::getCodeOperatorEvalOpenBody() {
    return "";
  }

/*
4.10 Function ~getCodeOperatorEvalRequestBody~

A ~get~-function returned a string with the code from the body of the ~eval request~-function.

*/
  std::string
  CECGImplSecondoOperator::getCodeOperatorEvalRequestBody
    (std::vector<CECGImplSecondoType*>(&signature)) throw (CECRuntimeError) {
    return "";
  }
    
/*
4.11 Function ~getCodeOperatorEvalCloseBody~

A ~get~-function returned a string with the code from the body of the ~eval close~-function.

*/
  std::string
  CECGImplSecondoOperator::getCodeOperatorEvalCloseBody() {
    return "";
  }


/*
5 Class ~CECodeGenerator~

The class ~CECodeGenerator~ implements the code generator. On one hand, it manages
the ~code store~ and, on the other hand, uses the ~CECOpTreeVisitorGenerateCode~-visitor
to create the source code files to be generated.

This class is implemented as a ~Singleton~.

5.1 Initialisation of the static class variable

*/
  bool CECodeGenerator::canInitialize = false;
  CECodeGenerator* CECodeGenerator::instance = 0;

/*
5.2 The Constructor.

*/
  CECodeGenerator::CECodeGenerator()
  : isLoadCECGImplTypesAndOperators(false),
    isInitCECGImplTypesAndOperators(false) {
    
    loadCECGImplAlgebrasTypesAndOperators();
    initCECGImplTypesAndOperators();
  }
    
/*
5.3 The Destructor.

*/
  CECodeGenerator::~CECodeGenerator() {
    deleteCECGImplAlgebrasTypesAndOperators();
  }
  
/*
5.4 Function ~getInstance~

This static function returned a pointer of the ~CECodeGenerator~-instance. 
If no instance has been created, the function creates an instance.

*/
  CECodeGenerator*
  CECodeGenerator::getInstance() {
    if (!CECodeGenerator::instance)
      CECodeGenerator::instance = new CECodeGenerator();

    return CECodeGenerator::instance;
  }
  
/*
5.5 Function ~deleteInstance~

This static function deleted the ~CECodeGenerator~-instance.

*/
  void
  CECodeGenerator::deleteInstance() {
    delete instance;
    instance = 0;    
  }
  
/*
5.6 Function ~setCanInitialize~

This static function set a flag, which indicates that the ~Secondo System~ has been started 
so far that the ~Code Store~ can be initialized. This point is reached as soon as
the ~Secondo Algebra Manager~ has loaded all active algebras.

*/
  void
  CECodeGenerator::setCanInitialize() {
    CECodeGenerator::canInitialize = true;
  }

/*
5.7 Function ~loadCECGImplSecondoType~

This function insert the given ~CECGImplSecondoType~-object ~implSecondoType~
in a data structure, in which all implemented ~CECGImplSecondoType~ are managed.

*/
  void
  CECodeGenerator::loadCECGImplSecondoType
                     (CECGImplSecondoType* implSecondoType) {
    std::string key = "type_" + implSecondoType->getTypeName();
    mapNameImplTypes[key] = implSecondoType;
  }
  
/*
5.8 Function ~getPtrSecondoType~

A ~get~-function returned a pointer to the ~CECGImplSecondoType~-objects is given
by the name ~name~.

*/
  CECGImplSecondoType*
  CECodeGenerator::getPtrSecondoType(std::string& name) {
    CECGImplSecondoType* ptrReturn = 0;
    std::string key = "type_" + name;

    initCECGImplTypesAndOperators();

    std::map<std::string, CECGImplSecondoType*>::const_iterator itType;
    itType = mapNameImplTypes.find(key);    
    if (itType != mapNameImplTypes.end())
      ptrReturn = itType->second;
    
    return ptrReturn;
  }
  
/*
5.9 Function ~loadCECGImplSecondoOperator~

This function insert the given ~CECGImplSecondoOperator~-object ~implSecondoOperator~
in a data structure, in which all implemented ~CECGImplSecondoOperator~ are managed.

*/
  void
  CECodeGenerator::loadCECGImplSecondoOperator
                     (const std::string& algName,
                      CECGImplSecondoOperator* implSecondoOperator) {
                       
    std::string key = "operator_"
                     + algName
                     + "_"
                     + implSecondoOperator->getOperatorName();
    mapNameImplOperators[key] = implSecondoOperator;
  }
  
/*
5.10 Functions ~getPtrSecondoOperator~

A ~get~-function returned a pointer to the ~CECGImplSecondoOperator~-objects is given
by the ~Algebraname~ and the ~Operatorname~.

*/
  CECGImplSecondoOperator*
  CECodeGenerator::getPtrSecondoOperator(std::string& algName,
                                         std::string& opName) {
    
    CECGImplSecondoOperator* ptrReturn = 0;
    std::string key = "operator_" + algName + "_" + opName;
    
    initCECGImplTypesAndOperators();
    
    std::map<std::string, CECGImplSecondoOperator*>::const_iterator itOperator;
    itOperator = mapNameImplOperators.find(key);    
    if (itOperator != mapNameImplOperators.end())
      ptrReturn = itOperator->second;
    
    return ptrReturn;
  }
  
  
/*
And a ~get~-function returned a pointer to the ~CECGImplSecondoOperator~-objects is given
by the ~Algebranumber~ and the ~Operatornumber~.

*/
  CECGImplSecondoOperator*
  CECodeGenerator::getPtrSecondoOperator(unsigned int algID,
                                         unsigned int opID) {
    
    CECGImplSecondoOperator* ptrReturn = 0;
    std::stringstream ss;
    ss << "operator_" << algID << "_" << opID;
    std::string key(ss.str());
    
    initCECGImplTypesAndOperators();
    
    std::map<std::string, CECGImplSecondoOperator*>::const_iterator itOperator;
    itOperator = mapIdImplOperators.find(key);    
    if (itOperator != mapIdImplOperators.end())
      ptrReturn = itOperator->second;
    
    return ptrReturn;
  }
  
/*
5.11 Function ~loadCECGImplAlgebrasTypesAndOperators~

This function loaded all implemented algebras, types, and operators from the ~Code Store~.

*/
  void
  CECodeGenerator::loadCECGImplAlgebrasTypesAndOperators() {
    if (isLoadCECGImplTypesAndOperators || isInitCECGImplTypesAndOperators)
      deleteCECGImplAlgebrasTypesAndOperators();
    
    loadCECGImplAlgebras();
    
    std::set<CECGImplSecondoAlgebra*>::iterator pos;
    for ( pos = classObjAlgebras.begin();
          pos != classObjAlgebras.end();
          ++pos ) {
      (*pos)->loadCECGImplSecondoTypes(this);
      (*pos)->loadCECGImplSecondoOperators(this);
    }
    
    isLoadCECGImplTypesAndOperators = true;
  }
  
/*
5.12 Function ~initCECGImplTypesAndOperators~

This function initialized all implemented types and operators from the ~Code Store~.

*/
  void
  CECodeGenerator::initCECGImplTypesAndOperators() {
    if (CECodeGenerator::canInitialize && !isInitCECGImplTypesAndOperators) {
      if (!isLoadCECGImplTypesAndOperators)
        loadCECGImplAlgebrasTypesAndOperators();
      
      int numAlgebra, numType, numOperator;
      std::string keyTypeName, keyOpName, keyIdName;
      
      numAlgebra = am->getMaxAlgebraId();
      for (int i = 0; i <= numAlgebra; i++) {
        if (am->IsAlgebraLoaded(i)) {
          numType = am->ConstrNumber(i);
          for (int j = 0; j < numType; j++) {
            keyTypeName = "type_" + am->GetTC(i, j)->Name();
            std::map<std::string, CECGImplSecondoType*>::const_iterator it;
            it = mapNameImplTypes.find(keyTypeName);
            if (it != mapNameImplTypes.end())
              it->second->initType(i, j, true);
          }
          
          numOperator = am->OperatorNumber(i);
          for (int j = 0; j < numOperator; j++) {
            keyOpName = "operator_"
                        + am->GetAlgebraName(i)
                        + "_"
                        + am->GetOP(i, j)->GetName();
            std::map<std::string, CECGImplSecondoOperator*>::const_iterator it;
            it = mapNameImplOperators.find(keyOpName);
            if (it != mapNameImplOperators.end()) {
              it->second->initOperator(i, j, true);
              std::stringstream ss;
              ss << "operator_" << i << "_" << j;
              std::string keyIdName(ss.str());
              mapIdImplOperators[keyIdName] = it->second;
            }
          }
        }
      }
      isInitCECGImplTypesAndOperators = true;
    }
    
  }
  
  
/*
5.13 Function ~deleteCECGImplAlgebrasTypesAndOperators~

This function deleted all ~Algebras~-, ~Types~- and ~Operator~-objects are loaded from
the ~Code Store~.

*/
  void
  CECodeGenerator::deleteCECGImplAlgebrasTypesAndOperators() {
    std::set<CECGImplSecondoAlgebra*>::iterator itAlgebra;
    for ( itAlgebra = classObjAlgebras.begin();
          itAlgebra != classObjAlgebras.end();
          ++itAlgebra )
         delete (*itAlgebra);
    
    classObjAlgebras.clear();
      
    std::map<std::string, CECGImplSecondoType*>::iterator itType;
    for (itType = mapNameImplTypes.begin();
         itType != mapNameImplTypes.end();
         ++itType)
      delete itType->second;
    
    mapNameImplTypes.clear();

    std::map<std::string, CECGImplSecondoOperator*>::iterator itOperator;
    for (itOperator = mapNameImplOperators.begin();
         itOperator != mapNameImplOperators.end();
         ++itOperator)
      delete itOperator->second;  
    
    mapNameImplOperators.clear();    
    mapIdImplOperators.clear();
    
    isLoadCECGImplTypesAndOperators = false;
    isInitCECGImplTypesAndOperators = false;
  }
  
  
/*
5.14 Function ~generateCECode~

This function generate the ~c++-code~ of the query.

*/
  bool
  CECodeGenerator::generateCECode(CECOpNode* ptrRootNode,
                                  CEQuery* ptrCEQY) {
    
    try {
      ptrRootNode->accept(ptrCEQY->getCodeGenVisitor());
      return true;
    } catch(...) {
      return false;
    }
  }

/*
5.15 Function ~getStreamGeneratedCECode~

This function write the ~c++-code~ of the query in the stream ~outPut~.

*/
  void
  CECodeGenerator::getStreamGeneratedCECode(std::ostream& outPut,
                                            CEQuery* ptrCEQY) {
    
    outPut << (ptrCEQY->getCodeGenVisitor()).getGenCodeIncludes()
    << "using namespace CompiledExpressions;" << endl 
    << "namespace CompiledExpressions {" << endl << endl
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeForwardDeclarations() << endl
    
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeClassCECNodeDefault() << endl
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeClassCECNodeApplyop()
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeClassCECNodeAbstraction()
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeClassCECNodeObject()
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeClassCECNodeConstant()
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeClassCECNodeVariable()
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeClassCECNodeIdentifier()
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeClassCECNodeArglist()
    
    << (ptrCEQY->getCodeGenVisitor())
                   .getGenCodeCodeCompiledExpressionsDeclaration() << endl
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeClassDefinitions() << endl
    << (ptrCEQY->getCodeGenVisitor())
                   .getGenCodeCodeCompiledExpressionsDefinition() << endl
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeClassConstructor() << endl
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeClassDestructor() << endl
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeFunctionEval() << endl
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeFunctionInit() << endl
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeFunctionOperator() << endl
    << (ptrCEQY->getCodeGenVisitor())
                   .getGenCodeDeclarationAdditionalFunction() << endl
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeEvalDefaultFunction() << endl
    << "}" << endl << endl
    << (ptrCEQY->getCodeGenVisitor()).getGenCodeLibCallFunction() << endl;
    
  }
  
/*
5.16 Function ~getStreamMake~

This function generate the code from a ~makefile~ and write this in the stream ~outPut~.

*/
  void
  CECodeGenerator::getStreamMake(std::ostream& outPut,
                                 std::string libName) {
    outPut << "include ../../../makefile.env" << endl
          << "CCFLAGS += $(ALG_INC_DIRS)" << endl << endl
          << ".PHONY: all" << endl
          << "all: " << libName << ".so" << endl << endl
          << "%.so: %.cpp" << endl
          << "\tg++ $(CCFLAGS) --shared -o $@ $<" << endl;
  }
  
  
/*
6 Class ~CECOpTreeVisitorGenerateCode~

The class ~CECOpTreeVisitorGenerateCode~, a subclass from the ~CECOpTreeVisitor~, searches for the
code parts in the operator tree and provides it to the ~CECodeGenerator~.

6.1 The Constructor.

*/
  CECOpTreeVisitorGenerateCode::CECOpTreeVisitorGenerateCode()
  : cecOpNodeApplyopIsInsert(false),
    cecOpNodeAbstractionIsInsert(false),
    cecOpNodeObjectIsInsert(false),
    cecOpNodeConstantIsInsert(false),
    cecOpNodeVariableIsInsert(false),
    cecOpNodeIdentifierIsInsert(false),
    cecOpNodeArglistIsInsert(false) {
    createCELibDefaultFunc();
  }
  
/*
6.2 The Destructor.

*/
  CECOpTreeVisitorGenerateCode::~CECOpTreeVisitorGenerateCode() {
    for (int i = 0; i < HF_SIZEOF; i++)
      headerfiles[i].clear();
    genCodeNodes.clear();
    operators_libcall_function.clear();
  }
    
/*
6.3 Function ~addVarNamesARG-idx~

This function completes the names of the parameters passed to a function for all necessary functions
of the source code to be generated, so that unique variable names are generated within a function.

*/
  void
  CECOpTreeVisitorGenerateCode::addVarNamesARG_idx() {
    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itMap;
             
    for(itMap = genCodeNodes.begin(); itMap != genCodeNodes.end(); ++itMap) {
      if (itMap->second[CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD]
                       .size() > 0) {
        int idx = 0;
        std::vector<std::string>::iterator itVec;
        for (itVec = itMap->second
                     [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD]
                     .begin() + 1;
             itVec != itMap->second
                     [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD]
                     .end();
             ++itVec) {
          if ((*itVec).find("arg_") != std::string::npos) {
            std::stringstream ss;
            ss << idx++;
            (*itVec).append(ss.str());
          }
        }
      }

      if (itMap->second[CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD]
                       .size() > 0) {
        int idx = 0;
        std::vector<std::string>::iterator itVec;
        for (itVec = itMap->second
                     [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD]
                     .begin() + 1;
             itVec != itMap->second
                      [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD]
                      .end();
             ++itVec) {
          if ((*itVec).find("arg_") != std::string::npos) {
            std::stringstream ss;
            ss << idx++;
            (*itVec).append(ss.str());
          }
        }
      }
      
    }
  }
  
/*
6.4 Function ~getGenCodeIncludes~

A ~get~-function returned a string with the generated ~c++-code~ of
the ~include headerfile section~.

*/
  std::string
  CECOpTreeVisitorGenerateCode::getGenCodeIncludes() {
    std::string code = "";
    std::set<std::string>::iterator pos;
    for(pos = headerfiles[HF_IDX_CPP].begin();
        pos != headerfiles[HF_IDX_CPP].end();
        ++pos) {
      code.append("#include <");
      code.append(*pos);
      code.append(">\n");
    }
    
    if (headerfiles[HF_IDX_CPP].size() > 0)
      code.append("\n");
    for(pos = headerfiles[HF_IDX_SECONDO].begin();
        pos != headerfiles[HF_IDX_SECONDO].end();
        ++pos) {
      code.append("#include \"");
      code.append(*pos);
      code.append("\"\n");
    }
    
    if (headerfiles[HF_IDX_SECONDO].size() > 0)
      code.append("\n");
    for(pos = headerfiles[HF_IDX_REST].begin();
        pos != headerfiles[HF_IDX_REST].end();
        ++pos) {
      code.append("#include \"");
      code.append(*pos);
      code.append("\"\n");
    }
    
    if (headerfiles[HF_IDX_REST].size() > 0)
      code.append("\n");
    code.append("\n");
    return code;
  }
  
/*
6.5 Function ~getGenCodeForwardDeclarations~

A ~get~-function returned a string with the generated ~c++-code~ of
the ~forward class declaration section~.

*/
  std::string 
  CECOpTreeVisitorGenerateCode::getGenCodeForwardDeclarations() {
    std::string code = "";
    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itPos;
    for(itPos = genCodeNodes.begin();
        itPos != genCodeNodes.end();
        ++itPos) {
      code.append("class ");
      code.append(itPos->first);
      code.append(";\n");
    }
    code.append("\n");
    return code;
  }
  
/*
6.6 Function ~getGenCodeClassDefinitions~

A ~get~-function returned a string with the generated ~c++-code~ of
the ~class definition section~.

*/
  std::string 
  CECOpTreeVisitorGenerateCode::getGenCodeClassDefinitions() {
    std::string code = "";
    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itPos;
    for(itPos = genCodeNodes.begin();
        itPos != genCodeNodes.end();
        ++itPos) {
      code.append("class ");
      code.append(itPos->first);
      code.append(" : public ");
      code.append(itPos->second[CE_VCG_SUPERCLASS].at(0));
      code.append(" {\n");
      code.append("public:\n");
      code.append(itPos->first);
      code.append("();\n");
      code.append("virtual ~");
      code.append(itPos->first);
      code.append("();\n");
      code.append("bool Init();\n");
      
      code.append("Word evalDefault();\n");
      
      std::vector<std::string>::iterator pos;
      for(pos = itPos->second[CE_VCG_DEFINITATION_FUNC_EVAL].begin();
          pos != itPos->second[CE_VCG_DEFINITATION_FUNC_EVAL].end();
          ++pos) {
        code.append(*pos);
      }
      for(pos = itPos->second[CE_VCG_DEFINITATION_ADDITIONAL_FUNCTION].begin();
          pos != itPos->second[CE_VCG_DEFINITATION_ADDITIONAL_FUNCTION].end();
          ++pos) {
        code.append(*pos);
      }
      code.append("\nprivate:\n");
      for(pos = itPos->second[CE_VCG_DEFINITATION_VAR].begin();
          pos != itPos->second[CE_VCG_DEFINITATION_VAR].end();
          ++pos) {
        code.append(*pos);
      }
      for(pos = itPos->second[CE_VCG_DEFINITATION_ADDITIONAL_VAR].begin();
          pos != itPos->second[CE_VCG_DEFINITATION_ADDITIONAL_VAR].end();
          ++pos) {
        code.append(*pos);
      }
      for(pos = itPos->second
                [CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_OPEN].begin();
          pos != itPos->second
                 [CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_OPEN].end();
          ++pos) {
        code.append(*pos);
      }
      for(pos = itPos->second
                [CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_REQUEST].begin();
          pos != itPos->second
                 [CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_REQUEST].end();
          ++pos) {
        code.append(*pos);
      }
      code.append("\n};\n\n");
    }
    return code;
  }
  
/*
6.7 Function ~getGenCodeClassConstructor~

A ~get~-function returned a string with the generated ~c++-code~ of
the ~class constructor function~.

*/
  std::string
  CECOpTreeVisitorGenerateCode::getGenCodeClassConstructor() {
    std::string code = "";
    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itPos;
    for(itPos = genCodeNodes.begin(); itPos != genCodeNodes.end(); ++itPos) {
      
      code.append(itPos->first);
      code.append("::");
      code.append(itPos->first);
      code.append("()\n");
      code.append(": ");
      code.append(itPos->second[CE_VCG_SUPERCLASS].at(0));
      code.append("() {\n");
      code.append("ceNodeKey = \"");
      code.append(itPos->first);
      code.append("\";\n");

      std::vector<std::string>::iterator pos;
      for(pos = itPos->second
                [CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY].begin();
          pos != itPos->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY].end();
          ++pos) {
        code.append(*pos);
      }
     
      code.append("}\n\n");
    }
    return code;
  }
  
/*
6.8 Function ~getGenCodeClassDestructor~

A ~get~-function returned a string with the generated ~c++-code~ of
the ~class destructor function~.

*/
  std::string
  CECOpTreeVisitorGenerateCode::getGenCodeClassDestructor() {
    std::string code = "";
    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itPos;
    for(itPos = genCodeNodes.begin(); itPos != genCodeNodes.end(); ++itPos) {
      
      code.append(itPos->first);
      code.append("::~");
      code.append(itPos->first);
      code.append("() {\n");

      std::vector<std::string>::iterator pos;
      for(pos = itPos->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY].begin();
          pos != itPos->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY].end();
          ++pos) {
        code.append(*pos);
      }

      if (itPos->second
          [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_CLOSE_BODY].size() > 0) {
        for(pos = itPos->second
                  [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_CLOSE_BODY].begin();
            pos != itPos->second
                   [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_CLOSE_BODY].end();
            ++pos) {
          code.append(*pos);
        }
      }
      code.append("}\n\n");
    }
    return code;
  }
  
/*
6.9 Function ~getGenCodeFunctionEval~

A ~get~-function returned a string with the generated ~c++-code~ of
the ~eval function~.

*/
  std::string
  CECOpTreeVisitorGenerateCode::getGenCodeFunctionEval() {
    std::string code = "";
    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itPos;
    for(itPos = genCodeNodes.begin(); itPos != genCodeNodes.end(); ++itPos) {
      if (itPos->first.find("arglist_") == std::string::npos
        && itPos->first.find("identifier_") == std::string::npos) {
        code.append("inline ");
        
        std::vector<std::string>::iterator pos;      
        for(pos = itPos->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD].begin();
            pos != itPos->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD].end();
            ++pos) {
          code.append(*pos);
        }
        
        code.append(" {\n");
        for(pos = itPos->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY].begin();
            pos != itPos->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY].end();
            ++pos) {
          code.append(*pos);
        }
        
        code.append("}\n\n");
      }
    }
    return code;
  }
  
/*
6.10 Function ~getGenCodeFunctionInit~

A ~get~-function returned a string with the generated ~c++-code~ of
the ~init function~.

*/
  std::string
  CECOpTreeVisitorGenerateCode::getGenCodeFunctionInit() {
    std::string code = "";
    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itPos;
    for(itPos = genCodeNodes.begin(); itPos != genCodeNodes.end(); ++itPos) {
      code.append("inline bool ");
      code.append(itPos->first);
      code.append("::Init() {\n");

      std::vector<std::string>::iterator pos;
      for(pos = itPos->second[CE_VCG_DECLARATION_FUNC_INIT_BODY].begin();
          pos != itPos->second[CE_VCG_DECLARATION_FUNC_INIT_BODY].end();
          ++pos) {
        code.append(*pos);
      }
     
      code.append("}\n\n");
    }
    return code;
  }
  
/*
6.11 Function ~getGenCodeFunctionOperator~

A ~get~-function returned a string with the generated ~c++-code~ of
the ~evalOpen- and evalRequest- function~.

*/
  std::string
  CECOpTreeVisitorGenerateCode::getGenCodeFunctionOperator() {
    std::string code = "";
    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itPos;
    for(itPos = genCodeNodes.begin(); itPos != genCodeNodes.end(); ++itPos) {
      if (itPos->second
          [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD].size() > 0) {
        code.append("inline ");
        
        std::vector<std::string>::iterator pos;      
        for(pos = itPos->second
                  [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD].begin();
            pos != itPos->second
                   [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD].end();
            ++pos) {
          code.append(*pos);
        }
        
        code.append(" {\n");
        for(pos = itPos->second
                  [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_BODY].begin();
            pos != itPos->second
                   [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_BODY].end();
            ++pos) {
          code.append(*pos);
        }
        
        code.append("}\n\n");
      }

      if (itPos->second
          [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD].size() > 0) {
        code.append("inline ");
        
        std::vector<std::string>::iterator pos;      
        for(pos = itPos->second
                  [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD].begin();
            pos != itPos->second
                   [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD].end();
            ++pos) {
          code.append(*pos);
        }
        
        code.append(" {\n");
        for(pos = itPos->second
                  [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_BODY].begin();
            pos != itPos->second
                   [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_BODY].end();
            ++pos) {
          code.append(*pos);
        }
        
        code.append("}\n\n");
      }
      
    }
    return code;
  }
  
/*
6.12 Function ~getGenCodeDeclarationAdditionalFunction~

A ~get~-function returned a string with the generated ~c++-code~ of
the ~declaration of additional functions~.

*/
  std::string 
  CECOpTreeVisitorGenerateCode::getGenCodeDeclarationAdditionalFunction() {
    std::string code = "";
    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itPos;
    for(itPos = genCodeNodes.begin(); itPos != genCodeNodes.end(); ++itPos) {
      std::vector<std::string>::iterator pos;      
      for(pos = itPos->second
                [CE_VGC_DECLARATION_ADDITIONAL_FUNCTION].begin();
          pos != itPos->second
                 [CE_VGC_DECLARATION_ADDITIONAL_FUNCTION].end();
          ++pos) {
        code.append(*pos);
      }
    }
    return code;
  }
  
/*
6.13 Function ~getGenCodeEvalDefaultFunction~

A ~get~-function returned a string with the generated ~c++-code~ of
the ~evalDefault function~.

*/
  std::string 
  CECOpTreeVisitorGenerateCode::getGenCodeEvalDefaultFunction() {
    std::string code = "";
    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itPos;
    for(itPos = genCodeNodes.begin(); itPos != genCodeNodes.end(); ++itPos) {
      code.append("inline Word ");
      code.append(itPos->first);
      code.append("::evalDefault() {\n");
      if (itPos->first.find("arglist_") == std::string::npos
        && itPos->first.find("identifier_") == std::string::npos)
        code.append("return SetWord(eval());\n");
      else
        code.append("Word w;\nreturn w;\n");
      code.append("}\n\n");
    }
    return code;
  }
  
  
/*
6.14 Function ~getGenCodeClassCECNodeDefault~

A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeDefault~.

*/
  std::string 
  CECOpTreeVisitorGenerateCode::getGenCodeClassCECNodeDefault() {
    std::string code = "";
    code.append("class CECNodeDefault {\n");
    code.append("public:\n");
    code.append("CECNodeDefault() : ceNodeKey("") {};\n");
    code.append("virtual ~CECNodeDefault() {\n");
    code.append("sons.clear();\n");
    code.append("}\n");
    code.append("inline int getNumOfSons() {\n");
    code.append("return sons.size();\n");
    code.append("}\n");
    
    code.append("inline CECNodeDefault*");
    code.append(" getSon(const unsigned int sonPos) {\n");
    
    code.append("CECNodeDefault* retValue = 0;\n");
    code.append("if (sonPos < sons.size())\n");
    code.append("retValue = sons[sonPos];\n");
    code.append("return retValue;\n");
    code.append("}\n");
    code.append("virtual Word evalDefault() = 0;\n");
    code.append("protected:\n");
    code.append("std::vector<CECNodeDefault*> sons;\n");
    code.append("std::string ceNodeKey;\n");
    code.append("};\n\n");
    return code;
  }
  
/*
6.15 Function ~getGenCodeClassCECNodeApplyop~

A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeApplyop~.

*/
  std::string 
  CECOpTreeVisitorGenerateCode::getGenCodeClassCECNodeApplyop() {
    std::string code = "";
    if (cecOpNodeApplyopIsInsert) {
      code.append("class CECNodeApplyop : public CECNodeDefault {\n");
      code.append("public:\n");
      code.append("CECNodeApplyop() {};\n");
      code.append("virtual ~CECNodeApplyop() {};\n");
      code.append("};\n\n\n");
    }
    return code;
  }
  
/*
6.16 Function ~getGenCodeClassCECNodeAbstraction~

A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeAbstraction~.

*/
  std::string 
  CECOpTreeVisitorGenerateCode::getGenCodeClassCECNodeAbstraction() {
    std::string code = "";
    if (cecOpNodeAbstractionIsInsert) {
      code.append("class CECNodeAbstraction : public CECNodeDefault {\n");
      code.append("public:\n");
      code.append("CECNodeAbstraction() {};\n");
      code.append("virtual ~CECNodeAbstraction() {};\n");
      code.append("virtual void setMapType(Word, unsigned int) = 0;\n");
      code.append("};\n\n\n");
    }
    return code;
  }
  
/*
6.17 Function ~getGenCodeClassCECNodeObject~

A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeObject~.

*/
  std::string 
  CECOpTreeVisitorGenerateCode::getGenCodeClassCECNodeObject() {
    std::string code = "";
    if (cecOpNodeObjectIsInsert) {
      code.append("class CECNodeObject : public CECNodeDefault {\n");
      code.append("public:\n");
      code.append("CECNodeObject() {};\n");
      code.append("virtual ~CECNodeObject() {};\n");
      code.append("};\n\n\n");
    }
    return code;
  }
  
/*
6.18 Function ~getGenCodeClassCECNodeConstant~

A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeConstant~.

*/
  std::string 
  CECOpTreeVisitorGenerateCode::getGenCodeClassCECNodeConstant() {
    std::string code = "";
    if (cecOpNodeConstantIsInsert) {
      code.append("class CECNodeConstant : public CECNodeDefault {\n");
      code.append("public:\n");
      code.append("CECNodeConstant() {};\n");
      code.append("virtual ~CECNodeConstant() {};\n");
      code.append("};\n\n\n");
    }
    return code;
  }
  
/*
6.19 Function ~getGenCodeClassCECNodeVariable~

A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeVariable~.

*/
  std::string 
  CECOpTreeVisitorGenerateCode::getGenCodeClassCECNodeVariable() {
    std::string code = "";
    if (cecOpNodeVariableIsInsert) {
      code.append("class CECNodeVariable : public CECNodeDefault {\n");
      code.append("public:\n");
      code.append("CECNodeVariable() {};\n");
      code.append("virtual ~CECNodeVariable() {};\n");
      code.append("};\n\n\n");
    }
    return code;
  }
  
/*
6.20 Function ~getGenCodeClassCECNodeIdentifier~

A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeIdentifier~.

*/
  std::string 
  CECOpTreeVisitorGenerateCode::getGenCodeClassCECNodeIdentifier() {
    std::string code = "";
    if (cecOpNodeIdentifierIsInsert) {
      code.append("class CECNodeIdentifier : public CECNodeDefault {\n");
      code.append("public:\n");
      code.append("CECNodeIdentifier() {};\n");
      code.append("virtual ~CECNodeIdentifier() {};\n");
      code.append("};\n\n\n");
    }
    return code;
  }
  
/*
6.21 Function ~getGenCodeClassCECNodeArglist~

A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeArglist~.

*/
  std::string 
  CECOpTreeVisitorGenerateCode::getGenCodeClassCECNodeArglist() {
    std::string code = "";
    if (cecOpNodeArglistIsInsert) {
      code.append("class CECNodeArglist : public CECNodeDefault {\n");
      code.append("public:\n");
      code.append("CECNodeArglist() {};\n");
      code.append("virtual ~CECNodeArglist() {};\n");
      code.append("};\n\n\n");
    }
    return code;
  }
  
/*
6.22 Function ~getGenCodeCodeCompiledExpressionsDeclaration~

A ~get~-function returned a string with the generated ~c++-code~ of
the declaration of the class ~CodeCompiledExpressions~.

*/
  std::string
  CECOpTreeVisitorGenerateCode::getGenCodeCodeCompiledExpressionsDeclaration()
  {
    bool foundAbstraction, foundApplyop;
    std::string code = "";
    code.append("class CodeCompiledExpressions {\n");
    code.append("public:\n");
    code.append("static CodeCompiledExpressions* getInstance();\n");
    code.append("static bool Init(CEQuery*);\n");
    code.append("static void CloseInstance();\n");
    code.append("virtual ~CodeCompiledExpressions();\n");
    code.append("CEQueryProcessor* getPtrCEQP();\n");
    code.append("CEQuery* getPtrCEQY();\n");

    std::map<std::string, std::string>::iterator itMapType;
    for(itMapType = genCodeCCEGlobalVar.begin();
        itMapType != genCodeCCEGlobalVar.end();
        ++itMapType) {
      code.append("void setMapType_");
      code.append(itMapType->first);
      code.append("(");
      code.append(itMapType->second);
      code.append("*);\n");
      
      code.append(itMapType->second);
      code.append("* getMapType_");
      code.append(itMapType->first);
      code.append("();\n");
    }
    
    std::set<std::string>::iterator itPos;
    for(itPos = genCodeCompExprRootNodes.begin();
        itPos != genCodeCompExprRootNodes.end();
        ++itPos) {
      code.append(*itPos);
      code.append("* getNodePtr_");
      code.append(*itPos);
      code.append("();\n");
    }
    
    
    foundAbstraction = false;
    foundApplyop = false;
    for(itPos = genCodeCompExprRootNodes.begin();
        itPos != genCodeCompExprRootNodes.end();
        ++itPos) {
      if (!foundAbstraction
        && (*itPos).find("abstraction") != std::string::npos) {
        code.append("void setGlobalTuple(Tuple&);\n");
        code.append("Tuple* getPtrGlobalTuple();\n");
        foundAbstraction = true;
      }
      if (!foundApplyop && (*itPos).find("applyop") != std::string::npos) {
        code.append("void setGlobalResultStorage(Word&);\n");
        code.append("Word* getPtrGlobalResultStorage();\n");
        foundApplyop = true;
      }
    }
    code.append("\nprivate:\n");
    code.append("static CodeCompiledExpressions* instance;\n\n");
    code.append("CEQuery* ptrCEQY;\n");
    
    for(itPos = genCodeCompExprRootNodes.begin();
        itPos != genCodeCompExprRootNodes.end();
        ++itPos) {
      code.append(*itPos);
      code.append("* ptr_");
      code.append(*itPos);
      code.append(";\n");
    }
    
    if (cecOpNodeAbstractionIsInsert) {
      code.append("CECNodeAbstraction* ptrActivAbstraction;\n");
    }

    for(itMapType = genCodeCCEGlobalVar.begin();
        itMapType != genCodeCCEGlobalVar.end();
        ++itMapType) {
      code.append(itMapType->second);
      code.append("* ptrMapType_");
      code.append(itMapType->first);
      code.append(";\n");
    }
    
    foundApplyop = false;
    for(itPos = genCodeCompExprRootNodes.begin();
        itPos != genCodeCompExprRootNodes.end();
        ++itPos) {
      if (!foundApplyop && (*itPos).find("applyop") != std::string::npos) {
        code.append("Word* ptrGlobalResultStorage;\n");
        foundApplyop = true;
      }
    }
    code.append("CodeCompiledExpressions() {};\n");
    code.append("CodeCompiledExpressions(CEQuery*);\n");
    code.append("};\n\n");
    return code;
  }
  
/*
6.23 Function ~getGenCodeCodeCompiledExpressionsDefinition~

A ~get~-function returned a string with the generated ~c++-code~ of
the definition of the class ~CodeCompiledExpressions~.

*/
  std::string
  CECOpTreeVisitorGenerateCode::getGenCodeCodeCompiledExpressionsDefinition() {
    std::string code = "";
    code.append("CodeCompiledExpressions*");
    code.append(" CodeCompiledExpressions::instance = 0;\n\n");
    
    code.append("CodeCompiledExpressions::CodeCompiledExpressions");
    code.append("(CEQuery* ceQY)\n");
    
    code.append(":ptrCEQY(ceQY) {\n");
    std::set<std::string>::iterator itPos;
    for(itPos = genCodeCompExprRootNodes.begin();
        itPos != genCodeCompExprRootNodes.end();
        ++itPos) {
      code.append("ptr_");
      code.append(*itPos);
      code.append(" = new ");
      code.append(*itPos);
      code.append("();\n");
    }
    code.append("}\n\n");

    code.append("CodeCompiledExpressions::~CodeCompiledExpressions() {\n");
    for(itPos = genCodeCompExprRootNodes.begin();
        itPos != genCodeCompExprRootNodes.end();
        ++itPos) {
      code.append("if (ptr_");
      code.append(*itPos);
      code.append(")\ndelete ptr_");
      code.append(*itPos);
      code.append(";\nptr_");
      code.append(*itPos);
      code.append(" = 0;\n");
    }
    code.append("ptrCEQY = 0;\n");
    code.append("}\n\n");
    
    code.append("inline CodeCompiledExpressions*");
    code.append(" CodeCompiledExpressions::getInstance() {\n");
    
    code.append("return CodeCompiledExpressions::instance;\n");
    code.append("}\n\n");
    
    code.append("bool CodeCompiledExpressions::Init(CEQuery* ceQY) {\n");
    code.append("bool retValue = true;\n");
    code.append("CodeCompiledExpressions::instance =");
    code.append(" new CodeCompiledExpressions(ceQY);\n");
    
    code.append("if (!CodeCompiledExpressions::instance)\n");
    code.append("retValue = false;\n");
    for(itPos = genCodeCompExprRootNodes.begin();
        itPos != genCodeCompExprRootNodes.end();
        ++itPos) {
      code.append("retValue = retValue");
      code.append(" && CodeCompiledExpressions::instance->getNodePtr_");
      code.append(*itPos);
      code.append("()->Init();\n");
    }
    code.append("return retValue;\n");
    code.append("}\n\n");
    
    code.append("void CodeCompiledExpressions::CloseInstance() {\n");
    code.append("if (CodeCompiledExpressions::instance)\n");
    code.append("delete CodeCompiledExpressions::instance;\n");
    code.append("CodeCompiledExpressions::instance = 0;\n");
    code.append("}\n\n");
    
    code.append("inline CEQueryProcessor*");
    code.append(" CodeCompiledExpressions::getPtrCEQP() {\n");
    
    code.append("return ptrCEQY->getPtrCEQP();\n");
    code.append("}\n\n");
    
    code.append("inline CEQuery* CodeCompiledExpressions::getPtrCEQY() {\n");
    code.append("return ptrCEQY;\n");
    code.append("}\n\n");
    
    for(itPos = genCodeCompExprRootNodes.begin();
        itPos != genCodeCompExprRootNodes.end();
        ++itPos) {
      code.append("inline ");
      code.append(*itPos);
      code.append("* CodeCompiledExpressions::getNodePtr_");
      code.append(*itPos);
      code.append("() {\n");
      code.append("return ptr_");
      code.append(*itPos);
      code.append(";\n");
      code.append("}\n\n");
    }
    
    std::map<std::string, std::string>::iterator itMapType;
    for(itMapType = genCodeCCEGlobalVar.begin();
        itMapType != genCodeCCEGlobalVar.end();
        ++itMapType) {
      code.append("inline void CodeCompiledExpressions::setMapType_");
      code.append(itMapType->first);
      code.append("(");
      code.append(itMapType->second);
      code.append("* arg) {\n");
      code.append("ptrMapType_");
      code.append(itMapType->first);
      code.append(" = arg;\n");
      code.append("}\n\n");
      
      code.append("inline ");
      code.append(itMapType->second);
      code.append("* CodeCompiledExpressions::getMapType_");
      code.append(itMapType->first);
      code.append("() {\n");
      code.append("return ptrMapType_");
      code.append(itMapType->first);
      code.append(";\n");
      code.append("}\n\n");
    }

    bool foundApplyop = false;
    for(itPos = genCodeCompExprRootNodes.begin();
        itPos != genCodeCompExprRootNodes.end();
        ++itPos) {
      if (!foundApplyop && (*itPos).find("applyop") != std::string::npos) {
        code.append("inline void");
        code.append(" CodeCompiledExpressions::setGlobalResultStorage");
        code.append(" (Word& result) {\n");
        
        code.append("ptrGlobalResultStorage = &result;\n");
        code.append("}\n\n");
        code.append("inline Word*");
        code.append(" CodeCompiledExpressions::getPtrGlobalResultStorage");
        code.append("() {\n");
        
        code.append("return ptrGlobalResultStorage;\n");
        code.append("}\n\n");
        foundApplyop = true;
      }
    }
    return code;
  }
  
  
/*
6.24 Function ~getGenCodeLibCallFunction~

A ~get~-function returned a string with the generated ~c++-code~ of
the the ~external library call functions section~.

*/
  std::string 
  CECOpTreeVisitorGenerateCode::getGenCodeLibCallFunction() {
    std::string code = "";
    code.append("#ifdef __cplusplus\n");
    code.append("extern \"C\" {\n");
    code.append("#endif\n\n");
    std::map<std::string, std::string>::iterator pos;
    for(pos = operators_libcall_function.begin();
        pos != operators_libcall_function.end();
        ++pos) {
      code.append(pos->first);
      code.append(pos->second);
      code.append("\n");
    }
    code.append("#ifdef __cplusplus\n");
    code.append("}\n");
    code.append("#endif\n\n");
    
    return code;
  }
  
/*
6.25 Function ~insertCommaInCodeVector~

This function adds commas to lists of arguments, parameters or similar in the
source code to be generated.

*/
  void 
  CECOpTreeVisitorGenerateCode::insertCommaInCodeVector
    (int startPos,
     int actualPos,
     std::vector<std::string>(&codeVector)) {
    
    int commaPos = actualPos + 1;
    bool setComma = false;
    while (!setComma && actualPos >= startPos) {
      if (codeVector.at(actualPos).length() > 0) {
        codeVector.at(commaPos) = ", ";
        setComma = true;
      }
      actualPos = actualPos - 2;
    }
    
  }

/*
6.26 Function ~initGenCodeNodesEntry~

This function initialized the data structure of code segments for a new node in
the ~operator tree~ and return a iterator of this structure.

*/
  std::map<std::string, std::vector<std::vector<std::string> > >::iterator
  CECOpTreeVisitorGenerateCode::initGenCodeNodesEntry(std::string strNodeKey,
                                                      unsigned int numSons) {
    
    std::map<std::string,
    std::vector<std::vector<std::string> > >::iterator itTestInsertMap;
    itTestInsertMap = genCodeNodes.find(strNodeKey);
    if (itTestInsertMap == genCodeNodes.end()) {
      std::vector<std::vector<std::string> > codeClassNode(CE_VCG_CT_SIZEOF);
      genCodeNodes[strNodeKey] = codeClassNode;
    }

    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itMap;
    itMap = genCodeNodes.find(strNodeKey);

    std::vector<std::string> ceSuperClass(1);
    itMap->second[CE_VCG_SUPERCLASS] = ceSuperClass;

    std::vector<std::string> ceDefinitionVariables(numSons + 1);
    itMap->second[CE_VCG_DEFINITATION_VAR] = ceDefinitionVariables;

    std::vector<std::string> ceDefinitionAdditionalVariables(numSons + 1);
    itMap->second[CE_VCG_DEFINITATION_ADDITIONAL_VAR] = 
                                     ceDefinitionAdditionalVariables;

    std::vector<std::string> ceDefinitionFuncGetSonNode(numSons);
    itMap->second[CE_VCG_DEFINITATION_FUNC_GET_SON_NODE] =
                                     ceDefinitionFuncGetSonNode;

    std::vector<std::string> ceDefinitionFuncEval(1);
    itMap->second[CE_VCG_DEFINITATION_FUNC_EVAL] = ceDefinitionFuncEval;
    
    std::vector<std::string> ceDefinitionFuncOperatorEvalOpen(0);
    itMap->second[CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_OPEN] =
                                     ceDefinitionFuncOperatorEvalOpen;

    std::vector<std::string> ceDefinitionFuncOperatorEvalRequest(0);
    itMap->second[CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_REQUEST] =
                                     ceDefinitionFuncOperatorEvalRequest;
    
    std::vector<std::string> ceDefinitionAdditionalFunction(0);
    itMap->second[CE_VCG_DEFINITATION_ADDITIONAL_FUNCTION] =
                                     ceDefinitionAdditionalFunction;

    std::vector<std::string> ceDeclarationContructorBody(numSons + 3);
    itMap->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY] =
                                     ceDeclarationContructorBody;
    
    std::vector<std::string> ceDeclarationDestructorBody(numSons + 1);
    itMap->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY] =
                                     ceDeclarationDestructorBody;
    
    std::vector<std::string> ceDeclarationFuncInitBody(3 * numSons + 3);
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY] =
                                     ceDeclarationFuncInitBody;

    std::vector<std::string> ceDeclarationFuncEvalHead(1);
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD] =
                                     ceDeclarationFuncEvalHead;

    std::vector<std::string> ceDeclarationFuncEvalBody(2 * numSons + 2);
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY] =
                                     ceDeclarationFuncEvalBody;
    
    std::vector<std::string> ceDeclarationFuncOperatorEvalOpenHead(0);
    itMap->second[CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD] =
                                     ceDeclarationFuncOperatorEvalOpenHead;
    
    std::vector<std::string> ceDeclarationFuncOperatorEvalOpenBody(0);
    itMap->second[CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_BODY] =
                                     ceDeclarationFuncOperatorEvalOpenBody;
    
    std::vector<std::string> ceDeclarationFuncOperatorEvalRequestHead(0);
    itMap->second[CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD] =
                                     ceDeclarationFuncOperatorEvalRequestHead;
    
    std::vector<std::string> ceDeclarationFuncOperatorEvalRequestBody(0);
    itMap->second[CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_BODY] =
                                     ceDeclarationFuncOperatorEvalRequestBody;
    
    std::vector<std::string> ceDeclarationFuncOperatorEvalCloseBody(0);
    itMap->second[CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_CLOSE_BODY] =
                                     ceDeclarationFuncOperatorEvalCloseBody;
    
    std::vector<std::string> ceDeclarationAdditionalFunction(0);
    itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION] =
                                     ceDeclarationAdditionalFunction;
    
    return itMap;
  }
  
/*
6.27 Function ~completeGenCodeFatherNodeEntry~

This function completes the code segments for the father node ~ceNode~
in the operator tree.

*/
  void
  CECOpTreeVisitorGenerateCode::completeGenCodeFatherNodeEntry
  (CECOpNode& ceNode) {
    
    int posSons = ceNode.getSonsPosition();
    if (posSons >= 0) {
      std::map<std::string,
               std::vector<std::vector<std::string> > >::iterator itFatherNode;
      itFatherNode = genCodeNodes.find(ceNode.getPtrFatherNode()
                                                ->getStrNodeKey());
      if (itFatherNode != genCodeNodes.end()) {
        itFatherNode->second[CE_VCG_DEFINITATION_VAR]
                        .at(posSons).append(ceNode.getStrNodeKey());
        itFatherNode->second[CE_VCG_DEFINITATION_VAR]
                        .at(posSons).append("* ptr_");
        itFatherNode->second[CE_VCG_DEFINITATION_VAR]
                        .at(posSons).append(ceNode.getStrNodeKey());
        itFatherNode->second[CE_VCG_DEFINITATION_VAR]
                        .at(posSons).append(";\n");
        
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
                        .at(posSons + 2).append("ptr_");
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
                        .at(posSons + 2).append(ceNode.getStrNodeKey());
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
                        .at(posSons + 2).append(" = new ");
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
                        .at(posSons + 2).append(ceNode.getStrNodeKey());
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
                        .at(posSons + 2).append("();\n");
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
                        .at(posSons + 2).append("sons.push_back(ptr_");
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
                        .at(posSons + 2).append(ceNode.getStrNodeKey());
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
                        .at(posSons + 2).append(");\n");
        
        
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY]
                        .at(posSons).append("if (ptr_");
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY]
                        .at(posSons).append(ceNode.getStrNodeKey());
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY]
                        .at(posSons).append(")\ndelete ptr_");
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY]
                        .at(posSons).append(ceNode.getStrNodeKey());
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY]
                        .at(posSons).append(";\nptr_");
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY]
                        .at(posSons).append(ceNode.getStrNodeKey());
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY]
                        .at(posSons).append(" = 0;\n");
        
        itFatherNode->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
                        .at(posSons).append("\n&& ptr_");
        itFatherNode->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
                        .at(posSons).append(ceNode.getStrNodeKey());
        itFatherNode->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
                        .at(posSons).append("->Init()");
        
        
        if (itFatherNode->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
                        .at(ceNode.getPtrFatherNode()->getNumSons() + 1)
                        .find("&& EvalOpen(") != std::string::npos) {
          if (posSons > 1) {
            insertCommaInCodeVector
              (ceNode.getPtrFatherNode()->getNumSons() + 2,
               ceNode.getPtrFatherNode()->getNumSons() + 2 * posSons - 2,
               itFatherNode->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]);
          }
          itFatherNode->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
                        .at(ceNode.getPtrFatherNode()->getNumSons()
                                                       + 2 * posSons)
                        .append("ptr_");
          itFatherNode->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
                        .at(ceNode.getPtrFatherNode()->getNumSons()
                                                       + 2 * posSons)
                        .append(ceNode.getStrNodeKey());
        }
        
        if (itFatherNode->second[CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_OPEN]
                            .size() > 0) {
          if (posSons > 1) {
            insertCommaInCodeVector
              (1,
               2 * (posSons - 1) - 1,
               itFatherNode->second
                             [CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_OPEN]);
          }
          itFatherNode->second[CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_OPEN]
                        .at(2 * (posSons - 1) + 1)
                        .append(ceNode.getStrNodeKey());
          itFatherNode->second[CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_OPEN]
                        .at(2 * (posSons - 1) + 1).append("*");
        }
        
        if (itFatherNode->second[CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_REQUEST]
                            .size() > 0) {
          if (posSons > 1) {
            insertCommaInCodeVector
              (1,
               2 * (posSons - 1) - 1,
               itFatherNode->second
                             [CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_REQUEST]);
          }
          itFatherNode->second[CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_REQUEST]
                        .at(2 * (posSons - 1) + 1)
                        .append(ceNode.getStrNodeKey());
          itFatherNode->second[CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_REQUEST]
                        .at(2 * (posSons - 1) + 1).append("*");
        }
        
        if (itFatherNode->second
                          [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD]
                          .size() > 0) {
          if (posSons > 1) {
            insertCommaInCodeVector
              (1,
               2 * (posSons - 1) - 1,
               itFatherNode->second
                             [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD]
              );
          }
          itFatherNode->second[CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD]
                        .at(2 * (posSons - 1) + 1)
                        .append(ceNode.getStrNodeKey());
          itFatherNode->second[CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD]
                        .at(2 * (posSons - 1) + 1).append("* arg_");
        }
        
        if (itFatherNode->second
                          [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD]
                          .size() > 0) {
          if (posSons > 1) {
            insertCommaInCodeVector
              (1,
               2 * (posSons - 1) - 1,
               itFatherNode->second
               [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD]);
          }
          itFatherNode->second
                        [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD]
                        .at(2 * (posSons - 1) + 1)
                        .append(ceNode.getStrNodeKey());
          itFatherNode->second
                        [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD]
                        .at(2 * (posSons - 1) + 1)
                        .append("* arg_");
        }
        
        if (itFatherNode->first.find("applyop") != std::string::npos 
          || itFatherNode->first.find("abstraction") != std::string::npos) {
          if (posSons > 1) {
            insertCommaInCodeVector
              (2,
               2 * (posSons - 1),
               itFatherNode->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]);
          }
          itFatherNode->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
                        .at(2 * (posSons - 1) + 2).append("ptr_");
          itFatherNode->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
                        .at(2 * (posSons - 1) + 2)
                        .append(ceNode.getStrNodeKey());
          if (itFatherNode->first.find("abstraction") != std::string::npos) {
            itFatherNode->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
                        .at(2 * (posSons - 1) + 2).append("->eval()");
          }
        }
      }
    }
  }
  
  
/*
6.27 Functions ~visit~

The following functions implement the visitor's behavior for each node type of the operator tree.

These functions implement the abstract functions of the base class ~CECOpTreeVisitor~.

*/
  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNode& ceNode) {
  }

  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNodeOperator& ceNode) {
    std::vector<CECGImplSecondoType*> localSignatureVector
                                        = ceNode.getSignatureVector();
                                        
    int posSons = ceNode.getSonsPosition();
    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itFatherNode;
    itFatherNode = genCodeNodes.find
                   (ceNode.getPtrFatherNode()->getStrNodeKey());
    if (itFatherNode != genCodeNodes.end()) {
      if (ceNode.getPtrCodeStoreOperator()->isStreamOperator()) {
        itFatherNode->second[CE_VCG_DEFINITATION_ADDITIONAL_VAR]
                      .at(posSons).append(
          ceNode.getPtrCodeStoreOperator()
                 ->getCodeOperatorAdditionalVariablesDefinition()
        );

        itFatherNode->second
                      [CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_OPEN]
                      .resize(2 * ceNode.getPtrFatherNode()->getNumSons() + 1);
        itFatherNode->second
                      [CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_OPEN]
                      .at(0).append("bool EvalOpen(");
        itFatherNode->second
                      [CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_OPEN]
                      .at(2 * ceNode.getPtrFatherNode()->getNumSons())
                      .append(");\n");

        itFatherNode->second
                      [CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
                      .at(posSons + 2).append(
          ceNode.getPtrCodeStoreOperator()
                 ->getCodeOperatorAdditionalVariablesInitialisation()
        );
        
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY]
                      .at(posSons).append(
          ceNode.getPtrCodeStoreOperator()->getCodeOperatorEvalCloseBody()
        );
        
        itFatherNode->second
                      [CE_VCG_DECLARATION_FUNC_INIT_BODY]
                      .at(ceNode.getPtrFatherNode()->getNumSons() + 1)
                      .append("retValue = retValue && EvalOpen(");
        itFatherNode->second
                      [CE_VCG_DECLARATION_FUNC_INIT_BODY]
                      .at(3 * ceNode.getPtrFatherNode()->getNumSons() + 1)
                      .append(");\n");

        itFatherNode->second
                      [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD]
                      .resize(2 * ceNode.getPtrFatherNode()->getNumSons() + 1);
        itFatherNode->second
                      [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD]
                      .at(0).append("bool ");
        itFatherNode->second
                      [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD]
                      .at(0)
                      .append(ceNode.getPtrFatherNode()->getStrNodeKey());
        itFatherNode->second
                      [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD]
                      .at(0).append("::EvalOpen(");
        itFatherNode->second
                      [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD]
                      .at(2 * ceNode.getPtrFatherNode()->getNumSons())
                      .append(")");
        
        itFatherNode->second
                      [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_BODY]
                      .resize(1);
        itFatherNode->second
                      [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_BODY]
                      .at(0).append
                             (ceNode.getPtrCodeStoreOperator()
                                     ->getCodeOperatorEvalOpenBody());
      }
      
      itFatherNode->second
                    [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_BODY]
                    .resize(1);
      itFatherNode->second
                    [CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_BODY]
                    .at(0).append
                           (ceNode.getPtrCodeStoreOperator()
                                   ->getCodeOperatorEvalRequestBody
                                       (localSignatureVector)
      );

      if (ceNode.getPtrCodeStoreOperator()->isGenerateNewObject()) {
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
                        .at(0).append("new ");
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
                        .at(2).append("(false);\n");
        
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY]
                        .at(0).append("if (resultStorage)\n");
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY]
                        .at(0).append("resultStorage->DeleteIfAllowed();\n");
      } else {
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
                        .at(0).append("0;\n");
        itFatherNode->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
                        .at(1) = "";
      }
    }
  }
  
  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNodeApplyop& ceNode) {
    cecOpNodeApplyopIsInsert = true;

    ceNode.getPtrCodeStoreType()->getHeaderFilesEntry(headerfiles);
    
    bool localUseSubType = 
            (ceNode.getPtrCodeStoreType()->getTypeName() == "stream");
    
    std::map<std::string, 
             std::vector<std::vector<std::string> > >::iterator itMap;
    itMap = initGenCodeNodesEntry(ceNode.getStrNodeKey(), ceNode.getNumSons());

    itMap->second[CE_VCG_SUPERCLASS].at(0).append("CECNodeApplyop");

    if (localUseSubType)
      itMap->second[CE_VCG_DEFINITATION_VAR].at(0)
               .append(ceNode.getPtrSubCodeStoreType()->getTypeClassName());
    else
      itMap->second[CE_VCG_DEFINITATION_VAR].at(0)
               .append(ceNode.getPtrCodeStoreType()->getTypeClassName());
    itMap->second[CE_VCG_DEFINITATION_VAR].at(0).append("* resultStorage;\n");

    if (localUseSubType)
      itMap->second[CE_VCG_DEFINITATION_FUNC_EVAL].at(0)
               .append(ceNode.getPtrSubCodeStoreType()->getTypeClassName());
    else
      itMap->second[CE_VCG_DEFINITATION_FUNC_EVAL].at(0)
               .append(ceNode.getPtrCodeStoreType()->getTypeClassName());
    itMap->second[CE_VCG_DEFINITATION_FUNC_EVAL].at(0).append("* eval();\n");

    itMap->second[CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_REQUEST]
             .resize(2 * ceNode.getNumSons() + 1);
    itMap->second[CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_REQUEST]
             .at(0).append("void EvalRequest(");
    itMap->second[CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_REQUEST]
             .at(2 * ceNode.getNumSons()).append(");\n");
    
    itMap->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
             .at(0).append("resultStorage = ");
    itMap->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
             .at(1).append(ceNode.getPtrCodeStoreType()->getTypeClassName());
    
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append("bool retValue = true");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(ceNode.getNumSons() + 1).append(";\n");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(3 * ceNode.getNumSons() + 2).append("return retValue;\n");
    
    if (localUseSubType)
      itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
               .at(0)
               .append(ceNode.getPtrSubCodeStoreType()->getTypeClassName());
    else
      itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
               .at(0)
               .append(ceNode.getPtrCodeStoreType()->getTypeClassName());
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append("* ");
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append(itMap->first);
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append("::eval()");

    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
             .at(1).append("EvalRequest(");
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
             .at(2 * ceNode.getNumSons() + 1).append(");\n");
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
             .at(2 * ceNode.getNumSons() + 1)
             .append("return resultStorage;\n");
    
    itMap->second[CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD]
             .resize(2 * ceNode.getNumSons() + 1);
    itMap->second[CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD]
             .at(0).append("void ");
    itMap->second[CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD]
             .at(0).append(itMap->first);
    itMap->second[CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD]
             .at(0).append("::EvalRequest(");
    itMap->second[CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD]
             .at(2 * ceNode.getNumSons()).append(")");
    
  
    completeGenCodeFatherNodeEntry(ceNode);

    if (ceNode.getIsCERootNode(true)) {
      genCodeCompExprRootNodes.insert(itMap->first);
    
      itMap->second[CE_VCG_DEFINITATION_VAR].at(0).append("Supplier sup;\n");
      
      itMap->second[CE_VCG_DEFINITATION_ADDITIONAL_FUNCTION].resize(1);
      itMap->second[CE_VCG_DEFINITATION_ADDITIONAL_FUNCTION]
               .at(0).append("void setSupplier(Supplier);\n");
      
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION].resize(1);
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("void ");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append(itMap->first);
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("::setSupplier(Supplier s) {\n");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("sup = s;\n");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("}\n\n");
      
      std::string function_head = "void ";
      function_head.append
                    (ceNode.getInstCEGenerateFunctionStatus()
                            ->getCallLibFunctionName());
      function_head.append("(Word& result, Supplier s)");
      std::string code = " {\n";
      code.append("CodeCompiledExpressions::getInstance()->getNodePtr_");
      code.append(itMap->first);
      code.append("()->setSupplier(s);\n");
      code.append(ceNode.getPtrCodeStoreType()->getTypeClassName());
      code.append("* retValue = CodeCompiledExpressions::getInstance()");
      code.append("->getNodePtr_");
      code.append(itMap->first);
      code.append("()->eval();\n");
      code.append("result.setAddr(retValue);\n}\n");
    
      operators_libcall_function[function_head] = code;
    }
    
  }
  
  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNodeAbstraction& ceNode) {
    cecOpNodeAbstractionIsInsert = true;

    std::vector<CECGImplSecondoType*> ptrCodeStoreTypes
                                        = ceNode.getVectorPtrCodeStoreTypes();
    for (unsigned int i = 0; i < ptrCodeStoreTypes.size(); i++)
      ptrCodeStoreTypes[i]->getHeaderFilesEntry(headerfiles);
    
    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itMap;
    itMap = initGenCodeNodesEntry(ceNode.getStrNodeKey(),
                                  ceNode.getNumSons());

    itMap->second[CE_VCG_SUPERCLASS].at(0).append("CECNodeAbstraction");

    itMap->second[CE_VCG_DEFINITATION_VAR]
             .at(0)
             .append(ptrCodeStoreTypes[ptrCodeStoreTypes.size() - 1]
                     ->getTypeClassName());
    itMap->second[CE_VCG_DEFINITATION_VAR].at(0)
             .append("* resultStorage;\n");
    for (unsigned int idx = 0; idx <= ptrCodeStoreTypes.size() - 2; idx++) {
      itMap->second[CE_VCG_DEFINITATION_VAR].at(0)
               .append(ptrCodeStoreTypes[idx]->getTypeClassName());
      itMap->second[CE_VCG_DEFINITATION_VAR].at(0)
               .append("* ptrLocalMapType_");
      std::stringstream ss;
      ss << idx + 1;
      itMap->second[CE_VCG_DEFINITATION_VAR].at(0).append(ss.str());
      itMap->second[CE_VCG_DEFINITATION_VAR].at(0).append(";\n");
    }

    itMap->second[CE_VCG_DEFINITATION_FUNC_EVAL]
             .at(0)
             .append(ptrCodeStoreTypes[ptrCodeStoreTypes.size() - 1]
                     ->getTypeClassName());
    itMap->second[CE_VCG_DEFINITATION_FUNC_EVAL].at(0).append("* eval();\n");
      
    itMap->second[CE_VCG_DEFINITATION_ADDITIONAL_FUNCTION].resize(1);
    itMap->second[CE_VCG_DEFINITATION_ADDITIONAL_FUNCTION].at(0)
             .append("void setMapType(Word, unsigned int);\n");
    for (unsigned int idx = 0; idx <= ptrCodeStoreTypes.size() - 2; idx++) {
      std::stringstream ss;
      ss << idx + 1;
      itMap->second[CE_VCG_DEFINITATION_ADDITIONAL_FUNCTION]
               .at(0).append("void setMapType_");
      itMap->second[CE_VCG_DEFINITATION_ADDITIONAL_FUNCTION]
               .at(0).append(ss.str());
      itMap->second[CE_VCG_DEFINITATION_ADDITIONAL_FUNCTION]
               .at(0).append("(");
      itMap->second[CE_VCG_DEFINITATION_ADDITIONAL_FUNCTION]
               .at(0).append(ptrCodeStoreTypes[idx]->getTypeClassName());
      itMap->second[CE_VCG_DEFINITATION_ADDITIONAL_FUNCTION]
               .at(0).append("*);\n");
    }

    itMap->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY]
             .at(0).append("resultStorage = 0;\n");

    itMap->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY]
             .at(0).append("resultStorage = 0;\n");

    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append("bool retValue = true");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(ceNode.getNumSons() + 1).append(";\n");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(3 * ceNode.getNumSons() + 2).append("return retValue;\n");
    
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0)
             .append(ptrCodeStoreTypes[ptrCodeStoreTypes.size() - 1]
                     ->getTypeClassName());
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append("* ");
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append(itMap->first);
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append("::eval()");

    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
             .at(1).append("resultStorage = ");
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
             .at(2 * ceNode.getNumSons() + 1).append(";\n");
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
             .at(2 * ceNode.getNumSons() + 1)
             .append("return resultStorage;\n");

    itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION].resize(1);
    itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
             .at(0).append("inline void ");
    itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
             .at(0).append(itMap->first);
    itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
             .at(0)
             .append("::setMapType(Word wordMapType, unsigned int idx) {\n");
    itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
             .at(0).append("switch (idx) {\n");
    for (unsigned int idx = 1; idx <= ptrCodeStoreTypes.size() - 1; idx++) {
      std::stringstream ss;
      ss << idx;
      std::string strArgIdx = ss.str();
      std::string strMapTypePosition = "";
      strMapTypePosition.append(ceNode.getStrFunctionNo());
      strMapTypePosition.append("_");
      strMapTypePosition.append(strArgIdx);

      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("case ");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append(strArgIdx);
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append(":\n{\n");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append(ptrCodeStoreTypes[idx-1]->getTypeClassName());
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("* mapType = (");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append(ptrCodeStoreTypes[idx-1]->getTypeClassName());
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("*) wordMapType.addr;\n");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("setMapType_");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append(strArgIdx);
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("(mapType);\n");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("break;\n");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("}\n");
    }
    itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
             .at(0).append("}\n}\n\n");
    
    
    
    for (unsigned int idx = 0; idx <= ptrCodeStoreTypes.size() - 2; idx++) {
      std::stringstream ss;
      ss << idx + 1;
      std::string strArgIdx = ss.str();
      std::string strMapTypePosition = "";
      strMapTypePosition.append(ceNode.getStrFunctionNo());
      strMapTypePosition.append("_");
      strMapTypePosition.append(strArgIdx);
      
      genCodeCCEGlobalVar[strMapTypePosition]
             = ptrCodeStoreTypes[idx]->getTypeClassName();
      
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("inline void ");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append(itMap->first);
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("::setMapType_");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append(strArgIdx);
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("(");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append(ptrCodeStoreTypes[idx]->getTypeClassName());
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("* mapType) {\n");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0)
               .append("CodeCompiledExpressions::getInstance()->setMapType_");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append(strMapTypePosition);
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("(mapType);\n");
      itMap->second[CE_VGC_DECLARATION_ADDITIONAL_FUNCTION]
               .at(0).append("}\n\n");
    }

    
    completeGenCodeFatherNodeEntry(ceNode);

    if (ceNode.getIsCERootNode(true)) {
      genCodeCompExprRootNodes.insert(itMap->first);
      std::string function_head;
      if ((ceNode.getInstCEGenerateFunctionStatus()->getCallLibFunctionName())
                    .find("filterFunction") != std::string::npos) {
        function_head = "bool ";
      } else {
        function_head = ptrCodeStoreTypes[1]->getTypeClassName();
        function_head.append("* ");
      }
      function_head.append
                    (ceNode.getInstCEGenerateFunctionStatus()
                            ->getCallLibFunctionName());
      function_head.append("(");
      function_head.append(ptrCodeStoreTypes[0]->getTypeClassName());
      function_head.append("* inputTuple)");
      
      std::string code = " {\n";
      code.append("CodeCompiledExpressions::getInstance()->getNodePtr_");
      code.append(itMap->first);
      code.append("()->setMapType_1(inputTuple);\n");
      if ((ceNode.getInstCEGenerateFunctionStatus()->getCallLibFunctionName())
                     .find("filterFunction") != std::string::npos) {
        code.append(ptrCodeStoreTypes[1]->getTypeClassName());
        code.append("* attrBool = CodeCompiledExpressions::getInstance()");
        code.append("->getNodePtr_");
        code.append(itMap->first);
        code.append("()->eval();\n");
      
        code.append("if (attrBool->IsDefined())\n");
        code.append("return attrBool->GetBoolval();\n");
        code.append("else\n");
        code.append("return false;\n");
      } else {
        code.append("return CodeCompiledExpressions::getInstance()");
        code.append("->getNodePtr_");
        code.append(itMap->first);
        code.append("()->eval();\n");
      }
      code.append("}\n");
      
      operators_libcall_function[function_head] = code;
    }
    
  }
  
  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNodeObject& ceNode) {
    cecOpNodeObjectIsInsert = true;
    
    ceNode.getPtrCodeStoreType()->getHeaderFilesEntry(headerfiles);
    
    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itMap;
    itMap = initGenCodeNodesEntry(ceNode.getStrNodeKey(),
                                  ceNode.getNumSons());

    itMap->second[CE_VCG_SUPERCLASS].at(0).append("CECNodeObject");

    itMap->second[CE_VCG_DEFINITATION_VAR].at(0)
             .append(ceNode.getPtrCodeStoreType()->getTypeClassName());
    itMap->second[CE_VCG_DEFINITATION_VAR].at(0)
             .append("* resultStorage;\n");

    itMap->second[CE_VCG_DEFINITATION_FUNC_EVAL].at(0)
             .append(ceNode.getPtrCodeStoreType()->getTypeClassName());
    itMap->second[CE_VCG_DEFINITATION_FUNC_EVAL].at(0).append("* eval();\n");
    
    itMap->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY].at(0)
             .append("resultStorage = 0;\n");
    
    itMap->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY].at(0)
             .append("resultStorage = 0;\n");
    
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append("bool retValue;\n");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append("resultStorage = (");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append(ceNode.getPtrCodeStoreType()->getTypeClassName());
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0)
             .append("*)(CodeCompiledExpressions::getInstance()");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0)
             .append("->getPtrCEQP()->getQPValues(");

    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append(ceNode.getQPValueIdx());
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append(")).addr;\n");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append("if (!resultStorage)\nretValue = false;\nelse\n");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append("retValue = true");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(ceNode.getNumSons() + 1).append(";\n");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(3 * ceNode.getNumSons() + 2).append("return retValue;\n");

    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append(ceNode.getPtrCodeStoreType()->getTypeClassName());
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append("* ");
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append(itMap->first);
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append("::eval()");

    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
             .at(0).append("return resultStorage;\n");
    
    completeGenCodeFatherNodeEntry(ceNode);
  }
  
  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNodeConstant& ceNode) {
    cecOpNodeConstantIsInsert = true;
    
    ceNode.getPtrCodeStoreType()->getHeaderFilesEntry(headerfiles);

    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itMap;
    itMap = initGenCodeNodesEntry(ceNode.getStrNodeKey(),
                                  ceNode.getNumSons());

    itMap->second[CE_VCG_SUPERCLASS].at(0).append("CECNodeConstant");

    itMap->second[CE_VCG_DEFINITATION_VAR].at(0)
             .append(ceNode.getPtrCodeStoreType()->getTypeClassName());
    itMap->second[CE_VCG_DEFINITATION_VAR].at(0)
             .append("* resultStorage;\n");

    itMap->second[CE_VCG_DEFINITATION_FUNC_EVAL].at(0)
             .append(ceNode.getPtrCodeStoreType()->getTypeClassName());
    itMap->second[CE_VCG_DEFINITATION_FUNC_EVAL].at(0)
             .append("* eval();\n");
    
    itMap->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY].at(0)
             .append("resultStorage = 0;\n");
    
    itMap->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY].at(0)
            .append("if (resultStorage)\nresultStorage->DeleteIfAllowed();\n");
    
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append("bool retValue;\n");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append("resultStorage = (");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append(ceNode.getPtrCodeStoreType()->getTypeClassName());
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0)
             .append("*)(CodeCompiledExpressions::getInstance()");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0)
             .append("->getPtrCEQP()->getQPValues(");
             
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append(ceNode.getQPValueIdx());
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append(")).addr;\n");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append("if (!resultStorage)\nretValue = false;\nelse\n");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append("retValue = resultStorage->IsDefined()");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(ceNode.getNumSons() + 1).append(";\n");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(3 * ceNode.getNumSons() + 2).append("return retValue;\n");

    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append(ceNode.getPtrCodeStoreType()->getTypeClassName());
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD].
             at(0).append("* ");
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append(itMap->first);
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append("::eval()");

    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
             .at(0).append("return resultStorage;\n");
    
    
    completeGenCodeFatherNodeEntry(ceNode);
  }
 
  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNodeVariable& ceNode) {
    cecOpNodeVariableIsInsert = true;
    
    ceNode.getPtrCodeStoreType()->getHeaderFilesEntry(headerfiles);

    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itMap;
    itMap = initGenCodeNodesEntry(ceNode.getStrNodeKey(),
                                  ceNode.getNumSons());

    itMap->second[CE_VCG_SUPERCLASS].at(0).append("CECNodeVariable");

    itMap->second[CE_VCG_DEFINITATION_VAR].at(0)
             .append(ceNode.getPtrCodeStoreType()->getTypeClassName());
    itMap->second[CE_VCG_DEFINITATION_VAR].at(0)
             .append("* resultStorage;\n");

    itMap->second[CE_VCG_DEFINITATION_FUNC_EVAL].at(0)
             .append(ceNode.getPtrCodeStoreType()->getTypeClassName());
    itMap->second[CE_VCG_DEFINITATION_FUNC_EVAL].at(0)
             .append("* eval();\n");
    
    itMap->second[CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY].at(0)
             .append("resultStorage = 0;\n");
    
    itMap->second[CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY].at(0)
             .append("resultStorage = 0;\n");
    
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append("bool retValue = true");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(ceNode.getNumSons() + 1).append(";\n");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(3 * ceNode.getNumSons() + 2).append("return retValue;\n");

    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append(ceNode.getPtrCodeStoreType()->getTypeClassName());
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append("* ");
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append(itMap->first);
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_HEAD]
             .at(0).append("::eval()");

    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
             .at(0).append("resultStorage = ");
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
             .at(0).append("CodeCompiledExpressions::getInstance()");
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
             .at(0).append("->getMapType_");
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
             .at(0).append(ceNode.getStrVarPosition());
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
             .at(0).append("();\n");
    itMap->second[CE_VGC_DECLARATION_FUNC_EVAL_BODY]
             .at(0).append("return resultStorage;\n");

    completeGenCodeFatherNodeEntry(ceNode);
  }
  
  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNodeIdentifier& ceNode) {
    cecOpNodeIdentifierIsInsert = true;

    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itMap;
    itMap = initGenCodeNodesEntry(ceNode.getStrNodeKey(), ceNode.getNumSons());

    itMap->second[CE_VCG_SUPERCLASS].at(0).append("CECNodeIdentifier");

    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append("bool retValue = true");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(ceNode.getNumSons() + 1).append(";\n");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(3 * ceNode.getNumSons() + 2).append("return retValue;\n");

    completeGenCodeFatherNodeEntry(ceNode);
  }
  
  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNodeArglist& ceNode) {
    cecOpNodeArglistIsInsert = true;
    
    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator itMap;
    itMap = initGenCodeNodesEntry(ceNode.getStrNodeKey(), ceNode.getNumSons());

    itMap->second[CE_VCG_SUPERCLASS].at(0).append("CECNodeArglist");

    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(0).append("bool retValue = true");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(ceNode.getNumSons() + 1).append(";\n");
    itMap->second[CE_VCG_DECLARATION_FUNC_INIT_BODY]
             .at(3 * ceNode.getNumSons() + 2).append("return retValue;\n");
    
    completeGenCodeFatherNodeEntry(ceNode);
  }
  
  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNodeFunction& ceNode) {
  }
  
  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNodeApplyabs& ceNode) {
  }
  
  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNodeApplyfun& ceNode) {
  }
  
  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNodeCounterdef& ceNode) {
  }
  
  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNodePointer& ceNode) {
  }
  
  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNodePredinfodef& ceNode) {
  }
  
  void
  CECOpTreeVisitorGenerateCode::visit(CECOpNodeMemorydef& ceNode) {
  }
  
/*
6.28 Functions ~createCELibDefaultFunc~

This function generated ~c++~-code of the the external library
call functions ~initCompiledExpressionsLib~ and ~closeCompiledExpressionsLib~.

*/
  void
  CECOpTreeVisitorGenerateCode::createCELibDefaultFunc() {
    std::string function_head_init
                  = "bool initCompiledExpressionsLib(CEQuery* ceQY)";
    std::string code_init = " {\n"; 
    code_init.append("if (ceQY)\n");
    code_init.append("return CodeCompiledExpressions::Init(ceQY);\n");
    code_init.append("else\n");
    code_init.append("return false;\n");
    code_init.append("}\n");    
    operators_libcall_function[function_head_init] = code_init;
    
    std::string function_head_close = "void closeCompiledExpressionsLib()";
    std::string code_close = " {\n";
    code_close.append("CodeCompiledExpressions::CloseInstance();\n");
    code_close.append("}\n");
    operators_libcall_function[function_head_close] = code_close;
    
  }
  
} // end of namespace CompiledExpressions

/*
7 External call functions

*/
  
#ifdef __cplusplus
extern "C"{
#endif
/*
7.1 Function ~FuncCECGSetCanInitImplCodeStore~

This external call function calls the ~setCanInitialize~-function from 
the ~CECodeGenerator~-Instance, with which a flag is set, which indicates that 
the ~Secondo System~ has been started so far that the ~Code Store~ can
be initialized.

*/
  void
  FuncCECGSetCanInitImplCodeStore() {
    CECodeGenerator::setCanInitialize();
  }
#ifdef __cplusplus
}
#endif

