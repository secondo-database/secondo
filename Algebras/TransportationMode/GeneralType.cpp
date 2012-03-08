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
#include "RoadNetwork.h"
#include <sys/timeb.h>
#include "GSLAlgebra.h"

///////////////////////////random number generator//////////////////////////
unsigned long int GetRandom()
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

double Gen_DiffTimeb(struct timeb* t1, struct timeb* t2)
{
  double dt1 = t1->time + (double)t1->millitm/1000.0;
  double dt2 = t2->time + (double)t2->millitm/1000.0;
  return dt1 - dt2; 
}
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

//   if(genl->GetOid() > 0){
//       Loc loc = genl->GetLoc();
//       if(loc.loc1 >= 0.0 && loc.loc2 >= 0.0){
//           ListExpr loc_list = nl->TwoElemList(
//                     nl->RealAtom(loc.loc1),
//                     nl->RealAtom(loc.loc2));
//       return nl->TwoElemList(nl->IntAtom(genl->GetOid()), loc_list);
//       }else if(loc.loc1 >= 0.0 && loc.loc2 < 0.0){
//         ListExpr loc_list = nl->OneElemList(nl->RealAtom(loc.loc1));
//         return nl->TwoElemList(nl->IntAtom(genl->GetOid()), loc_list);
//       }else if(loc.loc1 < 0.0 && loc.loc2 < 0.0){
//         return nl->OneElemList(nl->IntAtom(genl->GetOid()));
//       }else
//       return nl->TheEmptyList(); 
//   }else{  //free space oid = 0 
//           Loc loc = genl->GetLoc();
//           ListExpr loc_list = nl->TwoElemList(
//                     nl->RealAtom(loc.loc1),
//                     nl->RealAtom(loc.loc2));
//           return nl->OneElemList(loc_list);
//   }


  if(genl->GetOid() > 0){
      Loc loc = genl->GetLoc();
//      cout<<loc.loc1<<" "<<loc.loc2<<endl;
      if(loc.loc1 >= 0.0 && loc.loc2 >= 0.0){
          ListExpr loc_list = nl->TwoElemList(
                    nl->RealAtom(loc.loc1),
                    nl->RealAtom(loc.loc2));
          return nl->TwoElemList(nl->IntAtom(genl->GetOid()), loc_list);
      }else if(loc.loc1 >= 0.0 && loc.loc2 < 0.0){
        if(fabs(loc.loc2 - UNDEFVAL) < EPSDIST){
            ListExpr loc_list = nl->OneElemList(nl->RealAtom(loc.loc1));
            return nl->TwoElemList(nl->IntAtom(genl->GetOid()), loc_list);
        }else{
          ListExpr loc_list = nl->TwoElemList(
                    nl->RealAtom(loc.loc1),
                    nl->RealAtom(loc.loc2));
          return nl->TwoElemList(nl->IntAtom(genl->GetOid()), loc_list);
        }
      }else if(loc.loc1 < 0.0 && loc.loc2 < 0.0){
        if(fabs(loc.loc1 - UNDEFVAL) < EPSDIST && 
               fabs(loc.loc2 - UNDEFVAL) < EPSDIST)
        return nl->OneElemList(nl->IntAtom(genl->GetOid()));

        else{
          if(fabs(loc.loc2 - UNDEFVAL) < EPSDIST){
            ListExpr loc_list = nl->OneElemList(nl->RealAtom(loc.loc1));
            return nl->TwoElemList(nl->IntAtom(genl->GetOid()), loc_list);
          }
        }

      }else
          return nl->TheEmptyList(); 
  }else{  //free space oid = 0 
          Loc loc = genl->GetLoc();
          ListExpr loc_list = nl->TwoElemList(
                    nl->RealAtom(loc.loc1),
                    nl->RealAtom(loc.loc2));
          return nl->OneElemList(loc_list);
  }

  return nl->TheEmptyList();

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

ListExpr IntimeGenLocProperty()
{
    return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                       nl->StringAtom("intimegenloc"),
           nl->StringAtom("(instant genloc)"),
           nl->StringAtom("((instant 0.5 ) (2 (1.0, 1.0)))"))));

}

bool CheckIntimeGenLoc(ListExpr type, ListExpr& errorInfo)
{
  return nl->IsEqual(type, IGenLoc::BasicType());
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
each halfsegment is stored twice: left dom point and right dom point 

*/
double GenRange::Length()
{
  double l = 0; 
  for(int i = 0;i < SegSize();i++){
    HalfSegment hs;
    GetSeg(i, hs); 
    l += hs.Length(); 
  }
  return l/2.0; 
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
        errmsg = "InUGneLoc(): first instant must be defined!.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
       nl->Second( first ),
                                           errorPos, errorInfo, correct ).addr;

      if( !correct  || !end->IsDefined() )
      {
        errmsg = "InUGneLoc(): second instant must be defined!.";
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
        errmsg = "InUGneLoc(): two parameters for GenLoc.";
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
        errmsg = "InUGneLoc(): two parameters for GenLoc.";
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
//    o<<gloc.timeInterval<<" "<<gloc.gloc1<<" "
    gloc.timeInterval.Print(cout);
    o<<" "<<gloc.gloc1<<" "
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
const Rectangle<3> UGenLoc::BoundingBox(const Geoid* geoid) const
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
//    cout<<"CopyFrom "<<endl; 
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
/*  assert(unit.IsDefined());
  assert(unit.IsValid()); 
  if(!IsDefined()){
    SetDefined(false);
    return; 
  }
  assert(unit.gloc1.GetOid() == unit.gloc2.GetOid()); 
  units.Append(unit);*/ 
  

  /////////////////////////////////////////////////////////
  //////merge units reference to the same bus and metro////
  /////////////////////////////////////////////////////////
//  if(unit.IsValid() == false)cout<<unit<<endl;
  assert(unit.IsDefined());
  assert(unit.IsValid()); 
  if(!IsDefined()){
    SetDefined(false);
    return; 
  }
  assert(unit.gloc1.GetOid() == unit.gloc2.GetOid()); 
  if(units.Size() == 0)
    units.Append(unit); 
  else{
    UGenLoc last_unit;
    assert(units.Get(units.Size() - 1, last_unit));
    if((unit.tm == TM_BUS && last_unit.tm == TM_BUS) || 
       (unit.tm == TM_METRO && last_unit.tm == TM_METRO)){//only for bus, metro

        if(unit.gloc1.GetOid() == last_unit.gloc1.GetOid() &&
           unit.gloc2.GetOid() == last_unit.gloc2.GetOid()){//the same vehicle
             ///////////////gloc2 (-1.0, -1.0) ////////////////
            if(unit.gloc1.IsLocDef() == false && 
               unit.gloc2.IsLocDef() == false && 
               last_unit.gloc1.IsLocDef() == false && 
               last_unit.gloc2.IsLocDef() == false){ ///only when (-1, -1)
               /////////////not include the same route////////////////

//            cout<<unit.gloc2.GetOid()<<" "
//                <<last_unit.timeInterval<<" "<<unit.timeInterval<<endl;
            Interval<Instant> res = unit.timeInterval;
            res.Union(last_unit.timeInterval);
//            cout<<"union "<<res<<endl;
            last_unit.timeInterval = res;
            assert(units.Put(units.Size() - 1, last_unit));
            }else{
                units.Append(unit);
            }
        }else{
          units.Append(unit);
        }
    }else{
      units.Append(unit); 
    }
  }

}

void GenMO::Append(const UGenLoc& unit)
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
//    Mapping<UGenLoc, GenLoc>::EndBulkLoad(false, false);//sort needs time 
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
    mo.EndBulkLoad( false, false );
}

/*
get the trajectory of a generic moving object. it needs the space 
because loc1 loc2 have different meanings for different infrastructures 

a better method is we first open all infrastructures and store the pointers
after processing we close all of them.

if it references to the same bus trip, we load the trajectory in a bulkload way
  
*/
void GenMO::Trajectory(GenRange* genrange, Space* sp)
{
  genrange->Clear();
  
  vector<void*> infra_pointer; 
  sp->OpenInfra(infra_pointer);

  for(int i = 0;i < GetNoComponents();i++){
      UGenLoc unit;
      Get(i, unit);
      int oid = unit.gloc1.GetOid();
      int tm = unit.tm;

      int infra_type = sp->GetInfraType(oid);

      Line* l = new Line(0);
      l->StartBulkLoad();
      if(infra_type == IF_BUSNETWORK && i < GetNoComponents() - 1){
        int j = i + 1;
        UGenLoc unit2;
        Get(j, unit2);
        int oid2 = unit2.gloc1.GetOid();
        int infra_type2 = sp->GetInfraType(oid2);
        Interval<Instant> time_range = unit.timeInterval;
        ///////////////same bus trip/////////////////////////////
        while(infra_type2 == IF_BUSNETWORK && oid2 == oid){
            time_range.end = unit2.timeInterval.end;
            j++;
            if(j == GetNoComponents()) break;
            Get(j, unit2);
            oid2 = unit2.gloc1.GetOid();
            infra_type2 = sp->GetInfraType(oid2);
        }

        sp->GetLineInIFObject(oid, unit.gloc1, unit.gloc2, l, 
                            infra_pointer, time_range, infra_type);
        i = j - 1;

      }else
        sp->GetLineInIFObject(oid, unit.gloc1, unit.gloc2, l, 
                            infra_pointer, unit.timeInterval, infra_type);
      l->EndBulkLoad();

      genrange->Add(oid, l, tm);
      delete l;
  }

  sp->CloseInfra(infra_pointer);

}

/*
get the sub movement of a generic moving object according to a mode

*/
void GenMO::GenMOAt(string tm, GenMO* sub)
{
  int mode = GetTM(tm);
  if(mode < 0 || mode >= (int)(ARR_SIZE(str_tm))){
    cout<<"invalid mode "<<tm<<endl;
    return;
  }
  sub->Clear();
  sub->StartBulkLoad();
  for(int i = 0 ;i < GetNoComponents();i++){
    UGenLoc unit;
    Get( i, unit );
    if(unit.GetTM() == mode)
      sub->Add(unit);
  }

  sub->EndBulkLoad(false, false);
}

/*
the same function as above but with an index

*/
void GenMO::GenMOAt(string tm, MReal* index, GenMO* sub)
{
  int mode = GetTM(tm);
  if(mode < 0 || mode >= (int)(ARR_SIZE(str_tm))){
    cout<<"invalid mode "<<tm<<endl;
    return;
  }
  sub->Clear();
  sub->StartBulkLoad();
//   for(int i = 0 ;i < GetNoComponents();i++){
//     UGenLoc unit;
//     Get( i, unit );
//     if(unit.GetTM() == mode)
//       sub->Add(unit);
//   }
  for(int i = 0;i < index->GetNoComponents();i++){
    UReal ur;
    index->Get(i, ur);
    if((int)(ur.a) == mode){
      int start = (int)ur.b;
      int end = (int)ur.c;
      for(;start <= end;start++){
        UGenLoc unit;
        Get(start, unit);
        assert(unit.tm == mode);
//        sub->Add(unit);
        sub->Append(unit);

//        cout<<units.Size()<<endl;
//        cout<<sub->units.Size()<<endl;
      }
    }
  }

  sub->EndBulkLoad(false, false);

}

/*
with index built on units, efficiently to access certain units according to mode

*/
void GenMO::GenMOAt(GenLoc* gloc, MReal* index, GenMO* sub)
{

  sub->Clear();
  sub->StartBulkLoad();


  if(gloc->GetOid() > 0 ){
    sub->SetDefined(false);
  }else if(gloc->GetOid() == 0){
    Point loc(true, gloc->GetLoc().loc1, gloc->GetLoc().loc2);
    for(int i = 0;i < index->GetNoComponents();i++){
      UReal ur;
      index->Get(i, ur);
      if((int)(ur.a) == TM_FREE){//mode free
        int start = (int)ur.b;
        int end = (int)ur.c;
        for(;start <= end;start++){
          UGenLoc unit;
          Get(start, unit);
          assert(unit.GetOid() == 0);

          Point p1(true, unit.gloc1.GetLoc().loc1, unit.gloc1.GetLoc().loc2);
          Point p2(true, unit.gloc2.GetLoc().loc1, unit.gloc2.GetLoc().loc2);
          if(loc.Distance(p1) < EPSDIST && loc.Distance(p2) < EPSDIST){
            sub->Append(unit);
          }
        }
      }
    }
  }else{
    sub->SetDefined(false);
    cout<<"invalid genloc oid "<<endl;
  }

  sub->EndBulkLoad(false, false);
}


/*
get the sub movement of a generic moving object according to a genloc

*/
void GenMO::GenMOAt(GenLoc* genloc, GenMO* sub)
{
  if(!genloc->IsDefined()){
    return;
  }

  sub->Clear();
  sub->StartBulkLoad();
  if(genloc->GetOid() > 0){
    for(int i = 0 ;i < GetNoComponents();i++){
      UGenLoc unit;
      Get( i, unit );
      if(unit.GetOid() == (int)genloc->GetOid())
        sub->Add(unit);
    }
  }else if(genloc->GetOid() == 0){ //free space

    Point p(true, genloc->GetLoc().loc1, genloc->GetLoc().loc2);
    for(int i = 0 ;i < GetNoComponents();i++){
      UGenLoc unit;
      Get( i, unit );
      if(unit.tm == TM_FREE){
        assert(unit.GetOid() == 0);
        Point p1(true, unit.gloc1.GetLoc().loc1, unit.gloc1.GetLoc().loc2);
        Point p2(true, unit.gloc2.GetLoc().loc1, unit.gloc2.GetLoc().loc2);
        if(p1.Distance(p2) < EPSDIST && p.Distance(p1) < EPSDIST){
            sub->Append(unit);
        }
      }
    }
  }

  sub->EndBulkLoad(false, false);
}

/*
restrict the movement to some roads

*/
void GenMO::GenMOAt(Relation* rel, GenMO* sub)
{
//  cout<<rel->GetNoTuples()<<endl;
    static vector< vector< Interval<CcReal> > > roads;
    static unsigned road_init;
    static unsigned long cmd_no;
    static int min_rid, max_rid;
    if(road_init == 0){
      road_init++;
      SetRoads(roads, rel, min_rid, max_rid);
      /////////get the command number ///////////////////////
      SystemTables& st = SystemTables::getInstance();
      const SystemInfoRel* sysinfo = st.getInfoRel("SEC2COMMANDS");
      cmd_no = sysinfo->tuples.size();
    }else{
        SystemTables& st = SystemTables::getInstance();
        const SystemInfoRel* sysinfo = st.getInfoRel("SEC2COMMANDS");
        if(cmd_no != sysinfo->tuples.size()){
          cmd_no = sysinfo->tuples.size();
          roads.clear();
          SetRoads(roads, rel, min_rid, max_rid);
        }
    }

    sub->Clear();
    sub->StartBulkLoad();

//    cout<<"min_rid "<<min_rid<<"max_rid "<<max_rid<<endl;

//     cout<<"roads size "<<roads.size()<<endl;
// 
//     for(unsigned int i = 0;i < roads.size();i++){
//      if(roads[i].size() > 0){
//        cout<<"rid "<<i+1<<endl;
//        for(unsigned int j = 0;j < roads[i].size();j++)
//          cout<<roads[i][j].start<<" "<<roads[i][j].end<<endl;
//      }
//    }

    for(int i = 0 ;i < GetNoComponents();i++){
      UGenLoc unit;
      Get( i, unit );
      int oid = unit.GetOid();
      if(oid < min_rid || oid > max_rid) continue;
      if(roads[oid - 1].size() > 0){
//          cout<<"current size "<<roads[oid - 1].size()<<endl;
//          double loc1 = unit.gloc1.GetLoc().loc1;
//          double loc2 = unit.gloc2.GetLoc().loc1;

          CcReal loc1(true, unit.gloc1.GetLoc().loc1);
          CcReal loc2(true, unit.gloc2.GetLoc().loc1);

//          cout<<"rid "<<oid<<" loc1 "<<loc1<<" loc2 "<<loc2<<endl;
          Interval<CcReal> locs;
          if(loc1 < loc2){
            locs.start = loc1;
            locs.end = loc2;
          }else{
            locs.start = loc2;
            locs.end = loc2;
          }
          locs.lc = true;
          locs.rc = true;
          for(unsigned int j = 0;j < roads[oid - 1].size();j++){
//            cout<<locs<<endl;
//            cout<<loc1<<" "<<loc2<<endl;
//            cout<<roads[oid - 1][j].start<<" "<<roads[oid - 1][j].end<<endl;
//            locs.Print(cout);
//            roads[oid - 1][j].Print(cout);
            if(locs.Intersects(roads[oid - 1][j])){
                sub->Add(unit);
                break;
            }
          }
      }

    }

    sub->EndBulkLoad(false, false);
}

void GenMO::SetRoads(vector< vector<Interval<CcReal> > >& roads, 
                     Relation* rel, int& minrid, int& maxrid)
{
  int max_rid = 0;
  int min_rid = numeric_limits<unsigned int>::max();
  for(int i = 1;i <= rel->GetNoTuples();i++){
      Tuple* tuple = rel->GetTuple(i, false);
      int rid = ((CcInt*)tuple->GetAttribute(GenMObject::RS_RID))->GetIntval();
      tuple->DeleteIfAllowed();
      if(rid > max_rid) max_rid = rid;
      if(rid < min_rid) min_rid = rid;
  }

  maxrid = max_rid;
  minrid = min_rid;

  for(int i = 0;i < max_rid;i++){
    vector< Interval<CcReal> > temp;
    roads.push_back(temp);
  }
//  cout<<"roads size "<<roads.size()<<" max rid "<<max_rid<<endl;
  for(int i = 1;i <= rel->GetNoTuples();i++){
      Tuple* tuple = rel->GetTuple(i, false);
      int rid = ((CcInt*)tuple->GetAttribute(GenMObject::RS_RID))->GetIntval();
/*      double meas1 = 
        ((CcReal*)tuple->GetAttribute(GenMObject::RS_MEAS1))->GetRealval();
      double meas2 = 
        ((CcReal*)tuple->GetAttribute(GenMObject::RS_MEAS2))->GetRealval();*/

      CcReal meas1(true,
        ((CcReal*)tuple->GetAttribute(GenMObject::RS_MEAS1))->GetRealval());
      CcReal meas2(true,
        ((CcReal*)tuple->GetAttribute(GenMObject::RS_MEAS2))->GetRealval());

      Interval<CcReal> pos;
      pos.start = meas1;
      pos.lc = true;
      pos.end = meas2;
      pos.rc = true;
      roads[rid - 1].push_back(pos);
      tuple->DeleteIfAllowed();
  }

//   for(unsigned int i = 0;i < roads.size();i++){
//     if(roads[i].size() > 0){
//       cout<<"rid "<<i+1<<endl;
//       for(unsigned int j = 0;j < roads[i].size();j++)
//         cout<<roads[i][j].start<<" "<<roads[i][j].end<<endl;
//     }
//   }

}


/*
get the sub movement of a generic moving object according to a genloc
with index built on genmo units 

*/
void GenMO::GenMOAt(GenLoc* genloc, MReal* index, string tm, GenMO* sub)
{
  if(!genloc->IsDefined() || GetNoComponents() == 0){
    return;
  }

  int m = GetTM(tm);

  sub->Clear();
  sub->StartBulkLoad();
  
  for(int i = 0;i < index->GetNoComponents();i++){
    UReal ur;
    index->Get(i, ur);
    if((int)(ur.a) == m){
      int start = (int)ur.b;
      int end = (int)ur.c;
      for(;start <= end;start++){
        UGenLoc unit;
        Get(start, unit);
        if(unit.GetOid() == (int)genloc->GetOid()){
          sub->Append(unit);
        }
      }
    }
  }
  sub->EndBulkLoad(false, false);
}


/*
return the intime value of a generic moving object 

*/
void GenMO::AtInstant(Instant& t, Intime<GenLoc>& result)
{
  if(IsDefined() && GetNoComponents() >0 && t.IsDefined()){
    Periods* peri = new Periods(0);
    DefTime(*peri);
    if(peri->Contains(t) == false){
      result.SetDefined(false);
    }else{
      for(int i = 0;i < GetNoComponents();i++){
          UGenLoc unit;
          Get( i, unit );
          if(unit.timeInterval.Contains(t)){
            result = GetUnitInstant(unit, t);
            break;
          }
      }
    }
    delete peri; 
  }else
    result.SetDefined(false);

}

/*
get the movement in a given time period 

*/
void GenMO::AtPeriods(Periods* peri_q, GenMO& result) 
{
  result.Clear();
  if(IsDefined() && GetNoComponents() > 0 && peri_q->IsDefined()){
    Periods* peri = new Periods(0);
    DefTime(*peri);
    Instant s_q, e_q; 
    peri_q->Minimum(s_q);
    peri_q->Maximum(e_q);

    Instant s_mo, e_mo;
    peri->Minimum(s_mo);
    peri->Maximum(e_mo);
    if(s_q > e_mo || e_q < s_mo){
      delete peri;
      return;
    }
    delete peri;
    result.StartBulkLoad();

    int64_t st = s_q.ToDouble()*86400000.0;
    int64_t et = e_q.ToDouble()*86400000.0;

    for(int i = 0;i < GetNoComponents();i++){
      UGenLoc unit;
      Get( i, unit );
      Instant u_s = unit.timeInterval.start;
      Instant u_e = unit.timeInterval.end;

      if(u_e < s_q) continue;
      if(u_s > e_q) break;

      int64_t u_s_t = u_s.ToDouble()*86400000.0;
      int64_t u_e_t = u_e.ToDouble()*86400000.0;

      if(st <= u_s_t && u_e_t <= et){
        result.Add(unit);

        continue; 
      }

      if(u_s_t <= st && et <= u_e_t){//query time is contained by a unit 
          Intime<GenLoc> igloc1, igloc2;
          igloc1 = GetUnitInstant(unit, s_q);
          igloc2 = GetUnitInstant(unit, e_q);

          UGenLoc unit_tmp(true);
          unit_tmp.timeInterval.start = igloc1.instant;
          if(u_s_t == st && unit.timeInterval.lc)
            unit_tmp.timeInterval.lc = true;
          else
            unit_tmp.timeInterval.lc = false;

          unit_tmp.timeInterval.end = igloc2.instant;
          if(u_e_t == et && unit.timeInterval.rc)
            unit_tmp.timeInterval.rc = true;
          else
            unit_tmp.timeInterval.rc = false;

          unit_tmp.gloc1 = igloc1.value;
          unit_tmp.gloc2 = igloc2.value;
          unit_tmp.tm = unit.tm;
          result.Add(unit_tmp);

          break;
      }
      ///////////////////////////////////////////////////////////////////
      Instant min = MAX(s_q, u_s);
      int64_t min_t = min.ToDouble()*86400000.0;

      Instant max = MIN(e_q, u_e);
      int64_t max_t = max.ToDouble()*86400000.0;
//      cout<<unit.timeInterval<<" min "<<min<<" max "<<max<<endl;

      if(min > u_s && min_t < u_e_t)
        assert(unit.timeInterval.Contains(min));

      if(max < u_e && max_t > u_s_t)
        assert(unit.timeInterval.Contains(max));

      Intime<GenLoc> igloc_1, igloc_2;
      igloc_1 = GetUnitInstant(unit, min);
      igloc_2 = GetUnitInstant(unit, max);
      UGenLoc unit_temp(true);
      unit_temp.timeInterval.start = igloc_1.instant;
      unit_temp.timeInterval.lc = true;
      unit_temp.timeInterval.end = igloc_2.instant;
      unit_temp.timeInterval.rc = false;

      unit_temp.gloc1 = igloc_1.value;
      unit_temp.gloc2 = igloc_2.value;
      unit_temp.tm = unit.tm;

      if(unit_temp.timeInterval.IsValid())//check time interval 
        result.Add(unit_temp);


      s_q = MAX(s_q, u_s);
      st = s_q.ToDouble()*86400000.0;

      ///////////////////////////////////////////////////////////////////
    }
    result.EndBulkLoad(false, false);

  }
}

/*
return an intime genloc for a given unit 

*/
Intime<GenLoc> GenMO::GetUnitInstant(UGenLoc& unit, Instant& t)
{
    Intime<GenLoc> result;
    result.instant = t;
    GenLoc gloc;
    assert(unit.gloc1.GetOid() == unit.gloc2.GetOid());
    int oid = unit.gloc1.GetOid();
    Loc loc;
    if(unit.tm == TM_BUS || unit.tm == TM_METRO){//bus and metro 
        loc.loc1 = -1.0;
        loc.loc2 = -1.0;
    }else if (unit.tm == TM_FREE || unit.tm == TM_INDOOR || 
              unit.tm == TM_WALK){
        Instant t0 = unit.timeInterval.start;
        Instant t1 = unit.timeInterval.end;
        Point p1(true, unit.gloc1.GetLoc().loc1, 
                 unit.gloc1.GetLoc().loc2);
        Point p2(true, unit.gloc2.GetLoc().loc1,
                 unit.gloc2.GetLoc().loc2);

//      cout<<p1<<" "<<p2<<endl;

        double delta_time = (t-t0)/(t1-t0);

        double x = p1.GetX() + (p2.GetX() - p1.GetX())*delta_time;
        double y = p1.GetY() + (p2.GetY() - p1.GetY())*delta_time;
        loc.loc1 = x;
        loc.loc2 = y;
//      cout<<"x "<<x<<" y "<<y<<endl;

   }else if(unit.tm == TM_CAR || unit.tm == TM_BIKE ||
            unit.tm == TM_TAXI){

              Instant t0 = unit.timeInterval.start;
              Instant t1 = unit.timeInterval.end;
              double pos1 = unit.gloc1.GetLoc().loc1;
              double pos2 = unit.gloc2.GetLoc().loc1; 
              assert(unit.gloc1.GetLoc().loc2 < 0);
              assert(unit.gloc2.GetLoc().loc2 < 0);

              double pos = pos1 + (pos2 - pos1)*((t-t0)/(t1-t0));
              loc.loc1 = pos;
              loc.loc2 = -1.0;
    }else{
            cout<<"invalid mode "<<endl;
            result.SetDefined(false);
            return result;
         }

    gloc.SetValue(oid, loc);
    result.value = gloc;
    result.SetDefined(true);
    return result;

}

/*
check whether the moving object contains a transportation mode 

*/

bool GenMO::Contain(string tm)
{
  if(IsDefined() && GetNoComponents() > 0){
    
    int m = GetTM(tm);
    if(m < 0)
        return false;
    for(int i = 0 ;i < GetNoComponents();i++){
      UGenLoc unit;
      Get( i, unit );
      if(unit.GetTM() == m)
        return true;
    }
    return false;
  }else
    return false; 
}

bool GenMO::Contain(int refid)
{
  if(IsDefined() && GetNoComponents() > 0){

    for(int i = 0 ;i < GetNoComponents();i++){
      UGenLoc unit;
      Get( i, unit );
      if(unit.GetOid() == refid)
        return true;
    }
    return false;
  }else
    return false; 
}

/*
with an index built on units to check whether a reference int is included or not

*/
bool GenMO::Contain(MReal* index, int ref_id, string tm)
{
  bool res = false;
  int m = GetTM(tm);

//  cout<<"IF "<<GetSymbolStr(environment)<<endl;
  //we know the environment then the mode is also determined 
  if(m < 0){ 
    cout<<"invalid string "<<tm<<endl;
    return res;
  }
  for(int i = 0;i < index->GetNoComponents();i++){
      UReal ur;
      index->Get(i, ur);
      if((int)(ur.a) == m){///mode --bus 
        int start = (int)ur.b;
        int end = (int)ur.c;
        for(;start <= end;start++){
          UGenLoc unit;
          Get(start, unit);
          if(unit.GetOid() == ref_id){
            return true;
          }
        }
      }
  }

  return res;
}


/*
check a moving object passing a given region 

*/
bool GenMO::Passes(Region* reg, Space* sp)
{

  if(IsDefined() && GetNoComponents() > 0){

    static int box_init;
    static double min[2], max[2];
    Rectangle<2> bbox(false);

    for(int i = 0 ;i < GetNoComponents();i++){
//      cout<<"i "<<i<<endl;
      UGenLoc unit;
      Get( i, unit );
      Point p1, p2;
      if(unit.GetTM() == TM_FREE){
        p1.Set(unit.gloc1.GetLoc().loc1, unit.gloc1.GetLoc().loc2);
        p2.Set(unit.gloc2.GetLoc().loc1, unit.gloc2.GetLoc().loc2);
 //       cout<<p1<<" "<<p2<<endl;

//        if(p1.Inside(*reg) || p2.Inside(*reg)) return true;
        Line* l = new Line(0);
        GetLine(p1, p2, l);
        if(l->Intersects(*reg)){
          delete l;
          return true;
        }
        delete l;

      }else if(unit.GetTM() == TM_WALK){
        if(box_init == 0){
          Pavement* pm = sp->LoadPavement(IF_REGION);
          assert(pm != NULL);
          DualGraph* dg = pm->GetDualGraph();
          assert(dg != NULL);
          Rectangle<2> bbox_tmp =  dg->rtree_node->BoundingBox();

          min[0] = bbox_tmp.MinD(0);
          max[0] = bbox_tmp.MaxD(0);
          min[1] = bbox_tmp.MinD(1);
          max[1] = bbox_tmp.MaxD(1);

          pm->CloseDualGraph(dg);
          sp->ClosePavement(pm);
          box_init++;
        }
          bbox.Set(true, min, max);
//          cout<<bbox<<endl;

          p1.Set(unit.gloc1.GetLoc().loc1 + bbox.MinD(0),
               unit.gloc1.GetLoc().loc2 + bbox.MinD(1));
          p2.Set(unit.gloc2.GetLoc().loc1 + bbox.MinD(0),
               unit.gloc2.GetLoc().loc2 + bbox.MinD(1));
//          if(p1.Inside(*reg) || p2.Inside(*reg)) return true;
        Line* l = new Line(0);
        GetLine(p1, p2, l);
        if(l->Intersects(*reg)){
          delete l;
          return true;
        }
        delete l;
      }

    }
    return false;
  }else
    return false; 

}



/*
map a genmo to a mpoint 
now it only supports bus 


*/

void GenMO::MapGenMO(MPoint* in, MPoint& res)
{

  res.Clear();

  if(GetNoComponents() == 0){
    cout<<"empty genmo "<<endl;
    return;
  }

  Periods* peri1 = new Periods(0);
  Periods* peri2 = new Periods(0);
  DefTime(*peri1);
  in->DefTime(*peri2);
//  cout<<*peri1<<" "<<*peri2<<endl;


  Interval<Instant> time_span1;
  Interval<Instant> time_span2;
  peri1->Get(0, time_span1);
  peri2->Get(0, time_span2);
  in->AtPeriods(*peri1, res);

//  int day1 = time_span1.start.GetDay();
//  int day2 = time_span2.start.GetDay();

//  cout<<day1<<" "<<day2<<endl;
//   if(day1 == day2){
//     in->AtPeriods(*peri1, res);
// 
//   }else{
// 
//     Periods* peri_new = new Periods(0);
//     peri_new->StartBulkLoad();
// 
//     Instant st1 = time_span1.start;
//     Instant et1 = time_span1.end;
// 
//     Instant st2 = time_span2.start;
//     Instant et2 = time_span2.end;
// 
// //     if(st1.GetDay() != et1.GetDay() || st2.GetDay() != et2.GetDay()){
// //       cout<<" time "<<*peri1<<endl;
// //       cout<<"time should be in one day"<<endl;
// //       return;
// //     }
// 
//     Instant st = st1;
//     Instant et = st2;
// 
//     st.Set(st2.GetYear(), st2.GetMonth(), st2.GetGregDay(), 
//            st1.GetHour(), st1.GetMinute(), 
//            st1.GetSecond(), st1.GetMillisecond());
// 
//     et.Set(et2.GetYear(), et2.GetMonth(), et2.GetGregDay(), 
//            et1.GetHour(), et1.GetMinute(), 
//            et1.GetSecond(), et1.GetMillisecond());
// 
//     Interval<Instant> time_span_new;
// 
//     time_span_new.start = st;
//     time_span_new.lc = time_span1.lc;
//     time_span_new.end = et; 
//     time_span_new.rc = time_span1.rc;
// 
//     peri_new->MergeAdd(time_span_new);
//     peri_new->EndBulkLoad();
// 
// //    cout<<"new periods "<<*peri_new<<endl;
//     in->AtPeriods(*peri_new, res);
// 
//     delete peri_new;
//   }

  delete peri2; 
  delete peri1; 

}

/*
get the mode value for a generic moving object 
be careful about the bitset position 
0--right
from right to left, from low to high 
we use an integer (from bitset) to denote transportation mods

*/
int GenMO::ModeVal()
{
  bitset<ARR_SIZE(str_tm)> modebits;
  modebits.reset();
//  cout<<modebits.to_string()<<endl;
  for(int i = 0 ;i < GetNoComponents();++i){
      UGenLoc unit;
      Get( i, unit );
      assert(0 <= unit.tm && unit.tm < (int)(ARR_SIZE(str_tm)));
//      modebits.set(unit.tm, 1); //be careful bitset 0--right 
      modebits.set((int)ARR_SIZE(str_tm) - 1 - unit.tm, 1);
  }
//  modebits.set(0, 1); //(pos,val)
//  cout<<modebits.to_string()<<endl;

//  modebits.reset();
//  cout<<modebits.to_string()<<endl;
//  return 0;

//     int val = modebits.to_ulong();
//     bitset<ARR_SIZE(str_tm)> modebits_r(val);
//     cout<<modebits_r.to_string()<<endl;

    return modebits.to_ulong();
}

int GenMO::ModeVal(MReal* mr)
{
  bitset<ARR_SIZE(str_tm)> modebits;
  modebits.reset();

   for(int i = 0 ;i < mr->GetNoComponents();++i){
       UReal unit;
       mr->Get( i, unit );
       assert(0 <= unit.a && unit.a < (int)(ARR_SIZE(str_tm)));
       modebits.set((int)ARR_SIZE(str_tm) - 1 - unit.a, 1);
   }

  return modebits.to_ulong();

}

/*
ureal has a b c
a -- tm
b, c --start and end index in dbarray for genmo units 

*/
void GenMO::IndexOnUnits(MReal* res)
{
  res->Clear();

  if(IsDefined() && GetNoComponents() > 0){
    res->SetDefined(true);
    res->StartBulkLoad();
    for(int i = 0 ;i < GetNoComponents();i++){
      UGenLoc unit1;
      Get( i, unit1);
      int j = i + 1;
      Instant s = unit1.timeInterval.start;
      bool l = unit1.timeInterval.lc;
      Instant e = unit1.timeInterval.end;
      bool r = unit1.timeInterval.rc;

      if(j < GetNoComponents()){
        UGenLoc unit2;
        Get( j, unit2);
        while(unit1.tm == unit2.tm && j < GetNoComponents()){
          j++;
          e = unit2.timeInterval.end;
          r = unit2.timeInterval.rc;
          if(j < GetNoComponents())
            Get( j, unit2);
        }
        j--;

        Interval<Instant> t;
        t.start = s;
        t.lc = l;
        t.end = e;
        t.rc = r;
        UReal ur(t, (double)unit1.tm, (double)i, (double)j, true);
        res->MergeAdd(ur);

        i = j;
      }else{
        Interval<Instant> t;
        t.start = s;
        t.lc = l;
        t.end = e;
        t.rc = r;
        UReal ur(t, (double)unit1.tm, (double)i, (double)i, true);
        res->MergeAdd(ur);
      }

    }

    res->EndBulkLoad(false, false);

  }else
    res->SetDefined(false);

}

/*
check whether a building is visited 

*/
bool GenMO::BContains(int bid)
{

   char buffer1[64];
   sprintf(buffer1,"%d", bid);
   string number1(buffer1);
//   cout<<"length "<<number1.length()<<endl;
   for(int i = 0;i < GetNoComponents();i++){
      UGenLoc unit;
      Get( i, unit);
      if(unit.tm == TM_INDOOR){
           char buffer2[64];
           sprintf(buffer2,"%d", unit.GetOid());
           string number2(buffer2);
           string build_id = number2.substr(0, number1.length());
           int val = 0;
           sscanf(build_id.c_str(), "%d", &val);
           if(val == bid)return true;
      }
   }

   return false;
}

/*
check whether a building is visited, with an index 
in fact, we only need to check the first unit of an indoor movement because the
building id is the same for one building

*/
bool GenMO::BContains(MReal* index, int bid)
{

   char buffer1[64];
   sprintf(buffer1,"%d", bid);
   string number1(buffer1);

  for(int i = 0;i < index->GetNoComponents();i++){
      UReal ur;
      index->Get(i, ur);
      if((int)(ur.a) == TM_INDOOR){///mode -- indoor 
        int start = (int)ur.b;
        UGenLoc unit;
        Get(start, unit);
        char buffer2[64];
        sprintf(buffer2,"%d", unit.GetOid());
        string number2(buffer2);
        string build_id = number2.substr(0, number1.length());
        int val = 0;
        sscanf(build_id.c_str(), "%d", &val);
        if(val == bid)return true;
      }
  }

   return false;
}

void GetLine(Point& p1, Point& p2, Line* l)
{
    l->StartBulkLoad();
    HalfSegment hs;
    hs.Set(true, p1, p2);
    hs.attr.edgeno = 0;
    *l += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    *l += hs;
    l->EndBulkLoad();

}

/////////////////////////////////////////////////////////////////////////
/////////////// get information from generic moving objects///////////////
/////////////////////////////////////////////////////////////////////////
string GenMObject::StreetSpeedInfo = "(rel (tuple ((Id_b int) (Vmax real))))";
string GenMObject::CommPathInfo = "(rel (tuple ((cell_id1 int) \
(cell_area1 rect) (cell_id2 int) (cell_area2 rect) (path1 gline))))";
string GenMObject::RTreeCellInfo = "(rtree (tuple ((cell_id1 int) \
(cell_area1 rect) (cell_id2 int) (cell_area2 rect) (path1 gline))) rect FALSE)";

string GenMObject::BuildingInfoB = "(rel (tuple ((Tid int) (Type string)\
(Area_B rect))))";
string GenMObject::BuildingInfoM = "(rel (tuple ((Tid int) (Type string)\
(Area_M rect)(M bool))))";

string GenMObject::BenchModeDISTR = 
"(rel (tuple ((Mode string) (Para real))))";

string GenMObject::NNBuilding = 
"(rel (tuple ((B_id int) (GeoData rect))))";

string GenMObject::RoadSegment = 
"(rel (tuple ((rid int) (meas1 real) (meas2 real) (ncurve line) (SID int))))";

string GenMObject::GenMOTrip = 
"(rel (tuple ((oid int) (Trip1 genmo) (Trip2 mpoint) (def periods) (M int) \
(UIndex mreal))))";

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

/*
get the referenced infrastructure objects in a light way 

*/
void GenMObject::GetRef(GenMO* mo)
{

  for(int i = 0;i < mo->GetNoComponents();i++){
    UGenLoc unit;
    mo->Get(i, unit);
    int oid = unit.gloc1.GetOid();
    int tm = unit.tm;
    if(oid_list.size() == 0){
      int infra_label;
      if(oid == 0)
          infra_label = IF_FREESPACE;
      else
          infra_label = TM2InfraLabel(tm);

      oid_list.push_back(oid);
      label_list.push_back(infra_label);
    }else if(oid != oid_list[oid_list.size() - 1]){
      int infra_label;
      if(oid == 0)
          infra_label = IF_FREESPACE;
      else
          infra_label = TM2InfraLabel(tm);

      oid_list.push_back(oid);
      label_list.push_back(infra_label);
    }
  }

}

/*
get all units of a moving object

*/
void GenMObject::GetUnits(GenMO* genmo)
{

    for( int i = 0; i < genmo->GetNoComponents(); i++ ){
      UGenLoc unit;
      genmo->Get( i, unit );
      assert(unit.gloc1.GetOid() == unit.gloc2.GetOid()); 
      units_list.push_back(unit); 
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

void GenMObject::GetIdList(Door3D* d)
{
   int oid1 = d->GetOid(1);
   int oid2 = d->GetOid(2);
   id_list.push_back(oid1); 
   id_list.push_back(oid2); 
}

/*
according to type value, different sub functions are called 
7: car + walk
8: bus + walk
9: indoor + walk 
10: metro + walk 
11: taxi + walk;
13: car + indoor + walk;
14: bus + indoor + walk 
15: metro + indoor + walk 
16: taxi + indoor + walk 
18: bike + indoor + walk 

*/
void GenMObject::GenerateGenMO(Space* sp, Periods* peri, int mo_no, int type)
{

  if(type == 7 || type == 11 || type == 17){ //car taxi bike + walk 

      Relation* rel1 = sp->GetDualNodeRel();
      BTree* btree = sp->GetDGNodeBTree();
      Relation* rel2 = sp->GetSpeedRel();
      if(rel1 == NULL || btree == NULL || rel2 == NULL){
          cout<<"auxiliary relation empty car"<<endl;
          return;
      }else{
        GenerateGenMO2(sp, peri, mo_no, type, rel1, btree, rel2);
      }
    return;
  }
  if(type == 9){//indoor + walk

        Relation* rel = sp->GetNewTriRel();
        if(rel == NULL){
            cout<<"auxiliary rel empty "<<endl;
            return;
        }else{
            GenerateGenMO4(sp, peri, mo_no, type, rel);
        }
     return;
  }

  if(type == 13 || type == 16 || type == 18){//bike car taxi + indoor + walk
    GenerateGenMO5(sp, peri, mo_no, type);
    return;
  }

  if(type == 10){//metro + walk 

        Relation* rel1 = sp->GetNewTriRel();
        Relation* rel2 = sp->GetMSPaveRel();
        R_Tree<2,TupleId>* rtree = sp->GetMSPaveRtree();
        if(rel1 == NULL || rel2 == NULL || rtree == NULL){
          cout<<"auxiliary relation empty metro "<<endl;
          return;
        }else{
          GenerateGenMO7(sp, peri, mo_no, type, rel1, rel2, rtree);
        }
        return;
  }
  if(type == 15){//indoor + metro + walk 

      GenerateGenMO8(sp, peri, mo_no, type, 12);
      
      return;
  }
  if(type ==8){ // bus + walk 
  
        Relation* rel1 = sp->GetNewTriRel();
        Relation* rel2 = sp->GetBSPaveRel();
        R_Tree<2,TupleId>* rtree = sp->GetBSPaveRtree();
        if(rel1 == NULL || rel2 == NULL || rtree == NULL){
          cout<<"auxiliary relation empty bus"<<endl;
          return;
        }else{

          GenerateGenMO3(sp, peri, mo_no, type, rel1, rel2, rtree);
        }
    return;
  }
  if(type == 14){// bus + indoor + walk 

      GenerateGenMO6(sp, peri, mo_no, type, 12);//12 for time interval
      return;
  }

  cout<<"invalid type "<<type<<endl;
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
  if(!(0 < type && type <= int(ARR_SIZE(genmo_tmlist)))){
    cout<<" invalid type value "<<type<<endl;
  }

  switch(type){
    case 7:
      GenerateGenMO_CTBWalk(sp, peri, mo_no, rel1, btree, rel2, "Car");
      break;
    case 11:
      GenerateGenMO_CTBWalk(sp, peri, mo_no, rel1, btree, rel2, "Taxi");
      break;
    case 17:
      GenerateGenMO_CTBWalk(sp, peri, mo_no, rel1, btree, rel2, "Bike");
      break;

    default:
      assert(false);
      break;  
  }

}

/*
generate moving cars in road network (mpoint and mgpoint) to get traffic 
old method: old shortest path computation 
50 cars: 9.92 seconds
new method: road graph: 50 cars   1.71 seconds

*/
void GenMObject::GenerateCar(Space* sp, Periods* peri, int mo_no, 
                             Relation* rel)
{
  if(mo_no < 1){
    cout<<"invalid mo no "<<endl;
    return;
  }

  Network* rn = sp->LoadRoadNetwork(IF_LINE);
  if(rn == NULL){
    cout<<"road network loading error"<<endl;
    return;
  }
  RoadGraph* rg = sp->LoadRoadGraph();
  if(rg == NULL){
    cout<<"road graph loading error"<<endl;
    return;
  }
  Relation* routes = rn->GetRoutes();
  if(routes == NULL){
    cout<<"routes loading error"<<endl;
    return; 
  }

  const double min_len = 500.0;
  int count = 1;

  Interval<Instant> periods;
  peri->Get(0, periods);
  ///////////////////////////////////////////////////////////////////
  //////////  generate random gpoints in road network ///////////////
  //////////////////////////////////////////////////////////////////
  vector<GPoint> gp_loc_list;
  vector<Point> p_loc_list;
  GenerateGPoint2(rn, obj_scale*mo_no, gp_loc_list, p_loc_list);
  RoadNav* road_nav = new RoadNav();
  
   while(count <= mo_no){
    int index1 = GetRandom() % gp_loc_list.size();
    GPoint gp1 = gp_loc_list[index1];
//    GPoint gp1(true, 1, 1184, 500.0, None);////////testing
    Point gp_loc1 = p_loc_list[index1];

    int index2 = GetRandom() % gp_loc_list.size();
    GPoint gp2 = gp_loc_list[index2];
//    GPoint gp2(true, 1, 1966, 32.0, None);//////////testing 
    Point gp_loc2 = p_loc_list[index2];


    if(index1 == index2 || gp_loc1.Distance(gp_loc2) < min_len) continue;


    Point* start_loc = new Point();
    Tuple* road_tuple = routes->GetTuple(gp1.GetRouteId(), false);
    SimpleLine* sl = (SimpleLine*)road_tuple->GetAttribute(ROUTE_CURVE);
    assert(sl->GetStartSmaller());
    assert(sl->AtPosition(gp1.GetPosition(), true, *start_loc));
    road_tuple->DeleteIfAllowed();

//    cout<<gp1<<" "<<gp2<<endl;

    GLine* gl = new GLine(0);
    road_nav->ShortestPathSub(&gp1, &gp2, rg, rn, gl);
    GenerateCarMO(rn, count, peri, gl, rel, *start_loc);
    delete gl;

//    loc_list1.push_back(gp_loc1);
//    loc_list2.push_back(gp_loc2);

    delete start_loc; 
    cout<<count<<" generic moving object"<<endl;

    count++;
  }
  delete road_nav;
  
  sp->CloseRoadGraph(rg);
  sp->CloseRoadNetwork(rn);
  

}

/*
traverse the rtree to find which cells contains the point 

*/
void GenMObject::DFTraverse3(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                             Relation* rel,
                             Point query_loc, vector<int>& cellid_list)
{

  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple = rel->GetTuple(e.info, false);
              Rectangle<2>* rect = 
                    (Rectangle<2>*)dg_tuple->GetAttribute(CELL_AREA1);

              if(query_loc.Inside(*rect)){
                int cell_id = 
                    ((CcInt*)dg_tuple->GetAttribute(CELL_ID1))->GetIntval();
                cellid_list.push_back(cell_id);
              }
              dg_tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(query_loc.Inside(e.box)){
                DFTraverse3(rtree, e.pointer, rel, query_loc, cellid_list);
            }
      }
  }
  delete node;

}


/*
create a car moving object: mpoint and mgpoint

*/
void GenMObject::GenerateCarMO(Network* rn, int i, Periods* peri, 
                               GLine* newgl,
                               Relation* rel, Point start_loc)
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
  MGPoint* mo_gp = new MGPoint(0);

  mo->StartBulkLoad();

  mo_gp->SetDefined(true);/////////no bulkload for moving gpoint

//  const double delta_dist = 0.01;
  bool correct = true;
  
  for(int i = 0;i < newgl->Size();i++){
    RouteInterval* ri = new RouteInterval();
    newgl->Get(i, *ri);
    int rid = ri->GetRouteId();
    Tuple* speed_tuple = rel->GetTuple(rid, false);
    assert(((CcInt*)speed_tuple->GetAttribute(SPEED_RID))->GetIntval() == rid);
    double speed = 
      ((CcReal*)speed_tuple->GetAttribute(SPEED_VAL))->GetRealval();
    speed_tuple->DeleteIfAllowed();
// cout<<ri->GetRouteId()<<" "<<ri->GetStartPos()<<" "<<ri->GetEndPos()<<endl;

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

//    ri->Print(cout);
    double start_pos = ri->GetStartPos();
    double end_pos = ri->GetEndPos();

    bool bDual = rn->GetDual(ri->GetRouteId());
    bool bMovingUp = true;
    if(start_pos > end_pos) bMovingUp = false;
    Side side = None;
    if(bDual && bMovingUp) side = Up;
    else if(bDual && !bMovingUp) side = Down;
    else side = None;

    /////////////////////////////////////////////////////////////////////

    Interval<Instant> unit_interval;
    unit_interval.start = start_time;
    unit_interval.lc = true;

    Point temp_sp1 = seq_halfseg[0].from; 
    Point temp_sp2 = seq_halfseg[seq_halfseg.size() - 1].to; 
    
//    cout<<"dist1 "<<start_loc.Distance(temp_sp1)
//        <<" dist2 "<<start_loc.Distance(temp_sp2)<<endl;

//    if(start_loc.Distance(temp_sp1) < delta_dist){
    if(start_loc.Distance(temp_sp1) < EPSDIST){

      if(start_pos > end_pos){

        CreateCarMPMGP1(mo, mo_gp, seq_halfseg, start_time, speed, 
                      rn->GetId(), ri->GetRouteId(), side, 
                      start_pos, false);
      }else{
        CreateCarMPMGP1(mo, mo_gp, seq_halfseg, start_time, speed, 
                      rn->GetId(), ri->GetRouteId(), side, 
                      start_pos, true);
      }

      start_loc = temp_sp2;

//    }else if(start_loc.Distance(temp_sp2) < delta_dist){
    }else if(start_loc.Distance(temp_sp2) < EPSDIST){

      if(start_pos > end_pos ){

        CreateCarMPMGP2(mo, mo_gp, seq_halfseg, start_time, speed,
                     rn->GetId(), ri->GetRouteId(), side, 
                      start_pos, false);
      }else{
        CreateCarMPMGP2(mo, mo_gp, seq_halfseg, start_time, speed,
                     rn->GetId(), ri->GetRouteId(), side, 
                      start_pos, true);
      }

      start_loc = temp_sp1;

//    }else assert(false);
    }else {

      delete ri;
      correct = false;
      break;
    }

    unit_interval.end = start_time;
    unit_interval.rc = false;

    delete ri;

  }
  
  if(correct){
    mo->EndBulkLoad();
    trip2_list.push_back(*mo);
    trip3_list.push_back(*mo_gp);
  }

  delete mo; 
  delete mo_gp;


}

/*
create moving car units and add them into the trip, 
it travers from index small to big in myhalfsegment list 
use the maxspeed as car speed 
pos len increase

*/
void GenMObject::CreateCarMPMGP1(MPoint* mo, MGPoint* mgp,
                                 vector<MyHalfSegment> seq_halfseg,
                      Instant& start_time, double speed_val, 
                      int networkId, int routeId, Side side, 
                                 double pos_len, bool increase)
{
//  const double dist_delta = 0.01; 

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
//    if(dist < dist_delta){//ignore such small segment
    if(dist < EPSDIST){//ignore such small segment 
        if((i + 1) < seq_halfseg.size()){
          seq_halfseg[i + 1].from = from_loc; 
        }
        continue; 
    }
    /////////////////////////////////////////////////////////////////

    //double 1.0 means 1 day 
    et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0*60));
//    cout<<st<<" "<<et<<endl;

    int64_t start_time = st.ToDouble()*86400000.0;
    int64_t end_time = et.ToDouble()*86400000.0;
    if(start_time == end_time){/////////time is equal, unit is not valid
        if((i + 1) < seq_halfseg.size()){
          seq_halfseg[i + 1].from = from_loc; 
        }
        continue; 
    }

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

    double start_pos = pos_len;
    double end_pos;
    if(increase)
      end_pos = start_pos + from_loc.Distance(to_loc);
    else
      end_pos = start_pos - from_loc.Distance(to_loc);

//    if(fabs(end_pos) < dist_delta) end_pos = 0.0;
    if(fabs(end_pos) < EPSDIST) end_pos = 0.0;

    pos_len = end_pos;

//     cout<<"start "<<" network id "<<networkId<<" route id "<<routeId
//         <<" side "<<side<<" increase "<<increase
//         <<" pos1 "<<start_pos<<" pos2 "<<end_pos<<endl;

//    cout<<"1 time interval "<<up_interval<<endl;

    UGPoint* ugp = 
        new UGPoint(up_interval,networkId,routeId,side,start_pos,end_pos);
//    ugp->Print(cout);
    mgp->Add(*ugp);
    delete ugp;
    /////////////////////////////////////////////////////////////
    st = et; 
  }

  start_time = et;
  

}

/*
create moving carunits and add them into the trip, 
!!! it traverse from index big to small in myhalfsegment list 
use the maxspeed as car speed 
pos len decrease

*/
void GenMObject::CreateCarMPMGP2(MPoint* mo, MGPoint* mgp,
                                 vector<MyHalfSegment> seq_halfseg,
                      Instant& start_time, double speed_val, 
                      int networkId, int routeId, Side side, 
                                 double pos_len, bool increase)
{

//  const double dist_delta = 0.01;

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
//    if(dist < dist_delta){//ignore such small segment
    if(dist < EPSDIST){//ignore such small segment 
        if((i - 1) >= 0 ){
          seq_halfseg[i - 1].to = from_loc; 
        }
        continue; 
    }

    //double 1.0 means 1 day 
    et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0*60.0));

    int64_t start_time = st.ToDouble()*86400000.0;
    int64_t end_time = et.ToDouble()*86400000.0;
    if(start_time == end_time){/////////time is equal, unit is not valid
        if((i - 1) >= 0 ){
          seq_halfseg[i - 1].to = from_loc; 
        }
        continue; 
    }


//    cout<<st<<" "<<et<<endl;
    ////////////////////create a upoint////////////////////////
    up_interval.start = st;
    up_interval.lc = true;
    up_interval.end = et;
    up_interval.rc = false; 
    UPoint* up = new UPoint(up_interval,from_loc,to_loc);

    mo->Add(*up);
    delete up; 
    /////////////////////////////////////////////////////////////
    double start_pos = pos_len;
    double end_pos;
    if(increase)
      end_pos = start_pos + from_loc.Distance(to_loc);
    else
      end_pos = start_pos - from_loc.Distance(to_loc);

//    if(fabs(end_pos) < dist_delta) end_pos = 0.0;
    if(fabs(end_pos) < EPSDIST) end_pos = 0.0;

    pos_len = end_pos;

//       cout<<"end "<<" network id "<<networkId<<" route id "<<routeId
//           <<" side "<<side<<" increase "<<increase
//           <<" pos1 "<<start_pos<<" pos2 "<<end_pos<<endl;

//     cout<<"2 time interval "<<up_interval<<endl;

    UGPoint* ugp = 
        new UGPoint(up_interval,networkId,routeId,side,start_pos,end_pos);

    mgp->Add(*ugp);
//    ugp->Print(cout);
    delete ugp;
//    cout<<"components2 "<<mgp->GetNoComponents()<<endl;

    /////////////////////////////////////////////////////////////
    st = et; 
  }

  start_time = et; 

}



/*
create generic moving objects: bus, walk

*/
void GenMObject::GenerateGenMO3(Space* sp, Periods* peri, int mo_no, 
                      int type, Relation* rel1, Relation* rel2, 
                                R_Tree<2,TupleId>* rtree)
{
  if(mo_no < 1){
    cout<<" invalid number of moving objects "<<mo_no<<endl;
    return;
  }
  if(!(0 < type && type <= int(ARR_SIZE(genmo_tmlist)))){
    cout<<" invalid type value "<<type<<endl;
  }
    switch(type){
      case 8:
        GenerateGenMO_BusWalk(sp, peri, mo_no, rel1, rel2, rtree, "Bus");
        break;

      default:
        assert(false);
        break;  
  }


}


/*
generate locations on the pavment area 
!!! note that here we put the ----absolute position---- in loc instead of the 
relative position according to the triangle !!!

*/
void GenMObject::GenerateLocPave(Pavement* pm, int mo_no, 
                                 vector<GenLoc>& genloc_list, 
                                 vector<Point>& p_loc_list)
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

    Point p(true, loc1, loc2);
    p_loc_list.push_back(p);
  }

  delete wsp;
}


/*
provide an interface for creating moving objects with mode walk

*/
void GenMObject::GenerateWalkMovement(DualGraph* dg, Line* l, Point start_loc,
                            GenMO* genmo, MPoint* mo, Instant& start_time)
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
//   const double delta_dist = 0.01;

  ///////////////////////////////////////////////////////////////
   ////////we reference to the big overall large polygon/////////////
   //////////////////////////////////////////////////////////////////
   int gen_mo_ref_id = dg->min_tri_oid_1 + dg->GetNodeRel()->GetNoTuples() + 1;
//   cout<<gen_mo_ref_id<<endl;
    Rectangle<2> bbox = dg->rtree_node->BoundingBox();
//    cout<<bbox<<endl;
   ///////////////////////////////////////////////////////////////////

//   if(start_loc.Distance(temp_sp1) < delta_dist){
   if(start_loc.Distance(temp_sp1) < EPSDIST){
        Instant st = start_time;
        Instant et = start_time; 
        Interval<Instant> up_interval; 
        for(unsigned int i = 0;i < seq_halfseg.size();i++){
            Point from_loc = seq_halfseg[i].from;
            Point to_loc = seq_halfseg[i].to; 

            double dist = from_loc.Distance(to_loc);
            double time = dist; //assume the speed for pedestrian is 1.0m
 
//            if(dist < delta_dist){//ignore such small segment
            if(dist < EPSDIST){//ignore such small segment 
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
            start_time = et;
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
//   }else if(start_loc.Distance(temp_sp2) < delta_dist){
   }else if(start_loc.Distance(temp_sp2) < EPSDIST){
              Instant st = start_time;
              Instant et = start_time; 
              Interval<Instant> up_interval; 
              for(int i = seq_halfseg.size() - 1;i >= 0;i--){
                  Point from_loc = seq_halfseg[i].to;
                  Point to_loc = seq_halfseg[i].from; 
                  double dist = from_loc.Distance(to_loc);
                  double time = dist; 
//                  if(dist < delta_dist){//ignore such small segment
                  if(dist < EPSDIST){//ignore such small segment 
                        if((i + 1) < (int) seq_halfseg.size()){
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
                  start_time = et;
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

}

/*
free movement, transportation mode is given 

*/
void GenMObject::GenerateFreeMovement(Line* l, Point start_loc,
                            GenMO* genmo, MPoint* mo, Instant& start_time)
{
   string mode = "Free";
   SimpleLine* path = new SimpleLine(0);
   path->fromLine(*l);

   SpacePartition* sp = new SpacePartition();
   vector<MyHalfSegment> seq_halfseg; //reorder it from start to end
   sp->ReorderLine(path, seq_halfseg);
   delete sp;
   delete path;
   Point temp_sp1 = seq_halfseg[0].from; 
   Point temp_sp2 = seq_halfseg[seq_halfseg.size() - 1].to; 
//   const double delta_dist = 0.01;
   ///////////////////////////////////////////////////////////////////

//   if(start_loc.Distance(temp_sp1) < delta_dist){
   if(start_loc.Distance(temp_sp1) < EPSDIST){
        Instant st = start_time;
        Instant et = start_time; 
        Interval<Instant> up_interval; 
        for(unsigned int i = 0;i < seq_halfseg.size();i++){
            Point from_loc = seq_halfseg[i].from;
            Point to_loc = seq_halfseg[i].to; 

            double dist = from_loc.Distance(to_loc);
            double time = dist; //assume the speed for pedestrian is 1.0m
 
//            if(dist < delta_dist){//ignore such small segment
            if(dist < EPSDIST){//ignore such small segment 
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
            start_time = et;
            ////////////////////////////////////////////////////////////////
            Loc loc1(from_loc.GetX(),from_loc.GetY());
            Loc loc2(to_loc.GetX(), to_loc.GetY());
            GenLoc gloc1(0, loc1);
            GenLoc gloc2(0, loc2);
            int tm = GetTM(mode);
            UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
            genmo->Add(*unit); 
            delete unit; 
        }
//   }else if(start_loc.Distance(temp_sp2) < delta_dist){
   }else if(start_loc.Distance(temp_sp2) < EPSDIST){
              Instant st = start_time;
              Instant et = start_time; 
              Interval<Instant> up_interval; 
              for(int i = seq_halfseg.size() - 1;i >= 0;i--){
                  Point from_loc = seq_halfseg[i].to;
                  Point to_loc = seq_halfseg[i].from; 
                  double dist = from_loc.Distance(to_loc);
                  double time = dist; 
//                  if(dist < delta_dist){//ignore such small segment
                  if(dist < EPSDIST){//ignore such small segment 
                        if((i + 1) < (int)seq_halfseg.size()){
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
                  start_time = et;
                  /////////////////////////////////////////////////////////
                  Loc loc1(from_loc.GetX(), from_loc.GetY());
                  Loc loc2(to_loc.GetX(),to_loc.GetY());
                  GenLoc gloc1(0, loc1);
                  GenLoc gloc2(0, loc2);
                  int tm = GetTM(mode);
                  UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
                  genmo->Add(*unit); 
                  delete unit; 
              }
   }else assert(false);

}

/*
free movement, transportation mode is given 

*/
void GenMObject::GenerateFreeMovement2(Point start_loc, Point end_loc,
                            GenMO* genmo, MPoint* mo, Instant& start_time)
{
   string mode = "Free";
//   const double delta_dist = 0.01;
//   if(start_loc.Distance(end_loc) < delta_dist) return;
   if(start_loc.Distance(end_loc) < EPSDIST) return;
   ///////////////////////////////////////////////////////////////////
    Instant st = start_time;
    Instant et = start_time; 
    Interval<Instant> up_interval; 

    double dist = start_loc.Distance(end_loc);
    double time = dist; //assume the speed for pedestrian is 1.0m
 

    //double 1.0 means 1 day 
    et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0*60));
    ////////////////////create a upoint////////////////////////
    up_interval.start = st;
    up_interval.lc = true;
    up_interval.end = et;
    up_interval.rc = false; 
    UPoint* up = new UPoint(up_interval,start_loc,end_loc);
    mo->Add(*up);
    delete up; 
    //////////////////////////////////////////////////////////////
    st = et; 
    start_time = et;
   ////////////////////////////////////////////////////////////////
    Loc loc1(start_loc.GetX(),start_loc.GetY());
    Loc loc2(end_loc.GetX(), end_loc.GetY());
    GenLoc gloc1(0, loc1);
    GenLoc gloc2(0, loc2);
    int tm = GetTM(mode);
    UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
    genmo->Add(*unit); 
    delete unit; 
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
create a set of random gpoint locations 

*/
void GenMObject::GenerateGPoint2(Network* rn, int mo_no, 
                                 vector<GPoint>& gp_list, 
                                 vector<Point>& gp_loc_list)
{
  Relation* routes_rel = rn->GetRoutes();
  for (int i = 1; i <= mo_no;i++){
     int m = GetRandom() % routes_rel->GetNoTuples() + 1;
     Tuple* road_tuple = routes_rel->GetTuple(m, false);
     int rid = ((CcInt*)road_tuple->GetAttribute(ROUTE_ID))->GetIntval();
     SimpleLine* sl = (SimpleLine*)road_tuple->GetAttribute(ROUTE_CURVE);
//     cout<<"rid "<<rid<<" len: "<<sl->Length()<<endl; 
     double len = sl->Length();
     int pos = GetRandom() % (int)len;
     GPoint gp(true, rn->GetId(), rid, pos);
     Point* gp_loc = new Point();

     assert(sl->GetStartSmaller());
     assert(sl->AtPosition(pos, true, *gp_loc));

     gp_list.push_back(gp);
     gp_loc_list.push_back(*gp_loc);
     delete gp_loc;

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
//  const double dist_delta = 0.01; 
  
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
//    if(dist < dist_delta){//ignore such small segment
    if(dist < EPSDIST){//ignore such small segment 
        if((i + 1) < seq_halfseg.size()){
          seq_halfseg[i+1].from = from_loc; 
        }
        continue; 
    }
    /////////////////////////////////////////////////////////////////

    //double 1.0 means 1 day 
    et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0*60));
//    cout<<st<<" "<<et<<endl;

    int64_t start_time = st.ToDouble()*86400000.0;
    int64_t end_time = et.ToDouble()*86400000.0;
    if(start_time == end_time){/////////time is equal, unit is not valid
        if((i + 1) < seq_halfseg.size()){
          seq_halfseg[i + 1].from = from_loc; 
        }
        continue; 
    }

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
// const double dist_delta = 0.01; 
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
//    if(dist < dist_delta){//ignore such small segment
    if(dist < EPSDIST){//ignore such small segment 
        if((i - 1) >= 0 ){
          seq_halfseg[i - 1].to = from_loc; 
        }
        continue; 
    }

    //double 1.0 means 1 day 
    et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0*60.0));
//    cout<<st<<" "<<et<<endl;

    int64_t start_time = st.ToDouble()*86400000.0;
    int64_t end_time = et.ToDouble()*86400000.0;
    if(start_time == end_time){/////////time is equal, unit is not valid
        if((i - 1) >= 0 ){
          seq_halfseg[i - 1].to = from_loc; 
        }
        continue; 
    }

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

Note:
the start and end locations could be on the zebra crossing which intersect the
road line, then there is no movement in the free space 
so some movement have free space, while some do not have

*/
void GenMObject::GenerateGenMO_CTBWalk(Space* sp, Periods* peri, int mo_no,
                             Relation* rel, BTree* btree, 
                                       Relation* speed_rel, string mode)
{

//  cout<<"Mode Car-Walk "<<endl;
  const double min_len = 100.0;
  Pavement* pm = sp->LoadPavement(IF_REGION);
  vector<GenLoc> genloc_list;
  vector<Point> p_loc_list;
  //generate locations on pavements 
  GenerateLocPave(pm, mo_no, genloc_list, p_loc_list);
  int count = 1;
  Network* rn = sp->LoadRoadNetwork(IF_LINE);
  if(rn == NULL){
    cout<<"road network loading error"<<endl;
    return;
  }

  RoadGraph* rg = sp->LoadRoadGraph();
  if(rg == NULL){
    cout<<"road graph loading error"<<endl;
    return;
  }

  Interval<Instant> periods;
  peri->Get(0, periods);

  RoadNav* road_nav = new RoadNav();

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
    bool correct = true;
    PaveLoc2GPoint(loc1, loc2, sp, rel, btree, gpoint_list, p_list, correct,rn);
    if(correct == false){
      delete mo;
      delete genmo;
      continue;
    }

    GPoint gp1 = gpoint_list[0];
    GPoint gp2 = gpoint_list[1];
    Point start_loc = p_list[0];
    Point end_loc = p_list[1];
    if(start_loc.Distance(end_loc) < min_len){
      delete mo;
      delete genmo;
      continue;
    }

    //////////////////////////////////////////////////////////////////////
    ////////////2 connect the path to the end point//////////////////////
    //////////////////////////////////////////////////////////////////////

    ConnectStartMove(loc1, start_loc, mo, genmo, start_time, pm);

    //////////////////////////////////////////////////////////////////////
    ///////////3 shortest path between two gpoints////////////////////////
    //////////////////////////////////////////////////////////////////////
    GLine* gl = new GLine(0);
    road_nav->ShortestPathSub(&gp1, &gp2, rg, rn, gl);
    ConnectGP1GP2(rn, start_loc, gl, mo, genmo, start_time, 
                  speed_rel, mode);
    delete gl;

    //////////////////////////////////////////////////////////////////////
    ////////////4 connect the path to the end point//////////////////////
    //////////////////////////////////////////////////////////////////////

    ConnectEndMove(end_loc, loc2, mo, genmo, start_time, pm);

    ////////////////////////////////////////////////////////////////////
    cout<<count<<" generic moving object"<<endl;

    count++;

    mo->EndBulkLoad();
    genmo->EndBulkLoad(false, false);

    trip1_list.push_back(*genmo);
    trip2_list.push_back(*mo);

    delete mo;
    delete genmo; 
  }
  
  delete road_nav;

  sp->ClosePavement(pm);
  sp->CloseRoadGraph(rg);
  sp->CloseRoadNetwork(rn);

}

/*
map the two points on the pavement to loations on the road network 

*/
void GenMObject::PaveLoc2GPoint(GenLoc loc1, GenLoc loc2, Space* sp, 
                                Relation* rel, BTree* btree, 
                                vector<GPoint>& gpoint_list, 
                                vector<Point>& p_list, bool& correct, 
                                Network* rn)
{

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

    correct = wsp1->PaveLocToGPoint(&p1, rn, route_id_list1);
    if(correct == false) {
       delete wsp1;
       return;
    }
    
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

    correct = wsp2->PaveLocToGPoint(&p2, rn, route_id_list2);
    if(correct == false){
      delete wsp2;
      return;
    }
    GPoint gp2 = wsp2->gp_list[0];
    Point gp_loc2 = wsp2->p_list[0];
    delete wsp2;
    ///////////////////////////////////////////////////////////////////////

    gpoint_list.push_back(gp1);
    gpoint_list.push_back(gp2);
    p_list.push_back(gp_loc1);
    p_list.push_back(gp_loc2);

}

/*
build the connection from a point on the pavement to the road network point
subpath1: on the pavement
subpath2: in the free space (mode: free)
loc1: point on the pavement; end loc: point maps to a gpoint 

*/
void GenMObject::ConnectStartMove(GenLoc loc1, Point end_loc, MPoint* mo,
                        GenMO* genmo, Instant& start_time, 
                                  Pavement* pm)
{
//  const double delta_dist = 0.01;
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
      double time = dist/slowspeed;///// define as free space 
    //////////////////////////////////////////////////////////////
//      if(dist > delta_dist){//ingore too small segment
      if(dist > EPSDIST){//ingore too small segment 
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
//          int tm = GetTM(mode);
          int tm = GetTM("Free");//free space 
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
//  if(fabs(l->Length() - line->Length()) < delta_dist){///////one movement
  if(fabs(l->Length() - line->Length()) < EPSDIST){///////one movement

//        cout<<"still on pavement "<<endl;

        double dist = start_loc.Distance(end_loc);
        double time = dist/1.0;///// define as walk
    //////////////////////////////////////////////////////////////
//       if(dist > delta_dist){//ingore too small segment
       if(dist > EPSDIST){//ingore too small segment 

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
  ////////////////////movement---1//////from pavement to border/////////////
      double dist1 = start_loc.Distance(middle_loc);
      double time1 = dist1/1.0;///// define as walk
//      if(dist1 > delta_dist){//ingore too small segment
      if(dist1 > EPSDIST){//ingore too small segment 

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
//      if(dist2 > delta_dist){//ingore too small segment
      if(dist2 > EPSDIST){//ingore too small segment 

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
//          int tm = GetTM(mode);
          int tm = GetTM("Free");//free space 
//          cout<<up_interval<<" "<<middle_loc<<" "<<end_loc<<endl;

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

        if(fabs(ri->GetStartPos() - ri->GetEndPos()) < delta_dist){
            delete ri;
            continue;
        }

        int rid = ri->GetRouteId();
        Tuple* speed_tuple = rel->GetTuple(rid, false);
     assert(((CcInt*)speed_tuple->GetAttribute(SPEED_RID))->GetIntval() == rid);
        double speed = 
        ((CcReal*)speed_tuple->GetAttribute(SPEED_VAL))->GetRealval();
        speed_tuple->DeleteIfAllowed();

      if(speed < 0.0) speed = 10.0;
      speed = speed * 1000.0/3600.0;// meters in second 

      if(mode.compare("Bike") == 0){
          if(speed > 30 * 1000/3600) //get information from street 
             speed = 30 * 1000/3600.0;// maximum 30 km per hour for bike 
          else
             speed = 20 * 1000/3600.0;
      }

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
      double d1 = start_loc.Distance(temp_sp1);
      double d2 = start_loc.Distance(temp_sp2);

      if(d1 < d2 ){
        CreateCarTrip1(mo, seq_halfseg, start_time, speed);
        start_loc = temp_sp2;

      }else if(d1 > d2){
        CreateCarTrip2(mo, seq_halfseg, start_time, speed);
        start_loc = temp_sp1;
      }else{
        cout<<"start loc "<<start_loc
            <<" p1 "<<temp_sp1<<" p2 "<<temp_sp2<<endl;
        assert(false);
      }

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
//      cout<<unit_interval<<" "<<gloc1<<" "<<gloc2<<endl;
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
subpath2: in the free space (mode: free)
loc: point on the pavement; start loc: point maps to a gpoint 

*/
void GenMObject::ConnectEndMove(Point start_loc, GenLoc loc, MPoint* mo, 
                        GenMO* genmo, Instant& start_time, Pavement* pm)
{
//  const double delta_dist = 0.01;
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
      double time = dist/slowspeed;///// define as free 
    //////////////////////////////////////////////////////////////
//      if(dist > delta_dist){//ingore too small segment
      if(dist > EPSDIST){//ingore too small segment 
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
//          int tm = GetTM(mode);
          int tm = GetTM("Free");
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
//  if(fabs(l->Length() - line->Length()) < delta_dist){///////one movement
  if(fabs(l->Length() - line->Length()) < EPSDIST){///////one movement

//        cout<<"still on pavement "<<endl;

        double dist = start_loc.Distance(end_loc);
        double time = dist/1.0;///// define as walk
    //////////////////////////////////////////////////////////////
//       if(dist > delta_dist){//ingore too small segment
       if(dist > EPSDIST){//ingore too small segment 

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
        double time1 = dist1/lowspeed;///// define as free 
//        if(dist1 > delta_dist){//ingore too small segment
        if(dist1 > EPSDIST){//ingore too small segment 

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
//          int tm = GetTM(mode);
          int tm = GetTM("Free");//free space 
          UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
          genmo->Add(*unit); 
          delete unit; 
        }

        ///movement---2/////from pavement border to pavement loc///free space//
        double dist2 = middle_loc.Distance(end_loc);
        double time2 = dist2/1.0;///// define as walk
//        if(dist2 > delta_dist){//ingore too small segment
        if(dist2 > EPSDIST){//ingore too small segment 

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

/*
moving objects with mode bus and walk.
randomly select two locations on the pavement and their closest bus stops.
build the connection between them. this is not so correct for real trip 
plannings because the closest bus stops may not involve the best trip.
but it is ok to generate generic moving objects

*/
void GenMObject::GenerateGenMO_BusWalk(Space* sp, Periods* peri, int mo_no,
                             Relation* rel1, Relation* rel2, 
                             R_Tree<2,TupleId>* rtree, string mode)
{
    Pavement* pm = sp->LoadPavement(IF_REGION);
    BusNetwork* bn = sp->LoadBusNetwork(IF_BUSNETWORK);

    vector<GenLoc> genloc_list;
    //generate locations on pavements
    vector<Point> p_loc_list;
    GenerateLocPave(pm, obj_scale*mo_no, genloc_list, p_loc_list);
    DualGraph* dg = pm->GetDualGraph();
    VisualGraph* vg = pm->GetVisualGraph();

    Interval<Instant> periods;
    peri->Get(0, periods);
    int count = 1;
    const double min_len = 500.0;

    int obj_no_rep = 0;
    int max_obj = 1;
    if(mo_no <= 500) max_obj = 2;
    else if(mo_no <= 1000) max_obj = 3;
    else if(mo_no <= 5000) max_obj = 4;
    else max_obj = 5;


    int index1 = -1;
    int index2 = -1;

    int time_and_type = 4;
    while(count <= mo_no){

//         int index1 = GetRandom() % genloc_list.size();
//         GenLoc loc1 = genloc_list[index1];
//         int index2 = GetRandom() % genloc_list.size();
//         GenLoc loc2 = genloc_list[index2];
        if(obj_no_rep == 0){
            index1 = GetRandom() % genloc_list.size();
            index2 = GetRandom() % genloc_list.size();
        }

        GenLoc loc1 = genloc_list[index1];
        GenLoc loc2 = genloc_list[index2];

        if(index1 == index2) {
          obj_no_rep = 0;
          continue;
        }

        Point pave_loc1(true, loc1.GetLoc().loc1, loc1.GetLoc().loc2);
        Point pave_loc2(true, loc2.GetLoc().loc1, loc2.GetLoc().loc2);
        if(pave_loc1.Distance(pave_loc2) < min_len) {
          obj_no_rep = 0;
          continue; 
        }


        ////////////////////////////////////////////////////////////
        ///////////// initialization////////////////////////////////
        ////////////////////////////////////////////////////////////
        MPoint* mo = new MPoint(0);
        GenMO* genmo = new GenMO(0);
        mo->StartBulkLoad();
        genmo->StartBulkLoad();
        //////////////////////////////////////////////////////////////
        ///////////////set start time/////////////////////////////////
        /////////////////////////////////////////////////////////////
        Instant start_time = periods.start;
        int time_range = 14*60;//14 hours 
        bool time_type;
        if(count % time_and_type == 0) time_type = true;
        else time_type = false;

        if(time_type)// movement on sunday 
            start_time.ReadFrom(periods.start.ToDouble() + 
                        (GetRandom() % time_range)/(24.0*60.0));
        else
            start_time.ReadFrom(periods.end.ToDouble() -
                        (GetRandom() % time_range)/(24.0*60.0));

//        cout<<"start time "<<start_time<<endl; 
        ///////////////debuging//////////////////////////////
        /*
          Loc temp1(1.0, 2.0);
          Loc temp2(2.0, 3.0);
          loc1.Set(123, temp1);
          loc2.Set(232, temp2);
        */
        /////////////////////////////////////////////////////
        //////////////////////////////////////////////
//        cout<<"genloc1 "<<loc1<<" genloc2 "<<loc2<<endl; 
        ////////////////////////////////////////////////////////////////////
        //////1. find closest bus stops to the two points on the pavement///
        ///////////////////////////////////////////////////////////////////
        Bus_Stop bs1(true, 0, 0, true);
        vector<Point> ps_list1;//1 point on the pavement 2 point of bus stop
        GenLoc gloc1(0, Loc(0, 0));//bus stop on pavement by genloc 
        bool b1 = true;
        b1 = NearestBusStop(loc1,  rel2, rtree, bs1, ps_list1, gloc1, true);

        Bus_Stop bs2(true, 0, 0, true);
        vector<Point> ps_list2;//1 point on the pavement 2 point of bus stop
        GenLoc gloc2(0, Loc(0, 0));//bus stop on pavement by genloc
        bool b2 = true;
        b2 = NearestBusStop(loc2,  rel2, rtree, bs2, ps_list2, gloc2, false);

        if((b1 && b2) == false) {
            mo->EndBulkLoad();
            genmo->EndBulkLoad();
            delete mo;
            delete genmo;
            obj_no_rep = 0;
            continue;
        }
        /////////////////////////////////////////////////////////////////
        /////////2 connect start location to start bus stop/////////////
        ////////////////////////////////////////////////////////////////
        Line* res_path = new Line(0);
        ConnectStartStop(dg, vg, rel1, loc1, ps_list1, gloc1.GetOid(),
                            genmo, mo, start_time, res_path);

        /////////////////////////////////////////////////////////////////
        //////3. get the path in bus network/////////////////////////////
        /////////////////////////////////////////////////////////////////
//        cout<<"time type "<<time_type<<endl; 
//        cout<<"bs1 "<<bs1<<" bs2 "<<bs2<<endl; 


        BNNav* bn_nav = new BNNav(bn);
         if(count % time_and_type != 0){ //more movment with minimum time 
             bn_nav->ShortestPath_Time2(&bs1, &bs2, &start_time);
         }
         else{
             bn_nav->ShortestPath_Transfer2(&bs1, &bs2, &start_time);
         }


        if(bn_nav->path_list.size() == 0){
//          cout<<"two unreachable bus stops"<<endl;
          mo->EndBulkLoad();
          genmo->EndBulkLoad();
          delete mo;
          delete genmo;
          delete bn_nav;
          delete res_path;
          obj_no_rep = 0;
          continue;
        }
        ///////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////
//        cout<<"whole period "<<periods<<endl;

//         int temp_index = bn_nav->peri_list.size() - 1;
//         for(; temp_index >= 0; temp_index--){
//           if(bn_nav->peri_list[temp_index].GetNoComponents() > 0)break;
//         }
//         assert(temp_index >= 0);
//        Interval<Instant> periods_temp;
//        bn_nav->peri_list[temp_index].Get(0, periods_temp);
//        cout<<" "<<periods1<<" "<<periods2<<endl;

        Interval<Instant> temp_periods;
        temp_periods.start = start_time;
        temp_periods.lc = true;
        Instant temp_end(instanttype);
        temp_end.ReadFrom(start_time.ToDouble() + bn_nav->t_cost/86400.0);
        temp_periods.end = temp_end;
        temp_periods.lc = false;

        if(periods.Contains(temp_periods) == false){
            mo->EndBulkLoad();
            genmo->EndBulkLoad();
            delete mo;
            delete genmo;
            delete bn_nav;
            delete res_path;
            obj_no_rep = 0;
            continue;
        }

        ///////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////

        int last_walk_id = ConnectTwoBusStops(bn_nav, ps_list1[1], ps_list2[1],
                           genmo, mo, start_time, dg, res_path);

        ///////////////////////////////////////////////////////////
        if(last_walk_id > 0){
          //////change the last bus stop/////////
          ///change  ps list2, gloc2.oid ///
          Bus_Stop cur_bs1(true, 0, 0, true);
          StringToBusStop(bn_nav->bs1_list[last_walk_id], 
                          cur_bs1);

//           Bus_Stop cur_bs2(true, 0, 0, true);
//           StringToBusStop(bn_nav->bs2_list[bn_nav->bs2_list.size() - 1], 
//                           cur_bs2);
//          cout<<"cur_bs1 "<<cur_bs1<<" cur_bs2 "<<cur_bs2<<endl;

            ChangeEndBusStop(bn, dg, cur_bs1, ps_list2, gloc2, rel2, rtree);
        }

        ////////////////////////////////////////////////////////////
        delete bn_nav;
        ///////////////////////////////////////////////////////////////////
        /////////////4 connect end location to last bus stop////////////////
        //////////////////////////////////////////////////////////////////
        ConnectEndStop(dg, vg, rel1, loc2, ps_list2, gloc2.GetOid(),
                          genmo, mo, start_time, res_path);
        ///////////////////////////////////////////////////////////////////////
        cout<<count<<" generic moving object "<<endl;
        count++;

        mo->EndBulkLoad();
        genmo->EndBulkLoad(false, false);

//        line_list1.push_back(*res_path);
        trip1_list.push_back(*genmo);
        trip2_list.push_back(*mo);

        delete res_path;

        delete mo;
        delete genmo;

        //////////////////////////////////////////////////////////////////
        ////////////reduce the time cost of generating moving object//////
        ////////////if two bus stops are reachable, it generates the second////
        //////// moving object, but different time intervals //////////////////
        ///////////////////////////////////////////////////////////////////////

        obj_no_rep++;
        if(obj_no_rep == max_obj) obj_no_rep = 0;
    }

    pm->CloseDualGraph(dg);
    pm->CloseVisualGraph(vg);
    sp->CloseBusNetwork(bn);
    sp->ClosePavement(pm);

    
}

/*
find the nearest bus stops to the query location with e.g., 200 meters.
this method is not for trip planning with walk and bus
in that case, it should define a distance e.g., 200 meters and find all 
possible connections.
but here, it finds the closet, nearest bus stop to the location in the pavement

*/
bool GenMObject::NearestBusStop(GenLoc loc, Relation* rel,
                      R_Tree<2,TupleId>* rtree, Bus_Stop& res, 
                      vector<Point>& ps_list, GenLoc& bs_gloc, bool start_pos)
{
  Point p(true, loc.GetLoc().loc1, loc.GetLoc().loc2);
  SmiRecordId adr = rtree->RootRecordId();

  vector<int> tid_list;
  DFTraverse1(rtree, adr, rel, p, tid_list);

//  cout<<p<<" "<<tid_list.size()<<endl;
  
  if(tid_list.size() == 0) return false;
  
  vector<MyPoint_Tid> mp_tid_list;
  for(unsigned int i = 0;i < tid_list.size();i++){
    Tuple* tuple = rel->GetTuple(tid_list[i], false);
    Point* q = (Point*)tuple->GetAttribute(BN::BN_PAVE_LOC2);
    MyPoint_Tid mp_tid(*q, q->Distance(p), tid_list[i]);
    mp_tid_list.push_back(mp_tid);
    tuple->DeleteIfAllowed();
  }
  sort(mp_tid_list.begin(), mp_tid_list.end());

//  for(unsigned int i = 0;i < mp_tid_list.size();i++)
//    mp_tid_list[i].Print();
  Tuple* bs_pave = rel->GetTuple(mp_tid_list[0].tid, false);
  Bus_Stop* bs = (Bus_Stop*)bs_pave->GetAttribute(BN::BN_BUSSTOP);
  Point* bs_loc = (Point*)bs_pave->GetAttribute(BN::BN_BUSLOC);
  GenLoc* bs_loc2 = (GenLoc*)bs_pave->GetAttribute(BN::BN_PAVE_LOC1);

  res = *bs;
  ps_list.push_back(mp_tid_list[0].loc);
  ps_list.push_back(*bs_loc);
  bs_gloc = *bs_loc2;
  bs_pave->DeleteIfAllowed();

  ///////////////////////////////////////////////////////////////////////
  //////////////for debuging, they are located on the pavement//////////
  //////////////////////////////////////////////////////////////////////
  if(start_pos){
//    loc_list1.push_back(p);
//    loc_list2.push_back(mp_tid_list[0].loc);
  }else{
//    loc_list1.push_back(mp_tid_list[0].loc);
//    loc_list2.push_back(p);
  }
  ///////////////////////////////////////////////////////////////////////

  return true;
}

/*
find all points that their distance to query loc is smaller than the 
defined max dist

*/
void GenMObject::DFTraverse1(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                             Relation* rel,
                             Point query_loc, vector<int>& tid_list)
{
//  const double max_dist = 500.0;
  const double max_dist = NEARBUSSTOP;
  
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple = rel->GetTuple(e.info, false);
              Point* loc = (Point*)dg_tuple->GetAttribute(BN::BN_PAVE_LOC2);

              if(loc->Distance(query_loc) < max_dist){
                tid_list.push_back(e.info);
              }
              dg_tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(query_loc.Distance(e.box) < max_dist){
                DFTraverse1(rtree, e.pointer, rel, query_loc, tid_list);
            }
      }
  }
  delete node;
}

/*
connect the start location to the nearest bus stop
walk + bus (free space)

*/

void GenMObject::ConnectStartStop(DualGraph* dg, VisualGraph* vg,
                                     Relation* rel, GenLoc genloc1,
                                     vector<Point> ps_list1, int oid,
                                GenMO* genmo, MPoint* mo, 
                                Instant& start_time, Line* res_path)
{
  Walk_SP* wsp = new Walk_SP(dg, vg, NULL, NULL);
  wsp->rel3 = rel;

  int no_triangle = dg->GetNodeRel()->GetNoTuples();
  int mini_oid = dg->min_tri_oid_1;

  Point p1(true, genloc1.GetLoc().loc1, genloc1.GetLoc().loc2);
  Point p2 = ps_list1[0];
  int oid1 = genloc1.GetOid() - mini_oid;
  assert(1 <= oid1 && oid1 <= no_triangle);

  int oid2 = oid - mini_oid;
//    cout<<"oid2 "<<oid2<<endl;
  assert(1 <= oid2 && oid2 <= no_triangle);

  Line* path = new Line(0);

  ///////////////////walk segment////////////////////////////////////
  wsp->WalkShortestPath2(oid1, oid2, p1, p2, path);
  

  ////////////////create moving objects///////////////////////////////////
  if(path->IsDefined() && path->Length() > 0.01)
    GenerateWalkMovement(dg, path, p1, genmo, mo, start_time);
  /////////////////////////////////////////////////////////////////////////
  
  /////////////////////////////////////////////////////////////////////////
  ////connection between bus stop and its mapping point on the pavement////
  //////////////////////for debuging/////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  
  Line* path2 = new Line(0);
  path2->StartBulkLoad();
  HalfSegment hs(true, ps_list1[0], ps_list1[1]);
  hs.attr.edgeno = 0;
  *path2 += hs;
  hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
  *path2 += hs;
  path2->EndBulkLoad();

  path->Union(*path2, *res_path);

  delete path2;


  ///////////////////////////////////////////////////////////////////////
  /////////////// create moving objects ////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  ShortMovement(genmo, mo, start_time, &ps_list1[0], &ps_list1[1]);

  delete path;
  delete wsp;
}

/*
create the moving object between bus stops in bus network

*/
int GenMObject::ConnectTwoBusStops(BNNav* bn_nav, Point sp, Point ep,
                                    GenMO* genmo_input, MPoint* mo, 
                                    Instant& start_time, 
                                    DualGraph* dg, Line* res_path)
{
    GenMO* genmo = new GenMO(0);
    genmo->StartBulkLoad();

    const double delta_t = 0.01;//seconds
//    const double delta_dist = 0.01;

    Line* l = new Line(0); ////////trajectory in bus network
    l->StartBulkLoad();
    int edgeno = 0;

    ////////////////////////////////////////////////////////////////////
    ///////////////////find the last walk segment///////////////////////
    ///////////////////////////////////////////////////////////////////
    int last_walk_id = -1;
    for(int i = bn_nav->path_list.size() - 1; i >= 0; i--){
        int tm = GetTM(bn_nav->tm_list[i]);
        if(tm < 0 ) continue;
        if(tm == TM_WALK){
          last_walk_id = i;
        }else
          break;
    }
    ////////////////////////////////////////////////////////////////////

    Bus_Stop last_bs(false, 0, 0, true);
    MPoint mo_bus(0);
    int mo_bus_index = -1;
    int mobus_oid = 0;
    
//    double time1 = 0.0;

    for(unsigned int i = 0;i < bn_nav->path_list.size();i++){
      if(last_walk_id == (int)i) break;/////stop at the last walk segment 

      SimpleLine* sl = &(bn_nav->path_list[i]);
      ////////////time cost: second///////////////////
      Bus_Stop bs1(true, 0, 0, true);
      StringToBusStop(bn_nav->bs1_list[i], bs1);

      Bus_Stop bs2(true, 0, 0, true);
      StringToBusStop(bn_nav->bs2_list[i], bs2);

      double t = bn_nav->time_cost_list[i];
      int tm = GetTM(bn_nav->tm_list[i]);
      /////////////////////////////////////////////////////////////
//       cout<<bs1<<" "<<bs2<<" "<<bn_nav->tm_list[i]<<" "
//           <<sl->Size()<<" cost: "<<t<<" "<<"st "<<start_time<<endl;

      Point* start_loc = new Point(true, 0, 0);
      bn_nav->bn->GetBusStopGeoData(&bs1, start_loc);
      Point* end_loc = new Point(true, 0, 0);
      bn_nav->bn->GetBusStopGeoData(&bs2, end_loc);

//      cout<<*start_loc<<" "<<*end_loc<<endl;
      /////filter the first part and transfer without movement ////////
      if(sl->Size() == 0 && AlmostEqual(t, 0.0)) continue;

/*        cout<<sl->Length()<<" bs1 "<<bs1<<" bs2 "<<bs2
            <<" time "<<t<<" tm "<<bn_nav->tm_list[i]<<endl;

       cout<<"start time 1 "<<start_time<<endl;*/
      ///////////////////////////////////////////////////////////

      Instant temp_start_time = start_time; 

      if(tm == TM_WALK){


        Line* l1 = new Line(0);
        ///////////////////////////////////////////////////////////////////
        /////////////filter the first and last segment//////////////////////
        /////////////connection between bus stop and its mapping point//////
        ////this part is not considered as walk segment in the overall region//
        //////////////////////////////////////////////////////////////////
//        sl->toLine(*l1);

        Point new_start_loc;
        Point new_end_loc;
        bool init_start = false;
        bool init_end = false;
        l1->StartBulkLoad();
        int l_edgeno = 0;

        for(int j = 0;j < sl->Size();j++){
          HalfSegment hs1;
          sl->Get(j, hs1);
          if(!hs1.IsLeftDomPoint()) continue;
          Point lp = hs1.GetLeftPoint();
          Point rp = hs1.GetRightPoint();
/*          cout<<start_loc->Distance(lp)<<" "<<start_loc->Distance(rp)<<" "
              <<end_loc->Distance(lp)<<" "<<end_loc->Distance(rp)<<endl;*/
//          if(start_loc->Distance(lp) < delta_dist){
          if(start_loc->Distance(lp) < EPSDIST){
            new_start_loc = rp;
            init_start = true;
            continue;
          }
//          if(start_loc->Distance(rp) < delta_dist){
          if(start_loc->Distance(rp) < EPSDIST){
            new_start_loc = lp;
            init_start = true;
            continue;
          }
//          if(end_loc->Distance(lp) < delta_dist){
          if(end_loc->Distance(lp) < EPSDIST){
            new_end_loc = rp;
            init_end = true;
            continue;
          }
//          if(end_loc->Distance(rp) < delta_dist){
          if(end_loc->Distance(rp) < EPSDIST){
            new_end_loc = lp;
            init_end = true;
            continue;
          }
          HalfSegment hs2(true, lp, rp);
          hs2.attr.edgeno = l_edgeno++;
          *l1 += hs2;
          hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
          *l1 += hs2;
        }
        l1->EndBulkLoad();

        ///////////////////////////////////////////////////////////////
        assert(init_start && init_end); 

        ShortMovement(genmo, mo, start_time, start_loc, &new_start_loc);

        if(l1->IsDefined() && l1->Length() > 0.01)
          GenerateWalkMovement(dg, l1, new_start_loc, genmo, mo, start_time);
        delete l1;

        ShortMovement(genmo, mo, start_time, &new_end_loc, end_loc);
        /////////////////////////////////
        MPoint temp_mo(0);
        mo_bus = temp_mo;
        mo_bus_index = -1;
        mobus_oid = 0;

      }else{
        assert(bs1.GetId() == bs2.GetId() && bs1.GetUp() == bs2.GetUp());

        if(sl->Size() == 0){////////////no movement in space

            Instant st = start_time;
            Instant et = start_time; 
            Interval<Instant> up_interval; 
            et.ReadFrom(st.ToDouble() + t*1.0/(24.0*60.0*60.0));
            up_interval.start = st;
            up_interval.lc = true;
            up_interval.end = et;
            up_interval.rc = false; 

//            if(AlmostEqual(t, 30.0)){///bus waiting at the bus stop
            if(fabs(t - BUSWAITING) < delta_t){///bus waiting at the bus stop
                  ////////generic moving objects/////////////////
                  ////////reference to the bus///////////////////
                  /////with bs find br, with br.uoid find mobus///
                  //////////// mode = bus////////////////////////
                  ///////////////////////////////////////////////
//                 clock_t start1, finish1;
//                 start1 = clock();

//                 int mbus_oid = 
//                       bn_nav->bn->GetMOBus_Oid(&bs1, start_loc, start_time);
//                cout<<mobus_oid<<" "<<mbus_oid<<endl;
                int mbus_oid = mobus_oid;
                if(mbus_oid == 0){
                    mbus_oid = 
                      bn_nav->bn->GetMOBus_Oid(&bs1, start_loc, start_time);
                }

//                 finish1 = clock();
//                 time1 += (double)(finish1 - start1) / CLOCKS_PER_SEC;
//                 printf("%.4f\n", time1);


                Loc loc1(UNDEFVAL, UNDEFVAL);
                Loc loc2(UNDEFVAL, UNDEFVAL);

                GenLoc gloc1(mbus_oid, loc1);
                GenLoc gloc2(mbus_oid, loc2);
                int tm = GetTM("Bus");
                UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
                genmo->Add(*unit); 
                delete unit; 

            }else{/////////waiting for transfer//////////////////////
                /////////generic moving objects///////////////////
                /////////referenc to free space oid = 0, mode = free ////
                ////////////////////////////////////////////////////
                Loc loc1(start_loc->GetX(), start_loc->GetY());
                Loc loc2(start_loc->GetX(), start_loc->GetY());
                GenLoc gloc1(0, loc1);
                GenLoc gloc2(0, loc2);
//                int tm = GetTM("Bus");
                int tm = GetTM("Free");
                UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
                genmo->Add(*unit); 
                delete unit; 
                ////////////////////////////////////////////////
                MPoint temp_mo(0);
                mo_bus = temp_mo;
                mo_bus_index = -1;
                mobus_oid = 0;
            }

            UPoint* up = new UPoint(up_interval,*start_loc, *start_loc);
            mo->Add(*up);
            delete up;

            start_time = et;

            ///////////////////////////////////////////////////////////
            //////////// transfer to another bus route ////////////////
            //////////// chanage the moving bus////////////////////////
            //////////////////////////////////////////////////////////
//            cout<<"bs1 "<<bs1<<" last bs"<<last_bs<<endl;
            if(bs1.GetId() != last_bs.GetId()){ //clear the previous moving bus

              MPoint temp_mo(0);
              mo_bus = temp_mo;
              mo_bus_index = -1;
              mobus_oid = 0;
            }

        }else{//////////moving with the bus 
//           cout<<"moving with bus"
//               <<" mo_bus size "<<mo_bus.GetNoComponents()<<endl;

          if(mo_bus.GetNoComponents() == 0){
            ////////////////////////////////////////
            //////////find the bus//////////////////
            //////////mode = bus////////////////////
            ////////////////////////////////////////
            assert(mobus_oid == 0);
            mobus_oid = 
              bn_nav->bn->GetMOBus_MP(&bs1, start_loc, start_time, mo_bus);
/*            cout<<"bus mpoint size "<<mo_bus.GetNoComponents()
                <<" mobus oid "<<mobus_oid<<endl;*/
            assert(mobus_oid > 0);
            int pos1 = -1;
            int pos2 = -1;
            /////////find the range in mpoint for bs1, bs2//////////////
            FindPosInMP(&mo_bus, start_loc, end_loc, pos1, pos2, 0);

//          cout<<"bus oid "<<mobus_oid<<" "<<*start_loc<<" "<<*end_loc<<endl;

            assert(pos1 >= 0 && pos2 >= 0 && pos1 <= pos2);

            /////////////////////////////////////////////////////////////
            ///////////////set up mpoint//////////////////////////////
            ////////////////////////////////////////////////////////////
//            cout<<"initial start time "<<start_time<<endl;
            SetMO_GenMO(&mo_bus, pos1, pos2, start_time, mo, 
                        genmo, mobus_oid, "Bus");
            ///////////////////////////////////////////////////////////////
            mo_bus_index = pos2; 
            mo_bus_index++;/////////omit the 30 seconds waiting movement 

          }else{
            assert(last_bs.IsDefined());
            if(bs1.GetId() == last_bs.GetId() && 
               bs1.GetUp() == last_bs.GetUp()){//reference to the same bus

            int pos1 = -1;
            int pos2 = -1;
            /////////find the range in mpoint for bs1, bs2//////////////
            FindPosInMP(&mo_bus, start_loc, end_loc, pos1, pos2, mo_bus_index);
            assert(pos1 >= 0 && pos2 >= 0 && pos1 <= pos2);

            SetMO_GenMO(&mo_bus, pos1, pos2, start_time, mo, 
                        genmo, mobus_oid, "Bus");

            ///////////////////////////////////////////////////////////////
            mo_bus_index = pos2; 
            mo_bus_index++;/////////omit the 30 seconds waiting movement 

            }else{
                //////////doing transfer without any waiting time/////////////
//             cout<<"seldomly happend"<<endl;
            //////////////////////////////////////////
            /////////////generic unit/////////////////
            //////////reference to bus, mode = bus////
            //////////////////////////////////////////
              MPoint temp_mo(0);
              mo_bus = temp_mo;
              mo_bus_index = -1;
              mobus_oid = 0;

              mobus_oid = 
                bn_nav->bn->GetMOBus_MP(&bs1, start_loc, start_time, mo_bus);

              assert(mobus_oid > 0);
              int pos1 = -1;
              int pos2 = -1;
            /////////find the range in mpoint for bs1, bs2//////////////
              FindPosInMP(&mo_bus, start_loc, end_loc, pos1, pos2, 0);
              assert(pos1 >= 0 && pos2 >= 0 && pos1 <= pos2);
              SetMO_GenMO(&mo_bus, pos1, pos2, start_time, mo, 
                         genmo, mobus_oid, "Bus");
             ///////////////////////////////////////////////////////////////
              mo_bus_index = pos2; 
              mo_bus_index++;/////////omit the 30 seconds waiting movement 
            }
          } 

         ///////////remove this part when the above code works correctly//////
//         start_time.ReadFrom(temp_start_time.ToDouble() + 
//                            t*1.0/(24.0*60.0*60.0));

        }

      }

//      cout<<"start time 2 "<<start_time<<endl;

      for(int j = 0;j < sl->Size();j++){
        HalfSegment hs1;
        sl->Get(j, hs1);
        if(!hs1.IsLeftDomPoint()) continue;
        HalfSegment hs2(true, hs1.GetLeftPoint(), hs1.GetRightPoint());
        hs2.attr.edgeno = edgeno++;
        *l += hs2;
        hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
        *l += hs2;
      }

      last_bs = bs2;
      delete start_loc;
      delete end_loc; 

    }////////end of big for 

    l->EndBulkLoad();

    Line* temp_l = new Line(0);
    l->Union(*res_path, *temp_l);
    *res_path = *temp_l;
    delete temp_l;

    delete l;


    genmo->EndBulkLoad();
    /////////////////////////////////////////////////////////////////
    //////////////merge bus units referenc to the same bus /////////
    /////////////////////////////////////////////////////////////////
    vector<UGenLoc> ugloc_list;
    for(int i = 0;i < genmo->GetNoComponents();i++){
      UGenLoc u(true);
      genmo->Get(i, u);
      if(ugloc_list.size() == 0 || u.tm != TM_BUS){
        ugloc_list.push_back(u);

      }else{
        UGenLoc v = ugloc_list[ugloc_list.size() - 1];
        if(u.GetOid() != v.GetOid()){
            ugloc_list.push_back(u);

        }else{ //merge bus units 
            UGenLoc w = u;
            while(u.GetOid() == v.GetOid() && u.tm == v.tm &&
                  u.tm == TM_BUS && i < genmo->GetNoComponents()){
                w = u;
                i++;
//                cout<<"i "<<i<<endl;
                if(i < genmo->GetNoComponents())
                   genmo->Get(i, u);
          }
          assert(v.GetOid() == w.GetOid());
          Interval<Instant> up_interval; 
          up_interval.start = v.timeInterval.start;
          up_interval.lc = v.timeInterval.lc;
          up_interval.end = w.timeInterval.end;
          up_interval.rc = w.timeInterval.rc;

          UGenLoc* unit = new UGenLoc(up_interval, v.gloc1, w.gloc2, v.tm);
          ugloc_list[ugloc_list.size() - 1] = *unit;
          delete unit; 

          if(i < genmo->GetNoComponents())i--;
        }
      }
    }
    for(unsigned int i = 0;i < ugloc_list.size();i++)
      genmo_input->Add(ugloc_list[i]);
   ////////////////////////////////////////////////////
   delete genmo; 

   return last_walk_id;
}


/*
get the bus stop from input string with the format
br: 1 stop: 13 UP

*/
void GenMObject::StringToBusStop(string str, Bus_Stop& bs)
{
  char* cstr = new char [str.size()+1];
  strcpy (cstr, str.c_str());

  char* p=strtok (cstr," ");
  int count = 0;
  
  int br_id = 0;
  int stop_id = 0;
  bool direction;
  bool init = false;
  while (p!=NULL)
  {
//    cout << p << endl;

    if(count == 1){
      br_id = atoi(p);
    }
    if(count == 3){
      stop_id = atoi(p);
    }
    if(count == 4){
        char down[] = "DOWN";
        char up[] = "UP";
        if(strcmp(p, down) == 0){
          direction = false;
        }else if(strcmp(p, up) == 0)
          direction = true;

        init = true;
    }
    count++;
    p=strtok(NULL," ");
  }

  delete[] cstr;

  assert(br_id > 0 && stop_id > 0 && init);
  bs.Set(br_id, stop_id, direction);
//  cout<<bs<<endl;

}


/*
the movement between bus stops on the bus network and its mapping points on
the pavement, it is in free space 

*/
void GenMObject::ShortMovement(GenMO* genmo, MPoint* mo, Instant& start_time,
                     Point* p1, Point* p2)
{

  Instant st = start_time;
  Instant et = start_time; 
  Interval<Instant> up_interval; 
  
  double dist = p1->Distance(*p2);
  double slow_speed = 10.0*1000.0/3600.0;
  double time = dist/slow_speed;
  et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0*60));
  ////////////////////create a upoint////////////////////////
  up_interval.start = st;
  up_interval.lc = true;
  up_interval.end = et;
  up_interval.rc = false; 
  UPoint* up = new UPoint(up_interval, *p1, *p2);
  mo->Add(*up);
  delete up; 
  /////////////////generic units////////////////////////////////
  Loc loc1(p1->GetX(), p1->GetY());
  Loc loc2(p2->GetX(), p2->GetY());
  GenLoc gloc1(0, loc1);
  GenLoc gloc2(0, loc2);
//  int tm = GetTM("Bus");
  int tm = GetTM("Free");//in free space 
  UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
  genmo->Add(*unit); 
  delete unit; 

  start_time = et;

}

/*
find the two positions in dbarray units that their spatial location equal to
the two input locations, the start entry is given by the input index


*/
void GenMObject::FindPosInMP(MPoint* mo_bus, Point* start_loc, Point* end_loc,
                   int& pos1, int& pos2, int index)

{
//  cout<<"index "<<index<<endl; 
//  cout<<*start_loc<<" "<<*end_loc<<endl; 


//  const double delta_dist = 0.01;
  const double delta_dist = 0.05;

  for(int i = index;i < mo_bus->GetNoComponents();i++){
    UPoint unit;
    mo_bus->Get(i, unit);
    Point p1 = unit.p0;
    Point p2 = unit.p1;

/*    if(start_loc->Distance(p1) < 1.0)
      cout<<"s-p1 "<<start_loc->Distance(p1)<<endl;
    if(start_loc->Distance(p2) < 1.0)
      cout<<"s-p2 "<<start_loc->Distance(p2)<<endl;
    if(end_loc->Distance(p1) < 1,0)
      cout<<"e-p1 "<<end_loc->Distance(p1)<<endl;
    if(end_loc->Distance(p2) < 1.0)
      cout<<"e-p2 "<<end_loc->Distance(p2)<<endl;*/
//     cout<<"p1 "<<p1<<" p2 "<<p2<<endl;
    if(start_loc->Distance(p1) < delta_dist && 
       start_loc->Distance(p2) > delta_dist){
      pos1 = i;

    }
    if(end_loc->Distance(p1) > delta_dist && 
       end_loc->Distance(p2) < delta_dist){
       pos2 = i;
       assert(pos1 >= 0);
       break;
    }
  }
//  cout<<"pos1 "<<pos1<<" pos2 "<<pos2<<endl; 

}

/*
set the value for mpoint and genmo 

*/
void GenMObject::SetMO_GenMO(MPoint* mo_bus, int pos1, int pos2, 
                             Instant& start_time,  MPoint* mo, GenMO* genmo, 
                             int mobus_oid, string str_tm)
{
//      cout<<"pos1 "<<pos1<<" "<<pos2<<endl;

      Instant st_temp = start_time;
      Interval<Instant> up_interval;
      bool first_time = true;

      for(; pos1 <= pos2;pos1++){
          UPoint unit;
          mo_bus->Get(pos1, unit);
          Instant st = unit.timeInterval.start;
          Instant et = unit.timeInterval.end;
          if(first_time){////////////waiting for bus moving 
              if(st > st_temp){
                up_interval.start = st_temp;
                up_interval.lc = true;
                up_interval.end = st;
                up_interval.rc = false; 

                UPoint* temp_up = new UPoint(up_interval, unit.p0, unit.p0);
                mo->Add(*temp_up);
                delete temp_up;
              }
              first_time = false;
              start_time = st;
          }

          up_interval.start = st;
          up_interval.lc = true;
          up_interval.end = et;
          up_interval.rc = false; 

          UPoint* up = new UPoint(up_interval, unit.p0, unit.p1);
          mo->Add(*up);
          delete up;

          start_time = et;
//              cout<<"start time "<<start_time<<endl;
      }

      up_interval.start = st_temp;
      up_interval.lc = true;
      up_interval.end = start_time;
      up_interval.rc = false; 

      ////////////////////////////////////////////////////////////
      ////////////////generic moving object//////////////////////

      /////////////record the start and end position of such a unit/////////
      Loc loc1(UNDEFVAL, UNDEFVAL);
      Loc loc2(UNDEFVAL, UNDEFVAL);

      GenLoc gloc1(mobus_oid, loc1);
      GenLoc gloc2(mobus_oid, loc2);
//      int tm = GetTM("Bus");
      int tm = GetTM(str_tm);
//            cout<<up_interval<<endl; 

      ////////////////////////////////////////////////////////////
      //////merge units if reference to the same bus or metro////
      //////////////////////////////////////////////////////////

      UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
      genmo->Add(*unit);
      delete unit;

}

/*
change the last bus stop as the last connection in bus network is walk 
segment. so it can directly walk from the second last bus stop in bus network
to destination

*/
void GenMObject::ChangeEndBusStop(BusNetwork* bn, DualGraph* dg, 
                        Bus_Stop cur_bs1, vector<Point>& ps_list2, 
                        GenLoc& gloc2, Relation* rel, R_Tree<2,TupleId>* rtree)
{

  Point* bs_loc = new Point(true, 0, 0);
  bn->GetBusStopGeoData(&cur_bs1, bs_loc);

  Point res(true, 0, 0);
  int tri_id = 0;
  NearestBusStop2(*bs_loc, rel, rtree, res, tri_id);
  assert(tri_id > 0);
  ps_list2[0] = res;
  ps_list2[1] = *bs_loc;
  Loc loc(gloc2.GetLoc().loc1, gloc2.GetLoc().loc2);
  gloc2.SetValue(tri_id, loc);
  delete bs_loc;
}

/*
get the mapping point on the pavement of the bus stop and the triangle id

*/
void GenMObject::NearestBusStop2(Point loc, Relation* rel, 
                      R_Tree<2,TupleId>* rtree, Point& res, int& oid)
{

  SmiRecordId adr = rtree->RootRecordId();

  vector<int> tid_list;
  DFTraverse1(rtree, adr, rel, loc, tid_list);

//  cout<<p<<" "<<tid_list.size()<<endl;

  assert(tid_list.size() > 0);

  vector<MyPoint_Tid> mp_tid_list;
  for(unsigned int i = 0;i < tid_list.size();i++){
    Tuple* tuple = rel->GetTuple(tid_list[i], false);
    Point* q = (Point*)tuple->GetAttribute(BN::BN_PAVE_LOC2);
//    MyPoint_Tid mp_tid(q, q->Distance(loc), tid_list[i]);
    MyPoint_Tid mp_tid(*q, q->Distance(loc), tid_list[i]);
    mp_tid_list.push_back(mp_tid);
    tuple->DeleteIfAllowed();
  }
  sort(mp_tid_list.begin(), mp_tid_list.end());

//  for(unsigned int i = 0;i < mp_tid_list.size();i++)
//    mp_tid_list[i].Print();
  Tuple* bs_pave = rel->GetTuple(mp_tid_list[0].tid, false);
  Point* pave_loc = (Point*)bs_pave->GetAttribute(BN::BN_PAVE_LOC2);
  GenLoc* bs_gloc = (GenLoc*)bs_pave->GetAttribute(BN::BN_PAVE_LOC1);
  
  res = *pave_loc;
  oid = bs_gloc->GetOid();
  bs_pave->DeleteIfAllowed();

}

/*
connect the end location to the nearest bus stop
 bus (free space) + walk 

*/

void GenMObject::ConnectEndStop(DualGraph* dg, VisualGraph* vg,
                                     Relation* rel, GenLoc genloc1,
                                     vector<Point> ps_list1, int oid,
                                 GenMO* genmo, MPoint* mo, 
                                   Instant& start_time, Line* res_path)
{
  Walk_SP* wsp = new Walk_SP(dg, vg, NULL, NULL);
  wsp->rel3 = rel;

  int no_triangle = dg->GetNodeRel()->GetNoTuples();
  int mini_oid = dg->min_tri_oid_1;

  Point p1 = ps_list1[0];
  Point p2(true, genloc1.GetLoc().loc1, genloc1.GetLoc().loc2);

  int oid1 = oid - mini_oid;
//    cout<<"oid2 "<<oid2<<endl;
  assert(1 <= oid1 && oid1 <= no_triangle);
  
  int oid2 = genloc1.GetOid() - mini_oid;
  assert(1 <= oid2 && oid2 <= no_triangle);

  ///////////////////////////////////////////////////////////////////////
  /////////////// create moving objects ////////////////////////////////
  ///////////////////////////////////////////////////////////////////////

  ShortMovement(genmo, mo, start_time, &ps_list1[1], &ps_list1[0]);
  ///////////////////////////////////////////////////////////////////////
  //////////////////walk segment for debuging///////////////////////////
  ///////////////////////////////////////////////////////////////////////

  Line* path = new Line(0);

  wsp->WalkShortestPath2(oid1, oid2, p1, p2, path);
  
  /////////////////////////for debuging//////////////////////////////
//  line_list2.push_back(*path);

  ////////////////create moving objects///////////////////////////////////

  if(path->IsDefined() && path->Length() > 0.01)
    GenerateWalkMovement(dg, path, p1, genmo, mo, start_time);
  /////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////
  ////connection between bus stop and its mapping point on the pavement////
  //////////////////////////////////////////////////////////////////////////
  ///////////////////for debuging/////////////////////////////
   Line* path2 = new Line(0);
   path2->StartBulkLoad();
   HalfSegment hs(true, ps_list1[1], ps_list1[0]);
   hs.attr.edgeno = 0;
   *path2 += hs;
   hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
   *path2 += hs;
   path2->EndBulkLoad();

  Line* path12 = new Line(0);
  path->Union(*path2, *path12);
  Line* temp_l = new Line(0);
  path12->Union(*res_path, *temp_l);
  *res_path = *temp_l;
  delete temp_l;
  delete path12;

  delete path2; 

  //////////////////////////////////////////////////////////////////////////
  delete path;
  delete wsp;
}

/*
this structure is used to find the shortest path between the entrances of two
buildings

*/
struct ID_Length{
  int id1;
  int id2;
  double l;
  ID_Length(){}
  ID_Length(int a, int b, double len):id1(a), id2(b), l(len){}
  ID_Length(const ID_Length& id_l):id1(id_l.id1), id2(id_l.id2),l(id_l.l){}
  ID_Length& operator=(const ID_Length& id_l)
  {
    id1 = id_l.id1;
    id2 = id_l.id2;
    l = id_l.l;
    return *this;
  }
  bool operator<(const ID_Length& id_l) const
  {
    return l < id_l.l;
  }
  void Print()
  {
    cout<<"id1 "<<id1<<" id2 "<<id2<<" len "<<l<<endl;
  }

};

/*
create generic moving objects with indoor + walk

*/

void GenMObject::GenerateGenMO4(Space* sp,
                                 Periods* peri, int mo_no, int type, 
                                 Relation* tri_rel)
{

  if(mo_no < 1){
    cout<<" invalid number of moving objects "<<mo_no<<endl;
    return;
  }
  if(!(0 < type && type <= int(ARR_SIZE(genmo_tmlist)))){
    cout<<" invalid type value "<<type<<endl;
  }

  IndoorInfra* i_infra = sp->LoadIndoorInfra(IF_GROOM);
  if(i_infra == NULL){
    cout<<"indoor infrastructure does not exist "<<endl;
    return;
  }

  ////////////////////////////////////////////////////////////////
  //////////////////////Initialization/////////////////////////////
  ////////////////////////////////////////////////////////////////
  Pavement* pm = sp->LoadPavement(IF_REGION);
  DualGraph* dg = pm->GetDualGraph();
  int no_triangle = dg->GetNodeRel()->GetNoTuples();
  int mini_oid = dg->min_tri_oid_1;
  VisualGraph* vg = pm->GetVisualGraph();
  Walk_SP* wsp = new Walk_SP(dg, vg, NULL, NULL);
  wsp->rel3 = tri_rel;
  
  //////////////////////////////////////////////////////////////////
  ////////////////load all buildings and indoor graphs////////////////////////
  MaxRect* maxrect = new MaxRect();
  maxrect->OpenBuilding();
  maxrect->OpenIndoorGraph();

#ifdef INDOOR_PATH
  ///////////////load indoor paths/////////////////////////////
  maxrect->LoadIndoorPaths(indoor_paths_list, rooms_id_list);
  assert(indoor_paths_list.size() == rooms_id_list.size());
#endif

  ////////////////////////////////////////////////////////////////
  ////////////select a pair of buildings/////////////////////////
  //////////////////////////////////////////////////////////////
  vector<RefBuild> build_id1_list;
  vector<RefBuild> build_id2_list;
  CreateBuildingPair(i_infra, build_id1_list, build_id2_list, 
                    obj_scale*mo_no, maxrect);
  ///////////////////////////////////////////////////////////////////
//  Relation* build_type_rel = i_infra->BuildingType_Rel();
  Relation* build_path_rel = i_infra->BuildingPath_Rel();
  //////////////////////////////////////////////////////////////////
  ///////////////start time///////////////////////////////////////
   Interval<Instant> periods;
   peri->Get(0, periods);
   Instant start_time = periods.start;
   int time_range = 12*60;//12 hours in range 
  ////////////////////////////////////////////////////////////
  const double min_path = 0.01;
  
  int count = 0;
  int real_count = 1;

  ////////////////////////////////////////////////////////////
//  mo_no = 0;//////stop the while loop 
  ////////////////////////////////////////////////////////////

  while(real_count <= mo_no && count < obj_scale*mo_no){

   //////////////////////////////start time///////////////////////////
   if(count % 3 == 0)
     start_time.ReadFrom(periods.start.ToDouble() + 
                        (GetRandom() % time_range)/(24.0*60.0));
   else
     start_time.ReadFrom(periods.end.ToDouble() -
                        (GetRandom() % time_range)/(24.0*60.0));

    /////////////////////load all paths from this building////////////////
    vector<int> path_id_list1;
    i_infra->GetPathIDFromTypeID(build_id1_list[count].reg_id, path_id_list1);
//    cout<<"number of paths "<<path_id_list1.size()<<endl;

    vector<int> path_id_list2;
    i_infra->GetPathIDFromTypeID(build_id2_list[count].reg_id, path_id_list2);
//    cout<<"number of paths "<<path_id_list2.size()<<endl;
   ////////////////////////////////////////////////////////////////////
   //////////////if a building has several entrances///////////////////
   ///////////it selects the shortest outdoor pair///////////////////////
   ////////////////////////////////////////////////////////////////////
   if(path_id_list1.size() == 0 || path_id_list2.size() == 0){
      count++;
      continue;
    }
   //////////////////////////////////////////////////////////////////////
   cout<<"building 1 "<<GetBuildingStr(build_id1_list[count].type)
        <<" building 2 "<<GetBuildingStr(build_id2_list[count].type)<<endl;

   ////////////////////////////////////////////////////////////////////////
   MPoint* mo = new MPoint(0);
   GenMO* genmo = new GenMO(0);
   mo->StartBulkLoad();
   genmo->StartBulkLoad();

   /////////////////////////////////////////////////////////////////////
   //////calculate all possible outdoor paths between two buildings/////
   ////////////////////////////////////////////////////////////////////

    vector<ID_Length> id_len_list;
    for(unsigned int i = 0;i < path_id_list1.size();i++){
      int path_tid1 = path_id_list1[i];
      Tuple* path_tuple1 = build_path_rel->GetTuple(path_tid1, false);
      GenLoc* gloc1 = 
        (GenLoc*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
      Point* ep1_2 = (Point*)path_tuple1->GetAttribute(IndoorInfra::
                                                     INDOORIF_EP2);
      int oid1 = gloc1->GetOid() - mini_oid;
      assert(1 <= oid1 && oid1 <= no_triangle);
      for(unsigned int j = 0;j < path_id_list2.size();j++){
          int path_tid2 = path_id_list2[j];
          Tuple* path_tuple2 = build_path_rel->GetTuple(path_tid2, false);
          GenLoc* gloc2 = 
        (GenLoc*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
         Point* ep2_2 = (Point*)path_tuple2->GetAttribute(IndoorInfra::
                                                          INDOORIF_EP2);

         int oid2 = gloc2->GetOid() - mini_oid;

         assert(1 <= oid2 && oid2 <= no_triangle);

         Line* path = new Line(0);

//         cout<<path_tid1<<" "<<path_tid2<<endl;

         wsp->WalkShortestPath2(oid1, oid2, *ep1_2, *ep2_2, path);

         ID_Length id_len(path_tid1, path_tid2, path->Length());
         id_len_list.push_back(id_len);
         delete path;
      }
    }
    sort(id_len_list.begin(), id_len_list.end());


    //////////////////////////////////////////////////////////////////////////
     ////////////////select the shortest path between two entrances//////////
     /////////////////////////////////////////////////////////////////////////

     int path_tid1 = id_len_list[0].id1;
     int path_tid2 = id_len_list[0].id2;

     Tuple* path_tuple1 = build_path_rel->GetTuple(path_tid1, false);
     Tuple* path_tuple2 = build_path_rel->GetTuple(path_tid2, false);
    GenLoc* gloc1 = 
        (GenLoc*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);

    Point* ep1_1 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP);
    Point* ep1_2 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP2);


    GenLoc* gloc2 = 
        (GenLoc*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
    Point* ep2_1 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP);
    Point* ep2_2 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP2);
    
    int oid1 = gloc1->GetOid() - mini_oid;

    assert(1 <= oid1 && oid1 <= no_triangle);

    int oid2 = gloc2->GetOid() - mini_oid;

    assert(1 <= oid2 && oid2 <= no_triangle);
    /////////////////////////////////////////////////////////////////////////
    ///////////////////movement inside the first building////////////////////
    ////////////////////////////////////////////////////////////////////////
    ////////////////////to show which entrance it is////////////////////////
    int entrance_index1 = ((CcInt*)path_tuple1->GetAttribute(IndoorInfra::
                           INDOORIF_SP_INDEX))->GetIntval();
    int reg_id1 =  ((CcInt*)path_tuple1->GetAttribute(IndoorInfra::
                                               INDOORIF_REG_ID))->GetIntval();
     Point* sp1 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_SP);
     GenerateIndoorMovementToExit(i_infra, genmo, mo, start_time, *sp1,
                                  entrance_index1, reg_id1, maxrect, peri);


    ///////////////////////////////////////////////////////////////////////
    /////////////path1: from entrance to pavement//////////////////////////
    //////////////////////////////////////////////////////////////////////
    Line* path1 = (Line*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_PATH);

    if(path1->Length() > min_path)
      GenerateFreeMovement(path1, *sp1, genmo, mo, start_time);

    GenerateFreeMovement2(*ep1_1, *ep1_2, genmo, mo, start_time);

    //////////////////////////////////////////////////////////////////////////
    //////////////movement in pavement area///////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    Line* path = new Line(0);

    wsp->WalkShortestPath2(oid1, oid2, *ep1_2, *ep2_2, path);


//     cout<<" length "<<path->Length()
//         <<" start_time "<<start_time<<endl;
    if(path->Length() > min_path)
      GenerateWalkMovement(dg, path, *ep1_2, genmo, mo, start_time);

    delete path; 

    //////////////////path2: from pavement to building/////////////////////////
    GenerateFreeMovement2(*ep2_2, *ep2_1, genmo, mo, start_time);

    Line* path2 = (Line*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_PATH);
    if(path2->Length() > min_path)
      GenerateFreeMovement(path2, *ep2_1, genmo, mo, start_time);

    //////////////////////////////////////////////////////////////////////
    /////////////////////////movement inside the second building//////////
    //////////////////////////////////////////////////////////////////////
    ////////////////////to show which entrance it is////////////////////
   Point* sp2 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_SP);
   int entrance_index2 = ((CcInt*)path_tuple2->GetAttribute(IndoorInfra::
                                   INDOORIF_SP_INDEX))->GetIntval();
   int reg_id2 =  ((CcInt*)path_tuple2->GetAttribute(IndoorInfra::
                                              INDOORIF_REG_ID))->GetIntval();
   if(WorkBuilding(build_id2_list[count].type))
      GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                   entrance_index2, reg_id2, maxrect, 
                                      peri, TIMESPAN_W1);
   else
      GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                   entrance_index2, reg_id2, maxrect, peri,
                                      TIMESPAN_NO);

//    cout<<"reg_id1 "<<reg_id1<<" reg_id2 "<<reg_id2<<endl;

    path_tuple1->DeleteIfAllowed();
    path_tuple2->DeleteIfAllowed();
    ////////////////////////////////////////////////////////////////////////

    mo->EndBulkLoad();
    genmo->EndBulkLoad(false, false);

    trip1_list.push_back(*genmo);
    trip2_list.push_back(*mo);

    build_type_list1.push_back(build_id1_list[count].type); 
    build_type_list2.push_back(build_id2_list[count].type); 

    delete mo; 
    delete genmo; 
    ///////////////////////////////////////////////////////////////////////
    count++;

    cout<<real_count<<" generic moving object"<<endl;
    real_count++;

  }

  maxrect->CloseIndoorGraph();
  maxrect->CloseBuilding();
  delete maxrect;

  delete wsp;

  pm->CloseDualGraph(dg);
  pm->CloseVisualGraph(vg);
  sp->ClosePavement(pm);
  sp->CloseIndoorInfra(i_infra);

}

/*
select a pair of buildings. 
the buildings should not be too far away from each other

*/
void GenMObject::CreateBuildingPair(IndoorInfra* i_infra, 
                                    vector<RefBuild>& build_tid1_list, 
                                    vector<RefBuild>& build_tid2_list, int no,
                                    MaxRect* maxrect)
{
  const double max_dist = 500.0;//Euclidean distance
  Relation* build_type_rel = i_infra->BuildingType_Rel();
  int count = 0;
  while(count < no){
    int id1 = GetRandom() % build_type_rel->GetNoTuples() + 1; 
    int id2 = GetRandom() % build_type_rel->GetNoTuples() + 1; 
    if(id1 == id2) continue;
    
    Tuple* tuple1 = build_type_rel->GetTuple(id1, false);
    Tuple* tuple2 = build_type_rel->GetTuple(id2, false);

    Rectangle<2>* bbox1 = 
        (Rectangle<2>*)tuple1->GetAttribute(IndoorInfra::INDOORIF_GEODATA);
    Rectangle<2>* bbox2 = 
        (Rectangle<2>*)tuple2->GetAttribute(IndoorInfra::INDOORIF_GEODATA);
    if(bbox1->Distance(*bbox2) > max_dist){
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed();
      continue;
    }

    int type1 = ((CcInt*)tuple1->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_TYPE))->GetIntval();

    int type2 = ((CcInt*)tuple2->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_TYPE))->GetIntval();


//    cout<<GetBuildingStr(type1)<<" "<<GetBuildingStr(type2)<<endl;
    //////////////////check whether the building is available/////////////////
    ///////////////////there is no building and indoor graph ///////////////
    ///////////////////for personal houses/////////////////////////////////
    if(type1 > 1 && maxrect->build_pointer[type1] == NULL){
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed();

      continue;
    }
    if(type2 > 1 && maxrect->build_pointer[type2] == NULL){
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed(); 

      continue;
    }


    int reg_id1 = ((CcInt*)tuple1->GetAttribute(IndoorInfra::
                INDOORIF_REG_ID))->GetIntval();

    int reg_id2 = ((CcInt*)tuple2->GetAttribute(IndoorInfra::
                INDOORIF_REG_ID))->GetIntval();

    int build_id1 = ((CcInt*)tuple1->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_ID))->GetIntval();

    int build_id2 = ((CcInt*)tuple2->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_ID))->GetIntval();

    RefBuild ref_b1(true, reg_id1, build_id1, type1, *bbox1, id1 );
    RefBuild ref_b2(true, reg_id2, build_id2, type2, *bbox2, id2 );
    ////////////////////////////////////////////////////////////////////////
    build_tid1_list.push_back(ref_b1);
    build_tid2_list.push_back(ref_b2);

    count++;
//    cout<<"count "<<count<<endl;

//    rect_list1.push_back(*bbox1);
//    rect_list2.push_back(*bbox2);

    tuple1->DeleteIfAllowed();
    tuple2->DeleteIfAllowed();

  }
}

/*
generate an indoor movement from somewhere to the entrance
entrance index: which entrance it is
reg id: get the building id and type

*/
void GenMObject::GenerateIndoorMovementToExit(IndoorInfra* i_infra,
                                              GenMO* genmo, MPoint* mo, 
                                              Instant& start_time, Point loc,
                                              int entrance_index, 
                                              int reg_id, 
                                              MaxRect* maxrect, Periods* peri)
{
  ///////////////////////////////////////////////////////////////////////////
  //////////load the building: with reg id , we know the building type///////
  ///////////////////////////////////////////////////////////////////////////
  int build_type;
  int build_id;
  Rectangle<2> build_rect(false);
  i_infra->GetTypeFromRegId(reg_id, build_type, build_id, build_rect);
//  cout<<"to exit buiding: "<<GetBuildingStr(build_type)<<endl;
  if(build_type == BUILD_HOUSE){///not necessary to load the indoor graph
    MPoint3D* mp3d = new MPoint3D(0); 
    mp3d->StartBulkLoad();
    indoor_mo_list1.push_back(*mp3d);
    mp3d->EndBulkLoad();
    delete mp3d;
  }else{
//    cout<<"load indoor graph (to exit)"<<endl;
    assert(maxrect->igraph_pointer[build_type] != NULL);
    assert(maxrect->build_pointer[build_type] != NULL);
    Building* build = maxrect->build_pointer[build_type];
    Relation* room_rel = build->GetRoom_Rel();
    IndoorNav* indoor_nav = new IndoorNav(room_rel, NULL);
    

    unsigned int num_elev = indoor_nav->NumerOfElevators();

    IndoorGraph* ig = maxrect->igraph_pointer[build_type];
    BTree* btree_room = build->GetBTree();
    R_Tree<3, TupleId>* rtree_room = build->GetRTree();
    //////////////indoor bounding box///////////
    Rectangle<3> build_box = rtree_room->BoundingBox();
    double min[2], max[2];
    min[0] = build_box.MinD(0);
    min[1] = build_box.MinD(1);
    max[0] = build_box.MaxD(0);
    max[1] = build_box.MaxD(1);
    Rectangle<2> build_box2(true, min, max);

//    cout<<"number of elevators "<<num_elev<<endl;

#ifdef INDOOR_PATH
    ////////////////indoor paths and rooms id list ///////////////////
    indoor_nav->indoor_paths_list = indoor_paths_list[build_type];
    indoor_nav->rooms_list = rooms_id_list[build_type];
#endif 
    indoor_nav->ig = ig;


    MPoint3D* mp3d = new MPoint3D(0); 
    if(num_elev <= 1){
      Instant t1 = start_time;

      indoor_nav->GenerateMO3_End(ig, btree_room, rtree_room, start_time,
                                build_id, entrance_index,mp3d, genmo, peri);
      Instant t2 = start_time;

      ////////////////////////////////////////////////////////////////////
      ///////////////set up outdoor movement/////////////////////////////
      //////////////////////////////////////////////////////////////////
//       Interval<Instant> up_interval; 
//       up_interval.start = t1;
//       up_interval.lc = true;
//       up_interval.end = t2;
//       up_interval.rc = false; 
//       UPoint* up = new UPoint(up_interval, loc, loc);
//       mo->Add(*up);
//       delete up; 

    }else{
//      cout<<"several elevators "<<endl;
      Instant t1 = start_time;

      indoor_nav->GenerateMO3_New_End(ig, btree_room, rtree_room, start_time,
                            build_id, entrance_index,mp3d,genmo,peri,num_elev);

      Instant t2 = start_time;

      ////////////////////////////////////////////////////////////////////
      ///////////////set up outdoor movement/////////////////////////////
      //////////////////////////////////////////////////////////////////
/*      Interval<Instant> up_interval; 
      up_interval.start = t1;
      up_interval.lc = true;
      up_interval.end = t2;
      up_interval.rc = false; 
      UPoint* up = new UPoint(up_interval, loc, loc);
      mo->Add(*up);
      delete up; */
    }
    MapMP3DToMP(mo, mp3d, build_rect, build_box2);
    indoor_mo_list1.push_back(*mp3d);
    delete mp3d;

    delete indoor_nav;
  }
}

/*
almost the same as GenerateIndoorMovementToExit but specifying the start 
location

*/
void GenMObject::GenerateIndoorMovementToExitExt(IndoorInfra* i_infra,
                                              GenMO* genmo, MPoint* mo, 
                                              Instant& start_time, Point loc,
                                              int entrance_index, int reg_id, 
                                              MaxRect* maxrect, 
                                              Periods* peri, GenLoc gloc_input)
{
  ///////////////////////////////////////////////////////////////////////////
  //////////load the building: with reg id , we know the building type///////
  ///////////////////////////////////////////////////////////////////////////
  int build_type;
  int build_id;
  Rectangle<2> build_rect(false);
  i_infra->GetTypeFromRegId(reg_id, build_type, build_id, build_rect);
//  cout<<"to exit buiding: "<<GetBuildingStr(build_type)<<endl;
  if(build_type == BUILD_HOUSE){///not necessary to load the indoor graph
    MPoint3D* mp3d = new MPoint3D(0); 
    mp3d->StartBulkLoad();
    indoor_mo_list1.push_back(*mp3d);
    mp3d->EndBulkLoad();
    delete mp3d;
  }else{
//    cout<<"load indoor graph (to exit)"<<endl;
    assert(maxrect->igraph_pointer[build_type] != NULL);
    assert(maxrect->build_pointer[build_type] != NULL);
    Building* build = maxrect->build_pointer[build_type];
    Relation* room_rel = build->GetRoom_Rel();
    IndoorNav* indoor_nav = new IndoorNav(room_rel, NULL);
    

    unsigned int num_elev = indoor_nav->NumerOfElevators();

    IndoorGraph* ig = maxrect->igraph_pointer[build_type];
    BTree* btree_room = build->GetBTree();
    R_Tree<3, TupleId>* rtree_room = build->GetRTree();

    //////////////indoor bounding box///////////
    Rectangle<3> build_box = rtree_room->BoundingBox();
    double min[2], max[2];
    min[0] = build_box.MinD(0);
    min[1] = build_box.MinD(1);
    max[0] = build_box.MaxD(0);
    max[1] = build_box.MaxD(1);
    Rectangle<2> build_box2(true, min, max);

//    cout<<"number of elevators "<<num_elev<<endl;

#ifdef INDOOR_PATH
    ////////////////indoor paths and rooms id list ///////////////////
    indoor_nav->indoor_paths_list = indoor_paths_list[build_type];
    indoor_nav->rooms_list = rooms_id_list[build_type];
#endif 
    indoor_nav->ig = ig;

    MPoint3D* mp3d = new MPoint3D(0);
    if(num_elev <= 1){
      Instant t1 = start_time;
      indoor_nav->GenerateMO3_EndExt(ig, btree_room, rtree_room, start_time,
                                build_id, entrance_index,mp3d, 
                                     genmo, peri, gloc_input);
      Instant t2 = start_time;
      ////////////////////////////////////////////////////////////////////
      ///////////////set up outdoor movement/////////////////////////////
      //////////////////////////////////////////////////////////////////
//       Interval<Instant> up_interval;
//       up_interval.start = t1;
//       up_interval.lc = true;
//       up_interval.end = t2;
//       up_interval.rc = false; 
//       UPoint* up = new UPoint(up_interval, loc, loc);
//       mo->Add(*up);
//       delete up;

    }else{
//      cout<<"several elevators "<<endl;
      Instant t1 = start_time;

      indoor_nav->GenerateMO3_New_EndExt(ig, btree_room, rtree_room, start_time,
                       build_id, entrance_index, mp3d, genmo, 
                                         peri, num_elev, gloc_input);

      Instant t2 = start_time;
      ////////////////////////////////////////////////////////////////////
      ///////////////set up outdoor movement/////////////////////////////
      //////////////////////////////////////////////////////////////////
/*      Interval<Instant> up_interval; 
      up_interval.start = t1;
      up_interval.lc = true;
      up_interval.end = t2;
      up_interval.rc = false; 
      UPoint* up = new UPoint(up_interval, loc, loc);
      mo->Add(*up);
      delete up;*/
    }

    MapMP3DToMP(mo, mp3d, build_rect, build_box2);
    indoor_mo_list3.push_back(*mp3d);
    delete mp3d;

    delete indoor_nav;
  }
}

/*
create the indoor movement from the building entrance to 
somewhere in the building
entrance index: which entrance it is
reg id: get the building id and type

for working building (office, university), it create a unit without anymovement 
to represent the case that people stay in the room for some time 
almost the same as GenerateIndoorMovementFromExit

*/
void GenMObject::GenerateIndoorMovementFromExit(IndoorInfra* i_infra,
                                              GenMO* genmo, MPoint* mo, 
                                              Instant& start_time, Point loc,
                                              int entrance_index, 
                                              int reg_id,  MaxRect* maxrect,
                                              Periods* peri, int para)
{
  ///////////////////////////////////////////////////////////////////////////
  //////////load the building: with reg id , we know the building type///////
  ///////////////////////////////////////////////////////////////////////////
  int build_type;
  int build_id;
  Rectangle<2> build_rect(false);
  i_infra->GetTypeFromRegId(reg_id, build_type, build_id, build_rect);
//  cout<<"from exit buiding: "<<GetBuildingStr(build_type)<<endl;
  if(build_type == BUILD_HOUSE){///not necessary to load the indoor graph
    MPoint3D* mp3d = new MPoint3D(0); 
    indoor_mo_list2.push_back(*mp3d);
    delete mp3d;
  }else{
//    int t_span = 4*60; //4 hours
    int t_span = para*60; //4 hours 

//    cout<<"load indoor graph (from exit)"<<endl;
    assert(maxrect->igraph_pointer[build_type] != NULL);

    assert(maxrect->build_pointer[build_type] != NULL);
    Building* build = maxrect->build_pointer[build_type];
    Relation* room_rel = build->GetRoom_Rel();
    IndoorNav* indoor_nav = new IndoorNav(room_rel, NULL);
    unsigned int num_elev = indoor_nav->NumerOfElevators();

    IndoorGraph* ig = maxrect->igraph_pointer[build_type];
    BTree* btree_room = build->GetBTree();
    R_Tree<3, TupleId>* rtree_room = build->GetRTree();

    //////////////indoor bounding box///////////
    Rectangle<3> build_box = rtree_room->BoundingBox();
    double min[2], max[2];
    min[0] = build_box.MinD(0);
    min[1] = build_box.MinD(1);
    max[0] = build_box.MaxD(0);
    max[1] = build_box.MaxD(1);
    Rectangle<2> build_box2(true, min, max);
//     cout<<"outdoor "<<build_rect<<" indoor1 "<<build_box
//         <<" indoor2 "<<build_box2<<endl;


    ////////////////indoor paths and rooms id list /////////////////
#ifdef INDOOR_PATH
    indoor_nav->indoor_paths_list = indoor_paths_list[build_type];
    indoor_nav->rooms_list = rooms_id_list[build_type];
#endif 

    indoor_nav->ig = ig;
    /////////////////////////////////////////////////

    MPoint3D* mp3d = new MPoint3D(0); 
    if(num_elev <= 1){
      Instant t1 = start_time;

      indoor_nav->GenerateMO3_Start(ig, btree_room, rtree_room, start_time,
                                build_id, entrance_index, mp3d, genmo, peri);
      Instant t2 = start_time;


      ////////////////////////////////////////////////////////////////////
      ///set up outdoor movement and stay in the room for a while ////////
      ///////////////////////////////////////////////////////////////////
      if(t_span > 0){

        Instant t3 = t2;
        Interval<Instant> up_interval; 
        up_interval.start = t1;
        up_interval.lc = true;
      ////////////// at least 5 minutes, use float, otherwise get zero/////////
        t3.ReadFrom(t3.ToDouble() + (GetRandom()%t_span + 5)/(24*60.0));
        up_interval.end = t3;
        up_interval.rc = false; 
//        UPoint* up = new UPoint(up_interval, loc, loc);
//        mo->Add(*up);
//        delete up; 

        UPoint3D unit1;
        mp3d->Get(mp3d->GetNoComponents() - 1, unit1);
        up_interval.start = t2;
        unit1.timeInterval = up_interval;
        mp3d->Add(unit1);

        UGenLoc unit2;
        genmo->Get(genmo->GetNoComponents() - 1, unit2);
        unit2.timeInterval = up_interval;
        genmo->Add(unit2);

        start_time = t3;/////add the waiting time 
      }

    }else{
//      cout<<"several elevators "<<endl;
      Instant t1 = start_time;

      indoor_nav->GenerateMO3_New_Start(ig, btree_room, rtree_room, start_time,
                          build_id, entrance_index,mp3d,genmo,peri,num_elev);

      Instant t2 = start_time;

      ////////////////////////////////////////////////////////////////////
      ///set up outdoor movement and stay in the room for a while ////////
      ////////////////////////////////////////////////////////////////////
      if(t_span > 0){
        Instant t3 = t2;
        Interval<Instant> up_interval;
        up_interval.start = t1;
        up_interval.lc = true;
        ////////////// at least 5 minutes, use float, otherwise get zero/////
        t3.ReadFrom(t3.ToDouble() + (GetRandom()%t_span + 5)/(24*60.0));
        up_interval.end = t3;
        up_interval.rc = false; 
//        UPoint* up = new UPoint(up_interval, loc, loc);
//        mo->Add(*up);
//        delete up; 

        UPoint3D unit1;
        mp3d->Get(mp3d->GetNoComponents() - 1, unit1);
        up_interval.start = t2;
        unit1.timeInterval = up_interval;
        mp3d->Add(unit1);

        UGenLoc unit2;
        genmo->Get(genmo->GetNoComponents() - 1, unit2);
        unit2.timeInterval = up_interval;
        genmo->Add(unit2);

        start_time = t3;//add the waiting time 

      }
    }

   //////////////////project indoor movement into free space/////////////////
    MapMP3DToMP(mo, mp3d, build_rect, build_box2);
    indoor_mo_list2.push_back(*mp3d);
    delete mp3d;

    delete indoor_nav;
  }

}

/*
map a mpoint3d moving object to a mpoint 

*/
void GenMObject::MapMP3DToMP(MPoint* mo, MPoint3D* mp3d, 
                             Rectangle<2> out_door, Rectangle<2> in_door)
{
//  cout<<"outdoor "<<out_door<<" indoor "<<in_door<<endl;

  for(int i = 0;i < mp3d->GetNoComponents();i++){
      UPoint3D unit;
      mp3d->Get(i, unit);
//      cout<<unit.p0<<" "<<unit.p1<<endl;
      Point p1(true, unit.p0.GetX(), unit.p0.GetY());
      Point p2 = MapMP3D(out_door, in_door, p1);
//      cout<<"in "<<p1<<" out"<<p2<<endl;
      Point p3(true, unit.p1.GetX(), unit.p1.GetY());
      Point p4 = MapMP3D(out_door, in_door, p3);

      UPoint* up = new UPoint(unit.timeInterval, p2, p4);
      mo->Add(*up);
      delete up; 
  }

}
/*
map a point3d to point 

*/
Point GenMObject::MapMP3D(Rectangle<2> out_door, Rectangle<2> in_door, Point in)
{

  double o_min_0 = out_door.MinD(0);
  double o_min_1 = out_door.MinD(1);

  double len_x_o = out_door.MaxD(0) - out_door.MinD(0);
  double len_y_o = out_door.MaxD(1) - out_door.MinD(1);

  double i_min_0 = in_door.MinD(0);
  double i_min_1 = in_door.MinD(1);

  double len_x_i = in_door.MaxD(0) - in_door.MinD(0);
  double len_y_i = in_door.MaxD(1) - in_door.MinD(1);

  Point res(true);

  double delta_x = in.GetX() - i_min_0;
  double delta_y = in.GetY() - i_min_1;
  
  double x, y;
  if(fabs(delta_x) < EPSDIST) x = o_min_0;
  else{
    x = (delta_x/len_x_i)*len_x_o + o_min_0;
  }


  if(fabs(delta_y) < EPSDIST) y = o_min_1;
  else{
    y = (delta_y/len_y_i)*len_y_o + o_min_1;
  }

  res.Set(x,y);

  return res;
}


/*
create generic moving objects with modes: walk + indoor + car (taxi)

*/
void GenMObject::GenerateGenMO5(Space* sp, Periods* peri,
                                int mo_no, int type)
{
  if(mo_no < 1){
    cout<<" invalid number of moving objects "<<mo_no<<endl;
    return;
  }
  if(!(0 < type && type <= int(ARR_SIZE(genmo_tmlist)))){
    cout<<" invalid type value "<<type<<endl;
    return; 
  }

  switch(type){
    case 13:
      GenerateGenMO_IndoorWalkCTB(sp, peri, mo_no, "Car", 12);
      break;
    case 16:
      GenerateGenMO_IndoorWalkCTB(sp,peri, mo_no, "Taxi", 12);
      break;
    case 18:
      GenerateGenMO_IndoorWalkCTB(sp, peri, mo_no, "Bike", 12);
      break;

    default:
      assert(false);
      break;  
  }

}

/*
indoor + walk + car or taxi or bike, initialize buildings 

*/

void GenMObject::GenerateGenMO_IndoorWalkCTB(Space* sp, Periods* peri, 
                                             int mo_no, string mode, int para)
{

  IndoorInfra* i_infra = sp->LoadIndoorInfra(IF_GROOM);
  
  if(i_infra == NULL){
    cout<<"indoor infrastructure does not exist "<<endl;
    return;
  }

  //////////////////////////////////////////////////////////////////
  ////////////////load all buildings and indoor graphs/////////////
  ////////////////// and indoor paths//////////////////////////////
  MaxRect* maxrect = new MaxRect();
  maxrect->OpenBuilding();
  maxrect->OpenIndoorGraph();

#ifdef INDOOR_PATH
  maxrect->LoadIndoorPaths(indoor_paths_list, rooms_id_list);
  assert(indoor_paths_list.size() == rooms_id_list.size());
#endif 

  ////////////////////////////////////////////////////////////////
  ////////////select a pair of buildings/////////////////////////
  //////////////////////////////////////////////////////////////
  
  vector<RefBuild> build_id1_list;
  vector<RefBuild> build_id2_list;
  CreateBuildingPair2(i_infra, build_id1_list, build_id2_list, 
                     obj_scale*mo_no, maxrect);

  ///////////////////open infrastructures/////////////////////////////
  Pavement* pm = sp->LoadPavement(IF_REGION);
  if(pm == NULL){
    cout<<"pavement loading error"<<endl;
    return;
  }
  Network* rn = sp->LoadRoadNetwork(IF_LINE);
  if(rn == NULL){
    sp->ClosePavement(pm);
    cout<<"road network loading error"<<endl;
    return;
  }
  
  RoadGraph* rg = sp->LoadRoadGraph();
  if(rg == NULL){
    sp->CloseRoadNetwork(rn);
    sp->ClosePavement(pm);
    cout<<"road graph loading error"<<endl;
    return;
  }
  /////////////////////////////////////////////////////////////////
  GenerateGenMO_IWCTB(sp, maxrect, i_infra, pm, rn, rg, 
                      peri, mo_no, mode, 
                      build_id1_list, build_id2_list, para);

  ///////////////////close infrastructures////////////////////////
  sp->CloseRoadGraph(rg);
  sp->CloseRoadNetwork(rn);
  sp->ClosePavement(pm);
  maxrect->CloseIndoorGraph();
  maxrect->CloseBuilding();
  delete maxrect;

  sp->CloseIndoorInfra(i_infra);
}

/*
indoor + walk + car or taxi or bike, underlying implementation 

*/
void GenMObject::GenerateGenMO_IWCTB(Space* sp, MaxRect* maxrect,
                                     IndoorInfra* i_infra,
                                     Pavement* pm, Network* rn,
                                     RoadGraph* rg, Periods* peri, 
                                     int mo_no, string mode, 
                                     vector<RefBuild> build_id1_list,
                                     vector<RefBuild> build_id2_list, int para)
{
  ////////////////////////////////////////////////////////////////
  //////////////////////Initialization/////////////////////////////
  ////////////////////////////////////////////////////////////////
   Relation* dg_node_rel = sp->GetDualNodeRel();
   BTree* btree = sp->GetDGNodeBTree();
   Relation* speed_rel = sp->GetSpeedRel();

   if(dg_node_rel == NULL || btree == NULL || speed_rel == NULL){
      cout<<"GenerateGenMO_IWCTB():auxiliary relation empty IWCTB"<<endl;
      return;
   }

  RoadNav* road_nav = new RoadNav();

  ///////////////////////////////////////////////////////////////////
  Relation* build_path_rel = i_infra->BuildingPath_Rel();
  //////////////////////////////////////////////////////////////////
  ///////////////start time///////////////////////////////////////
   Interval<Instant> periods;
   peri->Get(0, periods);
   Instant start_time = periods.start;
   int time_range = para*60;//e.g., 12 hours in range 
  ////////////////////////////////////////////////////////////
   int count = 0;
   const double min_path = 0.1;
   int real_count = 1;

   while(real_count <= mo_no && count < obj_scale*mo_no &&
         count < (int)build_id1_list.size()){

   //////////////////////////////start time///////////////////////////
   if(count % 3 == 0)
     start_time.ReadFrom(periods.start.ToDouble() + 
                        (GetRandom() % time_range)/(24.0*60.0));
   else
     start_time.ReadFrom(periods.end.ToDouble() -
                        (GetRandom() % time_range)/(24.0*60.0));

    /////////////////////load all paths from this building////////////////
    vector<int> path_id_list1;
    i_infra->GetPathIDFromTypeID(build_id1_list[count].reg_id, path_id_list1);
//    cout<<"number of paths "<<path_id_list1.size()<<endl;

    vector<int> path_id_list2;
    i_infra->GetPathIDFromTypeID(build_id2_list[count].reg_id, path_id_list2);
//    cout<<"number of paths "<<path_id_list2.size()<<endl;
   ////////////////////////////////////////////////////////////////////
   //////////////if a building has several entrances///////////////////
   ///////////it randomly selects an entrance  ///////////////////////
   ////////////the buildings are far away from each///////////////////
   //////////which entrance to go out does not influence the distance a lot///
   ////////////////////////////////////////////////////////////////////
   if(path_id_list1.size() == 0 || path_id_list2.size() == 0){
     count++;
     continue;
   }
/*   cout<<"count "<<count<<" "<<build_id1_list.size()<<" "
       <<build_id1_list[count].type<<" "
       <<build_id2_list[count].type<<endl;*/

   int path_tid1 = path_id_list1[GetRandom() % path_id_list1.size()];
   int path_tid2 = path_id_list2[GetRandom() % path_id_list2.size()];

   Tuple* path_tuple1 = build_path_rel->GetTuple(path_tid1, false);
   Tuple* path_tuple2 = build_path_rel->GetTuple(path_tid2, false);

    /////////////////////////////////////////////////////////////////////
    //////////////////1 get buildings and two positions for network /////
    ////////////////// the end point of path in pavement/////////////////
    ////////////////////////////////////////////////////////////////////

    GenLoc* gloc1 = 
        (GenLoc*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
    Point* ep1_1 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP);
    Point* ep1_2 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP2);


    GenLoc* gloc2 = 
        (GenLoc*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
    Point* ep2_1 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP);
    Point* ep2_2 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP2);


    /////////////reset the general location//////////////////////
    Loc loc1(ep1_2->GetX(), ep1_2->GetY());
    GenLoc newgloc1(gloc1->GetOid(), loc1);
    Loc loc2(ep2_2->GetX(), ep2_2->GetY());
    GenLoc newgloc2(gloc2->GetOid(), loc2);
    
    /////////////////////////////////////////////////////////////////////
    //////////////////2 map two positions to gpoints////////////////////
    /////////////////get the start and end location of the trip/////////
    ////////////////////////////////////////////////////////////////////
    vector<GPoint> gpoint_list;
    vector<Point> p_list;
    bool correct;
    PaveLoc2GPoint(newgloc1, newgloc2, sp, dg_node_rel,
                   btree, gpoint_list, p_list, correct, rn);

    if(correct == false){
        count++;
        path_tuple1->DeleteIfAllowed();
        path_tuple2->DeleteIfAllowed();
        continue;
    }

    GPoint gp1 = gpoint_list[0];
    GPoint gp2 = gpoint_list[1];
    Point start_loc = p_list[0];
    Point end_loc = p_list[1];

//    cout<<"ep1 "<<*ep1_2<<" ep2 "<<*ep2_2<<endl;
//    cout<<"gp1 "<<start_loc<<" gp2 "<<end_loc<<endl;
//    loc_list1.push_back(start_loc);
//    loc_list2.push_back(end_loc);

    MPoint* mo = new MPoint(0);
    GenMO* genmo = new GenMO(0);
    mo->StartBulkLoad();
    genmo->StartBulkLoad();
    /////////////////////////////////////////////////////////////////////
    ///////////////3 indoor movement1 + from entrance to pavement//////
    ////////////////////////////////////////////////////////////////////

    //////////////////////indoor movement///////////////////////////////////
    ////////////////////to show which entrance it is////////////////////////
    int entrance_index1 = ((CcInt*)path_tuple1->GetAttribute(IndoorInfra::
                           INDOORIF_SP_INDEX))->GetIntval();
    int reg_id1 =  ((CcInt*)path_tuple1->GetAttribute(IndoorInfra::
                                               INDOORIF_REG_ID))->GetIntval();
     Point* sp1 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_SP);
     GenerateIndoorMovementToExit(i_infra, genmo, mo, start_time, *sp1,
                                  entrance_index1, reg_id1, maxrect, peri);


    ///////////////   outdoor  movement //////////////////////////////
    Line* path1 = (Line*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_PATH);
//    line_list1.push_back(*path1);
    if(path1->Length() > min_path)
      GenerateFreeMovement(path1, *sp1, genmo, mo, start_time);

    GenerateFreeMovement2(*ep1_1, *ep1_2, genmo, mo, start_time);

    /////////////////////////////////////////////////////////////////////
    //////////////////4 pavement to network (start location)/////////////
    ////////////////////////////////////////////////////////////////////
    ConnectStartMove(newgloc1, start_loc, mo, genmo, start_time, pm);

    /////////////////////////////////////////////////////////////////////
    //////////////////5 road network movement //////////////////////////
    ////////////////////////////////////////////////////////////////////
    GLine* gl = new GLine(0);
    road_nav->ShortestPathSub(&gp1, &gp2, rg, rn, gl);
    ConnectGP1GP2(rn, start_loc, gl, mo, genmo, start_time, speed_rel ,mode);
    delete gl;
    /////////////////////////////////////////////////////////////////////
    //////////////////6 end network location to pavement//////////////////
    ////////////////////////////////////////////////////////////////////
    ConnectEndMove(end_loc, newgloc2, mo, genmo, start_time, pm);

    /////////////////////////////////////////////////////////////////////
    ////////7 pavement to building entrance + indoor movement2//////////
    ////////////////////////////////////////////////////////////////////

    ////////////////outdoor movement/////////////////////////////////////////
    GenerateFreeMovement2(*ep2_2, *ep2_1, genmo, mo, start_time);

//    cout<<"length "<<path2->Length()<<endl;
    Line* path2 = (Line*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_PATH);
//    line_list2.push_back(*path2);
    if(path2->Length() > min_path)
      GenerateFreeMovement(path2, *ep2_1, genmo, mo, start_time);

    ///////////////////indoor movement//////////////////////////////////////
    ////////////////////to show which entrance it is////////////////////
    Point* sp2 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_SP);
    int entrance_index2 = ((CcInt*)path_tuple2->GetAttribute(IndoorInfra::
                                   INDOORIF_SP_INDEX))->GetIntval();
    int reg_id2 =  ((CcInt*)path_tuple2->GetAttribute(IndoorInfra::
                                              INDOORIF_REG_ID))->GetIntval();

    if(WorkBuilding(build_id2_list[count].type))
      GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                   entrance_index2, reg_id2, 
                                      maxrect, peri, TIMESPAN_W1);
    else
      GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                   entrance_index2, reg_id2, maxrect, peri,
                                       TIMESPAN_NO);
   /////////////////////////////////////////////////////////////////////////

    mo->EndBulkLoad();
    genmo->EndBulkLoad(false, false);

    trip1_list.push_back(*genmo);
    trip2_list.push_back(*mo);

    ///////////////////////store building type//////////////////////////////
//    build_type_list1.push_back(build_id1_list[count].type); 
//    build_type_list2.push_back(build_id2_list[count].type); 

    delete mo; 
    delete genmo; 


    path_tuple1->DeleteIfAllowed();
    path_tuple2->DeleteIfAllowed();


   //////////////////////////////////////////////////////////////////////
//   cout<<"building 1 "<<GetBuildingStr(build_id1_list[count].type)
//        <<" building 2 "<<GetBuildingStr(build_id2_list[count].type)<<endl;

    count++;
    cout<<real_count<<" generic moving object"<<endl;
    real_count++;

  }

  delete road_nav;

}

/*
select a pair of buildings. 
the buildings should be far away from each other
[the two cannot be both personal houses]

*/
void GenMObject::CreateBuildingPair2(IndoorInfra* i_infra, 
                                    vector<RefBuild>& build_tid1_list, 
                                    vector<RefBuild>& build_tid2_list, int no,
                                    MaxRect* maxrect)
{
  const double min_dist = 1000.0;//Euclidean distance
  Relation* build_type_rel = i_infra->BuildingType_Rel();
  int count = 0;
  while(count < no){
    int id1 = GetRandom() % build_type_rel->GetNoTuples() + 1; 
    int id2 = GetRandom() % build_type_rel->GetNoTuples() + 1; 
    if(id1 == id2) continue;
    
    Tuple* tuple1 = build_type_rel->GetTuple(id1, false);
    Tuple* tuple2 = build_type_rel->GetTuple(id2, false);

    Rectangle<2>* bbox1 = 
        (Rectangle<2>*)tuple1->GetAttribute(IndoorInfra::INDOORIF_GEODATA);
    Rectangle<2>* bbox2 = 
        (Rectangle<2>*)tuple2->GetAttribute(IndoorInfra::INDOORIF_GEODATA);
    if(bbox1->Distance(*bbox2) < min_dist){
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed();
      continue;
    }

    int type1 = ((CcInt*)tuple1->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_TYPE))->GetIntval();

    int type2 = ((CcInt*)tuple2->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_TYPE))->GetIntval();


    ////////////do not select two personal houses////////////
//     if(type1 == BUILD_HOUSE && type2 == BUILD_HOUSE){
//       tuple1->DeleteIfAllowed();
//       tuple2->DeleteIfAllowed();
//       continue;
//     }


//    cout<<GetBuildingStr(type1)<<" "<<GetBuildingStr(type2)<<endl;

    //////////////////check whether the building is available/////////////////
    ///////////////////there is no building and indoor graph ///////////////
    ///////////////////for personal houses/////////////////////////////////
    if(type1 > BUILD_HOUSE && maxrect->build_pointer[type1] == NULL){
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed();
//      cout<<"type1 not valid "<<endl;
      continue;
    }
    if(type2 > BUILD_HOUSE && maxrect->build_pointer[type2] == NULL){
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed(); 
//      cout<<"type2 not valid "<<endl;
      continue;
    }

    int reg_id1 = ((CcInt*)tuple1->GetAttribute(IndoorInfra::
                INDOORIF_REG_ID))->GetIntval();

    int reg_id2 = ((CcInt*)tuple2->GetAttribute(IndoorInfra::
                INDOORIF_REG_ID))->GetIntval();

    int build_id1 = ((CcInt*)tuple1->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_ID))->GetIntval();

    int build_id2 = ((CcInt*)tuple2->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_ID))->GetIntval();

    RefBuild ref_b1(true, reg_id1, build_id1, type1, *bbox1, id1 );
    RefBuild ref_b2(true, reg_id2, build_id2, type2, *bbox2, id2 );
    ////////////////////////////////////////////////////////////////////////
    build_tid1_list.push_back(ref_b1);
    build_tid2_list.push_back(ref_b2);

    count++;

//    rect_list1.push_back(*bbox1);
//    rect_list2.push_back(*bbox2);

    tuple1->DeleteIfAllowed();
    tuple2->DeleteIfAllowed();

  }
}


/*
select a pair of buildings. 
the buildings should be far away from each other
include personal houses

*/
void GenMObject::CreateBuildingPair3(IndoorInfra* i_infra, 
                                    vector<RefBuild>& build_tid1_list, 
                                    vector<RefBuild>& build_tid2_list, int no,
                                    MaxRect* maxrect)
{
  const double min_dist = 1000.0;//Euclidean distance
  Relation* build_type_rel = i_infra->BuildingType_Rel();
  int count = 0;
  while(count < no){
    int id1 = GetRandom() % build_type_rel->GetNoTuples() + 1; 
    int id2 = GetRandom() % build_type_rel->GetNoTuples() + 1; 
    if(id1 == id2) continue;
    
    Tuple* tuple1 = build_type_rel->GetTuple(id1, false);
    Tuple* tuple2 = build_type_rel->GetTuple(id2, false);

    Rectangle<2>* bbox1 = 
        (Rectangle<2>*)tuple1->GetAttribute(IndoorInfra::INDOORIF_GEODATA);
    Rectangle<2>* bbox2 = 
        (Rectangle<2>*)tuple2->GetAttribute(IndoorInfra::INDOORIF_GEODATA);
    if(bbox1->Distance(*bbox2) < min_dist){
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed();
      continue;
    }

    int type1 = ((CcInt*)tuple1->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_TYPE))->GetIntval();

    int type2 = ((CcInt*)tuple2->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_TYPE))->GetIntval();


//    cout<<GetBuildingStr(type1)<<" "<<GetBuildingStr(type2)<<endl;

    //////////////////check whether the building is available/////////////////
    ///////////////////there is no building and indoor graph ///////////////
    ///////////////////for personal houses/////////////////////////////////
    if(type1 > BUILD_HOUSE && maxrect->build_pointer[type1] == NULL){
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed();
//      cout<<"type1 not valid "<<endl;
      continue;
    }
    if(type2 > BUILD_HOUSE && maxrect->build_pointer[type2] == NULL){
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed(); 
//      cout<<"type2 not valid "<<endl;
      continue;
    }

    int reg_id1 = ((CcInt*)tuple1->GetAttribute(IndoorInfra::
                INDOORIF_REG_ID))->GetIntval();

    int reg_id2 = ((CcInt*)tuple2->GetAttribute(IndoorInfra::
                INDOORIF_REG_ID))->GetIntval();

    ////////////////////////////////////////////////////////
    vector<int> path_id_list1;
    i_infra->GetPathIDFromTypeID(reg_id1, path_id_list1);
    if(path_id_list1.size() == 0){ //no path available, not such a building
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed(); 
      continue;
    }
    
    vector<int> path_id_list2;
    i_infra->GetPathIDFromTypeID(reg_id2, path_id_list2);
    if(path_id_list2.size() == 0){
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed(); 
      continue;
    }

//    cout<<"reg_id1 "<<reg_id1<<" path size "<<path_id_list1.size()<<endl;
//    cout<<"reg_id2 "<<reg_id2<<" path size "<<path_id_list2.size()<<endl;

    ///////////////////////////////////////////////////////

    int build_id1 = ((CcInt*)tuple1->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_ID))->GetIntval();

    int build_id2 = ((CcInt*)tuple2->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_ID))->GetIntval();

    RefBuild ref_b1(true, reg_id1, build_id1, type1, *bbox1, id1 );
    RefBuild ref_b2(true, reg_id2, build_id2, type2, *bbox2, id2 );
    ////////////////////////////////////////////////////////////////////////
    build_tid1_list.push_back(ref_b1);
    build_tid2_list.push_back(ref_b2);

    count++;

//    rect_list1.push_back(*bbox1);
//    rect_list2.push_back(*bbox2);

    tuple1->DeleteIfAllowed();
    tuple2->DeleteIfAllowed();

  }
}

/*
select buildings from a given relation, these buildings are near to bus
and metro stops 

*/
void GenMObject::CreateBuildingPair4(IndoorInfra* i_infra, 
                                    vector<RefBuild>& build_tid1_list, 
                                    vector<RefBuild>& build_tid2_list, int no,
                                    MaxRect* maxrect, Relation* build_rel)
{
  const double min_dist = 1000.0;//Euclidean distance
  Relation* build_type_rel = i_infra->BuildingType_Rel();
  int count = 0;
  while(count < no){
    
    int build_id_1 = GetRandom() % build_rel->GetNoTuples() + 1; 
    int build_id_2 = GetRandom() % build_rel->GetNoTuples() + 1; 
    
    Tuple* build_tuple1 = build_rel->GetTuple(build_id_1, false);
    Tuple* build_tuple2 = build_rel->GetTuple(build_id_2, false);

    int id1 = ((CcInt*)build_tuple1->GetAttribute(Build_ID))->GetIntval();
    int id2 = ((CcInt*)build_tuple2->GetAttribute(Build_ID))->GetIntval();
    assert(1 <= id1 && id1 <= build_type_rel->GetNoTuples());
    assert(1 <= id2 && id2 <= build_type_rel->GetNoTuples());

    build_tuple1->DeleteIfAllowed();
    build_tuple2->DeleteIfAllowed();

    if(id1 == id2) continue;
    
    Tuple* tuple1 = build_type_rel->GetTuple(id1, false);
    Tuple* tuple2 = build_type_rel->GetTuple(id2, false);

    Rectangle<2>* bbox1 = 
        (Rectangle<2>*)tuple1->GetAttribute(IndoorInfra::INDOORIF_GEODATA);
    Rectangle<2>* bbox2 = 
        (Rectangle<2>*)tuple2->GetAttribute(IndoorInfra::INDOORIF_GEODATA);
    if(bbox1->Distance(*bbox2) < min_dist){
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed();
      continue;
    }

    int type1 = ((CcInt*)tuple1->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_TYPE))->GetIntval();

    int type2 = ((CcInt*)tuple2->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_TYPE))->GetIntval();


//    cout<<GetBuildingStr(type1)<<" "<<GetBuildingStr(type2)<<endl;

    //////////////////check whether the building is available/////////////////
    ///////////////////there is no building and indoor graph ///////////////
    ///////////////////for personal houses/////////////////////////////////
    if(type1 > BUILD_HOUSE && maxrect->build_pointer[type1] == NULL){
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed();
//      cout<<"type1 not valid "<<endl;
      continue;
    }
    if(type2 > BUILD_HOUSE && maxrect->build_pointer[type2] == NULL){
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed(); 
//      cout<<"type2 not valid "<<endl;
      continue;
    }

    int reg_id1 = ((CcInt*)tuple1->GetAttribute(IndoorInfra::
                INDOORIF_REG_ID))->GetIntval();

    int reg_id2 = ((CcInt*)tuple2->GetAttribute(IndoorInfra::
                INDOORIF_REG_ID))->GetIntval();

    ////////////////////////////////////////////////////////
    vector<int> path_id_list1;
    i_infra->GetPathIDFromTypeID(reg_id1, path_id_list1);
    if(path_id_list1.size() == 0){ //no path available, not such a building
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed(); 
      continue;
    }
    
    vector<int> path_id_list2;
    i_infra->GetPathIDFromTypeID(reg_id2, path_id_list2);
    if(path_id_list2.size() == 0){
      tuple1->DeleteIfAllowed();
      tuple2->DeleteIfAllowed(); 
      continue;
    }

//    cout<<"reg_id1 "<<reg_id1<<" path size "<<path_id_list1.size()<<endl;
//    cout<<"reg_id2 "<<reg_id2<<" path size "<<path_id_list2.size()<<endl;

    ///////////////////////////////////////////////////////

    int build_id1 = ((CcInt*)tuple1->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_ID))->GetIntval();

    int build_id2 = ((CcInt*)tuple2->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_ID))->GetIntval();

    RefBuild ref_b1(true, reg_id1, build_id1, type1, *bbox1, id1 );
    RefBuild ref_b2(true, reg_id2, build_id2, type2, *bbox2, id2 );
    ////////////////////////////////////////////////////////////////////////
    build_tid1_list.push_back(ref_b1);
    build_tid2_list.push_back(ref_b2);

    count++;

//    rect_list1.push_back(*bbox1);
//    rect_list2.push_back(*bbox2);

    tuple1->DeleteIfAllowed();
    tuple2->DeleteIfAllowed();

  }
}

/*
generic moving objects with modes: indoor walk bus with building relation

*/

void GenMObject::GenerateGenMO6(Space* sp, Periods* peri, int mo_no, 
                                int type, int para)
{

  if(mo_no < 1){
    cout<<" invalid number of moving objects "<<mo_no<<endl;
    return;
  }
  if(!(0 < type && type <= int(ARR_SIZE(genmo_tmlist)))){
    cout<<" invalid type value "<<type<<endl;
    return; 
  }

  IndoorInfra* i_infra = sp->LoadIndoorInfra(IF_GROOM);
  if(i_infra == NULL){
    cout<<"indoor infrastructure does not exist "<<endl;
    return;
  }
  
  ////////////////////////////////////////////////////////////////
  //////////////////////Initialization/////////////////////////////
  ////////////////////////////////////////////////////////////////
  Pavement* pm = sp->LoadPavement(IF_REGION);
  DualGraph* dg = pm->GetDualGraph();
  VisualGraph* vg = pm->GetVisualGraph();
  BusNetwork* bn = sp->LoadBusNetwork(IF_BUSNETWORK);
  
  //////////////////////////////////////////////////////////////////
  ////////////////load all buildings and indoor graphs/////////////
  /////////////// and indoor paths////////////////////////////////
  MaxRect* maxrect = new MaxRect();
  maxrect->OpenBuilding();
  maxrect->OpenIndoorGraph();

#ifdef INDOOR_PATH
  maxrect->LoadIndoorPaths(indoor_paths_list, rooms_id_list);
  assert(indoor_paths_list.size() == rooms_id_list.size());
#endif

  ////////////////////////////////////////////////////////////////
  ////////////select a pair of buildings/////////////////////////
  //////////////////////////////////////////////////////////////
  vector<RefBuild> build_id1_list;
  vector<RefBuild> build_id2_list;
  Relation* rel3 = sp->GetBSBuildRel();
  CreateBuildingPair4(i_infra, build_id1_list, build_id2_list, 
                     obj_scale*mo_no, maxrect, rel3);

  GenerateGenIBW(sp, maxrect, i_infra, pm, dg, vg, bn, peri, mo_no,
                 build_id1_list, build_id2_list, para); 


  maxrect->CloseIndoorGraph();
  maxrect->CloseBuilding();
  delete maxrect;

  ////////////////////////////////////////////////////////////////////////////
  sp->CloseBusNetwork(bn);
  pm->CloseDualGraph(dg);
  pm->CloseVisualGraph(vg);
  sp->ClosePavement(pm);

  sp->CloseIndoorInfra(i_infra);
}

/*
transportation modes with Indoor + Bus + Walk 

*/
void GenMObject::GenerateGenIBW(Space* sp, MaxRect* maxrect,
                                IndoorInfra* i_infra,
                                Pavement* pm, DualGraph* dg,
                                VisualGraph* vg, BusNetwork* bn,
                                Periods* peri_input, int mo_no, 
                                vector<RefBuild> build_id1_list,
                                vector<RefBuild> build_id2_list, int para)
{


  Relation* rel1 = sp->GetNewTriRel();
  Relation* rel2 = sp->GetBSPaveRel();
  Relation* rel3 = sp->GetBSBuildRel();
  R_Tree<2,TupleId>* rtree = sp->GetBSPaveRtree();
  if(rel1 == NULL || rel2 == NULL || rtree == NULL || rel3 == NULL){
      cout<<"auxiliary relation empty IBW"<<endl;
      return;
   }

  ///////////////////////////////////////////////////////////////////
  Relation* build_path_rel = i_infra->BuildingPath_Rel();
  //////////////////////////////////////////////////////////////////
  ///////////////start time///////////////////////////////////////
   Interval<Instant> periods;
   peri_input->Get(0, periods);
   Instant start_time = periods.start;
   int time_range = para*60;
  ////////////////////////////////////////////////////////////
   unsigned int count = 0;
   int real_count = 1;

   int obj_no_rep = 0;
   int max_obj = 1;
   if(mo_no <= 500) max_obj = 2;
   else if(mo_no <= 1000) max_obj = 3;
   else if(mo_no <= 5000) max_obj = 4;
   else max_obj = 5;


   int index1 = -1;
   int index2 = -1;
   
   int time_and_type = 4;
   const double min_path = 0.1;
   ////////////////////////////////////////////////////
   //////////bus network time periods ////////////////
   //////////////////////////////////////////////////
   BusGraph* bg = bn->GetBusGraph(); 
   if(bg == NULL){
    cout<<"bus graph is invalid"<<endl; 
    return;
   }
   Instant bs_start(bn->GetStartTime());
   Instant bs_end(bn->GetEndTime());
   Instant bg_min(instanttype);
   bg_min.ReadFrom(bg->min_t);
   bn->CloseBusGraph(bg);

   while(real_count <= mo_no && count < build_id1_list.size()){

//    cout<<"count "<<count<<" real_count "<<real_count<<endl;

   //////////////////////////////start time///////////////////////////
   if(count % time_and_type == 0) //less movement on sunday 
     start_time.ReadFrom(periods.start.ToDouble() + 
                        (GetRandom() % time_range)/(24.0*60.0));
   else
     start_time.ReadFrom(periods.end.ToDouble() -
                        (GetRandom() % time_range)/(24.0*60.0));

    bool time_transform = false;
    int day_deviation = 0;
    Instant old_st(start_time);

    Periods* peri = new Periods(0);
    peri->StartBulkLoad();
    if(start_time < bs_start || start_time > bs_end){
        time_transform = true;

        if(start_time.GetWeekday() == 6){
          start_time.Set(bg_min.GetYear(), bg_min.GetMonth(), 
                         bg_min.GetGregDay(), 
                         start_time.GetHour(), start_time.GetMinute(), 
                         start_time.GetSecond(), start_time.GetMillisecond());

        }else{ //Monday-Saturday 
          start_time.Set(bg_min.GetYear(), bg_min.GetMonth(), 
                         bg_min.GetGregDay() + 1,
                         start_time.GetHour(), start_time.GetMinute(), 
                         start_time.GetSecond(), start_time.GetMillisecond());

        }
        day_deviation = old_st.GetDay() - start_time.GetDay();
//        cout<<" day deviation "<<day_deviation<<endl;

        Interval<Instant> time_span;
        time_span.start = periods.start;
        time_span.start.ReadFrom(periods.start.ToDouble() - day_deviation);
        time_span.lc = true;
        time_span.end = periods.end;
        time_span.end.ReadFrom(periods.end.ToDouble() - day_deviation);
        time_span.rc = false;
        peri->MergeAdd(time_span);
    }

    peri->EndBulkLoad();

    int DAY1 = start_time.GetDay();

    /////////////////////load all paths from this building////////////////
    if(obj_no_rep == 0){
      index1 = GetRandom() % build_id1_list.size();
      index2 = GetRandom() % build_id2_list.size();
    }

    vector<int> path_id_list1;
    i_infra->GetPathIDFromTypeID(build_id1_list[index1].reg_id, path_id_list1);

    vector<int> path_id_list2;
    i_infra->GetPathIDFromTypeID(build_id2_list[index2].reg_id, path_id_list2);


   ////////////////////////////////////////////////////////////////////
   //////////////if a building has several entrances///////////////////
   ///////////it randomly selects an entrance  ///////////////////////
   ////////////the buildings are far away from each///////////////////
   //////////which entrance to go out does not influence the distance a lot///
   ////////////////////////////////////////////////////////////////////
   if(path_id_list1.size() == 0 || path_id_list2.size() == 0){
      count++;
      delete peri;
      continue;
   }

    int path_tid1 = path_id_list1[GetRandom() % path_id_list1.size()];
    int path_tid2 = path_id_list2[GetRandom() % path_id_list2.size()];


    Tuple* path_tuple1 = build_path_rel->GetTuple(path_tid1, false);
    Tuple* path_tuple2 = build_path_rel->GetTuple(path_tid2, false);
    /////////////////////////////////////////////////////////////////////////
    ////////////////get start and end locaton of the path in pavement////////
    /////////////////////////////////////////////////////////////////////////

    GenLoc* build_gloc1 = 
        (GenLoc*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
    Point* ep1_1 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP);
    Point* ep1_2 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP2);


    GenLoc* build_gloc2 = 
        (GenLoc*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
    Point* ep2_1 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP);
    Point* ep2_2 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP2);

//    cout<<"ep1 "<<*ep1_1<<" ep2 "<<*ep2_2<<endl; 
    
    /////////////////////////////////////////////////////////////
    /////////////reset the general location//////////////////////
    Loc loc1(ep1_2->GetX(), ep1_2->GetY());
    GenLoc newgloc1(build_gloc1->GetOid(), loc1);
    Loc loc2(ep2_2->GetX(), ep2_2->GetY());
    GenLoc newgloc2(build_gloc2->GetOid(), loc2);

    //////////////////////////////////////////////////////////////////////////
    //////find closest bus stops to the two locations on the pavement/////////
    Bus_Stop bs1(true, 0, 0, true);
    vector<Point> ps_list1;//1 point on the pavement 2 point of bus stop
    GenLoc gloc1(0, Loc(0, 0));//bus stop on pavement by genloc 
    bool b1 = true;
    b1 = NearestBusStop(newgloc1,  rel2, rtree, bs1, ps_list1, gloc1, true);

    Bus_Stop bs2(true, 0, 0, true);
    vector<Point> ps_list2;//1. point on the pavement 2. point of bus stop
    GenLoc gloc2(0, Loc(0, 0));//bus stop on pavement by genloc
    bool b2 = true;
    b2 = NearestBusStop(newgloc2,  rel2, rtree, bs2, ps_list2, gloc2, false);

    if((b1 && b2) == false){
      path_tuple1->DeleteIfAllowed();
      path_tuple2->DeleteIfAllowed();
      count++;
      delete peri;
      continue;
    }

//    cout<<"b1 "<<b1<<" b2 "<<b2<<endl;
    Point start_p, end_p; 
    bn->GetBusStopGeoData(&bs1, &start_p);
    bn->GetBusStopGeoData(&bs2, &end_p);

    if(start_p.Distance(end_p) < min_path){
      path_tuple1->DeleteIfAllowed();
      path_tuple2->DeleteIfAllowed();
      count++;
      delete peri;
      continue;
    }

//    cout<<"bs1 "<<bs1<<" bs2 "<<bs2<<endl;

    MPoint* mo = new MPoint(0);
    GenMO* genmo = new GenMO(0);
    mo->StartBulkLoad();
    genmo->StartBulkLoad();

    /////////////////////////////////////////////////////////////////////
    ///////////////1. indoor movement 1 + pavement//////////////////////
    ////////////////////////////////////////////////////////////////////

    //////////////////////indoor movement///////////////////////////////
    ////////////////////to show which entrance it is////////////////////

    int entrance_index1 = ((CcInt*)path_tuple1->GetAttribute(IndoorInfra::
                           INDOORIF_SP_INDEX))->GetIntval();
    int reg_id1 =  ((CcInt*)path_tuple1->GetAttribute(IndoorInfra::
                                               INDOORIF_REG_ID))->GetIntval();
    Point* sp1 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_SP);

    MPoint3D* mp3d = new MPoint3D(0);
    
    if(build_id1_list[index1].type > 1){ //not personal apartments 
      if(time_transform)//new time periods after transform
        GenerateIndoorMovementToExit2(i_infra, genmo, mo, start_time, *sp1,
                                 entrance_index1, reg_id1, maxrect, peri, mp3d);
      else  //input time period is consistent with the system configuration 
        GenerateIndoorMovementToExit2(i_infra, genmo, mo, start_time, *sp1,
                          entrance_index1, reg_id1, maxrect, peri_input, mp3d);

    }else{//empty for personal apartments
      mp3d->StartBulkLoad();
      mp3d->EndBulkLoad();
    }

    Line* path1 = (Line*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_PATH);
    if(path1->Length() > min_path)
      GenerateFreeMovement(path1, *sp1, genmo, mo, start_time);
    
    GenerateFreeMovement2(*ep1_1, *ep1_2, genmo, mo, start_time);

    ///////////////////////////////////////////////////////////////////
    ////////////////2. path (walk + bus)///////////////////////////////
    ///////////////////////////////////////////////////////////////////

    ////////2.1 connect from pavement to start bus stop////////////////////
    Line* res_path = new Line(0);
    ConnectStartStop(dg, vg, rel1, newgloc1, ps_list1, gloc1.GetOid(),
                            genmo, mo, start_time, res_path);

//    cout<<"path1 length "<<res_path->Length()<<endl; 
    /////////////////////////////////////////////////////////////////
    //////2.2. get the path in bus network////////////////////////////
    /////////////////////////////////////////////////////////////////
    BNNav* bn_nav = new BNNav(bn);

     if(count % time_and_type != 0) 
         bn_nav->ShortestPath_Time2(&bs1, &bs2, &start_time);
     else bn_nav->ShortestPath_Transfer2(&bs1, &bs2, &start_time);

    if(bn_nav->path_list.size() == 0 || (DAY1 != start_time.GetDay())){
//        cout<<"two unreachable bus stops"<<endl;
        mo->EndBulkLoad();
        genmo->EndBulkLoad();

        delete mo;
        delete genmo;
        delete bn_nav;
        delete res_path;
        delete mp3d;
        delete peri;
        path_tuple1->DeleteIfAllowed();
        path_tuple2->DeleteIfAllowed();
        count++;
        continue;
    }

    //////////put the first part of movement inside a building/////////////////
    indoor_mo_list1.push_back(*mp3d);
    delete mp3d; 


//    line_list1.push_back(*path1);

//     cout<<"building 1 "<<GetBuildingStr(build_id1_list[index1].type)
//         <<" building 2 "<<GetBuildingStr(build_id2_list[index2].type)<<endl;
    ///////////////////////////////////////////////////////////////////////

        Instant temp_end(instanttype);
        temp_end.ReadFrom(start_time.ToDouble() + bn_nav->t_cost/86400.0);

        //not in the bus schedule 
        if(temp_end > bs_end || temp_end.GetDay() != DAY1){
            mo->EndBulkLoad(); 
            genmo->EndBulkLoad();

            delete mo;
            delete genmo;
            delete bn_nav;
            delete res_path;
            delete peri;
            path_tuple1->DeleteIfAllowed();
            path_tuple2->DeleteIfAllowed();
            count++;
            continue;
        }
      /////////////////////////////////////////////////////////////////////
    int last_walk_id = ConnectTwoBusStops(bn_nav, ps_list1[1], ps_list2[1],
                           genmo, mo, start_time, dg, res_path);

//    cout<<"path2 length "<<res_path->Length()<<endl;
    ///////////////////////////////////////////////////////////
    if(last_walk_id > 0){
          //////change the last bus stop/////////
          ///change  ps list2, gloc2.oid ///
          Bus_Stop cur_bs1(true, 0, 0, true);
          StringToBusStop(bn_nav->bs1_list[last_walk_id], 
                          cur_bs1);
          ChangeEndBusStop(bn, dg, cur_bs1, ps_list2, gloc2, rel2, rtree);
    }

    ////////////////////////////////////////////////////////////
    delete bn_nav;
    /////////////////2.3 connect from end bus stop to pavement///////////
    ConnectEndStop(dg, vg, rel1, newgloc2, ps_list2, gloc2.GetOid(),
                          genmo, mo, start_time, res_path);
//    cout<<"path3 length "<<res_path->Length()<<endl;

     ////////////////////////////////////////////////////////////////////

     ////////////////////////////////////////////////////////////////////
     /////////////3. pavement + indoor movement 2////////////////////////
     ////////////////////////////////////////////////////////////////////


     ////////////////outdoor movement/////////////////////////////////////////

//    line_list2.push_back(*path2);
    GenerateFreeMovement2(*ep2_2, *ep2_1, genmo, mo, start_time);
    Line* path2 = (Line*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_PATH);
    if(path2->Length() > min_path)
        GenerateFreeMovement(path2, *ep2_1, genmo, mo, start_time);

    ///////////////////indoor movement//////////////////////////////////////
    ////////////////////to show which entrance it is////////////////////
    Point* sp2 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_SP);
    int entrance_index2 = ((CcInt*)path_tuple2->GetAttribute(IndoorInfra::
                                   INDOORIF_SP_INDEX))->GetIntval();
    int reg_id2 =  ((CcInt*)path_tuple2->GetAttribute(IndoorInfra::
                                              INDOORIF_REG_ID))->GetIntval();

    if(time_transform){
      if(WorkBuilding(build_id2_list[index2].type))
        GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                        entrance_index2, reg_id2, maxrect, 
                                        peri, TIMESPAN_W1);
      else
        GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                  entrance_index2, reg_id2, maxrect, peri,
                                        TIMESPAN_NO);
    }else{
      if(WorkBuilding(build_id2_list[index2].type))
          GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2,
                               entrance_index2, reg_id2, maxrect, 
                                          peri_input, TIMESPAN_W1);
      else
          GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2,
                               entrance_index2, reg_id2, maxrect, peri_input,
                                          TIMESPAN_NO);
    }
    //////////////////////////////////////////////////////////////////////
     mo->EndBulkLoad();
     genmo->EndBulkLoad(false, false);
     
     //////////////////////////////////change the time ///////////////////////
     if(time_transform){
            MPoint* mo_tmp = new MPoint(0);
            GenMO* genmo_tmp = new GenMO(0);
            mo_tmp->StartBulkLoad();
            genmo_tmp->StartBulkLoad();
            for(int i = 0;i < mo->GetNoComponents();i++){
               UPoint unit1;
               mo->Get(i, unit1);

               Instant st = unit1.timeInterval.start;
               Instant et = unit1.timeInterval.end;
  
               st.ReadFrom(st.ToDouble() + day_deviation);
               et.ReadFrom(et.ToDouble() + day_deviation);

               Interval<Instant> up_interval;
               up_interval.start = st;
               up_interval.lc = true;
               up_interval.end = et;
               up_interval.rc = false; 

               UPoint* up = new UPoint(up_interval, unit1.p0, unit1.p1);
               mo_tmp->Add(*up);
               delete up;     
            }

            for(int i = 0;i < genmo->GetNoComponents(); i++){
                UGenLoc ugloc;
                genmo->Get(i, ugloc);

                Instant st = ugloc.timeInterval.start;
                Instant et = ugloc.timeInterval.end;
  
                st.ReadFrom(st.ToDouble() + day_deviation);
                et.ReadFrom(et.ToDouble() + day_deviation);

                Interval<Instant> up_interval;
                up_interval.start = st;
                up_interval.lc = true;
                up_interval.end = et;
                up_interval.rc = false; 

                UGenLoc* unit_new = new UGenLoc(up_interval, ugloc.gloc1,
                                    ugloc.gloc2, ugloc.tm);
                genmo_tmp->Add(*unit_new);
                delete unit_new;
            }

            mo_tmp->EndBulkLoad();
            genmo_tmp->EndBulkLoad(false, false);

            trip1_list.push_back(*genmo_tmp);
            trip2_list.push_back(*mo_tmp);

            delete genmo_tmp;
            delete mo_tmp;

     }else{

      trip1_list.push_back(*genmo);
      trip2_list.push_back(*mo);
     }
      ////////////////////////////////////////////////////////////////////

//    path_list.push_back(*res_path);

     delete res_path;
     delete genmo;
     delete mo;
     delete peri;

    ///////////////////////////////////////////////////////////////////////
    path_tuple1->DeleteIfAllowed();
    path_tuple2->DeleteIfAllowed();

    ///////////////////////store building type//////////////////////////////
//    build_type_list1.push_back(build_id1_list[index1].type);
//    build_type_list2.push_back(build_id2_list[index2].type); 


//     cout<<"building 1 "<<GetBuildingStr(build_id1_list[index1].type)
//         <<" building 2 "<<GetBuildingStr(build_id2_list[index2].type)<<endl;

    //////////////////////////////////////////////////////////////////
    ////////////reduce the time cost of generating moving object//////
    ////////////if two bus stops are reachable, it generates the second////
    //////// moving object, but different time intervals //////////////////
    ///////////////////////////////////////////////////////////////////////

    obj_no_rep++;
    if(obj_no_rep == max_obj) obj_no_rep = 0;
    ///////////////////////////////////////////////////////////////////////

    cout<<real_count<<" generic moving object "<<endl;
    real_count++;
    count++;

  }

}


/*
almost the same procedure as GenerateIndoorMovementToExit but takes in the 
mpoint3d. because in querying on bus route, if there is no such route, the 
indoor movement should not be put into the result

*/
void GenMObject::GenerateIndoorMovementToExit2(IndoorInfra* i_infra,
                                              GenMO* genmo, MPoint* mo, 
                                              Instant& start_time, Point loc,
                                              int entrance_index, 
                                              int reg_id, 
                                              MaxRect* maxrect, 
                                              Periods* peri, MPoint3D* mp3d)
{
  ///////////////////////////////////////////////////////////////////////////
  //////////load the building: with reg id , we know the building type///////
  ///////////////////////////////////////////////////////////////////////////
  int build_type;
  int build_id;
  Rectangle<2> build_rect(false);
  i_infra->GetTypeFromRegId(reg_id, build_type, build_id, build_rect);
//  cout<<"to exit buiding: "<<GetBuildingStr(build_type)<<endl;
  if(build_type == BUILD_HOUSE){///not necessary to load the indoor graph

/*    MPoint3D* mp3d = new MPoint3D(0); 
    mp3d->StartBulkLoad();
    indoor_mo_list1.push_back(*mp3d);
    mp3d->EndBulkLoad();
    delete mp3d;*/

  }else{
//    cout<<"load indoor graph (to exit)"<<endl;
    assert(maxrect->igraph_pointer[build_type] != NULL);
    assert(maxrect->build_pointer[build_type] != NULL);
    Building* build = maxrect->build_pointer[build_type];
    Relation* room_rel = build->GetRoom_Rel();
    IndoorNav* indoor_nav = new IndoorNav(room_rel, NULL);
    unsigned int num_elev = indoor_nav->NumerOfElevators();

    IndoorGraph* ig = maxrect->igraph_pointer[build_type];
    BTree* btree_room = build->GetBTree();
    R_Tree<3, TupleId>* rtree_room = build->GetRTree();
    //////////////indoor bounding box///////////
    Rectangle<3> build_box = rtree_room->BoundingBox();
    double min[2], max[2];
    min[0] = build_box.MinD(0);
    min[1] = build_box.MinD(1);
    max[0] = build_box.MaxD(0);
    max[1] = build_box.MaxD(1);
    Rectangle<2> build_box2(true, min, max);

#ifdef INDOOR_PATH
    ////////////////indoor paths and rooms id list ///////////////////
    indoor_nav->indoor_paths_list = indoor_paths_list[build_type];
    indoor_nav->rooms_list = rooms_id_list[build_type];
#endif 
    indoor_nav->ig = ig;

//    cout<<"number of elevators "<<num_elev<<endl;


    if(num_elev <= 1){
      Instant t1 = start_time;
//    cout<<"t1 "<<t1<<endl;
      indoor_nav->GenerateMO3_End(ig, btree_room, rtree_room, start_time,
                                build_id, entrance_index,mp3d,genmo,peri);
      Instant t2 = start_time;
//    cout<<"t2 "<<t2<<endl;
      ////////////////////////////////////////////////////////////////////
      ///////////////set up outdoor movement/////////////////////////////
      //////////////////////////////////////////////////////////////////
//       Interval<Instant> up_interval; 
//       up_interval.start = t1;
//       up_interval.lc = true;
//       up_interval.end = t2;
//       up_interval.rc = false; 
//       UPoint* up = new UPoint(up_interval, loc, loc);
//       mo->Add(*up);
//       delete up; 

    }else{ //several elevators

      Instant t1 = start_time;
//      cout<<"t1 "<<t1<<endl;
      indoor_nav->GenerateMO3_New_End(ig, btree_room, rtree_room, start_time,
                            build_id, entrance_index,mp3d,genmo,peri,num_elev);

      Instant t2 = start_time;
//      cout<<"t2 "<<t2<<endl;
      ////////////////////////////////////////////////////////////////////
      ///////////////set up outdoor movement/////////////////////////////
      //////////////////////////////////////////////////////////////////
/*      Interval<Instant> up_interval; 
      up_interval.start = t1;
      up_interval.lc = true;
      up_interval.end = t2;
      up_interval.rc = false; 
      UPoint* up = new UPoint(up_interval, loc, loc);
      mo->Add(*up);
      delete up; */

    }

    MapMP3DToMP(mo, mp3d, build_rect, build_box2);
    delete indoor_nav;
  }
}


/*
generic moving objects with modes: metro + walk
randomly select two locations on the pavement and their closest metro stops.
build the connection between them.

*/
void GenMObject::GenerateGenMO7(Space* sp, Periods* peri, int mo_no, 
                      int type, Relation* rel1, Relation* rel2, 
                                R_Tree<2,TupleId>* rtree)
{
    Pavement* pm = sp->LoadPavement(IF_REGION);
    MetroNetwork* mn = sp->LoadMetroNetwork(IF_METRONETWORK);
    vector<GenLoc> genloc_list;
    vector<Point> p_loc_list;
    //generate locations on pavements
    GenerateLocPave(pm, obj_scale*mo_no, genloc_list, p_loc_list);

    DualGraph* dg = pm->GetDualGraph();
    VisualGraph* vg = pm->GetVisualGraph();

    const double min_len = 5000.0;
    Interval<Instant> periods;
    peri->Get(0, periods);
    int count = 1;
    int time_and_type = 4;


    while(count <= mo_no){

      int index1 = GetRandom() % genloc_list.size();
      GenLoc loc1 = genloc_list[index1];
      int index2 = GetRandom() % genloc_list.size();
      GenLoc loc2 = genloc_list[index2];


      if(index1 == index2) continue;

//      cout<<"genloc1 "<<loc1<<" genloc2 "<<loc2<<endl; 

      Point pave_loc1(true, loc1.GetLoc().loc1, loc1.GetLoc().loc2);
      Point pave_loc2(true, loc2.GetLoc().loc1, loc2.GetLoc().loc2);

      if(pave_loc1.Distance(pave_loc2) < min_len) continue; 


      ////////////////////////////////////////////////////////////
      ///////////// initialization////////////////////////////////
      ////////////////////////////////////////////////////////////
        MPoint* mo = new MPoint(0);
        GenMO* genmo = new GenMO(0);
        mo->StartBulkLoad();
        genmo->StartBulkLoad();

        //////////////////////////////////////////////////////////////
        ///////////////set start time/////////////////////////////////
        /////////////////////////////////////////////////////////////
        Instant start_time = periods.start;
        int time_range = 14*60;//14 hours 
        bool time_type;
        if(count % time_and_type == 0) time_type = true;
        else time_type = false;

        if(time_type)//////less trips on Sunday 
            start_time.ReadFrom(periods.start.ToDouble() + 
                        (GetRandom() % time_range)/(24.0*60.0));
        else{  //////20:00 before 
            start_time.ReadFrom(periods.end.ToDouble() - 2.0/24.0 -
                        (GetRandom() % time_range)/(24.0*60.0));
        }

      ////////////////////////////////////////////////////////////////////
      ///1. find closest metro stops to the two points on the pavement///
      ///////////////////////////////////////////////////////////////////
 
      Bus_Stop ms1(true, 0, 0, true);
      vector<Point> ps_list1;//1 point on the pavement 2 point of metro stop
      GenLoc gloc1(0, Loc(0, 0));//metro stop on pavement by genloc 
      bool b1 = true;
      b1 = NearestMetroStop(loc1,  rel2, rtree, ms1, ps_list1, gloc1);


      Bus_Stop ms2(true, 0, 0, true);
      vector<Point> ps_list2;//1 point on the pavement 2 point of metro stop
      GenLoc gloc2(0, Loc(0, 0));//metro stop on pavement by genloc
      bool b2 = true;
      b2 = NearestMetroStop(loc2,  rel2, rtree, ms2, ps_list2, gloc2);

      if((b1 && b2) == false) continue;


//     loc_list2.push_back(ps_list2[0]);//mapping point on the pavement of stop
//     loc_list3.push_back(ps_list2[1]);// point of metro stop 


       /////////////////////////////////////////////////////////////////
       /////////2 connect start location to start metro stop/////////////
       ////////////////////////////////////////////////////////////////
       Line* res_path = new Line(0);

       Instant last_t = start_time;
       ConnectStartStop(dg, vg, rel1, loc1, ps_list1, gloc1.GetOid(),
                            genmo, mo, start_time, res_path);

//       cout<<"last_t "<<last_t<<" cur "<<start_time<<endl;

       double delta_time = start_time.ToDouble() - last_t.ToDouble();
       double delta_minute = delta_time*86400.0/60.0;

       if(delta_minute > 60.0){ // more than one hour 
/*          cout<<" loc1 "<<pave_loc1<<" ms1 "<<ms1<<endl;
          cout<<" loc2 "<<pave_loc2<<" ms2 "<<ms2<<endl;
          cout<<"more than one hour"<<endl;*/
          delete res_path;
          continue;
       }

        /////////////////////////////////////////////////////////////////
        //////3. get the path in metro network///////////////////////////
        /////////////////////////////////////////////////////////////////

//        cout<<"start "<<ms1<<" end "<<ms2<<" time "<<start_time<<endl;

        MNNav* mn_nav = new MNNav(mn);
        mn_nav->ShortestPath_Time(&ms1, &ms2, &start_time);

        if(mn_nav->path_list.size() == 0){
//          cout<<"two unreachable metro stops"<<endl;
          mo->EndBulkLoad();
          genmo->EndBulkLoad();
          delete mo;
          delete genmo;
          delete mn_nav;
          delete res_path;
          continue;
        }

//        cout<<"metro path size "<<mn_nav->path_list.size()<<endl;

         ConnectTwoMetroStops(mn_nav, ps_list1[1], 
                                                ps_list2[1],
                           genmo, mo, start_time, dg, res_path);

        delete mn_nav;


        ///////////////////////////////////////////////////////////////////
        /////////////4 connect end location to last metro stop//////////////
        //////////////////////////////////////////////////////////////////
        ConnectEndStop(dg, vg, rel1, loc2, ps_list2, gloc2.GetOid(),
                          genmo, mo, start_time, res_path);
        ///////////////////////////////////////////////////////////////////////

        mo->EndBulkLoad();
        genmo->EndBulkLoad(false, false);

        trip1_list.push_back(*genmo);
        trip2_list.push_back(*mo);

//        line_list1.push_back(*res_path);

        delete mo;
        delete genmo;
        delete res_path;

        cout<<count<<" generic moving object "<<endl;

        count++;
    }


    pm->CloseDualGraph(dg);
    pm->CloseVisualGraph(vg);

    sp->CloseMetroNetwork(mn);
    sp->ClosePavement(pm);
}

/*
for a location on the pavement, find it closest metro stop 

*/
bool GenMObject::NearestMetroStop(GenLoc loc, Relation* rel,
                      R_Tree<2,TupleId>* rtree, Bus_Stop& res,
                      vector<Point>& ps_list, GenLoc& ms_gloc)
{

  Point p(true, loc.GetLoc().loc1, loc.GetLoc().loc2);
  SmiRecordId adr = rtree->RootRecordId();

  vector<int> tid_list;
  DFTraverse2(rtree, adr, rel, p, tid_list);

//  cout<<p<<" "<<tid_list.size()<<endl;
  
  if(tid_list.size() == 0) return false;

  vector<MyPoint_Tid> mp_tid_list;
  for(unsigned int i = 0;i < tid_list.size();i++){
    Tuple* tuple = rel->GetTuple(tid_list[i], false);
    Point* q = (Point*)tuple->GetAttribute(MetroNetwork::METRO_PAVE_LOC2);
    MyPoint_Tid mp_tid(*q, q->Distance(p), tid_list[i]);
    mp_tid_list.push_back(mp_tid);
    tuple->DeleteIfAllowed();
  }
  sort(mp_tid_list.begin(), mp_tid_list.end());
  
  //  for(unsigned int i = 0;i < mp_tid_list.size();i++)
//    mp_tid_list[i].Print();
  Tuple* ms_pave = rel->GetTuple(mp_tid_list[0].tid, false);
  Bus_Stop* ms_stop = 
      (Bus_Stop*)ms_pave->GetAttribute(MetroNetwork::METRO_PAVE_MS_STOP);
  Point* ms_loc = 
      (Point*)ms_pave->GetAttribute(MetroNetwork::METRO_PAVE_MS_STOP_LOC);
  GenLoc* ms_loc2 = 
      (GenLoc*)ms_pave->GetAttribute(MetroNetwork::METRO_PAVE_LOC1);

  res = *ms_stop;
  ps_list.push_back(mp_tid_list[0].loc);
  ps_list.push_back(*ms_loc);
  ms_gloc = *ms_loc2;
  ms_pave->DeleteIfAllowed();

  //////////////////////////////////////////////////////////////////
  /////////////////// debuging /////////////////////////////////////
  //////// output the metro stop and the location on the pavement////
  
//   loc_list2.push_back(ps_list[0]);
//   loc_list3.push_back(ps_list[1]);
  
  return true;

}

/*
find all points that their distance to query loc is smaller than the 
defined max dist

*/

void GenMObject::DFTraverse2(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                             Relation* rel,
                             Point query_loc, vector<int>& tid_list)
{
//  const double max_dist = 1500.0;
  const double max_dist = NEARMETROSTOP;
  
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple = rel->GetTuple(e.info, false);
              Point* loc = 
                  (Point*)dg_tuple->GetAttribute(MetroNetwork::METRO_PAVE_LOC2);

              if(loc->Distance(query_loc) < max_dist){
                tid_list.push_back(e.info);
              }
              dg_tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(query_loc.Distance(e.box) < max_dist){
                DFTraverse2(rtree, e.pointer, rel, query_loc, tid_list);
            }
      }
  }
  delete node;
}

/*
create the moving object between two metro stops in metro network

*/
void GenMObject::ConnectTwoMetroStops(MNNav* mn_nav, Point sp, Point ep, 
                                     GenMO* genmo_input,
                          MPoint* mo, Instant& start_time,
                          DualGraph* dg, Line* res_path)
{
    GenMO* genmo = new GenMO(0);
    genmo->StartBulkLoad();

    const double delta_t = 0.01;//seconds

    Line* l = new Line(0); ////////trajectory in metro network
    l->StartBulkLoad();
    int edgeno = 0;

    ////////////////////////////////////////////////////////////////////

    Bus_Stop last_ms(false, 0, 0, true);
    MPoint mo_metro(0);
    int mo_metro_index = -1;
    int mometro_oid = 0;

    for(unsigned int i = 0;i < mn_nav->path_list.size();i++){

        SimpleLine* sl = &(mn_nav->path_list[i]);
        ////////////time cost: second///////////////////
        Bus_Stop ms1(true, 0, 0, true);
        StringToBusStop(mn_nav->ms1_list[i], ms1);

        Bus_Stop ms2(true, 0, 0, true);
        StringToBusStop(mn_nav->ms2_list[i], ms2);

        double t = mn_nav->time_cost_list[i];
//        int tm = GetTM(mn_nav->tm_list[i]);

//         cout<<"ms1 "<<ms1<<" ms2 "<<ms2<<endl;
//         cout<<"i "<<i<<" time cost "<<t<<" path size "<<sl->Size()<<endl;

        /////filter the first part and transfer without movement ////////
        if(sl->Size() == 0 && AlmostEqual(t, 0.0)) continue;

        /////////////////////////////////////////////////////////////
        Point* start_loc = new Point(true, 0, 0);
        mn_nav->mn->GetMetroStopGeoData(&ms1, start_loc);
        Point* end_loc = new Point(true, 0, 0);
        mn_nav->mn->GetMetroStopGeoData(&ms2, end_loc);

        Instant temp_start_time = start_time;


        assert(ms1.GetId() == ms2.GetId() && ms1.GetUp() == ms2.GetUp());

        if(sl->Size() == 0){////////////no movement in space

            Instant st = start_time;
            Instant et = start_time; 
            Interval<Instant> up_interval; 
            et.ReadFrom(st.ToDouble() + t*1.0/(24.0*60.0*60.0));
            up_interval.start = st;
            up_interval.lc = true;
            up_interval.end = et;
            up_interval.rc = false; 

            if(fabs(t - 30.0) < delta_t){///metro waiting at the metro stop
                  ////////generic moving objects/////////////////
                  ////////reference to the metro///////////////////
                  /////with bs find br, with br.uoid find mobus///
                  //////////// mode = metro////////////////////////
                  ///////////////////////////////////////////////
/*                int metro_oid = 
                    mn_nav->mn->GetMOMetro_Oid(&ms1, start_loc, start_time);*/
                int metro_oid = mometro_oid;
                if(metro_oid == 0){
                    metro_oid = 
                      mn_nav->mn->GetMOMetro_Oid(&ms1, start_loc, start_time);
                }

                 Loc loc1(UNDEFVAL, UNDEFVAL);
                 Loc loc2(UNDEFVAL, UNDEFVAL);

                GenLoc gloc1(metro_oid, loc1);
                GenLoc gloc2(metro_oid, loc2);
                int tm = GetTM("Metro");
                UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
                genmo->Add(*unit); 
                delete unit; 

//                cout<<"metro_oid "<<metro_oid<<endl;

            }else{/////////waiting for transfer//////////////////////
                /////////generic moving objects///////////////////
                /////////referenc to free space oid = 0, mode = free ////
                ////////////////////////////////////////////////////

                Loc loc1(start_loc->GetX(), start_loc->GetY());
                Loc loc2(start_loc->GetX(), start_loc->GetY());
                GenLoc gloc1(0, loc1);
                GenLoc gloc2(0, loc2);

                int tm = GetTM("Free");
                UGenLoc* unit = new UGenLoc(up_interval, gloc1, gloc2, tm);
                genmo->Add(*unit); 
                delete unit; 
                ////////////////////////////////////////////////
                MPoint temp_mo(0);
                mo_metro = temp_mo;
                mo_metro_index = -1;
                mometro_oid = 0;
            }

              UPoint* up = new UPoint(up_interval,*start_loc, *start_loc);
              mo->Add(*up);
              delete up;

              start_time = et;

        }else{//////////moving with the metro 
//           cout<<"moving with metro"
//               <<" mo_metro size "<<mo_metro.GetNoComponents()<<endl;

          if(mo_metro.GetNoComponents() == 0){
            ////////////////////////////////////////
            //////////find the metro//////////////////
            //////////mode = metro////////////////////
            ////////////////////////////////////////
            assert(mometro_oid == 0);
            mometro_oid = 
              mn_nav->mn->GetMOMetro_MP(&ms1, start_loc, start_time, mo_metro);
//            cout<<"metro mpoint size "<<mo_metro.GetNoComponents()
//                <<" mometro oid "<<mometro_oid<<endl;

            assert(mometro_oid > 0);
            int pos1 = -1;
            int pos2 = -1;
            /////////find the range in mpoint for ms1, ms2//////////////
            FindPosInMP(&mo_metro, start_loc, end_loc, pos1, pos2, 0);

            assert(pos1 >= 0 && pos2 >= 0 && pos1 <= pos2);
//            cout<<"pos1 "<<pos1<<" pos2 "<<pos2<<endl;
            /////////////////////////////////////////////////////////////
            ///////////////set up mpoint//////////////////////////////
            ////////////////////////////////////////////////////////////
//            cout<<"initial start time "<<start_time<<endl;
            SetMO_GenMO(&mo_metro, pos1, pos2, start_time, mo, 
                        genmo, mometro_oid, "Metro");
            ///////////////////////////////////////////////////////////////
            mo_metro_index = pos2; 
            mo_metro_index++;/////////omit the 30 seconds waiting movement 

          }else{
            assert(last_ms.IsDefined());
            if(ms1.GetId() == last_ms.GetId() && 
               ms1.GetUp() == last_ms.GetUp()){////reference to the same metro

            int pos1 = -1;
            int pos2 = -1;
            /////////find the range in mpoint for bs1, bs2//////////////
            FindPosInMP(&mo_metro, start_loc, end_loc, pos1, pos2, 
                       mo_metro_index);

//            cout<<"pos1 "<<pos1<<" pos2 "<<pos2<<endl; 
            assert(pos1 >= 0 && pos2 >= 0 && pos1 <= pos2);

            SetMO_GenMO(&mo_metro, pos1, pos2, start_time, mo, 
                        genmo, mometro_oid, "Metro");

            ///////////////////////////////////////////////////////////////
            mo_metro_index = pos2; 
            mo_metro_index++;/////////omit the 30 seconds waiting movement 

            }else{
                //////////doing transfer without any waiting time/////////////
//             cout<<"seldomly happend"<<endl;
            //////////////////////////////////////////
            /////////////generic unit/////////////////
            //////////reference to metro, mode = metro////
            //////////////////////////////////////////
              MPoint temp_mo(0);
              mo_metro = temp_mo;
              mo_metro_index = -1;
              mometro_oid = 0;

              mometro_oid = 
              mn_nav->mn->GetMOMetro_MP(&ms1, start_loc, start_time, mo_metro);

              assert(mometro_oid > 0);
              int pos1 = -1;
              int pos2 = -1;
            /////////find the range in mpoint for ms1, ms2//////////////
              FindPosInMP(&mo_metro, start_loc, end_loc, pos1, pos2, 0);

              assert(pos1 >= 0 && pos2 >= 0 && pos1 <= pos2);
              SetMO_GenMO(&mo_metro, pos1, pos2, start_time, mo,
                         genmo, mometro_oid, "Metro");
             ///////////////////////////////////////////////////////////////
              mo_metro_index = pos2; 
              mo_metro_index++;/////////omit the 30 seconds waiting movement 
            }
          } 

         ///////////remove this part when the above code works correctly//////
//         start_time.ReadFrom(temp_start_time.ToDouble() + 
//                            t*1.0/(24.0*60.0*60.0));
        }

            for(int j = 0;j < sl->Size();j++){
              HalfSegment hs1;
              sl->Get(j, hs1);
              if(!hs1.IsLeftDomPoint()) continue;
              HalfSegment hs2(true, hs1.GetLeftPoint(), hs1.GetRightPoint());
              hs2.attr.edgeno = edgeno++;
              *l += hs2;
              hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
              *l += hs2;
            }

            last_ms = ms2;
            delete start_loc;
            delete end_loc; 


    } /////end for big for 

    l->EndBulkLoad();

    Line* temp_l = new Line(0);
    l->Union(*res_path, *temp_l);
    *res_path = *temp_l;
    delete temp_l;

    delete l;


    genmo->EndBulkLoad(false, false);
    /////////////////////////////////////////////////////////////////
    //////////////merge metro units referenc to the same metro /////////
    /////////////////////////////////////////////////////////////////
    vector<UGenLoc> ugloc_list;
    for(int i = 0;i < genmo->GetNoComponents();i++){
      UGenLoc u(true);
      genmo->Get(i, u);
      if(ugloc_list.size() == 0 || u.tm != TM_METRO){
        ugloc_list.push_back(u);

      }else{
        UGenLoc v = ugloc_list[ugloc_list.size() - 1];
        if(u.GetOid() != v.GetOid()){
            ugloc_list.push_back(u);

        }else{ //merge metro units 
            UGenLoc w = u;
            while(u.GetOid() == v.GetOid() && u.tm == v.tm &&
                  u.tm == TM_METRO && i < genmo->GetNoComponents()){
                w = u;
                i++;
//                cout<<"i "<<i<<endl;
                if(i < genmo->GetNoComponents())
                   genmo->Get(i, u);
          }
          assert(v.GetOid() == w.GetOid());
          Interval<Instant> up_interval; 
          up_interval.start = v.timeInterval.start;
          up_interval.lc = v.timeInterval.lc;
          up_interval.end = w.timeInterval.end;
          up_interval.rc = w.timeInterval.rc;

          UGenLoc* unit = new UGenLoc(up_interval, v.gloc1, w.gloc2, v.tm);
          ugloc_list[ugloc_list.size() - 1] = *unit;
          delete unit; 

          if(i < genmo->GetNoComponents())i--;
        }
      }
    }
    for(unsigned int i = 0;i < ugloc_list.size();i++)
      genmo_input->Add(ugloc_list[i]);
   ////////////////////////////////////////////////////
   delete genmo; 
}

/*
generic moving objects with modes: indoor walk metro + building relation

*/

void GenMObject::GenerateGenMO8(Space* sp, Periods* peri, int mo_no, 
                                int type, int para)
{
  if(mo_no < 1){
    cout<<" invalid number of moving objects "<<mo_no<<endl;
    return;
  }
  if(!(0 < type && type <= int(ARR_SIZE(genmo_tmlist)))){
    cout<<" invalid type value "<<type<<endl;
    return; 
  }

  IndoorInfra* i_infra = sp->LoadIndoorInfra(IF_GROOM);
  if(i_infra == NULL){
    cout<<"indoor infrastructure does not exist "<<endl;
    return;
  }

  ////////////////////////////////////////////////////////////////
  //////////////////////Initialization/////////////////////////////
  ////////////////////////////////////////////////////////////////
  Pavement* pm = sp->LoadPavement(IF_REGION);
  DualGraph* dg = pm->GetDualGraph();
  VisualGraph* vg = pm->GetVisualGraph();
  MetroNetwork* mn = sp->LoadMetroNetwork(IF_METRONETWORK);

  //////////////////////////////////////////////////////////////////
  ////////////////load all buildings and indoor graphs////////////////////////
  MaxRect* maxrect = new MaxRect();
  maxrect->OpenBuilding();
  maxrect->OpenIndoorGraph();

#ifdef INDOOR_PATH
  maxrect->LoadIndoorPaths(indoor_paths_list, rooms_id_list);
  assert(indoor_paths_list.size() == rooms_id_list.size());
#endif

  vector<RefBuild> build_id1_list;
  vector<RefBuild> build_id2_list;
  Relation* rel3 = sp->GetMSBuildRel();

  CreateBuildingPair4(i_infra, build_id1_list, build_id2_list, 
                      obj_scale*mo_no, maxrect, rel3);


  GenerateGenMOMIW(sp, i_infra, maxrect, pm, dg, vg, mn,
                   peri, mo_no, build_id1_list, build_id2_list, para);

  ///////////////////////////////////////////////////////////////////////////
  sp->CloseMetroNetwork(mn);
  pm->CloseDualGraph(dg);
  pm->CloseVisualGraph(vg);
  sp->ClosePavement(pm);

  maxrect->CloseIndoorGraph();
  maxrect->CloseBuilding();
  delete maxrect;
  
  sp->CloseIndoorInfra(i_infra);

}

/*
transportation modes with Metro + Indoor + Walk

*/

void GenMObject::GenerateGenMOMIW(Space* sp, IndoorInfra* i_infra,
                                  MaxRect* maxrect, Pavement* pm, 
                                  DualGraph* dg, VisualGraph* vg, 
                                  MetroNetwork* mn, 
                                  Periods* peri_input, int mo_no, 
                                  vector<RefBuild> build_id1_list,
                                  vector<RefBuild> build_id2_list,
                                  int para)
{

  Relation* rel1 = sp->GetNewTriRel();
  Relation* rel2 = sp->GetMSPaveRel();
  Relation* rel3 = sp->GetMSBuildRel();
  R_Tree<2,TupleId>* rtree = sp->GetMSPaveRtree();
  if(rel1 == NULL || rel2 == NULL || rtree == NULL || rel3 == NULL){
      cout<<"auxiliary relation empty MIW"<<endl;
      return;
  }


  ////////////////////////////////////////////////////////////////
  ////////////select a pair of buildings/////////////////////////
  //////////////////////////////////////////////////////////////
  int real_mo_count = obj_scale*mo_no;
  ////more buildings than input, because some pairs may be not available///

   /////////////////////////////////////////////////////////////////
   Relation* build_path_rel = i_infra->BuildingPath_Rel();
   /////////////////////////////////////////////////////////////////
   ///////////////start time///////////////////////////////////////
   Interval<Instant> periods;
   peri_input->Get(0, periods);
   Instant start_time = periods.start;
   int time_range = para*60;//12 hours in range 
    ////////////////////////////////////////////////////////////////
    ///////////// metro network time periods ///////////////////////
    ////////////////////////////////////////////////////////////////
    MetroGraph* mg = mn->GetMetroGraph();
    if(mg == NULL){
        cout<<"metro graph is not valid"<<endl;
        return;
    }
    Instant ms_start(mn->GetStartTime());
    Instant ms_end(mn->GetEndTime());
    Instant mg_min(instanttype);
    mg_min.ReadFrom(mg->GetMIN_T());

    mn->CloseMetroGraph(mg);
    /////////////////////////////////////////////////////////////////
   int count = 0;
   int real_count = 1;
   const double min_path = 0.1;
   
   int obj_no_rep = 0;
   int max_obj = 1;
   if(mo_no <= 500) max_obj = 2;
    else if(mo_no <= 1000) max_obj = 3;
    else if(mo_no <= 5000) max_obj = 4;
    else max_obj = 5;

    int index1 = -1;
    int index2 = -1;
    int time_and_type = 4;

   while(real_count <= mo_no && count < real_mo_count ){

   //////////////////////////////start time///////////////////////////
   if(count % time_and_type == 0)// less trips on Sunday 
     start_time.ReadFrom(periods.start.ToDouble() + 
                        (GetRandom() % time_range)/(24.0*60.0));
   else{ ////20:00 before 
     start_time.ReadFrom(periods.end.ToDouble() - 2.0/24.0 -
                        (GetRandom() % time_range)/(24.0*60.0));
   }

    if(periods.Contains(start_time) == false) continue;
    
    ///////////////////////////////////////////////////////////////////
    bool time_transform = false;
    int day_deviation = 0;
    Instant old_st(start_time);
    Periods* peri = new Periods(0);
    peri->StartBulkLoad();

    if(start_time < ms_start || start_time > ms_end){
        time_transform = true;

        start_time.Set(mg_min.GetYear(), mg_min.GetMonth(), 
                         mg_min.GetGregDay(), 
                         start_time.GetHour(), start_time.GetMinute(), 
                         start_time.GetSecond(), start_time.GetMillisecond());

        day_deviation = old_st.GetDay() - start_time.GetDay();
//        cout<<" day deviation "<<day_deviation<<endl;

        Interval<Instant> time_span;
        time_span.start = periods.start;
        time_span.start.ReadFrom(periods.start.ToDouble() - day_deviation);
        time_span.lc = true;
        time_span.end = periods.end;
        time_span.end.ReadFrom(periods.end.ToDouble() - day_deviation);
        time_span.rc = false;
        peri->MergeAdd(time_span);
    }

    peri->EndBulkLoad();

    //////////////////////////////////////////////////////////////////

    if(obj_no_rep == 0){
        index1 = GetRandom() % build_id1_list.size();
        index2 = GetRandom() % build_id2_list.size();
    }
    /////////////////////load all paths from this building////////////////
    vector<int> path_id_list1;
    i_infra->GetPathIDFromTypeID(build_id1_list[index1].reg_id, path_id_list1);

    vector<int> path_id_list2;
    i_infra->GetPathIDFromTypeID(build_id2_list[index2].reg_id, path_id_list2);

    ////////////////////////////////////////////////////////////////////
   //////////////if a building has several entrances///////////////////
   ///////////it randomly selects an entrance  ///////////////////////
   ////////////the buildings are far away from each///////////////////
   //////////which entrance to go out does not influence the distance a lot///
   ////////////////////////////////////////////////////////////////////
   if(path_id_list1.size() == 0 || path_id_list2.size() == 0) {
      count++;
      delete peri;
      continue;
    }
   //////////////////////////////////////////////////////////////////////

//     cout<<"building id1 "<<build_id1_list[index1].build_id
//         <<" building id2 "<<build_id2_list[index2].build_id<<endl;

    int path_tid1 = path_id_list1[GetRandom() % path_id_list1.size()];
    int path_tid2 = path_id_list2[GetRandom() % path_id_list2.size()];


    Tuple* path_tuple1 = build_path_rel->GetTuple(path_tid1, false);
    Tuple* path_tuple2 = build_path_rel->GetTuple(path_tid2, false);


    /////////////////////////////////////////////////////////////////////////
    ////////////////get start and end locaton of the path in pavement////////
    /////////////////////////////////////////////////////////////////////////

    GenLoc* build_gloc1 = 
        (GenLoc*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
    Point* ep1_1 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP);
    Point* ep1_2 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP2);


    GenLoc* build_gloc2 = 
        (GenLoc*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
    Point* ep2_1 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP);
    Point* ep2_2 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP2);

    /////////////////////////////////////////////////////////////
    /////////////reset the general location//////////////////////
    Loc loc1(ep1_2->GetX(), ep1_2->GetY());
    GenLoc newgloc1(build_gloc1->GetOid(), loc1);
    Loc loc2(ep2_2->GetX(), ep2_2->GetY());
    GenLoc newgloc2(build_gloc2->GetOid(), loc2);

    //////////////////////////////////////////////////////////////////////////
    //////find closest metro stops to the two locations on the pavement//////
    Bus_Stop ms1(true, 0, 0, true);
    vector<Point> ps_list1;//1 point on the pavement 2 point of metro stop
    GenLoc gloc1(0, Loc(0, 0));//metro stop on pavement by genloc 
    bool b1 = true;
    b1 = NearestMetroStop(newgloc1,  rel2, rtree, ms1, ps_list1, gloc1);

    
    Bus_Stop ms2(true, 0, 0, true);
    vector<Point> ps_list2;//1. point on the pavement 2. point of metro stop
    GenLoc gloc2(0, Loc(0, 0));//metro stop on pavement by genloc
    bool b2 = true;
    b2 = NearestMetroStop(newgloc2,  rel2, rtree, ms2, ps_list2, gloc2);

    if((b1 && b2) == false){
      path_tuple1->DeleteIfAllowed();
      path_tuple2->DeleteIfAllowed();
      count++;
      delete peri; 
      continue;
    }

    Point start_p, end_p; 
    mn->GetMetroStopGeoData(&ms1, &start_p);
    mn->GetMetroStopGeoData(&ms2, &end_p);
//    const double delta_dist = 0.01; 

//    if(start_p.Distance(end_p) < delta_dist){
    if(start_p.Distance(end_p) < EPSDIST){
      path_tuple1->DeleteIfAllowed();
      path_tuple2->DeleteIfAllowed();
      count++;
      delete peri; 
      continue;
    }

    MPoint* mo = new MPoint(0);
    GenMO* genmo = new GenMO(0);
    mo->StartBulkLoad();
    genmo->StartBulkLoad();

//    cout<<"building 1 "<<GetBuildingStr(build_id1_list[index1].type)
//         <<" building 2 "<<GetBuildingStr(build_id2_list[index2].type)<<endl;

    /////////////////////////////////////////////////////////////////////
    ///////////////1. indoor movement 1 + pavement//////////////////////
    ////////////////////////////////////////////////////////////////////

    //////////////////////indoor movement///////////////////////////////
    ////////////////////to show which entrance it is////////////////////

    int entrance_index1 = ((CcInt*)path_tuple1->GetAttribute(IndoorInfra::
                           INDOORIF_SP_INDEX))->GetIntval();
    int reg_id1 =  ((CcInt*)path_tuple1->GetAttribute(IndoorInfra::
                                               INDOORIF_REG_ID))->GetIntval();
    Point* sp1 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_SP);

    MPoint3D* mp3d = new MPoint3D(0);
    if(time_transform)
      GenerateIndoorMovementToExit2(i_infra, genmo, mo, start_time, *sp1,
                                 entrance_index1, reg_id1, maxrect, peri, mp3d);
    else
      GenerateIndoorMovementToExit2(i_infra, genmo, mo, start_time, *sp1,
                          entrance_index1, reg_id1, maxrect, peri_input, mp3d);

    Line* path1 = (Line*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_PATH);
    if(path1->Length() > min_path)
      GenerateFreeMovement(path1, *sp1, genmo, mo, start_time);
    GenerateFreeMovement2(*ep1_1, *ep1_2, genmo, mo, start_time);


    ///////////////////////////////////////////////////////////////////
    ////////////////2. path (walk + metro)///////////////////////////////
    ///////////////////////////////////////////////////////////////////

    ////////2.1 connect from pavement to start metro stop/////////////////
    Line* res_path = new Line(0);
    
    Instant last_t = start_time;
    
    ConnectStartStop(dg, vg, rel1, newgloc1, ps_list1, gloc1.GetOid(),
                            genmo, mo, start_time, res_path);

    double delta_time = start_time.ToDouble() - last_t.ToDouble();
    double delta_minute = delta_time*86400.0/60.0;

    if(delta_minute > 60.0){ // more than one hour, ignore such a place
          delete res_path;
          delete mp3d;
          mo->EndBulkLoad();
          genmo->EndBulkLoad();
          delete mo;
          delete genmo;
          path_tuple1->DeleteIfAllowed();
          path_tuple2->DeleteIfAllowed();
          continue;
    }

    /////////////////////////////////////////////////////////////////
    //////2.2. get the path in metro network///////////////////////////
    /////////////////////////////////////////////////////////////////

//   cout<<"start "<<ms1<<" end "<<ms2<<endl; 

     MNNav* mn_nav = new MNNav(mn);
     mn_nav->ShortestPath_Time(&ms1, &ms2, &start_time);

     if(mn_nav->path_list.size() == 0){
//          cout<<"two unreachable metro stops"<<endl;
        mo->EndBulkLoad();
        genmo->EndBulkLoad();
        delete mo;
        delete genmo;
        delete mn_nav;
        delete res_path;

        delete mp3d;
        path_tuple1->DeleteIfAllowed();
        path_tuple2->DeleteIfAllowed();
        count++;

        continue;
      }

      ////////////////////////////////////////////////////////////////////
        Interval<Instant> temp_periods;
        if(time_transform)
            temp_periods.start = old_st;
        else
          temp_periods.start = start_time;

        temp_periods.lc = true;
        Instant temp_end(instanttype);
        temp_end.ReadFrom(temp_periods.start.ToDouble() + 
                          mn_nav->t_cost/86400.0);

        temp_periods.end = temp_end;
        temp_periods.lc = false;

        if(periods.Contains(temp_periods) == false ){
            mo->EndBulkLoad();
            genmo->EndBulkLoad();

            delete mo;
            delete genmo;
            delete mn_nav;
            delete res_path;
            delete peri;
            delete mp3d;
            path_tuple1->DeleteIfAllowed();
            path_tuple2->DeleteIfAllowed();
            count++;
            continue;
        }

     /////////////////////////////////////////////////////////////////////

     ConnectTwoMetroStops(mn_nav, ps_list1[1], ps_list2[1],
                          genmo, mo, start_time, dg, res_path);

     delete mn_nav;

    /////////////////2.3 connect from end metro stop to pavement///////////
    ConnectEndStop(dg, vg, rel1, newgloc2, ps_list2, gloc2.GetOid(),
                          genmo, mo, start_time, res_path);


     ////////////////////////////////////////////////////////////////////
     /////////////3. pavement + indoor movement 2////////////////////////
     ////////////////////////////////////////////////////////////////////

    ////////////////outdoor movement/////////////////////////////////////////

//    line_list2.push_back(*path2);
    GenerateFreeMovement2(*ep2_2, *ep2_1, genmo, mo, start_time);
    Line* path2 = (Line*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_PATH);
    if(path2->Length() > min_path)
       GenerateFreeMovement(path2, *ep2_1, genmo, mo, start_time);

    ///////////////////indoor movement//////////////////////////////////////
    ////////////////////to show which entrance it is////////////////////
    Point* sp2 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_SP);
    int entrance_index2 = ((CcInt*)path_tuple2->GetAttribute(IndoorInfra::
                                   INDOORIF_SP_INDEX))->GetIntval();
    int reg_id2 =  ((CcInt*)path_tuple2->GetAttribute(IndoorInfra::
                                              INDOORIF_REG_ID))->GetIntval();

    if(time_transform){
      if(WorkBuilding(build_id2_list[count].type))
        GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                   entrance_index2, reg_id2, 
                                        maxrect, peri, TIMESPAN_W1);
      else
        GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2,
                                   entrance_index2, reg_id2, maxrect, peri, 
                                        TIMESPAN_NO);
    }else{
      if(WorkBuilding(build_id2_list[count].type))
        GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                               entrance_index2, reg_id2, maxrect, 
                                        peri_input, TIMESPAN_W1);
      else
        GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                               entrance_index2, reg_id2, maxrect, peri_input,
                                        TIMESPAN_NO);
    }
    //////////put the first part of movement inside a building//////////////
    indoor_mo_list1.push_back(*mp3d);
    delete mp3d; 


    ///////////////////////////////////////////////////////////////////////
    path_tuple1->DeleteIfAllowed();
    path_tuple2->DeleteIfAllowed(); 

    ///////////////////////store building type//////////////////////////////

    build_type_list1.push_back(build_id1_list[count].type);
    build_type_list2.push_back(build_id2_list[count].type); 


     //////////////////////////////////////////////////////////////////////
     mo->EndBulkLoad();
     genmo->EndBulkLoad(false, false);

     if(time_transform){
            MPoint* mo_tmp = new MPoint(0);
            GenMO* genmo_tmp = new GenMO(0);
            mo_tmp->StartBulkLoad();
            genmo_tmp->StartBulkLoad();
            for(int i = 0;i < mo->GetNoComponents();i++){
               UPoint unit1;
               mo->Get(i, unit1);

               Instant st = unit1.timeInterval.start;
               Instant et = unit1.timeInterval.end;
  
               st.ReadFrom(st.ToDouble() + day_deviation);
               et.ReadFrom(et.ToDouble() + day_deviation);

               Interval<Instant> up_interval;
               up_interval.start = st;
               up_interval.lc = true;
               up_interval.end = et;
               up_interval.rc = false; 

               UPoint* up = new UPoint(up_interval, unit1.p0, unit1.p1);
               mo_tmp->Add(*up);
               delete up;     
            }

            for(int i = 0;i < genmo->GetNoComponents(); i++){
                UGenLoc ugloc;
                genmo->Get(i, ugloc);

                Instant st = ugloc.timeInterval.start;
                Instant et = ugloc.timeInterval.end;
  
                st.ReadFrom(st.ToDouble() + day_deviation);
                et.ReadFrom(et.ToDouble() + day_deviation);

                Interval<Instant> up_interval;
                up_interval.start = st;
                up_interval.lc = true;
                up_interval.end = et;
                up_interval.rc = false; 

                UGenLoc* unit_new = new UGenLoc(up_interval, ugloc.gloc1,
                                    ugloc.gloc2, ugloc.tm);
                genmo_tmp->Add(*unit_new);
                delete unit_new;
            }

            mo_tmp->EndBulkLoad();
            genmo_tmp->EndBulkLoad(false, false);

            trip1_list.push_back(*genmo_tmp);
            trip2_list.push_back(*mo_tmp);

            delete genmo_tmp;
            delete mo_tmp;

     }else{
        trip1_list.push_back(*genmo);
        trip2_list.push_back(*mo);
     }
//     path_list.push_back(*res_path);

     delete genmo;
     delete mo;

     delete res_path;

//      cout<<"building 1 "<<GetBuildingStr(build_id1_list[index1].type)
//         <<" building 2 "<<GetBuildingStr(build_id2_list[index2].type)<<endl;

     cout<<real_count<<" generic moving object "<<endl;
     real_count++;

     count++;

     //////////////////////////////////////////////////////////////////
     ////////////reduce the time cost of generating moving object//////
     ////////////if two bus stops are reachable, it generates the second////
     //////// moving object, but different time intervals //////////////////
     ///////////////////////////////////////////////////////////////////////
     obj_no_rep++;
     if(obj_no_rep == max_obj) obj_no_rep = 0;
  }

}

////////////////////////////////////////////////////////////////////////
////////////////// benchmark functions ////////////////////////////////
//////////////////////////////////////////////////////////////////////

/*
create regular movement: (1) home-work; (2)work-home; (3)work-work

*/
void GenMObject::GenerateGenMOBench1(Space* sp, Periods* peri, int mo_no,
                          Relation* distri, Relation* build_rel1, 
                          Relation* build_rel2)
{
  ///////////////////////////////////////////////////////////////
  //////////  Initialization open infrastructures  /////////////
  //////////////////////////////////////////////////////////////

  IndoorInfra* i_infra = sp->LoadIndoorInfra(IF_GROOM);
  if(i_infra == NULL){
      cout<<"indoor infrastructure does not exist "<<endl;
     return;
  }

  Pavement* pm = sp->LoadPavement(IF_REGION);
  if(pm == NULL){
    cout<<"pavement loading error"<<endl;
    sp->CloseIndoorInfra(i_infra);
    return;
  }

  Network* rn = sp->LoadRoadNetwork(IF_LINE);
  if(rn == NULL){
    cout<<"road network loading error"<<endl;
    sp->ClosePavement(pm);
    sp->CloseIndoorInfra(i_infra);
    return;
  }

  RoadGraph* rg = sp->LoadRoadGraph();
  if(rg == NULL){
    sp->CloseRoadNetwork(rn);
    sp->ClosePavement(pm);
    sp->CloseIndoorInfra(i_infra);
    cout<<"road graph loading error"<<endl;
    return;
  }
  ///////////////get buildings with full data ////////////////////////
  ////////////////load all buildings and indoor graphs/////////////
  ////////////////// and indoor paths//////////////////////////////
  MaxRect* maxrect = new MaxRect();
  maxrect->OpenBuilding();
  maxrect->OpenIndoorGraph();

#ifdef INDOOR_PATH
  maxrect->LoadIndoorPaths(indoor_paths_list, rooms_id_list);
  assert(indoor_paths_list.size() == rooms_id_list.size());
#endif 


  ////////////get buildings information /////////////////////////

  vector<RefBuild> build_id1_list;
  vector<RefBuild> build_id2_list;
  GetSelectedBuilding(i_infra, build_id1_list, build_id2_list, 
                     maxrect, build_rel1, build_rel2);
  
//  cout<<build_id1_list.size()<<" "<<build_id2_list.size()<<endl; 
  /////////////////////set time interval ////////////////////////////
  
  Interval<Instant> time_span;
  peri->Get(0, time_span);
  
  if(time_span.start.GetDay() != time_span.end.GetDay()){
    cout<<"time period should be in one day "<<endl;
    maxrect->CloseIndoorGraph();
    maxrect->CloseBuilding();
    delete maxrect;
    sp->CloseIndoorInfra(i_infra);
    return;
  }
  int h1 = time_span.start.GetHour();
  int h2 = time_span.end.GetHour();
  int para = (h2 - h1);
  //////////////////////////////////////////////////////////////////////

  ////////////// for each kind mode, create moving objects //////////////
  for(int i = 1;i <= distri->GetNoTuples();i++){
      Tuple* dis_tuple = distri->GetTuple(i, false);
      float dis_para = 
          ((CcReal*)dis_tuple->GetAttribute(BENCH_PARA))->GetRealval();
      int genmo_no = (int)(ceil(mo_no*dis_para));
      string mode = 
        ((CcString*)dis_tuple->GetAttribute(BENCH_MODE))->GetValue();


      if(GetTM(mode) == TM_CAR || GetTM(mode) == TM_BIKE){

//          cout<<"mode "<<mode<<" trip no. "<<genmo_no<<endl; 

          ////////// include train station /////////////////////

          vector<RefBuild> build_list1;
          vector<RefBuild> build_list2;

          CreateBuildingPair5(build_id1_list, build_id2_list,
                              build_list1, build_list2, 
                              genmo_no * obj_scale_min, false);//no station

          GenerateGenMO_IWCTB(sp, maxrect, i_infra, 
                              pm, rn, rg, peri, genmo_no, mode,
                              build_list1, build_list2, para);

      }else if(GetTM(mode) == TM_TAXI){
//          cout<<"mode "<<mode<<" trip no. "<<genmo_no<<endl; 

          vector<RefBuild> build_list1;
          vector<RefBuild> build_list2;

          CreateBuildingPair5(build_id1_list, build_id2_list,
                              build_list1, build_list2,
                              genmo_no * obj_scale_min, true);//has station

          GenerateGenMO_IWCTB(sp, maxrect, i_infra, pm, rn, rg,
                              peri, genmo_no, mode,
                              build_list1, build_list2, para);

      }else if(GetTM(mode) == TM_BUS){
//          cout<<"mode "<<mode<<" trip no. "<<genmo_no<<endl;

          vector<RefBuild> build_list1;
          vector<RefBuild> build_list2;
          Relation* build_rel3 = sp->GetBSBuildRel();
          //////////the buildings should be not far away from bus stops//////
          GetSelectedBuilding2(i_infra, build_list1, build_list2, 
                     maxrect, build_rel1, build_rel2, build_rel3);

          vector<RefBuild> b_list1;
          vector<RefBuild> b_list2;

          CreateBuildingPair5(build_list1, build_list2, 
                             b_list1, b_list2, genmo_no * obj_scale_max, true);

          DualGraph* dg = pm->GetDualGraph();
          VisualGraph* vg = pm->GetVisualGraph();
          BusNetwork* bn = sp->LoadBusNetwork(IF_BUSNETWORK);

          GenerateGenIBW(sp, maxrect, i_infra, pm, dg, vg, bn, peri, genmo_no,
                        b_list1, b_list2, para);

          sp->CloseBusNetwork(bn);
          pm->CloseDualGraph(dg);
          pm->CloseVisualGraph(vg);

      }else if(GetTM(mode) == TM_METRO){// mode metro 

//              cout<<"mode "<<mode<<" trip no. "<<genmo_no<<endl;
              vector<RefBuild> build_list1;
              vector<RefBuild> build_list2;
              Relation* build_rel3 = sp->GetMSBuildRel();
             /////the buildings should be not far away from metro stops//////
              GetSelectedBuilding2(i_infra, build_list1, build_list2, 
                     maxrect, build_rel1, build_rel2, build_rel3);
              vector<RefBuild> b_list1;
              vector<RefBuild> b_list2;
              CreateBuildingPair5(build_list1, build_list2,
                        b_list1, b_list2, genmo_no * obj_scale, true);

              DualGraph* dg = pm->GetDualGraph();
              VisualGraph* vg = pm->GetVisualGraph();

              MetroNetwork* mn = sp->LoadMetroNetwork(IF_METRONETWORK);

              GenerateGenMOMIW(sp, i_infra, maxrect,
                            pm, dg, vg, mn, peri, genmo_no,
                            b_list1, b_list2, para);

              sp->CloseMetroNetwork(mn);

              pm->CloseDualGraph(dg);
              pm->CloseVisualGraph(vg);


      }else{

        cout<<"invalid mode "<<mode<<endl;
      }

      dis_tuple->DeleteIfAllowed();
  }

    ///////////////////////////////////////////////////////////////////
    /////////////////// close all infrastructures ///////////////////
    /////////////////////////////////////////////////////////////////
    maxrect->CloseIndoorGraph();
    maxrect->CloseBuilding();
    delete maxrect;

    sp->CloseRoadGraph(rg);
    sp->CloseRoadNetwork(rn);
    sp->ClosePavement(pm);
    sp->CloseIndoorInfra(i_infra);

}

/*
create generic moving objects in one environment: 
region based outdoor; indoor 

*/

void GenMObject::GenerateGenMOBench2(Space* sp, Periods* peri, int mo_no,
                          Relation* id_rel, string type)

{
  if(type.compare("REGION") == 0){
      GenMOBenchRBO(sp, peri, mo_no, id_rel);

  }else if(type.compare("INDOOR") == 0){
      GenMOBenchIndoor(sp, peri, mo_no, id_rel);

  }else{
    cout<<"wrong type. should be REGION or INDOOR"<<endl;
    return;
  }


}

/*
create moving objects in region based outdoor 

*/
void GenMObject::GenMOBenchRBO(Space* sp, Periods* peri, int mo_no, 
                               Relation* id_rel)
{

    if(mo_no < 1){
      cout<<" invalid number of moving objects "<<mo_no<<endl;
      return;
    }

    Relation* tri_rel = sp->GetNewTriRel();
    if(tri_rel == NULL){
        cout<<"auxiliary trirel empty "<<endl;
        return;
    }

    Interval<Instant> periods;
    peri->Get(0, periods);

    if(periods.start.GetDay() != periods.end.GetDay()){
      cout<<"time period should be in one day "<<endl;
      return;
    }
   int h1 = periods.start.GetHour();
   int h2 = periods.end.GetHour();
   int time_range = (h2 - h1)*60;
   Instant start_time = periods.start;

   vector<int> tri_id_list;

    for(int i = 1;i <= id_rel->GetNoTuples();i++){

      Tuple* tuple = id_rel->GetTuple(i, false);
      int tri_id = ((CcInt*)tuple->GetAttribute(0))->GetIntval();
      tri_id_list.push_back(tri_id);
    }

//     for(int i = 0;i < tri_id_list.size();i++)
//       cout<<tri_id_list[i].size()<<endl;

  ////////////////////////////////////////////////////////////////
  //////////////////////Initialization/////////////////////////////
  ////////////////////////////////////////////////////////////////
  Pavement* pm = sp->LoadPavement(IF_REGION);
  DualGraph* dg = pm->GetDualGraph();
  Relation* pave_rel = pm->GetPaveRel();
  Relation* dg_node_rel = dg->GetNodeRel();
  int no_triangle = dg_node_rel->GetNoTuples();
  assert(no_triangle == pave_rel->GetNoTuples());
  int mini_oid = dg->min_tri_oid_1;
  VisualGraph* vg = pm->GetVisualGraph();
  Walk_SP* wsp = new Walk_SP(dg, vg, dg_node_rel, NULL);
  
  wsp->rel3 = tri_rel;

  const double min_path = 200.0;

  int count = 1;
  while(count <= mo_no){

    if(count % 2 == 0)
     start_time.ReadFrom(periods.start.ToDouble() + 
                        (GetRandom() % time_range)/(24.0*60.0));
    else
     start_time.ReadFrom(periods.end.ToDouble() -
                        (GetRandom() % time_range)/(24.0*60.0));

//    cout<<start_time<<endl;

    int index1 = GetRandom() % tri_id_list.size();
    int index2 = GetRandom() % tri_id_list.size();

    int tid1 = tri_id_list[index1];
    int tid2 = tri_id_list[index2];

//    cout<<"tid1 "<<tid1<<" tid2 "<<tid2<<endl;
    assert(1 <= tid1 && tid1 <= no_triangle);
    assert(1 <= tid2 && tid2 <= no_triangle);
    ////////////////get point on the region-based outdoor/////////////
    Tuple* tuple1 = pave_rel->GetTuple(tid1, false);
//    Tuple* tuple1_tmp = dg_node_rel->GetTuple(tid1, false);
    int oid1 = ((CcInt*)tuple1->GetAttribute(Pavement::P_OID))->GetIntval();
//    int oid1_tmp = 
//          ((CcInt*)tuple1_tmp->GetAttribute(DualGraph::OID))->GetIntval();
//    assert(oid1 == oid1_tmp);
//    tuple1_tmp->DeleteIfAllowed();
    tuple1->DeleteIfAllowed();


    Tuple* tuple2 = pave_rel->GetTuple(tid2, false);
//    Tuple* tuple2_tmp = dg_node_rel->GetTuple(tid2, false);
    int oid2 = ((CcInt*)tuple2->GetAttribute(Pavement::P_OID))->GetIntval();
//    int oid2_tmp = 
//          ((CcInt*)tuple2_tmp->GetAttribute(DualGraph::OID))->GetIntval();
//    assert(oid2 == oid2_tmp);
//    tuple2_tmp->DeleteIfAllowed();
    tuple2->DeleteIfAllowed();
    oid1 = oid1 - mini_oid;
    oid2 = oid2 - mini_oid;

    /////////////////////////////////////////////////////////////////
    if(wsp->GenerateData4(tid1) == false){
       wsp->oids.clear();
       wsp->q_loc1.clear();
       wsp->q_loc2.clear();
       continue;
    }

    Point sp1 = wsp->q_loc1[0];
    Point sp2 = wsp->q_loc2[0];

//    cout<<"1 "<<wsp->oids[0]<<" "<<sp1<<" "<<sp2<<endl;
    if(wsp->GenerateData4(tid2) == false){
      wsp->oids.clear();
      wsp->q_loc1.clear();
      wsp->q_loc2.clear();

      continue;
    }


    Point ep1 = wsp->q_loc1[1];
    Point ep2 = wsp->q_loc2[1];
//    cout<<"2 "<<wsp->oids[1]<<" "<<ep1<<" "<<ep2<<endl;

    wsp->oids.clear();
    wsp->q_loc1.clear();
    wsp->q_loc2.clear();

    if(sp2.Distance(ep2) < min_path){
      wsp->oids.clear();
      wsp->q_loc1.clear();
      wsp->q_loc2.clear();
      continue;
    }


    Line* path = new Line(0);
    wsp->WalkShortestPath2(oid1, oid2, sp2, ep2, path);
//    cout<<path->Length()<<endl;
    MPoint* mo = new MPoint(0);
    GenMO* genmo = new GenMO(0);
    mo->StartBulkLoad();
    genmo->StartBulkLoad();
    GenerateWalkMovement(dg, path, sp2, genmo, mo, start_time);

    mo->EndBulkLoad();
    genmo->EndBulkLoad(false, false);

    trip1_list.push_back(*genmo);
    trip2_list.push_back(*mo);
    
    delete mo; 
    delete genmo; 

    delete path;

    cout<<count<<" generic moving object"<<endl;
    count++;

  }

  delete wsp;
  pm->CloseDualGraph(dg);
  pm->CloseVisualGraph(vg);
  sp->ClosePavement(pm);


}

/*
create moving objects only indoor (inside one building)

*/
void GenMObject::GenMOBenchIndoor(Space* sp, Periods* peri, int mo_no, 
                               Relation* id_rel)
{
   if(mo_no < 1){
      cout<<" invalid number of moving objects "<<mo_no<<endl;
      return;
   }

  Interval<Instant> periods;
  peri->Get(0, periods);
  Instant start_time = periods.start;  
  
  if(periods.start.GetDay() != periods.end.GetDay()){
    cout<<"time period should be in one day "<<endl;
    return;
  }
  

  IndoorInfra* i_infra = sp->LoadIndoorInfra(IF_GROOM);
  if(i_infra == NULL){
      cout<<"indoor infrastructure does not exist "<<endl;
     return;
  }

  int h1 = periods.start.GetHour();
  int h2 = periods.end.GetHour();
  int time_range = (h2 - h1)*60;


  MaxRect* maxrect = new MaxRect();
  maxrect->OpenBuilding();
  maxrect->OpenIndoorGraph();

#ifdef INDOOR_PATH
  maxrect->LoadIndoorPaths(indoor_paths_list, rooms_id_list);
  assert(indoor_paths_list.size() == rooms_id_list.size());
#endif 

  int count = 1;
  vector<RefBuild> build_tid_list;
  GetSelectedBuilding3(i_infra, build_tid_list, maxrect, id_rel);
  Relation* build_path_rel = i_infra->BuildingPath_Rel();

  while(count <= mo_no){

    if(count % 2 == 0)
     start_time.ReadFrom(periods.start.ToDouble() + 
                        (GetRandom() % time_range)/(24.0*60.0));
    else
     start_time.ReadFrom(periods.end.ToDouble() -
                        (GetRandom() % time_range)/(24.0*60.0));

//    cout<<"start time "<<start_time<<endl;
    int index = GetRandom() % build_tid_list.size();

    /////////////////////load all paths from this building////////////////
    vector<int> path_list;
    i_infra->GetPathIDFromTypeID(build_tid_list[index].reg_id, path_list);
    if(path_list.size() == 0)continue;

//    cout<<"number of paths "<<path_list.size()<<endl;

//    cout<<"building "<<GetBuildingStr(build_tid_list[index].type)<<endl;

    int path_tid = path_list[GetRandom() % path_list.size()];
    Tuple* path_tuple = build_path_rel->GetTuple(path_tid, false);

    int entrance_index = ((CcInt*)path_tuple->GetAttribute(IndoorInfra::
                           INDOORIF_SP_INDEX))->GetIntval();
    int reg_id =  ((CcInt*)path_tuple->GetAttribute(IndoorInfra::
                                               INDOORIF_REG_ID))->GetIntval();

    assert(reg_id == build_tid_list[index].reg_id);
    Point* sp = (Point*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_SP);

    MPoint* mo = new MPoint(0);
    GenMO* genmo = new GenMO(0);
    mo->StartBulkLoad();
    genmo->StartBulkLoad();

    if(count % 2 == 0)
      GenerateIndoorMovementToExit(i_infra, genmo, mo, start_time, *sp,
                                  entrance_index, reg_id, maxrect, peri);
    else{
      if(WorkBuilding(build_tid_list[index].type))
        GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp, 
                                   entrance_index, reg_id, maxrect, 
                                        peri, TIMESPAN_W2);
      else
        GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp,
                                   entrance_index, reg_id, maxrect, peri,
                                        TIMESPAN_NO);
    }

    mo->EndBulkLoad();
    genmo->EndBulkLoad(false, false);

    trip1_list.push_back(*genmo);
    trip2_list.push_back(*mo);

    delete mo;
    delete genmo;

    path_tuple->DeleteIfAllowed();
    cout<<count<<" generic moving objects "<<endl;
    count++;
  }


  maxrect->CloseIndoorGraph();
  maxrect->CloseBuilding();
  delete maxrect;

  sp->CloseIndoorInfra(i_infra);

}


/*
select buildings from two input relations, each of them store the builing id
in the original relation buiding stored in space 
the first relation might be home buildings + train station 
the second relation might be office buildings 

*/
void GenMObject::GetSelectedBuilding(IndoorInfra* i_infra, 
                          vector<RefBuild>& build_tid1_list,
                          vector<RefBuild>& build_tid2_list, MaxRect* maxrect, 
                          Relation* rel1, Relation* rel2)
{

  Relation* build_type_rel = i_infra->BuildingType_Rel();

  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple = rel1->GetTuple(i, false);
    int build_tid = ((CcInt*)tuple->GetAttribute(0))->GetIntval();
    assert(1 <= build_tid && build_tid <= build_type_rel->GetNoTuples());
    tuple->DeleteIfAllowed();

    Tuple* build_tuple = build_type_rel->GetTuple(build_tid, false);
    Rectangle<2>* bbox = 
       (Rectangle<2>*)build_tuple->GetAttribute(IndoorInfra::INDOORIF_GEODATA);
    int type =  ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_TYPE))->GetIntval();

//    cout<<GetBuildingStr(type)<<endl;
    if(type > BUILD_HOUSE && maxrect->build_pointer[type] == NULL){
      cout<<GetBuildingStr(type)<<" not valid "<<endl;
      build_tuple->DeleteIfAllowed();
      continue;
    }

    int reg_id = ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                INDOORIF_REG_ID))->GetIntval();

    ////////////////////////////////////////////////////////
    vector<int> path_id_list;
    i_infra->GetPathIDFromTypeID(reg_id, path_id_list);
    if(path_id_list.size() == 0){ //no path available, no such a building
      build_tuple->DeleteIfAllowed();
      cout<<GetBuildingStr(type)<<" path not valid "<<endl;
      continue;
    }

   int build_id = ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_ID))->GetIntval();

    RefBuild ref_b(true, reg_id, build_id, type, *bbox, build_tid);
    build_tid1_list.push_back(ref_b);
    build_tuple->DeleteIfAllowed();

  }

  for(int i = 1;i <= rel2->GetNoTuples();i++){
    Tuple* tuple = rel2->GetTuple(i, false);
    int build_tid = ((CcInt*)tuple->GetAttribute(0))->GetIntval();
    assert(1 <= build_tid && build_tid <= build_type_rel->GetNoTuples());
    tuple->DeleteIfAllowed();

    Tuple* build_tuple = build_type_rel->GetTuple(build_tid, false);
    Rectangle<2>* bbox = 
       (Rectangle<2>*)build_tuple->GetAttribute(IndoorInfra::INDOORIF_GEODATA);
    int type =  ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_TYPE))->GetIntval();

//    cout<<GetBuildingStr(type)<<endl;
    if(type > BUILD_HOUSE && maxrect->build_pointer[type] == NULL){
      cout<<GetBuildingStr(type)<<" not valid "<<endl;
      build_tuple->DeleteIfAllowed();
      continue;
    }

    int reg_id = ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                INDOORIF_REG_ID))->GetIntval();

    ////////////////////////////////////////////////////////
    vector<int> path_id_list;
    i_infra->GetPathIDFromTypeID(reg_id, path_id_list);
    if(path_id_list.size() == 0){ //no path available, no such a building
      build_tuple->DeleteIfAllowed();
      cout<<GetBuildingStr(type)<<" path not valid "<<endl;
      continue;
    }

   int build_id = ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_ID))->GetIntval();

    RefBuild ref_b(true, reg_id, build_id, type, *bbox, build_tid);
    build_tid2_list.push_back(ref_b);
    build_tuple->DeleteIfAllowed();

  }

}


/*
select buildings from two input relations, each of them store the builing id
in the original relation buiding stored in space 
the buildings should not be very far away from bus stops 
these buildings are to be selected for creating bus trips 

*/
void GenMObject::GetSelectedBuilding2(IndoorInfra* i_infra, 
                          vector<RefBuild>& build_tid1_list,
                          vector<RefBuild>& build_tid2_list, MaxRect* maxrect, 
                          Relation* rel1, Relation* rel2, Relation* rel3)
{
  map<int, int> tid_list;
  for(int i = 1;i <= rel3->GetNoTuples();i++){
    Tuple* tuple = rel3->GetTuple(i, false);
    int id = ((CcInt*)tuple->GetAttribute(Build_ID))->GetIntval();
    tid_list.insert(pair<int, int>(id, id));
    tuple->DeleteIfAllowed();
  }

  Relation* build_type_rel = i_infra->BuildingType_Rel();

  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple = rel1->GetTuple(i, false);
    int build_tid = ((CcInt*)tuple->GetAttribute(0))->GetIntval();
    assert(1 <= build_tid && build_tid <= build_type_rel->GetNoTuples());
    tuple->DeleteIfAllowed();

    map<int, int>::iterator iter = tid_list.find(build_tid);
    if(iter == tid_list.end()){
      continue;
    }

    Tuple* build_tuple = build_type_rel->GetTuple(build_tid, false);
    Rectangle<2>* bbox = 
       (Rectangle<2>*)build_tuple->GetAttribute(IndoorInfra::INDOORIF_GEODATA);
    int type =  ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_TYPE))->GetIntval();

//    cout<<GetBuildingStr(type)<<endl;
    if(type > BUILD_HOUSE && maxrect->build_pointer[type] == NULL){
      cout<<GetBuildingStr(type)<<" not valid "<<endl;
      build_tuple->DeleteIfAllowed();
      continue;
    }

    int reg_id = ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                INDOORIF_REG_ID))->GetIntval();

    ////////////////////////////////////////////////////////
    vector<int> path_id_list;
    i_infra->GetPathIDFromTypeID(reg_id, path_id_list);
    if(path_id_list.size() == 0){ //no path available, no such a building
      build_tuple->DeleteIfAllowed();
      cout<<GetBuildingStr(type)<<" path not valid "<<endl;
      continue;
    }

   int build_id = ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_ID))->GetIntval();

    RefBuild ref_b(true, reg_id, build_id, type, *bbox, build_tid);
    build_tid1_list.push_back(ref_b);
    build_tuple->DeleteIfAllowed();

  }
  
//  cout<<"build_tid1_list size "<<build_tid1_list.size()<<endl; 

  for(int i = 1;i <= rel2->GetNoTuples();i++){
    Tuple* tuple = rel2->GetTuple(i, false);
    int build_tid = ((CcInt*)tuple->GetAttribute(0))->GetIntval();
    assert(1 <= build_tid && build_tid <= build_type_rel->GetNoTuples());
    tuple->DeleteIfAllowed();

    map<int, int>::iterator iter = tid_list.find(build_tid);
    if(iter == tid_list.end()){
      continue;
    }

    Tuple* build_tuple = build_type_rel->GetTuple(build_tid, false);
    Rectangle<2>* bbox = 
       (Rectangle<2>*)build_tuple->GetAttribute(IndoorInfra::INDOORIF_GEODATA);
    int type =  ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_TYPE))->GetIntval();

//    cout<<GetBuildingStr(type)<<endl;
    if(type > BUILD_HOUSE && maxrect->build_pointer[type] == NULL){
      cout<<GetBuildingStr(type)<<" not valid "<<endl;
      build_tuple->DeleteIfAllowed();
      continue;
    }

    int reg_id = ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                INDOORIF_REG_ID))->GetIntval();

    ////////////////////////////////////////////////////////
    vector<int> path_id_list;
    i_infra->GetPathIDFromTypeID(reg_id, path_id_list);
    if(path_id_list.size() == 0){ //no path available, no such a building
      build_tuple->DeleteIfAllowed();
      cout<<GetBuildingStr(type)<<" path not valid "<<endl;
      continue;
    }

   int build_id = ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_ID))->GetIntval();

    RefBuild ref_b(true, reg_id, build_id, type, *bbox, build_tid);
    build_tid2_list.push_back(ref_b);
    build_tuple->DeleteIfAllowed();

  }

}

/*
select buildings from one input relation 

*/

void GenMObject::GetSelectedBuilding3(IndoorInfra* i_infra, 
                          vector<RefBuild>& build_tid_list,
                          MaxRect* maxrect, Relation* rel)
{

  Relation* build_type_rel = i_infra->BuildingType_Rel();

  for(int i = 1;i <= rel->GetNoTuples();i++){
    Tuple* tuple = rel->GetTuple(i, false);
    int build_tid = ((CcInt*)tuple->GetAttribute(0))->GetIntval();
    assert(1 <= build_tid && build_tid <= build_type_rel->GetNoTuples());
    tuple->DeleteIfAllowed();

    Tuple* build_tuple = build_type_rel->GetTuple(build_tid, false);
    Rectangle<2>* bbox = 
       (Rectangle<2>*)build_tuple->GetAttribute(IndoorInfra::INDOORIF_GEODATA);
    int type =  ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_TYPE))->GetIntval();

    if(type > BUILD_HOUSE && maxrect->build_pointer[type] == NULL){
      cout<<GetBuildingStr(type)<<" not valid "<<endl;
      build_tuple->DeleteIfAllowed();
      continue;
    }

    int reg_id = ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                INDOORIF_REG_ID))->GetIntval();

    ////////////////////////////////////////////////////////
    vector<int> path_id_list;
    i_infra->GetPathIDFromTypeID(reg_id, path_id_list);
    if(path_id_list.size() == 0){ //no path available, no such a building
      build_tuple->DeleteIfAllowed();
      cout<<GetBuildingStr(type)<<" path not valid "<<endl;
      continue;
    }

//    cout<<GetBuildingStr(type)<<endl;
   int build_id = ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_ID))->GetIntval();

    RefBuild ref_b(true, reg_id, build_id, type, *bbox, build_tid);
    build_tid_list.push_back(ref_b);
    build_tuple->DeleteIfAllowed();

  }

}

/*
select buildings from two input relations, each of them stores the builing id
in the original relation buiding stored in space 
flag: tru: consider train station  or not 

*/
void GenMObject::CreateBuildingPair5(
                          vector<RefBuild> b_list1,
                          vector<RefBuild> b_list2,
                          vector<RefBuild>& build_tid1_list,
                          vector<RefBuild>& build_tid2_list,  
                          int pair_no, bool flag)
{
   int b_no = 1;
   const double min_dist = 1000.0;//Euclidean distance
   if(flag){
      int station_id1 = 0;
      for(unsigned int i = 0; i < b_list1.size();i++){
        if(b_list1[i].type == BUILD_TRAINSTATION){
          station_id1 = i;
          break;
        }
      }
      /////////10 persent from train station ///////////////
      if(station_id1 > 0){
      int i = 1;
//      cout<<" pair_no * 0.15 "<<pair_no * 0.15<<endl;
      while(i <= pair_no * 0.3){
          int temp_id = GetRandom() % b_list2.size();

          RefBuild refb_1 = b_list1[station_id1];
          RefBuild refb_2 = b_list2[temp_id];
          if(refb_1.rect.Distance(refb_2.rect) > min_dist &&
             refb_2.type != BUILD_TRAINSTATION){

            build_tid1_list.push_back(refb_1);
            build_tid2_list.push_back(refb_2);
            b_no++;
            i++;
          }
      }
    }

      int station_id2 = 0;
      for(unsigned int i = 0; i < b_list2.size();i++){
        if(b_list2[i].type == BUILD_TRAINSTATION){
          station_id2 = i;
          break;
        }
      }

      if(station_id2 > 0){
          int i = 1;
          while(i <= pair_no * 0.3){
            int temp_id = GetRandom() % b_list1.size();

            RefBuild refb_1 = b_list1[temp_id];
            RefBuild refb_2 = b_list2[station_id2];
            if(refb_1.rect.Distance(refb_2.rect) > min_dist && 
              refb_1.type != BUILD_TRAINSTATION){
              build_tid1_list.push_back(refb_1);
              build_tid2_list.push_back(refb_2);
              b_no++;
              i++;
          }
        }
      }

   }

   /////////////////add train station ////////////////////
   while(b_no <= pair_no){
    int id1 = GetRandom() % b_list1.size();
    int id2 = GetRandom() % b_list2.size();

    RefBuild refb_1 = b_list1[id1];
    RefBuild refb_2 = b_list2[id2];
    if(refb_1.rect.Distance(refb_2.rect) > min_dist){
      build_tid1_list.push_back(refb_1);
      build_tid2_list.push_back(refb_2);
      b_no++;
    }
  }

//      for(unsigned int i = 0;i < build_tid1_list.size();i++)
//          cout<<GetBuildingStr(build_tid1_list[i].type)<<" "
//              <<GetBuildingStr(build_tid2_list[i].type)<<endl;


}

/*
create moving objects based on NN searching, start location in road network

*/
void GenMObject::GenerateGenMOBench3(Space* sp, Periods* peri, int mo_no, 
                                     Relation* build_rel, 
                                     R_Tree<2,TupleId>* rtree)
{
   if(mo_no < 1){
      cout<<" invalid number of moving objects "<<mo_no<<endl;
      return;
   }

  IndoorInfra* i_infra = sp->LoadIndoorInfra(IF_GROOM);

  if(i_infra == NULL){
    cout<<"indoor infrastructure does not exist "<<endl;
    return;
  }

  Network* rn = sp->LoadRoadNetwork(IF_LINE);
  if(rn == NULL){
    cout<<"road network loading error"<<endl;
    sp->CloseIndoorInfra(i_infra);
    return;
  }

  //////////////////////////////////////////////////////////////////
  ////////////////load all buildings and indoor graphs/////////////
  ////////////////// and indoor paths//////////////////////////////
  MaxRect* maxrect = new MaxRect();
  maxrect->OpenBuilding();
  maxrect->OpenIndoorGraph();

#ifdef INDOOR_PATH
  maxrect->LoadIndoorPaths(indoor_paths_list, rooms_id_list);
  assert(indoor_paths_list.size() == rooms_id_list.size());
#endif 

  ////////////////////////////////////////////////////////////////
  ////////////select a pair of buildings/////////////////////////
  //////////////////////////////////////////////////////////////

  vector<GPoint> gp_loc_list;
  vector<Point> p_loc_list;
  GenerateGPoint2(rn, obj_scale_min*mo_no, gp_loc_list, p_loc_list);

/*  for(unsigned int i = 0;i < gp_loc_list.size();i++)
    cout<<gp_loc_list[i]<<" "<<p_loc_list[i]<<endl;*/

  vector<RefBuild> nn_build_list;
  FindNNBuilding(p_loc_list, build_rel, rtree, i_infra->BuildingType_Rel(),
                 nn_build_list);

  ///////////////////open infrastructures/////////////////////////////
  Pavement* pm = sp->LoadPavement(IF_REGION);
  if(pm == NULL){
    cout<<"pavement loading error"<<endl;
    sp->CloseRoadNetwork(rn);
    sp->CloseIndoorInfra(i_infra);
    maxrect->CloseIndoorGraph();
    maxrect->CloseBuilding();
    delete maxrect;
    return;
  }

  RoadGraph* rg = sp->LoadRoadGraph();
  if(rg == NULL){
    cout<<"road graph loading error"<<endl;
    sp->CloseRoadNetwork(rn);
    sp->ClosePavement(pm);
    sp->CloseIndoorInfra(i_infra);
    maxrect->CloseIndoorGraph();
    maxrect->CloseBuilding();
    delete maxrect;
    return;
  }

  //////////////////////Initialization/////////////////////////////

   Relation* dg_node_rel = sp->GetDualNodeRel();
   BTree* btree = sp->GetDGNodeBTree();
   Relation* speed_rel = sp->GetSpeedRel();

   if(dg_node_rel == NULL || btree == NULL || speed_rel == NULL){
      cout<<"auxiliary relation empty IWCTB"<<endl;
      sp->CloseRoadGraph(rg);
      sp->CloseRoadNetwork(rn);
      sp->ClosePavement(pm);
      sp->CloseIndoorInfra(i_infra);
      maxrect->CloseIndoorGraph();
      maxrect->CloseBuilding();
      delete maxrect;

      return;
   }
  //////////////////////////////////////////////////////////////////////////


  RoadNav* road_nav = new RoadNav();
  ///////////////////////////////////////////////////////////////////
  Relation* build_path_rel = i_infra->BuildingPath_Rel();
  ///////////////start time///////////////////////////////////////
  Interval<Instant> periods;
  peri->Get(0, periods);
  Instant start_time = periods.start;
  
  int h1 = periods.start.GetHour();
  int h2 = periods.end.GetHour();
  int time_range = (h2 - h1)*60;

  ////////////////////////////////////////////////////////////
  int count_tmp = 0;
  const double min_path = 0.1;
  int real_count = 1;
  while(real_count <= mo_no && count < obj_scale_min*mo_no &&
         count_tmp < (int)nn_build_list.size()){


   //////////////////////////////start time///////////////////////////
   if(count_tmp % 2 == 0)
     start_time.ReadFrom(periods.start.ToDouble() + 
                        (GetRandom() % time_range)/(24.0*60.0));
   else
     start_time.ReadFrom(periods.end.ToDouble() -
                        (GetRandom() % time_range)/(24.0*60.0));

//    cout<<"start time "<<start_time<<endl;

    /////////////////////load all paths from this building////////////////
    vector<int> path_id_list;
    i_infra->GetPathIDFromTypeID(nn_build_list[count_tmp].reg_id, path_id_list);



   ////////////////////////////////////////////////////////////////////
   //////////////if a building has several entrances///////////////////
   ///////////it randomly selects an entrance  ///////////////////////
   ////////////the buildings are far away from each///////////////////
   //////////which entrance to go out does not influence the distance a lot///
   ////////////////////////////////////////////////////////////////////
   if(path_id_list.size() == 0){
     count_tmp++;
     continue;
   }


   int path_tid = path_id_list[GetRandom() % path_id_list.size()];

   Tuple* path_tuple = build_path_rel->GetTuple(path_tid, false);

    /////////////////////////////////////////////////////////////////////
    //////////////////1 get buildings and two positions for network /////
    ////////////////// the end point of path in pavement/////////////////
    ////////////////////////////////////////////////////////////////////

    GenLoc* gloc2 = 
        (GenLoc*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
    Point* ep2_1 = (Point*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_EP);
    Point* ep2_2 = (Point*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_EP2);

    /////////////reset the general location//////////////////////
    Loc loc2(ep2_2->GetX(), ep2_2->GetY());
    GenLoc newgloc2(gloc2->GetOid(), loc2);


    /////////////////////////////////////////////////////////////////////
    //////////////////2 map positions to gpoints////////////////////
    /////////////////get the start and end location of the trip/////////
    ////////////////////////////////////////////////////////////////////
    vector<GPoint> gpoint_list;
    vector<Point> p_list;
    bool correct;
    PaveLoc2GPoint(newgloc2, newgloc2, sp, dg_node_rel,
                   btree, gpoint_list, p_list, correct, rn);
    if(correct == false){
        count_tmp++;
        path_tuple->DeleteIfAllowed();
        continue;
    }

    GPoint gp1 = gp_loc_list[count_tmp];
    GPoint gp2 = gpoint_list[1];
    Point start_loc = p_loc_list[count_tmp];
    Point end_loc = p_list[1];
    
    if(start_loc.Distance(end_loc) < min_path){
        count_tmp++;
        path_tuple->DeleteIfAllowed();
        continue;
    }

    MPoint* mo = new MPoint(0);
    GenMO* genmo = new GenMO(0);
    mo->StartBulkLoad();
    genmo->StartBulkLoad();


    /////////////////////////////////////////////////////////////////////
    //////////////////5 road network movement //////////////////////////
    ////////////////////////////////////////////////////////////////////
    GLine* gl = new GLine(0);
    road_nav->ShortestPathSub(&gp1, &gp2, rg, rn, gl);
    ConnectGP1GP2(rn, start_loc, gl, mo, genmo, start_time, speed_rel, "Car");
    delete gl;

    /////////////////////////////////////////////////////////////////////
    //////////////////6 end network location to pavement//////////////////
    ////////////////////////////////////////////////////////////////////
    ConnectEndMove(end_loc, newgloc2, mo, genmo, start_time, pm);

    /////////////////////////////////////////////////////////////////////
    ////////7 pavement to building entrance + indoor movement2//////////
    ////////////////////////////////////////////////////////////////////

    ////////////////outdoor movement/////////////////////////////////////////
    Line* path2 = (Line*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_PATH);
    GenerateFreeMovement2(*ep2_2, *ep2_1, genmo, mo, start_time);

    if(path2->Length() > min_path)
      GenerateFreeMovement(path2, *ep2_1, genmo, mo, start_time);


    ///////////////////indoor movement//////////////////////////////////////
    ////////////////////to show which entrance it is////////////////////
    Point* sp2 = (Point*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_SP);
    int entrance_index2 = ((CcInt*)path_tuple->GetAttribute(IndoorInfra::
                                   INDOORIF_SP_INDEX))->GetIntval();
    int reg_id2 =  ((CcInt*)path_tuple->GetAttribute(IndoorInfra::
                                              INDOORIF_REG_ID))->GetIntval();
    GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                   entrance_index2, reg_id2, maxrect, peri,
                                    TIMESPAN_NO);

    mo->EndBulkLoad();
    genmo->EndBulkLoad(false, false);

    trip1_list.push_back(*genmo);
    trip2_list.push_back(*mo);

//    loc_list1.push_back(p_loc_list[count_tmp]);
//    rect_list1.push_back(nn_build_list[count_tmp].rect);

    delete mo; 
    delete genmo; 

    path_tuple->DeleteIfAllowed();   
   //////////////////////////////////////////////////////////////////////
//    cout<<"building  "<<GetBuildingStr(nn_build_list[count_tmp].type)<<endl;

    count_tmp++;
    cout<<real_count<<" generic moving object"<<endl;

    real_count++;
  }


  delete road_nav;
 //////////////////////////////////////////////////////////////////////////
  maxrect->CloseIndoorGraph();
  maxrect->CloseBuilding();
  delete maxrect;

  sp->CloseRoadGraph(rg);
  sp->ClosePavement(pm);
  sp->CloseRoadNetwork(rn);
  sp->CloseIndoorInfra(i_infra);
}

/*
for each network point, it finds the nn building

*/
void GenMObject::FindNNBuilding(vector<Point> p_loc_list,
                                Relation* build_rel, R_Tree<2,TupleId>* rtree,
                                Relation* build_type_rel,
                                vector<RefBuild>& nn_build_list)
{

  vector<int> nn_build_tid;
  SmiRecordId adr = rtree->RootRecordId();
  for(unsigned int i = 0; i < p_loc_list.size();i++){
    Point q_loc = p_loc_list[i];

    int b_tid = -1;
    double min_dist= numeric_limits<double>::max();
    bool dist_init = false;
    NNBTraverse(rtree, adr, build_rel, q_loc, b_tid, 
                        min_dist, dist_init);
    assert(1 <= b_tid && b_tid <= build_rel->GetNoTuples());
  
//    cout<<b_tid<<endl;
    nn_build_tid.push_back(b_tid);
  }

  for(unsigned int i = 0;i < nn_build_tid.size();i++){
//     loc_list1.push_back(p_loc_list[i]);
     Tuple* b_tuple = build_rel->GetTuple(nn_build_tid[i], false);
     int b_tid = ((CcInt*)b_tuple->GetAttribute(BM_NNB_ID))->GetIntval();
//  Rectangle<2>* b_area = (Rectangle<2>*)b_tuple->GetAttribute(BM_NNB_GEODATA);
//     rect_list1.push_back(*b_area);
     b_tuple->DeleteIfAllowed();

    Tuple* tuple = build_type_rel->GetTuple(b_tid, false);

    Rectangle<2>* bbox = 
        (Rectangle<2>*)tuple->GetAttribute(IndoorInfra::INDOORIF_GEODATA);
    int type = ((CcInt*)tuple->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_TYPE))->GetIntval();
    assert(type > BUILD_HOUSE);
    int reg_id = ((CcInt*)tuple->GetAttribute(IndoorInfra::
                INDOORIF_REG_ID))->GetIntval();

    int build_id = ((CcInt*)tuple->GetAttribute(IndoorInfra::
                INDOORIF_BUILD_ID))->GetIntval();

    RefBuild ref_b(true, reg_id, build_id, type, *bbox, b_tid);
    nn_build_list.push_back(ref_b);
    tuple->DeleteIfAllowed();

  }

}

/*
find the closest building to the query point 

*/
void GenMObject::NNBTraverse(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                    Relation* build_rel, Point q_loc, int& b_tid,
                    double& min_dist, bool& dist_init)
{
  const double NN_dist = 1.0;
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* tuple = build_rel->GetTuple(e.info, false);
              Rectangle<2>* rect = 
                (Rectangle<2>*)tuple->GetAttribute(BM_NNB_GEODATA);

              if(dist_init == false){
                dist_init = q_loc.Distance(*rect);
                if(dist_init > NN_dist){//////larger than a value 
                  dist_init = true;
                  b_tid = e.info;
                }
              }else{
                if(q_loc.Distance(*rect) < min_dist){
                  b_tid = e.info;
                  min_dist = q_loc.Distance(*rect);
                }
              }
              tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(dist_init == false)
               NNBTraverse(rtree, e.pointer, build_rel, 
                           q_loc, b_tid, min_dist, dist_init);
            else{
                if(q_loc.Distance(e.box) < min_dist){
                  NNBTraverse(rtree, e.pointer, build_rel, q_loc, b_tid,
                              min_dist, dist_init);
                }
            }
      }
  }
  delete node;

}



/*
create moving objects based on NN searching, 
start location in region-based outdoor, use taxi, bus or metro 

*/
void GenMObject::GenerateGenMOBench4(Space* sp, Periods* peri, int mo_no, 
                               Relation* para_rel,
                               Relation* build_rel, R_Tree<2,TupleId>* rtree)
{

    if(mo_no < 1){
      cout<<" invalid number of moving objects "<<mo_no<<endl;
      return;
    }

  ///////////////////////////////////////////////////////////////
  //////////  Initialization open infrastructures  /////////////
  //////////////////////////////////////////////////////////////

  IndoorInfra* i_infra = sp->LoadIndoorInfra(IF_GROOM);
  if(i_infra == NULL){
      cout<<"indoor infrastructure does not exist "<<endl;
     return;
  }

  Pavement* pm = sp->LoadPavement(IF_REGION);
  if(pm == NULL){
    cout<<"pavement loading error"<<endl;
    sp->CloseIndoorInfra(i_infra);
    return;
  }
  
  //////////////////////////////////////////////////////////////////
  ////////////////load all buildings and indoor graphs/////////////
  ////////////////// and indoor paths//////////////////////////////
  MaxRect* maxrect = new MaxRect();
  maxrect->OpenBuilding();
  maxrect->OpenIndoorGraph();

#ifdef INDOOR_PATH
  maxrect->LoadIndoorPaths(indoor_paths_list, rooms_id_list);
  assert(indoor_paths_list.size() == rooms_id_list.size());
#endif 

  for(int i = 1;i <= para_rel->GetNoTuples();i++){
      Tuple* dis_tuple = para_rel->GetTuple(i, false);
      float dis_para = 
          ((CcReal*)dis_tuple->GetAttribute(BENCH_PARA))->GetRealval();
      int genmo_no = (int)(ceil(mo_no*dis_para));
      string mode = 
        ((CcString*)dis_tuple->GetAttribute(BENCH_MODE))->GetValue();

      if(GetTM(mode) == TM_TAXI){ ///by taxi 
//        cout<<"taxi "<<genmo_no<<endl;
        vector<GenLoc> genloc_list;
        vector<Point> p_loc_list;
       //generate locations on pavements
       GenerateLocPave(pm, obj_scale_min*genmo_no, genloc_list, p_loc_list);
       ///////get nn building //////////////
       vector<RefBuild> nn_build_list;
       FindNNBuilding(p_loc_list, build_rel, rtree, 
                      i_infra->BuildingType_Rel(), nn_build_list);

//        for(unsigned int j = 0;j < p_loc_list.size();j++){
//           loc_list1.push_back(p_loc_list[j]);
//           rect_list1.push_back(nn_build_list[j].rect);
//        }


         GenerateBench4_Taxi(sp, i_infra, maxrect, pm, genmo_no, peri, 
                             genloc_list, p_loc_list, nn_build_list);


      }else if(GetTM(mode) == TM_BUS){
//        cout<<"bus "<<genmo_no<<endl;
        vector<GenLoc> genloc_list;
        vector<Point> p_loc_list;
       //generate locations on pavements
//       GenerateLocPave(pm, obj_scale*genmo_no, genloc_list, p_loc_list);
       GenerateLocPave(pm, obj_scale_max*genmo_no, genloc_list, p_loc_list);

       vector<RefBuild> nn_build_list;
       FindNNBuilding(p_loc_list, build_rel, rtree, 
                      i_infra->BuildingType_Rel(), nn_build_list);

       GenerateBench4_Bus(sp, i_infra, maxrect, pm, genmo_no, peri, 
                          genloc_list, p_loc_list, nn_build_list);

      }else{

        cout<<"invalid type "<<endl;
      }

       dis_tuple->DeleteIfAllowed();
  }


  maxrect->CloseIndoorGraph();
  maxrect->CloseBuilding();
  delete maxrect;
  sp->ClosePavement(pm);
  sp->CloseIndoorInfra(i_infra);
}


/*
a pedestrian on the pavement goes to the nn building by taxi 

*/
void GenMObject::GenerateBench4_Taxi(Space* sp, IndoorInfra* i_infra, 
                                     MaxRect* maxrect,
                                     Pavement* pm, 
                                     int mo_no, Periods* peri, 
                                     vector<GenLoc> genloc_list, 
                                     vector<Point> p_loc_list,
                                     vector<RefBuild> nn_build_list)
{
  Network* rn = sp->LoadRoadNetwork(IF_LINE);
  if(rn == NULL){
    cout<<"road network loading error"<<endl;
    return;
  }

  RoadGraph* rg = sp->LoadRoadGraph();
  if(rg == NULL){
    cout<<"road graph loading error"<<endl;
    sp->CloseRoadNetwork(rn);
    return;
  }
  DualGraph* dg = pm->GetDualGraph();
  VisualGraph* vg = pm->GetVisualGraph();
    
  if(dg == NULL || vg == NULL){
    cout<<"dual graph or visual graph error "<<endl;
    sp->CloseRoadGraph(rg);
    sp->CloseRoadNetwork(rn);
    return;
  }
  
  int no_triangle = dg->GetNodeRel()->GetNoTuples();
  int mini_oid = dg->min_tri_oid_1;
  //////////////////////Initialization/////////////////////////////

   Relation* dg_node_rel = sp->GetDualNodeRel();
   BTree* btree = sp->GetDGNodeBTree();
   Relation* speed_rel = sp->GetSpeedRel();

   if(dg_node_rel == NULL || btree == NULL || speed_rel == NULL){
      cout<<"auxiliary relation empty IWCTB"<<endl;
      sp->CloseRoadGraph(rg);
      sp->CloseRoadNetwork(rn);
      return;
   }

  //////////////////////////////////////////////////////////////////////////
  RoadNav* road_nav = new RoadNav();
  
   ///////////////////////////////////////////////////////////////////
  Relation* build_path_rel = i_infra->BuildingPath_Rel();
  ///////////////start time///////////////////////////////////////
  Interval<Instant> periods;
  peri->Get(0, periods);
  Instant start_time = periods.start;
  
  int h1 = periods.start.GetHour();
  int h2 = periods.end.GetHour();
  int time_range = (h2 - h1)*60;
  
  ////////////////////////////////////////////////////////////
  int count_tmp = 0;
  const double min_path = 0.1;
  int real_count = 1;

  while(real_count <= mo_no && count < obj_scale_min*mo_no &&
         count_tmp < (int)nn_build_list.size()){

   //////////////////////////////start time///////////////////////////
   if(count_tmp % 2 == 0)
     start_time.ReadFrom(periods.start.ToDouble() + 
                        (GetRandom() % time_range)/(24.0*60.0));
   else
     start_time.ReadFrom(periods.end.ToDouble() -
                        (GetRandom() % time_range)/(24.0*60.0));

//    cout<<"start time "<<start_time<<endl;
    /////////////////////load all paths from this building////////////////
    vector<int> path_id_list;
    i_infra->GetPathIDFromTypeID(nn_build_list[count_tmp].reg_id, path_id_list);

   ////////////////////////////////////////////////////////////////////
   //////////////if a building has several entrances///////////////////
   ///////////it randomly selects an entrance  ///////////////////////
   ////////////the buildings are far away from each///////////////////
   //////////which entrance to go out does not influence the distance a lot///
   ////////////////////////////////////////////////////////////////////
   if(path_id_list.size() == 0){
     count_tmp++;
     continue;
   }

   int path_tid = path_id_list[GetRandom() % path_id_list.size()];

   GenLoc loc1 = genloc_list[count_tmp];
    ////////////////////////////////////////////////////////////////////////
    /////////////1 map the pavement position to gpoint/////////////////////////
    ///////////////////////////////////////////////////////////////////////
    vector<GPoint> gpoint_list1;
    vector<Point> p_list1;
    bool correct1;
    PaveLoc2GPoint(loc1, loc1, sp, dg_node_rel, btree, gpoint_list1, p_list1, 
                   correct1, rn);
    if(correct1 == false){
      count_tmp++;
      continue;
    }

   Tuple* path_tuple = build_path_rel->GetTuple(path_tid, false);

    /////////////////////////////////////////////////////////////////////
    //////////////////2 get buildings and two positions for network /////
    ////////////////// the end point of path in pavement/////////////////
    ////////////////////////////////////////////////////////////////////

    GenLoc* gloc2 = 
        (GenLoc*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
    Point* ep2_1 = (Point*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_EP);
    Point* ep2_2 = (Point*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_EP2);

    /////////////reset the general location//////////////////////
    Loc loc2(ep2_2->GetX(), ep2_2->GetY());
    GenLoc newgloc2(gloc2->GetOid(), loc2);


    /////////////////////////////////////////////////////////////////////
    //////////////////3 map positions to gpoints////////////////////
    /////////////////get the start and end location of the trip/////////
    ////////////////////////////////////////////////////////////////////
    vector<GPoint> gpoint_list2;
    vector<Point> p_list2;
    bool correct2;
    PaveLoc2GPoint(newgloc2, newgloc2, sp, dg_node_rel,
                   btree, gpoint_list2, p_list2, correct2, rn);
    if(correct2 == false){
        count_tmp++;
        path_tuple->DeleteIfAllowed();
        continue;
    }


    GPoint gp1 = gpoint_list1[0];
    GPoint gp2 = gpoint_list2[1];
    Point start_loc = p_list1[0];
    Point end_loc = p_list2[1];


    MPoint* mo = new MPoint(0);
    GenMO* genmo = new GenMO(0);
    mo->StartBulkLoad();
    genmo->StartBulkLoad();

    //////////////////////////////////////////////////////////////////////
    ////////////4 from Region based Outdoor to Road Network Point ////////
    //////////////////////////////////////////////////////////////////////
    if(start_loc.Distance(end_loc) < BENCH_NN_DIST){//small, directly by walk 
        Walk_SP* wsp = new Walk_SP(dg, vg, NULL, NULL);
        wsp->rel3 = sp->GetNewTriRel();


        Point p1 = p_loc_list[count_tmp];
        Point p2 = *ep2_2;


        int oid1 = loc1.GetOid() - mini_oid;
        int oid2 = newgloc2.GetOid() - mini_oid;

        assert(1 <= oid1 && oid1 <= no_triangle);
        assert(1 <= oid2 && oid2 <= no_triangle);

        Line* w_path = new Line(0);

        ///////////////////walk segment////////////////////////////////////
        wsp->WalkShortestPath2(oid1, oid2, p1, p2, w_path);

        ///////////create moving objects///////////////////////////////////
        if(w_path->IsDefined() && w_path->Length() > min_path)
           GenerateWalkMovement(dg, w_path, p1, genmo, mo, start_time);

        delete w_path; 

        delete wsp;

    }else{////////long distance, by taxi 

    ConnectStartMove(loc1, start_loc, mo, genmo, start_time, pm);

    /////////////////////////////////////////////////////////////////////
    //////////////////5 road network movement //////////////////////////
    ////////////////////////////////////////////////////////////////////
    GLine* gl = new GLine(0);
    road_nav->ShortestPathSub(&gp1, &gp2, rg, rn, gl);
    ConnectGP1GP2(rn, start_loc, gl, mo, genmo, start_time, speed_rel, "Taxi");
    delete gl;

    /////////////////////////////////////////////////////////////////////
    //////////////////6 end network location to pavement//////////////////
    ////////////////////////////////////////////////////////////////////
    ConnectEndMove(end_loc, newgloc2, mo, genmo, start_time, pm);

    }

    /////////////////////////////////////////////////////////////////////
    ////////7 pavement to building entrance + indoor movement2//////////
    ////////////////////////////////////////////////////////////////////

    ////////////////outdoor movement/////////////////////////////////////////
    GenerateFreeMovement2(*ep2_2, *ep2_1, genmo, mo, start_time);

    Line* path2 = (Line*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_PATH);
    if(path2->Length() > min_path)
      GenerateFreeMovement(path2, *ep2_1, genmo, mo, start_time);


    ///////////////////indoor movement//////////////////////////////////////
    ////////////////////to show which entrance it is////////////////////
    Point* sp2 = (Point*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_SP);
    int entrance_index2 = ((CcInt*)path_tuple->GetAttribute(IndoorInfra::
                                   INDOORIF_SP_INDEX))->GetIntval();
    int reg_id2 =  ((CcInt*)path_tuple->GetAttribute(IndoorInfra::
                                              INDOORIF_REG_ID))->GetIntval();
    GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                   entrance_index2, reg_id2, maxrect, peri,
                                    TIMESPAN_NO);


    mo->EndBulkLoad();
    genmo->EndBulkLoad(false, false);

    trip1_list.push_back(*genmo);
    trip2_list.push_back(*mo);

//    loc_list1.push_back(p_loc_list[count_tmp]);
//    rect_list1.push_back(nn_build_list[count_tmp].rect);

    delete mo; 
    delete genmo; 

    path_tuple->DeleteIfAllowed(); 

    //////////////////////////////////////////////////////////////////////
//    cout<<"building  "<<GetBuildingStr(nn_build_list[count_tmp].type)<<endl;

    count_tmp++;
    cout<<real_count<<" generic moving object"<<endl;

    real_count++;

  }

  delete road_nav;
  sp->CloseRoadGraph(rg);
  sp->CloseRoadNetwork(rn);
  pm->CloseDualGraph(dg);
  pm->CloseVisualGraph(vg);
}



/*
a pedestrian on the pavement goes to the nn building by bus 

*/
void GenMObject::GenerateBench4_Bus(Space* sp, IndoorInfra* i_infra, 
                                     MaxRect* maxrect,
                                     Pavement* pm, 
                                     int mo_no, Periods* peri_input, 
                                     vector<GenLoc> genloc_list, 
                                     vector<Point> p_loc_list,
                                     vector<RefBuild> nn_build_list)
{
  BusNetwork* bn = sp->LoadBusNetwork(IF_BUSNETWORK);
  if(bn == NULL){
    cout<<"open bus network error "<<endl;
    return;
  }

  DualGraph* dg = pm->GetDualGraph();
  VisualGraph* vg = pm->GetVisualGraph();
  if(dg == NULL || vg == NULL){
    cout<<"dual graph or visual graph open error "<<endl;
    sp->CloseBusNetwork(bn);
    return;
  }

  int no_triangle = dg->GetNodeRel()->GetNoTuples();
  int mini_oid = dg->min_tri_oid_1;

  Relation* rel1 = sp->GetNewTriRel();
  Relation* rel2 = sp->GetBSPaveRel();
  Relation* rel3 = sp->GetBSBuildRel();
  R_Tree<2,TupleId>* rtree = sp->GetBSPaveRtree();
  if(rel1 == NULL || rel2 == NULL || rtree == NULL || rel3 == NULL){
      cout<<"auxiliary relation empty IBW"<<endl;
      return;
   }

  ///////////////////////////////////////////////////////////////////
  Relation* build_path_rel = i_infra->BuildingPath_Rel();
  //////////////////////////////////////////////////////////////////
  ///////////////start time///////////////////////////////////////
  Interval<Instant> periods;
  peri_input->Get(0, periods);
  Instant start_time = periods.start;
  
  int h1 = periods.start.GetHour();
  int h2 = periods.end.GetHour();
  int time_range = (h2 - h1)*60;

  ////////////////////////////////////////////////////////////
  
   int obj_no_rep = 0;
   int max_obj = 1;
   if(mo_no <= 500) max_obj = 2;
   else if(mo_no <= 1000) max_obj = 3;
   else if(mo_no <= 5000) max_obj = 4;
   else max_obj = 5;

  int count_tmp = 0; 
  int real_count = 1;

  const double min_path = 0.1;

   ////////////////////////////////////////////////////
   //////////bus network time periods ////////////////
   //////////////////////////////////////////////////
   BusGraph* bg = bn->GetBusGraph(); 
   if(bg == NULL){
    cout<<"bus graph is invalid"<<endl; 
    return;
   }
   Instant bs_start(bn->GetStartTime());
   Instant bs_end(bn->GetEndTime());
   Instant bg_min(instanttype);
   bg_min.ReadFrom(bg->min_t);
   bn->CloseBusGraph(bg);
   while(real_count <= mo_no && count_tmp < obj_scale*mo_no){

   //////////////////////////////start time///////////////////////////
   if(count_tmp % 2 == 0) //less movement on sunday 
     start_time.ReadFrom(periods.start.ToDouble() + 
                        (GetRandom() % time_range)/(24.0*60.0));
   else
     start_time.ReadFrom(periods.end.ToDouble() -
                        (GetRandom() % time_range)/(24.0*60.0));

   ////////////////////////////////////////////////////////////////////
    bool time_transform = false;
    int day_deviation = 0;
    Instant old_st(start_time);


    Periods* peri = new Periods(0);
    peri->StartBulkLoad();
    if(start_time < bs_start || start_time > bs_end){
        time_transform = true;

        if(start_time.GetWeekday() == 6){
          start_time.Set(bg_min.GetYear(), bg_min.GetMonth(), 
                         bg_min.GetGregDay(), 
                         start_time.GetHour(), start_time.GetMinute(), 
                         start_time.GetSecond(), start_time.GetMillisecond());

        }else{ //Monday-Saturday 
          start_time.Set(bg_min.GetYear(), bg_min.GetMonth(), 
                         bg_min.GetGregDay() + 1,
                         start_time.GetHour(), start_time.GetMinute(), 
                         start_time.GetSecond(), start_time.GetMillisecond());

        }
        day_deviation = old_st.GetDay() - start_time.GetDay();
//        cout<<" day deviation "<<day_deviation<<endl;

        Interval<Instant> time_span;
        time_span.start = periods.start;
        time_span.start.ReadFrom(periods.start.ToDouble() - day_deviation);
        time_span.lc = true;
        time_span.end = periods.end;
        time_span.end.ReadFrom(periods.end.ToDouble() - day_deviation);
        time_span.rc = false;
        peri->MergeAdd(time_span);
    }

    peri->EndBulkLoad();

//    cout<<"start time "<<start_time<<endl;
    int DAY1 = start_time.GetDay();
    /////////////////////load all paths from this building////////////////
   vector<int> path_id_list;
   i_infra->GetPathIDFromTypeID(nn_build_list[count_tmp].reg_id, path_id_list);

   int path_tid = path_id_list[GetRandom() % path_id_list.size()];
   GenLoc loc1 = genloc_list[count_tmp];
   ////////////////////////////////////////////////////////////////////
   //////////////if a building has several entrances///////////////////
   ///////////it randomly selects an entrance  ///////////////////////
   ////////////the buildings are far away from each///////////////////
   //////////which entrance to go out does not influence the distance a lot///
   ////////////////////////////////////////////////////////////////////
   if(path_id_list.size() == 0){
     count_tmp++;
     delete peri;
     continue;
   }

    Tuple* path_tuple = build_path_rel->GetTuple(path_tid, false);

    /////////////////////////////////////////////////////////////////////
    //////////////////2 get buildings and two positions for network /////
    ////////////////// the end point of path in pavement/////////////////
    ////////////////////////////////////////////////////////////////////

    GenLoc* build_gloc2 = 
        (GenLoc*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
    Point* ep2_1 = (Point*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_EP);
    Point* ep2_2 = (Point*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_EP2);

    /////////////reset the general location//////////////////////
    Loc loc2(ep2_2->GetX(), ep2_2->GetY());
    GenLoc newgloc2(build_gloc2->GetOid(), loc2);
    ///////////////////////////////////////////////////////////////
    MPoint* mo = new MPoint(0);
    GenMO* genmo = new GenMO(0);
    mo->StartBulkLoad();
    genmo->StartBulkLoad();

    //small distance, by walk 
    if(ep2_2->Distance(p_loc_list[count_tmp]) < BENCH_NN_DIST){

        Walk_SP* wsp = new Walk_SP(dg, vg, NULL, NULL);
        wsp->rel3 = rel1;

        Point p1 = p_loc_list[count_tmp];
        Point p2 = *ep2_2;

        int oid1 = loc1.GetOid() - mini_oid;
        int oid2 = newgloc2.GetOid() - mini_oid;

        assert(1 <= oid1 && oid1 <= no_triangle);
        assert(1 <= oid2 && oid2 <= no_triangle);

        Line* w_path = new Line(0);

        ///////////////////walk segment////////////////////////////////////
        wsp->WalkShortestPath2(oid1, oid2, p1, p2, w_path);

        ///////////create moving objects///////////////////////////////////
        if(w_path->IsDefined() && w_path->Length() > min_path)
           GenerateWalkMovement(dg, w_path, p1, genmo, mo, start_time);

        delete w_path; 

        delete wsp;

    }else{

    //////////////////////////////////////////////////////////////////////////
    //////find closest bus stops to the two locations on the pavement/////////
    Bus_Stop bs1(true, 0, 0, true);
    vector<Point> ps_list1;//1 point on the pavement 2 point of bus stop
    GenLoc gloc1(0, Loc(0, 0));//bus stop on pavement by genloc 
    bool b1 = true;
    b1 = NearestBusStop(genloc_list[count_tmp],  rel2, rtree, bs1, 
                        ps_list1, gloc1, true);

    Bus_Stop bs2(true, 0, 0, true);
    vector<Point> ps_list2;//1. point on the pavement 2. point of bus stop
    GenLoc gloc2(0, Loc(0, 0));//bus stop on pavement by genloc
    bool b2 = true;
    b2 = NearestBusStop(newgloc2,  rel2, rtree, bs2, ps_list2, gloc2, false);


    if((b1 && b2) == false){
      path_tuple->DeleteIfAllowed();
      count_tmp++;
      delete peri;
      continue;
    }

    Point start_p, end_p; 
    bn->GetBusStopGeoData(&bs1, &start_p);
    bn->GetBusStopGeoData(&bs2, &end_p);

    if(start_p.Distance(end_p) < min_path){
      path_tuple->DeleteIfAllowed();
      count_tmp++;
      delete peri;
      continue;
    }

//     loc_list1.push_back(p_loc_list[count_tmp]);////
//     loc_list2.push_back(start_p);//start bus stop 
//     rect_list1.push_back(nn_build_list[count_tmp].rect); 
//     loc_list3.push_back(end_p);///end bus stop

    
    ///////////////////////////////////////////////////////////////////
    ////////////////2. path (walk + bus)///////////////////////////////
    ///////////////////////////////////////////////////////////////////

    ////////2.1 connect from pavement to start bus stop////////////////////
    Line* res_path = new Line(0);
    ConnectStartStop(dg, vg, rel1, loc1, ps_list1, gloc1.GetOid(),
                            genmo, mo, start_time, res_path);

    /////////////////////////////////////////////////////////////////
    //////2.2. get the path in bus network////////////////////////////
    /////////////////////////////////////////////////////////////////
    BNNav* bn_nav = new BNNav(bn);

     if(count % 2 != 0) 
         bn_nav->ShortestPath_Time2(&bs1, &bs2, &start_time);
     else bn_nav->ShortestPath_Transfer2(&bs1, &bs2, &start_time);

    if(bn_nav->path_list.size() == 0 || (DAY1 != start_time.GetDay())){
//        cout<<"two unreachable bus stops"<<endl;
        mo->EndBulkLoad();
        genmo->EndBulkLoad();

        delete mo;
        delete genmo;
        delete bn_nav;
        delete res_path;
        delete peri;
        path_tuple->DeleteIfAllowed();
        count_tmp++;
        continue;
    }

      Instant temp_end(instanttype);
      temp_end.ReadFrom(start_time.ToDouble() + bn_nav->t_cost/86400.0);
      if(temp_end > bs_end){//not in the bus time schedule 
            mo->EndBulkLoad();
            genmo->EndBulkLoad();

            delete mo;
            delete genmo;
            delete bn_nav;
            delete res_path;
            delete peri;
            path_tuple->DeleteIfAllowed();
            count++;
            continue;
        }

     /////////////////////////////////////////////////////////////////////
     int last_walk_id = ConnectTwoBusStops(bn_nav, ps_list1[1], ps_list2[1],
                           genmo, mo, start_time, dg, res_path);
    ///////////////////////////////////////////////////////////
    if(last_walk_id > 0){
          //////change the last bus stop/////////
          ///change  ps list2, gloc2.oid ///
          Bus_Stop cur_bs1(true, 0, 0, true);
          StringToBusStop(bn_nav->bs1_list[last_walk_id], 
                          cur_bs1);
          ChangeEndBusStop(bn, dg, cur_bs1, ps_list2, gloc2, rel2, rtree);
    }

    ////////////////////////////////////////////////////////////
    delete bn_nav;

    /////////////////2.3 connect from end bus stop to pavement///////////
    ConnectEndStop(dg, vg, rel1, newgloc2, ps_list2, gloc2.GetOid(),
                          genmo, mo, start_time, res_path);
//    cout<<"path3 length "<<res_path->Length()<<endl;

     ////////////////////////////////////////////////////////////////////

    delete res_path; 

    }

    /////////////////////////////////////////////////////////////////////
    ////////7 pavement to building entrance + indoor movement2//////////
    ////////////////////////////////////////////////////////////////////

    ////////////////outdoor movement/////////////////////////////////////////
    GenerateFreeMovement2(*ep2_2, *ep2_1, genmo, mo, start_time);

    Line* path2 = (Line*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_PATH);
    if(path2->Length() > min_path)
      GenerateFreeMovement(path2, *ep2_1, genmo, mo, start_time);

    ///////////////////indoor movement//////////////////////////////////////
    ////////////////////to show which entrance it is////////////////////
    Point* sp2 = (Point*)path_tuple->GetAttribute(IndoorInfra::INDOORIF_SP);
    int entrance_index2 = ((CcInt*)path_tuple->GetAttribute(IndoorInfra::
                                   INDOORIF_SP_INDEX))->GetIntval();
    int reg_id2 =  ((CcInt*)path_tuple->GetAttribute(IndoorInfra::
                                              INDOORIF_REG_ID))->GetIntval();

    if(time_transform)
      GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                  entrance_index2, reg_id2, maxrect, peri, 
                                      TIMESPAN_NO);
    else
      GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                               entrance_index2, reg_id2, maxrect, peri_input,
                                      TIMESPAN_NO);


    mo->EndBulkLoad();
    genmo->EndBulkLoad(false, false);

     //////////////////////////////////change the time ///////////////////////
     if(time_transform){
            MPoint* mo_tmp = new MPoint(0);
            GenMO* genmo_tmp = new GenMO(0);
            mo_tmp->StartBulkLoad();
            genmo_tmp->StartBulkLoad();
            for(int i = 0;i < mo->GetNoComponents();i++){
               UPoint unit1;
               mo->Get(i, unit1);

               Instant st = unit1.timeInterval.start;
               Instant et = unit1.timeInterval.end;
  
               st.ReadFrom(st.ToDouble() + day_deviation);
               et.ReadFrom(et.ToDouble() + day_deviation);

               Interval<Instant> up_interval;
               up_interval.start = st;
               up_interval.lc = true;
               up_interval.end = et;
               up_interval.rc = false; 

               UPoint* up = new UPoint(up_interval, unit1.p0, unit1.p1);
               mo_tmp->Add(*up);
               delete up;     
            }

            for(int i = 0;i < genmo->GetNoComponents(); i++){
                UGenLoc ugloc;
                genmo->Get(i, ugloc);

                Instant st = ugloc.timeInterval.start;
                Instant et = ugloc.timeInterval.end;
  
                st.ReadFrom(st.ToDouble() + day_deviation);
                et.ReadFrom(et.ToDouble() + day_deviation);

                Interval<Instant> up_interval;
                up_interval.start = st;
                up_interval.lc = true;
                up_interval.end = et;
                up_interval.rc = false; 

                UGenLoc* unit_new = new UGenLoc(up_interval, ugloc.gloc1,
                                    ugloc.gloc2, ugloc.tm);
                genmo_tmp->Add(*unit_new);
                delete unit_new;
            }

            mo_tmp->EndBulkLoad();
            genmo_tmp->EndBulkLoad(false, false);

            trip1_list.push_back(*genmo_tmp);
            trip2_list.push_back(*mo_tmp);

            delete genmo_tmp;
            delete mo_tmp;

     }else{

      trip1_list.push_back(*genmo);
      trip2_list.push_back(*mo);
     }

    delete mo; 
    delete genmo; 

    path_tuple->DeleteIfAllowed();
    delete peri;
    //////////////////////////////////////////////////////////////////
    ////////////reduce the time cost of generating moving object//////
    ////////////if two bus stops are reachable, it generates the second////
    //////// moving object, but different time intervals //////////////////
    ///////////////////////////////////////////////////////////////////////

    obj_no_rep++;
    if(obj_no_rep == max_obj) obj_no_rep = 0;
    ///////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
//   cout<<"building  "<<GetBuildingStr(nn_build_list[count_tmp].type)<<endl;

    cout<<real_count<<" generic moving object "<<endl;
    real_count++;
    count_tmp++;

  }
   pm->CloseDualGraph(dg);
   pm->CloseVisualGraph(vg);

   sp->CloseBusNetwork(bn);

}


/*
create long distance trips

*/
void GenMObject::GenerateGenMOBench5(Space* sp, Periods* peri, int mo_no,
                                     Relation* build_rel1, Relation* build_rel2)
{
  ///////////////////////////////////////////////////////////////
  //////////  Initialization open infrastructures  /////////////
  //////////////////////////////////////////////////////////////

  IndoorInfra* i_infra = sp->LoadIndoorInfra(IF_GROOM);
  if(i_infra == NULL){
      cout<<"indoor infrastructure does not exist "<<endl;
     return;
  }

  Pavement* pm = sp->LoadPavement(IF_REGION);
  if(pm == NULL){
    cout<<"pavement loading error"<<endl;
    sp->CloseIndoorInfra(i_infra);
    return;
  }

  Network* rn = sp->LoadRoadNetwork(IF_LINE);
  if(rn == NULL){
    cout<<"road network loading error"<<endl;
    sp->ClosePavement(pm);
    sp->CloseIndoorInfra(i_infra);
    return;
  }

  RoadGraph* rg = sp->LoadRoadGraph();
  if(rg == NULL){
    sp->CloseRoadNetwork(rn);
    sp->ClosePavement(pm);
    sp->CloseIndoorInfra(i_infra);
    cout<<"road graph loading error"<<endl;
    return;
  }
  ///////////////get buildings with full data ////////////////////////
  ////////////////load all buildings and indoor graphs/////////////
  ////////////////// and indoor paths//////////////////////////////
  MaxRect* maxrect = new MaxRect();
  maxrect->OpenBuilding();
  maxrect->OpenIndoorGraph();

#ifdef INDOOR_PATH
  maxrect->LoadIndoorPaths(indoor_paths_list, rooms_id_list);
  assert(indoor_paths_list.size() == rooms_id_list.size());
#endif 


  ////////////get buildings information /////////////////////////

  vector<RefBuild> build_id1_list;
  vector<RefBuild> build_id2_list;
  GetSelectedBuilding(i_infra, build_id1_list, build_id2_list, 
                     maxrect, build_rel1, build_rel2);
  
//  cout<<build_id1_list.size()<<" "<<build_id2_list.size()<<endl; 
  /////////////////////set time interval ////////////////////////////
  
  Interval<Instant> time_span;
  peri->Get(0, time_span);
  
  if(time_span.start.GetDay() != time_span.end.GetDay()){
    cout<<"time period should be in one day "<<endl;
    maxrect->CloseIndoorGraph();
    maxrect->CloseBuilding();
    delete maxrect;
    sp->CloseIndoorInfra(i_infra);
    return;
  }
  int h1 = time_span.start.GetHour();
  int h2 = time_span.end.GetHour();
  int para = (h2 - h1);
  //////////////////////////////////////////////////////////////////////

   vector<RefBuild> build_list1;
   vector<RefBuild> build_list2;
   vector<RefBuild> build_list3;
   CreateBuildingPair6(build_id1_list, build_id2_list,
                       build_list1, build_list2, build_list3,
                       mo_no * obj_scale_min);//no station

   GenerateGenMO_IWC(sp, maxrect, i_infra, 
                     pm, rn, rg, peri, mo_no,
                     build_list1, build_list2, build_list3, para);

    ///////////////////////////////////////////////////////////////////
    /////////////////// close all infrastructures ///////////////////
    /////////////////////////////////////////////////////////////////
    maxrect->CloseIndoorGraph();
    maxrect->CloseBuilding();
    delete maxrect;

    sp->CloseRoadGraph(rg);
    sp->CloseRoadNetwork(rn);
    sp->ClosePavement(pm);
    sp->CloseIndoorInfra(i_infra);

}

/*
select three buildings to visit 

*/
void GenMObject::CreateBuildingPair6(vector<RefBuild> b_list1,
                          vector<RefBuild> b_list2,
                          vector<RefBuild>& build_tid1_list,
                          vector<RefBuild>& build_tid2_list,  
                          vector<RefBuild>& build_tid3_list,  int pair_no)
{
   int b_no = 1;
   const double min_dist = 1000.0;//Euclidean distance

   /////////////////add train station ////////////////////
   while(b_no <= pair_no){
    int id1 = GetRandom() % b_list1.size();
    int id2 = GetRandom() % b_list2.size();
    int id3 = GetRandom() % b_list2.size();

    RefBuild refb_1 = b_list1[id1];
    RefBuild refb_2 = b_list2[id2];
    RefBuild refb_3 = b_list2[id3];
    if(refb_1.rect.Distance(refb_2.rect) > min_dist && 
       refb_2.rect.Distance(refb_3.rect) > min_dist){
      build_tid1_list.push_back(refb_1);
      build_tid2_list.push_back(refb_2);
      build_tid3_list.push_back(refb_3);
      b_no++;
    }
  }

//      for(unsigned int i = 0;i < build_tid1_list.size();i++)
//          cout<<GetBuildingStr(build_tid1_list[i].type)<<" "
//              <<GetBuildingStr(build_tid2_list[i].type)<<endl;

//   for(unsigned int i = 0;i < build_tid1_list.size();i++){
//     rect_list1.push_back(build_tid1_list[i].rect);
//     rect_list2.push_back(build_tid2_list[i].rect);
//     rect_list3.push_back(build_tid3_list[i].rect);
//   }

}

/*
long trip with modes indoor + walk + car

*/
void GenMObject::GenerateGenMO_IWC(Space* sp, MaxRect* maxrect,
                                     IndoorInfra* i_infra,
                                     Pavement* pm, Network* rn,
                                     RoadGraph* rg, Periods* peri, 
                                     int mo_no, 
                                     vector<RefBuild> build_id1_list,
                                     vector<RefBuild> build_id2_list, 
                                     vector<RefBuild> build_id3_list, 
                                     int para)
{

  //////////////////////////////////////////////////////////////////
  ///////////////start time///////////////////////////////////////
   Interval<Instant> periods;
   peri->Get(0, periods);
   Instant start_time = periods.start;
   int time_range = para*60;//e.g., 12 hours in range 
  ////////////////////////////////////////////////////////////
   int count = 0;
   int real_count = 1;

   while(real_count <= mo_no && count < obj_scale*mo_no &&
         count < (int)build_id1_list.size()){

   //////////////////////////////start time///////////////////////////
   if(count % 3 == 0)
     start_time.ReadFrom(periods.start.ToDouble() + 
                        (GetRandom() % time_range)/(24.0*60.0));
   else
     start_time.ReadFrom(periods.end.ToDouble() -
                        (GetRandom() % time_range)/(24.0*60.0));

//     cout<<GetBuildingStr(build_id1_list[count].type)<<" "
//         <<GetBuildingStr(build_id2_list[count].type)<<endl;

    MPoint* mo = new MPoint(0);
    GenMO* genmo = new GenMO(0);
    mo->StartBulkLoad();
    genmo->StartBulkLoad();
//    cout<<"start time sub trip1 "<<endl;
    bool res1 = SubTrip_C1(sp, i_infra, maxrect, 
                           pm, rn, rg, peri,
                           build_id1_list, build_id2_list, count,
                           mo, genmo, start_time);
    if(res1 == false){
        mo->EndBulkLoad();
        genmo->EndBulkLoad();
        delete mo;
        delete genmo;
        count++;
        continue;
    }
    ////////////////////////////////////////////////////////////////
    //////get the last indoor location///////////////////////////
    //////////////////////////////////////////////////////////////
    int build_id = build_id2_list[count].build_id;
    UGenLoc last_unit;
    genmo->Get(genmo->GetNoComponents() - 1, last_unit);
    int groom_oid = GROOM_Oid(build_id, last_unit.GetOid());
//    cout<<groom_oid<<endl;
    if(groom_oid == 0){
      cout<<"groom oid cannot be zero"<<endl;
      delete mo;
      delete genmo;
      break;
    }
    Loc loc(last_unit.gloc2.GetLoc().loc1, last_unit.gloc2.GetLoc().loc2);
    GenLoc gloc(groom_oid, loc);
//    cout<<"last indoor "<<gloc<<endl;

    /////////////////////////////////////////////////////////////////
//    cout<<"start time for trip2 "<<start_time<<endl;
    bool res2 = SubTrip_C2(sp, i_infra, maxrect, 
                           pm, rn, rg, peri,
                           build_id2_list, build_id3_list, count,
                           mo, genmo, start_time, gloc);
    if(res2 == false){
        mo->EndBulkLoad();
        genmo->EndBulkLoad();
        delete mo;
        delete genmo;
        count++;
        continue;
    }
    //////////////////////////////////////////////////////////////////
    build_id = build_id3_list[count].build_id;

    genmo->Get(genmo->GetNoComponents() - 1, last_unit);
    groom_oid = GROOM_Oid(build_id, last_unit.GetOid());
//    cout<<groom_oid<<endl;
    if(groom_oid == 0){
      cout<<"groom oid cannot be zero"<<endl;
      delete mo;
      delete genmo;
      break;
    }
    loc.loc1 = last_unit.gloc2.GetLoc().loc1;
    loc.loc2 = last_unit.gloc2.GetLoc().loc2;
    gloc.SetValue(groom_oid, loc);
//    cout<<"last indoor "<<gloc<<endl;
//     cout<<"start time "<<start_time<<endl;
//    cout<<"start time for trip3 "<<start_time<<endl;
//     //////////////////////////////////////////////////////////////////
     bool res3 = SubTrip_C2(sp, i_infra, maxrect, 
                           pm, rn, rg, peri,
                           build_id3_list, build_id2_list, count,
                           mo, genmo, start_time, gloc);
    if(res3 == false){
        mo->EndBulkLoad();
        genmo->EndBulkLoad();
        delete mo;
        delete genmo;
        count++;
        continue;
    }
    ////////////////////////////////////////////////////////////////////
    build_id = build_id2_list[count].build_id;

    genmo->Get(genmo->GetNoComponents() - 1, last_unit);
    groom_oid = GROOM_Oid(build_id, last_unit.GetOid());
//    cout<<groom_oid<<endl;
    if(groom_oid == 0){
      cout<<"groom oid cannot be zero"<<endl;
      delete mo;
      delete genmo;
      break;
    }
    loc.loc1 = last_unit.gloc2.GetLoc().loc1;
    loc.loc2 = last_unit.gloc2.GetLoc().loc2;
    gloc.SetValue(groom_oid, loc);
//    cout<<"last indoor "<<gloc<<endl;
//     cout<<"start time "<<start_time<<endl;
//    cout<<"start time for trip4 "<<start_time<<endl;

    bool res4 = SubTrip_C2(sp, i_infra, maxrect, 
                           pm, rn, rg, peri,
                           build_id2_list, build_id1_list, count,
                           mo, genmo, start_time, gloc);
    if(res4 == false){
        mo->EndBulkLoad();
        genmo->EndBulkLoad();
        delete mo;
        delete genmo;
        count++;
        continue;
    }

    /////////////////////////////////////////////////////////////////////

    mo->EndBulkLoad();
    genmo->EndBulkLoad(false, false);

    trip1_list.push_back(*genmo);
    trip2_list.push_back(*mo);

    delete mo; 
    delete genmo; 


   //////////////////////////////////////////////////////////////////////
//   cout<<"building 1 "<<GetBuildingStr(build_id1_list[count].type)
//        <<" building 2 "<<GetBuildingStr(build_id2_list[count].type)<<endl;

    count++;
    cout<<real_count<<" generic moving object"<<endl;
    real_count++;

  }


}

int GenMObject::GROOM_Oid(int id1, int id2)
{
//    cout<<"second build id "<<id1<<" oid "<<id2<<endl;

    char buffer1[64];
    sprintf(buffer1, "%d", id2);
    char buffer2[64];
    sprintf(buffer2, "%d", id1);
    string s(buffer2);
    char buffer3[64];
    strcpy(buffer3, &(buffer1[s.length()]));
    string s2(buffer3);
    int groom_oid = 0;
    if(s2.length() > 0)
      sscanf(buffer3, "%d", &groom_oid);
//    cout<<"groom oid "<<groom_oid<<endl;
    return groom_oid;
}


/*
sub trip 1, by car from one building to another 

*/
bool GenMObject::SubTrip_C1(Space* sp, IndoorInfra* i_infra, MaxRect* maxrect,
                            Pavement* pm, Network* rn, RoadGraph* rg,
                            Periods* peri, vector<RefBuild> build_id1_list,
                            vector<RefBuild> build_id2_list, int count,
                            MPoint* mo, GenMO* genmo, Instant& start_time)
{
    ////////////////////////////////////////////////////////////////
  //////////////////////Initialization/////////////////////////////
  ////////////////////////////////////////////////////////////////
   Relation* dg_node_rel = sp->GetDualNodeRel();
   BTree* btree = sp->GetDGNodeBTree();
   Relation* speed_rel = sp->GetSpeedRel();

   if(dg_node_rel == NULL || btree == NULL || speed_rel == NULL){
      cout<<"GenerateGenMO_IWCTB():auxiliary relation empty IWCTB"<<endl;
      assert(false);
      return false;
   }

  const double min_path = 0.1;
  RoadNav* road_nav = new RoadNav();
  ///////////////////////////////////////////////////////////////////
  Relation* build_path_rel = i_infra->BuildingPath_Rel();

/////////////////////load all paths from this building////////////////
    vector<int> path_id_list1;
    i_infra->GetPathIDFromTypeID(build_id1_list[count].reg_id, path_id_list1);
//    cout<<"number of paths "<<path_id_list1.size()<<endl;

    vector<int> path_id_list2;
    i_infra->GetPathIDFromTypeID(build_id2_list[count].reg_id, path_id_list2);
//    cout<<"number of paths "<<path_id_list2.size()<<endl;

   ////////////////////////////////////////////////////////////////////
   //////////////if a building has several entrances///////////////////
   ///////////it randomly selects an entrance  ///////////////////////
   ////////////the buildings are far away from each///////////////////
   //////////which entrance to go out does not influence the distance a lot///
   ////////////////////////////////////////////////////////////////////
   if(path_id_list1.size() == 0 || path_id_list2.size() == 0){
     return false;
   }

   int path_tid1 = path_id_list1[GetRandom() % path_id_list1.size()];
   int path_tid2 = path_id_list2[GetRandom() % path_id_list2.size()];

   Tuple* path_tuple1 = build_path_rel->GetTuple(path_tid1, false);
   Tuple* path_tuple2 = build_path_rel->GetTuple(path_tid2, false);


    /////////////////////////////////////////////////////////////////////
    //////////////////1 get buildings and two positions for network /////
    ////////////////// the end point of path in pavement/////////////////
    ////////////////////////////////////////////////////////////////////

    GenLoc* gloc1 = 
        (GenLoc*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
    Point* ep1_1 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP);
    Point* ep1_2 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP2);


    GenLoc* gloc2 = 
        (GenLoc*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
    Point* ep2_1 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP);
    Point* ep2_2 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP2);


    /////////////reset the general location//////////////////////
    Loc loc1(ep1_2->GetX(), ep1_2->GetY());
    GenLoc newgloc1(gloc1->GetOid(), loc1);
    Loc loc2(ep2_2->GetX(), ep2_2->GetY());
    GenLoc newgloc2(gloc2->GetOid(), loc2);


    /////////////////////////////////////////////////////////////////////
    //////////////////2 map two positions to gpoints////////////////////
    /////////////////get the start and end location of the trip/////////
    ////////////////////////////////////////////////////////////////////
    vector<GPoint> gpoint_list;
    vector<Point> p_list;
    bool correct;
    PaveLoc2GPoint(newgloc1, newgloc2, sp, dg_node_rel,
                   btree, gpoint_list, p_list, correct, rn);

    if(correct == false){
        path_tuple1->DeleteIfAllowed();
        path_tuple2->DeleteIfAllowed();
        return false;
    }

    GPoint gp1 = gpoint_list[0];
    GPoint gp2 = gpoint_list[1];
    Point start_loc = p_list[0];
    Point end_loc = p_list[1];

    /////////////////////////////////////////////////////////////////////
    ///////////////3 indoor movement1 + from entrance to pavement//////
    ////////////////////////////////////////////////////////////////////

    //////////////////////indoor movement///////////////////////////////////
    ////////////////////to show which entrance it is////////////////////////
    int entrance_index1 = ((CcInt*)path_tuple1->GetAttribute(IndoorInfra::
                           INDOORIF_SP_INDEX))->GetIntval();
    int reg_id1 =  ((CcInt*)path_tuple1->GetAttribute(IndoorInfra::
                                               INDOORIF_REG_ID))->GetIntval();
     Point* sp1 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_SP);
     GenerateIndoorMovementToExit(i_infra, genmo, mo, start_time, *sp1,
                                  entrance_index1, reg_id1, maxrect, peri);


    ///////////////   outdoor  movement //////////////////////////////
    Line* path1 = (Line*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_PATH);

    if(path1->Length() > min_path)
      GenerateFreeMovement(path1, *sp1, genmo, mo, start_time);

    GenerateFreeMovement2(*ep1_1, *ep1_2, genmo, mo, start_time);

    /////////////////////////////////////////////////////////////////////
    //////////////////4 pavement to network (start location)/////////////
    ////////////////////////////////////////////////////////////////////
    ConnectStartMove(newgloc1, start_loc, mo, genmo, start_time, pm);

    /////////////////////////////////////////////////////////////////////
    //////////////////5 road network movement //////////////////////////
    ////////////////////////////////////////////////////////////////////
    GLine* gl = new GLine(0);
    road_nav->ShortestPathSub(&gp1, &gp2, rg, rn, gl);
    ConnectGP1GP2(rn, start_loc, gl, mo, genmo, start_time, speed_rel, "Car");
    delete gl;
    /////////////////////////////////////////////////////////////////////
    //////////////////6 end network location to pavement//////////////////
    ////////////////////////////////////////////////////////////////////
    ConnectEndMove(end_loc, newgloc2, mo, genmo, start_time, pm);

    /////////////////////////////////////////////////////////////////////
    ////////7 pavement to building entrance + indoor movement2//////////
    ////////////////////////////////////////////////////////////////////

    ////////////////outdoor movement/////////////////////////////////////////
    GenerateFreeMovement2(*ep2_2, *ep2_1, genmo, mo, start_time);

    Line* path2 = (Line*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_PATH);

    if(path2->Length() > min_path)
      GenerateFreeMovement(path2, *ep2_1, genmo, mo, start_time);

    ///////////////////indoor movement//////////////////////////////////////
    ////////////////////to show which entrance it is////////////////////
    Point* sp2 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_SP);
    int entrance_index2 = ((CcInt*)path_tuple2->GetAttribute(IndoorInfra::
                                   INDOORIF_SP_INDEX))->GetIntval();
    int reg_id2 =  ((CcInt*)path_tuple2->GetAttribute(IndoorInfra::
                                              INDOORIF_REG_ID))->GetIntval();

    if(WorkBuilding(build_id2_list[count].type)){
//      cout<<"work buiding"<<endl;
      GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                   entrance_index2, reg_id2, maxrect, 
                                      peri, TIMESPAN_W2);
    }else if(CIBuilding(build_id2_list[count].type)){
//       cout<<"cinema"<<endl;
       GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                   entrance_index2, reg_id2, maxrect, 
                                      peri, TIMESPAN_CI);
    }else if(HotelBuilding(build_id2_list[count].type)){

       GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                   entrance_index2, reg_id2, maxrect, 
                                      peri, TIMESPAN_H);
    }else{
//       cout<<"other"<<endl;
       GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                   entrance_index2, reg_id2, maxrect, peri,
                                   TIMESPAN_NO);
    }
   /////////////////////////////////////////////////////////////////////////

    path_tuple1->DeleteIfAllowed();
    path_tuple2->DeleteIfAllowed();


    delete road_nav; 
    return true;
}




/*
sub trip 2, by car from one building to another by specifying the start 
indoor location

*/
bool GenMObject::SubTrip_C2(Space* sp, IndoorInfra* i_infra, MaxRect* maxrect,
                            Pavement* pm, Network* rn, RoadGraph* rg,
                            Periods* peri, vector<RefBuild> build_id1_list,
                            vector<RefBuild> build_id2_list, int count,
                            MPoint* mo, GenMO* genmo, 
                            Instant& start_time, GenLoc gloc)
{

  ////////////////////////////////////////////////////////////////
  //////////////////////Initialization/////////////////////////////
  ////////////////////////////////////////////////////////////////
   Relation* dg_node_rel = sp->GetDualNodeRel();
   BTree* btree = sp->GetDGNodeBTree();
   Relation* speed_rel = sp->GetSpeedRel();

   if(dg_node_rel == NULL || btree == NULL || speed_rel == NULL){
      cout<<"GenerateGenMO_IWCTB():auxiliary relation empty IWCTB"<<endl;
      assert(false);
      return false;
   }

  const double min_path = 0.1;
  RoadNav* road_nav = new RoadNav();
  ///////////////////////////////////////////////////////////////////
  Relation* build_path_rel = i_infra->BuildingPath_Rel();

/////////////////////load all paths from this building////////////////
    vector<int> path_id_list1;
    i_infra->GetPathIDFromTypeID(build_id1_list[count].reg_id, path_id_list1);
//    cout<<"number of paths "<<path_id_list1.size()<<endl;

    vector<int> path_id_list2;
    i_infra->GetPathIDFromTypeID(build_id2_list[count].reg_id, path_id_list2);
//    cout<<"number of paths "<<path_id_list2.size()<<endl;

   ////////////////////////////////////////////////////////////////////
   //////////////if a building has several entrances///////////////////
   ///////////it randomly selects an entrance  ///////////////////////
   ////////////the buildings are far away from each///////////////////
   //////////which entrance to go out does not influence the distance a lot///
   ////////////////////////////////////////////////////////////////////
   if(path_id_list1.size() == 0 || path_id_list2.size() == 0){
     return false;
   }

   int path_tid1 = path_id_list1[GetRandom() % path_id_list1.size()];
   int path_tid2 = path_id_list2[GetRandom() % path_id_list2.size()];

   Tuple* path_tuple1 = build_path_rel->GetTuple(path_tid1, false);
   Tuple* path_tuple2 = build_path_rel->GetTuple(path_tid2, false);


    /////////////////////////////////////////////////////////////////////
    //////////////////1 get buildings and two positions for network /////
    ////////////////// the end point of path in pavement/////////////////
    ////////////////////////////////////////////////////////////////////

    GenLoc* gloc1 = 
        (GenLoc*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
    Point* ep1_1 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP);
    Point* ep1_2 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_EP2);


    GenLoc* gloc2 = 
        (GenLoc*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP2_GLOC);
    Point* ep2_1 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP);
    Point* ep2_2 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_EP2);


    /////////////reset the general location//////////////////////
    Loc loc1(ep1_2->GetX(), ep1_2->GetY());
    GenLoc newgloc1(gloc1->GetOid(), loc1);
    Loc loc2(ep2_2->GetX(), ep2_2->GetY());
    GenLoc newgloc2(gloc2->GetOid(), loc2);


    /////////////////////////////////////////////////////////////////////
    //////////////////2 map two positions to gpoints////////////////////
    /////////////////get the start and end location of the trip/////////
    ////////////////////////////////////////////////////////////////////
    vector<GPoint> gpoint_list;
    vector<Point> p_list;
    bool correct;
    PaveLoc2GPoint(newgloc1, newgloc2, sp, dg_node_rel,
                   btree, gpoint_list, p_list, correct, rn);

    if(correct == false){
        path_tuple1->DeleteIfAllowed();
        path_tuple2->DeleteIfAllowed();
        return false;
    }

    GPoint gp1 = gpoint_list[0];
    GPoint gp2 = gpoint_list[1];
    Point start_loc = p_list[0];
    Point end_loc = p_list[1];

    /////////////////////////////////////////////////////////////////////
    ///////////////3 indoor movement1 + from entrance to pavement//////
    ////////////////////////////////////////////////////////////////////

    //////////////////////indoor movement///////////////////////////////////
    ////////////////////to show which entrance it is////////////////////////
    int entrance_index1 = ((CcInt*)path_tuple1->GetAttribute(IndoorInfra::
                           INDOORIF_SP_INDEX))->GetIntval();
    int reg_id1 =  ((CcInt*)path_tuple1->GetAttribute(IndoorInfra::
                                               INDOORIF_REG_ID))->GetIntval();
     Point* sp1 = (Point*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_SP);
     GenerateIndoorMovementToExitExt(i_infra, genmo, mo, start_time, *sp1,
                               entrance_index1, reg_id1, maxrect, peri, gloc);


    ///////////////   outdoor  movement //////////////////////////////
    Line* path1 = (Line*)path_tuple1->GetAttribute(IndoorInfra::INDOORIF_PATH);

    if(path1->Length() > min_path)
      GenerateFreeMovement(path1, *sp1, genmo, mo, start_time);

    GenerateFreeMovement2(*ep1_1, *ep1_2, genmo, mo, start_time);

    /////////////////////////////////////////////////////////////////////
    //////////////////4 pavement to network (start location)/////////////
    ////////////////////////////////////////////////////////////////////
    ConnectStartMove(newgloc1, start_loc, mo, genmo, start_time, pm);

    /////////////////////////////////////////////////////////////////////
    //////////////////5 road network movement //////////////////////////
    ////////////////////////////////////////////////////////////////////
    GLine* gl = new GLine(0);
    road_nav->ShortestPathSub(&gp1, &gp2, rg, rn, gl);
    ConnectGP1GP2(rn, start_loc, gl, mo, genmo, start_time, speed_rel, "Car");
    delete gl;
    /////////////////////////////////////////////////////////////////////
    //////////////////6 end network location to pavement//////////////////
    ////////////////////////////////////////////////////////////////////
    ConnectEndMove(end_loc, newgloc2, mo, genmo, start_time, pm);

    /////////////////////////////////////////////////////////////////////
    ////////7 pavement to building entrance + indoor movement2//////////
    ////////////////////////////////////////////////////////////////////

    ////////////////outdoor movement/////////////////////////////////////////
    GenerateFreeMovement2(*ep2_2, *ep2_1, genmo, mo, start_time);

    Line* path2 = (Line*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_PATH);

    if(path2->Length() > min_path)
      GenerateFreeMovement(path2, *ep2_1, genmo, mo, start_time);

    ///////////////////indoor movement//////////////////////////////////////
    ////////////////////to show which entrance it is////////////////////
    Point* sp2 = (Point*)path_tuple2->GetAttribute(IndoorInfra::INDOORIF_SP);
    int entrance_index2 = ((CcInt*)path_tuple2->GetAttribute(IndoorInfra::
                                   INDOORIF_SP_INDEX))->GetIntval();
    int reg_id2 =  ((CcInt*)path_tuple2->GetAttribute(IndoorInfra::
                                              INDOORIF_REG_ID))->GetIntval();

    if(WorkBuilding(build_id2_list[count].type)){
      GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                   entrance_index2, reg_id2, maxrect, 
                                      peri, TIMESPAN_W2);
    }else if(CIBuilding(build_id2_list[count].type)){
          GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                   entrance_index2, reg_id2, maxrect, 
                                      peri, TIMESPAN_CI);
    }else if(HotelBuilding(build_id2_list[count].type)){
          GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                   entrance_index2, reg_id2, maxrect, 
                                      peri, TIMESPAN_H);
    }else{
       GenerateIndoorMovementFromExit(i_infra, genmo,mo,start_time, *sp2, 
                                   entrance_index2, reg_id2, maxrect, peri,
                                  TIMESPAN_NO);
    }
   /////////////////////////////////////////////////////////////////////////

    path_tuple1->DeleteIfAllowed();
    path_tuple2->DeleteIfAllowed();


    delete road_nav; 
    return true;
}


/*
compute the traffic value

*/
void GenMObject::GetTraffic(Relation* alltrips, Periods* peri, 
                            Relation* roadsegs, bool b)
{
   ////////////////initialize the road segments structure/////////////////
    vector< vector<Road_Seg> > roads_list;
    int max_rid = 0;
    for(int i = 1;i <= roadsegs->GetNoTuples();i++){
      Tuple* tuple = roadsegs->GetTuple(i, false);
      int rid = ((CcInt*)tuple->GetAttribute(RS_RID))->GetIntval();
      tuple->DeleteIfAllowed();
      if(rid > max_rid) max_rid = rid;
    }
//    cout<<"max rid "<<max_rid;
    for(int i = 0;i < max_rid;i++){
      vector<Road_Seg> temp;
      roads_list.push_back(temp);
    }
    for(int i = 1;i <= roadsegs->GetNoTuples();i++){
      Tuple* tuple = roadsegs->GetTuple(i, false);
      int rid = ((CcInt*)tuple->GetAttribute(RS_RID))->GetIntval();
      int sid = ((CcInt*)tuple->GetAttribute(RS_SID))->GetIntval();
//       double loc1 = ((CcReal*)tuple->GetAttribute(RS_MEAS1))->GetRealval();
//       double loc2 = ((CcReal*)tuple->GetAttribute(RS_MEAS2))->GetRealval();

      CcReal loc1(true, ((CcReal*)tuple->GetAttribute(RS_MEAS1))->GetRealval());
      CcReal loc2(true, ((CcReal*)tuple->GetAttribute(RS_MEAS2))->GetRealval());

      Interval<CcReal> range_loc;
      range_loc.start = loc1;
      range_loc.lc = true;
      range_loc.end = loc2;
      range_loc.rc = true;

      Road_Seg rs(sid, range_loc, 0);
      roads_list[rid - 1].push_back(rs);
      tuple->DeleteIfAllowed();
//      cout<<"rid "<<rid<<"sid "<<sid<<endl;
    }

//      for(unsigned int i = 0;i < roads_list.size();i++){
//  
//        for(unsigned int j = 0;j < roads_list[i].size();j++)
//          roads_list[i][j].Print();
//      }

    GetTrafficValue(alltrips, peri, roads_list, b);
    
//    vector<Road_Seg> res_list;
    for(unsigned int i = 0;i < roads_list.size();++i){
      for(unsigned int j = 0;j < roads_list[i].size();j++){
//        res_list.push_back(roads_list[i][j]);
        oid_list.push_back(roads_list[i][j].sid);
        count_list.push_back(roads_list[i][j].count);
      }
    }
//    sort(res_list.begin(), res_list.end());

/*    for(unsigned int i = 0;i < res_list.size();i++)
      res_list[i].Print();*/
//    for(unsigned int i = 0;i < res_list.size();i++){
//        oid_list.push_back(res_list[i].sid);
//        count_list.push_back(res_list[i].count);
//    }

}

/*
with and without optimization techniques to get the traffic 

*/
void GenMObject::GetTrafficValue(Relation* alltrips, Periods* peri, 
                              vector< vector<Road_Seg> >& roads_list, bool b)
{
//  cout<<"optimization techniques"<<endl;

  for(int i = 1;i <= alltrips->GetNoTuples();i++){
//  for(int i = 1;i <= 50000.0;i++){
    Tuple* tuple = alltrips->GetTuple(i, false);
    Periods* def = (Periods*)tuple->GetAttribute(GENMO_DEF);
    if(def->Intersects(*peri) == false){
      tuple->DeleteIfAllowed();
      continue;
    }
    GenMO* trip = (GenMO*)tuple->GetAttribute(GENMO_TRIP1);
    int mode_car = GetTM("Car");
    int mode_taxi = GetTM("Taxi");
    int mode_bike = GetTM("Bike");
    int pos1 = (int)ARR_SIZE(str_tm) - 1 - mode_car;
    int pos2 = (int)ARR_SIZE(str_tm) - 1 - mode_taxi;
    int pos3 = (int)ARR_SIZE(str_tm) - 1 - mode_bike;
    if(b){
      int tm = ((CcInt*)tuple->GetAttribute(GENMO_TM))->GetIntval();
      bitset<ARR_SIZE(str_tm)> modebits(tm);

      if(!(modebits.test(pos1) || modebits.test(pos2) || modebits.test(pos3))){
        tuple->DeleteIfAllowed();
        continue;
      }
    }else{
      //traverse units in genmo to check tm 
      bool found = false;
      for(int j = 0;j < trip->GetNoComponents();j++){
          UGenLoc unit;
          trip->Get(j, unit);
          if(unit.tm == mode_car || unit.tm == mode_taxi ||
             unit.tm == mode_bike){
            found = true;
            break;
          }
      }
      if(found == false){
          tuple->DeleteIfAllowed();
          continue;
      } 
    }

    if(b){//with  optimization techniques
        MReal* uindex = (MReal*)tuple->GetAttribute(GENMO_INDEX);
        for(int j = 0;j < uindex->GetNoComponents();j++){
            UReal ur;
            uindex->Get(j, ur);
            if((int)ur.a == mode_car || (int)ur.a == mode_taxi || 
               (int)ur.a == mode_bike){
                int start = (int)ur.b;
                int end = (int)ur.c;
                for(;start <= end;start++){//
                    UGenLoc unit;
                    trip->Get(start, unit);
                    int oid = unit.GetOid();
                    assert(1 <= oid && oid <= (int)roads_list.size());

//                     double loc1 = unit.gloc1.GetLoc().loc1;
//                     double loc2 = unit.gloc2.GetLoc().loc1;

                    CcReal loc1(true, unit.gloc1.GetLoc().loc1);
                    CcReal loc2(true, unit.gloc2.GetLoc().loc1);

                    Interval<CcReal> locs;
                    if(loc1 < loc2){
                        locs.start = loc1;
                        locs.end = loc2;
                    }else{
                      locs.start = loc2;
                      locs.end = loc2;
                    }
                    locs.lc = true;
                    locs.rc = true;
                    for(unsigned int k = 0;k < roads_list[oid - 1].size();++k){
                      if(roads_list[oid - 1][k].r_loc.Intersects(locs)){
                          roads_list[oid - 1][k].count++;
                          break;
                      }
                    }
                }
            }
        }

    }else{//simple method 

      for(int j = 0;j < trip->GetNoComponents();j++){
          UGenLoc unit;
          trip->Get(j, unit);
          if(unit.tm == mode_car || unit.tm == mode_taxi || 
             unit.tm == mode_bike){
              int oid = unit.GetOid();
              assert(1 <= oid && oid <= (int)roads_list.size());
//               double loc1 = unit.gloc1.GetLoc().loc1;
//               double loc2 = unit.gloc2.GetLoc().loc1;

              CcReal loc1(true, unit.gloc1.GetLoc().loc1);
              CcReal loc2(true, unit.gloc2.GetLoc().loc1);

              Interval<CcReal> locs;
              if(loc1 < loc2){
                  locs.start = loc1;
                  locs.end = loc2;
              }else{
                  locs.start = loc2;
                  locs.end = loc2;
              }
              locs.lc = true;
              locs.rc = true;
              for(unsigned int k = 0;k < roads_list[oid - 1].size();++k){
                if(roads_list[oid - 1][k].r_loc.Intersects(locs)){
                    roads_list[oid - 1][k].count++;
                    break;
                }
              }
          }
      }
    }

    tuple->DeleteIfAllowed();
  }

}

/////////////////////////////////////////////////////////////////////////
////////////////navigation system///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/*
find a trip with minimum traveling time from one location on the pavement 
to another location on the pavement 

*/
void Navigation::Navigation1(Space* sp, Relation* rel1, Relation* rel2, 
                   Instant* query_time, Relation* rel3, Relation* rel4, 
                   R_Tree<2,TupleId>* rtree)
{
  Pavement* pm = sp->LoadPavement(IF_REGION);
  BusNetwork* bn = sp->LoadBusNetwork(IF_BUSNETWORK);
  DualGraph* dg = pm->GetDualGraph();
  VisualGraph* vg = pm->GetVisualGraph();
  ///////////////////////////////////////////////////////////////////
  ////////////check the two query locations on the pavement/////////
  //////////////////////////////////////////////////////////////////
  if(rel1->GetNoTuples() != 1 || rel2->GetNoTuples() != 1){
    cout<<"input query relation is not correct"<<endl;
    return;
  }
  Tuple* t1 = rel1->GetTuple(1, false);
  int oid1 = ((CcInt*)t1->GetAttribute(VisualGraph::QOID))->GetIntval();
  Point* p1 = (Point*)t1->GetAttribute(VisualGraph::QLOC2);
  Point loc1(*p1);
  t1->DeleteIfAllowed(); 
  
  Tuple* t2 = rel2->GetTuple(1, false);
  int oid2 = ((CcInt*)t2->GetAttribute(VisualGraph::QOID))->GetIntval();
  Point* p2 = (Point*)t2->GetAttribute(VisualGraph::QLOC2);
  Point loc2(*p2);
  t2->DeleteIfAllowed(); 

  oid1 -= dg->min_tri_oid_1;
  oid2 -= dg->min_tri_oid_1; 

  int no_node_graph = dg->No_Of_Node();
  if(oid1 < 1 || oid1 > no_node_graph){
    cout<<"loc1 does not exist"<<endl;
    return;
  }
  if(oid2 < 1 || oid2 > no_node_graph){
    cout<<"loc2 does not exist"<<endl;
    return;
  }
  if(AlmostEqual(loc1,loc2)){
    cout<<"start location equals to end location"<<endl;
    return;
  }
  Tuple* tuple1 = dg->GetNodeRel()->GetTuple(oid1, false);
  Region* reg1 = (Region*)tuple1->GetAttribute(DualGraph::PAVEMENT);
  
  CompTriangle* ct1 = new CompTriangle(reg1);
  assert(ct1->PolygonConvex()); 
  delete ct1; 
  
  if(loc1.Inside(*reg1) == false){
    tuple1->DeleteIfAllowed();
    cout<<"point1 is not inside the polygon"<<endl;
    return;
  }
  Tuple* tuple2 = dg->GetNodeRel()->GetTuple(oid2, false);
  Region* reg2 = (Region*)tuple2->GetAttribute(DualGraph::PAVEMENT);

  CompTriangle* ct2 = new CompTriangle(reg2);
  assert(ct2->PolygonConvex()); 
  delete ct2; 
  
  if(loc2.Inside(*reg2) == false){
    tuple1->DeleteIfAllowed();
    tuple2->DeleteIfAllowed();
    cout<<"point2 is not inside the polygon"<<endl;
    return;
  }
  tuple1->DeleteIfAllowed();
  tuple2->DeleteIfAllowed();
  ///////////////////////////////////////////////////////////////////////
  //////////1. find all possible bus stops to the two locations/////////
  //////////////////////////////////////////////////////////////////////
  vector<Bus_Stop> bs_list1;///bus stops
  vector<Point> ps_list1;//bus stops mapping points on the pavement 
  vector<Point> ps_list2;//bus stops 2d locations
  vector<GenLoc> gloc_list1;//bus stops mapping points by genloc 

  bool b1 = true;
  b1 = NearestBusStop1(loc1, rel4, rtree, bs_list1, 
                       ps_list1, ps_list2, gloc_list1);
//  cout<<"neighbor bus stops size1 "<<bs_list1.size()<<endl; 



  vector<Bus_Stop> bs_list2;///bus stops
  vector<Point> ps_list3;//bus stops mapping points on the pavement 
  vector<Point> ps_list4;//bus stops 2d locations
  vector<GenLoc> gloc_list2;//bus stops mapping points by genloc 

  bool b2 = true;
  b2 = NearestBusStop1(loc2, rel4, rtree, bs_list2, 
                       ps_list3, ps_list4, gloc_list2);
//  cout<<"neighbor bus stops size2 "<<bs_list2.size()<<endl; 

  if((b1 && b2) == false){
    cout<<" no valid bus stops for such two locations"<<endl;
    pm->CloseDualGraph(dg);
    pm->CloseVisualGraph(vg);
    sp->CloseBusNetwork(bn);
    sp->ClosePavement(pm);
    return;

  }

  vector<GenMO> res_list1; 
  vector<MPoint> res_list2;
  
  const double min_dist = 200.0;
  
  for(unsigned int i = 0;i < bs_list1.size();i++){

    for(unsigned int j = 0;j < bs_list2.size();j++){

    //////////////////////////////////////////////////////////////////
      GenMObject* genobj = new GenMObject();
      Instant start_time = *query_time;

     //////////////////////////////////////////////////////////////////////
     ////////////////add a trip only by walk, bus is not required//////////
     /////////////////////////////////////////////////////////////////////
     if(loc1.Distance(loc2) < min_dist){
        Instant start_time2 = *query_time;
        Walk_SP* wsp = new Walk_SP(dg, vg, NULL, NULL);
        wsp->rel3 = rel3;

        Line* path = new Line(0);
        ///////////////////walk segment////////////////////////////////////
        wsp->WalkShortestPath2(oid1, oid2, loc1, loc2, path);

        MPoint* mo2 = new MPoint(0);
        GenMO* genmo2 = new GenMO(0);
        mo2->StartBulkLoad();
        genmo2->StartBulkLoad();
        if(path->IsDefined() && path->Length() > EPSDIST)
         genobj->GenerateWalkMovement(dg, path, loc1, genmo2, mo2, start_time2);

        mo2->EndBulkLoad();
        genmo2->EndBulkLoad(false, false);


        res_list1.push_back(*genmo2);
        res_list2.push_back(*mo2);

        delete genmo2;
        delete mo2;

        delete path;
        delete wsp; 
        continue;
     }
      MPoint* mo = new MPoint(0);
      GenMO* genmo = new GenMO(0);
      mo->StartBulkLoad();
      genmo->StartBulkLoad();
    //////////////////////////////////////////////////////////////////
    /////2.connect the movement from start location to the bus stop//////
    //////////////////////////////////////////////////////////////////
    Line* res_path = new Line(0);
    vector<Point> bs_loc_list1;
    bs_loc_list1.push_back(ps_list1[i]);//bus stop mapping point on the pavement
    bs_loc_list1.push_back(ps_list2[i]);//bus stop 2d location
    Loc temp_loc1(loc1.GetX(), loc1.GetY());
    GenLoc query_loc1(oid1 + dg->min_tri_oid_1, temp_loc1); 
    genobj->ConnectStartStop(dg, vg, rel3, query_loc1, bs_loc_list1, 
                                gloc_list1[i].GetOid(),
                                genmo, mo, start_time, res_path);


    //////////////////////////////////////////////////////////////////
    /////3.connect the movement between two bus stops/////////////////
    //////////////////////////////////////////////////////////////////
//    cout<<bs_list1[i]<<" "<<bs_list2[j]<<endl; 

    BNNav* bn_nav = new BNNav(bn);
    bn_nav->ShortestPath_Time(&bs_list1[i], &bs_list2[j], &start_time);

    if(bn_nav->path_list.size() == 0){
//          cout<<"two unreachable bus stops"<<endl;
        mo->EndBulkLoad();
        genmo->EndBulkLoad();
        delete mo;
        delete genmo;
        delete bn_nav;
        delete res_path;
        delete genobj;
        continue;
    }

    
    ///////////////////////////////////////////////////////////////////
    vector<Point> bs_loc_list2;
    bs_loc_list2.push_back(ps_list3[j]);//bus stop mapping point on the pavement
    bs_loc_list2.push_back(ps_list4[j]);//bus stop 2d location
    Loc temp_loc2(loc2.GetX(), loc2.GetY());
    GenLoc query_loc2(oid2 + dg->min_tri_oid_1, temp_loc2); 
    int end_bus_stop_tri_id = gloc_list2[j].GetOid(); 
    ////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////
    int last_walk_id = genobj->ConnectTwoBusStops(bn_nav, bs_loc_list1[1], 
                                                  bs_loc_list2[1],
                       genmo, mo, start_time, dg, res_path);

      ///////////////////////////////////////////////////////////
      if(last_walk_id > 0){
          //////change the last bus stop/////////
          ///change  ps list2, gloc2.oid ///
          Bus_Stop cur_bs1(true, 0, 0, true);
          genobj->StringToBusStop(bn_nav->bs1_list[last_walk_id], 
                          cur_bs1);
          GenLoc temp_gloc = gloc_list2[j];
          genobj->ChangeEndBusStop(bn, dg, cur_bs1, bs_loc_list2, 
                                 temp_gloc, rel4, rtree);
          end_bus_stop_tri_id = temp_gloc.GetOid(); 
      }
      delete bn_nav;


    //////////////////////////////////////////////////////////////////
    /////4.connect the movement from end bus stop to end location////
    //////////////////////////////////////////////////////////////////

    genobj->ConnectEndStop(dg, vg, rel3, query_loc2, bs_loc_list2, 
                              end_bus_stop_tri_id,
                              genmo, mo, start_time, res_path);
    ////////////////////////////////////////////////////////////////////
    //////////////store the paths//////////////////////////////////////
    ////////////////////////////////////////////////////////////////////
     mo->EndBulkLoad(); 
     genmo->EndBulkLoad(false, false);


     res_list1.push_back(*genmo);
     res_list2.push_back(*mo);

   /////////////////////////////////////////////////////////////////////
      delete res_path;
      delete genobj; 
      delete mo;
      delete genmo;
    }

  }

  //////////////////////////////////////////////////////////////////////
  pm->CloseDualGraph(dg);
  pm->CloseVisualGraph(vg);
  sp->CloseBusNetwork(bn);
  sp->ClosePavement(pm);
  
  /////////////////////////////////////////////////////////////////////
  int index = -1;
  double time_cost = numeric_limits<double>::max();
  

  for(unsigned int i = 0;i < res_list1.size();i++){
    MPoint mp = res_list2[i];
    UPoint unit1;
    mp.Get(0, unit1);
    UPoint unit2;
    mp.Get(mp.GetNoComponents() - 1, unit2);
    double t = unit2.timeInterval.end.ToDouble() - 
               unit1.timeInterval.start.ToDouble();
    if(t < time_cost){
      index = i;
      time_cost = t;
    }
  }

  if(index >= 0){
    trip_list1.push_back(res_list1[index]);
    trip_list2.push_back(res_list2[index]);
    loc_list1.push_back(loc1);
    loc_list2.push_back(loc2);
  }
}

/*
find all bus stops that their Euclidean distance to the query location is 
less than 200 meters

*/
bool Navigation::NearestBusStop1(Point loc, Relation* rel, 
                                 R_Tree<2,TupleId>* rtree, 
                       vector<Bus_Stop>& bs_list1, vector<Point>& ps_list1, 
                       vector<Point>& ps_list2, vector<GenLoc>& gloc_list1)
{

  Point p = loc;
  SmiRecordId adr = rtree->RootRecordId();

  vector<int> tid_list;
  GenMObject* genobj = new GenMObject();
  genobj->DFTraverse1(rtree, adr, rel, p, tid_list);
  delete genobj; 

//  cout<<p<<" "<<tid_list.size()<<endl;
  
  if(tid_list.size() == 0) return false;///no bus stops with Euclidean 500m
  
  vector<MyPoint_Tid> mp_tid_list;
  for(unsigned int i = 0;i < tid_list.size();i++){
    Tuple* tuple = rel->GetTuple(tid_list[i], false);
    Point* q = (Point*)tuple->GetAttribute(BN::BN_PAVE_LOC2);
    MyPoint_Tid mp_tid(*q, q->Distance(p), tid_list[i]);
    mp_tid_list.push_back(mp_tid);
    tuple->DeleteIfAllowed();
  }
  sort(mp_tid_list.begin(), mp_tid_list.end());

  const double delta_dist = 200.0;

  if(mp_tid_list[0].dist > delta_dist){//larger than delta dist, take the first
      Tuple* bs_pave = rel->GetTuple(mp_tid_list[0].tid, false);
      Bus_Stop* bs = (Bus_Stop*)bs_pave->GetAttribute(BN::BN_BUSSTOP);
      Point* bs_loc = (Point*)bs_pave->GetAttribute(BN::BN_BUSLOC);
      GenLoc* bs_loc1 = (GenLoc*)bs_pave->GetAttribute(BN::BN_PAVE_LOC1);
      Point* bs_loc2 = (Point*)bs_pave->GetAttribute(BN::BN_PAVE_LOC2);

      bs_list1.push_back(*bs);
      ps_list1.push_back(*bs_loc2);
      ps_list2.push_back(*bs_loc);
      gloc_list1.push_back(*bs_loc1);

      bs_pave->DeleteIfAllowed();
  }else{

    for(unsigned int i = 0;i < mp_tid_list.size();i++){
      if(mp_tid_list[i].dist > delta_dist) break;

      Tuple* bs_pave = rel->GetTuple(mp_tid_list[i].tid, false);
      Bus_Stop* bs = (Bus_Stop*)bs_pave->GetAttribute(BN::BN_BUSSTOP);
      Point* bs_loc = (Point*)bs_pave->GetAttribute(BN::BN_BUSLOC);
      GenLoc* bs_loc1 = (GenLoc*)bs_pave->GetAttribute(BN::BN_PAVE_LOC1);
      Point* bs_loc2 = (Point*)bs_pave->GetAttribute(BN::BN_PAVE_LOC2);

      bs_list1.push_back(*bs);
      ps_list1.push_back(*bs_loc2);
      ps_list2.push_back(*bs_loc);
      gloc_list1.push_back(*bs_loc1);

      bs_pave->DeleteIfAllowed();

      ///////filter bus stops with the same 2D location but different routes//
      ////////this can be found later by the bus graph///////////////////
      //////////transfer without moving/////////////////////////////////
      unsigned int j = i + 1;
      if( j < mp_tid_list.size()){
        double cur_dist = mp_tid_list[i].dist;

        while(fabs(mp_tid_list[j].dist - cur_dist) < EPSDIST && 
              j < mp_tid_list.size())
          j++;

        i = j - 1;
      }

    }
  }

  if(bs_list1.size() > 0) return true; 

  return false;
  
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

  ListExpr rg_list = nl->TwoElemList(nl->StringAtom("RoadGraph Id:"),
                        nl->IntAtom(sp->GetRGId())); 

//   if(sp->GetSpeedRel() != NULL)
//     cout<<"speed rel no. "<<sp->GetSpeedRel()->GetNoTuples()<<endl;
// 
//   if(sp->GetDualNodeRel() != NULL)
//     cout<<"dg node rel no. "<<sp->GetDualNodeRel()->GetNoTuples()<<endl;
// 
//   if(sp->GetNewTriRel() != NULL)
//     cout<<"new tri rel no. "<<sp->GetNewTriRel()->GetNoTuples()<<endl;
// 
//   if(sp->GetBSPaveRel() != NULL)
//     cout<<"bus stops pave rel no."<<sp->GetBSPaveRel()->GetNoTuples()<<endl;
// 
//   if(sp->GetMSPaveRel() != NULL)
//   cout<<"metro stops pave rel no. "<<sp->GetMSPaveRel()->GetNoTuples()<<endl;
// 
//   if(sp->GetBSBuildRel() != NULL)
//     cout<<"bus stops and building rel no. "
//         <<sp->GetBSBuildRel()->GetNoTuples()<<endl;
// 
//    if(sp->GetMSBuildRel() != NULL)
//      cout<<"metro stops and building rel no. "
//          <<sp->GetMSBuildRel()->GetNoTuples()<<endl;

// return nl->TwoElemList(space_list, infra_list);
   return nl->ThreeElemList(space_list, infra_list, rg_list);

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

Space::Space():def(false), space_id(0), rg_id(0), 
speed_exist(false), tri_new_exist(false), 
dg_node_exist(false), bs_pave_exist(false), ms_pave_exist(false),
build_exist_b(false), build_exist_m(false),
infra_list(0),
street_speed(NULL), tri_new(NULL), dg_node_rid(NULL),
btree_dg_node(NULL), bs_pave_sort(NULL), rtree_bs_pave(NULL),
ms_neighbor(NULL), rtree_ms_pave(NULL),
bs_building(NULL), ms_building(NULL)
{

}
Space::Space(bool d, int id):Attribute(d), def(true), 
space_id(id), rg_id(0), 
speed_exist(false), tri_new_exist(false),
dg_node_exist(false), bs_pave_exist(false), ms_pave_exist(false),
build_exist_b(false), build_exist_m(false),
infra_list(0),
street_speed(NULL), tri_new(NULL), dg_node_rid(NULL),
btree_dg_node(NULL), bs_pave_sort(NULL), rtree_bs_pave(NULL),
ms_neighbor(NULL), rtree_ms_pave(NULL),
bs_building(NULL), ms_building(NULL)

{


}

/*
initialization function for space 

*/
Space::Space(ListExpr in_xValue, int in_iErrorPos, ListExpr& inout_xErrorInfo,
        bool& inout_bCorrect):def(false), space_id(0), rg_id(0), 
        speed_exist(false), tri_new_exist(false),
        dg_node_exist(false), bs_pave_exist(false), ms_pave_exist(false),
        build_exist_b(false), build_exist_m(false),
        infra_list(0),
        street_speed(NULL), tri_new(NULL), dg_node_rid(NULL),
        btree_dg_node(NULL), bs_pave_sort(NULL), rtree_bs_pave(NULL),
        ms_neighbor(NULL), rtree_ms_pave(NULL), 
        bs_building(NULL), ms_building(NULL)
{


}

/*
construct space from records 

*/
Space::Space(SmiRecord& in_xValueRecord, size_t& inout_iOffset, 
             const ListExpr in_xTypeInfo):def(false), 
             space_id(0), rg_id(0), 
             speed_exist(false), tri_new_exist(false), 
             dg_node_exist(false), bs_pave_exist(false), ms_pave_exist(false),
             build_exist_b(false), build_exist_m(false),
             infra_list(0),
             street_speed(NULL), tri_new(NULL), dg_node_rid(NULL),
             btree_dg_node(NULL), rtree_bs_pave(NULL),
             ms_neighbor(NULL), rtree_ms_pave(NULL), 
             bs_building(NULL), ms_building(NULL)
{

  in_xValueRecord.Read(&def, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);

  in_xValueRecord.Read(&space_id, sizeof(int), inout_iOffset);
  inout_iOffset += sizeof(int);

  in_xValueRecord.Read(&rg_id, sizeof(int), inout_iOffset);
  inout_iOffset += sizeof(int);

  in_xValueRecord.Read(&speed_exist, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);

  in_xValueRecord.Read(&tri_new_exist, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);
  
  in_xValueRecord.Read(&dg_node_exist, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);

  in_xValueRecord.Read(&bs_pave_exist, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);

  in_xValueRecord.Read(&ms_pave_exist, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);
  
  in_xValueRecord.Read(&build_exist_b, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);
  
  in_xValueRecord.Read(&build_exist_m, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);

   size_t bufsize = sizeof(FlobId) + sizeof(SmiSize) + 2*sizeof(int);
   SmiSize offset = 0;
   char* buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   assert(buf != NULL);
   infra_list.restoreHeader(buf,offset);
   free(buf);

  ListExpr xType;
  ListExpr xNumericType;
  /***********************Open relation for speed*********************/
  nl->ReadFromString(GenMObject::StreetSpeedInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(speed_exist){
    street_speed = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
    if(!street_speed) {
      return;
    }
  }
  ///////////////////////trinew relation ///////////////////////////
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(tri_new_exist){
    tri_new = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
    if(!tri_new) {
      street_speed->Delete();
      return;
    }
  }
  
  /////////////////dual graph node + route id //////////////////////
  nl->ReadFromString(DualGraph::NodeTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(dg_node_exist){
    dg_node_rid = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
    if(!dg_node_rid) {
      street_speed->Delete();
      tri_new->Delete();
      return;
    }

   nl->ReadFromString(DualGraph::BTreeNodeTypeInfo,xType);
   xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
   btree_dg_node = BTree::Open(in_xValueRecord, inout_iOffset, xNumericType);
   if(!btree_dg_node) {
     dg_node_rid->Delete();
     street_speed->Delete();
     tri_new->Delete();
     return;
   }

   /////////////////////test btree on dgnode relation///////////////////////
/*    for(int i = 1;i <= dg_node_rid->GetNoTuples();i++){
        Tuple* node_tuple = dg_node_rid->GetTuple(i, false);
        int oid = 
            ((CcInt*)node_tuple->GetAttribute(DualGraph::OID))->GetIntval();

      CcInt* search_id = new CcInt(true, oid);
      BTreeIterator* btree_iter = btree_dg_node->ExactMatch(search_id);

      while(btree_iter->Next()){
         cout<<"tid1 "<<node_tuple->GetTupleId()
             <<"tid2 "<<btree_iter->GetId()<<endl;
      }
      delete btree_iter;
      delete search_id;
      node_tuple->DeleteIfAllowed();
    }*/

  }

  ///////////////bus stops and pavement relation //////////////////////
  nl->ReadFromString(BN::BusStopsPaveTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(bs_pave_exist){
    bs_pave_sort = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
    if(!bs_pave_sort) {
      street_speed->Delete();
      tri_new->Delete();
      dg_node_rid->Delete();
      delete btree_dg_node;
      return;
    }

    Word xValue;
    if(!(rtree_bs_pave->Open(in_xValueRecord, inout_iOffset, 
                      BN::RTreeBusStopsPaveTypeInfo, xValue))){
      bs_pave_sort->Delete();
      street_speed->Delete();
      tri_new->Delete();
      dg_node_rid->Delete();
      delete btree_dg_node;
      return;
    }
    rtree_bs_pave = ( R_Tree<2,TupleId>* ) xValue.addr;

   /////////////////////test rtree on bs pave relation///////////////////////
//     for(int i = 1;i <= bs_pave_sort->GetNoTuples();i++){
//        Tuple* node_tuple = bs_pave_sort->GetTuple(i, false);
//        Point* pave_loc = (Point*)node_tuple->GetAttribute(BN::BN_PAVE_LOC2);
//        vector<int> tid_list;
//        double min_dist = 500.0;
//        DFTraverse_BS(rtree_bs_pave, bs_pave_sort, 
//               rtree_bs_pave->RootRecordId(), pave_loc, tid_list, min_dist);
//       cout<<i<<" neighbor size "<<tid_list.size()<<endl;
//       node_tuple->DeleteIfAllowed();
//     }
    //////////////////////////////////////////////////////////////////////

  }

  ///////////////metro stops and pavement relation //////////////////////
  nl->ReadFromString(MetroNetwork::MetroPaveTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(ms_pave_exist){
    ms_neighbor = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
    if(!ms_neighbor) {
      street_speed->Delete();
      tri_new->Delete();
      dg_node_rid->Delete();
      delete btree_dg_node;
      delete rtree_bs_pave;
      return;
    }

    Word xValue;
    if(!(rtree_ms_pave->Open(in_xValueRecord, inout_iOffset, 
                      MetroNetwork::RTreeMetroPaveTypeInfo, xValue))){
      street_speed->Delete();
      tri_new->Delete();
      dg_node_rid->Delete();
      delete btree_dg_node;
      delete rtree_bs_pave;
      ms_neighbor->Delete();
      return;
    }
    rtree_ms_pave = ( R_Tree<2,TupleId>* ) xValue.addr;

//     ///////////////////test rtree on ms pave relation////////////////////
//     for(int i = 1;i <= ms_neighbor->GetNoTuples();i++){
//       Tuple* node_tuple = ms_neighbor->GetTuple(i, false);
//       Point* pave_loc = 
//           (Point*)node_tuple->GetAttribute(MetroNetwork::METRO_PAVE_LOC2);
//       vector<int> tid_list;
//       double min_dist = 500.0;
//       DFTraverse_MS(rtree_ms_pave, ms_neighbor, 
//                rtree_ms_pave->RootRecordId(), pave_loc, tid_list, min_dist);
//       cout<<i<<" neighbor size "<<tid_list.size()<<endl;
//       node_tuple->DeleteIfAllowed();
//     }

  }
  
  /////////////////bus stops and building //////////////////////
  nl->ReadFromString(GenMObject::BuildingInfoB, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(build_exist_b){
    bs_building = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
    if(!bs_building) {
      street_speed->Delete();
      tri_new->Delete();
      dg_node_rid->Delete();
      delete btree_dg_node;
      delete rtree_bs_pave;
      ms_neighbor->Delete();
      delete rtree_ms_pave;
      return;
    }
  }
  
   /////////////////metro stops and building //////////////////////
    nl->ReadFromString(GenMObject::BuildingInfoM, xType);
    xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
    if(build_exist_m){
     ms_building = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
    if(!ms_building) {
      street_speed->Delete();
      tri_new->Delete();
      dg_node_rid->Delete();
      delete btree_dg_node;
      delete rtree_bs_pave;
      ms_neighbor->Delete();
      delete rtree_ms_pave;
      bs_building->Delete();
      return;
    }
  }

}

/*
traverse the rtree to find all points with a certain distance 

*/
void Space::DFTraverse_BS(R_Tree<2,TupleId>* rtree, Relation* rel,
                             SmiRecordId adr, Point* loc, 
                             vector<int>& tid_list, double& min_dist)
{

  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));

  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* tuple = rel->GetTuple(e.info, false);
              Point* p = (Point*)tuple->GetAttribute(BN::BN_PAVE_LOC2);

              double d = p->Distance(*loc);
              if(d < min_dist){
                  tid_list.push_back(e.info);
              }
              tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(loc->Distance(e.box) < min_dist){
                DFTraverse_BS(rtree, rel, e.pointer, loc, tid_list, min_dist);
            }
      }
  }
  delete node;
}

void Space::DFTraverse_MS(R_Tree<2,TupleId>* rtree, Relation* rel,
                             SmiRecordId adr, Point* loc, 
                             vector<int>& tid_list, double& min_dist)
{

  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));

  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* tuple = rel->GetTuple(e.info, false);
              Point* p = 
                (Point*)tuple->GetAttribute(MetroNetwork::METRO_PAVE_LOC2);

              double d = p->Distance(*loc);
              if(d < min_dist){
                  tid_list.push_back(e.info);
              }
              tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(loc->Distance(e.box) < min_dist){
                DFTraverse_MS(rtree, rel, e.pointer, loc, tid_list, min_dist);
            }
      }
  }
  delete node;
}

Space::~Space()
{
  if(speed_exist && street_speed != NULL) street_speed->Close();
  if(tri_new_exist && tri_new != NULL) tri_new->Close();
  if(dg_node_exist){
      if(dg_node_rid != NULL) dg_node_rid->Close(); 
      if(btree_dg_node != NULL) delete btree_dg_node;
  }

  if(bs_pave_exist){
    if(bs_pave_sort != NULL) bs_pave_sort->Close();
    if(rtree_bs_pave != NULL) delete rtree_bs_pave;
  }

  if(ms_pave_exist){
    if(ms_neighbor != NULL) ms_neighbor->Close();
    if(rtree_ms_pave != NULL) delete rtree_ms_pave;
  }
  
  if(build_exist_b){
    if(bs_building != NULL) bs_building->Close();
  }

  if(build_exist_m){
    if(ms_building != NULL) ms_building->Close();
  }
}


bool Space::SaveSpace(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveSpace()"<<endl;
  Space* sp = (Space*)value.addr;
  bool result = sp->Save(valueRecord, offset, typeInfo);

  return result;
}


bool Space::OpenSpace(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenSpace()"<<endl;
  value.addr = Space::Open(valueRecord, offset, typeInfo);
  bool result = (value.addr != NULL);

  return result;
}

Space* Space::Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo)
{
  return new Space(valueRecord,offset,typeInfo);
}

bool Space::Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
              const ListExpr in_xTypeInfo)
{
//  cout<<"save "<<endl; 
  in_xValueRecord.Write(&def, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);

  in_xValueRecord.Write(&space_id, sizeof(int), inout_iOffset);
  inout_iOffset += sizeof(int);

  in_xValueRecord.Write(&rg_id, sizeof(int), inout_iOffset);
  inout_iOffset += sizeof(int);

  in_xValueRecord.Write(&speed_exist, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);

  in_xValueRecord.Write(&tri_new_exist, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);

  in_xValueRecord.Write(&dg_node_exist, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);

  in_xValueRecord.Write(&bs_pave_exist, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);

  in_xValueRecord.Write(&ms_pave_exist, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);
  
  in_xValueRecord.Write(&build_exist_b, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);
  
  in_xValueRecord.Write(&build_exist_m, sizeof(bool), inout_iOffset);
  inout_iOffset += sizeof(bool);

  /////////////////adjacency list 1//////////////////////////////
   SecondoCatalog *ctlg = SecondoSystem::GetCatalog();
   SmiRecordFile *rf = ctlg->GetFlobFile();

   infra_list.saveToFile(rf, infra_list);
   SmiSize offset = 0;
   size_t bufsize = infra_list.headerSize()+ 2*sizeof(int);
   char* buf = (char*) malloc(bufsize);
   infra_list.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   free(buf);

   
    ListExpr xType;
    ListExpr xNumericType;
    ////////////////////street speed relation//////////////////////////
    nl->ReadFromString(GenMObject::StreetSpeedInfo, xType);
    xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
    if(speed_exist){
      if(!street_speed->Save(in_xValueRecord, inout_iOffset, xNumericType))
      return false;
    }
 
    ////////////////// triangle new relation //////////////////////
    nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType);
    xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
    if(tri_new_exist){
      if(!tri_new->Save(in_xValueRecord, inout_iOffset, xNumericType))
      return false;
    }

    ////////////dual graph node + route id relation//////////////////////////
    nl->ReadFromString(DualGraph::NodeTypeInfo, xType);
    xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
    if(dg_node_exist){
      if(!dg_node_rid->Save(in_xValueRecord, inout_iOffset, xNumericType))
        return false;

      nl->ReadFromString(DualGraph::BTreeNodeTypeInfo, xType);
      xNumericType = SecondoSystem::GetCatalog()->NumericType(xType); 
      if(!btree_dg_node->Save(in_xValueRecord, inout_iOffset, xNumericType))
          return false; 

    }

    ///////////bus stops and pavement + rtree ////////////////////////
    nl->ReadFromString(BN::BusStopsPaveTypeInfo, xType);
    xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
    if(bs_pave_exist){
      if(!bs_pave_sort->Save(in_xValueRecord, inout_iOffset, xNumericType))
        return false;

      if(!rtree_bs_pave->Save(in_xValueRecord, inout_iOffset)){
        return false;
      }
    }


    ///////////metro stops and pavement + rtree ////////////////////////
    nl->ReadFromString(MetroNetwork::MetroPaveTypeInfo, xType);
    xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
    if(ms_pave_exist){
      if(!ms_neighbor->Save(in_xValueRecord, inout_iOffset, xNumericType))
        return false;

      if(!rtree_ms_pave->Save(in_xValueRecord, inout_iOffset)){
         return false;
      }
    }

    ///////////////bus stops and building relation///////////////////////
    nl->ReadFromString(GenMObject::BuildingInfoB, xType);
    xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
    if(build_exist_b){
      if(!bs_building->Save(in_xValueRecord, inout_iOffset, xNumericType))
      return false;
    }

    ///////////////metro stops and building relation///////////////////////
    nl->ReadFromString(GenMObject::BuildingInfoM, xType);
    xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
    if(build_exist_m){
      if(!ms_building->Save(in_xValueRecord, inout_iOffset, xNumericType))
      return false;
    }

   return true; 
}

/*
put auxiliary relation into the space

*/
void Space::AddRelation(Relation* rel, int type)
{
  if(rel->GetNoTuples() > 0){
    if(type == SPEED_REL){ /////street speed 
        ListExpr ptrList1 = listutils::getPtrList(rel);

        string strQuery = "(consume(feed(" + GenMObject::StreetSpeedInfo +
                "(ptr " + nl->ToString(ptrList1) + "))))";
        Word xResult;
        int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
        assert(QueryExecuted);
        street_speed = (Relation*)xResult.addr; 

        if(street_speed != NULL) speed_exist = true;
    } else if(type == DGNODE_REL){ //dual graph node 

        ListExpr ptrList1 = listutils::getPtrList(rel);

        string strQuery = "(consume(feed(" + DualGraph::NodeTypeInfo +
                "(ptr " + nl->ToString(ptrList1) + "))))";
        Word xResult;
        int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
        assert(QueryExecuted);
        dg_node_rid = (Relation*)xResult.addr; 

      ///////////////////////////btree on nodes///////////////////////////
      ListExpr ptrList2 = listutils::getPtrList(dg_node_rid);

      strQuery = "(createbtree (" + DualGraph::NodeTypeInfo +
             "(ptr " + nl->ToString(ptrList2) + "))" + "Oid)";
      QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
      assert(QueryExecuted);
      btree_dg_node = (BTree*)xResult.addr;

      if(dg_node_rid != NULL && btree_dg_node != NULL)
          dg_node_exist = true;

    } else if(type == TRINEW_REL){ //tri new 
        ListExpr ptrList1 = listutils::getPtrList(rel);

        string strQuery = "(consume(feed(" + DualGraph::TriangleTypeInfo3 +
                "(ptr " + nl->ToString(ptrList1) + "))))";
        Word xResult;
        int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
        assert(QueryExecuted);
        tri_new = (Relation*)xResult.addr; 

        if(tri_new != NULL) tri_new_exist = true; 

    }else if(type == BSPAVESORT_REL){//bus stop pave

        ListExpr ptrList1 = listutils::getPtrList(rel);

        string strQuery = "(consume(feed(" + BN::BusStopsPaveTypeInfo +
                "(ptr " + nl->ToString(ptrList1) + "))))";
        Word xResult;
        int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
        assert(QueryExecuted);
        bs_pave_sort = (Relation*)xResult.addr; 

        /////////////////////rtree on nodes///////////////////////////
        ListExpr ptrList2 = listutils::getPtrList(bs_pave_sort);

        strQuery = "(bulkloadrtree(sortby(addid(feed (" + 
                    BN::BusStopsPaveTypeInfo +
                   " (ptr " + nl->ToString(ptrList2) + 
                   "))))((Pave_loc2 asc))) Pave_loc2)";

        QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
        assert ( QueryExecuted );
        rtree_bs_pave = ( R_Tree<2,TupleId>* ) xResult.addr;

        if(bs_pave_sort != NULL && rtree_bs_pave != NULL)
            bs_pave_exist = true; 
    }else if(type == MSPAVE_REL){ //metro stops pave 
        ListExpr ptrList1 = listutils::getPtrList(rel);

        string strQuery = "(consume(feed(" + MetroNetwork::MetroPaveTypeInfo +
                "(ptr " + nl->ToString(ptrList1) + "))))";
        Word xResult;
        int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
        assert(QueryExecuted);
        ms_neighbor = (Relation*) xResult.addr; 

        /////////////////////rtree on ms nodes ////////////////////////
        ListExpr ptrList2 = listutils::getPtrList(ms_neighbor);

        strQuery = "(bulkloadrtree(sortby(addid(feed (" + 
                    MetroNetwork::MetroPaveTypeInfo +
                   " (ptr " + nl->ToString(ptrList2) + 
                   "))))((Loc2 asc))) Loc2)";

        QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
        assert ( QueryExecuted );
        rtree_ms_pave = ( R_Tree<2,TupleId>* ) xResult.addr;

        if(ms_neighbor != NULL && rtree_ms_pave != NULL)
            ms_pave_exist = true;

    }else if(type == BSBUILD_REL){ //bus stops and building relation 

        ListExpr ptrList1 = listutils::getPtrList(rel);

        string strQuery = "(consume(feed(" + GenMObject::BuildingInfoB +
                "(ptr " + nl->ToString(ptrList1) + "))))";
        Word xResult;
        int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
        assert(QueryExecuted);
        bs_building = (Relation*) xResult.addr; 

        if(bs_building != NULL) build_exist_b = true;

    }else if(type == MSBUILD_REL){ // metro stops and building relation 
    
        ListExpr ptrList1 = listutils::getPtrList(rel);

        string strQuery = "(consume(feed(" + GenMObject::BuildingInfoM +
                "(ptr " + nl->ToString(ptrList1) + "))))";
        Word xResult;
        int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
        assert(QueryExecuted);
        ms_building = (Relation*) xResult.addr; 

        if(ms_building != NULL) build_exist_m = true;

    }

  }else{
    cout<<"empty relation "<<endl;
  }

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
road rid for a triangle oid

*/
// void Space::GetRid(int i , int& rid) const
// {
//   assert(0 <= i && i < pave_rid_list.Size());
//   pave_rid_list.Get(i, rid);
//   
// }
// 
// void Space::GetEntry(int i, EntryItem& entry)const
// {
//   assert(0 <= i && i < entry_list.Size());
//   entry_list.Get(i, entry);
// }

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
//          nl->ReadFromString(Pavement::PaveTypeInfo, xTypeInfo);
          nl->ReadFromString(DualGraph::NodeTypeInfo, xTypeInfo);
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

   }else if(infra_type == IF_BUS){ ///bus trips 
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

   }else if(infra_type == IF_METROSTOP){ ///metro stops
        MetroNetwork* mn = LoadMetroNetwork(IF_METRONETWORK);
        if(mn != NULL){ 
          result = mn->GetMS_Rel()->Clone();
          CloseMetroNetwork(mn);
        }else{
          cout<<"metro network does exist "<<endl; 
          ListExpr xTypeInfo;
          nl->ReadFromString(MetroNetwork::MetroStopsTypeInfo, xTypeInfo);
          ListExpr xNumType = 
                SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
          result = new Relation(xNumType, true);
        }

   }else if(infra_type == IF_METROROUTE){ // metro routes 
        MetroNetwork* mn = LoadMetroNetwork(IF_METRONETWORK);
        if(mn != NULL){ 
          result = mn->GetMR_Rel()->Clone();
          CloseMetroNetwork(mn);
        }else{
          cout<<"metro network does exist "<<endl; 
          ListExpr xTypeInfo;
          nl->ReadFromString(MetroNetwork::MetroRoutesTypeInfo, xTypeInfo);
          ListExpr xNumType = 
                SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
          result = new Relation(xNumType, true);
        }

   }else if(infra_type == IF_METRO){// metro trips 
      MetroNetwork* mn = LoadMetroNetwork(IF_METRONETWORK);
      if(mn != NULL){ 
        result = mn->GetMetro_Rel()->Clone();
        CloseMetroNetwork(mn);
      }else{
        cout<<"metro network does exist "<<endl; 
        ListExpr xTypeInfo;
        nl->ReadFromString(MetroNetwork::MetroTripTypeInfo, xTypeInfo);
        ListExpr xNumType = 
                SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
        result = new Relation(xNumType, true);
      }
   }else if(infra_type == IF_INDOOR){ // indoor, outdoor areas of buildings 

      IndoorInfra* i_infra = LoadIndoorInfra(IF_GROOM);

      if(i_infra != NULL){ 
        result = i_infra->BuildingType_Rel()->Clone();
        CloseIndoorInfra(i_infra);
      }else{
        cout<<"indoor infrastructure does exist "<<endl; 
        ListExpr xTypeInfo;
        nl->ReadFromString(IndoorInfra::BuildingType_Info, xTypeInfo);
        ListExpr xNumType = 
                SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
        result = new Relation(xNumType, true);
      }

   }else if(infra_type == IF_INDOORPATH){
   ///indoor, path of each building connecting to pavement 
      IndoorInfra* i_infra = LoadIndoorInfra(IF_GROOM);

      if(i_infra != NULL){ 
        result = i_infra->BuildingPath_Rel()->Clone();
        CloseIndoorInfra(i_infra);
      }else{
        cout<<"indoor infrastructure does exist "<<endl; 
        ListExpr xTypeInfo;
        nl->ReadFromString(IndoorInfra::BuildingPath_Info, xTypeInfo);
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
    cout<<"insert infrastructure wrong"<<endl; 
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
add metro network into the space 

*/
void Space::AddMetroNetwork(MetroNetwork* mn)
{
  InfraRef inf_ref;
  if(!mn->IsDefined()){
    cout<<"metro network is not defined"<<endl;
    return; 
  }
  inf_ref.infra_id = mn->GetId();
  inf_ref.infra_type = GetSymbol("METRONETWORK");
  int min_id = numeric_limits<int>::max();
  int max_id = numeric_limits<int>::min();

  Relation* mn_routes = mn->GetMR_Rel();

  for(int i = 1;i <= mn_routes->GetNoTuples();i++){
    Tuple* mr_tuple = mn_routes->GetTuple(i, false);
    int mr_id = 
       ((CcInt*)mr_tuple->GetAttribute(MetroNetwork::M_R_OID))->GetIntval();
    if(mr_id < min_id) min_id = mr_id;
    if(mr_id > max_id) max_id = mr_id;
    mr_tuple->DeleteIfAllowed(); 
  }

  Relation* mn_metro = mn->GetMetro_Rel();
  for(int i = 1;i <= mn_metro->GetNoTuples();i++){
    Tuple* metro_tuple = mn_metro->GetTuple(i, false);
    int metro_id = 
     ((CcInt*)metro_tuple->GetAttribute(MetroNetwork::M_TRIP_OID))->GetIntval();
    if(metro_id < min_id) min_id = metro_id;
    if(metro_id > max_id) max_id = metro_id; 
    metro_tuple->DeleteIfAllowed(); 
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
load the metro network 

*/
MetroNetwork* Space::LoadMetroNetwork(int type)
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
              nl->SymbolValue(xObjectType) == "metronetwork"){
            // Get name of the metro graph
            ListExpr xObjectName = nl->Second(xCurrent);
            string strObjectName = nl->SymbolValue(xObjectName);

            // Load object to find out the id of the metro network
            Word xValue;
            bool bDefined;
            bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
            if(!bDefined || !bOk){
              // Undefined
              continue;
            }
            MetroNetwork* mn = (MetroNetwork*)xValue.addr;
            if((int)mn->GetId() == rn_ref.infra_id){
              // This is the metronetwork we have been looking for
              return mn;
            }
          }
      }
  }
  return NULL;

}


void Space::CloseMetroNetwork(MetroNetwork* mn)
{
  if(mn == NULL) return; 
  Word xValue;
  xValue.addr = mn;
  SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("metronetwork"),
                                           xValue);
}


/*
add indoor infrastructure to the space 

*/
void Space::AddIndoorInfra(IndoorInfra* indoor)
{
//  cout<<"AddIndoorInfra "<<endl;

  InfraRef inf_ref; 
  if(!indoor->IsDefined()){
    cout<<"indoor infrastructure is not defined"<<endl;
    return; 
  }
  inf_ref.infra_id = indoor->GetId();
  inf_ref.infra_type = GetSymbol("GROOM"); 
  int min_id = numeric_limits<int>::max();
  int max_id = numeric_limits<int>::min();
  Relation* build_type_rel = indoor->BuildingType_Rel();
  
  for(int i = 1;i <= build_type_rel->GetNoTuples();i++){
    Tuple* build_tuple = build_type_rel->GetTuple(i, false);
    int build_id = ((CcInt*)build_tuple->GetAttribute(IndoorInfra::
                                           INDOORIF_BUILD_ID))->GetIntval();
    if(build_id < min_id) min_id = build_id;
    if(build_id > max_id) max_id = build_id; 

    build_tuple->DeleteIfAllowed(); 
  }

  inf_ref.ref_id_low = min_id;
  inf_ref.ref_id_high = max_id;

  if(CheckExist(inf_ref) == false){
      inf_ref.Print(); 
      Add(inf_ref); 
  }else{
    cout<<"insert infrastructure wrong"<<endl; 
    cout<<"infrastructure exists already or wrong oid"<<endl; 
  }

}


/*
load indoor infrastructure 

*/
IndoorInfra* Space::LoadIndoorInfra(int type)
{
//  cout<<"loadindoorinfra"<<endl; 
  
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
              nl->SymbolValue(xObjectType) == "indoorinfra"){
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
            IndoorInfra* indoor_infra = (IndoorInfra*)xValue.addr;
            if((int)indoor_infra->GetId() == rn_ref.infra_id){
            // This is the indoor infrastructure we have been looking for
              return indoor_infra;
            }
          }
      }
  }
  return NULL;

}

void Space::CloseIndoorInfra(IndoorInfra* indoor_infra)
{
//  cout<<"close indoorinfra"<<endl; 
  
  if(indoor_infra == NULL) return; 
    Word xValue;
    xValue.addr = indoor_infra;
    SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom( "indoorinfra"),
                                           xValue);
}

void Space::AddRoadGraph(RoadGraph* rg)
{
  if(rg->GetRG_ID() > 0){
    rg_id = rg->GetRG_ID();
  }

}

/*
load road graph into space

*/
RoadGraph* Space:: LoadRoadGraph()
{
  ListExpr xObjectList = SecondoSystem::GetCatalog()->ListObjects();
  xObjectList = nl->Rest(xObjectList);
  while(!nl->IsEmpty(xObjectList)){
          // Next element in list
      ListExpr xCurrent = nl->First(xObjectList);
      xObjectList = nl->Rest(xObjectList);
          // Type of object is at fourth position in list
      ListExpr xObjectType = nl->First(nl->Fourth(xCurrent));
      if(nl->IsAtom(xObjectType) &&
          nl->SymbolValue(xObjectType) == "roadgraph"){
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
            RoadGraph* rg = (RoadGraph*)xValue.addr;
            if((int)rg->GetRG_ID() == rg_id){
            // This is the indoor infrastructure we have been looking for
              return rg;
            }
          }
      }
  return NULL;
}


void Space::CloseRoadGraph(RoadGraph* rg)
{
   if(rg == NULL) return; 
    Word xValue;
    xValue.addr = rg;
    SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom( "roadgraph"),
                                           xValue);
}


/*
open all infrastructures
"BUSSTOP", "BUSROUTE", "MPPTN", "BUSNETWORK", "GROOM", 
"REGION", "LINE", "FREESPACE"

*/
void Space::OpenInfra(vector<void*>& infra_pointer)
{

  infra_pointer.push_back(NULL);//bus stop 
  infra_pointer.push_back(NULL);//bus route
  infra_pointer.push_back(NULL);//mpptn
  infra_pointer.push_back(LoadBusNetwork(IF_BUSNETWORK));//bus network
  infra_pointer.push_back(LoadIndoorInfra(IF_GROOM));//indoor 
  infra_pointer.push_back(LoadPavement(IF_REGION));// region based outdoor
  infra_pointer.push_back(LoadRoadNetwork(IF_LINE));// road netowrk
  infra_pointer.push_back(NULL);// free space
  infra_pointer.push_back(LoadMetroNetwork(IF_METRONETWORK));// metro network 


  if((BusNetwork*)infra_pointer[IF_BUSNETWORK] == NULL){
    cerr<<__FILE__<<__LINE__<<"bus network can not be opend"<<endl;
  }

  if((Network*)infra_pointer[IF_LINE] == NULL){
    cerr<<__FILE__<<__LINE__<<"road network can not be opend"<<endl;
  }


  if((Pavement*)infra_pointer[IF_REGION] == NULL){
    cerr<<__FILE__<<__LINE__<<"pavement can not be opend"<<endl;
  }

  if((IndoorInfra*)infra_pointer[IF_GROOM] == NULL){
    cerr<<__FILE__<<__LINE__<<"indoor infrastructure can not be opend"<<endl;
  }

  if((MetroNetwork*)infra_pointer[IF_METRONETWORK] == NULL){
    cerr<<__FILE__<<__LINE__<<"metro network can not be opend"<<endl;
  }

}

/*
close all infrastructures

*/
void Space::CloseInfra(vector<void*>& infra_pointer)
{
  if(infra_pointer[IF_BUSNETWORK] != NULL)
    CloseBusNetwork((BusNetwork*)infra_pointer[IF_BUSNETWORK]);

   if(infra_pointer[IF_GROOM] != NULL)
     CloseIndoorInfra((IndoorInfra*)infra_pointer[IF_GROOM]);


  if(infra_pointer[IF_REGION] != NULL)
    ClosePavement((Pavement*)infra_pointer[IF_REGION]);

  if(infra_pointer[IF_LINE] != NULL)
    CloseRoadNetwork((Network*)infra_pointer[IF_LINE]);

  ////////////metro network //////////////////////////

  if(infra_pointer[IF_METRONETWORK] != NULL)
    CloseMetroNetwork((MetroNetwork*)infra_pointer[IF_METRONETWORK]);
}


/*
from the infrastructure object oid, it gets the infrastructure type 

*/
int Space::GetInfraType(int oid)
{
  int infra_type = -1;
  if(oid == 0){ ///////////////in free space
     infra_type = IF_FREESPACE;
  }else{
    if(oid > MaxRefId()){////////////indoor movement 
      infra_type = IF_GROOM;
    }else{

      for(int i = 0;i < infra_list.Size();i++){
        InfraRef elem;
        infra_list.Get(i, elem); 

        if(elem.ref_id_low <= oid && oid <= elem.ref_id_high){
          infra_type = elem.infra_type;
          break;
        }
      }
    }
  }

  if(infra_type < 0){
    cout<<"error no such infrastructure object"<<endl;
    assert(false);

  }

  return infra_type; 
}

/*
get the maximum reference id in the current space 

*/
int Space::MaxRefId()
{
  int max_ref_id = 0;
  
  for(int i = 0;i < infra_list.Size();i++){
      InfraRef elem;
      infra_list.Get(i, elem); 
      if(elem.ref_id_high > max_ref_id){
        max_ref_id = elem.ref_id_high;
      }
  }
  return max_ref_id;
}

/*
get the movement inside an infrastructure object 

*/
void Space::GetLineInIFObject(int& oid, GenLoc gl1, GenLoc gl2, 
                              Line* l, vector<void*> infra_pointer, 
                              Interval<Instant> time_interval, 
                              int infra_type)
{
  assert(oid >= 0); 

  switch(infra_type){
    case IF_LINE:
//      cout<<"road network "<<endl;
      GetLineInRoad(oid, gl1, gl2, l, (Network*)infra_pointer[IF_LINE]);
      break;

    case IF_REGION:
//      cout<<"region based outdoor"<<endl;
      GetLineInRegion(oid, gl1, gl2, l);
      break; 

    case IF_FREESPACE:
//      cout<<"free space"<<endl;
      GetLineInFreeSpace(gl1, gl2, l);
      break;

    case IF_BUSNETWORK:
//      cout<<"bus network"<<endl;
      GetLineInBusNetwork(oid, l,
                          (BusNetwork*)infra_pointer[IF_BUSNETWORK],
                          time_interval);
      break;

    case IF_GROOM:
//      cout<<"indoor "<<endl;
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
void Space::GetLineInRoad(int oid, GenLoc gl1, GenLoc gl2, Line* l, Network* rn)
{
  
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
  
  if(AlmostEqual(p1, p2)) return; 

  HalfSegment hs(true, p1, p2);
  hs.attr.edgeno = 0;
  *l += hs;
  hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
  *l += hs;
}

/*
get the sub movement in bus network, bus routes 
can be optimized that for the same bus trip, the same bus route. it does not
have to access many times, only once is enough. oid corresponds to a bus route

*/
void Space::GetLineInBusNetwork(int& oid, Line* l, BusNetwork* bn, 
                                Interval<Instant> time_range)
{
  //////////////////////////////////////////////////////
  ///////////////change the oid to be bus route uid/////
  /////////////////////////////////////////////////////
  MPoint mp(0);
  int br_uoid = 0;
  bn->GetMOBUS(oid, mp, br_uoid);
//  cout<<mp.GetNoComponents()<<endl;
//  cout<<"br ref id "<<br_uoid<<endl; 
  oid = br_uoid;

  SimpleLine br_sl(0);
  bn->GetBusRouteGeoData(br_uoid, br_sl);
//  cout<<"bus route length "<<br_sl.Length()<<endl; 


  Periods* peri = new Periods(0);
  peri->StartBulkLoad();
  peri->MergeAdd(time_range);
  peri->EndBulkLoad();
  MPoint sub_mp(0);
  mp.AtPeriods(*peri, sub_mp);
  delete peri; 

//  cout<<sub_mp.GetNoComponents()<<endl; 

  Line l1(0);
  sub_mp.Trajectory(l1);
//  cout<<"line size "<<l1.Size()<<endl;
  if(l1.Size() > 0){
    Rectangle<2> bbox = br_sl.BoundingBox();
    int edgeno = 0;
    for(int i = 0;i < l1.Size();i++){
      HalfSegment hs1;
      l1.Get(i, hs1);
      if(!hs1.IsLeftDomPoint()) continue;
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
  }

}

/*
get the sub movement in indoor environment
be careful with the movement inside an elevator or on a staircase 
the movement in an office room, corridor or bath room is clear 

*/
void Space::GetLineInGRoom(int oid, GenLoc gl1, GenLoc gl2, Line* l)
{
//  cout<<"indoor not implemented"<<endl;
  ////////////if the room is a staircase, then l is empty////////////////
  ////////       Loc loc_1(p1.GetZ(), -1);  //////////////////////////
  /////////////  Loc loc_2(p2.GetZ(), -1); ///////////////////////////
  ////////////////////////////////////////////////////////////////////
  if(gl1.GetLoc().loc2 < 0.0 && gl2.GetLoc().loc2 < 0.0){
//    cout<<"movement inside an elevator 2d line is empty"<<endl;

  }else{
//    cout<<"not inside an elevator "<<endl;

    Point p1(true, gl1.GetLoc().loc1, gl1.GetLoc().loc2);
    Point p2(true, gl2.GetLoc().loc1, gl2.GetLoc().loc2);

    if(AlmostEqual(p1, p2)) return; 

    HalfSegment hs(true, p1, p2);
    hs.attr.edgeno = 0;
    *l += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    *l += hs;

  }

}

/*
get the distance betwene a genloc and a point 

*/

double Space::Distance(GenLoc* gloc, Point* p)
{
  int oid = gloc->GetOid();
  int type = GetInfraType(oid);
  if(type < 0){
    cout<<"invalid location"<<endl;
    return -1;
  }
  double res = -1;

  if(type == IF_FREESPACE){
      Point loc(true, gloc->GetLoc().loc1, gloc->GetLoc().loc2);
      res = p->Distance(loc);
  }else if(type == IF_LINE){
//      cout<<"road network"<<endl;
      Point loc;
      if(GetLocOnRoad(gloc, loc)){
        res = p->Distance(loc);
      }
  }else if(type == IF_BUSNETWORK){
    Point loc;
    if(GetLocInBN(gloc, loc)){
      res = p->Distance(loc);
    }
  }else{

    cout<<"invalid type"<<endl;
  }

  return res;

}
/*
get the distance between a genloc and a line 

*/
double Space::Distance(GenLoc* gloc, Line* l)
{
  int oid = gloc->GetOid();
  int type = GetInfraType(oid);
  if(type < 0){
    cout<<"invalid location"<<endl;
    return -1;
  }

  double res = -1;

  if(type == IF_FREESPACE){
      Point loc(true, gloc->GetLoc().loc1, gloc->GetLoc().loc2);
      res = l->Distance(loc);
  }else if(type == IF_LINE){
//      cout<<"road network "<<endl;
      Point loc;
      if(GetLocOnRoad(gloc, loc)){
        res = l->Distance(loc);
//        cout<<"dist "<<res<<endl;
      }
  }else if(type == IF_BUSNETWORK){
    Point loc;
    if(GetLocInBN(gloc, loc)){
      res = l->Distance(loc);
    }
  }else{

    cout<<"invalid type"<<endl;
  }

  return res;
}

/*
get the point of a genloc on a road 

*/
bool Space::GetLocOnRoad(GenLoc* gloc, Point& loc)
{

  static unsigned long line_init;
  static unsigned long cmd_no; 
  static vector<SimpleLine> roads;
  static vector<bool> dir;
  static vector<unsigned int> oid_list;


  if(line_init == 0){ ////3030 cars 1.68 seconds 
    Network* rn = LoadRoadNetwork(IF_LINE);
    if(rn == NULL) return false;
    Relation* roads_rel = rn->GetRoutes();
    for(int i = 1;i <= roads_rel->GetNoTuples();i++){
      Tuple* tuple = roads_rel->GetTuple(i, false);
      int rid = ((CcInt*)tuple->GetAttribute(ROUTE_ID))->GetIntval();
      SimpleLine* sl = (SimpleLine*)tuple->GetAttribute(ROUTE_CURVE);
      roads.push_back(*sl);
      dir.push_back(sl->GetStartSmaller());
      oid_list.push_back(rid);
      tuple->DeleteIfAllowed();
    }
    CloseRoadNetwork(rn);
    line_init = (unsigned long)this;

    /////////get the command number ///////////////////////
    SystemTables& st = SystemTables::getInstance();
    const SystemInfoRel* sysinfo = st.getInfoRel("SEC2COMMANDS");
    cmd_no = sysinfo->tuples.size();

  }else{
    unsigned long sp_addr = (unsigned long)this;  
    if(line_init != sp_addr){
      Network* rn = LoadRoadNetwork(IF_LINE);
      if(rn == NULL) return false;
      Relation* roads_rel = rn->GetRoutes();
      roads.clear();
      dir.clear();
      oid_list.clear();
      for(int i = 1;i <= roads_rel->GetNoTuples();i++){
        Tuple* tuple = roads_rel->GetTuple(i, false);
        int rid = ((CcInt*)tuple->GetAttribute(ROUTE_ID))->GetIntval();
        SimpleLine* sl = (SimpleLine*)tuple->GetAttribute(ROUTE_CURVE);
        roads.push_back(*sl);
        dir.push_back(sl->GetStartSmaller());
        oid_list.push_back(rid);
        tuple->DeleteIfAllowed();
      }
      CloseRoadNetwork(rn);
      line_init = (unsigned long)this;
    }else{
        SystemTables& st = SystemTables::getInstance();
        const SystemInfoRel* sysinfo = st.getInfoRel("SEC2COMMANDS");
        if(cmd_no != sysinfo->tuples.size()){

            Network* rn = LoadRoadNetwork(IF_LINE);
            if(rn == NULL) return false;
            Relation* roads_rel = rn->GetRoutes();
            roads.clear();
            dir.clear();
            oid_list.clear();
            for(int i = 1;i <= roads_rel->GetNoTuples();i++){
              Tuple* tuple = roads_rel->GetTuple(i, false);
              int rid = ((CcInt*)tuple->GetAttribute(ROUTE_ID))->GetIntval();
              SimpleLine* sl = (SimpleLine*)tuple->GetAttribute(ROUTE_CURVE);
              roads.push_back(*sl);
              dir.push_back(sl->GetStartSmaller());
              oid_list.push_back(rid);
              tuple->DeleteIfAllowed();
            }
            CloseRoadNetwork(rn);
            line_init = (unsigned long)this;
            cmd_no = sysinfo->tuples.size();
        }
    }
  }

//   Network* rn = LoadRoadNetwork(IF_LINE); //3030 cars, 36 seconds 
//   if(rn == NULL) return false;
//   CloseRoadNetwork(rn);
  if((int)(roads.size()) == 0 || (int)(dir.size()) == 0) return false;
  ///////////get the position on the road ///////////////////
  if(gloc->GetOid() != oid_list[gloc->GetOid() - 1]){
    cout<<"roads should be inserted first into space"<<endl;
    cout<<"roads should be ordered by rid"<<endl;
  }
  assert(gloc->GetOid() == oid_list[gloc->GetOid() - 1]);
//  cout<<gloc->GetOid()<<" "<<oid_list[gloc->GetOid() - 1]<<endl;
  int index = gloc->GetOid();

//  cout<<roads.size()<<" size "<<roads[0].Length()<<endl;

  assert(roads[index - 1].AtPosition(gloc->GetLoc().loc1, 
                                     dir[index - 1], loc));
//  cout<<loc<<endl;
  return true;
}


/*
get a location on bus network 

*/

bool Space::GetLocInBN(GenLoc* gloc, Point& loc)
{
  double x = gloc->GetLoc().loc1;
  double y = gloc->GetLoc().loc2;
  if(x < 0.0 || y < 0.0) return false;
  
  loc.Set(x, y);
  return true;

}

