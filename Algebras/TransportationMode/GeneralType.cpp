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
#include "PaveGraph.h"
#include "BusNetwork.h"
#include "Indoor.h"

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
void GenMO::Trajectory(GenRange* genrange, Space* sp)
{
  for(int i = 0;i < GetNoComponents();i++){
      UGenLoc unit;
      Get(i, unit);
      int oid = unit.gloc1.GetOid();
      int tm = unit.tm;

      Line* l = new Line(0);
      l->StartBulkLoad();
      sp->GetLineInIFObject(oid, unit.gloc1, unit.gloc2, l);
      l->EndBulkLoad();

      genrange->Add(oid, l, tm);
      delete l;
  }

}


/////////////////////////////////////////////////////////////////////////
/////////////// get information from generic moving objects///////////////
/////////////////////////////////////////////////////////////////////////
string GenMObject::StreetSpeedInfo = "(rel (tuple ((id_b int) (Vmax real))))";


void GenMObject::GetMode(GenMO* mo)
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

/*
get the reference id of generic moving objects 

*/
void GenMObject::GetIdList(GenMO* genmo)
{
    for( int i = 0; i < genmo->GetNoComponents(); i++ ){
      UGenLoc unit;
      genmo->Get( i, unit );
      assert(unit.gloc1.GetOid() == unit.gloc2.GetOid()); 
      id_list.push_back(unit.gloc1.GetOid()); 
    }
}

/*
get the reference id of genrange objects

*/

void GenMObject::GetIdList(GenRange* gr)
{
    for(int i = 0;i < gr->ElemSize();i++){
      GenRangeElem elem;
      gr->GetElem(i, elem); 
      id_list.push_back(elem.oid); 
    }
}

/*
create generic moving objects

*/
void GenMObject::GenerateGenMO(Space* sp, Periods* peri, int mo_no, 
                               int type, Relation* rel)
{
//  cout<<"GenerateGenMO()"<<endl; 
  if(mo_no < 1){
    cout<<" invalid number of moving objects "<<mo_no<<endl;
    return;
  }
  
  if(!(0 <= type && type < int(ARR_SIZE(genmo_tmlist)))){
    cout<<" invalid type value "<<type<<endl;
  }

  switch(type){
    case 1:
      GenerateGenMO_Walk(sp, peri, mo_no, rel);
      break;

    case 4:
      GenerateGenMO_CarTaxi(sp, peri, mo_no, rel, "Car");
      break;

    case 6:
      GenerateGenMO_CarTaxi(sp, peri, mo_no, rel, "Taxi");
      break;

    default:
      assert(false);
      break;  
  }

}

/*
create generic moving objects

*/
void GenMObject::GenerateGenMO2(Space* sp, Periods* peri, int mo_no, 
                               int type, Relation* rel1, 
                               BTree* btree, Relation* rel2)
{
//  cout<<"GenerateGenMO()"<<endl; 
  if(mo_no < 1){
    cout<<" invalid number of moving objects "<<mo_no<<endl;
    return;
  }
  if(!(0 <= type && type < int(ARR_SIZE(genmo_tmlist)))){
    cout<<" invalid type value "<<type<<endl;
  }

  switch(type){
    case 7:
      GenerateGenMO_CarTaxiWalk(sp, peri, mo_no, rel1, btree, rel2, "Car");
      break;
    case 11:
      GenerateGenMO_CarTaxiWalk(sp, peri, mo_no, rel1, btree, rel2, "Taxi");
      break;

    default:
      assert(false);
      break;  
  }

}

/*
generic moving objects with transportation mode walk

*/
void GenMObject::GenerateGenMO_Walk(Space* sp, Periods* peri, 
                                    int mo_no, Relation* tri_rel)
{

//  cout<<"Mode Walk "<<endl;
  const double min_len = 50.0;
  Pavement* pm = sp->LoadPavement(IF_REGION);
  vector<GenLoc> genloc_list;
  GenerateLocPave(pm, mo_no, genloc_list);///generate locations on pavements 
  int count = 1;

  DualGraph* dg = pm->GetDualGraph();
  int no_triangle = dg->GetNodeRel()->GetNoTuples();
  int mini_oid = dg->min_tri_oid_1;
  VisualGraph* vg = pm->GetVisualGraph();


  Walk_SP* wsp = new Walk_SP(dg, vg, NULL, NULL);
  wsp->rel3 = tri_rel;
  
  while(count <= mo_no){
    int index1 = GetRandom() % genloc_list.size();
    GenLoc loc1 = genloc_list[index1];
    int index2 = GetRandom() % genloc_list.size();
    GenLoc loc2 = genloc_list[index2];

    if(index1 == index2) continue;
    Point p1(true, loc1.GetLoc().loc1, loc1.GetLoc().loc2);
    Point p2(true, loc2.GetLoc().loc1, loc2.GetLoc().loc2);
    if(p1.Distance(p2) < min_len) continue;


    int oid1 = loc1.GetOid() - mini_oid;
//    cout<<"oid1 "<<oid1<<endl;
    assert(1 <= oid1 && oid1 <= no_triangle);

    int oid2 = loc2.GetOid() - mini_oid;
//    cout<<"oid2 "<<oid2<<endl;
    assert(1 <= oid2 && oid2 <= no_triangle);

    Line* path = new Line(0);

    wsp->WalkShortestPath2(oid1, oid2, p1, p2, path);
//    cout<<"path length "<<path->Length()<<endl; 
    GenerateWalk(dg, count, peri, path, p1);

    delete path;
    count++;

  }
  delete wsp;

  pm->CloseDualGraph(dg);
  pm->CloseVisualGraph(vg);

  sp->ClosePavement(pm);
}

/*
generate locations on the pavment area 
!!! note that here we put the ----absolute position---- in loc instead of the 
relative position according to the triangle !!!

*/
void GenMObject::GenerateLocPave(Pavement* pm, int mo_no, 
                                 vector<GenLoc>& genloc_list)
{
  Walk_SP* wsp = new Walk_SP(NULL, NULL, pm->GetPaveRel(), NULL);
  wsp->GenerateData1(mo_no*2);
  for(unsigned int i = 0;i < wsp->oids.size();i++){
    int oid = wsp->oids[i];
/*    double loc1 = wsp->q_loc1[i].GetX();
    double loc2 = wsp->q_loc1[i].GetY();*/

    double loc1 = wsp->q_loc2[i].GetX();
    double loc2 = wsp->q_loc2[i].GetY();

//    cout<<"oid "<<oid<<" loc1 "<<loc1<<" loc2 "<<loc2<<endl; 
    Loc loc(loc1, loc2);
    GenLoc genloc(oid, loc);
    genloc_list.push_back(genloc);
  }

  delete wsp;
}

/*
generate moving objects with mode walk

*/
void GenMObject::GenerateWalk(DualGraph* dg, int count, Periods* peri, 
                              Line* l, Point start_loc)
{
   SimpleLine* path = new SimpleLine(0);
   path->fromLine(*l);

   SpacePartition* sp = new SpacePartition();
   vector<MyHalfSegment> seq_halfseg; //reorder it from start to end
   sp->ReorderLine(path, seq_halfseg);
   delete sp;
   delete path;
   Point temp_sp1 = seq_halfseg[0].from; 
   Point temp_sp2 = seq_halfseg[seq_halfseg.size() - 1].to; 
   const double delta_dist = 0.01;

   MPoint* mo = new MPoint(0);
   GenMO* genmo = new GenMO(0);
   mo->StartBulkLoad();
   genmo->StartBulkLoad();
   //////////////////////////////////////////////////////////////
   ///////////////set start time/////////////////////////////////
   /////////////////////////////////////////////////////////////
   Interval<Instant> periods;
   peri->Get(0, periods);
   Instant start_time = periods.start;
   int time_range = 14*60;//14 hours 
   if(count % 3 == 0)
     start_time.ReadFrom(periods.start.ToDouble() + 
                        (GetRandom() % time_range)/(24.0*60.0));
   else
    start_time.ReadFrom(periods.end.ToDouble() -
                        (GetRandom() % time_range)/(24.0*60.0));
   ///////////////////////////////////////////////////////////////
   ////////we reference to the big overall large polygon/////////////
   //////////////////////////////////////////////////////////////////
   int gen_mo_ref_id = dg->min_tri_oid_1 + dg->GetNodeRel()->GetNoTuples() + 1;
//   cout<<gen_mo_ref_id<<endl;
    Rectangle<2> bbox = dg->rtree_node->BoundingBox();
//    cout<<bbox<<endl;
   ///////////////////////////////////////////////////////////////////

   if(start_loc.Distance(temp_sp1) < delta_dist){
        Instant st = start_time;
        Instant et = start_time; 
        Interval<Instant> up_interval; 
        for(unsigned int i = 0;i < seq_halfseg.size();i++){
            Point from_loc = seq_halfseg[i].from;
            Point to_loc = seq_halfseg[i].to; 

            double dist = from_loc.Distance(to_loc);
            double time = dist; //assume the speed for pedestrian is 1.0m
 
            if(dist < delta_dist){//ignore such small segment 
                if((i + 1) < seq_halfseg.size()){
                seq_halfseg[i+1].from = from_loc; 
                }
                continue; 
            }
            //double 1.0 means 1 day 
            et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0*60));
        ////////////////////create a upoint////////////////////////
            up_interval.start = st;
            up_interval.lc = true;
            up_interval.end = et;
            up_interval.rc = false; 
            UPoint* up = new UPoint(up_interval,from_loc,to_loc);
            mo->Add(*up);
            delete up; 
            //////////////////////////////////////////////////////////////
            st = et; 
            ////////////////////////////////////////////////////////////////
            Loc loc1(from_loc.GetX() - bbox.MinD(0), 
                     from_loc.GetY() - bbox.MinD(1));
            Loc loc2(to_loc.GetX() - bbox.MinD(0),
                     to_loc.GetY() - bbox.MinD(1));
            GenLoc gloc1(gen_mo_ref_id, loc1);
            GenLoc gloc2(gen_mo_ref_id, loc2);
            int tm = GetTM("Walk");
            UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
            genmo->Add(*unit); 
            delete unit; 
        }
   }else if(start_loc.Distance(temp_sp2) < delta_dist){
              Instant st = start_time;
              Instant et = start_time; 
              Interval<Instant> up_interval; 
              for(int i = seq_halfseg.size() - 1;i >= 0;i--){
                  Point from_loc = seq_halfseg[i].to;
                  Point to_loc = seq_halfseg[i].from; 
                  double dist = from_loc.Distance(to_loc);
                  double time = dist; 
                  if(dist < delta_dist){//ignore such small segment 
                        if((i + 1) < seq_halfseg.size()){
                            seq_halfseg[i+1].from = from_loc; 
                        }
                    continue;
                  }
                //double 1.0 means 1 day 
                 et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0*60.0));

                 ////////////////////create a upoint////////////////////////
                  up_interval.start = st;
                  up_interval.lc = true;
                  up_interval.end = et;
                  up_interval.rc = false; 
                  UPoint* up = new UPoint(up_interval,from_loc,to_loc);
                  mo->Add(*up);
                  delete up; 
                  /////////////////////////////////////////////////////////
                  st = et;
                  /////////////////////////////////////////////////////////
                  Loc loc1(from_loc.GetX() - bbox.MinD(0), 
                           from_loc.GetY() - bbox.MinD(1));
                  Loc loc2(to_loc.GetX() - bbox.MinD(0),
                           to_loc.GetY() - bbox.MinD(1));
                  GenLoc gloc1(gen_mo_ref_id, loc1);
                  GenLoc gloc2(gen_mo_ref_id, loc2);
                  int tm = GetTM("Walk");
                  UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
                  genmo->Add(*unit); 
                  delete unit; 
              }
   }else assert(false);

   mo->EndBulkLoad();
   genmo->EndBulkLoad();
  
   trip1_list.push_back(*genmo);
   trip2_list.push_back(*mo);
   delete mo; 
   delete genmo; 
   
}

/*
generic moving objects with transportation mode car or taxi

*/
void GenMObject::GenerateGenMO_CarTaxi(Space* sp, Periods* peri, int mo_no, 
                                   Relation* rel, string mode)
{
//  cout<<"Mode Car "<<endl;

  const double min_len = 500.0;
  Network* rn = sp->LoadRoadNetwork(IF_LINE);
  vector<GPoint> gp_list; 
  GenerateGPoint(rn, mo_no, gp_list);/////////get road network locations 
  int count = 1;
  while(count <= mo_no){
    int index1 = GetRandom() % gp_list.size();
    GPoint gp1 = gp_list[index1];
//    cout<<"gp1 "<<gp1<<endl;
    int index2 = GetRandom() % gp_list.size();
    GPoint gp2 = gp_list[index2];
    if(index1 == index2) continue; 

    Point* start_loc = new Point();
    gp1.ToPoint(start_loc);

//      cout<<"gp2 "<<gp2<<endl; 
    GLine* gl = new GLine(0);
    gp1.ShortestPath(&gp2, gl);////////compute the shortest path 
//    cout<<"gline len "<<gl->GetLength()<<endl; 
    if(gl->GetLength() > min_len){
        BusRoute* br = new BusRoute(rn, NULL,NULL);
        GLine* newgl = new GLine(0);
        br->ConvertGLine(gl, newgl);
//        cout<<"new gl length "<<newgl->GetLength()<<endl; 
        GenerateCarTaxi(rn, count, peri, newgl, rel, *start_loc, mode);

        delete newgl; 
        delete br;
        count++;
    }
    delete gl;
    delete start_loc; 
  }

  sp->CloseRoadNetwork(rn);
}


/*
create a moving object with mode car or taxi

*/
void GenMObject::GenerateCarTaxi(Network* rn, int i, Periods* peri, 
                                 GLine* newgl,
                                 Relation* rel, Point start_loc, string mode)
{
  Interval<Instant> periods;
  peri->Get(0, periods);
  Instant start_time = periods.start;
  int time_range = 14*60;//14 hours 
  if(i % 3 == 0)
    start_time.ReadFrom(periods.start.ToDouble() + 
                        (GetRandom() % time_range)/(24.0*60.0));
  else
    start_time.ReadFrom(periods.end.ToDouble() -
                        (GetRandom() % time_range)/(24.0*60.0));
//  cout<<"start time "<<start_time<<endl;

  MPoint* mo = new MPoint(0);
  GenMO* genmo = new GenMO(0);
  mo->StartBulkLoad();
  genmo->StartBulkLoad(); 
  const double delta_dist = 0.01;

  for(int i = 0;i < newgl->Size();i++){
    RouteInterval* ri = new RouteInterval();
    newgl->Get(i, *ri);
    int rid = ri->GetRouteId();
    Tuple* speed_tuple = rel->GetTuple(rid, false);
    assert(((CcInt*)speed_tuple->GetAttribute(SPEED_RID))->GetIntval() == rid);
    double speed = 
      ((CcReal*)speed_tuple->GetAttribute(SPEED_VAL))->GetRealval();
    speed_tuple->DeleteIfAllowed();
//    cout<<"rid "<<rid<<" speed "<<speed<<endl; 

    if(speed < 0.0)speed = 10.0;
    speed = speed * 1000.0/3600.0;// meters in second 
//    cout<<"rid "<<rid<<" new speed "<<speed<<endl;
    SimpleLine* sl = new SimpleLine(0);
    rn->GetLineValueOfRouteInterval(ri, sl);
    ////////////////////////////////////////////////////////////////////
    /////////////get the subline of this route interval////////////////
    ///////////////////////////////////////////////////////////////////
    SpacePartition* sp = new SpacePartition();
    vector<MyHalfSegment> seq_halfseg; //reorder it from start to end
    sp->ReorderLine(sl, seq_halfseg);
    delete sp;
    //////////////////////////////////////////////////////////////////
    delete sl;
    
    /////////////////////////////////////////////////////////////////////

    Interval<Instant> unit_interval;
    unit_interval.start = start_time;
    unit_interval.lc = true;

    Point temp_sp1 = seq_halfseg[0].from; 
    Point temp_sp2 = seq_halfseg[seq_halfseg.size() - 1].to; 
    if(start_loc.Distance(temp_sp1) < delta_dist){
      CreateCarTrip1(mo, seq_halfseg, start_time, speed);

      start_loc = temp_sp2;
    }else if(start_loc.Distance(temp_sp2) < delta_dist){

      CreateCarTrip2(mo, seq_halfseg, start_time, speed);
      start_loc = temp_sp1;
    }else assert(false);

    unit_interval.end = start_time;
    unit_interval.rc = false;
    //////////////////////////////////////////////////////////////
    ////////////////generic units/////////////////////////////////
    //////////////////////////////////////////////////////////////
    Loc loc1(ri->GetStartPos(), -1); 
    Loc loc2(ri->GetEndPos(), -1); 
    GenLoc gloc1(ri->GetRouteId(), loc1);
    GenLoc gloc2(ri->GetRouteId(), loc2);
    int tm = GetTM(mode); 

    //////////////////////////////////////////////////////////////////
    /////////////correct way to create UGenLoc///////////////////////
    //////////////////////////////////////////////////////////////////
    UGenLoc* unit = new UGenLoc(unit_interval, gloc1, gloc2, tm);
    genmo->Add(*unit); 
    delete unit; 
    delete ri;

  }
  mo->EndBulkLoad();
  genmo->EndBulkLoad();
  
  trip1_list.push_back(*genmo);
  trip2_list.push_back(*mo);
  delete mo; 
  delete genmo; 

}

/*
create a set of random gpoint locations 

*/
void GenMObject::GenerateGPoint(Network* rn, int mo_no, vector<GPoint>& gp_list)
{
  Relation* routes_rel = rn->GetRoutes();
  for (int i = 1; i <= mo_no * 2;i++){
     int m = GetRandom() % routes_rel->GetNoTuples() + 1;
     Tuple* road_tuple = routes_rel->GetTuple(m, false);
     int rid = ((CcInt*)road_tuple->GetAttribute(ROUTE_ID))->GetIntval();
     SimpleLine* sl = (SimpleLine*)road_tuple->GetAttribute(ROUTE_CURVE);
//     cout<<"rid "<<rid<<" len: "<<sl->Length()<<endl; 
     double len = sl->Length();
     int pos = GetRandom() % (int)len;
     GPoint gp(true, rn->GetId(), rid, pos);
     gp_list.push_back(gp);
     road_tuple->DeleteIfAllowed();
   }
}


/*
create moving bus units and add them into the trip, 
it travers from index small to big in myhalfsegment list 
use the maxspeed as car speed 

*/
void GenMObject::CreateCarTrip1(MPoint* mo, vector<MyHalfSegment> seq_halfseg,
                      Instant& start_time, double speed_val)
{
  const double dist_delta = 0.01; 
  
  Instant st = start_time;
  Instant et = start_time; 
  Interval<Instant> up_interval; 
  for(unsigned int i = 0;i < seq_halfseg.size();i++){
    Point from_loc = seq_halfseg[i].from;
    Point to_loc = seq_halfseg[i].to; 

    double dist = from_loc.Distance(to_loc);
    double time = dist/speed_val; 

 //   cout<<"dist "<<dist<<" time "<<time<<endl; 
//    printf("%.10f, %.10f",dist, time); 
    //////////////////////////////////////////////////////////////
    if(dist < dist_delta){//ignore such small segment 
        if((i + 1) < seq_halfseg.size()){
          seq_halfseg[i+1].from = from_loc; 
        }
        continue; 
    }
    /////////////////////////////////////////////////////////////////

    //double 1.0 means 1 day 
    et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0*60));
//    cout<<st<<" "<<et<<endl;
    ////////////////////create a upoint////////////////////////
    up_interval.start = st;
    up_interval.lc = true;
    up_interval.end = et;
    up_interval.rc = false; 
    UPoint* up = new UPoint(up_interval,from_loc,to_loc);
//    cout<<*up<<endl; 
    mo->Add(*up);
    delete up; 
    //////////////////////////////////////////////////////////////
    st = et; 
  }

  start_time = et; 

}

/*
create moving carunits and add them into the trip, 
!!! it traverse from index big to small in myhalfsegment list 
use the maxspeed as car speed 

*/
void GenMObject::CreateCarTrip2(MPoint* mo, vector<MyHalfSegment> seq_halfseg,
                      Instant& start_time, double speed_val)
{
 const double dist_delta = 0.01; 
//  cout<<"trip2 max speed "<<speed_val<<" start time "<<start_time<<endl; 
  Instant st = start_time;
  Instant et = start_time; 
  Interval<Instant> up_interval; 
  for(int i = seq_halfseg.size() - 1;i >= 0;i--){
    Point from_loc = seq_halfseg[i].to;
    Point to_loc = seq_halfseg[i].from; 
    double dist = from_loc.Distance(to_loc);
    double time = dist/speed_val; 

 //   cout<<"dist "<<dist<<" time "<<time<<endl; 
    ///////////////////////////////////////////////////////////////////
    if(dist < dist_delta){//ignore such small segment 
        if((i + 1) < seq_halfseg.size()){
          seq_halfseg[i+1].from = from_loc; 
        }
        continue; 
    }

    //double 1.0 means 1 day 
    et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0*60.0));
//    cout<<st<<" "<<et<<endl;
    ////////////////////create a upoint////////////////////////
    up_interval.start = st;
    up_interval.lc = true;
    up_interval.end = et;
    up_interval.rc = false; 
    UPoint* up = new UPoint(up_interval,from_loc,to_loc);
    mo->Add(*up);
    delete up; 
    //////////////////////////////////////////////////////////////
    st = et; 
  }

  start_time = et; 

}

/*
generate generic moving objects with mode car or taxi and walk

*/
void GenMObject::GenerateGenMO_CarTaxiWalk(Space* sp, Periods* peri, int mo_no,
                             Relation* rel, BTree* btree, 
                                       Relation* speed_rel, string mode)
{

//  cout<<"Mode Car-Walk "<<endl;
  const double min_len = 50.0;
  Pavement* pm = sp->LoadPavement(IF_REGION);
  vector<GenLoc> genloc_list;
  GenerateLocPave(pm, mo_no, genloc_list);///generate locations on pavements 
  int count = 1;
  Network* rn = sp->LoadRoadNetwork(IF_LINE);

  Interval<Instant> periods;
  peri->Get(0, periods);
    
  while(count <= mo_no){
    int index1 = GetRandom() % genloc_list.size();
    GenLoc loc1 = genloc_list[index1];
    int index2 = GetRandom() % genloc_list.size();
    GenLoc loc2 = genloc_list[index2];

    if(index1 == index2) continue;

    MPoint* mo = new MPoint(0);
    mo->StartBulkLoad();
    GenMO* genmo = new GenMO(0);
    genmo->StartBulkLoad();
  
    
    Instant start_time = periods.start;
    int time_range = 14*60;//14 hours 
  
    if(count % 3 == 0)//////////one third on sunday 
      start_time.ReadFrom(periods.start.ToDouble() + 
                        (GetRandom() % time_range)/(24.0*60.0));
    else  ////////////two third on Monday 
      start_time.ReadFrom(periods.end.ToDouble() -
                        (GetRandom() % time_range)/(24.0*60.0));
    ////////////////////////////////////////////////////////////////////////
    /////////////1 map the two positions to gpoints/////////////////////////
    ///////////////////////////////////////////////////////////////////////
    vector<GPoint> gpoint_list;
    vector<Point> p_list;
    PaveLoc2GPoint(loc1, loc2, sp, rel, btree, gpoint_list, p_list);
    GPoint gp1 = gpoint_list[0];
    GPoint gp2 = gpoint_list[1];
    Point start_loc = p_list[0];
    Point end_loc = p_list[1];
    //////////////////////////////////////////////////////////////////////
    ////////////2 connect the path to the end point//////////////////////
    //////////////////////////////////////////////////////////////////////

    ConnectStartMove(loc1, start_loc, mo, genmo, start_time, pm, mode);

    //////////////////////////////////////////////////////////////////////
    ///////////3 shortest path between two gpoints////////////////////////
    //////////////////////////////////////////////////////////////////////
    GLine* gl = new GLine(0);
    gp1.ShortestPath(&gp2, gl);////////compute the shortest path 

    if(gl->GetLength() < min_len){
      delete gl;
      continue;
    }

    BusRoute* br = new BusRoute(rn, NULL,NULL);
    GLine* newgl = new GLine(0);
    br->ConvertGLine(gl, newgl);
    ConnectGP1GP2(rn, start_loc, newgl, mo, genmo, start_time, 
                  speed_rel, mode);

    delete newgl;

    delete br;
    delete gl;

    //////////////////////////////////////////////////////////////////////
    ////////////4 connect the path to the end point//////////////////////
    //////////////////////////////////////////////////////////////////////

    ConnectEndMove(end_loc, loc2, mo, genmo, start_time, pm, mode);

    ////////////////////////////////////////////////////////////////////

    count++;

    mo->EndBulkLoad();
    genmo->EndBulkLoad();

    trip1_list.push_back(*genmo);
    trip2_list.push_back(*mo);

    delete mo;
    delete genmo; 
  }

  sp->ClosePavement(pm);
  sp->CloseRoadNetwork(rn);

}

/*
map the two points on the pavement to loations on the road network 

*/
void GenMObject::PaveLoc2GPoint(GenLoc loc1, GenLoc loc2, Space* sp, 
                                Relation* rel,
                      BTree* btree, vector<GPoint>& gpoint_list, 
                                vector<Point>& p_list)
{
    Network* rn = sp->LoadRoadNetwork(IF_LINE);

  /////////////////////////////////////////////////////////////////////
    CcInt* search_id1 = new CcInt(true, loc1.GetOid());
    BTreeIterator* btree_iter1 = btree->ExactMatch(search_id1);
    vector<int> route_id_list1;
    while(btree_iter1->Next()){
      Tuple* tuple = rel->GetTuple(btree_iter1->GetId(), false);
      int rid = ((CcInt*)tuple->GetAttribute(DualGraph::RID))->GetIntval();
      route_id_list1.push_back(rid); 
      tuple->DeleteIfAllowed();
    }
    delete btree_iter1;
    delete search_id1;

//     for(unsigned int i = 0;i < route_id_list1.size();i++)
//       cout<<route_id_list1[i]<<endl;

    Point p1(true, loc1.GetLoc().loc1, loc1.GetLoc().loc2);

    Walk_SP* wsp1 = new Walk_SP();
    wsp1->PaveLocToGPoint(&p1, rn, route_id_list1);
    GPoint gp1 = wsp1->gp_list[0];
    Point gp_loc1 = wsp1->p_list[0];
    delete wsp1;
    /////////////////////////////////////////////////////////////////////////

    CcInt* search_id2 = new CcInt(true, loc2.GetOid());
    BTreeIterator* btree_iter2 = btree->ExactMatch(search_id2);
    vector<int> route_id_list2;
    while(btree_iter2->Next()){
        Tuple* tuple = rel->GetTuple(btree_iter2->GetId(), false);
        int rid = ((CcInt*)tuple->GetAttribute(DualGraph::RID))->GetIntval();
        route_id_list2.push_back(rid); 
        tuple->DeleteIfAllowed();
    }
    delete btree_iter2;
    delete search_id2;

//     for(unsigned int i = 0;i < route_id_list2.size();i++)
//       cout<<route_id_list2[i]<<endl;

    Point p2(true, loc2.GetLoc().loc1, loc2.GetLoc().loc2);

    Walk_SP* wsp2 = new Walk_SP();
    wsp2->PaveLocToGPoint(&p2, rn, route_id_list2);
    GPoint gp2 = wsp2->gp_list[0];
    Point gp_loc2 = wsp2->p_list[0];
    delete wsp2;
    ///////////////////////////////////////////////////////////////////////
    sp->CloseRoadNetwork(rn);

     /////////////////////for debuging/////////////////////////////////
    //////////start and end locations on the pavement////////////////

//      loc_list1.push_back(p1);
//    loc_list1.push_back(p2);
//    loc_list1.push_back(gp_loc1);

    ///////////////start and end gpoint ////////////////////////////////
//    gp_list.push_back(gp1);
//    gp_list.push_back(gp2);

    ////////////////////start and end point in space/////////////////////
//    loc_list2.push_back(gp_loc1);
//    loc_list2.push_back(gp_loc2);

//    loc_list2.push_back(gp_loc2);
//    loc_list2.push_back(p2);

    /////////////////////////////////////////////////////////////////////
    gpoint_list.push_back(gp1);
    gpoint_list.push_back(gp2);
    p_list.push_back(gp_loc1);
    p_list.push_back(gp_loc2);

}

/*
build the connection from a point on the pavement to the road network point
subpath1: on the pavement
subpath2: in the free space (mode: car)
loc1: point on the pavement; end loc: point maps to a gpoint 

*/
void GenMObject::ConnectStartMove(GenLoc loc1, Point end_loc, MPoint* mo,
                        GenMO* genmo, Instant& start_time, 
                                  Pavement* pm, string mode)
{
  const double delta_dist = 0.01;
  DualGraph* dg = pm->GetDualGraph();

  Point start_loc(true, loc1.GetLoc().loc1, loc1.GetLoc().loc2);

  HalfSegment hs(true, start_loc, end_loc);
  Line* l = new Line(0);
  l->StartBulkLoad();
  hs.attr.edgeno = 0;
  *l += hs;
  hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
  *l += hs;
  l->EndBulkLoad();

//  cout<<"gp "<<end_loc<<" pave p"<<start_loc<<endl;

  vector<Line> line_list; 
  dg->LineIntersectTri(l, line_list);
  
  if(line_list.size() == 0){/////start location is on the pavement border 
      double dist = start_loc.Distance(end_loc);
      double slowspeed = 10.0*1000.0/3600.0;
      double time = dist/slowspeed;///// define as car or taxi
    //////////////////////////////////////////////////////////////
      if(dist > delta_dist){//ingore too small segment 
          Instant st = start_time;
          Instant et = start_time; 
          Interval<Instant> up_interval; 
          et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0*60));

          ////////////////////create a upoint////////////////////////
          up_interval.start = st;
          up_interval.lc = true;
          up_interval.end = et;
          up_interval.rc = false; 
          UPoint* up = new UPoint(up_interval, start_loc, end_loc);
          mo->Add(*up);
          delete up; 
          start_time = et;
          /////////////generic unit/// tm--car or taxi//////////////////////
          Loc loc1(start_loc.GetX(), start_loc.GetY());
          Loc loc2(end_loc.GetX(), end_loc.GetY());
          GenLoc gloc1(0, loc1);
          GenLoc gloc2(0, loc2);
          int tm = GetTM(mode);
          UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
          genmo->Add(*unit); 
          delete unit; 
      }
    delete l;
    pm->CloseDualGraph(dg);
    return;
  }

  assert(line_list.size() > 0);
  /////////////////////////////////////////////////////////////////////
  ///////////////////get the point on the pavement border/////////////
  ///////////////////intersection line inside pavement////////////////
  /////////////////////////////////////////////////////////////////////
  Line* line = new Line(0);
  for(unsigned int i = 0;i < line_list.size();i++){
    Line* temp = new Line(0);
    line->Union(line_list[i], *temp);
    *line = *temp;
    delete temp;
  }  

     ///////////////////////////////////////////////////////////////
   ////////we reference to the big overall large polygon/////////////
   //////////////////////////////////////////////////////////////////
   
   int gen_mo_ref_id = dg->min_tri_oid_1 + dg->GetNodeRel()->GetNoTuples() + 1;
    Rectangle<2> bbox = dg->rtree_node->BoundingBox();


//  cout<<line->Length()<<" size "<<line->Size()<<endl; 
  ///////////////////////////////////////////////////////////////////////
  //////////// the intersection line still belongs to pavement area//////
  //////////////////////////////////////////////////////////////////////
  if(fabs(l->Length() - line->Length()) < delta_dist){///////one movement

//        cout<<"still on pavement "<<endl;

        double dist = start_loc.Distance(end_loc);
        double time = dist/1.0;///// define as walk
    //////////////////////////////////////////////////////////////
       if(dist > delta_dist){//ingore too small segment 

          Instant st = start_time;
          Instant et = start_time; 
          Interval<Instant> up_interval; 
          et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0*60));

          ////////////////////create a upoint////////////////////////
          up_interval.start = st;
          up_interval.lc = true;
          up_interval.end = et;
          up_interval.rc = false; 
          UPoint* up = new UPoint(up_interval, start_loc, end_loc);
          mo->Add(*up);
          delete up; 
          start_time = et;
          ////////////////generic unit/// tm--walk////////////////////////
          Loc loc1(start_loc.GetX() - bbox.MinD(0), 
                   start_loc.GetY() - bbox.MinD(1));
          Loc loc2(end_loc.GetX() - bbox.MinD(0),
                   end_loc.GetY() - bbox.MinD(1));
          GenLoc gloc1(gen_mo_ref_id, loc1);
          GenLoc gloc2(gen_mo_ref_id, loc2);
          int tm = GetTM("Walk");
          UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
          genmo->Add(*unit); 
          delete unit; 
        }
  }else{//////////two movements 

  vector<MyPoint> mp_list;
  for(int i = 0;i < line->Size();i++){
    HalfSegment hs;
    line->Get(i, hs);
    if(!hs.IsLeftDomPoint())continue;
    Point lp = hs.GetLeftPoint();
    Point rp = hs.GetRightPoint();
    MyPoint mp1(lp, lp.Distance(end_loc));
    mp_list.push_back(mp1);
    MyPoint mp2(rp, rp.Distance(end_loc));
    mp_list.push_back(mp2);
  }
  sort(mp_list.begin(), mp_list.end());

//    for(unsigned int i = 0;i < mp_list.size();i++)
//      mp_list[i].Print();
//   cout<<endl;
  Point middle_loc = mp_list[0].loc;
  ////////////////////movement---1//////from pavemet to border/////////////
      double dist1 = start_loc.Distance(middle_loc);
      double time1 = dist1/1.0;///// define as walk
      if(dist1 > delta_dist){//ingore too small segment 

          Instant st = start_time;
          Instant et = start_time; 
          Interval<Instant> up_interval; 
          et.ReadFrom(st.ToDouble() + time1*1.0/(24.0*60.0*60));

          ////////////////////create a upoint////////////////////////
          up_interval.start = st;
          up_interval.lc = true;
          up_interval.end = et;
          up_interval.rc = false; 
          UPoint* up = new UPoint(up_interval, start_loc, middle_loc);
          mo->Add(*up);
          delete up; 
          start_time = et;
          ////////////////generic unit////////////
          /////////////tm--walk///////////////////
          //////////////////////////////////////////
          Loc loc1(start_loc.GetX() - bbox.MinD(0), 
                   start_loc.GetY() - bbox.MinD(1));
          Loc loc2(middle_loc.GetX() - bbox.MinD(0),
                   middle_loc.GetY() - bbox.MinD(1));
          GenLoc gloc1(gen_mo_ref_id, loc1);
          GenLoc gloc2(gen_mo_ref_id, loc2);
          int tm = GetTM("Walk");
          UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
          genmo->Add(*unit); 
          delete unit; 
      }

  ////////////////movement---2/////from border to road line ///free space/////
      double dist2 = middle_loc.Distance(end_loc);
      double lowspeed = 10.0*1000/3600.0;
      double time2 = dist2/lowspeed;///// define as car (slow speed)
      if(dist2 > delta_dist){//ingore too small segment 

          Instant st = start_time;
          Instant et = start_time; 
          Interval<Instant> up_interval; 
          et.ReadFrom(st.ToDouble() + time2*1.0/(24.0*60.0*60));

          ////////////////////create a upoint////////////////////////
          up_interval.start = st;
          up_interval.lc = true;
          up_interval.end = et;
          up_interval.rc = false; 
          UPoint* up = new UPoint(up_interval, middle_loc, end_loc);
          mo->Add(*up);
          delete up; 
          start_time = et;
          ////////////////generic unit////////////
          /////////////tm--car or taxi///////////////////
          //////////////////////////////////////////
          Loc loc1(middle_loc.GetX(), middle_loc.GetY());
          Loc loc2(end_loc.GetX(), end_loc.GetY());
          GenLoc gloc1(0, loc1);///////////0 -- free space 
          GenLoc gloc2(0, loc2);///////////0 -- free space 
          int tm = GetTM(mode);
          UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
          genmo->Add(*unit);
          delete unit;

      }
  } 
  //////////////////for debuging show the line//////////////////////////////
//  cout<<line->Length()<<endl;
//  line_list1.push_back(*line);
//  line_list1.push_back(*l);

  delete l;
  delete line;
  //////////////////////////////////////////////////////////////////////

  pm->CloseDualGraph(dg);
}

/*
build the moving object on the road network

*/
void GenMObject::ConnectGP1GP2(Network* rn, Point start_loc, GLine* newgl, 
                               MPoint* mo, GenMO* genmo, Instant& start_time,
                     Relation* rel, string mode)
{

    const double delta_dist = 0.01;
    for(int i = 0;i < newgl->Size();i++){
        RouteInterval* ri = new RouteInterval();
        newgl->Get(i, *ri);
        int rid = ri->GetRouteId();
        Tuple* speed_tuple = rel->GetTuple(rid, false);
     assert(((CcInt*)speed_tuple->GetAttribute(SPEED_RID))->GetIntval() == rid);
        double speed = 
        ((CcReal*)speed_tuple->GetAttribute(SPEED_VAL))->GetRealval();
        speed_tuple->DeleteIfAllowed();

      if(speed < 0.0)speed = 10.0;
      speed = speed * 1000.0/3600.0;// meters in second 
//    cout<<"rid "<<rid<<" new speed "<<speed<<endl;
      SimpleLine* sl = new SimpleLine(0);
      rn->GetLineValueOfRouteInterval(ri, sl);
    ////////////////////////////////////////////////////////////////////
    /////////////get the subline of this route interval////////////////
    ///////////////////////////////////////////////////////////////////
      SpacePartition* sp = new SpacePartition();
      vector<MyHalfSegment> seq_halfseg; //reorder it from start to end
      sp->ReorderLine(sl, seq_halfseg);
      delete sp;
    //////////////////////////////////////////////////////////////////
      delete sl;

    /////////////////////////////////////////////////////////////////////

      Interval<Instant> unit_interval;
      unit_interval.start = start_time;
      unit_interval.lc = true;

      Point temp_sp1 = seq_halfseg[0].from; 
      Point temp_sp2 = seq_halfseg[seq_halfseg.size() - 1].to; 
      if(start_loc.Distance(temp_sp1) < delta_dist){
        CreateCarTrip1(mo, seq_halfseg, start_time, speed);

        start_loc = temp_sp2;
      }else if(start_loc.Distance(temp_sp2) < delta_dist){

        CreateCarTrip2(mo, seq_halfseg, start_time, speed);
        start_loc = temp_sp1;
      }else assert(false);

      unit_interval.end = start_time;
      unit_interval.rc = false;
      
    //////////////////////////////////////////////////////////////
    ////////////////generic units/////////////////////////////////
    //////////////////////////////////////////////////////////////
      Loc loc1(ri->GetStartPos(), -1); 
      Loc loc2(ri->GetEndPos(), -1); 
      GenLoc gloc1(ri->GetRouteId(), loc1);
      GenLoc gloc2(ri->GetRouteId(), loc2);
      int tm = GetTM(mode); 

    //////////////////////////////////////////////////////////////////
    /////////////correct way to create UGenLoc///////////////////////
    //////////////////////////////////////////////////////////////////
      UGenLoc* unit = new UGenLoc(unit_interval, gloc1, gloc2, tm);
      genmo->Add(*unit); 
      delete unit; 
      delete ri;

  }
   /////////////////////////////////////////////////////////////////////
   //////////////////for debuging////////////////////////////////////
   //////////////////////////////////////////////////////////////////
   
//     Line* l = new Line(0);
//     newgl->Gline2line(l);
//     Line* res = new Line(0);
//     l->Union(line_list1[line_list1.size() - 1], *res);
//     line_list1[line_list1.size() - 1] = *res;
//     delete res;
//     delete l;

}


/*
build the connection from a point on the pavement to the road network point
subpath1: on the pavement
subpath2: in the free space (mode: car)
loc: point on the pavement; start loc: point maps to a gpoint 

*/
void GenMObject::ConnectEndMove(Point start_loc, GenLoc loc, MPoint* mo, 
                        GenMO* genmo, Instant& start_time, 
                                Pavement* pm, string mode)
{
  const double delta_dist = 0.01;
  DualGraph* dg = pm->GetDualGraph();

  Point end_loc(true, loc.GetLoc().loc1, loc.GetLoc().loc2);

  HalfSegment hs(true, start_loc, end_loc);
  Line* l = new Line(0);
  l->StartBulkLoad();
  hs.attr.edgeno = 0;
  *l += hs;
  hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
  *l += hs;
  l->EndBulkLoad();

//  cout<<"gp "<<end_loc<<" pave p"<<start_loc<<endl;

  vector<Line> line_list; 
  dg->LineIntersectTri(l, line_list);

  if(line_list.size() == 0){///////the point on the pavement is on the border

      double dist = start_loc.Distance(end_loc);
      double slowspeed = 10.0*1000.0/3600.0;
      double time = dist/slowspeed;///// define as car or taxi
    //////////////////////////////////////////////////////////////
      if(dist > delta_dist){//ingore too small segment 
          Instant st = start_time;
          Instant et = start_time; 
          Interval<Instant> up_interval; 
          et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0*60));

          ////////////////////create a upoint////////////////////////
          up_interval.start = st;
          up_interval.lc = true;
          up_interval.end = et;
          up_interval.rc = false; 
          UPoint* up = new UPoint(up_interval, start_loc, end_loc);
          mo->Add(*up);
          delete up; 
          start_time = et;
          /////////////generic unit/// tm--car or taxi//////////////////////
          Loc loc1(start_loc.GetX(), start_loc.GetY());
          Loc loc2(end_loc.GetX(), end_loc.GetY());
          GenLoc gloc1(0, loc1);
          GenLoc gloc2(0, loc2);
          int tm = GetTM(mode);
          UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
          genmo->Add(*unit); 
          delete unit; 
      }
    delete l;
    pm->CloseDualGraph(dg);
    return;
  }

  assert(line_list.size() > 0); 

  /////////////////////////////////////////////////////////////////////
  ///////////////////get the point on the pavement border/////////////
  //////////////////intersection line inside the pavement//////////////
  /////////////////////////////////////////////////////////////////////
  Line* line = new Line(0);
  for(unsigned int i = 0;i < line_list.size();i++){
    Line* temp = new Line(0);
    line->Union(line_list[i], *temp);
    *line = *temp;
    delete temp;
  }

   ///////////////////////////////////////////////////////////////
   ////////we reference to the big overall large polygon/////////////
   //////////////////////////////////////////////////////////////////

   int gen_mo_ref_id = dg->min_tri_oid_1 + dg->GetNodeRel()->GetNoTuples() + 1;
   Rectangle<2> bbox = dg->rtree_node->BoundingBox();

  ///////////////////////////////////////////////////////////////////////
  //////////// the intersection line still belongs to pavement area//////
  //////////////////////////////////////////////////////////////////////
  if(fabs(l->Length() - line->Length()) < delta_dist){///////one movement

//        cout<<"still on pavement "<<endl;

        double dist = start_loc.Distance(end_loc);
        double time = dist/1.0;///// define as walk
    //////////////////////////////////////////////////////////////
       if(dist > delta_dist){//ingore too small segment 

          Instant st = start_time;
          Instant et = start_time; 
          Interval<Instant> up_interval; 
          et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0*60));

          ////////////////////create a upoint////////////////////////
          up_interval.start = st;
          up_interval.lc = true;
          up_interval.end = et;
          up_interval.rc = false; 
          UPoint* up = new UPoint(up_interval, start_loc, end_loc);
          mo->Add(*up);
          delete up; 
          start_time = et;
          ////////////////generic unit/// tm--walk////////////////////////
          Loc loc1(start_loc.GetX() - bbox.MinD(0), 
                   start_loc.GetY() - bbox.MinD(1));
          Loc loc2(end_loc.GetX() - bbox.MinD(0),
                   end_loc.GetY() - bbox.MinD(1));
          GenLoc gloc1(gen_mo_ref_id, loc1);
          GenLoc gloc2(gen_mo_ref_id, loc2);
          int tm = GetTM("Walk");
          UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
          genmo->Add(*unit); 
          delete unit; 
        }
  }else{  //////////two movements 
        vector<MyPoint> mp_list;
        for(int i = 0;i < line->Size();i++){
            HalfSegment hs;
            line->Get(i, hs);
            if(!hs.IsLeftDomPoint())continue;
            Point lp = hs.GetLeftPoint();
            Point rp = hs.GetRightPoint();
            MyPoint mp1(lp, lp.Distance(start_loc));
            mp_list.push_back(mp1);
            MyPoint mp2(rp, rp.Distance(start_loc));
            mp_list.push_back(mp2);
        }
        sort(mp_list.begin(), mp_list.end());
        Point middle_loc = mp_list[0].loc;
      //////////////movement---1//////from road line to pavemet border///////
        double dist1 = start_loc.Distance(middle_loc);
        double lowspeed = 10*1000/3600.0;
        double time1 = dist1/lowspeed;///// define as car or taxi 
        if(dist1 > delta_dist){//ingore too small segment 

          Instant st = start_time;
          Instant et = start_time; 
          Interval<Instant> up_interval; 
          et.ReadFrom(st.ToDouble() + time1*1.0/(24.0*60.0*60));

          ////////////////////create a upoint////////////////////////
          up_interval.start = st;
          up_interval.lc = true;
          up_interval.end = et;
          up_interval.rc = false; 
          UPoint* up = new UPoint(up_interval, start_loc, middle_loc);
          mo->Add(*up);
          delete up; 
          start_time = et;
          ////////////////generic unit////////////
          /////////////tm--car or taxi///////////////////
          //////////////////////////////////////////
          Loc loc1(start_loc.GetX(), start_loc.GetY());
          Loc loc2(middle_loc.GetX(), middle_loc.GetY());
          GenLoc gloc1(0, loc1);///////////0 -- free space 
          GenLoc gloc2(0, loc2);///////////0 -- free space 
          int tm = GetTM(mode);
          UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
          genmo->Add(*unit); 
          delete unit; 
        }

        ///movement---2/////from pavement border to pavement loc///free space//
        double dist2 = middle_loc.Distance(end_loc);
        double time2 = dist2/1.0;///// define as walk
        if(dist2 > delta_dist){//ingore too small segment 

          Instant st = start_time;
          Instant et = start_time; 
          Interval<Instant> up_interval; 
          et.ReadFrom(st.ToDouble() + time2*1.0/(24.0*60.0*60));

          ////////////////////create a upoint////////////////////////
          up_interval.start = st;
          up_interval.lc = true;
          up_interval.end = et;
          up_interval.rc = false; 
          UPoint* up = new UPoint(up_interval, middle_loc, end_loc);
          mo->Add(*up);
          delete up; 
          start_time = et;
          ////////////////generic unit////////////
          /////////////tm--walk///////////////////
          //////////////////////////////////////////
          Loc loc1(middle_loc.GetX() - bbox.MinD(0), 
                   middle_loc.GetY() - bbox.MinD(1));
          Loc loc2(end_loc.GetX() - bbox.MinD(0), 
                   end_loc.GetY() - bbox.MinD(1));
          GenLoc gloc1(gen_mo_ref_id, loc1);///walk on the overall pavement
          GenLoc gloc2(gen_mo_ref_id, loc2);///walk on the overall pavement
          int tm = GetTM("Walk");
          UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
          genmo->Add(*unit); 
          delete unit; 
        }
  }

  //////////////////for debuging show the line//////////////////////////////
//  cout<<line->Length()<<endl;

//   Line* res = new Line(0);
//   l->Union(line_list1[line_list1.size() - 1], *res);
//   line_list1[line_list1.size() - 1] = *res;
//   delete res;


  delete l;
  delete line;
  //////////////////////////////////////////////////////////////////////
  pm->CloseDualGraph(dg);

}


//////////////////////////////////////////////////////////////////////
//////////////////////Global Space////////////////////////////////////
//////////////////////////////////////////////////////////////////////
string Space::FreeSpaceTypeInfo = "(rel (tuple ((oid int))))";

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
    Space* sp = new Space(true, space_id);
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
 ListExpr space_list = nl->TwoElemList(nl->StringAtom("SpaceId:"),
                        nl->IntAtom(sp->GetSpaceId())); 

 ListExpr infra_list = nl->TheEmptyList();
 bool bFirst = true;
 ListExpr xNext = nl->TheEmptyList();
 ListExpr xLast = nl->TheEmptyList();
 for(int i = 0;i < sp->Size();i++){
  InfraRef inf_ref;
  sp->Get(i, inf_ref); 
  string str = GetSymbolStr(inf_ref.infra_type);
  ListExpr list1 = nl->OneElemList(nl->StringAtom(str));
  ListExpr list2 = nl->TwoElemList(nl->StringAtom("InfraId:"),
                        nl->IntAtom(inf_ref.infra_id));
  ListExpr list3 = nl->TwoElemList(
                           nl->TwoElemList(
                                nl->StringAtom("min ref id:"),
                                nl->IntAtom(inf_ref.ref_id_low)),
                           nl->TwoElemList(
                                nl->StringAtom("max ref id:"),
                                nl->IntAtom(inf_ref.ref_id_high)));

  xNext = nl->ThreeElemList(list1, list2, list3);
  if(bFirst){
      infra_list = nl->OneElemList(xNext);
      xLast = infra_list;
      bFirst = false;
  }else
    xLast = nl->Append(xLast,xNext);
 }
 
 return nl->TwoElemList(space_list, infra_list);
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
  return SetWord (new Space(false));
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


Space::Space(const Space& sp):Attribute(sp.IsDefined()), infra_list(0)
{
  if(sp.IsDefined()){
    def = sp.def;
    space_id = sp.space_id; 
    for(int i = 0;i < sp.Size();i++){
      InfraRef inf_ref; 
      sp.Get(i, inf_ref); 
      infra_list.Append(inf_ref);
    }
  }

}


Space& Space::operator=(const Space& sp)
{
  SetDefined(sp.IsDefined());
  if(def){
    space_id = sp.space_id;
    for(int i = 0;i < sp.Size();i++){
      InfraRef inf_ref; 
      sp.Get(i, inf_ref); 
      infra_list.Append(inf_ref);
    }
  }
  return *this; 
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


/*
get infrastructure ref representation 

*/
void Space::Get(int i, InfraRef& inf_ref) const 
{
  assert(0 <= i && i < infra_list.Size());
  infra_list.Get(i, inf_ref);
}

void Space::Add(InfraRef& inf_ref)
{
  infra_list.Append(inf_ref); 
}

/*
add network infrastructure to the space 

*/
void Space::AddRoadNetwork(Network* n)
{
  InfraRef inf_ref; 
  if(!n->IsDefined()){
    cout<<"road network is not defined"<<endl;
    return; 
  }
  inf_ref.infra_id = n->GetId();
  inf_ref.infra_type = GetSymbol("LINE"); 
  int min_id = numeric_limits<int>::max();
  int max_id = numeric_limits<int>::min();
  Relation* routes_rel = n->GetRoutes();
  for(int i = 1;i <= routes_rel->GetNoTuples();i++){
    Tuple* route_tuple = routes_rel->GetTuple(i, false);
    int rid = ((CcInt*)route_tuple->GetAttribute(ROUTE_ID))->GetIntval(); 
    if(rid < min_id) min_id = rid;
    if(rid > max_id) max_id = rid; 
    route_tuple->DeleteIfAllowed(); 
  }
  inf_ref.ref_id_low = min_id;
  inf_ref.ref_id_high = max_id; 
  if(CheckExist(inf_ref) == false){
      inf_ref.Print(); 
      Add(inf_ref); 
  }else
    cout<<"this infrastructure exists already"<<endl; 
}

/*
check whether the infrastructure has been added already 
and overlapping oids 

*/
bool Space::CheckExist(InfraRef& inf_ref)
{

  for(int i = 0;i < infra_list.Size();i++){
    InfraRef elem;
    infra_list.Get(i, elem); 
    if(elem.infra_type == inf_ref.infra_type && 
       elem.infra_id == inf_ref.infra_id) return true; 

    if(inf_ref.ref_id_low > elem.ref_id_high || 
       inf_ref.ref_id_high < elem.ref_id_low){
    }else
      return true; 
  }
  return false; 
}

/*
get the required infrastructure relation by type 

*/
Relation* Space::GetInfra(string type)
{
  Relation* result = NULL;
  int infra_type = GetSymbol(type);

  if(infra_type == IF_LINE){ //////////road network 
      Network* rn = LoadRoadNetwork(IF_LINE);
      if(rn != NULL){ 
          result = rn->GetRoutes()->Clone();
          CloseRoadNetwork(rn);
      }else{
          cout<<"road network does exist "<<endl; 
          ListExpr xTypeInfo;
          nl->ReadFromString(Network::routesTypeInfo, xTypeInfo);
          ListExpr xNumType = 
                SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
          result = new Relation(xNumType, true);
      }
   }else if(infra_type == IF_FREESPACE){//////////free space 
       ListExpr xTypeInfo;
       nl->ReadFromString(FreeSpaceTypeInfo, xTypeInfo);
       ListExpr xNumType = 
                SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
       result = new Relation(xNumType, true);
   }else if(infra_type == IF_REGION){ //////pavement 
      Pavement* pn = LoadPavement(IF_REGION);
      if(pn != NULL){ 
          result = pn->GetPaveRel()->Clone();
          ClosePavement(pn);
      }else{
          cout<<"pavement does exist "<<endl; 
          ListExpr xTypeInfo;
          nl->ReadFromString(Pavement::PaveTypeInfo, xTypeInfo);
          ListExpr xNumType = 
                SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
          result = new Relation(xNumType, true);
      }
   }else if(infra_type == IF_BUSSTOP){ ////bus stops 
        BusNetwork* bn = LoadBusNetwork(IF_BUSNETWORK);
        if(bn != NULL){ 
          result = bn->GetBS_Rel()->Clone();
          CloseBusNetwork(bn);
        }else{
          cout<<"bus network does exist "<<endl; 
          ListExpr xTypeInfo;
          nl->ReadFromString(BusNetwork::BusStopsInternalTypeInfo, xTypeInfo);
          ListExpr xNumType = 
                SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
          result = new Relation(xNumType, true);
        }
   }else if(infra_type == IF_BUSROUTE){ ///bus routes 
        BusNetwork* bn = LoadBusNetwork(IF_BUSNETWORK);
        if(bn != NULL){ 
          result = bn->GetBR_Rel()->Clone();
          CloseBusNetwork(bn);
        }else{
          cout<<"bus network does exist "<<endl; 
          ListExpr xTypeInfo;
          nl->ReadFromString(BusNetwork::BusRoutesTypeInfo, xTypeInfo);
          ListExpr xNumType = 
                SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
          result = new Relation(xNumType, true);
        }

   }else if(infra_type == IF_MPPTN){ ///bus trips 
        BusNetwork* bn = LoadBusNetwork(IF_BUSNETWORK);
        if(bn != NULL){ 
          result = bn->GetBT_Rel()->Clone();
          CloseBusNetwork(bn);
        }else{
          cout<<"bus network does exist "<<endl; 
          ListExpr xTypeInfo;
          nl->ReadFromString(BusNetwork::BusTripsTypeInfo, xTypeInfo);
          ListExpr xNumType = 
                SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
          result = new Relation(xNumType, true);
        }

   }else{
        cout<<"wrong type or does not exist"<<endl;
   }
  if(result == NULL){/////////returns an empty relation 
      ListExpr xTypeInfo;
      nl->ReadFromString(FreeSpaceTypeInfo, xTypeInfo);
      ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
      result = new Relation(xNumType, true);
  }
  return result; 
}

/*
Load the road network

*/
Network* Space:: LoadRoadNetwork(int type)
{
  InfraRef rn_ref; 
  bool found = false; 
  for(int i = 0;i < infra_list.Size();i++){
    InfraRef elem;
    infra_list.Get(i, elem); 
    if(elem.infra_type == type){
       rn_ref = elem; 
       found = true;
       break; 
    }
  }
  if(found){
      ListExpr xObjectList = SecondoSystem::GetCatalog()->ListObjects();
      xObjectList = nl->Rest(xObjectList);
      while(!nl->IsEmpty(xObjectList)){
          // Next element in list
          ListExpr xCurrent = nl->First(xObjectList);
          xObjectList = nl->Rest(xObjectList);
          // Type of object is at fourth position in list
          ListExpr xObjectType = nl->First(nl->Fourth(xCurrent));
          if(nl->IsAtom(xObjectType) &&
              nl->SymbolValue(xObjectType) == "network"){
            // Get name of the bus graph 
            ListExpr xObjectName = nl->Second(xCurrent);
            string strObjectName = nl->SymbolValue(xObjectName);

            // Load object to find out the id of the network. Normally their
            // won't be to much networks in one database giving us a good
            // chance to load only the wanted network.
            Word xValue;
            bool bDefined;
            bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
            if(!bDefined || !bOk){
              // Undefined
              continue;
            }
            Network* rn = (Network*)xValue.addr;
            if(rn->GetId() == rn_ref.infra_id){
              // This is the road network we have been looking for
              return rn;
            }
          }
      }
  }
    return NULL;
}

/*
close the road network 

*/
void Space::CloseRoadNetwork(Network* rn)
{
  if(rn == NULL) return; 
  Word xValue;
  xValue.addr = rn;
  SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom( "network" ),
                                           xValue);
}


/*
add pavement infrastructure to the space 
for the pavement infrastructure, we record the maximum is the largest 
  triangle id plus 1. the largest id means the overall polygon pavement 

*/
void Space::AddPavement(Pavement* pn)
{
  InfraRef inf_ref; 
  if(!pn->IsDefined()){
    cout<<"pavement is not defined"<<endl;
    return; 
  }
  inf_ref.infra_id = pn->GetId();
  inf_ref.infra_type = GetSymbol("REGION"); 
  int min_id = numeric_limits<int>::max();
  int max_id = numeric_limits<int>::min();
  Relation* pave_rel = pn->GetPaveRel();
  for(int i = 1;i <= pave_rel->GetNoTuples();i++){
    Tuple* pave_tuple = pave_rel->GetTuple(i, false);
    int id = ((CcInt*)pave_tuple->GetAttribute(Pavement::P_OID))->GetIntval();
    if(id < min_id) min_id = id;
    if(id > max_id) max_id = id; 
    pave_tuple->DeleteIfAllowed(); 
  }
  inf_ref.ref_id_low = min_id;
  inf_ref.ref_id_high = max_id + 1;///for the overall large region 
  if(CheckExist(inf_ref) == false){
      inf_ref.Print(); 
      Add(inf_ref); 
  }else{
    cout<<"insert infrastructure wroing"<<endl; 
    cout<<"infrastructure exists already or wrong oid"<<endl; 
  }
}

/*
load pavement infrastructure 

*/
Pavement* Space::LoadPavement(int type)
{
  InfraRef rn_ref; 
  bool found = false; 
  for(int i = 0;i < infra_list.Size();i++){
    InfraRef elem;
    infra_list.Get(i, elem); 
    if(elem.infra_type == type){
       rn_ref = elem; 
       found = true;
       break; 
    }
  }
  if(found){
      ListExpr xObjectList = SecondoSystem::GetCatalog()->ListObjects();
      xObjectList = nl->Rest(xObjectList);
      while(!nl->IsEmpty(xObjectList)){
          // Next element in list
          ListExpr xCurrent = nl->First(xObjectList);
          xObjectList = nl->Rest(xObjectList);
          // Type of object is at fourth position in list
          ListExpr xObjectType = nl->First(nl->Fourth(xCurrent));
          if(nl->IsAtom(xObjectType) &&
              nl->SymbolValue(xObjectType) == "pavenetwork"){
            // Get name of the bus graph 
            ListExpr xObjectName = nl->Second(xCurrent);
            string strObjectName = nl->SymbolValue(xObjectName);

            // Load object to find out the id of the pavement
            Word xValue;
            bool bDefined;
            bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
            if(!bDefined || !bOk){
              // Undefined
              continue;
            }
            Pavement* pn = (Pavement*)xValue.addr;
            if((int)pn->GetId() == rn_ref.infra_id){
              // This is the pavement we have been looking for

              return pn;
            }
          }
      }
  }
  return NULL;
}


void Space::ClosePavement(Pavement* pn)
{
 if(pn == NULL) return; 
  Word xValue;
  xValue.addr = pn;
  SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom( "pavenetwork" ),
                                           xValue);
}

/*
add bus network into the space 

*/
void Space::AddBusNetwork(BusNetwork* bn)
{
  InfraRef inf_ref; 
  if(!bn->IsDefined()){
    cout<<"bus network is not defined"<<endl;
    return; 
  }
  inf_ref.infra_id = bn->GetId();
  inf_ref.infra_type = GetSymbol("BUSNETWORK"); 
  int min_id = numeric_limits<int>::max();
  int max_id = numeric_limits<int>::min();
  Relation* bn_routes = bn->GetBR_Rel();
  for(int i = 1;i <= bn_routes->GetNoTuples();i++){
    Tuple* br_tuple = bn_routes->GetTuple(i, false);
    int br_id = 
       ((CcInt*)br_tuple->GetAttribute(BusNetwork::BN_BR_OID))->GetIntval();
    if(br_id < min_id) min_id = br_id;
    if(br_id > max_id) max_id = br_id; 
    br_tuple->DeleteIfAllowed(); 
  }

  Relation* bn_bus = bn->GetBT_Rel();
  for(int i = 1;i <= bn_bus->GetNoTuples();i++){
    Tuple* bus_tuple = bn_bus->GetTuple(i, false);
    int bus_id = 
       ((CcInt*)bus_tuple->GetAttribute(BusNetwork::BN_BUS_OID))->GetIntval();
    if(bus_id < min_id) min_id = bus_id;
    if(bus_id > max_id) max_id = bus_id; 
    bus_tuple->DeleteIfAllowed(); 
  }


  inf_ref.ref_id_low = min_id;
  inf_ref.ref_id_high = max_id; 
  if(CheckExist(inf_ref) == false){
      inf_ref.Print(); 
      Add(inf_ref); 
  }else
    cout<<"this infrastructure exists already"<<endl; 

}

/*
load the bus network 

*/
BusNetwork* Space::LoadBusNetwork(int type)
{

  InfraRef rn_ref; 
  bool found = false; 
  for(int i = 0;i < infra_list.Size();i++){
    InfraRef elem;
    infra_list.Get(i, elem); 
    if(elem.infra_type == type){
       rn_ref = elem; 
       found = true;
       break; 
    }
  }
  if(found){
      ListExpr xObjectList = SecondoSystem::GetCatalog()->ListObjects();
      xObjectList = nl->Rest(xObjectList);
      while(!nl->IsEmpty(xObjectList)){
          // Next element in list
          ListExpr xCurrent = nl->First(xObjectList);
          xObjectList = nl->Rest(xObjectList);
          // Type of object is at fourth position in list
          ListExpr xObjectType = nl->First(nl->Fourth(xCurrent));
          if(nl->IsAtom(xObjectType) &&
              nl->SymbolValue(xObjectType) == "busnetwork"){
            // Get name of the bus graph 
            ListExpr xObjectName = nl->Second(xCurrent);
            string strObjectName = nl->SymbolValue(xObjectName);

            // Load object to find out the id of the pavement
            Word xValue;
            bool bDefined;
            bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
            if(!bDefined || !bOk){
              // Undefined
              continue;
            }
            BusNetwork* bn = (BusNetwork*)xValue.addr;
            if((int)bn->GetId() == rn_ref.infra_id){
              // This is the busnetwork we have been looking for

              return bn;
            }
          }
      }
  }
  return NULL;

}

void Space::CloseBusNetwork(BusNetwork* bn)
{
  if(bn == NULL) return; 
  Word xValue;
  xValue.addr = bn;
  SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom( "busnetwork" ),
                                           xValue);
}


/*
get the movement inside an infrastructure object 

*/
void Space::GetLineInIFObject(int oid, GenLoc gl1, GenLoc gl2, Line* l)
{
  assert(oid >= 0); 
  
  InfraRef elem;
  int infra_type = -1;
  for(int i = 0;i < infra_list.Size();i++){
    infra_list.Get(i, elem); 
    if(elem.ref_id_low <= oid && oid <= elem.ref_id_high){
      infra_type = elem.infra_type;
      break;
    }
    if(oid == 0){ ///////////////in free space
      infra_type = IF_FREESPACE;
      break;
    }
  }
  if(infra_type < 0){
    cout<<"error no such infrastructure object"<<endl;
    return;
  }
  switch(infra_type){
    case IF_LINE:
      cout<<"road network "<<endl;
      GetLineInRoad(oid, gl1, gl2, l);
      break;

    case IF_REGION:
      cout<<"region based outdoor"<<endl;
      GetLineInRegion(oid, gl1, gl2, l);
      break; 

    case IF_FREESPACE:
      cout<<"free space"<<endl;
      GetLineInFreeSpace(gl1, gl2, l);
      break;

    case IF_MPPTN:
      cout<<"bus network"<<endl;
      GetLineInBusNetwork(oid, gl1, gl2, l);
      break;

    case IF_GROOM:
      cout<<"indoor "<<endl;
      GetLineInGRoom(oid, gl1, gl2, l);
      break;

    default:
      assert(false);
      break;
  }
}

/*
get the sub movement on a road 

*/
void Space::GetLineInRoad(int oid, GenLoc gl1, GenLoc gl2, Line* l)
{
  Network* rn = LoadRoadNetwork(IF_LINE);
  Tuple* route_tuple = rn->GetRoute(oid);
  SimpleLine* sl = (SimpleLine*)route_tuple->GetAttribute(ROUTE_CURVE);
  bool dual = ((CcBool*)route_tuple->GetAttribute(ROUTE_DUAL))->GetBoolval();

//   cout<<"pos1 "<<gl1.GetLoc().loc1<<" "
//       <<gl2.GetLoc().loc1<<" dual "<<dual<<endl;

  double pos1 = gl1.GetLoc().loc1;
  double pos2 = gl2.GetLoc().loc1;
  if(dual){
    if(pos1 > pos2){
      double pos = pos1;
      pos1 = pos2;
      pos2 = pos;
    }
  }else{
    if(pos1 < pos2){
      double pos = pos1;
      pos1 = pos2;
      pos2 = pos;
    }
  }

  SimpleLine* sub_sl = new SimpleLine(0);
  sl->SubLine(pos1, pos2, dual, *sub_sl);
  Rectangle<2> bbox = sl->BoundingBox();

//  cout<<"length1 "<<sl->Length()<<" length2 "<<sub_sl->Length()<<endl; 

  int edgeno = 0;
  for(int i = 0;i < sub_sl->Size();i++){
    HalfSegment hs1;
    sub_sl->Get(i, hs1);
    if(!hs1.IsLeftDomPoint())continue;
    Point lp = hs1.GetLeftPoint();
    Point rp = hs1.GetRightPoint();
    Point newlp(true, lp.GetX() - bbox.MinD(0), lp.GetY() - bbox.MinD(1));
    Point newrp(true, rp.GetX() - bbox.MinD(0), rp.GetY() - bbox.MinD(1));
    HalfSegment hs2(true, newlp, newrp);
    hs2.attr.edgeno = edgeno++;
    *l += hs2;
    hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
    *l += hs2;
  }

  delete sub_sl;
  route_tuple->DeleteIfAllowed();
  CloseRoadNetwork(rn);


}


/*
get the sub movement in a region

*/
void Space::GetLineInRegion(int oid, GenLoc gl1, GenLoc gl2, Line* l)
{
  Point p1(true, gl1.GetLoc().loc1, gl1.GetLoc().loc2);
  Point p2(true, gl2.GetLoc().loc1, gl2.GetLoc().loc2);
  HalfSegment hs(true, p1, p2);
  hs.attr.edgeno = 0;
  *l += hs;
  hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
  *l += hs;
}

/*
get the subment in free space 

*/
void Space::GetLineInFreeSpace(GenLoc gl1, GenLoc gl2, Line* l)
{
  Point p1(true, gl1.GetLoc().loc1, gl1.GetLoc().loc2);
  Point p2(true, gl2.GetLoc().loc1, gl2.GetLoc().loc2);
  HalfSegment hs(true, p1, p2);
  hs.attr.edgeno = 0;
  *l += hs;
  hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
  *l += hs;
}

/*
get the sub movement in bus network, bus routes 

*/
void Space::GetLineInBusNetwork(int oid, GenLoc gl1, GenLoc gl2, Line* l)
{
  cout<<"not implemented"<<endl;



}

/*
get the sub movement in indoor environment

*/
void Space::GetLineInGRoom(int oid, GenLoc gl1, GenLoc gl2, Line* l)
{
  cout<<"not implemented"<<endl;

}
