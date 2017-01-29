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

[10] Implementation file of the CECompiler

2016/2017 H.Brieschke created the new CompiledExpression Algebra


[TOC]


1 Overview 

In this file all classes needed for the ~CECompiler~ are implemented. These are

  * ~CECRuntimeError~
  
  * the different operator tree classes ~CECOpNodeXXX~
  
  * ~CECPtrCodeStoreType~ and ~CECPtrMap~
  
  * ~CEGenerateFunctionStatus~
  
  * the different visitor classes ~CECOpTreeVisitorXXX~
  
  * ~CECompiler~

The ~CECRuntimeError~-class implements ~ERROR~-objects, which are required for the error treatment.

The ~CECOpNodeXXX~-classes implements the nodes from a operator tree, which is created from the
annotated list of the ~Secondo QueryProcessor~.

The Classes ~CECPtrCodeStoreType~ and ~CECPtrMap~ act as data stores for the ~CodeStore~-Objects 
of the return types respectively the map types in the ~Map~-functions.

The ~CEGenerateFunctionStatus~-class implements various states on the progress of the code
generation, as well as other information required when using the generated shared library.

The ~CECOpTreeVisitorXXX~-classes implements the various visitor types, with which the operator tree
is traversed to collect the necessary information for code generation.

The ~CECompiler~class finally implements the concrete compiler.


2 Defines, includes, and constants

*/
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>

#include "AlgebraManager.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "Symbols.h"
#include "StringUtils.h"
#include "Trace.h"
#include "FileSystem.h"

#include "./CECodeGenerator.h"
#include "./CEQueryProcessor.h"
#include "./CECompiler.h"
#include "./CompiledExpressionsAlgebra.h"


extern NestedList* nl;
extern AlgebraManager *am;

using namespace CompiledExpressions;
namespace CompiledExpressions {

/*
3 Class ~CECRuntimeError~

This class implements ~ERROR~-objects, which are required for the error treatment.

*/

  CECRuntimeError::CECRuntimeError(std::string s) {
    errMsg = s;
  }
  
  const char*
  CECRuntimeError::what() const throw() {
    return errMsg.c_str();
  }
  
/*
4 Class ~CEGenerateFunctionStatus~

This class implements various states on the progress of the code generation, as
well as other information required when using the generated shared library.

4.1 The Constructor.

*/
  CEGenerateFunctionStatus::CEGenerateFunctionStatus()
  : generateStatus(CEC_GCE_UNDEF),
    ceCallLibFunctionName(""),
    libFunctionAddress(nl->TheEmptyList()) {
      
    }
  
    
/*
4.2 The Destructor

*/
  CEGenerateFunctionStatus::~CEGenerateFunctionStatus() {
  }
  
/*
4.3 Function ~setCodeGenerationOK~

A ~set~-function that can be used to store whether the code of the compiled query 
was generated correctly or not.

*/
  void
  CEGenerateFunctionStatus::setCodeGenerationOK(bool genCodeOK) {
    if (generateStatus == CEC_GCE_UNDEF) {
      if (genCodeOK) 
        generateStatus = CEC_GCE_GENCODE;
      else
        generateStatus = CEC_GCE_ERROR;
    }
  }
  
  
/*
4.4 Function ~isCodeGenerationOK~

A ~get~-function returned if the code of the compiled query was generated
correctly or not.

*/
  bool
  CEGenerateFunctionStatus::isCodeGenerationOK() {
    return (generateStatus == CEC_GCE_GENCODE);
  }
  
  
/*
4.5 Function ~setLibLoadOK~

A ~set~-function that can be used to store whether the generated shared library is
correctly loaded or not.

*/
  void
  CEGenerateFunctionStatus::setLibLoadOK(bool loadLibOK) {
    if (generateStatus == CEC_GCE_GENCODE) {
      if (loadLibOK) 
        generateStatus = CEC_GCE_LOADLIB;
      else
        generateStatus = CEC_GCE_ERROR;
    }
  }
  
/*
4.6 Function ~isLibLoadOK~

A ~get~-function returned if the generated shared library is correctly loaded or not.

*/
  bool
  CEGenerateFunctionStatus::isLibLoadOK() {
    return (generateStatus == CEC_GCE_LOADLIB);
  }
  
/*
4.7 Function ~setCallLibFunctionName~

A ~set~-function that can be used to store the function name to be called in the
shared library.

*/
  void
  CEGenerateFunctionStatus::setCallLibFunctionName(std::string libFuncName) {
    ceCallLibFunctionName = libFuncName;
  }
  
/*
4.8 Function ~getCallLibFunctionName~

A ~get~-function returned the function name to be called in the shared library.

*/
  std::string
  CEGenerateFunctionStatus::getCallLibFunctionName() {
    return ceCallLibFunctionName;
  }
  
/*
4.9 Function ~loadCELibFunction~

This function calls the ~CEQueryProcessor~ to load the generated shared library and
saved the nested list with the memory address of the function in the shared library.

*/
  void
  CEGenerateFunctionStatus::loadCELibFunction(CEQuery* ptrCEQY,
                                              ListExpr& returnType)
  throw (CECRuntimeError) {    
    if (ceCallLibFunctionName == "")
      throw CECRuntimeError("Functionname is not set.");
    setLibLoadOK(ptrCEQY->getPtrCEQP()
                        ->createLibFunctionAddressNList(libFunctionAddress,     
                                                        returnType,
                                                        ceCallLibFunctionName,
                                                        ptrCEQY));
  }
  
/*
4.10 Function ~getCELibFunctionAddressNList~

A ~get~-function returned a nested list with the memory address of the function in
the shared library.

*/
  ListExpr&
  CEGenerateFunctionStatus::getCELibFunctionAddressNList()
  throw (CECRuntimeError) {
    if (!isLibLoadOK())
      throw CECRuntimeError("Can't find CE-Libary function.");
    return libFunctionAddress;
  }
  

  
/*
5 Class ~CECOpNode~

This class is the base class of all nodes of the ~CECompiler Operator Tree~. 
In this class all functions that are required for all node types are implemented.

5.1 Function ~createCECOpTree~

A static function to create the ~CECompiler Operator Tree~. 

*/
  CECOpNode*
  CECOpNode::createCECOpTree(
    ListExpr& expr,
    CECOpNode* father)
  throw (CECRuntimeError) {
    
    ListExpr symbol = nl->Second(nl->First(expr));
    if (!nl->IsAtom(symbol) ||
        !nl->AtomType(symbol) == SymbolType) {
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
    } else {
      std::string s = nl->SymbolValue(symbol);
      if (s == "operator") {
        return new CECOpNodeOperator(expr, father);
      } else if (s == "applyop") {
        return new CECOpNodeApplyop(expr, father);
      } else if (s == "abstraction") {
        return new CECOpNodeAbstraction(expr, father);
      } else if (s == "object") {
        return new CECOpNodeObject(expr, father);
      } else if (s == "constant") {
        return new CECOpNodeConstant(expr, father);
      } else if (s == "variable") {
        return new CECOpNodeVariable(expr, father);
      } else if (s == "identifier") {
        return new CECOpNodeIdentifier(expr, father);
      } else if (s == "arglist") {
        return new CECOpNodeArglist(expr, father);
      } else if (s == "function") {
        return new CECOpNodeFunction(expr, father);
      } else if (s == "applyabs") {
        return new CECOpNodeApplyabs(expr, father);
      } else if (s == "applyfun") {
        return new CECOpNodeApplyfun(expr, father);
      } else if (s == "counterdef") {
        return new CECOpNodeCounterdef(expr, father);
      } else if (s == "pointer") {
        return new CECOpNodePointer(expr, father);
      } else if (s == "predinfodef") {
        return new CECOpNodePredinfodef(expr, father);
      } else if (s == "memorydef") {
        return new CECOpNodeMemorydef(expr, father);
      } else {
        return new CECOpNode(expr, father, std::string(""));
      }
    }
  }
  
/*
5.2 The Constructor.

*/
  CECOpNode::CECOpNode(ListExpr& list,
                       CECOpNode* fatherNode,
                       std::string nodeType)
  : ceAnnotateList( list ),
    ptrFatherNode( fatherNode ),
    cecGenerateAnnotateList( CEC_GAL_UNDEF ),
    strNodeNumber(""),
    strNodeType(nodeType) {
     
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
    
    setIsCECImplementedCodeGeneration();
    setIsCEAOperator();
    setIsCERootNode();
  }
  
  
/*
5.3 The Destructor.

*/
  CECOpNode::~CECOpNode() {
    std::vector<CECOpNode*>::iterator pos;
    for ( pos = this->sons.begin(); pos != this->sons.end(); pos++ ) {
      delete *pos;
    }
    this->sons.clear();
      
    this->ceAnnotateList = 0;
  }
    
/*
5.4 Function ~createApplyopTree~

This function constructs another tree structure within the tree,
but it contains only ~CECOpNodeApplyop~-nodes.

*/
  void
  CECOpNode::createApplyopTree(CECOpNodeApplyop* fatherNode,
                               CECOpNodeApplyop*& rootNodes) {
    std::vector<CECOpNode*>::iterator pos;
    for ( pos = sons.begin(); pos != sons.end(); ++pos )
      (*pos)->createApplyopTree(fatherNode, rootNodes);
  }
  
/*
5.5 Functions ~getIsCECImplementedCodeGeneration~

A ~get~-function returned ~TRUE~ or ~FALSE~ if the generation of the code is implemented
for the respective node.

*/
  bool
  CECOpNode::getIsCECImplementedCodeGeneration(bool onlyTrue) {
    if (onlyTrue) {
      return (isCECCodeGenerationImplemented == CEC_TV_TRUE);
    } else {
      return (isCECCodeGenerationImplemented == CEC_TV_CANDIDATE_TRUE
            || isCECCodeGenerationImplemented == CEC_TV_TRUE);
    }
  }

/*
And a ~get~-function returned the integer from the enumeration ~CECTruthValues~, which
indicates if the generation of the code is implemented for the respective node.

*/
  int
  CECOpNode::getIsCECImplementedCodeGeneration() {
    return isCECCodeGenerationImplemented;
  }

/*
5.6 Function ~checkIsCEAOperator~

A ~set~-function that checks whether the node is an operator, and if so,
whether it is also implemented in the ~Compiled Expressions Algebra~.

*/
  void 
  CECOpNode::checkIsCEAOperator(bool force) {
    std::vector<CECOpNode*>::iterator pos;
    for ( pos = sons.begin(); pos != sons.end(); ++pos )
      (*pos)->checkIsCEAOperator(force);
  }
  
/*
5.7 Functions ~getIsCEAOperator~

A ~get~-function returned ~TRUE~ or ~FALSE~ if the node is a operator, 
that is also implemented in the ~Compiled Expressions Algebra~.

*/
  bool
  CECOpNode::getIsCEAOperator(bool onlyTrue) {
    if (onlyTrue) {
      return (isCEAOperator == CEC_TV_TRUE);
    } else {
      return (isCEAOperator == CEC_TV_CANDIDATE_TRUE
            || isCEAOperator == CEC_TV_TRUE);
    }
  }
  
/*
And a ~get~-function returned the integer from the enumeration ~CECTruthValues~,
if the node is a operator, that is also implemented in the ~Compiled Expressions Algebra~.

*/
  int
  CECOpNode::getIsCEAOperator() {
    return isCEAOperator;
  }

/*
5.8 Function ~checkIsCERootNode~

A ~set~-function that checks whether the node is the root node from the compiled expression.

*/
  void 
  CECOpNode::checkIsCERootNode(bool force) {
    if (isCERootNode == CEC_TV_UNDEF || force) {
      bool lokalIsCERootNode;
      if (isCECCodeGenerationImplemented == CEC_TV_UNDEF)
        lokalIsCERootNode = true;
      else
        lokalIsCERootNode = getIsCECImplementedCodeGeneration(true);
      std::vector<CECOpNode*>::iterator pos;
      for ( pos = sons.begin(); pos != sons.end(); ++pos ) {
        (*pos)->checkIsCERootNode(force);
        if (lokalIsCERootNode)
          lokalIsCERootNode = lokalIsCERootNode
                           && (*pos)-> getIsCERootNode(false);
      }
      
      if (lokalIsCERootNode)
        isCERootNode = CEC_TV_CANDIDATE_TRUE;
      else
        isCERootNode = CEC_TV_FALSE;
    }
  }
 
/*
5.9 Function ~unsetIsCERootNodeNoCECOpNodeRoot~

A ~set~-function that delete the root node entry for the node.

*/
  void
  CECOpNode::unsetIsCERootNodeNoCECOpNodeRoot() {
    if (ptrFatherNode && isCERootNode == CEC_TV_TRUE)
      isCERootNode = CEC_TV_CANDIDATE_TRUE;
    std::vector<CECOpNode*>::iterator pos;
    for ( pos = sons.begin(); pos != sons.end(); ++pos )
      (*pos)->unsetIsCERootNodeNoCECOpNodeRoot();
  }
  
/*
5.10 Functions ~getIsCERootNode~

A ~get~-function returned ~TRUE~ or ~FALSE~ if the node is the root node from the
compiled expression.

*/
  bool
  CECOpNode::getIsCERootNode(bool onlyTrue) {
    if (onlyTrue) {
      return (isCERootNode == CEC_TV_TRUE);
    } else {
      return (isCERootNode == CEC_TV_CANDIDATE_TRUE
            || isCERootNode == CEC_TV_TRUE);
    }
  }
  
/*
And a ~get~-function returned the integer from the enumeration ~CECTruthValues~,
if the node is the root node from the compiled expression.

*/
  int
  CECOpNode::getIsCERootNode() {
    return isCERootNode;
  }

/*
5.11 Function ~generateCECallLibFunctionName~

This function generates a unique function name, which is used in the shared library
to be generated. The function is overwritten in the subclasses.

*/
  std::string
  CECOpNode::generateCECallLibFunctionName(int idxCERootNode) {
    return "";
  }
  
/*
5.12 Function ~checkCECGenerateAnnotateList~

A ~set~-function that checks to which form the annotated list, associated with the node,
must be converted for the ~Secondo QueryProcessor~.

*/
  void
  CECOpNode::checkCECGenerateAnnotateList() {
    std::vector<CECOpNode*>::iterator pos;
    for ( pos = sons.begin(); pos != sons.end(); ++pos )
      (*pos)->checkCECGenerateAnnotateList();
  }

/*
5.13 Function ~getCECGenerateAnnotateList~

A ~get~-function returned the integer from the enumeration ~CECGenerateAnnotateList~,
to which form the annotated list, associated with the node, must be converted for
the ~Secondo QueryProcessor~.

*/
  int
  CECOpNode::getCECGenerateAnnotateList() {
    return cecGenerateAnnotateList;
  }

  
/*
5.14 Function ~getListReturnType~

A ~get~-function returned the position number in the vector of sons in the father node.

*/
  ListExpr
  CECOpNode::getListReturnType() {
    return nl->Second(ceAnnotateList);
  }
  
  
/*
5.15 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

*/
  void
  CECOpNode::accept(CECOpTreeVisitor& v) {
    std::vector<CECOpNode*>::iterator pos;
    for ( pos = this->sons.begin(); pos != this->sons.end(); pos++ ) {
      (*pos)->accept(v);
    }
  }
    
/*
5.16 Function ~getSignatureVector~

This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

*/
  void
  CECOpNode::getSignatureVector(std::vector<CECGImplSecondoType*>& sVector,
                                bool callRecursiv) {
  }
  
/*
5.17 Function ~getNumSons~

A ~get~-function returned the number of sons.

*/
  unsigned int
  CECOpNode::getNumSons() {
    return sons.size();
  }
  
/*
5.18 Function ~setStrNodeNumber~

A ~set~-function sets the unique node number as string representation.

*/
  void
  CECOpNode::setStrNodeNumber(int nodeNum) {
    std::ostringstream convert;
    convert << nodeNum;
    strNodeNumber = convert.str();
  }
  
/*
5.19 Function ~getPtrFatherNode~

A ~get~-function returned a pointer to the fathernode.

*/
  CECOpNode*
  CECOpNode::getPtrFatherNode() {
    return ptrFatherNode;
  }
  
/*
5.20 Function ~getPtrSonNode~

A ~get~-function returned a pointer to the sonnode at the position ~posSon~.

*/
  CECOpNode*
  CECOpNode::getPtrSonNode(const unsigned int posSon)
  throw (CECRuntimeError) {
    if (posSon < 0 || posSon > sons.size())
      throw CECRuntimeError("posSon is incorrect.");
    return sons.at(posSon);
  }
  
/*
5.21 Function ~getSonsPosition~

A ~get~-function returned the position number in the vector of sons in the father node.

*/
  int 
  CECOpNode::getSonsPosition() {
    if (ptrFatherNode)
      return ptrFatherNode->searchSonsPosition(this);
    else
      return -1;
  }
  
/*
5.22 Function ~isSetNodeNumber~

A ~get~-function returned if the node number is already set.

*/
  bool
  CECOpNode::isSetNodeNumber() {
    return (strNodeNumber.length() > 0);
  }
  
/*
5.23 Function ~getStrNodeKey~

A ~get~-function returned the string with the unique node key.

*/
  std::string
  CECOpNode::getStrNodeKey() {
    return strNodeType + "_" + strNodeNumber;
  }

/*
5.24 Function ~generateCEAnnotateList~

The function converts the annotated list from the node into a form that
the ~Secondo QueryProcessor~ can process.

*/
  void
  CECOpNode::generateCEAnnotateList() {
    std::vector<CECOpNode*>::iterator pos;
    for ( pos = sons.begin(); pos != sons.end(); ++pos )
      (*pos)->generateCEAnnotateList();
  }
  
/*
5.25 Function ~checkNListStructureOK~

This function checks the correct form of the annotated list from the node.

*/
  bool
  CECOpNode::checkNListStructureOK() {  
    if (ceAnnotateList == 0) {
      return false;
    } else {
      return (nl->ListLength( ceAnnotateList ) >= 2)
          && (nl->ListLength( nl->First( ceAnnotateList )) >= 2)
          && (nl->AtomType(nl->Second(nl->First( ceAnnotateList )))
                                            == SymbolType);
    }
  }
  
/*
5.26 Function ~setIsCECImplementedCodeGeneration~

A ~set~-function for the member variable, if the generation of the code is implemented for
the respective node.

*/
  void
  CECOpNode::setIsCECImplementedCodeGeneration() {
    isCECCodeGenerationImplemented = CEC_TV_UNDEF;
  }

/*
5.27 Function ~setIsCEAOperator~

A ~set~-function for the member variable, if the respective node an operator from
the ~Compiled Expressions Algebra~.

*/
  void
  CECOpNode::setIsCEAOperator() {
    isCEAOperator = CEC_TV_UNDEF;
  }
  
/*
5.28 Function ~setIsCERootNode~

A ~set~-function for the member variable, if the respective node a root node
of any compiled expression.

*/
  void
  CECOpNode::setIsCERootNode() {
    isCERootNode = CEC_TV_UNDEF;
  }
  
/*
5.29 Function ~setCECGenerateAnnotateList~

A ~set~-function for the member variable, to which form the annotated list,
associated with the node, must be converted for the ~Secondo QueryProcessor~.

*/
  void
  CECOpNode::setCECGenerateAnnotateList() {
    cecGenerateAnnotateList = CEC_GAL_UNDEF;
  }
  
/*
5.30 Function ~searchSonsPosition~

A ~get~-function returned the position number of the node ~node~ in the vector of sons.

*/
  int
  CECOpNode::searchSonsPosition(CECOpNode* node) {
    bool found = false;
    int returnValue = 0;
    
    while (!found && returnValue < (int)sons.size()) {
      found = (sons[returnValue] == node);
      returnValue++;
    } 
    
    if (!found)
      returnValue = -1;
    
    return returnValue;
  }
  
   
   
   

/*
6 Class ~CECPtrCodeStoreType~

This class implements a data store for the ~CECGImplXXXTypeXXX~-Objects
of the return types from the nodes.

6.1 The Constructor.

*/
  CECPtrCodeStoreType::CECPtrCodeStoreType()
  : ptrCodeStoreType(0),
    ptrSubCodeStoreType(0),
    isSetPtrCodeStoreType(false),
    isStreamType(false) {}
  
/*
6.2 Function ~getPtrCodeStoreType~

A ~get~-function returned a pointer to the ~CECGImplXXXTypeXXX~-Objects
of the return types from the nodes.

*/
  CECGImplSecondoType*
  CECPtrCodeStoreType::getPtrCodeStoreType() {
    return ptrCodeStoreType;
  }
  
/*
6.3 Function ~getPtrSubCodeStoreType~

A ~get~-function returned a pointer to the ~CECGImplXXXTypeXXX~-Objects
of the return subtypes from the nodes.

*/
  CECGImplSecondoType*
  CECPtrCodeStoreType::getPtrSubCodeStoreType() {
    return ptrSubCodeStoreType;
  }
  
/*
6.4 Function ~getIsStreamType~

A ~get~-function returned if the return type of the node is a stream.

*/
  bool
  CECPtrCodeStoreType::getIsStreamType() {
    return isStreamType;
  }
  
/*
6.5 Function ~setPtrCodeStoreType~

A ~set~-function saved the pointer to the ~CECGImplXXXTypeXXX~-Objects
of the return type and return subtype from the node.

*/
  void
  CECPtrCodeStoreType::setPtrCodeStoreType(ListExpr& list) {
    std::string typeName = "";
    std::string typeSubName = "";
    if (nl->IsAtom(nl->Second(list)))
      typeName= nl->ToString(nl->Second(list));
    else {
      typeName= nl->ToString(nl->First(nl->Second(list)));
      if (nl->IsAtom(nl->Second(nl->Second(list))))
        typeSubName= nl->ToString(nl->Second(nl->Second(list)));
      else
        typeSubName= nl->ToString(nl->First(nl->Second(nl->Second(list))));
    }
    if (typeName == "stream") {
      isStreamType = true;
      ptrCodeStoreType = CECodeGenerator::getInstance()
                           ->getPtrSecondoType(typeSubName);
    } else {
      ptrCodeStoreType = CECodeGenerator::getInstance()
                           ->getPtrSecondoType(typeName);    
    }
    isSetPtrCodeStoreType = true;
  }
  
/*
6.6 Function ~setIsCECImplSecondoType~

A ~set~-function saved the integer from the enumeration ~CECTruthValues~, which
indicates if the generation of the code of the return type is implemented for the
respective node.

*/
  void
  CECPtrCodeStoreType::setIsCECImplSecondoType(int& retValue, bool force) {
    if (isSetPtrCodeStoreType
      && (retValue == CECOpNode::CEC_TV_UNDEF || force)) {
      if (ptrCodeStoreType && ptrCodeStoreType->isActivType())
        retValue = CECOpNode::CEC_TV_TRUE;
      else
        retValue = CECOpNode::CEC_TV_FALSE;
    }
  }
  
/*
6.7 Function ~addCSTtoSignatureVector~

This function added the pointer to the ~CECGImplXXXTypeXXX~-Objects of the 
return type to the signature vector ~sVector~.

*/
  void
  CECPtrCodeStoreType::addCSTtoSignatureVector
  (std::vector<CECGImplSecondoType*>& sVector) {
    if (isSetPtrCodeStoreType)
      sVector.push_back(ptrCodeStoreType);
  }
  

  
  
/*
7 Class ~CECPtrMap~

This class implements a data store for the ~CECGImplXXXTypeXXX~-Objects
of the map types in the ~Map~-functions from the node.

7.1 The Constructor.

*/
  CECPtrMap::CECPtrMap()
  : isSetVectorPtrCodeStoreTypes(false) {}
  
/*
7.2 The Destructor.

*/
  CECPtrMap::~CECPtrMap() {
    std::vector<CECGImplSecondoType*>::iterator pos;
    for (pos = ptrCodeStoreTypes.begin();
         pos != ptrCodeStoreTypes.end();
         ++pos)
      (*pos) = 0;
    ptrCodeStoreTypes.clear();
  }
  
/*
7.3 Function ~getVectorPtrCodeStoreTypes~

A ~get~-function returned a vector of pointers to the ~CECGImplXXXTypeXXX~-Objects
of the map types from the nodes.

*/
  std::vector<CECGImplSecondoType*>
  CECPtrMap::getVectorPtrCodeStoreTypes() {
    return ptrCodeStoreTypes;
  }
  
/*
7.4 Function ~setVectorPtrCodeStoreTypes~

A ~set~-function saved the pointers to the ~CECGImplXXXTypeXXX~-Objects
of the map types from the node.

*/
  void
  CECPtrMap::setVectorPtrCodeStoreTypes(ListExpr& list) {
    std::string typeName;
    ListExpr tmpList;

    for (int i = 2; i <= nl->ListLength(list); i++) {
      tmpList = nl->Nth(i, list);
      if (nl->IsAtom(tmpList))
        typeName= nl->ToString(tmpList);
      else
        typeName= nl->ToString(nl->First(tmpList));
      ptrCodeStoreTypes.push_back(CECodeGenerator::getInstance()
                                  ->getPtrSecondoType(typeName));
    }
    isSetVectorPtrCodeStoreTypes = true;
  }
  
/*
7.5 Function ~setIsCECImplSecondoTypes~

A ~set~-function saved the integer from the enumeration ~CECTruthValues~, which
indicates if the generation of the code of the map types are implemented for the
respective node.

*/
  void
  CECPtrMap::setIsCECImplSecondoTypes(int& retValue, bool force) {
    if (isSetVectorPtrCodeStoreTypes 
      && (retValue == CECOpNode::CEC_TV_UNDEF || force)) {
      bool lokalIsCECImplSecondoType = true;
      std::vector<CECGImplSecondoType*>::iterator pos;
      pos = ptrCodeStoreTypes.begin();
      while ( lokalIsCECImplSecondoType
              && pos != ptrCodeStoreTypes.end()) {
        if (!(*pos) || !(*pos)->isActivType())
          lokalIsCECImplSecondoType = false;
        ++pos;
      }
      
      if (lokalIsCECImplSecondoType)
        retValue = CECOpNode::CEC_TV_TRUE;
      else
        retValue = CECOpNode::CEC_TV_FALSE;
    }
  }
  

  
/*
8 Class ~CECOpNodeOperator~

This class is a subclass of ~CECOpNode~. It implements the node that is formed
from the following annotated list:

----        

((<name> operator <opID> <algID>) typeerror)

----

8.1 The Constructor.

*/
  CECOpNodeOperator::CECOpNodeOperator(ListExpr& list, CECOpNode* fatherNode)
  : CECOpNode(list, fatherNode, std::string("operator")) {
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");

    listAlgID = nl->TwoElemList
                    (nl->First(nl->Third(nl->First(ceAnnotateList))),
                     nl->Second(nl->Third(nl->First(ceAnnotateList))));
                    
    listOpFunID = nl->TwoElemList
                      (nl->First(nl->Fourth(nl->First(ceAnnotateList))),
                       nl->Second(nl->Fourth(nl->First(ceAnnotateList))));
    
    ptrCodeStoreOperator = CECodeGenerator::getInstance()
                             ->getPtrSecondoOperator
                               (nl->IntValue(nl->First(listAlgID)),
                               (nl->IntValue(nl->First(listOpFunID)) % 65536));

    setIsCECImplementedCodeGeneration();
    
    setIsCEAOperator();
  }
  
/*
8.2 The Destructor.

*/
  CECOpNodeOperator::~CECOpNodeOperator() {
    listAlgID = 0;
    listOpFunID = 0;
    ptrCodeStoreOperator = 0;
  }
   
/*
8.3 Function ~getPtrCodeStoreOperator~

A ~get~-function returned a pointer to the ~CECGImplXXXOperatorXXX~-Object
from the nodes.

*/
  CECGImplSecondoOperator*
  CECOpNodeOperator::getPtrCodeStoreOperator() {
    return ptrCodeStoreOperator;
  }
  
/*
8.4 Function ~checkIsCEAOperator~

A ~set~-function that checks whether the node is an operator, and if so,
whether it is also implemented in the ~Compiled Expressions Algebra~.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeOperator::checkIsCEAOperator(bool force) {
    CECOpNode::checkIsCEAOperator(force);
    if (isCEAOperator == CEC_TV_UNDEF || force) {
      if (getIsCEAOperator(false)
        && ptrFatherNode
        && ptrFatherNode->getIsCEAOperator(false))
        isCEAOperator = CEC_TV_TRUE;
      else
        isCEAOperator = CEC_TV_FALSE;
    }
      
  }
  
/*
8.5 Function ~generateCECallLibFunctionName~

This function generates a unique function name, which is used in the shared library
to be generated.

This function overrides the function of the base class ~CECOpNode~.

*/
  std::string
  CECOpNodeOperator::generateCECallLibFunctionName(int idxCERootNode) {
    return getOperatorName();
  }
  
/*
8.6 Function ~checkCECGenerateAnnotateList~

A ~set~-function that checks to which form the annotated list, associated with the node,
must be converted for the ~Secondo QueryProcessor~.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeOperator::checkCECGenerateAnnotateList() {
    cecGenerateAnnotateList = ptrFatherNode->getCECGenerateAnnotateList();
  }
  
/*
8.7 Function ~generateCEAnnotateList~

The function converts the annotated list from the node into a form that
the ~Secondo QueryProcessor~ can process.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeOperator::generateCEAnnotateList() {
    switch (cecGenerateAnnotateList) {
      case CEC_GAL_UNDEF:
      case CEC_GAL_ORIG_OP:
        nl->Replace(nl->Third(nl->First(ceAnnotateList)),
                    nl->First(listAlgID));
        nl->Replace(nl->Fourth(nl->First(ceAnnotateList)),
                    nl->First(listOpFunID));
        break;
      case CEC_GAL_CEA_OP:
        nl->Replace(nl->Third(nl->First(ceAnnotateList)),
                    nl->Second(listAlgID));
        nl->Replace(nl->Fourth(nl->First(ceAnnotateList)),
                    nl->Second(listOpFunID));
        break;
      case CEC_GAL_CEA_CQOP:
        {
        nl->Replace(nl->First(nl->First(ceAnnotateList)),
                    nl->Second(listAlgID));
        nl->Replace(nl->Third(nl->First(ceAnnotateList)),
                    nl->Second(listAlgID));
        nl->Replace(nl->Fourth(nl->First(ceAnnotateList)),
                    nl->Second(listOpFunID));
        }
        break;
      default:
        throw CECRuntimeError(
          "In generating the final annotated list an error has occurred.");
    }
  }
  
    
/*
8.8 Function ~checkNListStructureOK~

This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodeOperator::checkNListStructureOK() {  
    return CECOpNode::checkNListStructureOK()
        && (nl->ListLength( ceAnnotateList ) == 2)        
        && (nl->ListLength( nl->First( ceAnnotateList )) == 4)
        && (nl->IsAtom(nl->First(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Second(nl->First( ceAnnotateList ))))
        && (nl->ListLength( nl->Third(nl->First( ceAnnotateList ))) == 2)
        && (nl->IsAtom(nl->First(nl->Third(nl->First( ceAnnotateList )))))
        && (nl->IsAtom(nl->Second(nl->Third(nl->First( ceAnnotateList ))))
            || nl->IsEmpty(nl->Second(nl->Third(nl->First(ceAnnotateList)))))
        && (nl->ListLength( nl->Fourth(nl->First( ceAnnotateList ))) == 2)
        && (nl->IsAtom(nl->First(nl->Fourth(nl->First( ceAnnotateList )))))
        && (nl->IsAtom(nl->Second(nl->Fourth(nl->First( ceAnnotateList ))))
            || nl->IsEmpty(nl->Second(nl->Fourth(nl->First(ceAnnotateList)))))
        && (nl->IsAtom(nl->Second( ceAnnotateList )));
  }
  
/*
8.9 Function ~setIsCECImplementedCodeGeneration~

A ~set~-function that checks to which form the annotated list, associated with the node,
must be converted for the ~Secondo QueryProcessor~.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeOperator::setIsCECImplementedCodeGeneration() {
    if (ptrCodeStoreOperator && ptrCodeStoreOperator->isActivOperator())
      isCECCodeGenerationImplemented = CEC_TV_TRUE;
    else
      isCECCodeGenerationImplemented = CEC_TV_FALSE;
  }
  
/*
8.10 Function ~setIsCEAOperator~

A ~set~-function for the member variable, if the respective node an operator from
the ~Compiled Expressions Algebra~.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeOperator::setIsCEAOperator() {
    if (!nl->IsEmpty(nl->Second(listAlgID))
      && !nl->IsEmpty(nl->Second(listOpFunID))) {
      isCEAOperator = CEC_TV_CANDIDATE_TRUE;
    } else {
      isCEAOperator = CEC_TV_CANDIDATE_FALSE;
    }

  }
  
/*
8.11 Function ~getOperatorName~

This function generate and returned a unique name of the operator.

*/
  std::string 
  CECOpNodeOperator::getOperatorName() {
    return nl->SymbolValue(nl->First(nl->First(ceAnnotateList)));
  }
  
/*
8.12 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeOperator::accept(CECOpTreeVisitor& v) {
    v.visit(*this);
    CECOpNode::accept(v);
  }
    
/*
8.13 Function ~getAlgId~

A ~get~-function returned a string with the algebra ID of the operator.

*/
  std::string
  CECOpNodeOperator::getAlgId() {
    return nl->ToString(nl->First(listAlgID));
  }
  
/*
8.14 Function ~getOpId~

A ~get~-function returned a string with the operator ID of the operator.

*/
  std::string
  CECOpNodeOperator::getOpId() {
    int opId = (nl->IntValue(nl->First(listOpFunID)) % 65536);
    std::ostringstream tmpOpId;
    tmpOpId << opId;
    return tmpOpId.str();
  }

/*
8.15 Function ~getSignatureVector~

A ~get~-function returned a vector with pointers of ~CECGImplSecondoTypeXXX~-objects
so that the vector contains the complete return types as the signature of the operator.

*/
  std::vector<CECGImplSecondoType*>
  CECOpNodeOperator::getSignatureVector() {
    std::vector<CECGImplSecondoType*> sVector;
    CECOpNodeApplyop* ptrLocalFather 
                      = static_cast<CECOpNodeApplyop*>(ptrFatherNode);
    ptrLocalFather->getSignatureVector(sVector, true);
    return sVector;
  }
  

  
/*
9 Class ~CECOpNodeApplyop~

This class is a subclass of ~CECOpNode~ and ~CECPtrCodeStoreType~. It implements
the node that is formed from the following annotated list:

----        
              ((none applyop (ann(op) ann(arg1) ... ann(argn)))
                <resulttype>
                <opFunId>)
                
----

9.1 The Constructor.

*/
  CECOpNodeApplyop::CECOpNodeApplyop(ListExpr& list, CECOpNode* fatherNode)
  : CECOpNode(list, fatherNode, std::string("applyop")),
    CECPtrCodeStoreType() {
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
   
    listOpFunID = nl->TwoElemList(nl->First(nl->Third(ceAnnotateList)),
                              nl->Second(nl->Third(ceAnnotateList)));
    
    setPtrCodeStoreType(ceAnnotateList);
    setIsCECImplSecondoType(isCECCodeGenerationImplemented, true);

    setIsCEAOperator();

    ptrApplyopFatherNode = 0;
    ptrCEC = 0;
    
    ptrCEGenFuncStat = new CEGenerateFunctionStatus();
        
    int numLengthOp = nl->ExprLength(nl->Third(nl->First(ceAnnotateList)));

    ListExpr tmpList;
    for (int i = 1; i <= numLengthOp; i++) {
      tmpList = nl->Nth(i, nl->Third(nl->First(ceAnnotateList)));
      sons.push_back(CECOpNode::createCECOpTree(tmpList, this));
    }
    
  }
  
/*
9.2 The Destructor.

*/
  CECOpNodeApplyop::~CECOpNodeApplyop() {
    for (unsigned int i = 0; i < applyopSons.size(); i++)
      applyopSons[i] = 0;
    applyopSons.clear();
    
    listOpFunID = 0;
    
    if (ptrCEGenFuncStat)
      delete ptrCEGenFuncStat;
    ptrCEGenFuncStat = 0;
  }
    
/*
9.3 Function ~createApplyopTree~

This function constructs another tree structure within the tree,
but it contains only ~CECOpNodeApplyop~-nodes.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeApplyop::createApplyopTree(CECOpNodeApplyop* fatherNode,
                                      CECOpNodeApplyop*& rootNodes) {
    ptrApplyopFatherNode = fatherNode;
    if (ptrApplyopFatherNode)
      ptrApplyopFatherNode->addApplyopSon(*this);
    else {
      if (rootNodes == 0)
        rootNodes = this;
    }
    CECOpNode::createApplyopTree(this, rootNodes);
  }
  
/*
9.4 Function ~checkIsCEAOperator~

A ~set~-function that checks whether the node is an operator, and if so,
whether it is also implemented in the ~Compiled Expressions Algebra~.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeApplyop::checkIsCEAOperator(bool force) {
    CECOpNode::checkIsCEAOperator(force);
    if (isCEAOperator == CEC_TV_UNDEF || force) {
      if (getIsCEAOperator(false) && sons[0]->getIsCEAOperator(false))
        isCEAOperator = CEC_TV_TRUE;
      else
        isCEAOperator = CEC_TV_FALSE;
    }
  }

/*
9.5 Function ~checkIsCERootNode~

A ~set~-function that checks whether the node is the root node from the compiled expression.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeApplyop::checkIsCERootNode(bool force) {
    CECOpNode::checkIsCERootNode(force);
    if (getIsCERootNode(false) && !ptrFatherNode)
        isCERootNode = CEC_TV_TRUE;
  }
  
/*
9.6 Function ~getInstCEGenerateFunctionStatus~

A ~get~-function returned a pointer to the ~CEGenerateFunctionStatus~-object of the node.

*/
  CEGenerateFunctionStatus*
  CECOpNodeApplyop::getInstCEGenerateFunctionStatus() {
    return ptrCEGenFuncStat;
  }

/*
9.7 Function ~setPtrCECompiler~

A ~set~-function to save a pointer to the ~CECompiler~-object.

*/
  void
  CECOpNodeApplyop::setPtrCECompiler(CECompiler* cec) {
    ptrCEC = cec;
  }

/*
9.8 Function ~generateCECallLibFunctionName~

This function generates a unique function name, which is used in the shared library
to be generated.

This function overrides the function of the base class ~CECOpNode~.

*/
  std::string
  CECOpNodeApplyop::generateCECallLibFunctionName(int idxCERootNode) {
    return sons[0]->generateCECallLibFunctionName(idxCERootNode)
            + "Function_"
            + stringutils::int2str(idxCERootNode);
  }
  
/*
9.9 Function ~checkCECGenerateAnnotateList~

A ~set~-function that checks to which form the annotated list, associated with the node,
must be converted for the ~Secondo QueryProcessor~.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeApplyop::checkCECGenerateAnnotateList() {
  //For extensions of the CompiledExpressions-Algebra must be
  //distinguished here other cases.
    if (getIsCERootNode(true) && ptrCEGenFuncStat->isLibLoadOK()) {
      //entire query
      cecGenerateAnnotateList = CEC_GAL_CEA_CQOP;
    } else if (getIsCEAOperator(true) && sons[0]->getIsCEAOperator(true)) {
      if ((static_cast<CECOpNodeOperator*>(sons[0]))->getOperatorName()
          == "filter") {
        std::vector<CECOpNode*>::iterator pos;
        for ( pos = sons.begin() + 1; pos != sons.end(); ++pos ) {
          if (CECOpNodeAbstraction* tmpAbstraction
                               = dynamic_cast<CECOpNodeAbstraction*>(*pos)) {
            if (tmpAbstraction->getIsCERootNode(true)
              && (tmpAbstraction->getInstCEGenerateFunctionStatus())
                                  ->isLibLoadOK()) {
              cecGenerateAnnotateList = CEC_GAL_CEA_OP;
            } else {
              //fall back to standard processing
              cecGenerateAnnotateList = CEC_GAL_ORIG_OP;
            }
          }
        }
        
      } else if ((static_cast<CECOpNodeOperator*>(sons[0]))->getOperatorName()
                  == "extend") {
        bool localFoundLoadLib = false;
        
        std::vector<CECOpNode*>::iterator pos = sons.begin() + 1;
        while (!localFoundLoadLib && pos != sons.end()) {
          if (CECOpNodeArglist* tmpArglist
                                = dynamic_cast<CECOpNodeArglist*>(*pos))
            localFoundLoadLib = localFoundLoadLib
                                || tmpArglist->searchCELibFuncLoadOK();
          ++pos;
        }

        if (localFoundLoadLib) {
          cecGenerateAnnotateList = CEC_GAL_CEA_OP;
        } else {
          //fall back to standard processing
          cecGenerateAnnotateList = CEC_GAL_ORIG_OP;
        }
      }
      
    } else {
      //everything else too fall back to standard processing
      cecGenerateAnnotateList = CEC_GAL_ORIG_OP;
    }

    CECOpNode::checkCECGenerateAnnotateList();
  }

/*
9.10 Function ~generateCEAnnotateList~

The function converts the annotated list from the node into a form that
the ~Secondo QueryProcessor~ can process.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeApplyop::generateCEAnnotateList() {
    switch (cecGenerateAnnotateList) {
      case CEC_GAL_UNDEF:
      case CEC_GAL_ORIG_OP:
        nl->Replace(nl->Third(ceAnnotateList), nl->First(listOpFunID));
        CECOpNode::generateCEAnnotateList();
        break;
      case CEC_GAL_CEA_OP:
      {
        nl->Replace(nl->Third(ceAnnotateList), nl->Second(listOpFunID));
        sons[0]->generateCEAnnotateList();
        
        int idx = 2;
        std::vector<CECOpNode*>::iterator pos;
        for ( pos = sons.begin() + 1; pos != sons.end(); ++pos ) {
          if (CECOpNodeAbstraction* tmpAbstraction
                                 = dynamic_cast<CECOpNodeAbstraction*>(*pos)) {
            if (tmpAbstraction->getIsCERootNode(true)
              && (tmpAbstraction->getInstCEGenerateFunctionStatus())
                                  ->isLibLoadOK()) {
              nl->Replace(nl->Nth(idx,
                                  nl->Third(nl->First(ceAnnotateList))),
                          (tmpAbstraction->getInstCEGenerateFunctionStatus())
                                           ->getCELibFunctionAddressNList());
            } else {
              tmpAbstraction->generateCEAnnotateList();
            }
          } else {
            (*pos)->generateCEAnnotateList();
          }
          idx++;
        }
      }
        break;
      case CEC_GAL_CEA_CQOP:
      {
         std::string opName = "executeQuery";
        
         ListExpr tmpListOp
                  = nl->Cons(
                      nl->TwoElemList(
                        nl->FourElemList(
                          nl->SymbolAtom( opName ),
                          nl->SymbolAtom( "operator" ),
                          nl->IntAtom(CompiledExpressionsAlgebra::getInstance()
                                        ->getThisAlgebraID()),
                          nl->IntAtom(CompiledExpressionsAlgebra::getInstance()
                                        ->getOperatorID(opName))),
                        nl->SymbolAtom( "typeerror" )),
                      nl->OneElemList(ptrCEGenFuncStat
                                        ->getCELibFunctionAddressNList()));
        
        ListExpr tmpListApplyop
                 = nl->ThreeElemList(
                     nl->ThreeElemList(
                       nl->SymbolAtom("none"),
                       nl->SymbolAtom("applyop"),
                       tmpListOp),
                     nl->Second(ceAnnotateList),
                     nl->IntAtom(CompiledExpressionsAlgebra::getInstance()
                                   ->getOperatorID(opName)));
        
        nl->Replace(ceAnnotateList, tmpListApplyop);
      }
        break;
      default:
        throw CECRuntimeError
              ("In generating final annotated list an error has occurred.");
    }
  }

/*
9.11 Function ~checkNListStructureOK~

This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodeApplyop::checkNListStructureOK() {  
    return CECOpNode::checkNListStructureOK()
        && (nl->ListLength( ceAnnotateList ) == 3)
        && (nl->ListLength( nl->First( ceAnnotateList )) == 3)
        && (nl->IsAtom(nl->First(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Second(nl->First( ceAnnotateList ))))
        && (nl->ListLength(nl->Third(nl->First( ceAnnotateList ))) > 1)
        && (!nl->IsEmpty(nl->Second( ceAnnotateList )))
        && (nl->ListLength(nl->Third( ceAnnotateList )) == 2)
        && (nl->IsAtom(nl->First(nl->Third( ceAnnotateList ))))
        && (nl->IsAtom(nl->Second(nl->Third( ceAnnotateList )))
            || nl->IsEmpty(nl->Second(nl->Third( ceAnnotateList ))));
  }
  
/*
9.12 Function ~setIsCEAOperator~

A ~set~-function for the member variable, if the respective node an operator from
the ~Compiled Expressions Algebra~.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeApplyop::setIsCEAOperator() {
    if (!nl->IsEmpty(nl->Second(listOpFunID))) {
      isCEAOperator = CEC_TV_CANDIDATE_TRUE;
    } else {
      isCEAOperator = CEC_TV_CANDIDATE_FALSE;
    }
  }
  
/*
9.13 Function ~getOperatorName~

This function generate and returned a unique name of the operator.

*/
  std::string
  CECOpNodeApplyop::getOperatorName() {
    CECOpNodeOperator* tmpOpNodeOperator
                       = static_cast<CECOpNodeOperator*>(sons[0]);
    return tmpOpNodeOperator->getOperatorName();
  }
  

/*
9.14 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeApplyop::accept(CECOpTreeVisitor& v) {
    v.visit(*this);
    CECOpNode::accept(v);
  }
    
/*
9.15 Function ~getSignatureVector~

This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeApplyop::getSignatureVector
                    (std::vector<CECGImplSecondoType*>& sVector,
                     bool callRecursiv) {
                      
    addCSTtoSignatureVector(sVector);
    if (callRecursiv) {
      std::vector<CECOpNode*>::iterator pos;
      for ( pos = sons.begin() + 1; pos != sons.end(); pos++ ) {
        (*pos)->getSignatureVector(sVector, false);
      }
    }
  }
  
/*
9.16 Function ~addApplyopSon~

This function added the node in the ~Applyop Node Tree~.

*/
  void
  CECOpNodeApplyop::addApplyopSon(CECOpNodeApplyop& son) {
    bool isInsertSon = true;
    std::vector<CECOpNodeApplyop*>::iterator pos;
    pos = this->applyopSons.begin();
    while ( isInsertSon && pos != this->applyopSons.end() ) {
      if ((*pos) == (&son) )
        isInsertSon = false;
      pos++;
    }
      
    if (isInsertSon)
      this->applyopSons.push_back(&son);
  }
  
  
  
  
  
/*
10 Class ~CECOpNodeAbstraction~

This class is a subclass of ~CECOpNode~ and ~CECPtrMap~. It implements
the node that is formed from the following annotated list:

----        
              ((none abstraction Annotate(expr) <functionno>) <type>)
                
----

10.1 The Constructor.

*/    
  CECOpNodeAbstraction::CECOpNodeAbstraction(ListExpr& list,
                                             CECOpNode* fatherNode)
  : CECOpNode(list, fatherNode, std::string("abstraction")),
    CECPtrMap() {
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
    
    ListExpr map = nl->Second(ceAnnotateList);
    setVectorPtrCodeStoreTypes(map);
    setIsCECImplSecondoTypes(isCECCodeGenerationImplemented, true);

    ptrCEGenFuncStat = new CEGenerateFunctionStatus();

    ListExpr tmpList;
    int countArgument = nl->ListLength(nl->Second(ceAnnotateList)) - 2;    
    if (countArgument == 1) {
      tmpList = nl->Third(nl->First(ceAnnotateList));
      sons.push_back(CECOpNode::createCECOpTree(tmpList, this));
    } else {
      for (int i = 1; i < countArgument; i++) {
        tmpList = nl->Nth(i, nl->Third(nl->First(ceAnnotateList)));
        sons.push_back(CECOpNode::createCECOpTree(tmpList, this));
      }
    }
  }
  
/*
10.2 The Destructor.

*/
  CECOpNodeAbstraction::~CECOpNodeAbstraction() {
    if (ptrCEGenFuncStat)
      delete ptrCEGenFuncStat;
    ptrCEGenFuncStat = 0;
  }
  
/*
10.3 Function ~checkIsCERootNode~

A ~set~-function that checks whether the node is the root node from the compiled expression.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeAbstraction::checkIsCERootNode(bool force) {
    CECOpNode::checkIsCERootNode(force);
    if (getIsCERootNode(false)
      && ptrFatherNode && ptrFatherNode->getIsCEAOperator(false))
        isCERootNode = CEC_TV_TRUE;
  }
  
/*
10.4 Function ~generateCECallLibFunctionName~

This function generates a unique function name, which is used in the shared library
to be generated.

This function overrides the function of the base class ~CECOpNode~.

*/
  std::string
  CECOpNodeAbstraction::generateCECallLibFunctionName(int idxCERootNode) {
    return ptrFatherNode->generateCECallLibFunctionName(idxCERootNode)
          + "_"
          + stringutils::int2str(getSonsPosition());
  }
  
/*
10.5 Function ~getInstCEGenerateFunctionStatus~

A ~get~-function returned a pointer to the ~CEGenerateFunctionStatus~-object of the node.

*/
  CEGenerateFunctionStatus*
  CECOpNodeAbstraction::getInstCEGenerateFunctionStatus() {
    return ptrCEGenFuncStat;
  }

/*
10.6 Function ~deleteCEAnnotateList~

This function replaces the annotated list of the node with an empty list.   

*/
  void
  CECOpNodeAbstraction::deleteCEAnnotateList() {
    if (getIsCERootNode(true))
      ceAnnotateList = nl->Empty();
  }
  
/*
10.7 Function ~checkNListStructureOK~

This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodeAbstraction::checkNListStructureOK() {  
    return CECOpNode::checkNListStructureOK()
        && (nl->ListLength( ceAnnotateList ) == 2)
        && (nl->ListLength( nl->First( ceAnnotateList )) == 4)
        && (nl->IsAtom(nl->First(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Second(nl->First( ceAnnotateList ))))
        && (!nl->IsEmpty(nl->Third(nl->First( ceAnnotateList ))))
        && (!nl->IsEmpty(nl->Fourth(nl->First( ceAnnotateList ))))
        && (nl->ListLength( nl->Second( ceAnnotateList )) >= 3)
        && (nl->AtomType(nl->First(nl->Second(ceAnnotateList))) == SymbolType);
  }

/*
10.8 Function ~getOperatorName~

This function generate and returned a unique name of the operator.

*/
  std::string 
  CECOpNodeAbstraction::getOperatorName() {
    CECOpNodeApplyop* tmpOpNodeApplyop
                      = static_cast<CECOpNodeApplyop*>(ptrFatherNode);
    return tmpOpNodeApplyop->getOperatorName();
  }
  
/*
10.9 Function ~getListReturnType~

A ~get~-function returned nested list with the return type of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  ListExpr
  CECOpNodeAbstraction::getListReturnType() {
    return nl->Third(nl->Second(ceAnnotateList));
  }
  
/*
10.10 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeAbstraction::accept(CECOpTreeVisitor& v) {
    v.visit(*this);
    CECOpNode::accept(v);
  }

/*
10.11 Function ~getStrFunctionNo~

This function returned a string with the function nuber of the abstraction function.

*/
  std::string 
  CECOpNodeAbstraction::getStrFunctionNo() {
    return nl->ToString(nl->Fourth(nl->First(ceAnnotateList)));
  }
  
  
/*
11 Class ~CECOpNodeObject~

This class is a subclass of ~CECOpNode~ and ~CECPtrCodeStoreType~. It implements
the node that is formed from the following annotated list:

----        
              ((<object name> object <index>) <type>)
                
----

11.1 The Constructor.

*/
  CECOpNodeObject::CECOpNodeObject(ListExpr& list,
                                   CECOpNode* fatherNode)
  : CECOpNode(list, fatherNode, std::string("object")),
    CECPtrCodeStoreType() {
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
    
    setPtrCodeStoreType(ceAnnotateList);
    setIsCECImplSecondoType(isCECCodeGenerationImplemented, true);
  }

/*
11.2 The Destructor.

*/
  CECOpNodeObject::~CECOpNodeObject() {}

/*
11.3 Function ~checkNListStructureOK~

This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodeObject::checkNListStructureOK() {
    return CECOpNode::checkNListStructureOK()
        && (nl->ListLength( ceAnnotateList ) == 2)
        && (nl->ListLength( nl->First( ceAnnotateList )) == 3)
        && (nl->IsAtom(nl->First(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Second(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Third(nl->First( ceAnnotateList ))))
        && (!nl->IsEmpty(nl->Second( ceAnnotateList )));
  }

/*
11.4 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeObject::accept(CECOpTreeVisitor& v) {
    v.visit(*this);
    CECOpNode::accept(v);
  }

/*
11.5 Function ~getQPValueIdx~

This function returned a string with the index of the represented object in the array of values
from the ~Secondo QueryProcessor~.

*/
  std::string
  CECOpNodeObject::getQPValueIdx() {
    return nl->ToString(nl->Third(nl->First(ceAnnotateList)));
  }

/*
11.6 Function ~getSignatureVector~

This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeObject::getSignatureVector
                   (std::vector<CECGImplSecondoType*>& sVector,
                    bool callRecursiv) {
                     
    addCSTtoSignatureVector(sVector);
    if (callRecursiv) {
      std::vector<CECOpNode*>::iterator pos;
      for ( pos = sons.begin(); pos != sons.end(); pos++ ) {
        (*pos)->getSignatureVector(sVector, false);
      }
    }
  }

  
/*
12 Class ~CECOpNodeConstant~

This class is a subclass of ~CECOpNode~ and ~CECPtrCodeStoreType~. It implements
the node that is formed from the following annotated list:

----        
              ((<value> constant <index>) <type>)
                
----

12.1 The Constructor.

*/
  CECOpNodeConstant::CECOpNodeConstant(ListExpr& list,
                                       CECOpNode* fatherNode)
  : CECOpNode(list, fatherNode, std::string("constant")),
    CECPtrCodeStoreType() {
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
    
    setPtrCodeStoreType(ceAnnotateList);
    setIsCECImplSecondoType(isCECCodeGenerationImplemented, true);
  }

/*
12.2 The Destructor

*/
  CECOpNodeConstant::~CECOpNodeConstant() {}

/*
12.3 Function ~checkNListStructureOK~

This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodeConstant::checkNListStructureOK() {
    return CECOpNode::checkNListStructureOK()
        && (nl->ListLength( ceAnnotateList ) == 2)
        && (nl->ListLength( nl->First( ceAnnotateList )) == 3)
        && (!nl->IsEmpty(nl->First(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Second(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Third(nl->First( ceAnnotateList ))))
        && (!nl->IsEmpty(nl->Second( ceAnnotateList )));
  }

/*
12.4 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeConstant::accept(CECOpTreeVisitor& v) {
    v.visit(*this);
    CECOpNode::accept(v);
  }
  
/*
12.4 Function ~getQPValueIdx~

This function returned a string with the index of the represented object in the array of values
from the ~Secondo QueryProcessor~.

*/
  std::string
  CECOpNodeConstant::getQPValueIdx() {
    return nl->ToString(nl->Third(nl->First(ceAnnotateList)));
  }

/*
12.5 Function ~getSignatureVector~

This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeConstant::getSignatureVector
                     (std::vector<CECGImplSecondoType*>& sVector,
                      bool callRecursiv) {
                       
    addCSTtoSignatureVector(sVector);
    if (callRecursiv) {
      std::vector<CECOpNode*>::iterator pos;
      for ( pos = sons.begin(); pos != sons.end(); pos++ ) {
        (*pos)->getSignatureVector(sVector, false);
      }
    }
  }
  


/*
13 Class ~CECOpNodeVariable~

This class is a subclass of ~CECOpNode~ and ~CECPtrCodeStoreType~. It implements
the node that is formed from the following annotated list:

----        
              ((<var name> variable <position> <functionno>) <type>)
                
----

13.1 The Constructor.

*/
  CECOpNodeVariable::CECOpNodeVariable(ListExpr& list,
                                       CECOpNode* fatherNode)
  : CECOpNode(list, fatherNode, std::string("variable")),
    CECPtrCodeStoreType() {
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
    
    setPtrCodeStoreType(ceAnnotateList);
    setIsCECImplSecondoType(isCECCodeGenerationImplemented, true);
  }

/*
13.2 The Destructor.

*/
  CECOpNodeVariable::~CECOpNodeVariable() {}

/*
13.3 Function ~checkNListStructureOK~

This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodeVariable::checkNListStructureOK() {
    return CECOpNode::checkNListStructureOK()
        && (nl->ListLength( ceAnnotateList ) == 2)        
        && (nl->ListLength( nl->First( ceAnnotateList )) == 4)
        && (nl->IsAtom(nl->First(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Second(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Third(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Fourth(nl->First( ceAnnotateList ))))
        && (!nl->IsEmpty(nl->Second( ceAnnotateList )));
  }

/*
13.4 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeVariable::accept(CECOpTreeVisitor& v) {
    v.visit(*this);
    CECOpNode::accept(v);
  }

/*
13.5 Function ~getSignatureVector~

This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeVariable::getSignatureVector
                     (std::vector<CECGImplSecondoType*>& sVector,
                      bool callRecursiv) {
                       
    addCSTtoSignatureVector(sVector);
    if (callRecursiv) {
      std::vector<CECOpNode*>::iterator pos;
      for ( pos = sons.begin(); pos != sons.end(); pos++ ) {
        (*pos)->getSignatureVector(sVector, false);
      }
    }
  }

/*
13.6 Function ~isStreamElem~

This function returned if the represented variable is a stream.

*/
  bool
  CECOpNodeVariable::isStreamElem() {
    std::size_t found = nl->ToString(nl->First(nl->First(ceAnnotateList)))
                          .find("streamelem");
    return (found != std::string::npos);
  }
  
/*
13.7 Function ~isTupleElem~

This function returned if the represented variable is a tuple.

*/
  bool
  CECOpNodeVariable::isTupleElem() {
    std::size_t found = nl->ToString(nl->First(nl->First(ceAnnotateList)))
                          .find("tuple");
    return (found != std::string::npos);
  }
  
/*
13.8 Function ~getStrVarPosition~

This function returned a string with the position of the represented variable in the array of values
from the ~Secondo QueryProcessor~.

*/
  std::string 
  CECOpNodeVariable::getStrVarPosition() {
    std::string retValue = "";
    retValue.append(nl->ToString(nl->Fourth(nl->First(ceAnnotateList))));
    retValue.append("_");
    retValue.append(nl->ToString(nl->Third(nl->First(ceAnnotateList))));
    return retValue;
  }
  

  
  
/*
14 Class ~CECOpNodeIdentifier~

This class is a subclass of ~CECOpNode~. It implements
the node that is formed from the following annotated list:

----        
              ((<ident> identifier) <ident>)
                
----

14.1 The Constructor.

*/
  CECOpNodeIdentifier::CECOpNodeIdentifier(ListExpr& list,
                                           CECOpNode* fatherNode)
  : CECOpNode(list, fatherNode, std::string("identifier")) {
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
  }

/*
14.2 The Destructor.

*/
  CECOpNodeIdentifier::~CECOpNodeIdentifier() {}

/*
14.3 Function ~checkNListStructureOK~

This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodeIdentifier::checkNListStructureOK() {
    return CECOpNode::checkNListStructureOK()
        && (nl->ListLength( ceAnnotateList ) == 2)
        && (nl->ListLength( nl->First( ceAnnotateList )) == 2)
        && (nl->IsAtom(nl->First(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Second(nl->First( ceAnnotateList ))))
        && (!nl->IsEmpty(nl->Second( ceAnnotateList )));
  }

/*
14.4 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeIdentifier::accept(CECOpTreeVisitor& v) {
    v.visit(*this);
    CECOpNode::accept(v);
  }

  
/*
15 Class ~CECOpNodeArglist~

This class is a subclass of ~CECOpNode~. It implements
the node that is formed from the following annotated list:

----        
              ((none arglist (ann(t1) ann(t2) ... ann(tn)))
                                        (type(t1) type(t2) ... type(tn)))
                
----

15.1 The Constructor.

*/
  CECOpNodeArglist::CECOpNodeArglist(ListExpr& list,
                                     CECOpNode* fatherNode)
  : CECOpNode(list, fatherNode, std::string("arglist")) {
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
    
    int numArguments = nl->ExprLength(nl->Third(nl->First(ceAnnotateList)));

    ListExpr tmpList;
    for (int i = 1; i <= numArguments; i++) {
      tmpList = nl->Nth(i, nl->Third(nl->First(ceAnnotateList)));
      sons.push_back(CECOpNode::createCECOpTree(tmpList, this));
    }
  }

/*
15.2 The Destructor.

*/
  CECOpNodeArglist::~CECOpNodeArglist() {}

/*
15.3 Function ~checkNListStructureOK~

This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodeArglist::checkNListStructureOK() {
    return CECOpNode::checkNListStructureOK()
        && (nl->ListLength( ceAnnotateList ) == 2)
        && (nl->ListLength( nl->First( ceAnnotateList )) == 3)
        && (nl->IsAtom(nl->First(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Second(nl->First( ceAnnotateList ))));
  }

/*
15.4 Function ~getIsCEAOperator~

A ~get~-function returned ~TRUE~ or ~FALSE~ if the node is a operator, 
that is also implemented in the ~Compiled Expressions Algebra~.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodeArglist::getIsCEAOperator(bool onlyTrue) {
    if (ptrFatherNode)
      return ptrFatherNode->getIsCEAOperator(onlyTrue);
    else
      return false;
  }
  
/*
15.5 Function ~generateCECallLibFunctionName~

This function generates a unique function name, which is used in the shared library
to be generated.

This function overrides the function of the base class ~CECOpNode~.

*/
  std::string
  CECOpNodeArglist::generateCECallLibFunctionName(int idxCERootNode) {
    return ptrFatherNode->generateCECallLibFunctionName(idxCERootNode)
            + "_"
            + stringutils::int2str(getSonsPosition());
  }
  
/*
15.6 Function ~checkCECGenerateAnnotateList~

A ~set~-function that checks to which form the annotated list, associated with the node,
must be converted for the ~Secondo QueryProcessor~.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeArglist::checkCECGenerateAnnotateList() {
    cecGenerateAnnotateList = ptrFatherNode->getCECGenerateAnnotateList();
    CECOpNode::checkCECGenerateAnnotateList();
  }
  
/*
15.7 Function ~generateCEAnnotateList~

The function converts the annotated list from the node into a form that
the ~Secondo QueryProcessor~ can process.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeArglist::generateCEAnnotateList() {
    switch (cecGenerateAnnotateList) {
      case CEC_GAL_UNDEF:
      case CEC_GAL_ORIG_OP:
      case CEC_GAL_CEA_CQOP:
        CECOpNode::generateCEAnnotateList();
        break;
      case CEC_GAL_CEA_OP:
      {
        int idx = 1;
        std::vector<CECOpNode*>::iterator pos;
        for ( pos = sons.begin(); pos != sons.end(); ++pos ) {
          if (CECOpNodeAbstraction* tmpAbstraction
                                 = dynamic_cast<CECOpNodeAbstraction*>(*pos)) {
            if (tmpAbstraction->getIsCERootNode(true)
              && (tmpAbstraction->getInstCEGenerateFunctionStatus())
                  ->isLibLoadOK()) {
              nl->Replace(
                nl->Nth(idx, nl->Third(nl->First(ceAnnotateList))),
                (tmpAbstraction->getInstCEGenerateFunctionStatus())
                                 ->getCELibFunctionAddressNList());
            } else {
              tmpAbstraction->generateCEAnnotateList();
            }
          } else {
            (*pos)->generateCEAnnotateList();
          }
          idx++;
          
        }
      }
        break;
      default:
        throw CECRuntimeError
              ("In generating final annotated list an error has occurred.");
    }
  }

/*
15.8 Function ~searchCELibFuncLoadOK~

This function returned if the function in the generated shared library in that this
node inserted are correctly loaded.

*/
  bool
  CECOpNodeArglist::searchCELibFuncLoadOK() {
    bool localFound = false;
    
    std::vector<CECOpNode*>::iterator pos = sons.begin();
    while (!localFound && pos != sons.end()) {
      if (CECOpNodeArglist* tmpArglist = dynamic_cast<CECOpNodeArglist*>(*pos))
        localFound = localFound || tmpArglist->searchCELibFuncLoadOK();
      else if (CECOpNodeAbstraction* tmpAbstraction
                                 = dynamic_cast<CECOpNodeAbstraction*>(*pos)) {
        if (tmpAbstraction->getIsCERootNode(true))
          localFound = localFound
                       || (tmpAbstraction->getInstCEGenerateFunctionStatus())
                                             ->isLibLoadOK();
      }
      ++pos;
    }
    
    return localFound;
  }

/*
15.9 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeArglist::accept(CECOpTreeVisitor& v) {
    v.visit(*this);
    CECOpNode::accept(v);
  }

  

  
/*
16 Class ~CECOpNodeFunction~

This class is a subclass of ~CECOpNode~ and ~CECPtrCodeStoreType~. It implements
the node that is formed from the following annotated list:

----        
              ((none function <functionlist>) <resulttype>)
                
----

16.1 The Constructor.

*/
  CECOpNodeFunction::CECOpNodeFunction(ListExpr& list,
                                       CECOpNode* fatherNode)
  : CECOpNode(list, fatherNode, std::string("function")),
    CECPtrCodeStoreType() {
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
  }

/*
16.1 The Destructor

*/
  CECOpNodeFunction::~CECOpNodeFunction() {}

/*
16.3 Function ~checkNListStructureOK~

This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodeFunction::checkNListStructureOK() {
    return CECOpNode::checkNListStructureOK();
  }

/*
16.4 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeFunction::accept(CECOpTreeVisitor& v) {
    v.visit(*this);
    CECOpNode::accept(v);
  }

/*
16.5 Function ~getSignatureVector~

This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeFunction::getSignatureVector
                     (std::vector<CECGImplSecondoType*>& sVector,
                      bool callRecursiv) {
    addCSTtoSignatureVector(sVector);
    if (callRecursiv) {
      std::vector<CECOpNode*>::iterator pos;
      for ( pos = sons.begin(); pos != sons.end(); pos++ ) {
        (*pos)->getSignatureVector(sVector, false);
      }
    }
  }

  

  
/*
17 Class ~CECOpNodeApplyabs~

This class is a subclass of ~CECOpNode~ and ~CECPtrCodeStoreType~. It implements
the node that is formed from the following annotated list:

----        
              ((none applyabs (ann(abstraction) ann(arg1) ... ann(argn)))
        <resulttype>)
                
----

17.1 The Constructor.

*/
  CECOpNodeApplyabs::CECOpNodeApplyabs(ListExpr& list,
                                       CECOpNode* fatherNode)
  : CECOpNode(list, fatherNode, std::string("applyabs")),
    CECPtrCodeStoreType() {
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
    
    setPtrCodeStoreType(ceAnnotateList);
    setIsCECImplSecondoType(isCECCodeGenerationImplemented, true);

    int numLenghtList = nl->ExprLength(nl->Third(nl->First(ceAnnotateList)));

    ListExpr tmpList;
    for (int i = 1; i <= numLenghtList; i++) {
      tmpList = nl->Nth(i, nl->Third(nl->First(ceAnnotateList)));
      sons.push_back(CECOpNode::createCECOpTree(tmpList, this));
    }
  }

/*
17.2 The Destructor.

*/
  CECOpNodeApplyabs::~CECOpNodeApplyabs() {}

/*
17.3 Function ~checkNListStructureOK~

This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodeApplyabs::checkNListStructureOK() {
    return CECOpNode::checkNListStructureOK()
        && (nl->ListLength( ceAnnotateList ) == 2)
        && (nl->ListLength( nl->First( ceAnnotateList )) == 3)
        && (nl->IsAtom(nl->First(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Second(nl->First( ceAnnotateList ))))
        && (!nl->IsEmpty(nl->Second( ceAnnotateList )));
  }

/*
17.4 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeApplyabs::accept(CECOpTreeVisitor& v) {
    v.visit(*this);
    CECOpNode::accept(v);
  }

/*
17.5 Function ~getSignatureVector~

This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeApplyabs::getSignatureVector
                     (std::vector<CECGImplSecondoType*>& sVector,
                      bool callRecursiv) {
    addCSTtoSignatureVector(sVector);
    if (callRecursiv) {
      std::vector<CECOpNode*>::iterator pos;
      for ( pos = sons.begin(); pos != sons.end(); pos++ ) {
        (*pos)->getSignatureVector(sVector, false);
      }
    }
  }

  

/*
18 Class ~CECOpNodeApplyfun~

This class is a subclass of ~CECOpNode~ and ~CECPtrCodeStoreType~. It implements
the node that is formed from the following annotated list:

----        
              ((none applyfun (ann(function) ann(arg1) ... ann(argn)))
                <resulttype>)
                
----

18.1 The Constructor.

*/
  CECOpNodeApplyfun::CECOpNodeApplyfun(ListExpr& list,
                                       CECOpNode* fatherNode)
  : CECOpNode(list, fatherNode, std::string("applyfun")),
    CECPtrCodeStoreType()  {
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
    
    setPtrCodeStoreType(ceAnnotateList);
    setIsCECImplSecondoType(isCECCodeGenerationImplemented, true);

    int numLenghtList = nl->ExprLength(nl->Third(nl->First(ceAnnotateList)));

    ListExpr tmpList;
    for (int i = 1; i <= numLenghtList; i++) {
      tmpList = nl->Nth(i, nl->Third(nl->First(ceAnnotateList)));
      sons.push_back(CECOpNode::createCECOpTree(tmpList, this));
    }
  }

/*
18.2 The Destructor.

*/
  CECOpNodeApplyfun::~CECOpNodeApplyfun() {}
  
/*
18.3 Function ~checkNListStructureOK~

This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodeApplyfun::checkNListStructureOK() {
    return CECOpNode::checkNListStructureOK()
        && (nl->ListLength( ceAnnotateList ) == 2)
        && (nl->ListLength( nl->First( ceAnnotateList )) == 3)
        && (nl->IsAtom(nl->First(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Second(nl->First( ceAnnotateList ))))
        && (!nl->IsEmpty(nl->Second( ceAnnotateList )));
  }

/*
18.4 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeApplyfun::accept(CECOpTreeVisitor& v) {
    v.visit(*this);
    CECOpNode::accept(v);
  }

/*
18.5 Function ~getSignatureVector~

This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeApplyfun::getSignatureVector
                     (std::vector<CECGImplSecondoType*>& sVector,
                      bool callRecursiv) {
    addCSTtoSignatureVector(sVector);
    if (callRecursiv) {
      std::vector<CECOpNode*>::iterator pos;
      for ( pos = sons.begin(); pos != sons.end(); pos++ ) {
        (*pos)->getSignatureVector(sVector, false);
      }
    }
  }

  

  
  
/*
19 Class ~CECOpNodeCounterdef~

This class is a subclass of ~CECOpNode~. It implements
the node that is formed from the following annotated list:

----        
        ((none counterdef <idx> ann(subexpr))
          <resulttype>)
                
----

19.1 The Constructor.

*/
  CECOpNodeCounterdef::CECOpNodeCounterdef(ListExpr& list,
                                           CECOpNode* fatherNode)
  : CECOpNode(list, fatherNode, std::string("counterdef")) {
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
  }

/*
19.2 The Destructor.

*/
  CECOpNodeCounterdef::~CECOpNodeCounterdef() {}
  
/*
19.3 Function ~checkNListStructureOK~

This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodeCounterdef::checkNListStructureOK() {
    return CECOpNode::checkNListStructureOK()
        && (nl->ListLength( ceAnnotateList ) == 2)
        && (nl->ListLength( nl->First( ceAnnotateList )) == 4)
        && (nl->IsAtom(nl->First(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Second(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Third(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Fourth(nl->First( ceAnnotateList ))))
        && (!nl->IsEmpty(nl->Second( ceAnnotateList )));
  }

/*
19.4 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeCounterdef::accept(CECOpTreeVisitor& v) {
    v.visit(*this);
    CECOpNode::accept(v);
  }

  
  
/*
20 Class ~CECOpNodePointer~

This class is a subclass of ~CECOpNode~. It implements
the node that is formed from the following annotated list:

----        
        (pointer <memorx address>)
                
----

20.1 The Constructor.

*/
  CECOpNodePointer::CECOpNodePointer(ListExpr& list,
                                     CECOpNode* fatherNode)
  : CECOpNode(list, fatherNode, std::string("pointer")) {
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
  }

/*
20.2 The Destructor.

*/
  CECOpNodePointer::~CECOpNodePointer() {}
  
/*
20.3 Function ~checkNListStructureOK~

This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodePointer::checkNListStructureOK() {
    return (nl->ListLength( ceAnnotateList ) == 2)
        && (nl->ListLength( nl->Second( ceAnnotateList )) == 2)
        && (!nl->IsEmpty(nl->First(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->First(nl->Second( ceAnnotateList ))))
        && (nl->IsAtom(nl->Second(nl->Second( ceAnnotateList ))));
  }

/*
20.4 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodePointer::accept(CECOpTreeVisitor& v) {
    v.visit(*this);
    CECOpNode::accept(v);
  }

  
  
  
/*
21 Class ~CECOpNodePredinfodef~

This class is a subclass of ~CECOpNode~. It implements
the node that is formed from the following annotated list:

----        
        ((none predinfodef 0.012 0.1442 ann(subexpr))
          <resulttype>)
                
----

21.1 The Constructor.

*/
  CECOpNodePredinfodef::CECOpNodePredinfodef(ListExpr& list,
                                             CECOpNode* fatherNode)
  : CECOpNode(list, fatherNode, std::string("predinfodef")) {
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
  }

/*
21.2 The Destructor.

*/
  CECOpNodePredinfodef::~CECOpNodePredinfodef() {}
  
/*
21.3 Function ~checkNListStructureOK~

This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodePredinfodef::checkNListStructureOK() {
    return CECOpNode::checkNListStructureOK()
        && (nl->ListLength( ceAnnotateList ) == 2)
        && (nl->ListLength( nl->First( ceAnnotateList )) == 5)
        && (nl->IsAtom(nl->First(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Second(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Third(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Fourth(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Fifth(nl->First( ceAnnotateList ))))
        && (!nl->IsEmpty(nl->Second( ceAnnotateList )));
  }

/*
21.4 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodePredinfodef::accept(CECOpTreeVisitor& v) {
    v.visit(*this);
    CECOpNode::accept(v);
  }

  
  
  
/*
22 Class ~CECOpNodeMemorydef~

This class is a subclass of ~CECOpNode~. It implements
the node that is formed from the following annotated list:

----        
        ((none memorydef 512 ann(subexpr))
          <resulttype>)
                
----

22.1 The Constructor.

*/
  CECOpNodeMemorydef::CECOpNodeMemorydef(ListExpr& list,
                                         CECOpNode* fatherNode)
  : CECOpNode(list, fatherNode, std::string("memorydef")) {
    if (!checkNListStructureOK()) 
      throw CECRuntimeError("Structure of the  annotated list is incorrect.");
  }

/*
22.2 The Destructor.

*/
  CECOpNodeMemorydef::~CECOpNodeMemorydef() {}
  
/*
22.3 Function ~checkNListStructureOK~

This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  bool
  CECOpNodeMemorydef::checkNListStructureOK() {
    return CECOpNode::checkNListStructureOK()
        && (nl->ListLength( ceAnnotateList ) == 2)
        && (nl->ListLength( nl->First( ceAnnotateList )) == 4)
        && (nl->IsAtom(nl->First(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Second(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Third(nl->First( ceAnnotateList ))))
        && (nl->IsAtom(nl->Fourth(nl->First( ceAnnotateList ))))
        && (!nl->IsEmpty(nl->Second( ceAnnotateList )));
  }

/*
22.4 Function ~accept~

This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
  void
  CECOpNodeMemorydef::accept(CECOpTreeVisitor& v) {
    v.visit(*this);
    CECOpNode::accept(v);
  }

  
  
  
/*
23 Class ~CECOpTreeVisitorModifyAnnotateListFormQP~

This class is a subclass of ~CECOpTreeVisitor~. This visitor traversed the operator tree
and modified the annotate list in the format of the ~Secondo QueryProcessor~.

23.1 The Constructor.

*/
  
  
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::CECOpTreeVisitorModifyAnnotateListFormQP() {}
  
/*
23.2 The Destructor.

*/
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::~CECOpTreeVisitorModifyAnnotateListFormQP() {}
    
/*
23.3 Functions ~visit~

The following functions implement the visitor's behavior for each node type of the operator tree.

These functions implement the abstract functions of the base class ~CECOpTreeVisitor~.

*/
  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNode& ceNode) {}

  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNodeOperator& ceNode) {
    ceNode.generateCEAnnotateList();
  }
  
  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNodeApplyop& ceNode) {
    ceNode.generateCEAnnotateList();
  }
  
  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNodeAbstraction& ceNode) {}
  
  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNodeObject& ceNode) {}
  
  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNodeConstant& ceNode) {}
 
  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNodeVariable& ceNode) {}
  
  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNodeIdentifier& ceNode) {}
  
  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNodeArglist& ceNode) {}
  
  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNodeFunction& ceNode) {}
  
  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNodeApplyabs& ceNode) {}
  
  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNodeApplyfun& ceNode) {}
  
  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNodeCounterdef& ceNode) {}
  
  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNodePointer& ceNode) {}
  
  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNodePredinfodef& ceNode) {}
  
  void
  CECOpTreeVisitorModifyAnnotateListFormQP
  ::visit(CECOpNodeMemorydef& ceNode) {}
  


/*
24 Class ~CECOpTreeVisitorSearchCompiledExpressionRoot~

This class is a subclass of ~CECOpTreeVisitor~. This visitor traversed the operator tree
and searched for all nodes that represent a root node of a compiled expressions.

24.1 The Constructor.

*/
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::CECOpTreeVisitorSearchCompiledExpressionRoot() {
    cecOpTreeRootNode = 0;
  }
  
/*
24.2 The Destructor.

*/
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::~CECOpTreeVisitorSearchCompiledExpressionRoot() {}
  
/*
24.3 Functions ~visit~

The following functions implement the visitor's behavior for each node type of the operator tree.

These functions implement the abstract functions of the base class ~CECOpTreeVisitor~.

*/
  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNode& ceNode) {}
  
  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNodeOperator& ceNode) {}
  
  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNodeApplyop& ceNode) {}
  
  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNodeAbstraction& ceNode) {
    if (ceNode.getIsCERootNode(true)) {
      rootNodes.push_back(&ceNode);

    }
  }
  
  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNodeObject& ceNode) {}
  
  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNodeConstant& ceNode) {}

  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNodeVariable& ceNode) {}

  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNodeIdentifier& ceNode) {}

  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNodeArglist& ceNode) {}

  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNodeFunction& ceNode) {}

  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNodeApplyabs& ceNode) {}

  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNodeApplyfun& ceNode) {}

  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNodeCounterdef& ceNode) {}

  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNodePointer& ceNode) {}

  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNodePredinfodef& ceNode) {}

  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::visit(CECOpNodeMemorydef& ceNode) {}

/*
24.4 Function ~setCECOpTreeRootNode~

A ~set~-function that saved a found root node in a local data structure.

*/
  void
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::setCECOpTreeRootNode(CECOpNode& node) {
    std::vector<CECOpNodeAbstraction*>::iterator pos;
    for ( pos = rootNodes.begin(); pos != rootNodes.end(); ++pos ) {
      if ((*pos) == &node) {
        cecOpTreeRootNode = &node;
        break;
      }
    }
  }
  
/*
24.5 Function ~getRootNode~

A ~get~-function returned the ~idx~'th root node.

*/
  CECOpNodeAbstraction*
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::getRootNode(unsigned int idx) {
    if ( idx < 0 || idx > rootNodes.size())
      return NULL;
    else
      return rootNodes[idx];
  }
  
/*
24.6 Function ~sizeRootNodes~

This function returned the number of found root node.

*/
  int 
  CECOpTreeVisitorSearchCompiledExpressionRoot
  ::sizeRootNodes() {
    return rootNodes.size();
  }
  
  

  
  
/*
25 Class ~CECOpTreeVisitorNumberingCECOpNodes~

This class is a subclass of ~CECOpTreeVisitor~. This visitor traversed the operator tree
and assigns a unique number to each node. This number will be used for the generation of
unique class names in code generation phase.

25.1 The Constructor.

*/
  CECOpTreeVisitorNumberingCECOpNodes
  ::CECOpTreeVisitorNumberingCECOpNodes()
  : nextNodeNumber(0) {}
  
/*
25.2 The Destructor.

*/
  CECOpTreeVisitorNumberingCECOpNodes
  ::~CECOpTreeVisitorNumberingCECOpNodes() {}
    
/*
25.3 Functions ~visit~

The following functions implement the visitor's behavior for each node type of the operator tree.

These functions implement the abstract functions of the base class ~CECOpTreeVisitor~.

*/
  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNode& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }

  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNodeOperator& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }
  
  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNodeApplyop& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }

  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNodeAbstraction& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }

  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNodeObject& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }

  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNodeConstant& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }

  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNodeVariable& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }

  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNodeIdentifier& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }

  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNodeArglist& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }

  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNodeFunction& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }

  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNodeApplyabs& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }

  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNodeApplyfun& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }

  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNodeCounterdef& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }

  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNodePointer& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }

  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNodePredinfodef& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }

  void
  CECOpTreeVisitorNumberingCECOpNodes
  ::visit(CECOpNodeMemorydef& ceNode) {
    if (!ceNode.isSetNodeNumber())
      ceNode.setStrNodeNumber(++nextNodeNumber);
  }


  
/*
26 Class ~CECOpTreeVisitorNumberingCECOpNodes~

This class is a subclass of ~CECOpTreeVisitor~. This visitor traversed the operator tree
and assigns a unique number to each node. This number will be used for the generation of
unique class names in code generation phase.

26.1 The Constructor.

*/
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY()
  : ptrCEQY(0) {}
  
/*
26.2 The Destructor.

*/
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::~CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY() {}
    
/*
26.3 Functions ~visit~

The following functions implement the visitor's behavior for each node type of the operator tree.

These functions implement the abstract functions of the base class ~CECOpTreeVisitor~.

*/
  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNode& ceNode) {
    if (ptrCEQY && ceNode.isSetNodeNumber())
      ptrCEQY->addMapResultTypes(ceNode.getStrNodeKey(),
                                 ceNode.getListReturnType());
  }

  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNodeOperator& ceNode) {}
  
  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNodeApplyop& ceNode) {
    if (ptrCEQY && ceNode.isSetNodeNumber())
      ptrCEQY->addMapResultTypes(ceNode.getStrNodeKey(),
                                 ceNode.getListReturnType());
  }

  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNodeAbstraction& ceNode) {
    if (ptrCEQY && ceNode.isSetNodeNumber())
      ptrCEQY->addMapResultTypes(ceNode.getStrNodeKey(),
                                 ceNode.getListReturnType());
  }

  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNodeObject& ceNode) {
    if (ptrCEQY && ceNode.isSetNodeNumber())
      ptrCEQY->addMapResultTypes(ceNode.getStrNodeKey(),
                                 ceNode.getListReturnType());
  }

  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNodeConstant& ceNode) {
    if (ptrCEQY && ceNode.isSetNodeNumber())
      ptrCEQY->addMapResultTypes(ceNode.getStrNodeKey(),
                                 ceNode.getListReturnType());
  }

  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNodeVariable& ceNode) {
    if (ptrCEQY && ceNode.isSetNodeNumber())
      ptrCEQY->addMapResultTypes(ceNode.getStrNodeKey(),
                                 ceNode.getListReturnType());
  }

  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNodeIdentifier& ceNode) {
    if (ptrCEQY && ceNode.isSetNodeNumber())
      ptrCEQY->addMapResultTypes(ceNode.getStrNodeKey(),
                                 ceNode.getListReturnType());
  }

  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNodeArglist& ceNode) {
    if (ptrCEQY && ceNode.isSetNodeNumber())
      ptrCEQY->addMapResultTypes(ceNode.getStrNodeKey(),
                                 ceNode.getListReturnType());
  }

  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNodeFunction& ceNode) {
    if (ptrCEQY && ceNode.isSetNodeNumber())
      ptrCEQY->addMapResultTypes(ceNode.getStrNodeKey(),
                                 ceNode.getListReturnType());
  }

  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNodeApplyabs& ceNode) {
    if (ptrCEQY && ceNode.isSetNodeNumber())
      ptrCEQY->addMapResultTypes(ceNode.getStrNodeKey(),
                                 ceNode.getListReturnType());
  }

  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNodeApplyfun& ceNode) {
    if (ptrCEQY && ceNode.isSetNodeNumber())
      ptrCEQY->addMapResultTypes(ceNode.getStrNodeKey(),
                                 ceNode.getListReturnType());
  }

  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNodeCounterdef& ceNode) {
    if (ptrCEQY && ceNode.isSetNodeNumber())
      ptrCEQY->addMapResultTypes(ceNode.getStrNodeKey(),
                                 ceNode.getListReturnType());
  }

  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNodePointer& ceNode) {
    if (ptrCEQY && ceNode.isSetNodeNumber())
      ptrCEQY->addMapResultTypes(ceNode.getStrNodeKey(),
                                 ceNode.getListReturnType());
  }

  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNodePredinfodef& ceNode) {
    if (ptrCEQY && ceNode.isSetNodeNumber())
      ptrCEQY->addMapResultTypes(ceNode.getStrNodeKey(),
                                 ceNode.getListReturnType());
  }

  void
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::visit(CECOpNodeMemorydef& ceNode) {
    if (ptrCEQY && ceNode.isSetNodeNumber())
      ptrCEQY->addMapResultTypes(ceNode.getStrNodeKey(),
                                 ceNode.getListReturnType());
  }

/*
26.4 Function ~setCEQuery~

This function copies the annotated list of the return types from each node to
the ~CEQuery~-object.

*/
  void 
  CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  ::setCEQuery(CEQuery* ceQY) {
    ptrCEQY = ceQY;
  }
  

  
  
  
/*
27 Class ~CECompiler~

This class implements the concrete compiler. The class is a singleton.

Initialisation of the instance variable.

*/
  CECompiler* CECompiler::instance = 0;
  
/*
27.1 Function ~getInstance~

This static function returned a pointer of the ~CECompiler~-instance. 
If no instance has been created, the function creates an instance.

*/
  CECompiler*
  CECompiler::getInstance() {
    if (!instance)
      instance = new CECompiler();
    return instance;
  }
  
/*
27.2 Function ~deleteInstance~

This static function deleted the ~CECompiler~-instance. 

*/
  void
  CECompiler::deleteInstance() {
    if (instance) 
      delete instance;
    instance = 0;
  }
  
  
/*
27.3 The Constructor.

*/
  CECompiler::CECompiler() {}
  
/*
27.4 The Destructor.

*/
  CECompiler::~CECompiler() {
    CECodeGenerator::deleteInstance();
  }
  
/*
27.5 Function ~ceGenerateQuery~

This function is the central function of the ~CECompiler~ in which the
shared library is generated from the annotated list and loaded via
the ~CEQueryProcessor~.

*/
  void
  CECompiler::ceGenerateQuery(ListExpr& ceAnnotateList,
                              CEQuery* ptrCEQY) {
    bool debugMode = ptrCEQY
                    && ptrCEQY->getPtrCEQP()
                    && ptrCEQY->getPtrCEQP()->getQPDebugMode();
    bool traceMode = ptrCEQY
                    && ptrCEQY->getPtrCEQP()
                    && ptrCEQY->getPtrCEQP()->getQPTraceMode();
      
    CECOpNode* ptrCECOpTree = 0;
    CECOpNodeApplyop* ptrApplyopTree = 0;
    bool lokalInitQuery = true;
    bool codeGenerationOK = false;
    CECOpTreeVisitorNumberingCECOpNodes NumberingCECOpNodes;
    CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
                                        CopyCECOpNodesResultTypeToCEQY;
    CECOpTreeVisitorSearchCompiledExpressionRoot SearchCompiledExpressionRoot;
    CECOpNodeApplyop* castRootApplyop = 0;
    std::string libName = "ceQueryCode";
    
    std::string pwdCurrent, tmpPathGenCode;
    tmpPathGenCode = pwdCurrent = FileSystem::GetCurrentFolder();
    FileSystem::AppendItem(tmpPathGenCode, "..");
    FileSystem::AppendItem(tmpPathGenCode, "Algebras");
    FileSystem::AppendItem(tmpPathGenCode, "CompiledExpressions");
    FileSystem::AppendItem(tmpPathGenCode, "ceTmp");
    if (ptrCEQY)
      ptrCEQY->setCompilePath(pwdCurrent, tmpPathGenCode);
    
    if (debugMode)
      cout << endl
           << "*** Compiled Expressions Algebra: Generate Query Begin ***"
           << endl;
      
    try {
      if (traceMode)
        cout << "Create CEOpTree..."
             << endl;
      ptrCECOpTree = CECOpNode::createCECOpTree(ceAnnotateList, NULL);
      ptrCECOpTree->createApplyopTree(NULL, ptrApplyopTree);
      if (traceMode)
        cout << "Search for Compiled Expressions Algebra-Operators..."
             << endl;
      ptrCECOpTree->checkIsCEAOperator(true);
      if (traceMode)
        cout << "Search for Compiled-Root-Nodes..."
             << endl;
      ptrCECOpTree->checkIsCERootNode(true);
      if (traceMode)
        cout << "Annotate CEOpTree..." 
             << endl;
      ptrCECOpTree->accept(NumberingCECOpNodes);
      CopyCECOpNodesResultTypeToCEQY.setCEQuery(ptrCEQY);
      ptrCECOpTree->accept(CopyCECOpNodesResultTypeToCEQY);
    } catch (...) {
      lokalInitQuery = false;
      resetAnnotateListToOrigForm(ceAnnotateList, (debugMode || traceMode));
    }
    
    if (lokalInitQuery) {
      if (ptrCECOpTree->getIsCERootNode(true)) {
        if (traceMode)
          cout << "CEOpTree-RootNode is compiled Root-Node."
               << endl;
        try {
          castRootApplyop = static_cast<CECOpNodeApplyop*>(ptrCECOpTree);
          castRootApplyop->getInstCEGenerateFunctionStatus()
                           ->setCallLibFunctionName("executeQueryFunction");
          castRootApplyop->unsetIsCERootNodeNoCECOpNodeRoot();
          if (traceMode)
            cout << "Generate Code..."
                 << endl;
          codeGenerationOK = CECodeGenerator::getInstance()
                               ->generateCECode(ptrCECOpTree, ptrCEQY);
          castRootApplyop->getInstCEGenerateFunctionStatus()
                           ->setCodeGenerationOK(codeGenerationOK);
        } catch (...) {
          lokalInitQuery = false;
          resetAnnotateListToOrigForm(ceAnnotateList,
                                      (debugMode || traceMode));
        }
      } else {
        try {
          ptrCECOpTree->accept(SearchCompiledExpressionRoot);
        } catch (...) {
          lokalInitQuery = false;
          resetAnnotateListToOrigForm(ceAnnotateList,
                                      (debugMode || traceMode));
        }
        
        if (traceMode)
          cout << "CEOpTree insert "
               <<  SearchCompiledExpressionRoot.sizeRootNodes()
               << " Root-Nodes."
               << endl;
        
        if (lokalInitQuery
          && SearchCompiledExpressionRoot.sizeRootNodes() > 0) {
          if (traceMode)
            cout << "Generate Code..."
                 << endl;

          std::string ceCallLibFunctionName;
          bool lokalCodeGenerationOK;
          for (int i = 0;
               i < SearchCompiledExpressionRoot.sizeRootNodes();
               i++) {
            try {
              ceCallLibFunctionName
                    = (SearchCompiledExpressionRoot.getRootNode(i))
                         ->generateCECallLibFunctionName(i);
              SearchCompiledExpressionRoot.getRootNode(i)
                    ->getInstCEGenerateFunctionStatus()
                      ->setCallLibFunctionName(ceCallLibFunctionName);
              lokalCodeGenerationOK
                    = CECodeGenerator::getInstance()
                        ->generateCECode
                          (SearchCompiledExpressionRoot.getRootNode(i),
                           ptrCEQY);
              SearchCompiledExpressionRoot.getRootNode(i)
                    ->getInstCEGenerateFunctionStatus()
                      ->setCodeGenerationOK(lokalCodeGenerationOK);
              codeGenerationOK = (codeGenerationOK || lokalCodeGenerationOK);
            } catch (...) {
              ((SearchCompiledExpressionRoot.getRootNode(i))
                      ->getInstCEGenerateFunctionStatus())
                        ->setCodeGenerationOK(false);
            }
          }
        }
      }
      
      (ptrCEQY->getCodeGenVisitor()).addVarNamesARG_idx();
      
      if (lokalInitQuery 
        && (ptrCECOpTree->getIsCERootNode(true)
            || SearchCompiledExpressionRoot.sizeRootNodes() > 0)) {
        try {
          bool cfOK = true;
          if (!FileSystem::FileOrFolderExists(tmpPathGenCode))
            cfOK = cfOK && FileSystem::CreateFolder(tmpPathGenCode);
          if (cfOK) {
            FileSystem::SetCurrentFolder(tmpPathGenCode);
            
            std::ofstream oFileCode((libName + ".cpp").c_str());
            CECodeGenerator::getInstance()->getStreamGeneratedCECode(oFileCode,
                                                                     ptrCEQY);
            oFileCode.close();
            
            if (traceMode) {
              cout << "------------------------------"
                   << endl
                   << "The following code is created:"
                   << endl
                   << "------------------------------"
                   << endl;
              CECodeGenerator::getInstance()
                               ->getStreamGeneratedCECode(cout,
                                                          ptrCEQY);
              cout << "-----------------------------------"
                   << "-----------------------------------"
                   << endl;
            }

            std::ofstream oFileMake("makefile");
            CECodeGenerator::getInstance()->getStreamMake(oFileMake,
                                                          libName);
            oFileMake.close();
          }
        } catch (...) {
          FileSystem::SetCurrentFolder(pwdCurrent);
          lokalInitQuery = false;
          resetAnnotateListToOrigForm(ceAnnotateList,
                                      (debugMode || traceMode));
        }
      }
    }
    
    if (lokalInitQuery && codeGenerationOK) {
      if (traceMode)
        cout << "Compile Code..."
             << endl;
      try {
        int returnCode = std::system("(make >/dev/null 2>&1; exit $?)");
        if (WEXITSTATUS(returnCode)) {
          cout << "MAKE ERROR: Reset Query to original Queryprocessor..."
               << endl;
          throw CECRuntimeError("Can't compiled the generated code.");
        }
      } catch (...) {
        FileSystem::SetCurrentFolder(pwdCurrent);
        lokalInitQuery = false;
        codeGenerationOK = false;
        resetAnnotateListToOrigForm(ceAnnotateList,
                                    (debugMode || traceMode));
      }
    }
    
    
    if (lokalInitQuery && codeGenerationOK) {
      if (traceMode)
        cout << "Load the generated library..."
             << endl;
      try {
        ptrCEQY->getPtrCEQP()->loadCompiledExpressions
                                 (tmpPathGenCode + PATH_SLASH + libName,
                                  ptrCEQY);
        ListExpr tmpReturnType;
        if (castRootApplyop) {        
          tmpReturnType = ptrCECOpTree->getListReturnType();
          (castRootApplyop->getInstCEGenerateFunctionStatus())
                            ->loadCELibFunction(ptrCEQY, tmpReturnType);
        } else {
          CECOpNodeAbstraction* tmpRootNode;
          for (int i = 0;
               i < SearchCompiledExpressionRoot.sizeRootNodes();
               i++) {
            tmpRootNode = dynamic_cast<CECOpNodeAbstraction*>
                            (SearchCompiledExpressionRoot.getRootNode(i));
            if (tmpRootNode) {
              tmpReturnType = tmpRootNode->getListReturnType();
              (tmpRootNode->getInstCEGenerateFunctionStatus())
                            ->loadCELibFunction(ptrCEQY, tmpReturnType);
            }
          }
        }
      } catch (...) {
        lokalInitQuery = false;
        codeGenerationOK = false;
        resetAnnotateListToOrigForm(ceAnnotateList,
                                    (debugMode || traceMode));
      }
    }
    
    if (lokalInitQuery) {
      if (traceMode)
        cout << "Change the annotated list to format of the QueryProcessor..."
             << endl;
      try {
        ptrCECOpTree->checkCECGenerateAnnotateList();
        if (CECOpNodeApplyop* onApplyop = dynamic_cast<CECOpNodeApplyop*>
                                            (ptrCECOpTree))
          onApplyop->setPtrCECompiler(this);
        ptrCECOpTree->generateCEAnnotateList();
        ptrCEQY->setGeneratedQuery();
      } catch (...) {
        lokalInitQuery = false;
        resetAnnotateListToOrigForm(ceAnnotateList,
                                    (debugMode || traceMode));
      }
    }

    if (ptrCECOpTree) {
      if (traceMode)
        cout << "Delete CEOpTree..."
             << endl;

      delete ptrCECOpTree;
    }
    FileSystem::SetCurrentFolder(pwdCurrent);
    
    if (debugMode) {
      cout << endl
           << "Annotated list from CE-Compiler:"
           << endl;
      nl->WriteListExpr(ceAnnotateList, cout, 2);
      cout << endl
           << "*** Compiled Expressions Algebra: Generate Query End ***"
           << endl;
    }
  }
  
/*
27.6 Function ~resetAnnotateListToOrigForm~

This function is called as soon as something is wrong during the creation or loading
of the shared library. The annotated list is then changed to the format of
the ~Secondo QueryProcessor~ so that the ~Secondo QueryProcessor~ can process the query
without the functionality of the ~Compiled Expressions Algebra~.

*/
  void
  CECompiler::resetAnnotateListToOrigForm(ListExpr& list,
                                          bool output) {
    if (output)
      cout << "An Error is occurred!"
           << endl
           << "Change the annotated list to the original"
           << " format of the QueryProcessor."
           << endl;

    ListExpr symbol, tmpList;
    std::string strSymbol;
    int listLength = nl->ExprLength(list);
    
    if (listLength > 0) {
      symbol = nl->TheEmptyList();
      if (listLength == 1) {
        if (nl->ExprLength(list) > 1)
          symbol = nl->Second(list);
      } else {
        if (nl->ExprLength(nl->First(list)) > 1)
          symbol = nl->Second(nl->First(list));
      }
        
      if (nl->IsAtom(symbol)
        && nl->AtomType(symbol) == SymbolType) {
        strSymbol = nl->SymbolValue(symbol);
        if (strSymbol == "operator") {
          nl->Replace(nl->Third(nl->First(list)),
                      nl->First(nl->Third(nl->First(list))));
          nl->Replace(nl->Fourth(nl->First(list)),
                      nl->First(nl->Fourth(nl->First(list))));
        } else if (strSymbol == "applyop") {
          nl->Replace(nl->Third(list),
                      nl->First(nl->Third(list)));
        }
  
      }
      
      if (!nl->IsEmpty(list)
        && !nl->IsAtom(list)) {
        for (int i = 1; i <= listLength; i++) {
          tmpList = nl->Nth(i, list);        
          resetAnnotateListToOrigForm(tmpList, false);
        }
      }
    }
  }

} // end of namespace CompiledExpressions