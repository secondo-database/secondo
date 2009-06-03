
/*
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

paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
paragraph [10] Footnote: [{\footnote{] [}}]

March 2009 Brigitte Metzker

0.    Overview       UGrid - Algebra

This Algebra implements the datastructures and operators
for create a three- dimensional UGrid and for handle queries.

The three dimensions are the two-dimensional euklidian coordinate system
for the description of a position and a timestamp assign the positions
as the third dimension.

The applied data structures are
Mobpos         two double values identify the position

UGridArea      four double values identify an area limeted by a start- and
               an end- position

UpdateUnit     incoming information with an integer value for identifcation
               a moving object and a MobPos value for identify its position

CurrentUnit    evaluated information of a possible next position in a
               trajectory based to the last UpdateUnit for a moving object

HistoryUnit    merged information of the last two updateunits for a
               moving object

UGridCell      Information unit consisting of a header with indication of
               the assigned area and timeinterval of storaged data and of
               the data portion consisting of an Array of
               max = sizeHistoryArray (75) history units

UGrid          the master data structure of this Algebra consisting of a
               header with indication of the assigned area and timeinterval
               of storaged data and the link to a possible followed ugrid
               and the memory part with maximal 1024 ugridcells with
               requested cellcontainer.

The provided operators are
createugrid           for ugrid-creation

insertunit           for insertion of single updateunits into ugrid

insertstream         for insertion of a stream of updateunits into ugrid

windowintersectug    for request one of the three attributes id, area or
                     time after input of the two remaining attributes

windowintersectsug   for request two of the three attributes id,
                     area or time after input of the one remaining attribute
*/

/*
1.   Defines and Includes               

*/
#include <iostream>
#include <stack>

using namespace std;

#include "SpatialAlgebra.h"
#include "RelationAlgebra.h"
#include "TemporalAlgebra.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RectangleAlgebra.h"
#include "RTreeAlgebra.h"
#include "DBArray.h"
#include "CPUTimeMeasurer.h"
#include "TupleIdentifier.h"
#include "Messages.h"
#include "Progress.h"
#include "UGridAlgebra.h"
#include "Symbols.h"        

#include "UGridUtilities.h"
#include <cmath>

using namespace std;


extern NestedList* nl;
extern QueryProcessor* qp;

using namespace symbols;

namespace ugridAlgebra {
/*
1.1   global variables

*/
UGridUtilities myUtilities;
StACurrentUnit myCurrentUnit;
StAHistoryUnit myHistoryUnit;
/*
1.2   auxiliary functions

*/
string intToString(int i)
{
 char buffer[256];
 sprintf(buffer,"%d",i);
 return string(buffer);
}
string tostring ( double d )
{
	string s;
	stringstream str;
	str << d;
	str >> s;
	return s;
}
long LongTime()
{
   long zeit;
   zeit = time(NULL);
   time_t rawtime;
   rawtime = zeit;
   struct tm * timeinfo;
   timeinfo = localtime ( &rawtime );
   return zeit;
}
Interval<Instant> TimeInterval()
{
  DateTime t1(instanttype);
  t1.Now();
  DateTime t2(instanttype);
  t2.Today();
  Interval<Instant> t  = Interval<Instant>(t1, t2, true, false);	
  return t;
}
long InstantToLong(Instant entry)
{
  struct tm t; 
  t.tm_sec = entry.GetSecond();
  t.tm_min = entry.GetMinute();
  t.tm_hour = entry.GetHour()+1;
  t.tm_mday = entry.GetGregDay();
  t.tm_mon = entry.GetMonth()-1;
  t.tm_year = entry.GetYear()-1900;
  time_t tlong;
  tlong = mktime(&t);
  return tlong;	
}
Instant LongToInstant(long ltime)
{
  time_t rawtime;
  rawtime = ltime;
  struct tm * timeinfo;
  timeinfo = localtime ( &rawtime );
  DateTime inttime(instanttype); 
  inttime.Set(timeinfo->tm_year + 1900,timeinfo->tm_mon +1 , timeinfo->tm_mday,
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, 0);
  return inttime;	
}

/*
2  Implementations of DataStructures

2.1        - Class ~MobPos~

2.1.1           Public methods

2.1.2    Constructors and destructor

*/

MobPos::MobPos(double X, double Y) : x(X), y(Y) {}

MobPos::MobPos(const MobPos& Pos) : x(Pos.x), y(Pos.y) {}

////  Destructor ~mobpos~

MobPos::~MobPos() {} 
/*
2.1.3           Methods

*/
double MobPos::GetX() const { return x; }
double MobPos::GetY() const { return y; }

void MobPos::SetX(double X) { x = X; }
void MobPos::SetY(double Y) { y = Y; }

ostream& operator << (ostream &cout, const MobPos &pos)
{
 return cout << pos.GetX() << ';' << pos.GetY() << '>';
}
/*
2.1.4          static methods
2.1.5          private methods
2.1.6          List Representation
2.1.6.1    ~In~ function of class mobpos

*/
Word  MobPos::InMobPos( const ListExpr typeInfo, const ListExpr instance,
            const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Word w = SetWord(Address(0));
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr First = nl->First(instance);
    ListExpr Second = nl->Second(instance);

    if ( nl->IsAtom(First) && nl->AtomType(First) == RealType
      && nl->IsAtom(Second) && nl->AtomType(Second) == RealType )
    {
      correct = true;
      w.addr = new MobPos(nl->RealValue(First), nl->RealValue(Second));
      return w;
    }
  }
  correct = false;
  cmsg.inFunError("Expecting a list of two double atoms!");
  return w;
}
/*
2.1.6.2   ~Out~ function of class mobpos

*/
ListExpr  MobPos::OutMobPos( ListExpr typeInfo, Word value )
{
  MobPos* point = static_cast<MobPos*>( value.addr );
  return nl->TwoElemList(nl->RealAtom(point->GetX()),
                         nl->RealAtom(point->GetY()));
}
/*
2.1.6.3   ~SaveToList~ function of class mobpos

   not used
2.1.6.4   ~RestoreFromList~ function of class mobpos

   not used
2.1.6.5   ~Create~ function of class mobpos

*/
Word  MobPos::CreateMobPos( const ListExpr typeInfo )
{
  return (SetWord( new MobPos( 0, 0 ) ));
}

/*
2.1.6.6   ~Delete~ function of class mobpos

*/
void  MobPos::DeleteMobPos( const ListExpr typeInfo, Word& w )
{ 
  delete static_cast<MobPos*>( w.addr );
  w.addr = 0;
}

/*
2.1.6.7   ~Open~ function of class mobpos

   not used

2.1.6.8   ~Save~ function of class mobpos

   not used

2.1.6.9   ~Close~ function of class mobpos

*/
void  MobPos::CloseMobPos( const ListExpr typeInfo, Word& w )
{
  delete static_cast<MobPos*>( w.addr );
  w.addr = 0;
}

/*
2.1.6.10  ~Clone~ function of class mobpos

*/
Word  MobPos::CloneMobPos( const ListExpr typeInfo, const Word& w )
{
  MobPos* p = static_cast<MobPos*>( w.addr );
  return SetWord( new MobPos(*p) );
}

/*
2.1.6.11   ~Cast~ function of class mobpos

   not used
2.1.6.12   ~SizeOfObj~ function of class mobpos

*/
int  MobPos::SizeOfObjMobPos()
{
  return sizeof(MobPos);
}

/*
2.1.6.13   Type Description of class mobpos

*/
ListExpr  MobPos::PropertyMobPos()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
               nl->StringAtom("Example Type List"),
               nl->StringAtom("List Rep"),
               nl->StringAtom("Example List"),
               nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
               nl->StringAtom("mobpos"),
               nl->StringAtom("(<x> <y>)"),
               nl->StringAtom("(-3.2 15.44)"),
               nl->StringAtom("x-/y-coord of type double"))));
}

/*
2.1.6.14   ~KindCheck~ function of class mobpos

*/

bool  MobPos::KindCheckMobPos( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, MOBPOS ));
}

/*
2.1.6..15 Creation of TypeConstructor instance of class mobpos

*/
TypeConstructor mobposTC(
  MOBPOS,                                     // name of the type in SECONDO
  MobPos::PropertyMobPos,                     // property function 
  MobPos::OutMobPos, MobPos::InMobPos,        // Out and In functions
  0, 0,                                       // SaveToList, RestoreFromList 
  MobPos::CreateMobPos, MobPos::DeleteMobPos, // object creation and deletion
  0, 0,                                       // object open, save
  MobPos::CloseMobPos, MobPos::CloneMobPos,   // close, and clone
  0,                                          // cast function
  MobPos::SizeOfObjMobPos,                    // sizeof function
  MobPos::KindCheckMobPos );                  // kind checking function
/*
2.2        Class UGridArea
2.2.1        Public methods
2.2.2        Constructors and destructor

*/
UGridArea::UGridArea( double X1, double Y1, double X2, double Y2 )
{
  x1 = X1; y1 = Y1; x2 = X2; y2 = Y2;
}
UGridArea::UGridArea( const UGridArea& area )

{
  x1 = area.x1; y1 = area.y1; x2 = area.x2; y2 = area.y2;
}	
///  Destructor ~UGridArea~
UGridArea::~UGridArea() {} 
/*
2.2.3        Methods

*/
double UGridArea::GetX1() const { return x1; }
double UGridArea::GetY1() const { return y1; }
double UGridArea::GetX2() const { return x2; }
double UGridArea::GetY2() const { return y2; }

UGridArea UGridArea::GetArea() 
{
  UGridArea ar;
  ar.x1 = ar.GetX1();
  ar.y1 = ar.GetY1();
  ar.x2 = ar.GetX2();
  ar.y2 = ar.GetY2();
  return ar;
}
void UGridArea::SetX1(double a1) { x1 = a1; }
void UGridArea::SetY1(double b1) { y1 = b1; }
void UGridArea::SetX2(double a2) { x2 = a2; }
void UGridArea::SetY2(double b2) { y2 = b2; }
/*
2.2.4        static methods
2.2.5        private methods

*/
bool overlap ( double low1, double high1, double low2, double high2 )
{
  if ( high1 < low2 || high2 < low1 ) 
    return false; 
  else 
    return true;
}

bool  UGridArea::intersects( const UGridArea& r ) const
{
  return ( overlap(x1, x2, r.GetX1(), r.GetX2())
           && overlap(y1, y2, r.GetY1(), r.GetY2()) );
}
/*
2.2.6        List Representation
2.2.6.1   ~In~ function of class ugridarea

*/
Word  UGridArea::InUGridArea( const ListExpr typeInfo, const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = false;
  Word result = SetWord(Address(0));
  const string errMsg = "Expecting a list of four double atoms";

  if ( nl->ListLength(instance) != 4 ) 
  {
    cmsg.inFunError(errMsg);
    return result;
  } 
  else
  {
    ListExpr a1 = nl->First(instance);
    ListExpr a2 = nl->Second(instance);
    ListExpr a3 = nl->Third(instance);
    ListExpr a4 = nl->Fourth(instance);
    if (nl->IsAtom(a1) && nl->AtomType(a1) == RealType &&
        nl->IsAtom(a2) && nl->AtomType(a2) == RealType &&
	    nl->IsAtom(a3) && nl->AtomType(a3) == RealType &&
	    nl->IsAtom(a4) && nl->AtomType(a4) == RealType)
	{
	   double x1 = nl->RealValue(a1);
	   double y1 = nl->RealValue(a2);
	   double x2 = nl->RealValue(a3);
	   double y2 = nl->RealValue(a4);
       if (( x2 == x1 ) || (y1 == y2 ))      // vertical or horizontal line   
	   {
         const string errorMsg = "vertical or horizontal line or point";
		 cmsg.inFunError(errorMsg);
         return result;
	   }
	   else
	   {
         correct = true;
         UGridArea* r = new UGridArea(x1, y1, x2, y2);
         result.addr = r;
       }
    }
    else
    {	  
       cmsg.inFunError(errMsg);
    } 
  }
  return result;
}

/*
2.2.6.2   ~Out~ function of class ugridarea

*/
ListExpr  UGridArea::OutUGridArea( ListExpr typeInfo, Word value )
{
  UGridArea* area = static_cast<UGridArea*>( value.addr );
  NList fourElems(
           NList( area->GetX1() ),
           NList( area->GetY1() ),
           NList( area->GetX2() ),
           NList( area->GetY2() ) );

  return fourElems.listExpr();
}

/*
2.2.6.3   ~SaveToList~ function of class ugridarea

   not used
2.2.6.4   ~RestoreFromList~ function of class ugridarea

   not used
2.2.6.5   ~Create~ function of class ugridarea

*/
Word  UGridArea::CreateUGridArea( const ListExpr typeInfo )
{
  return (SetWord( new UGridArea( 0, 0, 0, 0 ) ));
}

/*
2.2.6.6   ~Delete~ function of class ugridarea

*/
void  UGridArea::DeleteUGridArea( const ListExpr typeInfo, Word& w )
{ 
  delete static_cast<UGridArea*>( w.addr );
  w.addr = 0;
}

/*
2.2.6.7   ~Open~ function of class ugridarea

*/
bool  UGridArea::OpenUGridArea( SmiRecord& valueRecord, 
                  size_t& offset,  const ListExpr typeInfo, 
                  Word& value ) 
{ 
  size_t size = sizeof(double); 	
  double x1 = 0, x2 = 0, y1 = 0, y2 = 0;

  bool ok = true;
  ok = ok && valueRecord.Read( &x1, size, offset );
  offset += size;  
  ok = ok && valueRecord.Read( &y1, size, offset );
  offset += size;  
  ok = ok && valueRecord.Read( &x2, size, offset );
  offset += size;  
  ok = ok && valueRecord.Read( &y2, size, offset );
  offset += size;  

  value.addr = new UGridArea(x1, y1, x2, y2); 

  return ok;
}	

/*
2.2.6.8   ~Save~ function of class ugridarea

*/
bool UGridArea::SaveUGridArea( SmiRecord& valueRecord, size_t& offset, 
                  const ListExpr typeInfo, Word& value )
{
  UGridArea* r = static_cast<UGridArea*>( value.addr );	
  size_t size = sizeof(double); 	

  bool ok = true;
  ok = ok && valueRecord.Write( &r->x1, size, offset );
  offset += size;  
  ok = ok && valueRecord.Write( &r->y1, size, offset );	
  offset += size;  
  ok = ok && valueRecord.Write( &r->x2, size, offset );	
  offset += size;  
  ok = ok && valueRecord.Write( &r->y2, size, offset );	
  offset += size;  

  return ok;
}	

/*
2.2.6.9   ~Close~ function of class ugridarea

*/
void  UGridArea::CloseUGridArea( const ListExpr typeInfo, Word& w )
{
  delete static_cast<UGridArea*>( w.addr );
  w.addr = 0;
}

/*
2.2.6.10   ~Clone~ function of class ugridarea

*/
Word  UGridArea::CloneUGridArea( const ListExpr typeInfo, const Word& w )
{
  UGridArea* p = static_cast<UGridArea*>( w.addr );
  return SetWord( new UGridArea(*p) );
}

/*
2.2.6.11   ~Cast~ function of class ugridarea

   not used
2.2.6.12   ~SizeOfObj~ function of class ugridarea

*/
int  UGridArea::SizeOfObjUGridArea()
{
  return sizeof(UGridArea);
}

/*
2.2.6.13 Type Description of class ugridarea

*/
ListExpr  UGridArea::PropertyUGridArea()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
               nl->StringAtom("Example Type List"),
               nl->StringAtom("List Rep"),
               nl->StringAtom("Example List"),
               nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
               nl->StringAtom("ugridarea"),
               nl->StringAtom("(<x1> <y1> <x2> <y2>)"),
               nl->StringAtom("(-3.2 15.34 123.4 22.4)"),
               nl->StringAtom("x-/y-coord of type double"))));
}

/*
2.2.6.14   ~KindCheck~ function of class ugridarea

*/
bool  UGridArea::KindCheckUGridArea( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, UGRIDAREA ));
}

/*
2.2.6.15 Creation of TypeConstructor instance of class ugridarea

*/
TypeConstructor ugridareaTC(
  UGRIDAREA,                        // name of the type in SECONDO
  UGridArea::PropertyUGridArea,     // property function describing signature
  UGridArea::OutUGridArea,          // Out functions
  UGridArea::InUGridArea,           // In functions
  0, 0,                             // SaveToList, RestoreFromList functions
  UGridArea::CreateUGridArea,       // object creation 
  UGridArea::DeleteUGridArea,       // object deletion
  UGridArea::OpenUGridArea,         // open object
  UGridArea::SaveUGridArea,         // save object 
  UGridArea::CloseUGridArea,        // close
  UGridArea::CloneUGridArea,        // clone
  0,                                // cast function
  UGridArea::SizeOfObjUGridArea,    // sizeof function
  UGridArea::KindCheckUGridArea );  // kind checking function

/*
2.3          Class UpdateUnit
2.3.1        Public methods
2.3.2        Constructors and destructor

create - Constructor ~updateunit~

*/
UpdateUnit::UpdateUnit( const int& Id, const MobPos& Pos)
{
  id = Id;
  DateTime t1(instanttype); 
  t1.Today();
  time = t1;
  pos = Pos;
}

UpdateUnit::UpdateUnit( const int& Id, const Instant& Time, const MobPos& Pos)
{
  id = Id;
  time = Time;
  pos = Pos;
}
UpdateUnit::UpdateUnit( const UpdateUnit& uu )
{
  id = uu.id; 
  time = uu.time; 
  pos = uu.pos;
}
/*
2.3.3        methods

*/
int UpdateUnit::GetUId()   const { return id; }

MobPos UpdateUnit::GetPos() const { 
	 return pos; }

double UpdateUnit::GetXPos() const { 
	return pos.GetX(); }
double UpdateUnit::GetYPos() const{ 
	return pos.GetY(); }
Instant UpdateUnit::GetTime() const {
	return time;}
/*
2.3.4       static  methods
  
*/
bool isEqual ( double low1, double high1)
{
  if ( high1 == low1 ) 
    return false; 
  else 
    return true;
}
/*
2.3.5       private methods
2.3.6       List Representation
2.3.6.1    ~In~ function of class UpdateUnit

*/
Word UpdateUnit::InUpdateUnit( const ListExpr typeInfo,
                const ListExpr instance,
                const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = false;
  Instant uutime = Instant (uutime);
  Word result = SetWord(Address(0));
  string errMsg = "InUpdateUnit0:Error defining UpdateUnit!";
  if ( nl->ListLength(instance) == 2 ) 
  {  
    if (nl->IsAtom(nl->First(instance)) && nl->AtomType(nl->First(instance))
		== IntType)
    {
      int id = nl->IntValue( nl->First(instance));
      if ( nl->ListLength(nl->Second(instance)) == 2)
	  {
	    ListExpr position = nl->Second(instance); 
	    if (nl->IsAtom( nl->First( position ) ) &&
            nl->AtomType( nl->First( position) ) == RealType &&
            nl->IsAtom( nl->Second( position ) ) &&
            nl->AtomType( nl->Second( position ) ) == RealType )
	    {
	      double x = nl->RealValue( nl->First( position));
	      double y = nl->RealValue( nl->Second( position));
	      MobPos pos(x,y);
	      correct = true;
          DateTime uutime(instanttype);
          uutime.Now();
          result.addr = new UpdateUnit (id, uutime, pos);
		  return result;
	    }
	    else
	    {
          errMsg = "InUpdateUnit4:pos in UpdateUnit is not real";
	      cmsg.inFunError(errMsg);
	      return result;
	    }  
      }  
	  else  //nl->ListLength(position) != 2
	  {
	    errMsg = "InUpdateUnit5:ListLength(position)!= 2";
	    cmsg.inFunError(errMsg);
	    return result;
	  }
	}
    else  //First != Inttype
    {
	  errMsg = "InUpdateUnit6:Id!= IntType";
      cmsg.inFunError(errMsg);
      return result;
	}
  } 
  else  //ListLength(instance) != 2
  {
    errMsg = "InUpdateUnit7:ListLength(instance)!= 2";
    cmsg.inFunError(errMsg);
    return result;
  }
}

/*
2.3.6.2   ~Out~ function of class UpdateUnit

*/
ListExpr  UpdateUnit::OutUpdateUnit( ListExpr typeInfo, Word value )
{
  UpdateUnit* upunit = static_cast<UpdateUnit*>(value.addr);
  ListExpr result;
  ListExpr timeList = OutDateTime(nl->TheEmptyList(),
                            SetWord((void*) &upunit->time));
  ListExpr pointsList = nl->TwoElemList(nl->RealAtom( upunit->pos.GetX() ),
	                                    nl->RealAtom( upunit->pos.GetY() ));
  result = nl->ThreeElemList(nl->IntAtom(upunit->GetUId()),
	                         timeList,pointsList);
  return result;
}

/*
2.3.6.3   ~SaveToList~ function of class UpdateUnit


   not used

2.3.6.4   ~RestoreFromList~ function of class UpdateUnit

   not used

2.3.6.5   ~Create~ function of class UpdateUnit

*/

Word UpdateUnit::CreateUpdateUnit( const ListExpr typeInfo )
{
  return SetWord(new UpdateUnit());
}

/*
2.3.6.6   ~Delete~ function of class UpdateUnit

*/
void UpdateUnit::DeleteUpdateUnit( const ListExpr typeInfo, Word& w )
{
  delete static_cast<UpdateUnit*>( w.addr );
  w.addr = 0;
}

/*
2.3.6.7   ~Open~ function of class UpdateUnit

*/
bool UpdateUnit::OpenUpdateUnit( SmiRecord& valueRecord, size_t& offset,
                                 const ListExpr typeInfo,
                                 Word& value ) 
{
  Instant time;
  MobPos pos;
  int id = 0;
  bool ok = true;
  size_t size = sizeof(int); 
  ok = ok && valueRecord.Read( &id, size, offset );
  offset += size;  
  size = sizeof(Instant);
  ok = ok && valueRecord.Read( &time, size, offset );
  offset += size; 
  size = sizeof(double);
  ok = ok && valueRecord.Read( &pos.x, size, offset );
  offset += size;  
  ok = ok && valueRecord.Read( &pos.y, size, offset );
  offset += size;  
  value.addr = new UpdateUnit(id, time,pos);
  return ok;
}

/*
2.3.6.8   ~Save~ function of class UpdateUnit

*/
bool UpdateUnit::SaveUpdateUnit( SmiRecord& valueRecord, size_t& offset, 
                                const ListExpr typeInfo, Word& value )
{
  UpdateUnit* uu = static_cast<UpdateUnit*>( value.addr );	
  size_t size = sizeof(int); 	
  bool ok = true;
  ok = ok && valueRecord.Write( &uu->id, size, offset );
  offset += size;  
  size = sizeof(Instant);	
  ok = ok && valueRecord.Write( &uu->time, size, offset );	
  offset += size;  
  size = sizeof(double);
  ok = ok && valueRecord.Write( &uu->pos.x, size, offset );	
  offset += size;  
  ok = ok && valueRecord.Write( &uu->pos.y, size, offset );	
  offset += size;  
  return ok; 
}

/*
2.3.6.9   ~Close~ function of class UpdateUnit

*/
void UpdateUnit::CloseUpdateUnit( const ListExpr typeInfo, Word& w )
{
  delete static_cast<UpdateUnit*>( w.addr );
  w.addr = 0;
}

/*
2.3.6.10   ~Clone~ function of class UpdateUnit

*/
Word UpdateUnit::CloneUpdateUnit( const ListExpr typeInfo, const Word& w )
{
  UpdateUnit* uu = static_cast<UpdateUnit*>( w.addr );
  return SetWord( new UpdateUnit(*uu));
}

/*
2.3.6.11   ~Cast~ function of class UpdateUnit

   not used

2.3.6.12   ~SizeOfObj~ function of class UpdateUnit

*/
int   UpdateUnit::SizeOfObjUpdateUnit()
{
  return sizeof(UpdateUnit);
}
 
/*
2.3.6.13 Type Description of class UpdateUnit

*/
ListExpr  UpdateUnit::PropertyUpdateUnit()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
               nl->StringAtom("Example Type List"),
               nl->StringAtom("List Rep"),
               nl->StringAtom("Example List"),
               nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
               nl->StringAtom("updateunit"),
               nl->StringAtom("(id <x1> <y1> timestamp)"),
               nl->StringAtom("(12(-3.2 15.34) 2003-01-01"),
               nl->StringAtom("id,x-/y-coord timestamp"))));
}

/*
2.3.6.14   ~KindCheck~ function of class UpdateUnit

*/
bool  UpdateUnit::KindCheckUpdateUnit( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, UPDATEUNIT ));
}

/*
2.3.6.15 Creation of TypeConstructor instance of class updateunit

*/
TypeConstructor updateunitTC(
  UPDATEUNIT,                         // name of the type in SECONDO
  UpdateUnit::PropertyUpdateUnit,     // property function describing signature
  UpdateUnit::OutUpdateUnit,          // Out functions
  UpdateUnit::InUpdateUnit,           // In functions
  0, 0,                               // SaveToList, RestoreFromList functions
  UpdateUnit::CreateUpdateUnit,       // object creation 
  UpdateUnit::DeleteUpdateUnit,       // object deletion
  UpdateUnit::OpenUpdateUnit,         // open object 
  UpdateUnit::SaveUpdateUnit,         // save object 
  UpdateUnit::CloseUpdateUnit,        // close
  UpdateUnit::CloneUpdateUnit,        // clone
  0,                                  // cast function
  UpdateUnit::SizeOfObjUpdateUnit,    // sizeof function
  UpdateUnit::KindCheckUpdateUnit);   // kind checking function

/*
2.4         Class CurrentUnit
2.4.1         Public methods
2.4.2         Constructors and destructor

*/
CurrentUnit::CurrentUnit( int CId, Interval<Instant> CTimeInterval,
						  MobPos Cpos, MobPos Npos)
{
  cid = CId; 
  cTimeInterval = CTimeInterval;
  timestamp = InstantToLong(CTimeInterval.start);
  cpos = Cpos;
  npos = Npos;
}

CurrentUnit::CurrentUnit( const CurrentUnit& cu )
{
  cid = cu.cid; 
  cTimeInterval = cu.cTimeInterval; 
  timestamp = InstantToLong(cTimeInterval.start);
  cpos = cu.cpos;
  npos = cu.npos;
}
/*
2.4.3         methods

*/
int CurrentUnit::GetCId()   const { return cid; }

MobPos CurrentUnit::GetCPos() const 
{ 
	 return cpos; 
}
MobPos CurrentUnit::GetNPos() const 
{ 
	 return npos; 
}
double CurrentUnit::GetCXPos() const 
{ 
	return cpos.x; 
}
double CurrentUnit::GetCYPos() const
{ 
	return cpos.y; 
}
double CurrentUnit::GetNXPos() const 
{ 
	return npos.x; 
}
double CurrentUnit::GetNYPos() const
{ 
	return npos.y; 
}
/*
2.4.4         static methods
2.4.5         private methods
2.4.6         List Representation
2.4.6.1   ~In~ function of class CurrentUnit

*/
Word CurrentUnit::InCurrentUnit(const ListExpr typeInfo,
                                const ListExpr instance,
                                const int errorPos, ListExpr& errorInfo,
                                bool& correct )
{
  correct = false;
  Interval<Instant> cuinterval;
  Word result = SetWord(Address(0));
  string errMsg = "InCurrentUnit0:Error in defining CurrentUnit!";
  if ( nl->ListLength(instance) != 4 ) 
  {
	errMsg = "CurrentUnit1:nl->ListLength(instance) != 4";
    cmsg.inFunError(errMsg);
    return result;
  }
  else
  {
  ListExpr ident = nl->First(instance);         //id in nestedList
  ListExpr timeinterval = nl->Second(instance); //time in nestedList
  ListExpr cposition = nl->Third(instance);     //current position
  ListExpr nposition = nl->Fourth(instance);    //next position in nestedList
  if (nl->IsAtom(ident) && nl->AtomType(ident) == IntType)
  {
    int id = nl->IntValue(ident);
    if ( nl->ListLength(timeinterval) != 4)
    {
        errMsg = "InCurrentUnit2:ListLength(timeint)!=4";
        cmsg.inFunError(errMsg);
        return result;
    }
    else  // timeintervall
    {
    if  (nl->IsAtom( nl->Third( timeinterval ) ) &&
         nl->AtomType( nl->Third( timeinterval ) ) == BoolType &&
		     nl->IsAtom( nl->Fourth( timeinterval ) ) &&
		     nl->AtomType( nl->Fourth( timeinterval ) ) == BoolType &&
			 nl->IsAtom( nl->First( timeinterval ) ) &&
		     nl->AtomType( nl->First( timeinterval ) ) == StringType &&
		     nl->IsAtom( nl->Second( timeinterval ) ) &&
		     nl->AtomType( nl->Second( timeinterval ) ) == StringType )
        {
          correct = true;
		  bool correct;
          Instant *start = (Instant*)InInstant(nl->TheEmptyList(),
                            nl->First(timeinterval),errorPos,errorInfo,
							correct).addr;
          if (!correct) 
		  {
            cerr << "currentunit interval invalid start time" << endl;
            return result;
          }
          Instant *end = (Instant*)InInstant(nl->TheEmptyList(),
                          nl->Second(timeinterval),errorPos,errorInfo,
						  correct ).addr;
          if (!correct) 
		  {
            cerr << "uregion interval invalid end time" << endl;
            correct = false;
            delete start;
            return result;
          }
          bool lc = nl->BoolValue(nl->Third(timeinterval));
          bool rc = nl->BoolValue(nl->Fourth(timeinterval)); 
          if (end->ToDouble() < start->ToDouble()
              && !(lc && rc)) 
		  {
            cerr << "uregion invalid interval" << endl;
            delete start;
            delete end;
            return result;
          }
		  Interval<Instant> tinterval(*start, *end, lc, rc);
		  cuinterval = tinterval;
		} // end if atom is timeinterval
		if ( nl->ListLength(cposition) == 2 &&
			 nl->ListLength(nposition) == 2)
		{
		  if (nl->IsAtom( nl->First( cposition ) ) &&
              nl->AtomType( nl->First( cposition) ) == RealType &&
              nl->IsAtom( nl->Second( cposition ) ) &&
              nl->AtomType( nl->Second( cposition ) ) == RealType &&
			  nl->IsAtom( nl->First( nposition ) ) &&
			  nl->AtomType( nl->First( nposition) ) == RealType &&
			  nl->IsAtom( nl->Second( nposition ) ) &&
			  nl->AtomType( nl->Second( nposition ) ) == RealType )
		  {
		  double cx = nl->RealValue( nl->First( cposition));
			double cy = nl->RealValue( nl->Second( cposition));
			MobPos cpos(cx,cy);
			double nx = nl->RealValue( nl->First( nposition));
			double ny = nl->RealValue( nl->Second( nposition));
			MobPos npos(nx,ny);
			correct = true;
      result.addr = new CurrentUnit(id, cuinterval, cpos, npos);
      return result;
		  }
		  else
		  {
			errMsg = "InCurrentUnit:Error in position, no real";
		    cmsg.inFunError(errMsg);
		    return result;
		  }
	    }  // end if nl->ListLength(position) = 2 
		else
		{
		  errMsg = "InCurrentUnit:ListLength(position) != 2";
		  cmsg.inFunError(errMsg);
		  return result;
		}
      }  // end else timeinterval
	}  // end if atom is integer
    else
    {	
      errMsg = "InCurrentUnit:Id not integer";
      cmsg.inFunError(errMsg);
      return result;
    } 
  } // end else 4 elem list
    errMsg = "InCurrentUnit:no four element list";
    cmsg.inFunError(errMsg);
    return result;
}

/*
2.4.6.2   ~Out~ function of class CurrentUnit

*/
ListExpr  CurrentUnit::OutCurrentUnit( ListExpr typeInfo, Word value )
{
  CurrentUnit* cuunit = (CurrentUnit*)(value.addr);
  ListExpr timeintervalList = nl->FourElemList(
      OutDateTime( nl->TheEmptyList(), SetWord(&cuunit->cTimeInterval.start)),
      OutDateTime( nl->TheEmptyList(), SetWord(&cuunit->cTimeInterval.end) ),
      nl->BoolAtom( cuunit->cTimeInterval.lc ),
      nl->BoolAtom( cuunit->cTimeInterval.rc));

  ListExpr cpointsList = nl->TwoElemList(
      nl->RealAtom( cuunit->cpos.GetX() ),
	  nl->RealAtom( cuunit->cpos.GetY() ));

  ListExpr npointsList = nl->TwoElemList(
      nl->RealAtom( cuunit->npos.GetX() ),
	  nl->RealAtom( cuunit->npos.GetY() ));

  return nl->FourElemList( nl->IntAtom(cuunit->GetCId()),
	         timeintervalList, cpointsList, npointsList );
}

/*
2.4.6.3   ~SaveToList~ function of class CurrentUnit

   not used

2.4.6.4   ~RestoreFromList~ function of class CurrentUnit

   not used

2.4.6.5   ~Create~ function of class CurrentUnit

*/
Word CurrentUnit::CreateCurrentUnit( const ListExpr typeInfo )
{
  return SetWord(new CurrentUnit());
}

/*
2.4.6.6   ~Delete~ function of class CurrentUnit

*/
void CurrentUnit::DeleteCurrentUnit( const ListExpr typeInfo, Word& w )
{
  delete static_cast<CurrentUnit*>( w.addr );
  w.addr = 0;
}

/*
2.4.6.7   ~Open~ function of class CurrentUnit

*/
bool CurrentUnit::OpenCurrentUnit( SmiRecord& valueRecord, 
                                   size_t& offset, const ListExpr typeInfo, 
                                   Word& value ) 
{
  size_t size = sizeof(int); 	
  DateTime t1(instanttype), t2(instanttype);
  Interval<Instant>  time  = Interval<Instant>(t1, t2, true, true);	
  MobPos cpos, npos;
  int id = 0;
  bool ok = true;
  ok = ok && valueRecord.Read( &id, size, offset );
  offset += size;  
  size = sizeof(Interval<Instant> );
  ok = ok && valueRecord.Read( &time, size, offset );
  offset += size;  
  size = sizeof(double);
  ok = ok && valueRecord.Read( &cpos.x, size, offset );
  offset += size;  
  ok = ok && valueRecord.Read( &cpos.y, size, offset );
  offset += size;  
  ok = ok && valueRecord.Read( &npos.x, size, offset );
  offset += size;  
  ok = ok && valueRecord.Read( &npos.y, size, offset );
  offset += size;  
  value.addr = new CurrentUnit(id,time,cpos,npos); 
  return ok;
}	

/*
2.4.6.8   ~Save~ function of class CurrentUnit

*/
bool CurrentUnit::SaveCurrentUnit( SmiRecord& valueRecord, size_t& offset, 
                  const ListExpr typeInfo, Word& value )
{
  CurrentUnit* c = static_cast<CurrentUnit*>( value.addr );	
  size_t size = sizeof(int); 	

  bool ok = true;
  ok = ok && valueRecord.Write( &c->cid, size, offset );
  offset += size;  
  size = sizeof(Interval<Instant> ); 	
  ok = ok && valueRecord.Write( &c->cTimeInterval, size, offset );	
  offset += size;  
  size = sizeof(double);
  ok = ok && valueRecord.Write( &c->cpos.x, size, offset );	
  offset += size;  
  ok = ok && valueRecord.Write( &c->cpos.y, size, offset );	
  offset += size;
  ok = ok && valueRecord.Write( &c->npos.x, size, offset );	
  offset += size;  
  ok = ok && valueRecord.Write( &c->npos.y, size, offset );	
  offset += size;
  return ok;
}

/*
2.4.6.9   ~Close~ function of class CurrentUnit

*/
void CurrentUnit::CloseCurrentUnit( const ListExpr typeInfo, Word& w )
{
  delete static_cast<CurrentUnit*>( w.addr );
  w.addr = 0;
}

/*
2.4.6.10   ~Clone~ function of class CurrentUnit

*/

Word CurrentUnit::CloneCurrentUnit( const ListExpr typeInfo, const Word& w )
{
  CurrentUnit* cu = static_cast<CurrentUnit*>( w.addr );
  return SetWord( new CurrentUnit(*cu));
}

/*
2.4.6.11   ~Cast~ function of class CurrentUnit

   not used

2.4.6.12   ~SizeOfObj~ function of class CurrentUnit

*/
int   CurrentUnit::SizeOfObjCurrentUnit()
{
  return sizeof(CurrentUnit);
}

/*
2.4.6.13 Type Description of class CurrentUnit

*/
ListExpr  CurrentUnit::PropertyCurrentUnit()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
               nl->StringAtom("Example Type List"),
               nl->StringAtom("List Rep"),
               nl->StringAtom("Example List"),
               nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
               nl->StringAtom("currentunit"),
               nl->StringAtom("(<id> <timeinterval> <cpos> <npos>)"),
               nl->StringAtom("(4(2008-10-10:08:00 infinite true false)"),
               nl->StringAtom("id,current time, current position"))));
}

/*
2.4.6.14   ~KindCheck~ function of class CurrentUnit

*/
bool  CurrentUnit::KindCheckCurrentUnit( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, CURRENTUNIT ));
}

/*
2.4.6.15 Creation of TypeConstructor instance of class updateunit

*/
TypeConstructor currentunitTC(
  CURRENTUNIT,                        // name of the type in SECONDO
  CurrentUnit::PropertyCurrentUnit,   // property function describing signature
  CurrentUnit::OutCurrentUnit,        // Out functions
  CurrentUnit::InCurrentUnit,         // In functions
  0, 0,                               // SaveToList, RestoreFromList functions
  CurrentUnit::CreateCurrentUnit,     // object creation 
  CurrentUnit::DeleteCurrentUnit,     // object deletion
  CurrentUnit::OpenCurrentUnit,       // open object 
  CurrentUnit::SaveCurrentUnit,       // save object 
  CurrentUnit::CloseCurrentUnit,      // close
  CurrentUnit::CloneCurrentUnit,      // clone
  0,                                  // cast function
  CurrentUnit::SizeOfObjCurrentUnit,  // sizeof function
  CurrentUnit::KindCheckCurrentUnit); // kind checking function
/*
2.5         Class HistoryUnit
2.5.1        public methods
2.5.2       Constructors and destructor

*/
HistoryUnit::HistoryUnit( int HId, Interval<Instant> HTimeInterval, 
						  MobPos Spos, MobPos Epos)
{
  hid = HId; 
  htimeInterval = HTimeInterval;
  htimestart = InstantToLong(HTimeInterval.start);
  htimeend = InstantToLong(HTimeInterval.end);
  spos = Spos;
  epos = Epos;
}
HistoryUnit::HistoryUnit( int HId, long HuSTime, long HuETime, 
						  MobPos Spos, MobPos Epos)
{
  hid = HId; 
  htimestart = HuSTime;
  htimeend = HuETime;
  htimeInterval.start = LongToInstant(htimestart);
  htimeInterval.end = LongToInstant(htimeend);
  htimeInterval = Interval<Instant>(htimeInterval.start, 
	                                htimeInterval.end, true, false);
  spos = Spos;
  epos = Epos;
}

HistoryUnit::HistoryUnit( const HistoryUnit& hu )
{
  hid = hu.hid; 
  htimeInterval = hu.htimeInterval; 
  htimestart = hu.htimestart;
  htimeend = hu.htimeend;
  spos = hu.spos;
  epos = hu.epos;
}
/*
2.5.3        methods

*/
int HistoryUnit::GetHId()   const { return hid; }

MobPos HistoryUnit::GetSPos() const 
{ 
	 return spos; 
}
MobPos HistoryUnit::GetEPos() const 
{ 
	 return epos; 
}
double HistoryUnit::GetSXPos() const 
{ 
	return spos.x; 
}
double HistoryUnit::GetSYPos() const
{ 
	return spos.y; 
}
double HistoryUnit::GetEXPos() const 
{ 
	return epos.x; 
}
double HistoryUnit::GetEYPos() const
{ 
	return epos.y; 
}
long HistoryUnit::GetHuTimeStart() const
{ 
	return htimestart; 
}
long HistoryUnit::GetHuTimeEnd() const
{ 
	return htimeend; 
}

int HistoryUnit::SizeOfHistoryUnit()
{
  return sizeof (int ) + // Id of moving object
         sizeof( MobPos ) +  // Startposition
         sizeof( MobPos ) +  // Endposition
         sizeof( Interval<Instant> );  // Timeinterval of HistoryUnit
}
/*
2.5.4        static methods
2.5.5        private methods
2.5.6        List Representation
2.5.6.1   ~In~ function of class HistoryUnit

*/
Word HistoryUnit::InHistoryUnit(const ListExpr typeInfo,
                                const ListExpr instance,
                                const int errorPos, ListExpr& errorInfo,
                                bool& correct )
{
  correct = false;
  Word result = SetWord(Address(0));
  string errMsg = "InHistoryUnit:Error in defining HistoryUnit!";
  Interval<Instant> huinterval;
  if ( nl->ListLength(instance) != 4 ) 
  {
    cmsg.inFunError(errMsg);
    return result;
  }  
  else  
  {
	ListExpr ident = nl->First(instance);            //id in nestedList
	ListExpr timeinterval = nl->Second(instance);    //time in nestedList
	ListExpr sposition = nl->Third(instance);        //start position in NL
	ListExpr eposition = nl->Fourth(instance);       //end position in NL
    if (nl->IsAtom(ident) && nl->AtomType(ident) == IntType) 
	{
	  int id = nl->IntValue(ident);
	  if ( nl->ListLength(timeinterval) != 4)
	  {
        errMsg = "InHistoryUnit:Error in time of HistoryUnit";
        cmsg.inFunError(errMsg);
        return result;
	  }
	  else  // timeintervall
	  { 
	    if  (nl->IsAtom( nl->Third( timeinterval ) ) &&
		     nl->AtomType( nl->Third( timeinterval ) ) == BoolType &&
		     nl->IsAtom( nl->Fourth( timeinterval ) ) &&
		     nl->AtomType( nl->Fourth( timeinterval ) ) == BoolType &&
			 nl->IsAtom( nl->First( timeinterval ) ) &&
		     nl->AtomType( nl->First( timeinterval ) ) == StringType &&
		     nl->IsAtom( nl->Second( timeinterval ) ) &&
		     nl->AtomType( nl->Second( timeinterval ) ) == StringType )
        {
          correct = true;
		  bool correct;
          Instant *start = (Instant*)InInstant(nl->TheEmptyList(),
                            nl->First(timeinterval),errorPos,
							errorInfo,correct).addr;
          if (!correct) 
		  {
            cerr << "historyunit interval invalid start time" << endl;
            return result;
          }
          Instant *end = (Instant*)InInstant(nl->TheEmptyList(),
                        nl->Second(timeinterval),errorPos,errorInfo,
						correct).addr;
          if (!correct) 
		  {
            cerr << "historyunit interval invalid end time" << endl;
            correct = false;
            delete start;
            return result;
          }
          bool lc = nl->BoolValue(nl->Third(timeinterval));
          bool rc = nl->BoolValue(nl->Fourth(timeinterval)); 
          if (end->ToDouble() < start->ToDouble()
              && !(lc && rc)) 
		  {
            cerr << "historyunit invalid interval" << endl;
            delete start;
            delete end;
            return result;
          }
		  Interval<Instant> tinterval(*start, *end, lc, rc);
		  huinterval = tinterval;
		} // end if atom is timeinterval
		if ( nl->ListLength(sposition) != 2)
	    {
		  errMsg = "InHistoryUnit:Error in spos of HistoryUnit";
		  cmsg.inFunError(errMsg);
		  return result;
		}
		else  // spos
		{
		  if (nl->IsAtom( nl->First( sposition ) ) &&
              nl->AtomType( nl->First( sposition) ) == RealType &&
              nl->IsAtom( nl->Second( sposition ) ) &&
              nl->AtomType( nl->Second( sposition ) ) == RealType )
		  {
			double sx = nl->RealValue( nl->First( sposition));
			double sy = nl->RealValue( nl->Second( sposition));
			MobPos spos(sx,sy);
		    if ( nl->ListLength(eposition) != 2)
	        {
			  errMsg = "InHistoryUnit:Error in spos of HistoryUnit";
			  cmsg.inFunError(errMsg);
			  return result;
		    }
		    else  
		    {
		      if (nl->IsAtom( nl->First( eposition ) ) &&
              nl->AtomType( nl->First( eposition) ) == RealType &&
              nl->IsAtom( nl->Second( eposition ) ) &&
              nl->AtomType( nl->Second( eposition ) ) == RealType )
          {
          double ex = nl->RealValue( nl->First( eposition));
          double ey = nl->RealValue( nl->Second( eposition));
          MobPos epos(ex,ey);
          correct = true;
          result.addr = new HistoryUnit(id, huinterval, spos, epos);
          return result;
          }
          else
			   {
                errMsg = "InHistoryUnit:Error in epos of HistoryUnit";
			    cmsg.inFunError(errMsg);
			    return result;
		      } // end endposition is o.k.
		    } // end else nl->ListLength(eposition) != 2
		  }
		  else  // startposition isn't two real type
		  {
		    errMsg = "InHistoryUnit:Error in spos of HistoryUnit";
			cmsg.inFunError(errMsg);
			return result;
		  }
        } // end else nl->ListLength(sposition) != 2
	  }  // end if nl->ListLength(timeinterval) != 4
	}// end if Id is integer
    else
    {	  
      errMsg = "InHistoryUnit:Error in Id of HistoryUnit!";
      cmsg.inFunError(errMsg);
      return result;
    } 
  } // end else nl->ListLength(instance) != 4
}

/*
2.5.6.2   ~Out~ function of class HistoryUnit

*/
ListExpr  HistoryUnit::OutHistoryUnit( ListExpr typeInfo, Word value )
{
  HistoryUnit* hiunit = (HistoryUnit*)(value.addr);
  ListExpr timeintervalList = nl->FourElemList(
      OutDateTime( nl->TheEmptyList(),SetWord(&hiunit->htimeInterval.start) ),
      OutDateTime( nl->TheEmptyList(), SetWord(&hiunit->htimeInterval.end)),
      nl->BoolAtom( hiunit->htimeInterval.lc ),
      nl->BoolAtom( hiunit->htimeInterval.rc));

  ListExpr spointsList = nl->TwoElemList(
      nl->RealAtom( hiunit->spos.GetX() ),
	  nl->RealAtom( hiunit->spos.GetY() ));

  ListExpr epointsList = nl->TwoElemList(
      nl->RealAtom( hiunit->epos.GetX() ),
	  nl->RealAtom( hiunit->epos.GetY() ));

  return nl->FourElemList( nl->IntAtom(hiunit->GetHId()),
	         timeintervalList, spointsList, epointsList );
}

/*
2.5.6.3   ~SaveToList~ function of class HistoryUnit

   not used

2.5.6.4   ~RestoreFromList~ function of class HistoryUnit

   not used

2.5.6.5   ~Create~ function of class HistoryUnit

*/
Word HistoryUnit::CreateHistoryUnit( const ListExpr typeInfo )
{
  return SetWord(new HistoryUnit());
}

/*
2.5.6.6   ~Delete~ function of class HistoryUnit

*/
void HistoryUnit::DeleteHistoryUnit( const ListExpr typeInfo, Word& w )
{
  delete static_cast<HistoryUnit*>( w.addr );
  w.addr = 0;
}

/*
2.5.6.7   ~Open~ function of class HistoryUnit

*/
bool HistoryUnit::OpenHistoryUnit( SmiRecord& valueRecord, 
                  size_t& offset, const ListExpr typeInfo, 
                  Word& value ) 
{
  DateTime t1(instanttype), t2(instanttype);
  Interval<Instant>  time  = Interval<Instant>(t1, t2, true, true);	
  MobPos spos, epos;
  int id = 0;
  bool ok = true;
  size_t size = sizeof(int);
  ok = ok && valueRecord.Read( &id, size, offset );
  offset += size;  
  size = sizeof(Interval<Instant>);
  ok = ok && valueRecord.Read( &time, size, offset );
  offset += size;  
  size = sizeof(double);
  ok = ok && valueRecord.Read( &spos.x, size, offset );
  offset += size;  
  ok = ok && valueRecord.Read( &spos.y, size, offset );
  offset += size;  
  ok = ok && valueRecord.Read( &epos.x, size, offset );
  offset += size;  
  ok = ok && valueRecord.Read( &epos.y, size, offset );
  offset += size;  
  value.addr = new HistoryUnit(id, time,spos,epos); 
  return ok;
}	

/*
2.5.6.8   ~Save~ function of class CurrentUnit

*/
bool HistoryUnit::SaveHistoryUnit( SmiRecord& valueRecord, size_t& offset, 
                  const ListExpr typeInfo, Word& value )
{
  HistoryUnit* r = static_cast<HistoryUnit*>( value.addr );	
  size_t size = sizeof(int); 	
  bool ok = true;
  ok = ok && valueRecord.Write( &r->hid, size, offset );
  offset += size;  
  size = sizeof(Interval<Instant>); 	
  ok = ok && valueRecord.Write( &r->htimeInterval, size, offset );	
  offset += size; 
  size = sizeof(double); 
  ok = ok && valueRecord.Write( &r->spos.x, size, offset );	
  offset += size;  
  ok = ok && valueRecord.Write( &r->spos.y, size, offset );	
  offset += size;  
  ok = ok && valueRecord.Write( &r->epos.x, size, offset );	
  offset += size;  
  ok = ok && valueRecord.Write( &r->epos.y, size, offset );	
  offset += size;  

  return ok;
}	

/*
2.5.6.9   ~Close~ function of class HistoryUnit

*/
void HistoryUnit::CloseHistoryUnit( const ListExpr typeInfo, Word& w )
{
  delete static_cast<HistoryUnit*>( w.addr );
  w.addr = 0;
}

/*
2.5.6.10   ~Clone~ function of class HistoryUnit

*/
Word HistoryUnit::CloneHistoryUnit( const ListExpr typeInfo, const Word& w )
{
  HistoryUnit* cu = static_cast<HistoryUnit*>( w.addr );
  return SetWord( new HistoryUnit(*cu));
}

/*
2.5.6.11   ~Cast~ function of class HistoryUnit

   not used

2.5.6.12   ~SizeOfObj~ function of class HistoryUnit

*/
int   HistoryUnit::SizeOfObjHistoryUnit()
{
  return sizeof(HistoryUnit);
}

/*
2.5.6.13 Type Description of class HistoryUnit

*/
ListExpr  HistoryUnit::PropertyHistoryUnit()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
               nl->StringAtom("Example Type List"),
               nl->StringAtom("List Rep"),
               nl->StringAtom("Example List"),
               nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
               nl->StringAtom("historyunit"),
               nl->StringAtom("(<id> <timeinterval> <spos> <epos>)"),
               nl->StringAtom("(4(2008-10-10:08:00 infinite true false)"),
               nl->StringAtom("id,tinterval, start/end position"))));
}

/*
2.5.6.14   ~KindCheck~ function of class HistoryUnit

*/
bool  HistoryUnit::KindCheckHistoryUnit( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, HISTORYUNIT ));
}

/*
2.5.6.15  Creation of TypeConstructor instance of class historyunit

*/
TypeConstructor historyunitTC(
  HISTORYUNIT,                        // name of the type in SECONDO
  HistoryUnit::PropertyHistoryUnit,   // property function describing signature
  HistoryUnit::OutHistoryUnit,        // Out functions
  HistoryUnit::InHistoryUnit,         // In functions
  0, 0,                               // SaveToList, RestoreFromList functions
  HistoryUnit::CreateHistoryUnit,     // object creation 
  HistoryUnit::DeleteHistoryUnit,     // object deletion
  HistoryUnit::OpenHistoryUnit,       // open object 
  HistoryUnit::SaveHistoryUnit,       // save object 
  HistoryUnit::CloseHistoryUnit,      // close
  HistoryUnit::CloneHistoryUnit,      // clone
  0,                                  // cast function
  HistoryUnit::SizeOfObjHistoryUnit,  // sizeof function
  HistoryUnit::KindCheckHistoryUnit); // kind checking function
/*
2.6         Class UGridCell
2.6.1        public methods
2.6.2        Constructors and destructor

*/
UGridCell::UGridCell( int labelIndex, dimsize dim, UGridArea area) :
                      intlabel(0), ucarea(area), noOfEntries (0),
                      modified(false), firstEntry (DateTime(instanttype)),
                      lastEntry (DateTime(instanttype))
{ 
  firstTime = LongTime();
  lastTime = LongTime();
  intlabel = ((labelIndex / dim)*100) + (labelIndex % dim);
  noOfEntries = 0;
  modified = false;
}

UGridCell::UGridCell(int intLabel, UGridArea area, int nofEntries, long t1,
                     long t2, bool modified,
                     HistoryUnit HistoryArray[sizeHistoryArray]):
                     intlabel(intLabel), ucarea(area), noOfEntries(nofEntries),
                     firstTime(t1), lastTime(t2), modified(modified)
{
  Instant firstEntry = LongToInstant(firstTime);
  Instant lastEntry = LongToInstant(lastTime);
  ugcInterval.start = firstEntry;
  ugcInterval.end = lastEntry;
  ugcInterval.lc = true;
  ugcInterval.rc = false;
  for (int i = 0; i < noOfEntries; i ++)
  {
    historyArray[ i ].hid = HistoryArray[ i ].hid;
    historyArray[ i ].spos.x = HistoryArray[ i ].spos.x;
    historyArray[ i ].spos.y = HistoryArray[ i ].spos.y;
    historyArray[ i ].epos.x = HistoryArray[ i ].epos.x;
    historyArray[ i ].epos.y = HistoryArray[ i ].epos.y;	
    historyArray[ i ].htimestart = HistoryArray[ i ].htimestart;
    historyArray[ i ].htimeend = HistoryArray[ i ].htimeend;
	Instant firstEntry = LongToInstant(historyArray[ i ].htimestart);
    Instant lastEntry = LongToInstant(historyArray[ i ].htimeend);
    historyArray[ i ].htimeInterval.start = firstEntry;
	historyArray[ i ].htimeInterval.end = lastEntry;
	historyArray[ i ].htimeInterval.lc = true;
	historyArray[ i ].htimeInterval.rc = false;
  }
}
UGridCell::UGridCell(int intLabel, UGridArea area, int nofEntries,
                     Instant int1, Instant int2, bool modified,
                     HistoryUnit HistoryArray[sizeHistoryArray]):
                     intlabel(intLabel), ucarea(area), noOfEntries(nofEntries),
                     modified(modified)
{
  ugcInterval.start = int1;
  ugcInterval.end = int2;
  ugcInterval.lc = true;
  ugcInterval.rc = false;
  firstTime = InstantToLong(ugcInterval.start);
  lastTime = InstantToLong(ugcInterval.end);
  for (int i = 0; i < noOfEntries; i ++)
  {
    historyArray[ i ].hid = HistoryArray[ i ].hid;
    historyArray[ i ].spos.x = HistoryArray[ i ].spos.x;
    historyArray[ i ].spos.y = HistoryArray[ i ].spos.y;
    historyArray[ i ].epos.x = HistoryArray[ i ].epos.x;
    historyArray[ i ].epos.y = HistoryArray[ i ].epos.y;	
    historyArray[ i ].htimestart = HistoryArray[ i ].htimestart;
    historyArray[ i ].htimeend = HistoryArray[ i ].htimeend;
	Instant firstEntry = LongToInstant(historyArray[ i ].htimestart);
    Instant lastEntry = LongToInstant(historyArray[ i ].htimeend);
    Interval<Instant> timeInterval = 
		   Interval<Instant>(firstEntry, lastEntry, true, false);
    historyArray[ i ].htimeInterval = timeInterval;
  }
} 

UGridCell::UGridCell(const UGridCell& ugridcell):
    intlabel(ugridcell.intlabel), ucarea(ugridcell.ucarea),
    noOfEntries(ugridcell.noOfEntries), firstTime(ugridcell.firstTime),
    lastTime(ugridcell.lastTime), modified(ugridcell.modified)
 {
   Instant firstEntry = LongToInstant(firstTime);
   Instant lastEntry = LongToInstant(lastTime);
   Interval<Instant> timeInterval(firstEntry, lastEntry, true, false);
   ugcInterval = timeInterval;
   for (int i = 0; i < noOfEntries; i ++)
   {
     historyArray[ i ].hid = ugridcell.historyArray[ i ].hid;
     historyArray[ i ].spos.x = ugridcell.historyArray[ i ].spos.x;
     historyArray[ i ].spos.y = ugridcell.historyArray[ i ].spos.y;
     historyArray[ i ].epos.x = ugridcell.historyArray[ i ].epos.x;
     historyArray[ i ].epos.y = ugridcell.historyArray[ i ].epos.y;	
     historyArray[ i ].htimestart = ugridcell.historyArray[ i ].htimestart;
     historyArray[ i ].htimeend = ugridcell.historyArray[ i ].htimeend;
     Instant firstEntry = LongToInstant(historyArray[ i ].htimestart);
     Instant lastEntry = LongToInstant(historyArray[ i ].htimeend);
     Interval<Instant> timeInterval(firstEntry, lastEntry, true, false);
     historyArray[ i ].htimeInterval = timeInterval;    
   }
 }

 UGridCell::UGridCell(int ind, SmiRecordId cellRecId, 
	                  const UGridCell& ugridcell) 
 {
     cellArray[ind ].cellRecId = cellRecId;
	 cellArray[ind ].cellabel = ugridcell.intlabel;
     cellArray[ind].ugcspos.x = ugridcell.ucarea.x1;
     cellArray[ind ].ugcspos.y = ugridcell.ucarea.y1;
     cellArray[ind].ugcepos.x = ugridcell.ucarea.x2;
     cellArray[ind].ugcepos.y = ugridcell.ucarea.y2;	
     cellArray[ind].ugctimestart = ugridcell.firstTime; 
	 cellArray[ind ].ugctimeend = ugridcell.lastTime;        
 }

/*
Destructor ~UGridCell~

*/
UGridCell::~UGridCell(){}
/*
2.6.3        methods

*/
int UGridCell::GetIntLabel() const {return intlabel;}
UGridArea UGridCell::GetArea() const 
{
  UGridArea area;
  area.x1 = ucarea.x1;
  area.y1 = ucarea.y1;
  area.x2 = ucarea.x2;
  area.y2 = ucarea.y2;
  return area;
}
int UGridCell::GetNoOfEntries() const {return noOfEntries;}  
long UGridCell::GetFirstEntry() const {return firstTime;}   // for calculations
long UGridCell::GetLastEntry() const {return lastTime;}     // for calculations
bool UGridCell::IsModified() const {return modified;}

string IntLabelToString(int label)  
{
  string xLabel, yLabel, stringLabel;
  xLabel = intToString(label / 100);      
  yLabel = intToString(label % 100);
  stringLabel = xLabel + ":" + yLabel;	
  return stringLabel;
}

/*
Clears all entries of historyunits.

*/
void UGridCell::ClearHu()
{
  for( int i = 0; i < noOfEntries; i++ )
  {  
    historyArray[ i ].hid = 0;
	historyArray[ i ].spos.x = 0;
	historyArray[ i ].spos.y = 0;
	historyArray[ i ].epos.x = 0;
	historyArray[ i ].epos.y = 0;
    historyArray[ i ].htimestart = time(NULL);
	historyArray[ i ].htimeend = time(NULL);
  }
  noOfEntries = 0;
  firstTime = time(NULL);
  lastTime = time(NULL);
  modified = false;
}
/*
Clears all entries of cellunits.

*/
void UGridCell::ClearCu()
{
  for( int i = 0; i < noOfEntries; i++ )
  {  
    cellArray[ i ].cellRecId = (SmiRecordId)0;
	cellArray[ i ].cellabel = 0;
	cellArray[ i ].ugcspos.x = 0;
	cellArray[ i ].ugcspos.y = 0;
	cellArray[ i ].ugcepos.x = 0;
	cellArray[ i ].ugcepos.y = 0;
    cellArray[ i ].ugctimestart = time(NULL);
	cellArray[ i ].ugctimeend = time(NULL);
  }
  noOfEntries = 0;
  firstTime = time(NULL);
  lastTime = time(NULL);
  modified = false;
}

/*
add historyunit in ugridcell

*/
void UGridCell::AddHistoryUnit(HistoryUnit Hu)
{
    int i = noOfEntries;
	historyArray[ i ].hid = Hu.hid;
    historyArray[ i ].spos.x = Hu.spos.x;
    historyArray[ i ].spos.y = Hu.spos.y;
    historyArray[ i ].epos.x = Hu.spos.x;
    historyArray[ i ].epos.y = Hu.spos.y;
    historyArray[ i ].htimestart = Hu.htimestart;
	historyArray[ i ].htimeend = Hu.htimeend;
    if (historyArray[ i ].htimestart < firstTime)
    {
	  firstTime = historyArray[ i ].htimestart;
    }
    if (historyArray[ i ].htimeend > lastTime)
    {
	  lastTime = historyArray[ i ].htimeend;
    }
	modified = true;
}
/*
2.6.4        static methods
2.6.5       private methods
2.6.6       List Representation
2.6.6.1   ~In~ function of class UGridCell

*/
Word UGridCell::InUGridCell(const ListExpr typeInfo, const ListExpr instance,
                            const int errorPos, ListExpr& errorInfo, 
							bool& correct)
{
  correct = false;
  Word result = SetWord(Address(0));
  string errMsg = "InUGridCell:Error in defining UGridCell!";
  int intlabel;
  int dim;
  if ( nl->ListLength(instance) != 3 ) 
  {
    cmsg.inFunError(errMsg);
    return result;
  }  
  else  
  {
  ListExpr ugcindex = nl->First(instance);        //index in nestedList
  ListExpr dimension = nl->Second(instance);      //dimension in nestedList
  ListExpr area = nl->Third(instance);            //area in nestedList
  if (!(nl->IsAtom(ugcindex) && nl->AtomType(ugcindex) == IntType))
  {
    errMsg = "InUGridCell:Error in defining index";
      cmsg.inFunError(errMsg);
      return result;
	}
	else
	{
      intlabel = nl->IntValue(ugcindex);
	  if (!(nl->IsAtom(dimension) && nl->AtomType(dimension) == IntType))
	  {
	    errMsg = "InUGridCell:Error in defining dimension";
        cmsg.inFunError(errMsg);
        return result;
	  }
	  else
	  {
	    dim = nl->IntValue(dimension);
	    if (dim != 64 && dim != 32 && dim != 16 && 
			dim != 8 && dim != 4 && dim != 2) 
        {
		  errMsg = "InUGridCell:Error in defining dim value!";
          cmsg.inFunError(errMsg);
          return result;
        }  
        else  
        {
		  if ( nl->ListLength(area) != 4)
	      {
		    errMsg = "InUGridCell:Error in area of UGridCell";
		    cmsg.inFunError(errMsg);
		    return result;
		  }
		  else  // area
		  {
		    if (nl->IsAtom( nl->First( area ) ) &&
                nl->AtomType( nl->First( area) ) == RealType &&
                nl->IsAtom( nl->Second( area ) ) &&
                nl->AtomType( nl->Second( area ) ) == RealType &&
			    nl->IsAtom( nl->Third( area ) ) &&
                nl->AtomType( nl->Third( area) ) == RealType &&
                nl->IsAtom( nl->Fourth( area ) ) &&
                nl->AtomType( nl->Fourth( area ) ) == RealType )
		    {
          double ax1 = nl->RealValue( nl->First( area));
          double ay1 = nl->RealValue( nl->Second( area));
          double ax2 = nl->RealValue( nl->Third( area));
          double ay2 = nl->RealValue( nl->Fourth( area));
          if (( ax2 == ax1 ) || (ay1 == ay2 ))   // vert or horizontal line
        {
            errMsg = "vertical or horizontal line or point";
            cmsg.inFunError(errMsg);
            return result;
	      }
			  else
			  {
			    UGridArea ugcarea(ax1,ay1,ax2,ay2);
			    dimsize ugcdim = (dimsize) dim;
          correct = true;
          result.addr = new UGridCell(intlabel, ugcdim, ugcarea);
          return result;
        }
		    }
		    else  // area isn't four real type
		    {
		      errMsg = "InUGridCell:Error in area of UGridCell";
			  cmsg.inFunError(errMsg);
			  return result;
		    }
          } // end else nl->ListLength(area) != 4
	    }  // end if dim element dimsize
	  }// end if dim is int
    } // end if label is int
  } // end else nl->ListLength(instance) != 3
}

/*
2.6.6.2   ~Out~ function of class UGridCell

*/
  ListExpr UGridCell::OutUGridCell(ListExpr typeInfo, Word value )
{
  UGridCell* ugridcell = (UGridCell*)(value.addr);  
  ListExpr timeintervalList = nl->FourElemList(
	OutDateTime( nl->TheEmptyList(), 
	  SetWord(&ugridcell->ugcInterval.start)),
    OutDateTime( nl->TheEmptyList(), 
	  SetWord(&ugridcell->ugcInterval.end)),
    nl->BoolAtom( true ),
    nl->BoolAtom( false));
  ListExpr areaList = nl->FourElemList(
	nl->RealAtom( ugridcell->ucarea.GetX1() ),
	nl->RealAtom( ugridcell->ucarea.GetY1()),
	nl->RealAtom( ugridcell->ucarea.GetX2() ),
	nl->RealAtom( ugridcell->ucarea.GetY2() ));
  ListExpr entriesList = nl->TwoElemList(
	nl->IntAtom(ugridcell->GetIntLabel()),
    nl->IntAtom(ugridcell->GetNoOfEntries()));
	  
  return nl->FourElemList( entriesList, areaList, timeintervalList,
	                       nl->BoolAtom(ugridcell->IsModified())); 
}

/*
2.6.6.3   ~SaveToList~ function of class UGridCell

   not used

2.6.6.4   ~RestoreFromList~ function of class UGridCell

   not used

2.6.6.5   ~Create~ function of class UGridCell

*/
Word     UGridCell::CreateUGridCell( const ListExpr typeInfo )
{
  return SetWord(new UGridCell());
}

/*
2.6.6.6   Implementations ~Delete~ function of class UGridCell

*/
void     UGridCell::DeleteUGridCell( const ListExpr typeInfo, Word& w )
{ 
  delete static_cast<UGridCell*>( w.addr );
  w.addr = 0;
}

/*
2.6.6.7   ~Open~ function of class UGridCell

*/
   bool UGridCell::OpenUGridCell( SmiRecord& valueRecord, 
                      size_t& offset, const ListExpr typeInfo, 
                      Word& value ) 
{
  int intLabel;
  UGridArea area;
  int nofEntries;
  long t1, t2;
  Instant int1, int2;
  HistoryUnit historyArray[sizeHistoryArray];
  bool modified;
  bool ok = true;
  size_t size = sizeof(int);
  ok = ok && valueRecord.Read( &intLabel, size, offset );
  offset += size;  
  size = sizeof(double);
  ok = ok && valueRecord.Read( &area.x1, size, offset );
  offset += size;  
  ok = ok && valueRecord.Read( &area.y1, size, offset );
  offset += size;  
  ok = ok && valueRecord.Read( &area.x2, size, offset );
  offset += size;  
  ok = ok && valueRecord.Read( &area.y2, size, offset );
  offset += size; 
  size = sizeof(int);
  ok = ok && valueRecord.Read( &nofEntries, size, offset );
  offset += size; 
  size = sizeof(long);
  ok = ok && valueRecord.Read( &t1, size, offset );
  offset += size;
  ok = ok && valueRecord.Read( &t2, size, offset );
  offset += size;
  size = sizeof(bool);
  ok = ok && valueRecord.Read( &modified, size, offset );
  offset += size;
  for (int i = 0; i < nofEntries; i ++)  
  {
	size = sizeof(int);
	ok = ok && valueRecord.Read(&historyArray[i].hid, size, offset );
	offset += size;
	size = sizeof(long);
	ok = ok && valueRecord.Read(&historyArray[i].htimestart,size, offset);
	offset += size;
	ok = ok && valueRecord.Read(&historyArray[i].htimeend,size, offset);
	offset += size;
	size = sizeof(double);
	ok = ok && valueRecord.Read(&historyArray[i].spos.x, size, offset );
	offset += size;
	ok = ok && valueRecord.Read(&historyArray[i].spos.y, size, offset );
	offset += size;
	ok = ok && valueRecord.Read(&historyArray[i].epos.x, size, offset );
	offset += size;
	ok = ok && valueRecord.Read(&historyArray[i].epos.y, size, offset );
	offset += size;
  }
  int1 = LongToInstant(t1);
  int2 = LongToInstant(t2);
  value.addr = new UGridCell(intLabel, area, nofEntries, int1, int2,
	  modified, historyArray); 
  return ok;
}	

/*
2.6.6.8   ~Save~ function of class UGridCell

*/
  bool     UGridCell::SaveUGridCell(SmiRecord& valueRecord, size_t& offset, 
                                    const ListExpr typeInfo, Word& value )
{
  UGridCell* uc = (UGridCell*)( value.addr );	
  size_t size = sizeof(int); 	
  bool ok = true;
  ok = ok && valueRecord.Write( &uc->intlabel, size, offset );
  offset += size;  
  size = sizeof(double); 	
  ok = ok && valueRecord.Write( &uc->ucarea.x1, size, offset );	
  offset += size;  
  ok = ok && valueRecord.Write( &uc->ucarea.y1, size, offset );	
  offset += size; 
  ok = ok && valueRecord.Write( &uc->ucarea.x2, size, offset );	
  offset += size;  
  ok = ok && valueRecord.Write( &uc->ucarea.y2, size, offset );	
  offset += size; 
  size = sizeof(int); 	
  ok = ok && valueRecord.Write( &uc->noOfEntries, size, offset );	
  offset += size;
  size = sizeof(long); 	
  ok = ok && valueRecord.Write( &uc->firstTime, size, offset );
  offset += size;	
  ok = ok && valueRecord.Write( &uc->lastTime, size, offset );
  offset += size;
  size = sizeof(bool); 	
  ok = ok && valueRecord.Write( &uc->modified, size, offset );	
  offset += size;
  int entries = uc->GetNoOfEntries();
  for (int i = 0; i < entries; i ++)
  {
  size = sizeof(int);
  ok = ok && valueRecord.Write(&uc->historyArray[i].hid, size, offset );
  offset += size;
  size = sizeof(double);
  ok = ok && valueRecord.Write(&uc->historyArray[i].spos.x, size, offset );
  offset += size;
  ok = ok && valueRecord.Write(&uc->historyArray[i].spos.y, size, offset );
  offset += size;
  ok = ok && valueRecord.Write(&uc->historyArray[i].epos.x, size, offset );
  offset += size;
  ok = ok && valueRecord.Write(&uc->historyArray[i].epos.y, size, offset );
  offset += size;
  size = sizeof(long);
  ok = ok && valueRecord.Write(&uc->historyArray[i].htimestart,
                                size, offset );
  offset += size;
  ok = ok && valueRecord.Write(&uc->historyArray[i].htimeend,
                                size, offset );
  offset += size;
  } 
  return ok;
}	

/*
2.6.6.9   ~Close~ function of class UGridCell

*/
void     UGridCell::CloseUGridCell( const ListExpr typeInfo, Word& w )
{ 
  delete static_cast<UGridCell*>( w.addr );
  w.addr = 0;
}

/*
2.6.6.10   ~Clone~ function of class UGridCell

*/
Word     UGridCell::CloneUGridCell( const ListExpr typeInfo, const Word& w)
{
  UGridCell* ug = static_cast<UGridCell*>( w.addr );
  return SetWord( new UGridCell(*ug) );
}

/*
2.6.6.11   ~Cast~ function of class UGridCell
   not used
2.6.6.12   Implementations ~SizeOfObj~ function of class UGridCell

*/
int      UGridCell::SizeOfObjUGridCell()
{
  return sizeof(UGridCell);
}

/*
2.6.6.13 Type Description of class  UGridCell

*/
ListExpr  UGridCell::PropertyUGridCell()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
               nl->StringAtom("Example Type List"),
               nl->StringAtom("List Rep"),
               nl->StringAtom("Example List"),
               nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
               nl->StringAtom("ugridcell"),
               nl->StringAtom("(<dim> <label> <carea> )"),
               nl->StringAtom("(32)(3131)(8.15 2.899 6.77 8.99))"),
               nl->StringAtom("dim, label, area"))));
}

////  2.6.6.14   ~KindCheck~ function of class UGridCell
bool  UGridCell::KindCheckUGridCell( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, UGRIDCELL ));
}

/*
2.6.6.15 Creation of TypeConstructor instance for ugridcell

*/
TypeConstructor ugridcellTC(
  UGRIDCELL,                        // name of the type in SECONDO
  UGridCell::PropertyUGridCell,     // property function describing signature
  UGridCell::OutUGridCell,          // Out functions
  UGridCell::InUGridCell,           // In functions
  0, 0,                             // SaveToList, RestoreFromList functions
  UGridCell::CreateUGridCell,       // object creation 
  UGridCell::DeleteUGridCell,       // object deletion
  UGridCell::OpenUGridCell,         // open object 
  UGridCell::SaveUGridCell,         // save object 
  UGridCell::CloseUGridCell,        // close
  UGridCell::CloneUGridCell,        // clone
  0,                                // cast function
  UGridCell::SizeOfObjUGridCell,    // sizeof function
  UGridCell::KindCheckUGridCell);   // kind checking function
/*
2.7         Class UGrid
2.7.1        public methods
2.7.2        Constructors and destructor

default constructor

*/
UGrid::UGrid() :
  file( true ),        //fixed record length 
  header(),
  cells(0)
  {};

/*
Create Constructor

*/
UGrid::UGrid( UGridArea area, const int pageSize ) :
  file( true, pageSize ),        //fixed record length , pageSize (= 4000)
  header(area),
  cells(0)
{
  file.Create();
  dimsize dim = (dimsize)32;      // TBD  je nach dim des Ugrid  32x32
  int intdim = (int)dim;                         
  // Calculate length and heigth of area in ugrid and in ugridcell
  double xLength, yLength, xCellength, yCellength;
  xLength = abs(area.x2 - area.x1);   // length of x-coordinate in ugrid       
  yLength = abs(area.y2 - area.y1);   // length of y-coordinate in ugrid
  xCellength = xLength / intdim;      // length of x-coordinate in an ugridcell 
  yCellength = yLength / intdim;      // length of y-coordinate in an ugridcell
  // store euklidian region of ugrid in header.area
  header.area.SetX1(area.x1);
  header.area.SetY1(area.y1);
  header.area.SetX2(area.x2);
  header.area.SetY2(area.y2);
  // generate the cells of the ugrid
  UGridArea ugcarea (0.0, 0.0, 0.0, 0.0);
  for (int j = 0; j <intdim * intdim; j++)             
  {
  // Calculate the area-partition of ugridcell 
	if (area.x1 < area.x2)
	{
      ugcarea.x1 = area.x1 + ((j % intdim) * xCellength); 
      ugcarea.x2 = ugcarea.x1 + xCellength;
	}
	else
	{
      ugcarea.x1 = area.x1 - ((j % intdim) * xCellength); 
      ugcarea.x2 = ugcarea.x1 - xCellength;
	}
	if (area.y1 < area.y2)
	{
      ugcarea.y1 = area.y1 + ((j / intdim) * yCellength);
      ugcarea.y2 = ugcarea.y1 + yCellength; 
	}
	else
	{
      ugcarea.y1 = area.y1 + ((j / intdim) * yCellength);
      ugcarea.y2 = ugcarea.y1 + yCellength; 
	}
	UGridCell* cell = new UGridCell(j, dim, ugcarea); // initiate ugridcell 
    cells.Append (*cell);                             // append ugridcell 
  }
  // store noOf Cells, ModCells and Entries in header
  header.noOfCells = (int)cells.Size();
  header.noOfModCells = 0;
  header.noOfEntries = 0;
  header.ugcPtr = NULL;
  header.slaveUgridId = (SmiRecordId)0;
  
  // Creating a new page for UGrid header.
  SmiRecordId headerRecno;
  SmiRecord headerRecord;
  int AppendedRecord = file.AppendRecord( headerRecno, headerRecord );
  assert( AppendedRecord );
  header.ugridRecordId = headerRecno;        // store headerRecno in header 
  
  // Creating new pages for each UGridCell.
  SmiRecordId cellRecno;
  SmiRecord cellRecord;
  for (int k = 0; k <(intdim * intdim); k++) 
  {
    int intlabel = ((k / intdim)*100) + (k % intdim);
	const int cellind = k;
	AppendedRecord = file.AppendRecord( cellRecno, cellRecord );
    assert( AppendedRecord );                // ugridcells still are empty
	// store RecordId for ugridcell in assigned CellContainer
	if (0<=cellind && cellind <=255) 
	{
	  cellContQ1[ (cellind % 256) ].celllabel = intlabel;        
	  cellContQ1[ (cellind % 256)].cellRecordId = cellRecno;  
	}
	if (256<=cellind && cellind<=511) 
	{
	  cellContQ2[ (cellind % 256) ].celllabel = intlabel;         
	  cellContQ2[ (cellind % 256) ].cellRecordId = cellRecno;
	}
	if (512<=cellind && cellind<=767) 
	{
    cellContQ3[ (cellind % 256) ].celllabel = intlabel;
    cellContQ3[ (cellind % 256) ].cellRecordId = cellRecno;
  }
	if (768<=cellind && cellind<=1023) 
	{
    cellContQ4[ (cellind % 256) ].celllabel = intlabel;
    cellContQ4[ (cellind % 256) ].cellRecordId = cellRecno;
  }
	if (1024<=cellind && cellind<=1279) 
	{
    cellContQ5[ cellind ].celllabel = intlabel;
    cellContQ5[ cellind ].cellRecordId = cellRecno;
	}
	if (1280<=cellind && cellind<=1535) 
	{
    cellContQ6[ cellind ].celllabel = intlabel;
    cellContQ6[ cellind ].cellRecordId = cellRecno;
  }
	if (1536<=cellind && cellind<=1791) 
	{
    cellContQ7[ cellind ].celllabel = intlabel;
    cellContQ7[ cellind ].cellRecordId = cellRecno;
  }
	if (1792<=cellind && cellind<=2047) 
	{
    cellContQ8[ cellind ].celllabel = intlabel;
    cellContQ8[ cellind ].cellRecordId = cellRecno;
	}
	if (2048<=cellind && cellind<=2303) 
	{
    cellContQ9[ cellind ].celllabel = intlabel;
    cellContQ9[ cellind ].cellRecordId = cellRecno;
	}
	if (2304<=cellind && cellind<=2559) 
	{
    cellContQ10[ cellind ].celllabel = intlabel;
    cellContQ10[ cellind ].cellRecordId = cellRecno;
	}
	if (2560<=cellind && cellind<=2815) 
	{
    cellContQ11[ cellind ].celllabel = intlabel;
    cellContQ11[ cellind ].cellRecordId = cellRecno;
	}
	if (2816<=cellind && cellind<=3071) 
	{
	  cellContQ12[ cellind ].celllabel = intlabel;   
	  cellContQ12[ cellind ].cellRecordId = cellRecno;                 
	}
	if (3072<=cellind && cellind<=3327) 
	{
      cellContQ13[ cellind ].celllabel = intlabel;   
	  cellContQ13[ cellind ].cellRecordId = cellRecno;                 
	}
	if (3328<=cellind && cellind<=3583) 
	{
	  cellContQ14[ cellind ].celllabel = intlabel; 
	  cellContQ14[ cellind ].cellRecordId = cellRecno;                 
	}
	if (3584<=cellind && cellind<=3839) 
	{
	  cellContQ15[ cellind ].celllabel = intlabel;  
	  cellContQ15[ cellind ].cellRecordId = cellRecno;                 
	}
	if (3840<=cellind && cellind<=4095) 
	{
	  cellContQ16[ cellind ].celllabel = intlabel;  
	  cellContQ16[ cellind ].cellRecordId = cellRecno; 
	}
  }
  // Creating new pages for all cellContainer.
  SmiRecordId cellContRecno;
  SmiRecord cellContRecord;
  for (int l = 0; l < maxCont; l++)  // append record for cellRecordContainer  
  {
    AppendedRecord = file.AppendRecord( cellContRecno, cellContRecord );
    assert( AppendedRecord );                                
	// store record of cellContainer in header
	header.contArray[l].containerId = contsuffix(l+1);        
	header.contArray[l].contRecordId = cellContRecno; 
  }
}

/*
Query Constructor

*/
UGrid::UGrid( const SmiFileId fileid ) :
file( true ),
header(),
cells(0)
{
  file.Open( fileid );
  ReadHeader(); 
  ReadCells();
}

/*
Copy constructor

*/
UGrid::UGrid( UGrid& ug):
file(true),
header (),
cells (0)
{
  // copy the header
  header.ugridRecordId = ug.header.ugridRecordId;
  header.noOfCells = ug.header.noOfCells; 
  header.noOfModCells = ug.header.noOfModCells;
  header.noOfEntries = ug.header.noOfEntries;
  header.area.x1 = ug.header.area.x1;
  header.area.x2 = ug.header.area.x2;
  header.area.y1 = ug.header.area.y1;
  header.area.y2 = ug.header.area.y2;
  header.firstTime = ug.header.firstTime; 
  header.firstTime = ug.header.lastTime;
  for (int k = 0; k < maxCont; k++) 
  {
    header.contArray[k] = ug.header.contArray[k];
  }
  header.ugcPtr = ug.header.ugcPtr;
  header.slaveUgridId = ug.header.slaveUgridId; 
  // copy the cell container
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ1[l].celllabel = ug.cellContQ1[l].celllabel;
	cellContQ1[l].cellRecordId = ug.cellContQ1[l].cellRecordId;
  }
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ2[l].celllabel = ug.cellContQ2[l].celllabel;
	cellContQ2[l].cellRecordId = ug.cellContQ2[l].cellRecordId;
  }
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ3[l].celllabel = ug.cellContQ3[l].celllabel;
	cellContQ3[l].cellRecordId = ug.cellContQ3[l].cellRecordId;
  }
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ4[l].celllabel = ug.cellContQ4[l].celllabel;
	cellContQ4[l].cellRecordId = ug.cellContQ4[l].cellRecordId;
  }
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ5[l].celllabel = ug.cellContQ5[l].celllabel;
	cellContQ5[l].cellRecordId = ug.cellContQ5[l].cellRecordId;
  }
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ6[l].celllabel = ug.cellContQ6[l].celllabel;
	cellContQ6[l].cellRecordId = ug.cellContQ6[l].cellRecordId;
  }
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ7[l].celllabel = ug.cellContQ7[l].celllabel;
	cellContQ7[l].cellRecordId = ug.cellContQ7[l].cellRecordId;
  }
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ8[l].celllabel = ug.cellContQ8[l].celllabel;
	cellContQ8[l].cellRecordId = ug.cellContQ8[l].cellRecordId;
  }
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ9[l].celllabel = ug.cellContQ9[l].celllabel;
	cellContQ9[l].cellRecordId = ug.cellContQ9[l].cellRecordId;
  }
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ10[l].celllabel = ug.cellContQ10[l].celllabel;
	cellContQ10[l].cellRecordId = ug.cellContQ10[l].cellRecordId;
  }
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ11[l].celllabel = ug.cellContQ11[l].celllabel;
	cellContQ11[l].cellRecordId = ug.cellContQ11[l].cellRecordId;
  }
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ12[l].celllabel = ug.cellContQ12[l].celllabel;
	cellContQ12[l].cellRecordId = ug.cellContQ12[l].cellRecordId;
  }
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ13[l].celllabel = ug.cellContQ13[l].celllabel;
	cellContQ13[l].cellRecordId = ug.cellContQ13[l].cellRecordId;
  }
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ14[l].celllabel = ug.cellContQ14[l].celllabel;
	cellContQ14[l].cellRecordId = ug.cellContQ14[l].cellRecordId;
  }
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ15[l].celllabel = ug.cellContQ15[l].celllabel;
	cellContQ15[l].cellRecordId = ug.cellContQ15[l].cellRecordId;
  }
  for (int l = 0; l < maxCellRec; l++) 
  {
    cellContQ16[l].celllabel = ug.cellContQ16[l].celllabel;
	cellContQ16[l].cellRecordId = ug.cellContQ16[l].cellRecordId;
  }
  // copy the cells
  const UGridCell* ugugc = new UGridCell();
  UGridCell* ugc = new UGridCell();
  for (int i = 0; i < ug.header.noOfCells; i++) 
  {
	const int k = i;
	ug.cells.Get(k, ugugc); 
	*ugc = *ugugc;
	cells.Append(*ugc);
  }
}
 
/*
the slave constructor to create a slave ugrid

*/
UGrid::UGrid(bool slave, UGrid& mug, UGrid& ug):
file(true),
header (),
cells (0)
{
  double thisDim = ((sqrt((double)mug.header.noOfCells)) / 2);
  //int d = (int)mug.cells.Size();
  int intdim = (int)thisDim;
  dimsize dim = (dimsize)intdim;
  // Calculate length and heigth of area in ugrid and in ugridcell
  double xLength, yLength, xCellength, yCellength;
  xLength = abs(ug. header.area.x2 -ug. header.area.x1); 
  // length of x-coord    
  yLength = abs(ug.header.area.y2 - ug.header.area.y1);  
  // length of y-coord
  xCellength = xLength / intdim;         
  // length of x-coord in an ugridcell 
  yCellength = yLength / intdim;         
  // length of y-coord in an ugridcell
  // store euklidian region of ugrid in header.area
  header.area.SetX1(ug.header.area.x1);
  header.area.SetY1(ug.header.area.y1);
  header.area.SetX2(ug.header.area.x2);
  header.area.SetY2(ug.header.area.y2);
  // generate the cells of the ugrid
  UGridArea ugcarea (0.0, 0.0, 0.0, 0.0);
  for (int j = 0; j <intdim * intdim; j++)
  {
  // Calculate the area-partition of ugridcell  
	if (header.area.x1 < header.area.x2)
	{
      ugcarea.x1 = header.area.x1 + ((j % intdim) * xCellength);
      ugcarea.x2 = ugcarea.x1 + xCellength;
	}
	else
	{
      ugcarea.x1 = header.area.x1 - ((j % intdim) * xCellength);
	  ugcarea.x2 = ugcarea.x1 -xCellength;
	}
	if (header.area.y1 < header.area.y2)
	{
      ugcarea.y1 = header.area.y1 + ((j / intdim) * yCellength);
      ugcarea.y2 = ugcarea.y1 + yCellength; 
	}
	else
	{
      ugcarea.y1 = header.area.y1 - ((j / intdim) * yCellength);
      ugcarea.y2 = ugcarea.y1 - yCellength; 
	}
	UGridCell* cell = new UGridCell(j, dim, ugcarea);  // initiate ugridcell
	cell->intlabel = cell->intlabel + (intdim * 10000);
    cells.Append (*cell);                              // append ugridcell
  }
  // store noOf Cells, ModCells and Entries in header
  header.noOfCells = (int)cells.Size();
  header.noOfModCells = 0;
  header.noOfEntries = 0;
  header.slaveUgridId = (SmiRecordId)0;
  header.ugcPtr = NULL;
  const UGridCell* testcell = new UGridCell();
  cells.Get(0,testcell);
  // Creating a new page for UGrid header.
  SmiRecordId headerRecno ;   // expected neue Recno
 
  SmiRecord headerRecord;
  int AppendedRecord = ug.file.AppendRecord( headerRecno, headerRecord );
  assert( AppendedRecord );
  header.ugridRecordId = headerRecno;    // store headerRecno in own header 
  mug.header.slaveUgridId = headerRecno; // store headerRecno in header
                                         // of master ugrid
  mug.header.ugcPtr = this;
  
  // Creating new pages for each UGridCell.
  SmiRecordId cellRecno;
  SmiRecord cellRecord;
  for (int k = 0; k <(cells.Size()); k++) 
  {
    int intlabel = ((intdim * 10000)+(((k / intdim)*100) + (k % intdim)));
	const int cellind = k;
	
	AppendedRecord = ug.file.AppendRecord( cellRecno, cellRecord );
    assert( AppendedRecord );               // ugridcells still are empty
	// store RecordId for ugridcell in assigned CellContainer
	if (0<=cellind && cellind <=255) 
	{
	  cellContQ1[ (cellind % 256) ].celllabel = intlabel;        
	  cellContQ1[ (cellind % 256)].cellRecordId = cellRecno;  
	}
	if (256<=cellind && cellind<=511) 
	{
	  cellContQ2[ (cellind % 256) ].celllabel = intlabel;         
	  cellContQ2[ (cellind % 256) ].cellRecordId = cellRecno; 
	}
	if (512<=cellind && cellind<=767) 
	{
    cellContQ3[ (cellind % 256) ].celllabel = intlabel;
    cellContQ3[ (cellind % 256) ].cellRecordId = cellRecno;
	}
	if (768<=cellind && cellind<=1023) 
	{
    cellContQ4[ (cellind % 256) ].celllabel = intlabel;
    cellContQ4[ (cellind % 256) ].cellRecordId = cellRecno;
  }
  }
  // Creating new pages for all cellContainer.
  SmiRecordId cellContRecno;
  SmiRecord cellContRecord;
  for (int l = 0; l < maxCont; l++)
  // append record for each cellRecordContainer in file
  {
    AppendedRecord = ug.file.AppendRecord( cellContRecno, cellContRecord );
    assert( AppendedRecord );
    // store record of cellContainer in header
	header.contArray[l].containerId = contsuffix(l+1);        
	header.contArray[l].contRecordId = cellContRecno; 
  }
}

/*
destructor

*/
UGrid::~UGrid()
{
  if( file.IsOpen() )
  {
    SaveCells();
	WriteHeader();
    file.Close();
  }
}
/*
2.7.3        methods
2.7.4        static methods
2.7.5        private methods
 Write cells

*/
  void UGrid::WriteCells (SmiRecord &cellRecord, CellRecId cont[maxCellRec], 
	                      UGridCell &ugc)
  {
	SmiRecordId cellRecno;
    for (int k = 0; k < maxCellRec; k++)
    {
      cellRecno = (SmiRecordId)cont[k].cellRecordId;
      size_t offset = 0 ;
      int RecordSelected = file.SelectRecord( cellRecno, cellRecord,
                                              SmiFile::Update );
      assert (RecordSelected);
      int RecordWritten = cellRecord.Write(&ugc.intlabel, sizeof( int), 0 );
      offset += sizeof( int );
      RecordWritten = cellRecord.Write( &ugc.ucarea.x1, sizeof( double),
                                         offset );
      offset += sizeof( double );
      RecordWritten = cellRecord.Write( &ugc.ucarea.y1, sizeof( double),
                                         offset );
      offset += sizeof( double );
      RecordWritten = cellRecord.Write( &ugc.ucarea.x2, sizeof( double),
                                         offset );
      offset += sizeof( double );
      RecordWritten = cellRecord.Write( &ugc.ucarea.y2, sizeof( double),
                                         offset );
      offset += sizeof( double );
      RecordWritten = cellRecord.Write( &ugc.noOfEntries, sizeof( int),
                                         offset );
      offset += sizeof( int );
      RecordWritten = cellRecord.Write( &ugc.firstTime, sizeof( long),
                                         offset );
      offset += sizeof( long );
      RecordWritten = cellRecord.Write( &ugc.lastTime, sizeof( long),
                                         offset );
      offset += sizeof( long );
      RecordWritten = cellRecord.Write( &ugc.modified, sizeof( bool),
                                         offset );
      offset += sizeof( bool );
      // read the history units
    for (int j = 0; j < ugc.noOfEntries; j ++)
    {
       // read current entry
      RecordWritten = cellRecord.Write( &ugc.historyArray[j].hid,
                                         sizeof (int), offset );
      offset += sizeof (int);
      RecordWritten = cellRecord.Write( &ugc.historyArray[j].htimestart,
                                         sizeof (long), offset );
      offset += sizeof (long);
      RecordWritten = cellRecord.Write( &ugc.historyArray[j].htimeend,
                                         sizeof (long), offset );
      offset += sizeof (long);
      RecordWritten = cellRecord.Write( &ugc.historyArray[j].spos.x,
                                       sizeof(double), offset );
    offset += sizeof(double);
    RecordWritten = cellRecord.Write( &ugc.historyArray[j].spos.y,
                                       sizeof(double), offset );
    offset += sizeof(double);
    RecordWritten = cellRecord.Write( &ugc.historyArray[j].epos.x,
                                       sizeof(double), offset );
    offset += sizeof(double);
    RecordWritten = cellRecord.Write( &ugc.historyArray[j].epos.y,
                                       sizeof(double), offset );
    offset += sizeof(double);
    }
  }
}

/*
Read the merged ugrids

*/
  void UGrid::ReadSlaveCells (UGrid &sug)
  {
    // Read cell container
    SmiRecordId cellContRecno;
    SmiRecord   cellContRecord;
    cellContRecno = sug.header.contArray[0].contRecordId; 
	                             // slave ugrid has max 1 container
	int RecordSelected = file.SelectRecord( cellContRecno, cellContRecord, 
		                                    SmiFile::ReadOnly );
	assert (RecordSelected);
	// Read cell recordIs
	size_t offset = 0;
	for (int k = 0; k < sug.header.noOfCells; k++)   // statt maxCellRec
	{
	  int RecordRead = cellContRecord.Read( &sug.cellContQ1[k].celllabel,
                                           sizeof(int), offset );
    offset += sizeof( int );
    RecordRead = cellContRecord.Read( &sug.cellContQ1[k].cellRecordId,
                                       sizeof( SmiRecordId), offset );
    offset += sizeof(  SmiRecordId );
    // Read cells
    SmiRecordId cellRecno = sug.cellContQ1[k].cellRecordId;
      SmiRecord cellRecord;
      size_t offset = 0;
      UGridCell * ugc = new UGridCell();
      cellRecno = (SmiRecordId)(sug.cellContQ1[(k % 256)].cellRecordId);
      int RecordSelected = file.SelectRecord( cellRecno, cellRecord,
                                            SmiFile::ReadOnly );
      assert (RecordSelected);
      int cellRecordRead = cellRecord.Read( &ugc->intlabel,
                                           sizeof( int), offset );
      offset += sizeof( int );
      cellRecordRead = cellRecord.Read( &ugc->ucarea.x1,
                                         sizeof( double), offset );
      offset += sizeof( double );
      cellRecordRead = cellRecord.Read( &ugc->ucarea.y1,
                                         sizeof( double), offset );
      offset += sizeof( double );
      cellRecordRead = cellRecord.Read( &ugc->ucarea.x2,
                                         sizeof( double), offset );
      offset += sizeof( double );
      cellRecordRead = cellRecord.Read( &ugc->ucarea.y2,
                                         sizeof( double), offset );
      offset += sizeof( double );
      cellRecordRead = cellRecord.Read( &ugc->noOfEntries,
                                         sizeof( int), offset );
      offset += sizeof( int );
      cellRecordRead = cellRecord.Read( &ugc->firstTime,
                                         sizeof( long), offset );
      offset += sizeof( long );
      cellRecordRead = cellRecord.Read( &ugc->lastTime,
                                         sizeof( long), offset );
      offset += sizeof( long );
      cellRecordRead = cellRecord.Read( &ugc->modified,
                                         sizeof( bool), offset );
      offset += sizeof( bool );
      // read the history units
      if (ugc->noOfEntries >0)
      {
        for (int j = 0; j < ugc->noOfEntries; j ++)
        {
          // read current entry
          cellRecordRead = cellRecord.Read( &ugc->cellArray[j].cellRecId,
                                             sizeof( SmiRecordId), offset );
          offset += sizeof( SmiRecordId);
          cellRecordRead = cellRecord.Read( &ugc->cellArray[j].cellabel,
                                             sizeof( int), offset );
          offset += sizeof( int );
          cellRecordRead = cellRecord.Read( &ugc->cellArray[j].ugctimestart,
                                             sizeof( long), offset );
          offset += sizeof( long );
          cellRecordRead = cellRecord.Read( &ugc->cellArray[j].ugctimeend,
                                             sizeof( long), offset );
          offset += sizeof( long );
          cellRecordRead = cellRecord.Read( &ugc->cellArray[j].ugcspos.x,
                                             sizeof( double), offset );
          offset += sizeof( double );
          cellRecordRead = cellRecord.Read( &ugc->cellArray[j].ugcspos.y,
                                             sizeof( double), offset );
          offset += sizeof( double );
          cellRecordRead = cellRecord.Read( &ugc->cellArray[j].ugcepos.x,
                                             sizeof( double), offset );
          offset += sizeof( double );
          cellRecordRead = cellRecord.Read( &ugc->cellArray[j].ugcepos.y,
                                             sizeof( double), offset );
          offset += sizeof( double );
        }
      }
      sug.cells.Append (*ugc);               // append ugridcell to ugrid
    } // noOfCells
  }
/*
Write the merged ugrids

*/
  void UGrid::WriteSlaveCells (UGrid* ugPtr)
  {
    //SmiRecordFile file = this.file;
	//  UGrid* sug = new UGrid(*ugPtr);
    SmiRecordId cellRecno;
    SmiRecord cellRecord;
    for (int k = 0; k < ugPtr->cells.Size(); k++) 
    { 
      size_t offset = 0 ;
	  cellRecno = (SmiRecordId)(ugPtr->cellContQ1[(k % 256)].cellRecordId);
      // read current entry
	  const UGridCell* ugc;
	  ugPtr->cells.Get(k, ugc);  
	  // read the slave-ugridcell out of main memory
	  int RecordSelected = file.SelectRecord( cellRecno, cellRecord, 
		                                      SmiFile::Update );
	  assert (RecordSelected);
	  int RecordWritten = cellRecord.Write(&ugc->intlabel, 
		                                   sizeof( int), 0 );
	  offset += sizeof( int );
	  RecordWritten = cellRecord.Write( &ugc->ucarea.x1, 
		                                sizeof( double), offset );
	  offset += sizeof( double );
	  RecordWritten = cellRecord.Write( &ugc->ucarea.y1, 
		                                sizeof( double), offset );
	  offset += sizeof( double );
	  RecordWritten = cellRecord.Write( &ugc->ucarea.x2, 
		                                sizeof( double), offset );
	  offset += sizeof( double );
	  RecordWritten = cellRecord.Write( &ugc->ucarea.y2, 
		                                sizeof( double), offset );
	  offset += sizeof( double );
	  RecordWritten = cellRecord.Write( &ugc->noOfEntries, 
		                                sizeof( int), offset );
	  offset += sizeof( int );
	  RecordWritten = cellRecord.Write( &ugc->firstTime, 
		                                sizeof( long), offset );
	  offset += sizeof( long );
	  RecordWritten = cellRecord.Write( &ugc->lastTime, 
		                                sizeof( long), offset );
	  offset += sizeof( long );
	  RecordWritten = cellRecord.Write( &ugc->modified, 
		                                sizeof( bool), offset );
	  offset += sizeof( bool );
	  // read the history units
	  for (int j = 0; j < ugc->noOfEntries; j ++)
	  {
	    // read current entry
	    RecordWritten = cellRecord.Write( &ugc->cellArray[j].cellRecId,
			sizeof (SmiRecordId), offset );
	    offset += sizeof (SmiRecordId);
		RecordWritten = cellRecord.Write( &ugc->cellArray[j].cellabel, 
			                              sizeof (int), offset );
	    offset += sizeof (int);
		RecordWritten = cellRecord.Write(
		&ugc->cellArray[j].ugctimestart,sizeof (long), offset );	
		offset += sizeof (long); 
		RecordWritten = cellRecord.Write( &ugc->cellArray[j].ugctimeend,
			sizeof (long), offset );	
		offset += sizeof (long); 
		RecordWritten = cellRecord.Write( &ugc->cellArray[j].ugcspos.x,
			sizeof(double), offset );	
		offset += sizeof(double);  
		RecordWritten = cellRecord.Write( &ugc->cellArray[j].ugcspos.y,
			sizeof(double), offset );	
		offset += sizeof(double);  
		RecordWritten = cellRecord.Write( &ugc->cellArray[j].ugcepos.x,
			sizeof(double), offset );	
		offset += sizeof(double);  
		RecordWritten = cellRecord.Write( &ugc->cellArray[j].ugcepos.y,
			sizeof(double), offset );	
		offset += sizeof(double); 
	  }
	}
    SmiRecordId cellContRecno;
    SmiRecord cellContRecord;
    size_t offset = 0 ;
	cellContRecno = (ugPtr->header.contArray[0].contRecordId);  
	    // max one cell container in slave ugrid 
    int RecordSelected = file.SelectRecord( cellContRecno, cellContRecord, 
		                                    SmiFile::Update );
	assert (RecordSelected);
	for (int k = 0; k < maxCellRec; k ++)
	{
	  int RecordWritten = cellContRecord.Write(
	   &ugPtr->cellContQ1[k].celllabel,sizeof( int), offset);
	  offset += sizeof( int );
	  RecordWritten = cellContRecord.Write( 
		  &ugPtr->cellContQ1[k].cellRecordId,
		  sizeof( SmiRecordId), offset );
	  offset += sizeof( SmiRecordId); 
	}
  }
/*
Read the header of a slave ugrid

*/
  void UGrid::ReadSlaveHeader(SmiRecordId  sUgridId, UGrid* mugPtr) 
  {
    UGrid* sugrid = new UGrid();
	SmiRecord sUgridRecord;
    int RecordSelected = file.SelectRecord( sUgridId, sUgridRecord,
                                             SmiFile::ReadOnly );
    assert( RecordSelected );
    int RecordRead = sUgridRecord.Read( &sugrid->header, sizeof( Header ), 0 );
    assert( RecordRead );
	ReadSlaveCells(*sugrid );
	if (sugrid->header.noOfCells == 1024)
	{
       // copy the slave ugrid in ugrid named s32ugrid
      UGrid* s32ugrid = new UGrid(*sugrid);
	  mugPtr->header.ugcPtr = s32ugrid;
	  if (!(s32ugrid->header.slaveUgridId == (SmiRecordId)0))
	  {
	    ReadSlaveHeader(s32ugrid->header.slaveUgridId, s32ugrid);
	  }
	}
	if (sugrid->header.noOfCells == 256)
	{
       // copy the slave ugrid in ugrid named s32ugrid
      UGrid* s16ugrid = new UGrid(*sugrid);
	  mugPtr->header.ugcPtr = s16ugrid;	 
	  if (!(s16ugrid->header.slaveUgridId == (SmiRecordId)0))
	  {
		ReadSlaveHeader(s16ugrid->header.slaveUgridId, s16ugrid);
	  }
	}
	if (sugrid->header.noOfCells == 64)
	{
       // copy the slave ugrid in ugrid named s32ugrid
      UGrid* s8ugrid = new UGrid(*sugrid);
	  mugPtr->header.ugcPtr = s8ugrid;
	  if (!(s8ugrid->header.slaveUgridId == (SmiRecordId)0))
	  {
		ReadSlaveHeader(s8ugrid->header.slaveUgridId, s8ugrid);
	  }
	}
	if (sugrid->header.noOfCells == 16)
	{
       // copy the slave ugrid in ugrid named s32ugrid
      UGrid* s4ugrid = new UGrid(*sugrid);
	  mugPtr->header.ugcPtr = s4ugrid;
	  if (!(s4ugrid->header.slaveUgridId == (SmiRecordId)0))
	  {
		ReadSlaveHeader(s4ugrid->header.slaveUgridId, s4ugrid);
	  }
	}
	if (sugrid->header.noOfCells == 4)
	{
       // copy the slave ugrid in ugrid named s32ugrid
      UGrid* s2ugrid = new UGrid(*sugrid);
	  mugPtr->header.ugcPtr = s2ugrid;
	  if (!(s2ugrid->header.slaveUgridId == (SmiRecordId)0))
	  {
		ReadSlaveHeader(s2ugrid->header.slaveUgridId, s2ugrid);
	  }
	}
	if (sugrid->header.noOfCells == 1)
	{
      UGrid* s1ugrid = new UGrid(*sugrid);
	  mugPtr->header.ugcPtr = s1ugrid;
	  // no more child slave possible
	}
	// max 6 child slave ugrids
	delete sugrid;
  }
/*
Write the header of a slave ugrid

*/
  void UGrid::WriteSlaveHeader(SmiRecordId sUgridId, UGrid *ugPtr)
  {
    // write cellcontainer und cells of ugrid
    WriteSlaveCells(ugPtr);
	// write ugrid header
	SmiRecord record;
	int RecordSelected = file.SelectRecord( sUgridId, record, 
		SmiFile::Update);
    assert( RecordSelected );
    int RecordWritten = record.Write( &ugPtr->header, sizeof( Header ), 0 );
    assert( RecordWritten );
	if ((!(ugPtr->header.slaveUgridId == (SmiRecordId)0) )  &&
		  (ugPtr->header.noOfCells > 1 ))
	{  
      WriteSlaveHeader(ugPtr->header.slaveUgridId, ugPtr->header.ugcPtr);
	}
  }
/*
Read the header

*/
  void UGrid::ReadHeader()
  {
    SmiRecord record;
    int RecordSelected = file.SelectRecord( (SmiRecordId)1, record,
                                             SmiFile::ReadOnly );
    assert( RecordSelected );
    int RecordRead = record.Read( &header, sizeof( Header ), 0 );
       // == sizeof( Header );
    assert( RecordRead );
	if (!(header.slaveUgridId == (SmiRecordId)0))
	{
      ReadSlaveHeader(header.slaveUgridId, this);
	}
  }
/*
Write the header

*/
  void UGrid::WriteHeader()  
  {
    SmiRecord record;
	if (!(header.slaveUgridId == (SmiRecordId)0))  
	{  
	  WriteSlaveHeader(header.slaveUgridId, header.ugcPtr);
	}
    int RecordSelected =
        file.SelectRecord( (SmiRecordId)1, record, SmiFile::Update );
    assert( RecordSelected );
    int RecordWritten =
        record.Write( &header, sizeof( Header ), 0 );
    assert( RecordWritten );
  }
/*
Read appended ugridcells from file via cellContainer

*/
  void UGrid::ReadCells ()
  {
	SmiRecordId cellContRecno;
    SmiRecord   cellContRecord;
	for (int l = 0; l < maxCont; l ++)
	{
      cellContRecno = header.contArray[l].contRecordId;
	  int RecordSelected = file.SelectRecord( cellContRecno, cellContRecord,
		  SmiFile::ReadOnly );
	  assert (RecordSelected);
	  size_t offset = 0;
	  if (l == 0)
	  {
	    for (int k = 0; k < maxCellRec; k++)
		{
		  int RecordRead = cellContRecord.Read( 
			  &cellContQ1[k].celllabel, 
			  sizeof(int), offset );
	      offset += sizeof( int );
	      RecordRead = cellContRecord.Read( &cellContQ1[k].cellRecordId,
			  sizeof( SmiRecordId), offset );  
	      offset += sizeof(  SmiRecordId );
		}
	  }
	  if (l == 1)
	  {
	    for (int k = 0; k < maxCellRec; k++)
		{
		  int RecordRead = cellContRecord.Read( 
			  &cellContQ2[k].celllabel, 
			        sizeof(int), offset);
	      offset += sizeof( int );
	      RecordRead = cellContRecord.Read( &cellContQ2[k].cellRecordId, 
			        sizeof( SmiRecordId), offset );  
	      offset += sizeof(  SmiRecordId );
		}
	  }
	  if (l == 2)
	  {
	    for (int k = 0; k < maxCellRec; k++)
		{
		  int RecordRead = cellContRecord.Read( 
			  &cellContQ3[k].celllabel, 
			        sizeof(int), offset );
	      offset += sizeof( int );
	      RecordRead = cellContRecord.Read( &cellContQ3[k].cellRecordId, 
			        sizeof( SmiRecordId), offset );  
	      offset += sizeof(  SmiRecordId );
		}
	  }
	  if (l == 3)
	  {
	    for (int k = 0; k < maxCellRec; k++)
		{
		  int RecordRead = cellContRecord.Read( 
			  &cellContQ4[k].celllabel, 
			        sizeof(int), offset );
	      offset += sizeof( int );
	      RecordRead = cellContRecord.Read( 
			  &cellContQ4[k].cellRecordId, 
			        sizeof( SmiRecordId), offset );  
	      offset += sizeof(  SmiRecordId );
		}
	  }
	}	  
	SmiRecordId cellRecno;
    SmiRecord cellRecord;
    for (int k = 0; k < header.noOfCells; k++)  //HeaderRecord must be read
    { 
      size_t offset = 0;
      UGridCell * ugc = new UGridCell();
	  if ((k / 256) == 0 )
	  {
	    cellRecno = (SmiRecordId)(cellContQ1[(k % 256)].cellRecordId);
	    int RecordSelected = file.SelectRecord( cellRecno, cellRecord, 
			                                    SmiFile::ReadOnly );
		assert (RecordSelected);
	    int RecordRead = cellRecord.Read( &ugc->intlabel, 
			      sizeof( int), offset );
	    offset += sizeof( int );
	    RecordRead = cellRecord.Read( &ugc->ucarea.x1, 
			      sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->ucarea.y1, 
			      sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->ucarea.x2, 
			      sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->ucarea.y2, 
			      sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->noOfEntries, 
			      sizeof( int), offset );
	    offset += sizeof( int );
	    RecordRead = cellRecord.Read( &ugc->firstTime, 
			      sizeof( long), offset );
	    offset += sizeof( long );
	    RecordRead = cellRecord.Read( &ugc->lastTime, 
			      sizeof( long), offset );
	    offset += sizeof( long );
	    RecordRead = cellRecord.Read( &ugc->modified, 
			      sizeof( bool), offset );
	    offset += sizeof( bool );
	    // read the history units
	    if (ugc->noOfEntries >0)
	    {
		  for (int j = 0; j < ugc->noOfEntries; j ++)
		  {
		    // read current entry
		    RecordRead = cellRecord.Read( &ugc->historyArray[j].hid, 
				      sizeof( int), offset );
	        offset += sizeof( int );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].htimestart,
			    sizeof( long), offset );
	        offset += sizeof( long );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].htimeend,
				sizeof( long), offset );
	        offset += sizeof( long );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].spos.x, 
				sizeof( double), offset );
	        offset += sizeof( double );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].spos.y,
				sizeof( double), offset );
	        offset += sizeof( double );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].epos.x,
				sizeof( double), offset );
	        offset += sizeof( double );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].epos.y,
				sizeof( double), offset );
	        offset += sizeof( double );
		  }
	    }
	  }  //cellContQ1
      if ((k / 256) == 1 )
	  {
	    cellRecno = (SmiRecordId)(cellContQ2[(k % 256)].cellRecordId);
	    int RecordSelected = file.SelectRecord( cellRecno, cellRecord, 
			                                    SmiFile::ReadOnly );
		assert (RecordSelected);
	    int RecordRead = cellRecord.Read( &ugc->intlabel, 
			                              sizeof( int), offset );
	    offset += sizeof( int );
	    RecordRead = cellRecord.Read( &ugc->ucarea.x1, 
			                          sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->ucarea.y1, 
			                          sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->ucarea.x2, 
			                          sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->ucarea.y2, 
			                          sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->noOfEntries, 
			                          sizeof( int), offset );
	    offset += sizeof( int );
	    RecordRead = cellRecord.Read( &ugc->firstTime, 
			                          sizeof( long), offset );
	    offset += sizeof( long );
	    RecordRead = cellRecord.Read( &ugc->lastTime, 
			                          sizeof( long), offset );
	    offset += sizeof( long );
	    RecordRead = cellRecord.Read( &ugc->modified, 
			                          sizeof( bool), offset );
	    offset += sizeof( bool );
	    // read the history units
	    if (ugc->noOfEntries >0)
	    {
		  for (int j = 0; j < ugc->noOfEntries; j ++)
		  {
		    // read current entry
		    RecordRead = cellRecord.Read( &ugc->historyArray[j].hid, 
				      sizeof( int), offset );
	        offset += sizeof( int );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].htimestart, 
				      sizeof( long), offset );
	        offset += sizeof( long );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].htimeend, 
				      sizeof( long), offset );
	        offset += sizeof( long );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].spos.x, 
				      sizeof( double), offset );
	        offset += sizeof( double );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].spos.y, 
				      sizeof( double), offset );
	        offset += sizeof( double );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].epos.x, 
				      sizeof( double), offset );
	        offset += sizeof( double );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].epos.y, 
				      sizeof( double), offset );
	        offset += sizeof( double );
		  }
	    }
	  } //cellContQ2
      if ((k / 256 )== 2 )
	  {
	    cellRecno = (SmiRecordId)(cellContQ3[(k % 256)].cellRecordId);
	    int RecordSelected = file.SelectRecord( cellRecno, cellRecord, 
			                                    SmiFile::ReadOnly );
		assert (RecordSelected);
	    int RecordRead = cellRecord.Read( &ugc->intlabel, 
			                              sizeof( int), offset );
	    offset += sizeof( int );
	    RecordRead = cellRecord.Read( &ugc->ucarea.x1, 
			                          sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->ucarea.y1, 
			                          sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->ucarea.x2, 
			                          sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->ucarea.y2, 
			                          sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->noOfEntries, 
			                          sizeof( int), offset );
	    offset += sizeof( int );
	    RecordRead = cellRecord.Read( &ugc->firstTime, 
			                          sizeof( long), offset );
	    offset += sizeof( long );
	    RecordRead = cellRecord.Read( &ugc->lastTime, 
			                          sizeof( long), offset );
	    offset += sizeof( long );
	    RecordRead = cellRecord.Read( &ugc->modified, 
			                          sizeof( bool), offset );
	    offset += sizeof( bool );
	    // read the history units
	    if (ugc->noOfEntries >0)
	    {
		  for (int j = 0; j < ugc->noOfEntries; j ++)
		  {
		    // read current entry
		    RecordRead = cellRecord.Read( 
			 &ugc->historyArray[j].hid, sizeof( int), offset );
			offset += sizeof( int );
		    RecordRead = cellRecord.Read(
			&ugc->historyArray[j].htimestart,
			sizeof( long), offset );
	        offset += sizeof( long );
		    RecordRead = cellRecord.Read( 
			&ugc->historyArray[j].htimeend,sizeof( long), offset );
	        offset += sizeof( long );
		    RecordRead = cellRecord.Read( 
			&ugc->historyArray[j].spos.x,sizeof( double), offset );
	        offset += sizeof( double );
		    RecordRead = cellRecord.Read( 
			&ugc->historyArray[j].spos.y,sizeof( double), offset );
	        offset += sizeof( double );
		    RecordRead = cellRecord.Read( 
			&ugc->historyArray[j].epos.x,sizeof( double), offset );
	        offset += sizeof( double );
		    RecordRead = cellRecord.Read( &ugc->historyArray[j].epos.y, 
				      sizeof( double), offset );
	        offset += sizeof( double );
		  }
	    }
	  }  //cellContQ3
      if ((k / 256) == 3 )
	  {
	    cellRecno = (SmiRecordId)(cellContQ4[(k % 256)].cellRecordId);
	    int RecordSelected = file.SelectRecord( cellRecno, cellRecord, 
			                                    SmiFile::ReadOnly );
		assert (RecordSelected);
	    int RecordRead = cellRecord.Read( &ugc->intlabel, 
			                              sizeof( int), offset );
	    offset += sizeof( int );
	    RecordRead = cellRecord.Read( &ugc->ucarea.x1, 
			                          sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->ucarea.y1, 
			                          sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->ucarea.x2, 
			                          sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->ucarea.y2, 
			                          sizeof( double), offset );
	    offset += sizeof( double );
	    RecordRead = cellRecord.Read( &ugc->noOfEntries, 
			                          sizeof( int), offset );
	    offset += sizeof( int );
	    RecordRead = cellRecord.Read( &ugc->firstTime, 
			                          sizeof( long), offset );
	    offset += sizeof( long );
	    RecordRead = cellRecord.Read( &ugc->lastTime, 
			                          sizeof( long), offset );
	    offset += sizeof( long );
	    RecordRead = cellRecord.Read( &ugc->modified, 
			                          sizeof( bool), offset );
	    offset += sizeof( bool );
	    // read the history units
	    if (ugc->noOfEntries >0)
	    {
		  for (int j = 0; j < ugc->noOfEntries; j ++)
		  {
		    // read current entry
		    RecordRead = cellRecord.Read( &ugc->historyArray[j].hid, 
				      sizeof( int), offset );
	        offset += sizeof( int );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].htimestart, 
				      sizeof( long), offset );
	        offset += sizeof( long );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].htimeend, 
				      sizeof( long), offset );
	        offset += sizeof( long );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].spos.x, 
				      sizeof( double), offset );
	        offset += sizeof( double );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].spos.y, 
				      sizeof( double), offset );
	        offset += sizeof( double );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].epos.x, 
				     sizeof( double), offset );
	        offset += sizeof( double );
		    RecordRead = cellRecord.Read( 
				&ugc->historyArray[j].epos.y, 
				sizeof( double), offset );
	        offset += sizeof( double );
		  }
	    }
	  } //cellContQ4
      cells.Append (*ugc);                    // append ugridcell to ugrid 
	} // noOfCells
  }
/*
Given that all ugridcells are resistent stored

*/
  void UGrid::SaveCells ()                             //###
  {   
	//SmiRecordFile file = this.file;
    SmiRecordId cellRecno;
    SmiRecord cellRecord;
    for (int k = 0; k < cells.Size(); k++) 
    { 
      size_t offset = 0 ;
	  if ((k / 256) == 0 )
	  {
	    cellRecno = (SmiRecordId)(cellContQ1[(k % 256)].cellRecordId);
        // read current entry
	    const UGridCell* ugc;
	    cells.Get(k, ugc);         // read the ugridcell out of main memory
	    int RecordSelected = file.SelectRecord( cellRecno, cellRecord, 
			                                    SmiFile::Update );
		assert (RecordSelected);
	    int RecordWritten = cellRecord.Write(&ugc->intlabel, 
			                                 sizeof( int), 0 );
	    offset += sizeof( int );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.x1, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.y1, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.x2, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.y2, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->noOfEntries, 
			                              sizeof( int), offset );
	    offset += sizeof( int );
	    RecordWritten = cellRecord.Write( &ugc->firstTime, 
			                              sizeof( long), offset );
	    offset += sizeof( long );
	    RecordWritten = cellRecord.Write( &ugc->lastTime, 
			                              sizeof( long), offset );
	    offset += sizeof( long );
	    RecordWritten = cellRecord.Write( &ugc->modified, 
			                              sizeof( bool), offset );
	    offset += sizeof( bool );
	    // read the history units
		for (int j = 0; j < ugc->noOfEntries; j ++)
		{
		  // read current entry
	      RecordWritten = cellRecord.Write( &ugc->historyArray[j].hid, 
			                                sizeof (int), offset );
		  offset += sizeof (int);
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].htimestart, 
			        sizeof (long), offset );	
		  offset += sizeof (long); 
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].htimeend, 
			        sizeof (long), offset );	
		  offset += sizeof (long); 
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].spos.x, 
			        sizeof(double), offset );	
		  offset += sizeof(double);  
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].spos.y, 
			        sizeof(double), offset );	
		  offset += sizeof(double);  
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].epos.x, 
			        sizeof(double), offset );	
		  offset += sizeof(double);  
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].epos.y, 
			        sizeof(double), offset );	
		  offset += sizeof(double); 
		}
	  } // cellContQ1
	  if ((k / 256) == 1 )
	  {
	    cellRecno = (SmiRecordId)(cellContQ2[(k % 256)].cellRecordId);
        // read current entry
        size_t offset = 0 ;
	    const UGridCell* ugc;
	    cells.Get(k, ugc);
	    int RecordSelected = file.SelectRecord( cellRecno, cellRecord, 
			                                    SmiFile::Update );
		assert (RecordSelected);
	    int RecordWritten = cellRecord.Write(&ugc->intlabel, 
			                                 sizeof( int), 0 );
	    offset += sizeof( int );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.x1, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.y1, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.x2, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.y2, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->noOfEntries, 
			                              sizeof( int), offset );
	    offset += sizeof( int );
	    RecordWritten = cellRecord.Write( &ugc->firstTime, 
			                              sizeof( long), offset );
	    offset += sizeof( long );
	    RecordWritten = cellRecord.Write( &ugc->lastTime, 
			                              sizeof( long), offset );
	    offset += sizeof( long );
	    RecordWritten = cellRecord.Write( &ugc->modified, 
			                              sizeof( bool), offset );
	    offset += sizeof( bool );
	    // read the history units
		for (int j = 0; j < ugc->noOfEntries; j ++)
		{
		  // read current entry
	      RecordWritten = cellRecord.Write( &ugc->historyArray[j].hid, 
			                                sizeof (int), offset );
		  offset += sizeof (int);
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].htimestart,   
			        sizeof (long), offset );	
		  offset += sizeof (long); 
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].htimeend, 
			        sizeof (long), offset );	
		  offset += sizeof (long); 
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].spos.x, 
			        sizeof(double), offset );	
		  offset += sizeof(double);  
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].spos.y, 
			        sizeof(double), offset );	
		  offset += sizeof(double);  
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].epos.x, 
			        sizeof(double), offset );	
		  offset += sizeof(double);  
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].epos.y, 
			         sizeof(double), offset );	
		  offset += sizeof(double); 
		}
	  } // cellContQ2
	  if ((k / 256) == 2 )
	  {
	    cellRecno = (SmiRecordId)(cellContQ3[(k % 256)].cellRecordId);
        // read current entry
        size_t offset = 0 ;
	    const UGridCell* ugc;
	    cells.Get(k, ugc);
	    int RecordSelected = file.SelectRecord( cellRecno, cellRecord, 
			                                    SmiFile::Update );
		assert (RecordSelected);
	    int RecordWritten = cellRecord.Write(&ugc->intlabel, 
			                                 sizeof( int), 0 );
	    offset += sizeof( int );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.x1, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.y1, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.x2, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.y2, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->noOfEntries, 
			                              sizeof( int), offset );
	    offset += sizeof( int );
	    RecordWritten = cellRecord.Write( &ugc->firstTime, 
			                              sizeof( long), offset );
	    offset += sizeof( long );
	    RecordWritten = cellRecord.Write( &ugc->lastTime, 
			                              sizeof( long), offset );
	    offset += sizeof( long );
	    RecordWritten = cellRecord.Write( &ugc->modified, 
			                              sizeof( bool), offset );
	    offset += sizeof( bool );
	    // read the history units
		for (int j = 0; j < ugc->noOfEntries; j ++)
		{
		  // read current entry
	      RecordWritten = cellRecord.Write( &ugc->historyArray[j].hid, 
			                                sizeof (int), offset );
		  offset += sizeof (int);
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].htimestart,		  
			  sizeof (long), offset );	
		  offset += sizeof (long); 
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].htimeend, 
			        sizeof (long), offset );	
		  offset += sizeof (long); 
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].spos.x, 
			        sizeof(double), offset );	
		  offset += sizeof(double);  
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].spos.y, 
			        sizeof(double), offset );	
		  offset += sizeof(double);  
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].epos.x, 
			        sizeof(double), offset );	
		  offset += sizeof(double);  
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].epos.y, 
			        sizeof(double), offset );	
		  offset += sizeof(double); 
		}
	  } // cellContQ3
	  if ((k / 256) == 3 )
	  {
	    cellRecno = (SmiRecordId)(cellContQ4[(k % 256)].cellRecordId);
        // read current entry
        size_t offset = 0 ;
	    const UGridCell* ugc;
	    cells.Get(k, ugc);
	    int RecordSelected = file.SelectRecord( cellRecno, cellRecord, 
			                                    SmiFile::Update );
		assert (RecordSelected);
	    int RecordWritten = cellRecord.Write(&ugc->intlabel, 
			                                 sizeof( int), 0 );
	    offset += sizeof( int );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.x1, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.y1, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.x2, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->ucarea.y2, 
			                              sizeof( double), offset );
	    offset += sizeof( double );
	    RecordWritten = cellRecord.Write( &ugc->noOfEntries, 
			                              sizeof( int), offset );
	    offset += sizeof( int );
	    RecordWritten = cellRecord.Write( &ugc->firstTime, 
			                              sizeof( long), offset );
	    offset += sizeof( long );
	    RecordWritten = cellRecord.Write( &ugc->lastTime, 
			                              sizeof( long), offset );
	    offset += sizeof( long );
	    RecordWritten = cellRecord.Write( &ugc->modified, 
			                              sizeof( bool), offset );
	    offset += sizeof( bool );
	    // read the history units
		for (int j = 0; j < ugc->noOfEntries; j ++)
		{
		  // read current entry
	      RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].hid, 
			        sizeof (int), offset );
		  offset += sizeof (int);
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].htimestart, 
			        sizeof (long), offset );	
		  offset += sizeof (long); 
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].htimeend, 
			        sizeof (long), offset );	
		  offset += sizeof (long); 
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].spos.x, 
			        sizeof(double), offset );	
		  offset += sizeof(double);  
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].spos.y, 
			        sizeof(double), offset );	
		  offset += sizeof(double);  
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].epos.x, 
			        sizeof(double), offset );	
		  offset += sizeof(double);  
		  RecordWritten = cellRecord.Write( 
			  &ugc->historyArray[j].epos.y, 
			        sizeof(double), offset );	
		  offset += sizeof(double); 
		}
	  } // cellContQ4
    }// for cells.Size() all cells are written in the cell record
	// save cellContainer
	SmiRecordId cellContRecno;
    SmiRecord   cellContRecord;
	for (int l = 0; l < maxCont; l ++)
	{
      size_t offset = 0;
	  if (l == 0)
	  {
	    cellContRecno = header.contArray[l].contRecordId;
		int RecordSelected = file.SelectRecord( 
			cellContRecno, cellContRecord, SmiFile::Update );
		assert (RecordSelected);
        for (int k = 0; k < maxCellRec; k++)
		{
		  int RecordWritten = cellContRecord.Write( 
			  &cellContQ1[k].celllabel, sizeof(int), offset );
		  offset += sizeof(  int );
		  RecordWritten = cellContRecord.Write( 
		  &cellContQ1[k].cellRecordId, sizeof(SmiRecordId ), offset );
		  offset += sizeof(  SmiRecordId );
		}
	  }
	  if (l == 1)
	  {
	    cellContRecno = header.contArray[l].contRecordId;
		int RecordSelected = file.SelectRecord( 
			cellContRecno, cellContRecord, SmiFile::Update );
		assert (RecordSelected);
		for (int k = 0; k < maxCellRec; k++)
		{
		  int RecordWritten = cellContRecord.Write( 
			  &cellContQ2[k].celllabel, sizeof(int), offset );
		  offset += sizeof(  int );
		  RecordWritten = cellContRecord.Write( 
		  &cellContQ2[k].cellRecordId, sizeof(SmiRecordId ), offset );
		  offset += sizeof(  SmiRecordId );
		}
	  }
	  if (l == 2)
	  {
	    cellContRecno = header.contArray[l].contRecordId;
		int RecordSelected = file.SelectRecord( 
			cellContRecno, cellContRecord, SmiFile::Update );
		assert (RecordSelected);
		for (int k = 0; k < maxCellRec; k++)
		{
		  int RecordWritten = cellContRecord.Write( 
			  &cellContQ3[k].celllabel, sizeof(int), offset );
		  offset += sizeof(  int );
		  RecordWritten = cellContRecord.Write( 
		  &cellContQ3[k].cellRecordId, sizeof(SmiRecordId ), offset );
		  offset += sizeof(  SmiRecordId );
		}
	  }
	  if (l == 3)
	  {
	    cellContRecno = header.contArray[l].contRecordId;
		int RecordSelected = file.SelectRecord( 
			cellContRecno, cellContRecord, SmiFile::Update );
		assert (RecordSelected);
		for (int k = 0; k < maxCellRec; k++)
		{
		  int RecordWritten = cellContRecord.Write( 
		  &cellContQ4[k].celllabel, sizeof(int), offset );
		  offset += sizeof(  int );
		  RecordWritten = cellContRecord.Write( 
		  &cellContQ4[k].cellRecordId, sizeof(SmiRecordId ), offset );
		  offset += sizeof(  SmiRecordId );
		}
	  }
	}	  
  }
/*
2.7.6        List Representation
2.7.6.1   ~Out~ function of class UGrid

*/
  ListExpr UGrid::OutUGrid(ListExpr typeInfo, Word value )
{
  UGrid* ugrid = (UGrid*)(value.addr);
  string recordId = intToString(ugrid->header.ugridRecordId);
  string fileid = intToString(ugrid->file.GetFileId());
  Instant Time1 = LongToInstant(ugrid->header.firstTime);
  ugrid->ugInterval.start = Time1;
  Instant Time2 = LongToInstant(ugrid->header.lastTime);
  ugrid->ugInterval.end = Time2;
  ListExpr timeintervalList = nl->FourElemList(
	  OutDateTime( nl->TheEmptyList(), SetWord(&ugrid->ugInterval.start)), 
	                                                 // = &ugrid->firstTime 
      OutDateTime( nl->TheEmptyList(), SetWord(&ugrid->ugInterval.end)),  
	                                                 // = &ugrid->lastTime
	  nl->BoolAtom( true ),
      nl->BoolAtom( false));
  ListExpr areaList = nl->FourElemList(
	  nl->RealAtom( ugrid->header.area.GetX1()),
	  nl->RealAtom( ugrid->header.area.GetY1()),
	  nl->RealAtom( ugrid->header.area.GetX2()),
	  nl->RealAtom( ugrid->header.area.GetY2()));
   int noOfCells = (int)ugrid->cells.Size();
   int noOfModCells = (int)ugrid->header.noOfModCells;
   int noOfEntries = (int)ugrid->header.noOfEntries;	
  return nl->FiveElemList(
	       nl->StringAtom( "UGrid statistics" ),
	       nl->TwoElemList( 
			 nl->StringAtom( 
			    "RecordId / FileId / number of UgridCells :" ),
			 nl->ThreeElemList(nl->StringAtom( recordId ),
			   nl->StringAtom( fileid ),
			   nl->IntAtom( noOfCells ))),
			nl->TwoElemList( nl->StringAtom( "TimeInterval: " ),
                             timeintervalList),
			nl->TwoElemList( nl->StringAtom( "Area : " ),
                             areaList),
            nl->TwoElemList( 
			  nl->StringAtom( 
			    "number of Entries / number of mod Cells : " ),
              nl->TwoElemList(nl->IntAtom( noOfEntries ),
			                  nl->IntAtom( noOfModCells )))
                             );  
}

/*
2.7.6.2   ~In~ function of class UGrid

*/
  Word UGrid::InUGrid( ListExpr typeInfo, ListExpr value,
              int errorPos, ListExpr& errorInfo, bool& correct )

  {
  correct = false;
  string errMsg = "Reading an UGrid !";
  Word result = SetWord(Address(0));
  if ( nl->ListLength(value)!= 4 && 
	   nl->ListLength(value) != 1) 
  {
    cmsg.inFunError(errMsg);
    return result;
  }  
  else  
  {
    if (nl->ListLength(value) == 4 &&                 //UGridArea gridarea;
	    nl->IsAtom( nl->First( value ) ) &&
        nl->AtomType( nl->First( value) ) == RealType &&
        nl->IsAtom( nl->Second( value ) ) &&
        nl->AtomType( nl->Second( value ) ) == RealType &&
		nl->IsAtom( nl->Third( value ) ) &&
        nl->AtomType( nl->Third( value) ) == RealType &&
        nl->IsAtom( nl->Fourth( value ) ) &&
        nl->AtomType( nl->Fourth( value ) ) == RealType )
	{
	  double ax1 = nl->RealValue( nl->First( value));
	  double ay1 = nl->RealValue( nl->Second( value));
	  double ax2 = nl->RealValue( nl->Third( value));
	  double ay2 = nl->RealValue( nl->Fourth( value));
      if (( ax2 == ax1 ) || (ay1 == ay2 ))     // vertical or horizontal line   
	  {
        errMsg = "vertical or horizontal line or point";
		cmsg.inFunError(errMsg);
        return result;
	  }
	    else
	  {
	    UGridArea ugarea(ax1,ay1,ax2,ay2);
	    correct = true;
	    result.addr = new UGrid(ugarea, 4000);  
	    return result;
	  }
	}
	else  // area isn't four real type but an ugridarea type
	{
	  if( nl->ListLength(typeInfo) == 1)
	  {
        ListExpr area = nl->First(value);
	    if (nl->IsEqual( area, UGRIDAREA))
	    {
		  double ax1 = nl->RealValue( nl->First( area));
	      double ay1 = nl->RealValue( nl->Second( area));
	      double ax2 = nl->RealValue( nl->Third( area));
	      double ay2 = nl->RealValue( nl->Fourth( area));
		  if (( ax2 == ax1 ) || (ay1 == ay2 ))    
			  // vert or horizontal line   
	      {
            errMsg = "vertical or horizontal line or point";
		    cmsg.inFunError(errMsg);
            return result;
	      }
	      else
	      {
	        UGridArea ugarea(ax1,ay1,ax2,ay2);
		    correct = true;
            return SetWord( new UGrid(ugarea, 4000 ) );
		  }
		}
	  }	  
	}
	errMsg = "InUGrid:Error in area of UGrid!";
	cmsg.inFunError(errMsg);
	return result;
  } 
}

/*
2.6.7.3   ~SaveToList~ function of class UGrid

   not used

2.6.7.4   ~RestoreFromList~ function of class UGrid

   not used

2.6.7.5   ~Create~ function of class UGrid

*/
Word UGrid::CreateUGrid( const ListExpr typeInfo )
{
  string errMsg;
  Word result = SetWord(Address(0));
  if( nl->ListLength(typeInfo) == 4 )
  {
    if (nl->IsAtom( nl->First( typeInfo ) ) &&
        nl->AtomType( nl->First( typeInfo) ) == RealType &&
        nl->IsAtom( nl->Second( typeInfo ) ) &&
        nl->AtomType( nl->Second( typeInfo ) ) == RealType &&
		nl->IsAtom( nl->Third( typeInfo ) ) &&
        nl->AtomType( nl->Third( typeInfo) ) == RealType &&
        nl->IsAtom( nl->Fourth( typeInfo ) ) &&
        nl->AtomType( nl->Fourth( typeInfo ) ) == RealType )
	{
	  double ax1 = nl->RealValue( nl->First( typeInfo));
	  double ay1 = nl->RealValue( nl->Second( typeInfo));
	  double ax2 = nl->RealValue( nl->Third( typeInfo));
	  double ay2 = nl->RealValue( nl->Fourth( typeInfo));
	  if (( ax2 == ax1 ) || (ay1 == ay2 ))      
		  // vertical or horizontal line   
	  {
        errMsg = "vertical or horizontal line or point";
		cmsg.inFunError(errMsg);
        return result;
	  }
	    else
	  {
	    UGridArea ugarea(ax1,ay1,ax2,ay2);
	    result.addr = new UGrid(ugarea, 4000);  
	    return result;
	  }
	}
	else 
	{
      if( nl->ListLength(typeInfo) == 1)
	  {
        ListExpr area = nl->First(typeInfo);
		if (nl->IsEqual( area, UGRIDAREA))
		{
		  double ax1 = nl->RealValue( nl->First( area));
	      double ay1 = nl->RealValue( nl->Second( area));
	      double ax2 = nl->RealValue( nl->Third( area));
	      double ay2 = nl->RealValue( nl->Fourth( area));
		  if (( ax2 == ax1 ) || (ay1 == ay2 ))      
			  // vert or horizontal line   
	      {
            errMsg = "vertical or horizontal line or point";
		    cmsg.inFunError(errMsg);
            return result;
	      }
	      else
	      {
	        UGridArea ugarea(ax1,ay1,ax2,ay2);
            return SetWord( new UGrid(ugarea, 4000 ) );
		  }
		}
	  }	  
	}
  }
  return result;
}

/*
2.7.6.6   ~Delete~ function of class UGrid

*/
void UGrid::DeleteUGrid( const ListExpr typeInfo, Word& w )
{
  UGrid* ug = (UGrid*)w.addr;
  ug->DeleteFile();
  delete ug;
}

/*
2.7.6.7   ~Open~ function of class UGrid

*/
bool UGrid::OpenUGrid( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value )
{
  // read the fileid of the ugrid
  SmiFileId fileid;
  // Read the fileId of the grid
  valueRecord.Read( &fileid, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );
  // Create existing ugrid
  UGrid* ug = new UGrid(fileid);
  value.addr = ug;
  return true;
}

/*
2.7.6.8   ~Save~ function of class UGrid

*/
bool UGrid::SaveUGrid( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value )
{
  // Save fileid  of the grid
  SmiFileId fileId;
  UGrid *ug = (UGrid *)value.addr;
  fileId = ug->FileId();
  valueRecord.Write( &fileId, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );
  // Save the cells and cellcontainers of the grid
  ug->SaveCells();
  // Save the header of the grid
  ug->WriteHeader();
  return true;
}

/*
2.7.6.9   ~Close~ function of class UGrid

*/
void UGrid::CloseUGrid( const ListExpr typeInfo, Word& w )
{
  UGrid* ug = (UGrid*)w.addr;
  delete ug;
}

/*
2.7.6.10   ~Clone~ function of class UGrid

*/
Word UGrid::CloneUGrid( const ListExpr typeInfo, const Word& w )
{
  return SetWord( Address(0) );
}

/*
2.7.6.11   ~Cast~ function of class UGrid

*/
void* CastUGrid( void* addr)
{
  return ( 0 );
}

/*
2.7.6.12   ~SizeOfObj~ function of class UGrid

*/
int UGrid::SizeOfObjUGrid()
{
  return sizeof(UGrid);
}

/*
2.7.6.13   Type Description of class UGrid

*/
ListExpr UGrid::UGridProp()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,
	  "createugrid (area)"
    " where area = eukl.region");

  return
    (nl->TwoElemList(
         nl->TwoElemList(nl->StringAtom("Creation"),
                         nl->StringAtom("Example Creation")),
         nl->TwoElemList(examplelist,
                         nl->StringAtom("(let myugrid = createugrid"
                         " (area))"))));
}

/*
2.7.6.14   ~Check~ function of class UGrid

*/
bool UGrid::CheckUGrid(ListExpr type, ListExpr& errorInfo)
{
  return (nl->IsEqual(type, "ugrid"));
}

/*
2.7.6.15 creation of TypeConstructor instance of class UGrid

*/
TypeConstructor ugridTC( UGRID,
                        UGrid::UGridProp,
                        UGrid::OutUGrid,
                        UGrid::InUGrid,
                        0,
                        0,
                        UGrid::CreateUGrid,
                        UGrid::DeleteUGrid,
                        UGrid::OpenUGrid,
                        UGrid::SaveUGrid,
                        UGrid::CloseUGrid,
                        UGrid::CloneUGrid,
                        CastUGrid,
                        UGrid::SizeOfObjUGrid,
                        UGrid::CheckUGrid );
/*
3 Creating Operators
3.1  Create UGrid
3.1.1  auxiliary functions of operator ~createugrid~
3.1.2 Type Mapping of operator ~createugrid~

*/
ListExpr CreateUGridOpTypeMap(ListExpr args)
{
  string errMsg = "Expecting ugridarea";
  if (nl->ListLength(args) != 1 &&
	  nl->ListLength(args) != 4 )
  {
    errMsg = "createugridTMErr:wrong arg is inserted";
	ErrorReporter::ReportError(errMsg);
	return nl->SymbolAtom("typeerror");
  }
  else
  {
    if (nl->ListLength(args) == 1)
	{	
	  ListExpr area = nl->First(args);
	  if (nl->IsEqual( area, UGRIDAREA)) 
	  { 
	    return nl->SymbolAtom(UGRID) ;
      }
	  else
	  {
        errMsg = "createugridTMErr:ugridarea is expected";
	    ErrorReporter::ReportError(errMsg);
	    return nl->SymbolAtom("typeerror");
	  }
	}
	else
	{
      ListExpr x1 = nl->First(args),
               y1 = nl->Second(args),
	           x2 = nl->Third(args),
               y2 = nl->Fourth(args);
	  if (nl->IsEqual( x1, REAL)  &&
	      nl->IsEqual( y1, REAL)  &&
	      nl->IsEqual( x2, REAL)  &&
	      nl->IsEqual( y2, REAL) )
	  {
	    return nl->SymbolAtom(UGRID) ;
	  }
	  else
      {
        errMsg = "createugridTMErr:area is expected";
        ErrorReporter::ReportError(errMsg);
        return nl->SymbolAtom("typeerror");
	  }
    } // end  if(nl->ListLength(args) == 4)
  }
}
/*
3.1.3 Value Mapping of operator ~createugrid~

3.1.3.1  CreateUGridOpVMarea

*/
int CreateUGridOpVM_area(Word* args, Word& result, int message,
                          Word& local, Supplier s)
{
  UGrid *ug = (UGrid*)qp->ResultStorage(s).addr;
  UGridArea* area = static_cast<UGridArea*>( args[0].addr );
  ug = new UGrid(*area, 4000);
  result.setAddr( ug );
  return 0;
}

/*
3.1.3.2 CreateUGridOpVMdouble

*/
int CreateUGridOpVM_double(Word* args, Word& result, int message,
                          Word& local, Supplier s)
{
  UGrid *ug = (UGrid*)qp->ResultStorage(s).addr;
  double* ugx1 = static_cast<double*>(args[0].addr);
  double* ugy1 = (double*)args[1].addr;
  double* ugx2 = (double*)args[2].addr;
  double* ugy2 = (double*)args[3].addr;
  if (( *ugx1 == *ugx2 ) || (*ugy1 == *ugy2 ))      
	  // vert or horizontal line   
  {
	 ug = new UGrid();
	 ug->header.noOfCells = 0;
  }
  else
  {
     UGridArea* area = new UGridArea(*ugx1, *ugy1, *ugx2,*ugy2);
     ug = new UGrid(*area, 4000);
  }
  result.setAddr( ug );
  return 0;
}

/*
3.1.3.3 CreateUGridOpMap

*/
ValueMapping CreateUGridOpMap[] = {
  CreateUGridOpVM_area,
  CreateUGridOpVM_double
};
/*
3.1.4 Selection of operator ~createugrid~

*/
int  CreateUGridOpSelect( ListExpr args )
{
  ListExpr arg1 = nl->First(args);
  if ( nl->SymbolValue(arg1) == "ugridarea")
  {
    return 0;
  }
  if ( nl->SymbolValue(arg1) == "real")
  {
    return 1;
  }
  return -1; // This point should never be reached
}
/*
3.1.5 Specification of operator ~createugrid~

*/ 
const string CreateUGridSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>ugridarea -> ugrid " "</text--->"
  "<text>createugrid( _ )</text--->"
  "<text>Creates an UGrid with Euclidean area </text--->"
  "<text>let myugrid = createugrid(ugridarea)</text--->"
  ") )";
Operator createugrid (
         "createugrid",           // name
          CreateUGridSpec,        // specification
          2,                      // 2 value mappings
		  CreateUGridOpMap,       // value mapping
		  CreateUGridOpSelect,    // selection function
          CreateUGridOpTypeMap    // type mapping
);
/*
3.2  Insertunit
3.2.1  auxiliary functions of operator ~insertunit~
3.2.1.1   CalculatePosX
3.2.1.2   CalculatePosY
3.2.1.1   IdentifyColumn

*/
int IdentifyColumn(UGridArea  area, double posX, double xLength)
{
  int col, i;
  if (area.x1 < area.x2)
  {
    i = 0;
	while (area.x1 + (i * xLength) < posX)
	{	
	  i++;
	}
	col = i - 1;                                     //column of position
	return col;
  }
  if (area.x1 > area.x2)
  {
    i = 0;
	while (area.x1 - (i * xLength) > posX)
	{	
	  i++;
	}
	col = i - 1; 
	return col;
  }
  return 0;
}	

////  3.2.1.2   IdentifyLine
int IdentifyLine(UGridArea  area, double posY, double yLength)
{
  int line, i;
  if (area.y1 < area.y2)
  {
	i = 0;
	while (area.y1 + (i * yLength) < posY)
	{	
	  i++;
	}
	line = i - 1;                                    // line of position
	return line;
  }
  if (area.y1 > area.y2)
  {
	i = 0;
	while (area.y1 - (i * yLength) > posY)
	{	
	  i++;
	}
	line = i - 1;                                    // line of position
	return line;
  }
  return 0;
}

/*
3.2.1.1   InsertCell

*/
void InsertCell(UGridCell* modcell, int index, 
				SmiRecordId cellRecId, UGridCell *ugce)
{
  // insert cell unit
  modcell->cellArray[index].cellRecId = cellRecId;
  modcell->cellArray[index].cellabel = ugce->intlabel;
  modcell->cellArray[index].ugctimestart = ugce->firstTime;
  modcell->cellArray[index].ugctimeend = ugce->lastTime;
  modcell->cellArray[index].ugcspos.x = ugce->ucarea.x1;
  modcell->cellArray[index].ugcspos.y = ugce->ucarea.y1;
  modcell->cellArray[index].ugcepos.x = ugce->ucarea.x2;
  modcell->cellArray[index].ugcepos.y = ugce->ucarea.y2;
  modcell->modified = true;
  modcell->noOfEntries = index;
}

/*
3.2.1.2   InsertCellUnit

*/
bool InsertCellUnit(UGrid* ug, UGrid *mug, SmiRecordId cellRecId, 
					UGridCell *ugce)
{    
  // first evaluate the cellabel in slave ugrid
  int intlabel = ((((ugce->intlabel % 10000) /100) / 2) * 100) +  
	              (((ugce->intlabel % 10000) % 100) / 2);               
                   //line (*100) in slave ugrid, column in slave ugrid
  int slintlabel;
  dimsize dim = (dimsize)(sqrt((double)mug->header.noOfCells));
  slintlabel = (dim *10000 )+ intlabel;
  // read evaluated cell of slave ugrid
  int slindex = ((intlabel  / 100 )*
((int)(sqrt((double)mug->header.noOfCells)) ) + 
	                (intlabel % 100));
  const int ci = slindex;
  const UGridCell * cellunit = new UGridCell();
  mug->cells.Get(ci, cellunit);                // read the cell of slave ugrid
  // initiate new ugridcell for modifications
  UGridCell* modcell = new UGridCell();
  // check the cell is the right
  if (!(cellunit->intlabel == slintlabel))
  {
	return false;
  }
   *modcell = *cellunit;                   // cellunit is const
  int maxEntries = cellunit->noOfEntries;
  maxEntries ++;
  if (maxEntries > sizeCellArray)
  {
	return false;
  }
  // insert cell unit
  InsertCell(modcell, maxEntries, cellRecId, ugce);
  mug->cells.Put(ci, *modcell);
  return true;
}

////  3.2.1.3   InsertCellUnit2
bool InsertCellUnit2(UGrid* ug, UGrid *mug, SmiRecordId cellRecId, 
					 UGridCell *ugce)
{  
  // first evaluate the cellabel in slave ugrid
  int intlabel = ((((ugce->intlabel % 10000) /100) / 2) * 100) + 
	             (((ugce->intlabel % 10000) % 100) / 2);               
                    //line (*100) in slave ugrid, column in slave ugrid
  int slintlabel;
  dimsize dim = (dimsize)(sqrt((double)mug->header.noOfCells));
 
  slintlabel = (dim *10000 )+ intlabel;
  // read evaluated cell of slave ugrid
  int slindex = ((intlabel  / 100 )*
((int)(sqrt((double)mug->header.noOfCells)) ) + 
	                (intlabel % 100));
  const int ci = slindex;
  const UGridCell * cellunit = new UGridCell();
  mug->cells.Get(ci, cellunit);                // read the cell of slave ugrid
  // initiate new ugridcell for modifications
  UGridCell* modcell = new UGridCell();
  // check the cell is the right
  if (!(cellunit->intlabel == slintlabel))
  {
	return false;
  }
  *modcell = *cellunit;                   // cellunit is const
  int maxEntries = cellunit->noOfEntries;
  maxEntries ++;
  if (maxEntries > sizeCellArray)
  {
	return false;
  }
  else
  {
  // insert cell unit
  InsertCell(modcell, maxEntries, cellRecId, ugce);
  mug->cells.Put(ci, *modcell);
  return true;
  }
}

////  3.2.1.4   InsertCellUnit4
bool InsertCellUnit4(UGrid* ug, UGrid *mug, SmiRecordId cellRecId, 
					 UGridCell *ugce)
{  
  // first evaluate the cellabel in slave ugrid
  int intlabel = ((((ugce->intlabel % 10000) /100) / 2) * 100) + 
	             (((ugce->intlabel % 10000) % 100) / 2);               
                   //line (*100) in slave ugrid, column in slave ugrid
  int slintlabel;
  dimsize dim = (dimsize)(sqrt((double)mug->header.noOfCells));
  slintlabel = (dim *10000 )+ intlabel;
  // read evaluated cell of slave ugrid
  int slindex = ((intlabel  / 100 )*
((int)(sqrt((double)mug->header.noOfCells)) ) + 
	                (intlabel % 100));
  const int ci = slindex;
  const UGridCell * cellunit = new UGridCell();
  mug->cells.Get(ci, cellunit);               // read the cell of slave ugrid
  // initiate new ugridcell for modifications
  UGridCell* modcell = new UGridCell();
  // check the cell is the right
  if (!(cellunit->intlabel == slintlabel))
  {
	return false;
  }
  *modcell = *cellunit;                   // cellunit is const
  int maxEntries = cellunit->noOfEntries;
  maxEntries ++;
  if (maxEntries > sizeCellArray)
  {
	UGrid* sug = mug->header.ugcPtr;
	// If slave ugrid doesn't exist, initiate a slave ugrid
	if (mug->header.slaveUgridId == (SmiRecordId)0)
	{
      // ug = 32x32 master with the sigle link to file
	  sug = new UGrid(true, *mug, *ug);              // generate slave ugrid
	  // insert header recordid of slave ugrid in master ugrid
      mug->header.slaveUgridId = sug->header.ugridRecordId; 
	}
	// append the full ugridcell in the file
	SmiRecordId fcellRecno;
    SmiRecord fcellRecord;
	int AppendedRecord = ug->file.AppendRecord(fcellRecno, fcellRecord);
	assert (AppendedRecord);
    // insert cell unit incl. fcellRecno in slave ugrid
    bool slavestore = InsertCellUnit2(ug, sug, fcellRecno, modcell); 
    if (slavestore = false)
	{
	  return false;
	}
	modcell->ClearCu(); 
	maxEntries  = modcell->noOfEntries + 1;
  }
  // insert cell unit
  InsertCell(modcell, maxEntries, cellRecId, ugce);
  mug->cells.Put(ci, *modcell);
  return true;
}
/*
3.2.1.5   InsertCellUnit8

*/
bool InsertCellUnit8(UGrid* ug, UGrid *mug, SmiRecordId cellRecId, 
					 UGridCell *ugce)
{  
  // first evaluate the cellabel in slave ugrid
  int intlabel = ((((ugce->intlabel % 10000) /100) / 2) * 100) + 
	              (((ugce->intlabel % 10000) % 100) / 2);               
                    //line (*100) in slave ugrid, column in slave ugrid
  int slintlabel;
  dimsize dim = (dimsize)
(sqrt((double)mug->header.noOfCells));
  slintlabel = (dim *10000 )+ intlabel;
  // read evaluated cell of slave ugrid
  int slindex = ((intlabel  / 100 )*
((int)(sqrt((double)mug->header.noOfCells)) ) + 
	                (intlabel % 100));
  const int ci = slindex;
  const UGridCell * cellunit = new UGridCell();
  mug->cells.Get(ci, cellunit);                // read the cell of slave ugrid
  // initiate new ugridcell for modifications
  UGridCell* modcell = new UGridCell();
  // check the cell is the right
  if (!(cellunit->intlabel == slintlabel))
  {
	return false;
  }
  *modcell = *cellunit;                             // cellunit is const
  int maxEntries = cellunit->noOfEntries;
  maxEntries ++;
  if (maxEntries > sizeCellArray)
  {
	UGrid* sug = mug->header.ugcPtr;
	// If slave ugrid doesn't exist, initiate a slave ugrid
	if (mug->header.slaveUgridId == (SmiRecordId)0)
	{
      // ug = 32x32 master with the sigle link to file
	  sug = new UGrid(true, *mug, *ug);         // generate slave ugrid
		  // insert header recordid of slave ugrid in master ugrid
      mug->header.slaveUgridId = sug->header.ugridRecordId; 
	}
	// append the full ugridcell in the file
	SmiRecordId fcellRecno;
    SmiRecord fcellRecord;
	int AppendedRecord = ug->file.AppendRecord(fcellRecno, fcellRecord);
	assert (AppendedRecord);
    // insert cell unit incl. fcellRecno in slave ugrid
    bool slavestore = InsertCellUnit4(ug, sug, fcellRecno, modcell); 
    if (slavestore = false)
	{
	  return false;
	}
	modcell->ClearCu(); 
	maxEntries  = modcell->noOfEntries + 1;
  }
  // insert cell unit
  InsertCell(modcell, maxEntries, cellRecId, ugce);
  mug->cells.Put(ci, *modcell);
  return true;
}

/*
3.2.1.6   InsertCellUnit16

*/
bool InsertCellUnit16(UGrid* ug, UGrid *mug, SmiRecordId cellRecId, 
					  UGridCell *ugce)
{  
  // first evaluate the cellabel in slave ugrid
  int intlabel = ((((ugce->intlabel % 10000) /100) / 2) * 100) + 
	              (((ugce->intlabel % 10000) % 100) / 2);               
                   //line (*100) in slave ugrid, column in slave ugrid
  int slintlabel;
  dimsize dim = (dimsize)(sqrt((double)mug->header.noOfCells));
  slintlabel = (dim *10000 )+ intlabel;
  // read evaluated cell of slave ugrid
  int slindex = ((intlabel  / 100 )*
((int)(sqrt((double)mug->header.noOfCells)) ) + 
	                (intlabel % 100));
  const int ci = slindex;
  const UGridCell * cellunit = new UGridCell();
  mug->cells.Get(ci, cellunit);              // read the cell of slave ugrid
  // initiate new ugridcell for modifications
  UGridCell* modcell = new UGridCell();
  // check the cell is the right
  if (!(cellunit->intlabel == slintlabel))
  {
	return false;
  }
  *modcell = *cellunit;                   // cellunit is const
  int maxEntries = cellunit->noOfEntries;
  maxEntries ++;
  if (maxEntries > sizeCellArray)
  {
	// If slave ugrid doesn't exist, initiate a slave ugrid
	UGrid* sug = mug->header.ugcPtr;
	if (mug->header.slaveUgridId == (SmiRecordId)0)
	{
      // ug = 32x32 master with the sigle link to file
	  sug = new UGrid(true, *mug, *ug);         // generate slave ugrid
		  // insert header recordid of slave ugrid in master ugrid
      mug->header.slaveUgridId = sug->header.ugridRecordId; 
	}
	// append the full ugridcell in the file
	SmiRecordId fcellRecno;
    SmiRecord fcellRecord;
	int AppendedRecord = ug->file.AppendRecord(fcellRecno, fcellRecord);
	assert (AppendedRecord);
    // insert cell unit incl. fcellRecno in slave ugrid
    bool slavestore = InsertCellUnit8(ug, sug, fcellRecno, modcell); 
    if (slavestore = false)
	{
	  return false;
	}
	modcell->ClearCu(); 
	maxEntries  = modcell->noOfEntries + 1;
  }
  // insert cell unit
  InsertCell(modcell, maxEntries, cellRecId, ugce);
  mug->cells.Put(ci, *modcell);
  return true;
}

/*
3.2.1.7.  InsertCellUnit32

*/
bool InsertCellUnit32(UGrid* ug, UGrid *mug, SmiRecordId cellRecId, 
					  UGridCell *ugce)
{ 
  // append full ugridcell ugce in ugrid ug file, append a cellunit in 
  // slave ugrid mug with the assigned recordid and clear the ugridcell ugce
  // first evaluate the cellabel in slave ugrid
  int intlabel = (((ugce->intlabel /100) / 2) * 100) +       
	                           //line (*100) in slave ugrid
			      ((ugce->intlabel % 100) / 2);               
                              //column in slave ugrid
  int slintlabel;
  dimsize dim = (dimsize)(sqrt((double)mug->header.noOfCells));
  slintlabel = (dim *10000 )+ intlabel;
  // read evaluated cell of slave ugrid
  int slindex = ((intlabel  / 100 )*
((int)(sqrt((double)mug->header.noOfCells)) ) + 
	                (intlabel % 100));
  const int ci = slindex;
  const UGridCell * cellunit = new UGridCell();
  mug->cells.Get(ci, cellunit);             // read the cell of slave ugrid
  // initiate new ugridcell for modifications
  UGridCell* modcell = new UGridCell();
  // check the cell is the right
  if (!(cellunit->intlabel == slintlabel))
  {
	return false;
  } 
  *modcell = *cellunit;                   // cellunit is const
  int maxEntries = cellunit->noOfEntries;
  maxEntries ++;
  if (maxEntries > sizeCellArray)
  {
    UGrid* sug = mug->header.ugcPtr;
	// If slave ugrid doesn't exist, initiate a slave ugrid
	if (mug->header.slaveUgridId == (SmiRecordId)0)
	{
      // ug = 32x32 master with the sigle link to file
	  sug = new UGrid(true, *mug, *ug);    // generate slave ugrid
		  // insert header recordid of slave ugrid in master ugrid
      mug->header.slaveUgridId = sug->header.ugridRecordId; 
	}
	// append the full ugridcell in the file
	SmiRecordId fcellRecno;
    SmiRecord fcellRecord;
	int AppendedRecord = ug->file.AppendRecord(fcellRecno, fcellRecord);
	assert (AppendedRecord);
    // insert cell unit incl. fcellRecno in slave ugrid
    bool slavestore = InsertCellUnit16(ug, sug, fcellRecno, modcell); 
    if (slavestore = false)
	{
	  return -1;
	}
	modcell->ClearCu(); 
	maxEntries  = modcell->noOfEntries + 1;
  }
  // insert cell unit
  InsertCell(modcell, maxEntries, cellRecId, ugce);
  mug->cells.Put(ci, *modcell);
  return true;
}
/*
3.2.1.8.  InsertHistoryUnit

*/
void InsertHistoryUnit (UGrid * ug, UGridCell* ugc, int index, HistoryUnit* hu)
{
  ugc->historyArray[ index - 1 ].hid = hu->hid;
  ugc->historyArray[ index - 1 ].spos.x = hu->spos.x;
  ugc->historyArray[ index - 1 ].spos.y = hu->spos.y;
  ugc->historyArray[ index - 1 ].epos.x = hu->epos.x;
  ugc->historyArray[ index - 1 ].epos.y = hu->epos.y;	
  ugc->historyArray[ index - 1 ].htimestart = hu->htimestart;
  ugc->historyArray[ index - 1 ].htimeend = hu->htimeend;
  ugc->noOfEntries = index;
  if (ugc->firstTime > hu->htimestart)
  {
	ugc->firstTime = hu->htimestart;
  }
  if (ugc->lastTime < hu->htimeend)
  {
    ugc->lastTime = hu->htimeend;
  }
  if (ug->header.firstTime > hu->htimestart)
  {
    ug->header.firstTime = hu->htimestart;
  }
  if (ug->header.lastTime < hu->htimeend)
  {
    ug->header.lastTime = hu->htimeend;
  }
}
/*
3.2.1.9.  Inserthandle

*/
int Inserthandle (CurrentUnit *cu, UGrid* ug, UpdateUnit* upunit,Word& result)
{
  dimsize dim = (dimsize)32; 
  int intdim = (int)dim;
  int id = upunit->GetUId();
  double x = upunit->GetXPos();
  double y = upunit->GetYPos();
  MobPos pos;
  pos.x = x;
  pos.y = y;
  //
  // check position inside area of ugrid
  //
  UGridArea gridarea = ug->header.area;
  if  (((gridarea.x1 <= x && gridarea.x2 >= x ) ||
       (gridarea.x1 >= x && gridarea.x2 <= x ))     &&
      ((gridarea.y1 <= y && gridarea.y2 >= y ) ||
	   (gridarea.y1 >= y && gridarea.y2 <= y )))
  {
  //
  // current timestamp / datatype = Instant
  //
  //Instant t1 = upunit->GetTime(); //timestamp in updateunit isn't current
  DateTime t2(instanttype);        // current timestamp
  t2.Now();
  //
  // current timestamp in seconds / datatype = long
  //
  long currenttime = LongTime();
  //
  //search entry (id) in currentmap - no entry in ugrid
  // 
  bool firstUpdate;
  firstUpdate = myCurrentUnit.searchEntry(id); 
  if (!firstUpdate)
  { // generate new entry in currentmap, no entry in ugrid
	double nx = 0;
	double ny = 0;
	//generate currentunit
	myUtilities.firstPredictPosition (nx, ny, x, y);
	MobPos npos;
	npos.x = nx;
	npos.y = ny;
	long predtime = currenttime + 60;    // timestamp for predicted position
	// entry in currentmap
	myCurrentUnit.enter (id, currenttime, predtime, pos.x, pos.y, 
		                 npos.x, npos.y);
	myCurrentUnit.dump();
	// time in Secondo-format Instant for result
	Instant ctime1 = LongToInstant(currenttime);
	Instant ctime2 = LongToInstant(predtime);
	//currentunit generated according to the updateunit and prediction 
	Interval<Instant>  t  = 
		Interval<Instant>(ctime1, ctime2, true, false);	
	cu = new CurrentUnit(id, t, pos, npos);
	// output
	result.addr = cu;
  }
  else // entry in ugrid - entry in current map exists and is modified
  { // modify existing entry in currentunit with id 
    // read current unit
    long cutimestamp = 0;
	double cux = 0;
	double cuy = 0;
    myCurrentUnit.readEntry (id, cutimestamp, cux, cuy);  
	                      // predicted timestamp not relevant
	//
	// generate historyunit
	//
	MobPos spos;     // startposition = position out of currentunit
	spos.x = cux;
	spos.y = cuy;
	// starttimestamp = cutimestamp, endtimestamp = currenttime
	HistoryUnit* newhu = new HistoryUnit(id, cutimestamp, currenttime, 
		                                 spos, pos);
	// enter historyunit in historymap and display it
	myHistoryUnit.enter(id, cutimestamp, currenttime, spos.x, spos.y, 
		                pos.x, pos.y);
	myHistoryUnit.dump();
	//
	// insert historyunit in ugrid-ugridcell
	//
	//
	// Read UGridarea and length/height of ugridcell
	//
	// Calculate length and heigth of area in ugrid and in ugridcell
    double xLength, yLength, xCellength, yCellength;
    xLength = abs(gridarea.x2 - gridarea.x1); // length of x-coord in ugrid 
    yLength = abs(gridarea.y2 - gridarea.y1); // length of y-coord in ugrid
    xCellength = xLength / intdim;            // length of x-coord in ugridcell 
    yCellength = yLength / intdim;            // length of y-coord in ugridcell
	//select ugridcell for entry
	// first we have to identify line and column of start- and endposition
	int sposcol, sposline, eposcol, eposline;
	sposcol = IdentifyColumn(gridarea, spos.x, xCellength);
	sposline = IdentifyLine(gridarea, spos.y, yCellength);
	eposcol = IdentifyColumn(gridarea, pos.x, xCellength);
	eposline = IdentifyLine(gridarea, pos.y, yCellength);
    int celllabel = 0 ;                  // label of ugridcell
	int cellindex = 0 ;                  // index of ugridcell
    husplit contflag = none;             // flag continue splitting
	// while trajectory merge more than one ugridcell 
    while ((sposcol < eposcol) || (sposline < eposline) ||    
		   (sposcol > eposcol) || (sposline > eposline))
    {
	  //
	  // identify the ugridcell of the start position
      //
      celllabel = (sposline * 100) + sposcol;               
	  cellindex = (sposline * intdim )+ (sposcol % intdim); 
	  // calculate the the coordinates of the boundary   
	  double boundxl, boundxr,boundyu, boundyl;
	  if (gridarea.x1 < gridarea.x2)
	  {
        boundxl = gridarea.x1 + (sposcol * xCellength);            
		                    // x-coord of the left edge
	    //boundxr = gridarea.x1 + ((sposcol + 1 ) * xCellength);   
		                    // x-coord of the right edge
		boundxl = boundxl +  xCellength;  
	  }
	  else
	  {
        boundxl = gridarea.x1 - (sposcol * xCellength);            
		                     // x-coord of the left edge
	    //boundxr = gridarea.x1 - ((sposcol + 1 ) * xCellength);   
		                     // x-coord of the right edge
		boundxr = boundxl -  xCellength;                         
		                     // x-coord of the right edge
	  }
	  if (gridarea.y1 < gridarea.y2)
	  {
	    boundyu = gridarea.y1 + ((sposline + 1) * yCellength);   
		                              // y-coord of the upper edge 
	    boundyl = gridarea.y1 + (sposline * yCellength);         
		                             // y-coord of the lower edge
	  }
	  else 
	  {
        boundyu = gridarea.y1 - ((sposline + 1)  * yCellength);   
		                             // y-coord of the upper edge 
	    boundyl = gridarea.y1 - (sposline  * yCellength);         
		                            // y-coord of the lower edge
	  }
	  
	  UGridCell* ugcput = new UGridCell();                        
	                               // create ugridcell for Put new track  
      const UGridCell* ugcget = new UGridCell(); 
	    // create ugridcell for Get stored ugridcell
	  const int ci = cellindex;
      if ((cellindex < ug->cells.Size()) && (cellindex >= 0))     
		                          // check index of ugridcell
	  {
		ug->cells.Get(ci, ugcget); 
		//int ugclabel = ugcget->intlabel;
	  }
	  else 
	  {
	    break;     // ugridcell can not be identified - should never happen
	  }
      //
	  //  Utilities for entry history unit
	  //
	  // determine delta time
	  long dtime = newhu->htimeend - newhu->htimestart;
      // direction of trajectory
	  double trajdir = myUtilities.Direction (newhu->spos.x, newhu->spos.y, 
		                          newhu->epos.x, newhu->epos.y);
	  double dir = trajdir / 180 * PI ;              // direction in radian 
	  // calculate the track of trajectory line
      double trajectory = sqrt(
(double)(pow((newhu->epos.y - newhu->spos.y),2) + 
              pow((newhu->epos.x - newhu->spos.x),2))); 
	  // speed of the trajectory
	  double vel = myUtilities.Speed(trajectory,dtime);
      //
      // find intersections
	  //
	  if ((0 <= trajdir) && (trajdir <= 180))
	  { 
		//possible intersection with the upper horizontal boundary line 
        // intersetion: y-coord known - calculate time of intersection
	    double sectimeuh = (boundyu - newhu->spos.y) / (vel * sin(dir));
	    // calculate the x-coord at the sectime
        double secpointx = newhu->spos.x + ((vel * sectimeuh) * (cos(dir)));
	    if (((sectimeuh > 0) && (sectimeuh < dtime))   &&
		    (boundxl <= secpointx  &&  secpointx  <=  boundxr))
	    { 
          //
		  // Intersection with upper horizontal boundary line
          //
		  // split trajectory in additional history unit 
		  // newhu1 and mod newhu
          HistoryUnit * newhu1 = new HistoryUnit();
          newhu1->hid = newhu->hid;
          newhu1->spos.x = newhu->spos.x;
		  newhu1->spos.y = newhu->spos.y;
          newhu1->epos.x = secpointx;
		  newhu1->epos.y = boundyu;
          newhu1->htimestart = newhu->htimestart;
          newhu1->htimeend = newhu->htimestart + (long)sectimeuh;
		  // newhu1 put in ugridcell 
		  if (!ugcget->IsModified())
		  {
            ug->header.noOfModCells ++;  
		  }
		  *ugcput = *ugcget;            // ugcget is const
		  int maxEntries = ugcput->noOfEntries;
		  maxEntries ++;
		  if (maxEntries > (sizeHistoryArray))
		  {
		    // If slave ugrid doesn't exist, initiate a slave ugrid
		    if (ug->header.slaveUgridId == (SmiRecordId)0)
		    {
              //SmiFileId ugridFileId = ug->FileId();
			  UGrid* ugL1 = new UGrid(true,*ug, *ug);     
			  // generate slave ugrid
			  // insert header recid of slave ugrid in master ugrid
			  ug->header.slaveUgridId = ugL1->header.ugridRecordId; 
		    }
		    // append the full ugridcell in the file
		    SmiRecordId fcellRecno;
            SmiRecord fcellRecord;
		    int AppendedRecord = ug->file.AppendRecord
				(fcellRecno, fcellRecord);
		    assert (AppendedRecord);
            // insert cell unit incl. fcellRecno in slave ugrid
            bool slavestore = InsertCellUnit32(ug, ug, fcellRecno, ugcput); 
            if (slavestore = false)
		    {
			  return -1;
		    }
		    ugcput->ClearHu(); 
		    maxEntries  = ugcput->noOfEntries + 1;
		  }
		  InsertHistoryUnit (ug, ugcput, maxEntries, newhu1);
		  // new history unit (spos secpoint)inserted
		  ug->header.noOfEntries ++;
          ug->cells.Put(ci,*ugcput);   // store ugridcell with new historyunit 
		  // modify historyunit (secpoint epos)
          newhu->spos.x = secpointx;
          newhu->spos.y = boundyu;
          newhu->htimestart = newhu->htimestart + (long)sectimeuh;
          contflag = linesp;  
		  // start position of modified history unit now 1 line upper
		}
		else  // no intersection with upper horizontal boundary line
		{
          if ((0 <= trajdir) && (trajdir <= 90))
	      { 
			// possible intersection with the 
			//  right vertical boundary line 
            // intersetion: x-coord known - calculate time of intersection
	        double sectimerv = (boundxr - newhu->spos.x) / (vel * cos(dir));
		    // calculate the y-coord at the sectime
            double secpointy = newhu->spos.y + (vel * sectimerv * sin(dir));
		    if (((sectimerv > 0) && (sectimerv < dtime))  &&
                ((boundyl <= secpointy)  && ( secpointy  <=  boundyu))) 
	        {
              // Intersection with right vertical boundary line
		      // split trajectory in add. history unit 
			  // newhu1 and mod newhu
              HistoryUnit * newhu1 = new HistoryUnit();
              newhu1->hid = newhu->hid;
              newhu1->spos = newhu->spos;
              newhu1->epos.x = boundxr;
		      newhu1->epos.y = secpointy;
              newhu1->htimestart = newhu->htimestart;
              newhu1->htimeend = newhu->htimestart + (long)sectimerv;
		      // nhu1 put in ugridcell
		      if (!ugcget->IsModified())
		      {
                ug->header.noOfModCells ++;  
		      }
              *ugcput = *ugcget;
		      int maxEntries = ugcput->noOfEntries;
		      maxEntries ++;
		      if (maxEntries > (sizeHistoryArray))
		      {
		        // If slave ugrid doesn't exist, initiate a slave ugrid
		        if (ug->header.slaveUgridId == (SmiRecordId)0)
		        {
                  //SmiFileId ugridFileId = ug->FileId();
			      UGrid* ugL1 = new UGrid(true,*ug, *ug); 
				                    // generate slave ugrid
			      // insert header recordid of slave ugrid 
				  // in master ugrid
                  ug->header.slaveUgridId = ugL1->header.ugridRecordId; 
		        }
		        // append the full ugridcell in the file
		        SmiRecordId fcellRecno;
                SmiRecord fcellRecord;
		        int AppendedRecord = ug->file.AppendRecord(fcellRecno, 
					fcellRecord);
		        assert (AppendedRecord);
                // insert cell unit incl. fcellRecno in slave ugrid
                bool slavestore = InsertCellUnit32(ug, ug, fcellRecno, ugcput); 
                if (slavestore = false)
		        {
			      return -1;
		        }
		        ugcput->ClearHu(); 
		        maxEntries  = ugcput->noOfEntries + 1;
		      }
			  // hu inserted
		      ug->header.noOfEntries ++;
              ug->cells.Put(ci,*ugcput);
		      // modify newhu
              newhu->spos.x = boundxr;
              newhu->spos.y = secpointy;
              newhu->htimestart = newhu->htimestart + (long)sectimerv;
              contflag = colsp;            
			  // start position of modified history unit
			  // now 1 column right
			}
            else  // no intersection historyunit - right vertical boundary line
            {     // 90 <= trajdir <= 180
              // possible intersection with left vertical boundaryline
              // intersetion: x-coord known - calculate time of intersection 
	          double sectimelv = (newhu->spos.x - boundxl) 
				  / (vel * cos(dir));
			  // calculate the y-coord at the sectime
              double secpointy = newhu->spos.y + (vel * sectimelv * sin(dir));
              if (((sectimelv > 0) && (sectimelv < dtime))  &&
				  ((boundyl <= secpointy)  &&  (secpointy 
				  <=  boundyu)))
	          {
                // Intersection with left vertical boundary line
		        // split trajectory in add. history unit newhu1 
				// and mod newhu
                HistoryUnit * newhu1 = new HistoryUnit();
                newhu1->hid = newhu->hid;
                newhu1->spos.x = newhu->spos.x;
				newhu1->spos.y = newhu->spos.y;
                newhu1->epos.x = boundxl;
		        newhu1->epos.y = secpointy;
                newhu1->htimestart = newhu->htimestart;
                newhu1->htimeend = newhu->htimestart + (long)sectimelv;
		        // nhu1 put in ugridcell
		        if (!ugcget->IsModified())
		        {
                  ug->header.noOfModCells ++;  //only first entry in Cell
		        }
				*ugcput = *ugcget;      // ugcget is const
		        int maxEntries = ugcput->noOfEntries;
		        maxEntries ++;
		        if (maxEntries >= (sizeHistoryArray+1))
		        {
		          //If slave ugrid doesn't exist, initiate a slave ugrid
		          if (ug->header.slaveUgridId == (SmiRecordId)0)
		          {
                    //SmiFileId ugridFileId = ug->FileId();
			        UGrid* ugL1 = new UGrid(true,*ug, *ug);     
					                // generate slave ugrid
			        // insert header recordid of slave ugrid 
					// in master ugrid
                    ug->header.slaveUgridId = ugL1->header.ugridRecordId; 
		          }
		          // append the full ugridcell in the file
		          SmiRecordId fcellRecno;
                  SmiRecord fcellRecord;
		          int AppendedRecord = ug->file.AppendRecord(
					  fcellRecno, fcellRecord);
		          assert (AppendedRecord);
                  // insert cell unit incl. fcellRecno in slave ugrid
                  bool slavestore = InsertCellUnit32(ug, ug, 
					    fcellRecno, ugcput); 
                  if (slavestore = false)
		          {
			        return -1;
		          }
		          ugcput->ClearHu(); 
		          maxEntries  = ugcput->noOfEntries + 1;
		        }
				InsertHistoryUnit (ug, ugcput, 
					maxEntries, newhu1);
			    // new historyunit (spos secpoint) inserted
		        ug->header.noOfEntries ++;
                ug->cells.Put(ci,*ugcput);
				// modify newhu
                newhu->spos.x = boundxl;
                newhu->spos.y = secpointy;
                newhu->htimestart = newhu->htimestart + (long)sectimelv;
                contflag = colsm;        
				// start position of modified history unit 
				// now 1 column left
			  }// no intersection with right vertical line
			} //  end else 90 <= trajdir <= 180 
		  }  // end if 0 <= trajdir <= 90
		}   // end else no intersection with upper horizontal line
	  }  // end  if (0 <= trajdir <= 180)
	  if ((180 < trajdir) && (trajdir <= 360))  // trajectory downward
	  { 
		//possible intersection with the lower horizontal boundary line 
        // intersetion: y-coord known - calculate time of intersection
	    double sectimelh = - ((newhu->spos.y - boundyl)
			  / (vel * sin(dir))); 
		// sin(dir) <0 because downwards
	    // calculate the x-coord at the sectimelh
        double secpointx = newhu->spos.x + ((vel * sectimelh) * (cos(dir)));
	    if (((sectimelh > 0) && (sectimelh < dtime))   &&
		    (boundxl <= secpointx  &&  secpointx  <=  boundxr))
	    { 
          //
		  // Intersection with lower horizontal boundary line
          //
		  // split trajectory in add. history unit newhu1 and mod newhu
          HistoryUnit * newhu1 = new HistoryUnit();
          newhu1->hid = newhu->hid;
          newhu1->spos.x = newhu->spos.x;
		  newhu1->spos.y = newhu->spos.y;
          newhu1->epos.x = secpointx;
		  newhu1->epos.y = boundyl;
          newhu1->htimestart = newhu->htimestart;
          newhu1->htimeend = newhu->htimestart + (long)sectimelh;
		  // nhu1 put in ugridcell
		  if (!ugcget->IsModified())
		  {
            ug->header.noOfModCells ++;  // TBD  nur bei erstem Eintrag in Cell
		  }
		  *ugcput = *ugcget;                // ugcget is const
		  int maxEntries = ugcput->noOfEntries;
		  maxEntries ++;
		  UGrid* sug = ug->header.ugcPtr;
		  if (maxEntries > (sizeHistoryArray))
		  {
		    // If slave ugrid doesn't exist, initiate a slave ugrid
		    if (ug->header.slaveUgridId == (SmiRecordId)0)
		    {
              //SmiFileId ugridFileId = ug->FileId();
			  sug = new UGrid(true, *ug, *ug);    
			    // generate slave ugrid
			  // insert header recordid of slave ugrid 
			  // in master ugrid
              ug->header.slaveUgridId = sug->header.ugridRecordId; 
		    }
		    // append the full ugridcell in the file
		    SmiRecordId fcellRecno;
            SmiRecord fcellRecord;
		    int AppendedRecord = ug->file.AppendRecord(
				fcellRecno, fcellRecord);
		    assert (AppendedRecord);
            // insert cell unit incl. fcellRecno in slave ugrid
            bool slavestore = InsertCellUnit32(ug, sug, fcellRecno, ugcput); 
            if (slavestore = false)
		    {
			  return -1;
		    }
		    ugcput->ClearHu(); 
		    maxEntries  = ugcput->noOfEntries + 1;
		  }
		  InsertHistoryUnit (ug, ugcput, maxEntries, newhu1);
		  // new history unit (spos secpoint)inserted
		  ug->header.noOfEntries ++;
          ug->cells.Put(ci,*ugcput);   // store ugridcell with new historyunit 
		  // modify historyunit (secpoint epos)
          newhu->spos.x = secpointx;
          newhu->spos.y = boundyl;
          newhu->htimestart = newhu->htimestart + (long)sectimelh;
          contflag = linesm;           
		    // start position of modified history unit now 1 line upper
		}
		else  // no intersection with lower horizontal boundary line
		{
          if ((180 < trajdir) && (trajdir <= 270))
		  {
            // possible intersection with left vertical boundaryline
            // intersetion: x-coord known - calculate time of intersection 
	        double sectimelv = -((newhu->spos.x - boundxl) 
				/ (vel * cos(dir)));
			// calculate the y-coord at the sectime
            double secpointy = newhu->spos.y + (vel * sectimelv * sin(dir));
            if (((sectimelv > 0) && (sectimelv < dtime))  &&
			    ((boundyl <= secpointy)  &&  (secpointy  
				<=  boundyu)))
	        {
              // Intersection with left vertical boundary line
		      // split trajectory in add. history unit newhu1 
			  // and mod newhu
              HistoryUnit * newhu1 = new HistoryUnit();
              newhu1->hid = newhu->hid;
              newhu1->spos.x = newhu->spos.x;
			  newhu1->spos.y = newhu->spos.y;
              newhu1->epos.x = boundxl;
		      newhu1->epos.y = secpointy;
              newhu1->htimestart = newhu->htimestart;
              newhu1->htimeend = newhu->htimestart + (long)sectimelv;
		      // nhu1 put in ugridcell
		      if (!ugcget->IsModified())
		      {
                ug->header.noOfModCells ++;  //only first entry in Cell
		      }
			  *ugcput = *ugcget;             // ugcget is const
 		      int maxEntries = ugcput->noOfEntries;
		      maxEntries ++;
		      if (maxEntries > (sizeHistoryArray))
		      {
		        // If slave ugrid doesn't exist, initiate a slave ugrid
		        if (ug->header.slaveUgridId == (SmiRecordId)0)
		        {
                  //SmiFileId ugridFileId = ug->FileId();
			      UGrid* ugL1 = new UGrid(true, *ug, *ug);     
				                   // generate slave ugrid
			      // insert header recordid of slave ugrid 
				  // in master ugrid
                  ug->header.slaveUgridId = ugL1->header.ugridRecordId; 
		        }
		        // append the full ugridcell in the file
		        SmiRecordId fcellRecno;
                SmiRecord fcellRecord;
		        int AppendedRecord = ug->file.AppendRecord(
					fcellRecno, fcellRecord);
		        assert (AppendedRecord);
                // insert cell unit incl. fcellRecno in slave ugrid
                bool slavestore = InsertCellUnit32(ug, ug, fcellRecno, ugcput); 
                if (slavestore = false)
		        {
			      return -1;
		        }
		        ugcput->ClearHu(); 
		        maxEntries  = ugcput->noOfEntries + 1;
		      }
			  InsertHistoryUnit (ug, ugcput, maxEntries, newhu1);
			  // new historyunit (spos secpoint) inserted
		      ug->header.noOfEntries ++;
              ug->cells.Put(ci,*ugcput);
			  // modify newhu
              newhu->spos.x = boundxl;
              newhu->spos.y = secpointy;
              newhu->htimestart = newhu->htimestart + (long)sectimelv;
              contflag = colsm;      
			   // start position of modified history unit 
			   // now 1 column left
			} //  intersection with left vertical line
			else // ((270 <= trajdir) && (trajdir <= 360))
			{ 
			  // possible intersection with the 
			  // right vertical boundary line 
			  // intersetion: x-coord known - 
			  // calculate time of intersection
			  double sectimerv = -((boundxr - newhu->spos.x) / 
				                   (vel * cos(dir)));
			  // calculate the y-coord at the sectime
			  double secpointy = newhu->spos.y - 
				  (vel * sectimerv * sin(dir));
			  if (((sectimerv > 0) && 
				  (sectimerv < dtime))  &&
			      ((boundyl <= secpointy)  && 
				  ( secpointy  <=  boundyu))) 
			  {
			    // Intersection with right vertical boundary line
				// split trajectory in add. history unit 
				// newhu1 and mod newhu
				HistoryUnit * newhu1 = new HistoryUnit();
				newhu1->hid = newhu->hid;
				newhu1->spos = newhu->spos;
				newhu1->epos.x = boundxr;
				newhu1->epos.y = secpointy;
				newhu1->htimestart = newhu->htimestart;
				newhu1->htimeend = newhu->htimestart + 
					(long)sectimerv;
				// nhu1 put in ugridcell
				if (!ugcget->IsModified())
				{
				  ug->header.noOfModCells ++;  
				  //only first entry in Cell
				}
				*ugcput = *ugcget;             
				// ugcget is const
				int maxEntries = ugcput->noOfEntries;
				maxEntries ++;
				if (maxEntries >= (sizeHistoryArray+1))
				{
		          // If slave ugrid doesn't exist, 
				  // initiate a slave ugrid
		          if (ug->header.slaveUgridId == (SmiRecordId)0)
		          {
                    //SmiFileId ugridFileId = ug->FileId();
			        UGrid* ugL1 = new UGrid(true, *ug, *ug);    
					// generate slave ugrid
			        // insert header recordid of slave ugrid 
					// in master ugrid
                    ug->header.slaveUgridId = ugL1->header.ugridRecordId; 
		          }
		          // append the full ugridcell in the file
		          SmiRecordId fcellRecno;
                  SmiRecord fcellRecord;
		          int AppendedRecord = ug->file.AppendRecord(
					  fcellRecno, fcellRecord);
		          assert (AppendedRecord);
                  // insert cell unit incl. fcellRecno in slave ugrid
                  bool slavestore = InsertCellUnit32(ug, ug, 
					  fcellRecno, ugcput); 
                  if (slavestore = false)
		          {
			        return -1;
		          }
		          ugcput->ClearHu(); 
		          maxEntries  = ugcput->noOfEntries + 1;
				}
				InsertHistoryUnit 
					(ug, ugcput, maxEntries, newhu1);
				// hu inserted
				ug->header.noOfEntries ++;
				ug->cells.Put(ci,*ugcput);
				// modify newhu
				newhu->spos.x = boundxr;
				newhu->spos.y = secpointy;
				newhu->htimestart = 
					newhu->htimestart + (long)sectimerv;
				contflag = colsp;       
				// start position of modified history unit 
				// now 1 column  
			  } // intersection with right vertical boundary line
            } //((270 <= trajdir) && (trajdir <= 360))
		  }  // end ((180 < trajdir) && (trajdir <= 270))
		}   // end else no intersection with lower horizontal line
	  }  // end  if (180 <= trajdir <= 360)

	  if (contflag == none)          // no secpoint found
	  {
        break;
	  }
	  else
	  {
        if (contflag == colsp) 
        {
	      sposcol  =   sposcol + 1;
		  contflag = none;
		  if (sposcol < 0 || sposcol > intdim)
		  {
			  break;
		  }
	    }
	    if (contflag == colsm) 
        { 
	      sposcol  =   sposcol - 1;
		  contflag = none;
		  if (sposcol < 0 || sposcol > intdim)
		  {
			  break;
		  }
	    }
	    if (contflag == linesp)      
	    {
		  sposline  =   sposline + 1;
		  contflag = none;
		  if (sposline < 0 || sposline > intdim)
		  {
			  break;
		  }
	    }
	    if (contflag == linesm)     
	    {
	      sposline  =   sposline - 1;
		  contflag = none;
		  if (sposline < 0 || sposline > intdim)
		  {
			  break;
		  }
	    }
	  }
    } // end while
    //
    // insert newhu: spos and epos in the same ugridcell
    //
    // identify the ugridcell of the start position
    celllabel = (sposline * 100) + sposcol;                      
	cellindex = (sposline * intdim )+ (sposcol % intdim);       
    // read ugridcell
    if ((cellindex < ug->cells.Size()) && (cellindex >= 0))
    {
	  UGridCell* ugcput = new UGridCell();                  // for Put
      const UGridCell* ugcget = new UGridCell();            // for Get
	  const int ci = cellindex;
	  //cellRecord.Read(&ugc,sizeof(UGRIDCELL),offset);
	  ug->cells.Get(ci, ugcget); 
	  //int ugclabel = ugcget->intlabel;
	  if (!ugcget->IsModified())
	  {
        ug->header.noOfModCells ++;  // TBD  nur bei erstem Eintrag in Cell
	  }
	  if (!(ugcget->intlabel == celllabel))
	  {
		  ;
	  }
	  else
	  {
	    *ugcput = *ugcget;
		int maxEntries = ugcget->noOfEntries;
	    maxEntries ++;
		//UGrid* sug = new UGrid();    
	    if (maxEntries > (sizeHistoryArray))
	    {
		  // If slave ugrid doesn't exist, initiate a slave ugrid
		  if (ug->header.slaveUgridId == (SmiRecordId)0)
		  {
             //SmiFileId ugridFileId = ug->FileId();
			 UGrid* ugL1 = new UGrid(true, *ug, *ug);   
			 // generate slave ugrid
			 ugL1->header.noOfModCells = 0;
		  }
		  // append the full ugridcell in the file
		  SmiRecordId fcellRecno;
          SmiRecord fcellRecord;
		  int AppendedRecord = 
			  ug->file.AppendRecord(fcellRecno, fcellRecord);
		  assert (AppendedRecord);
          // insert cell unit incl. fcellRecno in slave ugrid
          bool slavestore = InsertCellUnit32(
			  ug, ug->header.ugcPtr, fcellRecno, ugcput); 
		  //bool slavestore = InsertCellUnit32
		  // (ug, sug, fcellRecno, ugcput);
          if (slavestore = false)
		  {
			return -1;
		  }
		  ugcput->ClearHu(); 
		  maxEntries  = ugcput->noOfEntries + 1;
	    }
		  ugcput->historyArray[ maxEntries - 1 ] = *newhu;
		  ugcput->noOfEntries = maxEntries;
		  if (ugcput->firstTime > newhu->htimestart)
		  {
		    ugcput->firstTime = newhu->htimestart;
		  }
		  if (ugcput->lastTime < newhu->htimeend)
		  {
            ugcput->lastTime = newhu->htimeend;
		  }
		  if (ug->header.firstTime > newhu->htimestart)
		  {
		    ug->header.firstTime = newhu->htimestart;
		  }
		  if (ug->header.lastTime < newhu->htimeend)
		  {
            ug->header.lastTime = newhu->htimeend;
		  }
		// hu inserted
	    ug->header.noOfEntries ++;
      ug->cells.Put(ci,*ugcput);

	  }// intlabel == celllabel
	} //newhu1 put in ugridcell
	//
	// evaluate next possible position for currentunit
	//
	double nx = 0;
	double ny = 0;
	myUtilities.predictPosition(nx, ny, x, y);
    MobPos npos;
	npos.x = nx;
	npos.y = ny;
	// entry in currentmap, time as long type
	myCurrentUnit.enter (id, currenttime, 
		currenttime+60, pos.x, pos.y, npos.x,npos.y);
	myCurrentUnit.dump();
	Instant cutime1 = LongToInstant(currenttime);
	Instant cutime2 = LongToInstant(currenttime + 60);
	Interval<Instant>  t  = Interval<Instant>(
		cutime1, cutime2, true, false);	
    cu = new CurrentUnit(id, t, pos, npos);
    result.addr = cu;
  }//end no first entry
  return 0;
  }
  else
  {
  return -1;
  }
}
/*
3.2.2 Type Mapping of operator ~insertunit~

*/
ListExpr InsertUnitOpTypeMap(ListExpr args)
{
  string errMsg = "Expecting updateunit";
  if (nl->ListLength(args) != 2 )
  {
    errMsg = "insertupdateunitTMErr:wrong arg is inserted";
	ErrorReporter::ReportError(errMsg);
	return nl->SymbolAtom("typeerror");
  }
  else
  {	  
	ListExpr ugrid = nl->First(args);
	ListExpr upunit = nl->Second(args);
	if (nl->IsEqual( ugrid, UGRID)    &&
		nl->IsEqual( upunit, UPDATEUNIT)) 
	{ 
	  return nl->SymbolAtom(CURRENTUNIT) ;
    }
  }
  return nl->SymbolAtom("typeerror");
}
/*
3.2.3 Value Mapping of operator ~insertunit~
3.3.3.1  InsertUnitOpVMunit

*/
int InsertUnitOpVM(Word* args, Word& result, int message,
                          Word& local, Supplier s)
{
  //dimsize dim = (dimsize)32; 
  // prepare result
  CurrentUnit *cu = (CurrentUnit*)qp->ResultStorage(s).addr;    
                      //resultaddress linked with currentunit
  // open assigned ugrid
  UGrid* ug = static_cast<UGrid*>( args[0].addr );              
                     // ugrid in args[0] will be opened
  //read inserted updateunit and display it
  UpdateUnit* upunit = static_cast<UpdateUnit*>( args[1].addr ); 
                         // updateunit in arg[1] is read
  int ret = Inserthandle (cu, ug, upunit, result);
  if (ret == -1 )
  { 
     cu->cid = 9999999;
	 cu->cTimeInterval.start.Today();
	 cu->cTimeInterval.end.Today();
	 cu->cpos.x = 9999999.99;
	 cu->cpos.y = 9999999.99;
	 cu->npos.x = 9999999.99;
	 cu->npos.y = 9999999.99;
	 result.setAddr(cu);
  }
  return 0;
}
/*
3.2.4 Selection of operator ~insertunit~
3.2.5 Specification of operator ~insertunit~

*/
const string InsertUnitSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>updateunit -> entry in ugrid" "</text--->"
  "<text>insertupdateunit _ infixop _</text--->"
  "<text>Inserts an Updateunit with Euclidean point </text--->"
  "<text>query ugrid insertunit(12(1005.5 1005.5))</text--->"
  ") )";

Operator insertunit (
         "insertunit",              // name
          InsertUnitSpec,           // specification
          InsertUnitOpVM,           // value mapping
		  Operator::SimpleSelect,    // selection function
          InsertUnitOpTypeMap       // type mapping
);
/*
3.3  Insertsingle
3.3.1  auxiliary functions of operator ~insertsingle~
3.3.2  Type Mapping of operator ~insertsingle~

*/
ListExpr InsertSingleOpTypeMap(ListExpr args)
{
  string errMsg = "Expecting moving object";
  if (nl->ListLength(args) != 4 )
  {
    errMsg = "insertsingleTMErr:wrong arg is inserted";
	ErrorReporter::ReportError(errMsg);
	return nl->SymbolAtom("typeerror");
  }
  else
  {		  
	ListExpr ugrid = nl->First(args);
	ListExpr id = nl->Second(args);
    ListExpr x = nl->Third(args);
    ListExpr y = nl->Third(args);
	if (nl->IsEqual( ugrid, UGRID)  &&
		nl->IsEqual( id, INT)       &&
		nl->IsEqual( x, REAL)       &&
	    nl->IsEqual(y, REAL)  )
	{
	  return  nl->SymbolAtom(CURRENTUNIT);
	}
	else
	{
      errMsg = "insertsingleTMErr:int, reals is expected";
	  ErrorReporter::ReportError(errMsg);
	  return nl->SymbolAtom("typeerror");
	}
  }
}
/*
3.3.3  Value mapping of operator ~insertsingle~

*/
int InsertSingleOpVM(Word* args, Word& result, int message,
                          Word& local, Supplier s)
{ //dimsize dim = (dimsize)32; 
  //output will be a currentunit
  CurrentUnit *cu = (CurrentUnit*)qp->ResultStorage(s).addr;
  // open assigned ugrid
  UGrid* ug = static_cast<UGrid*>( args[0].addr ); 
  // ugrid in args[0] will be opened
  // read the three arguments
  CcInt* id = static_cast<CcInt*>(args[1].addr);
  CcReal* x1 = static_cast<CcReal*>(args[2].addr);
  CcReal* y1 = static_cast<CcReal*>(args[3].addr);
  MobPos pos;
  pos.x = x1->GetRealval();
  pos.y = y1->GetRealval();
  int Id = id->GetIntval();
  UpdateUnit* up = new UpdateUnit(Id, pos);
  //currentunit according the processed updateunit
  int ret = Inserthandle (cu, ug, up, result);
  if (ret == -1 )
  { 
     cu->cid = 9999999;
	 cu->cTimeInterval.start.Now();
	 cu->cTimeInterval.end.Now();
	 cu->cpos.x = 9999999.99;
	 cu->cpos.y = 9999999.99;
	 cu->npos.x = 9999999.99;
	 cu->npos.y = 9999999.99;
	 result.setAddr(cu);
  }
  return 0;
}
/*
3.3.4  Selection of operator ~insertsingle~
3.3.5  Specification of operator ~insertsingle~

*/
const string InsertSingleSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>ugrid insertsingle Id x X1 x Y1-> int" "</text--->"
  "<text>insertsingle(12 123.4 345.6)</text--->"
  "<text>Inserts a single  of Euclidean points </text--->"
  "<text>query ugrid insertsingle(id ,x1, y1)</text--->"
  ") )";

Operator insertsingle (
         "insertsingle",              // name
          InsertSingleSpec,           // specification
          InsertSingleOpVM,           // value mapping
		  Operator::SimpleSelect,     // selection function
          InsertSingleOpTypeMap       // type mapping
);
/*
3.4  Insertstream
3.4.1  auxiliary functions of operator ~insertstream~
3.4.2  Type Mapping of operator ~insertstream~

*/
ListExpr InsertStreamOpTypeMap(ListExpr args)
{
  string errMsg = "Expecting moving object";
  if (nl->ListLength(args) != 3 )
  {
    errMsg = "insertstreamTMErr:wrong arg is inserted";
	ErrorReporter::ReportError(errMsg);
	return nl->SymbolAtom("typeerror");
  }
  else
  {		  
	ListExpr id = nl->First(args);
	ListExpr x = nl->Second(args);
	ListExpr y = nl->Third(args);
	if (nl->IsEqual( id, INT)  &&
		nl->IsEqual( x, REAL)  &&
		nl->IsEqual( y, REAL)  ) 
	{ 
	  return nl->SymbolAtom(CURRENTUNIT) ;
    }
	else
	{
      errMsg = "insertstreamTMErr:int, reals is expected";
	  ErrorReporter::ReportError(errMsg);
	  return nl->SymbolAtom("typeerror");
	}
  }
}
/*
3.4.3  Value mapping of operator ~insertstream~

*/
int InsertStreamOpVM(Word* args, Word& result, int message,
                          Word& local, Supplier s)
{
  CurrentUnit *cu = (CurrentUnit*)qp->ResultStorage(s).addr;
  //result.setAddr( cu );

  int id = (int)args[0].addr;
  double* x1 = static_cast<double*>(args[1].addr);
  double* y1 = (double*)(args[2].addr);
  MobPos pos;
  //UpdateUnit* upunit;
  pos.x = *x1;
  pos.y = *y1;
  cu = new CurrentUnit();
  cu->cid = id;
  result.setAddr( cu );
  return 0;
}
/*
3.4.4  Selection of operator ~insertstream~
3.4.5  Specification of operator ~insertstream~

*/
const string InsertStreamSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>ugrid insertstream Id x X1 x Y1-> int" "</text--->"
  "<text>insertstream(12 (123.4 345.6))</text--->"
  "<text>Inserts a stream  of Euclidean points </text--->"
  "<text>query ugrid insertstream(id(x1 y1))</text--->"
  ") )";

Operator insertstream (
         "insertstream",              // name
          InsertStreamSpec,           // specification
          InsertStreamOpVM,           // value mapping
		  Operator::SimpleSelect,     // selection function
          InsertStreamOpTypeMap       // type mapping
);
/*
3.5  windowintersectug
3.5.1  auxiliary functions of operator ~windowintersectug~
3.5.1.1   Winintersecttime

*/
long Winintersecttime (UGrid* ugrid, int index, UGridArea area)
{ 
  UGridArea gridarea = ugrid->header.area;
  if  ((((gridarea.x1 <= area.x1 && gridarea.x2 >= area.x1 ) ||
       (gridarea.x1 >= area.x1 && gridarea.x2 <= area.x1 ))     &&
      ((gridarea.y1 <= area.y1 && gridarea.y2 >= area.y1 ) ||
	   (gridarea.y1 >= area.y1 && gridarea.y2 <= area.y1 )))     &&
      (((gridarea.x1 <= area.x2 && gridarea.x2 >= area.x2 ) ||
       (gridarea.x1 >= area.x2 && gridarea.x2 <= area.x2 ))     &&
      ((gridarea.y1 <= area.y2 && gridarea.y2 >= area.y2 ) ||
	   (gridarea.y1 >= area.y2 && gridarea.y2 <= area.y2 ))) )
  {
    if (ugrid->header.noOfEntries != 0)
	{
      //int celllabel = 0 ;     
	  int cellindex = 0 ;     
	  int intdim = (int)(sqrt((double)ugrid->header.noOfCells));
	  // Identify xLenth, yLenth of ugrid and the ugridcell 
	  // according the area of the request 
	  double xLength, yLength, xCellength, yCellength;
	  xLength = abs(ugrid->header.area.x2 - ugrid->header.area.x1);
	  yLength = abs(ugrid->header.area.y2 - ugrid->header.area.y1);
	  xCellength = xLength / intdim;
	  yCellength = yLength / intdim;
	  long rtimes = 0;
	  long rtimee = 0;
	  //
	  // identify the ugridcells of the requested area
      //
      int sposcol = IdentifyColumn (ugrid->header.area, area.x1, xCellength);
	  int sposline = IdentifyLine (ugrid->header.area, area.y1, yCellength);
	  int eposcol = IdentifyColumn (ugrid->header.area, 
		  area.x2, xCellength);
	  int eposline = IdentifyLine (ugrid->header.area, area.y2, yCellength);
	  if (sposcol <= eposcol)
	  {
        if (sposline <= eposline)
		{
          for (int j = 0; (sposline + j)<= eposline; j++)
		  {
            for (int i = 0; (sposcol + i)<= eposcol; i++)
		    {
              sposcol = sposcol + i;
		      cellindex = (sposline * intdim) + (sposcol % intdim);
		      const UGridCell* ugc = new UGridCell();
		      const int ci = cellindex;
		      ugrid->cells.Get(ci, ugc);
		      for (j = 0; j < ugc->noOfEntries; j++)
		      {
                if (ugc->historyArray[j].hid == index)
			    {
                  rtimes =  ugc->historyArray[j].htimestart;
			      rtimee =  ugc->historyArray[j].htimeend;
			      //break;
			    }
		      }
		    } // all cells in one line read
		  } // all cells are read
		}
		else
		{
          for (int j = 0; (sposline - j)>= eposline; j++)
		  {
            for (int i = 0; (sposcol + i)<= eposcol; i++)
		    {
              sposcol = sposcol + i;
		      cellindex = (sposline * intdim) + (sposcol % intdim);
		      const UGridCell* ugc = new UGridCell();
		      const int ci = cellindex;
		      ugrid->cells.Get(ci, ugc);
		      for (j = 0; j < ugc->noOfEntries; j++)
		      {
                if (ugc->historyArray[j].hid == index)
			    {
                  rtimes =  ugc->historyArray[j].htimestart;
			      rtimee =  ugc->historyArray[j].htimeend;
			      break;
			    }
		      }
		    } // all cells in one line read
		  } // all cells are read 
		}
	  }
	  else   // sposcol > eposcol
	  {
      if (sposline <= eposline)
		{
          for (int j = 0; (sposline + j)<= eposline; j++)
		  {
            for (int i = 0; (sposcol - i)>= eposcol; i++)
		    {
              sposcol = sposcol + i;
		      cellindex = (sposline * intdim) + (sposcol % intdim);
		      const UGridCell* ugc = new UGridCell();
		      const int ci = cellindex;
		      ugrid->cells.Get(ci, ugc);
		      for (j = 0; j < ugc->noOfEntries; j++)
		      {
                if (ugc->historyArray[j].hid == index)
			    {
                  rtimes =  ugc->historyArray[j].htimestart;
			      rtimee =  ugc->historyArray[j].htimeend;
			      break;
			    }
		      }
		    } // all cells in one line read
		  } // all cells are read
		}
		else
		{
          for (int j = 0; (sposline - j)>= eposline; j++)
		  {
            for (int i = 0; (sposcol - i)>= eposcol; i++)
		    {
              sposcol = sposcol + i;
		      cellindex = (sposline * intdim) + (sposcol % intdim);
		      const UGridCell* ugc = new UGridCell();
		      const int ci = cellindex;
		      ugrid->cells.Get(ci, ugc);
		      for (j = 0; j < ugc->noOfEntries; j++)
		      {
                if (ugc->historyArray[j].hid == index)
			    {
                  rtimes =  ugc->historyArray[j].htimestart;
			      rtimee =  ugc->historyArray[j].htimeend;
			      //break;
			    }
		      }
		    } // all cells in one line read
		  } // all cells are read 
		}
	  }
	  // only a single result is possible for an given id
	  if (!(rtimes == 0))
	  {
        long retime = rtimes;
        return retime;
	  }
	  else 
	  {
		  return 0;
	  }
	}
	else  // no entrie in the ugrid
	{
      return 0;
	}
  }
  else  // requested area not assigned to the ugrid
  {
     return 0; 
  }
}

/*
3.5.1.2   

*/
UGridArea Winintersectarea (UGrid* ugrid, int index, long time)
{ 
UGridArea gridarea = ugrid->header.area; 
DateTime t2(instanttype);           //current datetime
t2.Now();	
long t1 = InstantToLong(t2);       // current datetime in long
long reqtime = t1 + time;          // requested time
if (time >= 0)
{
 cout << " Abfrage die nahe Zukunft betreffend " << endl;
 UGridArea* area = new UGridArea();
 bool entry = myCurrentUnit.searchEntry(index); 
 if (entry)
 {
   long cutime = 0;
   myCurrentUnit.readEntry(index,cutime, area->x1, area->y1);
   if (area->x1 != 0 &&  area->y1 != 0)
   {
     long t3 = (long) cutime;
     long reqtime = (t1 - t3) + time;
	 double m, speed, a, nx, ny;
     speed = 16.66;                   // angenommene Geschwindigkei = 60 km / h
     m = area->y1/area->x1;           // Steigungswinkel 
     a = atan(m);                     // Steigungswinkel in Grad
     nx = area->x1 + (reqtime * speed * cos(a));  // berechnete x-Koord in time
     ny = area->y1 + (reqtime * speed * sin(a));  // berechnete y-Koord in time 
	 area->x2 = nx;
	 area->y2 = ny;
   }
   else 
   {
     area->x1 = 0;
     area->y1 = 0;
     area->x2 = 0;
     area->y2 = 0;
   }
   return *area;
 }
 else
 {
   area->x1 = 0;
   area->y1 = 0;
   area->x2 = 0;
   area->y2 = 0;
 }
 return *area;
}
else
{  
   if ((ugrid->header.firstTime <= reqtime) &&
       (ugrid->header.lastTime >= reqtime))
   {
      for (int i = 0; i < ugrid->header.noOfCells; i++)
	  {
        //int x = i;
		const UGridCell* ugc = new UGridCell();
	    const int ci = i;
        ugrid->cells.Get(ci, ugc);
		for (int j = 0; j < ugc->noOfEntries; j++)
		{  
          if (ugc->historyArray[j].hid == index)
		  {
            if ((ugc->historyArray[j].htimestart <= time)  &&
			    (ugc->historyArray[j].htimeend   >= time))
			{
			  //break;
			  UGridArea* resarea = new UGridArea();
			  resarea->x1 = ugc->historyArray[j].spos.x;
			  resarea->y1 = ugc->historyArray[j].spos.y;
		      resarea->x2 = ugc->historyArray[j].epos.x;
			  resarea->y2 = ugc->historyArray[j].epos.y;
			  return *resarea;
			}
			else
			{
              if (i == 1)
			  {
				UGridArea* harea = new UGridArea();
               harea->x1 = ugc->historyArray[j].spos.x;
			   harea->y1 = ugc->historyArray[j].spos.y;
			   harea->x2 = ugc->historyArray[j].epos.x;
			   harea->y2 = ugc->historyArray[j].epos.y;
               return *harea ;
			  }
			}
	      }
		}
	  }
   }
}
  return gridarea;
}
/*
3.5.1.2   Winintersectarea

*/
int Winintersectid (UGrid* ugrid, UGridArea area, long time)
{ 
	// not yet implemented
	int id = 12;
	return id;
}
/*
3.5.2  Type Mapping of operator ~windowintersectug~

*/
ListExpr WindowIntersectUGOpTypeMap(ListExpr args)
{
  string errMsg = "Expecting ugrid with arguments";
  if (nl->ListLength(args) != 3)
  {
    errMsg = "wintersectTMErr:wrong arg is inserted";
	ErrorReporter::ReportError(errMsg);
	return nl->SymbolAtom("typeerror");
  }
  else
  {	
	ListExpr ugrid = nl->First(args);
	ListExpr arg1 = nl->Second(args);
	ListExpr arg2 = nl->Third(args);
	if (nl->IsEqual( ugrid, UGRID)  &&
		nl->IsEqual( arg1, UGRIDAREA) &&
		nl->IsEqual( arg2, REAL))
	{
	  return nl->SymbolAtom("int");
	}
	else 
	{
      if (nl->IsEqual( ugrid, UGRID)  &&
		  nl->IsEqual( arg1, INT)   &&
	      nl->IsEqual( arg2, UGRIDAREA) )
	  {
		return nl->SymbolAtom("instant");
	  }
      else
	  {
		if (nl->IsEqual( ugrid, UGRID)  &&
		    nl->IsEqual( arg1, INT) &&
	        nl->IsEqual( arg2, REAL) )
	    {  
	      return nl->SymbolAtom(UGRIDAREA) ;
	    }
		else
		{
          errMsg = "windowintersectTMErr:int, reals is expected";
	      ErrorReporter::ReportError(errMsg);
	      return nl->SymbolAtom("typeerror");
		}
	  }
	}
  }
}
/*
3.5.3  Value Mapping of operator ~windowintersectug~
3.5.3.1   WindowIntersectUGOpVMid

*/
int WindowIntersectUGOpVM_id(Word* args, Word& result, int message,
                              Word& local, Supplier s)
{
  CcInt *index = ( CcInt*)qp->ResultStorage(s).addr;
  
  UGrid* ugrid = (UGrid*)args[0].addr;
  CcReal* time = static_cast<CcReal*>(args[2].addr);
  UGridArea* area = static_cast<UGridArea*>(args[1].addr);
  double dtime = time->GetRealval();
  long ltime = (long)dtime;
  int res =  Winintersectid (ugrid, *area, ltime);
  index->Set(true, res); 
  result.setAddr(index);
  return 0;
}

/*
3.5.3.2   WindowIntersectUGOpVMtime

*/
int WindowIntersectUGOpVM_time(Word* args, Word& result, int message,
                              Word& local, Supplier s)
{
  Instant *time = (Instant*)qp->ResultStorage(s).addr;
  UGrid* ugrid = (UGrid*)args[0].addr;
  CcInt* index = static_cast<CcInt*>(args[1].addr);
  UGridArea* area = static_cast<UGridArea*>(args[2].addr);
  int id = index->GetIntval();
  long res =  Winintersecttime (ugrid, id, *area);
  Instant rtime = LongToInstant(res);
  *time = rtime;
  result.setAddr(time);
  return 0;
}

/*
3.5.3.3   WindowIntersectUGOpVMarea

*/
int WindowIntersectUGOpVM_area(Word* args, Word& result, 
							   int message,
                              Word& local, Supplier s)
{
  UGridArea* area = (UGridArea*)qp->ResultStorage(s).addr;
  UGrid* ugrid = (UGrid*)args[0].addr;
  CcInt* index = static_cast<CcInt*>(args[1].addr);
  CcReal* time = static_cast<CcReal*>(args[2].addr);
  double rtime = time->GetRealval();
  int id = index->GetIntval();
  long ltime = (long)rtime;
  UGridArea resarea = Winintersectarea (ugrid, id, ltime);
  area->x1 = resarea.x1;
  area->y1 = resarea.y1;
  area->x2 = resarea.x2;
  area->y2 = resarea.y2;
  result.setAddr( area );
  return 0;
}

/*
3.5.3.4   WindowIntersectUGMap

*/
ValueMapping WindowIntersectUGMap[] = {
  WindowIntersectUGOpVM_id,
  WindowIntersectUGOpVM_time,
  WindowIntersectUGOpVM_area
};
/*
3.5.4 Selection of operator ~windowintersectug~

*/
int  WindowIntersectUGOpSelect( ListExpr args )
{
  ListExpr arg1 = nl->Second(args);
  ListExpr arg2 = nl->Third(args);
  if ( nl->SymbolValue(arg1) == "ugridarea")
  {
    return 0;         
  }
  else 
  {
    if (( nl->SymbolValue(arg1) == "int") &&
	    ( nl->SymbolValue(arg2) == "ugridarea"))
    {
      return 1;
    }
    else
    {
	  if (( nl->SymbolValue(arg1) == "int") &&
	      ( nl->SymbolValue(arg2) == "real"))
      {
        return 2;
	  }
    }
  }
  return -1; // This point should never be reached
}
/*
3.5.5  Specification of operator ~windowintersectug~

*/
const string WindowIntersectUGSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>id x pos -> time " "</text--->"
  "<text>windowintersectug ( _, _ ) </text--->"
  "<text>Returns the ids resulting intersection"
     " with area .</text--->"
	 "<text>query ugrid = windowintersectsug (12,(123.4 345.6)) </text--->"
  ") )";

Operator windowintersectug (
         "windowintersectug",              // name
          WindowIntersectUGSpec,           // specification
		  3,
          WindowIntersectUGMap,           // value mapping
		  WindowIntersectUGOpSelect,       // selection function
          WindowIntersectUGOpTypeMap       // type mapping
);
/*
3.6  windowintersectsug
3.6.1  auxiliary functions of operator ~windowintersectsug~
3.6.2  Type Mapping of operator ~windowintersectsug~

*/
ListExpr WindowIntersectsUGOpTypeMap(ListExpr args)
{
  string errMsg = "Expecting ugrid with arguments";
  if (nl->ListLength(args) != 2 )
  {
    errMsg = "windowintersectsTMErr:wrong arg is inserted";
	ErrorReporter::ReportError(errMsg);
	return nl->SymbolAtom("typeerror");
  }
  else
  {	
	ListExpr ugrid = nl->First(args);
	ListExpr attr = nl->Second(args);
	if (nl->IsEqual( ugrid, UGRID)  &&
		nl->IsEqual( attr, INT) )
	{ 
	 return nl->ThreeElemList(
		        nl->TwoElemList(nl->StringAtom("TimeInterval: "),
			                    nl->SymbolAtom(REAL)),
			    nl->TwoElemList(nl->StringAtom("Area: "),
			                    nl->SymbolAtom(UGRIDAREA)),
                nl->TwoElemList(nl->StringAtom("Count: "),
			                    nl->SymbolAtom(INT))) ;
    }
	else
	{
      if (nl->IsEqual( ugrid, UGRID)  &&
		  nl->IsEqual( attr, REAL) )
	  { 
		return nl->ThreeElemList(
			       nl->TwoElemList(nl->StringAtom("Id: "),
			                       nl->SymbolAtom(INT)),
			       nl->TwoElemList(nl->StringAtom("Area: "),
			                       nl->SymbolAtom(UGRIDAREA)),
				   nl->TwoElemList(nl->StringAtom("Count: "),
			                       nl->SymbolAtom(INT))) ;
      } 
      else
      {
        if (nl->IsEqual( ugrid, UGRID)  &&
		    nl->IsEqual( attr, UGRIDAREA) )
	    { 
		  return nl->ThreeElemList(
			            nl->TwoElemList(
			                nl->StringAtom("TimeInterval: "),
			                nl->SymbolAtom(REAL)),
			            nl->TwoElemList(nl->StringAtom("Id: "),
			                nl->SymbolAtom(INT)),
					    nl->TwoElemList(
						    nl->StringAtom("Count: "),
						    nl->SymbolAtom(INT)) );
        } 
	    else
		{
          errMsg = "windowintersectTMErr:int, reals is expected";
	      ErrorReporter::ReportError(errMsg);
	      return nl->SymbolAtom("typeerror");
	    }
	  } // ugrid && real
    }
  }
}
/*
3.6.3  Vale Mapping of operator ~windowintersectsug~
3.6.3.1  WindowIntersectsUGOpVMid

*/
int WindowIntersectsUGOpVM_id(Word* args, Word& result, int message,
                              Word& local, Supplier s)
{
  int *id = (int*)qp->ResultStorage(s).addr;
  // not implemented - probably not lifelike
  /*UGrid* ugrid = (UGrid*)args[0].addr;
  int* index = static_cast<int*>(args[1].addr);
  UGridArea* area = (UGridArea*)(args[2].addr);
  double* x1 = static_cast<double*>(args[3].addr);*/
  *id = 5;
  result.setAddr( id );
  return 0;
}
/*
3.6.3.2  WindowIntersectsUGOpVMtime

*/
int WindowIntersectsUGOpVM_time(Word* args, Word& result, int message,
                              Word& local, Supplier s)
{
  int *id = (int*)qp->ResultStorage(s).addr;
   // not implemented - probably not lifelike
  /*UGrid* ugrid = (UGrid*)args[0].addr;
  int* index = static_cast<int*>(args[1].addr);
  UGridArea* area = (UGridArea*)(args[2].addr);
  double* x1 = static_cast<double*>(args[3].addr);
  *id = 5;*/
  result.setAddr( id );
  return 0;
}

/*
3.6.3.3  WindowIntersectsUGOpVMarea

*/
int WindowIntersectsUGOpVM_area(Word* args, Word& result, int message,
                              Word& local, Supplier s)
{
  int *id = (int*)qp->ResultStorage(s).addr;
   // not implemented - probably not lifelike
  /*UGrid* ugrid = (UGrid*)args[0].addr;
  int* index = static_cast<int*>(args[1].addr);
  UGridArea* area = (UGridArea*)(args[2].addr);
  double* x1 = static_cast<double*>(args[3].addr);*/
  *id = 5;
  result.setAddr( id );
  return 0;
}

/*
3.6.3.4  WindowIntersectsUGMap

*/
ValueMapping WindowIntersectsUGMap[] = {
  WindowIntersectsUGOpVM_id,
  WindowIntersectsUGOpVM_time,
  WindowIntersectsUGOpVM_area
};
/*
3.6.4  Selection of operator ~windowintersectsug~

*/
int  WindowIntersectsUGOpSelect( ListExpr args )
{
  ListExpr arg1 = nl->Second(args);
  if ( nl->SymbolValue(arg1) == "int")
  {
    return 0;         
  }
  if ( nl->SymbolValue(arg1) == "time")
  {
    return 1;
  }
  if ( nl->SymbolValue(arg1) == "ugridarea")
  {
    return 2;
  }
  return -1; // This point should never be reached
}
/*
3.6.5  Specification of operator ~windowintersectsug~

*/ 
const string WindowIntersectsUGSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>pos -> id x time " "</text--->"
  "<text>windowintersectsug ( _ ) </text--->"
  "<text>Returns the ids and timeinterval resulting intersection"
     " with area .</text--->"
  "<text>query ugrid = windowintersectsug (123.4 345.6) </text--->"
  ") )";

Operator windowintersectsug (
         "windowintersectsug",              // name
          WindowIntersectsUGSpec,           // specification
		  3,
          WindowIntersectsUGMap,           // value mapping
		  WindowIntersectsUGOpSelect,       // selection function
          WindowIntersectsUGOpTypeMap       // type mapping
);
/*
4.1   T y p e   M a p p i n g   Functions
4.1.1  CoordCoordBool

*/

ListExpr  RectRectBool( ListExpr args )
{	
  NList type(args);
  if ( type != NList(UGRIDAREA, UGRIDAREA) ) {
    return NList::typeError("Expecting two rectangles");
  }  

  return NList(BOOL).listExpr();
}
ListExpr  verintTypeMap( ListExpr args )
{	
  NList type(args);
  if ( type != NList(INT,INT) ) {
    return NList::typeError("Expecting two integers");
  }  

  return NList(INT).listExpr();
}
/*
4.1.2  insideTypeMap

*/

ListExpr  insideTypeMap( ListExpr args )
{
  NList type(args);
  const string errMsg = "Expecting two rectangles ";
	              //  "or a point and a rectangle"
				  //	"or two updateunits";

  // first alternative: mobpos x ugridarea -> bool
  if ( type == NList(MOBPOS, UGRIDAREA) ) {
    return NList(BOOL).listExpr();
  }  
  // second alternative: coord x coord -> bool
  if ( type == NList(MOBPOS, UPDATEUNIT) ) {
    return NList(BOOL).listExpr();
  } 
  // third alternative: coord x coord -> bool
  if ( type == NList(MOBPOS, CURRENTUNIT) ) {
    return NList(BOOL).listExpr();
  } 
  // fourth alternative: updateunit x updateunit -> bool
   if ( type == NList(MOBPOS, HISTORYUNIT) ) {
    return NList(BOOL).listExpr();
  }  
  // fifth alternative: ugridarea x updateunit -> bool
  if ( type == NList(UGRIDAREA, UGRIDAREA) ) {
    return NList(BOOL).listExpr();
  } 
  // sixth alternative: ugridarea x updateunit -> bool
  if ( type == NList(UGRIDAREA, CURRENTUNIT) ) {
    return NList(BOOL).listExpr();
  }  
  // seventh alternative: ugridarea x updateunit -> bool
  if ( type == NList(UGRIDAREA, HISTORYUNIT) ) {
    return NList(BOOL).listExpr();
  }     
  // eigth alternative: ugridarea x updateunit -> bool
  if ( type == NList(UPDATEUNIT, UPDATEUNIT) ) {
    return NList(BOOL).listExpr();
  }  
  return NList::typeError(errMsg);
}
/*
4.1.4  inserthuTypeMap

The operator ~inserthu~ inserts a historyunit in UGridCell .

*/
ListExpr inserthuTypeMap(ListExpr args)
{	  
  string errMsg = "Expecting historyunit";
  if (nl->ListLength(args) != 1)
  {
    errMsg = "InserthuTypeMap:one arg is expected";
	ErrorReporter::ReportError(errMsg);
    return nl->SymbolAtom("typeerror");
  }
  else
  {
    //ListExpr ugrid = nl->First(args);
	ListExpr historyunit = nl->First(args);
    if (nl->IsEqual( historyunit, HISTORYUNIT)) 
	{ 
		return nl->SymbolAtom(UGRIDCELL) ;
    }
  }
  errMsg = "InserthuTM: historyunit expected";
  ErrorReporter::ReportError(errMsg);
  return nl->SymbolAtom("typeerror");
}

/*
4.2.1 The ~inside~ selection for two coords, two UpdateUnits or MobPos/Coord

*/
int  insideSelect( ListExpr args )
{
  NList type(args);
  if ( type.first().isSymbol( MOBPOS ) )
  {
    if ( type.second().isSymbol( UGRIDAREA) )
    return 0;
	else
	{
      if ( type.second().isSymbol( UPDATEUNIT) )
      return 1;
	  else
	  {
        if ( type.second().isSymbol( CURRENTUNIT) )
        return 2;
	    else
	    {
          if ( type.second().isSymbol( HISTORYUNIT) )
          return 3;
		}
	  }
	}
  }
  else
  {
    if ( type.first().isSymbol( UGRIDAREA ) )
    {
      if ( type.second().isSymbol( UGRIDAREA) )
      return 4;
	  else
	  {
        if ( type.second().isSymbol( CURRENTUNIT) )
        return 5;
	    else
	    {
          if ( type.second().isSymbol( HISTORYUNIT) )
          return 6;
	    }
	  }
	}
    else 
	{
	  if (type.first().isSymbol(UPDATEUNIT) && 
		  type.second().isSymbol(UPDATEUNIT))
      return 7;
	}
  return -1;  // Should never happen - error   
  } 
return -2;  // Should never happen - error  
}  
/*
4.3   V a l u e   M a p p i n g   Functions
4.3.1  intersectsFun

*/
int  intersectsFun (Word* args, Word& result, int message, 
              Word& local, Supplier s)
{
  UGridArea *r1 = (UGridArea*)( args[0].addr );
  UGridArea *r2 = (UGridArea*)( args[1].addr );

  result = qp->ResultStorage(s);  
  CcBool* b = (CcBool*)( result.addr );
  b->Set(true, r1->intersects(*r2));
                               
  return 0;
}

/*
4.3.2  insideFunPA:The ~inside~ predicate for a point and a area

*/
int  insideFun_PA (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  MobPos* p = static_cast<MobPos*>( args[0].addr );
  UGridArea* a = static_cast<UGridArea*>( args[1].addr );
  result = qp->ResultStorage(s);   
  CcBool* b = static_cast<CcBool*>( result.addr );
  bool res = ( p->GetX() <= a->GetX2() 
            && p->GetX() >= a->GetX1()
            && p->GetY() >= a->GetY1() 
            && p->GetY() <= a->GetY2() );

  b->Set(true, res); 
  return 0;
}

/*
4.3.3  insideFunPU:The ~inside~ predicate for a point and a Updateunit

*/

int  insideFun_PU (Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  MobPos* p = static_cast<MobPos*>( args[0].addr );
  UpdateUnit* u = static_cast<UpdateUnit*>( args[1].addr );

  result = qp->ResultStorage(s);   
  CcBool* b = static_cast<CcBool*>( result.addr );
  
  bool res = ( p->GetX() == u->GetXPos() 
            && p->GetY() == u->GetYPos());

  b->Set(true, res);
  return 0;
}
/*
4.3.4  insideFunPC:The ~inside~ predicate for a point and a currentunit

*/

int  insideFun_PC (Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  MobPos* p = static_cast<MobPos*>( args[0].addr );
  CurrentUnit* c = static_cast<CurrentUnit*>( args[1].addr );

  result = qp->ResultStorage(s);   
  CcBool* b = static_cast<CcBool*>( result.addr );
  
  bool res = ( p->GetX() >= c->GetCXPos() 
            && p->GetY() >= c->GetCYPos()
			&& p->GetX() <= c->GetNXPos() 
            && p->GetY() <= c->GetNYPos());

  b->Set(true, res); 
  return 0;
}
/*
4.3.5  insideFunPH:The ~inside~ predicate for a point and a historyunit

*/
int  insideFun_PH (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  MobPos* p = static_cast<MobPos*>( args[0].addr );
  HistoryUnit* h = static_cast<HistoryUnit*>( args[1].addr );
  result = qp->ResultStorage(s);   
  CcBool* b = static_cast<CcBool*>( result.addr );
  bool res = ( p->GetX() <= h->GetEXPos() 
            && p->GetX() >= h->GetSXPos()
            && p->GetY() >= h->GetSYPos() 
            && p->GetY() <= h->GetEYPos() );

  b->Set(true, res); 
  return 0;
}

/*
4.3.6 insideFunAA:The ~inside~ predicate for two areas

*/

int  insideFun_AA (Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  UGridArea* a1 = static_cast<UGridArea*>( args[0].addr );
  UGridArea* a2 = static_cast<UGridArea*>( args[1].addr );
  result = qp->ResultStorage(s);   
  CcBool* b = static_cast<CcBool*>( result.addr );
  bool res = false;
  if ((a1->GetX2() <= a2->GetX2()) && 
	  (a1->GetY2() <= a2->GetY2()) &&
	  (a1->GetX1() >= a2->GetX1()) &&
	  (a1->GetY1() >= a2->GetY1()))
  {
    res = true;
  }

  b->Set(true, res); 
  return 6;
}
/*
4.3.7 insideFunAC :The ~inside~ predicate for a area and a currentunit

*/

int  insideFun_AC (Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  UGridArea* a = static_cast<UGridArea*>( args[0].addr );
  CurrentUnit* c = static_cast<CurrentUnit*>( args[1].addr );
  result = qp->ResultStorage(s);   
  CcBool* b = static_cast<CcBool*>( result.addr );
  bool res = false;
  if ((a->GetX1() >= c->GetCXPos()) &&
      (a->GetX2() <= c->GetNXPos()) && 
      (a->GetY1() >= c->GetCYPos()) &&
      (a->GetY2() <= c->GetNYPos()))
  { 
    res = true;
  }
  b->Set(true, res); 
  return 7;
}
/*
4.3.8 insideFunAH :The ~inside~ predicate for a area and a historyunit

*/

int  insideFun_AH (Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  UGridArea* a = static_cast<UGridArea*>( args[0].addr );
  HistoryUnit* h = static_cast<HistoryUnit*>( args[1].addr );
  result = qp->ResultStorage(s);   
  CcBool* b = static_cast<CcBool*>( result.addr );
  bool res = false;
  if ((a->GetX1() >= h->GetSXPos()) &&
	  (a->GetX2() <= h->GetEXPos()) && 
      (a->GetY1() >= h->GetSYPos()) &&
	  (a->GetY2() <= h->GetEYPos()))
  { 
    res = true;
  }
  b->Set(true, res); 
  return 8;
}
/*
4.3.9 insideFunUU :The ~inside~ predicate for two updateunit

*/

int  insideFun_UU (Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  UpdateUnit* u1 = static_cast<UpdateUnit*>( args[0].addr );
  UpdateUnit* u2 = static_cast<UpdateUnit*>( args[1].addr );

  result = qp->ResultStorage(s);   
  CcBool* b = static_cast<CcBool*>( result.addr );
  
  bool res = false;
  if (u1->GetUId() ==  u2->GetUId())
  {
    if (((u1->GetXPos() <= u2->GetXPos()) ||
		 (u1->GetXPos() >= u2->GetXPos())) &&
	    ((u1->GetYPos() <= u2->GetYPos()) || 
         (u1->GetYPos() >= u2->GetYPos())) &&
	     (u1->GetTime() <= u2->GetTime()))
	{ 
      res = true;
	}
  }
  b->Set(true, res); 
  return 9;
}
/*
4.3.10 verintFun:The ~verint~ compares two points

*/

int  verintFun (Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  CcInt* p1 = static_cast<CcInt*>( args[0].addr );
  CcInt* p2 = static_cast<CcInt*>( args[1].addr );
  result = qp->ResultStorage(s);   
  CcInt* res = static_cast<CcInt*> (result.addr);
  int x,y;
  x = p1->GetIntval();
  y = p2->GetIntval();
  
  if (x < y) 
  {
    res->Set(true, x);
  }
  else
  {
    res->Set(true, y);
  }
  return 1;
}

/*
4.3.6 inserthuFun :The ~inserthu~ insert historyunit in UGridCell

*/

int  inserthuFun (Word* args, Word& result, int message, 
                        Word& local, Supplier s)
{  
  HistoryUnit* hunit = static_cast<HistoryUnit*>( args[0].addr );
  result = qp->ResultStorage(s);   //result number of UGridcell in which
                                   //the historyunit is splitted
  hunit->hid = 0;
  UGridCell* ugc = (UGridCell*) result.addr;
   int cellindex = 0;
  dimsize dim = (dimsize) 2;
  UGridArea ucarea;
  ucarea.x1 = 123.4;
  ucarea.y1 = 234.5;
  ucarea.x2 = 345.6;
  ucarea.y2 = 567.8;
  ugc = new UGridCell( cellindex, dim, ucarea);
  return 0;
}
/*
4.4  O p e r a t o r   D e s c r i p t i o n s
4.4.1  Operator Descriptions  intersectsInfo

*/

struct intersectsInfo : OperatorInfo {
  intersectsInfo()
  {
    name      = INTERSECTS;
    signature = UGRIDAREA + " x " + UGRIDAREA + " -> " + BOOL;
    syntax    = "_" + INTERSECTS + "_";
    meaning   = "Intersection predicate for two areas.";
  }
}; 

/*
4.4.2  Operator Descriptions  insideInfo

*/

struct insideInfo : OperatorInfo {
  insideInfo()
  {
    name      = INSIDE; 

    signature = MOBPOS + " x " + UGRIDAREA + " -> " + BOOL;
	appendSignature( MOBPOS + " x " + UPDATEUNIT + " -> " + BOOL );
	appendSignature( MOBPOS + " x " + CURRENTUNIT + " -> " + BOOL );
	appendSignature( MOBPOS + " x " + HISTORYUNIT + " -> " + BOOL );
	appendSignature( UGRIDAREA + " x " + UGRIDAREA + " -> " + BOOL );
	appendSignature( UGRIDAREA + " x " + CURRENTUNIT + " -> " + BOOL );
    appendSignature( UGRIDAREA + " x " + HISTORYUNIT + " -> " + BOOL );
	appendSignature( UPDATEUNIT + " x " + UPDATEUNIT + " -> " + BOOL );
    syntax    = "_" + INSIDE + "_";
    meaning   = "Inside predicate.";
  }
};
struct verintInfo : OperatorInfo {
  verintInfo()
  {
    name      = VERINT; 

    signature = INT + " x " + INT + " -> " + INT;
    syntax    = "_" + INT + "_";
    meaning   = "Integer predicate.";
  }
};

/*
4.4.4  Operator Descriptions  inserthuInfo

*/

struct inserthuInfo : OperatorInfo {
  inserthuInfo()
  {
    name      = INSERTHU; 
    signature = HISTORYUNIT +  " -> " + UGRIDCELL;
    syntax    = INSERTHU + " ( _ )";
    meaning   = "Insert Historyunit in UgridCell.";
  }
};  
/*
4  Implementation of the Algebra Class

*/

class UGridAlgebra : public Algebra
{
 public:
  UGridAlgebra() : Algebra()
  {
	AddTypeConstructor( &ugridTC );
	AddTypeConstructor( &ugridareaTC );
	AddTypeConstructor( &updateunitTC );
	AddTypeConstructor( &currentunitTC );
	AddTypeConstructor( &historyunitTC );
    AddTypeConstructor( &ugridcellTC );
	AddTypeConstructor( &mobposTC );

    
    ugridTC.AssociateKind( "UGRID" );
    ugridareaTC.AssociateKind("UGRIDAREA");

    AddOperator( &createugrid );
	AddOperator( &insertunit);
	AddOperator( &insertsingle);
	AddOperator( &insertstream);
	AddOperator(&windowintersectug);
	AddOperator(&windowintersectsug);

	AddOperator( intersectsInfo(), intersectsFun, RectRectBool );  
   
    ValueMapping insideFuns[] = { 
		 insideFun_PA, insideFun_PU, insideFun_PC,
		 insideFun_PH, insideFun_AA, insideFun_AC,
         insideFun_AH, insideFun_UU, 0 };
    AddOperator( insideInfo(), insideFuns, insideSelect, insideTypeMap );
    AddOperator( verintInfo(), verintFun,  verintTypeMap );
  }
  ~UGridAlgebra() {};
};
} // end of namespace ugridAlgebra

/*
6 Initialization of the Algebra

*/
extern "C"
Algebra*
InitializeUGridAlgebra( NestedList* nlRef, 
                               QueryProcessor* qpRef )
{
  // The C++ scope-operator :: must be used to qualify the full name 
  return new ugridAlgebra::UGridAlgebra; 
}

