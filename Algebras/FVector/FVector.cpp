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
#include "FVector.h"
#include "Tools/Flob/DbArray.h"
#include "Tools/Flob/Flob.h"
#include "GenericTC.h"

using namespace std;


// constructors
FVector::FVector(){}
FVector::FVector(double dummy ):
    Attribute(true), content(0) {}

// copy constructor
FVector::FVector (const FVector &c) : Attribute(c.IsDefined()) ,
    content(c.content.Size()){
    content.copyFrom(c.content);
  }

// assignment operator
FVector& FVector::operator =( const FVector &src ){
  SetDefined (src.IsDefined());
  content.copyFrom(src.content);
  return *this;
}

// desctructor
FVector::~FVector (){}
void FVector::append(CcReal *i) {
  if (!i->IsDefined ()){
      SetDefined(false);
  } else if (IsDefined()){
  content.Append(i->GetValue());
  }
}

void FVector::append(double i) {
  content.Append(i);
}

int FVector::getDim() const{
  return content.Size();
}

double FVector::getElem(int index) const{
  assert (index<content.Size());
  double elem;
  if (content.Get(index,&elem)) {
    return elem;
  }
  else { //should not happen
    return 0.0;
  }
}



// compare
int FVector::Compare( const Attribute *arg ) const {
  if (!IsDefined()){
    return arg->IsDefined()? -1:0;
  }
  if (!arg->IsDefined()){
    return 1;
  }
  FVector* i = (FVector*) arg ;
  // first criterion number of entries
  if (content.Size () < i->content.Size ()){
    return -1;
  }
  if (content.Size () < i->content.Size ()){
    return 1;
  }
  for (int k =0; k<content.Size (); k ++){
    double i1 , i2;
    content.Get(k , i1);
    i->content.Get(k , i2);
    if ( i1 < i2 ) return -1;
    if ( i1 > i2 ) return 1;
  }
  return 0;
}

// there is no meaningful Adjacent implementation
bool FVector::Adjacent(const Attribute *arg ) const {
  return false;
}

// standard implementation of Sizeof
size_t FVector::Sizeof() const {
  return sizeof(*this);
}

// hash function
size_t FVector::HashValue() const {
  if (!IsDefined()){
    return 0;
  }
  // sum up the first 5 elements
  double sum = 0;
  int max = min (5 ,content.Size());
  for (int i =0; i<max ; i ++){
    double v;
    content.Get(i , v);
    sum += v;
  }

  return (size_t) sum ;
}

//copyfrom
void FVector::CopyFrom( const Attribute *arg ){
  *this = *(( FVector*) arg );
}

//clone
Attribute* FVector::Clone() const {
  return new FVector(*this);
}

// replacement for the IN function
bool FVector::ReadFrom( ListExpr LE , const ListExpr typeInfo ){
  // handle undefined value
  if (listutils :: isSymbolUndefined (LE)){
    SetDefined (false);
    return true ;
  }
  if (nl->AtomType(LE)!= NoAtom ){
    return false;
  }
  SetDefined (true);
  content.clean();
  while (!nl->IsEmpty(LE)){
    ListExpr f = nl->First(LE);
    LE = nl->Rest(LE);
    if (nl->AtomType(f)!= RealType ){
      return false;
    }
    append(nl->RealValue(f));
  }
  return true ;
}

// replacement for the out function
ListExpr FVector::ToListExpr(ListExpr typeInfo ) const {
  if (!IsDefined()){
    return listutils::getUndefined();
  }
  if (content.Size()==0){
    return nl->TheEmptyList();
  }
  double v;
  content.Get(0 , v);
  ListExpr res = nl->OneElemList(nl->RealAtom(v));
  ListExpr last = res ;
  for (int i =1; i < content.Size(); i ++){
    content.Get(i , v );
    last = nl->Append(last,nl->RealAtom(v));
  }
  return res;
}


