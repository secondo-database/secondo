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
///////////////////////////random number generator//////////////////////////
unsigned long GetRandom()
{
    /////// only works in linux /////////////////////////////////////
  //  struct timeval tval;
//  struct timezone tzone;
//  gettimeofday(&tval, &tzone);
//  srand48(tval.tv_sec);
// return lrand48();  
  ///////////////////////////////////////////////////////////////////
   return gsl_random.NextInt();  
}


//////////////////////////////////////////////////////////////////////


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
//  cout<<"OutIORef"<<endl; 
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
      cout<< "ioref(): parameter2 must be string type"<<endl;
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
  
  if(genl->GetOid() > 0){
      Loc loc = genl->GetLoc();
      if(loc.loc1 >= 0.0 && loc.loc2 >= 0.0){
          ListExpr loc_list = nl->TwoElemList(
                    nl->RealAtom(loc.loc1),
                    nl->RealAtom(loc.loc2));
      return nl->TwoElemList(nl->IntAtom(genl->GetOid()), loc_list);
      }else if(loc.loc1 >= 0.0 && loc.loc2 < 0.0){
        ListExpr loc_list = nl->OneElemList(nl->RealAtom(loc.loc1));
        return nl->TwoElemList(nl->IntAtom(genl->GetOid()), loc_list);
      }else if(loc.loc1 < 0.0 && loc.loc2 < 0.0){
        return nl->OneElemList(nl->IntAtom(genl->GetOid()));
      }else
      return nl->TheEmptyList(); 
  }else{  //free space oid = 0 
          Loc loc = genl->GetLoc();
          ListExpr loc_list = nl->TwoElemList(
                    nl->RealAtom(loc.loc1),
                    nl->RealAtom(loc.loc2));
          return nl->OneElemList(loc_list);
  }

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

ostream& operator<<(ostream& o, const GenLoc& gloc)
{
  if(gloc.IsDefined()){
    o<<"oid "<<gloc.GetOid()<<" ("<<gloc.GetLoc().loc1
     <<" "<<gloc.GetLoc().loc2<<" )"; 
  }else
    o<<" undef";
  return o;

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
the length of a genrange object 

*/
double GenRange::Length()
{
  double l = 0; 
  for(int i = 0;i < SegSize();i++){
    HalfSegment hs;
    GetSeg(i, hs); 
    l += hs.Length(); 
  }
  return l; 
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

/////////////////////////////////////////////////////////////////////////////
//////////////////////Data Type: UGenLoc//////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

ListExpr UGenLocProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("ugenloc"),
           nl->StringAtom("(<interval,gloc,gloc,tm>)"),
      nl->StringAtom("((interval (1 (10.0 1.0))(1 (12.0 3.0)) Indoor))"))));
}


/*
Output  (interval, genloc1, genloc2, tm)

*/

ListExpr OutUGenLoc( ListExpr typeInfo, Word value )
{
//  cout<<"OutUGenLoc"<<endl; 
  UGenLoc* ugenloc = (UGenLoc*)(value.addr);
  if(ugenloc->IsDefined() == false){
    return nl->SymbolAtom("undef");
  }

  if( ugenloc->IsEmpty() ){
    return nl->TheEmptyList();
  }
    ListExpr timeintervalList = nl->FourElemList(
          OutDateTime( nl->TheEmptyList(),
          SetWord(&ugenloc->timeInterval.start) ),
          OutDateTime( nl->TheEmptyList(), 
                       SetWord(&ugenloc->timeInterval.end) ),
          nl->BoolAtom( ugenloc->timeInterval.lc ),
          nl->BoolAtom( ugenloc->timeInterval.rc)); 
    ListExpr genloc1 = OutGenLoc(nl->TheEmptyList(), &(ugenloc->gloc1));
    ListExpr genloc2 = OutGenLoc(nl->TheEmptyList(), &(ugenloc->gloc2));
    ListExpr tm = nl->StringAtom(GetTMStr(ugenloc->tm)); 


    return nl->FourElemList(timeintervalList,genloc1,genloc2,tm); 
}


/*
In function

*/
Word InUGenLoc( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  string errmsg;
  if ( nl->ListLength( instance ) == 4 ){
    ListExpr first = nl->First( instance );

    if( nl->ListLength( first ) == 4 &&
        nl->IsAtom( nl->Third( first ) ) &&
        nl->AtomType( nl->Third( first ) ) == BoolType &&
        nl->IsAtom( nl->Fourth( first ) ) &&
        nl->AtomType( nl->Fourth( first ) ) == BoolType ){

       correct = true;
       Instant *start = (Instant *)InInstant( nl->TheEmptyList(),
       nl->First( first ),
        errorPos, errorInfo, correct ).addr;

      if( !correct || !start->IsDefined() ){
        errmsg = "InUGneLoc(): Error in first instant (Must be defined!).";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
       nl->Second( first ),
                                           errorPos, errorInfo, correct ).addr;

      if( !correct  || !end->IsDefined() )
      {
        errmsg = "InUGneLoc(): Error in second instant (Must be defined!).";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        delete end;
        return SetWord( Address(0) );
      }

      Interval<Instant> tinterval( *start, *end,
                                   nl->BoolValue( nl->Third( first ) ),
                                   nl->BoolValue( nl->Fourth( first ) ) );
      delete start;
      delete end;

      correct = tinterval.IsValid();
      if (!correct)
        {
          errmsg = "InUGneLoc(): Non valid time interval.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
        }

      //////////////////////////////////////////////////////////////////
      ListExpr second = nl->Second( instance );
      if(nl->ListLength(second) != 2){
        errmsg = "InUGneLoc(): the length for GenLoc should be 2.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        return SetWord( Address(0) );
      }
      
      bool gloc1_correct; 
      GenLoc* gloc1 = (GenLoc*)InGenLoc(typeInfo, second, 
                                        errorPos,errorInfo, gloc1_correct).addr;
      if(gloc1_correct == false){
          errmsg = "InUGneLoc(): Non correct first location.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
      }

      /////////////////////////////////////////////////////////////////////
      ListExpr third = nl->Third(instance); 
      if(nl->ListLength(third) != 2){
        errmsg = "InUGneLoc(): the length for GenLoc should be 2.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        return SetWord( Address(0) );
      }
      bool gloc2_correct; 
      GenLoc* gloc2 = (GenLoc*)InGenLoc(typeInfo, third, 
                                        errorPos,errorInfo, gloc2_correct).addr;
      if(gloc2_correct == false){
          errmsg = "InUGneLoc(): Non correct second location.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
      }

      /////////////////////////////////////////////////////////////////////
      if(gloc1->GetOid() != gloc2->GetOid()){
         errmsg = "InUGneLoc(): two oid should be the same.";
         errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
         return SetWord( Address(0) );
      }

      ListExpr fourth = nl->Fourth(instance);

      if(nl->AtomType( fourth ) == StringType){
        string str_tm = nl->StringValue(fourth);
        int tm = GetTM(str_tm); 

//        cout<<tinterval<<" "<<*gloc1<<" "<<*gloc2<<" "<<str_tm<<endl; 

        UGenLoc *ugenloc = new UGenLoc( tinterval, *gloc1, *gloc2, tm);

        correct = ugenloc->IsValid();
        if( correct )
          return SetWord( ugenloc );

        errmsg = "InUGenLoc(): Error in start/end point.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete ugenloc;
      }
    }
  }else if ( nl->IsAtom( instance ) && nl->AtomType( instance ) == SymbolType
            && nl->SymbolValue( instance ) == "undef" ){
      UGenLoc *ugenloc = new UGenLoc(false);
      ugenloc->timeInterval=
                Interval<DateTime>(DateTime(instanttype),
                           DateTime(instanttype),true,true);
      correct = ugenloc->timeInterval.IsValid();
      if ( correct )
        return (SetWord( ugenloc ));
  }
  errmsg = "InGenLoc(): Error in representation.";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  correct = false;
  return SetWord( Address(0) );
}

/*
Open a UGenLoc object 

*/
bool OpenUGenLoc(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenGenLoc()"<<endl; 

  UGenLoc* genl = (UGenLoc*)Attribute::Open(valueRecord, offset, typeInfo);
  value = SetWord(genl);
  return true; 
}

/*
Save a UGenLoc object 

*/
bool SaveUGenLoc(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveUGenLoc"<<endl; 
  UGenLoc* genl = (UGenLoc*)value.addr; 
  Attribute::Save(valueRecord, offset, typeInfo, genl);
  return true; 
  
}

Word CreateUGenLoc(const ListExpr typeInfo)
{
// cout<<"CreateUGenLoc()"<<endl;
  return SetWord (new UGenLoc(0));
}


void DeleteUGenLoc(const ListExpr typeInfo, Word& w)
{
// cout<<"DeleteUGenLoc()"<<endl;
  UGenLoc* ugenl = (UGenLoc*)w.addr;
  delete ugenl;
   w.addr = NULL;
}


void CloseUGenLoc( const ListExpr typeInfo, Word& w )
{
//  cout<<"CloseUGenLoc"<<endl; 
  ((UGenLoc*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word CloneUGenLoc( const ListExpr typeInfo, const Word& w )
{
//  cout<<"CloneUGenLoc"<<endl; 
  return SetWord( new UGenLoc( *((UGenLoc*)w.addr) ) );
}

int SizeOfUGenLoc()
{
//  cout<<"SizeOfUGenLoc"<<endl; 
  return sizeof(UGenLoc);
}

bool CheckUGenLoc( ListExpr type, ListExpr& errorInfo )
{
//  cout<<"CheckUGenLoc"<<endl; 
  return (nl->IsEqual( type, "ugenloc" ));
}

/*
interporlation function for the location 

*/
void UGenLoc::TemporalFunction( const Instant& t, GenLoc& result,
                               bool ignoreLimits ) const
{
  if( !IsDefined() ||
      !t.IsDefined() ||
      (!timeInterval.Contains( t ) && !ignoreLimits) )
    {
      result.SetDefined(false);
    }
  else if( t == timeInterval.start )
    {
      result = gloc1;
      result.SetDefined(true);
    }
  else if( t == timeInterval.end )
    {
      result = gloc2;
      result.SetDefined(true);
    }
  else
    {
      Instant t0 = timeInterval.start;
      Instant t1 = timeInterval.end;

      Point p0(gloc1.GetLoc().loc1, gloc1.GetLoc().loc2);
      Point p1(gloc2.GetLoc().loc1, gloc2.GetLoc().loc2);

      double x = (p1.GetX() - p0.GetX()) * ((t - t0) / (t1 - t0)) + p0.GetX();
      double y = (p1.GetY() - p0.GetY()) * ((t - t0) / (t1 - t0)) + p0.GetY();

      Loc loc(x,y);
      int oid = (const_cast<GenLoc*>(&gloc1))->GetOid(); 
      result.SetValue(oid, loc);
      result.SetDefined(true);
    }
}

/*
check whether a location is visited 

*/
bool UGenLoc::Passes( const GenLoc& gloc ) const
{
/*
VTA - I could use the spatial algebra like this

----    HalfSegment hs;
        hs.Set( true, p0, p1 );
        return hs.Contains( p );
----
but the Spatial Algebra admit rounding errors (floating point operations). It
would then be very hard to return a true for this function.

*/
  assert( gloc.IsDefined() );
  assert( IsDefined() );
  

  if(gloc.GetOid() != gloc1.GetOid()) return false;
  if(gloc.GetOid() != gloc2.GetOid()) return false;

  Point p(true, gloc.GetLoc().loc1, gloc.GetLoc().loc2); 
  Point p0(true, gloc1.GetLoc().loc1, gloc1.GetLoc().loc2);
  Point p1(true, gloc2.GetLoc().loc1, gloc2.GetLoc().loc2);
  

  if( (timeInterval.lc && AlmostEqual( p, p0 )) ||
      (timeInterval.rc && AlmostEqual( p, p1 )) )
    return true;

  if( AlmostEqual( p0.GetX(), p1.GetX() ) &&
      AlmostEqual( p0.GetX(), p.GetX() ) )
    // If the segment is vertical
  {
    if( ( p0.GetY() <= p.GetY() && p1.GetY() >= p.GetY() ) ||
        ( p0.GetY() >= p.GetY() && p1.GetY() <= p.GetY() ) )
      return true;
  }
  else if( AlmostEqual( p0.GetY(), p1.GetY() ) &&
      AlmostEqual( p0.GetY(), p.GetY() ) )
    // If the segment is horizontal
  {
    if( ( p0.GetX() <= p.GetX() && p1.GetX() >= p.GetX() ) ||
        ( p0.GetX() >= p.GetX() && p1.GetX() <= p.GetX() ) )
      return true;
  }
  else
  {
    double k1 = ( p.GetX() - p0.GetX() ) / ( p.GetY() - p0.GetY() ),
           k2 = ( p1.GetX() - p0.GetX() ) / ( p1.GetY() - p0.GetY() );

    if( AlmostEqual( k1, k2 ) &&
        ( ( p0.GetX() < p.GetX() && p1.GetX() > p.GetX() ) ||
          ( p0.GetX() > p.GetX() && p1.GetX() < p.GetX() ) ) )
      return true;
  }
  return false;
}

/*
restrict the movement at a location 

*/

bool UGenLoc::At( const GenLoc& genloc, TemporalUnit<GenLoc>& res ) const 
{

  assert(genloc.IsDefined());
  assert(this->IsDefined());

  UGenLoc* result = static_cast<UGenLoc*>(&res);
  *result = *this;

  Point p0(true, gloc1.GetLoc().loc1, gloc1.GetLoc().loc2); 
  Point p1(true, gloc2.GetLoc().loc1, gloc2.GetLoc().loc2); 
  Point p(true, genloc.GetLoc().loc1, genloc.GetLoc().loc2); 
  
  // special case: static unit
  if(AlmostEqual(p0,p1)){
     if(AlmostEqual(p,p0) || AlmostEqual(p,p1)){
        return true;
     } else {
        result->SetDefined(false);
        return false;
     }
  }
  // special case p on p0
  if(AlmostEqual(p0,p)){
    if(!timeInterval.lc){
       result->SetDefined(false);
      return false;
    } else {
//       result->p1 = result->p0;
       result->gloc2 = result->gloc1; 
       result->timeInterval.rc = true;
       result->timeInterval.end = timeInterval.start;
       return true;
    }
  }
  // special case p on p1
  if(AlmostEqual(p,p1)){
    if(!timeInterval.rc){
      result->SetDefined(false);
      return false;
    } else {
//      result->p0 = result->p1;
      result->gloc1 = result->gloc2; 
      result->timeInterval.lc = true;
      result->timeInterval.start = timeInterval.end;
      return true;
    }
  }

  double d_x = p1.GetX() - p0.GetX();
  double d_y = p1.GetY() - p0.GetY();
  double delta;
  bool useX;

  if(fabs(d_x)> fabs(d_y)){
     delta = (p.GetX()-p0.GetX() ) / d_x;
     useX = true;
  } else {
     delta = (p.GetY()-p0.GetY() ) / d_y;
     useX = false;
  }

  if(AlmostEqual(delta,0)){
    delta = 0;
  }
  if(AlmostEqual(delta,1)){
    delta = 1;
  }

  if( (delta<0) || (delta>1)){
    result->SetDefined(false);
    return false;
  }

  if(useX){ // check y-value
    double y = p0.GetY() + delta*d_y;
    if(!AlmostEqual(y,p.GetY())){
       result->SetDefined(false);
       return false;
    }
  } else { // check x-value
    double x = p0.GetX() + delta*d_x;
    if(!AlmostEqual(x,p.GetX())){
       result->SetDefined(false);
       return false;
    }
  }


  Instant time = timeInterval.start +
                 (timeInterval.end-timeInterval.start)*delta;

//  result->p0 = p;
//  result->p1 = p;
  result->gloc1 = genloc;
  result->gloc2 = genloc; 
  result->timeInterval.lc = true;
  result->timeInterval.rc = true;
  result->timeInterval.start = time;
  result->timeInterval.end = time;
  return true;
  
}


UGenLoc* UGenLoc::Clone() const
{
  UGenLoc* res;
  if(this->IsDefined()){
    
    res = new UGenLoc(timeInterval, gloc1, gloc2, tm); 
    res->del.isDefined = del.isDefined;
  }else{
    res = new UGenLoc(false); 
  }
  return res; 
}

void UGenLoc::CopyFrom(const Attribute* right)
{
  const UGenLoc* ugloc = static_cast<const UGenLoc*>(right); 
  if(ugloc->del.isDefined){
    timeInterval.CopyFrom(ugloc->timeInterval);
    gloc1 = ugloc->gloc1;
    gloc2 = ugloc->gloc2;
    tm = ugloc->tm; 
  }
  del.isDefined = ugloc->del.isDefined; 
}

ostream& operator<<(ostream& o, const UGenLoc& gloc)
{
  if(gloc.IsDefined()){
    o<<gloc.timeInterval<<" "<<gloc.gloc1<<" "
     <<gloc.gloc2<<" "<<GetTMStr(gloc.tm); 
  }else
    o<<" undef";
  return o;

}

/*
it returns the 3D bounding box. at somewhere else, the program should check
the object identifier to know whether the movement is outdoor or indoor. if 
it is indoor, it has to convert 3D box to 4D box (x,y,z,t). And it also has to
calculate the absolute coordinates in space. 

*/
const Rectangle<3> UGenLoc::BoundingBox() const
{
  if(this->IsDefined()){

    return Rectangle<3>(true, 
                       MIN(gloc1.GetLoc().loc1, gloc2.GetLoc().loc1),
                       MAX(gloc1.GetLoc().loc1, gloc2.GetLoc().loc1), 
                       MIN(gloc1.GetLoc().loc2, gloc2.GetLoc().loc2),
                       MAX(gloc1.GetLoc().loc2, gloc2.GetLoc().loc2),  
                       timeInterval.start.ToDouble(),
                       timeInterval.end.ToDouble());
  }else
      return Rectangle<3>(false); 

}

//////////////////////////////////////////////////////////////////////////
//////////////////////general moving objects//////////////////////////////
//////////////////////////////////////////////////////////////////////////
ListExpr GenMOProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                       nl->StringAtom("genmpoint"),
           nl->StringAtom("(u1,...,un)"),
      nl->StringAtom("((interval (1 (10.0 1.0))(1 (12.0 3.0)) Indoor))"))));
}

bool CheckGenMO( ListExpr type, ListExpr& errorInfo )
{
//  cout<<"CheckGenMPoint"<<endl; 
  return (nl->IsEqual( type, "genmo" ));
}


void GenMO::Clear()
{
  Mapping<UGenLoc, GenLoc>::Clear();
}


GenMO::GenMO(const GenMO& mo):Mapping<UGenLoc,GenLoc>(0)
{
    del.refs = 1;
    del.SetDelete();
    del.isDefined = mo.IsDefined();

    Clear();
    if( !this->IsDefined() ) {
      return;
    }
    StartBulkLoad();
    UGenLoc unit;
    for( int i = 0; i < mo.GetNoComponents(); i++ ){
      mo.Get( i, unit );
      Add( unit );
//      cout<<unit<<endl; 
    }
    EndBulkLoad( false );

}

/*
copy it from another data 

*/
void GenMO::CopyFrom(const Attribute* right)
{
    cout<<"CopyFrom "<<endl; 
    const GenMO *genmo = (const GenMO*)right;
    assert( genmo->IsOrdered() );
    Clear();
    this->SetDefined(genmo->IsDefined());
    if( !this->IsDefined() ) {
      return;
    }
    StartBulkLoad();
    UGenLoc unit;
    for( int i = 0; i < genmo->GetNoComponents(); i++ ){
      genmo->Get( i, unit );
      Add( unit );
    }
    EndBulkLoad( false );
}

Attribute* GenMO::Clone() const
{
    assert( IsOrdered() );
    GenMO *result;
    if( !this->IsDefined() ){
      result = new GenMO( 0 );
    } else {
      result = new GenMO( GetNoComponents() );
      if(GetNoComponents()>0){
        result->units.resize(GetNoComponents());
      }
      result->StartBulkLoad();
      UGenLoc unit;
      for( int i = 0; i < GetNoComponents(); i++ ){
        Get( i, unit );
        result->Add( unit );
      }
      result->EndBulkLoad( false );
    }
    result->SetDefined(this->IsDefined());
    return (Attribute*) result;

}

/*
put new unit. it only inserts the new unit and does not calculate the bounding
box. 

*/
void GenMO::Add(const UGenLoc& unit)
{
  assert(unit.IsDefined());
  assert(unit.IsValid()); 
  if(!IsDefined()){
    SetDefined(false);
    return; 
  }
  assert(unit.gloc1.GetOid() == unit.gloc2.GetOid()); 
  units.Append(unit); 
}

void GenMO::EndBulkLoad(const bool sort, const bool checkvalid)
{
  Mapping<UGenLoc, GenLoc>::EndBulkLoad(sort, checkvalid); 

}


/*
low resolution for a generic moving object (oid,tm)

*/
void GenMO::LowRes(GenMO& mo)
{
    mo.Clear();

    mo.StartBulkLoad();
    UGenLoc unit;
    Loc loc(-1.0, -1.0);
    for( int i = 0; i < GetNoComponents(); i++ ){
      Get( i, unit );
      unit.gloc1.SetLoc(loc); 
      unit.gloc2.SetLoc(loc); 
      mo.Add( unit );
//      cout<<unit<<endl; 
    }
    mo.EndBulkLoad( false );
}

/*
get the trajectory of a generic moving object. it needs the space 
because loc1 loc2 have different meanings for different infrastructures 

*/
void GenMO::Trajectory(GenRange& genrange)
{
  cout<<"GenMO::Trajectory() not implemented"<<endl; 
}


/////////////////////////////////////////////////////////////////////////
/////////////// get information from generic moving objects///////////////
/////////////////////////////////////////////////////////////////////////
void GenMObject::GetTM(GenMO* mo)
{
  tm_list.clear(); 
  for(int i = 0;i < mo->GetNoComponents();i++){
    UGenLoc unit;
    mo->Get(i, unit);
    int tm = unit.GetTM(); 
    assert(tm >= 0); 
    if(tm_list.size() == 0)tm_list.push_back(tm); 
    else if(tm_list[tm_list.size() - 1] != tm)
      tm_list.push_back(tm); 
  }
}

void GenMObject::GetTMStr(bool v)
{
  if(v == false)return;
  for(unsigned int i = 0;i < ARR_SIZE(genmo_tmlist);i++){
      tm_str_list.push_back(genmo_tmlist[i]);
  }

}

//////////////////////////////////////////////////////////////////////
//////////////////////Global Space////////////////////////////////////
//////////////////////////////////////////////////////////////////////
ListExpr SpaceProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                       nl->StringAtom("space"),
           nl->StringAtom("(infrastructures)"),
      nl->StringAtom("(infrastructure objects)"))));
}

/*
In function

*/
Word InSpace( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{

  if(!(nl->ListLength(instance) == 1)){
     string strErrorMessage = "Space(): initial length must be 1.";
     errorInfo =
        nl->Append ( errorInfo, nl->StringAtom ( strErrorMessage ) );
      correct = false;
    return SetWord(Address(0));
  }

  ListExpr xIdList = nl->First ( instance );
  
  // Read Id
  if ( !nl->IsAtom ( xIdList ) ||
          nl->AtomType ( xIdList ) != IntType )
  {
    string strErrorMessage = "Space(): Id is missing.";
    errorInfo = nl->Append ( errorInfo,
                             nl->StringAtom ( strErrorMessage ) );
    correct = false;
    return SetWord(Address(0));
  }
  
    unsigned int space_id = nl->IntValue(xIdList);
    Space* sp = new Space(space_id);
    correct = true; 
    return SetWord(sp);

}

ListExpr OutSpace( ListExpr typeInfo, Word value )
{
//  cout<<"OutSpace"<<endl; 
  Space* sp = (Space*)(value.addr);

 if(!sp->IsDefined()){
   return nl->SymbolAtom("undef");
 }

  if(sp->GetNetworkId() > 0){
      ListExpr list1 = nl->TwoElemList(nl->StringAtom("SpaceId:"), 
                             nl->IntAtom(sp->GetSpaceId()));
      ListExpr list2 = nl->TwoElemList(nl->StringAtom("NetworkId:"), 
                             nl->IntAtom(sp->GetNetworkId()));
    
      return nl->TwoElemList(list1,list2); 

  }else{  //free space oid = 0 
          return nl->TwoElemList(nl->StringAtom("SpaceId:"),
                                 nl->IntAtom(sp->GetSpaceId()));
  }

}


bool CheckSpace( ListExpr type, ListExpr& errorInfo )
{
//  cout<<"CheckSpace"<<endl;
  return (nl->IsEqual( type, "space" ));
}

int SizeOfSpace()
{
//  cout<<"SizeOfSpacc"<<endl; 
  return sizeof(Space);
}


Word CreateSpace(const ListExpr typeInfo)
{
// cout<<"CreateSpace()"<<endl;
  return SetWord (new Space());
}


void DeleteSpace(const ListExpr typeInfo, Word& w)
{
//  cout<<"DeleteSpace()"<<endl;
  Space* sp = (Space*)w.addr;
  delete sp;
   w.addr = NULL;
}


void CloseSpace( const ListExpr typeInfo, Word& w )
{
//  cout<<"CloseSpace"<<endl; 
  delete static_cast<Space*>(w.addr);
  w.addr = NULL;
}

Word CloneSpace( const ListExpr typeInfo, const Word& w )
{
//  cout<<"CloneSpace"<<endl; 
//  return SetWord( new Space( *((Space*)w.addr) ) );
  return SetWord(Address(0));
}

bool SaveSpace(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value)
{
  Space* sp = (Space*)value.addr;
  return sp->Save(valueRecord, offset, typeInfo); 
}

bool OpenSpace(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value)
{
  value.addr = Space::Open(valueRecord, offset, typeInfo);
  return value.addr != NULL; 
}


Space::Space():
def(false), space_id(0), network_id(0)
{
 
}

Space::Space(int id):
def(true), space_id(id), network_id(0)
{
  if(space_id > 0) def = true;
  else
    def = false; 
//  cout<<"space_id "<<space_id<<" def "<<def<<endl; 
}

Space::Space(SmiRecord& valueRecord, size_t& offset, 
                     const ListExpr typeInfo):
def(false), space_id(0), network_id(0)
{
//  cout<<"Space::Open()"<<endl; 
  
  valueRecord.Read(&def, sizeof(bool), offset);
  offset += sizeof(bool);

  valueRecord.Read(&space_id, sizeof(int), offset);
  offset += sizeof(int);

  valueRecord.Read(&network_id, sizeof(int), offset);
  offset += sizeof(int);

}
Space::~Space()
{

}

void Space::SetId(int id)
{
  if(id > 0){
    space_id = id;
    def = true;
  }else
    def = false; 

}

bool Space::Save(SmiRecord& valueRecord, size_t& offset, 
                 const ListExpr typeInfo)
{
//  cout<<"Space::Save()"<<endl; 

  valueRecord.Write(&def, sizeof(bool), offset); 
  offset += sizeof(bool); 

  valueRecord.Write(&space_id, sizeof(int), offset); 
  offset += sizeof(int); 

  valueRecord.Write(&network_id, sizeof(int), offset); 
  offset += sizeof(int); 

  return true; 
}

Space* Space::Open(SmiRecord& valueRecord, size_t& offset, 
                 const ListExpr typeInfo)
{
    return new Space(valueRecord, offset, typeInfo); 
}
