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

[10] Header file of the CompiledExpression Algebra

2016/2017 H.Brieschke created the new CompiledExpression Algebra


[TOC]


1 Overview 

The ~Compiled Expression Algebra~ basically implements three operators,
Namely ~filter~, ~extend~, and ~executeQuery~. However, these
can not be used directly in a query, but are exchanged by the algebra
against the original operators during the query processing. 

The ~filter~-operator corresponds to the ~filter~-operator from
the ~relation algebra~, the ~extend~-operator is the ~extend~-operator
from the ~ExtRelation algebra~, and the ~excecuteQuery-~operator is used
if the complete query could be compiled by this algebra.

2 Defines, includes, and constants

*/

#ifndef _COMPILED_EXPRESSIONS_ALGEBRA_H_
#define _COMPILED_EXPRESSIONS_ALGEBRA_H_

//define macro TRACE_ON if trace outputs are needed
//#define TRACE_ON
#undef TRACE_ON


#include "Algebra.h"
#include "AlgebraManager.h"

#ifdef USE_PROGRESS
#include "Progress.h"
#endif

#include "../Stream/Stream.h"
#include "../Relation-C++/RelationAlgebra.h"

extern AlgebraManager* am;

// forward declarations
class DynamicLibrary;

using namespace CompiledExpressions;
namespace CompiledExpressions {

// forward declarations from classes in the CompiledExpressions-namespace
  class CEQueryProcessor;
  class CECompiler;
  
  typedef bool (*filterOp_t)(Tuple*);
  typedef Attribute* (*extendOp_t)(Tuple*);
  typedef void (*executeQueryOp_t)(Word&, Supplier);


/*
3 Class ~CEARuntimeError~

This class implements ~ERROR~-objects, which are required for the error treatment.

*/
  class CEARuntimeError : public std::exception {
  public:
    CEARuntimeError(std::string s);
    virtual ~CEARuntimeError() throw() {}
    virtual const char * what() const throw();
  private:
    CEARuntimeError() {};
    std::string errMsg;
  };

  
/*
4 The Operators of the ~Compiled Expressions Algebra~.

4.1  Operator ~filter~

This operator corrosponded to the ~filter-~operator in the ~Realation-Algebra~.
Only tuples, fulfilling a certain condition are passed on to the
output stream.

4.1.1 Class ~CEAFilterLocalInfo~

In the ~Progress~-Version used this class objects as data structure for the
progress informations.

*/
  #ifdef USE_PROGRESS
  class CEAFilterLocalInfo : public ProgressLocalInfo {
  public:
/*
The Constructor.

*/
    CEAFilterLocalInfo();
/*
The Destructor.

*/
    virtual ~CEAFilterLocalInfo();
    
/*
The following methods are the ~get~er- and ~set~er-methods for these
private variables.

*/
    void addCurrent();
    void addReturned();
    void setDone();
    
    int getCurrent();
    int getReturned();
    bool getDone();
    
/*
Definition of private variables and function.

*/
  private:
    int current;    //tuples read
    int returned;   //tuples returned
    bool done;      //arg stream exhaused
  };
  #endif

/*
4.1.2 Type mapping function of operator ~filter~

Result type of filter operation.

----    ((stream (tuple x)) (map (tuple x) bool))
               -> (stream (tuple x))
----

Type mapping function modified to show the possibility of getting
not only types but also arguments in the type mapping. This happens
when an operator
registers "UsesArgsinTypeMapping". Type list now has the form

----  ( (type1 arg1) (type2 arg2) )
----

that is

----  (
      ( (stream (tuple x))  arg1 )
      ( (map (tuple x) bool)  arg2 )
    )
----

*/

  ListExpr ceaFilterTypeMap( ListExpr args );
  
/*
4.1.3 Value mapping function of operator ~filter~

*/
  int ceaFilterValueMap( Word* args,
                         Word& result,
                         int message,
                         Word& local,
                         Supplier s);



/*
4.2 Operator ~extend~

This operator corrosponded to the ~extend-~operator in the ~ExtRealation-Algebra~.
Extends each input tuple by new attributes as specified in the parameter list.


4.2.1 Class ~CEAExtendLocalInfo~

In the ~Progress~-Version used this class objects as data structure for the
progress informations.

*/
  #ifdef USE_PROGRESS
  class CEAExtendLocalInfo : public ProgressLocalInfo {
  public:

/*
The Constructor.

*/
    CEAExtendLocalInfo(TupleType* ptrTupleType,
                       int sv,
                       bool sf,
                       int nna);
/*
The Destructor.

*/
    virtual ~CEAExtendLocalInfo();
    
/*
The following methods are the ~get~er- and ~set~er-methods for these
private variables.

*/
    void setResultTupleType(TupleType* ptrTupleType);
    TupleType* getResultTupleType();
    
    void setStableValue(int val);
    int getStableValue();
    
    void setSizesFinal(bool sf);
    bool getSizesFinal();
    
    void setNoOldAttrs(int noa);
    int getNoOldAttrs();
    
    void setNoNewAttrs(int nna);
    int getNoNewAttrs();
    
    void setAttrSizeTmp(double asp, int idx);
    void addAttrSizeTmp(double asp, int idx);
    double getAttrSizeTmp(int idx);
    
    void setAttrSizeExtTmp(double asep, int idx);
    void addAttrSizeExtTmp(double asep, int idx);
    double getAttrSizeExtTmp(int idx);
    
/*
Definition of private variables and function.

*/
  private:
    CEAExtendLocalInfo() {}
    
    TupleType* resultTupleType;
    int stableValue;
    bool sizesFinal;        //true after stableValue reached
    int noOldAttrs, noNewAttrs;
    double* attrSizeTmp;
    double* attrSizeExtTmp;
    
  };
  #endif

/*
4.2.2 Type mapping function of operator ~extend~

Type mapping for ~extend~ is

----     ((stream X) ((b1 (map x y1)) ... (bm (map x ym))))

        -> (stream (tuple ((a1 x1) ... (an xn) (b1 y1) ... (bm ym)))))

        where X = (tuple ((a1 x1) ... (an xn)))
----

*/
  ListExpr ceaExtendTypeMap( ListExpr args );

/*
4.2.3 Value mapping function of operator ~extend~

*/
  int ceaExtendValueMap( Word* args,
                         Word& result,
                         int message,
                         Word& local,
                         Supplier s );

/*
4.3 Operator ~executeQuery~

This operator is used, if the complete query could be compiled by this algebra.

4.3.1 Progress version

Currently the ~executeQuery~-operator does not support the progress indicator.
Therefore there is no subclass of ~ProgressLocalInfo~.

4.3.2 Type mapping function of operator ~executeQuery~

Since this operator can not be used directly in queries, there is no valid
type mappings. A type mapping error is always generated.

*/
  ListExpr ceaExecuteQueryTypeMap( ListExpr args );

/*
4.2.3 Value mapping function of operator ~executeQuery~

*/
  int ceaExecuteQueryValueMap( Word* args,
                               Word& result,
                               int message,
                               Word& local,
                               Supplier s );

/*
5 Class ~CompiledExpressionsAlgebra~

A new subclass ~CompiledExpressionsAlgebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all operators are registered at the actual algebra.

After declaring the new class, its only instance ~CompiledExpressionsAlgebra~ is defined.

*/
  class CompiledExpressionsAlgebra : public Algebra {
  public:

/*
This function gives a pointer to the ~CompiledExpressionsAlgebra~-Instance.

*/
    static CompiledExpressionsAlgebra* getInstance();

/*
This function delete the ~CompiledExpressionsAlgebra~-Instance.

*/
    static void deleteInstance();
    
/*
This function returned, if one instance of the ~CompiledExpressionsAlgebra~ are created.

*/
    static bool isActiv();

/*
The Constructor.

*/
    CompiledExpressionsAlgebra();  

/*
The Destructor.

*/
    virtual ~CompiledExpressionsAlgebra();
    
/*
This function returned the algebra-ID from the algebramanager or 0, if the
algera is not loaded.

*/
    int getThisAlgebraID() throw (CEARuntimeError);

/*
This function returned the operator-ID of the spezified operator name. If the operator is
not found, the function throws an error.

*/
    int getOperatorID(std::string name) throw (CEARuntimeError);

  private:
    static CompiledExpressionsAlgebra* instance;
    static bool ceaActiv;
    
    int thisAlgID;
};

} // end of namespace CompiledExpressions

#endif /* COMPILED_EXPRESSIONS_ALGEBRA_H */

