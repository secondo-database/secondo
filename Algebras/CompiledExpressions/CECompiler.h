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

[10] Header file of the CECompiler

2016/2017 H.Brieschke created the new CompiledExpression Algebra


[TOC]


1 Overview 

This header file declares all classes needed for the ~CECompiler~. These are

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
#ifndef _COMPILED_EXPRESSIONS_COMPILER_H_
#define _COMPILED_EXPRESSIONS_COMPILER_H_

//define macro TRACE_ON if trace outputs are needed
//#define TRACE_ON
#undef TRACE_ON

#include <set>
#include <string>
#include <vector>
#include <exception>

#include "NestedList.h"

namespace CompiledExpressions {

// forward declarations from classes in the CompiledExpressions-namespace
  class CECOpNodeApplyop;
  class CECOpTreeVisitor;
  class CECGImplSecondoType;
  class CECGImplSecondoOperator;
  class CEQuery;
  class CEQueryProcessor;
  class CECompiler;
  class CECodeGenerator;
  
/*
3 Class ~CECRuntimeError~

This class implements ~ERROR~-objects, which are required for the error treatment.

*/
  class CECRuntimeError : public std::exception {
  public:
    CECRuntimeError(std::string s);
    virtual ~CECRuntimeError() throw() {}
    virtual const char * what() const throw();
  private:
    std::string errMsg;
  };
  
/*
4 Class ~CEGenerateFunctionStatus~

This class implements various states on the progress of the code generation, as
well as other information required when using the generated shared library.

*/
  class CEGenerateFunctionStatus {    
  public:
/*
The Constructor.

*/
    CEGenerateFunctionStatus();
    
/*
The Destructor.

*/
    virtual ~CEGenerateFunctionStatus();
    
/*
A ~set~-function that can be used to store whether the code of the compiled query 
was generated correctly or not.

*/
    void setCodeGenerationOK(bool genCodeOK);
/*
A ~get~-function returned if the code of the compiled query was generated
correctly or not.

*/
    bool isCodeGenerationOK();
    
/*
A ~set~-function that can be used to store whether the generated shared library is
correctly loaded or not.

*/
    void setLibLoadOK(bool loadLibOK);
/*
A ~get~-function returned if the generated shared library is correctly loaded or not.

*/
    bool isLibLoadOK();
    
/*
A ~set~-function that can be used to store the function name to be called in the
shared library.

*/
    void setCallLibFunctionName(std::string libFuncName);
/*
A ~get~-function returned the function name to be called in the shared library.

*/
    std::string getCallLibFunctionName();
    
/*
This function calls the ~CEQueryProcessor~ to load the generated shared library and
saved the nested list with the memory address of the function in the shared library.

*/
    void loadCELibFunction(CEQuery* ptrCEQY,
                           ListExpr& returnType)
    throw (CECRuntimeError);
/*
A ~get~-function returned a nested list with the memory address of the function in
the shared library.

*/
    ListExpr& getCELibFunctionAddressNList()
    throw (CECRuntimeError);
    
  private:
    int generateStatus;
    std::string ceCallLibFunctionName;
    ListExpr libFunctionAddress;
    
/*
A enumeration with the various statics of the generated shared library.

*/
    enum CEGenerateCompiledExpression {
      CEC_GCE_UNDEF,
      CEC_GCE_GENCODE,
      CEC_GCE_LOADLIB,
      CEC_GCE_ERROR
    };
  }; //end of class CEGenerateFunctionStatus
  

  
/*
5 Class ~CECOpNode~

This class is the base class of all nodes of the ~CECompiler Operator Tree~. 
In this class all functions that are required for all node types are implemented.

*/
  class CECOpNode {    
  public:
    
/*
A static function to create the ~CECompiler Operator Tree~. 

*/
    static CECOpNode* createCECOpTree(ListExpr& expr,
                                      CECOpNode* father)
    throw (CECRuntimeError);

/*
The Destructor.

*/
    virtual ~CECOpNode();
    
/*
This function constructs another tree structure within the tree,
but it contains only ~CECOpNodeApplyop~-nodes.

*/
    virtual void createApplyopTree(CECOpNodeApplyop* fatherNode,
                                   CECOpNodeApplyop*& rootNodes);
    
/*
A ~get~-function returned ~TRUE~ or ~FALSE~ if the generation of the code is implemented
for the respective node.

*/
    bool getIsCECImplementedCodeGeneration(bool onlyTrue);

/*
A ~get~-function returned the integer from the enumeration ~CECTruthValues~, which
indicates if the generation of the code is implemented for the respective node.

*/
    int getIsCECImplementedCodeGeneration();
    
/*
A ~set~-function that checks whether the node is an operator, and if so,
whether it is also implemented in the ~Compiled Expressions Algebra~.

*/
    virtual void checkIsCEAOperator(bool force = false);

/*
A ~get~-function returned ~TRUE~ or ~FALSE~ if the node is a operator, 
that is also implemented in the ~Compiled Expressions Algebra~.

*/
    virtual bool getIsCEAOperator(bool onlyTrue);

/*
A ~get~-function returned the integer from the enumeration ~CECTruthValues~,
if the node is a operator, that is also implemented in the ~Compiled Expressions Algebra~.

*/
    int getIsCEAOperator();
    
/*
A ~set~-function that checks whether the node is the root node from the compiled expression.

*/
    virtual void checkIsCERootNode(bool force = false);
    
/*
A ~set~-function that delete the root node entry for the node.

*/
    void unsetIsCERootNodeNoCECOpNodeRoot();

/*
A ~get~-function returned ~TRUE~ or ~FALSE~ if the node is the root node from the
compiled expression.

*/
    bool getIsCERootNode(bool onlyTrue);

/*
A ~get~-function returned the integer from the enumeration ~CECTruthValues~,
if the node is the root node from the compiled expression.

*/
    int getIsCERootNode();
    
/*
This function generates a unique function name, which is used in the shared library
to be generated.

*/
    virtual std::string generateCECallLibFunctionName(int idxCERootNode);

/*
A ~set~-function that checks to which form the annotated list, associated with the node,
must be converted for the ~Secondo QueryProcessor~.

*/
    virtual void checkCECGenerateAnnotateList();

/*
A ~get~-function returned the integer from the enumeration ~CECGenerateAnnotateList~,
to which form the annotated list, associated with the node, must be converted for
the ~Secondo QueryProcessor~.

*/
    int getCECGenerateAnnotateList();
    
/*
A ~get~-function returned the position number in the vector of sons in the father node.

*/
    int getSonsPosition();

/*
The function converts the annotated list from the node into a form that
the ~Secondo QueryProcessor~ can process.

*/
    virtual void generateCEAnnotateList();
    
/*
A ~get~-function returned nested list with the return type of the node.

*/
    virtual ListExpr getListReturnType();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

*/
    virtual void accept(CECOpTreeVisitor&);

/*
This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

*/
    virtual void getSignatureVector(std::vector<CECGImplSecondoType*>& sVector,
                                    bool callRecursiv);

/*
A ~get~-function returned the number of sons.

*/
    unsigned int getNumSons();

/*
A ~set~-function sets the unique node number as string representation.

*/
    void setStrNodeNumber(int nodeNum);

/*
A ~get~-function returned if the node number is already set.

*/
    bool isSetNodeNumber();

/*
A ~get~-function returned the string with the unique node key.

*/
    std::string getStrNodeKey();

/*
A ~get~-function returned a pointer to the fathernode.

*/
    CECOpNode* getPtrFatherNode();

/*
A ~get~-function returned a pointer to the sonnode at the position ~posSon~.

*/
    CECOpNode* getPtrSonNode(const unsigned int posSon)
    throw (CECRuntimeError);

/*
A enumeration with the various truth values that are used in the nodes.

*/
    enum CECTruthValues {
      CEC_TV_UNDEF,
      CEC_TV_CANDIDATE_FALSE,
      CEC_TV_FALSE,
      CEC_TV_CANDIDATE_TRUE,
      CEC_TV_TRUE
    };
    
/*
A enumeration with the various values in which form the annotated list from the
node are converted.

*/
    enum CECGenerateAnnotateList {
      CEC_GAL_UNDEF,
      CEC_GAL_ORIG_OP,
      CEC_GAL_CEA_OP,
      CEC_GAL_CEA_CQOP
    };
    
  protected:
/*
Different local variables...

*/
    std::vector<CECOpNode*> sons;
    ListExpr ceAnnotateList;
    CECOpNode* ptrFatherNode;
    int isCECCodeGenerationImplemented;
    int isCEAOperator;
    int isCERootNode;
    int cecGenerateAnnotateList;
    std::string strNodeNumber;
    std::string strNodeType;
    
/*
The Constructors.

*/
    CECOpNode() {};
    CECOpNode(ListExpr& list,
              CECOpNode* fatherNode,
              std::string nodeType);
    
/*
This function checks the correct form of the annotated list from the node.

*/
    virtual bool checkNListStructureOK();

/*
A ~set~-function for the member variable, if the generation of the code is implemented for
the respective node.

*/
    virtual void setIsCECImplementedCodeGeneration();

/*
A ~set~-function for the member variable, if the respective node an operator from
the ~Compiled Expressions Algebra~.

*/
    virtual void setIsCEAOperator();
    
/*
A ~set~-function for the member variable, if the respective node a root node
of any compiled expression.

*/
    virtual void setIsCERootNode();
    
/*
A ~set~-function for the member variable, to which form the annotated list,
associated with the node, must be converted for the ~Secondo QueryProcessor~.

*/
    virtual void setCECGenerateAnnotateList();
    
/*
A ~get~-function returned the position number of the node ~node~ in the vector of sons.

*/
    int searchSonsPosition(CECOpNode* node);
    
  }; //end of class CECOpNode
  

  
/*
6 Class ~CECPtrCodeStoreType~

This class implements a data store for the ~CECGImplXXXTypeXXX~-Objects
of the return types from the nodes.

*/
  class CECPtrCodeStoreType {
  public:
/*
The Destructor

*/
    virtual ~CECPtrCodeStoreType() {}
    
/*
A ~get~-function returned a pointer to the ~CECGImplXXXTypeXXX~-Objects
of the return types from the nodes.

*/
    CECGImplSecondoType* getPtrCodeStoreType();
    
/*
A ~get~-function returned a pointer to the ~CECGImplXXXTypeXXX~-Objects
of the return subtypes from the nodes.

*/
    CECGImplSecondoType* getPtrSubCodeStoreType();
    
/*
A ~get~-function returned if the return type of the node is a stream.

*/
    bool getIsStreamType();
    
  protected:
/*
Different local variables...

*/
    CECGImplSecondoType* ptrCodeStoreType;
    CECGImplSecondoType* ptrSubCodeStoreType;
    
/*
The Constructor

*/
    CECPtrCodeStoreType();
    
/*
A ~set~-function saved the pointer to the ~CECGImplXXXTypeXXX~-Objects
of the return type and return subtype from the node.

*/
    void setPtrCodeStoreType(ListExpr& list);

/*
A ~set~-function saved the integer from the enumeration ~CECTruthValues~, which
indicates if the generation of the code of the return type is implemented for the
respective node.

*/
    void setIsCECImplSecondoType(int& retValue, bool force);

/*
This function added the pointer to the ~CECGImplXXXTypeXXX~-Objects of the 
return type to the signature vector ~sVector~.

*/
    void addCSTtoSignatureVector(std::vector<CECGImplSecondoType*>& sVector);
    
    
  private:
/*
Other local variables...

*/
    bool isSetPtrCodeStoreType;
    bool isStreamType;
  };  //end of class CECPtrCodeStoreType
  

/*
7 Class ~CECPtrMap~

This class implements a data store for the ~CECGImplXXXTypeXXX~-Objects
of the map types in the ~Map~-functions from the node.

*/
  class CECPtrMap {
  public:
/*
The Destructor

*/
    virtual ~CECPtrMap();
    
/*
A ~get~-function returned a vector of pointers to the ~CECGImplXXXTypeXXX~-Objects
of the map types from the nodes.

*/
    std::vector<CECGImplSecondoType*> getVectorPtrCodeStoreTypes();
    
  protected:
/*
A local variable 

*/
    std::vector<CECGImplSecondoType*> ptrCodeStoreTypes;
    
/*
The Constructor

*/
    CECPtrMap();
    
/*
A ~set~-function saved the pointers to the ~CECGImplXXXTypeXXX~-Objects
of the map types from the node.

*/
    void setVectorPtrCodeStoreTypes(ListExpr& list);

/*
A ~set~-function saved the integer from the enumeration ~CECTruthValues~, which
indicates if the generation of the code of the map types are implemented for the
respective node.

*/
    void setIsCECImplSecondoTypes(int& retValue, bool force);
    
  private:
/*
A other local variable...

*/
    bool isSetVectorPtrCodeStoreTypes;
    
  };  //end of class CECPtrMap
  
  
/*
8 Class ~CECOpNodeOperator~

This class is a subclass of ~CECOpNode~. It implements the node that is formed
from the following annotated list:

----        

((<name> operator <opID> <algID>) typeerror)

----


*/
  class CECOpNodeOperator : public CECOpNode {
  public:
/*
The Constructor

*/
    CECOpNodeOperator(ListExpr& list, CECOpNode* fatherNode);

/*
The Destructor

*/
    virtual ~CECOpNodeOperator();
    
/*
A ~get~-function returned a pointer to the ~CECGImplXXXOperatorXXX~-Object
from the nodes.

*/
    CECGImplSecondoOperator* getPtrCodeStoreOperator();

/*
A ~set~-function that checks whether the node is an operator, and if so,
whether it is also implemented in the ~Compiled Expressions Algebra~.

This function overrides the function of the base class ~CECOpNode~.

*/
    void checkIsCEAOperator(bool force = false);
    
/*
This function generates a unique function name, which is used in the shared library
to be generated.

This function overrides the function of the base class ~CECOpNode~.

*/
    std::string generateCECallLibFunctionName(int idxCERootNode);

/*
A ~set~-function that checks to which form the annotated list, associated with the node,
must be converted for the ~Secondo QueryProcessor~.

This function overrides the function of the base class ~CECOpNode~.

*/
    void checkCECGenerateAnnotateList();
    
/*
The function converts the annotated list from the node into a form that
the ~Secondo QueryProcessor~ can process.

This function overrides the function of the base class ~CECOpNode~.

*/
    void generateCEAnnotateList();

/*
This function generate and returned a unique name of the operator.

*/
    std::string getOperatorName();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void accept(CECOpTreeVisitor&);

/*
A ~get~-function returned a string with the algebra ID of the operator.

*/
    std::string getAlgId();
    
/*
A ~get~-function returned a string with the operator ID of the operator.

*/
    std::string getOpId();
    
/*
A ~get~-function returned a vector with pointers of ~CECGImplSecondoTypeXXX~-objects
so that the vector contains the complete return types as the signature of the operator.

*/
    std::vector<CECGImplSecondoType*> getSignatureVector();
    
  private:
/*
Different local variables...

*/
    ListExpr listAlgID;
    ListExpr listOpFunID;    
    CECGImplSecondoOperator* ptrCodeStoreOperator;

/*
The Standard Constructor

*/
    CECOpNodeOperator() {};

/*
This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool checkNListStructureOK();

/*
A ~set~-function for the member variable, if the generation of the code is implemented for
the respective node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void setIsCECImplementedCodeGeneration();

/*
A ~set~-function for the member variable, if the respective node an operator from
the ~Compiled Expressions Algebra~.

This function overrides the function of the base class ~CECOpNode~.

*/
    void setIsCEAOperator();
  }; //end of class CECOpNodeOperator
  
  
/*
9 Class ~CECOpNodeApplyop~

This class is a subclass of ~CECOpNode~ and ~CECPtrCodeStoreType~. It implements
the node that is formed from the following annotated list:

----        
              ((none applyop (ann(op) ann(arg1) ... ann(argn)))
                <resulttype>
                <opFunId>)
                
----

*/
  class CECOpNodeApplyop : public CECOpNode, public CECPtrCodeStoreType {    
  public:
/*
The Constructor

*/
    CECOpNodeApplyop(ListExpr& list, CECOpNode* fatherNode);

/*
The Destructor

*/
    virtual ~CECOpNodeApplyop();
    
/*
This function constructs another tree structure within the tree,
but it contains only ~CECOpNodeApplyop~-nodes.

This function overrides the function of the base class ~CECOpNode~.

*/
    void createApplyopTree(CECOpNodeApplyop* fatherNode,
                           CECOpNodeApplyop*& rootNodes);

/*
A ~set~-function that checks whether the node is an operator, and if so,
whether it is also implemented in the ~Compiled Expressions Algebra~.

This function overrides the function of the base class ~CECOpNode~.

*/
    void checkIsCEAOperator(bool force = false);
    
/*
A ~set~-function that checks whether the node is the root node from the compiled expression.

This function overrides the function of the base class ~CECOpNode~.

*/
    void checkIsCERootNode(bool force = false);
    
/*
A ~get~-function returned a pointer to the ~CEGenerateFunctionStatus~-object of the node.

*/
    CEGenerateFunctionStatus* getInstCEGenerateFunctionStatus();
    
/*
A ~set~-function to save a pointer to the ~CECompiler~-object.

*/
    void setPtrCECompiler(CECompiler* cec);
    
/*
This function generates a unique function name, which is used in the shared library
to be generated.

This function overrides the function of the base class ~CECOpNode~.

*/
    std::string generateCECallLibFunctionName(int idxCERootNode);

/*
A ~set~-function that checks to which form the annotated list, associated with the node,
must be converted for the ~Secondo QueryProcessor~.

This function overrides the function of the base class ~CECOpNode~.

*/
    void checkCECGenerateAnnotateList();
    
/*
The function converts the annotated list from the node into a form that
the ~Secondo QueryProcessor~ can process.

This function overrides the function of the base class ~CECOpNode~.

*/
    void generateCEAnnotateList();
    

/*
This function generate and returned a unique name of the operator.

*/
    std::string getOperatorName();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void accept(CECOpTreeVisitor&);
    //std::string generateOperatorFunctionSignatur();
    
/*
This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void getSignatureVector(std::vector<CECGImplSecondoType*>& sVector,
                            bool callRecursiv);
    
  private:
/*
Different local variables...

*/
    std::vector<CECOpNodeApplyop*> applyopSons;
    CECOpNodeApplyop* ptrApplyopFatherNode;
    ListExpr listOpFunID;
    CECompiler* ptrCEC;
    CEGenerateFunctionStatus* ptrCEGenFuncStat;
    
/*
The Standard Constructor

*/
    CECOpNodeApplyop() {};

/*
This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool checkNListStructureOK();

/*
A ~set~-function for the member variable, if the respective node an operator from
the ~Compiled Expressions Algebra~.

This function overrides the function of the base class ~CECOpNode~.

*/
    void setIsCEAOperator();

/*
This function added the node in the ~Applyop Node Tree~.

*/
    void addApplyopSon(CECOpNodeApplyop& son);
    
  }; //end of class CECOpNodeApplyop
  

/*
10 Class ~CECOpNodeAbstraction~

This class is a subclass of ~CECOpNode~ and ~CECPtrMap~. It implements
the node that is formed from the following annotated list:

----        
              ((none abstraction Annotate(expr) <functionno>) <type>)
                
----

*/  
  class CECOpNodeAbstraction : public CECOpNode, public CECPtrMap {    
  public:
/*
The Constructor

*/
    CECOpNodeAbstraction(ListExpr& list, CECOpNode* fatherNode);
    
/*
The Destructor

*/
    virtual ~CECOpNodeAbstraction();
    
/*
A ~set~-function that checks whether the node is the root node from the compiled expression.

This function overrides the function of the base class ~CECOpNode~.

*/
    void checkIsCERootNode(bool force = false);
    
/*
A ~get~-function returned a pointer to the ~CEGenerateFunctionStatus~-object of the node.

*/
    CEGenerateFunctionStatus* getInstCEGenerateFunctionStatus();

/*
This function replaces the annotated list of the node with an empty list.   

*/
    void deleteCEAnnotateList();

/*
This function generates a unique function name, which is used in the shared library
to be generated.

This function overrides the function of the base class ~CECOpNode~.

*/
    std::string generateCECallLibFunctionName(int idxCERootNode);

/*
This function generate and returned a unique name of the operator.

*/
    std::string getOperatorName();
    
/*
A ~get~-function returned nested list with the return type of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    ListExpr getListReturnType();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void accept(CECOpTreeVisitor&);
    
/*
This function returned a string with the function nuber of the abstraction function.

*/
    std::string getStrFunctionNo();
    
  private:
/*
A local variable...

*/
    CEGenerateFunctionStatus* ptrCEGenFuncStat;

/*
The Standard Constructor

*/
    CECOpNodeAbstraction();

/*
This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool checkNListStructureOK();
  }; //end of class CECOpNodeAbstraction
  
  
/*
11 Class ~CECOpNodeObject~

This class is a subclass of ~CECOpNode~ and ~CECPtrCodeStoreType~. It implements
the node that is formed from the following annotated list:

----        
              ((<object name> object <index>) <type>)
                
----

*/
  class CECOpNodeObject : public CECOpNode, public CECPtrCodeStoreType {    
  public:
/*
The Constructor

*/
    CECOpNodeObject(ListExpr& list, CECOpNode* fatherNode);
    
/*
The Destructor

*/
    virtual ~CECOpNodeObject();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void accept(CECOpTreeVisitor&);
    
/*
This function returned a string with the index of the represented object in the array of values
from the ~Secondo QueryProcessor~.

*/
    std::string getQPValueIdx();
    
/*
This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void getSignatureVector(std::vector<CECGImplSecondoType*>& sVector,
                            bool callRecursiv);
    
  private:
/*
The Standard Constructor

*/
    CECOpNodeObject();

/*
This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool checkNListStructureOK();
    
  }; //end of class CECOpNodeObject
  
  
/*
12 Class ~CECOpNodeConstant~

This class is a subclass of ~CECOpNode~ and ~CECPtrCodeStoreType~. It implements
the node that is formed from the following annotated list:

----        
              ((<value> constant <index>) <type>)
                
----

*/
  class CECOpNodeConstant : public CECOpNode, public CECPtrCodeStoreType {    
  public:
/*
The Constructor

*/
    CECOpNodeConstant(ListExpr& list, CECOpNode* fatherNode);
    
/*
The Destructor

*/
    virtual ~CECOpNodeConstant();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void accept(CECOpTreeVisitor&);
    
/*
This function returned a string with the index of the represented object in the array of values
from the ~Secondo QueryProcessor~.

*/
    std::string getQPValueIdx();
    
/*
This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void getSignatureVector(std::vector<CECGImplSecondoType*>& sVector,
                            bool callRecursiv);
    
  private:
/*
The Standard Constructor

*/
    CECOpNodeConstant();

/*
This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool checkNListStructureOK();
    
  }; //end of class CECOpNodeConstant
  
  
/*
13 Class ~CECOpNodeVariable~

This class is a subclass of ~CECOpNode~ and ~CECPtrCodeStoreType~. It implements
the node that is formed from the following annotated list:

----        
              ((<var name> variable <position> <functionno>) <type>)
                
----

*/
  class CECOpNodeVariable : public CECOpNode, public CECPtrCodeStoreType {    
  public:
/*
The Constructor

*/
    CECOpNodeVariable(ListExpr& list, CECOpNode* fatherNode);
    
/*
The Destructor

*/
    virtual ~CECOpNodeVariable();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void accept(CECOpTreeVisitor&);
    
/*
This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void getSignatureVector(std::vector<CECGImplSecondoType*>& sVector,
                            bool callRecursiv);
    
/*
This function returned if the represented variable is a stream.

*/
    bool isStreamElem();
    
/*
This function returned if the represented variable is a tuple.

*/
    bool isTupleElem();
    
/*
This function returned a string with the position of the represented variable in the array of values
from the ~Secondo QueryProcessor~.

*/
    std::string getStrVarPosition();
    
  private:
/*
The Standard Constructor

*/
    CECOpNodeVariable();

/*
This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool checkNListStructureOK();
    
  }; //end of class CECOpNodeVariable
  
  
/*
14 Class ~CECOpNodeIdentifier~

This class is a subclass of ~CECOpNode~. It implements
the node that is formed from the following annotated list:

----        
              ((<ident> identifier) <ident>)
                
----

*/
  class CECOpNodeIdentifier : public CECOpNode {    
  public:
/*
The Constructor

*/
    CECOpNodeIdentifier(ListExpr& list, CECOpNode* fatherNode);
    
/*
The Destructor

*/
    virtual ~CECOpNodeIdentifier();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void accept(CECOpTreeVisitor&);
    
  private:
/*
The Standard Constructor

*/
    CECOpNodeIdentifier();

/*
This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool checkNListStructureOK();
    
  }; //end of class CECOpNodeIdentifier
  


/*
15 Class ~CECOpNodeArglist~

This class is a subclass of ~CECOpNode~. It implements
the node that is formed from the following annotated list:

----        
              ((none arglist (ann(t1) ann(t2) ... ann(tn)))
                                        (type(t1) type(t2) ... type(tn)))
                
----

*/
  class CECOpNodeArglist : public CECOpNode {    
  public:
/*
The Constructor

*/
    CECOpNodeArglist(ListExpr& list, CECOpNode* fatherNode);
    
/*
The Destructor

*/
    virtual ~CECOpNodeArglist();
    
/*
A ~get~-function returned ~TRUE~ or ~FALSE~ if the node is a operator, 
that is also implemented in the ~Compiled Expressions Algebra~.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool getIsCEAOperator(bool onlyTrue);
    
/*
This function generates a unique function name, which is used in the shared library
to be generated.

This function overrides the function of the base class ~CECOpNode~.

*/
    std::string generateCECallLibFunctionName(int idxCERootNode);

/*
A ~set~-function that checks to which form the annotated list, associated with the node,
must be converted for the ~Secondo QueryProcessor~.

This function overrides the function of the base class ~CECOpNode~.

*/
    void checkCECGenerateAnnotateList();
    
/*
The function converts the annotated list from the node into a form that
the ~Secondo QueryProcessor~ can process.

This function overrides the function of the base class ~CECOpNode~.

*/
    void generateCEAnnotateList();
    
/*
This function returned if the function in the generated shared library in that this
node inserted are correctly loaded.

*/
    bool searchCELibFuncLoadOK();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void accept(CECOpTreeVisitor&);
    
  private:
/*
The Standard Constructor

*/
    CECOpNodeArglist();

/*
This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool checkNListStructureOK();
  }; //end of class CECOpNodeArglist
  

/*
16 Class ~CECOpNodeFunction~

This class is a subclass of ~CECOpNode~ and ~CECPtrCodeStoreType~. It implements
the node that is formed from the following annotated list:

----        
              ((none function <functionlist>) <resulttype>)
                
----

*/
  class CECOpNodeFunction : public CECOpNode, public CECPtrCodeStoreType {    
  public:
/*
The Constructor

*/
    CECOpNodeFunction(ListExpr& list, CECOpNode* fatherNode);
    
/*
The Destructor

*/
    virtual ~CECOpNodeFunction();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void accept(CECOpTreeVisitor&);
    
/*
This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void getSignatureVector(std::vector<CECGImplSecondoType*>& sVector,
                            bool callRecursiv);
    
  private:
/*
The Standard Constructor

*/
    CECOpNodeFunction();

/*
This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool checkNListStructureOK();
    
  }; //end of class CECOpNodeFunction
  

/*
17 Class ~CECOpNodeApplyabs~

This class is a subclass of ~CECOpNode~ and ~CECPtrCodeStoreType~. It implements
the node that is formed from the following annotated list:

----        
              ((none applyabs (ann(abstraction) ann(arg1) ... ann(argn)))
        <resulttype>)
                
----

*/
  class CECOpNodeApplyabs : public CECOpNode, public CECPtrCodeStoreType {    
  public:
/*
The Constructor

*/
    CECOpNodeApplyabs(ListExpr& list, CECOpNode* fatherNode);
    
/*
The Destructor

*/
    virtual ~CECOpNodeApplyabs();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void accept(CECOpTreeVisitor&);
    
/*
This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void getSignatureVector(std::vector<CECGImplSecondoType*>& sVector,
                            bool callRecursiv);
    
  private:
/*
The Standard Constructor

*/
    CECOpNodeApplyabs();

/*
This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool checkNListStructureOK();
    
  }; //end of class CECOpNodeApplyabs
  

/*
18 Class ~CECOpNodeApplyfun~

This class is a subclass of ~CECOpNode~ and ~CECPtrCodeStoreType~. It implements
the node that is formed from the following annotated list:

----        
              ((none applyfun (ann(function) ann(arg1) ... ann(argn)))
                <resulttype>)
                
----

*/
  class CECOpNodeApplyfun : public CECOpNode, public CECPtrCodeStoreType {    
  public:
/*
The Constructor

*/
    CECOpNodeApplyfun(ListExpr& list, CECOpNode* fatherNode);
    
/*
The Destructor

*/
    virtual ~CECOpNodeApplyfun();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void accept(CECOpTreeVisitor&);
    
/*
This function fill in the vector ~sVector~ pointer from
the ~CECGImplSecondoTypeXXX~-objects so that the vector contains the complete 
return types as the signature of the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void getSignatureVector(std::vector<CECGImplSecondoType*>& sVector,
                            bool callRecursiv);
    
  private:
/*
The Standard Constructor

*/
    CECOpNodeApplyfun();

/*
This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool checkNListStructureOK();
    
  }; //end of class CECOpNodeApplyfun
  

/*
19 Class ~CECOpNodeCounterdef~

This class is a subclass of ~CECOpNode~. It implements
the node that is formed from the following annotated list:

----        
        ((none counterdef <idx> ann(subexpr))
          <resulttype>)
                
----

*/
  class CECOpNodeCounterdef : public CECOpNode {    
  public:
/*
The Constructor

*/
    CECOpNodeCounterdef(ListExpr& list, CECOpNode* fatherNode);
    
/*
The Destructor

*/
    virtual ~CECOpNodeCounterdef();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void accept(CECOpTreeVisitor&);
    
  private:
/*
The Standard Constructor

*/
    CECOpNodeCounterdef();

/*
This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool checkNListStructureOK();
  }; //end of class CECOpNodeCounterdef
  

/*
20 Class ~CECOpNodePointer~

This class is a subclass of ~CECOpNode~. It implements
the node that is formed from the following annotated list:

----        
        (pointer <memorx address>)
                
----

*/
  class CECOpNodePointer : public CECOpNode {    
  public:
/*
The Constructor

*/
    CECOpNodePointer(ListExpr& list, CECOpNode* fatherNode);
    
/*
The Destructor

*/
    virtual ~CECOpNodePointer();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void accept(CECOpTreeVisitor&);
    
  private:
/*
The Standard Constructor

*/
    CECOpNodePointer();

/*
This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool checkNListStructureOK();
    
  }; //end of class CECOpNodePointer
  

/*
21 Class ~CECOpNodePredinfodef~

This class is a subclass of ~CECOpNode~. It implements
the node that is formed from the following annotated list:

----        
        ((none predinfodef 0.012 0.1442 ann(subexpr))
          <resulttype>)
                
----

*/
  class CECOpNodePredinfodef : public CECOpNode {    
  public:
/*
The Constructor

*/
    CECOpNodePredinfodef(ListExpr& list, CECOpNode* fatherNode);
    
/*
The Destructor

*/
    virtual ~CECOpNodePredinfodef();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void accept(CECOpTreeVisitor&);
    
  private:
/*
The Standard Constructor

*/
    CECOpNodePredinfodef();

/*
This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool checkNListStructureOK();
  }; //end of class CECOpNodePredinfodef
  

/*
22 Class ~CECOpNodeMemorydef~

This class is a subclass of ~CECOpNode~. It implements
the node that is formed from the following annotated list:

----        
        ((none memorydef 512 ann(subexpr))
          <resulttype>)
                
----

*/
  class CECOpNodeMemorydef : public CECOpNode {    
  public:
/*
The Constructor

*/
    CECOpNodeMemorydef(ListExpr& list, CECOpNode* fatherNode);
    
/*
The Destructor

*/
    virtual ~CECOpNodeMemorydef();
    
/*
This function handles all ~CECOpTreeVisitorXXX~-objects who visit the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    void accept(CECOpTreeVisitor&);
    
  private:
/*
The Standard Constructor

*/
    CECOpNodeMemorydef();

/*
This function checks the correct form of the annotated list from the node.

This function overrides the function of the base class ~CECOpNode~.

*/
    bool checkNListStructureOK();
  }; //end of class CECOpNodeMemorydef
  


/*
23 Class ~CECOpTreeVisitor~

This class is a  abstract base class for all vistor classes with which the operator tree
is traversed to collect the necessary information for code generation.

*/
  class CECOpTreeVisitor {
  public:
/*
The Constructor

*/
    CECOpTreeVisitor() {};
    
/*
The Destructor

*/
    virtual ~CECOpTreeVisitor() {};
    
/*
The following functions are the abstract functions, in which the respective behavior
for the specific ~CECOpTreeVisitorXXX~ will implemented for the specific visitors for each
node type of the operator tree.

*/
    virtual void visit(CECOpNode&) = 0;
    virtual void visit(CECOpNodeOperator&) = 0;
    virtual void visit(CECOpNodeApplyop&) = 0;
    virtual void visit(CECOpNodeAbstraction&) = 0;
    virtual void visit(CECOpNodeObject&) = 0;
    virtual void visit(CECOpNodeConstant&) = 0;
    virtual void visit(CECOpNodeVariable&) = 0;
    virtual void visit(CECOpNodeIdentifier&) = 0;
    virtual void visit(CECOpNodeArglist&) = 0;
    virtual void visit(CECOpNodeFunction&) = 0;
    virtual void visit(CECOpNodeApplyabs&) = 0;
    virtual void visit(CECOpNodeApplyfun&) = 0;
    virtual void visit(CECOpNodeCounterdef&) = 0;
    virtual void visit(CECOpNodePointer&) = 0;
    virtual void visit(CECOpNodePredinfodef&) = 0;
    virtual void visit(CECOpNodeMemorydef&) = 0;
  }; //end of class CECOpTreeVisitor
  
  
/*
24 Class ~CECOpTreeVisitorModifyAnnotateListFormQP~

This class is a subclass of ~CECOpTreeVisitor~. This visitor traversed the operator tree
and modified the annotate list in the format of the ~Secondo QueryProcessor~.

*/
  class CECOpTreeVisitorModifyAnnotateListFormQP : public CECOpTreeVisitor {
  public:
/*
The Constructor

*/
    CECOpTreeVisitorModifyAnnotateListFormQP();
    
/*
The Destructor

*/
    ~CECOpTreeVisitorModifyAnnotateListFormQP();
    
    
/*
The following functions implement the visitor's behavior for each node type of the operator tree.

These functions implement the abstract functions of the base class ~CECOpTreeVisitor~.

*/
    void visit(CECOpNode&);
    void visit(CECOpNodeOperator&);
    void visit(CECOpNodeApplyop&);
    void visit(CECOpNodeAbstraction&);
    void visit(CECOpNodeObject&);
    void visit(CECOpNodeConstant&);
    void visit(CECOpNodeVariable&);
    void visit(CECOpNodeIdentifier&);
    void visit(CECOpNodeArglist&);
    void visit(CECOpNodeFunction&);
    void visit(CECOpNodeApplyabs&);
    void visit(CECOpNodeApplyfun&);
    void visit(CECOpNodeCounterdef&);
    void visit(CECOpNodePointer&);
    void visit(CECOpNodePredinfodef&);
    void visit(CECOpNodeMemorydef&);

  }; //end of class CECOpTreeVisitorModifyAnnotateListFormQP
  
  
/*
25 Class ~CECOpTreeVisitorSearchCompiledExpressionRoot~

This class is a subclass of ~CECOpTreeVisitor~. This visitor traversed the operator tree
and searched for all nodes that represent a root node of a compiled expressions.

*/
  class CECOpTreeVisitorSearchCompiledExpressionRoot : public CECOpTreeVisitor {
  public:
/*
The Constructor

*/
    CECOpTreeVisitorSearchCompiledExpressionRoot();
    
/*
The Destructor

*/
    ~CECOpTreeVisitorSearchCompiledExpressionRoot();
    

/*
The following functions implement the visitor's behavior for each node type of the operator tree.

These functions implement the abstract functions of the base class ~CECOpTreeVisitor~.

*/
    void visit(CECOpNode&);
    void visit(CECOpNodeOperator&);
    void visit(CECOpNodeApplyop&);
    void visit(CECOpNodeAbstraction&);
    void visit(CECOpNodeObject&);
    void visit(CECOpNodeConstant&);
    void visit(CECOpNodeVariable&);
    void visit(CECOpNodeIdentifier&);
    void visit(CECOpNodeArglist&);
    void visit(CECOpNodeFunction&);
    void visit(CECOpNodeApplyabs&);
    void visit(CECOpNodeApplyfun&);
    void visit(CECOpNodeCounterdef&);
    void visit(CECOpNodePointer&);
    void visit(CECOpNodePredinfodef&);
    void visit(CECOpNodeMemorydef&);
    
/*
A ~set~-function that saved a found root node in a local data structure.

*/
    void setCECOpTreeRootNode(CECOpNode& node);

/*
A ~get~-function returned the ~idx~'th root node.

*/
    CECOpNodeAbstraction* getRootNode(unsigned int idx);
    
/*
This function returned the number of found root node.

*/
    int sizeRootNodes();
    
  private:
/*
Different local variables...

*/
    std::vector<CECOpNodeAbstraction*> rootNodes;
    CECOpNode* cecOpTreeRootNode;
  }; //end of class CECOpTreeVisitorSearchCompiledExpressionRoot
  

/*
26 Class ~CECOpTreeVisitorNumberingCECOpNodes~

This class is a subclass of ~CECOpTreeVisitor~. This visitor traversed the operator tree
and assigns a unique number to each node. This number will be used for the generation of
unique class names in code generation phase.

*/
  class CECOpTreeVisitorNumberingCECOpNodes : public CECOpTreeVisitor {
  public:
/*
The Constructor

*/
    CECOpTreeVisitorNumberingCECOpNodes();
    
/*
The Destructor

*/
    ~CECOpTreeVisitorNumberingCECOpNodes();
    
    
/*
The following functions implement the visitor's behavior for each node type of the operator tree.

These functions implement the abstract functions of the base class ~CECOpTreeVisitor~.

*/
    void visit(CECOpNode&);
    void visit(CECOpNodeOperator&);
    void visit(CECOpNodeApplyop&);
    void visit(CECOpNodeAbstraction&);
    void visit(CECOpNodeObject&);
    void visit(CECOpNodeConstant&);
    void visit(CECOpNodeVariable&);
    void visit(CECOpNodeIdentifier&);
    void visit(CECOpNodeArglist&);
    void visit(CECOpNodeFunction&);
    void visit(CECOpNodeApplyabs&);
    void visit(CECOpNodeApplyfun&);
    void visit(CECOpNodeCounterdef&);
    void visit(CECOpNodePointer&);
    void visit(CECOpNodePredinfodef&);
    void visit(CECOpNodeMemorydef&);
    
  private:
/*
A local variable...

*/
    int nextNodeNumber;
  }; //end of class CECOpTreeVisitorNumberingCECOpNodes
  

/*
27 Class ~CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY~

This class is a subclass of ~CECOpTreeVisitor~. This visitor traversed the operator tree
and copies the annotated list of the return types from each node to the ~CEQuery~-object.

*/
  class CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY
  : public CECOpTreeVisitor {
  public:
/*
The Constructor

*/
    CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY();
    
/*
The Destructor

*/
    ~CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQY();
    
    
/*
The following functions implement the visitor's behavior for each node type of the operator tree.

These functions implement the abstract functions of the base class ~CECOpTreeVisitor~.

*/
    void visit(CECOpNode&);
    void visit(CECOpNodeOperator&);
    void visit(CECOpNodeApplyop&);
    void visit(CECOpNodeAbstraction&);
    void visit(CECOpNodeObject&);
    void visit(CECOpNodeConstant&);
    void visit(CECOpNodeVariable&);
    void visit(CECOpNodeIdentifier&);
    void visit(CECOpNodeArglist&);
    void visit(CECOpNodeFunction&);
    void visit(CECOpNodeApplyabs&);
    void visit(CECOpNodeApplyfun&);
    void visit(CECOpNodeCounterdef&);
    void visit(CECOpNodePointer&);
    void visit(CECOpNodePredinfodef&);
    void visit(CECOpNodeMemorydef&);
    
/*
This function copies the annotated list of the return types from each node to
the ~CEQuery~-object.

*/
    void setCEQuery(CEQuery* ceQY);
    
    
  private:
    CEQuery* ptrCEQY;
  }; //end of class CECOpTreeVisitorCopyCECOpNodesResultTypeToCEQP
  

/*
28 Class ~CECompiler~

This class implements the concrete compiler. The class is a singleton.

*/
  class CECompiler {    
  public:
/*
This static function returned a pointer of the ~CECompiler~-instance. 
If no instance has been created, the function creates an instance.

*/
    static CECompiler* getInstance();
    
/*
This static function deleted the ~CECompiler~-instance. 

*/
    static void deleteInstance();
    
/*
The Destructor

*/
    virtual ~CECompiler();
    
    
/*
This function is the central function of the ~CECompiler~ in which the
shared library is generated from the annotated list and loaded via
the ~CEQueryProcessor~.

*/
    void ceGenerateQuery(ListExpr& annotateListCE, CEQuery* ptrCEQY);
    
  private:
/*
A static pointer variable...

*/
    static CECompiler* instance;
    
/*
The Constructor

*/
    CECompiler();
    
/*
This function is called as soon as something is wrong during the creation or loading
of the shared library. The annotated list is then changed to the format of
the ~Secondo QueryProcessor~ so that the ~Secondo QueryProcessor~ can process the query
without the functionality of the ~Compiled Expressions Algebra~.

*/
    void resetAnnotateListToOrigForm(ListExpr& list, bool output);
    
  }; // end of class CECompiler
  
  
} // end of namespace CompiledExpressions
#endif // _COMPILED_EXPRESSIONS_COMPILER_H_