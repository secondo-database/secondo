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

[10] Header file of the CEQueryProcessor

2016/2017 H.Brieschke created the new CompiledExpression Algebra


[TOC]


1 Overview 

In this header file, 3 classes are declared.

  * CEQRuntimeError
  
  * CEQuery
  
  * CEQueryProcessor

The ~CEQRuntimeError~-class implements ~ERROR~-objects, which are required for the error treatment.
The ~CEQuery~-class implements a data structure in which all information required for the processing
of a data request is stored over the entire working flow.
The ~CEQueryProcessor~-class implements the interface to the ~QueryProcessor~ of the secondo kernel.


2 Defines, includes, and constants

*/
#ifndef _COMPILED_EXPRESSIONS_QUERY_PROCESSOR_H_
#define _COMPILED_EXPRESSIONS_QUERY_PROCESSOR_H_

//define macro TRACE_ON if trace outputs are needed
//#define TRACE_ON
#undef TRACE_ON

#include "AlgebraTypes.h"
#include "NestedList.h"
#include "DynamicLibrary.h"
#include "CECodeGenerator.h"

extern QueryProcessor* ptrQP;

// forward declarations
struct OpNode;
typedef OpNode* OpTree;

typedef bool (*FuncInitCompiledExpressionsLib_t)(CompiledExpressions::CEQuery*);
typedef void (*FuncCloseCompiledExpressionsLib_t)();


namespace CompiledExpressions {
// forward declarations from classes in the CompiledExpressions-namespace
  class CECOpTreeVisitorGenerateCode;

/*
3 Class ~CEQRuntimeError~

This class implements ~ERROR~-objects, which are required for the error treatment.

*/
  class CEQRuntimeError : public std::exception {
  public:
    CEQRuntimeError(std::string s);
    virtual ~CEQRuntimeError() throw() {}
    virtual const char * what() const throw();
  private:
    std::string errMsg;
  };
  
/*
4 Class ~CEQuery~

This class implements a data structure in which all information required for the processing
of a data request is stored over the entire working flow. For each query, a new object will
be created which will be deleted after completion of the query processing.

*/
  class CEQuery {
  public:

/*
The Constructor.

*/
   CEQuery(bool);

/*
The Destructor.

*/
    virtual ~CEQuery();
    
/*
A function-pointer to the ~setPtrQueryProcessors~-function. This functionpointer is used
from outside the algebra library.

*/
    void (CEQuery::*FuncPtrSetPtrQueryProcessors)(QueryProcessor*,
                                                  CEQueryProcessor*);

/*
A ~set~-function for the pointers of the Secondo-~QueryProcessor~ and the ~CEQueryProcessor~.

*/
    void setPtrQueryProcessors(QueryProcessor* ptrQP,
                               CEQueryProcessor* ptrCEQP);
/*
A ~get~-function for the pointers of the Secondo-~QueryProcessor~.

*/
    QueryProcessor* getPtrQP();
/*
A ~get~-function for the pointers of the ~CEQueryProcessor~.

*/
    CEQueryProcessor* getPtrCEQP();
    
/*
A function-pointer to the ~getUseCEQP~-function. This functionpointer is used
from outside the algebra library.

*/
    bool (CEQuery::*FuncPtrGetUseCEQP)();

/*
This function specifies whether the functionality of the ~CEQueryProcessor~
should be used.

*/
    bool getUseCEQP();
/*
A ~set~-function for the path of the actual working directory and the directory
in that the code are generated and the shared library are generated.

*/
    void setCompilePath(std::string current,
                        std::string codeGen);
/*
A ~get~-function returned the name of the actual working directory.

*/
    std::string getCurrentPath();
/*
A ~get~-function returned the name of the directory in which the code are
generated and the shared library are generated.

*/
    std::string getCodeGenPath();
/*
This ~set~-function is called when the shared library is created successfully.

*/
    void setGeneratedQuery();
/*
A ~get~-function returned if the shared library is created successfully.

*/
    bool getGeneratedQuery();
    
/*
This function returned a ~CECOpTreeVisitorGenerateCode~-object.

*/
    CECOpTreeVisitorGenerateCode& getCodeGenVisitor();

    //void loadCompiledLibrary(std::string libName) throw (CEQRuntimeError);

/*
This function returned a pointer to the ~DynamicLibrary~-object, which
handles the generated shared library.

*/
    DynamicLibrary* getPtrCEQYLibrary();
/*
A ~set~-function for the function pointer of the function that is called
to close the generated shared library.

*/
    void setFuncCloseCompiledExpressionsLib(void*&);
    
/*
This function creates a map in which the nested list is assigned the return type
for each node key in the operator tree from the ~CECompiler~.

*/
    void addMapResultTypes(std::string ceNodeKey,
                           ListExpr list);
/*
A ~get~-function returned the nested list  is assigned the return type for
the given node key.

*/
    ListExpr getMapResultTypes(std::string ceNodeKey);

  
  private:
    QueryProcessor* ptrQP;
    CEQueryProcessor* ptrCEQP;
    bool useCEQP;
    bool delTempFiles;
    bool isGeneratedQuery;
    std::string currentPath;
    std::string codeGenPath;
    CECOpTreeVisitorGenerateCode* ptrCodeGenVisitor;
    DynamicLibrary* ceLibrary;
    FuncCloseCompiledExpressionsLib_t ptrFuncCloseCELib;
    
    std::map<std::string, ListExpr> mapResultTypes;
  
/*
The  standard constructor.

*/
    CEQuery(){}
  };
  

/*
5 Class ~CEQueryProcessor~

This class implements the interface to the ~QueryProcessor~ of the secondo kernel.

*/
  class CEQueryProcessor {
  public:
/*
The Constructor.

*/
    CEQueryProcessor(QueryProcessor*);
/*
The Destructor.

*/
    virtual ~CEQueryProcessor() {};
    
/*
This function returns whether the passed ~algID~ is that of
the ~CompiledExpressionsAlgebra~.

*/
    bool isCEAlgebra(int algID);
    
/*
A function-pointer to the ~TestOverloadedOperators~-function. This functionpointer
is used from outside the algebra library.

*/
    ListExpr (CEQueryProcessor::*FuncPtrTestOverloadedOperators)(
      const std::string& operatorSymbolStr, 
      ListExpr opList, 
      ListExpr typeList, 
      ListExpr typeArgList, 
      int& alId, 
      int& opId, 
      int& opFunId, 
      bool checkFunId, 
      bool traceMode,
      ListExpr& resultAlgList,
      ListExpr& resultFunIdList,
      CEQuery* ptrCEQY);

/*
The adapted ~TestOverloadedOperators~-function from the ~Secondo QueryProcessor~.

*/
    ListExpr TestOverloadedOperators(
      const std::string& operatorSymbolStr, 
      ListExpr opList, 
      ListExpr typeList, 
      ListExpr typeArgList, 
      int& alId, 
      int& opId, 
      int& opFunId, 
      bool checkFunId, 
      bool traceMode,
      ListExpr& resultAlgList,
      ListExpr& resultFunIdList,
      CEQuery* ptrCEQY);
    
/*
A function-pointer to the ~generateApplyopList~-function. This functionpointer
is used from outside the algebra library.

*/
    void (CEQueryProcessor::*FuncPtrGenerateApplyopList)(
      ListExpr& applyopList,
      ListExpr first,
      ListExpr list,
      ListExpr resultAlgList,
      ListExpr resultFunIdList,
      ListExpr resultType);

/*
The adapted ~generateApplyopList~-functionality included in 
the ~Secondo QueryProcessor Annotate~-function.

*/
    void generateApplyopList(
      ListExpr& applyopList,
      ListExpr first,
      ListExpr list,
      ListExpr resultAlgList,
      ListExpr resultFunIdList,
      ListExpr resultType);

    
/*
A function-pointer to the ~GenerateCompiledExpressions~-function. This functionpointer
is used from outside the algebra library.

*/
    void (CEQueryProcessor::*FuncPtrGenerateCompiledExpressions)(
      CEQuery* ceQuery,
      ListExpr& list);
    
/*
This function calls the ~CECompiler~ to generate the shared library with the query.

*/
    void GenerateCompiledExpressions(
      CEQuery* ceQuery,
      ListExpr& list);
    
/*
This function returns whether the debug mode is set.

*/
    bool getQPDebugMode();
/*
This function returns whether the trace mode is set.

*/
    bool getQPTraceMode();

/*
The function loaded the generated shared library.

*/
    void loadCompiledExpressions(std::string libName, CEQuery* ptrCEQY)
        throw (CEQRuntimeError);

/*
The function generates a list that contains the memory address of the functions
to be called by the operators in the generated shared library.

*/
    bool createLibFunctionAddressNList(ListExpr& ptrListFunction,
                                    ListExpr& returnType,
                                    const std::string& functionName,
                                    CEQuery* ptrCEQY);
  
/*
This function returns the value with the index ~idx~ from the ~Secondo QueryProcessor~.

*/
    Word getQPValues(unsigned int idx);
/*
This function returns the result storage from the given supplier.

*/
    Word qpResultStorage(const Supplier s);

/*
This function returns whether the given supplier is a pointer node.

*/
    bool IsPointerNode( const Supplier s ) const;
/*
A ~get~-function returned the nested list is assigned the return type for
the given node key.

*/
    ListExpr GetTupleResultType(std::string ceNodeKey, CEQuery* ceQY);
    
  private:
    QueryProcessor* ptrQP;
    
/*
The  standard constructor.

*/
    CEQueryProcessor() {};
/*
This function extracts the main type from the given type in the nested list ~args~.
This function is copied from the ~QueryProcessor.cpp~.

*/
    std::string getMainTypesAsString(ListExpr args);
  };

}
#endif /* _COMPILED_EXPRESSIONS_QUERY_PROCESSOR_H_ */


/*
6 External call functions

Definition of the external call functions.

*/
#ifdef _COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_
#ifdef _COMPILED_EXPRESSIONS_ALGEBRA_STATIC_
#ifdef __cplusplus
extern "C"{
#endif

/*
The external call function generate a new ~CEQuery~-object and
returned a pointer of them.

*/
  CompiledExpressions::CEQuery* FuncCEQYGetNewInstance(bool);
  
/*
The external call function generate a new ~CEQueryProcessor~-object and
returned a pointer of them.

*/
  CompiledExpressions::CEQueryProcessor* 
  FuncCEQPGetNewCEQueryProcessor(QueryProcessor*);

#ifdef __cplusplus
}
#endif

#endif //_COMPILED_EXPRESSIONS_ALGEBRA_STATIC_
#endif //_COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_
