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

[1] Implementation of the class Path

[TOC]

1 Overview

This implementation file contains the implementation of the class Path

*/


#include "GraphAlgebra.h"


/*
2  class EdgeDirection

this local class is needed to check for parallel edges in a path

*/


class EdgeDirection
{
public:
    explicit EdgeDirection(int nSource, int nTarget) 
        : m_nSource(nSource), m_nTarget(nTarget){}
    
    int GetSource() const{return m_nSource;}
    int GetTarget() const{return m_nTarget;}
    
    bool operator<(EdgeDirection const & edge) const
    {
        return m_nSource < edge.GetSource() || 
            m_nSource == edge.GetSource() && m_nTarget < edge.GetTarget();
    }

private:
    int m_nSource;
    int m_nTarget;
};


/*
3 Implementation of class Path

*/


Path::Path()
{
}

Path::Path(bool bDefined)
    : myPath(0), cost(0.0), defined(bDefined)
{
}

Path::~Path()
{
}

void Path::SetDefined(bool def)
{
    defined=def;
}


bool Path::IsDefined()const
{
    return(defined);
}

        
float Path::GetCost() const 
{ 
    if (!defined || (myPath.Size() == 0)) 
        return -1.0f; 

    const pathStruct* s; 
    
    myPath.Get(myPath.Size()-1,s); 
    return cost-s->cost; 
};


Path* Path::Clone() const 
{ 
    Path* p = new Path(defined);
    for (int i = 0; i < GetNoPathStructs(); ++i)
    {
        p->Append(*GetPathStruct(i));
    }
    p->cost = cost;
    return p;
}

size_t Path::Sizeof() const
{
    return sizeof(*this);
}

/*
~EqualWay~

This function checks the equality of this path with the
argument with respect to the key in the path, i.e. ignoring
costs and positions.

*/

void Path::EqualWay(const Path* p, CcBool& result) const{
  if(!IsDefined() || !p->IsDefined()){
      result.SetDefined(false);
      return; 
  }
  int mysize = this->GetNoPathStructs();
  int psize = p->GetNoPathStructs();
  if(mysize!=psize){
     result.Set(true,false);
     return;
  }
  const pathStruct* mps = 0;
  const pathStruct* pps = 0;
  for(int i=0; i<mysize;i++){
     this->myPath.Get(i,mps);
     p->myPath.Get(i,pps);
     if(mps->key != pps->key){
        result.Set(true,false);
        return;
     }  
  }
  result.Set(true,true);
}

int Path::Compare(const Attribute* arg) const
{ 
    Path const * p = dynamic_cast<Path const *>(arg);
    if(!defined || !p->defined){
      return -1; 
    }
    // first criterion is the length of the path
    int mlength = myPath.Size();
    int plength = p->myPath.Size();
    if(mlength<plength){
       return -1;
    }
    if(mlength>plength){
       return 1;
    }
    // second criterion are the costs
    if(this->cost < p->cost){
       return -1;
    }
    if(this->cost > p->cost){
       return 1;
    }

    // both paths have the same length
    // and the same costs
    // scan both path in parallel
    const pathStruct* mps=0;
    const pathStruct* pps=0;
    for(int i=0;i<mlength;i++){
       myPath.Get(i,mps);
       p->myPath.Get(i,pps);
       if(mps->key < pps->key){
          return -1;
       }
       if(mps->key > pps->key){
          return 1;
       }
       int cmp = mps->pos.Compare(&(pps->pos));
       if(cmp!=0){
          return cmp;
       }
       if(!AlmostEqual(mps->cost,pps->cost)){
          if(mps->cost < pps->cost){
             return -1;
          } else {
             return 1;
          }
       }
    }
    return 0;
}


bool Path::Adjacent(const Attribute*) const
{
    return false;
}

size_t Path::HashValue() const
{
    size_t nRet = 0;
    if (defined)
    {
        int nCount = GetNoPathStructs();
        for (int i = 0; i < nCount; ++i)
        {
            pathStruct const * p = GetPathStruct(i);
            nRet += p->key;
            nRet += *(reinterpret_cast<size_t const *>(&p->cost));
        }
    }
    return 0;
}

void Path::CopyFrom(const StandardAttribute* arg)
{
    Path const * pArg = dynamic_cast<Path const *>(arg);
    myPath.Clear();
    int nCount = pArg->GetNoPathStructs();
    for (int i = 0; i < nCount; ++i)
    {
        Append(*pArg->GetPathStruct(i));
    }
        cost = pArg->cost;
        defined = pArg->IsDefined(); 
}


int Path::NumOfFLOBs() const
{
    return 1;    
}

FLOB* Path::GetFLOB(const int i)
{
    return i == 0 ? &myPath : NULL;
}

pathStruct const * Path::GetPathStruct(int nIndex) const
{
    pathStruct const * pRet = NULL;
    if (nIndex >= 0 && nIndex < GetNoPathStructs())
    {
        myPath.Get(nIndex, pRet);
    }
    
    return pRet;
}

/*
3.1.1 Function for Operator ~edges~ of GraphAlgebra

*/
vector<Edge>* Path::GetEdges() const
{
    vector<Edge>* vEdges = new vector<Edge>(0);
    pathStruct const * pStruct = NULL;
    int source;
    int target;
    float cost;

    if (myPath.Size() < 2)
        return vEdges; 
    pStruct = GetPathStruct(0);
    source = pStruct->key;
    cost = pStruct->cost;
    for (int i = 1; i < myPath.Size(); i++) {
        pStruct = GetPathStruct(i);
        target = pStruct->key;
        vEdges->push_back(Edge(source,target,cost));
        source = target;
        cost = pStruct->cost;
    }

    return vEdges;
}


/*
3.1.2 Function for Operator ~vertices~ of GraphAlgebra

*/

vector<Vertex>* Path::GetVertices() const
{

    vector<Vertex>* vVertices = new vector<Vertex>(0);
    pathStruct const * pStruct = NULL;

    for (int i = 0; i < myPath.Size(); i++) {
        pStruct = GetPathStruct(i);
        vVertices->push_back(Vertex(pStruct->key, pStruct->pos));
    }

    return vVertices;
}



/*
4 Additional functions needed to define the type path

*/

void* CastPath (void* addr)
{
    return (new (addr) Path);
}

ListExpr OutPath( ListExpr typeInfo, Word value )
{      
    Path const * pPath = static_cast<Path const *>(value.addr);
    if (pPath->IsDefined())
    {
        int nCount = pPath->GetNoPathStructs();
        if (nCount == 0)
        {
            return nl->TheEmptyList();
        }
        else
        {
            
            pathStruct const * pStruct = pPath->GetPathStruct(0);
            ListExpr result;
            if (pStruct->pos.IsDefined())
            {
                result = nl->OneElemList(nl->TwoElemList(
                    nl->IntAtom(pStruct->key), nl->TwoElemList(
                    nl->RealAtom(pStruct->pos.GetX()), 
                    nl->RealAtom(pStruct->pos.GetY()))));
            }
            else
            {
                result = nl->OneElemList(nl->TwoElemList(
                    nl->IntAtom(pStruct->key), nl->SymbolAtom("undef")));
            }
            
            ListExpr last = result;
            for (int i = 1; i < nCount; ++i)
            {
                last = nl->Append(last, nl->RealAtom(pStruct->cost));
                pStruct = pPath->GetPathStruct(i);
                if (pStruct->pos.IsDefined())
                {
                    last = nl->Append(last, nl->TwoElemList(
                        nl->IntAtom(pStruct->key), nl->TwoElemList(
                        nl->RealAtom(pStruct->pos.GetX()), 
                        nl->RealAtom(pStruct->pos.GetY()))));
                }
                else
                {
                    last = nl->Append(last, nl->TwoElemList(
                        nl->IntAtom(pStruct->key), nl->SymbolAtom("undef")));
                }
            }
            
            return result;
        }
    }
    else
    {
        return nl->SymbolAtom("undef");
    }
    
}

Word InPath( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{    
    Path* pPath = new Path(true);
    correct = true;
    if (nl->IsAtom(instance) && nl->AtomType(instance) == SymbolType && 
        nl->SymbolValue(instance) == "undef")
    {
        pPath->SetDefined(false);
    }
    else if (!nl->IsEmpty(instance))
    {
        map<EdgeDirection, float> mapUsedEdges;
        map<int, Point> mapUsedVertices;
        
        pathStruct ps;
        ListExpr first = nl->First(instance);
        ListExpr rest = nl->Rest(instance);
        
        if (nl->ListLength(first) == 2)
        {
            ListExpr First = nl->First(first);
            ListExpr Second = nl->Second(first);
            if (nl->IsAtom(First) && nl->AtomType(First) == IntType)
            {
                ps.key = nl->IntValue(First);
                if (nl->ListLength(Second) == 2)
                {
                    First = nl->First(Second);
                    Second = nl->Second(Second);
                    if (nl->IsAtom(First) 
                        && nl->AtomType(First) == RealType && 
                        nl->IsAtom(Second) && 
                        nl->AtomType(Second) == RealType)
                    {
                        ps.pos = Point(true, nl->RealValue(First), 
                            nl->RealValue(Second));
                    }
                    else
                    {
                        correct = false;
                    }
                }
                else if (nl->IsAtom(Second) && 
                    nl->AtomType(Second) == SymbolType && 
                    nl->SymbolValue(Second) == "undef")
                {
                    ps.pos = Point(false);
                }
                else
                {
                    correct = false;
                }
            }
            else
            {
                correct = false;
            }
        }
        else
        {
            correct = false;
        }
        
        mapUsedVertices.insert(pair<int, Point>(ps.key, ps.pos));
        
        while (!nl->IsEmpty(rest) && correct)
        {
            first = nl->First(rest);
            rest = nl->Rest(rest);
            
            if (nl->IsAtom(first) && nl->AtomType(first) == RealType && 
                !nl->IsEmpty(rest))
            {
                ps.cost = nl->RealValue(first);
                pPath->Append(ps);
            }
            else
            {
                correct = false;
            }
            
            if (ps.cost < 0.0)
            {
                cout << "Negative costs are not allowed!" << endl;
                correct = false;
            }
            
            if (correct)
            {
                first = nl->First(rest);
                rest = nl->Rest(rest);
                if (nl->ListLength(first) == 2)
                {
                    ListExpr First = nl->First(first);
                    ListExpr Second = nl->Second(first);
                    if (nl->IsAtom(First) && nl->AtomType(First) == IntType)
                    {
                        int nLastKey = ps.key;
                        ps.key = nl->IntValue(First);
                        EdgeDirection edge(nLastKey, ps.key);
                        map<EdgeDirection, float>::const_iterator itEdge = 
                            mapUsedEdges.find(edge);
                        if (itEdge == mapUsedEdges.end())
                        {
                            mapUsedEdges.insert(
                                pair<EdgeDirection, float>(edge, ps.cost));
                        }
                        else if (ps.cost != (*itEdge).second)
                        {
                            cout << "Parallel borders are not allowed" << endl;
                            correct = false;
                        }
                        
                        if (nl->ListLength(Second) == 2)
                        {
                            First = nl->First(Second);
                            Second = nl->Second(Second);
                            if (nl->IsAtom(First) && 
                                nl->AtomType(First) == RealType && 
                                nl->IsAtom(Second) && 
                                nl->AtomType(Second) == RealType)
                            {
                                ps.pos = Point(true, nl->RealValue(First), 
                                    nl->RealValue(Second));
                            }
                            else
                            {
                                correct = false;
                            }
                        }
                        else if (nl->IsAtom(Second) && 
                            nl->AtomType(Second) == SymbolType && 
                            nl->SymbolValue(Second) == "undef")
                        {
                            ps.pos = Point(false);
                        }
                        else
                        {
                            correct = false;
                        }
                        
                        if (correct)
                        {
                            map<int, Point>::const_iterator itVertex = 
                                mapUsedVertices.find(ps.key);
                            if (itVertex == mapUsedVertices.end())
                            {
                                mapUsedVertices.insert(
                                    pair<int, Point>(ps.key, ps.pos));
                            }
                            else
                            {
                                Point const & rPos2 = (*itVertex).second;
                                if (ps.pos.IsDefined())
                                {
                                    if (rPos2.IsDefined())
                                    {
                                        correct = ps.pos == rPos2;
                                    }
                                    else
                                    {
                                        correct = false;
                                    }
                                }
                                else if (rPos2.IsDefined())
                                {
                                    correct = false;
                                }
                                if (!correct)
                                {
                                    cout << "Vertex " << ps.key << 
                                        " mustn't have different positions!" 
                                        << endl;
                                }
                            }
                        }
                    }
                    else
                    {
                        correct = false;
                    }
                }
                else
                {
                    correct = false;
                }
            }
        }
        
        //Append the last vertex with cost 0.0
        ps.cost = 0.0;
        pPath->Append(ps);
    }
    
    if (!correct)
    {
        delete pPath;
        pPath = NULL;
    }

    return SetWord(pPath);
}

ListExpr PathProperty()
{
    return (nl->TwoElemList(
        nl->FiveElemList(nl->StringAtom("Signature"),
            nl->StringAtom("Example Type List"),
            nl->StringAtom("List Rep"),
            nl->StringAtom("Example List"),
            nl->StringAtom("Remarks")),
        nl->FiveElemList(nl->StringAtom("-> DATA"),
            nl->StringAtom("path"),
            nl->StringAtom("(<fromV> <cost> <toV> ... <cost> <toV>)"),
            nl->StringAtom("((1 (1.0 2.0)) 0.5 (2 (2.0 3.0)))"),
            nl->StringAtom("fromV, toV: vertex, cost: float"))));
}

Word CreatePath( const ListExpr typeInfo )
{    
    return SetWord(new Path(true));
}

void DeletePath( const ListExpr typeInfo, Word& w )
{
    Path * pPath = static_cast<Path *>(w.addr);
    pPath->Destroy();
    pPath->DeleteIfAllowed();
    w.addr = 0;

}

void ClosePath( const ListExpr typeInfo, Word& w )
{
    static_cast<Path *>(w.addr)->DeleteIfAllowed();
    w.addr = 0;
    
}

Word ClonePath( const ListExpr typeInfo, const Word& w )
{
    return SetWord((static_cast<Path const *>(w.addr))->Clone());
}

int SizeofPath()
{
    return sizeof(Path);
}


bool CheckPath( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual(type, "path"));
}

bool OpenPath(SmiRecord& valueRecord, size_t& offset, 
    const ListExpr typeInfo, Word& value)
{
    value = SetWord(Attribute::Open(valueRecord, offset, typeInfo));
    return true;
}


bool SavePath( SmiRecord& valueRecord, size_t& offset, 
    const ListExpr typeInfo, Word& value)
{
    Attribute::Save(valueRecord, offset, typeInfo, 
        static_cast<Attribute*>(value.addr));
    return true;
}



TypeConstructor pathCon(
    "path",                 //name
    PathProperty,           //property function describing signature
    OutPath, InPath,        //Out and In functions
    0, 0,                   //SaveToList and RestoreFromList functions
    CreatePath, DeletePath, //object creation and deletion
    OpenPath, SavePath,     //object open, save
    ClosePath, ClonePath,   //object close, and clone
    CastPath,               //cast function
    SizeofPath,             //sizeof function
    CheckPath);             //kind checking function

