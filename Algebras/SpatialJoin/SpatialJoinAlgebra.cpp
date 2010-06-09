/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[->] [$\rightarrow $]

{\Large \bf Anhang G: SpatialJoin-Algorithmus }

[1] Implementation of SpatialJoin-Algebra

The TypeMap is taken 1:1 from the PlugJoin-Algebra.

[TOC]

0 Overview

This algebra implements the Partition Based Spatial Merge-Join-Algorithm of Patel and
de Witt.

1 Defines and Includes

*/


using namespace std;

#include <string.h>
#include "SpatialJoinAlgebra.h"

#include <vector>
#include <list>
#include <set>
#include <queue>

#include "LogMsg.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "CPUTimeMeasurer.h"
#include "QueryProcessor.h"
#include "SecondoInterface.h"
#include "StopWatch.h"
#include "Counter.h"
#include "Progress.h"
#include "RTuple.h"

extern NestedList* nl;
extern QueryProcessor* qp;

#define BBox Rectangle

#ifndef DOUBLE_MAX
#define DOUBLE_MAX (1.7E308)
#endif

#define CHECK_COND(cond, msg) \
  if(!(cond)) \
  {\
    ErrorReporter::ReportError(msg);\
    return nl->SymbolAtom("typeerror");\
  };

/*
2 Auxiliary Functions

*/


/*
3 Operator ~spatjoin~

3.1 Overview

The operator ~spatjoin~ has following inputs and semantic:

  * First=left input stream: A stream of tuples which is collected in a
  temporary relation. The temporary relation corresponding to the left
  input stream is treated as the outer relation.

  * Second=right input stream: A stream of tuples which is collected in another
  temporary relation. The temporary relation corresponding to the right input
  stream is treated as the inner relation.

  * Outer Relation S: The outer relation S should have more tuples than the
  inner relation R. For the inner relation R no index is needed, R is
  sequential traversed. R must have at least one attribut of type ~rect~,
  ~rect3~, ~rect4~ or of ~spatialkind~. The name of this attribut is needed
  using the operator.

  * Inner Relation R: The inner relation of this operator should be the smaller
  one of the two input relations. R must have at least one attribut of type
  ~rect~, ~rect3~, ~rect4~ or of ~spatialkind~. The name of this attribut is
  needed using the operator.

  * Name of joining attribute of the inner Relation R: must be of type ~rect~,
  ~rect3~, ~rect4~ or of ~spatialkind~.

  * Name of joining attribute of the outer Relation S: must be of type ~rect~,
  ~rect3~, ~rect4~ or of ~spatialkind~.


3.2 Syntax and Signature

The operator ~spatjoin~ has the following syntax:

outerStream innerStream spatjoin [attrName\_outerRelation, attrName\_innerRelation]

The signature of ~spatjoin~ is:

----
( (stream (tuple ((x1 t1)...(xn tn))))
  (stream (tuple ((y1 yt1)...(yn ytn))))
  rect||rect3||rect4||spatialtype
  rect||rect3||rect4||spatialtype )
  -> (stream (tuple ((x1 t1)...(xn tn)((y1 yt1)...(yn ytn)))))
----

A query-example is: query Rel1 feed Rel2 feed spatjoin [attrNameRel1, attrNameRel2] consume;

*/

/*
3.3 Type mapping function of operator ~spatjoin~

*/

ListExpr spatjoinTypeMap(ListExpr args)
{
  char* errmsg = "Incorrect input for operator spatjoin.";
  string relDescStrS, relDescStrR;

  CHECK_COND(!nl->IsEmpty(args),
   "Operator spatjoin expects a list of length four.");
  CHECK_COND(!nl->IsAtom(args), errmsg);
  CHECK_COND(nl->ListLength(args) == 4,
   "Operator spatjoin expects a list of length four.");

  /* Split argument in four parts */
  ListExpr relDescriptionS = nl->First(args);      //outerRelation
  ListExpr relDescriptionR = nl->Second(args);     //innerRelation
  nl->WriteToString (relDescStrS, relDescriptionS); //for error message
  nl->WriteToString (relDescStrR, relDescriptionR); //for error message

  ListExpr attrNameS_LE = nl->Third(args);         //attrName of outerRel
  ListExpr attrNameR_LE = nl->Fourth(args);        //attrName of innerRelation

  /* handle stream part of outerRelation */

  CHECK_COND(nl->ListLength(relDescriptionS) == 2 &&
             !nl->IsEmpty(relDescriptionS) &&
             !nl->IsAtom(relDescriptionS)  ,
    "Operator spatjoin expects a first list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "but gets a first list with structure "+relDescStrS+".");

  ListExpr relSymbolS = nl->First(relDescriptionS);;
  ListExpr tupleDescriptionS = nl->Second(relDescriptionS);

  CHECK_COND(nl->IsAtom(relSymbolS) &&
             nl->AtomType(relSymbolS) == SymbolType &&
             nl->SymbolValue(relSymbolS) == "stream",
    "Operator spatjoin expects a first list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "but gets a first list with structure "+relDescStrS+".");

  ListExpr tupleSymbolS = nl->First(tupleDescriptionS);
  ListExpr attrListS = nl->Second(tupleDescriptionS);
  CHECK_COND(nl->IsAtom(tupleSymbolS) &&
             nl->AtomType(tupleSymbolS) == SymbolType &&
             nl->SymbolValue(tupleSymbolS) == "tuple" &&
             IsTupleDescription(attrListS) &&
             !nl->IsEmpty(tupleDescriptionS) &&
             !nl->IsAtom(tupleDescriptionS) &&
             nl->ListLength(tupleDescriptionS) == 2,
    "Operator spatjoin expects a first list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "but gets a first list with structure "+relDescStrS+".");

  /* handle attrName of outerRelation */
  int attrIndexS;
  ListExpr attrTypeS;
  string attrNameS = nl->SymbolValue(attrNameS_LE);
  CHECK_COND((attrIndexS = FindAttribute(attrListS, attrNameS, attrTypeS)) > 0,
    "Operator spatjoin expects an attributename "
    +attrNameS+" in first list\n"
    "but gets a first list with structure '"+relDescStrS+"'.");

  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

/*
CAUTION: Adjust the following check, if new types are introduced to join.

*/
  CHECK_COND(algMgr->CheckKind("SPATIAL2D", attrTypeS, errorInfo) ||
             nl->IsEqual(attrTypeS, "rect")||
             nl->IsEqual(attrTypeS, "rect3")||
             nl->IsEqual(attrTypeS, "rect4"),
    "Operator spatjoin expects that attribute "+attrNameS+"\n"
    "is of TYPE rect, rect3, rect4, point, points, line or region.");

  /* handle stream part of innerRelation */
  CHECK_COND(nl->ListLength(relDescriptionR) == 2 &&
             !nl->IsEmpty(relDescriptionR) &&
             !nl->IsAtom(relDescriptionR),
    "Operator spatjoin expects a second list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "but gets a second list with structure '"+relDescStrR+"'.");

  ListExpr relSymbolR = nl->First(relDescriptionR);;
  ListExpr tupleDescriptionR = nl->Second(relDescriptionR);

  CHECK_COND(nl->IsAtom(relSymbolR) &&
             nl->AtomType(relSymbolR) == SymbolType &&
             nl->SymbolValue(relSymbolR) == "stream",
    "Operator spatjoin expects a second list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "but gets a second list with structure '"+relDescStrR+"'.");

  CHECK_COND(!nl->IsEmpty(tupleDescriptionR), errmsg);
  CHECK_COND(!nl->IsAtom(tupleDescriptionR), errmsg);
  CHECK_COND(nl->ListLength(tupleDescriptionR) == 2, errmsg);

  ListExpr tupleSymbolR = nl->First(tupleDescriptionR);
  ListExpr attrListR = nl->Second(tupleDescriptionR);
  CHECK_COND(nl->IsAtom(tupleSymbolR) &&
             nl->AtomType(tupleSymbolR) == SymbolType &&
             nl->SymbolValue(tupleSymbolR) == "tuple" &&
             IsTupleDescription(attrListR) &&
             !nl->IsEmpty(tupleDescriptionR) &&
             !nl->IsAtom(tupleDescriptionR) &&
             nl->ListLength(tupleDescriptionR) == 2,
    "Operator spatjoin expects a second list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "but gets a second list with structure '"+relDescStrR+"'.");

  /* handle attrName of innerRelation */
  int attrIndexR;
  ListExpr attrTypeR;
  string attrNameR = nl->SymbolValue(attrNameR_LE);
  CHECK_COND((attrIndexR = FindAttribute(attrListR, attrNameR, attrTypeR)) > 0,
    "Operator spatjoin expects an attributename "
    +attrNameR+" in second list\n"
    "but gets a second list with structure '"+relDescStrR+"'.");

/*
CAUTION: Adjust the following check, if new types are introduced to join.

*/
  CHECK_COND(algMgr->CheckKind("SPATIAL2D", attrTypeR, errorInfo) ||
             nl->IsEqual(attrTypeR, "rect")||
             nl->IsEqual(attrTypeR, "rect3")||
             nl->IsEqual(attrTypeR, "rect4"),
    "Operator spatjoin expects that attribute "+attrNameR+"\n"
    "is of TYPE rect, rect3, rect4, point, points, line or region.");

/*
Check, if the joining attributes are of the same dimension. At this time the
following types are known in Secondo:

CAUTION: Further new types, providing a k-dimensional BoundingBox will cause
adjusting this typemapping-method at this point.

  * Two-dimenionale types are: ~rect~ and all types of ~SpatialKind~

  * Three-dimensional types are: ~rect3~

  * Four-dimensional types are: ~rect4~

*/
  string attrTypeR_str, attrTypeS_str;
  nl->WriteToString (attrTypeS_str, attrTypeS); //left stream
  nl->WriteToString (attrTypeR_str, attrTypeR); //right stream

  CHECK_COND( ( algMgr->CheckKind("SPATIAL2D", attrTypeR, errorInfo) &&
                algMgr->CheckKind("SPATIAL2D", attrTypeS, errorInfo) ) ||
              ( algMgr->CheckKind("SPATIAL2D", attrTypeR, errorInfo ) &&
                nl->IsEqual(attrTypeS, "rect") ) ||
              ( algMgr->CheckKind("SPATIAL2D", attrTypeS, errorInfo ) &&
                nl->IsEqual(attrTypeR, "rect") ) ||
              ( nl->IsEqual(attrTypeR, "rect") &&
                nl->IsEqual(attrTypeS, "rect") ) ||
              ( nl->IsEqual(attrTypeR, "rect3") &&
                nl->IsEqual(attrTypeS, "rect3") ) ||
              ( nl->IsEqual(attrTypeR, "rect4") &&
                nl->IsEqual(attrTypeS, "rect4") ),
    "Operator spatjoin expects joining attributes of same dimension.\n"
    "But gets "+attrTypeS_str+" as left type and "
    +attrTypeR_str+" as right type.\n");

  /* check if all new attribute names are ditinct */
  ListExpr attrOutlist = ConcatLists(attrListS, attrListR);

  if ( CompareNames(attrOutlist) )
  {
    /* Creating the resulting ListExpr */
    ListExpr resultList =
      nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->ThreeElemList(
            nl->IntAtom(attrIndexS),
            nl->IntAtom(attrIndexR),
            nl->StringAtom(nl->SymbolValue(attrTypeS))),
        nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                attrOutlist)));

    return resultList;

  }
  else
  {
    ErrorReporter::ReportError("Operator spatialloopjoin: Attribute names of "
      "result tuples are not distinct.\n");
    return nl->SymbolAtom("typeerror");
  }

}

/*
3.4 Selection function of operator ~spatjoin~

*/

int
spatjoinSelection (ListExpr args)
{
  /* find out type of key; similar to typemapping function */
  /* Split argument in four parts */
  ListExpr relDescriptionS = nl->First(args);      //outerRelation
  ListExpr attrNameS_LE = nl->Third(args);         //attrName of outerRel
  ListExpr tupleDescriptionS = nl->Second(relDescriptionS);
  ListExpr attrListS = nl->Second(tupleDescriptionS);

  /* handle attrName of outerRelation */
  int attrIndexS;
  ListExpr attrTypeS;
  string attrNameS = nl->SymbolValue(attrNameS_LE);
  attrIndexS = FindAttribute(attrListS, attrNameS, attrTypeS);

  /* selection function */
  ListExpr errorInfo = nl->OneElemList ( nl->SymbolAtom ("ERRORS"));
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  if ( (algMgr->CheckKind("SPATIAL2D", attrTypeS, errorInfo)) ||
       ( nl->SymbolValue (attrTypeS) == "rect") )
  return 0;  //two-dimensional objects to join
  else if ( nl->SymbolValue (attrTypeS) == "rect3")
       return 1; //three-dimensiona objects to join
       else if ( nl->SymbolValue (attrTypeS) == "rect4")
            return 2;  //four-dimensional objects to join
            else return -1; /* should not happen */
}

/*
3.5 ~SpatJoinLocalInfo~: Auxiliary Class for operator ~spatjoin~

*/

template<unsigned dim>
class SpatJoinLocalInfo : public ProgressLocalInfo
{
/*
3.5.1 Private declarations

*/
  private:

  struct PartitionHeader
  {

    TupleBuffer *RelationA; //the buffer corresponding to the right stream
    TupleBuffer *RelationB; //the buffer corresponding to the left stream

    int total_n; // nRelationA + nRelationB
    BBox<dim> PartitionBoundBox; //BoundBox of PartitionA + PartitionB
	bool fitInMemory; // both Partitions together fit in Memory
	bool isEmpty;  //  one of the both Partitions is empty 
    size_t PartitionMem; // the memory used by both Partitions
	
   };

  BBox<2> TotalBoundingBox; //BoundBox of all Objects
  size_t nPartitions; //nBuckets
  size_t maxMemory; // the maximal memory that can be used by both Partitions

  int attrIndexA;
  int attrIndexB;

  Word streamA;
  Word streamB;
  bool streamAClosed;
  bool streamBClosed;

  Tuple *tupleA;
  Tuple *tupleB;

  bool firstPass;
  bool memInfoShown;
  bool showMemInfo;

  typedef vector< PartitionHeader* > typePartitions;
  typedef typename vector< PartitionHeader* >::iterator typeIterPartitions;
  
  typePartitions Partitions;
  typeIterPartitions iterPartitions;
  
  GenericRelationIterator* iterRelationA;
  GenericRelationIterator* iterRelationB;

  Word wTupleB;

  TupleType *resultTupleType;

  // CompareTuples will by used by sort ()
  //  sort the tuples by the left x-Value of the BoundingBox
  bool CompareTuples(Tuple* a, Tuple* b)
  {

      BBox<dim> boxA = ((StandardSpatialAttribute<dim>*)a->
                         GetAttribute(attrIndexA))->BoundingBox();
	  BBox<dim> boxB = ((StandardSpatialAttribute<dim>*)b->
                         GetAttribute(attrIndexB))->BoundingBox();
	
	return (boxA.MinD (0) < boxB.MinD (0));
   }

  void ClearPartitions()
  {
    typeIterPartitions iterPart = Partitions.begin();

    while(iterPart != Partitions.end() )
    {
      (*iterPart).RelationA->Clear();
	  (*iterPart).RelationB->Clear();
      delete (*iterPart).RelationA;
	  delete (*iterPart).RelationB;
      iterPart++;
    }
	Partitions.clear();
  }

  void InsertTupleInPartition(bool insertInA, Tuple* insertTuple)
  {
    BBox<dim> box = ((StandardSpatialAttribute<dim>*)insertTuple->
                         GetAttribute(attrIndexA))->BoundingBox();  
    
	//insert in that Partition which intersects with the BoundingBox
	bool foundPartition = false;
	typeIterPartitions iterPart = Partitions.begin();
	while(iterPart != Partitions.end() )
    {
      if ((*iterPart).PartitionBoundBox.Intersects(box))
	  {
	    if (insertInA) 
		{(*iterPart).RelationA.insert(insertTuple);}
		else
		{(*iterPart).RelationB.insert(insertTuple);}
		(*iterPart).fitInMemory = 
		( maxMemory < (((*iterPart).RelationA->GetTotalSize())
		          + ((*iterPart).RelationB->GetTotalSize())) );
	  }
      iterPart++;
    }
	insertTuple->DeleteIfAllowed();
  }// InsertTupleInPartition
  
  bool FillPartitions()
  {
    // firstPass evtl nicht noetig
      Word wTupleA, wTupleB;
      qp->Request(streamA.addr, wTupleA);
      qp->Request(streamB.addr, wTupleB);
      if(!(qp->Received(streamA.addr)) && !(qp->Received(streamB.addr)))
	  {
	    cout << "one stream is empty" << endl;
	    return false;
	  }
	  else
      {
        // max Memory
        maxMemory = qp->MemoryAvailableForOperator();

	    if (showMemInfo) {
          cmsg.info()
          << "MAX_MEMORY ("
          << qp->MemoryAvailableForOperator()/1024
          << " kb " << endl;
          cmsg.send();
	    }
      }
    // read the first 40 tuples to estimate the TotalBoundingBox, Total_n, 
    // Total_Memory and to calculate the Partitions_N
	int NTuplesForEstimate = 40;
	bool InsertMoreTuples = true;
	TupleBuffer* tempBufferA = new TupleBuffer();
	TupleBuffer* tempBufferB = new TupleBuffer();
	// before the TotalBoundingBox can 
	//be calculated, the first Tuples have to bee stored 
	//temporyrarily in a Buffer
	BBox<dim> boxA;
	BBox<dim> boxB;
	BBox<dim> tempTBB; //temporal TotalBoundBox

    while(InsertMoreTuples)
    {
	  Tuple *ta = (Tuple*)wTupleA.addr;
	  Tuple *tb = (Tuple*)wTupleB.addr;
	  
      tempBufferA->AppendTuple ( ta );
      ta->DeleteIfAllowed();

	  tempBufferB->AppendTuple ( tb );
      tb->DeleteIfAllowed();

      qp->Request(streamA.addr, wTupleA);
      qp->Request(streamB.addr, wTupleB);
	  
	  boxA = ((StandardSpatialAttribute<dim>*)ta->
                         GetAttribute(attrIndexA))->BoundingBox();
	  boxB = ((StandardSpatialAttribute<dim>*)tb->
                         GetAttribute(attrIndexB))->BoundingBox();
						 

	  if ( tempBufferA->GetNoTuples() == 1 )
	  { 
	    tempTBB = boxA.Union(boxB);
	  }
	  else
	  {
	    tempTBB = tempTBB.Union(boxA);
	    tempTBB = tempTBB.Union(boxB);
	  }
	  InsertMoreTuples =
	   ((qp->Received(streamA.addr)) && (qp->Received(streamB.addr))
	         && ((tempBufferA->GetNoTuples()
	           +  tempBufferB->GetNoTuples())
	                   < NTuplesForEstimate));
		// both tempBuffers have the same NoTuples and max 20
    }
	
	// Estimation of NoStreamA + NoStreamB; MemStreamA + MemStreamB;
	
	nPartitions = 3;  //Default if NoStreams can not be estimated 
	                  //is 3 * 3 = 9 Partitions
	Partitions.resize((nPartitions * nPartitions));
	
	double *Min, *Max;
	
	Min[0] = tempTBB.MinD (0);
	Min[1] = tempTBB.MinD (1);
	Max[0] = tempTBB.MaxD (0);
	Max[1] = tempTBB.MaxD (1);
	
	TotalBoundingBox = new Rectangle<2>(true, min, max);
	
	delete tempTBB;
	//initialize the Partitions with the BoundingBoxes 
	//as part of the TotalBoundingBox.
	//The boundingBoxes of the partitons at the border 
	//get +-DOUBLE_MAX as frontier.
	for(int i = 1; i <= nPartitions; i++)
    {
	  for(int j = 1; i <= nPartitions; j++)
      {
	     double *min, *max;
		 min[0]= TotalBoundingBox.MinD (0) + 
		       ( (TotalBoundingBox.MaxD (0) - TotalBoundingBox.MinD (0))
				          / nPartitions * (i-1) );
		 max[0]= TotalBoundingBox.MinD (0) + 
		       ( (TotalBoundingBox.MaxD (0) - TotalBoundingBox.MinD (0))
			                   / nPartitions * i);
		 min[1]= TotalBoundingBox.MinD (1) + 
		       ( (TotalBoundingBox.MaxD (1) - TotalBoundingBox.MinD (1))
			               / nPartitions * (j-1));
		 max[1]= TotalBoundingBox.MinD (1) + 
		       ( (TotalBoundingBox.MaxD (1) - TotalBoundingBox.MinD (1))
			                   / nPartitions * j);

		 if (i == 1) min[1] = - DOUBLE_MAX;
		 if (i == nPartitions ) max[1] = DOUBLE_MAX;
		 if (j == 1) min[0] = - DOUBLE_MAX;
		 if (j == nPartitions ) max[0] = DOUBLE_MAX;
		 
		 Partitions[((i*j)-1)].PartitionBoundBox 
		                 = new Rectangle<2>(true, min, max);
		 Partitions[((i*j)-1)].RelationA = new *TupleBuffer;
		 Partitions[((i*j)-1)].RelationB = new *TupleBuffer;
         Partitions[((i*j)-1)].fitInMemory = true;
         Partitions[((i*j)-1)].isEmpty = true;

	  }
	}
	
	// Insert the buffered Tuples in the Partitions
    GenericRelationIterator* iterTempBufferA;
    GenericRelationIterator* iterTempBufferB;
	
	iterTempBufferA = tempBufferA->MakeScan();
	iterTempBufferB = tempBufferB->MakeScan();
	
	 Tuple *ta = iterTempBufferA->GetNextTuple();
	 Tuple *tb = iterTempBufferB->GetNextTuple();
	 
    
	while(!(ta == 0) && !(tb == 0))
      {
      InsertTupleInPartition (true, ta); // true for insert in PartitionA
	  InsertTupleInPartition (false, tb);//false for insert in PartitionB

	  *ta = iterTempBufferA->GetNextTuple();
	  *tb = iterTempBufferB->GetNextTuple();
      }
	
	// and now insert the rest of the StreamA and StreamB
    qp->Request(streamA.addr, wTupleA);
    qp->Request(streamB.addr, wTupleB);
	bool ReceivedA = qp->Received(streamA.addr);
	bool ReceivedB = qp->Received(streamB.addr);
	
	while(ReceivedA || ReceivedB)
      {
      Tuple *ta = (Tuple*)wTupleA.addr;
	  Tuple *tb = (Tuple*)wTupleB.addr;

	  
	  if (ReceivedA)
	    { InsertTupleInPartition (true, ta);
		}
	  if (ReceivedB)
	    { InsertTupleInPartition (false, tb);
		}

	  ta->DeleteIfAllowed();
      tb->DeleteIfAllowed();

      qp->Request(streamA.addr, wTupleA);
      qp->Request(streamB.addr, wTupleB);
	  ReceivedA = qp->Received(streamA.addr);
	  ReceivedB = qp->Received(streamB.addr);
      }
	
    qp->Close(streamA.addr);
    qp->Close(streamB.addr);
	streamAClosed = true;
	streamBClosed = true;
	 
    return true;

  }//End FillPartitions

/*
3.5.2 Public declarations

*/
  public:
    bool PartitionFilled;
    SpatJoinLocalInfo(Word StreamAWord,
                         Word attrIndexAWord,
                         Word StreamBWord,
                         Word attrIndexBWord,
						 Supplier s, 
						 ProgressLocalInfo* p);


    Tuple* NextResultTuple ();
/*
Returns the next result tuple

*/
	SpatJoinLocalInfo *ptr;

/*
The tuple type of result tuples.

*/

    ~SpatJoinLocalInfo()
/*
The destructor

*/
    {
#ifdef PART_JOIN_VERBOSE_MODE
      cout << "__________________________________________________" << endl;
      cout << endl << "Partitions built in query : "  << endl;
      cout << "__________________________________________________" << endl;
#endif

    ClearPartitions();
	
    // close open streams if necessary
    if ( !streamAClosed )
      qp->Close(streamA.addr);
    if ( !streamBClosed )
      qp->Close(streamB.addr);

    resultTupleType->DeleteIfAllowed();


    };

};

/*
3.5.3 The constructor

*/
template <unsigned dim>
SpatJoinLocalInfo<dim>::SpatJoinLocalInfo
                        (Word StreamAWord, Word attrIndexAWord,
                         Word StreamBWord, Word attrIndexBWord, 
					     Supplier s, ProgressLocalInfo* p)
{
  
/*
Do some initializing work.

*/
    memInfoShown = false;
    showMemInfo = RTFlag::isActive("ERA:ShowMemInfo");
    this->streamA = StreamAWord;
    this->streamB = StreamBWord;

    ListExpr resultType =
      SecondoSystem::GetCatalog()->NumericType( qp->GetType( s ) );
    resultTupleType = new TupleType( nl->Second( resultType ) );

	
    //the attribute-numbers of the joining attributes
    attrIndexA = (((CcInt*)attrIndexAWord.addr)->GetIntval()) - 1;
    attrIndexB = (((CcInt*)attrIndexBWord.addr)->GetIntval()) - 1;


    streamAClosed = false;
    streamBClosed = false;
    bool PartitionFilled = FillPartitions();

/*
If one of the both input relations is empty, no further work is to be done.
~NextResultTuple~ proofs with PartitionFilled, if one Stream was empty.

*/

};

/*
3.5.3 Method NextResultTuple

*/
template <unsigned dim>
Tuple* SpatJoinLocalInfo<dim>::NextResultTuple ()
{


  bool nextResultTupleFound = false;
  
  iterPartitions = Partitions.begin();
  
  vector< Tuple* > bucketA;
  vector< Tuple* >::iterator iterBucketA;
  
  vector< Tuple* > bucketB;
  vector< Tuple* >::iterator iterBucketB;
  if (!PartitionFilled)
  {
    return 0;
  }
  else
  {
    while(iterPartions != Partitions.end() )
    {
	  if ( !((*iterPartitions).isempty) )
	  {
	   
		iterRelationA = (*iterPartitions).RelationA->MakeScan();
	    iterRelationB = (*iterPartitions).RelationB->MakeScan();
	
	    Tuple *ta = iterRelationA->GetNextTuple();
	    Tuple *tb = iterRelationB->GetNextTuple(); 
		
		//bucketA.resize(iterRelationA->GetNoTuples());
		//bucketB.resize(iterRelationB->GetNoTuples());
		bucketA.clear();
		bucketB.clear();

		
		// read TupleBuffer in vector
	    while((ta != 0) || (tb != 0))
        {
	      if ( ta != 0 )
		  {
		    bucketA.insert( ta );
		  }
		  if ( tb != 0 )
		  {
		    bucketA.insert( tb);
		  }

	    *ta = iterTempBufferA->GetNextTuple();
	    *tb = iterTempBufferB->GetNextTuple();
        }

		// sort vector
		sort (bucketA.begin(), bucketA.end(), compareTuples);
		sort (bucketB.begin(), bucketB.end(), compareTuples);
		
		//sequentieller Durchlauf und nach
		//überlappenden Rechtecken suchen
		iterBucketA = bucketA.begin();
		iterBucketB = bucketB.begin();
		
		BBox<dim> BoundBoxA = (*iterBucketA)->BoundingBox();
		BBox<dim> BoundBoxB = (*iterBucketB)->BoundingBox();



		bool stepBucketA = ( BoundBoxA.MinD (0) < BoundBoxB.MinD (0) );
		if ( (iterBucketA == bucketA.end())
		      && (iterBucketB == bucketB.end()) )
		{
		  if (BoundBoxA.Intersects(BoundBoxB))
		  {
		  	Tuple* resultTuple = new Tuple( resultTupleType );
			Concat((*iterBucketA), (*iterBucketB), resultTuple);

		    return resultTuple;
		  }
		}
		while ( (iterBucketA != bucketA.end())
		         || (iterBucketB!= bucketB.end()) )
		{
		  if (stepBucketA)
		  {
		    if ( (BoundBoxA.MaxD (0) >= BoundBoxB.MinD (0)) 
			             && BoundBoxA.Intersects(BoundBoxB) )
			{ 
		  	  Tuple* resultTuple = new Tuple( resultTupleType );
			  Concat((*iterBucketA), (*iterBucketB), resultTuple);

		      return resultTuple;
		    }
	        iterBucketA++;
		    BoundBoxA = (*iterBucketA)->BoundingBox();
		    stepBucketA = ( BoundBoxA.MinD (0) < BoundBoxB.MinD (0) );
	      }
     
		  if (!stepBucketA)
		  {
		    if (( BoundBoxB.MaxD (0) >= BoundBoxA.MinD (0) ) 
			            && BoundBoxA.Intersects(BoundBoxB) )
            {
		  	  Tuple* resultTuple = new Tuple( resultTupleType );
			  Concat((*iterBucketA), (*iterBucketB), resultTuple);
		      return resultTuple;
		    }
			iterBucketB++;
			BoundBoxB = (*iterBucketB)->BoundingBox();
			stepBucketA =(BoundBoxA.MinD(0) < BoundBoxB.MinD(0));
		    
	      }
		  
		}
      }
	  iterPartions++;
	}
    return 0;
  }
};



/*
3.5 Value mapping function of operator ~spatjoin~

*/
double minimum(double a, double b) {return (a < b ? a : b);}
template<int D>
int
spatjoinValueMapping(Word* args, Word& result, int message, 
                         Word& local, Supplier s)
{
  SpatJoinLocalInfo<D> *li;
  li = static_cast<SpatJoinLocalInfo<D>*>( local.addr );
  SpatJoinLocalInfo<D> *partLocalInfo;

  switch (message)
  {
    case OPEN:
    {
      if ( li ) delete li;

      li = new SpatJoinLocalInfo<D>();
      li->memorySecond = 12582912;	 //default, reset by constructor below
      local.addr = li;

      li->ptr = 0;

      qp->Open(args[0].addr);
      qp->Open(args[1].addr);

      return 0;
    }

    case REQUEST:
    {
      
	   if ( li->ptr == 0 )	//first request;
				//constructor moved here to avoid delays in OPEN
				//which are a problem for progress estimation
      {
      Word StreamAWord,
           StreamBWord,
           attrIndexAWord,
           attrIndexBWord;

      StreamAWord = args[0];
      StreamBWord = args[1];
      attrIndexAWord = args[4];  //APPENDED - Value no 1
      attrIndexBWord = args[5]; //APPENDED - Value no 2
	  
	  li->ptr = new SpatJoinLocalInfo<D>(StreamBWord, 
                                         attrIndexBWord,
                                         StreamAWord, 
                                         attrIndexAWord,
									s, li);
      }

      partLocalInfo = li->ptr;
      result = SetWord( partLocalInfo->NextResultTuple() );
      li->returned++;

      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
    {

      delete li->ptr;
      return 0;
    }
	
    case CLOSEPROGRESS:
	{
      qp->CloseProgress(args[0].addr);
      qp->CloseProgress(args[1].addr);

      if ( li ) delete li;

      return 0;
	}
	
	case REQUESTPROGRESS:
    {
      bool trace = false;

      ProgressInfo p1, p2;
      ProgressInfo* pRes = static_cast<ProgressInfo*>( result.addr );
      const double uSpatJoin = 0.0023;  //millisecs per probe tuple
      const double wSpatJoin = 0.025;  //millisecs per tuple returned

      if( !li ) return CANCEL;
      else
      {
       if (qp->RequestProgress(args[0].addr, &p1)
         && qp->RequestProgress(args[1].addr, &p2))
        {
	    li->SetJoinSizes(p1, p2);

	    double defaultSelectivity = ( 10 / (p1.Card + p2.Card));

        if ( li->returned > 50 ) // stable state  
          {
            pRes->Card = p1.Card * p2.Card *
              ((double) li->returned 
			  / (double) (li->readFirst + li->readSecond));
          }
          else
          {
            pRes->Card = p1.Card * p2.Card * 
              (qp->GetSelectivity(s) == 0.1 ? defaultSelectivity : 
	        qp->GetSelectivity(s));
          }

	    pRes->CopySizes(li);

        pRes->Time = p1.Time + p2.Time
        + pRes->Card * wSpatJoin;	//output tuples

	    pRes->Progress = (p1.Progress * p1.Time + p2.Progress * p2.Time
	    + li->readFirst * uSpatJoin
	    + li->readSecond * uSpatJoin
	    + minimum(li->returned, pRes->Card) * wSpatJoin)
            / pRes->Time;
/*
	    pRes->BTime = 
	    p1.BTime 
	    + p2.Time * (firstBuffer / p2.Card)
	    + firstBuffer * vHashJoin;

	    pRes->BProgress = 
	    (p1.BProgress * p1.BTime 
            + p2.Time * minimum((double) li->readSecond, firstBuffer) / p2.Card 
	    + minimum((double) li->readSecond, firstBuffer) * vHashJoin)
            / pRes->BTime;
			
*/

	    if ( trace ) 
		  {
		  cout << "li->readFirst = " << li->readFirst << endl;
		  cout << "li->readSecond = " << li->readSecond << endl;
		  cout << "li->returned = " << li->returned << endl;
		  cout << "li->state = " << li->state << endl;
	      }

        return YIELD;
        }
        else
        {
        return CANCEL;
        }
      }
    }//requestprogress
  }
  

  return 0;

};//spatjoinValueMapping

/*
3.7 Definition of value mapping vectors

*/
ValueMapping spatjoinMap [] = {spatjoinValueMapping<2>,
                               spatjoinValueMapping<3>,
                               spatjoinValueMapping<4> };

/*
3.8 Specification of operator ~spatjoin~

*/
const string spatjoinSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\""
      " \"Example\" )"
      "( <text>( (stream (tuple ((x1 t1)...(xn tn)))) "
      "(stream (tuple ((y1 yt1)...(yn ytn))))"
      " rect||rect3||rect4||SpatialType rect||rect3||rect4||SpatialType) -> "
      "(stream (tuple ((x1 t1)...(xn tn)((y1 yt1)...(yn ytn)))))</text--->"
      "<text>outerStream innerStream spatjoin "
      "[outerAttr, innerAttr]</text--->"
      "<text>Uses the Plug&Join-Algorithm to find all pairs of intersecting"
      " tuples in the given relations. The joining tuples are reported."
      "</text--->"
      "<text>query trees feed streets feed spatjoin [pos_trees, pos_streets]"
      " consume; the joining attributes must be of the same dimension."
      "</text--->"
      ") )";

/*
3.9 Definition of operator ~spatjoin~

*/

Operator spatjoin (
         "spatjoin",             // name
         spatjoinSpec,           // specification
         3,                         // number of overloaded functions
         spatjoinMap,            // value mapping
         spatjoinSelection,      // trivial selection function
         spatjoinTypeMap         // type mapping
);

/*
4 Definition and initialization of ~PartJoin~ algebra

*/
class SpatialJoinAlgebra : public Algebra
{
 public:
  SpatialJoinAlgebra() : Algebra()
  {
    AddOperator(&spatjoin);
  }
  ~SpatialJoinAlgebra() {};
};

SpatialJoinAlgebra spatialjoinalgebra;


extern "C"
Algebra*
InitializeSpatialJoinAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&spatialjoinalgebra);
}

