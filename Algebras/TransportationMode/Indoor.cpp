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

[1] Source File of the Transportation Mode Algebra

Dec, 2010 Jianqiu Xu

[TOC]

1 Overview

This source file essentially contains the necessary implementations for
indoor environment 

*/

#include "Indoor.h"
////////////////////////////////////////////////////////////////////////
/////////////////////////// Point3D ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Point3D::Point3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo)
{
  valueRecord.Read(&x, sizeof(double), offset);
  offset += sizeof(double);
  valueRecord.Read(&y, sizeof(double), offset);
  offset += sizeof(double);
  valueRecord.Read(&z, sizeof(double), offset);
  offset += sizeof(double);
  SetDefined(true);
}

bool Point3D::Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo)
{
//  cout<<"Point3D::Save()"<<endl;
  valueRecord.Write(&x, sizeof(double),offset);
  offset += sizeof(double);
  valueRecord.Write(&y, sizeof(double),offset);
  offset += sizeof(double);
  valueRecord.Write(&z, sizeof(double), offset);
  offset += sizeof(double);
//  cout<<x<<" "<<y<<" "<<z<<endl;
  return true;
}

///////////////////////////////////////////////////////////////////////////
/////////////////////// Line3D ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void Line3D::RemoveDuplicates()
{

}

/*
sort 3d points in a line 

*/
void Line3D::Sort(const bool exact /*= true*/)
{

}

void Line3D::Clear()
{
  points.clean();
}

void Line3D::StartBulkLoad()
{

}

Line3D& Line3D::operator+=( const Point3D& p )
{
  if(!IsDefined()){
    return *this;
  }
  points.Append(p);
  return *this;
}

bool Line3D::IsValid() const
{
  return true;
}

size_t Line3D::HashValue() const
{
  return (size_t)0.0;
}

void Line3D::CopyFrom( const Attribute* right )
{
//  cout<<"Line3D CopyFrom"<<endl; 
  const Line3D *ps = (const Line3D*)right;
  assert( ps->IsOrdered() );
  *this = *ps;
}

/*
end bulkload for a line3d 

*/
void Line3D::EndBulkLoad( bool sort, bool remDup, bool trim )
{
  if( !IsDefined() ) {
    Clear();
    SetDefined( false );
  }

  if( sort ){
    Sort();
  }

  if( remDup ){
    RemoveDuplicates();
  }
  if(trim){
    points.TrimToSize();
  }
}

Line3D& Line3D::operator=( const Line3D& ps )
{
//  cout<<"Line3D = "<<endl; 
  assert( ps.IsOrdered() );
//  points.copyFrom(ps.points);
  
  points.clean();
  Line3D* l = const_cast<Line3D*>(&ps);
  for(int i = 0;i < l->Size();i++){
    Point3D p;
    l->Get(i, p);
    points.Append(p);
  }

  SetDefined(ps.IsDefined());
  return *this;
}

bool Line3D::Adjacent( const Attribute* arg ) const
{
  return 0;
}

int Line3D::Compare( const Attribute* arg ) const
{
  return 0;
}

int Line3D::CompareAlmost( const Attribute* arg ) const
{
  return 0;
}


double Line3D::Distance( const Rectangle<3>& r ) const
{
  return 0.0;
}

double Line3D::Length()
{
  double length = 0.0; 
  for(int i = 0;i < points.Size();i++){
    if(i == points.Size() - 1)continue;
    Point3D p1;
    points.Get(i, p1);
    Point3D p2;
    points.Get( + 1, p2);
    length += p1.Distance(p2); 
  }
  return length; 
}

void Line3D::Print()
{
  for(int i = 0;i < points.Size();i++){
    Point3D p;
    points.Get(i , p);
    p.Print();
  }

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////  Floor3D /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/*
Constructor function for Floor3D

*/
Floor3D::Floor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo):
StandardSpatialAttribute<2>(true),reg(0)
{
//  cout<<"Floor3D(SmiRecord& , size_t& , const ListExpr) here"<<endl;
  valueRecord.Read(&floor_height, sizeof(float), offset);
  offset += sizeof(float);
  ListExpr xType = nl->SymbolAtom("region");
  ListExpr xNumericType =
    SecondoSystem::GetCatalog()->NumericType(xType);
  Region* r = (Region*)Attribute::Open(valueRecord,offset,xNumericType);
  reg = *r;
//  cout<<*r<<endl; 
  delete r;
  SetDefined(true);
}

bool Floor3D::Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo)
{
//  cout<<"Save()"<<endl;
  valueRecord.Write(&floor_height, sizeof(float),offset);
  offset += sizeof(float);
  ListExpr xType = nl->SymbolAtom("region");
  ListExpr xNumericType =
    SecondoSystem::GetCatalog()->NumericType(xType);
  Attribute::Save(valueRecord, offset, xNumericType, &reg);
  return true;
}

/*
Creation of the type constructor instance for floor3d

*/
ListExpr Floor3DProperty()
{
//  cout<<"Floor3DProperty()"<<endl;
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"thefloor(floor_height, polygon)");
  return nl->TwoElemList(
          nl->TwoElemList(nl->StringAtom("Creation"),
                          nl->StringAtom("Example Creation")),
          nl->TwoElemList(examplelist,
                         nl->StringAtom("(let room1=thefloor(0, r))"))

      );
}

/*
OutPut function for floor3d

*/
ListExpr OutFloor3D(ListExpr typeInfo, Word value)
{
//  cout<<"OutFloor3D()"<<endl;
  Floor3D* fl = (Floor3D*)(value.addr);

  if(!fl->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  if( fl->IsEmpty() ){
    return (nl->TheEmptyList());
  }
  else{
    Region* cr = const_cast<Region*>(fl->GetRegion());
    ListExpr regionNL = OutRegion(nl->TheEmptyList(), SetWord(cr));
    return nl->TwoElemList(nl->RealAtom(fl->GetHeight()), regionNL);
  }
}

/*
input function for data type floor3d
a float value for ground height and a region for space covered

*/
Word InFloor3D(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct)
{
//  cout<<"InFloor3D()"<<endl;
  if(nl->ListLength(instance) != 2){
    string strErrorMessage = "floor3d(): List length must be 2";
    errorInfo = nl->Append(errorInfo,nl->StringAtom(strErrorMessage));
    correct = false;
    return SetWord(Address(0));
  }

  if (nl->IsAtom(instance)){
     correct=false;
     return SetWord( Address(0) );
  }

  if(nl->IsEqual(instance,"undef")){
    correct=false;
    return SetWord(Address(0));
  }

  ListExpr height_list = nl->First(instance);
  if(!nl->IsAtom(height_list) || nl->AtomType(height_list) != RealType){
    string strErrorMessage = "floor3d(): height must be float type";
    errorInfo = nl->Append(errorInfo,nl->StringAtom(strErrorMessage));
    correct = false;
    return SetWord(Address(0));
  }
  float height = nl->RealValue(height_list);
//  cout<<"height "<<height<<endl;



    /////  new implementation from Thomas 
    Word reg_addr= InRegion(typeInfo,nl->Second(instance),errorPos,
                                     errorInfo,correct);

    Region* cr = (Region*)reg_addr.addr; 
    Floor3D* fl = new Floor3D(height, *cr);
//    delete cr; 
    return SetWord(fl);

}

               
void CloseFloor3D(const ListExpr typeInfo, Word& w)
{
//  cout<<"CloseFloor3D()"<<endl;
//  ((Floor3D*)w.addr)->DeleteIfAllowed();
  delete static_cast<Floor3D*> (w.addr);
  w.addr = NULL;
}

Word CloneFloor3D(const ListExpr typeInfo, const Word& w)
{
//  cout<<"CloneFloor3D()"<<endl;
  Floor3D* fl = new Floor3D(*(Floor3D*)w.addr);
  return SetWord(fl);
}

Word CreateFloor3D(const ListExpr typeInfo)
{
//  cout<<"CreateFloor3D()"<<endl;
  return SetWord (new Floor3D(0.0));
}

void DeleteFloor3D(const ListExpr typeInfo, Word& w)
{
//  cout<<"DeleteFloor3D()"<<endl;
  Floor3D* fl = (Floor3D*)w.addr;
  delete fl;
   w.addr = NULL;
}


int SizeOfFloor3D()
{
//  cout<<"SizeOfFloor3D()"<<endl;
  return sizeof(Floor3D);
}

bool CheckFloor3D(ListExpr type, ListExpr& errorInfo)
{
//  cout<<"CheckFloor3D()"<<endl;
  return nl->IsEqual(type, "floor3d");
}

/*
open function for floor3d

*/

bool OpenFloor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenFloor3D()"<<endl;
  value.addr = new Floor3D(valueRecord, offset, typeInfo);
  return value.addr != NULL;
}

/*
save function for floor3d

*/
bool SaveFloor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//    cout<<"SaveFloor3D()"<<endl;
    Floor3D* fl = (Floor3D*)value.addr;
    return fl->Save(valueRecord, offset, typeInfo);
}
///////////////////////////////////////////////////////////////////////////
///////////////////////////// Door3D /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/*
Constructor function for Door3D

*/
Door3D::Door3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo):StandardSpatialAttribute<2>(true),
                 door_pos1(0),door_pos2(0), tpstate(0)
{

  valueRecord.Read(&oid1, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);
  ListExpr xType = nl->SymbolAtom("line");
  ListExpr xNumericType =
    SecondoSystem::GetCatalog()->NumericType(xType);
  Line* l1 = (Line*)Attribute::Open(valueRecord,offset,xNumericType);
  door_pos1 = *l1;
  delete l1;

  valueRecord.Read(&oid2, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);
  Line* l2 = (Line*)Attribute::Open(valueRecord,offset,xNumericType);
  door_pos2 = *l2;
  delete l2;
  

  xType = nl->SymbolAtom("mbool");
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  MBool* mb = (MBool*)Attribute::Open(valueRecord,offset,xNumericType);
  tpstate = *mb; 
//  cout<<tpstate<<endl; 
  
  
  valueRecord.Read(&lift_door, sizeof(bool), offset);
  offset += sizeof(bool);

  SetDefined(true);
}

bool Door3D::Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo)
{
//  cout<<"Save()"<<endl;
  
  valueRecord.Write(&oid1, sizeof(unsigned int),offset);
  offset += sizeof(unsigned int);
  ListExpr xType = nl->SymbolAtom("line");
  ListExpr xNumericType =
    SecondoSystem::GetCatalog()->NumericType(xType);
  Attribute::Save(valueRecord, offset, xNumericType, &door_pos1);

  valueRecord.Write(&oid2, sizeof(unsigned int),offset);
  offset += sizeof(unsigned int);
  Attribute::Save(valueRecord, offset, xNumericType, &door_pos2);


  xType = nl->SymbolAtom("mbool");
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  Attribute::Save(valueRecord, offset, xNumericType, &tpstate);
  
  valueRecord.Write(&lift_door, sizeof(bool), offset);
  offset += sizeof(bool);
  return true;
}


/*
Door3D Property function 

*/
ListExpr Door3DProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("door3d"),
           nl->StringAtom("(<door3d>) (int line int line mbool bool)"),
           nl->StringAtom("(oid1 l1 oid2 l2 doorstat type)"))));
}


ListExpr OutDoor3D( ListExpr typeInfo, Word value )
{
//  cout<<"OutDoor3D()"<<endl; 
  Door3D* dr = (Door3D*)(value.addr);
  if(!dr->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  if( dr->IsEmpty() ){
    return nl->TheEmptyList();
  }
  
  
  ListExpr l1 = OutLine(nl->TheEmptyList(), SetWord(dr->GetLoc(1)));
  ListExpr l2 = OutLine(nl->TheEmptyList(), SetWord(dr->GetLoc(2)));
  ListExpr tempstate = OutMapping<MBool,UBool,
  OutConstTemporalUnit<CcBool,OutCcBool> >(nl->TheEmptyList(), 
                                          SetWord(dr->GetTState()));
  
  ListExpr list1 = nl->TwoElemList(nl->IntAtom(dr->GetOid(1)), l1);
  ListExpr list2 = nl->TwoElemList(nl->IntAtom(dr->GetOid(2)), l2);
  return nl->FourElemList(list1, list2, 
                          tempstate, nl->BoolAtom(dr->GetDoorType()));

}

/*
In function for door3d 

*/
Word InDoor3D( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if(nl->IsEqual(instance,"undef")) {
      Door3D* dr = new Door3D();
      dr->SetDefined(false);
      correct=true;
      return SetWord(Address(dr));
  }
  ////////////////////////////////////////////////////////////////////////
  /////////////////////// get the first genrange /////////////////////////
  ///////////////////////////////////////////////////////////////////////
  ListExpr first_instance = nl->First(instance); 
  ListExpr oid_list1 = nl->First(first_instance);

  if(!nl->IsAtom(oid_list1) || nl->AtomType(oid_list1) != IntType){
    string strErrorMessage = "genrange(): obj id must be int type";
    errorInfo = nl->Append(errorInfo,nl->StringAtom(strErrorMessage));
    correct = false;
    return SetWord(Address(0));
  }
 
  unsigned int oid1 = nl->IntValue(oid_list1);
  ListExpr rest = nl->Second(first_instance);
  Line* l1 = (Line*)(InLine(typeInfo,rest,errorPos,errorInfo,correct).addr);

  ///////////////////////////////////////////////////////////////////////////
  ////////////////////////  get the second oid and line //////////////////////
  ///////////////////////////////////////////////////////////////////////////
  ListExpr second_instance = nl->Second(instance); 
  ListExpr oid_list2 = nl->First(second_instance);

  if(!nl->IsAtom(oid_list2) || nl->AtomType(oid_list2) != IntType){
    string strErrorMessage = "genrange(): obj id must be int type";
    errorInfo = nl->Append(errorInfo,nl->StringAtom(strErrorMessage));
    correct = false;
    return SetWord(Address(0));
  }

  unsigned int oid2 = nl->IntValue(oid_list2);
  rest = nl->Second(second_instance);
  Line* l2 = (Line*)(InLine(typeInfo,rest,errorPos,errorInfo,correct).addr);

  ////////////////////////////////////////////////////////////////////////
  /////////////  time-dependent state mbool //////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  ListExpr temporal_state = nl->Third(instance);
  int numUnits = 0;
  if(nl->AtomType(temporal_state)==NoAtom){
    numUnits = nl->ListLength(temporal_state);
  }
  MBool* m = new MBool( numUnits );
  correct = true;
  int unitcounter = 0;
  string errmsg;

  m->StartBulkLoad();

  rest = temporal_state;
  if (nl->AtomType( rest ) != NoAtom)
  { if(nl->IsEqual(rest,"undef")){
       m->EndBulkLoad();
       m->SetDefined(false);
       return SetWord( Address( m ) );
    } else {
      correct = false;
      m->Destroy();
      delete m;
      return SetWord( Address( 0 ) );
    }
  }
  else while( !nl->IsEmpty( rest ) )
  {
    ListExpr first = nl->First( rest );
    rest = nl->Rest( rest );

//    UBool *unit = (UBool*)InUnit( nl->TheEmptyList(), first,
//                                errorPos, errorInfo, correct ).addr;
    UBool *unit = (UBool*)InConstTemporalUnit<CcBool,InCcBool>( 
                                nl->TheEmptyList(), first,
                                errorPos, errorInfo, correct ).addr;

    if ( !correct )
    {
      errmsg = "InMapping(): Representation of Unit "
          + int2string(unitcounter) + " is wrong.";
      errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      m->Destroy();
      delete m;
      return SetWord( Address(0) );
    }
    if(  !unit->IsDefined() || !unit->IsValid() )
    {
      errmsg = "InMapping(): Unit " + int2string(unitcounter) + " is undef.";
      errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      correct = false;
      delete unit;
      m->Destroy();
      delete m;
      return SetWord( Address(0) );
    }
    m->Add( *unit );
    unitcounter++;
    delete unit;
  }

  m->EndBulkLoad( true ); 
//  cout<<*m<<endl; 
  ////////////////////////////////////////////////////////////////////////
  ////////////// type of door: non-lift or lift////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  ListExpr door_type = nl->Fourth(instance);
  if(!nl->IsAtom(door_type) || nl->AtomType(door_type) != BoolType){
    cout<< "door3d(): type must be bool type"<<endl;
    correct = false;
    return SetWord(Address(0));
  }

  bool dr_type = nl->BoolValue(door_type);
  ///////////////////////////////////////////////////////////////////////
  Door3D* dr = new Door3D(oid1, oid2,*l1,*l2, *m, dr_type);

  delete l1;
  delete l2;
  delete m; 
  return SetWord(dr);

}


Word CreateDoor3D(const ListExpr typeInfo)
{
//  cout<<"CreateDoor3D()"<<endl;
  return SetWord (new Door3D(false));
}


void DeleteDoor3D(const ListExpr typeInfo, Word& w)
{
//  cout<<"DeleteDoor3D()"<<endl;
  Door3D* dr = (Door3D*)w.addr;
  delete dr;
   w.addr = NULL;
}

bool OpenDoor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenDoor3D()"<<endl;
  value.addr = new Door3D(valueRecord, offset, typeInfo);
  return value.addr != NULL;
}


/*
save function for GenRange 

*/
bool SaveDoor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//    cout<<"SaveDoor3D()"<<endl;
    Door3D* dr = (Door3D*)value.addr;
    return dr->Save(valueRecord, offset, typeInfo);
}

void CloseDoor3D( const ListExpr typeInfo, Word& w )
{
  ((Door3D*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word CloneDoor3D( const ListExpr typeInfo, const Word& w )
{
  return SetWord( new Door3D( *((Door3D *)w.addr) ) );
}

void* CastDoor3D(void* addr)
{
  return (new (addr) Door3D());
}

int SizeOfDoor3D()
{
  return sizeof(Door3D);
}

bool CheckDoor3D( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "door3d" ));
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////  GRoom  //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/*
old version of region input 
pointlist is to store the point in such a way that it is the same as for the
input. 
1) Outercycle in a clock-wise and holes in a counter-clock wise 
2) index list stores the start position of the first point for the outercycle 
and the hole 

*/
Word MyInRegion(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct, 
                int& cycno)
{

    Region* cr = new Region( 0 );
    cr->StartBulkLoad();

    ListExpr RegionNL = instance;
    ListExpr FaceNL, CycleNL;
    int fcno = -1;
    int ccno = -1;
    int edno = -1;
    int partnerno = 0;
    if(nl->ListLength(RegionNL) > 1){
       correct = false;
       return SetWord(Address(0));
    }

    FaceNL = nl->First( RegionNL );
    bool isCycle = true;

    //A face is composed by 1 cycle, and can have holes.
    //All the holes must be inside the face. (TO BE IMPLEMENTED0)
    //Region *faceCycle;

    fcno++;
    ccno=-1;
    edno=-1;

    if (nl->IsAtom( FaceNL )){
        correct=false;
        return SetWord( Address(0) );
    }

    while (!nl->IsEmpty( FaceNL) ){ //several cycles (outer and hole)
        CycleNL = nl->First( FaceNL );
        FaceNL = nl->Rest( FaceNL );


        ccno++;
        edno=-1;

        if (nl->IsAtom( CycleNL )){
          correct=false;
          return SetWord( Address(0) );
        }

        if (nl->ListLength( CycleNL) <3){
          cerr << __PRETTY_FUNCTION__ << ": A cycle must have at least 3 edges!"
               << endl;
          correct=false;
          return SetWord( Address(0) );
        }else{
          ListExpr firstPoint = nl->First( CycleNL );
          ListExpr prevPoint = nl->First( CycleNL );
          ListExpr flagedSeg, currPoint;
          CycleNL = nl->Rest( CycleNL );

          //Starting to compute a new cycle

          Points *cyclepoints= new Points( 8 ); // in memory

          Point *currvertex,p1,p2,firstP;

          //This function has the goal to store the half segments of
          //the cycle that is been treated. When the cycle's computation
          //is terminated the region rDir will be used to compute the
          //insideAbove
          //attribute of the half segments of this cycle.
          Region *rDir = new Region(32);
          rDir->StartBulkLoad();


          currvertex = (Point*) InPoint ( nl->TheEmptyList(),
              firstPoint, 0, errorInfo, correct ).addr;
          if (!correct) {
             // todo: delete temp objects
             return SetWord( Address(0) );
          }

          cyclepoints->StartBulkLoad();
          (*cyclepoints) += (*currvertex);
          p1 = *currvertex;
          firstP = p1;
          cyclepoints->EndBulkLoad();
         

          ///////////////////////////////////////////////////////////////
          ////////////////////////one cycle /////////////////////////////
          ////////////////////////////////////////////////////////////////
          delete currvertex;
          cycno++; 
          while ( !nl->IsEmpty( CycleNL) ){
//            cout<<"cycle "<<endl;
            currPoint = nl->First( CycleNL );
            CycleNL = nl->Rest( CycleNL );

            currvertex = (Point*) InPoint( nl->TheEmptyList(),
                  currPoint, 0, errorInfo, correct ).addr;


//            cout<<"curvertex "<<*currvertex<<endl;
            if (!correct) return SetWord( Address(0) );

            if (cyclepoints->Contains(*currvertex)){
              cerr<< __PRETTY_FUNCTION__ << ": The same vertex: "
                  <<(*currvertex)
                  <<" appears repeatedly within the current cycle!"<<endl;
              correct=false;
              return SetWord( Address(0) );
            }else{
              p2 = *currvertex;
              cyclepoints->StartBulkLoad();
              (*cyclepoints) += (*currvertex);
              cyclepoints->EndBulkLoad(true,false,false);
            }
            delete currvertex;

            flagedSeg = nl->TwoElemList
            (nl-> BoolAtom(true),
             nl->TwoElemList(prevPoint, currPoint));
            prevPoint=currPoint;
            edno++;
            //Create left dominating half segment
            HalfSegment * hs = (HalfSegment*)InHalfSegment
                      ( nl->TheEmptyList(), flagedSeg,
                       0, errorInfo, correct ).addr;
            if(!correct){
              if(hs){
                cerr << __PRETTY_FUNCTION__ << ": Creation of left dominating "
                     << "half segment (1) failed!" << endl;
                delete hs;
              }
              cr->DeleteIfAllowed();
              return SetWord( Address(0) );
            }
            hs->attr.faceno=fcno;
            hs->attr.cycleno=ccno;
            hs->attr.edgeno=edno;
            hs->attr.partnerno=partnerno;
            partnerno++;
            hs->attr.insideAbove = (hs->GetLeftPoint() == p1);
              //true (L-->R ),false (R--L)
            p1 = p2;

            if (( correct )&&( cr->InsertOk(*hs) )){
              (*cr) += (*hs);
//              cout<<"cr+1 "<<*hs<<endl;
              if( hs->IsLeftDomPoint() ){
                (*rDir) += (*hs);
//                cout<<"rDr+1 "<<*hs<<endl;
                hs->SetLeftDomPoint( false );
              }else{
                hs->SetLeftDomPoint( true );
//                cout<<"rDr+2 "<<*hs<<endl;
                (*rDir) += (*hs);
              }
              (*cr) += (*hs);
//              cout<<"cr+2 "<<*hs<<endl;
              delete hs;
            }
            else{
              cerr<< __PRETTY_FUNCTION__ << ": Problematic HalfSegment: "
                  << endl;
              if(correct)
                cerr << "\nhs = " << (*hs) << " cannot be inserted." << endl;
              else
                cerr << "\nInvalid half segment description." << endl;
              correct=false;
              return SetWord( Address(0) );
            }

          }
          delete cyclepoints;
          

          edno++;
          flagedSeg= nl->TwoElemList
            (nl-> BoolAtom(true),
             nl->TwoElemList(firstPoint, currPoint));
          HalfSegment * hs = (HalfSegment*)InHalfSegment
                  ( nl->TheEmptyList(), flagedSeg,
                    0, errorInfo, correct ).addr;
          if(!correct){
            if(hs){
                cerr << __PRETTY_FUNCTION__ << ": Creation of "
                     << "half segment (2) failed!" << endl;
                delete hs;
            }
            cr->DeleteIfAllowed();
            return SetWord( Address(0) );
          }
          hs->attr.faceno=fcno;
          hs->attr.cycleno=ccno;
          hs->attr.edgeno=edno;
          hs->attr.partnerno=partnerno;
          hs->attr.insideAbove = (hs->GetRightPoint() == firstP);
          //true (L-->R ),false (R--L),
          //the order of typing is last point than first point.
          partnerno++;

          //The last half segment of the region
          if (( correct )&&( cr->InsertOk(*hs) )){
            (*cr) += (*hs);
//             cout<<"cr+3 "<<*hs<<endl;
            if( hs->IsLeftDomPoint() ){
              (*rDir) += (*hs);
//              cout<<"rDr+3 "<<*hs<<endl;
              hs->SetLeftDomPoint( false );
            }else{
              hs->SetLeftDomPoint( true );
//              cout<<"rDr+4 "<<*hs<<endl;
              (*rDir) += (*hs);
            }
            (*cr) += (*hs);
//            cout<<"cr+4 "<<*hs<<endl;
            delete hs;
            rDir->EndBulkLoad(true, false, false, false);


            //To calculate the inside above attribute
            bool direction = rDir->GetCycleDirection();

            int h = cr->Size() - ( rDir->Size() * 2 );
            while ( h < cr->Size()){
              //after each left half segment of the region is its
              //correspondig right half segment
              HalfSegment hsIA;
              bool insideAbove;
              cr->Get(h,hsIA);

              if (direction == hsIA.attr.insideAbove)
                insideAbove = false;
              else
                insideAbove = true;
              if (!isCycle)
                insideAbove = !insideAbove;
              HalfSegment auxhsIA( hsIA );
              auxhsIA.attr.insideAbove = insideAbove;
              cr->UpdateAttr(h,auxhsIA.attr);
              //Get right half segment
              cr->Get(h+1,hsIA);
              auxhsIA = hsIA;
              auxhsIA.attr.insideAbove = insideAbove;
              cr->UpdateAttr(h+1,auxhsIA.attr);
              h+=2;
            }

            rDir->DeleteIfAllowed();
            isCycle = false;

          }
          else{
            correct=false;
            return SetWord( Address(0) );
          }
        }
      }


    cr->SetNoComponents( fcno+1 );
    cr->EndBulkLoad( true, true, true, false );
    //////////////////////////////////////////////////////////////////////////
//    cout<<"Area "<<cr->Area()<<endl; 
    correct = true;
    return SetWord( cr ); 
}

/*
add one 3D region to the object 

*/
void GRoom::Add(int id, float h,
                vector<HalfSegment>& hs_list)
{
//  cout<<"GRoom::Add()"<<endl; 

  int cur_points_size = seg_list.Size();
  
  FloorElem felem;
  felem.id = id;
//      cout<<"start_pos "<<start_pos<<endl; 
  felem.start_pos = cur_points_size;
  felem.num = hs_list.size();
  felem.h = h;

//  felem.Print(); 
  elem_list.Append(felem);
  for(unsigned int j = 0;j < hs_list.size();j++){
      seg_list.Append(hs_list[j]);
  }

}

/*
get one 3D region with the height value 

*/
void GRoom::Get(const int i, float& h, Region& r) const 
{
    if( 0 <= i && i < elem_list.Size()){
          /////////colllect outer cycles and holes for the region  //////////
          vector<FloorElem> felem_list;
          for(int j = 0;j < elem_list.Size();j++){
            FloorElem elem;
            elem_list.Get(j, elem);
            if(elem.id == i){
              felem_list.push_back(elem);
              h = elem.h; 
            }  
          }
          ///////////////////////////////////////////////////////////////////
          r.StartBulkLoad();
//          cout<<"elem size "<<felem_list.size()<<endl; 
          for(unsigned int i = 0;i < felem_list.size();i++){
            FloorElem felem = felem_list[i];
            int start_pos = felem.start_pos;
            int end_pos = felem.start_pos + felem.num; 
//            cout<<"start_pos "<<start_pos<<" end_pos "<<end_pos<<endl; 
            while(start_pos < end_pos){
              HalfSegment hs;
              seg_list.Get(start_pos, hs);
              r += hs;
              start_pos++; 
            }
          }
          r.SetNoComponents(1);
          r.EndBulkLoad( true, true, true, false );
//          cout<<r<<endl; 
    }else{
      cout<<"not valid index in Get()"<<endl;
      assert(false);
    }
}

/*
translate the 2D region of the groom 

*/
void GRoom::Translate(const Coord& x, const Coord& y, GRoom& result)
{
  result = *this; 
  for(int i = 0;i < result.SegSize();i++){
    HalfSegment hs;
    result.GetSeg(i, hs);
    hs.Translate(x, y);
    result.PutSeg(i, hs);
  }
}

/*get the 2D area covered by the region*/

void GRoom::GetRegion(Region& r)
{
  for( int i = 0; i < Size(); i++ ){
    Region temp_reg(0);
    float h;
    Get( i, h, temp_reg);
    Region* res = new Region(0);
    r.Union(temp_reg, *res);
    r = *res;
    delete res;
  }
}

void GRoom::Print()
{
   for( int i = 0; i < Size(); i++ ){
    Region r(0);
    float h;
    Get( i, h, r);
    cout<<"elem "<<i<<" height "<<h<<endl; 
    cout<<r<<endl; 
  }

}

/*
return the lowest height of a groom 

*/
float GRoom::GetLowHeight()
{
  float h = numeric_limits<float>::max(); 
  for( int i = 0; i < elem_list.Size(); i++ ){
    FloorElem felem;
    elem_list.Get(i, felem);
    if(felem.h < h)
      h = felem.h; 
  }
  return h; 
}


/*
Groom Property function 

*/
ListExpr GRoomProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("groom"),
           nl->StringAtom("(<floor3d>*) ((f1 floor3d)(f2 floor3d))"),
           nl->StringAtom("((f1)(f2))"))));
}

/*
output the list expression for Staircase3D 

*/
ListExpr OutGRoom( ListExpr typeInfo, Word value )
{
//  cout<<"OutGRoom()"<<endl; 

  GRoom* gr = (GRoom*)(value.addr);
  if(!gr->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  if( gr->IsEmpty() ){
    return nl->TheEmptyList();
  }
//  cout<<"stair size "<<stair->Size()<<endl; 

  bool first = true;
  ListExpr result = nl->TheEmptyList();
  ListExpr last = result; 
  ListExpr sb_list; 
  for( int i = 0; i < gr->Size(); i++ ){
    Region r(0);
    float h;
    gr->Get( i, h, r);

    sb_list = nl->TwoElemList(nl->RealAtom(h),
                  OutRegion(nl->TheEmptyList(), SetWord(&r)));

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
Word InGRoom( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if(nl->IsEqual(instance,"undef")) {
      correct = false;
      return SetWord(Address(0));
  }

  GRoom* gr = new GRoom(0);
//  cout<<"length "<<nl->ListLength(instance)<<endl;
  ListExpr floor_nl = instance;
  int count = 0; 
  while(!nl->IsEmpty(floor_nl)){
    ListExpr first = nl->First(floor_nl);
    floor_nl = nl->Rest(floor_nl);
    
    ListExpr height_nl = nl->First(first);
    
    if(!(nl->IsAtom(height_nl) && nl->AtomType(height_nl) == RealType)){
      cout<<"real value expected for height"<<endl; 
      correct = false;
      return SetWord(Address(0));
    }
    float height = nl->RealValue(height_nl);
//    cout<<"height "<<height<<endl; 
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////read the region ///////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    int cycleno = 0;
    Region* reg = (Region*)MyInRegion(typeInfo, nl->Second(first),
               errorPos, errorInfo, correct, cycleno).addr; 

    ////////////////////////////////////////////////////////////////
//    cout<<"Area "<<reg->Area()<<endl; 
//    cout<<"point size "<<point_list.size()
//        <<"index size "<<index_list.size()<<endl; 

//    for(unsigned int i = 0;i < point_list.size();i++)
//      cout<<point_list[i];

//    for(unsigned int i = 0;i < index_list.size();i++)
//      cout<<"index "<<index_list[i]<<" num "<<num_list[i]<<endl; 
    ///////////////////////////////////////////////////////////////
    if(reg->NoComponents() != 1 || cycleno == 0){//only one face is allowed 
      cout<<"invalid region input "<<endl; 
      correct = false;
      return SetWord(Address(0));
    }
    for(int i = 0;i < cycleno;i++){
        vector<HalfSegment> hs_list; 
        for(int j = 0;j < reg->Size();j++){
          HalfSegment hs;
          reg->Get(j, hs);
          if(hs.attr.cycleno == i){
              hs_list.push_back(hs);
          }
        }
//        cout<<i <<" seg list size "<<hs_list.size()<<endl; 
        if(i == 0)
          gr->Add(count, height, hs_list);
        else
          gr->Add(count, height, hs_list);

    }
    
    count++;
  }
  ////////////////very important ////////////////////////////////
  correct = true; 
  ////////////////////////////////////////////////////////
  return SetWord(gr);

}


/*
Open a GRoom object 

*/
bool OpenGRoom(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenGRoom()"<<endl; 

  GRoom* gr = (GRoom*)Attribute::Open(valueRecord, offset, typeInfo);
  value = SetWord(gr);
//  cout<<gr->ElemSize()<<" "<<gr->SegSize()<<endl; 
  return true; 
}

/*
Save a GRoom object 

*/
bool SaveGRoom(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveGRoom"<<endl; 
  GRoom* gr = (GRoom*)value.addr; 
  Attribute::Save(valueRecord, offset, typeInfo, gr);
//  cout<<gr->ElemSize()<<" "<<gr->SegSize()<<endl; 
  return true; 
}


/*
create function for GRoom 

*/

Word CreateGRoom(const ListExpr typeInfo)
{
//  cout<<"CreateGRoom()"<<endl;
  return SetWord (new GRoom(0));
}

/*
Delete function for GRoom 

*/

void DeleteGRoom(const ListExpr typeInfo, Word& w)
{
//  cout<<"DeleteGRoom()"<<endl;
  GRoom* gr = (GRoom*)w.addr;
  delete gr;
   w.addr = NULL;
}


void CloseGRoom( const ListExpr typeInfo, Word& w )
{
//  cout<<"CloseGRoom"<<endl; 
  ((GRoom*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word CloneGRoom( const ListExpr typeInfo, const Word& w )
{
//  cout<<"CloneGRoom"<<endl; 
  return SetWord( new GRoom( *((GRoom *)w.addr) ) );
}

void* CastGRoomD(void* addr)
{
//  cout<<"CastGRoomD"<<endl; 
  return (new (addr) GRoom());
}

int SizeOfGRoom()
{
//  cout<<"SizeOfGroom"<<endl; 
  return sizeof(GRoom);
}

bool CheckGRoom( ListExpr type, ListExpr& errorInfo )
{
//  cout<<"CheckGroom"<<endl; 
  return (nl->IsEqual( type, "groom" ));
}


//////////////////////////////////////////////////////////////////////////////
/////////////////////////////Indoor Navigation///////////////////////////////
////////////////////////////////////////////////////////////////////////////
string IndoorNav::Indoor_GRoom_Door = 
"(rel(tuple((oid int)(Name string)(Type string)(Room groom)(Door line))))";

/*
create a 3d line for the door 

*/
void IndoorNav::CreateDoor3D()
{
//  cout<<"CreateDoor3D()"<<endl; 

  vector<float> floor_height; 
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* indoor_tuple = rel1->GetTuple(i, false);
    string s = ((CcString*)indoor_tuple->GetAttribute(I_Type))->GetValue();
    if(GetRoomEnum(s) == EL){
      GRoom* groom = (GRoom*)indoor_tuple->GetAttribute(I_Room);
      float h = groom->GetLowHeight();
      if(floor_height.size() == 0)
        floor_height.push_back(h);
      else{
        unsigned int j = 0;
        for(;j < floor_height.size();j++){
          if(AlmostEqual(floor_height[j], h))break;
        }
        if(j == floor_height.size())
          floor_height.push_back(h);
      }
    }
    indoor_tuple->DeleteIfAllowed(); 
  }
  sort(floor_height.begin(), floor_height.end());
  
//  cout<<"rel1 no "<<rel1->GetNoTuples()<<endl; 
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* indoor_tuple = rel1->GetTuple(i, false);
    GRoom* groom = (GRoom*)indoor_tuple->GetAttribute(I_Room); 
    Line* l = (Line*)indoor_tuple->GetAttribute(I_Door); 
    int oid = ((CcInt*)indoor_tuple->GetAttribute(I_OID))->GetIntval();

    string s = ((CcString*)indoor_tuple->GetAttribute(I_Type))->GetValue();
    
//    groom->Print();
//    cout<<*l<<endl;
//    cout<<"type "<<s<<endl; 
    float h = groom->GetLowHeight();
    CreateLine3D(oid, l, h);

    /////////////////3D box x,y,z ////////////////////////
    if(GetRoomEnum(s) == ST || GetRoomEnum(s) == EL){//Staircase, Elevator
//      cout<<"one more box needed"<<endl;
      h = NextFloorHeight(h, floor_height);  
      if(h > 0.0){
          CreateLine3D(oid, l, h);
      }
    }
    indoor_tuple->DeleteIfAllowed();
  }
}

/*
create a 3d line 

*/
void IndoorNav::CreateLine3D(int oid, Line* l, float h)
{
//  cout<<"oid "<<oid<<" line "<<*l<<" h "<<h<<endl; 
  for(int i = 0;i < l->Size();i++){
      HalfSegment hs;
      l->Get(i, hs);
      if(hs.IsLeftDomPoint() == false) continue; 
      Point lp = hs.GetLeftPoint();
      Point rp = hs.GetRightPoint();
      Line3D* l3d = new Line3D(0);
      l3d->StartBulkLoad();
      Point3D q1(true, lp.GetX(), lp.GetY(), h);
      *l3d += q1;
      Point3D q2(true, rp.GetX(), rp.GetY(), h);
      *l3d += q2;
      l3d->EndBulkLoad();

      groom_oid_list.push_back(oid);
      path_list.push_back(*l3d); 
      delete l3d; 

  }

}


/*
create a 3D box for each door. return (oid, tid, box3d)  
for staircase and elevator. In the original data, it only stores the door for
  each floor, but in fact the staircase and elevator builds the connection
  between two floors. so, we create one more door for staircase and elevator 
  
  If a groom has several 3D regions, the door normally is located at the lowest
  level 

*/
void IndoorNav::CreateDoorBox()
{
 /////////////// collect the height value for each floor///////////////////
  //////////////// for staircase and elevator //////////////////////////////
  vector<float> floor_height; 
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* indoor_tuple = rel1->GetTuple(i, false);
    string s = ((CcString*)indoor_tuple->GetAttribute(I_Type))->GetValue();
    if(GetRoomEnum(s) == EL){
      GRoom* groom = (GRoom*)indoor_tuple->GetAttribute(I_Room);
      float h = groom->GetLowHeight();
      if(floor_height.size() == 0)
        floor_height.push_back(h);
      else{
        unsigned int j = 0;
        for(;j < floor_height.size();j++){
          if(AlmostEqual(floor_height[j], h))break;
        }
        if(j == floor_height.size())
          floor_height.push_back(h);
      }
    }
    indoor_tuple->DeleteIfAllowed(); 
  }
  sort(floor_height.begin(), floor_height.end()); 
  
//  for(unsigned int i = 0;i < floor_height.size();i++)
//    cout<<"floor "<<i<<" height "<<floor_height[i]<<endl; 


//  cout<<"rel1 no "<<rel1->GetNoTuples()<<endl; 
  
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* indoor_tuple = rel1->GetTuple(i, false);
    GRoom* groom = (GRoom*)indoor_tuple->GetAttribute(I_Room); 
    Line* l = (Line*)indoor_tuple->GetAttribute(I_Door); 
    int oid = ((CcInt*)indoor_tuple->GetAttribute(I_OID))->GetIntval();
    int tid = indoor_tuple->GetTupleId();
    string s = ((CcString*)indoor_tuple->GetAttribute(I_Type))->GetValue();
//    groom->Print();
//    cout<<*l<<endl;
//    cout<<"type "<<s<<endl; 
    float h = groom->GetLowHeight();
    CreateBox3D(oid, tid, l, h);

    /////////////////3D box x,y,z ////////////////////////

    if(GetRoomEnum(s) == ST || GetRoomEnum(s) == EL){//Staircase, Elevator
//      cout<<"one more box needed"<<endl;
      h = NextFloorHeight(h, floor_height);  
      if(h > 0.0){
          CreateBox3D(oid, tid, l, h);
      }
    }

    indoor_tuple->DeleteIfAllowed();
  }
}

/*
get the floor height in the next higher level 

*/
float IndoorNav::NextFloorHeight(float h, vector<float>& floor_height)
{
//  cout<<"NextFloorHeight()"<<endl; 
  for(unsigned int i = 0;i < floor_height.size();i++){
    if(AlmostEqual(h, floor_height[i])){
        if(i != floor_height.size() - 1)
          return floor_height[ i + 1];
        else
          return -1.0; 
    }
  }
  assert(false); 
}

void IndoorNav::CreateBox3D(int oid, int tid, Line* l, float h)
{
  const double delta_h = 0.01; 
//  cout<<"oid "<<oid<<" tid "<<tid<<" height "<<h<<endl; 
//  cout<<"line "<<*l<<endl; 
  for(int i = 0;i < l->Size();i++){
      HalfSegment hs;
      l->Get(i, hs);
      if(hs.IsLeftDomPoint() == false) continue; 
      double min[3];
      double max[3];
      Point lp = hs.GetLeftPoint();
      Point rp = hs.GetRightPoint();
      min[0] = MIN(lp.GetX(), rp.GetX());
      min[1] = MIN(lp.GetY(), rp.GetY());
      min[2] = h;
      
      max[0] = MAX(lp.GetX(), rp.GetX());
      max[1] = MAX(lp.GetY(), rp.GetY());
      max[2] = h + delta_h;
      Rectangle<3> bbox3d(true, min, max);
      oid_list.push_back(oid);
      tid_list.push_back(tid);
      box_list.push_back(bbox3d);
//      cout<<"i "<<i<<bbox3d<<endl; 
  }

}

/*
create a relation storing the doors of a building 

*/
void IndoorNav::CreateDoor(R_Tree<3, TupleId>* rtree, 
                           int attr1, int attr2, int attr3)
{
//  cout<<"CreateDoor()"<<endl;
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<" attr3 "<<attr3<<endl; 
  SmiRecordId adr = rtree->RootRecordId();
  vector<bool> visit_flag; 
  for(int i = 1;i <= rel2->GetNoTuples();i++){
      visit_flag.push_back(true);
  }


  for(int i = 1;i <= rel2->GetNoTuples();i++){
    if(visit_flag[i - 1] == false) continue; 
    
    Tuple* door_tuple = rel2->GetTuple(i, false);
    int oid = ((CcInt*)door_tuple->GetAttribute(attr1))->GetIntval();
    int tid = ((CcInt*)door_tuple->GetAttribute(attr2))->GetIntval();
    Rectangle<3>* bbox3d = (Rectangle<3>*)door_tuple->GetAttribute(attr3);
    
    unsigned int id = door_tuple->GetTupleId();

/*    if(oid != 81){ 
      door_tuple->DeleteIfAllowed();
      continue; 
    }*/

    vector<TupleId> id_list; 
//    cout<<"oid "<<oid<<" tid "<<tid<<endl; 
//    cout<<"bbox3d -- line "<<*bbox3d<<endl; 
    DFTraverse(rtree, adr, id, bbox3d, attr1, attr2, attr3, id_list);

/*    if(oid != 384){
      door_tuple->DeleteIfAllowed();
      continue; 
    }*/

//    cout<<"neighbor size "<<id_list.size()<<endl; 

/*    for(unsigned int j = 0;j < id_list.size();j++)
      cout<<"neighbor tuple id "<<id_list[j]<<endl; 
    cout<<endl; */

    //for staircase and elevator, size = 2
    //the floor at i and i+1 are both represented 
    // the size can be 0 at some special places after translating 
    assert(id_list.size() <= 2);
    ///////////// create the result  ///////////////////////////////
    ////////// use tid to get the tuple 
    ///////////use the tuple to get oid and line 
//    if(id_list.size() == 1)
    CreateResDoor(id, oid, tid, id_list, attr1, attr2, attr3, visit_flag);

    /////////////////////////////////////////////////////////////////
    door_tuple->DeleteIfAllowed();
  }

}

/*
create the result for door relation: door3d line groomid1 groomid2 
for data type line, region, mbool, the function operator=() is not 
  correctly impelmented. it should copy the value from the dbarray instead of
  calling copyfrom. 
  
  It is no problem in memory, but has problems in disk for storage  

*/
void IndoorNav::CreateResDoor(int id, int oid, int tid, vector<TupleId> id_list,
                     int attr1, int attr2, int attr3, vector<bool>& visit_flag)
{
   
  for(unsigned int i = 0;i < id_list.size();i++){

    ///////////////   first door in one groom   ////////////////////
    Tuple* indoor_tuple1 = rel1->GetTuple(tid, false);
    int oid1 = ((CcInt*)indoor_tuple1->GetAttribute(I_OID))->GetIntval();
    assert(oid1 == oid);
    string type1 = ((CcString*)indoor_tuple1->GetAttribute(I_Type))->GetValue();
    GRoom* groom1 = (GRoom*)indoor_tuple1->GetAttribute(I_Room);
    Tuple* box_tuple1 = rel2->GetTuple(id, false);
    Rectangle<3>* bbox3d_1 = (Rectangle<3>*)box_tuple1->GetAttribute(attr3);
//    cout<<"bbox3d 1 "<<*bbox3d_1<<endl; 
    Rectangle<2> groom_box1 = groom1->BoundingBox();

   ///////////////   second door in one groom   ////////////////////
    Tuple* box_tuple2 = rel2->GetTuple(id_list[i], false);
    int oid2 = ((CcInt*)box_tuple2->GetAttribute(attr1))->GetIntval();
    int groom_tid = ((CcInt*)box_tuple2->GetAttribute(attr2))->GetIntval();
/*    if(oid2 != 385){
        indoor_tuple1->DeleteIfAllowed();
        box_tuple1->DeleteIfAllowed();
        box_tuple2->DeleteIfAllowed();
        continue; 
    }*/
    
    
    assert(1 <= groom_tid && groom_tid <= rel1->GetNoTuples());
    
    Tuple* indoor_tuple2 = rel1->GetTuple(groom_tid, false);

    string type2 = ((CcString*)indoor_tuple2->GetAttribute(I_Type))->GetValue();
    GRoom* groom2 = (GRoom*)indoor_tuple2->GetAttribute(I_Room);
    
    Rectangle<3>* bbox3d_2 = (Rectangle<3>*)box_tuple2->GetAttribute(attr3);
    Rectangle<2> groom_box2 = groom2->BoundingBox();
//    cout<<"bbox3d 2 "<<*bbox3d_2<<endl; 
    ///////////////////////////////////////////////////////////////////
    ///////////////create moving bool/////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    Instant start_time(instanttype);
    start_time.ReadFrom("2010-12-5-0:0:0");
    Instant end_time(instanttype);
    end_time.ReadFrom("2010-12-6-0:0:0");

    Instant begin_time(instanttype);
    begin_time.ReadFrom("begin of time");
    Instant finish_time(instanttype);
    finish_time.ReadFrom("end of time");
    
    MBool* mb1 = new MBool(0);
    MBool* mb2 = new MBool(0);
    mb1->StartBulkLoad();
    mb2->StartBulkLoad();
    UBool ub1;
    ub1.timeInterval.start = begin_time;
    ub1.timeInterval.end = start_time;

    ub1.timeInterval.lc = true;
    ub1.timeInterval.rc = false;
    ub1.constValue.Set(true, true);
    ub1.SetDefined(true);


    UBool ub2;
    ub2.timeInterval.start = start_time;
    ub2.timeInterval.end = end_time;

    ub2.timeInterval.lc = true;
    ub2.timeInterval.rc = false;
    ub2.constValue.Set(true, true);
    ub2.SetDefined(true);

    UBool ub3;
    ub3.timeInterval.start = end_time;
    ub3.timeInterval.end = finish_time;

    ub3.timeInterval.lc = true;
    ub3.timeInterval.rc = false;
    ub3.constValue.Set(true, true);
    ub3.SetDefined(true);

//    ub1.Print(cout);
//    ub2.Print(cout);
//    ub3.Print(cout);
    
    mb1->Add(ub1);
    mb1->Add(ub2);
    mb1->Add(ub3);
    mb1->EndBulkLoad();
    mb2->Add(ub1);
    mb2->Add(ub2);
    mb2->Add(ub3);
    mb2->EndBulkLoad();
//    cout<<*mb<<endl; 
    bool lift_door = false;
    if(GetRoomEnum(type1) == EL || GetRoomEnum(type2) == EL)
      lift_door = true; 
    //////////////////////////////////////////////////////////////////
    ///////////////// the position of the door in space /////////////
    //////////////////////////////////////////////////////////////////

//    cout<<"line1 "<<*l1<<" line2 "<<*l2<<endl; 
    ////////////////////////////////////////////////////////////////
    /////////////////////create line //////////////////////////////////
    ////////////////////////////////////////////////////////////////////
    
    Line* l1 = new Line(0);
    Line* l2 = new Line(0);
    Line* l3 = new Line(0);
    GRoomDoorLine(bbox3d_1, bbox3d_2, l1, l2, l3, &groom_box1, &groom_box2);

    ///////////////////// create the door //////////////////////////////
    Door3D* door_obj1 = new Door3D(oid1, oid2, *l1, *l2, *mb1, lift_door);
    //////////////////////  result   /////////////////////////////// 
    door_list.push_back(*door_obj1);
    line_list.push_back(*l3);
    groom_id_list1.push_back(oid1);
    groom_id_list2.push_back(oid2);
    door_heights.push_back(bbox3d_1->MinD(2));

    if(GetRoomEnum(type1) == ST || GetRoomEnum(type2) == ST)
      door_types.push_back(1);
    else if(GetRoomEnum(type1) == EL || GetRoomEnum(type2) == EL)
      door_types.push_back(2);
    else
      door_types.push_back(0);
    
    delete l1;
    delete l2;
    delete l3; 
    delete door_obj1; 
    //////////////////////////////////////////////////////////////////////
    
    Line* l4 = new Line(0);
    Line* l5 = new Line(0);
    Line* l6 = new Line(0);
    GRoomDoorLine(bbox3d_1, bbox3d_2, l4, l5, l6, &groom_box1, &groom_box2);

    ///////////////////// create the door //////////////////////////////
    Door3D* door_obj2 = new Door3D(oid2, oid1, *l4, *l5, *mb2, lift_door);
    
    door_list.push_back(*door_obj2);
    line_list.push_back(*l6);
    groom_id_list1.push_back(oid2);
    groom_id_list2.push_back(oid1);
    door_heights.push_back(bbox3d_1->MinD(2));

    if(GetRoomEnum(type1) == ST || GetRoomEnum(type2) == ST)
      door_types.push_back(1);
    else if(GetRoomEnum(type1) == EL || GetRoomEnum(type2) == EL)
      door_types.push_back(2);
    else
      door_types.push_back(0);

    
    ////////////////////////////////////////////////////////////////////
    delete l4;
    delete l5;
    delete l6; 
    delete door_obj2; 
    
    delete mb1; 
    delete mb2; 

    /////////////////////////////////////////////////////////////
    box_tuple1->DeleteIfAllowed();
    box_tuple2->DeleteIfAllowed();
    indoor_tuple2->DeleteIfAllowed();
    indoor_tuple1->DeleteIfAllowed(); 
    
    visit_flag[id_list[i] - 1] = false; 
  }

}


/*
depth-first traverse the Rtree to find the element (line3d) intersects the 
bbox3d

*/
void IndoorNav::DFTraverse(R_Tree<3,TupleId>* rtree, SmiRecordId adr, 
                           unsigned int id,
                           Rectangle<3>* bbox3d, 
                           int attr1, int attr2, int attr3,
                           vector<TupleId>& id_list)
{
  R_TreeNode<3,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<3,TupleId> e =
                 (R_TreeLeafEntry<3,TupleId>&)(*node)[j];
              Tuple* box_tuple = rel2->GetTuple(e.info,false);
              Rectangle<3>* b =
                     (Rectangle<3>*)box_tuple->GetAttribute(attr3);
              if(b->Intersects(*bbox3d) && id != e.info){
                    Tuple* box_tuple = rel2->GetTuple(e.info, false); 
                    Rectangle<3>* bbox_3d = 
                      (Rectangle<3>*)box_tuple->GetAttribute(attr3);

                    if(BBox3DEqual(bbox3d, bbox_3d))
                        id_list.push_back(e.info);
                    box_tuple->DeleteIfAllowed(); 
              }
              box_tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<3> e =
                (R_TreeInternalEntry<3>&)(*node)[j];
            if(bbox3d->Intersects(e.box)){
              DFTraverse(rtree, e.pointer, id, bbox3d, 
                         attr1, attr2, attr3, id_list);
            }
      }
  }
  delete node;

}

/*
Compare two 3D box 

*/
bool IndoorNav::BBox3DEqual(Rectangle<3>* bbox3d, Rectangle<3>* bbox_3d)
{
  for(unsigned int i = 0;i < 3;i ++){
    if(!AlmostEqual(bbox3d->MinD(i), bbox_3d->MinD(i)))return false; 
    if(!AlmostEqual(bbox3d->MaxD(i), bbox_3d->MaxD(i)))return false; 
  }
  return true; 
}

/*
create a line denoting the positio of a door. we record the relative position
according to the groom 

*/

void IndoorNav::GRoomDoorLine(Rectangle<3>* bbox3d_1, Rectangle<3>* bbox3d_2, 
                     Line* l1, Line* l2, Line* l3, 
                     const Rectangle<2>* groom_box1,
                     const Rectangle<2>* groom_box2)
{
//  cout<<"door box1 "<<*bbox3d_1<<endl; 
//  cout<<"GRoom box1 "<<*groom_box1<<endl; 
//  cout<<"door box2 "<<*bbox3d_2<<endl; 
//  cout<<"GRoom box2 "<<*groom_box2<<endl; 

  double min_x1 = bbox3d_1->MinD(0) - groom_box1->MinD(0);
  double min_y1 = bbox3d_1->MinD(1) - groom_box1->MinD(1);
  double max_x1 = bbox3d_1->MaxD(0) - groom_box1->MinD(0);
  double max_y1 = bbox3d_1->MaxD(1) - groom_box1->MinD(1);
  
  
  double min_x2 = bbox3d_2->MinD(0) - groom_box2->MinD(0);
  double min_y2 = bbox3d_2->MinD(1) - groom_box2->MinD(1);
  double max_x2 = bbox3d_2->MaxD(0) - groom_box2->MinD(0);
  double max_y2 = bbox3d_2->MaxD(1) - groom_box2->MinD(1);

//  cout<<" x1 "<<min_x1<<" y1 "<<min_y1<<" x2 "<<max_x1<<" y2 "<<max_y1<<endl;
//  cout<<" x1 "<<min_x2<<" y1 "<<min_y2<<" x2 "<<max_x2<<" y2 "<<max_y2<<endl;

  l1->StartBulkLoad();
  Point lp1(true, min_x1, min_y1);
  Point rp1(true, max_x1, max_y1);
  HalfSegment hs1(true, lp1, rp1);
  int edgeno1 = 0; 
  hs1.attr.edgeno = edgeno1++;
  *l1 += hs1;
  hs1.SetLeftDomPoint(!hs1.IsLeftDomPoint());
  *l1 += hs1; 
  l1->EndBulkLoad();
  
  
  l2->StartBulkLoad();
  Point lp2(true, min_x2, min_y2);
  Point rp2(true, max_x2, max_y2);
  HalfSegment hs2(true, lp2, rp2);
  int edgeno2 = 0; 
  hs2.attr.edgeno = edgeno2++;
  *l2 += hs2;
  hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
  *l2 += hs2; 
  l2->EndBulkLoad();
  
  
  l3->StartBulkLoad();
  Point lp3(true, bbox3d_1->MinD(0), bbox3d_1->MinD(1));
  Point rp3(true, bbox3d_1->MaxD(0), bbox3d_1->MaxD(1));
  HalfSegment hs3(true, lp3, rp3);
  int edgeno3 = 0; 
  hs3.attr.edgeno = edgeno3++;
  *l3 += hs3;
  hs3.SetLeftDomPoint(!hs3.IsLeftDomPoint());
  *l3 += hs3; 
  l3->EndBulkLoad();
}

/*
create the edges connecting two doors inside one room 

*/

void IndoorNav::CreateAdjDoor(BTree* btree,int attr1, int attr2, 
                              int attr3, int attr4)
{
//  cout<<"CreateAdjDoor() "<<endl;
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<" attr3 "<<attr3
//      <<" attr4 "<<attr4<<endl; 

  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* groom_tuple = rel1->GetTuple(i, false);
    int groom_oid = ((CcInt*)groom_tuple->GetAttribute(I_OID))->GetIntval();
    //////////////////////////////////////////////////////////////////////////
    /////////////find all doors belong to this groom/////////////////////////
    /////////////////////////////////////////////////////////////////////////
    
    CcInt* search_id = new CcInt(true, groom_oid);
    BTreeIterator* btree_iter = btree->ExactMatch(search_id);
    vector<int> tid_list; 
    while(btree_iter->Next()){
        Tuple* tuple = rel2->GetTuple(btree_iter->GetId(), false);
        tid_list.push_back(tuple->GetTupleId());
        tuple->DeleteIfAllowed();
    }
    delete btree_iter;
    delete search_id;

/*    cout<<"groom_oid "<<groom_oid<<" door num "<<tid_list.size()<<endl; 
    for(unsigned int j = 0;j < tid_list.size();j++)
      cout<<"door tid "<<tid_list[j] <<endl;
    cout<<endl; */
    ///////////////////////////////////////////////////////////////////////
    /////////////compute shortest path between each pair of doors/////////
    /////////////the doors should not be equal to each other////////////
    ////////////////////////////////////////////////////////////////////
    if(tid_list.size() > 1){
      GRoom* groom = (GRoom*)groom_tuple->GetAttribute(I_Room);
      string groom_type = 
          ((CcString*)groom_tuple->GetAttribute(I_Type))->GetValue();
      ///OR, CO, 
      /////////////////////////////////////////////////////////
      ////// the path inside a elevator: EL //////////////////////
      ///////////////////////////////////////////////////////////
      if(GetRoomEnum(groom_type) == EL){
//        BuildPathEL(groom_oid, groom, tid_list, attr1, attr2, attr3, attr4);
      }
      /////////////////////////////////////////////////////////
      ////// the path inside a elevator: ST //////////////////////
      ///////////////////////////////////////////////////////////
      if(GetRoomEnum(groom_type) == ST){
//        BuildPathST(groom_oid, groom, tid_list, attr1, attr2, attr3, attr4);
      }
      /////////////////////////////////////////////////////////////
      ////// the path inside an office room or corridor: OR or CO///
      //////////////////////////////////////////////////////////////
      if(GetRoomEnum(groom_type) == OR || GetRoomEnum(groom_type) == CO){
        BuildPathORAndCO(groom_oid, groom, 
                         tid_list, attr1, attr2, attr3, attr4);
      }
    }
    groom_tuple->DeleteIfAllowed();
  }
}
/////////////////////////////////////////////////////////////////////////////
//////////// compute the path for each pair of doors inside one room  //////
//////////// The rooms needs to be considered: OR, CO, ST, EL  /////////////
////////////////  BR (bath room) is excluded //////////////////////////////

/*
the path for an elevator, a straight line connecting two floors  

*/
void IndoorNav::BuildPathEL(int groom_oid, GRoom* groom, vector<int> tid_list, 
                            int attr1, int attr2, 
                            int attr3, int attr4)
{

  cout<<"BuildPathEL() tid_lise size "<<tid_list.size()<<endl; 
//  cout<<"low height "<<groom->GetLowHeight()<<endl; 
  ////////////////////use tid id to collect the door tuple/////////////
  assert(tid_list.size() == 2);
  
  Tuple* door_tuple1 = rel2->GetTuple(tid_list[0], false);
  Tuple* door_tuple2 = rel2->GetTuple(tid_list[1], false);

  Line* l = (Line*)door_tuple1->GetAttribute(attr2);
  assert(l->Size() == 2);
  HalfSegment hs;
  l->Get(0, hs);
//  cout<<"hs "<<hs<<endl; 
  float h1 = ((CcReal*)door_tuple1->GetAttribute(attr4))->GetRealval();
  float h2 = ((CcReal*)door_tuple2->GetAttribute(attr4))->GetRealval();
//  cout<<"h1 "<<h1<<" h2 "<<h2<<endl; 
  double x = (hs.GetLeftPoint().GetX() + hs.GetRightPoint().GetX())/2;
  double y = (hs.GetLeftPoint().GetY() + hs.GetRightPoint().GetY())/2;
  Point3D p1(true, x, y, h1);
  Point3D p2(true, x, y, h2);
  
  door_tuple1->DeleteIfAllowed();
  door_tuple2->DeleteIfAllowed(); 
  
  Line3D* l3d = new Line3D(0);
  l3d->StartBulkLoad();
  *l3d += p1;
  *l3d += p2; 
  l3d->EndBulkLoad();
//  l3d->Print();
  //////////////////////  result   ////////////////////////////////////////
  groom_oid_list.push_back(groom_oid);
  door_tid_list1.push_back(tid_list[0]);
  door_tid_list2.push_back(tid_list[1]);
  path_list.push_back(*l3d);
  
  groom_oid_list.push_back(groom_oid);
  door_tid_list1.push_back(tid_list[1]);
  door_tid_list2.push_back(tid_list[0]);
  path_list.push_back(*l3d);

  /////////////////////////////////////////////////////////////////////////
  delete l3d; 
}


/*
the path for the staircase

*/
void IndoorNav::BuildPathST(int groom_oid, GRoom* groom, vector<int> tid_list, 
                            int attr1, int attr2, 
                            int attr3, int attr4)
{
  const double dist_delta = 0.001; 
//  cout<<"BuildPathST()"<<endl; 
//  cout<<"groom_oid "<<groom_oid<<" door num "<<tid_list.size()<<endl; 

  //////////////construct a standard path for the staircase///////////////////
  vector<MySegHeight> middle_path; 
  ConstructMiddlePath(groom, middle_path); 

//  cout<<"low height "<<groom->GetLowHeight()<<endl; 
  ////////////////////use tid id to collect the door tuple/////////////
  for(unsigned int i = 0;i < tid_list.size();i++){
//      cout<<"door tid "<<tid_list[i]<<endl; 
      Tuple* door_tuple1 = rel2->GetTuple(tid_list[i], false);
      float h1 = ((CcReal*)door_tuple1->GetAttribute(attr4))->GetRealval();
      Line* l1 = (Line*)door_tuple1->GetAttribute(attr2);

      for(unsigned int j = 0;j < tid_list.size();j++){
        if(tid_list[j] == tid_list[i]) continue; 
        Tuple* door_tuple2 = rel2->GetTuple(tid_list[j], false);
        float h2 = ((CcReal*)door_tuple2->GetAttribute(attr4))->GetRealval();
        Line* l2 = (Line*)door_tuple2->GetAttribute(attr2);
        if(fabs(h1-h2) < dist_delta){ // on the same floor 
            ST_ConnectOneFloor(groom_oid, groom, l1, l2,
                               tid_list[i], tid_list[j], h1);

        }else{ //not on the same floor 

         ST_ConnectFloors(groom_oid, groom, l1, l2,
                               tid_list[i], tid_list[j], h1, h2, middle_path);
        }
        door_tuple2->DeleteIfAllowed(); 
      }

    door_tuple1->DeleteIfAllowed();
  }

}

/*
build the connection for two doors of a staircase at the same level 

*/
void IndoorNav::ST_ConnectOneFloor(int groom_oid, GRoom* groom, Line* l1, 
                                   Line* l2, int tid1, int tid2, float h)
{
//  cout<<" ST_ConnectOneFloor "
//      <<"groom oid "<<groom_oid<<" id1 "<<tid1<<" id2 "<<tid2<<endl; 

  const double dist_delta = 0.001; 
  HalfSegment hs1;
  assert(l1->Size() == 2);
  l1->Get(0, hs1); 
  
  HalfSegment hs2; 
  assert(l2->Size() == 2);
  l2->Get(0, hs2); 
  
  double x1 = (hs1.GetLeftPoint().GetX() + hs1.GetRightPoint().GetX())/2;
  double y1 = (hs1.GetLeftPoint().GetY() + hs1.GetRightPoint().GetY())/2; 
  Point p1(true, x1, y1);

  double x2 = (hs2.GetLeftPoint().GetX() + hs2.GetRightPoint().GetX())/2;
  double y2 = (hs2.GetLeftPoint().GetY() + hs2.GetRightPoint().GetY())/2; 
  Point p2(true, x2, y2);

  if(p1.Distance(p2) < dist_delta) return; 


  vector<MyHalfSegment> mhs;

  FindPathInRegion(groom, h, mhs, &p1, &p2); 
  ///////////////// conver to 3D line //////////////////////////////
  Line3D* l3d = new Line3D(0);
  l3d->StartBulkLoad();
  
  for(unsigned int i = 0;i < mhs.size();i++){
    Point p = mhs[i].from; 
    Point3D q(true, p.GetX(), p.GetY(), h);
    *l3d += q;
    if(i == mhs.size() - 1){
      Point p1 = mhs[i].to; 
      Point3D q1(true, p1.GetX(), p1.GetY(), h);
      *l3d += q1;
    }
  }
  l3d->EndBulkLoad();
  
  groom_oid_list.push_back(groom_oid);
  door_tid_list1.push_back(tid1);
  door_tid_list2.push_back(tid2);
  path_list.push_back(*l3d);
  
  delete l3d; 
}

/*
find the path in one floor 

vector MyHalfSegment  store the result 

*/
void IndoorNav::FindPathInRegion(GRoom* groom, float h, 
                                 vector<MyHalfSegment>& mhs, 
                                 Point* p1, Point* p2)
{
  const double dist_delta = 0.001; 
  Region* r = new Region(0);

  int i = 0;
  for(;i < groom->Size();i++){
    float temp_h;
    groom->Get(i, temp_h, *r);
    if(fabs(h - temp_h) < dist_delta){
      break; 
    }
    r->Clear();
  }
  if(i == groom->Size()){
    cout<<" such height " <<h <<"does not exist in the GRoom"<<endl;
    assert(false);
  }

  Line* l = new Line(0);

//  cout<<*r<<endl;
//  cout<<" p1 "<<*p1<<" p2 "<<*p2<<endl; 
  ShortestPath_InRegion(r, p1, p2, l); 
//  cout<<"l "<<*l<<endl; 
  
  SimpleLine* sl = new SimpleLine(0);
  sl->fromLine(*l); 
  delete l; 
  SpacePartition* sp = new SpacePartition();

  sp->ReorderLine(sl, mhs);
  delete sp; 
  delete sl;
  delete r; 

  if(mhs[0].from.Distance(*p1) < dist_delta && 
     mhs[mhs.size() - 1].to.Distance(*p2) < dist_delta) return; 
  
  
  assert(mhs[mhs.size() - 1].to.Distance(*p1) < dist_delta && 
         mhs[0].from.Distance(*p2) < dist_delta);
  vector<MyHalfSegment> temp_mhs;
  for(int i = mhs.size() - 1;i >= 0;i--){
    MyHalfSegment seg = mhs[i]; 
    Point p = seg.from;
    seg.from = seg.to;
    seg.to = p; 
    temp_mhs.push_back(seg);
  }
  mhs.clear();
  for(unsigned int i = 0;i < temp_mhs.size();i++)
    mhs.push_back(temp_mhs[i]);

  if(mhs[0].from.Distance(*p1) < dist_delta && 
     mhs[mhs.size() - 1].to.Distance(*p2) < dist_delta) return; 

  assert(false); 
}


/*
constuct a middle path in the staircase 
the middle path does not include the lowest and highest level 

*/
bool Floor3DCompare(const Floor3D& f1, const Floor3D& f2)
{
  return f1.GetHeight() < f2.GetHeight(); 
}
void IndoorNav::ConstructMiddlePath(GRoom* groom, 
                                    vector<MySegHeight>& middle_path)
{
  vector<Floor3D> floors; 
  for(int i = 0;i < groom->Size();i++){
    float h;
    Region r(0);
    groom->Get(i, h, r);
    Floor3D floor(h, r);
    floors.push_back(floor); 
  }
  
  sort(floors.begin(), floors.end(), Floor3DCompare);
  
  ///////// from low height to high height ////////////////////
  for(unsigned int i = 0; i < floors.size();i++){
//    cout<<floors[i].GetHeight()<<endl; 
    ///////////////////////////////////////////////////////////
    if(i == floors.size() - 1 || i == 0) continue; 
    ////////////////////////////////////////////////////////////
    
    Region* cur_r = const_cast<Region*>(floors[i].GetRegion());
    Region* r1 = const_cast<Region*>(floors[i - 1].GetRegion());
    Region* r2 = const_cast<Region*>(floors[i + 1].GetRegion());
    
    Line* l1 = new Line(0);
    r1->Boundary(l1);
    Line* l2 = new Line(0);
    r2->Boundary(l2);
    
    Line* il1 = new Line(0);
    Line* il2 = new Line(0);
    cur_r->Intersection(*l1, *il1);
    cur_r->Intersection(*l2, *il2);
    assert(il1->Size() == 2 && il2->Size() == 2);

    HalfSegment hs1;
    HalfSegment hs2; 
    il1->Get(0, hs1);
    il2->Get(0, hs2); 

    delete il1;
    delete il2; 
    delete l1;
    delete l2; 

    
    double x1 = (hs1.GetLeftPoint().GetX() + hs1.GetRightPoint().GetX())/2;
    double y1 = (hs1.GetLeftPoint().GetY() + hs1.GetRightPoint().GetY())/2;
    Point p1(true, x1, y1);
    
    double x2 = (hs2.GetLeftPoint().GetX() + hs2.GetRightPoint().GetX())/2;
    double y2 = (hs2.GetLeftPoint().GetY() + hs2.GetRightPoint().GetY())/2;
    Point p2(true, x2, y2);

    MySegHeight mysegh(true, p1, p2, floors[i].GetHeight()); 
    middle_path.push_back(mysegh);
  }
  
  //////////////////////////////////////////////////////////////////////
  ///////////////////for debuging///////////////////////////////////////
  /////////////////////////////////////////////////////////////////////
  
/*  float low_h = floors[0].GetHeight();
  float high_h = floors[floors.size() - 1].GetHeight(); 

  Line3D* l3d = new Line3D(0);
  l3d->StartBulkLoad();
  for(unsigned int i = 0 ;i < middle_path.size();i++){
    if(i == 0){
      Point3D p(true, middle_path[i].from.GetX(), 
                      middle_path[i].from.GetY(), low_h);
      *l3d += p;
    }

    Point3D p1(true, middle_path[i].from.GetX(), 
               middle_path[i].from.GetY(), middle_path[i].h);
    Point3D p2(true, middle_path[i].to.GetX(), 
               middle_path[i].to.GetY(), middle_path[i].h);
    *l3d += p1;
    *l3d += p2; 

    if(i == middle_path.size() - 1){
      Point3D p(true, middle_path[i].to.GetX(), 
                      middle_path[i].to.GetY(), high_h);
      *l3d += p;
    }

  }

  l3d->EndBulkLoad();

  groom_oid_list.push_back(1);
  door_tid_list1.push_back(1);
  door_tid_list2.push_back(1);
  path_list.push_back(*l3d);

  delete l3d; */

}

/*
build the connection for two doors of a staircase at different levels
middle path from low height to heigh height 

*/
void IndoorNav::ST_ConnectFloors(int groom_oid, GRoom* groom, Line* l1, 
                                   Line* l2, int tid1, int tid2, 
                                   float h1, float h2,
                                   vector<MySegHeight>& middle_path)
{
//  cout<<" ST_ConnectFloorS "<<endl; 
  HalfSegment hs1;
  assert(l1->Size() == 2);
  l1->Get(0, hs1); 
  
  HalfSegment hs2; 
  assert(l2->Size() == 2);
  l2->Get(0, hs2); 

  double x1 = (hs1.GetLeftPoint().GetX() + hs1.GetRightPoint().GetX())/2;
  double y1 = (hs1.GetLeftPoint().GetY() + hs1.GetRightPoint().GetY())/2; 
  Point p1(true, x1, y1);

  double x2 = (hs2.GetLeftPoint().GetX() + hs2.GetRightPoint().GetX())/2;
  double y2 = (hs2.GetLeftPoint().GetY() + hs2.GetRightPoint().GetY())/2; 
  Point p2(true, x2, y2);

  
  vector<Floor3D> floors; 
  for(int i = 0;i < groom->Size();i++){
    float h;
    Region r(0);
    groom->Get(i, h, r);
    Floor3D floor(h, r);
    floors.push_back(floor); 
  }
  sort(floors.begin(), floors.end(), Floor3DCompare);


  if(h1 < h2){
    Point q1 = middle_path[0].from;
    Point q2 = middle_path[middle_path.size() - 1].to; 

    vector<MyHalfSegment> mhs1;
    FindPathInRegion(groom, h1, mhs1, &p1, &q1);
    vector<MyHalfSegment> mhs2;
    FindPathInRegion(groom, h2, mhs2, &q2, &p2);

    Line3D* l3d = new Line3D(0);
    l3d->StartBulkLoad();

    for(unsigned int i = 0 ;i < mhs1.size();i++){
        Point p = mhs1[i].from;
        Point3D q(true, p.GetX(), p.GetY(), h1);
        *l3d += q;
    }

    for(unsigned int i = 0 ;i < middle_path.size();i++){
        Point p = middle_path[i].from;
        if(i == 0){
          Point3D q(true, p.GetX(), p.GetY(), h1);
          *l3d += q;
        }
        Point3D q(true, p.GetX(), p.GetY(), middle_path[i].h);
        *l3d += q;

    }
//    cout<<"mhs2 size "<<mhs2.size()<<endl; 
    for(unsigned int i = 0 ;i < mhs2.size();i++){
        Point p = mhs2[i].from;
        Point3D q(true, p.GetX(), p.GetY(), h2);
        *l3d += q;
        if(i == mhs2.size() - 1){
          Point p = mhs2[i].to;
          Point3D q(true, p.GetX(), p.GetY(), h2);
          *l3d += q;
        }
    }

    l3d->EndBulkLoad();

    groom_oid_list.push_back(groom_oid);
    door_tid_list1.push_back(tid1);
    door_tid_list2.push_back(tid2);
    path_list.push_back(*l3d);

  }else{ //h1 > h2 

    Point q1 = middle_path[middle_path.size() - 1].to; 
    Point q2 = middle_path[0].from;

    vector<MyHalfSegment> mhs1;
    FindPathInRegion(groom, h1, mhs1, &p1, &q1);
    vector<MyHalfSegment> mhs2;
    FindPathInRegion(groom, h2, mhs2, &q2, &p2);

    Line3D* l3d = new Line3D(0);
    l3d->StartBulkLoad();
    
    for(unsigned int i = 0 ;i < mhs1.size();i++){
        Point p = mhs1[i].from;
        Point3D q(true, p.GetX(), p.GetY(), h1);
        *l3d += q;
    }

    for(int i = middle_path.size() - 1;i >= 0; i--){
        Point p = middle_path[i].from;
        if(i == middle_path.size() - 1){
          Point3D q(true, p.GetX(), p.GetY(), h1);
          *l3d += q;
        }
        Point3D q(true, p.GetX(), p.GetY(), middle_path[i].h);
        *l3d += q;
    }

    for(unsigned int i = 0 ;i < mhs2.size();i++){
        Point p = mhs2[i].from;
        Point3D q(true, p.GetX(), p.GetY(), h2);
        *l3d += q;
        if(i == mhs2.size() - 1){
            Point p = mhs2[i].to;
            Point3D q(true, p.GetX(), p.GetY(), h2);
            *l3d += q;
        }
    }

    l3d->EndBulkLoad();

    groom_oid_list.push_back(groom_oid);
    door_tid_list1.push_back(tid1);
    door_tid_list2.push_back(tid2);
    path_list.push_back(*l3d);

  }

}

/*
compute the path for each pair of doors inside one room 
The rooms needs to be considered: OR, CO
BR (bath room) is excluded 
Note: in the original data. 
!!! the position of the door should be covered by the room !!!
otherwise, it can not find the shorest path connecting two doors inside one 
room

*/

void IndoorNav::BuildPathORAndCO(int groom_oid, 
                          GRoom* groom, vector<int> tid_list, 
                          int attr1, int attr2, 
                          int attr3, int attr4)
{
  if(groom->Size() > 1){
    cout<<"groom has several levels"<<endl; 
    cout<<"not implemented yet"<<endl; 
    assert(false);
  }
  
  const double dist_delta = 0.001; 
  for(unsigned int i = 0;i < tid_list.size();i++){
    Tuple* door_tuple1 = rel2->GetTuple(tid_list[i], false);
    float h1 = ((CcReal*)door_tuple1->GetAttribute(attr4))->GetRealval();
    Line* l1 = (Line*)door_tuple1->GetAttribute(attr2);

    for(unsigned int j = 0;j < tid_list.size();j++){
        if(tid_list[j] == tid_list[i]) continue; 
        Tuple* door_tuple2 = rel2->GetTuple(tid_list[j], false);
        float h2 = ((CcReal*)door_tuple2->GetAttribute(attr4))->GetRealval();
        Line* l2 = (Line*)door_tuple2->GetAttribute(attr2);
        if(fabs(h1-h2) < dist_delta){ // on the same floor 
          ST_ConnectOneFloor(groom_oid, groom, l1, l2,
                               tid_list[i], tid_list[j], h1);
        }else
            assert(false); 
        door_tuple2->DeleteIfAllowed(); 
    }

    door_tuple1->DeleteIfAllowed();
  }

}
/////////////////////////////////////////////////////////////////////////////
////////// shortest path inside a polygon (convex, concave, holes) /////////
////////////////////////////////////////////////////////////////////////////

string PointsTypeInfo =
  "(rel(tuple((v point)(neighbor1 point)(neighbor2 point)(regid int))))";
  
  
void ShortestPath_InRegion(Region* reg, Point* s, Point* d, Line* pResult)
{
  const double dist_delta = 0.001; 
  if(reg->Contains(*s) == false || reg->Contains(*d) == false){
    cout<<"the point should be inside the region"<<endl;
    return;
  }
  if(reg->NoComponents() > 1){
    cout<<"only one face is allowed"<<endl;
    return; 
  }
  if(s->Distance(*d) < dist_delta){
    cout<<"start location equals to the end locaton"<<endl;
    return; 
  }

/*  Word xResult;
  string strQuery = "(region(((";
  /////////////first create a region object by reg //////////////////////////
  Hole* reg_hole = new Hole();
  reg_hole->GetHole(reg);
  for(unsigned int i = 0;i < reg_hole->regs.size();i++){
    Region* r = &reg_hole->regs[i];
    vector<Point> ps;
    GetBoundaryPoints(r, ps, i);
    for(unsigned int j = 0;j < ps.size();j++){
      string str = "(";
      char coord[256]; 
      memset(coord,'\0',256);
      sprintf(coord,"%f %f", ps[j].GetX(), ps[j].GetY());
      str += coord;
      str += ")";
      strQuery += str; 
    }
  }
  
  strQuery += "))))"; 
  delete reg_hole; 
  cout<<strQuery<<endl; 
  
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  Region* temp = (Region*)xResult.addr; 
  cout<<*temp<<endl; */
  
  //////////////////create the relation of all points////////////////////
/* strQuery = "(consume(getallpoints r1))";
  cout<<strQuery<<endl; 
  
  
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  Relation* noderel = (Relation*)xResult.addr;
  
  cout<<noderel->GetNoTuples()<<endl; 
  delete noderel;*/

   /////////////////////////////////////////////////////////////////////
   ////////////// naive method to compute shortest path in a polygon////
   /////////////////////////////////////////////////////////////////////


  /////////////////////// collect all points//////////////////////////////
  vector<PointAndID> ps_list; 
  Points* ps = new Points(0);
  reg->Vertices(ps);
  vector<bool> visit_flag;
  for(int i = 0;i < ps->Size();i++){
    Point p;
    ps->Get(i, p);
    PointAndID* paid = new PointAndID(i + 1, p);
    ps_list.push_back(*paid);
//    paid->Print();
    delete paid; 
    visit_flag.push_back(true);
  }
  delete ps; 
  ////////////////////collec all segments////////////////////////////////////
  vector<HalfSegment> seg_list; 
  for(int i = 0;i < reg->Size();i++){
      HalfSegment hs;
      reg->Get(i, hs);
      if(!hs.IsLeftDomPoint())continue; 
      seg_list.push_back(hs);
  }
  //////////////////create start and end point /////////////////////////////
  PointAndID start_loc;
  PointAndID end_loc; 
  bool start = false;
  bool end = false; 
  
  for(unsigned int i = 0;i < ps_list.size();i++){
    Point loc = ps_list[i].loc;
    if(s->Distance(loc) < dist_delta){
        start_loc = ps_list[i];
        start = true; 
    }
    if(d->Distance(loc) < dist_delta){
        end_loc = ps_list[i];
        end = true; 
    }
    if(start && end) break; 
  }
  if(start == false){
    start_loc.pid = 0;
    start_loc.loc = *s;
  }else
    visit_flag[start_loc.pid - 1] = false; 


  if(end == false){
    end_loc.pid = ps_list.size() + 1;
    end_loc.loc = *d; 
    ps_list.push_back(end_loc); 
    visit_flag.push_back(true);
  }
  /////////////////////////////////////////////////////////////////////////
  ////////////first check whether Euclidean distance between s and d//////
  HalfSegment temp_hs(true, start_loc.loc, end_loc.loc);

  if(SegAvailable(temp_hs, seg_list) && RegContainHS(reg, temp_hs)){
//    cout<<"find the path"<<endl; 
    pResult->StartBulkLoad();
    int edgeno = 0;
    HalfSegment hs(true, start_loc.loc, end_loc.loc); 
    hs.attr.edgeno = edgeno++;
    *pResult += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    *pResult += hs; 
    pResult->EndBulkLoad(); 
    return; 
  }
  //////////////searching inside the region//////////////////////////////
//  for(unsigned int i = 0;i < pslist.size();i++)
//    pslist[i].Print(); 
  
  
  priority_queue<RPath_elem> path_queue; 
  vector<RPath_elem> expand_path;

  InitializeQueue(reg, path_queue, expand_path, start_loc, end_loc,  
                  ps_list, seg_list, visit_flag); 
//  cout<<" initialize size "<<pathqueue.size()<<endl; 
//  cout<<" expand size "<<expandpath.size()<<endl; 
  bool find = false; 
  RPath_elem dest;
   while(path_queue.empty() == false){
    RPath_elem top = path_queue.top();
    path_queue.pop();
//    top.Print();
    PointAndID cur_loc = ps_list[top.tri_index - 1];

    if(top.tri_index == end_loc.pid){
//       cout<<"find the path"<<endl;
       find = true;
       dest = top; 
       break;
    }

    vector<int> adj_list;
    FindAdj(reg, cur_loc, visit_flag, adj_list, ps_list, seg_list);
    int pos_expand_path = top.cur_index;
    for(unsigned int i = 0;i < adj_list.size();i++){
//      cout<<" neighbor ";

      int expand_path_size = expand_path.size();
      double w = cur_loc.loc.Distance(ps_list[adj_list[i] - 1].loc); 
      double hw = ps_list[adj_list[i] - 1].loc.Distance(end_loc.loc);
      
      path_queue.push(RPath_elem(pos_expand_path, expand_path_size,
                                adj_list[i], top.real_w + w + hw,
                                 top.real_w + w));
      expand_path.push_back(RPath_elem(pos_expand_path, expand_path_size,
                                adj_list[i], top.real_w + w + hw, 
                                top.real_w + w));
    }

    visit_flag[top.tri_index - 1] = false; 

//    cout<<endl; 
  }
  if(find){
   vector<Point> points; 
    while(dest.prev_index != -1){
      points.push_back(ps_list[dest.tri_index - 1].loc);
      dest = expand_path[dest.prev_index];
    }
   points.push_back(ps_list[dest.tri_index - 1].loc);
   points.push_back(start_loc.loc);

   pResult->StartBulkLoad();
   int edgeno = 0;

   for(unsigned int i = 0;i < points.size() - 1;i++){

    HalfSegment hs(true, points[i], points[i + 1]); 
    hs.attr.edgeno = edgeno++;
    *pResult += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    *pResult += hs; 
   }
    pResult->EndBulkLoad(); 
  }

}


/*
Initialize the queue for searching, put all points which are reachable to the
start point 

*/
void InitializeQueue(Region* reg, priority_queue<RPath_elem>& path_queue, 
                     vector<RPath_elem>& expand_queue,
                     PointAndID start_loc, PointAndID end_loc,
                     vector<PointAndID>& ps_list,
                     vector<HalfSegment>& seg_list, vector<bool>& visit_flag)
{
//  cout<<"Initialize Queue "<<endl;

  vector<int> candidate; 
  for(unsigned int i = 0;i < ps_list.size();i++){
    if(start_loc.pid != ps_list[i].pid){
      HalfSegment hs(true, start_loc.loc, ps_list[i].loc);
      if(SegAvailable(hs, seg_list)){
            candidate.push_back(i);
      }
    }
  }
  for(unsigned int i = 0;i < candidate.size();i++){
    unsigned int j = candidate[i]; 
    HalfSegment hs(true, start_loc.loc, ps_list[j].loc);
    if(RegContainHS(reg, hs)){
        int cur_size = expand_queue.size();
        double w = start_loc.loc.Distance(ps_list[j].loc);
        double hw = ps_list[j].loc.Distance(end_loc.loc);
        RPath_elem elem(-1, cur_size, ps_list[j].pid, w + hw, w);
        path_queue.push(elem);
        expand_queue.push_back(elem); 
    }
  }
}

/*
find points which are available

*/
void FindAdj(Region* reg, PointAndID top, vector<bool>& visit_flag, 
             vector<int>& adj_list, vector<PointAndID>& ps_list,
             vector<HalfSegment>& seg_list)
{

  vector<int> candidate; 
  for(unsigned int i = 0;i < ps_list.size();i++){
    if(top.pid != ps_list[i].pid && visit_flag[i]){
      HalfSegment hs(true, top.loc, ps_list[i].loc);
      if(SegAvailable(hs, seg_list)){
        candidate.push_back(i);
      }
    }
  }

  for(unsigned int i = 0;i < candidate.size();i++){
    unsigned int j = candidate[i];
    HalfSegment hs(true, top.loc, ps_list[j].loc);
    if(RegContainHS(reg, hs))
        adj_list.push_back(ps_list[j].pid); 
  }
}

/*
check whether the segment middle intersects another one 

*/
bool SegAvailable(HalfSegment hs, vector<HalfSegment>& seg_list)
{
  const double dist_delta = 0.001; 
  for(unsigned int i = 0;i < seg_list.size();i++){
    HalfSegment seg = seg_list[i];

    if( (hs.GetLeftPoint().Distance(seg.GetLeftPoint()) < dist_delta &&
         hs.GetRightPoint().Distance(seg.GetRightPoint()) < dist_delta )|| 
       (hs.GetLeftPoint().Distance(seg.GetRightPoint()) < dist_delta &&
         hs.GetRightPoint().Distance(seg.GetLeftPoint()) < dist_delta ))
      return true;

    HalfSegment iseg; 
    if(hs.Intersection(seg, iseg) && iseg.Length() > dist_delta){
      if(fabs(iseg.Length() - hs.Length()) < dist_delta) return true;
      return false; 
    } 

    Point ip;
    if(hs.Intersection(seg, ip)){
      if( hs.GetLeftPoint().Distance(ip) < dist_delta ||
          hs.GetRightPoint().Distance(ip) < dist_delta)
          continue;
      else
        return false; 
    }
  }
  return true; 
}

/*
collect the boundary points of a region cycle 

*/

void GetBoundaryPoints(Region* reg, vector<Point>& ps, unsigned int i)
{   
    SimpleLine* sl = new SimpleLine(0);
    sl->StartBulkLoad();
    int edgeno = 0;
    for(int j = 0;j < reg->Size();j++){
        HalfSegment hs1;
        reg->Get(j, hs1);
        if(!hs1.IsLeftDomPoint()) continue;
        HalfSegment hs2;
        hs2.Set(true, hs1.GetLeftPoint(), hs1.GetRightPoint());

        hs2.attr.edgeno = edgeno++;
        *sl += hs2;
        hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
        *sl += hs2;
    }
    sl->EndBulkLoad();
    
    SpacePartition* sp = new SpacePartition();
    vector<MyHalfSegment> mhs;
    sp->ReorderLine(sl, mhs);
    for(unsigned int j = 0;j < mhs.size();j++)
       ps.push_back(mhs[j].from);
    delete sp; 
    delete sl; 
    
    
    CompTriangle* ct = new CompTriangle();
    bool clock;
    if(0.0f < ct->Area(ps)){//points counter-clockwise order
        clock = false;
    }else{// points clockwise
        clock = true;
    }
    
    vector<Point> temp_ps1; //clockwise 
    vector<Point> temp_ps2; //counter-clockwise 
    if(clock){
      for(unsigned int i = 0;i < ps.size();i++){
        temp_ps1.push_back(ps[i]);
        temp_ps2.push_back(ps[ps.size() - 1 - i]); 
      }
    }else{
      for(unsigned int i = 0;i < ps.size();i++){
        temp_ps2.push_back(ps[i]);
        temp_ps1.push_back(ps[ps.size() - 1 - i]); 
      }
    }
    if(i == 0){
      ps.clear();
      for(unsigned int i = 0;i < temp_ps1.size();i++)
        ps.push_back(temp_ps1[i]);
    }else{
      ps.clear();
      for(unsigned int i = 0;i < temp_ps2.size();i++)
        ps.push_back(temp_ps2[i]);
    }

    delete ct; 
}
//////////////////////////////////////////////////////////////////////////
///////////////////Indoor Graph for Navigation//////////////////////////
/////////////////////////////////////////////////////////////////////////


string IndoorGraph::NodeTypeInfo =
"(rel (tuple ((Door door3d) (door_loc line) (groom_oid1 int) (groom_oid2 int)\
  (doorheight real) (doortype int))))";
  
string IndoorGraph::EdgeTypeInfo =
"(rel (tuple ((groom_oid int) (door_tid1 int) (door_tid2 int) (Path line3d))))";



bool IndoorGraph::CheckIndoorGraph(ListExpr type, ListExpr& errorInfo)
{
  cout<<"CheckIndoorGraph()"<<endl;
  return nl->IsEqual(type, "indoorgraph");
}

void IndoorGraph::CloseIndoorGraph(const ListExpr typeInfo, Word& w)
{
  cout<<"CloseIndoorGraph()"<<endl;
  delete static_cast<IndoorGraph*> (w.addr);
  w.addr = NULL;
}

void IndoorGraph::DeleteIndoorGraph(const ListExpr typeInfo, Word& w)
{
  cout<<"DeleteIndoorGraph()"<<endl;
  IndoorGraph* ig = (IndoorGraph*)w.addr;
  delete ig;
  w.addr = NULL;
}

Word IndoorGraph::CreateIndoorGraph(const ListExpr typeInfo)
{
  cout<<"CreateIndoorGraph()"<<endl;
  return SetWord(new IndoorGraph());
}

/*
Input a indoor graph 

*/

Word IndoorGraph::InIndoorGraph(ListExpr in_xTypeInfo,
                            ListExpr in_xValue,
                            int in_iErrorPos, ListExpr& inout_xErrorInfo,
                            bool& inout_bCorrect)
{
//  cout<<"InVisualGraph()"<<endl;
  VisualGraph* vg = new VisualGraph(in_xValue, in_iErrorPos, inout_xErrorInfo,
                                inout_bCorrect);
  if(inout_bCorrect) return SetWord(vg);
  else{
    delete vg;
    return SetWord(Address(0));
  }
}

