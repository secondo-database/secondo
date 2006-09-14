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

[1] Implementation of Module Network Algebra

March 2004 Victor Almeida

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

[TOC]

1 Overview

This file contains the implementation of the type constructors ~network~,
~gpoint~, and ~gline~ and the temporal corresponding ~moving~(~gpoint~)
and ~moving~(~gline~).

2 Defines, includes, and constants

*/
#include <sstream>

#include "Algebra.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "NetworkAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;

/*
3 Type Constructor ~network~

3.1 Implementation of the class ~Network~

*/
string Network::routesTypeInfo =
      "(rel (tuple ((id int) (length real) (curve line) "
      "(dual bool) (startsSmaller bool))))";
string Network::routesBTreeTypeInfo =
      "(btree (tuple ((id int) (length real) (curve line) "
      "(dual bool) (startsSmaller bool))) int)";
string Network::junctionsTypeInfo =
      "(rel (tuple ((r1id int) (meas1 real) (r2id int) "
      "(meas2 int) (cc int))))";
string Network::junctionsInternalTypeInfo =
      "(rel (tuple ((r1id int) (meas1 real) (r2id int) "
      "(meas2 real) (cc int) (pos point) (r1rc int) (r2rc int))))";
string Network::junctionsAppendTypeInfo =
      "(rel (tuple ((pos point) (r1rc int) (r2rc int))))";
string Network::sectionsTypeInfo =
      "(rel (tuple ((rid int) (meas1 real) (meas2 real) "
      "(dual bool) (curve line))))";
string Network::sectionsInternalTypeInfo =
      "(rel (tuple ((rid int) (meas1 real) (meas2 real) "
      "(dual bool) (curve line) (rrc int))))";
string Network::sectionsAppendTypeInfo =
      "(rel (tuple ((rrc int))))";

Network::Network():
routes( 0 ),
junctions( 0 ),
sections( 0 ),
routesBTree( 0 )
{}

Network::Network( Relation *routes, Relation *junctions, 
                  Relation *sections, BTree *routesBTree ):
routes( routes ),
junctions( junctions ),
sections( sections ),
routesBTree( routesBTree )
{}

Network::~Network()
{
  delete routes;
  delete junctions;
  delete sections;
  delete routesBTree;
}

void Network::Destroy()
{
  assert( routes != 0 );
  routes->Delete(); routes = 0;
  assert( junctions != 0 );
  junctions->Delete(); junctions = 0;
  assert( sections != 0 );
  sections->Delete(); sections = 0;
  assert( routesBTree != 0 );
  routesBTree->DeleteFile(); 
  delete routesBTree; routesBTree = 0;
}

void Network::Load( const Relation *routes, const Relation *junctions )
{
  FillRoutes( routes );
  FillJunctions( junctions );
  FillSections();
}

void Network::FillRoutes( const Relation *routes )
{
  ostringstream strRoutesPtr;
  strRoutesPtr << (int)routes;

  string querystring = "(consume (sort (feed (" + routesTypeInfo + 
                       " (ptr " + strRoutesPtr.str() + ")))))";

  Word resultWord;
  assert( QueryProcessor::ExecuteQuery( querystring, resultWord ) );
  this->routes = (Relation *)resultWord.addr;

  ostringstream strThisRoutesPtr;
  strThisRoutesPtr << (int)this->routes;

  querystring = "(createbtree (" + routesTypeInfo + 
                " (ptr " + strThisRoutesPtr.str() + "))" + " id)";

  assert( QueryProcessor::ExecuteQuery( querystring, resultWord ) );
  this->routesBTree = (BTree*)resultWord.addr;
}

Relation *Network::GetRoutes()
{
  return routes->Clone();
}

Relation *Network::GetRoutesInternal()
{
  return routes;
}

void Network::FillJunctions( const Relation *junctions )
{
  ListExpr 
    junctionsNumInt = 
      SecondoSystem::GetCatalog()->NumericType( GetJunctionsIntTypeInfo() );

  Relation *unJunctions = new Relation( junctionsNumInt, true );

  RelationIterator *junctionsIter = junctions->MakeScan();
  Tuple *j;

  while( (j = junctionsIter->GetNextTuple()) != 0)
  {
    Tuple *intJ = new Tuple( nl->Second( junctionsNumInt ) );

    for( int i = 0; i < j->GetNoAttributes(); i++ )
      intJ->CopyAttribute( i, j, i );
    
    CcInt *r1id = (CcInt*)j->GetAttribute( POS_JR1ID ),
          *r2id = (CcInt*)j->GetAttribute( POS_JR2ID );

    BTreeIterator *routesIter = routesBTree->ExactMatch( r1id );
    assert( routesIter->Next() );

    CcInt *r1rc = new CcInt( true, routesIter->GetId() );
    intJ->PutAttribute( POS_JR1RC, r1rc );
    
    Tuple *r = this->routes->GetTuple( routesIter->GetId() );
    assert( r != 0 );

    Line *l = (Line*)r->GetAttribute( POS_RCURVE );
    assert( l != 0 );
    CcReal *meas = (CcReal*)j->GetAttribute( POS_JMEAS1 );
    Point *p = new Point( false );
    l->AtPosition( meas->GetRealval(), true, *p );
    intJ->PutAttribute( POS_JPOS, p );

    r->DeleteIfAllowed();
    delete routesIter;


    routesIter = routesBTree->ExactMatch( r2id );
    assert( routesIter->Next() );
    CcInt *r2rc = new CcInt( true, routesIter->GetId() );
    intJ->PutAttribute( POS_JR2RC, r2rc );
    delete routesIter;


    unJunctions->AppendTuple( intJ );
  
    CcInt *aux = (CcInt*)intJ->GetAttribute( POS_JR1ID )->Copy();
    intJ->PutAttribute( POS_JR1ID, 
                           intJ->GetAttribute( POS_JR2ID )->Copy() );
    intJ->PutAttribute( POS_JR2ID, aux );

    aux = (CcInt*)intJ->GetAttribute( POS_JMEAS1 )->Copy();
    intJ->PutAttribute( POS_JMEAS1, 
                           intJ->GetAttribute( POS_JMEAS2 )->Copy() );
    intJ->PutAttribute( POS_JMEAS2, aux );

    unJunctions->AppendTuple( intJ );

    intJ->DeleteIfAllowed();
    j->DeleteIfAllowed();
  }

  ostringstream strJunctionsPtr;
  strJunctionsPtr << (int)unJunctions;

  string querystring = "(consume (sortby (feed (" + junctionsInternalTypeInfo +
                       " (ptr " + strJunctionsPtr.str() + 
                       "))) ((r1id asc)(r2id asc))))";

  Word resultWord;
  assert( QueryProcessor::ExecuteQuery( querystring, resultWord ) );
  this->junctions = (Relation *)resultWord.addr;
  
  delete junctionsIter;
  unJunctions->Delete();
}

Relation *Network::GetJunctionsInternal()
{
  return junctions;
}

Relation *Network::GetJunctions()
{
  ostringstream strJunctionsPtr;
  strJunctionsPtr << (int)junctions;

  string querystring = "(consume (feed (" + junctionsInternalTypeInfo +
                       " (ptr " + strJunctionsPtr.str() + "))))";

  Word resultWord;
  assert( QueryProcessor::ExecuteQuery( querystring, resultWord ) );
  return (Relation *)resultWord.addr;
}

void Network::FillSections()
{
  RelationIterator *routesIter = routes->MakeScan(),
                   *junctionsIter = junctions->MakeScan();
  Tuple *rTuple, *jTuple = junctionsIter->GetNextTuple();

  ListExpr sectionsNumInt = 
    SecondoSystem::GetCatalog()->NumericType( GetSectionsInternalTypeInfo() );
  this->sections = new Relation( sectionsNumInt );

  while( (rTuple = routesIter->GetNextTuple()) != 0 )
  {
    CcReal *meas1 = new CcReal( true, 0 );
    
    while( jTuple != 0 &&
           ((CcInt*)jTuple->GetAttribute( POS_JR1ID ))->GetIntval() == 
           ((CcInt*)rTuple->GetAttribute( POS_RID ))->GetIntval() )
    {
      if( ((CcReal*)jTuple->GetAttribute( POS_JMEAS1 ))->GetRealval() > 
          meas1->GetRealval() )
      // Create a section from meas1 to the actual junction.
      {
        Tuple *sTuple = new Tuple( nl->Second( sectionsNumInt ) );
        sTuple->PutAttribute( POS_SRID, 
                              rTuple->GetAttribute( POS_RID )->Clone() );
        sTuple->PutAttribute( POS_SDUAL, 
                              rTuple->GetAttribute( POS_RDUAL )->Clone() );
        sTuple->PutAttribute( POS_SMEAS1, meas1 );
        sTuple->PutAttribute( POS_SMEAS2, 
                              jTuple->GetAttribute( POS_JMEAS1 )->Clone() );
        sTuple->PutAttribute( POS_SRRC, 
                              new CcInt( true, rTuple->GetTupleId() ) );

        Line *l = new Line( 0 ),
             *curve = (Line*)rTuple->GetAttribute( POS_RCURVE );
        curve->SubLine( meas1->GetRealval(),
                        ((CcReal*)jTuple->GetAttribute( POS_JMEAS1 ))->
                          GetRealval(),
                        true,
                        *l );
        sTuple->PutAttribute( POS_SCURVE, l );

        this->sections->AppendTuple( sTuple );
        sTuple->DeleteIfAllowed();

        meas1 = (CcReal*)jTuple->GetAttribute( POS_JMEAS1 )->Clone();
      }
      jTuple->DeleteIfAllowed();
      jTuple = junctionsIter->GetNextTuple();
    }

    if( meas1->GetRealval() < 
        ((CcReal*)rTuple->GetAttribute( POS_RLENGTH ))->GetRealval() )
    {
      Tuple *sTuple = new Tuple( nl->Second( sectionsNumInt ) );
      sTuple->PutAttribute( POS_SRID, 
                            rTuple->GetAttribute( POS_RID )->Clone() );
      sTuple->PutAttribute( POS_SDUAL, 
                            rTuple->GetAttribute( POS_RDUAL )->Clone() );
      sTuple->PutAttribute( POS_SMEAS1, meas1 );
      sTuple->PutAttribute( POS_SMEAS2, 
                            rTuple->GetAttribute( POS_RLENGTH )->Clone() );
      sTuple->PutAttribute( POS_SRRC, 
                            new CcInt( true, rTuple->GetTupleId() ) );
      Line *l = new Line( 0 ),
           *curve = (Line*)rTuple->GetAttribute( POS_RCURVE );
      curve->SubLine( meas1->GetRealval(),
                      ((CcReal*)rTuple->GetAttribute( POS_RLENGTH ))->
                        GetRealval(),
                      true,
                      *l );
      sTuple->PutAttribute( POS_SCURVE, l );

      this->sections->AppendTuple( sTuple );
      sTuple->DeleteIfAllowed();
    }
    rTuple->DeleteIfAllowed();
  }
  delete routesIter;
  delete junctionsIter;
}

Relation *Network::GetSectionsInternal()
{
  return sections;
}

Relation *Network::GetSections()
{
  ostringstream strSectionsPtr;
  strSectionsPtr << (int)sections;

  string querystring = "(consume (feed (" + sectionsInternalTypeInfo +
                       " (ptr " + strSectionsPtr.str() + "))))";

  Word resultWord;
  assert( QueryProcessor::ExecuteQuery( querystring, resultWord ) );
  return (Relation *)resultWord.addr;
}

ListExpr Network::GetRoutesTypeInfo()
{
  ListExpr result;

  if( nl->ReadFromString( routesTypeInfo, result ) )
    return result;

  return 0; 
}

ListExpr Network::GetRoutesBTreeTypeInfo()
{
  ListExpr result;

  if( nl->ReadFromString( routesBTreeTypeInfo, result ) )
    return result;

  return 0;
}

ListExpr Network::GetJunctionsTypeInfo()
{
  ListExpr result;

  if( nl->ReadFromString( junctionsTypeInfo, result ) )
    return result;

  return 0;
}

ListExpr Network::GetJunctionsIntTypeInfo()
{
  ListExpr result;

  if( nl->ReadFromString( junctionsInternalTypeInfo, result ) )
    return result;

  return 0;
}

ListExpr Network::GetJunctionsAppTypeInfo()
{
  ListExpr result;

  if( nl->ReadFromString( junctionsAppendTypeInfo, result ) )
    return result;

  return 0;
}

ListExpr Network::GetSectionsTypeInfo()
{
  ListExpr result;

  if( nl->ReadFromString( sectionsTypeInfo, result ) )
    return result;

  return 0;
}

ListExpr Network::GetSectionsInternalTypeInfo()
{
  ListExpr result;

  if( nl->ReadFromString( sectionsInternalTypeInfo, result ) )
    return result;

  return 0;
}

ListExpr Network::GetSectionsAppendTypeInfo()
{
  ListExpr result;

  if( nl->ReadFromString( sectionsAppendTypeInfo, result ) )
    return result;

  return 0;
}

/*
3.2 Type property of type constructor ~network~

*/
ListExpr NetworkProp()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,
                 "thenetwork( <routes-relation>, <junctions-relation> )");

  return (nl->TwoElemList(
            nl->TwoElemList(nl->StringAtom("Creation"),
                             nl->StringAtom("Example Creation")),
            nl->TwoElemList(examplelist,
                             nl->StringAtom("(let n = thenetwork( r, j ))"))));
}

/*
3.3 ~Out~-function of type constructor ~network~

*/
ListExpr Network::Out( ListExpr typeInfo )
{
  return nl->ThreeElemList( 
    nl->TwoElemList( 
      nl->StringAtom( "Routes: " ),
      nl->IntAtom( routes->GetNoTuples() ) ),
    nl->TwoElemList( 
      nl->StringAtom( "Junctions: " ),
      nl->IntAtom( junctions->GetNoTuples() ) ),
    nl->TwoElemList( 
      nl->StringAtom( "Sections: " ),
      nl->IntAtom( sections->GetNoTuples() ) ) );
}

ListExpr OutNetwork(ListExpr typeInfo, Word value)
{
  Network *n = (Network*)value.addr;
  return n->Out( typeInfo );
}

/*
3.4 ~In~-function of type constructor ~network~

*/
Word InNetwork(ListExpr typeInfo, ListExpr value,
               int errorPos, ListExpr& errorInfo, bool& correct)
{
  return SetWord( Address(0) );
}

/*
3.5 ~Create~-function of type constructor ~network~

*/
Word CreateNetwork(const ListExpr typeInfo)
{
  return SetWord( new Network() );
}

/*
3.6 ~Close~-function of type constructor ~network~

*/
void CloseNetwork(const ListExpr typeInfo, Word& w)
{
  delete (Network*)w.addr;
}

/*
3.7 ~Clone~-function of type constructor ~network~

Not implemented yet.

*/
Word CloneNetwork(const ListExpr typeInfo, const Word& w)
{
  return SetWord( Address(0) );
}

/*
3.8 ~Delete~-function of type constructor ~network~

*/
void DeleteNetwork(const ListExpr typeInfo, Word& w)
{
  Network* n = (Network*)w.addr;
  n->Destroy();
  delete n;
}

/*
3.9 ~Check~-function of type constructor ~network~

*/
bool CheckNetwork(ListExpr type, ListExpr& errorInfo)
{
  return (nl->IsEqual( type, "network" ));
}

/*
3.10 ~Cast~-function of type constructor ~network~

*/
void* CastNetwork(void* addr)
{
  return ( 0 );
}

/*
3.11 ~Save~-function of type constructor ~network~

*/
ListExpr Network::Save( SmiRecord& valueRecord, 
                        size_t& offset,
                        const ListExpr typeInfo )
{
  if( !routes->Save( valueRecord, offset, 
                     SecondoSystem::GetCatalog()->
                       NumericType( GetRoutesTypeInfo() ) ) )
    return false;

  if( !junctions->Save( valueRecord, offset, 
                        SecondoSystem::GetCatalog()->
                          NumericType( GetJunctionsIntTypeInfo() ) ) )
    return false;

  if( !sections->Save( valueRecord, offset, 
                       SecondoSystem::GetCatalog()->
                         NumericType( GetSectionsInternalTypeInfo() ) ) )
    return false;

  if( !routesBTree->Save( valueRecord, offset,
                          SecondoSystem::GetCatalog()->
                            NumericType( GetRoutesBTreeTypeInfo() ) ) )
    return false;

  return true; 
}

bool SaveNetwork( SmiRecord& valueRecord,
                  size_t& offset,
                  const ListExpr typeInfo,
                  Word& value )
{
  Network *n = (Network*)value.addr;
  return n->Save( valueRecord, offset, typeInfo );
}

/*
3.12 ~Open~-function of type constructor ~network~

*/
Network *Network::Open( SmiRecord& valueRecord, size_t& offset, 
                        const ListExpr typeInfo )
{
  Relation *routes = 0, 
           *junctions = 0,  
           *sections = 0;
  BTree *routesBTree = 0;

  if( !( routes = 
         Relation::Open( valueRecord, offset, 
                         SecondoSystem::GetCatalog()->
                           NumericType( GetRoutesTypeInfo() ) ) ) ) 
    return 0;

  if( !( junctions = 
         Relation::Open( valueRecord, offset, 
                         SecondoSystem::GetCatalog()->
                           NumericType( GetJunctionsIntTypeInfo() ) ) ) ) 
  {  
    routes->Delete(); 
    return 0;
  }

  if( !( sections = 
           Relation::Open( valueRecord, offset, 
                           SecondoSystem::GetCatalog()->
                             NumericType( GetSectionsInternalTypeInfo() ) ) ) ) 
  {
    routes->Delete(); 
    junctions->Delete(); 
    return 0;
  }

  if( !( routesBTree = 
           BTree::Open( valueRecord, offset, 
                        SecondoSystem::GetCatalog()->
                          NumericType( GetRoutesBTreeTypeInfo() ) ) ) ) 
  {
    routes->Delete(); 
    junctions->Delete(); 
    sections->Delete();
    return 0;
  }
  return new Network( routes, junctions, sections, routesBTree );
}

bool OpenNetwork( SmiRecord& valueRecord, size_t& offset, 
                  const ListExpr typeInfo, Word& value )
{
  value.addr = Network::Open( valueRecord, offset, typeInfo );
  return value.addr != 0;
}

/*
3.13 ~SizeOf~-function of type constructor ~network~

*/
int SizeOfNetwork()
{
  return 0;
}

/*
3.14 Type Constructor object for type constructor ~network~

*/
TypeConstructor network( "network",            NetworkProp,
                         OutNetwork,           InNetwork,
                         0,                    0,
                         CreateNetwork,        DeleteNetwork,
                         OpenNetwork,          SaveNetwork,
                         CloseNetwork,         CloneNetwork,
                         CastNetwork,          SizeOfNetwork,
                         CheckNetwork );

/*
4 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

4.1 Operator ~thenetwork~

4.1.1 Type Mapping of operator ~thenetwork~

*/
ListExpr NetworkTheNetworkTypeMap(ListExpr args)
{
  if( nl->ListLength(args) != 2 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr rel1Desc = nl->First(args),
           rel2Desc = nl->Second(args);

  if( !IsRelDescription( rel1Desc ) ||
      !IsRelDescription( rel2Desc ) )
    return (nl->SymbolAtom( "typeerror" ));

  if( !CompareSchemas( rel1Desc, Network::GetRoutesTypeInfo() ) )
    return (nl->SymbolAtom( "typeerror" ));
  
  if( !CompareSchemas( rel2Desc, Network::GetJunctionsTypeInfo() ) )
    return (nl->SymbolAtom( "typeerror" ));
  
  return nl->SymbolAtom( "network" );
}

/*
4.1.2 Value mapping function of operator ~thenetwork~

*/
int
NetworkTheNetworkValueMapping( Word* args, Word& result, 
                               int message, Word& local, Supplier s )
{
  Network *network = (Network*)qp->ResultStorage(s).addr;

  Relation *routes = (Relation*)args[0].addr,
           *junctions = (Relation*)args[1].addr;
 
  network->Load( routes, junctions );

  result = SetWord( network ); 
  return 0;
}

/*
4.1.3 Specification of operator ~thenetwork~

*/
const string NetworkTheNetworkSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>rel x rel -> network" "</text--->"
  "<text>thenetwork(_, _)</text--->"
  "<text>Creates a network.</text--->"
  "<text>let n = thenetwork(r, j)</text--->"
  ") )";

/*
4.1.4 Definition of operator ~thenetwork~

*/
Operator networkthenetwork (
          "thenetwork",                // name
          NetworkTheNetworkSpec,              // specification
          NetworkTheNetworkValueMapping,      // value mapping
          Operator::SimpleSelect,               // trivial selection function
          NetworkTheNetworkTypeMap            // type mapping
);

/*
4.2 Operator ~routes~

4.2.1 Type Mapping of operator ~routes~

*/
ListExpr NetworkRoutesTypeMap(ListExpr args)
{
  ListExpr arg1;
  if( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( nl->IsAtom( arg1 ) && 
        nl->AtomType( arg1 ) == SymbolType &&
        nl->SymbolValue( arg1 ) == "network" ) 
      return Network::GetRoutesTypeInfo();
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.2 Value mapping function of operator ~routes~

*/
int
NetworkRoutesValueMapping( Word* args, Word& result, int message, 
                           Word& local, Supplier s )
{
  Network *network = (Network*)args[0].addr;
  result = SetWord( network->GetRoutes() );

  Relation *resultSt = (Relation*)qp->ResultStorage(s).addr;
  resultSt->Close();
  qp->ChangeResultStorage(s, result);

  return 0;
}

/*
4.2.3 Specification of operator ~routes~

*/
const string NetworkRoutesSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>network -> rel" "</text--->"
  "<text>routes(_)</text--->"
  "<text>Return the routes of a network.</text--->"
  "<text>let r = routes(n)</text--->"
  ") )";

/*
4.2.4 Definition of operator ~routes~

*/
Operator networkroutes (
          "routes",                // name
          NetworkRoutesSpec,              // specification
          NetworkRoutesValueMapping,      // value mapping
          Operator::SimpleSelect,               // trivial selection function
          NetworkRoutesTypeMap            // type mapping
);

/*
4.3 Operator ~junctions~

4.3.1 Type Mapping of operator ~junctions~

*/
ListExpr NetworkJunctionsTypeMap(ListExpr args)
{
  ListExpr arg1;
  if( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( nl->IsAtom( arg1 ) &&
        nl->AtomType( arg1 ) == SymbolType &&
        nl->SymbolValue( arg1 ) == "network" )
      return Network::GetJunctionsIntTypeInfo();
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.3.2 Value mapping function of operator ~junctions~

*/
int
NetworkJunctionsValueMapping( Word* args, Word& result, int message, 
                              Word& local, Supplier s )
{
  Network *network = (Network*)args[0].addr;
  result = SetWord( network->GetJunctions() );

  Relation *resultSt = (Relation*)qp->ResultStorage(s).addr;
  resultSt->Close();
  qp->ChangeResultStorage(s, result);

  return 0;
}

/*
4.3.3 Specification of operator ~junctions~

*/
const string NetworkJunctionsSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>network -> rel" "</text--->"
  "<text>junctions(_)</text--->"
  "<text>Return the junctions of a network.</text--->"
  "<text>let j = junctions(n)</text--->"
  ") )";

/*
4.3.4 Definition of operator ~junctions~

*/
Operator networkjunctions (
          "junctions",                // name
          NetworkJunctionsSpec,              // specification
          NetworkJunctionsValueMapping,      // value mapping
          Operator::SimpleSelect,               // trivial selection function
          NetworkJunctionsTypeMap            // type mapping
);

/*
4.4 Operator ~sections~

4.4.1 Type Mapping of operator ~sections~

*/
ListExpr NetworkSectionsTypeMap(ListExpr args)
{
  ListExpr arg1;
  if( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( nl->IsAtom( arg1 ) &&
        nl->AtomType( arg1 ) == SymbolType &&
        nl->SymbolValue( arg1 ) == "network" )
      return Network::GetSectionsInternalTypeInfo();
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.4.2 Value mapping function of operator ~sections~

*/
int
NetworkSectionsValueMapping( Word* args, Word& result, int message, 
                             Word& local, Supplier s )
{
  Network *network = (Network*)args[0].addr;
  result = SetWord( network->GetSections() );

  Relation *resultSt = (Relation*)qp->ResultStorage(s).addr;
  resultSt->Close();
  qp->ChangeResultStorage(s, result);

  return 0;
}

/*
4.4.3 Specification of operator ~sections~

*/
const string NetworkSectionsSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>network -> rel" "</text--->"
  "<text>sections(_)</text--->"
  "<text>Return the sections of a network.</text--->"
  "<text>let j = sections(n)</text--->"
  ") )";

/*
4.4.4 Definition of operator ~sections~

*/
Operator networksections (
          "sections",                // name
          NetworkSectionsSpec,              // specification
          NetworkSectionsValueMapping,      // value mapping
          Operator::SimpleSelect,               // trivial selection function
          NetworkSectionsTypeMap            // type mapping
);

/*
5 Creating the Algebra

*/

class NetworkAlgebra : public Algebra
{
 public:
  NetworkAlgebra() : Algebra()
  {
    AddTypeConstructor( &network );

    AddOperator(&networkthenetwork);
    AddOperator(&networkroutes);
    AddOperator(&networkjunctions);
    AddOperator(&networksections);
  }
  ~NetworkAlgebra() {};
};

NetworkAlgebra networkAlgebra;

/*
6 Initialization

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
InitializeNetworkAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;

  return (&networkAlgebra);
}


