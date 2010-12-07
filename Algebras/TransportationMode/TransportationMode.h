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

[1] Header File of the Transportation Mode Algebra

August, 2009 Jianqiu Xu
March, 2010 Jianqiu xu

[TOC]

1 Overview

2 Defines and includes

*/

#ifndef TransportationMode_H
#define TransportationMode_H


#include "Algebra.h"

#include "NestedList.h"

#include "QueryProcessor.h"
#include "RTreeAlgebra.h"
#include "BTreeAlgebra.h"
#include "TemporalAlgebra.h"
#include "StandardTypes.h"
#include "LogMsg.h"
#include "NList.h"
#include "RelationAlgebra.h"
#include "ListUtils.h"
#include "NetworkAlgebra.h"
#include "FTextAlgebra.h"
#include <fstream>


Word InHalfSegment( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr OutHalfSegment(ListExpr typeInfo, Word value);

///////////////////////////////////////////////////////////////////////////
////////////////////////// GenRange ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/*
data structure for general location: a set of possible locations 

*/
class GenRange:public StandardSpatialAttribute<2>{
public:  
    /////////// it will call the default constructor of line   ////////////
    /////////////     In SpatialAlgebra   //////////////////////////////
    //////// inline Line::Line(){}  needs to be public instead of protected//
    ////////// Cast function will this constructor  //////////////////////
    ////////////////////////////////////////////////////////////////////////
    GenRange(){}
    GenRange(const unsigned int o):
    StandardSpatialAttribute<2>(true),oid(o), loc_range(0)
    {
//      cout<<"Constructor1()"<<endl;
    }
    
  inline GenRange(unsigned int o, Line& l):
  StandardSpatialAttribute<2>(true), oid(o), loc_range(l){}
  
  GenRange(const GenRange& gr):
  StandardSpatialAttribute<2>(gr.IsDefined()), 
  oid(gr.GetObjId()),loc_range(*(gr.GetLine())){}

  ~GenRange()
  {

  }
  void SetValue(unsigned int o, Line* l)
  {
      oid = o;
      loc_range = *l;
      SetDefined(true);
  }
   
  unsigned int GetObjId() const {return oid;}
  const Line* GetLine() const {
    const Line* p_to_l = &loc_range;
    if(loc_range.IsDefined()) return p_to_l;
     else return NULL;
  }
  GenRange(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo);
  bool Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo);
  inline size_t Sizeof() const{return sizeof(*this);}
  inline int Size() const {return loc_range.Size();}
  inline bool IsEmpty() const{return !IsDefined() || Size() == 0;}
  int Compare(const Attribute* arg) const
  {
      return 0;
  }
  inline bool Adjacent(const Attribute* arg)const{return false;}
  GenRange* Clone() const {return new GenRange(*this);}
  size_t HashValue() const
  {
    return loc_range.HashValue();
  }
  void CopyFrom(const Attribute* right)
  {
      *this = *(const GenRange*)right;
  }
  const Rectangle<2> BoundingBox() const
  {
      return loc_range.BoundingBox();
  }
  double Distance(const Rectangle<2>& r)const
  {
      return loc_range.BoundingBox().Distance(r);
  }
    
  static void* Cast(void* addr){return new (addr)GenRange();}
  /////////////very important two functions////////////////////
  ////////especially genrange is an attribute in a relation/////
  inline int NumOfFLOBs() const { 
//    cout<<"NumOfFLOBs "<<loc_range.NumOfFLOBs()<<endl;
    return loc_range.NumOfFLOBs();
  }
  inline Flob* GetFLOB(const int i) { 
//    cout<<"GetFLOB"<<endl; 
    return loc_range.GetFLOB(i);
  }
  /////////////////////////////////////////////////////////////////

  unsigned int oid;
  Line loc_range;

};


/*
Constructor function for GenRange

*/
GenRange::GenRange(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo):
StandardSpatialAttribute<2>(true),loc_range(0)
{
//  cout<<"Floor3D(SmiRecord& , size_t& , const ListExpr) here"<<endl;
  valueRecord.Read(&oid, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);
  ListExpr xType = nl->SymbolAtom("line");
  ListExpr xNumericType =
    SecondoSystem::GetCatalog()->NumericType(xType);
  Line* l = (Line*)Attribute::Open(valueRecord,offset,xNumericType);
  loc_range = *l;
//  cout<<oid<<*l<<endl; 
  delete l;
  SetDefined(true);
}

bool GenRange::Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo)
{
//  cout<<"Save()"<<endl;
  valueRecord.Write(&oid, sizeof(unsigned int),offset);
  offset += sizeof(unsigned int);
  ListExpr xType = nl->SymbolAtom("line");
  ListExpr xNumericType =
    SecondoSystem::GetCatalog()->NumericType(xType);
  Attribute::Save(valueRecord, offset, xNumericType, &loc_range);  
  return true;
}

/////////////////////////////////////////////////////////////////////////
//////////////////////// Gen Range  /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

ListExpr GenRangeProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("genrange"),
           nl->StringAtom("(<genrange>) (int,line)"),
           nl->StringAtom("(1 [const line value ((10 1 5 3))])"))));
}

ListExpr OutGenRange( ListExpr typeInfo, Word value )
{
//  cout<<"OutGenRange"<<endl; 
  GenRange* gen_range = (GenRange*)(value.addr);
  if(!gen_range->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  if( gen_range->IsEmpty() ){
    return nl->TheEmptyList();
  }
  
  ListExpr lineNL, last;
  Line* l = const_cast<Line*>(gen_range->GetLine());
  if(!l->IsDefined() || l->IsEmpty())
    lineNL = nl->TheEmptyList();
  else{
    lineNL = nl->TheEmptyList();
    last = lineNL;
    bool first = true;
    HalfSegment hs;
    ListExpr halfseg, halfpoints, flatseg;
    for( int i = 0; i < l->Size(); i++ ){
      l->Get( i, hs );
      if( hs.IsLeftDomPoint() == true ){
        halfseg = OutHalfSegment( nl->TheEmptyList(), SetWord( (void*)&hs));
        halfpoints = nl->Second( halfseg );
        flatseg = nl->FourElemList(
                  nl->First( nl->First( halfpoints ) ),
                  nl->Second( nl->First( halfpoints ) ),
                  nl->First( nl->Second( halfpoints ) ),
                  nl->Second( nl->Second( halfpoints ) ) );
        if( first == true ){
          lineNL = nl->OneElemList( flatseg );
          last = lineNL;
          first = false;
        }
        else
          last = nl->Append( last, flatseg );
      }
    }
  }
  
  return nl->TwoElemList(nl->IntAtom(gen_range->GetObjId()), lineNL);
  
}


/*
In function

*/
Word InGenRange( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
//  cout<<"Ingenrange"<<endl; 
  if(nl->IsEqual(instance,"undef")) {
      GenRange* gr = new GenRange(0);
      gr->SetDefined(false);
      correct=true;
      return SetWord( Address(gr) );
  }
 
 
  ListExpr oid_list = nl->First(instance);

  if(!nl->IsAtom(oid_list) || nl->AtomType(oid_list) != IntType){
    string strErrorMessage = "genrange(): obj id must be int type";
    errorInfo = nl->Append(errorInfo,nl->StringAtom(strErrorMessage));
    correct = false;
    return SetWord(Address(0));
  }
  unsigned int oid = nl->IntValue(oid_list);

  ListExpr LineNL = nl->Second(instance);
  
  
  Line* l = new Line( 0 );

  HalfSegment * hs;
  l->StartBulkLoad();
  ListExpr first, halfseg, halfpoint;
  ListExpr rest = instance;
  int edgeno = 0;

  while( !nl->IsEmpty( rest )){
      first = nl->First( LineNL );
      rest = nl->Rest( LineNL );

      if( nl->ListLength( first ) == 4 )
      {
        halfpoint = nl->TwoElemList(
                      nl->TwoElemList(
                        nl->First(first),
                        nl->Second(first)),
                      nl->TwoElemList(
                        nl->Third(first),
                        nl->Fourth(first)));
      } else { // wrong list representation
         l->DeleteIfAllowed();
         correct = false;
         return SetWord( Address(0) );
      }
      halfseg = nl->TwoElemList(nl->BoolAtom(true), halfpoint);
      hs = (HalfSegment*)InHalfSegment( nl->TheEmptyList(), halfseg,
                                        0, errorInfo, correct ).addr;
      if( correct )
      {
        hs->attr.edgeno = edgeno++;
        *l += *hs;
        hs->SetLeftDomPoint( !hs->IsLeftDomPoint() );
        *l += *hs;
      }
      delete hs;
  }
  l->EndBulkLoad();

  correct = true;
  GenRange* gr = new GenRange(oid, *l);
// cout<<"oid "<<oid<<"line "<<*l<<endl; 
  delete l; 
  return SetWord(gr);

}


Word CreateGenRange(const ListExpr typeInfo)
{
//  cout<<"CreateGenRange()"<<endl;
  return SetWord (new GenRange(0));
}


void DeleteGenRange(const ListExpr typeInfo, Word& w)
{
//  cout<<"GenRange()"<<endl;
  GenRange* gr = (GenRange*)w.addr;
  delete gr;
   w.addr = NULL;
}

bool OpenGenRange(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenGenRange()"<<endl;
  value.addr = new GenRange(valueRecord, offset, typeInfo);
  return value.addr != NULL;

}

/*
save function for GenRange 

*/
bool SaveGenRange(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//    cout<<"SaveGenRange()"<<endl;
    GenRange* gr = (GenRange*)value.addr;
    return gr->Save(valueRecord, offset, typeInfo);  
}

void CloseGenRange( const ListExpr typeInfo, Word& w )
{
//  cout<<"CloseGenRange()"<<endl; 
  ((GenRange *)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word CloneGenRange( const ListExpr typeInfo, const Word& w )
{
//  cout<<"CloneGenRange()"<<endl; 
  return SetWord( new GenRange( *((GenRange *)w.addr) ) );
}

int SizeOfGenRange()
{
  return sizeof(GenRange);
}

bool CheckGenRange( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "genrange" ));
}

#endif

