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

Jan, 2011 Jianqiu xu

[TOC]

1 Overview

2 Defines and includes

*/


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
#include "GeneralType.h"

////////////////////////////////////////////////////////////////////////////
//////////////////// Data Type: IORef ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

ListExpr IORefProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("ioref"),
           nl->StringAtom("(<oid, symbol>) (int string)"),
           nl->StringAtom("((1 \"mpptn\" ))"))));
}


/*
Output  (oid, symbol) 

*/

ListExpr OutIORef( ListExpr typeInfo, Word value )
{
//  cout<<"OutGenRange"<<endl; 
  IORef* ref = (IORef*)(value.addr);
  if(!ref->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  if( ref->IsEmpty() ){
    return nl->TheEmptyList();
  }
  return nl->TwoElemList(nl->IntAtom(ref->GetOid()), 
                         nl->StringAtom(GetSymbolStr(ref->GetLabel())));
}


/*
In function

*/
Word InIORef( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if(nl->IsEqual(instance,"undef")) {
      IORef* ref = new IORef();
      ref->SetDefined(false);
      correct=true;
      return SetWord(Address(ref));
  }
  
  
//  cout<<"length "<<nl->ListLength(instance)<<endl;

  if( !nl->IsAtom( instance ) ){
    IORef* ref = new IORef();
    if(nl->ListLength(instance) != 2){
      cout<<"length should be 2"<<endl; 
      correct = false;
      return SetWord(Address(0));
    }
    ListExpr first = nl->First(instance);
    if(!nl->IsAtom(first) || nl->AtomType(first) != IntType){
      cout<< "ioref(): oid must be int type"<<endl;
      correct = false;
      return SetWord(Address(0));
    }
    unsigned int oid = nl->IntValue(first);
    
    ListExpr second = nl->Second(instance);
    if(!nl->IsAtom(second) || nl->AtomType(second) != StringType){
      cout<< "ioref(): loc.loc2 must be string type"<<endl;
      correct = false;
      return SetWord(Address(0));
    }
    string s = nl->StringValue(second);
   ////////////////very important /////////////////////////////
    correct = true; 
  ///////////////////////////////////////////////////////////
    ref->SetValue(oid, s);
    return SetWord(ref);
  }

  correct = false;
  return SetWord(Address(0));
}



/*
Open an reference object 

*/
bool OpenIORef(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenGenLoc()"<<endl; 

  IORef* ref = (IORef*)Attribute::Open(valueRecord, offset, typeInfo);
  value = SetWord(ref);
  return true; 
}

/*
Save an reference object 

*/
bool SaveIORef(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveIORef"<<endl; 
  IORef* ref = (IORef*)value.addr; 
  Attribute::Save(valueRecord, offset, typeInfo, ref);
  return true; 
}

Word CreateIORef(const ListExpr typeInfo)
{
// cout<<"CreateIORef()"<<endl;
  return SetWord (new IORef(false, 0, 0));
}


void DeleteIORef(const ListExpr typeInfo, Word& w)
{
// cout<<"DeleteIORef()"<<endl;
  IORef* ref = (IORef*)w.addr;
  delete ref;
   w.addr = NULL;
}


void CloseIORef( const ListExpr typeInfo, Word& w )
{
//  cout<<"CloseIORef"<<endl; 
  ((IORef*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word CloneIORef( const ListExpr typeInfo, const Word& w )
{
//  cout<<"CloneGenLoc"<<endl; 
  return SetWord( new IORef( *((IORef*)w.addr) ) );
}

int SizeOfIORef()
{
//  cout<<"SizeOfIORef"<<endl; 
  return sizeof(IORef);
}

bool CheckIORef( ListExpr type, ListExpr& errorInfo )
{
//  cout<<"CheckIORef"<<endl; 
  return (nl->IsEqual( type, "ioref" ));
}
/////////////////////////////////////////////////////////////////////////////
//////////////////////Data Type: GenLoc//////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

ListExpr GenLocProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("genloc"),
           nl->StringAtom("(<oid,loc>) (int (real, real))"),
           nl->StringAtom("((1 (10.0 1.0 )))"))));
}

/*
Output  (oid, loc)

*/

ListExpr OutGenLoc( ListExpr typeInfo, Word value )
{
//  cout<<"OutGenLoc"<<endl; 
  GenLoc* genl = (GenLoc*)(value.addr);
  if(!genl->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  if( genl->IsEmpty() ){
    return nl->TheEmptyList();
  }
  
  Loc loc = genl->GetLoc();
  ListExpr loc_list = nl->TwoElemList(
                    nl->RealAtom(loc.loc1),
                    nl->RealAtom(loc.loc2));
  return nl->TwoElemList(nl->IntAtom(genl->GetOid()), loc_list);
}


/*
In function

*/
Word InGenLoc( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if(nl->IsEqual(instance,"undef")) {
      GenLoc* genl = new GenLoc();
      genl->SetDefined(false);
      correct=true;
      return SetWord(Address(genl));
  }
  
  
//  cout<<"length "<<nl->ListLength(instance)<<endl;

  if( !nl->IsAtom( instance ) ){
    GenLoc* genl = new GenLoc();
    if(nl->ListLength(instance) != 2){
      cout<<"length should be 2"<<endl; 
      correct = false;
      return SetWord(Address(0));
    }
    ListExpr first = nl->First(instance);
    if(!nl->IsAtom(first) || nl->AtomType(first) != IntType){
      cout<< "genloc(): oid must be int type"<<endl;
      correct = false;
      return SetWord(Address(0));
    }
    unsigned int oid = nl->IntValue(first);
    ListExpr second = nl->Second(instance);
    if(nl->ListLength(second) != 2){
      cout<<"length should be 2"<<endl; 
      correct = false;
      return SetWord(Address(0));
    }
    
    ListExpr list_l1 = nl->First(second);
    if(!nl->IsAtom(list_l1) || nl->AtomType(list_l1) != RealType){
      cout<< "genloc(): loc.loc1 must be real type"<<endl;
      correct = false;
      return SetWord(Address(0));
    }
    double loc1 = nl->RealValue(list_l1);
    
    ListExpr list_l2 = nl->Second(second);
    if(!nl->IsAtom(list_l2) || nl->AtomType(list_l2) != RealType){
      cout<< "genloc(): loc.loc2 must be real type"<<endl;
      correct = false;
      return SetWord(Address(0));
    }
    double loc2 = nl->RealValue(list_l2);
    Loc loc(loc1,loc2);
   ////////////////very important /////////////////////////////
    correct = true; 
  ///////////////////////////////////////////////////////////
    genl->SetValue(oid, loc);
    return SetWord(genl);
  }

  correct = false;
  return SetWord(Address(0));
}



/*
Open a GenLoc object 

*/
bool OpenGenLoc(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenGenLoc()"<<endl; 

  GenLoc* genl = (GenLoc*)Attribute::Open(valueRecord, offset, typeInfo);
  value = SetWord(genl);
  return true; 
}

/*
Save a GenLoc object 

*/
bool SaveGenLoc(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveGenRange"<<endl; 
  GenLoc* genl = (GenLoc*)value.addr; 
  Attribute::Save(valueRecord, offset, typeInfo, genl);
  return true; 
  
}

Word CreateGenLoc(const ListExpr typeInfo)
{
// cout<<"CreateLoc()"<<endl;
  return SetWord (new GenLoc(0));
}


void DeleteGenLoc(const ListExpr typeInfo, Word& w)
{
// cout<<"DeleteGenLoc()"<<endl;
  GenLoc* genl = (GenLoc*)w.addr;
  delete genl;
   w.addr = NULL;
}


void CloseGenLoc( const ListExpr typeInfo, Word& w )
{
//  cout<<"CloseGenLoc"<<endl; 
  ((GenLoc*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word CloneGenLoc( const ListExpr typeInfo, const Word& w )
{
//  cout<<"CloneGenLoc"<<endl; 
  return SetWord( new GenLoc( *((GenLoc*)w.addr) ) );
}

int SizeOfGenLoc()
{
//  cout<<"SizeOfGenLoc"<<endl; 
  return sizeof(GenLoc);
}

bool CheckGenLoc( ListExpr type, ListExpr& errorInfo )
{
//  cout<<"CheckGenLoc"<<endl; 
  return (nl->IsEqual( type, "genloc" ));
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////Data Type: GenRange///////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*
Add new elements into the array (oid,l)

*/
void GenRange::Add(unsigned int id, Line* l, int s)
{
  GenRangeElem* grelem = new GenRangeElem(id, seglist.Size(), l->Size(), s);
  elemlist.Append(*grelem);
  delete grelem; 
  for(int i = 0;i < l->Size();i++){
    HalfSegment hs;
    l->Get(i, hs);
    seglist.Append(hs);
  }
}

/*
Output a list of elements (oid, l)

*/

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

  ListExpr result = nl->TheEmptyList();
  ListExpr last = result;
  bool first = true;
  ListExpr sb_list; 
  for( int i = 0; i < gen_range->Size(); i++ ){
    Line l(0);
    GenRangeElem grelem;
    gen_range->Get( i, grelem, l);

    sb_list = nl->ThreeElemList(nl->IntAtom(grelem.oid),
                  OutLine(nl->TheEmptyList(), SetWord(&l)),
                                nl->StringAtom(GetTMStr(grelem.tm)));

    if( first == true ){
        result = nl->OneElemList( sb_list );
        last = result;
        first = false;
    }else
      last = nl->Append( last, sb_list);
  }
  return result;
}

/*
In function

*/
Word InGenRange( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if(nl->IsEqual(instance,"undef")) {
      GenRange* gr = new GenRange();
      gr->SetDefined(false);
      correct=true;
      return SetWord(Address(gr));
  }
  
  GenRange* gr = new GenRange(0);
//  cout<<"length "<<nl->ListLength(instance)<<endl;

  ListExpr first;
  ListExpr rest = instance;
  if( !nl->IsAtom( instance ) ){
     while( !nl->IsEmpty( rest ) ){
        first = nl->First( rest );
        rest = nl->Rest( rest );
        assert(nl->ListLength(first) == 3);
        ListExpr oid_list = nl->First(first);
        ListExpr line_list = nl->Second(first);
        Line* l = (Line*)(InLine(typeInfo,line_list,errorPos,
                                 errorInfo,correct).addr);
        ListExpr tm_list = nl->Third(first);
        unsigned int oid = nl->IntValue(oid_list);
        string str_tm = nl->StringValue(tm_list);
        
//        assert(oid > 0 && l->Size() > 0 && GetTM(str_tm) >= 0);
        if(oid <= 0){
          cout<<"oid invalid"<<endl;
          correct = false;
          return SetWord(Address(0));
        }
        if(l->Size() == 0){
          cout<<"line empty"<<endl;
          correct = false;
          return SetWord(Address(0));
        }
        if(GetTM(str_tm) < 0 ){
          cout<<"this transportation mode does not exist"<<endl;
          correct = false; 
          return SetWord(Address(0));
        }
        gr->Add(oid, l, GetTM(str_tm));
    }
  }
  ////////////////very important /////////////////////////////
  correct = true; 
  ///////////////////////////////////////////////////////////
  return SetWord(gr);

}

ListExpr GenRangeProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("genrange"),
           nl->StringAtom("(<subrange>*) ((int, line)(int , line))"),
           nl->StringAtom("((1 [const line value ((10 1 5 3))]))"))));
}


/*
Open a GenRange object 

*/
bool OpenGenRange(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenGenRange()"<<endl; 

  GenRange* gr = (GenRange*)Attribute::Open(valueRecord, offset, typeInfo);
  value = SetWord(gr);
  return true; 
  
}

/*
Save a GenRange object 

*/
bool SaveGenRange(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveGenRange"<<endl; 
  GenRange* gr = (GenRange*)value.addr; 
  Attribute::Save(valueRecord, offset, typeInfo, gr);
  return true; 
  
}

Word CreateGenRange(const ListExpr typeInfo)
{
// cout<<"CreateGenRange()"<<endl;
  return SetWord (new GenRange(0));
}


void DeleteGenRange(const ListExpr typeInfo, Word& w)
{
//  cout<<"DeleteGenRange()"<<endl;
  GenRange* gr = (GenRange*)w.addr;
  delete gr;
   w.addr = NULL;
}


void CloseGenRange( const ListExpr typeInfo, Word& w )
{
//  cout<<"CloseGenRange"<<endl; 
  ((GenRange*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word CloneGenRange( const ListExpr typeInfo, const Word& w )
{
//  cout<<"CloneGenRange"<<endl; 
  return SetWord( new GenRange( *((GenRange*)w.addr) ) );
}

int SizeOfGenRange()
{
//  cout<<"SizeOfGenRange"<<endl; 
  return sizeof(GenRange);
}

bool CheckGenRange( ListExpr type, ListExpr& errorInfo )
{
//  cout<<"CheckGenRange"<<endl; 
  return (nl->IsEqual( type, "genrange" ));
}


