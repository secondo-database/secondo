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

[10] Implementation file of the CEQueryProcessor

2016/2017 H.Brieschke created the new CompiledExpression Algebra


[TOC]


1 Overview 

In this file the following 3 classes are implemented.

  * CEQRuntimeError
  
  * CEQuery
  
  * CEQueryProcessor

The ~CEQRuntimeError~-class implements ~ERROR~-objects, which are required for the error treatment.
The ~CEQuery~-class implements a data structure in which all information required for the processing
of a data request is stored over the entire working flow.
The ~CEQueryProcessor~-class implements the interface to the ~QueryProcessor~ of the secondo kernel.


2 Defines, includes, and constants

*/

#include <set>
#include <string>
#include <algorithm>
#include <stdlib.h>

#include <fstream>
#include <iostream>


#include "NestedList.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include "Symbols.h"
#include "Trace.h"
#include "StandardTypes.h"
#include "FileSystem.h"

#include "./CEQueryProcessor.h"
#include "./CompiledExpressionsAlgebra.h"
#include "./CECompiler.h"
#include "./CECodeGenerator.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager *am;

using namespace std;

typedef Operator* OpPtr;

/*
3 Copied code from the ~QueryProcessor.cpp~-File


The following code has been copied from the ~QueryProcessor.cpp~-File because the
enum ~OpNodeType~ and struct ~OpNode~ is not declared in a header file, but in
the ~QueryProcessor.cpp~-file and the ~QueryProcessor.cpp~-file would be necessary to access
this data structure. Due to this code coupling, however, in return, if the original code is
changed, the following code must also be adapted. The current code is identical to the code
version, which was implemented in January 2017.

3.1 Enum ~OpNodeType~

In addition, the enum ~OpNodeType~ has been copied from the ~QueryProcessor.cpp~-file
because it is not declared in a header file, but in the implementation file.

*/
enum OpNodeType { Pointer, Object, IndirectObject, Operator };
/*
3.2 Struct ~OpNode~

In addition, the struct "OpNode" has been copied from the ~QueryProcessor.cpp~-file because
it is not declared in a header file, but in the implementation file, which means that access
to this data structure would not be possible.

*/
struct OpNode
{
  bool         evaluable;
  ListExpr     typeExpr;
  ListExpr     numTypeExpr;
  const OpNodeType   nodetype;
  int          id;
  bool         isRoot;
  ArgVector    tmpArg;
  struct OpNodeStruct
  {
  /*
     common members of OpNodeDirecObject and OpNodeOperator ~symbol~
     common members of OpNodeInDirecObject and OpNodeOperator ~received~

  */
    ListExpr symbol;
    bool received;
    struct OpNodeDirectObject
    {
      Word value;
      int  valNo; // needed for testing only
      bool isConstant;
      bool isModified;
    } dobj;
    struct OpNodeIndirectObject
    {
      ArgVectorPointer vector;
      int funNumber;            /* needed for testing only */
      int argIndex;

/*
The three attributes below are used in the case when an
operator has a parameter function which may have streams as
arguments. In this case the operator must

*/
    } iobj;
    struct OpNodeOperator
    {
      int algebraId;
      int opFunId;
      int noSons;
      ValueMapping valueMap;
      OpPtr theOperator;
      ArgVector sons;
      ArgVector sonresults;
      bool isFun;
      ArgVectorPointer funArgs;
      int funNo; // needed for testing only
      bool isStream;
      Word local;
      Word local2; // can be used for init/finish messages
      int resultAlgId;
      int resultTypeId;
      Word resultWord;
      ObjectDeletion deleteFun; // substitute for the algebras
                                // delete function
      int counterNo;
      double selectivity;       //these two fields can be used by operators
      double predCost;          //implementing predicates to get and set
                                //such properties, e.g. for progress est.
      bool supportsProgress;
      bool usesMemory;          // true if the operator uses a memory buffer
      size_t memorySize;        // amount of memory assigned to this operator
                                // in MB
      CostEstimation* costEstimation;
      bool argsAvailable;
    } op;
  } u;

};

/*
End of copied code from the ~QueryProcessor.cpp~-File.

*/
using namespace CompiledExpressions;
namespace CompiledExpressions {

/*
4 Class ~CEQRuntimeError~

This class implements ~ERROR~-objects, which are required for the error treatment.

*/
  CEQRuntimeError::CEQRuntimeError(std::string s) {
    errMsg = s;
  }
  
  const char*
  CEQRuntimeError::what() const throw() {
    return errMsg.c_str();
  }
  

/*
5 Class ~CEQuery~

This class implements a data structure in which all information required for the processing
of a data request is stored over the entire working flow. For each query, a new object will
be created which will be deleted after completion of the query processing.

5.1 The Constructor

*/
  CEQuery::CEQuery(bool use)
  : ptrQP(0),
    ptrCEQP(0),
    useCEQP(use),
    delTempFiles(true),
    isGeneratedQuery(false),
    ptrCodeGenVisitor(new CECOpTreeVisitorGenerateCode()),
    ceLibrary(new DynamicLibrary()),
    ptrFuncCloseCELib(0) {
      FuncPtrGetUseCEQP = &CEQuery::getUseCEQP;
      FuncPtrSetPtrQueryProcessors = &CEQuery::setPtrQueryProcessors;
      
      if (use) {
        std::string cfgFileName = FileSystem::GetCurrentFolder();
        FileSystem::AppendItem(cfgFileName, "..");
        FileSystem::AppendItem(cfgFileName, "Algebras");
        FileSystem::AppendItem(cfgFileName, "CompiledExpressions");
        FileSystem::AppendItem(cfgFileName, "CEARunTime.cfg");

        if (FileSystem::FileOrFolderExists(cfgFileName)) {
          std::string line;
          std::ifstream cfgFileStream;
          bool foundUseCEQP, foundDeleteTempFiles;
          foundUseCEQP = foundDeleteTempFiles = false;
          cfgFileStream.open(cfgFileName.c_str(), ios::in);
          while (!cfgFileStream.eof()
                 && !(foundUseCEQP && foundDeleteTempFiles)) {
            getline(cfgFileStream, line);
            if (!foundUseCEQP && line.find("usedCEA=") == 0) {
              if (line.substr(8, 5) == "FALSE"
                 || line.substr(8, 5) == "false")
                useCEQP = false;
              else
                useCEQP = true;
              foundUseCEQP = true;
            }
    
            if (!foundDeleteTempFiles
                && line.find("deleteTempFiles=") == 0) {
              if (line.substr(16, 5) == "FALSE"
                 || line.substr(16, 5) == "false")
                delTempFiles = false;
              else
                delTempFiles = true;
              foundDeleteTempFiles = true;
            }

          }
          cfgFileStream.close();
          if (!foundUseCEQP)
            useCEQP = true;
          if (!foundDeleteTempFiles)
            delTempFiles = true;
        }
      }
    }
    
/*
5.2 The Destructor

*/
  CEQuery::~CEQuery() {
    if (ptrFuncCloseCELib)
      ptrFuncCloseCELib();
    if (ceLibrary)
      delete ceLibrary;
    if (ptrCodeGenVisitor)
      delete ptrCodeGenVisitor;
    mapResultTypes.clear();
    FileSystem::SetCurrentFolder(currentPath);
    
    if (delTempFiles)
      FileSystem::EraseFolder(codeGenPath);
  }
    
/*
5.3 Function ~setPtrQueryProcessors~

A ~set~-function for the pointers of the Secondo-~QueryProcessor~ and the ~CEQueryProcessor~.

*/
  void
  CEQuery::setPtrQueryProcessors(QueryProcessor* _ptrQP,
                                 CEQueryProcessor* _ptrCEQP) {
    ptrQP = _ptrQP;
    ptrCEQP = _ptrCEQP;
  }
  
/*
5.4 Function ~getPtrQP~

A ~get~-function for the pointers of the Secondo-~QueryProcessor~.

*/
  QueryProcessor*
  CEQuery::getPtrQP() {
    return ptrQP;
  }
  
/*
5.5 Function ~getPtrCEQP~

A ~get~-function for the pointers of the ~CEQueryProcessor~.

*/
  CEQueryProcessor*
  CEQuery::getPtrCEQP() {
    return ptrCEQP;
  }
  
/*
5.6 Function ~setCompilePath~

A ~set~-function for the path of the actual working directory and the directory
in that the code are generated and the shared library are generated.

*/
  void
  CEQuery::setCompilePath(std::string current, std::string codeGen) {
    currentPath = current;
    codeGenPath = codeGen;
  }
  
/*
5.7 Function ~getCurrentPath~

A ~get~-function returned the name of the actual working directory.

*/
  std::string
  CEQuery::getCurrentPath() {
    return currentPath;
  }
  
/*
5.8 Function ~getCurrentPath~

A ~get~-function returned the name of the directory in which the code are
generated and the shared library are generated.

*/
  std::string
  CEQuery::getCodeGenPath() {
    return codeGenPath;
  }
  
/*
5.9 Function ~setGeneratedQuery~

This ~set~-function is called when the shared library is created successfully.

*/
  void
  CEQuery::setGeneratedQuery() {
    isGeneratedQuery = true;
  }
  
/*
5.10 Function ~getGeneratedQuery~

A ~get~-function returned if the shared library is created successfully.

*/
  bool
  CEQuery::getGeneratedQuery() {
    return isGeneratedQuery;
  }
  
/*
5.11 Function ~getUseCEQP~

A ~get~-function returned whether the functionality of the ~CEQueryProcessor~
should be used.

*/
  bool
  CEQuery::getUseCEQP() {
    return useCEQP;
  }
  

/*
5.12 Function ~getCodeGenVisitor~

A ~get~-function returned a ~CECOpTreeVisitorGenerateCode~-object.

*/
  CECOpTreeVisitorGenerateCode&
  CEQuery::getCodeGenVisitor() {
    return *(ptrCodeGenVisitor);
  }
  
/*
5.13 Function ~getPtrCEQYLibrary~

A ~get~-function returned a pointer to the ~DynamicLibrary~-object, which
handles the generated shared library.

*/
  DynamicLibrary*
  CEQuery::getPtrCEQYLibrary() {
    if (!ceLibrary)
      ceLibrary = new DynamicLibrary();
    return ceLibrary;
  }
  
/*
5.14 Function ~setFuncCloseCompiledExpressionsLib~

A ~set~-function for the function pointer of the function that is called
to close the generated shared library.

*/
  void
  CEQuery::setFuncCloseCompiledExpressionsLib(void*& ptrFunction) {
    ptrFuncCloseCELib = (FuncCloseCompiledExpressionsLib_t) ptrFunction;
  }
  
/*
5.15 Function ~addMapResultTypes~

This function creates a map in which the nested list is assigned the return type
for each node key in the operator tree from the ~CECompiler~.

*/
  void
  CEQuery::addMapResultTypes(std::string ceNodeKey, ListExpr list) {
    mapResultTypes[ceNodeKey] = list;
  }
  
/*
5.16 Function ~getMapResultTypes~

A ~get~-function returned the nested list  is assigned the return type for
the given node key. If the node key not found, returned an empty list.

*/
  ListExpr
  CEQuery::getMapResultTypes(std::string ceNodeKey) {
    std::map<std::string, ListExpr>::const_iterator it;
    it = mapResultTypes.find(ceNodeKey);
    if (it != mapResultTypes.end())
      return it->second;
    else
      return nl->TheEmptyList();
  }
  

  
/*
6 Class ~CEQueryProcessor~

This class implements the interface to the original ~QueryProcessor~ of the secondo kernel.
Here, all functions that are required for the functionality of the ~Compiled Expressions Algebra~
and which have not yet existed or whose functionality had to be changed or extended.


6.1 The Constructor

*/
 CEQueryProcessor::CEQueryProcessor(QueryProcessor* _ptrQP) 
  : ptrQP(_ptrQP) {
      FuncPtrTestOverloadedOperators = 
                  &CEQueryProcessor::TestOverloadedOperators;
      FuncPtrGenerateApplyopList = 
                  &CEQueryProcessor::generateApplyopList;
      FuncPtrGenerateCompiledExpressions = 
                  &CEQueryProcessor::GenerateCompiledExpressions;
  }

/*
6.2 Function ~isCEAlgebra~

This function returns whether the passed ~algID~ is that of
the ~CompiledExpressionsAlgebra~.

*/
  bool
  CEQueryProcessor::isCEAlgebra( int algID) {
    int ceAlgebraID = am->GetAlgebraId("CompiledExpressionsAlgebra");
    if ((am->IsAlgebraLoaded(ceAlgebraID)) && (ceAlgebraID == algID)) {
      return true;
    } else {
      return false;
    }  
  }

/*
6.3 Function ~getMainTypesAsString~

This function extracts the main type from the given type in the nested list ~args~.
This function is also copied from the ~QueryProcessor.cpp~-file. The reasons are the
same as indicated above for enum ~OpNodeType~ and struct ~OpNode~. Thus, here too,
a possible change of code in the ~QueryProcessor.cpp~-file must be added here.

*/
  std::string
  CEQueryProcessor::getMainTypesAsString(ListExpr args){
    if(nl->AtomType(args)!=NoAtom){
      return "invalid type list";
    }
    std::stringstream ss;
    ss << "(";
    bool first = true;
    while(!nl->IsEmpty(args)){
      if(!first) ss << ", ";
      ListExpr arg = nl->First(args);    
      args = nl->Rest(args);
      while(nl->AtomType(arg)==NoAtom && !nl->IsEmpty(arg)){
        arg = nl->First(arg);
      }
      ss << nl->ToString(arg);
      first = false; 
    }
    ss << ")";
    return ss.str();
  }


/*
6.4 Function ~TestOverloadedOperators~

This function test all possible type mappings for overloaded operators.
In contrast to the original function in the ~QueryProcessor.cpp~-file, the function
no longer breaks after the first hit, but instead searches for an operator from
the ~Compiled Expressions Algebra~.

*/
  ListExpr
  CEQueryProcessor::TestOverloadedOperators(
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
    CEQuery* ptrCEQY) {

    if (!ptrCEQY->getUseCEQP()) {
      return ptrCEQY->getPtrQP()->TestOverloadedOperators(operatorSymbolStr,
                                                          opList,
                                                          typeList,
                                                          typeArgList,
                                                          alId,
                                                          opId,
                                                          opFunId,
                                                          checkFunId,
                                                          traceMode);
    } else if (!CompiledExpressionsAlgebra::isActiv()) {
      return ptrCEQY->getPtrQP()->TestOverloadedOperators(operatorSymbolStr,
                                                          opList,
                                                          typeList,
                                                          typeArgList,
                                                          alId,
                                                          opId,
                                                          opFunId,
                                                          checkFunId,
                                                          traceMode);
    } else {
      ListExpr resultTypeOrigOp = nl->TheEmptyList();
      ListExpr resultTypeCEAlgOp = nl->TheEmptyList();
      ListExpr resultType = nl->TheEmptyList();
  
      ListExpr origAlgOpList = nl->TheEmptyList();
      ListExpr ceaAlgOpList = nl->TheEmptyList();

      bool foundOrigOp = false;
      bool foundCEAOp = false;
      
      int selFunIdxOrigOp = -1;
  
      static const int width=70;
      static const std::string sepLine = "\n" + std::string(width,'-') + "\n";

      if ( traceMode ) {
        cout << sepLine
            << "Type mapping for operator " << operatorSymbolStr << ": "
            << NList(opList).convertToString() << endl;
      }

      std::string typeErrorMsg =
        "Type map error for operator " + operatorSymbolStr + "!"
        + sepLine
        + wordWrap("Input: ", width, NList(typeList).convertToString())
        + sepLine
        + wordWrap("Short: ", width, getMainTypesAsString(typeList))
        + sepLine + "Error Message(s):" + sepLine;

      do { // Overloading: test operator candidates
        resultType = nl->TheEmptyList();
        alId = nl->IntValue( nl->First( nl->First( opList ) ) );
        opId = nl->IntValue( nl->Second( nl->First( opList ) ) );
        
/*
apply the operator's type mapping:
  (i) standard case: pass the list of types
  (ii) special case: pass the list of pairs (type, argument)
This is for operators who need to see argument expressions within the type mapping,
for example, a filename passed as a string, to get some type information from the
file. This case applies if the operator has registered  "UsesArgumentsInTypeMapping".
It is demonstrated with the filter operator in the relation algebra.

*/
        if ( !am->getOperator( alId, opId )->UsesArgsInTypeMapping() ) {
          resultType = am->TransformType( alId, opId, typeList );
        } else {
          resultType = am->TransformType( alId, opId, typeArgList );
        }
        std::string algName = am->GetAlgebraName(alId);

        if(traceMode) {
          std::stringstream traceMsg;
          traceMsg << algName << ": " << operatorSymbolStr << " (algId="
            << alId << ", opId=" << opId << ") "<< std::ends;

          if( nl->IsEqual( resultType, "typeerror" ) )
            cout  << traceMsg.str() << "rejected!" << endl;
          else
            cout << traceMsg.str() << "accepted!" << endl;
        }

        bool excluded = am->getOperator(alId,opId)->isExcluded();
        if(excluded) {
          resultType = nl->TypeError();
          std::string msg = "";
          ErrorReporter::GetErrorMessage(msg);
          ErrorReporter::ReportError("operator has been excluded (no example)");
        }

        if ( !ErrorReporter::TypeMapError ) {
          std::string msg = "";
          ErrorReporter::GetErrorMessage(msg);
          // remove errors produced by testing operators
          if ( msg == "" )
            msg = "<No error message specified>";
          typeErrorMsg += wordWrap(algName + ": ",4 ,width, msg) + "\n";
        }

        opList = nl->Rest( opList );

        // end of the original while-loop
        if (nl->IsEmpty( opList ) ||
        !nl->IsEqual( resultType, "typeerror" )) {
/*
check if the final result of testing is still a typeerror.
If so save the messages in the error reporter. Errors detected
afterwards will not be reported any more.

*/
          int selFunIndex=-1;
          if ( nl->IsEmpty( opList ) && nl->IsEqual( resultType, "typeerror" )
            && !foundOrigOp ) {
            if(!ErrorReporter::TypeMapError){
              ErrorReporter::ReportError(typeErrorMsg + sepLine);
            }
            ErrorReporter::TypeMapError = true;
          } else {
/*
use the operator's selection function to get the index
(opFunId) of the evaluation function for this operator:

*/
            if ( checkFunId ) {
              selFunIndex = am->Select( alId, opId, typeList );
              opFunId = selFunIndex * 65536 + opId;

/*
Check whether this is a type operator; in that case
opFunId will be negative. A type operator does only a type
mapping, nothing else; hence it is wrong here and we return
type error.

*/
              if ( opFunId < 0 )
                resultType = nl->SymbolAtom( "typeerror" );
            }
      
            if ( isCEAlgebra(alId)) {
              if ( !foundCEAOp ) {
                ceaAlgOpList = nl->TwoElemList(nl->IntAtom( alId ),
                                               nl->IntAtom( opFunId ));
                resultTypeCEAlgOp = resultType;
                foundCEAOp = true;
              }
            } else {
              if ( !foundOrigOp ) {
                origAlgOpList = nl->TwoElemList(nl->IntAtom( alId ),
                                                nl->IntAtom( opFunId ));
                selFunIdxOrigOp = selFunIndex;
                resultTypeOrigOp = resultType;
                foundOrigOp = true;
              }
            }
          }
        }
      } while ( !nl->IsEmpty(opList) && (!foundOrigOp || !foundCEAOp));
      
      if (traceMode) {
        cout << wordWrap( "IN: ",
                          width, NList(typeList).convertToString() )
          << endl
          << wordWrap( "OUT-OrigOP: ",
                       width, NList(resultTypeOrigOp).convertToString() )
          << endl
          << wordWrap( "OUT-CEAOP: ",
                       width, NList(resultTypeCEAlgOp).convertToString() )
          << endl
          << "SelectionFunction: index = " << selFunIdxOrigOp << endl
          << sepLine << endl;
      }
      
      if ((foundOrigOp && foundCEAOp) &&
          nl->Equal(resultTypeOrigOp, resultTypeCEAlgOp)) {
        resultAlgList = nl->TwoElemList(nl->First(origAlgOpList),
                                        nl->First(ceaAlgOpList));
        resultFunIdList = nl->TwoElemList(nl->Second(origAlgOpList),
                                          nl->Second(ceaAlgOpList));
        resultType = resultTypeOrigOp;
      } else if (foundOrigOp && !foundCEAOp) {
        resultAlgList = nl->TwoElemList(nl->First(origAlgOpList),
                                        nl->TheEmptyList());
        resultFunIdList = nl->TwoElemList(nl->Second(origAlgOpList),
                                      nl->TheEmptyList());
        resultType = resultTypeOrigOp;
      } else {
        resultAlgList = nl->TwoElemList(nl->IntAtom( alId ),
                                        nl->TheEmptyList());
        resultFunIdList = nl->TwoElemList(nl->IntAtom( opId ),
                                      nl->TheEmptyList());
      }
    
      return resultType;
    }
  }

/*
6.5 Function ~generateApplyopList~

This function adapted ~generateApplyopList~-functionality included in 
the ~Secondo QueryProcessor Annotate~-function.
In this case, the structure of the annotated list is changed to a form containing
further necessary information for the ~CECompiler~ of the ~Compiled Expressions Algebra~.

The change refers to the ~operator~-list and the ~applyop~-list.
In the ~operator~-list, the atom ~operator-ID~ and ~algebra-ID~ are replaced by a  two element 
list, whose first atom is the ~operator-ID~ or ~algebra-ID~ of the original operator and the
second atom is the ~operator-ID~ or ~algebra-ID~ of the operator from
the ~Compiled Expressions Algebra~. The replacement in the ~applyop~-list for the ~opFun-ID~
is done. So instead of the ~operator~-list 

----        

((<name> operator <opID> <algID>) typeerror)

----

this list is created

----       

((<name> operator ((<opID origOp>) (<opID ceaOp>)) ((<algID origAlg>) (<algID ceaAlg>)) typeerror)

----

The ~applyop~-list is created instead

----          ((none applyop (ann(op) ann(arg1) ... ann(argn)))
                <resulttype>
                <opFunId>)
----


this list is created

----          ((none applyop (ann(op) ann(arg1) ... ann(argn)))
                <resulttype>
                ((<opFunID orig>) (<opFunID cea>)))
----

*/
  void
  CEQueryProcessor::generateApplyopList(
    ListExpr& applyopList,
    ListExpr first,
    ListExpr list,
    ListExpr resultAlgList,
    ListExpr resultFunIdList,
    ListExpr resultType) {
    
    if (CompiledExpressionsAlgebra::isActiv()) {
      ListExpr tmpFunIdListOperator = nl->TwoElemList(
                                        nl->First(resultFunIdList),
                                        nl->Second(resultFunIdList));
      ListExpr tmpFunIdListApplyop = nl->TwoElemList(
                                        nl->First(resultFunIdList),
                                        nl->Second(resultFunIdList));
      ListExpr newList = nl->Cons(
                           nl->TwoElemList(
                             nl->FourElemList(
                               nl->First(first),
                               nl->Second(first),
                               resultAlgList,
                               tmpFunIdListOperator),
                           nl->Second(nl->First(list))),
                         nl->Rest(list));

      applyopList = nl->ThreeElemList(
                      nl->ThreeElemList(
                        nl->SymbolAtom("none"),
                        nl->SymbolAtom("applyop"),
                        newList),
                      resultType,
                      tmpFunIdListApplyop);
    }
  }
  
/*
6.6 Function ~GenerateCompiledExpressions~

This function calls the ~CECompiler~ to create the shared library with
the compiled query.

*/
  void
  CEQueryProcessor::GenerateCompiledExpressions(
    CEQuery* ptrCEQY,
    ListExpr& list) {
    if (ptrCEQY && ptrCEQY->getUseCEQP()) {
      CECompiler::getInstance()->ceGenerateQuery(list, ptrCEQY);
    }
  }

/*
6.7 Function ~getQPDebugMode~

This function returns whether the debug mode is set.

*/
  bool
  CEQueryProcessor::getQPDebugMode() {
    return ptrQP->debugMode;
  }
  
/*
6.7 Function ~getQPTraceMode~

This function returns whether the trace mode is set.

*/
  bool
  CEQueryProcessor::getQPTraceMode() {
    return ptrQP->traceMode;
  }
  
/*
6.7 Function ~loadCompiledExpressions~

This function loaded the generated shared library.

*/
  void
  CEQueryProcessor::loadCompiledExpressions(std::string libName,
                                            CEQuery* ptrCEQY)
  throw (CEQRuntimeError) {
    bool loadOK = false;
    if (ptrCEQY) {
      if (ptrCEQY->getPtrCEQYLibrary()->Load(libName)) {
        void* ptrFuncInit = ptrCEQY->getPtrCEQYLibrary()
            ->GetFunctionAddress("initCompiledExpressionsLib");
        void* ptrFuncClose = ptrCEQY->getPtrCEQYLibrary()
            ->GetFunctionAddress("closeCompiledExpressionsLib");
        if (ptrFuncInit && ptrFuncClose) {
          ptrCEQY->setFuncCloseCompiledExpressionsLib(ptrFuncClose);
          FuncInitCompiledExpressionsLib_t initCELib = 
                (FuncInitCompiledExpressionsLib_t) ptrFuncInit;
          if (initCELib(ptrCEQY))
            loadOK = true;
        }
      }
    }
    
    if (!loadOK) {
      cout << "Error by loading CompiledExpression-Library." << endl;
      throw CEQRuntimeError("Error by loading CompiledExpression-Library.");
    }
  }
  
/*
6.7 Function ~createLibFunctionAddressNList~

The function generates a list that contains the memory address of the functions
to be called by the operators in the generated shared library.

*/
  bool
  CEQueryProcessor::createLibFunctionAddressNList
    (ListExpr& ptrListFunction,
     ListExpr& returnType,
     const std::string& functionName,
     CEQuery* ptrCEQY) {
      
    bool retValue = false;
    if (ptrCEQY) {
      if(ptrCEQY->getPtrCEQYLibrary()->IsLoaded()) {
        void* ptr = ptrCEQY->getPtrCEQYLibrary()
                               ->GetFunctionAddress(functionName);
                               
        if (ptr) {
          NameIndex varnames;
          VarEntryTable vartable;
          bool defined = true;
          
          ListExpr ptrList = listutils::getPtrList(ptr);
          ListExpr ptrAnnList = nl->TwoElemList(
            returnType,
            nl->TwoElemList(
              nl->SymbolAtom("ptr"),
              ptrList));
          ptrListFunction = ptrQP->Annotate(ptrAnnList,
                                            varnames,
                                            vartable,
                                            defined,
                                            nl->TheEmptyList(),
                                            nl->TheEmptyList());
          retValue = true;
        }
      }
    }
    return retValue;
  }
  
/*
6.7 Function ~getQPValues~

This function returns the value with the index ~idx~ from the ~Secondo QueryProcessor~.

*/
  Word
  CEQueryProcessor::getQPValues(unsigned int idx) {
    if (idx < ptrQP->values.size())
      return ptrQP->values[idx].value;
    else 
      return SetWord(Address(0));
  }

/*
6.7 Function ~qpResultStorage~

This function returns the result storage from the given supplier ~s~.

*/
  Word
  CEQueryProcessor::qpResultStorage(const Supplier s) {
    return ptrQP->ResultStorage(s);
  }

/*
6.7 Function ~IsPointerNode~

This function checks if the given supplier ~s~ a pointer node in the
operator tree.

*/
  bool
  CEQueryProcessor::IsPointerNode( const Supplier s ) const {
    OpTree tree = static_cast<OpTree>( s );
    return tree->nodetype == Pointer;
  }

/*
6.8 Function ~GetTupleResultType~

A ~get~-function returned the nested list is assigned the return type for
the given node key.

*/
  ListExpr
  CEQueryProcessor::GetTupleResultType(std::string ceNodeKey,
                                       CEQuery* ceQY) {
    ListExpr tmpReturnTypeList = ceQY->getMapResultTypes(ceNodeKey);
    if (nl->IsEmpty(tmpReturnTypeList))
      return tmpReturnTypeList;
    else
      return SecondoSystem::GetCatalog()->NumericType(tmpReturnTypeList);
  }
  
} // end of namespace CompiledExpressions

/*
7 External call functions

*/
#ifdef __cplusplus
extern "C"{
#endif
  
/*
7.1 Function ~FuncCEQYGetNewInstance~

This external call function generate a new ~CEQuery~-object and returned
a pointer of them.

*/
  CompiledExpressions::CEQuery*
  FuncCEQYGetNewInstance(bool useCEQP) {
    return new CompiledExpressions::CEQuery(useCEQP);
  }
  
/*
7.2 Function ~FuncCEQPGetNewInstance~

This external call function generate a new ~CEQueryProcessor~-object and
returned a pointer of them.

*/
  CompiledExpressions::CEQueryProcessor*
  FuncCEQPGetNewInstance(QueryProcessor* ptrQP) {
    return new CEQueryProcessor(ptrQP);
  }
  
#ifdef __cplusplus
}
#endif
