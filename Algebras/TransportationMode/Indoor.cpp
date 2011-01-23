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

/*
output function for data type point3d

*/
ListExpr OutPoint3D(ListExpr typeInfo, Word value)
{
  Point3D* p3d = (Point3D*)value.addr;
  if(p3d->IsDefined()){
      return nl->ThreeElemList(
              nl->RealAtom(p3d->GetX()),
              nl->RealAtom(p3d->GetY()),
              nl->RealAtom(p3d->GetZ())
         );
  }else
    return nl->SymbolAtom("undef");
}

/*
input function for data type point3d

*/
Word InPoint3D(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct)
{
  correct = true;
  if( nl->ListLength( instance ) == 3 ) {
    ListExpr first = nl->First(instance);
    ListExpr second = nl->Second(instance);
    ListExpr third = nl->Third(instance);

    correct = listutils::isNumeric(first) &&
              listutils::isNumeric(second) && listutils::isNumeric(third);
    if(!correct){
       return SetWord( Address(0) );
    } else {
      return SetWord(new Point3D(true, listutils::getNumValue(first),
                                       listutils::getNumValue(second),
                                        listutils::getNumValue(third)));
    }
  } else if( listutils::isSymbol( instance, "undef" ) ){
     return SetWord(new Point3D(false));
  }
  correct = false;
  return SetWord( Address(0) );
}


/*
Creation of the type constructor instance for point3d

*/

ListExpr Point3DProperty()
{
  return nl->TwoElemList(
           nl->FourElemList(
             nl->StringAtom("Signature"),
             nl->StringAtom("Example Type List"),
             nl->StringAtom("List Rep"),
             nl->StringAtom("Example List")),
           nl->FourElemList(
             nl->StringAtom("-> DATA"),
             nl->StringAtom("point3d"),
             nl->StringAtom("(x y z)"),
             nl->StringAtom("(10 5 2)")));
}


Word CreatePoint3D(const ListExpr typeInfo)
{
  return SetWord (new Point3D(false));
}

void DeletePoint3D(const ListExpr typeInfo, Word& w)
{
//  ((Point3D*)w.addr)->DeleteIfAllowed();
  Point3D* p3d = (Point3D*)w.addr;
  delete p3d;
   w.addr = NULL;
}

void ClosePoint3D(const ListExpr typeInfo, Word& w)
{
//  ((Point3D*)w.addr)->DeleteIfAllowed();
  Point3D* p3d = (Point3D*)w.addr;
  delete p3d;
  w.addr = NULL;
}


Word ClonePoint3D(const ListExpr typeInfo, const Word& w)
{
  Point3D* p3d = new Point3D(*(Point3D*)w.addr);
  return SetWord(p3d);
}

void* CastPoint3D(void* addr)
{
  return new (addr)Point3D();
}

int SizeOfPoint3D()
{
  return sizeof(Point3D);
}

bool CheckPoint3D(ListExpr type, ListExpr& errorInfo)
{
  return nl->IsEqual(type, "point3d");
}

/*
save function for point3d

*/
bool SavePoint3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
    Point3D* p3d = (Point3D*)value.addr;
    return p3d->Save(valueRecord, offset, typeInfo);
}

bool OpenPoint3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
  value.addr = new Point3D(valueRecord, offset, typeInfo);
  return value.addr != NULL;
}

ostream& operator<<(ostream& o, const Point3D& loc)
{
  if(loc.IsDefined()){
    o<<"( "<<loc.GetX()<<" "<<loc.GetY()<<" "<<loc.GetZ()<<" )"; 
  }else
    o<<" undef";
  return o;

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

bool Line3D::operator==( const Line3D& l) const
{
  vector<Point3D> ps_list1;
  vector<Point3D> ps_list2;
  
  if(Size() != l.Size()) return false; 
  
  for(int i = 0;i < points.Size();i++){
    Point3D p;
    points.Get(i, p);
    ps_list1.push_back(p);
  }
  
  for(int i = 0;i < l.Size();i++){
    Point3D p;
    l.Get(i, p);
    ps_list2.push_back(p);
  }

  sort(ps_list1.begin(), ps_list1.end());
  sort(ps_list2.begin(), ps_list2.end());

  for(unsigned int i = 0;i < ps_list1.size();i++){
    if(!(ps_list1[i] == ps_list2[i])) return false; 
  }

  return true; 
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
  for(int i = 0;i < points.Size() - 1;i++){
    Point3D p1;
    points.Get(i, p1);
    Point3D p2;
    points.Get(i + 1, p2);
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

const Rectangle<3> Line3D::BoundingBox() const
{
//  return new Rectangle<3>(true,0.0,0.0,0.0,0.0,0.0,0.0);
//  cout<<"Rectangle<3> BoundingBox "<<endl; 
  double min[3];
  double max[3];
  for(int i = 0;i < 3;i++){
    min[i] = numeric_limits<double>::max();
    max[i] = numeric_limits<double>::min();
  }

  for(int i = 0;i < points.Size();i++){
      Point3D p;
      points.Get(i, p);

      min[0] = MIN(p.GetX(), min[0]);
      min[1] = MIN(p.GetY(), min[1]);
      min[2] = MIN(p.GetZ(), min[2]);
      
      max[0] = MAX(p.GetX(), max[0]);
      max[1] = MAX(p.GetY(), max[1]);
      max[2] = MAX(p.GetZ(), max[2]);
  }

  Rectangle<3> bbox3d(true, min, max);
  return bbox3d; 
}


/*
List Representation

The list representation of a point is

----  (x y)
----

~Out~-function

*/
ListExpr OutLine3D( ListExpr typeInfo, Word value )
{
  Line3D* points = (Line3D*)(value.addr);
  if(!points->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  if( points->IsEmpty() )
    return nl->TheEmptyList();

//  Point p;
  Point3D p;
  assert( points->Get( 0, p ) );
/*  ListExpr result =
    nl->OneElemList( OutPoint( nl->TheEmptyList(), SetWord( (void*)&p ) ) );*/
 ListExpr result =
    nl->OneElemList( OutPoint3D( nl->TheEmptyList(), SetWord( (void*)&p ) ) );
  ListExpr last = result;

  for( int i = 1; i < points->Size(); i++ )
  {
    assert( points->Get( i, p ) );
/*   last = nl->Append( last,
                      OutPoint( nl->TheEmptyList(), SetWord( (void*)&p ) ) );*/
    last = nl->Append( last,
                       OutPoint3D( nl->TheEmptyList(), SetWord( (void*)&p)));
  }
  return result;
}

/*
In function

*/
Word InLine3D( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if(nl->IsEqual(instance,"undef")) {
      Points* points = new Points(0);
      points->Clear();
      points->SetDefined(false);
      correct=true;
      return SetWord( Address(points) );
  }
  Line3D* points = new Line3D( max(0,nl->ListLength( instance) ) );
  points->SetDefined( true );
  if(nl->AtomType(instance)!=NoAtom) {
    points->DeleteIfAllowed();
    correct = false;
    cout << __PRETTY_FUNCTION__ << ": Unexpected Atom!" << endl;
    return SetWord( Address(points) );
  }

  ListExpr rest = instance;
  points->StartBulkLoad();
  while( !nl->IsEmpty( rest ) ) {
    ListExpr first = nl->First( rest );
    rest = nl->Rest( rest );

/*    Point *p = (Point*)InPoint( nl->TheEmptyList(),
                                first, 0, errorInfo, correct ).addr;*/
    Point3D *p = (Point3D*)InPoint3D( nl->TheEmptyList(),
                                first, 0, errorInfo, correct ).addr;

    if( correct && p->IsDefined() ) {
      (*points) += (*p);
      delete p;
    } else {
      if(p) {
        delete p;
      }
      cout << __PRETTY_FUNCTION__ << ": Incorrect or undefined point!" << endl;
      points->DeleteIfAllowed();
      correct = false;
      return SetWord( Address(0) );
    }

  }
  points->EndBulkLoad();

  if( points->IsValid() ) {
    correct = true;
    return SetWord( points );
  }
  points->DeleteIfAllowed();
  correct = false;
  cout << __PRETTY_FUNCTION__ << ": Invalid points value!" << endl;
  return SetWord( Address(0) );
}

/*
Create function

*/
Word CreateLine3D( const ListExpr typeInfo )
{
//  cout<<"CreateLine3D "<<endl; 
  return SetWord( new Line3D( 0 ) );
}

/*
Delete function

*/
void DeleteLine3D( const ListExpr typeInfo, Word& w )
{
//  cout<<"DeleteLine3D "<<endl; 
  Line3D *ps = (Line3D *)w.addr;
  ps->Destroy();
  ps->DeleteIfAllowed(false);
  w.addr = 0;
}

/*
Close function

*/
void CloseLine3D( const ListExpr typeInfo, Word& w )
{
//  cout<<"CloseLine3D "<<endl; 
  ((Line3D *)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

/*
Clone function

*/
Word CloneLine3D( const ListExpr typeInfo, const Word& w )
{
  return SetWord( new Line3D( *((Line3D *)w.addr) ) );
}

/*
Open function

*/
bool OpenLine3D( SmiRecord& valueRecord, size_t& offset,
            const ListExpr typeInfo, Word& value )
{
  Line3D *ps = (Line3D*)Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord( ps );
  return true;
}

/*
Save function

*/
bool SaveLine3D( SmiRecord& valueRecord, size_t& offset,
            const ListExpr typeInfo, Word& value )
{
  Line3D *ps = (Line3D*)value.addr;
  Attribute::Save( valueRecord, offset, typeInfo, ps );
  return true;
}

/*
SizeOf function

*/
int SizeOfLine3D()
{
  return sizeof(Line3D);
}


ListExpr Line3DProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("line3d"),
           nl->StringAtom("(<point3d>*) where point3d is (<x><y><z>)"),
           nl->StringAtom("( (10 1 2)(4 5 3) )"))));
}


bool CheckLine3D( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "line3d" ));
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

/*
get the 2D area covered by the region

*/

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
return the highest height of a groom 

*/
float GRoom::GetHighHeight()
{
  float h = numeric_limits<float>::min(); 
  for( int i = 0; i < elem_list.Size(); i++ ){
    FloorElem felem;
    elem_list.Get(i, felem);
    if(felem.h > h)
      h = felem.h; 
  }
  return h; 
}

/*
get the 3D box of a groom 

*/
const Rectangle<3> GRoom::BoundingBox3D() const
{

    Rectangle<3> bbox;
    for( int i = 0; i < Size(); i++ ){
        Region r(0);
        float h;
        Get( i, h, r);
        if( i == 0 ){
          Rectangle<2> reg_box = r.BoundingBox(); 
          bbox = Rectangle<3>(true, reg_box.MinD(0), reg_box.MaxD(0), 
                       reg_box.MinD(1), reg_box.MaxD(1), 
                       h, h + ApplyFactor(h));
        }else{
          Rectangle<2> reg_box = r.BoundingBox(); 
          Rectangle<3> bbox1 = 
                       Rectangle<3>(true, reg_box.MinD(0), reg_box.MaxD(0), 
                       reg_box.MinD(1), reg_box.MaxD(1), 
                       h, h + ApplyFactor(h));
          bbox = bbox.Union(bbox1);

        } 
    }
    return bbox;
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
create a 3d line for the door. We use RTree to find the doors that their 
locations are the same in space. 
Because for the data type door3d, we have to know the relative position in 
each room.  

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
    if(GetRoomEnum(s) == ST){//Staircase
//      cout<<"one more box needed"<<endl;
      h = groom->GetHighHeight();
      CreateLine3D(oid, l, h);
    }
    
    if(GetRoomEnum(s) == EL){//Elevator 
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

The result is: for each groom, we create its doors. so usually, the door 
(spatial locatin in space) will be represented twice because it connects two
rooms. 

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

    if(GetRoomEnum(s) == ST){//Staircase
//      cout<<"one more box needed"<<endl;
        h = groom->GetHighHeight();
        CreateBox3D(oid, tid, l, h);
    }

    if(GetRoomEnum(s) == EL){// Elevator
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
//  return -1.0;
}

void IndoorNav::CreateBox3D(int oid, int tid, Line* l, float h)
{
//  const double delta_h = 0.01; 
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
//      max[2] = h + delta_h;
      max[2] = h + ApplyFactor(h); 

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
void IndoorNav::CreateDoor1(R_Tree<3, TupleId>* rtree, 
                           int attr1, int attr2, int attr3)
{
    ///////////// minimum and maximum height of the building////////////////
    vector<float> height_list; 
    for(int i = 1;i <= rel1->GetNoTuples();i++){
      Tuple* groom_tuple = rel1->GetTuple(i, false);
      GRoom* groom = (GRoom*)groom_tuple->GetAttribute(I_Room); 
      height_list.push_back(groom->GetLowHeight());
      groom_tuple->DeleteIfAllowed();
    }
    sort(height_list.begin(), height_list.end()); 
    ///////////////////////////////////////////////////////////////////////
    float max_height = height_list[ height_list.size() - 1]; 
//    cout<<"max_height "<<max_height<<endl; 

//  cout<<"CreateDoor()"<<endl;
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<" attr3 "<<attr3<<endl; 
  SmiRecordId adr = rtree->RootRecordId();

  for(int i = 1;i <= rel2->GetNoTuples();i++){

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

//    cout<<"neighbor size "<<id_list.size()<<endl; 

/*    for(unsigned int j = 0;j < id_list.size();j++)
      cout<<"neighbor tuple id "<<id_list[j]<<endl; 
    cout<<endl; */

    //for staircase and elevator, size = 2
    //the floor at i and i+1 are both represented 
    // the size can be 0 at some special places after translating 
    assert(id_list.size() <= 2);
    ///////////// create the result  ///////////////////////////////
    //////////   use tid to get the tuple /////////////////////////
    ///////////  use the tuple to get oid and line ////////////////
//    if(id_list.size() == 1)
    CreateResDoor(id, oid, tid, id_list, attr1, attr2, attr3);

    //////////////special door for the building entrance ///////////////
    if(id_list.size() == 0 && IsGRoom(tid, rel1)){ 
      if(AlmostEqual(max_height, bbox3d->MinD(2))){
//        cout<<"find "<<endl;
//        cout<<"oid "<<oid<<"tid "<<tid<<" rect "<<*bbox3d<<endl; 
        /////////////create entrance door for the building//////////////
        CreateEntranceDoor(id, oid, tid, attr1, attr2, attr3);
      }
    }

    door_tuple->DeleteIfAllowed();

  }

}

/*
check whether the room is OR or CO. excludes BR, ST, EL

*/
bool IndoorNav::IsGRoom(int tid, Relation* rel)
{
  assert(1 <= tid && tid <= rel->GetNoTuples());
  Tuple* groom_tuple = rel->GetTuple(tid, false);
  string type = ((CcString*)groom_tuple->GetAttribute(I_Type))->GetValue(); 
  groom_tuple->DeleteIfAllowed(); 
  if(GetRoomEnum(type) == OR || GetRoomEnum(type) == CO)
    return true;
  else
    return false; 
}

/*
create the result for door relation: door3d line groomid1 groomid2 
for data type line, region, mbool, the function operator=() is not 
  correctly impelmented. it should copy the value from the dbarray instead of
  calling copyfrom. 
  
  It is no problem in memory, but has problems in disk for storage  

*/
void IndoorNav::CreateResDoor(int id, int oid, int tid, vector<TupleId> id_list,
                     int attr1, int attr2, int attr3)
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
    MBool* mb1 = new MBool(0);
    CreateDoorState(mb1); 

    
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
    Line3D* l3d_1 = new Line3D(0);
    GRoomDoorLine(bbox3d_1, bbox3d_2, l1, l2, l3, &groom_box1, 
                  &groom_box2, l3d_1, bbox3d_1->MinD(2));

    ///////////////////// create the door //////////////////////////////
    Door3D* door_obj1 = new Door3D(oid1, oid2, *l1, *l2, *mb1, lift_door);
    //////////////////////  result   /////////////////////////////// 
    door_list.push_back(*door_obj1);
    line_list.push_back(*l3);
    groom_id_list1.push_back(oid1);
    groom_id_list2.push_back(oid2);
    path_list.push_back(*l3d_1);
    door_heights.push_back(bbox3d_1->MinD(2));


    delete l1;
    delete l2;
    delete l3; 
    delete door_obj1; 
    delete l3d_1; 
    delete mb1; 

    /////////////////////////////////////////////////////////////
    box_tuple1->DeleteIfAllowed();
    box_tuple2->DeleteIfAllowed();
    indoor_tuple2->DeleteIfAllowed();
    indoor_tuple1->DeleteIfAllowed(); 

  }

}

/*
create the time-dependent state for the door 

*/
void IndoorNav::CreateDoorState(MBool* mb)
{
   Instant start_time(instanttype);
   start_time.ReadFrom("2010-12-5-0:0:0");
   Instant end_time(instanttype);
   end_time.ReadFrom("2010-12-6-0:0:0");

   Instant begin_time(instanttype);
   begin_time.ReadFrom("begin of time");
   Instant finish_time(instanttype);
   finish_time.ReadFrom("end of time");

    mb->StartBulkLoad();
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

    mb->Add(ub1);
    mb->Add(ub2);
    mb->Add(ub3);
    mb->EndBulkLoad();
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
create entrance door of the building 
if it is the entrance of the building, we set groom oid1: oid1, groom oid2:-1

*/
void IndoorNav::CreateEntranceDoor(int id, int oid, int tid, 
                               int attr1, int attr2, int attr3)
{

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
    Tuple* box_tuple2 = rel2->GetTuple(tid, false);
    int oid2 = ((CcInt*)box_tuple2->GetAttribute(attr1))->GetIntval();
    int groom_tid = ((CcInt*)box_tuple2->GetAttribute(attr2))->GetIntval();

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
    MBool* mb1 = new MBool(0);
    CreateDoorState(mb1); 

    bool lift_door = false;

    ////////////////////////////////////////////////////////////////
    /////////////////////create line //////////////////////////////////
    ////////////////////////////////////////////////////////////////////
    
    Line* l1 = new Line(0);
    Line* l2 = new Line(0);
    Line* l3 = new Line(0);
    Line3D* l3d_1 = new Line3D(0);
    GRoomDoorLine(bbox3d_1, bbox3d_2, l1, l2, l3, &groom_box1, 
                  &groom_box2, l3d_1, bbox3d_1->MinD(2));

    ///////////////////// create the door //////////////////////////////
    Door3D* door_obj1 = new Door3D(oid1, oid2, *l1, *l2, *mb1, lift_door);
    //////////////////////  result   ///////////////////////////////////
    door_list.push_back(*door_obj1);
    line_list.push_back(*l3);
    groom_id_list1.push_back(oid1);//the room oid 
    groom_id_list2.push_back(-1);//-1 for entrance door 
    path_list.push_back(*l3d_1);
    door_heights.push_back(bbox3d_1->MinD(2));


    delete l1;
    delete l2;
    delete l3; 
    delete door_obj1; 
    delete l3d_1; 
    delete mb1; 

    /////////////////////////////////////////////////////////////
    box_tuple1->DeleteIfAllowed();
    box_tuple2->DeleteIfAllowed();
    indoor_tuple2->DeleteIfAllowed();
    indoor_tuple1->DeleteIfAllowed(); 

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
create virtual doors for the staircase. The position is the place of the 
first footstep (higher than the ground level) which is the place for entering 
the staircase 

*/
void IndoorNav::CreateDoor2()
{
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* groom_tuple = rel1->GetTuple(i, false);
    string type = ((CcString*)groom_tuple->GetAttribute(I_Type))->GetValue();
    if(!(GetRoomEnum(type) == ST)){
      groom_tuple->DeleteIfAllowed();
      continue; 
    }
    int oid = ((CcInt*)groom_tuple->GetAttribute(I_OID))->GetIntval();
    GRoom* groom = (GRoom*)groom_tuple->GetAttribute(I_Room);
    Rectangle<2> groom_box = groom->BoundingBox();
    Line* l1 = new Line(0);
    Line* l2 = new Line(0);
    Line* l3 = new Line(0);
    Line* l4 = new Line(0);
    
    Line3D* l3d1 = new Line3D(0);
    Line3D* l3d2 = new Line3D(0);
    
    
    float h1 = GetVirtualDoor1(groom, l1, l2, l3d1);
    float h2 = GetVirtualDoor2(groom, l3, l4, l3d2);

    
    MBool* mb1 = new MBool(0);
    CreateDoorState(mb1); 

    MBool* mb2 = new MBool(0);
    CreateDoorState(mb2); 


    Door3D* door_obj1 = new Door3D(oid, oid, *l1, *l1, *mb1, false);
    Door3D* door_obj2 = new Door3D(oid, oid, *l3, *l3, *mb2, false);

    door_list.push_back(*door_obj1);
    line_list.push_back(*l2);
    groom_id_list1.push_back(oid);
    groom_id_list2.push_back(oid);
    path_list.push_back(*l3d1);
    door_heights.push_back(h1);


    door_list.push_back(*door_obj2);
    line_list.push_back(*l4);
    groom_id_list1.push_back(oid);
    groom_id_list2.push_back(oid);
    path_list.push_back(*l3d2);
    door_heights.push_back(h2);


    delete l1;
    delete l2; 
    delete l3;
    delete l4; 
    delete l3d1;
    delete l3d2; 
    delete mb1;
    delete mb2; 
    delete door_obj1; 
    delete door_obj2; 
    groom_tuple->DeleteIfAllowed();
  }

}

/*
constuct a middle path in the staircase 
the middle path does not include the lowest and highest level 

*/
bool Floor3DCompare1(const Floor3D& f1, const Floor3D& f2)
{
  return f1.GetHeight() < f2.GetHeight(); 
}


bool Floor3DCompare2(const Floor3D& f1, const Floor3D& f2)
{
  return f1.GetHeight() > f2.GetHeight(); 
}

/*
Create a virtual door, relative position in a groom, absolute position 2D,
absolute position 3D. low level height.
Note: the height for the foorsteps in the input data is set in the reverse way 

*/

float IndoorNav::GetVirtualDoor1(GRoom* groom, Line* l1, Line* l2, Line3D* l3d)
{

  vector<Floor3D> floors; 
  for(int i = 0;i < groom->Size();i++){
    float h;
    Region r(0);
    groom->Get(i, h, r);
    Floor3D floor(h, r);
    floors.push_back(floor);
  }

  sort(floors.begin(), floors.end(), Floor3DCompare2);

  Region* r1 = const_cast<Region*>(floors[0].GetRegion());
  Region* r2 = const_cast<Region*>(floors[1].GetRegion());

  Line* boundary = new Line(0);
  r1->Boundary(boundary);
  r2->Intersection(*boundary, *l2);

  assert(l2->Size() == 2);

  HalfSegment hs;
  l2->Get(0, hs);


  delete boundary;


  Rectangle<2> groom_box = groom->BoundingBox();
   
  double x1 = hs.GetLeftPoint().GetX() - groom_box.MinD(0);
  double y1 = hs.GetLeftPoint().GetY() - groom_box.MinD(1);
  Point p1(true, x1, y1);

  double x2 = hs.GetRightPoint().GetX() - groom_box.MinD(0);
  double y2 = hs.GetRightPoint().GetY() - groom_box.MinD(1);
  Point p2(true, x2, y2);

  //////////////////relative position in the groom////////////////////////
  l1->StartBulkLoad();
  int edgeno = 0;
  HalfSegment temp_hs(true, p1, p2); 
  temp_hs.attr.edgeno = edgeno++;
  *l1 += temp_hs;
  temp_hs.SetLeftDomPoint(!temp_hs.IsLeftDomPoint());
  *l1 += temp_hs; 
  l1->EndBulkLoad(); 
  //////////////////////////////////////////////////////////////////////////
  ////////////////absolute position in 3D space/////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  float h = floors[0].GetHeight(); 
 
  l3d->StartBulkLoad();
  Point3D q1(true, hs.GetLeftPoint().GetX(), hs.GetLeftPoint().GetY(), h);
  Point3D q2(true, hs.GetRightPoint().GetX(), hs.GetRightPoint().GetY(), h);
  *l3d += q1;
  *l3d += q2; 
  l3d->EndBulkLoad();

  return h; 
  //////////////////////////////////////////////////////////////////////////

}

/*
Create a virtual door, relative position in a groom, absolute position 2D,
absolute position 3D. high level height  
Note: the height for the foorsteps in the input data is set in the reverse way 

*/

float IndoorNav::GetVirtualDoor2(GRoom* groom, Line* l1, Line* l2, Line3D* l3d)
{
  vector<Floor3D> floors; 
  for(int i = 0;i < groom->Size();i++){
    float h;
    Region r(0);
    groom->Get(i, h, r);
    Floor3D floor(h, r);
    floors.push_back(floor);
  }

  sort(floors.begin(), floors.end(), Floor3DCompare2);

  Region* r1 = const_cast<Region*>(floors[0].GetRegion());
  Region* r2 = const_cast<Region*>(floors[1].GetRegion());

  Line* boundary = new Line(0);
  r1->Boundary(boundary);
  r2->Intersection(*boundary, *l2);

  assert(l2->Size() == 2);

  HalfSegment hs;
  l2->Get(0, hs);


  delete boundary;


  Rectangle<2> groom_box = groom->BoundingBox();
   
  double x1 = hs.GetLeftPoint().GetX() - groom_box.MinD(0);
  double y1 = hs.GetLeftPoint().GetY() - groom_box.MinD(1);
  Point p1(true, x1, y1);

  double x2 = hs.GetRightPoint().GetX() - groom_box.MinD(0);
  double y2 = hs.GetRightPoint().GetY() - groom_box.MinD(1);
  Point p2(true, x2, y2);

  //////////////////relative position in the groom////////////////////////
  l1->StartBulkLoad();
  int edgeno = 0;
  HalfSegment temp_hs(true, p1, p2); 
  temp_hs.attr.edgeno = edgeno++;
  *l1 += temp_hs;
  temp_hs.SetLeftDomPoint(!temp_hs.IsLeftDomPoint());
  *l1 += temp_hs; 
  l1->EndBulkLoad(); 
  //////////////////////////////////////////////////////////////////////////
  ////////////////absolute position in 3D space/////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  float h = floors[floors.size() - 1].GetHeight(); 
 
  l3d->StartBulkLoad();
  Point3D q1(true, hs.GetLeftPoint().GetX(), hs.GetLeftPoint().GetY(), h);
  Point3D q2(true, hs.GetRightPoint().GetX(), hs.GetRightPoint().GetY(), h);
  *l3d += q1;
  *l3d += q2; 
  l3d->EndBulkLoad();
  
  return h; 
  //////////////////////////////////////////////////////////////////////////

}

/*
create a line denoting the positio of a door. we record the relative position
according to the groom 

*/

void IndoorNav::GRoomDoorLine(Rectangle<3>* bbox3d_1, Rectangle<3>* bbox3d_2, 
                     Line* l1, Line* l2, Line* l3, 
                     const Rectangle<2>* groom_box1,
                     const Rectangle<2>* groom_box2,
                     Line3D* l3d, float h)
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
  
  
  
  l3d->StartBulkLoad();
  Point3D lq(true, bbox3d_1->MinD(0), bbox3d_1->MinD(1), h);
  Point3D rq(true, bbox3d_1->MaxD(0), bbox3d_1->MaxD(1), h);
  *l3d += lq;
  *l3d += rq; 

  l3d->EndBulkLoad(); 
}

/*
create the edges connecting two doors inside one room 

*/

void IndoorNav::CreateAdjDoor1(BTree* btree,int attr1, int attr2, 
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
        BuildPathEL(groom_oid, groom, tid_list, attr1, attr2, attr3, attr4);
      }
      /////////////////////////////////////////////////////////
      ////// the path inside a elevator: ST //////////////////////
      ///////////////////////////////////////////////////////////
      if(GetRoomEnum(groom_type) == ST){
        BuildPathST(groom_oid, groom, tid_list, attr1, attr2, attr3, attr4);

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

//  cout<<"BuildPathEL() tid_lise size "<<tid_list.size()<<endl; 
//  cout<<"low height "<<groom->GetLowHeight()<<endl; 
  ////////////////////use tid id to collect the door tuple/////////////
  
//  cout<<"tid list size "<<tid_list.size()<<endl; 
//  assert(tid_list.size() == 2);
  for(unsigned int i = 0;i < tid_list.size(); i++){
    Tuple* door_tuple1 = rel2->GetTuple(tid_list[i], false);
    float h1 = ((CcReal*)door_tuple1->GetAttribute(attr4))->GetRealval();
    
    for(unsigned int j = 0;j < tid_list.size();j++){
      if(tid_list[i] == tid_list[j])continue; 
      Tuple* door_tuple2 = rel2->GetTuple(tid_list[j], false);
      float h2 = ((CcReal*)door_tuple2->GetAttribute(attr4))->GetRealval();
      if(AlmostEqual(h1, h2))continue; //ignore the doors on the same level 

      Line* l = (Line*)door_tuple1->GetAttribute(attr2);
      assert(l->Size() == 2);
      HalfSegment hs;
      l->Get(0, hs);

      double x = (hs.GetLeftPoint().GetX() + hs.GetRightPoint().GetX())/2;
      double y = (hs.GetLeftPoint().GetY() + hs.GetRightPoint().GetY())/2;
      Point3D p1(true, x, y, h1);
      Point3D p2(true, x, y, h2);

      Line3D* l3d = new Line3D(0);
      l3d->StartBulkLoad();
      *l3d += p1;
      *l3d += p2; 
      l3d->EndBulkLoad();

  //////////////////////  result   ////////////////////////////////////////
      groom_oid_list.push_back(groom_oid);
      door_tid_list1.push_back(tid_list[i]);
      door_tid_list2.push_back(tid_list[j]);
      path_list.push_back(*l3d);
      delete l3d; 

//      cout<<"groom_oid "<<groom_oid<<"tid1 "<<tid_list[i]
//          <<" tid2 "<<tid_list[j]<<endl; 

      door_tuple2->DeleteIfAllowed(); 
    }
  
    door_tuple1->DeleteIfAllowed();
  }


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

//        return; 
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
//  cout<<" ST_ConnectOneFloor "<<endl; 
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

  if(l->Size() == 0)return; 
  
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
build a path inside a staircase and it connects from the second footstep to 
the last second footstep. 

*/
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
  
  sort(floors.begin(), floors.end(), Floor3DCompare1);
  
  ///////// from low height to high height ////////////////////
  for(unsigned int i = 0; i < floors.size();i++){
//    cout<<"height "<<floors[i].GetHeight()<<endl; 
    ///////////////////////////////////////////////////////////
    if(i == floors.size() - 1 || i == 0) continue; 
    ////////////////////////////////////////////////////////////
    
    Region* cur_r = const_cast<Region*>(floors[i].GetRegion());
    Region* r1 = const_cast<Region*>(floors[i - 1].GetRegion());
    Region* r2 = const_cast<Region*>(floors[i + 1].GetRegion());

//    cout<<"cur "<<*cur_r<<" r1 "<<*r1<<" r2 "<<*r2<<endl; 
    
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

//  cout<<"p1 "<<p1<<"h1 "<<h1<<endl;
//  cout<<"p2 "<<p2<<"h2 "<<h2<<endl; 

  vector<Floor3D> floors; 
  for(int i = 0;i < groom->Size();i++){
    float h;
    Region r(0);
    groom->Get(i, h, r);
    Floor3D floor(h, r);
    floors.push_back(floor); 
  }
  sort(floors.begin(), floors.end(), Floor3DCompare1);


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
        Point3D q1(true, p.GetX(), p.GetY(), middle_path[i].h);
        *l3d += q1;

        p = middle_path[i].to; 
        Point3D q2(true, p.GetX(), p.GetY(), middle_path[i].h);
        *l3d += q2;
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
        Point p = middle_path[i].to;
        if(i == middle_path.size() - 1){
          Point3D q(true, p.GetX(), p.GetY(), h1);
          *l3d += q;
        }
        Point3D q1(true, p.GetX(), p.GetY(), middle_path[i].h);
        *l3d += q1;

        p = middle_path[i].from; 
        Point3D q2(true, p.GetX(), p.GetY(), middle_path[i].h);
        *l3d += q2;
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




/*
create the edges connecting the same door but belong to two rooms 

*/

void IndoorNav::CreateAdjDoor2(R_Tree<3,TupleId>* rtree)
{
  SmiRecordId adr = rtree->RootRecordId();

  for(int i = 1;i <= rel1->GetNoTuples();i++){

    Tuple* door_tuple = rel1->GetTuple(i, false);
    int groom_oid = 
    ((CcInt*)door_tuple->GetAttribute(IndoorGraph::I_GROOM_OID1))->GetIntval(); 
    Line3D* l = (Line3D*)door_tuple->GetAttribute(IndoorGraph::I_DOOR_LOC_3D); 
    vector<TupleId> id_list; 
//    l->Print(); 
    ////// the number of neighbor should be 2 or 3 //////////////////
    DFTraverse(rtree, adr, i, l, id_list, groom_oid);

//    assert(id_list.size() > 0);


//    cout<<"door tid "<<i<<endl; 
//    cout<<"neighbor size "<<id_list.size()<<endl; 

    for(unsigned int j = 0;j < id_list.size();j++){
//      cout<<"neighbor "<<id_list[j]<<endl; 

      groom_oid_list.push_back(groom_oid);
      door_tid_list1.push_back(i);
      door_tid_list2.push_back(id_list[j]);
      Line3D* l3d = new Line3D(0); 
      path_list.push_back(*l3d); 
      delete l3d; 
    }


    door_tuple->DeleteIfAllowed(); 
  }

}

/*
traverse the RTree to find the same door but belongs to two rooms 

*/
void IndoorNav::DFTraverse(R_Tree<3,TupleId>* rtree, SmiRecordId adr, 
                           unsigned int id, Line3D* l, 
                           vector<TupleId>& id_list, int groom_oid)
{

  R_TreeNode<3,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  Rectangle<3> bbox3d_1 = l->BoundingBox(); 

  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<3,TupleId> e =
                 (R_TreeLeafEntry<3,TupleId>&)(*node)[j];
              Tuple* door_tuple = rel1->GetTuple(e.info,false);
              Line3D* l3d = 
                  (Line3D*)door_tuple->GetAttribute(IndoorGraph::I_DOOR_LOC_3D);

              if(l3d->BoundingBox().Intersects(bbox3d_1) && id != e.info){
                  if(*l == *l3d)id_list.push_back(e.info);
              }


/*              if(l3d->BoundingBox().Intersects(bbox3d_1)){
                 cout<<"l2 ";
                 l3d->Print();
                 if(*l == *l3d)id_list.push_back(e.info);
              }*/

              door_tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<3> e = (R_TreeInternalEntry<3>&)(*node)[j];
            if(l->BoundingBox().Intersects(e.box)){
                DFTraverse(rtree, e.pointer, id, l, id_list, groom_oid);
            }
      }
  }
  delete node;
}

/////////////////////////////////////////////////////////////////////////////
////////// shortest path inside a polygon (convex, concave, holes) /////////
////////////////////////////////////////////////////////////////////////////

string PointsTypeInfo =
  "(rel(tuple((v point)(neighbor1 point)(neighbor2 point)(regid int))))";
  
  
void ShortestPath_InRegion(Region* reg, Point* s, Point* d, Line* pResult)
{
  const double dist_delta = 0.001; 
  if(reg->Contains(*s) == false){
    cout<<"region "<<*reg<<"start point "<<*s<<" end point "<<*d<<endl; 
    cout<<"start point should be inside the region"<<endl;
    return;
  }
  if(reg->Contains(*d) == false){
    cout<<"region "<<*reg<<"start point "<<*s<<" end point "<<*d<<endl; 
    cout<<"end point should be inside the region"<<endl;
    return;
  }

  if(reg->NoComponents() > 1){
    cout<<"only one face is allowed"<<endl;
    return; 
  }
  if(s->Distance(*d) < dist_delta){
//    cout<<"start location equals to the end locaton"<<endl;
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
                  ps_list, seg_list); 
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
                     vector<HalfSegment>& seg_list)
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
  (door_loc3d line3d) (doorheight real))))";
  
string IndoorGraph::EdgeTypeInfo =
"(rel (tuple ((groom_oid int) (door_tid1 int) (door_tid2 int) (Path line3d))))";

string IndoorGraph::NodeBTreeTypeInfo =
"(btree (tuple ((Door door3d) (door_loc line) (groom_oid1 int) (groom_oid2 int)\
  (door_loc3d line3d) (doorheight real))) int)";


IndoorGraph::~IndoorGraph()
{
//  cout<<"~IndoorGraph()"<<endl;
  if(btree_node) delete btree_node; 
}

IndoorGraph::IndoorGraph()
{
//  cout<<"IndoorGraph::IndoorGraph()"<<endl;
}

IndoorGraph::IndoorGraph(ListExpr in_xValue,int in_iErrorPos,
                     ListExpr& inout_xErrorInfo,
                     bool& inout_bCorrect)
{
//  cout<<"IndoorGraph::IndoorGraph(ListExpr)"<<endl;
}

bool IndoorGraph::CheckIndoorGraph(ListExpr type, ListExpr& errorInfo)
{
//  cout<<"CheckIndoorGraph()"<<endl;
  return nl->IsEqual(type, "indoorgraph");
}

void IndoorGraph::CloseIndoorGraph(const ListExpr typeInfo, Word& w)
{
//  cout<<"CloseIndoorGraph()"<<endl;
  delete static_cast<IndoorGraph*> (w.addr);
  w.addr = NULL;
}

void IndoorGraph::DeleteIndoorGraph(const ListExpr typeInfo, Word& w)
{
//  cout<<"DeleteIndoorGraph()"<<endl;
  IndoorGraph* ig = (IndoorGraph*)w.addr;
  delete ig;
  w.addr = NULL;
}

Word IndoorGraph::CreateIndoorGraph(const ListExpr typeInfo)
{
//  cout<<"CreateIndoorGraph()"<<endl;
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
//  cout<<"InIndoorGraph()"<<endl;
  IndoorGraph* ig = new IndoorGraph(in_xValue, in_iErrorPos, inout_xErrorInfo,
                                inout_bCorrect);
  if(inout_bCorrect) return SetWord(ig);
  else{
    delete ig;
    return SetWord(Address(0));
  }
}


ListExpr IndoorGraph::OutIndoorGraph(ListExpr typeInfo, Word value)
{
//  cout<<"OutVisualGraph()"<<endl;
  IndoorGraph* ig = (IndoorGraph*)value.addr;
  return ig->Out(typeInfo);
}
/*
Output the indoor graph 

*/
ListExpr IndoorGraph::Out(ListExpr typeInfo)
{
//  cout<<"Out()"<<endl;
  ListExpr xNode = nl->TheEmptyList();
  ListExpr xLast = nl->TheEmptyList();
  ListExpr xNext = nl->TheEmptyList();

  bool bFirst = true;
  for(int i = 1;i <= edge_rel->GetNoTuples();i++){
      Tuple* edge_tuple = edge_rel->GetTuple(i, false);
      CcInt* groom_oid = (CcInt*)edge_tuple->GetAttribute(I_GROOM_OID);
      CcInt* oid1 = (CcInt*)edge_tuple->GetAttribute(I_DOOR_TID1);
      CcInt* oid2 = (CcInt*)edge_tuple->GetAttribute(I_DOOR_TID2);
      Line* connection = (Line*)edge_tuple->GetAttribute(I_PATH);

      ListExpr xline = OutLine3D(nl->TheEmptyList(),SetWord(connection));
      xNext = nl->FourElemList(nl->IntAtom(groom_oid->GetIntval()),
                               nl->IntAtom(oid1->GetIntval()),
                               nl->IntAtom(oid2->GetIntval()),
                               xline);
      if(bFirst){
        xNode = nl->OneElemList(xNext);
        xLast = xNode;
        bFirst = false;
      }else
          xLast = nl->Append(xLast,xNext);
      edge_tuple->DeleteIfAllowed();
  }
  return nl->TwoElemList(nl->IntAtom(g_id),xNode);

}

IndoorGraph::IndoorGraph(SmiRecord& in_xValueRecord, size_t& inout_iOffset,
const ListExpr in_xTypeInfo)
{
//   cout<<"IndoorGraph::IndoorGraph(SmiRecord)"<<endl;
   /***********************Read graph id********************************/
  in_xValueRecord.Read(&g_id,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);


  ListExpr xType;
  ListExpr xNumericType;
  /***********************Open relation for node*********************/
  nl->ReadFromString(NodeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  node_rel = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!node_rel) {
    return;
  }
  /***********************Open relation for edge*********************/
  nl->ReadFromString(EdgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  edge_rel = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!edge_rel) {
    node_rel->Delete();
    return;
  }

  ////////////////////adjaency list////////////////////////////////
   size_t bufsize = sizeof(FlobId) + sizeof(SmiSize) + 2*sizeof(int);
   SmiSize offset = 0;
   char* buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   assert(buf != NULL);
   adj_list.restoreHeader(buf,offset);
   free(buf);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   assert(buf != NULL);
   entry_adj_list.restoreHeader(buf,offset);
   inout_iOffset += bufsize;
   free(buf);

   nl->ReadFromString(NodeBTreeTypeInfo,xType);
   xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
   btree_node = BTree::Open(in_xValueRecord, inout_iOffset, xNumericType);
   if(!btree_node) {
     node_rel->Delete();
     edge_rel->Delete();
     return;
   }
   

}

IndoorGraph* IndoorGraph::Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo)
{

  return new IndoorGraph(valueRecord,offset,typeInfo);
}


bool IndoorGraph::OpenIndoorGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenIndoorGraph()"<<endl;
  value.addr = IndoorGraph::Open(valueRecord, offset, typeInfo);
  bool result = (value.addr != NULL);

  return result;
}


bool IndoorGraph::SaveIndoorGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveIndoorGraph()"<<endl;
  IndoorGraph* ig = (IndoorGraph*)value.addr;
  bool result = ig->Save(valueRecord, offset, typeInfo);

  return result;
}

/*
Save an indoor graph 

*/

bool IndoorGraph::Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
              const ListExpr in_xTypeInfo)
{

//  cout<<"Save()"<<endl;
  /********************Save graph id ****************************/
  in_xValueRecord.Write(&g_id,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);


  ListExpr xType;
  ListExpr xNumericType;
  /************************save node****************************/
  nl->ReadFromString(NodeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!node_rel->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;

  /************************save edge****************************/
  nl->ReadFromString(EdgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!edge_rel->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;


   SecondoCatalog *ctlg = SecondoSystem::GetCatalog();
   SmiRecordFile *rf = ctlg->GetFlobFile();
   adj_list.saveToFile(rf, adj_list);
   SmiSize offset = 0;
   size_t bufsize = adj_list.headerSize()+ 2*sizeof(int);
   char* buf = (char*) malloc(bufsize);
   adj_list.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   free(buf);

   entry_adj_list.saveToFile(rf, entry_adj_list);
   offset = 0;
   buf = (char*) malloc(bufsize);
   entry_adj_list.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf,bufsize, inout_iOffset);
   free(buf);
   inout_iOffset += bufsize;

   
   /////////////////////////////save btree on node ////////////////////
   nl->ReadFromString(NodeBTreeTypeInfo, xType);
   xNumericType = SecondoSystem::GetCatalog()->NumericType(xType); 
   if(!btree_node->Save(in_xValueRecord, inout_iOffset, xNumericType))
     return false; 
   /////////////////////////////////////////////////////////////////////
   
  return true;

}

/*
Load the Indoor graph 

*/

void IndoorGraph::Load(int id, Relation* r1, Relation* r2)
{
//  cout<<"IndoorGraph::Load()"<<endl;
  g_id = id;
  //////////////////node relation////////////////////

  ostringstream xNodePtrStream;
  xNodePtrStream<<(long)r1;
//  string strQuery = "(consume(sort(feed(" + NodeTypeInfo +
//                "(ptr " + xNodePtrStream.str() + ")))))";

  string strQuery = "(consume(feed(" + NodeTypeInfo +
                "(ptr " + xNodePtrStream.str() + "))))";

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  node_rel = (Relation*)xResult.addr;

  /////////////////edge relation/////////////////////
  ostringstream xEdgePtrStream;
  xEdgePtrStream<<(long)r2;
  strQuery = "(consume(sort(feed(" + EdgeTypeInfo +
                "(ptr " + xEdgePtrStream.str() + ")))))";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  edge_rel = (Relation*)xResult.addr;

  ////////////adjacency list ////////////////////////////////

  ostringstream xNodeOidPtrStream1;
  xNodeOidPtrStream1 << (long)edge_rel;
  strQuery = "(createbtree (" + EdgeTypeInfo +
             "(ptr " + xNodeOidPtrStream1.str() + "))" + "door_tid1)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  BTree* btree_node_oid1 = (BTree*)xResult.addr;


//  cout<<"b-tree on edge is finished....."<<endl;
  /////////////////////////////////////////////////////////////////////////
  /////////the adjacent list here is different from dual graph and 
  ///////// visibility graph. before we store the node id
  /////////now we store the edge id because the weight, path is stored
  ////////in the edge relation ////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  for(int i = 1;i <= node_rel->GetNoTuples();i++){
    CcInt* nodeid = new CcInt(true, i);
    BTreeIterator* btree_iter1 = btree_node_oid1->ExactMatch(nodeid);
    int start = adj_list.Size();
//    cout<<"start "<<start<<endl;
    while(btree_iter1->Next()){
      Tuple* edge_tuple = edge_rel->GetTuple(btree_iter1->GetId(), false);
//      int tid2 = ((CcInt*)edge_tuple->GetAttribute(I_DOOR_TID2))->GetIntval();

      adj_list.Append(edge_tuple->GetTupleId());//get the edge tuple id 
      edge_tuple->DeleteIfAllowed();
    }
    delete btree_iter1;

    int end = adj_list.Size();
    entry_adj_list.Append(ListEntry(start, end));
//    cout<<"end "<<end<<endl;
    delete nodeid;
  }

  delete btree_node_oid1;
  /////////////////////////////////////////////////////////////////////////
  /////////////////////////build a btree on node rel////////////////////////
  ostringstream xNodeOidPtrStream2;
  xNodeOidPtrStream2<< (long)node_rel;
  strQuery = "(createbtree (" + NodeTypeInfo +
             "(ptr " + xNodeOidPtrStream2.str() + "))" + "groom_oid1)";
//  cout<<strQuery<<endl; 
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  btree_node = (BTree*)xResult.addr;
  /////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////
  ///////////////////calculate the number of grooms////////////////////
  /////////////////////////////////////////////////////////////////////
  
/*  vector<int> groom_id_list; 
  for(int i = 1;i <= r2->GetNoTuples();i++){
    Tuple* groom_tuple = r2->GetTuple(i, false);
    int gid = 
    ((CcInt*)groom_tuple->GetAttribute(IndoorGraph::I_GROOM_OID))->GetIntval();
    groom_id_list.push_back(gid);
    groom_tuple->DeleteIfAllowed();
  }
  sort(groom_id_list.begin(), groom_id_list.end());
  vector<int>::iterator last;
  last = unique(groom_id_list.begin(), groom_id_list.end());
  num_of_grooms = 0;
  for(vector<int>::iterator iter = groom_id_list.begin(); iter != last;iter++){
    num_of_grooms++;
  }*/  


}

/*
get all adjacent nodes for a given node. indoor graph

*/

void IndoorNav::GetAdjNodeIG(int oid)
{
    vector<int> adj_list;
    ig->FindAdj(oid, adj_list);
//    cout<<"adj_list size "<<adj_list.size()<<endl; 
    for(unsigned int i = 0;i < adj_list.size();i++){
      Tuple* edge_tuple = ig->GetEdgeRel()->GetTuple(adj_list[i], false);
      int neighbor_id = 
      ((CcInt*)edge_tuple->GetAttribute(IndoorGraph::I_DOOR_TID2))->GetIntval();
      Line3D* path = (Line3D*)edge_tuple->GetAttribute(IndoorGraph::I_PATH);
      
      door_tid_list1.push_back(oid);
      door_tid_list2.push_back(neighbor_id); 
      path_list.push_back(*path); 
      edge_tuple->DeleteIfAllowed();
    }
}

/*
generate interesting indoor points 
do not include the position on the staircase and elevator 
it contains OR, CO, BR 

*/
void IndoorNav::GenerateIP1(int num)
{
  int no_rooms = rel1->GetNoTuples(); 
  struct timeval tval;
  struct timezone tzone;

  gettimeofday(&tval, &tzone);

  srand48(tval.tv_sec);//second 
//    srand48(tval.tv_usec);//Microseconds

  for(int i = 1;i <= num;){
    int room_oid; 
    room_oid = lrand48() % no_rooms + 1;

   /////////////////////////////////////////////////////////////////
   ////////////////////special generation//////////////////////////
   ///////////////////////////////////////////////////////////////
/*    if( i % 2 == 0)room_oid = 14;
    else 
      room_oid = 96;  */
    /////////////////////////////////////////////////////////////
    
    Tuple* room_tuple = rel1->GetTuple(room_oid, false);
    string type = ((CcString*)room_tuple->GetAttribute(I_Type))->GetValue();
    if(GetRoomEnum(type) == ST || GetRoomEnum(type) == EL){
        room_tuple->DeleteIfAllowed();
        continue; 
    }
    
    GRoom* groom = (GRoom*)room_tuple->GetAttribute(I_Room);
    Region* reg = new Region(0);
    groom->GetRegion(*reg); 
    
    BBox<2> bbox = reg->BoundingBox();
    int xx = (int)(bbox.MaxD(0) - bbox.MinD(0)) + 1;
    int yy = (int)(bbox.MaxD(1) - bbox.MinD(1)) + 1;

    Point p1;
    Point p2;
    bool inside = false;
    int count = 1;
    while(inside == false && count <= 100){
        //signed long integers, uniformly distributed over
        //the interval [-2(31), 2(31)]

        //non-negative, long integers, uniformly distributed over
        //the interval [0, 2(31)]
        int x = (lrand48() + 1)% (xx*100);
        int y = (lrand48() + 1)% (yy*100);

        double coord_x = x/100.0;
        double coord_y = y/100.0;

        p1.Set(coord_x, coord_y); //set back to relative position
        //lower the precision
        Modify_Point_3(p1);

        Coord x_cord = p1.GetX() + bbox.MinD(0);
        Coord y_cord = p1.GetY() + bbox.MinD(1);
        p2.Set(x_cord, y_cord); //absolute position 

        inside = p2.Inside(*reg);
        count++;
      }
      if(inside){
        float h = groom->GetLowHeight();////////////always on the lowest level 
        Loc loc(p1.GetX(), p1.GetY());
        GenLoc genl(room_oid, loc);
        Point3D q(true, p2.GetX(), p2.GetY(), h);
        genloc_list.push_back(genl);
        p3d_list.push_back(q); 
        i++;
      }

    delete reg; 
    room_tuple->DeleteIfAllowed(); 
  }

}


/*
generate interesting indoor moving objects.
the result is represented by mpoint3d 

*/
void IndoorNav::GenerateMO1(IndoorGraph* ig, BTree* btree, 
                            R_Tree<3,TupleId>* rtree, int num, 
                            Periods* peri, bool convert)
{
  GenerateIP1(num*2); 
  
  IndoorNav* indoor_nav = new IndoorNav(ig); 

  Interval<Instant> periods;
  if(peri->GetNoComponents() == 0){
    cout<<"not correct time periods"<<endl; 
    assert(false);
  }
  peri->Get(0, periods);

  double speed = GetMinimumDoorWidth(); 
//  cout<<"human speed "<<speed<<endl; 


  int count = 0; 
  for(unsigned int i = 0;i < genloc_list.size();i++){
    if(i < genloc_list.size() - 1){
      GenLoc loc1 = genloc_list[i];
      GenLoc loc2 = genloc_list[i + 1];

//      GenLoc loc1(144, Loc(4.65, 1.61));
//      GenLoc loc2(317, Loc(0.91, 2.0));

      if(loc1.GetOid() == loc2.GetOid()) continue; 

      cout<<"loc1 "<<loc1<<" loc2 "<<loc2<<endl;
      indoor_nav->ShortestPath_Length(&loc1, &loc2, rel1, btree);
//      cout<<indoor_nav->path_list[count].Length()<<endl;
      //////////////////////////////////////////////////////////////////
      Instant start_time = periods.start;
      start_time.ReadFrom(periods.start.ToDouble() + (i % 5)/(24.0*60.0*60.0));
      Line3D* l3d = &indoor_nav->path_list[count];
//      l3d->Print();
      if(l3d->Length() > 0.0){
        MPoint3D* mp3d = new MPoint3D(0); 
        mp3d->StartBulkLoad();
        for(int j = 0;j < l3d->Size();j++){
          if(j < l3d->Size() - 1){
            Point3D p1, p2;
            l3d->Get(j, p1);
            l3d->Get(j + 1, p2);
            AddUnitToMO(mp3d, p1, p2, start_time, speed);
          }
        }
        mp3d->EndBulkLoad(); 
        mo_list.push_back(*mp3d);
      ///////////////////////////////////////////////////////////////////
        if(convert)
          ToGenLoc(mp3d, rtree);
        delete mp3d; 
        count++; 
      }
      if(count == num)break; 

//      break; 
    }
  }

  delete indoor_nav; 

}

/*
get the minimum width of door 

*/
float IndoorNav::GetMinimumDoorWidth()
{
  float door_width = numeric_limits<float>::max(); //the speed for person 
  for(int i = 1;i <= rel1->GetNoTuples();i++){
      Tuple* groom_tuple = rel1->GetTuple(i, false);
      Line* door = (Line*)groom_tuple->GetAttribute(I_Door); 
      for(int j = 0;j < door->Size();j++){
        HalfSegment hs;
        door->Get(j, hs);
        if(!hs.IsLeftDomPoint())continue; 
        if(hs.Length() < door_width)door_width = hs.Length(); 
      }
      groom_tuple->DeleteIfAllowed();
  }
  return door_width; 
}

/*
add units to the result moving objects 

*/
void IndoorNav::AddUnitToMO(MPoint3D* mp3d, Point3D& p1, Point3D& p2, 
                    Instant& start_time, double speed)
{
  const double dist_delta = 0.01; 

  double d = p1.Distance(p2); 
  if(d < speed || AlmostEqual(d, speed) ||
    (AlmostEqual(p1.GetX(), p2.GetX()) && //for the movement inside an elevator 
     AlmostEqual(p1.GetY(), p2.GetY()))){ // if split, it can not find a groom 

    Interval<Instant> up_interval; 
    up_interval.start = start_time;
    Instant end = start_time;
    end.ReadFrom(start_time.ToDouble() + d/(24.0*60.0*60.0*speed));
    up_interval.end = end;

    up_interval.lc = true;
    up_interval.rc = false; 

    UPoint3D* unit = new UPoint3D(up_interval, p1, p2); 
    mp3d->Add(*unit); 
    delete unit;
    start_time = end; 
  }else{
    int no = (int)floor(d/speed); 
    double x = (p2.GetX() - p1.GetX())/no; 
    double y = (p2.GetY() - p1.GetY())/no;
    double z = (p2.GetZ() - p1.GetZ())/no; 
    for(int i = 0;i < no ;i++){
      Point3D q1(true, p1.GetX() + i*x, p1.GetY() + i*y, p1.GetZ() + i*z);
      Point3D q2(true, p1.GetX() + (i+1)*x, 
                       p1.GetY() + (i+1)*y, p1.GetZ() + (i+1)*z);
      double dist = q1.Distance(q2); 

      Interval<Instant> up_interval; 
      up_interval.start = start_time;
      Instant end = start_time;
      end.ReadFrom(start_time.ToDouble() + dist/(24.0*60.0*60.0*speed));
      up_interval.end = end; 

      up_interval.lc = true;
      up_interval.rc = false; 

      UPoint3D* unit = new UPoint3D(up_interval, q1,q2); 
      mp3d->Add(*unit); 
      delete unit; 
      start_time = end; 
      if(i == no - 1){
        Point3D q3 = q2;
        Point3D q4 = p2;
        double dist = q3.Distance(q4); 
        if(dist < dist_delta)continue; 

        up_interval.start = start_time;
        Instant end = start_time;
        end.ReadFrom(start_time.ToDouble() + dist/(24.0*60.0*60.0*speed));
        up_interval.end = end;
        up_interval.lc = true;
        up_interval.rc = false; 
        start_time = up_interval.end; 
//        cout<<"start "<<up_interval.start<<" end "<<up_interval.end<<endl; 
//        cout<<"t: "<<up_interval<<"p0: "<<q3<<"p1: "<<q4<<endl;

        UPoint3D* unit = new UPoint3D(up_interval, q3, q4); 

        mp3d->Add(*unit); 
        delete unit;
        start_time = end; 
      }
    }
  }
}

/*
convert a 3d point to genloc 

*/
void IndoorNav::ToGenLoc(MPoint3D* mp3d, R_Tree<3,TupleId>* rtree)
{
  GenMO* genmo = new GenMO(0); 
  genmo->StartBulkLoad(); 
  
  for(int i = 0;i < mp3d->GetNoComponents();i++){
      UPoint3D unit;
      mp3d->Get(i, unit); 
      GenLoc loc1, loc2;
      Get_GenLoc(unit.p0, unit.p1, loc1, loc2, rtree);

      UGenLoc* ugenloc = new UGenLoc(unit.timeInterval, loc1, loc2, 
                                     GetTM("Indoor")); 
      genmo->Add(*ugenloc);
      delete ugenloc; 
  }
  genmo->EndBulkLoad(); 
  genmo_list.push_back(*genmo);
  delete genmo; 
}

/*
the two genloc should have the same oid 

*/
void IndoorNav::Get_GenLoc(Point3D p1, Point3D p2,
                           GenLoc& loc1, GenLoc& loc2, R_Tree<3,TupleId>* rtree)
{
  SmiRecordId adr = rtree->RootRecordId();

  vector<int> tid_list1;
  vector<int> tid_list2; 
  DFTraverse(rtree, adr, p1, tid_list1);
  DFTraverse(rtree, adr, p1, tid_list2); 

// cout<<"p3d_1 "<<p1<<" p3d_2"<<p2<<endl;

  assert(tid_list1.size() > 0);
  assert(tid_list2.size() > 0);

/*  Tuple* groom_tuple = rel1->GetTuple(tid, false);
  GRoom* groom = (GRoom*)groom_tuple->GetAttribute(I_Room); 
  int oid = ((CcInt*)groom_tuple->GetAttribute(I_OID))->GetIntval(); 
  Rectangle<2> bbox = groom->BoundingBox(); 
  Loc loc(p.GetX() - bbox.MinD(0), p.GetY() - bbox.MinD(1)); 
  groom_tuple->DeleteIfAllowed();
  loc1.SetValue(oid, loc); */

  int tid1, tid2; 
  for(unsigned int i = 0;i < tid_list1.size();i++){
    tid1 = tid_list1[i];
    for(unsigned int j = 0;j < tid_list2.size();j++){
      tid2 = tid_list2[j]; 
      if(tid1 == tid2){
//        cout<<"find "<<endl; 
        break; 
      }  
    }
  }
  if(tid1 != tid2){
    cout<<"do not find the same groom for two locations"<<endl; 
    assert(false); 
  }
 /////////////////////////////////////////////////////////////////////////

  Tuple* groom_tuple = rel1->GetTuple(tid1, false);
  GRoom* groom = (GRoom*)groom_tuple->GetAttribute(I_Room); 
  string type = ((CcString*)groom_tuple->GetAttribute(I_Type))->GetValue(); 
  int oid = ((CcInt*)groom_tuple->GetAttribute(I_OID))->GetIntval(); 
  Rectangle<2> bbox = groom->BoundingBox(); 
  groom_tuple->DeleteIfAllowed();

  if(GetRoomEnum(type) == OR || GetRoomEnum(type) == BR || 
     GetRoomEnum(type) == CO || GetRoomEnum(type) == ST){
    Loc loc_1(p1.GetX() - bbox.MinD(0), p1.GetY() - bbox.MinD(1)); 
    Loc loc_2(p2.GetX() - bbox.MinD(0), p2.GetY() - bbox.MinD(1)); 

    loc1.SetValue(oid, loc_1);
    loc2.SetValue(oid, loc_2);
  }else if(GetRoomEnum(type) == EL){//move in an elevator,we record the height
    Loc loc_1(p1.GetZ(), -1); 
    Loc loc_2(p2.GetZ(), -1); 

    loc1.SetValue(oid, loc_1);
    loc2.SetValue(oid, loc_2);
  }else{
    cout<<"should not be here"<<endl;
    assert(false); 
  }
  

}


/*
check whether a 3d box contains a 3d point 

*/
bool BBoxContainPoint3D(Rectangle<3> bbox, Point3D& p)
{
  assert(bbox.IsDefined());
  assert(p.IsDefined()); 
  if( (bbox.MinD(0) < p.GetX() || AlmostEqual(bbox.MinD(0), p.GetX())) &&
      (bbox.MaxD(0) > p.GetX() || AlmostEqual(bbox.MaxD(0), p.GetX())) &&
      (bbox.MinD(1) < p.GetY() || AlmostEqual(bbox.MinD(1), p.GetY())) &&
      (bbox.MaxD(1) > p.GetY() || AlmostEqual(bbox.MaxD(1), p.GetY())) &&
      (bbox.MinD(2) < p.GetZ() || AlmostEqual(bbox.MinD(2), p.GetZ())) &&
      (bbox.MaxD(2) > p.GetZ() || AlmostEqual(bbox.MaxD(2), p.GetZ())) )
    return true; 
  else
    return false; 

}

/*
traverse the 3D rtree to find whether a groom contains the 3d point 

*/
void IndoorNav::DFTraverse(R_Tree<3,TupleId>* rtree, SmiRecordId adr, 
                           Point3D p, vector<int>& tid_list)
{
  R_TreeNode<3,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<3,TupleId> e =
                 (R_TreeLeafEntry<3,TupleId>&)(*node)[j];
              Tuple* groom_tuple = rel1->GetTuple(e.info,false);
              GRoom* groom = (GRoom*)groom_tuple->GetAttribute(I_Room);
              Rectangle<3> groom_box = groom->BoundingBox3D();
              if(BBoxContainPoint3D(groom_box, p)){
                    tid_list.push_back(e.info);
              }
              groom_tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<3> e =
                (R_TreeInternalEntry<3>&)(*node)[j];
            if(BBoxContainPoint3D(e.box, p)){
              DFTraverse(rtree, e.pointer, p, tid_list);
            }
      }
  }
  delete node;

}



ostream& operator<<(ostream& o, const IPath_elem& elem)
{
  o<<"tri_index "<< elem.tri_index<<" "
   <<"realweight "<<elem.real_w<<" "
   <<"weight "<<elem.weight<<endl; 
  return o;     
}

/*
return the shortest path with minimum length for indoor navigation 

*/
void IndoorNav::ShortestPath_Length(GenLoc* gloc1, GenLoc* gloc2, 
                                   Relation* rel, BTree* groom_btree)
{

  if(IsLocEqual(gloc1, gloc2, rel)){
    cout<<"the two locations are equal to each other"<<endl;
    Line3D* l3d = new Line3D(0);
    path_list.push_back(*l3d);
    delete l3d; 
    return; 
  }
  unsigned int groom_oid1 = gloc1->GetOid();
  unsigned int groom_oid2 = gloc2->GetOid(); 
//  cout<<"groom oid1 "<<groom_oid1<<" groom oid2 "<<groom_oid2<<endl; 
  if(groom_oid1 == groom_oid2){
    PathInOneRoom(gloc1, gloc2, rel, groom_btree); 
    return; 
  }


  Relation* node_rel = ig->GetNodeRel(); 

  ///////////////collect all doors in that room//////////////////////
  BTree* btree = ig->GetBTree(); 
  CcInt* search_id = new CcInt(true, groom_oid1);
  BTreeIterator* btree_iter = btree->ExactMatch(search_id);
  vector<int> tid_list; 
  while(btree_iter->Next()){
     Tuple* tuple = node_rel->GetTuple(btree_iter->GetId(), false);
     tid_list.push_back(tuple->GetTupleId());
     tuple->DeleteIfAllowed();
  }
  delete btree_iter;
  delete search_id;

  ////////////////////for each possible door, search the path/////////////
  vector<Line3D> candidate_path; 
  for(unsigned int i = 0;i < tid_list.size();i++){
//    cout<<"source door tid "<<tid_list[i]<<endl; 
    Tuple* door_tuple = node_rel->GetTuple(tid_list[i], false);
    unsigned int gri1 = 
     ((CcInt*)door_tuple->GetAttribute(IndoorGraph::I_GROOM_OID1))->GetIntval();
//    unsigned int gri2 = 
//   ((CcInt*)door_tuple->GetAttribute(IndoorGraph::I_GROOM_OID2))->GetIntval();

//    cout<<"gri1 "<<gri1<<"gri2 "<<gri2<<endl; 
    assert(gri1 == groom_oid1); 

    Line3D* l3d_start = new Line3D(0); 
    ConnectStartLoc(gloc1, tid_list[i], rel, groom_btree, l3d_start);


    Path_StartDoor_EndLoc(tid_list[i], gloc2, candidate_path, l3d_start, 
                          rel, groom_btree);
    door_tuple->DeleteIfAllowed(); 

    delete l3d_start; 

//    break; 
  }
  

  ///////////////////select the path with minimum length////////////////////
  if(candidate_path.size() > 0){
    double l = candidate_path[0].Length();
    int index = 0;
    for(unsigned int i = 1; i < candidate_path.size();i++){
//      cout<<"i "<<i<<" length "<<candidate_path[i].Length()<<endl; 
      if(candidate_path[i].Length() < l){
        l = candidate_path[i].Length();
        index = i; 
      }
    }

    path_list.push_back(candidate_path[index]); 

  }else
    cout<<"no path available"<<endl; 

}

/*
check whether the two locations are equal to each other 

*/
bool IndoorNav::IsLocEqual(GenLoc* gloc1, GenLoc* gloc2, Relation* rel)
{
  int groom_oid1 = gloc1->GetOid();
  int groom_oid2 = gloc2->GetOid(); 
    
  assert(1 <= groom_oid1 && groom_oid1 <= rel->GetNoTuples());
  assert(1 <= groom_oid2 && groom_oid2 <= rel->GetNoTuples());

  Tuple* groom_tuple1 = rel->GetTuple(groom_oid1, false);
  Tuple* groom_tuple2 = rel->GetTuple(groom_oid2, false);
  GRoom* groom1 = (GRoom*)groom_tuple1->GetAttribute(I_Room);
  GRoom* groom2 = (GRoom*)groom_tuple2->GetAttribute(I_Room);
  
  Region* reg1 = new Region(0);
  Region* reg2 = new Region(0);
  groom1->GetRegion(*reg1); 
  groom2->GetRegion(*reg2); 
  BBox<2> bbox1 = reg1->BoundingBox();
  BBox<2> bbox2 = reg2->BoundingBox();
  
  float h1 = groom1->GetLowHeight();
  float h2 = groom2->GetLowHeight();
  
  delete reg1; 
  delete reg2; 
  groom_tuple1->DeleteIfAllowed();
  groom_tuple2->DeleteIfAllowed(); 
  
  double x1 = gloc1->GetLoc().loc1 + bbox1.MinD(0);
  double y1 = gloc1->GetLoc().loc2 + bbox1.MinD(1);
  
  double x2 = gloc2->GetLoc().loc1 + bbox2.MinD(0);
  double y2 = gloc2->GetLoc().loc2 + bbox2.MinD(1);
  
  Point3D p1(true, x1, y1, h1);
  Point3D p2(true, x2, y2, h2); 
  
  const double dist_delta = 0.001; 
  if(p1.Distance(p2) < dist_delta) return true;
  
  return false; 
}

/*
two locations are in the same room 

*/
void IndoorNav::PathInOneRoom(GenLoc* gloc1, GenLoc* gloc2, 
                              Relation* rel, BTree* btree)
{
  int groom_oid = gloc1->GetOid();
  ///////////////find the room////////////////////////////////////////////
  CcInt* search_id = new CcInt(true, groom_oid);
  BTreeIterator* btree_iter = btree->ExactMatch(search_id);
  int groom_tid = 0; 
  while(btree_iter->Next()){
     groom_tid = btree_iter->GetId();
     break; 
  }
  delete btree_iter;
  delete search_id;
  //////////////////////////////////////////////////////////////////////////
  
  
//  Tuple* groom_tuple = rel->GetTuple(groom_oid, false);
  assert(1<= groom_tid && groom_tid <= rel->GetNoTuples()); 
  
  Tuple* groom_tuple = rel->GetTuple(groom_tid, false);
  string type = ((CcString*)groom_tuple->GetAttribute(I_Type))->GetValue(); 
  if(GetRoomEnum(type) == ST || GetRoomEnum(type) == EL){
      groom_tuple->DeleteIfAllowed();
      
      Line3D* l3d = new Line3D(0);
      path_list.push_back(*l3d); 
      delete l3d; 
      cout<<"inside the staircase and elevator. not interesting places"<<endl;
      cout<<"it should not arrive here"<<endl; 
      return; 
  }
  
  
  GRoom* groom = (GRoom*)groom_tuple->GetAttribute(I_Room);
  Region* reg = new Region(0);
  groom->GetRegion(*reg); 
  BBox<2> bbox = reg->BoundingBox();
  float h = groom->GetLowHeight();
  

  double x1 = gloc1->GetLoc().loc1 + bbox.MinD(0);
  double y1 = gloc1->GetLoc().loc2 + bbox.MinD(1);

  double x2 = gloc2->GetLoc().loc1 + bbox.MinD(0);
  double y2 = gloc2->GetLoc().loc2 + bbox.MinD(1);
  
  Point p1(true, x1, y1);
  Point p2(true, x2, y2);
  
//  cout<<"p1 "<<p1<<" p2"<<p2<<endl; 
  
  vector<MyHalfSegment> mhs;

  FindPathInRegion(groom, h, mhs, &p1, &p2); 
  
  delete reg; 
  groom_tuple->DeleteIfAllowed();
  
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
  
  path_list.push_back(*l3d); 
  delete l3d; 
}

/*
build the connection from start location to the door 

*/
void IndoorNav::ConnectStartLoc(GenLoc* gloc1, int door_tid, Relation* rel, 
                        BTree* btree, Line3D* l3d_start)
{

  int groom_oid = gloc1->GetOid();
  
    ///////////////find the room////////////////////////////////////////////
  CcInt* search_id = new CcInt(true, groom_oid);
  BTreeIterator* btree_iter = btree->ExactMatch(search_id);
  int groom_tid = 0; 
  while(btree_iter->Next()){
     groom_tid = btree_iter->GetId();
     break; 
  }
  delete btree_iter;
  delete search_id;
  //////////////////////////////////////////////////////////////////////////
//  cout<<"groom tid "<<groom_tid<<endl; 
  
  assert(1<= groom_tid && groom_tid <= rel->GetNoTuples()); 
  
  Tuple* groom_tuple = rel->GetTuple(groom_tid, false);
  
  string type = ((CcString*)groom_tuple->GetAttribute(I_Type))->GetValue(); 
  if(GetRoomEnum(type) == ST || GetRoomEnum(type) == EL){
      groom_tuple->DeleteIfAllowed();
      cout<<"inside the staircase and elevator. not interesting places"<<endl;
      cout<<"it should not arrive here"<<endl; 
      return; 
  }
  
  
  GRoom* groom = (GRoom*)groom_tuple->GetAttribute(I_Room);
  Region* reg = new Region(0);
  groom->GetRegion(*reg); 
  BBox<2> bbox = reg->BoundingBox();
  float h = groom->GetLowHeight();
  double x1 = gloc1->GetLoc().loc1 + bbox.MinD(0);
  double y1 = gloc1->GetLoc().loc2 + bbox.MinD(1);
  Point p1(true, x1, y1);

  
  Tuple* door_tuple = ig->GetNodeRel()->GetTuple(door_tid, false);
  Line* l = (Line*)door_tuple->GetAttribute(IndoorGraph::I_DOOR_LOC); 
  assert(l->Size() == 2);
  HalfSegment hs;
  l->Get(0, hs);
  double x2 = (hs.GetLeftPoint().GetX() + hs.GetRightPoint().GetX())/2;
  double y2 = (hs.GetLeftPoint().GetY() + hs.GetRightPoint().GetY())/2;
  door_tuple->DeleteIfAllowed(); 
  Point p2(true, x2, y2); 

//  cout<<"p1 "<<p1<<" p2"<<p2<<endl; 

  vector<MyHalfSegment> mhs;

  FindPathInRegion(groom, h, mhs, &p1, &p2); 
  
  delete reg; 
  groom_tuple->DeleteIfAllowed();
  
  ///////////////// conver to 3D line //////////////////////////////
  
  l3d_start->StartBulkLoad();
  
  for(unsigned int i = 0;i < mhs.size();i++){
    Point p = mhs[i].from; 
    Point3D q(true, p.GetX(), p.GetY(), h);
    *l3d_start += q;
    if(i == mhs.size() - 1){
      Point p1 = mhs[i].to; 
      Point3D q1(true, p1.GetX(), p1.GetY(), h);
      *l3d_start += q1;
    }
  }
  l3d_start->EndBulkLoad();
  
//  cout<<"start length "<<l3d_start->Length()<<endl; 
}

/*
find the path from the door (id) to the destination 

*/
void IndoorNav::Path_StartDoor_EndLoc(int id, GenLoc* gloc2, 
                                   vector<Line3D>& candidate_path, 
                                   Line3D* l3d_s, Relation* rel,
                                   BTree* groom_btree)
{

  unsigned int groom_oid2 = gloc2->GetOid(); 
  Relation* node_rel = ig->GetNodeRel(); 
  
  ///////////////collect all doors in that room//////////////////////
  CcInt* search_id = new CcInt(true, groom_oid2);
  BTree* btree = ig->GetBTree();
  BTreeIterator* btree_iter = btree->ExactMatch(search_id);
  vector<int> tid_list; 
  while(btree_iter->Next()){
     Tuple* tuple = node_rel->GetTuple(btree_iter->GetId(), false);
     tid_list.push_back(tuple->GetTupleId());
     tuple->DeleteIfAllowed();
  }
  delete btree_iter;
  delete search_id;

  
  for(unsigned int i = 0;i < tid_list.size();i++){
//     cout<<"dest door tid "<<tid_list[i]<<endl; 

    Tuple* door_tuple = node_rel->GetTuple(tid_list[i], false);
    unsigned int gri1 = 
     ((CcInt*)door_tuple->GetAttribute(IndoorGraph::I_GROOM_OID1))->GetIntval();

//    unsigned int gri2 = 
//   ((CcInt*)door_tuple->GetAttribute(IndoorGraph::I_GROOM_OID2))->GetIntval();
 
//    cout<<"gri1 "<<gri1<<"gri2 "<<gri2<<endl; 

    Line3D* l3d_end = new Line3D(0);
    ConnectEndLoc(gloc2, tid_list[i], rel, groom_btree, l3d_end); 
   
    assert(gri1 == groom_oid2); 
    IndoorShortestPath(id, tid_list[i], candidate_path, l3d_s, l3d_end);
    door_tuple->DeleteIfAllowed(); 

    delete l3d_end; 
//    break; 
  }

}

/*
connect the end location to the path 

*/
void IndoorNav::ConnectEndLoc(GenLoc* gloc, int door_tid, Relation* rel, 
                        BTree* btree, Line3D* l3d_end)
{
  int groom_oid = gloc->GetOid();

   ///////////////find the room////////////////////////////////////////////
  CcInt* search_id = new CcInt(true, groom_oid);
  BTreeIterator* btree_iter = btree->ExactMatch(search_id);
  int groom_tid = 0; 
  while(btree_iter->Next()){
     groom_tid = btree_iter->GetId();
     break; 
  }
  delete btree_iter;
  delete search_id;
  //////////////////////////////////////////////////////////////////////////
//  cout<<"groom tid "<<groom_tid<<endl; 
  
  assert(1<= groom_tid && groom_tid <= rel->GetNoTuples()); 
  Tuple* groom_tuple = rel->GetTuple(groom_tid, false);

  string type = ((CcString*)groom_tuple->GetAttribute(I_Type))->GetValue(); 
  if(GetRoomEnum(type) == ST || GetRoomEnum(type) == EL){
      groom_tuple->DeleteIfAllowed();
      cout<<"inside the staircase and elevator. not interesting places"<<endl;
      cout<<"it should not arrive here"<<endl; 
      return; 
  }
  
  
  GRoom* groom = (GRoom*)groom_tuple->GetAttribute(I_Room);
  Region* reg = new Region(0);
  groom->GetRegion(*reg); 
  BBox<2> bbox = reg->BoundingBox();
  float h = groom->GetLowHeight();
  double x1 = gloc->GetLoc().loc1 + bbox.MinD(0);
  double y1 = gloc->GetLoc().loc2 + bbox.MinD(1);
  Point p1(true, x1, y1);

  
  Tuple* door_tuple = ig->GetNodeRel()->GetTuple(door_tid, false);
  Line* l = (Line*)door_tuple->GetAttribute(IndoorGraph::I_DOOR_LOC); 
  assert(l->Size() == 2);
  HalfSegment hs;
  l->Get(0, hs);
  double x2 = (hs.GetLeftPoint().GetX() + hs.GetRightPoint().GetX())/2;
  double y2 = (hs.GetLeftPoint().GetY() + hs.GetRightPoint().GetY())/2;
  door_tuple->DeleteIfAllowed(); 
  Point p2(true, x2, y2); 

//  cout<<"p1 "<<p1<<" p2"<<p2<<endl; 

  vector<MyHalfSegment> mhs;

  FindPathInRegion(groom, h, mhs, &p2, &p1); 
  
  delete reg; 
  groom_tuple->DeleteIfAllowed();
  
  ///////////////// conver to 3D line //////////////////////////////
  
  l3d_end->StartBulkLoad();
  
  for(unsigned int i = 0;i < mhs.size();i++){
    Point p = mhs[i].from; 
    Point3D q(true, p.GetX(), p.GetY(), h);
    *l3d_end += q;
    if(i == mhs.size() - 1){
      Point p1 = mhs[i].to; 
      Point3D q1(true, p1.GetX(), p1.GetY(), h);
      *l3d_end += q1;
    }
  }
  l3d_end->EndBulkLoad();
//  cout<<"end length "<<l3d_end->Length()<<endl; 
  
}

/*
find the path from one door (id1) to another (id2) 

*/
bool IndoorNav::MiddlePoint(Line3D* l, Point3D& p)
{
  if(l->Size() != 2) return false; 
  Point3D q1, q2;
  l->Get(0, q1);
  l->Get(1, q2);
  
  double x = (q1.GetX() + q2.GetX())/2;
  double y = (q1.GetY() + q2.GetY())/2;
  double z = (q1.GetZ() + q2.GetZ())/2; 
  
  Point3D result(true, x, y, z);
  p = result; 
  return true; 
}

/*
compute the shortest path from one door to another door 

*/
void IndoorNav::IndoorShortestPath(int id1, int id2,
                                   vector<Line3D>& candidate_path, 
                                    Line3D* l3d_s, Line3D* l3d_d)
{

//  cout<<"IndoorShortestPath "<<"doorid1 "<<id1<<" doorid2 "<<id2<<endl; 
  if(id1 == id2){
    cout<<"two doors are equal"<<endl;
//    assert(false); 
    return; 
  }
  
  Relation* node_rel = ig->GetNodeRel(); 

  Tuple* door_tuple1 = node_rel->GetTuple(id1, false);
  Tuple* door_tuple2 = node_rel->GetTuple(id2, false);
  Line3D* l3d1 = (Line3D*)door_tuple1->GetAttribute(IndoorGraph::I_DOOR_LOC_3D);
  Line3D* l3d2 = (Line3D*)door_tuple2->GetAttribute(IndoorGraph::I_DOOR_LOC_3D);
  
  Point3D start_p, end_p;
  
  if(MiddlePoint(l3d1, start_p) == false || 
     MiddlePoint(l3d2, end_p) == false){
    cout<<"incorrect door "<<endl;
    assert(false);
  }

//  start_p.Print(); 
//  end_p.Print(); 


  door_tuple1->DeleteIfAllowed(); 
  door_tuple2->DeleteIfAllowed(); 
  
  priority_queue<IPath_elem> path_queue;
  vector<IPath_elem> expand_queue;
  
  vector<bool> visit_flag1;////////////door visit 
  for(int i = 1; i <= node_rel->GetNoTuples();i++)
    visit_flag1.push_back(false);


//  ofstream output("debug.txt");
  ///////////  initialize the queue //////////////////////////////
  InitializeQueue(id1, &start_p, &end_p, path_queue, expand_queue);
 ////////////////////////////////////////////////////////////////
  bool find = false;
  IPath_elem dest;//////////destination
  while(path_queue.empty() == false){
    IPath_elem top = path_queue.top();
    path_queue.pop();

    if(visit_flag1[top.tri_index - 1])continue; 

//    top.Print();
//    output<<top<<endl; 

    if(top.tri_index == id2){
//       cout<<"find the shortest path"<<endl;
       find = true;
       dest = top;
       break;
    }
    
    
    Tuple* door_tuple = node_rel->GetTuple(top.tri_index, false);
    /////////////////get the position of the door////////////////////////////
    Point3D q;
    Line3D* door_loc = 
      (Line3D*)door_tuple->GetAttribute(IndoorGraph::I_DOOR_LOC_3D); 
    assert(MiddlePoint(door_loc, q));
//    int groom_oid1 = 
//   ((CcInt*)door_tuple->GetAttribute(IndoorGraph::I_GROOM_OID1))->GetIntval();
//    int groom_oid2 = 
//   ((CcInt*)door_tuple->GetAttribute(IndoorGraph::I_GROOM_OID2))->GetIntval();

    door_tuple->DeleteIfAllowed(); 

//    output<<"top door goid1 "<<groom_oid1<<" goid2 "<<groom_oid2<<endl;

   ////////find its adjacecy element, and push them into queue and path//////
    vector<int> adj_list;
    ig->FindAdj(top.tri_index, adj_list);

    /////////////////////////////////////////////////////////////
    int pos_expand_path = top.cur_index;

    for(unsigned int i = 0;i < adj_list.size();i++){

      Tuple* edge_tuple = ig->GetEdgeRel()->GetTuple(adj_list[i], false);
      int neighbor_id = 
      ((CcInt*)edge_tuple->GetAttribute(IndoorGraph::I_DOOR_TID2))->GetIntval();
      Line3D* path = (Line3D*)edge_tuple->GetAttribute(IndoorGraph::I_PATH);

//     int groom_oid = 
//    ((CcInt*)edge_tuple->GetAttribute(IndoorGraph::I_GROOM_OID))->GetIntval();
//      output<<"edge groom_oid "<<groom_oid<<endl; 
//      output<<"neighbor_ id "<<neighbor_id<<endl;

      if(visit_flag1[neighbor_id - 1]){
//        output<<"door visit already"<<endl; 
        edge_tuple->DeleteIfAllowed();
        continue; 
      }

      Tuple* door_tuple1 = node_rel->GetTuple(neighbor_id, false);
//      unsigned int groom_id1 = 
//  ((CcInt*)door_tuple1->GetAttribute(IndoorGraph::I_GROOM_OID1))->GetIntval();
//      unsigned int groom_id2 = 
//  ((CcInt*)door_tuple1->GetAttribute(IndoorGraph::I_GROOM_OID2))->GetIntval();

//     output<<"neighbor goid1 "<<groom_id1<<" goid2 "<<groom_id2<<endl; 

//      output<<"available"<<endl; 
//      cout<<"groom oid "<<groom_oid<<" neighbor door tid "<<neighbor_id<<endl;

     Line3D* l = (Line3D*)door_tuple1->GetAttribute(IndoorGraph::I_DOOR_LOC_3D);
     Point3D p;

      assert(MiddlePoint(l, p));//get the point of next door 
      door_tuple1->DeleteIfAllowed(); 


      int cur_size = expand_queue.size();

      double w = top.real_w + path->Length(); 
      double hw = p.Distance(end_p);
      IPath_elem elem(pos_expand_path, cur_size, 
                      neighbor_id, w + hw, w, *path);
      path_queue.push(elem);
      expand_queue.push_back(elem); 

      edge_tuple->DeleteIfAllowed();
    }

    visit_flag1[top.tri_index - 1] = true; 
//    output<<"pop door tid "<<top.tri_index<<" groom_id "<<groom_id<<endl; 

  }

  ////////////////construct the result//////////////////////////////
  if(find){
    
    vector<Point3D> ps_list; 
    while(dest.prev_index != -1){
//      cout<<"sub path "<<endl; 
//      dest.Print();
      if(dest.path.Size() > 0){
          if(ps_list.size() == 0){
              for(int i = dest.path.Size() - 1;i >= 0;i--){
                  Point3D q;
                  dest.path.Get(i, q);
                  ps_list.push_back(q);
              }
          }else{
              for(int i = dest.path.Size() - 2;i >= 0;i--){
                  Point3D q;
                  dest.path.Get(i, q);
                  ps_list.push_back(q);
              }
          }
      }
      dest = expand_queue[dest.prev_index];
    }
//    cout<<"sub path"<<endl; 
//    dest.path.Print();
    if(dest.path.Size() > 0){
      for(int i = dest.path.Size() - 2;i >= 0;i--){
        Point3D q;
        dest.path.Get(i, q);
        ps_list.push_back(q);
      }
    }

    Line3D* l3d = new Line3D(0);
    l3d->StartBulkLoad();
    //////////////////connect to the start point////////////////////
    for(int i = 0;i < l3d_s->Size() - 1;i++){
      Point3D q;
      l3d_s->Get(i, q);
      *l3d += q; 
    }
    ///////////////////////////////////////////////////////////////

    for(int i = ps_list.size() - 1;i >= 0; i--){
      *l3d += ps_list[i];
    }
    ////////////////////connect to the end point/////////////////////
    for(int i = 1;i < l3d_d->Size();i++){
      Point3D q;
      l3d_d->Get(i, q);
      *l3d += q; 
    }

    l3d->EndBulkLoad(); 
    candidate_path.push_back(*l3d);
    delete l3d; 
  }
  
}

/*
Initialize the queue, put the start door into the queue  

*/
void IndoorNav::InitializeQueue(int id, Point3D* start_p, 
                                Point3D* end_p, 
                        priority_queue<IPath_elem>& path_queue, 
                        vector<IPath_elem>& expand_queue)
{
//    cout<<"InitializeQueue "<<endl; 

    int cur_size = expand_queue.size();

    double w = 0.0; 
    double hw = start_p->Distance(*end_p);
    Line3D* l3d = new Line3D(0);
    IPath_elem elem(-1, cur_size, id, w + hw, w, *l3d);
    path_queue.push(elem);
    expand_queue.push_back(elem); 
    delete l3d; 
}

/////////////////////////////////////////////////////////////////////////////
//////////////////////////minimum number of rooms////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void IndoorNav::ShortestPath_Room(GenLoc* gloc1, GenLoc* gloc2, 
                            Relation* rel, BTree* groom_btree)
{

  if(IsLocEqual(gloc1, gloc2, rel)){
    cout<<"the two locations are equal to each other"<<endl;
  
      CcInt* search_id = new CcInt(true, gloc1->GetOid());
      BTreeIterator* btree_iter = groom_btree->ExactMatch(search_id);
      int groom_tid = 0; 
      while(btree_iter->Next()){
        groom_tid = btree_iter->GetId();
        break; 
      }
      delete btree_iter;
      delete search_id;
      assert(1 <= groom_tid && groom_tid <= rel->GetNoTuples());
      Tuple* groom_tuple = rel->GetTuple(groom_tid, false);
      GRoom* groom = (GRoom*)groom_tuple->GetAttribute(I_Room); 
      
      groom_oid_list.push_back(gloc1->GetOid());
      room_list.push_back(*groom);
      groom_tuple->DeleteIfAllowed();
  
      return; 
  }
  unsigned int groom_oid1 = gloc1->GetOid();
  unsigned int groom_oid2 = gloc2->GetOid(); 
//  cout<<"groom oid1 "<<groom_oid1<<" groom oid2 "<<groom_oid2<<endl; 
  if(groom_oid1 == groom_oid2){

      CcInt* search_id = new CcInt(true, groom_oid1);
      BTreeIterator* btree_iter = groom_btree->ExactMatch(search_id);
      int groom_tid = 0; 
      while(btree_iter->Next()){
        groom_tid = btree_iter->GetId();
        break; 
      }
      delete btree_iter;
      delete search_id;
      assert(1 <= groom_tid && groom_tid <= rel->GetNoTuples());
      Tuple* groom_tuple = rel->GetTuple(groom_tid, false);
      GRoom* groom = (GRoom*)groom_tuple->GetAttribute(I_Room); 

      groom_oid_list.push_back(groom_oid1);
      room_list.push_back(*groom);
      groom_tuple->DeleteIfAllowed();
      return;
  }


  Relation* node_rel = ig->GetNodeRel(); 

  ///////////////collect all doors in that room//////////////////////
  BTree* btree = ig->GetBTree(); 
  CcInt* search_id = new CcInt(true, groom_oid1);
  BTreeIterator* btree_iter = btree->ExactMatch(search_id);
  vector<int> tid_list; 
  while(btree_iter->Next()){
     Tuple* tuple = node_rel->GetTuple(btree_iter->GetId(), false);
     tid_list.push_back(tuple->GetTupleId());
     tuple->DeleteIfAllowed();
  }
  delete btree_iter;
  delete search_id;

  ////////////////////for each possible door, search the path/////////////
  vector< vector<TupleId> > candidate_path; 
  int start_groom_oid = gloc1->GetOid();

  int end_groom_oid = gloc2->GetOid(); 

  for(unsigned int i = 0;i < tid_list.size();i++){
//    cout<<"source door tid "<<tid_list[i]<<endl; 
    Tuple* door_tuple = node_rel->GetTuple(tid_list[i], false);
    unsigned int gri1 = 
     ((CcInt*)door_tuple->GetAttribute(IndoorGraph::I_GROOM_OID1))->GetIntval();
    assert(gri1 == groom_oid1); 

    Path_StartDoor_EndLoc_Room(tid_list[i], gloc2, candidate_path, 
                               start_groom_oid, end_groom_oid);

    door_tuple->DeleteIfAllowed(); 

  }
  

  ///////////////////select the path with minimum length////////////////////
  if(candidate_path.size() > 0){
    unsigned int l = candidate_path[0].size();
    int index = 0;
    for(unsigned int i = 1; i < candidate_path.size();i++){
//      cout<<"i "<<i<<" length "<<candidate_path[i].Length()<<endl; 
      if(candidate_path[i].size() < l){
        l = candidate_path[i].size();
        index = i; 
      }
    }

    for(unsigned int i = 0;i < candidate_path[index].size();i++){

      Tuple* groom_tuple = rel->GetTuple(candidate_path[index][i],false);
      
      groom_oid_list.push_back(candidate_path[index][i]); 

      GRoom* groom = (GRoom*)groom_tuple->GetAttribute(I_Room);
      room_list.push_back(*groom); 
      groom_tuple->DeleteIfAllowed(); 
    }

  }else
    cout<<"no path available"<<endl; 

}

/*
find the path from the door (id) to the destination 

*/
void IndoorNav::Path_StartDoor_EndLoc_Room(int id, GenLoc* gloc2, 
                       vector< vector<TupleId> >& candidate_path, 
                       int start_groom_oid, int end_groom_oid)
{
  unsigned int groom_oid2 = gloc2->GetOid(); 
  Relation* node_rel = ig->GetNodeRel(); 

  ///////////////collect all doors in that room//////////////////////
  CcInt* search_id = new CcInt(true, groom_oid2);
  BTree* btree = ig->GetBTree();
  BTreeIterator* btree_iter = btree->ExactMatch(search_id);
  vector<int> tid_list; 
  while(btree_iter->Next()){
     Tuple* tuple = node_rel->GetTuple(btree_iter->GetId(), false);
     tid_list.push_back(tuple->GetTupleId());
     tuple->DeleteIfAllowed();
  }
  delete btree_iter;
  delete search_id;

  for(unsigned int i = 0;i < tid_list.size();i++){
//     cout<<"dest door tid "<<tid_list[i]<<endl; 

    Tuple* door_tuple = node_rel->GetTuple(tid_list[i], false);
    unsigned int gri1 = 
     ((CcInt*)door_tuple->GetAttribute(IndoorGraph::I_GROOM_OID1))->GetIntval();

//    unsigned int gri2 = 
//   ((CcInt*)door_tuple->GetAttribute(IndoorGraph::I_GROOM_OID2))->GetIntval();
 
//    cout<<"gri1 "<<gri1<<"gri2 "<<gri2<<endl; 


    assert(gri1 == groom_oid2); 
    IndoorShortestPath_Room(id, tid_list[i], candidate_path,
                            start_groom_oid, end_groom_oid);
    door_tuple->DeleteIfAllowed(); 

  }

}


/*
shortest path from one door to another door, decided by the number of rooms 

*/

void IndoorNav::IndoorShortestPath_Room(int id1, int id2,
                                   vector<vector<TupleId> >& candidate_path,
                                   int s_room_oid, int e_room_oid)
{
//  cout<<"s_room_oid "<<s_room_oid<<" e_room_oid "<<e_room_oid<<endl; 

  if(id1 == id2){
    cout<<"two doors are equal"<<endl;
//    assert(false); 
    return; 
  }
  
  Relation* node_rel = ig->GetNodeRel(); 

  Tuple* door_tuple1 = node_rel->GetTuple(id1, false);
  Tuple* door_tuple2 = node_rel->GetTuple(id2, false);
  Line3D* l3d1 = (Line3D*)door_tuple1->GetAttribute(IndoorGraph::I_DOOR_LOC_3D);
  Line3D* l3d2 = (Line3D*)door_tuple2->GetAttribute(IndoorGraph::I_DOOR_LOC_3D);
  
  Point3D start_p, end_p;
  
  if(MiddlePoint(l3d1, start_p) == false || 
     MiddlePoint(l3d2, end_p) == false){
    cout<<"incorrect door "<<endl;
    assert(false);
  }


  door_tuple1->DeleteIfAllowed(); 
  door_tuple2->DeleteIfAllowed(); 
  
  priority_queue<IPath_elem> path_queue;
  vector<IPath_elem> expand_queue;
  
  vector<bool> visit_flag1;////////////door visit 
  for(int i = 1; i <= node_rel->GetNoTuples();i++)
    visit_flag1.push_back(false);


//  ofstream output("debug.txt");
  ///////////  initialize the queue //////////////////////////////
  int cur_size = expand_queue.size();

  double w = 0.0; 
//  double hw = start_p.Distance(end_p);
  double hw = 0.0; 
  Line3D* l3d = new Line3D(0);
  IPath_elem elem(-1, cur_size, id1, w + hw, w, *l3d, s_room_oid);
  path_queue.push(elem);
  expand_queue.push_back(elem); 
  delete l3d; 
 ////////////////////////////////////////////////////////////////
  bool find = false;
  IPath_elem dest;//////////destination
  while(path_queue.empty() == false){
    IPath_elem top = path_queue.top();
    path_queue.pop();

    if(visit_flag1[top.tri_index - 1])continue; 

    if(top.tri_index == id2){
//       cout<<"find the shortest path"<<endl;
       find = true;
       dest = top;
       break;
    }
    
    
    Tuple* door_tuple = node_rel->GetTuple(top.tri_index, false);
    /////////////////get the position of the door////////////////////////////
    Point3D q;
    Line3D* door_loc = 
      (Line3D*)door_tuple->GetAttribute(IndoorGraph::I_DOOR_LOC_3D); 
    assert(MiddlePoint(door_loc, q));

    door_tuple->DeleteIfAllowed(); 


   ////////find its adjacecy element, and push them into queue and path//////
    vector<int> adj_list;
    ig->FindAdj(top.tri_index, adj_list);

    /////////////////////////////////////////////////////////////
    int pos_expand_path = top.cur_index;

    for(unsigned int i = 0;i < adj_list.size();i++){

      Tuple* edge_tuple = ig->GetEdgeRel()->GetTuple(adj_list[i], false);
      int neighbor_id = 
      ((CcInt*)edge_tuple->GetAttribute(IndoorGraph::I_DOOR_TID2))->GetIntval();
      Line3D* path = (Line3D*)edge_tuple->GetAttribute(IndoorGraph::I_PATH);
      int groom_oid = 
      ((CcInt*)edge_tuple->GetAttribute(IndoorGraph::I_GROOM_OID))->GetIntval();

      if(visit_flag1[neighbor_id - 1]){
        edge_tuple->DeleteIfAllowed();
        continue; 
      }

     Tuple* door_tuple1 = node_rel->GetTuple(neighbor_id, false);

     Line3D* l = (Line3D*)door_tuple1->GetAttribute(IndoorGraph::I_DOOR_LOC_3D);
     Point3D p;

      assert(MiddlePoint(l, p));//get the point of next door 
      door_tuple1->DeleteIfAllowed(); 


      int cur_size = expand_queue.size();

//      double w = top.real_w + path->Length(); 
//      double hw = p.Distance(end_p);
      double w = top.real_w; 
      if(path->Length() > 0.0) w++; //one more room 
      double hw = 0.0; 
      
      IPath_elem elem(pos_expand_path, cur_size, 
                      neighbor_id, w + hw, w, *path, groom_oid);
      path_queue.push(elem);
      expand_queue.push_back(elem); 

      edge_tuple->DeleteIfAllowed();
    }

    visit_flag1[top.tri_index - 1] = true; 

  }

  ////////////////construct the result//////////////////////////////
  if(find){

    vector<int> groom_oid_list; 
    groom_oid_list.push_back(e_room_oid); 
    while(dest.prev_index != -1){
//      cout<<"sub path "<<endl; 
//      dest.Print();
//        cout<<"groom oid "<<dest.groom_oid<<endl; 
      if(dest.path.Size() > 0){
        int oid = groom_oid_list[groom_oid_list.size() - 1]; 
        if(oid != dest.groom_oid)
          groom_oid_list.push_back(dest.groom_oid); 
      }
      dest = expand_queue[dest.prev_index];
    }
//    cout<<"sub path"<<endl; 
//    dest.path.Print();
    
    groom_oid_list.push_back(dest.groom_oid); 

    vector<TupleId> temp_list; 
    for(int i = groom_oid_list.size() - 1;i >= 0;i--){
//      cout<<"groom_oid "<<groom_oid_list[i]<<endl; 
      temp_list.push_back(groom_oid_list[i]);
    }  
    candidate_path.push_back(temp_list); 
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////// minimum travelling time////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void IndoorNav::ShortestPath_Time(GenLoc* gloc1, GenLoc* gloc2, 
                            Relation* rel, BTree* groom_btree)
{
    if(IsLocEqual(gloc1, gloc2, rel)){
      cout<<"the two locations are equal to each other"<<endl;
      Line3D* l3d = new Line3D(0);
      path_list.push_back(*l3d);
      cost_list.push_back(0.0); 
      delete l3d; 
      return; 
    }

    ////////////get some parameters///////////////////////////////////////
    ///// 1. the height of the elevator, 2. the number of floors ////////
    ///// 3. the average speed of people 4.the speed of the elevator /////
    ////////////               1 = 4    /////////////////////////////////
    //////////////////////////////////////////////////////////////////////

    vector<float> height_list; 
    float door_width = numeric_limits<float>::max(); //the speed for person 
    for(int i = 1;i <= rel->GetNoTuples();i++){
      Tuple* groom_tuple = rel->GetTuple(i, false);
      Line* door = (Line*)groom_tuple->GetAttribute(I_Door); 
      for(int j = 0;j < door->Size();j++){
        HalfSegment hs;
        door->Get(j, hs);
        if(!hs.IsLeftDomPoint())continue; 
        if(hs.Length() < door_width)door_width = hs.Length(); 
      }
      string type = 
          ((CcString*)groom_tuple->GetAttribute(I_Type))->GetValue(); 
      if(GetRoomEnum(type) == EL){
        GRoom* groom = (GRoom*)groom_tuple->GetAttribute(I_Room); 
        height_list.push_back(groom->GetLowHeight());
      }
      groom_tuple->DeleteIfAllowed();
    }
    sort(height_list.begin(), height_list.end()); 


    float floor_height = 0;
    if(height_list.size() >= 2){
      floor_height = height_list[1] - height_list[0];
    }

    int num_floors = height_list.size(); 

//    cout<<"floor height "<<floor_height<<endl; //the speed for elevator 

    float speed_person = door_width; 
    float speed_elevator = floor_height; 

//    cout<<"speed person "<<speed_person
//        <<" speed elevator "<<speed_elevator<<endl;

    struct I_Parameter param(num_floors, floor_height, 
                             speed_person, speed_elevator); 
   ////////////////two locations are inside the same room/////////////////////

    unsigned int groom_oid1 = gloc1->GetOid();
    unsigned int groom_oid2 = gloc2->GetOid(); 

    if(groom_oid1 == groom_oid2){
      PathInOneRoom(gloc1, gloc2, rel, groom_btree); 
      cost_list.push_back(path_list[0].Length() / speed_person); 
      return; 
    }

   /////////////////////////////////////////////////////////////////////////

    Relation* node_rel = ig->GetNodeRel(); 

   ///////////////collect all doors in that room//////////////////////
    BTree* btree = ig->GetBTree(); 
    CcInt* search_id = new CcInt(true, groom_oid1);
    BTreeIterator* btree_iter = btree->ExactMatch(search_id);
    vector<int> tid_list; 
    while(btree_iter->Next()){
      Tuple* tuple = node_rel->GetTuple(btree_iter->GetId(), false);
      tid_list.push_back(tuple->GetTupleId());
      tuple->DeleteIfAllowed();
    }
    delete btree_iter;
    delete search_id;

  ////////////////////for each possible door, search the path/////////////
  vector<Line3D> candidate_path; 
  vector<double> timecost; 
  for(unsigned int i = 0;i < tid_list.size();i++){
 
    Tuple* door_tuple = node_rel->GetTuple(tid_list[i], false);
    unsigned int gri1 = 
     ((CcInt*)door_tuple->GetAttribute(IndoorGraph::I_GROOM_OID1))->GetIntval();

    assert(gri1 == groom_oid1); 

    Line3D* l3d_start = new Line3D(0); 
    ConnectStartLoc(gloc1, tid_list[i], rel, groom_btree, l3d_start);


    Path_StartDoor_EndLoc_Time(tid_list[i], gloc2, candidate_path, l3d_start, 
                          rel, groom_btree, timecost, param);
    door_tuple->DeleteIfAllowed(); 

    delete l3d_start; 

  }

  assert(candidate_path.size() == timecost.size());
  ///////////////////select the path with minimum length////////////////////
  if(candidate_path.size() > 0 && timecost.size() > 0){
    double l = timecost[0];
    int index = 0;
    for(unsigned int i = 1; i < timecost.size();i++){
      if(timecost[i] < l){
        l = timecost[i];
        index = i; 
      }
    }

    path_list.push_back(candidate_path[index]); 
    cost_list.push_back(timecost[index]); 

  }else
    cout<<"no path available"<<endl; 

}


/*
path with minimum travelling time.
it should calculate two shortest path: one is without elevator and the other
might include elevator. if it includes the elevator, then the cost on the 
elevator is rest (uncertain value). Finally, compare the two costs. 

*/
void IndoorNav::Path_StartDoor_EndLoc_Time(int id, GenLoc* gloc2, 
                           vector<Line3D>& candidate_path, Line3D* l3d_s,
                Relation* rel, BTree* groom_btree, vector<double>& timecost,
                                           I_Parameter& param)
{

  unsigned int groom_oid2 = gloc2->GetOid(); 
  Relation* node_rel = ig->GetNodeRel(); 
  
  ///////////////collect all doors in that room//////////////////////
  CcInt* search_id = new CcInt(true, groom_oid2);
  BTree* btree = ig->GetBTree();
  BTreeIterator* btree_iter = btree->ExactMatch(search_id);
  vector<int> tid_list; 
  while(btree_iter->Next()){
     Tuple* tuple = node_rel->GetTuple(btree_iter->GetId(), false);
     tid_list.push_back(tuple->GetTupleId());
     tuple->DeleteIfAllowed();
  }
  delete btree_iter;
  delete search_id;


  for(unsigned int i = 0;i < tid_list.size();i++){

    Tuple* door_tuple = node_rel->GetTuple(tid_list[i], false);
    unsigned int gri1 = 
     ((CcInt*)door_tuple->GetAttribute(IndoorGraph::I_GROOM_OID1))->GetIntval();


    Line3D* l3d_end = new Line3D(0);
    ConnectEndLoc(gloc2, tid_list[i], rel, groom_btree, l3d_end); 
   
    assert(gri1 == groom_oid2); 
    IndoorShortestPath_Time1(id, tid_list[i], candidate_path, l3d_s, 
                            l3d_end, timecost, param, rel, groom_btree);
    IndoorShortestPath_Time2(id, tid_list[i], candidate_path, l3d_s, 
                            l3d_end, timecost, param, rel, groom_btree); 
    door_tuple->DeleteIfAllowed(); 
    
    delete l3d_end; 

  }

}

/*
the weight in the priority queue is changed to time instead of distance 
A star algorithm can still be applied. 
distance: Euclidean distance is set as the path length
speed: the maximum speed between person and elevator 
heuristic value = distacne div speed 
It searches the path without elevator 

*/
void IndoorNav::IndoorShortestPath_Time1(int id1, int id2, 
                           vector<Line3D>& candidate_path, Line3D* l3d_s, 
                           Line3D* l3d_d, vector<double>& timecost,
                           I_Parameter& param, Relation* rel, BTree* btree)
{

  if(id1 == id2){
    cout<<"two doors are equal"<<endl;
//    assert(false); 
    return; 
  }

  Relation* node_rel = ig->GetNodeRel(); 

  Tuple* door_tuple1 = node_rel->GetTuple(id1, false);
  Tuple* door_tuple2 = node_rel->GetTuple(id2, false);
  Line3D* l3d1 = (Line3D*)door_tuple1->GetAttribute(IndoorGraph::I_DOOR_LOC_3D);
  Line3D* l3d2 = (Line3D*)door_tuple2->GetAttribute(IndoorGraph::I_DOOR_LOC_3D);
  
  Point3D start_p, end_p;

  if(MiddlePoint(l3d1, start_p) == false || 
     MiddlePoint(l3d2, end_p) == false){
    cout<<"incorrect door "<<endl;
    assert(false);
  }


  door_tuple1->DeleteIfAllowed(); 
  door_tuple2->DeleteIfAllowed(); 
  
  priority_queue<IPath_elem> path_queue;
  vector<IPath_elem> expand_queue;
  
  vector<bool> visit_flag1;////////////door visit 
  for(int i = 1; i <= node_rel->GetNoTuples();i++)
    visit_flag1.push_back(false);


  float max_speed = MAX(param.speed_person, param.speed_elevator); 
  ///////////  initialize the queue //////////////////////////////
  int cur_size = expand_queue.size();

  double w = 0; 
  double hw = start_p.Distance(end_p) / max_speed; 
  Line3D* l3d = new Line3D(0);
  IPath_elem elem(-1, cur_size, id1, w + hw, w, *l3d);
  path_queue.push(elem);
  expand_queue.push_back(elem); 
  delete l3d; 
 ////////////////////////////////////////////////////////////////
   ///////////////////the path belongs to an elevator//////////////////////
  vector<float> h_list; //all possible values 
  for(int i = 0;i <= 2*param.num_floors;i++){
    h_list.push_back(param.floor_height + i*param.floor_height);
  }
 
 //////////////////////////////////////////////////////////////////
  bool find = false;
  IPath_elem dest;//////////destination
  while(path_queue.empty() == false){
    IPath_elem top = path_queue.top();
    path_queue.pop();

    if(visit_flag1[top.tri_index - 1])continue; 

    if(top.tri_index == id2){
//       cout<<"find the shortest path"<<endl;
       find = true;
       dest = top;
       break;
    }
    
    
    Tuple* door_tuple = node_rel->GetTuple(top.tri_index, false);
    /////////////////get the position of the door////////////////////////////
    Point3D q;
    Line3D* door_loc = 
      (Line3D*)door_tuple->GetAttribute(IndoorGraph::I_DOOR_LOC_3D); 
    assert(MiddlePoint(door_loc, q));

    door_tuple->DeleteIfAllowed(); 

   ////////find its adjacecy element, and push them into queue and path//////
    vector<int> adj_list;
    ig->FindAdj(top.tri_index, adj_list);

    /////////////////////////////////////////////////////////////
    int pos_expand_path = top.cur_index;

    for(unsigned int i = 0;i < adj_list.size();i++){

      Tuple* edge_tuple = ig->GetEdgeRel()->GetTuple(adj_list[i], false);
      int neighbor_id = 
      ((CcInt*)edge_tuple->GetAttribute(IndoorGraph::I_DOOR_TID2))->GetIntval();
      Line3D* path = (Line3D*)edge_tuple->GetAttribute(IndoorGraph::I_PATH);
      int groom_oid = 
      ((CcInt*)edge_tuple->GetAttribute(IndoorGraph::I_GROOM_OID))->GetIntval();
     
      if(IsElevator(groom_oid, rel, btree)){
        edge_tuple->DeleteIfAllowed(); 
        continue; 
      }

      if(visit_flag1[neighbor_id - 1]){

        edge_tuple->DeleteIfAllowed();
        continue; 
      }

     Tuple* door_tuple1 = node_rel->GetTuple(neighbor_id, false);

     Line3D* l = (Line3D*)door_tuple1->GetAttribute(IndoorGraph::I_DOOR_LOC_3D);
     Point3D p;

     assert(MiddlePoint(l, p));//get the point of next door 
     door_tuple1->DeleteIfAllowed(); 


     int cur_size = expand_queue.size();
     ////////////// set the weight //////////////////////////////////////

     double w = top.real_w + 
      SetTimeWeight(path->Length(), groom_oid, rel, btree, param, h_list);

     double hw =  p.Distance(end_p) / max_speed; 
     //////////////////////////////////////////////////////////////////////
     IPath_elem elem(pos_expand_path, cur_size, 
                      neighbor_id, w + hw, w, *path);
     path_queue.push(elem);
     expand_queue.push_back(elem); 

     edge_tuple->DeleteIfAllowed();
    }

    visit_flag1[top.tri_index - 1] = true; 

  }

  ////////////////construct the result//////////////////////////////
  if(find){
    double cost_t = dest.real_w; 
//    cout<<"cost_t "<<cost_t<<endl; 

    vector<Point3D> ps_list; 
    while(dest.prev_index != -1){
      if(dest.path.Size() > 0){
          if(ps_list.size() == 0){
              for(int i = dest.path.Size() - 1;i >= 0;i--){
                  Point3D q;
                  dest.path.Get(i, q);
                  ps_list.push_back(q);
              }
          }else{
              for(int i = dest.path.Size() - 2;i >= 0;i--){
                  Point3D q;
                  dest.path.Get(i, q);
                  ps_list.push_back(q);
              }
          }
      }
      dest = expand_queue[dest.prev_index];
    }

    if(dest.path.Size() > 0){
      for(int i = dest.path.Size() - 2;i >= 0;i--){
        Point3D q;
        dest.path.Get(i, q);
        ps_list.push_back(q);
      }
    }

    Line3D* l3d = new Line3D(0);
    l3d->StartBulkLoad();
    //////////////////connect to the start point////////////////////
    for(int i = 0;i < l3d_s->Size() - 1;i++){
      Point3D q;
      l3d_s->Get(i, q);
      *l3d += q; 
    }
    ///////////////////////////////////////////////////////////////

    for(int i = ps_list.size() - 1;i >= 0; i--){
      *l3d += ps_list[i];
    }
    ////////////////////connect to the end point/////////////////////
    for(int i = 1;i < l3d_d->Size();i++){
      Point3D q;
      l3d_d->Get(i, q);
      *l3d += q; 
    }

    l3d->EndBulkLoad(); 
    candidate_path.push_back(*l3d);
    delete l3d; 

    if(l3d_s->Size() > 0)
      cost_t += l3d_s->Length() / param.speed_person; 
    if(l3d_d->Size() > 0)
      cost_t += l3d_d->Length() / param.speed_person; 

    timecost.push_back(cost_t);
  }

}

/*
check whether the room is an elevator 

*/
bool IndoorNav::IsElevator(int groom_oid, Relation* rel, BTree* btree)
{

  CcInt* search_id = new CcInt(true, groom_oid);
  BTreeIterator* btree_iter = btree->ExactMatch(search_id);
  int groom_tid = 0; 
  while(btree_iter->Next()){
     groom_tid = btree_iter->GetId();
     break; 
  }
  delete btree_iter;
  delete search_id;

  assert(1 <= groom_tid && groom_tid <= rel->GetNoTuples()); 
  Tuple* groom_tuple = rel->GetTuple(groom_tid, false);
  string type = ((CcString*)groom_tuple->GetAttribute(I_Type))->GetValue();
  groom_tuple->DeleteIfAllowed(); 

  if(GetRoomEnum(type) == EL){//////////// an elevator
    return true; 
  }else
    return false; 
}


/*
calculate the time cost of an edge: use the length of path and speed value 

*/

float IndoorNav::SetTimeWeight(double l, int groom_oid, Relation* rel, 
                       BTree* btree, I_Parameter& param, vector<float>& h_list)
{
   ///////////////find the room////////////////////////////////////////////
  CcInt* search_id = new CcInt(true, groom_oid);
  BTreeIterator* btree_iter = btree->ExactMatch(search_id);
  int groom_tid = 0; 
  while(btree_iter->Next()){
     groom_tid = btree_iter->GetId();
     break; 
  }
  delete btree_iter;
  delete search_id;

  assert(1 <= groom_tid && groom_tid <= rel->GetNoTuples()); 
  Tuple* groom_tuple = rel->GetTuple(groom_tid, false);
  string type = ((CcString*)groom_tuple->GetAttribute(I_Type))->GetValue();
  groom_tuple->DeleteIfAllowed(); 

  if(GetRoomEnum(type) != EL){////////////not an elevator
    return l/param.speed_person; 
  }
  cout<<"should not visit here"<<endl;
  assert(false);
  return l/param.speed_elevator; 

}

/*
the weight in the priority queue is changed to time instead of distance 
A star algorithm can still be applied. 
distance: Euclidean distance is set as the path length
speed: the maximum speed between person and elevator 
heuristic value = distacne div speed 

If the shortest path uses the elevator, the weight value should be reset 

*/
void IndoorNav::IndoorShortestPath_Time2(int id1, int id2, 
                           vector<Line3D>& candidate_path, Line3D* l3d_s, 
                           Line3D* l3d_d, vector<double>& timecost,
                           I_Parameter& param, Relation* rel, BTree* btree)
{

  if(id1 == id2){
    cout<<"two doors are equal"<<endl;
//    assert(false); 
    return; 
  }

  Relation* node_rel = ig->GetNodeRel(); 

  Tuple* door_tuple1 = node_rel->GetTuple(id1, false);
  Tuple* door_tuple2 = node_rel->GetTuple(id2, false);
  Line3D* l3d1 = (Line3D*)door_tuple1->GetAttribute(IndoorGraph::I_DOOR_LOC_3D);
  Line3D* l3d2 = (Line3D*)door_tuple2->GetAttribute(IndoorGraph::I_DOOR_LOC_3D);
  
  Point3D start_p, end_p;

  if(MiddlePoint(l3d1, start_p) == false || 
     MiddlePoint(l3d2, end_p) == false){
    cout<<"incorrect door "<<endl;
    assert(false);
  }


  door_tuple1->DeleteIfAllowed(); 
  door_tuple2->DeleteIfAllowed(); 
  
  priority_queue<IPath_elem> path_queue;
  vector<IPath_elem> expand_queue;
  
  vector<bool> visit_flag1;////////////door visit 
  for(int i = 1; i <= node_rel->GetNoTuples();i++)
    visit_flag1.push_back(false);

  ///////////  initialize the queue //////////////////////////////
  int cur_size = expand_queue.size();

  double w = 0; 
  double hw = start_p.Distance(end_p); 
  Line3D* l3d = new Line3D(0);
  IPath_elem elem(-1, cur_size, id1, w + hw, w, *l3d);
  path_queue.push(elem);
  expand_queue.push_back(elem); 
  delete l3d; 

 //////////////////////////////////////////////////////////////////
  bool find = false;
  IPath_elem dest;//////////destination
  while(path_queue.empty() == false){
    IPath_elem top = path_queue.top();
    path_queue.pop();

    if(visit_flag1[top.tri_index - 1])continue; 

    if(top.tri_index == id2){
//       cout<<"find the shortest path"<<endl;
       find = true;
       dest = top;
       break;
    }
    
    
    Tuple* door_tuple = node_rel->GetTuple(top.tri_index, false);
    /////////////////get the position of the door////////////////////////////
    Point3D q;
    Line3D* door_loc = 
      (Line3D*)door_tuple->GetAttribute(IndoorGraph::I_DOOR_LOC_3D); 
    assert(MiddlePoint(door_loc, q));

    door_tuple->DeleteIfAllowed(); 

   ////////find its adjacecy element, and push them into queue and path//////
    vector<int> adj_list;
    ig->FindAdj(top.tri_index, adj_list);

    /////////////////////////////////////////////////////////////
    int pos_expand_path = top.cur_index;

    for(unsigned int i = 0;i < adj_list.size();i++){

      Tuple* edge_tuple = ig->GetEdgeRel()->GetTuple(adj_list[i], false);
      int neighbor_id = 
      ((CcInt*)edge_tuple->GetAttribute(IndoorGraph::I_DOOR_TID2))->GetIntval();
      Line3D* path = (Line3D*)edge_tuple->GetAttribute(IndoorGraph::I_PATH);
      int groom_oid = 
      ((CcInt*)edge_tuple->GetAttribute(IndoorGraph::I_GROOM_OID))->GetIntval();

      if(visit_flag1[neighbor_id - 1]){

        edge_tuple->DeleteIfAllowed();
        continue; 
      }

     Tuple* door_tuple1 = node_rel->GetTuple(neighbor_id, false);

     Line3D* l = (Line3D*)door_tuple1->GetAttribute(IndoorGraph::I_DOOR_LOC_3D);
     Point3D p;

     assert(MiddlePoint(l, p));//get the point of next door 
     door_tuple1->DeleteIfAllowed(); 


     int cur_size = expand_queue.size();
     ////////////// set the weight //////////////////////////////////////

     double w = top.real_w + path->Length(); 
     double hw =  p.Distance(end_p); 
     //////////////////////////////////////////////////////////////////////
     IPath_elem elem(pos_expand_path, cur_size, 
                      neighbor_id, w + hw, w, *path, groom_oid);
     path_queue.push(elem);
     expand_queue.push_back(elem); 

     edge_tuple->DeleteIfAllowed();
    }

    visit_flag1[top.tri_index - 1] = true; 

  }

  ////////////////construct the result//////////////////////////////
  if(find){
    double cost_t = 0; 
//    cout<<"cost_t "<<cost_t<<endl; 

    vector<Point3D> ps_list; 
    double elevator_length = 0; 
    while(dest.prev_index != -1){
      if(dest.path.Size() > 0){
          int groom_oid = dest.groom_oid;
          if(IsElevator(groom_oid, rel, btree))//if the path belongs to lift 
              elevator_length += dest.path.Length(); 

          if(ps_list.size() == 0){
              for(int i = dest.path.Size() - 1;i >= 0;i--){
                  Point3D q;
                  dest.path.Get(i, q);
                  ps_list.push_back(q);
              }
            }else{
              for(int i = dest.path.Size() - 2;i >= 0;i--){
                  Point3D q;
                  dest.path.Get(i, q);
                  ps_list.push_back(q);
              }
          }
      }
      dest = expand_queue[dest.prev_index];
    }

    if(dest.path.Size() > 0){
      for(int i = dest.path.Size() - 2;i >= 0;i--){
        Point3D q;
        dest.path.Get(i, q);
        ps_list.push_back(q);
      }
    }

    Line3D* l3d = new Line3D(0);
    l3d->StartBulkLoad();
    //////////////////connect to the start point////////////////////
    for(int i = 0;i < l3d_s->Size() - 1;i++){
      Point3D q;
      l3d_s->Get(i, q);
      *l3d += q; 
    }
    ///////////////////////////////////////////////////////////////

    for(int i = ps_list.size() - 1;i >= 0; i--){
      *l3d += ps_list[i];
    }
    ////////////////////connect to the end point/////////////////////
    for(int i = 1;i < l3d_d->Size();i++){
      Point3D q;
      l3d_d->Get(i, q);
      *l3d += q; 
    }

    l3d->EndBulkLoad(); 
    candidate_path.push_back(*l3d);
    if(elevator_length > 0.0){
      cost_t = (l3d->Length() - elevator_length) / param.speed_person; 
      cost_t += CostInElevator( elevator_length, param); 
    }else
      cost_t = l3d->Length() / param.speed_person; 

    delete l3d; 

    timecost.push_back(cost_t);
  }

}

/*
the uncertain cost value on an elevator: 
let l be absolute length of two floors and h be the distance between two 
consecutive floors, there are n floors. 
the distribution is: l, l+h, l+2h, l+3h, ...l+2nh

*/
float IndoorNav::CostInElevator(double l, I_Parameter& param)
{
  vector<float> h_list; //all possible values 
  for(int i = 0;i <= 2*param.num_floors;i++){
    h_list.push_back(l + i*param.floor_height);
  }
  
  struct timeval tval;
  struct timezone tzone;

  gettimeofday(&tval, &tzone);
  srand48(tval.tv_sec); //second
//  srand48(tval.tv_usec);  //Microseconds

  const int size = h_list.size();
  int index =  lrand48() % size;  
//  cout<<"index "<<index<<endl; 

  return h_list[index]/param.speed_elevator; 
}

////////////////////////////////////////////////////////////////////////
/////////////temporal unit: UPoint3D ///////////////////////////////////
///////////////////////////////////////////////////////////////////////
/*
interporlation function for the location 

*/
void UPoint3D::TemporalFunction( const Instant& t, Point3D& result,
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
      result = p0;
      result.SetDefined(true);
    }
  else if( t == timeInterval.end )
    {
      result = p1;
      result.SetDefined(true);
    }
  else
    {
      Instant t0 = timeInterval.start;
      Instant t1 = timeInterval.end;

      
      double x = (p1.GetX() - p0.GetX()) * ((t - t0) / (t1 - t0)) + p0.GetX();
      double y = (p1.GetY() - p0.GetY()) * ((t - t0) / (t1 - t0)) + p0.GetY();
      double z = (p1.GetZ() - p0.GetZ()) * ((t - t0) / (t1 - t0)) + p0.GetZ();

      Point3D p(true, x, y, z);
      result = p;
    }
}

/*
check whether a location is visited 

*/
bool UPoint3D::Passes( const Point3D& gloc ) const
{
  return false;
}

/*
restrict the movement at a location 

*/

bool UPoint3D::At( const Point3D& loc, TemporalUnit<Point3D>& res ) const 
{
  return false; 
}

UPoint3D* UPoint3D::Clone() const
{
  UPoint3D* res;
  if(this->IsDefined()){
    
    res = new UPoint3D(timeInterval, p0, p1); 
    res->del.isDefined = del.isDefined;
  }else{
    res = new UPoint3D(false); 
  }
  return res; 
}


void UPoint3D::CopyFrom(const Attribute* right)
{
  const UPoint3D* loc = static_cast<const UPoint3D*>(right); 
  if(loc->del.isDefined){
    timeInterval.CopyFrom(loc->timeInterval);
    p0 = loc->p0;
    p1 = loc->p1;
  }
  del.isDefined = loc->del.isDefined; 
}

const Rectangle<4> UPoint3D::BoundingBox() const
{
  if(this->IsDefined()){

    return Rectangle<4>(true, 
                       MIN(p0.GetX(), p1.GetX()),
                       MAX(p0.GetX(), p1.GetX()), 
                       MIN(p0.GetY(), p1.GetY()),
                       MAX(p0.GetY(), p1.GetY()),  
                       MIN(p0.GetZ(), p1.GetZ()),
                       MAX(p0.GetZ(), p1.GetZ()),  
                       timeInterval.start.ToDouble(),
                       timeInterval.end.ToDouble());
  }else
      return Rectangle<4>(false); 

}


ListExpr UPoint3DProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("upoint3d"),
           nl->StringAtom("(<interval,x,y,z>)"),
      nl->StringAtom("((interval (1.0 12.0 3.0)))"))));
}

/*
Output  (interval, p0, p1)

*/

ListExpr OutUPoint3D( ListExpr typeInfo, Word value )
{
//  cout<<"OutUPoint3D"<<endl; 
  UPoint3D* loc = (UPoint3D*)(value.addr);
  if(!loc->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  if( loc->IsEmpty() ){
    return nl->TheEmptyList();
  }
    ListExpr timeintervalList = nl->FourElemList(
          OutDateTime( nl->TheEmptyList(),
          SetWord(&loc->timeInterval.start) ),
          OutDateTime( nl->TheEmptyList(), 
                       SetWord(&loc->timeInterval.end) ),
          nl->BoolAtom( loc->timeInterval.lc ),
          nl->BoolAtom( loc->timeInterval.rc)); 
    ListExpr loc1 = OutPoint3D(nl->TheEmptyList(), &(loc->p0));
    ListExpr loc2 = OutPoint3D(nl->TheEmptyList(), &(loc->p1));
    
    return nl->ThreeElemList(timeintervalList,loc1,loc2); 
}

/*
In function

*/
Word InUPoint3D( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  string errmsg;
  if ( nl->ListLength( instance ) == 3 ){
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
        errmsg = "InUPoint3D(): Error in first instant (Must be defined!).";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
       nl->Second( first ),
                                           errorPos, errorInfo, correct ).addr;

      if( !correct  || !end->IsDefined() )
      {
        errmsg = "InUPoint3D(): Error in second instant (Must be defined!).";
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
          errmsg = "InUPoint3D(): Non valid time interval.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
        }

      //////////////////////////////////////////////////////////////////
      ListExpr second = nl->Second( instance );
      if(nl->ListLength(second) != 3){
        errmsg = "InUPoint3D(): the length for Point3D should be 3.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        return SetWord( Address(0) );
      }
      
      bool loc1_correct; 
      Point3D* loc1 = (Point3D*)InPoint3D(typeInfo, second, 
                                        errorPos,errorInfo, loc1_correct).addr;
      if(loc1_correct == false){
          errmsg = "InUPoint3D(): Non correct first location.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
      }

      /////////////////////////////////////////////////////////////////////
      ListExpr third = nl->Third(instance); 
      if(nl->ListLength(third) != 3){
        errmsg = "InUPoint3D(): the length for Point3D should be 3.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        return SetWord( Address(0) );
      }
      bool loc2_correct; 
      Point3D* loc2 = (Point3D*)InPoint3D(typeInfo, third, 
                                        errorPos,errorInfo, loc2_correct).addr;
      if(loc2_correct == false){
          errmsg = "InUPoint3D(): Non correct second location.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
      }

      /////////////////////////////////////////////////////////////////////
//      cout<<tinterval<<*loc1<<*loc2<<endl;

      UPoint3D *up = new UPoint3D( tinterval, *loc1, *loc2);

      correct = up->IsValid();
      if( correct )
          return SetWord( up );

      errmsg = "InUPoint3D(): Error in start/end point.";
      errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      delete up;
    }
  }else if ( nl->IsAtom( instance ) && nl->AtomType( instance ) == SymbolType
            && nl->SymbolValue( instance ) == "undef" ){
      UPoint3D *loc = new UPoint3D(false);
      loc->timeInterval=
                Interval<DateTime>(DateTime(instanttype),
                           DateTime(instanttype),true,true);
      correct = loc->timeInterval.IsValid();
      if ( correct )
        return (SetWord( loc ));
  }
  errmsg = "InUPoint3D(): Error in representation.";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  correct = false;
  return SetWord( Address(0) );
}

/*
Open a UPoint3D object 

*/
bool OpenUPoint3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenUPoint3D()"<<endl; 

  UPoint3D* genl = (UPoint3D*)Attribute::Open(valueRecord, offset, typeInfo);
  value = SetWord(genl);
  return true; 
}

/*
Save a UPoint3D object 

*/
bool SaveUPoint3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveUPoint3D"<<endl; 
  UPoint3D* genl = (UPoint3D*)value.addr; 
  Attribute::Save(valueRecord, offset, typeInfo, genl);
  return true; 
  
}

Word CreateUPoint3D(const ListExpr typeInfo)
{
// cout<<"CreateUPoint3D()"<<endl;
  return SetWord (new UPoint3D(0));
}


void DeleteUPoint3D(const ListExpr typeInfo, Word& w)
{
// cout<<"DeleteUPoint3D()"<<endl;
  UPoint3D* up = (UPoint3D*)w.addr;
  delete up;
   w.addr = NULL;
}


void CloseUPoint3D( const ListExpr typeInfo, Word& w )
{
//  cout<<"CloseUPoint3D"<<endl; 
  ((UPoint3D*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word CloneUPoint3D( const ListExpr typeInfo, const Word& w )
{
//  cout<<"CloneUPoint3D"<<endl; 
  return SetWord( new UPoint3D( *((UPoint3D*)w.addr) ) );
}

int SizeOfUPoint3D()
{
//  cout<<"SizeOfUPoint3D"<<endl; 
  return sizeof(UPoint3D);
}

bool CheckUPoint3D( ListExpr type, ListExpr& errorInfo )
{
//  cout<<"CheckUPoint3D"<<endl; 
  return (nl->IsEqual( type, "upoint3d" ));
}

//////////////////////////////////////////////////////////////////
//////////  MPoinr3D /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
ListExpr MPoint3DProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                       nl->StringAtom("mpoint3d"),
           nl->StringAtom("(u1,...,un)"),
      nl->StringAtom("((interval (1 10.0 1.0)(1 12.0 3.0)))"))));
}


bool CheckMPoint3D( ListExpr type, ListExpr& errorInfo )
{
//  cout<<"CheckMPoint3D"<<endl; 
  return (nl->IsEqual( type, "mpoint3d" ));
}

void MPoint3D::Clear()
{
  Mapping<UPoint3D, Point3D>::Clear();
}


/*
copy it from another data 

*/
void MPoint3D::CopyFrom(const Attribute* right)
{

    const MPoint3D *mo = (const MPoint3D*)right;
    assert( mo->IsOrdered() );
    Clear();
    this->SetDefined(mo->IsDefined());
    if( !this->IsDefined() ) {
      return;
    }
    StartBulkLoad();
    UPoint3D unit;
    for( int i = 0; i < mo->GetNoComponents(); i++ ){
      mo->Get( i, unit );
      Add( unit );
    }
    EndBulkLoad( false );

}

Attribute* MPoint3D::Clone() const
{
    assert( IsOrdered() );
    MPoint3D* result;
    if( !this->IsDefined() ){
      result = new MPoint3D( 0 );
    } else {
      result = new MPoint3D( GetNoComponents() );
      if(GetNoComponents()>0){
        result->units.resize(GetNoComponents());
      }
      result->StartBulkLoad();
      UPoint3D unit;
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
void MPoint3D::Add(const UPoint3D& unit)
{
  assert(unit.IsDefined());
  assert(unit.IsValid()); 
  if(!IsDefined()){
    SetDefined(false);
    return; 
  }
  units.Append(unit); 
}

void MPoint3D::EndBulkLoad(const bool sort, const bool checkvalid)
{
  Mapping<UPoint3D, Point3D>::EndBulkLoad(sort, checkvalid); 

}

/*
3d line representing the trajectory of indoor moving objects 

*/
void MPoint3D::Trajectory(Line3D& l)
{
  l.StartBulkLoad(); 
  for(int i = 0;i < GetNoComponents();i++){
    UPoint3D unit;
    Get(i, unit); 
    l += unit.p0;
    if(i == GetNoComponents() - 1)
      l+= unit.p1; 
  }
  l.EndBulkLoad(); 
}

