/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

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

May 2008, Victor Almeida 

[1] Implementation of MON-Tree Algebra

[TOC]

1 Includes and Defines

*/

#include <iostream>
#include "MONTreeAlgebra.h"
#include "RectangleAlgebra.h"
#include "TupleIdentifier.h"
#include "SpatialAlgebra.h"
#include "HashAlgebra.h"
#include "HashAlgebra.h"
#include "NetworkAlgebra.h"
#include "TemporalAlgebra.h"
#include "TemporalNetAlgebra.h"

using namespace std;

MON_Tree::MON_Tree():
index( true, 4000 ),
routeHash( new Hash(SmiKey::Integer) ),
network( NULL ),
top_RTree( NULL ),
bottom_RTree( NULL )
{
  index.Create();
  top_RTree = new R_Tree<2, TopR_TreeLeafInfo>( &index );
}

void MON_Tree::SetNetwork( Network *network )
{
  this->network = network;
}

MON_Tree::MON_Tree( Network *network,
                    SmiFileId indexFileId,
                    SmiFileId hashFileId ):
index( true ),
routeHash( new Hash(SmiKey::Integer, hashFileId) ),
network( network ),
top_RTree( NULL ),
bottom_RTree( NULL )
{
  index.Open( indexFileId );
  top_RTree = new R_Tree<2, TopR_TreeLeafInfo>( &index, 1 );
}

MON_Tree::~MON_Tree()
{
  delete routeHash;
  if( bottom_RTree != NULL )
    delete bottom_RTree;
  delete top_RTree;
  if( index.IsOpen() )
    index.Close();
}

void MON_Tree::Insert( const int routeId, 
                       const SmiRecordId bottomId )
{
  routeHash->Append( SmiKey((long)routeId), bottomId );
}

void MON_Tree::Insert( const MGPoint& mgpoint, 
                       const SmiRecordId mgpointId )
{
  assert( network != NULL );

  for( int i = 0; i < mgpoint.GetNoComponents(); i++ )
  {
    const UGPoint *ugpoint;
    mgpoint.Get( i, ugpoint );

    int routeId = ugpoint->p0.GetRouteId();
    CcInt key( true, routeId );
    HashIterator *iter = routeHash->ExactMatch( &key );

    Rectangle<3> box3D = ugpoint->NetBoundingBox3d();
    Rectangle<2> box2D = BBox<2>( true, box3D.MinD(1), box3D.MaxD(1), 
                                        box3D.MinD(2), box3D.MaxD(2) );
    // box is constructed first with the position, then the interval.
    
    R_TreeLeafEntry<2, SmiRecordId> 
      entry( box2D, mgpointId );
    // VTA - The original code of the MON-Tree is expecting the entries 
    // in the bounding in a different order. There will be an error in
    // query processing.

    if( iter->Next() )
    {
      assert( iter->GetId() > 0 );
      bottom_RTree = new R_Tree<2, SmiRecordId>( &index, iter->GetId() );
    }
    else
    {
      Tuple *t = network->GetRoute( routeId );
      Line *curve = (Line*)t->GetAttribute( ROUTE_CURVE );  
      Rectangle<2> routeBox = curve->BoundingBox();

      assert( bottom_RTree == NULL );
      bottom_RTree = new R_Tree<2, SmiRecordId>( &index );
         
      routeHash->Append( SmiKey( (long)key.GetIntval() ), 
                         bottom_RTree->HeaderRecordId() );

      TopR_TreeLeafInfo info( routeId, bottom_RTree->HeaderRecordId() );
      top_RTree->Insert( R_TreeLeafEntry<2, TopR_TreeLeafInfo>( routeBox, 
                                                               info ) );
      t->DeleteIfAllowed();
    }
    delete iter;

    bottom_RTree->Insert( entry );
    delete bottom_RTree; bottom_RTree = NULL;
  }
}

void MON_Tree::CalculateSearchBoxSet( const Rectangle<2>& box,
                                      const SimpleLine& curve,
                                      const Interval<Instant>& timeInterval,
                                      RectangleSet<2>& result ) const
{
  assert( box.Intersects( curve.BoundingBox() ) );

  if( box.Contains( curve.BoundingBox() ) )
  {
    result.Add( BBox<2>( true, 
                         0.0, curve.Length(), 
                         timeInterval.start.ToDouble(), 
                         timeInterval.end.ToDouble() ) );
  }
  else
  {
    double p1 = -1, 
           p2 = -1;

    for( int i = 0; i < curve.Size()/2; i++ )
    {
      const LRS *lrs;
      curve.Get( i, lrs );
      const HalfSegment *hs;
      curve.Get( lrs->hsPos, hs );

      if( box.Intersects( hs->BoundingBox() ) )
      {
        if( p1 < 0 )
        {
          assert( p2 < 0 );
          p1 = lrs->lrsPos;
          p2 = p1;
        }
        p2 += hs->Length();
      }
      else 
      {
        if( p2 >= 0 )
        {
          assert( p1 >= 0 );
          result.Add( BBox<2>( true,
                               p1, p2, 
                               timeInterval.start.ToDouble(),
                               timeInterval.end.ToDouble() ) );
        }
        p1 = -1;
        p2 = -1;
      }
    }
    if( p2 >= 0 )
    {
      assert( p1 >= 0 );
      result.Add( BBox<2>( true, 
                           p1, p2, 
                           timeInterval.start.ToDouble(),
                           timeInterval.end.ToDouble() ) );
    }
  }
}

bool MON_Tree::First( const Rectangle<2>& box, 
                      const Interval<Instant>& timeInterval,
                      R_TreeLeafEntry<2, SmiRecordId>& result )
{
  assert( network != NULL );

  R_TreeLeafEntry<2, TopR_TreeLeafInfo> entry;
  if( !top_RTree->First( box, entry ) )
    return false;

  assert( bottom_RTree == NULL );
  searchBox = box;
  searchTimeInterval = timeInterval;
  begin = true;
  assert( entry.info.childTreeId > 0 );

  bottom_RTree = new R_Tree<2, SmiRecordId>( &index, entry.info.childTreeId );
  Tuple *t = network->GetRoute( entry.info.routeId );
  SimpleLine *curve = (SimpleLine*)t->GetAttribute( ROUTE_CURVE );  

  searchBoxSet.Clear();
  CalculateSearchBoxSet( searchBox, 
                         *curve, 
                         searchTimeInterval, 
                         searchBoxSet );

  t->DeleteIfAllowed();

  return Next( result );
}

bool MON_Tree::Next( R_TreeLeafEntry<2, SmiRecordId>& result )
{
  assert( network != NULL );

  R_TreeLeafEntry<2, TopR_TreeLeafInfo> topEntry;

  bool found = false;
  do
  {
    if( begin )
    {
      if( bottom_RTree == NULL )
        found = false;
      else
      {
        found = bottom_RTree->First( searchBoxSet, result );
        begin = false;
      }
    }
    else
    {
      assert( bottom_RTree != NULL );
      found = bottom_RTree->Next( result );
    }

    if( !found )
    {
      if( !top_RTree->Next( topEntry ) )
      {
        delete bottom_RTree; 
        bottom_RTree = NULL;
        return false;
      }

      delete bottom_RTree; 
      bottom_RTree = NULL;
      begin = true;
      assert( topEntry.info.childTreeId > 0 );
      bottom_RTree = new R_Tree<2, SmiRecordId>( &index, 
                                                 topEntry.info.childTreeId );
      Tuple *t = network->GetRoute( topEntry.info.routeId );
      SimpleLine *curve = (SimpleLine*)t->GetAttribute( ROUTE_CURVE );  
      searchBoxSet.Clear();
      CalculateSearchBoxSet( searchBox, 
                             *curve, 
                             searchTimeInterval, 
                             searchBoxSet );
      t->DeleteIfAllowed();
    }
  } while( !found );

  return true;
}

/*
1 Type constructor ~montree~

1.1 Type property of type constructor ~montree~

*/
ListExpr MON_TreeProp()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,
    "<network> <relation> createmontree [<attrname>]"
    " where <attrname> is the key of type mgpoint");

  return
    (nl->TwoElemList(
         nl->TwoElemList(nl->StringAtom("Creation"),
                         nl->StringAtom("Example Creation")),
         nl->TwoElemList(examplelist,
                         nl->StringAtom("(let mon = net1 trains1"
                         " createmontree [trip])"))));
}

/*
1.8 ~Check~-function of type constructor ~montree~

*/
bool CheckMON_Tree(ListExpr type, ListExpr& errorInfo)
{
  AlgebraManager* algMgr;

  if((!nl->IsAtom(type))
    && (nl->ListLength(type) == 3)
    && nl->Equal(nl->First(type), nl->SymbolAtom("montree")))
  {
    algMgr = SecondoSystem::GetAlgebraManager();
    return
      algMgr->CheckKind("TUPLE", nl->Second(type), errorInfo) &&
      nl->IsEqual(nl->Third(type), "mgpoint"); 
  }
  errorInfo = nl->Append(errorInfo,
    nl->ThreeElemList(
      nl->IntAtom(60),
      nl->SymbolAtom("MONTREE"),
      type));
  return false;
}

/*
6 Functions for the type constructors

6.1 ~Out~-function

It does not make sense to have an index as an independent value
since the record ids stored in it become obsolete as soon as
the underlying relation is deleted. Therefore this function
does nothing.

*/
ListExpr OutMON_Tree(ListExpr typeInfo, Word value)
{
  return nl->StringAtom("OutMON_Tree");
}

/*
6.2 ~In~-function

Reading an index from a list does not make sense because it
is not an independent value. Therefore calling this function leads
to program abort.

*/
Word InMON_Tree( ListExpr typeInfo, ListExpr value,
                int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = false;
  return SetWord(Address(0));
}

/*
6.3 ~Create~-function

*/
Word CreateMON_Tree( const ListExpr typeInfo )
{
  return SetWord( new MON_Tree() );
}

/*
6.4 ~Close~-function

*/
void CloseMON_Tree( const ListExpr typeInfo, Word& w )
{
  MON_Tree *montree = (MON_Tree*)w.addr;
  delete montree;
}

/*
6.5 ~Clone~-function

Not implemented yet.

*/
Word CloneMON_Tree( const ListExpr typeInfo, const Word& w )
{
  return SetWord( Address(0) );
}

/*
6.6 ~Delete~-function

*/
void DeleteMON_Tree( const ListExpr typeInfo, Word& w )
{
  MON_Tree *montree = (MON_Tree*)w.addr;
  delete montree;
}

/*
6.7 ~Cast~-function

*/
void* CastMON_Tree( void* addr)
{
  return ( 0 );
}

/*
6.8 ~Open~-function

*/
bool OpenMON_Tree( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value )
{
  SmiFileId indexFileId;
  valueRecord.Read( &indexFileId, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );

  SmiRecordId hashFileId;
  valueRecord.Read( &hashFileId, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );

  MON_Tree *montree = new MON_Tree( NULL,
                                    indexFileId,
                                    hashFileId );

  value = SetWord( montree );
  return true;
}

/*
6.9 ~Save~-function

*/
bool SaveMON_Tree( SmiRecord& valueRecord,
                  size_t& offset,
                  const ListExpr typeInfo,
                  Word& value )
{
  MON_Tree *montree = (MON_Tree *)value.addr;
  SmiFileId indexFileId = montree->GetIndexFileId(),
            hashFileId = montree->GetHashFileId();
  
  valueRecord.Write( &indexFileId, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );

  valueRecord.Write( &hashFileId, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );

  return true;
}

/*
6.10 ~SizeOf~-function

*/
int SizeOfMON_Tree()
{
  return 0;
}

/*
1.12 Type Constructor object for type constructor ~montree~

*/
TypeConstructor montree( "montree",
                         MON_TreeProp,
                         OutMON_Tree,
                         InMON_Tree,
                         0,
                         0,
                         CreateMON_Tree,
                         DeleteMON_Tree,
                         OpenMON_Tree,
                         SaveMON_Tree,
                         CloseMON_Tree,
                         CloneMON_Tree,
                         CastMON_Tree,
                         SizeOfMON_Tree,
                         CheckMON_Tree );


/*
7 Operators of the MON-Tree Algebra

7.1 Operator ~createmontree~

7.1.1 Type Mapping of operator ~createmontree~

*/
ListExpr CreateMONTreeTypeMap(ListExpr args)
{
  string attrName, relDescriptionStr, argstr;
  string errmsg = "Incorrect input for operator createmontree.";
  int attrIndex;
  ListExpr attrType;

  CHECK_COND(!nl->IsEmpty(args) &&
    nl->ListLength(args) == 3,
    errmsg + "\nOperator createmontree expects three arguments.");

  CHECK_COND(nl->IsEqual(nl->First(args), "network"),
    errmsg + "\nThe first argument must be of type network.");

  ListExpr relDescription = nl->Second(args),
           attrNameLE = nl->Third(args);

  CHECK_COND(nl->IsAtom(attrNameLE) &&
    nl->AtomType(attrNameLE) == SymbolType,
    errmsg + "\nThe third argument must be the name of "
    "the attribute to index.");
  attrName = nl->SymbolValue(attrNameLE);

  // Check for relation or tuplestream as first argument
  if( !IsRelDescription(relDescription) &&
      !IsStreamDescription(relDescription) )
  {
    nl->WriteToString (relDescriptionStr, relDescription);
    ErrorReporter::ReportError( "\nOperator createmontree expects a relation "
        " or a stream of tuples as its second argument, but gets '"
        + relDescriptionStr + "'.");
    return nl->TypeError();
  }

  // Test for index attribute
  ListExpr tupleDescription = nl->Second(relDescription);
  ListExpr attrList = nl->Second(tupleDescription);
  CHECK_COND(
    (attrIndex = FindAttribute(attrList, attrName, attrType)) > 0,
    errmsg +
    "\nOperator createmontree expects the attribute " +
    attrName + "\npassed as third argument to be part of "
    "the relation or stream description\n'" +
    relDescriptionStr + "'.");

  CHECK_COND(nl->IsEqual(attrType, "mgpoint"),
    errmsg +
    "\nOperator createmontree expects that attribute "+attrName+"\n"
    "belongs to type mgpoint.");

  if( nl->IsEqual(nl->First(relDescription), "rel") )
  {
    return
      nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->OneElemList(
          nl->IntAtom(attrIndex)),
        nl->ThreeElemList(
          nl->SymbolAtom("montree"),
          tupleDescription,
          attrType));
  }
  else
  {
    ListExpr first, rest, newAttrList, lastNewAttrList;
    int tidIndex = 0;
    string type;
    bool firstcall = true;

    rest = attrList;
    int j = 1;
    while (!nl->IsEmpty(rest))
    {
      first = nl->First(rest);
      rest = nl->Rest(rest);

      type = nl->SymbolValue(nl->Second(first));
      if (type == "tid")
      {
        CHECK_COND( tidIndex == 0,
          "Operator createmontree expects as second argument a stream "
          "with\none and only one attribute of type 'tid'\n'"
          "but gets\n'" + relDescriptionStr + "'.");

        tidIndex = j;
      }
      else
      {
        if (firstcall)
        {
          firstcall = false;
          newAttrList = nl->OneElemList(first);
          lastNewAttrList = newAttrList;
        }
        else
        {
          lastNewAttrList = nl->Append(lastNewAttrList, first);
        }
      }
      j++;
    }
    CHECK_COND( tidIndex != 0,
      "Operator createmontree expects as second argument a stream "
      "with\none and only one attribute of type 'tid'\n'"
      "but gets\n'" + relDescriptionStr + "'.");

    return
      nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->TwoElemList(
          nl->IntAtom(attrIndex),
          nl->IntAtom(tidIndex)),
        nl->ThreeElemList(
          nl->SymbolAtom("montree"),
          nl->TwoElemList(
            nl->SymbolAtom("tuple"),
            newAttrList),
          attrType));
  }
}

/*
4.1.2 Selection function of operator ~createmontree~

*/
int
CreateMONTreeSelect (ListExpr args)
{
  ListExpr relDescription = nl->Second(args),
           attrNameLE = nl->Third(args),
           tupleDescription = nl->Second(relDescription),
           attrList = nl->Second(tupleDescription);
  string attrName = nl->SymbolValue(attrNameLE);

  int attrIndex;
  ListExpr attrType;
  attrIndex = FindAttribute(attrList, attrName, attrType);

  if( nl->SymbolValue(nl->First(relDescription)) == "rel")
    return 0;
  if( nl->SymbolValue(nl->First(relDescription)) == "stream")
    return 1;

  return -1;
}

/*
4.1.3 Value mapping function of operator ~createmontree~

*/
int CreateMONTreeRel(Word* args, Word& result, int message,
                     Word& local, Supplier s)
{
  Relation* relation;
  int attrIndex;
  GenericRelationIterator* iter;
  Tuple* tuple;
  Network *network;

  MON_Tree *montree = (MON_Tree*)qp->ResultStorage(s).addr;
  result.setAddr( montree );

  network = (Network*)args[0].addr;
  relation = (Relation*)args[1].addr;
  attrIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;

  montree->SetNetwork( network );

  iter = relation->MakeScan();
  while( (tuple = iter->GetNextTuple()) != 0 )
  {
    MGPoint* mgpoint = (MGPoint*)tuple->GetAttribute(attrIndex);
    if( mgpoint->IsDefined() )
    {
      montree->Insert( *mgpoint, tuple->GetTupleId() );
    }
    tuple->DeleteIfAllowed();
  }
  delete iter;

  return 0;
}

int CreateMONTreeStream(Word* args, Word& result, int message,
                        Word& local, Supplier s)
{
  Word wTuple;
  MON_Tree *montree = (MON_Tree*)qp->ResultStorage(s).addr;
  result.setAddr( montree );

  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1,
      tidIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wTuple);
  while (qp->Received(args[0].addr))
  {
    Tuple* tuple = (Tuple*)wTuple.addr;
    MGPoint *mgpoint = (MGPoint*)tuple->GetAttribute(attrIndex);
    TupleIdentifier *tupleId = (TupleIdentifier *)tuple->GetAttribute(tidIndex);

    if( mgpoint->IsDefined() && tupleId->IsDefined() )
    {
      montree->Insert( *mgpoint, tupleId->GetTid() );
    }
    tuple->DeleteIfAllowed();
    qp->Request(args[0].addr, wTuple);
  }
  qp->Close(args[0].addr);

  return 0;
}

/*
4.1.5 Definition of value mapping vectors

*/
ValueMapping createmontreemap [] = { CreateMONTreeRel,
                                     CreateMONTreeStream };

/*
4.1.6 Specification of operator ~createmontree~

*/

const string CreateMONTreeSpec  =
  "( ( \"1st Signature\""
  " \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>((rel (tuple (x1 t1)...(xn tn)))) xi)"
  " -> (montree (tuple ((x1 t1)...(xn tn))) ti)\n"
  "((stream (tuple (x1 t1)...(xn tn) (id tid))) xi)"
  " -> (montree (tuple ((x1 t1)...(xn tn))) ti)\n"
  "<text>_ createmontree [ _ ]</text--->"
  "<text>Creates a montree. The key type ti must "
  "be of type mgpoint.</text--->"
  "<text>let mymontree = Trains "
  "createmontree[Trip]</text--->"
  ") )";

/*
4.1.7 Definition of operator ~createmontree~

*/
Operator createmontree (
          "createmontree",       // name
          CreateMONTreeSpec,     // specification
          2,                  // Number of overloaded functions
          createmontreemap, // value mapping
          CreateMONTreeSelect,   // trivial selection function
          CreateMONTreeTypeMap   // type mapping
);

/*
7.2 Operator ~windowintersects~

7.2.1 Type mapping function of operator ~windowintersects~

*/
ListExpr MON_WindowIntersectsTypeMap(ListExpr args)
{
  string errmsg = "Incorrect input for operator windowintersects.";
  string monDescriptionStr, relDescriptionStr;

  CHECK_COND(!nl->IsEmpty(args), errmsg);
  CHECK_COND(!nl->IsAtom(args), errmsg);
  CHECK_COND(nl->ListLength(args) == 6, errmsg);

  /* Split argument in three parts */
  ListExpr networkDescription = nl->First(args),
           monDescription = nl->Second(args),
           relDescription = nl->Third(args),
           searchWindow = nl->Fourth(args),
           searchStart = nl->Fifth(args),
           searchEnd = nl->Sixth(args);

  CHECK_COND(nl->IsEqual(networkDescription, "network"),
    "Operator windowintersects expects the first argument\n"
    "to be of type network.");
    
  /* Query window: find out type of key */
  CHECK_COND(nl->IsEqual(searchWindow, "rect"),
    "Operator windowintersects expects that the search window\n"
    "is of type rect.");

  // Handling of the time interval
  CHECK_COND(nl->IsEqual(searchStart, "instant") &&
             nl->IsEqual(searchEnd, "instant"),
    "Operator windowintersects expects the fifth and sixth\n"
    "arguments to be a time interval.");

  /* handle montree part of argument */
  nl->WriteToString (monDescriptionStr, monDescription);
  CHECK_COND(!nl->IsEmpty(monDescription) &&
    !nl->IsAtom(monDescription) &&
    nl->ListLength(monDescription) == 3,
    "Operator windowintersects expects a MON-Tree with structure "
    "(montree (tuple ((a1 t1)...(an tn))) attrtype\n"
    "\nbut gets the following '"
    +monDescriptionStr+"'.");

  ListExpr monSymbol = nl->First(monDescription),
           monTupleDescription = nl->Second(monDescription);

  /* handle rtree type constructor */
  CHECK_COND(nl->IsEqual(monSymbol, "montree"),
   "Operator windowintersects expects a montree \n"
   "as second argument.");

  /* handle rel part of argument */
  nl->WriteToString (relDescriptionStr, relDescription);
  CHECK_COND(IsRelDescription(relDescription),
    "Operator windowintersects expects a "
    "relation as its third argument, but gets '"
        + relDescriptionStr + "'.");

  return
    nl->TwoElemList(
      nl->SymbolAtom("stream"),
      monTupleDescription);
}

/*
5.1.3 Value mapping function of operator ~windowintersects~

*/
struct MON_WindowIntersectsLocalInfo
{
  Relation* relation;
  MON_Tree* montree;
  BBox<2> searchBox;
  Interval<Instant> searchTimeInterval;
  bool first;
};

int MON_WindowIntersects( Word* args, Word& result,
                      int message, Word& local,
                      Supplier s )
{
  MON_WindowIntersectsLocalInfo *localInfo;

  switch (message)
  {
    case OPEN :
    {
      localInfo = new MON_WindowIntersectsLocalInfo;
      localInfo->montree = (MON_Tree*)args[1].addr;
      localInfo->montree->SetNetwork( (Network*)args[0].addr );
      localInfo->relation = (Relation*)args[2].addr;
      localInfo->first = true;

      localInfo->searchBox = *(BBox<2>*)args[3].addr;
      localInfo->searchTimeInterval = 
        Interval<Instant>( *(Instant*)args[4].addr,
                           *(Instant*)args[5].addr,
                           true, true ); 
 
      assert(localInfo->montree != 0);
      assert(localInfo->relation != 0);
      local.setAddr(localInfo);
      return 0;
    }

    case REQUEST :
    {
      localInfo = (MON_WindowIntersectsLocalInfo*)local.addr;
       R_TreeLeafEntry<2, SmiRecordId> e;

      if ( !localInfo->searchBox.IsDefined() ||
           !localInfo->searchTimeInterval.IsValid() )
      { 
        return CANCEL;
      }
      else
      {
        if(localInfo->first)
        {
          localInfo->first = false;
          if( localInfo->montree->First( localInfo->searchBox, 
                                         localInfo->searchTimeInterval,
                                         e ) )
          {
            Tuple *tuple = localInfo->relation->GetTuple(e.info);
            result.setAddr(tuple);
            return YIELD;
          }
          else
            return CANCEL;
        }
        else
        {
          if( localInfo->montree->Next( e ) )
          {
            Tuple *tuple = localInfo->relation->GetTuple(e.info);
            result.setAddr(tuple);
            return YIELD;
          }
          else
            return CANCEL;
        }
      }
    }

    case CLOSE :
    {
      if(local.addr)
      {
        localInfo = (MON_WindowIntersectsLocalInfo*)local.addr;
        delete localInfo;
        local.setAddr(Address(0));
      }
      return 0;
    }
  }
  return 0;
}

/*
5.1.5 Specification of operator ~windowintersects~

*/
const string windowintersectsSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>montree(tuple ((x1 t1)...(xn tn))"
      " ti) x rel(tuple ((x1 t1)...(xn tn))) "
      "x rect x instant x instant ->\n"
      " (stream (tuple ((x1 t1)...(xn tn))))\n</text--->"
      "<text>_ _ windowintersects [ _, _, _ ]</text--->"
      "<text>Uses the given MON-Tree to find all tuples"
      " in the given relation with .xi intersects the "
      " argument value's bounding box within "
      " the time interval argument.</text--->"
      "<text>query trainsInd trains windowintersects"
      " [r, t1, t2] consume; where trainsInd "
      "is e.g. created with 'let trainsInd = "
      "net trains createmontree [trip]'</text--->"
      ") )";

/*
5.1.6 Definition of operator ~windowintersects~

*/
Operator windowtimeintersects (
         "windowtimeintersects",        // name
         windowintersectsSpec,      // specification
         MON_WindowIntersects,
         Operator::SimpleSelect,    // trivial selection function
         MON_WindowIntersectsTypeMap    // type mapping
);


/*
6 Definition and initialization of RTree Algebra

*/
class MONTreeAlgebra : public Algebra
{
 public:
  MONTreeAlgebra() : Algebra()
  {
    AddTypeConstructor( &montree );

    AddOperator( &createmontree );
    AddOperator( &windowtimeintersects );
  }
  ~MONTreeAlgebra() {};
};



extern "C"
Algebra*
InitializeMONTreeAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new MONTreeAlgebra);
}

