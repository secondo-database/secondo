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

[1] Implementation of the MTree Algebra

November 2007 Mirko Dibbert

[TOC]

1 Overview

TODO: enter algebra discription

2 Defines and includes

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "MTree.h"
#include "MetricAttribute.h"
#include "MetricRegistry.h"

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;
extern SecondoInterface *si;

/*
1 Type Constructors

*/
static ListExpr
MTreeProp()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"<relation> createmtree [<attrname>] where "
  "<attrname> is the key");

  return (nl->TwoElemList(
            nl->TwoElemList(nl->StringAtom("Creation"),
           nl->StringAtom("Example Creation")),
            nl->TwoElemList(examplelist,
           nl->StringAtom("(let mymtree = ten "
           "createmtree "))));
}

ListExpr OutMTree(ListExpr typeInfo, Word  value)
{
  return nl->TheEmptyList();
}

Word InMTree(ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr &errorInfo, bool &correct)
{
  correct = false;
  return SetWord(0);
}

ListExpr SaveToListMTree(ListExpr typeInfo, Word  value)
{
  return nl->IntAtom( 0 );
}

Word RestoreFromListMTree( ListExpr typeInfo, ListExpr value,
                           int errorPos, ListExpr &errorInfo, bool &correct)
{
  return SetWord( Address( 0 ) );
}

Word CreateMTree(const ListExpr typeInfo)
{
  MetricRegistry::Initialize( si );
  return SetWord( new MTree() );
}

void DeleteMTree(const ListExpr typeInfo, Word &w)
{
    MTree *mtree = (MTree*)w.addr;
    mtree->DeleteFile();
    delete mtree;
}

bool
OpenMTree( SmiRecord &valueRecord,
           size_t &offset,
           const ListExpr typeInfo,
           Word &value )
{
  SmiFileId fileid;
  valueRecord.Read( &fileid, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );

  MTree* mtree = new MTree( fileid );
  value = SetWord( mtree );
  return true;
}

bool
SaveMTree( SmiRecord &valueRecord,
           size_t &offset,
           const ListExpr typeInfo,
           Word &value )
{
  SmiFileId fileId;
  MTree *mtree = (MTree*)value.addr;
  fileId = mtree->GetFileId();
  valueRecord.Write( &fileId, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );
  return true;
}

void CloseMTree(const ListExpr typeInfo, Word &w)
{
    MTree *mtree = (MTree*)w.addr;
    delete mtree;
}

Word CloneMTree(const ListExpr typeInfo, const Word &w)
{
  return SetWord( 0 );
}

void *CastMTree(void *addr)
{
  return ( 0 );
}

int SizeOfMTree()
{
  return 0;
}

bool CheckMTree(ListExpr type, ListExpr &errorInfo)
{
  //TODO not yet implemented
  return false;
}

TypeConstructor 
mtree( "mtree",          MTreeProp,
        OutMTree,        InMTree,
        SaveToListMTree, RestoreFromListMTree,
        CreateMTree,     DeleteMTree,
        OpenMTree,       SaveMTree,
        CloseMTree,      CloneMTree,
        CastMTree,       SizeOfMTree,
        CheckMTree );

/*
1 Operators

1.1 Operator ~createmtree~

*/
const string CreateMTreeSpec = "(( \"...\" )( \"...\" ))";

int CreateMTreeValueMapping_Rel( Word  *args, Word  &result,
                    int message, Word  &local, Supplier s )
{
  Relation *relation;
  int attrIndex;
  GenericRelationIterator *iter;
  Tuple *tuple;

  MTree *mtree = (MTree*)qp->ResultStorage(s).addr;
  result = SetWord( mtree );

  relation = (Relation*)args[0].addr;
  attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;

  iter = relation->MakeScan();
  while( (tuple = iter->GetNextTuple()) != 0 )
  {
    if( ((StandardAttribute*)(tuple->
          GetAttribute(attrIndex)))->IsDefined() )
    {
      if (!mtree->IsInitialized())
      {
         mtree->Initialize(relation, tuple, attrIndex, 
                           RANDOM, GENERALIZED_HYPERPLANE);
      }
      mtree->Insert( tuple->GetTupleId() );
    }
    tuple->DeleteIfAllowed();
  }
  delete iter;

  return 0;
}

int CreateMTreeValueMapping_Stream( Word  *args, Word  &result,
                    int message, Word  &local, Supplier s )
{
  return 0;
}

ListExpr CreateMTreeTypeMapping( ListExpr args )
{
 string argstr;

  // check argument count
  CHECK_COND(nl->ListLength(args) == 2,
    "Operator createmtree expects two arguments.");

  // check, if first argument is a relation or a stream tuples
  ListExpr first = nl->First(args);
  nl->WriteToString(argstr, first);
  CHECK_COND( ( !nl->IsAtom(first) &&
                (
                  (
                    nl->IsEqual(nl->First(first), "rel") &&
                    IsRelDescription(first)
                  ) ||
                  (
                    nl->IsEqual(nl->First(first), "stream") &&
                    nl->ListLength(first) == 2 &&
                    !nl->IsAtom(nl->Second(first)) &&
                    nl->ListLength(nl->Second(first)) == 2 &&
                    nl->IsEqual(nl->First(nl->Second(first)), "tuple") &&
                    IsTupleDescription(nl->Second(nl->Second(first)))
                  )
                )
              ),
    "Operator createmtree expects as first argument a list with structure\n"
    "rel(tuple ((a1 t1)...(an tn))) or stream (tuple ((a1 t1)...(an tn)))\n"
    "Operator createmtree gets a list with structure '" + argstr + "'."); 


  // check, if secend argument is an attribute name
  ListExpr second = nl->Second(args);
  nl->WriteToString(argstr, second);
  CHECK_COND( ( nl->IsAtom(second) &&nl->AtomType(second) == SymbolType ),
    "Operator createbtree expects as second argument an attribute name\n"
    "bug gets '" + argstr + "'.");

  string attrName = nl->SymbolValue(second);
  ListExpr tupleDescription = nl->Second(first),
           attrList = nl->Second(tupleDescription);

  // check, if tuple name can be found in attribute list
  nl->WriteToString(argstr, attrList);
  int attrIndex;
  ListExpr attrType;
  CHECK_COND((attrIndex = FindAttribute(attrList, attrName, attrType)) > 0,
    "Operator createmtree expects as second argument an attribute name\n"
    "Attribute name '" + attrName + "' is not known.\n"
    "Known Attribute(s): " + argstr);

  // check, if attribute is of METRICAL type
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  nl->WriteToString(argstr, attrType);
  CHECK_COND(am->CheckKind("METRICAL", attrType, errorInfo), 
    "Operator createmtree expects as second argument an attribute of type\n"
    "METRICAL, but gets '" + argstr + "'.");

  // create type
  if( nl->IsEqual(nl->First(first), "rel") )
  {
    return 
      nl->ThreeElemList (
        nl->SymbolAtom("APPEND"),
        nl->OneElemList( nl->IntAtom(attrIndex)),
        nl->ThreeElemList (
          nl->SymbolAtom("mtree"),
          tupleDescription,
          attrType )
        );
  }
  else
  { // streams are not implemented yet
    return nl->SymbolAtom( "typeerror" );
  }
}

int CreateMTreeSelect( ListExpr args )
{
  if( nl->IsEqual(nl->First(nl->First(args)), "rel") )
    return 0;
  if( nl->IsEqual(nl->First(nl->First(args)), "stream") )
    return 1; 
  return -1;
}

ValueMapping CreateMTreeMap[] = { CreateMTreeValueMapping_Rel,
                                  CreateMTreeValueMapping_Stream };

Operator createmtree (
  "createmtree",
  CreateMTreeSpec,
  2,
  CreateMTreeMap,
  CreateMTreeSelect,
  CreateMTreeTypeMapping
);

/*
1 Create and initialize the Algebra

*/
class MTreeAlgebra : public Algebra
{
public:
  MTreeAlgebra() : Algebra()
  {
    AddTypeConstructor( &mtree );

    AddOperator( &createmtree );
  }

  ~MTreeAlgebra() {};
};

MTreeAlgebra mtreeAlgebra;

extern "C"
  Algebra*
  InitializeMTreeAlgebra( NestedList  *nlRef, QueryProcessor  *qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return ( &mtreeAlgebra );
}
