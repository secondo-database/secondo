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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Implementation of the class Edge

[TOC]

1 Overview

This implementation file contains the implementation of the class Edge

*/

#include "GraphAlgebra.h"


/*
2 Implementation of the class Edge

*/


Edge::Edge()
{
}

Edge::Edge(int nSource, int nTarget, float fCost) : 
  source(nSource), target(nTarget), cost(fCost), defined(true)
{
}


Edge::~Edge()
{
  
}

void Edge::SetDefined(bool def)
{  
  defined=def;
}


bool Edge::IsDefined()const
{  
  return(defined);
}


Edge* Edge::Clone()const 
{ 
  Edge* pRet;
  if (defined)
  {
    pRet = new Edge(source, target, cost);
  }
  else
  {
    pRet = new Edge;
    pRet->SetDefined(false);
  }
  return pRet; 
}

size_t Edge::Sizeof() const
{
  return sizeof(*this);
}

int Edge::Compare(const Attribute* pAttr) const
{
  int nRet = 0;
  Edge const * pEdge = dynamic_cast<Edge const *>(pAttr);
  if (pEdge != NULL)
  {
    if (pEdge->IsDefined())
    {
      if (!defined)
      {
        nRet = -1;
      }
      else if (source > pEdge->GetSource())
      {
        nRet = 1;
      }
      else if (source < pEdge->GetSource())
      {
        nRet = -1;
      }
      else if (target > pEdge->GetTarget())
      {
        nRet = 1;
      }
      else if (target < pEdge->GetTarget())
      {
        nRet = -1;
      }
    }
    else if (defined)
    {
      nRet = 1;
    }
  }
  else
  {
    nRet = -1;
  }
  return nRet;
}


bool Edge::Adjacent(const Attribute*) const
{  
  return false;
}

size_t Edge::HashValue() const
{
  size_t nRet = 0;
  if (defined)
    nRet = source + target + *reinterpret_cast<size_t const *>(&cost);
  return nRet;
}

void Edge::CopyFrom(const StandardAttribute* arg)
{
  Edge const * pEdge = dynamic_cast<Edge const *>(arg);
  if (pEdge != NULL)
  {
    defined = pEdge->IsDefined();
    source = pEdge->GetSource();
    target = pEdge->GetTarget();
    cost = pEdge->GetCost();
  }
}

/*
3 Additional functions needed to define the type edge

*/

void* CastEdge (void* addr)
{
  return (new (addr) Edge);
}

ListExpr OutEdge( ListExpr typeInfo, Word value )
{
  Edge const * pEdge = static_cast<Edge const *>(value.addr);
  if (pEdge->IsDefined())
  {
    return nl->ThreeElemList(nl->IntAtom(pEdge->GetSource()),
      nl->IntAtom(pEdge->GetTarget()),
      nl->RealAtom(pEdge->GetCost()));
  }
  else
  {
    return nl->SymbolAtom("undef");
  }
}

Word InEdge( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{  
  if (nl->ListLength(instance) == 3)
  {
    ListExpr first = nl->First(instance);
    ListExpr second = nl->Second(instance);
    ListExpr third = nl->Third(instance);
    
    if (nl->IsAtom(first) && nl->AtomType(first) == IntType &&
      nl->IsAtom(second) && nl->AtomType(second) == IntType &&
      nl->IsAtom(third) && nl->AtomType(third) == RealType)
    {
      float fCost = nl->RealValue(third);
      if (fCost >= 0.0)
      {
        correct = true;
        return SetWord(new Edge(nl->IntValue(first), 
          nl->IntValue(second), fCost));
      }
      else
      {
        cout << "Negative costs are not allowed!" << endl;
      }
    }
    else
    {
      correct = false;
    }
  }
  else if (nl->AtomType(instance) == SymbolType && 
    nl->SymbolValue(instance) == "undef")
  {
    correct = true;
    Edge* pEdge = new Edge;
    pEdge->SetDefined(false);
    return SetWord(pEdge);
  }
  
  correct = false;
  return SetWord(Address(0));
}

ListExpr EdgeProperty()
{  
  return (nl->TwoElemList(
      nl->FiveElemList(nl->StringAtom("Signature"),
        nl->StringAtom("Example Type List"),
      nl->StringAtom("List Rep"),
      nl->StringAtom("Example List"),
      nl->StringAtom("Remarks")),
    nl->FiveElemList(nl->StringAtom("-> DATA"),
        nl->StringAtom("edge"),
      nl->StringAtom("(<source> <target> <cost>)"),
      nl->StringAtom("(1 2 1.0)"),
      nl->StringAtom("source, target: int; cost: float."))));
}

Word
CreateEdge( const ListExpr typeInfo )
{  
    return SetWord(new Vertex(0, 0, 1.0));
}

void
DeleteEdge( const ListExpr typeInfo, Word& w )
{
  static_cast<Edge *>(w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void
CloseEdge( const ListExpr typeInfo, Word& w )
{  
  DeleteEdge(typeInfo, w);
}

Word
CloneEdge( const ListExpr typeInfo, const Word& w )
{
  return SetWord((static_cast<Edge const *>(w.addr))->Clone());
}

int
SizeofEdge()
{  
  return sizeof(Edge);
}


bool
CheckEdge( ListExpr type, ListExpr& errorInfo )
{  
  return (nl->IsEqual(type, "edge"));
}


bool OpenEdge( SmiRecord& valueRecord, 
  size_t& offset, const ListExpr typeInfo, Word& value )
{
  value = SetWord(Attribute::Open(valueRecord, offset, typeInfo));
  return true; 
}


bool SaveEdge( SmiRecord& valueRecord, 
  size_t& offset, const ListExpr typeInfo, Word& value )
{
  Attribute::Save(valueRecord, offset, typeInfo, 
    static_cast<Attribute*>(value.addr));
  return true;
}


TypeConstructor edgeCon(
  "edge",                 //name
  EdgeProperty,          //property function describing signature
    OutEdge, InEdge,        //Out and In functions
    0, 0,                   //SaveToList and RestoreFromList functions
  CreateEdge, DeleteEdge, //object creation and deletion
    OpenEdge, SaveEdge,     //object open, save
  CloseEdge, CloneEdge,   //object close, and clone
  CastEdge,               //cast function
    SizeofEdge,             //sizeof function
  CheckEdge);             //kind checking function

  
/*
4 operators

*/
  
  
ListExpr EdgeIntTypeMap(ListExpr args)
{
  if (nl->ListLength(args) == 1)
  {
    ListExpr arg = nl->First(args);
    if (nl->IsEqual(arg, "edge"))
    {
      return nl->SymbolAtom("int");
    }
    else
    {
      ErrorReporter::ReportError(
        "Type mapping function got paramater of type " +
        nl->SymbolValue(arg));
    }
  }
  else
  {
    ErrorReporter::ReportError(
      "Type mapping function got a parameter of length != 1.");
  }
  return nl->SymbolAtom("typeerror");
}

ListExpr EdgeRealTypeMap(ListExpr args)
{
  if (nl->ListLength(args) == 1)
  {
    ListExpr arg = nl->First(args);
    if (nl->IsEqual(arg, "edge"))
    {
      return nl->SymbolAtom("real");
    }
    else
    {
      ErrorReporter::ReportError(
        "Type mapping function got paramater of type " +
        nl->SymbolValue(arg));
    }
  }
  else
  {
    ErrorReporter::ReportError(
      "Type mapping function got a parameter of length != 1.");
  }
  return nl->SymbolAtom("typeerror");
}


int graphsource(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Edge const * pEdge = static_cast<Edge const *>(args[0].addr);
  result = qp->ResultStorage(s);
  CcInt* pRet = static_cast<CcInt *>(result.addr);
  pRet->Set(true, pEdge->GetSource());
  
  return 0;
}


int graphtarget(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Edge const * pEdge = static_cast<Edge const *>(args[0].addr);
  result = qp->ResultStorage(s);
  CcInt* pRet = static_cast<CcInt *>(result.addr);
  pRet->Set(true, pEdge->GetTarget());
  
  return 0;
}

int graphcost(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Edge const * pEdge = static_cast<Edge const *>(args[0].addr);
  result = qp->ResultStorage(s);
  CcReal* pRet = static_cast<CcReal *>(result.addr);
  pRet->Set(true, pEdge->GetCost());
  
  return 0;
}

string const sourceSpec = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text>edge -> int</text--->"
   "<text>source ( _ )</text--->"
   "<text>the source vertex of the edge</text--->"
   "<text>source(e1)</text---> ) )";

 
string const targetSpec = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>edge -> int</text--->"
  "<text>target ( _ )</text--->"
  "<text>the target vertex of the edge</text--->"
  "<text>target(e1)</text---> ) )";

 
string const costSpec = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>edge -> real</text--->"
  "<text>cost ( _ )</text--->"
  "<text>the cost of the edge</text--->"
  "<text>cost(e1)</text---> ) )";



/*
4.1 operator source

returns the key of the source vertex

*/
Operator graph_source("source", sourceSpec, graphsource, 
  Operator::SimpleSelect, EdgeIntTypeMap);

/*
4.2 operator target

returns the key of the target vertex

*/
Operator graph_target("target", targetSpec, graphtarget, 
  Operator::SimpleSelect, EdgeIntTypeMap);

/*
4.3 operator cost

returns the cost of the edge

*/
Operator graph_cost("cost", costSpec, graphcost, 
  Operator::SimpleSelect, EdgeRealTypeMap);

