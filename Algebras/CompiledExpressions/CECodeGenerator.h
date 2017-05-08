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

[10] Header file of the CECodeGenerator

2016/2017 H.Brieschke created the new CompiledExpression Algebra


[TOC]


1 Overview 

This header file declares all classes needed for the ~CECodeGenerator~. These are

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
#ifndef _COMPILED_EXPRESSIONS_CODEGENERATOR_H_
#define _COMPILED_EXPRESSIONS_CODEGENERATOR_H_

//define macro TRACE_ON if trace outputs are needed
//#define TRACE_ON
#undef TRACE_ON

#include <map>
#include <set>
#include <vector>

#include "./CECompiler.h"

namespace CompiledExpressions {
// forward declarations from classes in the CompiledExpressions-namespace
  class CECodeGenerator;
  class CECOpTreeVisitorGenerateCode;

/*
3 Enumeration definitions

3.1 Enumeration ~ceCGHeaderFileIdx~

This enumeration define names for the various vectors to save the entry of include files.

*/
  enum ceCGHeaderFileIdx {
    HF_IDX_CPP,
    HF_IDX_SECONDO,
    HF_IDX_REST,
    HF_SIZEOF
  };
  
/*
3.2 Enumeration ~ceCGCodeNodesTypes~

This enumeration define names for the various vectors to save the entry of code parts.

*/
  enum ceCGCodeNodesTypes {
    CE_VCG_SUPERCLASS,
    
    CE_VCG_DEFINITATION_VAR, 
    CE_VCG_DEFINITATION_ADDITIONAL_VAR,
    CE_VCG_DEFINITATION_FUNC_GET_SON_NODE,
    CE_VCG_DEFINITATION_FUNC_EVAL,
    CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_OPEN,
    CE_VCG_DEFINITATION_FUNC_OPERATOR_EVAL_REQUEST,
    CE_VCG_DEFINITATION_ADDITIONAL_FUNCTION,
    
    CE_VGC_DECLARATION_FUNC_CONSTRUCTOR_BODY,
    CE_VGC_DECLARATION_FUNC_DESTRUCTOR_BODY,
    CE_VCG_DECLARATION_FUNC_INIT_BODY,
    CE_VGC_DECLARATION_FUNC_GET_SON_NODE_HEAD,
    CE_VGC_DECLARATION_FUNC_GET_SON_NODE_BODY,
    CE_VGC_DECLARATION_FUNC_EVAL_HEAD,
    CE_VGC_DECLARATION_FUNC_EVAL_BODY,
    CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_HEAD,
    CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_OPEN_BODY,
    CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_HEAD,
    CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_REQUEST_BODY,
    CE_VGC_DECLARATION_FUNC_OPERATOR_EVAL_CLOSE_BODY,
    CE_VGC_DECLARATION_ADDITIONAL_FUNCTION,
    
    CE_VCG_CT_SIZEOF
  };


/*
4 Class ~CECGImplSecondoType~

This class is the base class of all ~ceCodeStore~-classes to implements
the code generation of a ~Secondo Type~.

*/
  class CECGImplSecondoType {
  public:
/*
The Destructor 

*/
    virtual ~CECGImplSecondoType() {}    

/*
A ~set~-function that initializes the class objects with the current values
of the ~Secondo Algebra Manager~ at runtime.

*/
    void initType(int algID,
                  int typeID,
                  bool isActiv);
    
/*
A ~get~-function returned ~TRUE~ or ~FALSE~ if the 
corresponding ~Secondo Type~ activ or not.

*/
    bool isActivType();
    
/*
A ~get~-function returned ~TRUE~ or ~FALSE~ whether the 
corresponding ~Secondo Type~ has a correspondence as ~C++ Type~.

*/
    virtual bool isCPPType();
    
/*
A ~get~-function returned a string with the ~Secondo Type Name~.

*/
    virtual std::string getTypeName();    
    
/*
A ~get~-function returned a string with the ~Secondo Type Class Name~.

*/
    virtual std::string getTypeClassName();
    
/*
A ~get~-function returned a string with the ~C++ Type Name~.

*/
    virtual std::string getCPPTypeName();
    
/*
This function insert all in the code to included headerfiles
in the array ~hf~.

*/
    virtual void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {}
    
  protected:    
/*
The Constructor 

*/
    CECGImplSecondoType();    
    
  private:
/*
Different local variables...

*/
    int algID, typeID;
    bool isActiv;
  };
  

/*
5 Class ~CECGImplSecondoOperator~

This class is the base class of all ~ceCodeStore~-classes to implements
the code generation of a ~Secondo Operator~.

*/
  class CECGImplSecondoOperator {
  public:
/*
The Destructor 

*/
    virtual ~CECGImplSecondoOperator() {}    
    
/*
A ~set~-function that initializes the class objects with the current values
of the ~Secondo Algebra Manager~ at runtime.

*/
    void initOperator(int algID,
                      int opID, 
                      bool isActiv);
    
/*
A ~get~-function returned ~TRUE~ or ~FALSE~ if the 
corresponding ~Secondo Operator~ activ or not.

*/
    bool isActivOperator();
    
/*
A ~get~-function returned a string with the ~Secondo Operator Name~.

*/
    virtual std::string getOperatorName();   
    
/*
A ~get~-function returned ~TRUE~ or ~FALSE~ if the 
corresponding ~Secondo Operator~ in his ~ResultStorage~ created new object or not.

*/
    virtual bool isGenerateNewObject();
    
/*
A ~get~-function returned ~TRUE~ or ~FALSE~ if the 
corresponding ~Secondo Operator~ a stream operator or not.

*/
    virtual bool isStreamOperator();
    
/*
This function insert all in the code to included headerfiles
in the array ~hf~.

*/
    virtual void getHeaderFilesEntry(std::set<std::string>(&hf)[HF_SIZEOF]) {}
    
/*
A ~get~-function returned a string with the code to defintion additional variables.

*/
    virtual std::string getCodeOperatorAdditionalVariablesDefinition();
    
/*
A ~get~-function returned a string with the code to initialisation additional variables.

*/
    virtual std::string getCodeOperatorAdditionalVariablesInitialisation();
    
/*
A ~get~-function returned a string with the code from the body of the ~eval open~-function.

*/
    virtual std::string getCodeOperatorEvalOpenBody();
    
/*
A ~get~-function returned a string with the code from the body of the ~eval request~-function.

*/
    virtual std::string getCodeOperatorEvalRequestBody
                        (std::vector<CECGImplSecondoType*>(&signature))
      throw (CECRuntimeError);
    
/*
A ~get~-function returned a string with the code from the body of the ~eval close~-function.

*/
    virtual std::string getCodeOperatorEvalCloseBody();
    
  protected:
/*
The Constructor 

*/
    CECGImplSecondoOperator();
    
  private:
/*
Different local variables...

*/
    int algID, opID;
    bool isActiv;
  };
  
  
  
  
/*
6 Class ~CECGImplSecondoAlgebra~

This class is the base class of all ~ceCodeStore~-classes to implements
the code generation of a ~Secondo Algebra~.

*/
  class CECGImplSecondoAlgebra {
  public:
/*
The Destructor 

*/
    virtual ~CECGImplSecondoAlgebra() {}

/*
This function loads the implemented types into the code store of the ~CECompiler~.

*/
    virtual void loadCECGImplSecondoTypes(CECodeGenerator* ceCG) {}   
    
/*
This function loads the implemented operators into the code store of the ~CECompiler~.

*/
    virtual void loadCECGImplSecondoOperators(CECodeGenerator* ceCG) {}
    
  protected:
/*
A local variable...

*/    
    const std::string algName;
    
/*
The Constructor

*/
    CECGImplSecondoAlgebra(const std::string& name) : algName(name) {}
    
  private:
/*
The Standard Constructor

*/
    CECGImplSecondoAlgebra() : algName(std::string("")) {}
  };

  
/*
7 Class ~CECodeGenerator~

The class ~CECodeGenerator~ implements the code generator. On one hand, it manages
the ~code store~ and, on the other hand, uses the ~CECOpTreeVisitorGenerateCode~-visitor
to create the source code files to be generated.

This class is also a ~Singleton~.

*/
  class CECodeGenerator {    
  public:
/*
This static function returned a pointer of the ~CECodeGenerator~-instance. 
If no instance has been created, the function creates an instance.

*/    
    static CECodeGenerator* getInstance();
    
/*
This static function deleted the ~CECodeGenerator~-instance. 

*/
    static void deleteInstance();
    
/*
This static function set a flag, which indicates that the ~Secondo System~ has been started 
so far that the ~Code Store~ can be initialized. This point is reached as soon as
the ~Secondo Algebra Manager~ has loaded all active algebras.

*/
    static void setCanInitialize();
    
/*
The Destructor 

*/
    virtual ~CECodeGenerator();
    
/*
This function insert the given ~CECGImplSecondoType~-object ~implSecondoType~
in a data structure, in which all implemented ~CECGImplSecondoType~ are managed.

*/
    void loadCECGImplSecondoType(CECGImplSecondoType* implSecondoType);
    
/*
A ~get~-function returned a pointer to the ~CECGImplSecondoType~-objects is given
by the name ~name~.

*/
    CECGImplSecondoType* getPtrSecondoType(std::string& name);
    
/*
This function insert the given ~CECGImplSecondoOperator~-object ~implSecondoOperator~
in a data structure, in which all implemented ~CECGImplSecondoOperator~ are managed.

*/
    void loadCECGImplSecondoOperator
                               (const std::string& algName,
                                CECGImplSecondoOperator* implSecondoOperator);
    
/*
A ~get~-function returned a pointer to the ~CECGImplSecondoOperator~-objects is given
by the ~Algebraname~ and the ~Operatorname~.

*/
    CECGImplSecondoOperator* getPtrSecondoOperator(std::string& algName,
                                                   std::string& opName);
    
/*
A ~get~-function returned a pointer to the ~CECGImplSecondoOperator~-objects is given
by the ~Algebranumber~ and the ~Operatornumber~.

*/
    CECGImplSecondoOperator* getPtrSecondoOperator(unsigned const int algID,
                                                   unsigned const int opID);

    
/*
This function generate the ~c++-code~ of the query.

*/
    bool generateCECode(CECOpNode* ptrRootNode,
                        CEQuery* ptrCEQY);
    
/*
This function write the ~c++-code~ of the query in the stream ~outPut~.

*/
    void getStreamGeneratedCECode(std::ostream& outPut,
                                  CEQuery* ptrCEQY);
    
/*
This function generate the code from a ~makefile~ and write this in the stream ~outPut~.

*/
    void getStreamMake(std::ostream& outPut,
                       std::string libName);
    
  private:
/*
Different static variables...

*/
    static bool canInitialize;
    static CECodeGenerator* instance;

/*
Different local variables...

*/
    bool isLoadCECGImplTypesAndOperators, isInitCECGImplTypesAndOperators;
    std::set<CECGImplSecondoAlgebra*> classObjAlgebras;
    std::map<std::string, CECGImplSecondoType*> mapNameImplTypes;
    std::map<std::string, CECGImplSecondoOperator*> mapNameImplOperators;
    std::map<std::string, CECGImplSecondoOperator*> mapIdImplOperators;
    
/*
The Standard Constructor 

*/
    CECodeGenerator();
    
/*
This function loaded all implemented algebras, types, and operators from the ~Code Store~.

*/
    void loadCECGImplAlgebrasTypesAndOperators();
    
/*
In this function the loading of the implemented algebras from the ~Code Store~
is encapsulated.

*/
    void loadCECGImplAlgebras();

/*
This function initialized all implemented types and operators from the ~Code Store~.

*/
    void initCECGImplTypesAndOperators();

/*
This function deleted all ~Algebras~-, ~Types~- and ~Operator~-objects are loaded from
the ~Code Store~.

*/
    void deleteCECGImplAlgebrasTypesAndOperators();

  };
  
/*
8 Class ~CECOpTreeVisitorGenerateCode~

The class ~CECOpTreeVisitorGenerateCode~, a subclass from the ~CECOpTreeVisitor~, searches for the
code parts in the operator tree and provides it to the ~CECodeGenerator~.

*/
  class CECOpTreeVisitorGenerateCode : public CECOpTreeVisitor {
  public:
/*
The Constructor 

*/
    CECOpTreeVisitorGenerateCode();
    
/*
The Destructor 

*/
    ~CECOpTreeVisitorGenerateCode();
    
/*
This function completes the names of the parameters passed to a function for all necessary functions
of the source code to be generated, so that unique variable names are generated within a function.

*/
    void addVarNamesARG_idx();

/*
A ~get~-function returned a string with the generated ~c++-code~ of
the ~include headerfile section~.

*/
    std::string getGenCodeIncludes();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the ~forward class declaration section~.

*/
    std::string getGenCodeForwardDeclarations();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the ~class definition section~.

*/
    std::string getGenCodeClassDefinitions();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the ~class constructor function~.

*/
    std::string getGenCodeClassConstructor();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the ~class destructor function~.

*/
    std::string getGenCodeClassDestructor();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the ~eval function~.

*/
    std::string getGenCodeFunctionEval();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the ~init function~.

*/
    std::string getGenCodeFunctionInit();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the ~evalOpen- and evalRequest- function~.

*/
    std::string getGenCodeFunctionOperator();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the ~declaration of additional functions~.

*/
    std::string getGenCodeDeclarationAdditionalFunction();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the ~evalDefault function~.

*/
    std::string getGenCodeEvalDefaultFunction();

    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeDefault~.

*/
    std::string getGenCodeClassCECNodeDefault();    
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeApplyop~.

*/
    std::string getGenCodeClassCECNodeApplyop();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeAbstraction~.

*/
    std::string getGenCodeClassCECNodeAbstraction();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeObject~.

*/
    std::string getGenCodeClassCECNodeObject();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeConstant~.

*/
    std::string getGenCodeClassCECNodeConstant();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeVariable~.

*/
    std::string getGenCodeClassCECNodeVariable();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeIdentifier~.

*/
    std::string getGenCodeClassCECNodeIdentifier();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the class ~CECNodeArglist~.

*/
    std::string getGenCodeClassCECNodeArglist();
    
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the declaration of the class ~CodeCompiledExpressions~.

*/
    std::string getGenCodeCodeCompiledExpressionsDeclaration();
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the definition of the class ~CodeCompiledExpressions~.

*/
    std::string getGenCodeCodeCompiledExpressionsDefinition();
    
    
/*
A ~get~-function returned a string with the generated ~c++-code~ of
the the ~external library call functions section~.

*/
    std::string getGenCodeLibCallFunction();
    
    
/*
This function adds commas to lists of arguments, parameters or similar in the
source code to be generated.

*/
    void insertCommaInCodeVector(int startPos,
                                 int actualPos,
                                 std::vector<std::string>(&codeVector));

/*
This function initialized the data structure of code segments for a new node in
the ~operator tree~ and return a iterator of this structure.

*/
    std::map<std::string,
             std::vector<std::vector<std::string> > >::iterator
      initGenCodeNodesEntry(std::string strNodeKey,
                            unsigned int numSons);
      
/*
This function completes the code segments for the father node ~ceNode~
in the operator tree.

*/
    void completeGenCodeFatherNodeEntry(CECOpNode& ceNode);
    
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
Different local variables...

*/
    std::set<std::string> headerfiles[HF_SIZEOF];
    std::map<std::string,
             std::vector<std::vector<std::string> > > genCodeNodes;

    std::set<std::string> genCodeCompExprRootNodes;
    std::map<std::string, std::string> genCodeCCEGlobalVar;
    std::map<std::string, std::string> operators_libcall_function;
    
    bool cecOpNodeApplyopIsInsert;
    bool cecOpNodeAbstractionIsInsert;
    bool cecOpNodeObjectIsInsert;
    bool cecOpNodeConstantIsInsert;
    bool cecOpNodeVariableIsInsert;
    bool cecOpNodeIdentifierIsInsert;
    bool cecOpNodeArglistIsInsert;
    
    void createCELibDefaultFunc();
  }; //end of class CECOpTreeVisitorGenerateCode
  
} // end of namespace CompiledExpressions
#endif // _COMPILED_EXPRESSIONS_CODEGENERATOR_H_
