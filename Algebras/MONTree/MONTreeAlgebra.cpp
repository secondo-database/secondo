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
#include "./MONTreeAlgebra.h"
#include "../Rectangle/RectangleAlgebra.h"
#include "../TupleIdentifier/TupleIdentifier.h"
#include "../Spatial/SpatialAlgebra.h"
#include "../Hash/HashAlgebra.h"
#include "../Network/NetworkAlgebra.h"
#include "../Temporal/TemporalAlgebra.h"
#include "../TemporalNet/TemporalNetAlgebra.h"
#include "Symbols.h"
#include "ListUtils.h"

using namespace std;

template<class BottomR_TreeLeafInfo>
MON_Tree<BottomR_TreeLeafInfo>::MON_Tree():
index( true, 4000 ),
routeHash( new Hash(SmiKey::Integer) ),
network( NULL ),
top_RTree( NULL ),
bottom_RTree( NULL )
{
  index.Create();
  top_RTree = new R_Tree<2, TopR_TreeLeafInfo>( &index );
}

template<class BottomR_TreeLeafInfo>
void MON_Tree<BottomR_TreeLeafInfo>::SetNetwork( network::Network *network )
{
  this->network = network;
}

template<class BottomR_TreeLeafInfo>
MON_Tree<BottomR_TreeLeafInfo>::MON_Tree( network::Network *network,
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

template<class BottomR_TreeLeafInfo>
MON_Tree<BottomR_TreeLeafInfo>::~MON_Tree()
{
  delete routeHash;
  if( bottom_RTree != NULL )
    delete bottom_RTree;
  delete top_RTree;
  if( index.IsOpen() )
    index.Close();
}

template<class BottomR_TreeLeafInfo>
void MON_Tree<BottomR_TreeLeafInfo>::Insert( const int routeId,
                       const SmiRecordId bottomId )
{
  routeHash->Append( SmiKey((long)routeId), bottomId );
}

template<class BottomR_TreeLeafInfo>
void MON_Tree<BottomR_TreeLeafInfo>::Insert( 
                       const temporalnet::MGPoint& mgpoint,
                       const BottomR_TreeLeafInfo& info )
{
  assert( network != NULL );

  for( int i = 0; i < mgpoint.GetNoComponents(); i++ )
  {
    temporalnet::UGPoint ugpoint;
    mgpoint.Get( i, ugpoint );
    Insert( ugpoint, info );
  }
}

template<class BottomR_TreeLeafInfo>
void MON_Tree<BottomR_TreeLeafInfo>::Insert( 
                       const temporalnet::UGPoint& ugpoint,
                       const BottomR_TreeLeafInfo& info )
{
  int routeId = ugpoint.p0.GetRouteId();
  CcInt key( true, routeId );
  HashIterator *iter = routeHash->ExactMatch( &key );

  Rectangle<3> box3D = ugpoint.NetBoundingBox3d();
  Rectangle<2> box2D = BBox<2>( true, box3D.MinD(1), box3D.MaxD(1),
                                      box3D.MinD(2), box3D.MaxD(2) );
  // box is constructed first with the position, then the interval.

  R_TreeLeafEntry<2, BottomR_TreeLeafInfo>
    entry( box2D, info );

  if( iter->Next() )
  {
    assert( iter->GetId() > 0 );
    bottom_RTree = new R_Tree<2, BottomR_TreeLeafInfo>( &index, iter->GetId() );
  }
  else
  {
    Tuple *t = network->GetRoute( routeId );
    Line *curve = (Line*)t->GetAttribute( ROUTE_CURVE );
    Rectangle<2> routeBox = curve->BoundingBox();

    assert( bottom_RTree == NULL );
    bottom_RTree = new R_Tree<2, BottomR_TreeLeafInfo>( &index );

    routeHash->Append( SmiKey( key.GetIntval() ),
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

template<class BottomR_TreeLeafInfo>
void MON_Tree<BottomR_TreeLeafInfo>::
  CalculateSearchBoxSet( const Rectangle<2>& box,
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
      LRS lrs;
      curve.Get( i, lrs );
      HalfSegment hs;
      curve.Get( lrs.hsPos, hs );

      if( box.Intersects( hs.BoundingBox() ) )
      {
        if( p1 < 0 )
        {
          assert( p2 < 0 );
          p1 = lrs.lrsPos;
          p2 = p1;
        }
        p2 += hs.Length();
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

template<class BottomR_TreeLeafInfo>
bool MON_Tree<BottomR_TreeLeafInfo>::First( const Rectangle<2>& box,
                      const Interval<Instant>& timeInterval,
                      R_TreeLeafEntry<2, BottomR_TreeLeafInfo>& result )
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

  bottom_RTree =
    new R_Tree<2, BottomR_TreeLeafInfo>( &index, entry.info.childTreeId );
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

template<class BottomR_TreeLeafInfo>
bool MON_Tree<BottomR_TreeLeafInfo>::
  Next( R_TreeLeafEntry<2, BottomR_TreeLeafInfo>& result )
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
      bottom_RTree =
        new R_Tree<2, BottomR_TreeLeafInfo>( &index,
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
    && (nl->ListLength(type) == 4)
    && nl->Equal(nl->First(type), nl->SymbolAtom("montree")))
  {
    algMgr = SecondoSystem::GetAlgebraManager();
    return
      algMgr->CheckKind(Kind::TUPLE(), nl->Second(type), errorInfo) &&
      nl->IsEqual(nl->Third(type), "mgpoint") &&
      nl->IsAtom(nl->Fourth(type)) &&
      nl->AtomType(nl->Fourth(type)) == BoolType;
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
  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
    return SetWord( new MON_Tree<BottomR_TreeLeafInfo>() );
  else
    return SetWord( new MON_Tree<SmiRecordId>() );
}

/*
6.4 ~Close~-function

*/
void CloseMON_Tree( const ListExpr typeInfo, Word& w )
{
  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
  {
    MON_Tree<BottomR_TreeLeafInfo> *montree =
      (MON_Tree<BottomR_TreeLeafInfo>*)w.addr;
    delete montree;
  }
  else
  {
    MON_Tree<SmiRecordId> *montree =
      (MON_Tree<SmiRecordId>*)w.addr;
    delete montree;
  }
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
  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
  {
    MON_Tree<BottomR_TreeLeafInfo> *montree =
      (MON_Tree<BottomR_TreeLeafInfo>*)w.addr;
    delete montree;
  }
  else
  {
    MON_Tree<SmiRecordId> *montree =
      (MON_Tree<SmiRecordId>*)w.addr;
    delete montree;
  }
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


  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
  {
    MON_Tree<BottomR_TreeLeafInfo> *montree =
      new MON_Tree<BottomR_TreeLeafInfo>( NULL,
                                          indexFileId,
                                          hashFileId );
    value = SetWord( montree );
  }
  else
  {
    MON_Tree<SmiRecordId> *montree =
      new MON_Tree<SmiRecordId>( NULL,
                                 indexFileId,
                                 hashFileId );
    value = SetWord( montree );
  }
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
  SmiFileId indexFileId, hashFileId;

  if( nl->BoolValue(nl->Fourth(typeInfo)) == true )
  {
    MON_Tree<BottomR_TreeLeafInfo> *montree =
      (MON_Tree<BottomR_TreeLeafInfo> *)value.addr;
    indexFileId = montree->GetIndexFileId();
    hashFileId = montree->GetHashFileId();
  }
  else
  {
    MON_Tree<SmiRecordId> *montree =
      (MON_Tree<SmiRecordId> *)value.addr;
    indexFileId = montree->GetIndexFileId();
    hashFileId = montree->GetHashFileId();
  }

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

  if(nl->ListLength(args) != 3){
    return listutils::typeError(errmsg +
    "\nOperator createmontree expects three arguments.");
  }

    if(!nl->IsEqual(nl->First(args), "network")){
      return listutils::typeError( errmsg +
      "\nThe first argument must be of type network.");
    }

  ListExpr relDescription = nl->Second(args),
           attrNameLE = nl->Third(args);

  if(!nl->IsAtom(attrNameLE) || (nl->AtomType(attrNameLE) != SymbolType)){
    return listutils::typeError( errmsg +
    "\nThe third argument must be the name of "
    "the attribute to index.");
  }
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
  attrIndex = FindAttribute(attrList, attrName, attrType);
  if( attrIndex <= 0 ){
    return listutils::typeError( errmsg +
    "\nOperator createmontree expects the attribute " +
    attrName + "\npassed as third argument to be part of "
    "the relation or stream description\n'" +
    relDescriptionStr + "'.");
  }
  if(!nl->IsEqual(attrType, "mgpoint") &&
     !nl->IsEqual(attrType, "ugpoint")){
    return listutils::typeError(errmsg +
    "\nOperator createmontree expects that attribute "+attrName+"\n"
    "belongs to type mgpoint or ugpoint.");
  }

  bool mgpoint = nl->IsEqual(attrType, "mgpoint");

  if( nl->IsEqual(nl->First(relDescription), Relation::BasicType()) )
  {
    return
      nl->ThreeElemList(
        nl->SymbolAtom(Symbol::APPEND()),
        nl->OneElemList(
          nl->IntAtom(attrIndex)),
        nl->FourElemList(
          nl->SymbolAtom("montree"),
          tupleDescription,
          attrType,
          nl->BoolAtom(mgpoint)));
  }
  else
  {
/*
Here we can have two possibilities:

- multi-entry indexing, or
- double indexing

For multi-entry indexing, one and only one of the attributes
must be a tuple identifier. In the latter, together with
a tuple identifier, the last two attributes must be of
integer type (~int~).

In the first case, a standard MON-Tree is created
containing several entries to the same tuple identifier, and
in the latter, a double index MON-Tree is created using as low
and high parameters these two last integer numbers.

*/
    ListExpr first = nl->TheEmptyList();
    ListExpr rest = nl->TheEmptyList();
    ListExpr newAttrList = nl->TheEmptyList();
    ListExpr lastNewAttrList = nl->TheEmptyList();
    int tidIndex = 0;
    string type;
    bool firstcall = true,
         doubleIndex = false;

    int nAttrs = nl->ListLength( attrList );
    rest = attrList;
    int j = 1;
    while (!nl->IsEmpty(rest))
    {
      first = nl->First(rest);
      rest = nl->Rest(rest);

      type = nl->SymbolValue(nl->Second(first));
      if (type == TupleIdentifier::BasicType())
      {
        if( tidIndex != 0){
          return listutils::typeError(
          "Operator createmontree expects as first argument a stream "
          "with\none and only one attribute of type 'tid'\n'"
          "but gets\n'" + relDescriptionStr + "'.");
        }
        tidIndex = j;
      }
      else if( j == nAttrs - 1 && type == CcInt::BasicType() &&
               nl->SymbolValue(
                 nl->Second(nl->First(rest))) == CcInt::BasicType() )
      { // the last two attributes are integers
        doubleIndex = true;
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
    if( tidIndex == 0){
      return listutils::typeError(
      "Operator createmontree expects as first argument a stream "
      "with\none and only one attribute of type 'tid'\n'"
      "but gets\n'" + relDescriptionStr + "'.");
    }

    return
      nl->ThreeElemList(
        nl->SymbolAtom(Symbol::APPEND()),
        nl->TwoElemList(
          nl->IntAtom(attrIndex),
          nl->IntAtom(tidIndex)),
        nl->FourElemList(
          nl->SymbolAtom("montree"),
          nl->TwoElemList(
            nl->SymbolAtom(Tuple::BasicType()),
            newAttrList),
          attrType,
          nl->BoolAtom(doubleIndex)));
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

  ListExpr attrType;
  FindAttribute(attrList, attrName, attrType);

  int doubleUp = 0;
  if(nl->IsEqual(attrType, "ugpoint"))
    doubleUp = 3;

  if( nl->SymbolValue(nl->First(relDescription)) == Relation::BasicType())
    return doubleUp + 0;
  if( nl->SymbolValue(nl->First(relDescription)) == Symbol::STREAM())
  {
    ListExpr first = nl->TheEmptyList();
    ListExpr rest = attrList;
    while (!nl->IsEmpty(rest))
    {
      first = nl->First(rest);
      rest = nl->Rest(rest);
    }
    if( nl->IsEqual( nl->Second( first ), CcInt::BasicType() ) )
      // Double indexing
      return doubleUp + 2;
    else
      // Multi-entry indexing
      return doubleUp + 1;
  }

  return -1;
}

/*
4.1.3 Value mapping function of operator ~createmontree~

*/
int CreateMONTreeRelMGPoint(Word* args, Word& result, int message,
                            Word& local, Supplier s)
{
  Relation* relation;
  int attrIndex;
  GenericRelationIterator* iter;
  Tuple* tuple;
  network::Network *network;

  MON_Tree<BottomR_TreeLeafInfo> *montree =
    (MON_Tree<BottomR_TreeLeafInfo> *)qp->ResultStorage(s).addr;
  result.setAddr( montree );

  network = (network::Network*)args[0].addr;
  relation = (Relation*)args[1].addr;
  attrIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;

  montree->SetNetwork( network );

  iter = relation->MakeScan();
  while( (tuple = iter->GetNextTuple()) != 0 )
  {
    temporalnet::MGPoint* mgpoint = 
                   (temporalnet::MGPoint*)tuple->GetAttribute(attrIndex);
    if( mgpoint->IsDefined() )
    {
      for( int i = 0; i < mgpoint->GetNoComponents(); i++ )
      {
        BottomR_TreeLeafInfo info(tuple->GetTupleId(), i, i);
        temporalnet::UGPoint ugpoint;
        mgpoint->Get( i, ugpoint );
        montree->Insert( ugpoint, info );
      }
    }
    tuple->DeleteIfAllowed();
  }
  delete iter;

  return 0;
}

int CreateMONTreeRelUGPoint(Word* args, Word& result, int message,
                            Word& local, Supplier s)
{
  Relation* relation;
  int attrIndex;
  GenericRelationIterator* iter;
  Tuple* tuple;
  network::Network *network;

  MON_Tree<SmiRecordId> *montree =
    (MON_Tree<SmiRecordId> *)qp->ResultStorage(s).addr;
  result.setAddr( montree );

  network = (network::Network*)args[0].addr;
  relation = (Relation*)args[1].addr;
  attrIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;

  montree->SetNetwork( network );

  iter = relation->MakeScan();
  while( (tuple = iter->GetNextTuple()) != 0 )
  {
    temporalnet::UGPoint* ugpoint = 
                (temporalnet::UGPoint*)tuple->GetAttribute(attrIndex);
    if( ugpoint->IsDefined() )
      montree->Insert( *ugpoint, tuple->GetTupleId() );
    tuple->DeleteIfAllowed();
  }
  delete iter;

  return 0;
}

template<class T>
int CreateMONTreeStream(Word* args, Word& result, int message,
                        Word& local, Supplier s)
{
  Word wTuple;
  MON_Tree<SmiRecordId> *montree =
    (MON_Tree<SmiRecordId> *)qp->ResultStorage(s).addr;
  result.setAddr( montree );

  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1,
      tidIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wTuple);
  while (qp->Received(args[0].addr))
  {
    Tuple* tuple = (Tuple*)wTuple.addr;
    T *t = (T*)tuple->GetAttribute(attrIndex);
    TupleIdentifier *tupleId = (TupleIdentifier *)tuple->GetAttribute(tidIndex);

    if( t->IsDefined() && tupleId->IsDefined() )
    {
      montree->Insert( *t, tupleId->GetTid() );
    }
    tuple->DeleteIfAllowed();
    qp->Request(args[0].addr, wTuple);
  }
  qp->Close(args[0].addr);

  return 0;
}

template<class T>
int CreateMONTreeDblStream(Word* args, Word& result, int message,
                           Word& local, Supplier s)
{
  Word wTuple;
  MON_Tree<BottomR_TreeLeafInfo> *montree =
    (MON_Tree<BottomR_TreeLeafInfo> *)qp->ResultStorage(s).addr;
  result.setAddr( montree );

  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1,
      tidIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wTuple);
  while (qp->Received(args[0].addr))
  {
    Tuple* tuple = (Tuple*)wTuple.addr;
    T *t = (T*)tuple->GetAttribute(attrIndex);
    TupleIdentifier *tupleId =
      (TupleIdentifier *)tuple->GetAttribute(tidIndex);
    CcInt *low = (CcInt*)tuple->GetAttribute(tuple->GetNoAttributes()-2),
          *high = (CcInt*)tuple->GetAttribute(tuple->GetNoAttributes()-1);

    if( t->IsDefined() &&
        tupleId->IsDefined() &&
        low->IsDefined() &&
        high->IsDefined() )
    {
      BottomR_TreeLeafInfo info( tupleId->GetTid(),
                                 low->GetIntval(),
                                 high->GetIntval() );
      montree->Insert( *t, info );
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
ValueMapping createmontreemap [] = { 
            CreateMONTreeRelMGPoint,
            CreateMONTreeStream<temporalnet::MGPoint>,
            CreateMONTreeDblStream<temporalnet::MGPoint>,
            CreateMONTreeRelUGPoint,
            CreateMONTreeStream<temporalnet::UGPoint>,
            CreateMONTreeDblStream<temporalnet::UGPoint> };

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
          6,                  // Number of overloaded functions
          createmontreemap, // value mapping
          CreateMONTreeSelect,   // trivial selection function
          CreateMONTreeTypeMap   // type mapping
);

/*
7.2 Operator ~windowtimeintersects~

7.2.1 Type mapping function of operator ~windowtimeintersects~

*/
ListExpr MON_WindowTimeIntersectsTypeMap(ListExpr args)
{
  string errmsg = "Incorrect input for operator windowtimeintersects.";
  string monDescriptionStr, relDescriptionStr;

  if(nl->ListLength(args) != 6){
    return listutils::typeError(errmsg);
  }

  /* Split argument in three parts */
  ListExpr networkDescription = nl->First(args),
           monDescription = nl->Second(args),
           relDescription = nl->Third(args),
           searchWindow = nl->Fourth(args),
           searchStart = nl->Fifth(args),
           searchEnd = nl->Sixth(args);

  if(!nl->IsEqual(networkDescription, "network")){
     return listutils::typeError(
    "Operator windowtimeintersects expects the first argument\n"
    "to be of type network.");
  }

  /* Query window: find out type of key */
  if(!nl->IsEqual(searchWindow, Rectangle<2>::BasicType())){
    return listutils::typeError(
    "Operator windowtimeintersects expects that the search window\n"
    "is of type rect.");
  }

  // Handling of the time interval
  if(!nl->IsEqual(searchStart, Instant::BasicType()) ||
    !nl->IsEqual(searchEnd, Instant::BasicType())){
    return listutils::typeError(
    "Operator windowtimeintersects expects the fifth and sixth\n"
    "arguments to be a time interval.");
  }

  /* handle montree part of argument */
  nl->WriteToString (monDescriptionStr, monDescription);
  if(nl->ListLength(monDescription) != 4){
    return listutils::typeError(
    "Operator windowtimeintersects expects a MON-Tree with structure "
    "(montree (tuple ((a1 t1)...(an tn))) attrtype bool)\n"
    "\nbut gets the following '"
    +monDescriptionStr+"'.");
  }

  ListExpr monSymbol = nl->First(monDescription);

  /* handle rtree type constructor */
  if(!nl->IsEqual(monSymbol, "montree")){
    return listutils::typeError(
      "Operator windowtimeintersects expects a MON-Tree \n"
      "as second argument.");
  }

  ListExpr monTupleDescription = nl->Second(monDescription);

  /* handle rtree type constructor */
/*  if(nl->IsEqual(monSymbol, "montree")){
    return listutils::typeError(
   "Operator windowtimeintersects expects a MON-Tree \n"
   "as second argument.);
  }*/

  /* handle rel part of argument */
  nl->WriteToString (relDescriptionStr, relDescription);
  if(!IsRelDescription(relDescription)){
    return listutils::typeError(
    "Operator windowtimeintersects expects a "
    "relation as its third argument, but gets '"
        + relDescriptionStr + "'.");
  }

  return
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      monTupleDescription);
}

/*
5.1.3 Value mapping function of operator ~windowintersects~

*/
struct MON_WindowTimeIntersectsLocalInfo
{
  Relation* relation;
  MON_Tree<SmiRecordId>* montree;
  BBox<2> searchBox;
  Interval<Instant> searchTimeInterval;
  bool first;
};

int MON_WindowTimeIntersects( Word* args, Word& result,
                              int message, Word& local,
                              Supplier s )
{
  MON_WindowTimeIntersectsLocalInfo *localInfo;

  switch (message)
  {
    case OPEN :
    {
      localInfo = new MON_WindowTimeIntersectsLocalInfo;
      localInfo->montree = (MON_Tree<SmiRecordId>*)args[1].addr;
      localInfo->montree->SetNetwork( (network::Network*)args[0].addr );
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
      localInfo = (MON_WindowTimeIntersectsLocalInfo*)local.addr;
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
            Tuple *tuple = localInfo->relation->GetTuple(e.info, false);
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
            Tuple *tuple = localInfo->relation->GetTuple(e.info, false);
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
        localInfo = (MON_WindowTimeIntersectsLocalInfo*)local.addr;
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
         MON_WindowTimeIntersects,
         Operator::SimpleSelect,    // trivial selection function
         MON_WindowTimeIntersectsTypeMap    // type mapping
);

/*
7.2 Operator ~windowtimeintersectsS~

7.2.1 Type mapping function of operator ~windowtimeintersectsS~

*/
ListExpr MON_WindowTimeIntersectsSTypeMap(ListExpr args)
{
  string errmsg = "Incorrect input for operator windowtimeintersectsS.";
  string monDescriptionStr, relDescriptionStr;

  if(nl->ListLength(args) != 6){
    return listutils::typeError(errmsg);
  }

  /* Split argument in three parts */
  ListExpr networkDescription = nl->First(args),
           monDescription = nl->Second(args),
           relDescription = nl->Third(args),
           searchWindow = nl->Fourth(args),
           searchStart = nl->Fifth(args),
           searchEnd = nl->Sixth(args);

  if(!nl->IsEqual(networkDescription, "network")){
    return listutils::typeError(
    "Operator windowtimeintersectsS expects the first argument\n"
    "to be of type network.");
  }

  /* Query window: find out type of key */
  if(!nl->IsEqual(searchWindow, Rectangle<2>::BasicType())){
    return listutils::typeError(
    "Operator windowtimeintersectsS expects that the search window\n"
    "is of type rect.");
  }

  // Handling of the time interval
  if(!nl->IsEqual(searchStart, Instant::BasicType()) ||
    !nl->IsEqual(searchEnd, Instant::BasicType())){
    return listutils::typeError(
    "Operator windowtimeintersectsS expects the fifth and sixth\n"
    "arguments to be a time interval.");
  }

  /* handle montree part of argument */
  nl->WriteToString (monDescriptionStr, monDescription);
  if( nl->ListLength(monDescription) != 4 ){
    return listutils::typeError(
    "Operator windowtimeintersectsS expects a MON-Tree with structure "
    "(montree (tuple ((a1 t1)...(an tn))) attrtype\n"
    "\nbut gets the following '"
    +monDescriptionStr+"'.");
  }

  ListExpr monSymbol = nl->First(monDescription);

  /* handle rtree type constructor */
  if(!nl->IsEqual(monSymbol, "montree")){
    return listutils::typeError(
   "Operator windowtimeintersectsS expects a MON-Tree \n"
   "as second argument.");
  }

  /* handle rel part of argument */
  nl->WriteToString (relDescriptionStr, relDescription);
  if(!IsRelDescription(relDescription)){
    return listutils::typeError(
    "Operator windowtimeintersectsS expects a "
    "relation as its third argument, but gets '"
        + relDescriptionStr + "'.");
  }

  ListExpr isDouble = nl->BoolValue(nl->Fourth(monDescription));
  if(!isDouble)
  {
    return
      nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->OneElemList(
            nl->TwoElemList(
              nl->SymbolAtom("id"),
              nl->SymbolAtom(TupleIdentifier::BasicType())))));
  }
  else
  {
    return
      nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->ThreeElemList(
            nl->TwoElemList(
              nl->SymbolAtom("id"),
              nl->SymbolAtom(TupleIdentifier::BasicType())),
            nl->TwoElemList(
              nl->SymbolAtom("low"),
              nl->SymbolAtom(CcInt::BasicType())),
            nl->TwoElemList(
              nl->SymbolAtom("high"),
              nl->SymbolAtom(CcInt::BasicType())))));
  }
}

/*
5.1.3 Value mapping function of operator ~windowintersectsS~

*/
template<class BottomR_TreeLeafInfo>
struct MON_WindowTimeIntersectsSLocalInfo
{
  Relation* relation;
  MON_Tree<BottomR_TreeLeafInfo>* montree;
  BBox<2> searchBox;
  Interval<Instant> searchTimeInterval;
  bool first;
  TupleType *resultTupleType;
};

int MON_WindowTimeIntersectsS( Word* args, Word& result,
                               int message, Word& local,
                               Supplier s )
{
  MON_WindowTimeIntersectsSLocalInfo<SmiRecordId> *localInfo;

  switch (message)
  {
    case OPEN :
    {
      localInfo = new MON_WindowTimeIntersectsSLocalInfo<SmiRecordId>;
      localInfo->montree = (MON_Tree<SmiRecordId>*)args[1].addr;
      localInfo->montree->SetNetwork( (network::Network*)args[0].addr );
      localInfo->relation = (Relation*)args[2].addr;
      localInfo->first = true;

      localInfo->searchBox = *(BBox<2>*)args[3].addr;
      localInfo->searchTimeInterval =
        Interval<Instant>( *(Instant*)args[4].addr,
                           *(Instant*)args[5].addr,
                           true, true );
      localInfo->resultTupleType =
        new TupleType(nl->Second(GetTupleResultType(s)));

      assert(localInfo->montree != 0);
      assert(localInfo->relation != 0);
      local.setAddr(localInfo);
      return 0;
    }

    case REQUEST :
    {
      localInfo = (MON_WindowTimeIntersectsSLocalInfo<SmiRecordId>*)local.addr;
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
            Tuple *tuple = new Tuple( localInfo->resultTupleType );
            tuple->PutAttribute(0, new TupleIdentifier(true, e.info));
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
            Tuple *tuple = new Tuple( localInfo->resultTupleType );
            tuple->PutAttribute(0, new TupleIdentifier(true, e.info));
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
        localInfo =
          (MON_WindowTimeIntersectsSLocalInfo<SmiRecordId>*)local.addr;
        delete localInfo;
        local.setAddr(Address(0));
      }
      return 0;
    }
  }
  return 0;
}

int MON_WindowTimeIntersectsSDbl( Word* args, Word& result,
                                  int message, Word& local,
                                  Supplier s )
{
  MON_WindowTimeIntersectsSLocalInfo<BottomR_TreeLeafInfo> *localInfo;

  switch (message)
  {
    case OPEN :
    {
      localInfo = new MON_WindowTimeIntersectsSLocalInfo<BottomR_TreeLeafInfo>;
      localInfo->montree = (MON_Tree<BottomR_TreeLeafInfo>*)args[1].addr;
      localInfo->montree->SetNetwork( (network::Network*)args[0].addr );
      localInfo->relation = (Relation*)args[2].addr;
      localInfo->first = true;

      localInfo->searchBox = *(BBox<2>*)args[3].addr;
      localInfo->searchTimeInterval =
        Interval<Instant>( *(Instant*)args[4].addr,
                           *(Instant*)args[5].addr,
                           true, true );
      localInfo->resultTupleType =
        new TupleType(nl->Second(GetTupleResultType(s)));

      assert(localInfo->montree != 0);
      assert(localInfo->relation != 0);
      local.setAddr(localInfo);
      return 0;
    }

    case REQUEST :
    {
      localInfo =
        (MON_WindowTimeIntersectsSLocalInfo<BottomR_TreeLeafInfo>*)local.addr;
       R_TreeLeafEntry<2, BottomR_TreeLeafInfo> e;

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
            Tuple *tuple = new Tuple( localInfo->resultTupleType );
            tuple->PutAttribute(
              0, new TupleIdentifier( true, e.info.tupleId ) );
            tuple->PutAttribute( 1, new CcInt( true, e.info.low ) );
            tuple->PutAttribute( 2, new CcInt( true, e.info.high ) );
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
            Tuple *tuple = new Tuple( localInfo->resultTupleType );
            tuple->PutAttribute(
              0, new TupleIdentifier( true, e.info.tupleId ) );
            tuple->PutAttribute( 1, new CcInt( true, e.info.low ) );
            tuple->PutAttribute( 2, new CcInt( true, e.info.high ) );
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
        localInfo =
          (MON_WindowTimeIntersectsSLocalInfo<BottomR_TreeLeafInfo>*)local.addr;
        if(localInfo->resultTupleType){
           localInfo->resultTupleType->DeleteIfAllowed();
        }
        delete localInfo;
        local.setAddr(Address(0));
      }
      return 0;
    }
  }
  return 0;
}

/*
5.1.2 Selection function of operator ~windowintersectsS~

*/
int
WindowTimeIntersectsSSelection( ListExpr args )
{
  if(nl->BoolValue(nl->Fourth(nl->Second(args))))
    return 1;
  return 0;
}

/*
5.1.5 Specification of operator ~windowintersectsS~

*/
const string windowtimeintersectsSSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>montree(tuple ((x1 t1)...(xn tn))"
      " ti) x rel(tuple ((x1 t1)...(xn tn))) "
      "x rect x instant x instant ->\n"
      " (stream (tuple ((id tid))))\n</text--->"
      "<text>_ _ windowintersectsS [ _, _, _ ]</text--->"
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
4.1.5 Definition of value mapping vectors

*/
ValueMapping windowtimeintersectss [] = { MON_WindowTimeIntersectsS,
                                          MON_WindowTimeIntersectsSDbl };

/*
5.1.6 Definition of operator ~windowtimeintersectsS~

*/
Operator windowtimeintersectsS (
         "windowtimeintersectsS",
         windowtimeintersectsSSpec,
         3,
         windowtimeintersectss,
         WindowTimeIntersectsSSelection,
         MON_WindowTimeIntersectsSTypeMap
);

/*
6 Definition and initialization of MON-Tree Algebra

*/
class MONTreeAlgebra : public Algebra
{
 public:
  MONTreeAlgebra() : Algebra()
  {
    AddTypeConstructor( &montree );

    AddOperator( &createmontree );
    AddOperator( &windowtimeintersects );
    AddOperator( &windowtimeintersectsS );
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



