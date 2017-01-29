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

[10] Header file to integration the ~CompiledExpression Algebra~-functionality in the ~Secondo~-kernel

2016/2017 H.Brieschke created the new CompiledExpression Algebra


[TOC]


1 Overview 

In this header file the necessary basic settings  are made for the ~Compiled Expressions Algebra~.
Furthermore, the non-standard functions of ~Secondo~ of the ~Compiled Expressions Algebra~ are declared.


2 Defines, includes, and constants

*/


#ifndef _COMPILED_EXPRESSIONS_LINK_CE_ALGEBRA_H_
#define _COMPILED_EXPRESSIONS_LINK_CE_ALGEBRA_H_

/*
The two following entries are activated or deactivated by the make process, whereby the precompiler
can generate the corresponding code with or without the functionality of the ~Compiled Expressions Algebra~
or for static or dynamic integration of the code.

*/
#define _COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_
#define _COMPILED_EXPRESSIONS_ALGEBRA_STATIC_


#ifdef _COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_

#define _COMPILED_EXPRESSIONS_ALGEBRA_NAME_ "CompiledExpressionsAlgebra"

/*
3 Forward declarations

*/
namespace CompiledExpressions {
  class CompiledExpressionsAlgebra;
  class CEQuery;
  class CEQueryProcessor;
  //class CECompiler;
}

/*
4 Type definitions

*/
typedef void (*FuncCECGSetCanInitImplCodeStore_t)();
typedef CompiledExpressions::CEQuery*
      (*FuncCEQYGetNewInstance_t)(bool);
typedef CompiledExpressions::CEQueryProcessor*
      (*FuncCEQPGetNewInstance_t)(QueryProcessor*);


/*
5 External call functions

Definition of the external call functions.

*/

#ifdef __cplusplus
extern "C"{
#endif
/*
The external call function is called when the ~Second~ system starts so far that
the runtime configuration of the ~Compiled Expressions Code Store~ can be created.

*/
  extern  void FuncCECGSetCanInitImplCodeStore();
  
/*
The external call function generate a new ~CEQuery~-object and
returned a pointer of them.

*/
  extern  CompiledExpressions::CEQuery*
        FuncCEQYGetNewInstance(bool);
  
/*
The external call function generate a new ~CEQueryProcessor~-object and
returned a pointer of them.

*/
  extern  CompiledExpressions::CEQueryProcessor*
        FuncCEQPGetNewInstance(QueryProcessor*);

#ifdef __cplusplus
}
#endif


#endif //_COMPILED_EXPRESSIONS_ALGEBRA_AKTIV_
#endif //_COMPILED_EXPRESSIONS_LINK_CE_ALGEBRA_H_
