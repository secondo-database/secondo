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
13.01.2012         Introduced phases to handle memory constraints
19.01.2012         Introduced symmetric merging
23.01.2012         Refined UsedMemory calculation 

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



#define CAPBUCKETS 99997
#define Normal_Merge 0
#define Symmetric_Merge 1


// Type Mapping Funktion für groupby2 =========================================
ListExpr GroupByTypeMapCap(ListExpr args)
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

// Diagnose
//  nl->WriteToString(resstring, args);
//  cout << "GroupbyTypeMapCap Input = " << resstring << endl;
 
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
 
// Testausgabe 
//  nl->WriteToString(resstring, result);
//  cout << "groupbyTypeMapCap Result = " << resstring << endl;

// Abbruch
//  return listutils::typeError("Type Mapping End. Abbruch.");

  return result;
}  // end of type mapping for groupby2 operator



// GroupByLocalInfo2 class ====================================================

class GroupByLocalInfo2
{
public:
  Tuple *t;
  TupleType *resultTupleType;

  // Actual buffers are created during OPEN
  TupleBuffer *TB_In, *TB_Out, *TB_Temp;
  GenericRelationIterator* In_Rit;

  long MAX_MEMORY, Used_Memory;   // max. memory and actually used memory
  bool FirstREQUEST;   // indicates the start of a phase
  unsigned int ReturnBucket, ReturnElem, 
    No_RetTuples,      // number of result tuples returned
    No_GTuples,        // number of group tuples in memory 
    Phase;             // phase the operator currently runs in

  vector<Tuple*> hBucket[CAPBUCKETS];   // data structure for hash buckets

  GroupByLocalInfo2() : t(NULL), resultTupleType(NULL), 
  TB_In(NULL), TB_Out(NULL), TB_Temp(NULL), In_Rit (NULL),
  MAX_MEMORY(0), Used_Memory(0), FirstREQUEST(true), ReturnBucket(0),
  No_RetTuples(0), No_GTuples(0), Phase(0)
  {}
};

struct AggrStackEntry {
  int level;
  Attribute* value;
};




// Value Mapping für groupby2 =================================================
int GroupByValueMapping2 (Word* args, Word& result, int message, Word& local, 
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
  GroupByLocalInfo2 *gbli = 0;
  ListExpr resultType;
  Tuple *current = NULL, *s = NULL, *tres = NULL;
  int numberatt = 0;            // number of grouping attributes
  int attribIdx = 0;
  int indexOfCountArgument = 3; // position of numberatt info
  // start if positions for grouping attributes
  int startIndexOfExtraArguments = indexOfCountArgument + 1; 
  int i, j, k; 
  Attribute *sattr, *tattr;
  int AnzahlTupelimBucket = 0;
  size_t myhash1, myhash2;
  bool GruppeGleich, GruppeDoppelt;
  ArgVectorPointer funargs;
  Word funres;
  Supplier supp1, supp2, supp3, supp4;
  int noOffun;                  // number of aggregate functions to build
  Supplier value2;
  int funstart, funagg;      // position of aggr. function info and value
  // declarations for symmetic merging (aggregateB algorithm)
  AggrStackEntry FirstEntry;   // first stack element
  stack<AggrStackEntry> *newstack = 0;
  int StackLevel = 0;



  switch(message)
  {
    case OPEN:
      // open the stream and get the first tuple ==============================
      qp->Open(args[0].addr);
      qp->Request(args[0].addr, sWord);
     
      // Allocate localinfo class and store first tuple
      if (qp->Received(args[0].addr)) {
        gbli = new GroupByLocalInfo2();
        local.setAddr(gbli);

        gbli->t = (Tuple*)sWord.addr;
        resultType = GetTupleResultType(supplier);
        gbli->resultTupleType = new TupleType(nl->Second(resultType));

        // TupleBuffers without memory, all tuples go to disk
        gbli->Phase = 1;
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
        assert(gbli->MAX_MEMORY > 1048576); 

      } else {
        local.setAddr(0);         // no tuple received
      }
      return 0;

    case REQUEST:
      // pointer to localinfo class ===========================================
      gbli = (GroupByLocalInfo2 *)local.addr;
      if (!gbli) return CANCEL;                 // empty input stream
      if (gbli->t == 0) return CANCEL;          // stream has ended
      numberatt = ((CcInt*)args[indexOfCountArgument].addr)->GetIntval();

      // get the APPEND info on merge type for aggregate funcions 
      funstart = indexOfCountArgument + numberatt + 1;
      funagg = ((CcInt*)args[funstart].addr)->GetIntval();

      int MergeType [funagg];
      for (i=0; i < funagg; i++)
        MergeType[i] = ((CcInt*)args[funstart+i+1].addr)->GetIntval();

      if (gbli->FirstREQUEST) {
        // Test
        cout << "Start (FirstREQUEST) of Phase: " << gbli->Phase << endl;

        // first REQUEST call: aggregate and return first result tuple 
        s = gbli->t;        // first tuple is available from OPEN
        gbli->FirstREQUEST = false;

        // get the number of functions
        value2 = (Supplier) args[2].addr;
        noOffun = qp->GetNoSons(value2);
        assert(noOffun == funagg);  // be sure on the # of functions

        // no grouping, aggretate totals only ===============================
        if (numberatt == 0) {
          // there is a single result tuple with function aggregates only
          tres = new Tuple(gbli->resultTupleType);

          // Process first input tuple: process initial value or create stack
          for (i=0; i < noOffun; i++) {
            if (MergeType[i] == Normal_Merge) {
              // normal merge, process initial value
              supp1 = (Supplier) args[2].addr;  // funlist: list of functions
              supp2 = qp->GetSupplier( supp1, i);
              supp3 = qp->GetSupplier( supp2, 1);
              funargs = qp->Argument(supp3);    // get function argument vector
              supp4 = qp->GetSupplier( supp2, 2); // supp4 is the initial value
              qp->Request( supp4, funres);        // function evaluation
              tattr = ((Attribute*)funres.addr)->Clone();

              (*funargs)[0].setAddr(s);           // tuple is first argument
              (*funargs)[1].setAddr(tattr);
              qp->Request( supp3, funres);        // function evaluation
              sattr = ((Attribute*)funres.addr)->Clone();
              tres->PutAttribute( i, sattr);      // link attribute into tuple
            } else {
              // symmetric merge
              // evaluate the tuple->data function
              supp1 = (Supplier) args[2].addr;  // funlist: list of functions
              supp2 = qp->GetSupplier( supp1, i);
              supp3 = qp->GetSupplier( supp2, 2);  // second function
              funargs = qp->Argument(supp3);
              (*funargs)[0].setAddr(s);           // tuple is first argument
              qp->Request( supp3, funres);        // function evaluation
              sattr = ((Attribute*)funres.addr)->Clone();

              // build a stack element from the attribute, push this
              FirstEntry.level = 0;
              FirstEntry.value = sattr;
              newstack = new stack<AggrStackEntry> ();
              newstack->push(FirstEntry);
              // Link the stack into the tuple 
              tres->PutAttribute(i, (Attribute*) newstack);
            } // end-if per attribute
          } // end-for    

          s->DeleteIfAllowed();

          qp->Request(args[0].addr, sWord);      // get next tuple
          s = (Tuple*) sWord.addr;

          // main loop to process input tuples
          while (qp->Received(args[0].addr)) {
            // Get function value from tuple and current value            
            for (i=0; i < noOffun; i++) {
              // get function arguments from qp
              supp1 = (Supplier) args[2].addr;    // list of functions
              supp2 = qp->GetSupplier( supp1, i);
              supp3 = qp->GetSupplier( supp2, 1);
              funargs = qp->Argument(supp3);      // get argument vector 

              if (MergeType[i] == Normal_Merge) {
                // normal merge, aggregate tuple and intermedite result
                sattr = tres->GetAttribute(i);      // result so far     
                (*funargs)[0].setAddr(s);           // tuple is first argument
                (*funargs)[1].setAddr(sattr);
                qp->Request( supp3, funres);        // call parameter function
                sattr = ((Attribute*)funres.addr)->Clone();
                tres->PutAttribute( i, sattr);
              } else {
                // symmetric merge, evaluate tuple->data, merge stack
                StackLevel = 0;
                // pointer to the stack
                newstack = (stack<AggrStackEntry> *) tres->GetAttribute(i); 
                
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
                  sattr = ((Attribute*)funres.addr)->Clone();

                  // delete top element
                  tattr->DeleteIfAllowed();
                  newstack->pop();
                  
                  StackLevel++;
                } //end-while  

                // write a stack element
                FirstEntry.level = StackLevel;
                FirstEntry.value = sattr;
                newstack->push(FirstEntry);
              }  // end-if process an attribute
            } // end-for 
 
            s->DeleteIfAllowed();
            // get next tuple
            qp->Request(args[0].addr, sWord);
            s = (Tuple*) sWord.addr;
          }  // end-while: get tuples 

          // end processing for symmetric merging only
          for (i=0; i < noOffun; i++) {
            if (MergeType[i] == Symmetric_Merge) {
              // get function arguments from qp
              supp1 = (Supplier) args[2].addr;    // list of functions
              supp2 = qp->GetSupplier( supp1, i);
              supp3 = qp->GetSupplier( supp2, 1);
              funargs = qp->Argument(supp3);      // get argument vector 

              // collapse the stack to an attribute value, delete the stack
              newstack = (stack<AggrStackEntry> *) tres->GetAttribute(i); 
              // Test
              assert(!newstack->empty());   // at least one element required

              tattr = newstack->top().value;
              newstack->pop();

              while (!newstack->empty()) {
                  (*funargs)[0].setAddr(tattr);  
                  sattr = newstack->top().value;
                  (*funargs)[1].setAddr(sattr);  
                  qp->Request( supp3, funres);   // call parameter function

                  sattr->DeleteIfAllowed();
                  tattr->DeleteIfAllowed();
                  tattr = ((Attribute*)funres.addr)->Clone();
                  newstack->pop();
              } // end-while

              // write the end result to the tuple
              tres->PutAttribute(i, tattr);
              delete newstack;
            } // end-if
          } // end-for

          // tres is the completed result tuple            
          result.addr = tres;
          return YIELD;
        } // end-if: aggregation without grouping

        // aggregation with grouping starts here ============================
        // process individual tuple s
        while (s) {
        // get the hash value from the grouping attributes
          myhash1 = 0;
          for (k = 0; k < numberatt; k++) {
            attribIdx = 
              ((CcInt*)args[startIndexOfExtraArguments+k].addr)->GetIntval();
            j = attribIdx-1;                // 0 based
            myhash1 += s->HashValue(j);
          }
          myhash2 = myhash1 % CAPBUCKETS;
          // myhash2 can overflow and would be negative then
          if (myhash2 < 0) myhash2 = -1 * myhash2;

          // check the hast bucket if the group is created already
          GruppeDoppelt = false;
          AnzahlTupelimBucket = gbli->hBucket[myhash2].size();

          // Compare new tuple s to the tuples available in the bucket          
          // s: group attributes Gi o non grouping attributes Ai
          // tuples from bucket: group attributes Gi o function values
          for (i=0; (i<AnzahlTupelimBucket) && !GruppeDoppelt; i++) {
            current = gbli->hBucket[myhash2][i];
            if (!current) break;

            // Compare new tuple to a single one from the hash bucket
            GruppeGleich = true;

            // Compare the grouping attributes
            for (j=0; (j < numberatt) && GruppeGleich && current; j++) {
              attribIdx = 
                ((CcInt*)args[startIndexOfExtraArguments+j].addr)->GetIntval();
              k = attribIdx - 1;        // 0 based
              // Compare each attribute
              // k is index for input tuple. j is index for aggregate tuple
              if (s->GetAttribute(k)->Compare(current->GetAttribute(j)) != 0){
                GruppeGleich = false;
                break;
              }
            } // end-for
            if (GruppeGleich) GruppeDoppelt = true;
          } // end for


          if ((GruppeDoppelt == false) && 
              (gbli->Used_Memory >= gbli->MAX_MEMORY)) {
            // no more heap space, write original input tuple to tuple buffer
            gbli->TB_Out->AppendTuple(s);
          
          } else if (GruppeDoppelt == false) {  // new group ==================
            // Build group aggregate: grouping attributes o function values
            tres = new Tuple(gbli->resultTupleType);
            // add the memory used by this raw tuple (without attributes)
            gbli->Used_Memory += sizeof(*tres);
            gbli->No_GTuples++;

            // copy grouping attributes 
            for(i = 0; i < numberatt; i++) {
              attribIdx = 
                ((CcInt*)args[startIndexOfExtraArguments+i].addr)->GetIntval();
              tres->CopyAttribute(attribIdx-1, s, i);

              // add memory sizes
              sattr = s->GetAttribute(attribIdx-1);
              gbli->Used_Memory += 
                sattr->Sizeof() + sattr->getUncontrolledFlobSize();
            }

            // get first function values from tuple and initial values
            for (i=0; i < noOffun; i++) {
              if (MergeType[i] == Normal_Merge) {
                supp1 = (Supplier) args[2].addr;    // list of functions
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
                tres->PutAttribute( numberatt+i, sattr); 
                gbli->Used_Memory += 
                  sattr->Sizeof() + sattr->getUncontrolledFlobSize();
    
              } else {
                // symmetric merge
                // evaluate the tuple->data function
                supp1 = (Supplier) args[2].addr;  // funlist: list of functions
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
                tres->PutAttribute(numberatt+i, (Attribute*) newstack);

                gbli->Used_Memory += sizeof( *newstack) 
                  + sizeof(AggrStackEntry)
                  + sattr->Sizeof() + sattr->getUncontrolledFlobSize();
              } // end-if per attribute
            } // end-for 

            // store aggregate tuple in hash bucket
            gbli->hBucket[myhash2].push_back(tres);

            // delete the tuple from the input stream
            s->DeleteIfAllowed();

          } else {  // group exists already ===================================

            // get function values n+1 from new tuple and function value n
            for (i=0; i < noOffun; i++) {
              supp1 = (Supplier) args[2].addr;    // list of functions
              supp2 = qp->GetSupplier( supp1, i);
              supp3 = qp->GetSupplier( supp2, 1);
              funargs = qp->Argument(supp3);      // get argument vector
              
              if (MergeType[i] == Normal_Merge) {
                // normal merge 
                sattr = current->GetAttribute( numberatt+i);  
                (*funargs)[0].setAddr(s);          
                (*funargs)[1].setAddr(sattr);
                qp->Request( supp3, funres);  
                tattr = ((Attribute*)funres.addr)->Clone();

                // subtract old attribute size
                gbli->Used_Memory -= 
                  sattr->Sizeof() + sattr->getUncontrolledFlobSize();
                // add new attribute size, it can change during merge
                gbli->Used_Memory += 
                  tattr->Sizeof() + tattr->getUncontrolledFlobSize(); 
                // PutAttribute does an implice DeleteIfAllowed on the old attr
                current->PutAttribute( numberatt+i, tattr);

              } else {
                // symmetric merge, evaluate tuple->data, merge stack
                StackLevel = 0;
                // pointer to the stack
                newstack = 
                  (stack<AggrStackEntry> *) current->GetAttribute(numberatt+i); 
                
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
                  sattr = ((Attribute*)funres.addr)->Clone();

                  // delete top element
                  gbli->Used_Memory -= sizeof(AggrStackEntry)
                    + tattr->Sizeof() + tattr->getUncontrolledFlobSize();
                  tattr->DeleteIfAllowed();
                  newstack->pop();

                  StackLevel++;
                } //end-while  

                // write a stack element
                FirstEntry.level = StackLevel;
                FirstEntry.value = sattr;
                newstack->push(FirstEntry);

                gbli->Used_Memory += sizeof(AggrStackEntry)
                  + sattr->Sizeof() + sattr->getUncontrolledFlobSize();
              }  // end-if process an attribute
            } // end-for 

            // delete tuple from input stream
            s->DeleteIfAllowed();
          }  // end-if

          // get next tuple
          if (gbli->Phase == 1) {
            // read from input stream           
            qp->Request(args[0].addr, sWord);
            s = (Tuple*)sWord.addr;
          } else {
            // read from tuple buffer
            s = gbli->In_Rit->GetNextTuple();
          }
        } // end-while 

        // all input processd; delete the tuple buffer scan        
        if (gbli->In_Rit) delete gbli->In_Rit;

        // Aggregates are built completely. Find and return first result tuple.
        result.addr = 0;
        // Find the first hash bucket containing a result tuple
        for(i = 0; i<CAPBUCKETS; i++) {
          AnzahlTupelimBucket = gbli->hBucket[i].size();

          if (AnzahlTupelimBucket > 0) {
            gbli->ReturnElem = 1;        // next element (0 based)
            gbli->ReturnBucket = i+1;    // next hash bucket
            current = gbli->hBucket[i][0];

            // end processing for symmetric merging attributes
            for (i=0; i < noOffun; i++) {
              if (MergeType[i] == Symmetric_Merge) {
                // get function arguments from qp
                supp1 = (Supplier) args[2].addr;    // list of functions
                supp2 = qp->GetSupplier( supp1, i);
                supp3 = qp->GetSupplier( supp2, 1);
                funargs = qp->Argument(supp3);      // get argument vector 

                // collapse the stack to an attribute value, delete the stack
                newstack = 
                  (stack<AggrStackEntry> *) current->GetAttribute(numberatt+i); 
                // Test
                assert(!newstack->empty());   // at least one element required

                tattr = newstack->top().value;
                newstack->pop();

                while (!newstack->empty()) {
                    (*funargs)[0].setAddr(tattr);  
                    sattr = newstack->top().value;
                    (*funargs)[1].setAddr(sattr);  
                    qp->Request( supp3, funres);   // call parameter function

                    sattr->DeleteIfAllowed();
                    tattr->DeleteIfAllowed();
                    tattr = ((Attribute*)funres.addr)->Clone();
                    newstack->pop();
                } // end-while

                // write the end result to the tuple
                current->PutAttribute(numberatt+i, tattr);
                delete newstack;
              } // end-if
            } // end-for

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
        if (numberatt == 0) return CANCEL;

        // If a scan is active or there are still buckets to process
        if (gbli->ReturnElem || gbli->ReturnBucket < CAPBUCKETS) {
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
          if (result.addr == 0 && (gbli->ReturnBucket) < CAPBUCKETS) {
            for(i = gbli->ReturnBucket; i<CAPBUCKETS; i++) {
              AnzahlTupelimBucket = gbli->hBucket[i].size();

              if (AnzahlTupelimBucket > 0) {
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


        // end processing for symmetric merging only
        if (current) {
          for (i=0; i < funagg; i++) {
            if (MergeType[i] == Symmetric_Merge) {
              // get function arguments from qp
              supp1 = (Supplier) args[2].addr;    // list of functions
              supp2 = qp->GetSupplier( supp1, i);
              supp3 = qp->GetSupplier( supp2, 1);
              funargs = qp->Argument(supp3);      // get argument vector 

              // collapse the stack to an attribute value, delete the stack
              newstack = 
                (stack<AggrStackEntry> *) current->GetAttribute(numberatt+i); 
              // Test
              assert(!newstack->empty());   // at least one element required

              tattr = newstack->top().value;
              newstack->pop();

              while (!newstack->empty()) {
                  (*funargs)[0].setAddr(tattr);  
                  sattr = newstack->top().value;
                  (*funargs)[1].setAddr(sattr);  
                  qp->Request( supp3, funres);   // call parameter function

                  sattr->DeleteIfAllowed();
                  tattr->DeleteIfAllowed();
                  tattr = ((Attribute*)funres.addr)->Clone();
                  newstack->pop();
              } // end-while

              // write the end result to the tuple
              current->PutAttribute(numberatt+i, tattr);
              delete newstack;
            } // end-if
          } // end-for
        } // end-if end processing for symmetric merging

        if (result.addr) {
          (gbli->No_RetTuples)++;

          // Phase change: last group tuple is returned =======================
          // Output buffer contains unprocessed tuples
          if ((gbli->No_RetTuples == gbli->No_GTuples)
              && (gbli->TB_Out->GetNoTuples() > 0)) 
          {
            // Test
            cout << "End of Phase: " << gbli->Phase << endl;
            cout << "# of group tuples: " << gbli->No_GTuples << endl;


            gbli->FirstREQUEST = true;   // need to aggregate on next REQUEST
            gbli->No_RetTuples = 0;      // init for next phase
            gbli->No_GTuples = 0;
            (gbli->Phase)++;

            // clear all vectors, the tuples are not touched
            for (i=0; i<CAPBUCKETS; i++) gbli->hBucket[i].clear();
            // the result tuples are now owned by the following operator
            gbli->Used_Memory = 0;
                       
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
        } else
          return CANCEL;
     } // end-if REQUEST processing

    case CLOSE: 
      // delete LocalInfo =====================================================
      if(local.addr != 0) {
        gbli = (GroupByLocalInfo2 *) local.addr;
        if(gbli->resultTupleType != 0) gbli->resultTupleType->DeleteIfAllowed();
        
        delete gbli->TB_In;
        delete gbli->TB_Out;
        delete gbli;
        local.setAddr(0);
      }
      qp->Close(args[0].addr);      // close input stream
      return 0;
  } // end message switch


  return(0);
} // Ende GroupByValueMapping2 ================================================


const string GroupBySpec2 = 
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



Operator groupby2 (
         "groupby2",             // name
         GroupBySpec2,           // specification
         GroupByValueMapping2,   // value mapping
         Operator::SimpleSelect, // trivial selection function
         GroupByTypeMapCap       // type mapping; 
);


/*

3 Class ~ExtRelationAlgebra~

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
  }

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
InitializeGroupbyAlgebra(     NestedList* nlRef,
                              QueryProcessor* qpRef,
                              AlgebraManager* amRef )
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return (new GroupbyAlgebra());
}



