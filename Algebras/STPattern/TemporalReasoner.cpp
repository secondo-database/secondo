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

*/

#include<TemporalReasoner.h>

namespace STP{

PointAlgebraReasoner::PointAlgebraReasoner(){
}

PointAlgebraReasoner::PointAlgebraReasoner(unsigned int _n):
    n(_n),
    Table(_n)
{
  for(unsigned int i=0; i<_n; ++i)
  {
    Table[i].resize(_n, uni);
    Table[i][i] = eql;
  }
  PARelation _plusTable[49]=
  {lss,lss,inc,inc,inc,lss,lss,
   lss,leq,inc,eql,eql,lss,leq,
   inc,inc,grt,grt,inc,grt,grt,
   inc,eql,grt,geq,eql,grt,geq,
   inc,eql,inc,eql,eql,inc,eql,
   lss,lss,grt,grt,inc,neq,neq,
   lss,leq,grt,geq,eql,neq,uni};

  PARelation _multTable[49]=
  {lss,lss,uni,uni,lss,uni,uni,
   lss,leq,uni,uni,leq,uni,uni,
   uni,uni,grt,grt,grt,uni,uni,
   uni,uni,grt,geq,geq,uni,uni,
   lss,leq,grt,geq,eql,neq,uni,
   uni,uni,uni,uni,neq,uni,uni,
   uni,uni,uni,uni,uni,uni,uni};

  int j,k;
  for(int i=0; i<49; ++i)
  {
    j= i/7; k= i%7;
    plusTable[j][k]= _plusTable[i];
    multTable[j][k]= _multTable[i];
  }
}
PointAlgebraReasoner::~PointAlgebraReasoner(){}

void PointAlgebraReasoner::Clear()
{
  this->n=0;
  this->Table.clear();
  this->Intervals.clear();
  while(!this->Queue.empty())
    this->Queue.pop();
}

void PointAlgebraReasoner::Add(int i, int j, PARelation Rij)
{
  PARelation oldRij= Table[i][j],
      newRij= plusTable[oldRij][Rij];
  if(newRij != oldRij)
  {
    Table[i][j]= newRij;
    Queue.push(make_pair<int, int>(i, j));
  }
  Intervals.insert(i);
  Intervals.insert(j);
}
bool PointAlgebraReasoner::Close()
{
  pair<int, int> ij;
  int signal=0;
  while(!Queue.empty())
  {
    ij= Queue.front();
    signal= Propagate(ij.first, ij.second);
    if(! signal) //contradiction (inconsistent network)
      return signal;
    Queue.pop();
  }
  return true;
}
bool PointAlgebraReasoner::Propagate(int i, int j)
{
  int k;
  PARelation Rij, Rik, Rjk, Rij_mul_Rjk, Rik_plus_Rij_mul_Rjk;
  PARelation Rki, Rkj, Rki_mul_Rij, Rkj_plus_Rki_mul_Rij;
  for(set<int>::iterator it= Intervals.begin(); it!= Intervals.end(); ++it)
  {
    k= *it;
    Rij= Table[i][j];  Rik= Table[i][k];  Rjk= Table[j][k];
    Rij_mul_Rjk= multTable[Rij][Rjk];
    Rik_plus_Rij_mul_Rjk= plusTable[Rik][Rij_mul_Rjk];
    if(Rik_plus_Rij_mul_Rjk == inc)
      return false; // signal contradiction
    if(Rik != Rik_plus_Rij_mul_Rjk)
    {
      Queue.push(make_pair<int, int>(i, k));
      Table[i][k] = Rik_plus_Rij_mul_Rjk;
    }


    Rki= Table[k][i];  Rkj=Table[k][j];
    Rki_mul_Rij= multTable[Rki][Rij];
    Rkj_plus_Rki_mul_Rij= plusTable[Rkj][Rki_mul_Rij];
    if(Rkj_plus_Rki_mul_Rij == inc)
      return false; // signal contradiction
    if(Rkj != Rkj_plus_Rki_mul_Rij)
    {
      Queue.push(make_pair<int, int>(k, j));
      Table[k][j]= Rkj_plus_Rki_mul_Rij;
    }
  }
  return true;
}

ostream& PointAlgebraReasoner::Print(ostream& os)
{
  string PAR[]= {"lss", "leq", "grt", "geq", "eql", "neq", "uni", "inc"};
  os<<'\t';
  for(unsigned int i=0; i<n; ++i)
    os<<"v"<<i<<'\t';
  os<<endl;
  for(unsigned int i=0; i<n; ++i)
  {
    os<<"v"<<i<<'\t';
    for(unsigned int j=0; j<n; ++j)
      os<<PAR[Table[i][j]]<<'\t';
    os<<endl;
  }
  return os;
}
vector<PARelation>& PointAlgebraReasoner::GetRelations(int varIndex)
{
  return Table[varIndex];

}

ListExpr PointAlgebraReasoner::ExportToNestedList()
{
/*
Yields a nested list with the format
(n (Table[0][0],..., Table[0][n-1]) (Table[1][0],..., Table[0][n-1])
(Table[n-1][0],..., Table[n-1][n-1]))

*/
  bool debugme=false;
  ListExpr outer, lastOuter, inner, lastInner;

  outer= nl->OneElemList(nl->IntAtom(n));
  lastOuter= outer;
  for(unsigned int i=0; i< this->n; ++i)
  {
    inner= nl->OneElemList(nl->IntAtom(Table[i][0]));
    lastInner= inner;
    for(unsigned int j=1; j<this->n; ++j)
      lastInner=
          nl->Append(lastInner, nl->IntAtom(static_cast<int>(Table[i][j])));
    lastOuter=
        nl->Append(lastOuter, inner);
  }
  if(debugme)
    cerr<<endl<<nl->ListLength(outer)<<nl->ToString(outer);
  return outer;
}

bool PointAlgebraReasoner::ImportFromNestedList(ListExpr& args)
{
/*
Yields a nested list with the format
(n Table[0][0],..., Table[0][n-1], Table[1][0],..., Table[n-1][n-1])

*/
//  Clear();
//  this->n = nl->First(args);
//  ListExpr list= nl->Rest(args);
//  if(n == 0) return false;
//
//  assert(nl->ListLength(list) == n * n);
//  for(unsigned int i=0; i<n; ++i)
//  {
//    for(unsigned int j=0; j<n; ++j)
//    {
//      Table[i][j]= static_cast<PARelation>(nl->IntValue(nl->First(list)));
//      list= nl->Rest(list);
//    }
//  }
  return true;
}

bool PointAlgebraReasoner::ImportFromArray(int* args)
{
/*
Yields a nested list with the format
(n Table[0][0],..., Table[0][n-1], Table[1][0],..., Table[n-1][n-1])

*/
  if(args[0] == 0) return false;
//  Clear();
//  this->n = args[0];
//  Table.resize(this->n);
//  for(unsigned int i=0; i<this->n; ++i)
//    Table[i].resize(this->n);

  for(unsigned int i=0; i<n; ++i)
    for(unsigned int j=0; j<n; ++j)
      Table[i][j]= static_cast<PARelation>(args[i*n + j +1]);
  return true;
}
};

