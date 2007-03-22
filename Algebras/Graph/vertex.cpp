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

[1] Implementation of the class Vertex

[TOC]

1 Overview

This implementation file contains the implementation of the class Vertex

*/


#include "GraphAlgebra.h"

/*
2 Implementation of the class Vertex

*/


Vertex::Vertex()
{
}

Vertex::Vertex(int nKey, Point const & pntPos) : 
    key(nKey), defined(true), pos(pntPos)
{
}

Vertex::Vertex(int nKey, Coord coordX, Coord coordY) : 
    key(nKey), defined(true), pos(true, coordX, coordY)
{
}

Vertex::~Vertex()
{
    
}

void Vertex::SetDefined(bool def)
{    
    defined = def;
}


bool Vertex::IsDefined() const
{    
    return (defined);
}


Vertex* Vertex::Clone() const 
{ 
    Vertex* pRet;
    if (defined)
    {
        pRet = new Vertex(key, pos);
    }
    else
    {
        pRet = new Vertex;
        pRet->SetDefined(defined);
    }
    return pRet;    
}

size_t Vertex::Sizeof() const
{    
    return sizeof(*this);
}

int Vertex::Compare(const Attribute* pAttr) const
{    
    int nRet = 0;
    Vertex const * pVertex = dynamic_cast<Vertex const *>(pAttr);
    if (pVertex != NULL)
    {
        if (pVertex->IsDefined())
        {
            if (!defined)
            {
                nRet = -1;
            }
            else if (key > pVertex->GetKey())
            {
                nRet = 1;
            }
            else if (key < pVertex->GetKey())
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


bool Vertex::Adjacent(const Attribute* pAttr) const
{    
    bool bRet = false;
    Vertex const * pVertex = dynamic_cast<Vertex const *>(pAttr);
    if (pVertex != NULL)
    {
        bRet = abs(key - pVertex->GetKey()) == 1;
    }
    return bRet;
}

size_t Vertex::HashValue() const
{
    size_t nRet = 0;
    if (defined)
    {
        nRet = key;
    }
    return nRet;
}

void Vertex::CopyFrom( const StandardAttribute* arg)
{
    Vertex const * pVertex = dynamic_cast<Vertex const *>(arg);
    if (pVertex != NULL)
    {
        defined = pVertex->IsDefined();
        key = pVertex->GetKey();
        pos = pVertex->GetPos();
    }
}


void Vertex::SetPos(Coord coordX, Coord coordY)
{
    pos.Set(coordX, coordY);
}

/*
3 Additional functions needed to define the type vertex

*/

void* CastVertex (void* addr)
{    
    return (new (addr) Vertex);
}

ListExpr OutVertex( ListExpr typeInfo, Word value )
{  
    Vertex const * pVertex = static_cast<Vertex const *>(value.addr);
    if (pVertex->IsDefined())
    {
        if (pVertex->GetPos().IsDefined())
        {
            return nl->TwoElemList(nl->IntAtom(pVertex->GetKey()),
                nl->TwoElemList(nl->RealAtom(pVertex->GetPos().GetX()), 
                nl->RealAtom(pVertex->GetPos().GetY())));
        }
        else
        {
            return nl->TwoElemList(nl->IntAtom(pVertex->GetKey()),
                nl->SymbolAtom("undef"));
        
        }
    }
    else
    {
        return nl->SymbolAtom("undef");
    }
}

Word InVertex( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{
    if (nl->ListLength(instance) == 2)
    {
        ListExpr first = nl->First(instance);
        ListExpr second = nl->Second(instance);
        
        correct = true;
        int nKey = 0;
        if (nl->IsAtom(first) && nl->AtomType(first) == IntType)
        {
            nKey = nl->IntValue(first);
        }
        else
        {
            correct = false;
        } 
        
        Coord coordX = 0.0;
        Coord coordY = 0.0;
        if (!nl->IsAtom(second) && nl->ListLength(second) == 2)
        {
            first = nl->First(second);
            second = nl->Second(second);
            if (nl->IsAtom(first) && nl->AtomType(first) == RealType &&
                nl->IsAtom(second) && nl->AtomType(second) == RealType)
            {
                coordX = nl->RealValue(first);
                coordY = nl->RealValue(second);
            }
            else
            {
                correct = false;
            }
            if (correct)
            {
                return SetWord(new Vertex(nKey, coordX, coordY));
            }
        }
        else if (nl->AtomType(second) == SymbolType && 
            nl->SymbolValue(second) == "undef")
        {
            if (correct)
            {
                Point pnt = Point(false);
                return SetWord(new Vertex(nKey, pnt));
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
        Vertex* pVertex = new Vertex;
        pVertex->SetDefined(false);
        return SetWord(pVertex);
    }
    
    correct = false;
    return SetWord(Address(0));
}

ListExpr VertexProperty()
{    
    return (nl->TwoElemList(
        nl->FiveElemList(nl->StringAtom("Signature"),
            nl->StringAtom("Example Type List"),
            nl->StringAtom("List Rep"),
            nl->StringAtom("Example List"),
            nl->StringAtom("Remarks")),
        nl->FiveElemList(nl->StringAtom("-> DATA"),
            nl->StringAtom("vertex"),
            nl->StringAtom("(key (<x> <y>))"),
            nl->StringAtom("(1 (-3.0 15.3))"),
            nl->StringAtom("key: int; x, y: float."))));
}

Word CreateVertex( const ListExpr typeInfo )
{
    return SetWord(new Vertex(0, 0.0, 0.0));
}

void DeleteVertex( const ListExpr typeInfo, Word& w )
{
    static_cast<Vertex *>(w.addr)->DeleteIfAllowed();
    w.addr = 0;
}

void CloseVertex( const ListExpr typeInfo, Word& w )
{
    DeleteVertex(typeInfo, w);
}

Word CloneVertex( const ListExpr typeInfo, const Word& w )
{
    return SetWord((static_cast<Vertex const *>(w.addr))->Clone());
}

int SizeofVertex()
{    
    return sizeof(Vertex);
}


bool CheckVertex( ListExpr type, ListExpr& errorInfo )
{    
    return (nl->IsEqual(type, "vertex"));
}


bool OpenVertex( SmiRecord& valueRecord, 
    size_t& offset, const ListExpr typeInfo, Word& value )
{
    value = SetWord(Attribute::Open(valueRecord, offset, typeInfo));
    return true; 
}


bool SaveVertex( SmiRecord& valueRecord, 
    size_t& offset, const ListExpr typeInfo, Word& value )
{
    Attribute::Save(valueRecord, offset, typeInfo, 
        static_cast<Attribute*>(value.addr));
    return true;
}


TypeConstructor vertexCon(
    "vertex",                   //name
    VertexProperty,             //property function describing signature
    OutVertex, InVertex,        //Out and In functions
    0, 0,                       //SaveToList and RestoreFromList functions
    CreateVertex, DeleteVertex, //object creation and deletion
    OpenVertex, SaveVertex,     //object open and save
    CloseVertex, CloneVertex,   //object close, and clone
    CastVertex,                 //cast function
    SizeofVertex,               //sizeof function
    CheckVertex);               //kind checking function

    
    
/*
4 operators

*/

ListExpr VertexIntTypeMap(ListExpr args)
{
    if (nl->ListLength(args) == 1)
    {
        ListExpr arg = nl->First(args);
        if (nl->IsEqual(arg, "vertex"))
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


ListExpr VertexPointTypeMap(ListExpr args)
{
    if (nl->ListLength(args) == 1)
    {
        ListExpr arg = nl->First(args);
        if (nl->IsEqual(arg, "vertex"))
        {
            return nl->SymbolAtom("point");
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


int graphkey(Word* args, Word& result, int message, Word& local, Supplier s)
{
    Vertex const * pVertex = static_cast<Vertex const *>(args[0].addr);
    result = qp->ResultStorage(s);
    CcInt* pRet = static_cast<CcInt *>(result.addr);
    pRet->Set(true, pVertex->GetKey());
    
    return 0;
}


int graphpos(Word* args, Word& result, int message, Word& local, Supplier s)
{
    Vertex const * pVertex = static_cast<Vertex const *>(args[0].addr);
    result = qp->ResultStorage(s);
    Point* pRet = static_cast<Point *>(result.addr);
    pRet->CopyFrom(&pVertex->GetPos());
    
    return 0;
}


string const keySpec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>vertex -> int</text--->"
    "<text>key ( _ )</text--->"
    "<text>the key of the vertex</text--->"
    "<text>key(v1)</text---> ) )";

string const posSpec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>vertex -> point</text--->"
    "<text>pos ( _ )</text--->"
    "<text>the position of the vertex</text--->"
    "<text>pos(v1)</text---> ) )";


/*
4.1 operator key

returns the key of the vertex

*/
Operator graph_key("key", keySpec, graphkey, Operator::SimpleSelect, 
    VertexIntTypeMap);

/*
4.2 operator pos

returns the position of the vertex

*/

Operator graph_pos("pos", posSpec, graphpos, Operator::SimpleSelect, 
    VertexPointTypeMap);

