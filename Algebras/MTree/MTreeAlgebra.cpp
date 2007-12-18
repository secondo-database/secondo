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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

5.1 Implementation of the MTreeAlgebra (file: MTreeAlgebra.cpp)

November/December 2007, Mirko Dibbert

5.1.1 Imcludes and Defines

*/
using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "TupleIdentifier.h"
#include "MTree.h"
#include "MTreeConfig.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

/*
5.1.2 Type Constructors

5.1.2.1 Type Constructor ~MTree~

*/
static ListExpr
MTreeProp()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText( examplelist, "<relation> createmtree [<attrname>] "
          "where <attrname> is the key" );

  return ( nl->TwoElemList(
         nl->TwoElemList( nl->StringAtom( "Creation" ),
                  nl->StringAtom( "Example Creation" ) ),
         nl->TwoElemList( examplelist,
                  nl->StringAtom( "(let mymtree = ten "
                          "createmtree " ) ) ) );
}

ListExpr
OutMTree( ListExpr typeInfo, Word  value )
{
  MT::MTree* mtree = static_cast<MT::MTree*>(value.addr);

  return nl->FiveElemList(
    nl->StringAtom( "M-Tree statistics" ),
        nl->TwoElemList( nl->StringAtom( "Height" ),
                         nl->IntAtom( mtree->getHeight() ) ),
        nl->TwoElemList( nl->StringAtom( "# of routing nodes" ),
                         nl->IntAtom( mtree->getRoutingCount() ) ),
        nl->TwoElemList( nl->StringAtom( "# of leafes" ),
                         nl->IntAtom( mtree->getLeafCount() ) ),
        nl->TwoElemList( nl->StringAtom( "# of (leaf) entries" ),
                         nl->IntAtom( mtree->getEntryCount() ) ) );
}

Word
InMTree( ListExpr typeInfo, ListExpr value,
        int errorPos, ListExpr &errorInfo, bool &correct )
{
  correct = false;
  return SetWord( 0 );
}

ListExpr
SaveToListMTree( ListExpr typeInfo, Word  value )
{
  return nl->IntAtom( 0 );
}

Word
RestoreFromListMTree( ListExpr typeInfo, ListExpr value,
               int errorPos, ListExpr &errorInfo,
               bool &correct )
{
  return SetWord( Address( 0 ) );
}

Word
createMTree( const ListExpr typeInfo )
{
  return SetWord( new MT::MTree() );
}

void
DeleteMTree( const ListExpr typeInfo, Word& w )
{
  static_cast<MT::MTree*>(w.addr)->deleteFile();
  delete static_cast<MT::MTree*>(w.addr);
  w.addr = 0;
}

bool
OpenMTree( SmiRecord &valueRecord,
       size_t &offset,
       const ListExpr typeInfo,
       Word& value )
{
  SmiFileId fileid;
  valueRecord.Read( &fileid, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );

  MT::MTree* mtree = new MT::MTree( fileid );
  value = SetWord( mtree );
  return true;
}

bool
SaveMTree( SmiRecord &valueRecord,
       size_t &offset,
       const ListExpr typeInfo,
       Word& value )
{
  SmiFileId fileId;
  MT::MTree *mtree = ( MT::MTree* )value.addr;
  fileId = mtree->getFileId();
  if (fileId)
  {
    valueRecord.Write( &fileId, sizeof( SmiFileId ), offset );
    offset += sizeof( SmiFileId );
    return true;
  }
  else
  {
    return false;
  }
}

void CloseMTree( const ListExpr typeInfo, Word& w )
{
  MT::MTree *mtree = ( MT::MTree* )w.addr;
  delete mtree;
}

Word CloneMTree( const ListExpr typeInfo, const Word& w )
{
  MT::MTree* res = new MT::MTree(*static_cast<MT::MTree*>(w.addr));
  return SetWord( res );
}

void *CastMTree( void *addr )
{
  return ( 0 );
}

int SizeOfMTree()
{
  return 0;
}

bool CheckMTree( ListExpr type, ListExpr &errorInfo )
{
  //TODO not yet implemented
  return true;
}

TypeConstructor
mtree( "mtree",      MTreeProp,
     OutMTree,    InMTree,
     SaveToListMTree, RestoreFromListMTree,
     createMTree,   DeleteMTree,
     OpenMTree,     SaveMTree,
     CloseMTree,    CloneMTree,
     CastMTree,     SizeOfMTree,
     CheckMTree );

/********************************************************************
5.1.3 Operators

5.1.3.1 Operator ~createmtree~

********************************************************************/
int createMTreeValueMapping_Rel(
    Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MT::MTree *mtree = ( MT::MTree* )result.addr;

  Relation* relation =
      static_cast<Relation*>(args[0].addr);

  int attrIndex =
      static_cast<CcInt*>(args[2].addr)->GetIntval();

  string type =
      static_cast<CcString*>(args[3].addr)->GetValue();

  string mfName =
      static_cast<CcString*>(args[4].addr)->GetValue();

  string configName =
      static_cast<CcString*>(args[5].addr)->GetValue();

  mtree->initialize( type, mfName, configName );

  #ifdef __MT_PRINT_CONFIG_INFO
  mtree->printMTreeConfig();
  #endif

  Tuple* tuple;
  GenericRelationIterator* iter = relation->MakeScan();
  while (( tuple = iter->GetNextTuple() ) != 0 )
  {
    Attribute* attr = tuple->GetAttribute( attrIndex );
    if( attr->IsDefined() )
    {
      mtree->insert( attr, tuple->GetTupleId() );
    }
    tuple->DeleteIfAllowed();
  }
  delete iter;

  mtree->finalizeInsert();
  return 0;
}

int createMTreeValueMapping_Stream(
    Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MT::MTree *mtree = ( MT::MTree* )result.addr;

  void* stream =
      static_cast<Relation*>(args[0].addr);

  int attrIndex =
      static_cast<CcInt*>(args[2].addr)->GetIntval();

  string type =
      static_cast<CcString*>(args[3].addr)->GetValue();

  string mfName =
      static_cast<CcString*>(args[4].addr)->GetValue();

  string configName =
      static_cast<CcString*>(args[5].addr)->GetValue();

  mtree->initialize( type, mfName, configName );

  #ifdef __MT_PRINT_CONFIG_INFO
  mtree->printMTreeConfig();
  #endif

  Word wTuple;
  qp->Open( stream );
  qp->Request( stream, wTuple );
  while ( qp->Received( stream ) )
  {
    Tuple* tuple = static_cast<Tuple*>(wTuple.addr);
    Attribute* attr = tuple->GetAttribute( attrIndex );
    if( attr->IsDefined() )
    {
      mtree->insert( attr, tuple->GetTupleId() );
    }
    tuple->DeleteIfAllowed();
    qp->Request( stream, wTuple );
  }
  qp->Close( stream );

  mtree->finalizeInsert();
  return 0;
}

int createMTreeSelect( ListExpr args )
{
  if ( nl->IsEqual( nl->First( nl->First( args ) ), "rel" ) )
    return 0;

  if ( nl->IsEqual( nl->First( nl->First( args ) ), "stream" ) )
    return 1;

  return -1;
}

ValueMapping createMTreeMap[] = {
    createMTreeValueMapping_Rel,
    createMTreeValueMapping_Stream
};

ListExpr createMTreeTypeMapping( ListExpr args )
{
  string errmsg;
  bool cond;
  NList nl_args( args );

  errmsg = "Operator createmtree expects two arguments.";
  CHECK_COND( nl_args.length() == 2, errmsg );

  NList arg1 = nl_args.first();
  NList arg2 = nl_args.second();

  // check first argument (should be relation or stream)
  cond = !(arg1.isAtom()) &&
         (
           ( arg1.first().isEqual( "rel" ) &&
             IsRelDescription( arg1.listExpr() )) ||
           ( arg1.first().isEqual( "stream" ) &&
             IsStreamDescription( arg1.listExpr() ))
         );
  errmsg = "Operator createmtree expects a list with structure\n"
           "   rel (tuple ((a1 t1)...(an tn))) or\n"
           "   stream (tuple ((a1 t1)...(an tn)))\n"
           "as first argument, but got a list with structure '" +
       arg1.convertToString() + "'.";
  CHECK_COND( cond , errmsg);

  // check, if fourth argument is an attribute name
  errmsg = "Operator createmtree expects an attribute name "
           "as fourth argument, but got '" +
           arg2.convertToString() + "'.";
  CHECK_COND( arg2.isSymbol(), errmsg);

  string attrName = arg2.str();
  NList tupleDescription = arg1.second();
  NList attrList = tupleDescription.second();

  // check, if attribute can be found in attribute list
  errmsg = "Attribute name '" + attrName + "' is not known.\n"
           "Known Attribute(s):\n" + attrList.convertToString();
  ListExpr attrTypeLE;

  int attrIndex = FindAttribute( attrList.listExpr(),
                                 attrName, attrTypeLE );
  CHECK_COND( attrIndex > 0, errmsg );
  NList attrType ( attrTypeLE );
  string attrTypeStr = attrType.convertToString();

  // check if default metric exists
  string mfName = MetricRegistry::getDefaultName( attrTypeStr );
  errmsg = "Missing default metric for type constructor " +
           attrTypeStr + "!";
  CHECK_COND( mfName != "unknown", errmsg);

  // check if default mtree-config object exists
  errmsg = "Operator createmtree expects the name of a registered"
           "mtree-config object as third argument, but got a list "
           "with structure '" + arg2.convertToString() + "'.";
  string configName = "default";
  errmsg = "Missing default mtreeconfig!";
  cond = MT::MTreeConfigReg::contains( configName );
  CHECK_COND( cond, errmsg);

  NList result (
      NList( "APPEND" ),
      NList(
        attrIndex - 1,
        NList ( attrTypeStr, true ),
        NList ( mfName, true ),
        NList ( configName, true ) ),
      NList( NList( "mtree" ), tupleDescription, attrType ) );
  return result.listExpr();
}

struct createMTreeInfo : OperatorInfo
{
  createMTreeInfo()
  {
    name = "createmtree";
    signature =
      "( <text>(rel (tuple ((id tid) (x1 t1)...(xn tn)))"
      " metricName, xi) ->"
      " (mtree (tuple ((x1 t1)...(xn tn))) ti)";
    syntax = "_ createmtree [_]";
    meaning =
        "creates a new mtree from relation or stream in arg1\n"
        "arg2 must be the name of the attribute in arg1, "
        "which should be indexed by the mtree";
    example = "pictures createmtree [Pic]";
  }
};

/********************************************************************
5.1.3.2 Operator ~createmtree2~

********************************************************************/
int createMTree2ValueMapping_Rel(
    Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MT::MTree *mtree = ( MT::MTree* )result.addr;

  Relation* relation =
      static_cast<Relation*>(args[0].addr);

  int attrIndex =
      static_cast<CcInt*>(args[3].addr)->GetIntval();

  string type =
      static_cast<CcString*>(args[4].addr)->GetValue();

  string mfName =
      static_cast<CcString*>(args[5].addr)->GetValue();

  string configName =
      static_cast<CcString*>(args[6].addr)->GetValue();

  mtree->initialize( type, mfName, configName );

  #ifdef __MT_PRINT_CONFIG_INFO
  mtree->printMTreeConfig();
  #endif

  Tuple* tuple;
  GenericRelationIterator* iter = relation->MakeScan();
  while (( tuple = iter->GetNextTuple() ) != 0 )
  {
    Attribute* attr = tuple->GetAttribute( attrIndex );
    if( attr->IsDefined() )
    {
      mtree->insert( attr, tuple->GetTupleId() );
    }
    tuple->DeleteIfAllowed();
  }
  delete iter;

  mtree->finalizeInsert();
  return 0;
}

int createMTree2ValueMapping_Stream(
    Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MT::MTree *mtree = ( MT::MTree* )result.addr;

  void* stream =
      static_cast<Relation*>(args[0].addr);

  int attrIndex =
      static_cast<CcInt*>(args[3].addr)->GetIntval();

  string type =
      static_cast<CcString*>(args[4].addr)->GetValue();

  string mfName =
      static_cast<CcString*>(args[5].addr)->GetValue();

  string configName =
      static_cast<CcString*>(args[6].addr)->GetValue();

  mtree->initialize( type, mfName, configName );

  #ifdef __MT_PRINT_CONFIG_INFO
  mtree->printMTreeConfig();
  #endif

  Word wTuple;
  qp->Open( stream );
  qp->Request( stream, wTuple );
  while ( qp->Received( stream ) )
  {
    Tuple* tuple = static_cast<Tuple*>(wTuple.addr);
    Attribute* attr = tuple->GetAttribute( attrIndex );
    if( attr->IsDefined() )
    {
      mtree->insert( attr, tuple->GetTupleId() );
    }
    tuple->DeleteIfAllowed();
    qp->Request( stream, wTuple );
  }
  qp->Close( stream );

  mtree->finalizeInsert();
  return 0;
}

int createMTree2Select( ListExpr args )
{
  if ( nl->IsEqual( nl->First( nl->First( args ) ), "rel" ) )
    return 0;

  if ( nl->IsEqual( nl->First( nl->First( args ) ), "stream" ) )
    return 1;

  return -1;
}

ValueMapping createMTree2Map[] = {
    createMTree2ValueMapping_Rel,
    createMTree2ValueMapping_Stream
};

ListExpr createMTree2TypeMapping( ListExpr args )
{
  string errmsg;
  bool cond;
  NList nl_args( args );

  errmsg = "Operator createmtree2 expects three arguments.";
  CHECK_COND( nl_args.length() == 3, errmsg );

  NList arg1 = nl_args.first();
  NList arg2 = nl_args.second();
  NList arg3 = nl_args.third();

  // check first argument (should be relation or stream)
  cond = !(arg1.isAtom()) &&
         (
           ( arg1.first().isEqual( "rel" ) &&
             IsRelDescription( arg1.listExpr() )) ||
           ( arg1.first().isEqual( "stream" ) &&
             IsStreamDescription( arg1.listExpr() ))
         );
  errmsg = "Operator createmtree2 expects a list with structure\n"
           "   rel (tuple ((a1 t1)...(an tn))) or\n"
           "   stream (tuple ((a1 t1)...(an tn)))\n"
           "as first argument, but got a list with structure '" +
       arg1.convertToString() + "'.";
  CHECK_COND( cond , errmsg);

  // check, if fourth argument is an attribute name
  errmsg = "Operator createmtree2 expects an attribute name "
           "as fourth argument, but got '" +
           arg3.convertToString() + "'.";
  CHECK_COND( arg3.isSymbol(), errmsg);

  string attrName = arg3.str();
  NList tupleDescription = arg1.second();
  NList attrList = tupleDescription.second();

  // check, if attribute can be found in attribute list
  errmsg = "Attribute name '" + attrName + "' is not known.\n"
           "Known Attribute(s):\n" + attrList.convertToString();
  ListExpr attrTypeLE;
  int attrIndex = FindAttribute( attrList.listExpr(),
                                 attrName, attrTypeLE );
  CHECK_COND( attrIndex > 0, errmsg );
  NList attrType ( attrTypeLE );
  string attrTypeStr = attrType.convertToString();

  // check if the metric given in second argument is defined
  errmsg = "Operator createmtree2 expects the name of a registered"
           "metric as second argument, but got a list with structure"
           " '" + arg2.convertToString() + "'.";
  CHECK_COND( arg2.isSymbol(), errmsg);
  string mfName = arg2.str();
  if (mfName == "default")
  {
    mfName = MetricRegistry::getDefaultName( attrTypeStr );
    errmsg = "Missing default metric for type constructor " +
             attrTypeStr + "!";
    CHECK_COND( mfName != "unknown", errmsg);
  }

  errmsg = "Metric " + mfName + " for type constructor " +
           attrTypeStr + " not defined!";
  cond = MetricRegistry::getMetric( attrTypeStr, mfName ) != 0;
  CHECK_COND( cond, errmsg);

  // check if default mtree-config object exists
  errmsg = "Operator createmtree2 expects the name of a registered"
           "mtree-config object as third argument, but got a list "
           "with structure '" + arg2.convertToString() + "'.";
  string configName = "default";
  errmsg = "Missing default mtreeconfig!";
  cond = MT::MTreeConfigReg::contains( configName );
  CHECK_COND( cond, errmsg);

  NList result (
      NList( "APPEND" ),
      NList(
        attrIndex - 1,
        NList ( attrTypeStr, true ),
        NList ( mfName, true ),
        NList ( configName, true ) ),
      NList( NList( "mtree" ), tupleDescription, attrType ) );
  return result.listExpr();
}

struct createMTree2Info : OperatorInfo
{
  createMTree2Info()
  {
    name = "createmtree2";
    signature =
      "( <text>(rel (tuple ((id tid) (x1 t1)...(xn tn)))"
      " metricName, xi) ->"
      " (mtree (tuple ((x1 t1)...(xn tn))) ti)";
    syntax = "_ createmtree2 [_, _]";
    meaning =
        "creates a new mtree from relation or stream in arg1\n"
        "arg2 must be the name of a registered metric\n"
        "arg3 must be the name of the attribute in arg1, "
        "which should be indexed by the mtree";
    example = "pictures createmtree2 [Pic, lab]";
  }
};

/********************************************************************
5.1.3.3 Operator ~createmtree3~

********************************************************************/
int createMTree3ValueMapping_Rel(
    Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MT::MTree *mtree = ( MT::MTree* )result.addr;

  Relation* relation =
      static_cast<Relation*>(args[0].addr);

  int attrIndex =
      static_cast<CcInt*>(args[4].addr)->GetIntval();

  string type =
      static_cast<CcString*>(args[5].addr)->GetValue();

  string mfName =
      static_cast<CcString*>(args[6].addr)->GetValue();

  string configName =
      static_cast<CcString*>(args[7].addr)->GetValue();

  mtree->initialize( type, mfName, configName );

  #ifdef __MT_PRINT_CONFIG_INFO
  mtree->printMTreeConfig();
  #endif

  Tuple* tuple;
  GenericRelationIterator* iter = relation->MakeScan();
  while (( tuple = iter->GetNextTuple() ) != 0 )
  {
    Attribute* attr = tuple->GetAttribute( attrIndex );
    if( attr->IsDefined() )
    {
      mtree->insert( attr, tuple->GetTupleId() );
    }
    tuple->DeleteIfAllowed();
  }
  delete iter;

  mtree->finalizeInsert();
  return 0;
}

int createMTree3ValueMapping_Stream(
    Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MT::MTree *mtree = ( MT::MTree* )result.addr;

  void* stream =
      static_cast<Relation*>(args[0].addr);

  int attrIndex =
      static_cast<CcInt*>(args[4].addr)->GetIntval();

  string type =
      static_cast<CcString*>(args[5].addr)->GetValue();

  string mfName =
      static_cast<CcString*>(args[6].addr)->GetValue();

  string configName =
      static_cast<CcString*>(args[7].addr)->GetValue();

  mtree->initialize( type, mfName, configName );

  #ifdef __MT_PRINT_CONFIG_INFO
  mtree->printMTreeConfig();
  #endif

  Word wTuple;
  qp->Open( stream );
  qp->Request( stream, wTuple );
  while ( qp->Received( stream ) )
  {
    Tuple* tuple = static_cast<Tuple*>(wTuple.addr);
    Attribute* attr = tuple->GetAttribute( attrIndex );
    if( attr->IsDefined() )
    {
      mtree->insert( attr, tuple->GetTupleId() );
    }
    tuple->DeleteIfAllowed();
    qp->Request( stream, wTuple );
  }
  qp->Close( stream );

  mtree->finalizeInsert();
  return 0;
}

int createMTree3Select( ListExpr args )
{
  if ( nl->IsEqual( nl->First( nl->First( args ) ), "rel" ) )
    return 0;

  if ( nl->IsEqual( nl->First( nl->First( args ) ), "stream" ) )
    return 1;

  return -1;
}

ValueMapping createMTree3Map[] = {
    createMTree3ValueMapping_Rel,
    createMTree3ValueMapping_Stream
};

ListExpr createMTree3TypeMapping( ListExpr args )
{
  string errmsg;
  bool cond;
  NList nl_args( args );

  errmsg = "Operator createmtree3 expects four arguments.";
  CHECK_COND( nl_args.length() == 4, errmsg );

  NList arg1 = nl_args.first();
  NList arg2 = nl_args.second();
  NList arg3 = nl_args.third();
  NList arg4 = nl_args.fourth();

  // check first argument (should be relation or stream)
  cond = !(arg1.isAtom()) &&
         (
           ( arg1.first().isEqual( "rel" ) &&
             IsRelDescription( arg1.listExpr() )) ||
           ( arg1.first().isEqual( "stream" ) &&
             IsStreamDescription( arg1.listExpr() ))
         );
  errmsg = "Operator createmtree3 expects a list with structure\n"
           "   rel (tuple ((a1 t1)...(an tn))) or\n"
           "   stream (tuple ((a1 t1)...(an tn)))\n"
           "as first argument, but got a list with structure '" +
       arg1.convertToString() + "'.";
  CHECK_COND( cond , errmsg);

  // check, if fourth argument is an attribute name
  errmsg = "Operator createmtree3 expects an attribute name "
           "as fourth argument, but got '" +
           arg4.convertToString() + "'.";
  CHECK_COND( arg4.isSymbol(), errmsg);

  string attrName = arg4.str();
  NList tupleDescription = arg1.second();
  NList attrList = tupleDescription.second();

  // check, if attribute can be found in attribute list
  errmsg = "Attribute name '" + attrName + "' is not known.\n"
           "Known Attribute(s):\n" + attrList.convertToString();
  ListExpr attrTypeLE;
  int attrIndex = FindAttribute( attrList.listExpr(),
                                 attrName, attrTypeLE );
  CHECK_COND( attrIndex > 0, errmsg );
  NList attrType ( attrTypeLE );
  string attrTypeStr = attrType.convertToString();

  // check if the metric given in second argument is defined
  errmsg = "Operator createmtree3 expects the name of a registered"
           "metric as second argument, but got a list with structure"
           " '" + arg2.convertToString() + "'.";
  CHECK_COND( arg2.isSymbol(), errmsg);
  string mfName = arg2.str();
  if (mfName == "default")
  {
    mfName = MetricRegistry::getDefaultName( attrTypeStr );
    errmsg = "Missing default metric for type constructor " +
             attrTypeStr + "!";
    CHECK_COND( mfName != "unknown", errmsg);
  }

  errmsg = "Metric " + mfName + " for type constructor " +
           attrTypeStr + " not defined!";
  cond = MetricRegistry::getMetric( attrTypeStr, mfName ) != 0;
  CHECK_COND( cond, errmsg);

  // check if the mtree-config given in third argument is defined
  errmsg = "Operator createmtree3 expects the name of a registered"
           "mtree-config object as third argument, but got a list "
           "with structure '" + arg2.convertToString() + "'.";
  CHECK_COND( arg3.isSymbol(), errmsg);
  string configName = arg3.str();
  errmsg = "MTreeConfig " + configName + " for type constructor " +
           attrTypeStr + " not defined!";
  cond = MT::MTreeConfigReg::contains( configName );
  CHECK_COND( cond, errmsg);

  NList result (
      NList( "APPEND" ),
      NList(
        attrIndex - 1,
        NList ( attrTypeStr, true ),
        NList ( mfName, true ),
        NList ( configName, true ) ),
      NList( NList( "mtree" ), tupleDescription, attrType ) );
  return result.listExpr();
}

struct createMTree3Info : OperatorInfo
{
  createMTree3Info()
  {
    name = "createmtree3";
    signature =
      "( <text>(rel (tuple ((id tid) (x1 t1)...(xn tn)))"
      " metric, mtreeconfig, xi) ->"
      " (mtree (tuple ((x1 t1)...(xn tn))) ti)";
    syntax = "_ createmtree3 [_, _, _]";
    meaning =
        "creates a new mtree from relation or stream in arg1\n"
        "arg2 must be the name of a registered metric\n"
        "arg3 must be the name of a registered mtree config object\n"
        "arg4 must be the name of the attribute in arg1, "
        "which should be indexed by the mtree";
    example = "pictures createmtree [lab, mlbDistHP, Pic]";
  }
};

/********************************************************************
5.1.3.4 Operator range

********************************************************************/

struct rangeSearchLocalInfo
{
  Relation* relation;
  list<TupleId>* results;
  list<TupleId>::iterator iter;

  rangeSearchLocalInfo( Relation* rel ) :
    relation( rel ),
    results( new list<TupleId> )
    {}

  void initResultIterator()
  {
    iter = results->begin();
  }

  ~rangeSearchLocalInfo()
  {
    delete results;
  }

  TupleId next()
  {
    if ( iter != results->end() )
    {
      TupleId tid = *iter;
      *iter++;
      return tid;
    }
    else
    {
      return 0;
    }
  }
};

int rangeSearchValueMapping(
    Word* args, Word& result, int message, Word& local, Supplier s )
{
  rangeSearchLocalInfo *localInfo;

  switch (message)
  {
    case OPEN :
    {
      localInfo = new rangeSearchLocalInfo(
          static_cast<Relation*>( args[0].addr ) );
      MT::MTree* mtree = static_cast<MT::MTree*>( args[1].addr );
      Attribute* attr = static_cast<Attribute*>( args[2].addr );
      double searchRad = ((CcReal*)args[3].addr)->GetValue();

      #ifdef __MT_PRINT_CONFIG_INFO
      mtree->printMTreeConfig();
      #endif

      mtree->rangeSearch( attr, searchRad, localInfo->results );
      localInfo->initResultIterator();

      assert(localInfo->relation != 0);
      local = SetWord(localInfo);
      return 0;
    }

    case REQUEST :
    {
      localInfo = (rangeSearchLocalInfo*)local.addr;

      TupleId tid = localInfo->next();
      if( tid )
      {
        Tuple *tuple = localInfo->relation->GetTuple( tid );
        result = SetWord( tuple );
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    {
      localInfo = (rangeSearchLocalInfo*)local.addr;
      delete localInfo;
      return 0;
    }
  }
  return 0;
}

ListExpr rangeSearchTypeMapping( ListExpr args )
{
  string errmsg;
  bool cond;
  NList nl_args( args );

  errmsg = "Operator range expects three arguments.";
  CHECK_COND( nl_args.length() == 4, errmsg );

  NList arg1 = nl_args.first();
  NList arg2 = nl_args.second();
  NList arg3 = nl_args.third();
  NList arg4 = nl_args.fourth();

  // check first argument (should be relation or stream)
  cond = !(arg1.isAtom()) &&
         (
           ( arg1.first().isEqual( "rel" ) &&
             IsRelDescription( arg1.listExpr() )) ||
           ( arg1.first().isEqual( "stream" ) &&
             IsStreamDescription( arg1.listExpr() ))
         );
  errmsg = "Operator rangesearch expects a list with structure\n"
           "   rel (tuple ((a1 t1)...(an tn))) or\n"
           "   stream (tuple ((a1 t1)...(an tn)))\n"
           "as first argument, but got a list with structure '" +
       arg1.convertToString() + "'.";
  CHECK_COND( cond , errmsg );

  cond = !(arg2.isAtom()) &&
         arg2.first().isEqual( "mtree" ) &&
         arg2.second().first().isEqual( "tuple" ) &&
         IsTupleDescription( arg2.second().second().listExpr() ) &&
         arg2.third().isSymbol();

  errmsg = "Operator rangesearch expects a mtree "
           "as second argument, but got '" +
           arg2.convertToString() + "'.";
  CHECK_COND( cond , errmsg );

  // check second argument

  // check if used attribute is equal to attribute used in m-tree
  cond = arg2.third().isEqual( arg3.convertToString() );
  errmsg = "The used m-tree contains attributes of type " +
           arg2.third().convertToString() + ", but the given "
           " attribute argument is of type " +
           arg3.convertToString();
  CHECK_COND( cond, errmsg );

  // check fourth argument
  errmsg = "Operator createmtree expects an real value as fourth "
           "argument, but got '" + arg4.convertToString() + "'.";
  CHECK_COND( arg4.isEqual( "real" ), errmsg );

  return
    nl->TwoElemList(
      nl->SymbolAtom("stream"),
      arg1.second().listExpr());
}

struct rangeSearchInfo : OperatorInfo
{
  rangeSearchInfo()
  {
    name = "rangesearch";
    signature =
      "( <text>(rel (tuple ((id tid) (x1 t1)...(xn tn)))) x"
      " (mtree (tuple ((x1 t1)...(xn tn))) ti) ->"
      " (stream (tuple ((x1 t1)...(xn tn))))";
    syntax = "_ _ rangesearch [_, _]";
    meaning = "arg1 should be the relation, from which the mtree in "
              "arg2 has been created\n"
              "arg3 contain the reference attribute\n"
              "arg4 contain the maximal distance to arg3";
    example = "pictures pictree rangesearch [pic1, 0.2]";
  }
};

/********************************************************************
5.1.3.5 Operator nnsearch

********************************************************************/

struct nnSearchLocalInfo
{
  Relation* relation;
  list<TupleId>* results;
  list<TupleId>::iterator iter;

  nnSearchLocalInfo( Relation* rel ) :
    relation( rel ),
    results( new list<TupleId> )
    {}

  void initResultIterator()
  {
    iter = results->begin();
  }

  ~nnSearchLocalInfo()
  {
    delete results;
  }

  TupleId next()
  {
    if ( iter != results->end() )
    {
      TupleId tid = *iter;
      *iter++;
      return tid;
    }
    else
    {
      return 0;
    }
  }
};

int nnSearchValueMapping(
    Word* args, Word& result, int message, Word& local, Supplier s )
{
  nnSearchLocalInfo *localInfo;

  switch (message)
  {
    case OPEN :
    {
      localInfo = new nnSearchLocalInfo(
          static_cast<Relation*>( args[0].addr ) );
      MT::MTree* mtree = static_cast<MT::MTree*>( args[1].addr );
      Attribute* attr = static_cast<Attribute*>( args[2].addr );
      int nncount= ((CcInt*)args[3].addr)->GetValue();

      #ifdef __MT_PRINT_CONFIG_INFO
      mtree->printMTreeConfig();
      #endif

      mtree->nnSearch( attr, nncount, localInfo->results );
      localInfo->initResultIterator();

      assert(localInfo->relation != 0);
      local = SetWord(localInfo);
      return 0;
    }

    case REQUEST :
    {
      localInfo = (nnSearchLocalInfo*)local.addr;

      TupleId tid = localInfo->next();
      if( tid )
      {
        Tuple *tuple = localInfo->relation->GetTuple( tid );
        result = SetWord( tuple );
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    {
      localInfo = (nnSearchLocalInfo*)local.addr;
      delete localInfo;
      return 0;
    }
  }
  return 0;
}

ListExpr nnSearchTypeMapping( ListExpr args )
{
  string errmsg;
  bool cond;
  NList nl_args( args );

  errmsg = "Operator nnsearch expects three arguments.";
  CHECK_COND( nl_args.length() == 4, errmsg );

  NList arg1 = nl_args.first();
  NList arg2 = nl_args.second();
  NList arg3 = nl_args.third();
  NList arg4 = nl_args.fourth();

  // check first argument (should be relation or stream)
  cond = !(arg1.isAtom()) &&
         (
           ( arg1.first().isEqual( "rel" ) &&
             IsRelDescription( arg1.listExpr() )) ||
           ( arg1.first().isEqual( "stream" ) &&
             IsStreamDescription( arg1.listExpr() ))
         );
  errmsg = "Operator nnsearch expects a list with structure\n"
           "   rel (tuple ((a1 t1)...(an tn))) or\n"
           "   stream (tuple ((a1 t1)...(an tn)))\n"
           "as first argument, but got a list with structure '" +
       arg1.convertToString() + "'.";
  CHECK_COND( cond , errmsg);

  // check second argument
  errmsg = "Operator nnearch expects a mtree "
           "as second argument, but got '" +
           arg2.convertToString() + "'.";
  CHECK_COND( arg2.first().isEqual( "mtree" ), errmsg );

  // check if used attribute is equal to attribute used in m-tree
  cond = arg2.third().isEqual( arg3.convertToString() );
  errmsg = "The used m-tree contains attributes of type " +
           arg2.third().convertToString() + ", but the given "
           " attribute argument is of type " +
           arg3.convertToString();
  CHECK_COND( cond, errmsg );

  // check fourth argument
  errmsg = "Operator nnsearch expects an int value as fourth "
           "argument, but got '" + arg4.convertToString() + "'.";
  CHECK_COND( arg4.isEqual( "int" ), errmsg );

  return
    nl->TwoElemList(
      nl->SymbolAtom("stream"),
      arg1.second().listExpr());
}

struct nnSearchInfo : OperatorInfo
{
  nnSearchInfo()
  {
    name = "nnsearch";
    signature =
      "( <text>(rel (tuple ((id tid) (x1 t1)...(xn tn)))) x"
      " (mtree (tuple ((x1 t1)...(xn tn))) ti) ->"
      " (stream (tuple ((x1 t1)...(xn tn))))";
    syntax = "_ _ nnsearch [_, _]";
    meaning = "arg1 should be the relation, from which the mtree in "
              "arg2 has been created\n"
              "arg3 contain the reference attribute\n"
              "arg4 contain the count of nearest neighbours of arg3 "
              "which should be searched";
    example = "pictures pictree nnsearch [pic1, 5]";
  }
};

/********************************************************************
5.1.3.5 Operator mdistance

********************************************************************/
int mdistanceValueMapping(
    Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcReal* resultValue = static_cast<CcReal*>(result.addr);

  Attribute* attr1 =
      static_cast<Attribute*>(args[0].addr);

  Attribute* attr2 =
      static_cast<Attribute*>(args[1].addr);

  string type =
      static_cast<CcString*>(args[2].addr)->GetValue();

  string mfName =
      static_cast<CcString*>(args[3].addr)->GetValue();

  TMetric metric = MetricRegistry::getMetric( type, mfName );
  TGetDataFun getData = MetricRegistry::getDataFun( type, mfName );

  double dist;
  (*metric)( (*getData)(attr1), (*getData)(attr2), dist );
  resultValue->Set( true, dist );

  return 0;
}

ListExpr mdistanceTypeMapping( ListExpr args )
{
  string errmsg;
  NList nl_args( args );

  errmsg = "Operator mdistance expects two arguments.";
  CHECK_COND( nl_args.length() == 2, errmsg );

  NList arg1 = nl_args.first();
  NList arg2 = nl_args.second();

  errmsg = "Operator mdistance expects two attributes of the "
           "same type!";
  CHECK_COND( arg1 == arg2, errmsg );

  string type = arg1.convertToString();

  // check if default metric exists
  string mfName = MetricRegistry::getDefaultName( type );
  errmsg = "Missing default metric for type constructor " +
           type + "!";
  CHECK_COND( mfName != "unknown", errmsg);

  NList result (
      NList( "APPEND" ),
      NList(
        NList ( type, true ),
        NList ( mfName, true ) ),
      NList( "real" ) );

  return result.listExpr();
}

struct mdistanceInfo : OperatorInfo
{
  mdistanceInfo()
  {
    name = "mdistance";
    signature = "attribute x attribute x metricName -> real";
    syntax = "mdistance(_, _)";
    meaning = "computes the distance between arg1 and arg2 "
              "by applying the default metric for the arg1 and arg2";
    example = "mdistance(pic1, pic2)";
    remark = "arg1 and arg2 must be of the same type";
  }
};

/********************************************************************
5.1.3.5 Operator mdistance2

********************************************************************/
int mdistance2ValueMapping(
    Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcReal* resultValue = static_cast<CcReal*>(result.addr);

  Attribute* attr1 =
      static_cast<Attribute*>(args[0].addr);

  Attribute* attr2 =
      static_cast<Attribute*>(args[1].addr);

  string type =
      static_cast<CcString*>(args[3].addr)->GetValue();

  string mfName =
      static_cast<CcString*>(args[4].addr)->GetValue();

  TMetric metric = MetricRegistry::getMetric( type, mfName );
  TGetDataFun getData = MetricRegistry::getDataFun( type, mfName );

  double dist;
  (*metric)( (*getData)(attr1), (*getData)(attr2), dist );
  resultValue->Set( true, dist );

  return 0;
}

ListExpr mdistance2TypeMapping( ListExpr args )
{
  string errmsg;
  NList nl_args( args );

  errmsg = "Operator mdistance2 expects three arguments.";
  CHECK_COND( nl_args.length() == 3, errmsg );

  NList arg1 = nl_args.first();
  NList arg2 = nl_args.second();
  NList arg3 = nl_args.third();

  errmsg = "Operator mdistance2 expects two attributes of the "
           "same type!";
  CHECK_COND( arg1 == arg2, errmsg );

  string type = arg1.convertToString();

  // check if the metric given in third argument is defined
  errmsg = "Operator mdistance2 expects the name of a registered"
           "metric as third argument, but got a list with structure"
           " '" + arg3.convertToString() + "'.";
  CHECK_COND( arg3.isSymbol(), errmsg);
  string mfName = arg3.str();
  if (mfName == "default")
  {
    mfName = MetricRegistry::getDefaultName( type );
    errmsg = "Missing default metric for type constructor " +
             type + "!";
    CHECK_COND( mfName != "unknown", errmsg);
  }

  errmsg = "Metric " + mfName + " for type constructor " +
           type + " not defined!";
  CHECK_COND(MetricRegistry::getMetric( type, mfName ) != 0, errmsg);

  NList result (
      NList( "APPEND" ),
      NList(
        NList ( type, true ),
        NList ( mfName, true ) ),
      NList( "real" ) );

  return result.listExpr();
}

struct mdistance2Info : OperatorInfo
{
  mdistance2Info()
  {
    name = "mdistance2";
    signature = "attribute x attribute x metricName -> real";
    syntax = "mdistance2(_, _, _)";
    meaning = "computes the distance between arg1 and arg2 "
              "by applying the metric given in arg3";
    example = "mdistance2(pic1, pic2, lab)";
    remark = "arg1 and arg2 must be of the same type";
  }
};

/*
5.1.5 Create and initialize the Algebra

*/

class MTreeAlgebra : public Algebra
{

public:
  MTreeAlgebra() : Algebra()
  {
    AddTypeConstructor( &mtree );

    AddOperator(
        createMTreeInfo(),
        createMTreeMap,
        createMTreeSelect,
        createMTreeTypeMapping );

    AddOperator(
        createMTree2Info(),
        createMTree2Map,
        createMTree2Select,
        createMTree2TypeMapping );

    AddOperator(
        createMTree3Info(),
        createMTree3Map,
        createMTree3Select,
        createMTree3TypeMapping );

    AddOperator(
        rangeSearchInfo(),
        rangeSearchValueMapping,
        rangeSearchTypeMapping );

    AddOperator(
        nnSearchInfo(),
        nnSearchValueMapping,
        nnSearchTypeMapping );

    AddOperator(
        mdistanceInfo(),
        mdistanceValueMapping,
        mdistanceTypeMapping );

    AddOperator(
        mdistance2Info(),
        mdistance2ValueMapping,
        mdistance2TypeMapping );

  }

  ~MTreeAlgebra() {};
};

MTreeAlgebra mtreeAlgebra;

extern "C"
  Algebra*
  InitializeMTreeAlgebra( NestedList  *nlRef,
              QueryProcessor  *qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return ( &mtreeAlgebra );
}
