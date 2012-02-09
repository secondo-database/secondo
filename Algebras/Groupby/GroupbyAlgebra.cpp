/*
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
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
----

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]

 

Dieter Capek: Module Groupby Algebra

from Sept 2011    Implementation of Module Groupby Algebra
15.12.2011         Changed data type for hash buckets to STL vectors
19.01.2012         Introduced symmetric merging
23.01.2012         Refined UsedMemory calculation
08.01.2010         Phases to handle memory constraints

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

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

using namespace listutils;

const string groupby2Spec = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\"  ) "
  "( <text>stream(Tuple) x AttrList x "
  "(NewAttr-1 x (Tuple x Data -> Data) x Data) .. "
  "(NewAttr-j  x (Tuple x Data -> Data) x Data) -> "
  "stream(Tuple(Attrlist) o Tuple([NewAttr-1: Data]..[NewAttr-j: Data])"
  "</text--->"

  "<text>_ groupby2 [list; funlist]</text--->"
  
  "<text>groupby2: Groups a tuple stream according to the attributes "
  "in AttrList and computes aggregate functions for each group. "
  "The result functions are appended to the grouping attributes." 
  "</text--->"
  ") )";


#define NUMBUCKETS 99997
#define Normal_Merge 0
#define Symmetric_Merge 1


// Type Mapping Funktion für groupby2 =========================================
ListExpr groupby2TypeMap(ListExpr args)
{
  ListExpr first, second, third;     // list used for analysing input

  // listp will contain the positions of the groupint attributes
  // listn will contain the names and data types
  ListExpr listn, lastlistn, listp, lastlistp;  // for constructing output
  ListExpr first2, t, result, attrtype, t1, t2, t3, t4;
  ListExpr rest, firstr, newAttr, mapDef, firstInit, mapOut;
  ListExpr merge, lastmerge;     // indicates merge type for aggregate functions


  string err = 
    "stream(tuple(X)) x (g1..gn) x (tuple(X)xtxt -> t), t in DATA expected";
  string resstring;
 
  first = second = third = nl->TheEmptyList();
  listn = lastlistn = listp = nl->TheEmptyList();
  merge = lastmerge = nl->TheEmptyList();

  string relSymbolStr = Relation::BasicType();
  string tupleSymbolStr = Tuple::BasicType();

 
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
  bool firstcall = true;

  while (!nl->IsEmpty(rest))
  {
    attrtype = nl->TheEmptyList();
    first2 = nl->First(rest);
    if(nl->AtomType(first2)!=SymbolType)
      return listutils::typeError("Wrong format for an attribute name");

    string attrname = nl->SymbolValue(first2);

    // Get position of attribute within tuple
    int j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);

    if (j) {
      if (!firstcall) {
        lastlistn = nl->Append(lastlistn,nl->TwoElemList(first2,attrtype));
        lastlistp = nl->Append(lastlistp,nl->IntAtom(j));
      } else {
        firstcall = false;
        listn = nl->OneElemList(nl->TwoElemList(first2,attrtype));
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
    firstr = nl->First(rest);  // functions
    rest = nl->Rest(rest);

    // Format must be Name:Funktion::Initial Value 
    if(nl->ListLength(firstr) != 3)
      return listutils::typeError("Each function must have three elements.");

    newAttr  = nl->First(firstr);       // function name
    mapDef   = nl->Second(firstr);      // aggregate function 
    firstInit = nl->Third(firstr);      // function definition or inital value

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

// Abbruch
//  return listutils::typeError("Type Mapping End. Abbruch.");

  return result;
}  // end of type mapping for groupby2 operator



struct AggrStackEntry {
  int level;
  Attribute* value;
};


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
    Phase;             // phase the operator currently runs in

  // progress information
  int stableValue;
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
    stableValue(50),sizesFinal(false),
    attrSizeTmp(0), attrSizeExtTmp(0)
  {}


  void InitTuple (Tuple* tres, Tuple* s, Supplier addr)  
  { // get first function values from tuple and initial values
    // tres is the group tuple, s the tuple from the input stream
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


  void AggregateTuple (Tuple* tres, Tuple* s, Supplier addr)  
  { // aggregate input tuple s into group tuple tres
    // tres is the group tuple, s the tuple from the input stream
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


  void ShrinkStack (Tuple* t, Supplier addr)  
  { // shrink the stacks for symmetric merging to a regular tuple
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


  void RestoreGroup (Tuple* t)
  { // restore a group tuple to memory
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


  // Test function
  void ReportStatus (string text)
  { // report information on processing
    cout << endl << text << endl;
    cout << "Phase:             " << Phase << endl;
    cout << "Used_Memory:       " << Used_Memory << endl;
    cout << "#groups returned. No_RetTuples = " << No_RetTuples << endl;
    cout << "#groups created. No_GTuples    = " << No_GTuples << endl;    
    cout << "#groups in TB_Group:    " << TB_Group->GetNoTuples() << endl;  
    cout << "#raw tuples in TB_Out:  " << TB_Out->GetNoTuples()  << endl;  
  } // end of ReportStatus
}; 


// Value Mapping für groupby2 =================================================
int groupby2ValueMapping (Word* args, Word& result, int message, Word& local, 
                          Supplier supplier)
{
  // The argument vector contains the following values:
  // args[0] = input stream of tuples
  // args[1] = list of grouping attributes
  // args[2] = list of functions (with elements name, function, initial value)
  // args[3] = number of grouping attributes (added by APPEND)
  // args[4..m] =  position of grouping attributes (added by APPEND);
  // args[m] = number of aggregate functions (added by APPEND)
  // args[m+1 ..] = type of merging for each aggregate function, 
  //                values Normal_Merge, Symmetric_Merge

  /* Sample with three grouping attributes and two aggregate functions 
     APPEND (3 1 2 3 2 1 0) is created during type mapping. The result is:
     arg[3] = 3    Number of grouping attributes
     arg[4] = 1    Index of first grouping attribute within tuple
     arg[5] = 2    Index of second grouping attribute within tuple
     arg[6] = 3    Index of third grouping attribute within tuple
     arg[7] = 2    Number of aggregate functions
     arg[8] = 1    Symmetric merging required for function 1
     arg[9] = 0    Normal merging required for function 0   
  */

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
  const double ugroupby2 = 0.008;  


  switch(message)
  {
    case OPEN:
      // open the stream and get the first tuple ==============================
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

        // Test
        cout << "groupby2 Memory allowance: " << gbli->MAX_MEMORY << endl;

        cmsg.info("ERA:ShowMemInfo") << "groupby2.MAX_MEMORY ("
                   << (gbli->MAX_MEMORY)/1024 << " KiloByte): " << endl;
        cmsg.send();

        // Adjust memory avaliable for LocalInfo (around 1.2 MB)
        gbli->MAX_MEMORY -= sizeof(*gbli);

        // need at least 1 MB memory to run
        // ?? für Tests: heruntergesetzt
        // assert(gbli->MAX_MEMORY > 1048576); 
        assert(gbli->MAX_MEMORY > 50000); 
      } else {
        local.setAddr(0);         // no tuple received
      }
      return 0;

    case REQUEST:
      if (!gbli) return CANCEL;                 // empty input stream
      if (gbli->t == 0) return CANCEL;          // stream has ended

      if (gbli->FirstREQUEST) {
        // Test
        // gbli->ReportStatus( "Start (FirstREQUEST) of a Phase." );

        // first REQUEST call: aggregate and return first result tuple 
        s = gbli->t;        // first tuple is available from OPEN
        gbli->FirstREQUEST = false;

        // no grouping, aggretate totals only ===============================
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

        // aggregation with grouping =========================================
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

      // following REQUEST calls: return result tuples ========================
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

          // Phase change: last group tuple from memory is returned ===========
          // Output buffer contains unprocessed tuples
          if ((gbli->No_RetTuples + gbli->TB_Group->GetNoTuples()
               == gbli->No_GTuples)
              && (gbli->TB_Out->GetNoTuples() > 0)) 
          {
            // Test
            // gbli->ReportStatus( "End of a Phase." );

            gbli->FirstREQUEST = true;   // need to aggregate on next REQUEST
            gbli->No_RetTuples = 0;      // init for next phase
            gbli->No_GTuples = 0;
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
          // Test
          // gbli->ReportStatus( "End of a Phase." );        
          return CANCEL;
        }
     } // end-if REQUEST processing

    case CLOSE: 
      qp->Close(args[0].addr);      // close input stream
      return 0;

    case CLOSEPROGRESS: 
      if (gbli) {
        delete gbli;
        local.setAddr(0);
      }
      return 0;

    case REQUESTPROGRESS:
      pRes = (ProgressInfo*) result.addr;
      if (!gbli) return CANCEL;

      if (qp->RequestProgress(args[0].addr, &p1) ) {
        // first start: copy predecessor info
        // pRes->Copy(p1);

        // according Programmes Guide p 118
        // update size fields for output tuples
        gbli->sizesChanged = false;

        if (!gbli->sizesInitialized) {
          // gbli->noAttrs is set during OPEN
          gbli->attrSize = new double[gbli->noAttrs];
          gbli->attrSizeExt = new double[gbli->noAttrs];
        }

        if (!gbli->sizesInitialized || p1.sizesChanged) {
          gbli->Size = 0;
          gbli->SizeExt = 0;

          // for grouping atts: copy predecessor info
          for (i=0; i < gbli->numberatt; i++) {
            attribIdx = 
              ((CcInt*)args[PosExtraArguments+i].addr)->GetIntval();

            gbli->attrSize[i] = p1.attrSize[attribIdx-1];
            gbli->attrSizeExt[i] = p1.attrSizeExt[attribIdx-1];   
            gbli->Size += gbli->attrSize[i];
            gbli->SizeExt += gbli->attrSizeExt[i];
          }
          // ?? das als "kalte" Schätzung
          // ?? for aggregation results: assume integer
          for (i=0; i < gbli->noOffun; i++) {
            gbli->attrSize[i+gbli->numberatt] = 12;
            gbli->attrSizeExt[i+gbli->numberatt] = 12;   
            gbli->Size += gbli->attrSize[i+gbli->numberatt];
            gbli->SizeExt += gbli->attrSizeExt[i+gbli->numberatt];
          }

          gbli->sizesInitialized = true;
          gbli->sizesChanged = true;
        }

        // write progress information
        if (gbli->numberatt == 0)
          pRes->Card = 1;
        else
          pRes->Card = min( p1.Card/10, 
                            gbli->No_GTuples * min( 4.0, p1.Card/gbli->read)); 

        pRes->CopySizes(gbli);              // ?? enhält das die #Attribute

        // erste einfache Schätzung
        pRes->Time = p1.Time + (p1.Card * ugroupby2);
        // eigener Fortschritt proportional zum gelesenen Input
        pRes->Progress = (p1.Progress*p1.Time + gbli->read * ugroupby2)
                         / pRes->Time;

        // Annahme: groupby2 blockt zu 100%, Ausgabe ist unwesentlich
        pRes->BTime = p1.BTime + (p1.Card * ugroupby2);
        pRes->BProgress = (p1.BProgress*p1.BTime + gbli->read * ugroupby2)
                          / pRes->BTime;
        return YIELD;
      } else {
        return CANCEL;
      }

  } // end message switch

  return(0);
} // Ende groupby2ValueMapping ================================================


Operator groupby2 (
         "groupby2",             // name
         groupby2Spec,           // specification
         groupby2ValueMapping,   // value mapping
         Operator::SimpleSelect, // trivial selection function
         groupby2TypeMap         // type mapping; 
);


/*

3 Class ~GroupbyAlgebra~

A new subclass ~GroupbyAlgebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all type constructors and operators are registered at the
actual algebra.

After declaring the new class, its only instance ~extendedRelationAlgebra~
is defined.

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

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeGroupbyAlgebra( NestedList* nlRef,
                          QueryProcessor* qpRef,
                          AlgebraManager* amRef )
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return (new GroupbyAlgebra());
}


