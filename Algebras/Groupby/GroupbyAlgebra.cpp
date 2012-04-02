/*

This file is part of SECONDO.

Copyright (C) 2004-2012, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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


//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]

\newpage

[1] Implementation of Module Groupby Algebra

April 2012. Dieter Capek

[TOC]

\newpage


1 Includes and Defines

*/


#include <vector>
#include <stack>
#include <limits.h>

#undef TRACE_ON
#include "LogMsg.h"
#define TRACE_OFF

#include "RelationAlgebra.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "CPUTimeMeasurer.h"
#include "StandardTypes.h"
#include "Counter.h"
#include "TupleIdentifier.h"
#include "Progress.h"
#include "RTuple.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "DateTime.h"
#include "Stream.h"
#include "FTextAlgebra.h"
#include "SecondoCatalog.h"

#define NUMBUCKETS 99997
#define Normal_Merge 0
#define Symmetric_Merge 1


extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

using namespace listutils;


/*
2 Operator groupby2

Groups an input tuple stream using hashing. 
The operator supports aggregation with or witout grouping.
A list of aggregation functions is supported.
The operator supports normal and symmetric merging of attributes.
The operator respects main memory limits and partitions the problem into phases.
The operator does support Secondo progess estimation.

2.1 Specification

Operator description for the Secondo user.

*/

const string groupby2Spec = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\"  ) "
  "( <text>stream(Tuple) x AttrList x "
  "(NewAttr-1 x (Tuple x Data -> Data) x Data) .. "
  "(NewAttr-j x (Data x Data -> Data) x (Tuple -> Data) -> "
  "stream(Tuple(Attrlist) o Tuple([NewAttr-1: Data]..[NewAttr-j: Data])"
  "</text--->"

  "<text>_ groupby2 [list; funlist]</text--->"
  
  "<text>Groups a tuple stream according to the attributes "
  "in AttrList and computes aggregate functions for each group. "
  "The aggregation attributes are appended to the grouping attributes." 
  "</text--->"
  ") )";



struct AggrStackEntry {
  int level;
  Attribute* value;
};


/*
2.2 Type Mapping Function

Checks if the supplied data types are correct for the operator.
Returs the data types which result from the operator run.
Using the Secondo Append mechanism the number of groupting attributes, their
positions, the number of aggregate functions and the types of merging required
are forwarded to the value mapping function.

*/


// Type Mapping Function for groupby2 =========================================
ListExpr groupby2TypeMap(ListExpr args)
{
  int j;
  ListExpr first, second, third;           // analysing input
  ListExpr listn,                          // names and data types
    lastlistn, 
    listp,                                 // positions of grouping attributes
    lastlistp;
  ListExpr attrtype, result, t, t1, t2, t3, t4;
  ListExpr rest, newAttr, mapDef, firstInit, mapOut;
  ListExpr merge, lastmerge;                    // indicates merge type
  string attrname, resstring;
  string tupleSymbolStr = Tuple::BasicType();
  string err = 
    "stream(tuple(X)) x (g1..gn) x (tuple(X)xtxt -> t), t in DATA expected"; 
  bool firstcall;


  first = second = third = merge = lastmerge = nl->TheEmptyList();
  listn = lastlistn = listp = nl->TheEmptyList();
  // Number of input parameters must be three
  if(! nl->HasLength(args,3))
    return listutils::typeError("Need to specify three parameters.");
  // Get the three arguments
  first  = nl->First(args);         // input stream
  second = nl->Second(args);        // list of grouping attributes
  third  = nl->Third(args);         // aggregation functions

  // Missing values are only allowed for grouping attributes
  if ( nl->IsEmpty(first) || nl->IsEmpty(third))
    return listutils::typeError("Mandatory argument is missing.");
  // First argument must be of type stream
  if(!Stream<Tuple>::checkType(first))
    return listutils::typeError("First argument must be of type stream.");
  // Each grouping attribute must be part of the input stream
  rest = second;
  lastlistp = nl->TheEmptyList();
  firstcall = true;

  while (!nl->IsEmpty(rest))
  {
    attrtype = nl->TheEmptyList();
    t = nl->First(rest);
    if(nl->AtomType(t)!=SymbolType)
      return listutils::typeError("Wrong format for an attribute name");
    attrname = nl->SymbolValue(t);

    // Get position of attribute within tuple
    j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);
    if (j) {
      if (!firstcall) {
        lastlistn = nl->Append(lastlistn,nl->TwoElemList(t,attrtype));
        lastlistp = nl->Append(lastlistp,nl->IntAtom(j));
      } else {
        firstcall = false;
        listn = nl->OneElemList(nl->TwoElemList(t,attrtype));
        lastlistn = listn;
        listp = nl->OneElemList(nl->IntAtom(j));
        lastlistp = listp;
      }
    } else {
      // Grouping attribute not in input stream
      string errMsg = "groupby2: Attribute " + attrname + 
        " not present in input stream";
      return listutils::typeError(errMsg);
    }
    rest = nl->Rest(rest);
  } // end while; checking grouping attributes

  // Must specify at least one aggregation function
  if(nl->ListLength(third) < 1)
    return listutils::typeError("Must specify one aggregation function.");

  rest = third;         // List of functions
  // Checking of aggregate functions
  while (!(nl->IsEmpty(rest))) 
  {
    // Iterate over function list and initial values
    t = nl->First(rest);  // functions
    rest = nl->Rest(rest);
    // Format must be Name:Funktion::Initial Value 
    if(nl->ListLength(t) != 3)
      return listutils::typeError("Each function must have three elements.");
    newAttr  = nl->First(t);       // function name
    mapDef   = nl->Second(t);      // aggregate function 
    firstInit = nl->Third(t);      // function definition or inital value
    // Checking attribute name
    if ( !(nl->IsAtom(newAttr)) || !(nl->AtomType(newAttr) == SymbolType) )
      return listutils::typeError("Attribut name for function is not valid.");
    // Checking aggregate function
    if(!listutils::isMap<2>(mapDef))  
      return listutils::typeError("Aggregation function is not valid.");
    mapOut = nl->Third(mapDef);         // type of second argument

    // assume normal or symmetric merging based on the firstInit parameter
    if(listutils::isDATA(firstInit)) {  
      // check the syntax for normal merging:  name:function:initial value
      // Tuple must be first function argument 
      t = nl->Second(first);
      if(!nl->Equal(t, nl->Second(mapDef)))
      return listutils::typeError("Map argument 1 must be tuple from stream.");
   
      // Second function argument and initial value must be from same type
      if(! nl->Equal(firstInit, nl->Third(mapDef)))
        return listutils::typeError(
        "Map argument 2 and start value must have same type.");
      // Function result and initial value must be from same type
      if(! nl->Equal(firstInit, nl->Fourth(mapDef)))
      return listutils::typeError(
        "Map result and start value must have same type.");
      // indicate normal merging 
      if (nl->IsEmpty(merge)) {
        merge = nl->OneElemList(nl->IntAtom(Normal_Merge));
        lastmerge = merge;
      } else
        lastmerge = nl->Append(lastmerge, nl->IntAtom(Normal_Merge));
    } else {
      // check the syntax for symmetric merging:  name:function1:function2
      // Checking function2
      if(!listutils::isMap<1>(firstInit))  
        return listutils::typeError("Function2 must have one argument.");
      // Tuple must be first function2 argument 
      t = nl->Second(first);
      if(!nl->Equal(t, nl->Second(firstInit)))
      return listutils::typeError("Function2 argument must be stream tuple.");
      t1 = nl->Second(mapDef);    // Function1 first argument
      t2 = nl->Third(mapDef);     // Function1 second argument
      t3 = nl->Fourth(mapDef);    // Function1 result
      t4 = nl->Third(firstInit);  // Function2 result      

      if ( !listutils::isDATA(t1) || !listutils::isDATA(t2) ||
           !listutils::isDATA(t3) || !listutils::isDATA(t4) )
      return listutils::typeError(
      "Function1 arguments and both functions results must be of kind DATA.");
      if ( !nl->Equal(t1,t2) || !nl->Equal(t1,t3) || !nl->Equal(t1,t4) )
      return listutils::typeError(
      "Function1 arguments and both functions results must be of same type.");
      // indicate symmetric merging 
      if (nl->IsEmpty(merge)) {
        merge = nl->OneElemList(nl->IntAtom(Symmetric_Merge));
        lastmerge = merge;
      } else
        lastmerge = nl->Append(lastmerge, nl->IntAtom(Symmetric_Merge));
    } // end-if: check a single aggregate function

    // add function name and result type to list
    if (    (nl->EndOfList( lastlistn ) == true)
         && (nl->IsEmpty( lastlistn ) == false)
         && (nl->IsAtom( lastlistn ) == false)
       )
    { // List already contains group-attributes (not empty)
      lastlistn = nl->Append(lastlistn,(nl->TwoElemList(newAttr,mapOut)));
    } else { 
      // No group attribute (list is still empty)
      listn = nl->OneElemList(nl->TwoElemList(newAttr,mapOut));
      lastlistn = listn;
    }
  } // end while for aggregate functions

  // sample: (2 1 2 4 0 1 0 0)
  // # of gourp attributes, followed by their positions
  // # of aggregation functions, followed by merging type
  t1 = nl->OneElemList( nl->IntAtom(nl->ListLength(listp)) );
  t2 = nl->OneElemList( nl->IntAtom(nl->ListLength(merge)) );
  t3 = concat( t1, listp );
  t4 = concat( t2, merge );
  t1 = concat( t3, t4 );

  // Check if the name for the aggregate is used already
  if ( !CompareNames(listn) )
    return listutils::typeError("Attribute names are not unique.");
  // Type mapping is correct, return result type.
  result =
    nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),     // text APPEND  
      t1,                                   // List of gouping and merging info
      nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),   // text STREAM
        nl->TwoElemList( nl->SymbolAtom(tupleSymbolStr), listn))
    );
  return result;
}  // end of type mapping for groupby2 operator


/*
2.3 C++ Class groupby2LocalInfo

This class keeps all local operator information between the individuall 
calls from the query processor. The class contains all methods required to
initialize groups, aggregate tuples into groups and estimate operator cost.

2.3.1 Local Data Elements

Local data contains all aggregation results, information on tuple buffers and 
tuple scans, available and used memory and progress information.

*/


// groupby2LocalInfo class ====================================================

class groupby2LocalInfo: public ProgressLocalInfo
{
public:
  Tuple *t;
  TupleType *resultTupleType;
 
  int numberatt;             // number of grouping attributes
  int noOffun;               // number of aggregate functions to build
  int *MergeType;            // indicates normal or symmetric merging 

  // Actual buffers are created during OPEN
  TupleBuffer *TB_In, *TB_Out, *TB_Temp;   // to save unprocessed input tuples
  TupleBuffer *TB_Group;    // to save group tuples in memory out situations
  GenericRelationIterator* In_Rit;
  bool newGroupsAllowed;

  long MAX_MEMORY, Used_Memory;   // max. memory and actually used memory
  bool FirstREQUEST;   // indicates the start of a phase
  unsigned int ReturnBucket, ReturnElem, 
    No_RetTuples,      // number of result tuples returned
    No_GTuples,        // number of group tuples in memory 
    Phase,             // phase the operator currently runs in
    SumGTuples,        // sum of group tuples returned
    SumITuples,        // sum of input tuples processed
    SumDiskData,       // data volume written to secondary storage
    tup_aggr,          // number of input tuples aggregated into groups
    tup_n,             // number of input tuples (avalailable from phase 2)
    read_this_phase;   // tuples read from TB_In this phase (from phase 2)

  // progress information
  unsigned int stableValue;
  bool sizesFinal;
  double *attrSizeTmp;
  double *attrSizeExtTmp;

  vector<Tuple*> hBucket[NUMBUCKETS];   // data structure for hash buckets

  groupby2LocalInfo() : t(NULL), resultTupleType(NULL), 
    numberatt(0), noOffun(0), MergeType(NULL),
    TB_In(NULL), TB_Out(NULL), TB_Temp(NULL), TB_Group(NULL), 
    In_Rit (NULL), newGroupsAllowed(true),
    MAX_MEMORY(0), Used_Memory(0), FirstREQUEST(true), ReturnBucket(0),
    No_RetTuples(0), No_GTuples(0), Phase(0), 
    SumGTuples(0), SumITuples(0), SumDiskData(0), tup_aggr(0), tup_n(0),
    read_this_phase(0),
    stableValue(50),sizesFinal(false),
    attrSizeTmp(0), attrSizeExtTmp(0)
  {}


/*
2.3.2 Function InitTuple

Get first function values from tuple and initial values.
tres is the group tuple, s the tuple from the input stream.

*/


  void InitTuple (Tuple* tres, Tuple* s, Supplier addr)  
  { 
    int i;
    Supplier supp1, supp2, supp3, supp4;
    ArgVectorPointer funargs;
    stack<AggrStackEntry> *newstack;    
    Attribute *sattr, *tattr;
    Word funres;
    AggrStackEntry FirstEntry;

    // get first function values from tuple and initial values
    for (i=0; i < noOffun; i++) {
      if (MergeType[i] == Normal_Merge) {
        supp1 = (Supplier) addr;    // list of functions
        supp2 = qp->GetSupplier( supp1, i);
        supp3 = qp->GetSupplier( supp2, 1);
        funargs = qp->Argument(supp3);      // get argument vector   
        supp4 = qp->GetSupplier( supp2, 2); // supp4 is initial value
        qp->Request( supp4, funres);        // function evaluation
        tattr = ((Attribute*)funres.addr)->Clone();

        (*funargs)[0].setAddr(s);          
        (*funargs)[1].setAddr(tattr);
        qp->Request( supp3, funres);        // function evaluation  
        sattr = ((Attribute*)funres.addr)->Clone();
        // after the group attributes
        tres->PutAttribute( numberatt + i, sattr); 
        Used_Memory += 
          sattr->Sizeof() + sattr->getUncontrolledFlobSize();
        tattr->DeleteIfAllowed();    
      } else {
        // symmetric merge
        // evaluate the tuple->data function
        supp1 = (Supplier) addr;  // funlist: list of functions
        supp2 = qp->GetSupplier( supp1, i);
        supp3 = qp->GetSupplier( supp2, 2);  // second function
        funargs = qp->Argument(supp3);
        (*funargs)[0].setAddr(s);           // tuple is only argument
        qp->Request( supp3, funres);        // function evaluation
        sattr = ((Attribute*)funres.addr)->Clone();

        // build a stack element from the attribute, push this
        FirstEntry.level = 0;
        FirstEntry.value = sattr;
        newstack = new stack<AggrStackEntry> ();
        newstack->push(FirstEntry);
        // Link the stack into the tuple after the group attributes 
        tres->PutAttribute(numberatt + i, (Attribute*) newstack);

        Used_Memory += sizeof( *newstack) 
          + sizeof(AggrStackEntry)
          + sattr->Sizeof() + sattr->getUncontrolledFlobSize();
      } // end-if per attribute
    } // end-for 
  } // end of InitTuple


/*
2.3.3 Function AggregateTuple

Aggregate input tuple s into group tuple tres.
tres is the group tuple, s the tuple from the input stream.

*/

  void AggregateTuple (Tuple* tres, Tuple* s, Supplier addr)  
  { 
    int i, StackLevel;
    Supplier supp1, supp2, supp3, supp4;
    ArgVectorPointer funargs;
    stack<AggrStackEntry> *newstack;    
    Attribute *sattr, *tattr;
    Word funres;
    AggrStackEntry FirstEntry;

    // get function values n+1 from new tuple and function value n
    for (i=0; i < noOffun; i++) {
      supp1 = (Supplier) addr;    // list of functions
      supp2 = qp->GetSupplier( supp1, i);
      supp3 = qp->GetSupplier( supp2, 1);
      funargs = qp->Argument(supp3);      // get argument vector
      
      if (MergeType[i] == Normal_Merge) {
        // normal merge 
        sattr = tres->GetAttribute( numberatt+i);  
        (*funargs)[0].setAddr(s);          
        (*funargs)[1].setAddr(sattr);
        qp->Request( supp3, funres);  
        tattr = ((Attribute*)funres.addr)->Clone();

        // subtract old attribute size
        Used_Memory -= 
          sattr->Sizeof() + sattr->getUncontrolledFlobSize();
        // add new attribute size, it can change during merge
        Used_Memory += 
          tattr->Sizeof() + tattr->getUncontrolledFlobSize(); 
        // PutAttribute does an implice DeleteIfAllowed on the old attr
        tres->PutAttribute( numberatt+i, tattr);

      } else {
        // symmetric merge, evaluate tuple->data, merge stack
        StackLevel = 0;
        // pointer to the stack
        newstack = (stack<AggrStackEntry> *) 
          tres->GetAttribute(numberatt+i); 
        
        // evaluate tuple->data
        supp4 = qp->GetSupplier( supp2, 2); // second function
        funargs = qp->Argument(supp4);
        (*funargs)[0].setAddr(s);           // tuple is first argument
        qp->Request( supp4, funres);        // function evaluation
        sattr = ((Attribute*)funres.addr)->Clone();

        // merge stack if possible, else push element
        while (!newstack->empty() && 
               (StackLevel==newstack->top().level)) {
          // merging is possible
          funargs = qp->Argument(supp3);
          (*funargs)[0].setAddr(sattr);   
          tattr = newstack->top().value;
          (*funargs)[1].setAddr(tattr);  // attr from top stack entry
          qp->Request( supp3, funres);   // call parameter function

          sattr->DeleteIfAllowed(); 
          sattr = ((Attribute*)funres.addr)->Clone();

          // delete top element
          Used_Memory -= sizeof(AggrStackEntry)
            + tattr->Sizeof() + tattr->getUncontrolledFlobSize();
          tattr->DeleteIfAllowed();
          newstack->pop();

          StackLevel++;
        } //end-while  

        // write a stack element
        FirstEntry.level = StackLevel;
        FirstEntry.value = sattr;
        newstack->push(FirstEntry);

        Used_Memory += sizeof(AggrStackEntry)
          + sattr->Sizeof() + sattr->getUncontrolledFlobSize();
      }  // end-if process an attribute
    } // end-for 
  } // end of AggregateTuple


/*
2.3.4 Function ShrinkStack

Shrink the stacks for symmetric merging to a regular tuple.

*/

  void ShrinkStack (Tuple* t, Supplier addr)  
  { 
    int i;
    Supplier supp1, supp2, supp3;
    ArgVectorPointer funargs;
    stack<AggrStackEntry> *newstack;    
    Attribute *sattr, *tattr;
    Word funres;

    for (i=0; i < noOffun; i++) {
      if (MergeType[i] == Symmetric_Merge) {
        // get function arguments from qp
        supp1 = (Supplier) addr;    // list of functions
        supp2 = qp->GetSupplier( supp1, i);
        supp3 = qp->GetSupplier( supp2, 1);
        funargs = qp->Argument(supp3);      // get argument vector 

        // collapse the stack to an attribute value, delete the stack
        newstack = (stack<AggrStackEntry> *) t->GetAttribute(numberatt+i); 
        // assert(!newstack->empty());   // at least one element required

        tattr = newstack->top().value;
        Used_Memory -= sizeof(AggrStackEntry);
        newstack->pop();

        while (!newstack->empty()) {
          (*funargs)[0].setAddr(tattr);  
          sattr = newstack->top().value;
          (*funargs)[1].setAddr(sattr);  
          qp->Request( supp3, funres);   // call parameter function

          Used_Memory -= sizeof(AggrStackEntry) 
            + sattr->Sizeof() + sattr->getUncontrolledFlobSize()
            + tattr->Sizeof() + tattr->getUncontrolledFlobSize();
          sattr->DeleteIfAllowed();
          tattr->DeleteIfAllowed();

          tattr = ((Attribute*)funres.addr)->Clone();
          Used_Memory += tattr->Sizeof() + tattr->getUncontrolledFlobSize();
          newstack->pop();
        } // end-while

        // write the end result to the tuple
        t->PutAttribute(numberatt+i, tattr);
        Used_Memory -= sizeof(*newstack);
        delete newstack;
      } // end-if
    } // end-for
  } // end ShrinkStack

/*
2.3.5 Function RestoreGroup

Restore a group tuple to memory.

*/

  void RestoreGroup (Tuple* t)
  { 
    size_t hash1, hash2;
    int i, k;
    AggrStackEntry FirstEntry;
    stack <AggrStackEntry> *newstack;

    Used_Memory += t->GetExtSize();
    // get the hash code
    hash1 = 0;
    for (k = 0; k < numberatt; k++) hash1 += t->HashValue(k);
    hash2 = hash1 % NUMBUCKETS;
    if (hash2 < 0) hash2 = -1 * hash2;

    // create one element stacks for symmetric merge
    for (i=0; i < noOffun; i++) {
      if (MergeType[i] == Symmetric_Merge) {
        // build a stack element from the attribute, push this
        FirstEntry.level = 0;
        FirstEntry.value = t->GetAttribute(numberatt + i);
        // increment reference counter to survice putattribute
        FirstEntry.value->Copy();
        newstack = new stack<AggrStackEntry> ();
        newstack->push(FirstEntry);
        Used_Memory += sizeof( *newstack) + sizeof(AggrStackEntry);
        // Link the stack into the tuple after the group attributes 
        t->PutAttribute(numberatt + i, (Attribute*) newstack);
      } // end-if per attribute
    } 
    // insert group tuple into vector
    hBucket[hash2].push_back(t);
  } // end RestoreGroup


  ~groupby2LocalInfo() {
     if(resultTupleType) {
       resultTupleType->DeleteIfAllowed();
       resultTupleType = 0;
     }

     delete TB_Group;   
     delete TB_In;
     delete TB_Out;

     if(MergeType) {
       delete[] MergeType;
       MergeType = 0;
     }
     if(attrSizeTmp) {
       delete[] attrSizeTmp;
       attrSizeTmp = 0;
     }
     if(attrSizeExtTmp) {
       delete[] attrSizeExtTmp;
       attrSizeExtTmp = 0;
     }
  } // end destructor



/*
2.3.6 Function ReportStatus

Test function: report status information on processing in phases.

*/

  void ReportStatus (string text)
  { 
    cout << endl << text << endl;
    cout << "Phase:             " << Phase << endl;
    cout << "Used_Memory:       " << Used_Memory << endl;
    cout << "#groups returned. No_RetTuples = " << No_RetTuples << endl;
    cout << "#groups created. No_GTuples    = " << No_GTuples << endl;    
    cout << "#groups in TB_Group:    " << TB_Group->GetNoTuples() << endl;  
    cout << "#raw tuples in TB_Out:  " << TB_Out->GetNoTuples()  << endl;  
    cout << "GetTotalSize() of TB_Out:  " << TB_Out->GetTotalSize()  << endl; 
    cout << "Sum GTuples returned:  " << SumGTuples  << endl; 
    cout << "Sum ITuples processed: " << read  << endl;
    cout << "Sum Data sec.storage:  " << SumDiskData  << endl; 
  } // end of ReportStatus
 

/*
2.3.7 Function M3Cost

Calculate cost model M3 in milliseconds.
n=no input tuples, g=no groups, tpct=fraction of input tupes merged already
tpct=0: cost of complete model; tpct in 0-1: cost of remaining problem

*/

  float M3Cost (float n, float g, float tpct)
  { 
    const double model_ct2 = 8.35E-08;
    const double model_cg  = 3.35E-07;
    const double model_cf  = 1.46E-06;
    const double model_cio = 6.45E-07;
    double model_n;
    double model_g;
    double model_np;
    double model_gp;
    double model_P;
    double groupby2_cost;
    float tuple_size;
    float completed;

    model_n = n;
    model_g = g;

    model_gp = (Phase == 1) ? 
      (float)MAX_MEMORY / (float)Used_Memory * No_GTuples 
      : (float)SumGTuples/(float)(Phase-1);
    model_P = ceil(model_g / model_gp);
    model_np = model_n / model_g * model_gp;

    // this is the model M3 cost formula; cost in milliseconds
    // tuple processing cost
    groupby2_cost = (1.0-tpct) * max(0.0, (model_P*(model_n-
      (model_P-1)/2*model_np)-model_g)*ceil(model_gp/(2*NUMBUCKETS))*model_ct2);
    
    // group building cost
    groupby2_cost += (1.0-tpct) * 
                     model_g * ceil(model_gp/(2*NUMBUCKETS)) * model_cg;
    // function evaluation cost
    groupby2_cost += (1.0-tpct) * model_n * noOffun * model_cf;

    // disk storage cost
    if (Phase > 1 && TB_In->GetNoTuples() > 0)
      tuple_size = TB_In->GetTotalSize() / TB_In->GetNoTuples();
    else if (Phase == 1 && TB_Out->GetNoTuples() > 0)
      tuple_size = TB_Out->GetTotalSize() / TB_Out->GetNoTuples();
    else
      tuple_size = 100; // just to assume something

    // calculate the fraction of spooled input tuples
    if (tpct < 0.05)
      completed = 0;    
    else if (Phase ==1)
      completed = TB_Out->GetNoTuples()/
                  (2.0*(model_P-1)*(model_n-model_P/2.0*model_np));
    else
      completed = 
        (SumITuples - (TB_In->GetNoTuples()-read_this_phase)/2 
         + TB_Out->GetNoTuples()/2)
        / ((model_P-1)*(model_n-model_P/2.0*model_np));

    groupby2_cost += max(0.0, (1.0-completed) * tuple_size * model_cio * 
      (model_P-1)*(model_n-model_P/2.0*model_np));

    groupby2_cost *= 1000;          
    return (groupby2_cost);
  } // end of M3Cost

}; // end of class groupby2LocalInfo


/*
2.4 Value Mapping Function

The argument vector contains the following values:                            \\
args[0] =     input stream of tuples                                          \\
args[1] =     list of grouping attributes                                     \\
args[2] =     list of functions (with elements name, function, initial value) \\
args[3] =     number of grouping attributes (added by APPEND)                 \\
args[4..m] =  position of grouping attributes (added by APPEND)               \\      
args[m] =     number of aggregate functions (added by APPEND)                 \\
args[m+1..] = type of merging for each aggregate function                     \\

Sample with three grouping attributes and two aggregate functions:            \\ 
     APPEND (3 1 2 3 2 1 0) is created during type mapping. 

The result is:    \\
     arg[3] = 3    Number of grouping attributes                              \\
     arg[4] = 1    Index of first grouping attribute within tuple             \\
     arg[5] = 2    Index of second grouping attribute within tuple            \\
     arg[6] = 3    Index of third grouping attribute within tuple             \\
     arg[7] = 2    Number of aggregate functions                              \\
     arg[8] = 1    Symmetric merging required for function 1                  \\
     arg[9] = 0    Normal merging required for function 0   


*/

int groupby2ValueMapping (Word* args, Word& result, int message, Word& local, 
                          Supplier supplier)
{
  Word sWord(Address(0));
  groupby2LocalInfo *gbli = (groupby2LocalInfo *) local.addr;
  ListExpr resultType;
  Tuple *current = NULL, *s = NULL, *tres = NULL;
  int attribIdx = 0, vectorpos;
  int PosCountArgument = 3; // position of numberatt info
  // start if positions for grouping attributes
  int PosExtraArguments = PosCountArgument + 1; 
  int i, j, k; 
  int TuplesInBucket = 0;
  size_t myhash1, myhash2;
  bool SameGroup, DuplicateGroup;
  Word funres;
  int funstart;      // position of aggr. function info 
  // for progress estimation
  ProgressInfo p1;
  ProgressInfo *pRes; 

  // constants for cost estimation
  const double model_cf  = 1.46E-06;
  double mod_cost, mod_cost_rest;
  double mod_progress;
  double mod_n;
  double mod_g;
  double tuple_consumed;


  switch(message)
  {
/*
2.4.1 OPEN message processing

*/
    case OPEN:
      qp->Open(args[0].addr);
      qp->Request(args[0].addr, sWord);
     
      // Allocate localinfo class and store first tuple
      if (qp->Received(args[0].addr)) {
        if (gbli) delete gbli;
        gbli = new groupby2LocalInfo();
        local.setAddr(gbli);

        gbli->read = 1;
        gbli->t = (Tuple*)sWord.addr;
        resultType = GetTupleResultType(supplier);
        gbli->resultTupleType = new TupleType(nl->Second(resultType));

        // no of grouping attributes
        gbli->numberatt = 
          ((CcInt*)args[PosCountArgument].addr)->GetIntval();
        // get the APPEND info on merge type for aggregate funcions 
        funstart = PosCountArgument + gbli->numberatt + 1;
        // no of attribute functions to build
        gbli->noOffun = ((CcInt*)args[funstart].addr)->GetIntval();
        gbli->noAttrs = gbli->numberatt + gbli->noOffun;

        // check how the aggregates need to be merged
        gbli->MergeType = new int [gbli->noOffun];
        for (i=0; i < gbli->noOffun; i++)
          gbli->MergeType[i] = ((CcInt*)args[funstart+i+1].addr)->GetIntval();

        // TupleBuffers without memory, all tuples go to disk
        gbli->Phase = 1;
        gbli->newGroupsAllowed = true;
        gbli->TB_Group = new TupleBuffer(0);
        gbli->TB_In = new TupleBuffer(0);
        gbli->TB_Out = new TupleBuffer(0);
        gbli->MAX_MEMORY = (qp->GetMemorySize(supplier) * 1024 * 1024);

        cmsg.info("ERA:ShowMemInfo") << "groupby2.MAX_MEMORY ("
                   << (gbli->MAX_MEMORY)/1024 << " KiloByte): " << endl;
        cmsg.send();

        // Adjust memory avaliable for LocalInfo (around 1.2 MB)
        gbli->MAX_MEMORY -= sizeof(*gbli);
        // need at least 1 MB memory to run
        assert(gbli->MAX_MEMORY > 1048576); 

        // initialize data for progress estimation
        gbli->attrSizeTmp = new double[gbli->noAttrs];
        gbli->attrSizeExtTmp = new double[gbli->noAttrs];
        for (int i = 0; i < gbli->noAttrs; i++) {
          gbli->attrSizeTmp[i] = 0.0;
          gbli->attrSizeExtTmp[i] = 0.0;
        }
      } else {
        local.setAddr(0);         // no tuple received
      }
      return 0;

/*
2.4.2 REQUEST message processing

*/
    case REQUEST:
      if (!gbli) return CANCEL;                 // empty input stream
      if (gbli->t == 0) return CANCEL;          // stream has ended

      if (gbli->FirstREQUEST) {
        // Test
        // gbli->ReportStatus( "Start (FirstREQUEST) of a Phase." );

        // first REQUEST call: aggregate and return first result tuple 
        s = gbli->t;        // first tuple is available from OPEN
        gbli->FirstREQUEST = false;
/*
2.4.3 First REQUEST message: Aggregation without grouping

*/
        if (gbli->numberatt == 0) {
          // there is a single result tuple with function aggregates only
          tres = new Tuple(gbli->resultTupleType);

          // process the first input tuple
          gbli->InitTuple( tres, s, (Supplier) args[2].addr); 
          s->DeleteIfAllowed();

          qp->Request(args[0].addr, sWord);      // get next tuple
          s = (Tuple*) sWord.addr;

          // main loop to process input tuples
          while (qp->Received(args[0].addr)) {
            gbli->read++;

            // Get function value from tuple and current value 
            gbli->AggregateTuple( tres, s, (Supplier) args[2].addr); 
            s->DeleteIfAllowed();
            // get next tuple
            qp->Request(args[0].addr, sWord);
            s = (Tuple*) sWord.addr;
          }  // end-while: get tuples 

          // end processing for symmetric merging 
          gbli->ShrinkStack (tres, (Supplier) args[2].addr); 
          // tres is the completed result tuple            
          result.addr = tres;
          return YIELD;
        } // end-if: aggregation without grouping

/*
2.4.4 First REQUEST message: Aggregation with grouping 

*/
        // process individual tuple s
        while (s) {
        // get the hash value from the grouping attributes
          myhash1 = 0;
          for (k = 0; k < gbli->numberatt; k++) {
            attribIdx = 
              ((CcInt*)args[PosExtraArguments+k].addr)->GetIntval();
            j = attribIdx-1;                // 0 based
            myhash1 += s->HashValue(j);
          }
          myhash2 = myhash1 % NUMBUCKETS;
          // myhash2 can overflow and would be negative then
          if (myhash2 < 0) myhash2 = -1 * myhash2;

          // check the hash bucket if the group is created already
          DuplicateGroup = false;
          TuplesInBucket = gbli->hBucket[myhash2].size();

          // Compare new tuple s to the tuples available in the bucket          
          // s: group attributes Gi o non grouping attributes Ai
          // tuples from bucket: group attributes Gi o function values
          for (i=0; (i<TuplesInBucket) && !DuplicateGroup; i++) {
            current = gbli->hBucket[myhash2][i];
            if (!current) break;
            // Compare new tuple to a single one from the hash bucket
            SameGroup = true;
            // Compare the grouping attributes
            for (j=0; (j < gbli->numberatt) && SameGroup && current; j++) {
              attribIdx = 
                ((CcInt*)args[PosExtraArguments+j].addr)->GetIntval();
              k = attribIdx - 1;        // 0 based
              // Compare each attribute
              // k is index for input tuple. j is index for aggregate tuple
              if (s->GetAttribute(k)->Compare(current->GetAttribute(j)) != 0){
                SameGroup = false;
                break;
              }
            } // end-for
            if (SameGroup) {
              DuplicateGroup = true;
              vectorpos = i;    // save the index in the vector for deleting
            }
          } // end for

          if ((DuplicateGroup == false) && 
              (gbli->Used_Memory >= gbli->MAX_MEMORY || 
               gbli->newGroupsAllowed == false)  ) {
            // new group required but no more heap space available or
            // no new groups allowed: write original input tuple to tuple buffer
            gbli->TB_Out->AppendTuple(s);
       
          } else if (DuplicateGroup == false) {  // new group ==================
            // Build group aggregate: grouping attributes o function values
            tres = new Tuple(gbli->resultTupleType);
            // add the memory used by this raw tuple (without attributes)
            gbli->Used_Memory += sizeof(*tres);
            gbli->No_GTuples++;
            gbli->tup_aggr++;

            // copy grouping attributes 
            for(i = 0; i < gbli->numberatt; i++) {
              attribIdx = 
                ((CcInt*)args[PosExtraArguments+i].addr)->GetIntval();
              tres->CopyAttribute(attribIdx-1, s, i);
            }

            // process first input tuple for this group
            gbli->InitTuple( tres, s, (Supplier) args[2].addr); 

            // store aggregate tuple in hash bucket
            gbli->hBucket[myhash2].push_back(tres);
            s->DeleteIfAllowed();

          } else {  // group exists already ===================================
            gbli->AggregateTuple( current, s, args[2].addr);
            gbli->tup_aggr++;

            // memory exceeded: group tuple needs to be transferred to disk
            if (gbli->Used_Memory >= gbli->MAX_MEMORY) {
              gbli->newGroupsAllowed = false;
              gbli->ShrinkStack( current, args[2].addr);
              gbli->TB_Group->AppendTuple( current);
              gbli->Used_Memory -= current->GetExtSize();
              gbli->hBucket[myhash2].
                erase( gbli->hBucket[myhash2].begin()+vectorpos);
            }
            s->DeleteIfAllowed();
          }  // end-if

          // get next tuple
          if (gbli->Phase == 1) {
            // read from input stream           
            qp->Request(args[0].addr, sWord);
            s = (Tuple*)sWord.addr;
            gbli->read++;
          } else {
            // read from tuple buffer
            s = gbli->In_Rit->GetNextTuple();
            gbli->read++;
            gbli->read_this_phase++;
          }
        } // end-while 

        // all input processed; delete the tuple buffer scan        
        if (gbli->In_Rit) delete gbli->In_Rit;

        // Aggregates are built completely. Find and return first result tuple.
        result.addr = 0;
        // Find the first hash bucket containing a result tuple
        for(i = 0; i<NUMBUCKETS; i++) {
          TuplesInBucket = gbli->hBucket[i].size();

          if (TuplesInBucket > 0) {
            gbli->ReturnElem = 1;        // next element (0 based)
            gbli->ReturnBucket = i+1;    // next hash bucket
            current = gbli->hBucket[i][0];
            // end processing for symmetric merging attributes
            gbli->ShrinkStack (current, (Supplier) args[2].addr); 
            result.setAddr(current);
            gbli->No_RetTuples = 1;
            return YIELD;
          }
        } // end-for
        // empty result
        return CANCEL;
/*
2.4.5 Following REQUEST messages: return result tuples

*/
      } else {
        result.addr = 0;          // did not yet find a tuple

        // no grouping: there is a single result tuple, 
        // this was returned already
        if (gbli->numberatt == 0) return CANCEL;

        // If a scan is active or there are still buckets to process
        if (gbli->ReturnElem || gbli->ReturnBucket < NUMBUCKETS) {
          // Find the next result tuple. Process an active scan
          if (gbli->ReturnElem) { 
            // There are still tuples available in this bucket
            if (gbli->ReturnElem 
                < gbli->hBucket[(gbli->ReturnBucket)-1].size() ) {
              current = gbli->hBucket[(gbli->ReturnBucket)-1][gbli->ReturnElem];
              (gbli->ReturnElem)++;
              result.setAddr(current);            
            } else
              gbli->ReturnElem = 0;
          } // end-if

          // No tuples found so far, check next bucket
          if (result.addr == 0 && (gbli->ReturnBucket) < NUMBUCKETS) {
            for(i = gbli->ReturnBucket; i<NUMBUCKETS; i++) {
              TuplesInBucket = gbli->hBucket[i].size();

              if (TuplesInBucket > 0) {
                current = gbli->hBucket[i][0];      // first tuple in bucket
                result.setAddr(current);
                gbli->ReturnElem = 1;        // next tuple
                gbli->ReturnBucket = i+1;    // next bucket
                break;
              } else {
                gbli->ReturnElem = 0;
              }
            } // end-for
          }  // end-if
        } // end-if

        if (current) 
          // found a group tuple to return: end processing 
          gbli->ShrinkStack (current, (Supplier) args[2].addr); 
        else if (gbli->TB_Out->GetNoTuples() == 0  && 
                 gbli->TB_Group->GetNoTuples() > 0) 
        { // all groups from memory returned, all input tuples processed
          // completed group tuples stored in TB_Group to return
          // end of program run, no next phase
          if (gbli->Phase > 0) {
            gbli->Phase = 0;        // indicate end of program run
            gbli->In_Rit = gbli->TB_Group->MakeScan();
          }
          current = gbli->In_Rit->GetNextTuple();
          result.setAddr(current);
        }

        if (result.addr) {
          (gbli->No_RetTuples)++;

          // sum up attribute sizes for progress information
          if ( gbli->No_RetTuples <= gbli->stableValue ) {
            for (int i = 0; i < gbli->noAttrs; i++) {
              gbli->attrSizeTmp[i] += current->GetSize(i);
              gbli->attrSizeExtTmp[i] += current->GetExtSize(i);
            }
          }
/*
2.4.6 Phase change

*/
          // last group tuple from memory is returned,
          // output buffer contains unprocessed tuples
          if ((gbli->No_RetTuples + gbli->TB_Group->GetNoTuples()
               == gbli->No_GTuples)
              && (gbli->TB_Out->GetNoTuples() > 0)) 
          {
            // update progress data 
            gbli->SumITuples += gbli->TB_Out->GetNoTuples();
            gbli->SumDiskData += gbli->TB_Out->GetTotalSize();
            gbli->SumGTuples += gbli->No_RetTuples;
            if (gbli->Phase==1) gbli->tup_n = gbli->read;

            // Test
            // gbli->ReportStatus( "End of a Phase." );

            // initialize counters for next phase
            gbli->FirstREQUEST = true;   // need to aggregate on next REQUEST
            gbli->No_RetTuples = 0;      // init for next phase
            gbli->No_GTuples = 0;
            gbli->read_this_phase=0;
            (gbli->Phase)++;

            // clear all vectors, the tuples are not touched
            for (i=0; i<NUMBUCKETS; i++) gbli->hBucket[i].clear();
            // the result tuples are now owned by the following operator
            gbli->Used_Memory = 0;
    
            // bring back to memory as many group tuples as possible
            if (gbli->TB_Group->GetNoTuples() > 0) {
              gbli->In_Rit = gbli->TB_Group->MakeScan();
              current = gbli->In_Rit->GetNextTuple();

              while (current && (gbli->Used_Memory < gbli->MAX_MEMORY)) {
                gbli->RestoreGroup( current);
                gbli->No_GTuples++;         
                current = gbli->In_Rit->GetNextTuple();
              }
              gbli->TB_Temp = new TupleBuffer(0);
              if (current) {
                // write the remaining tuples to a new tuple buffer
                while (current) {
                  gbli->TB_Temp->AppendTuple(current);
                  current = gbli->In_Rit->GetNextTuple();
                } 
              } else {
                // all groups could be restored new ones can be created
                gbli->newGroupsAllowed = true;
              }
              delete gbli->In_Rit;
              delete gbli->TB_Group;
              gbli->TB_Group = gbli->TB_Temp;
            } // end-if; bring back group tuples

            delete gbli->TB_In;   // input was processed completely
            // the output buffer of the current phase becomes the input for
            // the next phase
            gbli->TB_In = gbli->TB_Out;
            gbli->TB_Out = new TupleBuffer(0);
    
            // get the first input tuple
            gbli->In_Rit = gbli->TB_In->MakeScan();
            gbli->t = gbli->In_Rit->GetNextTuple();
          } // end of phase change

          return YIELD;
        } else {
            // update progress data 
          gbli->SumITuples += gbli->TB_Out->GetNoTuples();
          gbli->SumDiskData += gbli->TB_Out->GetTotalSize();
          gbli->SumGTuples += gbli->No_RetTuples;

          // Test
          // gbli->ReportStatus( "End of a Phase." );        
          return CANCEL;
        }
     } // end-if REQUEST processing

/*
2.4.7 CLOSE message processing

*/
    case CLOSE: 
      qp->Close(args[0].addr);      // close input stream
      return 0;

/*
2.4.8 CLOSEPROGRESS message processing

*/
    case CLOSEPROGRESS: 
      if (gbli) {
        delete gbli;
        local.setAddr(0);
      }
      return 0;

/*
2.4.9 REQUESTPROGRESS message processing

*/
    case REQUESTPROGRESS:
      pRes = (ProgressInfo*) result.addr;
      if (!gbli) return CANCEL;

      if (qp->RequestProgress(args[0].addr, &p1) ) {
        gbli->sizesChanged = false;

        if (!gbli->sizesInitialized) {
          gbli->attrSize = new double[gbli->noAttrs];
          gbli->attrSizeExt = new double[gbli->noAttrs];
        }

        // the third condition makes sure that the actual sizes are used
        if (!gbli->sizesInitialized || p1.sizesChanged ||
            (gbli->No_RetTuples > gbli->stableValue && !gbli->sizesFinal)) {

          if (gbli->No_RetTuples < gbli->stableValue) {
            // for grouping atts: copy predecessor info
            for (i=0; i < gbli->numberatt; i++) {
              attribIdx = ((CcInt*)args[PosExtraArguments+i].addr)->GetIntval();
              gbli->attrSize[i] = p1.attrSize[attribIdx-1];
              gbli->attrSizeExt[i] = p1.attrSizeExt[attribIdx-1];   
            }
            // for aggregation results: assume integer
            for (i=0; i < gbli->noOffun; i++) {
              gbli->attrSize[i+gbli->numberatt] = 12;
              gbli->attrSizeExt[i+gbli->numberatt] = 12;   
            }
          } else {           
            // actual sizes from returned group tuples
            for (int i = 0; i < gbli->noAttrs; i++) {
              gbli->attrSize[i] = gbli->attrSizeTmp[i] / gbli->stableValue;
              gbli->attrSizeExt[i] = gbli->attrSizeExtTmp[i]/gbli->stableValue;
            }
            // this is run only once
            gbli->sizesFinal = true;
          }

          // summary sizes
          gbli->Size = 0.0;
          gbli->SizeExt = 0.0;
          for (int i = 0; i < gbli->noAttrs; i++) {
            gbli->Size += gbli->attrSize[i];
            gbli->SizeExt += gbli->attrSizeExt[i];
          }

          gbli->sizesInitialized = true;
          gbli->sizesChanged = true;
        }
        pRes->CopySizes(gbli);
        pRes->noAttrs = gbli->noAttrs;

        // write progress information
        if (gbli->numberatt == 0 ||
             (gbli->Phase == 1 && gbli->newGroupsAllowed && 
              gbli->No_GTuples < NUMBUCKETS)) {
          // aggregate without grouping, use model M1   OR
          // Phase 1, small number of groups only; cost in milliseconds
          mod_cost = p1.Card * gbli->noOffun * model_cf * 1000;
        
          pRes->Card = (gbli->numberatt == 0) ? 1 : gbli->No_GTuples;
          pRes->Time = p1.Time + mod_cost;
          pRes->Progress = 
            (p1.Progress*p1.Time + gbli->read/p1.Card*mod_cost)/ pRes->Time;
          pRes->BTime = pRes->Time;      
          pRes->BProgress = pRes->Progress;

        } else {
          mod_n = (gbli->Phase == 1) ? p1.Card : gbli->tup_n;
          // fraction of input tuples already aggregated
          tuple_consumed = (gbli->Phase == 1) ?
            (float)(gbli->read - gbli->TB_Out->GetNoTuples()) / mod_n 
            : (float) gbli->tup_aggr / mod_n;
          mod_g = (gbli->No_GTuples + gbli->SumGTuples)/tuple_consumed;

          // cost for complete problem
          mod_cost = gbli->M3Cost( mod_n, mod_g, 0.0);
          // cost for remaining piece of problem
          mod_cost_rest = gbli->M3Cost( mod_n, mod_g, tuple_consumed);

          pRes->Card = mod_g;
          pRes->Time = p1.Time + mod_cost;
          mod_progress = (mod_cost - mod_cost_rest) / mod_cost;
          pRes->Progress = 
            (p1.Progress*p1.Time + mod_progress*mod_cost) / pRes->Time;
          pRes->BTime = (gbli->Phase == 1) ? pRes->Time : 0;      
          pRes->BProgress = (gbli->Phase == 1) ? pRes->Progress : 1;
        }
        return YIELD;
      } else {
        return CANCEL;
      }
  } // end message switch

  return(0);
} // Ende groupby2ValueMapping ================================================


/*
2.5 Operator definition

*/

Operator groupby2 (
         "groupby2",             // name
         groupby2Spec,           // specification
         groupby2ValueMapping,   // value mapping
         Operator::SimpleSelect, // trivial selection function
         groupby2TypeMap         // type mapping; 
);


/*

3 Class GroupbyAlgebra

A new subclass GroupbyAlgebra of class Algebra is declared. The only
specialization with respect to class Algebra takes place within the
constructor: all type constructors and operators are registered at the
actual algebra.

*/


class GroupbyAlgebra : public Algebra
{
 public:
  GroupbyAlgebra() : Algebra()
  {
    AddOperator(&groupby2);    
    groupby2.SetUsesMemory();
    groupby2.EnableProgress();
  };

  ~GroupbyAlgebra() {};
};

/*

4 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

*/

extern "C" Algebra*
InitializeGroupbyAlgebra( NestedList* nlRef,
                          QueryProcessor* qpRef,
                          AlgebraManager* amRef )
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return (new GroupbyAlgebra());
}


