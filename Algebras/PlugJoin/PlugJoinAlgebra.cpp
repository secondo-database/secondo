/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[->] [$\rightarrow $]

{\Large \bf Anhang G: Plug\&Join-Algorithmus }

[1] Implementation of PlugJoin-Algebra

October 2004, Herbert Schoenhammer

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006 Victor Almeida replaced the ~free~ tuples concept to
reference counters. There are reference counters on tuples and also
on attributes.

[TOC]

0 Overview

This algebra implements the Plug\&Join-Algorithm of Bercken, Schneider and
Seeger: ~Plug\&Join: An Easy-To-Use Generic Algorithm for Efficiently
Processing Equi and Non-equi Joins~. The algebra uses a main memory
representation of a R-Tree implemented in the PlugJoin.h file.

This file especially implements the managing functions (type mapping,
value mapping, ...) to correspond with the system frame of Secondo.

The value mapping function implements the Plug\&Join-Algorithm managing
the recursive algorithm of Plug\&Join in such a way, that requests of
following stream-operators may get the result-tuples one by one.

This algebra provides the operator ~spatialjoin~ which joins two input-
streams by a spatial attribute. The result is a output-stream containing
all tuples of the input-streams intersecting each other.

The main problem using input streams is, that the tuples in streams
do not know, from which relation they come from first. So it is necessary
to collect the tuples in a temporary relation (TupleBuffer) and give them
a new identificator (ID). The Plug\&Join-Algorithm uses this temporary
relations.

1 Defines and Includes

*/


using namespace std;

#include <string.h>
#include "PlugJoinAlgebra.h"

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
If ~PLUGJOIN\_VERBOSE\_MODE~ is set, some interesting things are reported.
This is statistical info for the initial R-Tree or the number of recursive
instances of the algorithm and the number of R-Trees built in the Plug\&Join-
Algorithm.

*/
//#define PLUGJOIN_VERBOSE_MODE

/*
If ~PLUGJOIN\_VERY\_VERBOSE\_MODE~ is set, further Information about the number
or R-Trees built in every recursive instance, the number of overflowed leaves,
the number of not inserted data entries and the number of
the number of not answerable window queries is reported for each recursive
instance.

Could be of interest if you want to study partitioning the data through the
R-Trees.

*/
//#define PLUGJOIN_VERY_VERBOSE_MODE

/*
2 Auxiliary Functions

*/

int myComparePnJ( const void* a, const void* b )
{
 if( ((SortedArrayItem *) a)->pri < ((SortedArrayItem *) b)->pri )
   return -1;
 else if( ((SortedArrayItem *) a)->pri > ((SortedArrayItem *) b)->pri )
   return 1;
 else
   return 0;
}

/*
3 Operator ~spatialjoin~

3.1 Overview

The operator ~spatialjoin~ has following inputs and semantic:

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

The operator ~spatialjoin~ has the following syntax:

outerStream innerStream spatialjoin [attrName\_outerRelation, attrName\_innerRelation]

The signature of ~spatialjoin~ is:

----
( (stream (tuple ((x1 t1)...(xn tn))))
  (stream (tuple ((y1 yt1)...(yn ytn))))
  rect||rect3||rect4||spatialtype
  rect||rect3||rect4||spatialtype )
  -> (stream (tuple ((x1 t1)...(xn tn)((y1 yt1)...(yn ytn)))))
----

A query-example is: query Rel1 feed Rel2 feed spatialjoin [attrNameRel1, attrNameRel2] consume;

*/

/*
3.3 Type mapping function of operator ~spatialjoin~

*/

ListExpr spatialjoinTypeMap(ListExpr args)
{
  char* errmsg = "Incorrect input for operator spatialjoin.";
  string relDescStrS, relDescStrR;

  CHECK_COND(!nl->IsEmpty(args),
   "Operator spatialjoin expects a list of length four.");
  CHECK_COND(!nl->IsAtom(args), errmsg);
  CHECK_COND(nl->ListLength(args) == 4,
   "Operator spatialjoin expects a list of length four.");

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
    "Operator spatialjoin expects a first list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "but gets a first list with structure "+relDescStrS+".");

  ListExpr relSymbolS = nl->First(relDescriptionS);;
  ListExpr tupleDescriptionS = nl->Second(relDescriptionS);

  CHECK_COND(nl->IsAtom(relSymbolS) &&
             nl->AtomType(relSymbolS) == SymbolType &&
             nl->SymbolValue(relSymbolS) == "stream",
    "Operator spatialjoin expects a first list with structure "
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
    "Operator spatialjoin expects a first list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "but gets a first list with structure "+relDescStrS+".");

  /* handle attrName of outerRelation */
  int attrIndexS;
  ListExpr attrTypeS;
  string attrNameS = nl->SymbolValue(attrNameS_LE);
  CHECK_COND((attrIndexS = FindAttribute(attrListS, attrNameS, attrTypeS)) > 0,
    "Operator spatialjoin expects an attributename "
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
    "Operator spatialjoin expects that attribute "+attrNameS+"\n"
    "is of TYPE rect, rect3, rect4, point, points, line or region.");

  /* handle stream part of innerRelation */
  CHECK_COND(nl->ListLength(relDescriptionR) == 2 &&
             !nl->IsEmpty(relDescriptionR) &&
             !nl->IsAtom(relDescriptionR),
    "Operator spatialjoin expects a second list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "but gets a second list with structure '"+relDescStrR+"'.");

  ListExpr relSymbolR = nl->First(relDescriptionR);;
  ListExpr tupleDescriptionR = nl->Second(relDescriptionR);

  CHECK_COND(nl->IsAtom(relSymbolR) &&
             nl->AtomType(relSymbolR) == SymbolType &&
             nl->SymbolValue(relSymbolR) == "stream",
    "Operator spatialjoin expects a second list with structure "
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
    "Operator spatialjoin expects a second list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "but gets a second list with structure '"+relDescStrR+"'.");

  /* handle attrName of innerRelation */
  int attrIndexR;
  ListExpr attrTypeR;
  string attrNameR = nl->SymbolValue(attrNameR_LE);
  CHECK_COND((attrIndexR = FindAttribute(attrListR, attrNameR, attrTypeR)) > 0,
    "Operator spatialjoin expects an attributename "
    +attrNameR+" in second list\n"
    "but gets a second list with structure '"+relDescStrR+"'.");

/*
CAUTION: Adjust the following check, if new types are introduced to join.

*/
  CHECK_COND(algMgr->CheckKind("SPATIAL2D", attrTypeR, errorInfo) ||
             nl->IsEqual(attrTypeR, "rect")||
             nl->IsEqual(attrTypeR, "rect3")||
             nl->IsEqual(attrTypeR, "rect4"),
    "Operator spatialjoin expects that attribute "+attrNameR+"\n"
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
    "Operator spatialjoin expects joining attributes of same dimension.\n"
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
3.4 Selection function of operator ~spatialjoin~

*/

int
spatialjoinSelection (ListExpr args)
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
3.5 ~SpatialJoinLocalInfo~: Auxiliary Class for operator ~spatialjoin~

*/

template<unsigned dim>
class SpatialJoinLocalInfo
{
/*
3.5.1 Private declarations

*/
  private:


  typedef multimap< ArrayIndex,R_TreeEntryPnJ<dim> > Partition;

/*
   Type to store the partitions:

  1 Multimap for creating the R.Partitions. Pairs (nodeNumber in
    which the insertion would be done, the entry which should be inserted)
    are gathered. This multimap is only a buffer: if pagesize is reached,
    the content of the multimap is stored to a SMI-File.

  2 Multimap for creating the S.Partitions. Pairs (nodeNumber of
    overflowed leave, the searchWindow) are gathered. This multimap is only
    a buffer: if pagesize is reached, the content of the multimap is stored
    to a SMI-File.

*/

  struct NodeInfo
  {
     unsigned int counter;      //Number of overflows/queries
     SmiRecordId firstRId;      //First Record of overflows/queries
     SmiRecordId actualRId;     //Record to which next write will be done

  NodeInfo() :
    counter ( 0 ),
    firstRId ( 0 ),
    actualRId ( 0 )
    {}

  NodeInfo(unsigned int counter, SmiRecordId firstRId, SmiRecordId actualRId):
    counter ( counter ), firstRId (firstRId), actualRId (actualRId)
    {}

  };

  typedef map< ArrayIndex, NodeInfo > PartitionInfo;
  typedef typename PartitionInfo::iterator PartInfoIter;

/*
   Type to store some information about the partitions:

  1 Map for counting the number of not inserted entries. Necessary for
    flushing overflowed nodes to SMI-File. The RecordId of th first record
    is stored (and a temporary RecordId only used for building the chained
    list).

  2 Map for counting the number of not answerable queries (because of
    node overflow).The RecordId of th first record
    is stored (and a temporary RecordId only used for building the chained
    list).

*/

  struct PartitionsInfo
  {
    PartitionInfo* innerInfo;
    PartitionInfo* outerInfo;
  };

/*
Information about partitions of one recursive instance of the Plug\&Join-
Algorithm.

  1 Map for counting the number of not inserted entries. Necessary for
    flushing overflowed nodes to SMI-File.

  2 Map for counting the number of not answerable queries (because of
    node overflow).

*/

  struct Header
  {
    R_TreePnJ<dim>* rtree;  //pointer to the R-Tree

    ArrayIndex outerAttrIndex;  //outerRelation: # of joining attribute

    TupleBuffer *innerRelation; //the buffer corresponding to the right stream
    TupleBuffer *outerRelation; //the buffer corresponding to the left stream

    int recordHeaderSize; //size of record Header (# of partition, # of entries
                          //in record, nextRecordId);
    int maxEntriesInRecord; //# of entries which can be written in one record

    SmiRecordFile file;     //SMI-file to store partitions of overflowed leaves

    PartitionInfo* firstPartitionInfo;  //PartitionInfo created by constructor

    bool firstRTree;                //controls for NextResultTuple
    bool newPartitionsFromStack;
    bool nextQueryInPartition;
    bool nextTupleOuterRelation;
    bool firstSearchForTuple;
    bool searchForTuple;
    bool lastPartition;
    bool getEntryFromFirstRecord;   //read first record of chained list
    bool GetNextEntry_readFirst;    //controls for GetNextEntry
    bool GetNextEntry_read;

    TupleBufferIterator *outerIter; //iterates OuterRelation in NextResultTuple
    SmiRecordId outerActualTupleId; //RecordId of actual Tuple in
                                    //NextResultTuple
    R_TreeEntryPnJ<dim> outerEntry;
    R_TreeEntryPnJ<dim> foundEntry; 
      //Next Entry found by First/Next of the R-Tree

    Partition* outerRelPart;        //gathering overflows in one partition
    PartitionInfo* outerRelInfo;    //inserts of outer relation

    PartitionsInfo* actualMaps;     //all Partitions in one recursive level
    PartInfoIter actualMapsIter;
    ArrayIndex actualNodeNo;

    Partition* innerRelPart;        //gathering overflows in one partition
    PartitionInfo* innerRelInfo;    //queries of inner relation

    Header ():
       rtree ( 0 ),
       outerAttrIndex ( 0 ),
       innerRelation ( ),
       outerRelation ( ),
       recordHeaderSize ( 0 ),
       maxEntriesInRecord ( 0 ),
       file ( true, page_size),
       firstPartitionInfo ( 0 ),
       firstRTree ( true ),
       newPartitionsFromStack ( true ),
       nextQueryInPartition ( true ),
       nextTupleOuterRelation ( true ),
       firstSearchForTuple ( true ),
       searchForTuple ( true ),
       lastPartition ( false ),
       getEntryFromFirstRecord (true ),
       GetNextEntry_readFirst ( true ),
       GetNextEntry_read ( true ),
       outerIter ( 0 ),
       outerActualTupleId ( 0 ),
       outerEntry(),
       outerRelPart ( 0 ),
       outerRelInfo ( 0 ),
       actualMaps ( 0 ),
       actualMapsIter ( ),
       actualNodeNo ( 0 ),
       innerRelPart ( 0 ),
       innerRelInfo ( 0 )

       {}
   } hdr;

/*
The header stores a lot of information needed to give information from
constructor ~SpatialJoinLocalInfo~ to ~NextResultTuple~. Further all
variables and data for several calls of ~NextResultTuple~ (initialized by
stream-REQUEST) are stored in the header.

*/

    typedef stack <PartitionsInfo> AllPartitions;

    AllPartitions Parts;
/*
The stack to gather all ParttionInfo in recursive order.

*/

#ifdef PLUGJOIN_VERBOSE_MODE
    int rtreeCounter;
/*
Counts the number of built R-Trees.

*/
#endif

    TupleBuffer* StreamBuffer (const Word stream);
/*
Creates a TupleBuffer and receives the tuples of the ~stream~ one by one.
They are appended to the buffer. If the stream is empty '0' is returned.

*/

    int UseNodesInTree (const int noTuplesR, const int noTuplesS);
/*
Computes the nodes to use in the R-Tree of the next recursive instance of the
Plug\&Join-Algorithm. The formula is suggested by Bercken et al..

*/


    void BufferInsert (multimap< ArrayIndex,R_TreeEntryPnJ<dim> >* mm,
                   map< ArrayIndex,NodeInfo >* m,
                   ArrayIndex& nodeNo, R_TreeEntryPnJ<dim>& entry);
/*
Inserts ~entry~ in partition and partitionInfo with key value ~nodeNo~:

  1 Inserting the entry into innerRelPart ~mm~  and increment counter ~m~ for
    overflowed leave.

  2 Inserting the entry into outerRelPart ~mm~  and increment counter ~m~ for
   not answerable query.

*/

    void FlushAllLeavesToBuffer(Partition* innerRelPart,
                                PartitionInfo* innerRelInfo);
/*
Reads all Entries of overflowed Leaves and stores it in the innerRelPart
(R.Part).

*/

    void FlushBufferToFile (Partition* mm,
                            PartitionInfo* m);
/*
For every nodeNo in ~m~ all entries in ~mm~ are stored to file.

*/
    void FlushLeavesOverflowed(vector <ArrayIndex>& leavesOverflowed,
                               R_TreeEntryPnJ<dim>& entry,
                               Partition* outerRelPart,
                               PartitionInfo* outerRelCounter);
/*
Flush auxiliary vector with overflowed leaves (of not answerable queries)
and build S.Partition

*/

    void Write (Partition* mm,
                ArrayIndex& nodeNo,
                SmiRecordId& recNo,
                SmiRecordId& nextRecordId);
/*
Writes all entries for node ~nodeNo~ in multimap ~mm~ into record ~recNo~.

*/

    void ReadFirstEntries (PartitionInfo* m, ArrayIndex& actualNodeNo,
                           SmiRecordId& nextRecordId,
                           vector < R_TreeEntryPnJ<dim> >& entriesOfRecord );

    void ReadNextEntries ( SmiRecordId& NextRecordId,
                           vector < R_TreeEntryPnJ<dim> >&entriesOfRecord );

    R_TreeEntryPnJ<dim>* GetNextEntry(PartitionInfo* m,
                                      ArrayIndex& actualNodeNo);

/*
~ReadFirstEntries~ searches in ~m~ the entry with m[->]first == actualNodeNo.
Reads the record with m[->]second.firstRId == recordId. Interprets the record
and returns the entries in the vector ~entriesOfRecord~. The recordid of the next
record is returned in ~nextRecordId~. If there's no further next record, the
returned ~nextRecord~ is 0. (Note: The first recordid generated from the SMI is 1).

~ReadNextEntries~ reads the ~NextRecord~.

~GetNextEntry~ uses ~ReadFirstEntries~ and ~ReadNextEntries~ to get the first/
next entry from SMI-File.

~hdr.GetNextEntry\_readFirst~ and ~hdr.GetNextEntry\_read~ are needed for controlling
~GetNextEntry~ over a sequence of requests.

*/

    Tuple* NextResultTupleFromStack();
/*
Mostly the same than the ~constructor~ and ~NextResultTuple~, but
generating R-Tree and queries from partitions generated by ~constructor~
and ~NextResultTuple~.

*/

    Tuple* newResultTupleFromEntries (R_TreeEntryPnJ<dim>& outerEntry,
                                      R_TreeEntryPnJ<dim>& foundEntry);
/*
Generates a new Tuple (that must be deleted somewhere), evaluating the
pointers of the entries and concatenating the tuples from inner and
outer relation.

*/

/*
3.5.2 Public declarations

*/
  public:

    SpatialJoinLocalInfo(Word innerStreamWord,
                         Word innerAttrIndexWord,
                         Word outerStreamWord,
                         Word outerAttrIndexWord);

/*
The constructor. It builds the initial R-Tree. Further the first
R.Partitions and S.Partitions are built. At this point of time
the Partitions are gathered in maps and Multimaps.

*/

    Tuple* NextResultTuple ();
/*
Returns the next result tuple

*/

   TupleType *resultTupleType;
/*
The tuple type of result tuples.

*/

    ~SpatialJoinLocalInfo()
/*
The destructor

*/
    {
#ifdef PLUGJOIN_VERBOSE_MODE
      cout << "__________________________________________________" << endl;
      cout << endl << "R-Trees built in query : " << rtreeCounter << endl;
      cout << "__________________________________________________" << endl;
#endif

      //delete the SMI-File
      hdr.file.Close();
      hdr.file.Drop();

      // delete the last R-Tree
      delete hdr.rtree;
      hdr.rtree = 0;

      // delete the partitions and Infos
      if (hdr.firstPartitionInfo)
        delete hdr.firstPartitionInfo;

      if (hdr.outerRelPart)
        delete hdr.outerRelPart;

      if(hdr.outerRelInfo)
        delete hdr.outerRelInfo;

      if (hdr.actualMaps)
        delete hdr.actualMaps;

      if (hdr.innerRelPart)
        delete hdr.innerRelPart;

      if (hdr.innerRelInfo)
        delete hdr.innerRelInfo;

      //delete the TupleBuffers
      if ( hdr.innerRelation )
      {
        hdr.innerRelation->Clear();
        delete hdr.innerRelation;
      }

      if ( hdr.outerRelation )
      {
        hdr.outerRelation->Clear();
        delete hdr.outerRelation;
      }
    };

};

/*
3.5.3 The constructor

*/
template <unsigned dim>
SpatialJoinLocalInfo<dim>::SpatialJoinLocalInfo
                           (Word innerStreamWord, Word innerAttrIndexWord,
                            Word outerStreamWord, Word outerAttrIndexWord)
{
/*
First buffer the input streams in TupleBuffers.

*/

  hdr.innerRelation = StreamBuffer (innerStreamWord);   //inserts
  hdr.outerRelation = StreamBuffer (outerStreamWord);   //queries


/*
Create the SMI-File for storing partitions.

*/
  assert ( hdr.file.Create() );
  assert ( hdr.file.IsOpen() );


/*
If one of the both input relations is empty, no further work is do be done.
~NextResultTuple~ proofs, if one Buffer is empty.

*/
  if ( hdr.innerRelation && hdr.outerRelation )
  {

/*
Do some initializing work.

*/
    //the attribute-numbers of the joining attributes
    hdr.outerAttrIndex = (((CcInt*)outerAttrIndexWord.addr)->GetIntval()) - 1;
    ArrayIndex innerAttrIndexPnJ = 
      (((CcInt*)innerAttrIndexWord.addr)->GetIntval())-1;

    //some sizes for storing entries in SMI-Records
    hdr.recordHeaderSize = sizeof(ArrayIndex) //NodeNo of the following
                                              //entries in Record
                       + sizeof(int)          //# of entries in Record
                       + sizeof(SmiRecordId); //# of next Record in SMI-File

    hdr.maxEntriesInRecord = (int)((page_size - hdr.recordHeaderSize) /
                        (sizeof(R_TreeEntryPnJ<dim>)));

    Tuple* innerTuple;

    ArrayIndex nodeNo;  //No of the node in which the insertion would be
                      //done, if the leave overflows

/*
Structures for S.Partition and informations about S.Partition.

*/
    Partition* innerRelPart = new Partition();
    PartitionInfo* innerRelInfo = new PartitionInfo();

/*
Building the initial R-Tree and gathering overflowed entries in partiton.

*/

    //iterator of the inner relation for building initial R-Tree
    TupleBufferIterator *innerIter = hdr.innerRelation->MakeScan();

    //computing the number of nodes to use for initial R-Tree
    int nodesToUse = UseNodesInTree (hdr.outerRelation->GetNoTuples(),
                                     hdr.innerRelation->GetNoTuples());

    hdr.rtree = new R_TreePnJ<dim>( page_size,
                                    default_entries_per_node,
                                    min_entries_per_centage,
                                    nodesToUse );

#ifdef PLUGJOIN_VERBOSE_MODE
    rtreeCounter = 1;
#endif

    while ( (innerTuple = innerIter->GetNextTuple()) != 0 )
    {

      BBox<dim> box = ((StandardSpatialAttribute<dim>*)innerTuple->
                         GetAttribute(innerAttrIndexPnJ))->BoundingBox();
      R_TreeEntryPnJ<dim> e( box, innerIter->GetTupleId() );

      if ( !(hdr.rtree)->Insert( e, nodeNo ) )
      {
        BufferInsert( innerRelPart, innerRelInfo, nodeNo, e);
      }

      innerTuple->DeleteIfAllowed();
    }

    delete innerIter;

/*
Flushing all overflowed leaves to buffer.

*/
    FlushAllLeavesToBuffer(innerRelPart, innerRelInfo);

/*
The buffers may contain some information not stored to SMI-File until now,
so flushes buffer to SMI-file.

*/
    FlushBufferToFile (innerRelPart, innerRelInfo);

    //partitionBuffer no longer needed
    delete innerRelPart;
    innerRelPart = 0;  //safety first

/*
PartitionInfo needed in NextResultTuple for answering the queries.

*/
    hdr.firstPartitionInfo = innerRelInfo;
    innerRelInfo = 0;

#ifdef PLUGJOIN_VERBOSE_MODE
    cout << "==========================================" << endl;
    cout << "Some information about the initial R-Tree:" << endl;
    hdr.rtree->Info();
    cout << "==========================================" << endl;
#endif

/*
Setting controls for REQUEST / nextResultTuple(..).

*/
    hdr.outerIter = hdr.outerRelation->MakeScan();
    hdr.outerRelPart = new Partition();
    hdr.outerRelInfo = new PartitionInfo();

  } // if (hdr.innerRelation || hdr.outerRelation)

};

/*
3.5.3 Method NextResultTuple

*/
template <unsigned dim>
Tuple* SpatialJoinLocalInfo<dim>::NextResultTuple ()
{

/*
If one of the input relations is empty, there could be no result-tuples.

*/

  if ( (hdr.innerRelation == 0) ||
       (hdr.outerRelation == 0) )
  {
    return 0;
  }

/*
Otherwise the join must be computed.

*/

  bool nextResultTupleFound = false;

  Tuple* outerTuple;

  BBox<dim> SBox;                       //searchBox for window-query
  R_TreeEntryPnJ<dim> foundEntry;
  vector <ArrayIndex> leavesOverflowed; //if leaf is empty (overflow in
                                        //constructor) the nodeNo for
                                        //S.Partition

  if (hdr.firstRTree )
  {
/*
Building the queries.

*/
    while ( !nextResultTupleFound )
    {

      if ( hdr.nextTupleOuterRelation )
      {
        if ( (outerTuple = hdr.outerIter->GetNextTuple()) != 0)
        {
/*
Next tuple found in outer Relation.

*/
          SBox = ((StandardSpatialAttribute<dim>*)outerTuple->
                            GetAttribute(hdr.outerAttrIndex))->BoundingBox();
          hdr.outerActualTupleId = hdr.outerIter->GetTupleId();

          hdr.outerEntry = R_TreeEntryPnJ<dim>(SBox, hdr.outerActualTupleId);
        }
        else
        {
/*
No further tuple in outer relation.

*/
          delete hdr.outerIter;
          hdr.outerIter = 0;

          FlushBufferToFile ( hdr.outerRelPart, hdr.outerRelInfo );

          if ( (*hdr.outerRelInfo).empty() )
          {
            //no overflow in initial R-Tree.

            delete hdr.firstPartitionInfo;
            hdr.firstPartitionInfo = 0;

            delete hdr.outerRelPart;
            hdr.outerRelPart = 0;

            delete hdr.outerRelInfo;
            hdr.outerRelInfo = 0;

            return 0;
            break;
          }
          else
          {
/*
Overflows in initial R-Tree, so store gathered overflows and not answerable
queries into stack.

*/

            PartitionsInfo InitialParts;

            InitialParts.innerInfo = hdr.firstPartitionInfo;
            InitialParts.outerInfo = hdr.outerRelInfo;

            Parts.push( InitialParts );

            //safety and deleting
            hdr.firstPartitionInfo = 0;
            hdr.outerRelInfo = 0;

            delete hdr.outerRelPart;
            hdr.outerRelPart = 0;

            //initialize first call for NextResultTupleFromStack
            hdr.firstRTree = false;
            hdr.newPartitionsFromStack = true;  //newMapFromStack
            hdr.nextQueryInPartition = true;  //nextPartitionInMap
            hdr.nextTupleOuterRelation = true;
            hdr.firstSearchForTuple = true;
            hdr.searchForTuple = true;
            hdr.lastPartition = false;

            return NextResultTupleFromStack();

          }

        }
      };


      if ( hdr.firstSearchForTuple )
      {
        if ( hdr.rtree->First (SBox, foundEntry, leavesOverflowed) )
        {
/*
First search for SBox in R-Tree finds an entry.

*/
          hdr.firstSearchForTuple = false;
          hdr.nextTupleOuterRelation = false;
          nextResultTupleFound = true;

/*
Building S.Part of outerRelation.

*/
          FlushLeavesOverflowed(leavesOverflowed, hdr.outerEntry,
                                hdr.outerRelPart, hdr.outerRelInfo);

          Tuple* innerTuple = 
            ((TupleBuffer*)hdr.innerRelation)->GetTuple(foundEntry.pointer);
          Tuple* resultTuple = new Tuple( resultTupleType );
          Concat(outerTuple, innerTuple, resultTuple);

          outerTuple->DeleteIfAllowed();
          outerTuple = 0;
          innerTuple->DeleteIfAllowed();
          innerTuple = 0;

          return resultTuple;
          break;
        }
        else
        {
/*
First search for SBox in R-Tree did not find an entry.

*/
          hdr.firstSearchForTuple = true;
          hdr.nextTupleOuterRelation = true;
          nextResultTupleFound = false;

          outerTuple->DeleteIfAllowed();
          outerTuple = 0;

/*
Building S.Part of outerRelation.

*/
          FlushLeavesOverflowed(leavesOverflowed, hdr.outerEntry,
                                hdr.outerRelPart, hdr.outerRelInfo);
        }
      }

      else   //if ( localInfo->firstSearchForTuple )
      {
        if ( hdr.rtree->Next (foundEntry, leavesOverflowed) )
        {
/*
Next search for SBox finds an entry in the R-Tree.

*/
          hdr.firstSearchForTuple = false;
          hdr.nextTupleOuterRelation = false;
          nextResultTupleFound = true;

/*
Building S.Part of outerRelation.

*/
          FlushLeavesOverflowed(leavesOverflowed, hdr.outerEntry,
                                hdr.outerRelPart, hdr.outerRelInfo);

          Tuple* innerTuple = 
            ((TupleBuffer*)hdr.innerRelation)->GetTuple(foundEntry.pointer);
          Tuple* resultTuple = new Tuple( resultTupleType );

          outerTuple = 
            ((TupleBuffer*)hdr.outerRelation)->GetTuple(hdr.outerActualTupleId);
          Concat(outerTuple, innerTuple, resultTuple);

          outerTuple->DeleteIfAllowed();
          outerTuple = 0;
          innerTuple->DeleteIfAllowed();
          innerTuple = 0;

          return resultTuple;


        }
        else
        {
/*
Next search for SBox did not find an entry in the R-Tree.

*/

          hdr.firstSearchForTuple = true;
          hdr.nextTupleOuterRelation = true;
          nextResultTupleFound = false;

/*
Building S.Part of outerRelation.

*/
          FlushLeavesOverflowed(leavesOverflowed, hdr.outerEntry,
                                hdr.outerRelPart, hdr.outerRelInfo);

        }
      }  //if ( localInfo->firstSearchForTuple )
    }  //while ( !nextResultTupleFound )
  }
  else
  {
/*
Computing the recursive instances of Plug\_Join getting the entries from Stack
and temporary SMI-File. In this case the extracting of bounding boxes and
Tuple-Id is not necessary.

*/
    return NextResultTupleFromStack();
  }

  assert( !nextResultTupleFound );
  return 0;
};

/*
3.5.3 Method NextResultTupleFromStack

*/
template <unsigned dim>
Tuple* SpatialJoinLocalInfo<dim>::NextResultTupleFromStack()
{

  vector <ArrayIndex> leavesOverflowed;

  bool nextResultTupleFound = false;

  //needed for computing the size of next R-Tree to build
  int noEntriesOuterRelation, noEntriesInnerRelation, nodesToUse;

  while ( !nextResultTupleFound )
  {

    if ( hdr.newPartitionsFromStack )
    {
/*
Getting all information about the next recursive instance of the Plug\&Join-
Algorithm from the stack.

*/
      if ( Parts.empty() )
      {
        // no further result tuples
        delete (hdr.actualMaps)->innerInfo;
        (hdr.actualMaps)->innerInfo = 0;

        delete (hdr.actualMaps)->outerInfo;
        (hdr.actualMaps)->outerInfo = 0;

        return 0;
        break;
      }
      else
      {
/*
In every recursive instance of the Plug\&Join-Algorithm a R-Tree for each
overflowed leaf of a former recursive instance must be built. Now extract
the informations needed for building the R-Trees of this instance one by one.

*/

#ifdef PLUGJOIN_VERY_VERBOSE_MODE
        cout << endl
             << "**********************************************************"
             << "***************************" << endl;
        cout << "Stacksize = " << Parts.size()
             << "  (=number of recursive instances in the stack)" << endl;
        cout << "=========================================================="
             << "======" << endl;
#endif

        hdr.actualMaps = new PartitionsInfo;
        (hdr.actualMaps)->innerInfo = Parts.top().innerInfo;
        (hdr.actualMaps)->outerInfo = Parts.top().outerInfo;
        Parts.pop();

#ifdef PLUGJOIN_VERY_VERBOSE_MODE
      cout << "Number partitions of outer relation: "
           << ((*hdr.actualMaps).outerInfo)->size() << endl;
      cout << "Number partitions of inner relation: "
           << ((*hdr.actualMaps).innerInfo)->size() << endl;
      cout << "************************************************************"
           << "*************************" << endl;
#endif

#ifdef PLUGJOIN_VERY_VERBOSE_MODE
      cout << "============================================================="
           << "===" << endl;
      cout << "Not inserted entries in the R-Tree " << endl;
      cout << "(LeafNumber NumberOfNotInsertedEntries SmiRecordIDFirst "
           << "SmiRecordIdNext)" << endl;
      typedef typename  PartitionInfo::const_iterator CI;
      for (CI p=((*hdr.actualMaps).innerInfo)->begin();
              p!=((*hdr.actualMaps).innerInfo)->end(); ++p)
      {
        cout << p->first << "   " << p->second.counter << "    ";
        cout <<p->second.firstRId << "   " << p->second.actualRId << endl;
      }
      cout << "Number of Leaves with insertOverflow:"
           << ((*hdr.actualMaps).innerInfo)->size() << endl;
      cout << "============================================================="
           << "===" << endl << endl;

      cout << "============================================================="
           << "===" << endl;
      cout << "Queries which touched a leaf with insertOverflow " << endl;
      cout << "(LeafNumber NumberOfQueries SmiRecordIDFirst SmiRecordIdNext)" 
           << endl;
      typedef typename  PartitionInfo::const_iterator CI;
      for (CI p=((*hdr.actualMaps).outerInfo)->begin();
              p!=((*hdr.actualMaps).outerInfo)->end(); ++p)
      {
        cout << p->first << "   " << p->second.counter << "    ";
        cout <<p->second.firstRId << "   " << p->second.actualRId << endl;
      }
      cout << "Number of Leaves with touching queries:"
           << ((*hdr.actualMaps).outerInfo)->size() << endl;
      cout << "================================================================"
           << endl << endl;
#endif
        //Caution: actualMapsIter iterates the queries
        //innerPartitons without queries need not to
        //be considered
        hdr.actualMapsIter = ((*hdr.actualMaps).outerInfo)->end();


        hdr.newPartitionsFromStack = false;
        hdr.searchForTuple = true;

      }
    }

/*
Information for one recursive instance of Plug\&Join is iterated leaf by leaf.
For each leaf a R-Tree is built and the corresponding entries of the inner relation
are inserted in the Tree.

*/
    if ( hdr.nextQueryInPartition )
    {
      hdr.actualMapsIter--;    //the algorithm guarantees at least one partition
      hdr.actualNodeNo = hdr.actualMapsIter->first;

      if (  hdr.actualMapsIter == ((*hdr.actualMaps).outerInfo)->begin() )
        hdr.lastPartition = true;
      else
        hdr.lastPartition = false;

      hdr.newPartitionsFromStack = false;
      hdr.nextQueryInPartition = false;
      hdr.firstSearchForTuple = true;
      hdr.nextTupleOuterRelation = true;
      hdr.getEntryFromFirstRecord = true;

      //delete the old R-Tree
      delete hdr.rtree;
      hdr.rtree = 0;

      //number of nodes to use in next R-Tree
      noEntriesOuterRelation = hdr.actualMapsIter->second.counter;

      NodeInfo innerInfo = ((*(*hdr.actualMaps).innerInfo) [hdr.actualNodeNo]);
      noEntriesInnerRelation = innerInfo.counter;

      nodesToUse = UseNodesInTree (noEntriesOuterRelation,
                                   noEntriesInnerRelation);

      hdr.rtree = new R_TreePnJ<dim>( page_size,                  //the R-Tree
                                      default_entries_per_node,
                                      min_entries_per_centage,
                                      nodesToUse );

#ifdef PLUGJOIN_VERBOSE_MODE
      rtreeCounter++;
#endif

      R_TreeEntryPnJ<dim>* entry;
      ArrayIndex nodeNoOverflow;

      hdr.innerRelPart = new Partition;
      hdr.innerRelInfo = new PartitionInfo;

      hdr.outerRelPart = new Partition;
      hdr.outerRelInfo = new PartitionInfo;

      hdr.GetNextEntry_readFirst = true;
      hdr.GetNextEntry_read = true;

      while ( (entry = GetNextEntry ( ((*hdr.actualMaps).innerInfo),
                                      hdr.actualNodeNo)) != 0 )
      {
        if ( !hdr.rtree->Insert (*entry, nodeNoOverflow) )
        { 
          BufferInsert (hdr.innerRelPart, hdr.innerRelInfo, 
                        nodeNoOverflow, *entry);
        }
      }

      FlushAllLeavesToBuffer (hdr.innerRelPart, hdr.innerRelInfo);
      FlushBufferToFile (hdr.innerRelPart, hdr.innerRelInfo);
    }

/*
Getting the queries and build resultTuples for stream-REQUEST.

*/

    if ( hdr.nextTupleOuterRelation )
    {
      if ( hdr.getEntryFromFirstRecord )
      {
       hdr.GetNextEntry_readFirst = true;
       hdr.GetNextEntry_read = true;
       hdr.getEntryFromFirstRecord = false;
      }

      R_TreeEntryPnJ<dim>* outerEntry;
      if ( (outerEntry =
           GetNextEntry (((*hdr.actualMaps).outerInfo), hdr.actualNodeNo)) == 0)
      {
        //all queries of partition computed

        hdr.searchForTuple = false;
        hdr.nextQueryInPartition = true;

        if ( hdr.lastPartition )
        {
          // initalize call for next map from Stack
          hdr.newPartitionsFromStack = true;
          hdr.nextQueryInPartition = true;
          hdr.firstSearchForTuple = true;
          hdr.nextTupleOuterRelation = true;
          hdr.getEntryFromFirstRecord = true;
        }


        if ( ! hdr.outerRelInfo->empty() )
        {

          FlushBufferToFile ( hdr.outerRelPart, hdr.outerRelInfo);

          PartitionsInfo newPartitionsInfo;
          newPartitionsInfo.innerInfo = hdr.innerRelInfo;
          newPartitionsInfo.outerInfo = hdr.outerRelInfo;
          Parts.push( newPartitionsInfo);

          hdr.innerRelInfo = 0;
          hdr.outerRelInfo = 0;

          delete hdr.outerRelPart;
          hdr.outerRelPart = 0;

          delete hdr.innerRelPart;
          hdr.innerRelPart = 0;
        }
        else
        {

          delete hdr.innerRelInfo;
          hdr.innerRelInfo = 0;

          delete hdr.outerRelInfo;
          hdr.outerRelInfo = 0;

          delete hdr.outerRelPart;
          hdr.outerRelPart = 0;

          delete hdr.innerRelPart;
          hdr.innerRelPart = 0;
        }
      }
      else
      {
        hdr.outerEntry = *outerEntry;
        delete outerEntry;
        hdr.searchForTuple = true;
      }

    };


    if ( hdr.searchForTuple )
    {
      if ( hdr.firstSearchForTuple )
      {
        if ( hdr.rtree->First (hdr.outerEntry.box, 
                               hdr.foundEntry, 
                               leavesOverflowed) )
        {
          hdr.firstSearchForTuple = false;
          hdr.nextTupleOuterRelation = false;
          nextResultTupleFound = true;

          FlushLeavesOverflowed (leavesOverflowed, hdr.outerEntry,
                                   hdr.outerRelPart, hdr.outerRelInfo );

          return newResultTupleFromEntries (hdr.outerEntry, hdr.foundEntry);
          break;
        }
        else
        {
          hdr.firstSearchForTuple = true;
          hdr.nextTupleOuterRelation = true;
          nextResultTupleFound = false;
          FlushLeavesOverflowed (leavesOverflowed, hdr.outerEntry,
                                 hdr.outerRelPart, hdr.outerRelInfo );
        }
      }
      else   // if ( firstSearchForTuple)
      {
       if ( hdr.rtree->Next (hdr.foundEntry, leavesOverflowed) )
       {
          hdr.firstSearchForTuple = false;
          hdr.nextTupleOuterRelation = false;
          nextResultTupleFound = true;

          FlushLeavesOverflowed (leavesOverflowed, hdr.outerEntry,
                                 hdr.outerRelPart, hdr.outerRelInfo);

          return newResultTupleFromEntries (hdr.outerEntry, hdr.foundEntry);
          break;
        }
        else
        {
          hdr.firstSearchForTuple = true;
          hdr.nextTupleOuterRelation = true;
          nextResultTupleFound = false;

          FlushLeavesOverflowed (leavesOverflowed, hdr.outerEntry,
                                 hdr.outerRelPart, hdr.outerRelInfo);

        }
      }  // if ( firstSearchForTuple )
    }  // if ( nextTupleOuterRelation )

  }// while (!nextResultTupleFound )

  assert( !nextResultTupleFound );
  return 0;
};

/*
3.5.3 Method BufferInsert

*/
template <unsigned dim>
void SpatialJoinLocalInfo<dim>::BufferInsert
                               (multimap< ArrayIndex,R_TreeEntryPnJ<dim> >* mm,
                                map< ArrayIndex,NodeInfo >* m,
                                ArrayIndex& nodeNo, R_TreeEntryPnJ<dim>& entry)
{
  (*mm).insert(make_pair(nodeNo, entry));

  typename map< ArrayIndex,NodeInfo >::iterator p;
  p = (*m).find(nodeNo);

  if ( p != (*m).end() )   //nodeNo found in map
  {
    p->second.counter = p->second.counter + 1;

    if ( (p->second.counter % hdr.maxEntriesInRecord) == 0)
    {
      SmiRecordId nextRecno;
      SmiRecord*  nextRecord = new SmiRecord();
      assert( hdr.file.AppendRecord( nextRecno, *nextRecord ) );

      Write ( mm, nodeNo, p->second.actualRId, nextRecno);

      delete nextRecord;

      p->second.actualRId = nextRecno;
    }

  }
  else  //nodeNo not found in map;create FirstRecord for leave
  {
    SmiRecordId firstRecno;
    SmiRecord*  firstRecord = new SmiRecord();
    assert( hdr.file.AppendRecord( firstRecno, *firstRecord ) );

    delete firstRecord;

    (*m).insert ( make_pair (nodeNo, NodeInfo(1, firstRecno, firstRecno) ) );

  };

};

/*
3.5.3 Method FlushAllLeavesToBuffer

*/
template <unsigned dim>
void 
SpatialJoinLocalInfo<dim>::FlushAllLeavesToBuffer(Partition* innerRelPart,
                                                  PartitionInfo* innerRelInfo)
{
  typedef typename PartitionInfo::const_iterator CI;
  ArrayIndex nodeNo;

  for (CI p=(innerRelInfo)->begin(); p!=(innerRelInfo)->end(); ++p)
  {
    R_TreeNodePnJ<dim>* leave = hdr.rtree->FlushLeave(p->first);

    for(int entryCount = 0; entryCount <= leave->EntryCount() - 1; entryCount++)
    {
      R_TreeEntryPnJ<dim> e ((*leave)[entryCount]);
      nodeNo = p->first;
      BufferInsert ( innerRelPart, innerRelInfo, nodeNo, e);

    }
    delete leave;

  }
};

/*
3.5.3 Method FlushBufferToFile

*/
template <unsigned dim>
void SpatialJoinLocalInfo<dim>::FlushBufferToFile
                               (Partition* mm,
                                PartitionInfo* m)
{
  typedef typename PartitionInfo::iterator CI;

  for (CI p=m->begin(); p!=m->end(); ++p)
  {
    ArrayIndex i = p->first;
    Write (mm, i, p->second.actualRId, p->second.actualRId);
  }
};

/*
3.5.3 Method FlushLeavesOverflowed

*/
template <unsigned dim>
void SpatialJoinLocalInfo<dim>::FlushLeavesOverflowed
                                (vector <ArrayIndex>& leavesOverflowed,
                                 R_TreeEntryPnJ<dim>& entry,
                                 Partition* outerRelPart,
                                 PartitionInfo* outerRelInfo)
{
  for (int i = 0; i <= ((int)leavesOverflowed.size() - 1); i++)
  {
    BufferInsert ( outerRelPart, outerRelInfo, leavesOverflowed[i], entry);
  }
  leavesOverflowed.clear();
};

/*
3.5.3 Method Write

*/
template <unsigned dim>
void SpatialJoinLocalInfo<dim>::Write
                                (Partition* mm,
                                 ArrayIndex& nodeNo,
                                 SmiRecordId& recNo,
                                 SmiRecordId& nextRecordId)
{
  int ctr = mm->count(nodeNo);
  int offset = 0;
  int sizeOfRecord = hdr.recordHeaderSize + ctr * sizeof(R_TreeEntryPnJ<dim>);
  char buffer[sizeOfRecord + 1];  // reserving space for buffer
  memset ( buffer, 0, sizeOfRecord + 1); //initializes the buffer with 0

  //writes record header
  memcpy ( buffer + offset, &nodeNo, sizeof(nodeNo) );
  offset += sizeof(nodeNo);
  memcpy ( buffer + offset, &ctr, sizeof(ctr) );
  offset += sizeof(ctr);
  memcpy ( buffer + offset, &nextRecordId, sizeof(nextRecordId) );
  offset += sizeof(nextRecordId);

  //collecting all entries for node in buffer
  typedef typename Partition::iterator MI;
  pair <MI, MI> g = mm->equal_range (nodeNo);

  for (MI p = g.first; p != g.second; ++p)
  {
    memcpy ( buffer + offset, &(p->second), sizeof(p->second) );
    offset += sizeof(p->second);
  }

  //writing to SMI-file
  SmiRecord* record = new SmiRecord();
  assert ( hdr.file.SelectRecord ( recNo, *record, SmiFile::Update) );
  assert ( (*record).Write( buffer, sizeOfRecord, 0) );
  delete record;

  //erase entries for node ~nodeNo~
  mm->erase (g.first, g.second);
};

/*
3.5.3 Method ReadFirstEntries

*/
template <unsigned dim>
void SpatialJoinLocalInfo<dim>::ReadFirstEntries
      (PartitionInfo* m, 
       ArrayIndex& actualNodeNo,
       SmiRecordId& nextRecordId, 
       vector < R_TreeEntryPnJ<dim> >& entriesOfRecord )
{
  //getting number of first record
  typename PartitionInfo::const_iterator inner_iter;
  assert ( (inner_iter = m->find(actualNodeNo)) != m->end() );

  SmiRecordId firstRecordId = inner_iter->second.firstRId;

  //interpreting the record
  int sizeOfRecord = hdr.recordHeaderSize + 
                     hdr.maxEntriesInRecord * 
                     sizeof(R_TreeEntryPnJ<dim>);
  char buffer[sizeOfRecord + 1];
  memset ( buffer, 0, sizeOfRecord + 1);
  int offset = 0;

  ArrayIndex nodeNoInRecord;
  int numberOfEntries;

  //reading first record
  SmiRecord* record = new SmiRecord();
  assert ( hdr.file.SelectRecord (firstRecordId, *record) );
  assert ( (*record).Read(buffer, sizeOfRecord, 0) );
  delete record;

  //reads nodeNo, number of entries and nextRecordId
  memcpy (&nodeNoInRecord, buffer + offset, sizeof (nodeNoInRecord));
  offset += sizeof (nodeNoInRecord);
  memcpy (&numberOfEntries, buffer + offset, sizeof (numberOfEntries));
  offset += sizeof (numberOfEntries);
  memcpy (&nextRecordId, buffer + offset, sizeof (nextRecordId));
  offset += sizeof (nextRecordId);

  assert ( nodeNoInRecord == actualNodeNo );

  //now read the entries and store in entriesOfRecord
  for (int i = 1; i <= numberOfEntries; i++)
  {
    R_TreeEntryPnJ<dim> *e = 
      new ((void *)(buffer + offset)) R_TreeEntryPnJ<dim>;
    entriesOfRecord.push_back(*e);
    offset += sizeof (R_TreeEntryPnJ<dim>);
  };

  //no further nextRecord to read
  if (nextRecordId == firstRecordId)
    nextRecordId = 0;

};

/*
3.5.3 Method ReadNextEntries

*/
template <unsigned dim>
void SpatialJoinLocalInfo<dim>::ReadNextEntries ( SmiRecordId& nextRecordId,
                           vector < R_TreeEntryPnJ<dim> >&entriesOfRecord )
{
  //interpreting the record
  int sizeOfRecord = 
    hdr.recordHeaderSize + hdr.maxEntriesInRecord * sizeof(R_TreeEntryPnJ<dim>);
  char buffer[sizeOfRecord + 1];
  memset ( buffer, 0, sizeOfRecord + 1);
  int offset = 0;

  ArrayIndex nodeNoInRecord;
  int numberOfEntries;
  SmiRecordId oldNextRecordId = nextRecordId;

  //reading first record
  SmiRecord* record = new SmiRecord();
  assert ( hdr.file.SelectRecord (oldNextRecordId, *record) );
  assert ( (*record).Read(buffer, sizeOfRecord, 0) );
  delete record;

  //reads nodeNo, number of entries and nextRecordId
  memcpy (&nodeNoInRecord, buffer + offset, sizeof (nodeNoInRecord));
  offset += sizeof (nodeNoInRecord);
  memcpy (&numberOfEntries, buffer + offset, sizeof (numberOfEntries));
  offset += sizeof (numberOfEntries);
  memcpy (&nextRecordId, buffer + offset, sizeof (nextRecordId));
  offset += sizeof (nextRecordId);

  //now read the entries and store in entriesOfRecord
  for (int i = 1; i <= numberOfEntries; i++)
  {
    R_TreeEntryPnJ<dim> *e = 
      new ((void *)(buffer + offset)) R_TreeEntryPnJ<dim>;
    entriesOfRecord.push_back(*e);
    offset += sizeof (R_TreeEntryPnJ<dim>);
  };

  //no further nextRecord to read
  if (nextRecordId == oldNextRecordId)
    nextRecordId = 0;

};

/*
3.5.3 Method GetNextEntry

*/
template <unsigned dim>
R_TreeEntryPnJ<dim>* SpatialJoinLocalInfo<dim>::GetNextEntry
            (PartitionInfo* m, ArrayIndex& actualNodeNo)
{
  static SmiRecordId NextRecordId;
  static int counter;
  static vector < R_TreeEntryPnJ<dim> > entriesOfRecord;

  if ( hdr.GetNextEntry_read && hdr.GetNextEntry_readFirst )
  {
    entriesOfRecord.clear();
    ReadFirstEntries (m, actualNodeNo, NextRecordId, entriesOfRecord);
    hdr.GetNextEntry_readFirst = false;
    counter = entriesOfRecord.size() - 1;

    if (NextRecordId == 0)
    { hdr.GetNextEntry_read = false; }
    else
    { hdr.GetNextEntry_read = true; }
  }

  if ( counter >= 0 )
  {
    R_TreeEntryPnJ<dim>* entry = 
      new R_TreeEntryPnJ<dim> (entriesOfRecord[counter]);
    counter--;
    return entry;
  }
  else
  {
    if ( hdr.GetNextEntry_read )
    {
      entriesOfRecord.clear();
      ReadNextEntries (NextRecordId, entriesOfRecord);

      if ( NextRecordId == 0)
      { hdr.GetNextEntry_read = false; }

      counter = entriesOfRecord.size() - 1;

      if ( counter >= 0 )
      {
        R_TreeEntryPnJ<dim>* entry = 
          new R_TreeEntryPnJ<dim> (entriesOfRecord[counter]);
        counter--;
        return entry;
      }
      else
      { // could be the last read record has maxEntriesInRecord entries
        // but the actual record is empty
        return 0;
      }
    }
    else
    { // no further entry in non-empty record
      return 0;
    }

  }
}

/*
3.5.3 Method StreamBuffer

*/
template <unsigned dim>
TupleBuffer* SpatialJoinLocalInfo<dim>::StreamBuffer (const Word stream)
{
  Word streamTupleWord;
  TupleBuffer *streamBuffer;

  //OPEN the stream
  qp->Open (stream.addr);

  //REQUQEST the stream
  qp->Request (stream.addr, streamTupleWord);

  if ( qp->Received ( stream.addr) )
  {
    streamBuffer = new TupleBuffer ();  //StandardSize is 32 MB
  }
  else
  {
    streamBuffer = 0;
  }

  while (qp->Received (stream.addr) )
  {
    Tuple *t = (Tuple*)streamTupleWord.addr;
    streamBuffer->AppendTuple ( t );
    t->DeleteIfAllowed();
    qp->Request ( stream.addr, streamTupleWord);
  }

  //CLOSE the stream
  qp->Close (stream.addr);

  return streamBuffer;

}

/*
3.5.3 Method newResultTupleFromEntries

*/
template <unsigned dim>
Tuple*  SpatialJoinLocalInfo<dim>::newResultTupleFromEntries (
                                               R_TreeEntryPnJ<dim>& outerEntry,
                                               R_TreeEntryPnJ<dim>& foundEntry)
{
  Tuple* innerTuple = 
    ((TupleBuffer*)hdr.innerRelation)->GetTuple(foundEntry.pointer);
  Tuple* outerTuple = 
    ((TupleBuffer*)hdr.outerRelation)->GetTuple(outerEntry.pointer);
  Tuple* resultTuple = new Tuple (resultTupleType);
  Concat (outerTuple, innerTuple, resultTuple);

  innerTuple->DeleteIfAllowed();
  outerTuple->DeleteIfAllowed();

  return resultTuple;
}

/*
3.5.3 Method UseNodesInTree

*/
template <unsigned dim>
int SpatialJoinLocalInfo<dim>::UseNodesInTree (const int noTuplesR,
                                                   const int noTuplesS)
{
  int nodesToUse = int (scalingFactor * (noTuplesR + noTuplesS) /
                       (default_entries_per_node * max_leaves_of_rtree) +1.0);

/*
Only the maximum number of nodes may be used, but the minimum number of nodes
must be used.

*/
  return max (min (nodesToUse, max_leaves_of_rtree), min_leaves_of_rtree);
}

/*
3.5 Value mapping function of operator ~spatialjoin~

*/
int
spatialjoin2ValueMapping(Word* args, Word& result, int message, 
                         Word& local, Supplier s)
{
  SpatialJoinLocalInfo<2> *localInfo;

  switch (message)
  {
    case OPEN:
    {
      Word leftStreamWord,
           rightStreamWord,
           leftAttrIndexWord,
           rightAttrIndexWord;

      leftStreamWord = args[0];
      rightStreamWord = args[1];
      leftAttrIndexWord = args[4];  //APPENDED - Value no 1
      rightAttrIndexWord = args[5]; //APPENDED - Value no 2

      localInfo = new SpatialJoinLocalInfo<2>(rightStreamWord, 
                                              rightAttrIndexWord,
                                              leftStreamWord, 
                                              leftAttrIndexWord);

      ListExpr resultType = GetTupleResultType( s );
      localInfo->resultTupleType = new TupleType( nl->Second( resultType ) );

      local = SetWord(localInfo);

      return 0;
    }

    case REQUEST:
    {
      localInfo = (SpatialJoinLocalInfo<2>*)local.addr;
      result = SetWord(localInfo->NextResultTuple());
      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
    {
      localInfo = (SpatialJoinLocalInfo<2>*)local.addr;
      //the TupleBuffers are deleted in the destructor of localInfo
      localInfo->resultTupleType->DeleteIfAllowed();
      delete localInfo;

      return 0;
    }

  }

  return 0;

};

int
spatialjoin3ValueMapping(Word* args, Word& result, int message, 
                         Word& local, Supplier s)
{
  SpatialJoinLocalInfo<3> *localInfo;

  switch (message)
  {
    case OPEN:
    {
      Word leftStreamWord,
           rightStreamWord,
           leftAttrIndexWord,
           rightAttrIndexWord;

      leftStreamWord = args[0];
      rightStreamWord = args[1];
      leftAttrIndexWord = args[4];  //APPENDED - Value no 1
      rightAttrIndexWord = args[5]; //APPENDED - Value no 2


      localInfo = new SpatialJoinLocalInfo<3>(rightStreamWord, 
                                              rightAttrIndexWord,
                                              leftStreamWord, 
                                              leftAttrIndexWord);

      ListExpr resultType = GetTupleResultType( s );
      localInfo->resultTupleType = new TupleType( nl->Second( resultType ) );

      local = SetWord(localInfo);

      return 0;
    }

    case REQUEST:
    {
      //cout << endl << "REQUEST" << endl;
      localInfo = (SpatialJoinLocalInfo<3>*)local.addr;
      result = SetWord(localInfo->NextResultTuple());
      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
    {
      localInfo = (SpatialJoinLocalInfo<3>*)local.addr;

      //the TupleBuffers are deleted in the destructor of localInfo
      localInfo->resultTupleType->DeleteIfAllowed();
      delete localInfo;

      return 0;
    }

  }

  return 0;

};

int
spatialjoin4ValueMapping(Word* args, Word& result, int message, 
                         Word& local, Supplier s)
{
  SpatialJoinLocalInfo<4> *localInfo;

  switch (message)
  {
    case OPEN:
    {
      Word leftStreamWord,
           rightStreamWord,
           leftAttrIndexWord,
           rightAttrIndexWord;

      leftStreamWord = args[0];
      rightStreamWord = args[1];
      leftAttrIndexWord = args[4];  //APPENDED - Value no 1
      rightAttrIndexWord = args[5]; //APPENDED - Value no 2


      localInfo = new SpatialJoinLocalInfo<4>(rightStreamWord, 
                                              rightAttrIndexWord,
                                              leftStreamWord, 
                                              leftAttrIndexWord);

      ListExpr resultType = GetTupleResultType( s );
      localInfo->resultTupleType = new TupleType( nl->Second( resultType ) );

      local = SetWord(localInfo);

      return 0;
    }

    case REQUEST:
    {
      //cout << endl << "REQUEST" << endl;
      localInfo = (SpatialJoinLocalInfo<4>*)local.addr;
      result = SetWord(localInfo->NextResultTuple());
      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
    {
      localInfo = (SpatialJoinLocalInfo<4>*)local.addr;

      //the TupleBuffers are deleted in the destructor of localInfo
      localInfo->resultTupleType->DeleteIfAllowed();
      delete localInfo;

      return 0;
    }

  }

  return 0;

};

/*
3.7 Definition of value mapping vectors

*/
ValueMapping spatialjoinMap [] = {spatialjoin2ValueMapping,
                                  spatialjoin3ValueMapping,
                                  spatialjoin4ValueMapping };

/*
3.8 Specification of operator ~spatialjoin~

*/
const string spatialjoinSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\""
      " \"Example\" )"
      "( <text>( (stream (tuple ((x1 t1)...(xn tn)))) "
      "(stream (tuple ((y1 yt1)...(yn ytn))))"
      " rect||rect3||rect4||SpatialType rect||rect3||rect4||SpatialType) -> "
      "(stream (tuple ((x1 t1)...(xn tn)((y1 yt1)...(yn ytn)))))</text--->"
      "<text>outerStream innerStream spatialjoin "
      "[outerAttr, innerAttr]</text--->"
      "<text>Uses the Plug&Join-Algorithm to find all pairs of intersecting"
      " tuples in the given relations. The joining tuples are reported."
      "</text--->"
      "<text>query trees feed streets feed spatialjoin [pos_trees, pos_streets]"
      " consume; the joining attributes must be of the same dimension."
      "</text--->"
      ") )";

/*
3.9 Definition of operator ~spatialjoin~

*/

Operator spatialjoin (
         "spatialjoin",             // name
         spatialjoinSpec,           // specification
         3,                         // number of overloaded functions
         spatialjoinMap,            // value mapping
         spatialjoinSelection,      // trivial selection function
         spatialjoinTypeMap         // type mapping
);

/*
4 Definition and initialization of ~PlugJoin~ algebra

*/
class PlugJoinAlgebra : public Algebra
{
 public:
  PlugJoinAlgebra() : Algebra()
  {
    AddOperator(&spatialjoin);
  }
  ~PlugJoinAlgebra() {};
};

PlugJoinAlgebra plugjoinalgebra;


extern "C"
Algebra*
InitializePlugJoinAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&plugjoinalgebra);
}

