/*


//[_] [\_]
//[TOC] [\tableofcontents]
//[Title] [ \title{RobustGeometry} \author{Katja Koch} \maketitle]
//[times] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]



[1]  Implementation of the RobustGeometryAlgebra

[TOC]

1 Overview

This algebra implements the Snap Rounding technique described in Hobby,
Guibas and Marimont to compute a fully rounded arrangement from a set of linesegments.
Snap Rounding assumes that all vertices lie on a uniform grid.
Hence all the input vertices must be rounded to a given precision.
This implementation uses an iteration over the line segments and
appears to be fully robust using an adjustable precision model.
To change the precision adapt the ScaleFactor.
This algebra provides just the type constructors ~line~
for the intersection operation.
There are tree steps to provide a robust operation in SECONDO.
First Step is a plane-sweep-algorithm to find the intersection points, then
snap rounding and finally a operation specific check.

1 Preliminaries

1.1 Includes and global declarations

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "GenericTC.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecondoConfig.h"
#include "AvlTree.h"
#include "AlmostEqual.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "Symbols.h"
#include "NList.h"
#include "LogMsg.h"
#include "ConstructorTemplates.h"
#include "TypeMapUtils.h"
#include "SpatialAlgebra.h"
#include "RobustGeometryAlgebra.h"
#include "AVLSegment.h"
#include "HalfSegment.h"
#include <vector>
#include <set>
#include <queue>
#include <stdexcept>
#include <iostream>
#include <string.h>
#include <string>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <queue>
#include <iterator>
#include <sstream>
#include <limits>
#include <errno.h>
#include <cerrno>

#ifndef M_PI
const double M_PI = acos( -1.0 );
#endif

extern NestedList* nl;
extern QueryProcessor* qp;

/*
 Algebra Implementation
 Type Mapping Functions check whether the correct argument types are supplied for an
 operator; if so, returns a list expression for the result type, otherwise the
 symbol ~typeerror~. This implementation uses the implementation
of the classes ~line~ and ~region~ from SpatialAlgebra.
So see SpatialAlgebra for Type Mapping Functions

*/

/*
 An algebra module must provide for each type functions which take a nested
 list and create an instance of the class representing the type and vice versa.
 These functions are called In- and Out-functions. [->] see SpatialAlgebra

*/


enum SpatialTypeRG {
	stpoint, stpoints, stline, stregion, stbox, sterror
};


SpatialTypeRG SpatialTypeOfSymbolRG(ListExpr symbol) {
	if (nl->AtomType(symbol) == SymbolType) {
		string s = nl->SymbolValue(symbol);
		if (s == Point::BasicType())
			return (stpoint);
		if (s == Points::BasicType())
			return (stpoints);
		if (s == Line::BasicType())
			return (stline);
		if (s == Region::BasicType())
			return (stregion);
		if (s == Rectangle<2>::BasicType())
			return (stbox);
	}
	return (sterror);
}

/*
10.1.2 Type Mapping for intersection

Signature is line x line [->] line

*/

static ListExpr intersectionTM(ListExpr args) {
	ListExpr arg1, arg2;
	if (nl->ListLength(args) == 2) {
		arg1 = nl->First(args);
		arg2 = nl->Second(args);
		if (SpatialTypeOfSymbolRG(arg1) == stline
				&& SpatialTypeOfSymbolRG(arg2) == stline)
			return (nl->SymbolAtom(Line::BasicType()));
	}
	return (nl->SymbolAtom(Symbol::TYPEERROR()));
}

int RobustGeometrySetOpSelect(ListExpr args) {
	string a1 = nl->SymbolValue(nl->First(args));
	string a2 = nl->SymbolValue(nl->Second(args));

	if (a1 == Line::BasicType()) {
		if (a2 == Line::BasicType())
			return 1;
		return -1;
	}

	return -1;
}

/*
10.5.2 Definition of specification strings

*/

const string intersectionSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
	"( <text>{line} x"
	"   {line} -> T, "
	" where T = points if any point or point type is one of the "
	" arguments or the argument having the smaller dimension </text--->"
	"<text>intersectionBO(arg1, arg2)</text--->"
	"<text>intersectionBO of two spatial objects</text--->"
	"<text>query intersectionBO(tiergarten, thecenter) </text--->"
	") )";

/*
3 Implementation of Class ~BOLine~

*/

/*
documentation see RobustGeometryAlgebra.h

*/
robustGeometry::BOLine::BOLine(const BOLine& line){
	x1 = line.getX1();
	x2 = line.getX2();
	y1 = line.getY1();
	y2 = line.getY2();
	owner = line.getOwner();
	x1_r = line.getX1_round();
	y1_r = line.getY1_round();
	x2_r = line.getX2_round();
	y2_r = line.getY2_round();
}

robustGeometry::BOLine::BOLine( const double x1,
	const double y1,
	const double x2,
	const double y2,
	const robustGeometry::BOOwnerType owner )
	{
		setX1( x1 );
		setY1( y1 );
		setX2( x2 );
		setY2( y2 );
		setOwner( owner );
	};
void robustGeometry::BOLine::setX1( const double x ) const
	{
		this->x1 = x;
	};
void robustGeometry::BOLine::setX2( const double x ) const
	{
		this->x2 = x;
	};
void robustGeometry::BOLine::setY1( const double y ) const
	{
		this->y1 = y;
	};
void robustGeometry::BOLine::setY2( const double y ) const
	{
		this->y2 = y;
	};
void robustGeometry::BOLine::setX1_round( const double x )
	{
		this->x1_r = x;
	};
void robustGeometry::BOLine::setX2_round( const double x )
	{
		this->x2_r = x;
	};
void robustGeometry::BOLine::setY1_round( const double y )
	{
		this->y1_r = y;
	};
void robustGeometry::BOLine::setY2_round( const double y )
	{
		this->y2_r = y;
	};
void robustGeometry::BOLine::setOwner( const BOOwnerType owner )
	{
		this->owner = owner;
	};


/*
3 Implementation of Class ~BOEvent~

*/
robustGeometry::BOEvent::BOEvent(const double x, const double y,
		const robustGeometry::BOPointType pType,
		const robustGeometry::BOOwnerType owner)
{
	setX( x );
	setY( y );
	origX = x;
	origY = y;
	setPointType( pType );
};

robustGeometry::BOEvent::BOEvent(const double x, const double y,
	   const robustGeometry::BOPointType pType,
	   const robustGeometry::BOOwnerType owner,
	   const robustGeometry::BOLine& line)
{
	setX( x );
	setY( y );
	origX = x;
	origY = y;
	setPointType( pType );
	setOwner( owner);
	setLine( line );

};

robustGeometry::BOEvent::BOEvent(const double x, const double y,
		   const robustGeometry::BOPointType pType,
		   const robustGeometry::BOOwnerType owner,
		   const robustGeometry::BOLine& line1,
		   const robustGeometry::BOLine& line2)
{
	setX( x );
	setY( y );
	origX = x;
	origY = y;
	setPointType( pType );
	setOwner( owner);
	setLine1(line1);
	setLine2(line2);
	if ( line1.getY1() > line2.getY1()){
		lineAbove = line1;
		lineBelow = line2;
	} else {
		lineAbove = line2;
		lineBelow = line1;
	};
};

void robustGeometry::BOEvent::setX( const double x )
{
	this->x = x;
};
void robustGeometry::BOEvent::setY( const double y )
{
	this->y = y;
};
void robustGeometry::BOEvent::setPointType( const BOPointType pointType )
{
	this->pointType = pointType;
};
void robustGeometry::BOEvent::setOwner( const BOOwnerType owner )
{
	this->owner = owner;
};
void robustGeometry::BOEvent::setLine( const BOLine & line )
{
	this->line = line;
};

void robustGeometry::BOEvent::setLine1( const BOLine & line1 )
{
	this->line1 = line1;
};
void robustGeometry::BOEvent::setLine2( const BOLine & line2 )
{
	this->line2 = line2;
};

void robustGeometry::BOEvent::Print(ostream& out) const
{
	out << "X : " << getX( ) <<
		   ",Y : " << getY( ) <<
		   ", owner : " << getOwner( );

};

void robustGeometry::Point::setX( const double x )
{
	this->x = x;
};
void robustGeometry::Point::setY( const double y )
{
	this->y = y;
};

/*
3 Implementation of Class ~HOBatch~

*/
robustGeometry::HOBatch::HOBatch(const double x, const double y){
	setX( x );
	setY( y );
}

void robustGeometry::HOBatch::setX( const double x )
{
	this->x = x;
};
void robustGeometry::HOBatch::setY( const double y )
{
	this->y = y;
};

/*
3 Implementation of Class ~ToleranceSquare~

*/
void robustGeometry::ToleranceSquare::setX11( const double x11 )
{
	this->x11 = x11;
};
void robustGeometry::ToleranceSquare::setY11( const double y11 )
{
	this->y11 = y11;
};
void robustGeometry::ToleranceSquare::setX12( const double x12 )
{
	this->x12 = x12;
};
void robustGeometry::ToleranceSquare::setY12( const double y12 )
{
	this->y12 = y12;
};
void robustGeometry::ToleranceSquare::setX21( const double x21 )
{
	this->x21 = x21;
};
void robustGeometry::ToleranceSquare::setY21( const double y21 )
{
	this->y21 = y21;
};
void robustGeometry::ToleranceSquare::setX22( const double x22 )
{
	this->x22 = x22;
};
void robustGeometry::ToleranceSquare::setY22( const double y22 )
{
	this->y22 = y22;
};
void robustGeometry::ToleranceSquare::setSnapX( const double snap_x )
{
	this->snap_x = snap_x;
};
void robustGeometry::ToleranceSquare::setSnapY( const double snap_y )
{
	this->snap_y = snap_y;
};
void robustGeometry::ToleranceSquare::setBOEvent( const BOEvent & boEv )
{
	this->boEv = boEv;
};

/*
1 Implementation of Class ~MakeBO~

Class for plane-sweep-algorithm like [Bentley/Ottmann 1979]

*/
class MakeBO
{

public:


/*
3.1 Constructors and Destructors

*/
  MakeBO()
 { };

 ~MakeBO() {};

/*
3.2 Operation ~intersection~

*/
	void IntersectionBO(const Line& line1,
			const Line& line2, Line& result);
/*
10.1 Functions for computing intersection points

*/

/*
compute the intersection point from two segments
Returns zero[->]OK, one[->]lines parallel, two[->]no intersection point

*/
	int intersect(const double l1_x1,const double l1_y1,
				const double l1_x2,const double l1_y2,
				const double l2_x1,const double l2_y1,
				const double l2_x2,const double l2_y2,
				double& x,double& y);
/*
check intesection, if exists and intersectionpoint i is not in
event queue boEvents insert i into boEvents

*/
	  void checkIS(robustGeometry::BOEvent& currEv,
			  const robustGeometry::BOLine& currSeg);

/*
check intesection for line object, if exists and intersectionpoint i is not in
event queue boEvents insert i into boEvents

*/
	  void checkIS(const robustGeometry::BOLine& line1,
			  const robustGeometry::BOLine& line2);
/*
Swap their positions so that above and below are interchanged;

*/
	  void findAndSwap(const robustGeometry::BOLine& aboveSeg,
		      const robustGeometry::BOLine& belowSeg);
/*
Gets the line above according to the x,y value

*/
	  const robustGeometry::BOLine*
	  getAbove(const double x, const double y);
/*
Gets the line below according to the x,y value

*/
	  const robustGeometry::BOLine*
	  getBelow(const double aktXPos,double aktYPos);
/*
Gets the line above according to the current event

*/
	  const robustGeometry::BOLine*
	  getAbove(robustGeometry::BOEvent& event);
/*
Gets the line below according to the current event

*/
	  const robustGeometry::BOLine*
	  getBelow(robustGeometry::BOEvent& event);
/*
Add line to the sweep-line

*/
	  void addLineToSweepLine(const robustGeometry::BOLine& line );
/*
Add event to event queue boEvents

*/
	  void addBOEvent(const robustGeometry::BOEvent& event );
/*
event e is a left endpoint

*/
	  void doBOCaseA( robustGeometry::BOEvent & event );
/*
event e is a right endpoint

*/
	  void doBOCaseB( robustGeometry::BOEvent & event );
/*
event e is a intersectionpoint

*/
	  void doBOCaseC( robustGeometry::BOEvent & event );
/*
add event to event queue boEvents

*/
	  void addInitialEvent(const robustGeometry::BOEvent& event );
/*
find and delete event in event queue boEvents

*/
	  void findEventAndDelete( robustGeometry::BOEvent & event );
/*
call the case processing

*Precondition:* ~there are elements in boEvents~

*/
	  void doBOCases( );
	  void printInitialEvents();
	  void printBOEvents();
	  set<robustGeometry::BOEvent, robustGeometry::CompBOEventXY>
	  & get_BOEvents( ){ return boEvents; };


private:
/*
sweepLine

*/
  set<robustGeometry::BOLine, robustGeometry::CompBOLine > sL;
/*
input event queue

*/
  set<robustGeometry::BOEvent, robustGeometry::CompBOEventXY> initialEvents;
/*
event queue

*/
  set<robustGeometry::BOEvent, robustGeometry::CompBOEventXY> boEvents;
};

void MakeBO::doBOCaseA( robustGeometry::BOEvent & event ){
	addBOEvent(event);

	addLineToSweepLine(event.getLine());
	const robustGeometry::BOLine* segAbove = 0;

	segAbove = getAbove( event);
	if ( segAbove!= 0 ){
		checkIS(event,*segAbove);
	};
	const robustGeometry::BOLine* segBelow = 0;
	segBelow=getBelow(event);
	if ( segBelow != 0 ){
		checkIS(event,*segBelow);
	};
}

void MakeBO::doBOCaseB( robustGeometry::BOEvent & event ){
	addBOEvent(event);
	const robustGeometry::BOLine* segAbove = 0;
	segAbove=getAbove(event);
	const robustGeometry::BOLine* segBelow = 0;
	segBelow = getBelow(event);

	findEventAndDelete( event );

	if ( ( segAbove != 0 ) && ( segBelow != 0 )){
	  checkIS(*segAbove,*segBelow);
	};
}

void MakeBO::doBOCaseC( robustGeometry::BOEvent & event ){
	addBOEvent(event);
	const robustGeometry::BOLine* seg1 = &event.getLineAbove( );
	const robustGeometry::BOLine* seg2 = &event.getLineBelow( );

	findAndSwap(*seg1,*seg2);

	const robustGeometry::BOLine *segAbove = 0;
	segAbove=getAbove(seg2->getX2(), seg2->getY2());
	const robustGeometry::BOLine *segBelow = 0;
	segBelow=getBelow(seg1->getX2(), seg1->getY2());

	if ( ( segAbove != 0 ) && ( seg2 != 0 )){
	  checkIS(*segAbove,*seg2);
	};
	if ( ( segBelow != 0 ) && ( seg1 != 0 )){
	  checkIS(*seg1, *segBelow);
	};
}


void MakeBO::doBOCases(){
	set <robustGeometry::BOEvent, robustGeometry::CompBOEventXY>
	::iterator event_it;

	boEvents.clear();
	sL.clear();

	for ( event_it = initialEvents.begin();
			event_it != initialEvents.end(); ++event_it){
		robustGeometry::BOEvent event = *event_it;
		if ( event.getPointType()==robustGeometry::start ){
			doBOCaseA( event );
		} else if  ( event.getPointType()==robustGeometry::end ){
			doBOCaseB( event );
		} else if ( event.getPointType()==robustGeometry::intersect ){
			doBOCaseC( event );
		};

	}

}

void MakeBO::printInitialEvents( ){

	set<robustGeometry::BOEvent>::iterator it;
	for ( it = initialEvents.begin(); it != initialEvents.end(); ++it){
	  cout << it->getX() << " " << it->getY() << " Pointtype ";
	  if ( it->getPointType() == robustGeometry::start )
		  cout << "start";
	  else if ( it->getPointType() == robustGeometry::end )
	  	  cout << "end";
	  else
		  cout << "intersect";
	  cout << endl;
	}
}
void MakeBO::printBOEvents( ){

	set<robustGeometry::BOEvent>::iterator it;
	for ( it = boEvents.begin(); it != boEvents.end(); ++it){
	  cout << it->getX() << " " << it->getY();
	  if ( it->getPointType() == robustGeometry::start )
	 		  cout << "start";
	 	  else if ( it->getPointType() == robustGeometry::end )
	 	  	  cout << "end";
	 	  else {
	 		 robustGeometry::BOEvent tmpEv = *it;
			  cout << "intersect" << " line1 "
			  << tmpEv.getLine1().getX1()
			  << " line2 " << tmpEv.getLine1().getX2();
	 	  }
	  cout << endl;
	}
}

const robustGeometry::BOLine * MakeBO::getAbove(
		const double x, const double y)
{

	set< robustGeometry::BOLine>::iterator seg_it;
	for ( seg_it = sL.begin(); seg_it != sL.end(); ++seg_it)
	{
		robustGeometry::BOLine tmpSeg = *seg_it;
		if ( ( x > seg_it->getX1() ) &&
			 ( x < seg_it->getX2() ) &&
			 ( y < seg_it->getY1() ) )
		{
			return &*seg_it;
		}
		if ( tmpSeg.getX1() > x ) break;
	}
	return 0;
}


const robustGeometry::BOLine * MakeBO::getAbove(
		robustGeometry::BOEvent& event)
{
	return getAbove( event.getX(), event.getY());
}
const robustGeometry::BOLine * MakeBO::getBelow
(const double x, const double y)

{
	set< robustGeometry::BOLine,
	robustGeometry::CompBOLine >::iterator seg_it;

	for ( seg_it = sL.begin(); seg_it != sL.end(); ++seg_it)
	{
		robustGeometry::BOLine tmpSeg = *seg_it;
		if ( ( x > tmpSeg.getX1() ) &&
			 ( x < tmpSeg.getX2() ) &&
			 ( y > tmpSeg.getY1() ) )
		{
			return &*seg_it;
		}
		if ( tmpSeg.getX1() > x ) break;
	}
	return 0;
}

const robustGeometry::BOLine * MakeBO::getBelow(robustGeometry::BOEvent& event){
	return getBelow(event.getX(), event.getY());
}

void MakeBO::findEventAndDelete( robustGeometry::BOEvent & event ){
	if ( sL.size() == 0) return;

	robustGeometry::BOLine eventSeg = event.getLine();

	sL.erase(robustGeometry::BOLine(eventSeg.getX1(), eventSeg.getY1(),
			eventSeg.getX2(), eventSeg.getY2(),
			eventSeg.getOwner() ));
}

void MakeBO::findAndSwap(const robustGeometry::BOLine& aboveSeg,
		const robustGeometry::BOLine& belowSeg)
{
	int pos1=-1;
	int pos2=-1;

	set< robustGeometry::BOLine,
	robustGeometry::CompBOLine >::iterator sL_it;
	int i = 0;
	double tempx1=0.0;
	double tempy1=0.0;
	for ( sL_it = sL.begin(); sL_it != sL.end(); ++sL_it)
	{

		robustGeometry::BOLine tmpSeg = *sL_it;
	    tempx1=tmpSeg.getX1();
		tempy1=tmpSeg.getY1();

		if((tempx1 == aboveSeg.getX1()) && (tempy1 == aboveSeg.getY1())
		&&(tempx1 == aboveSeg.getX2()) && (tempy1 == aboveSeg.getY2()))
		{
			pos1=i;
		}
		if((tempx1 == belowSeg.getX1()) && (tempy1 == belowSeg.getY1())
		&&(tempx1 == belowSeg.getX2()) && (tempy1 == belowSeg.getY2()))
		{
			pos2=i;
		}
		i++;
		if(pos1>=0 && pos2>=0)
		{
			robustGeometry::BOLine tmp = aboveSeg;
		}
	}

}

int MakeBO::intersect(const double l1_x1,const double l1_y1,
		              const double l1_x2,const double l1_y2,
		              const double l2_x1,const double l2_y1,
		              const double l2_x2,const double l2_y2,
		              double & x,double & y){
//return =0 OK, 1 lines parallel, 2 no intersection point
//x,y intersection point
	double a1=0.0;
	double a2=0.0;
	double b1=0.0;
	double b2=0.0;
	double c1=0.0;
	double c2=0.0;
// Coefficients of line
	double m=0.0;

	a1= l1_y2 - l1_y1;
	b1= l1_x1 - l1_x2;
	c1= l1_x2 * l1_y1 - l1_x1 * l1_y2;
//a1*x + b1*y + c1 = 0 is segment 1

	a2= l2_y2 - l2_y1;
	b2= l2_x1 - l2_x2;
	c2= l2_x2 * l2_y1 - l2_x1 * l2_y2;
//a2*x + b2*y + c2 = 0 is segment 2

	m= a1*b2-a2*b1;
	if (m == 0) return 1;

	x=(b1*c2-b2*c1)/m;
// intersection range

	if(x < l1_x1 || x < l2_x1 || x > l1_x2 || x > l2_x2) return 2;

	y=(a2*c1-a1*c2)/m;

  return 0;
}

void MakeBO::checkIS(const robustGeometry::BOLine& line1,
		const robustGeometry::BOLine& line2)
{
	double sx1=0.0;
	double sy1=0.0;
	double ex1=0.0;
	double ey1=0.0;
	double sx2=0.0;
	double sy2=0.0;
	double ex2=0.0;
	double ey2=0.0;
	double is_x=0.0;
	double is_y=0.0;
	int isIntersected = 1;

	robustGeometry::BOOwnerType currOwner;
	robustGeometry::BOOwnerType nextOwner;

	sx2 = line2.getX1();
	sy2 = line2.getY1();
	ex2 = line2.getX2();
	ey2 = line2.getY2();
	nextOwner = line2.getOwner();

	sx1 = line1.getX1();;
	sy1 = line1.getY1();
	ex1 = line1.getX2();
	ey1 = line1.getY2();
	currOwner = line1.getOwner();

//check owner first
	if (currOwner != nextOwner)
	{
		isIntersected = intersect
				(sx1,sy1,ex1,ey1,sx2,sy2,ex2,ey2,is_x,is_y);
		if (isIntersected==0)
		{
			robustGeometry::BOEvent intersP =
			robustGeometry::BOEvent( is_x, is_y,
			robustGeometry::intersect,
			robustGeometry::both, line1, line2);
			addLineToSweepLine(intersP.getLine());
			addBOEvent(intersP);
		}
	}
}
void MakeBO::checkIS(robustGeometry::BOEvent& currEv,
		const robustGeometry::BOLine& currSeg){
	robustGeometry::BOLine* evSeg = &currEv.getLine();
	 checkIS( *evSeg, currSeg);
}

void MakeBO::addLineToSweepLine( const robustGeometry::BOLine& line )
{
	sL.insert(line);
}

void MakeBO::addBOEvent( const robustGeometry::BOEvent& event )
{
	boEvents.insert( event );
}
void MakeBO::addInitialEvent( const robustGeometry::BOEvent& event )
{
	initialEvents.insert( event );
}


/*
1 Implementation of Class ~MakeHobby~

This class implements the snap rounding technique described in
papers by Hobby, Guibas and Marimont.

*/

class MakeHobby {
public:

/*
3.1 Constructors and Destructors

*/
	MakeHobby() {
	};
	~MakeHobby() {
	};
/*
10.1 Functions for computing a rounded arrangement of line segments

*/

/*
processing the step for the hobby algorithm

*/
	void doHO();
/*
compute ToleranceSquare for all BOEvents

*/
	void computeToleranceSq();
/*
add BOEvent to batch

*/
	void addBatch(robustGeometry::BOEvent& boEv);
/*
locate value in the mainActive list

*/
	void locateValue(robustGeometry::ToleranceSquare & ts);
/*
round to the value corresponding to the ScaleFactor

*/
	double getRoundFactor(const double & x );
/*
round the x,-y coordinates of the BOEvent to the center of the ToleranceSquare

*/
	void snapSegment(robustGeometry::BOEvent & boEv,
			robustGeometry::ToleranceSquare & tolS);
/*
compute all possible intersections of the given BOEvent
and the given ToleranceSquare

*/
	bool computeAllIntersections(robustGeometry::BOEvent & boEv,
			robustGeometry::ToleranceSquare & tolS);
/*
tests whether the BOEvent from the argument intersects a ToleranceSquare

*/
	void intersectsToleranceSquare(robustGeometry::BOEvent & boEv);
/*
update the main active list corresponding to the given range

*/
	void updateMainActive(double von, double bis);
/*
 set values to BOEvent

*/
	void set_BOEvents( set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY> & events)
	{ events_org = events;};
/*
split segments except at intersection points

*/
	void splitSegment(robustGeometry::BOEvent & boEv);
/*
split segments at intersection points

*/
	void splitIntersectedSegments();
/*
3.2 Functions for printing out

*/
	void printHOInitialEvents();
	void printHOEvents();
	void printHOResult_events();
	void printHOResult_lines();
	void printSegment();
	void addSegment( robustGeometry::BOLine& line );
	void sortSegments();
	void sortRoundedSegments();

	vector<robustGeometry::BOLine>& get_result_segments( )
			{ return segments; };

private:
	set<robustGeometry::ToleranceSquare,
	robustGeometry::CompToleranceSquareY> tolSquare;

	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY> mainActive;

	set<robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY> events_org;

	set<robustGeometry::HOBatch,
	robustGeometry::CompBatch> batch;

	set<robustGeometry::BOLine,
	robustGeometry::CompBOLineXYOwn> result_line;

	vector<robustGeometry::BOEvent> result_e;
	vector<robustGeometry::BOLine> segments;
/*
compute the intersection of a given segment

*/
	int intersect(double sx1, double sy1, double ex2,
			double ey2, double sx3,
			double sy3, double ex4, double ey4);

};

/*
speed up the search for a segment

*/
struct FindSegment{
private :
	robustGeometry::BOLine line_;
public:
	FindSegment( const robustGeometry::BOLine& line) : line_(line){}
    bool operator()(const robustGeometry::BOLine & line) const{
        return line == line_;
    }

};


void MakeHobby::addSegment( robustGeometry::BOLine& line )
{
	segments.push_back( line );
}

void MakeHobby::printSegment( ){
vector<robustGeometry::BOLine>::iterator sg_it;
 int i = 0;
 for (sg_it = segments.begin(); sg_it != segments.end(); ++sg_it ){
	cout << ++i
	 << " X1 : " << sg_it->getX1() << " Y1 : " << sg_it->getY1()
	 << " X2 : " << sg_it->getX2() << " Y2 " << sg_it->getY2()
	 << " X1_r : " << sg_it->getX1_round() << " Y1_r : "
	 << sg_it->getY1_round()
	 << " X2 _r : " << sg_it->getX2_round() << " Y2_r "
	 << sg_it->getY2_round()
	 << " Owner " << sg_it->getOwner() << endl;

		 }
}

bool sortEvents( const robustGeometry::BOEvent &i,
		         const robustGeometry::BOEvent &j){

	if( i.getX()==j.getX()) return (i.getY()<j.getY());
  	return (i.getX()<j.getX());

}

bool sortRoundedLines( const robustGeometry::BOLine &i,
		          const robustGeometry::BOLine &j){

	if ( i.getX1_round() < j.getX1_round() )
		return true;
	else if ( (i.getX1_round() == j.getX1_round() ) &&
			  (i.getY1_round() <  j.getY1_round() ) )
		return true;
	else if ( (i.getX1_round() == j.getX1_round() ) &&
			  (i.getY1_round() == j.getY1_round() ) &&
			  (i.getX2_round() < j.getX2_round() ) )
		return true;
	else if ( (i.getX1_round() == j.getX1_round() ) &&
			  (i.getY1_round() == j.getY1_round() ) &&
			  (i.getX2_round() == j.getX2_round() ) &&
			  (i.getY2_round() <  j.getY2_round() ) )
		return true;
	else
		return false;
}
bool sortSegment( const robustGeometry::BOLine &i,
		          const robustGeometry::BOLine &j){

	return i < j ;
}
double MakeHobby::getRoundFactor(const double & x ){
	return ceil(floor( x / robustGeometry::ScaleFactor)
			* robustGeometry::ScaleFactor);
}

void  MakeHobby::sortSegments( ){
	sort(segments.begin() , segments.end(), sortSegment);
}
void  MakeHobby::sortRoundedSegments( ){
	sort(segments.begin() , segments.end(), sortRoundedLines);
}
void MakeHobby::doHO()
{

//build batch, round events
	robustGeometry::BOEvent be;
	set< robustGeometry::BOEvent,
	robustGeometry::CompBOEventXY >::iterator

	row_it = events_org.begin();

	for (; row_it != events_org.end(); ++row_it)
	{
		be = *row_it;
		addBatch(be);
	}

//build tolerance square for events
	computeToleranceSq();

	set< robustGeometry::HOBatch,
	robustGeometry::CompBatch >::iterator

	batch_it = batch.begin();

	for (; batch_it != batch.end(); ++batch_it)
	{

		robustGeometry::HOBatch ba_tmp;
		ba_tmp = *batch_it;
		double x = ba_tmp.getX();
		updateMainActive(x-robustGeometry::ScaleFactor-
				robustGeometry::Epsilon,
				x+robustGeometry::ScaleFactor);

//locate y values in „main active list“ and snap

		robustGeometry::BOEvent be;
		robustGeometry::ToleranceSquare ts;
		set<robustGeometry::ToleranceSquare,
		robustGeometry::CompToleranceSquareY>::iterator
		ts_it = tolSquare.begin();

		for (; ts_it != tolSquare.end(); ts_it++ ) {
			ts = *ts_it;
			locateValue(ts);
		}


//update „main active list“ valid for X=i-Epsilon
		updateMainActive(x-robustGeometry::Epsilon,
		x+robustGeometry::ScaleFactor);

		set<robustGeometry::BOEvent,
		robustGeometry::CompBOEventXY>::iterator
		bo_it = mainActive.begin();

		robustGeometry::BOEvent boEv_tmp;
		for (; bo_it != bo_it; bo_it++) {
				boEv_tmp = *bo_it;
				intersectsToleranceSquare(boEv_tmp);
		}

//update „main active list“ valid for x=i+1/2-Epsilon
	}

   splitIntersectedSegments();

   sortSegments();

}

void MakeHobby::printHOInitialEvents( ){

	set<robustGeometry::BOEvent>::iterator it;
	for ( it = events_org.begin(); it != events_org.end(); ++it){
	  cout << it->getX() << " " << it->getY() << " Pointtype ";
	  if ( it->getPointType() == robustGeometry::start )
		  cout << "start";
	  else if ( it->getPointType() == robustGeometry::end )
	  	  cout << "end";
	  else
	  {
		  robustGeometry::BOEvent tmpEv = *it;
		  cout << "intersect" << " line1 " << tmpEv.getLine1().getX1()
				<< " line2 " << tmpEv.getLine1().getX2();

	  }
	  cout << endl;
	}
}
void MakeHobby::printHOEvents( ){

	vector<robustGeometry::BOEvent>::iterator it;
	for ( it =result_e.begin(); it != result_e.end(); ++it){
	  robustGeometry::BOEvent boev = *it;
	  cout << " X " << it->getX() << " Y " << it->getY();
	  if ( it->getPointType() == robustGeometry::start )
	 		  cout << " start line x1 "
	 		  <<  boev.getLine().getX1() <<
			  " y1 " <<  boev.getLine().getY1() <<
			  " x2 " <<  boev.getLine().getX2() <<
			  " y2 " <<  boev.getLine().getY2();
	 	  else if ( it->getPointType() == robustGeometry::end )
	 	  	  cout << " end  line x1 " <<
	 	  	  boev.getLine().getX1() <<
			  " y1 " <<  boev.getLine().getY1() <<
			  " x2 " <<  boev.getLine().getX2() <<
			  " y2 " <<  boev.getLine().getY2();
	 	  else
	 		  cout << " intersect  line1 x1 " <<
	 		  boev.getLine1().getX1() <<
			  " y1 " <<  boev.getLine1().getY1() <<
			  " x2 " <<  boev.getLine1().getX2() <<
			  " y2 " <<  boev.getLine1().getY2() <<
			  " line2 x1 " <<  boev.getLine2().getX1() <<
			  " y1 " <<  boev.getLine2().getY1() <<
			  " x2 " <<  boev.getLine2().getX2() <<
			  " y2 " <<  boev.getLine2().getY2();
	  cout << endl;
	}
}

void MakeHobby::printHOResult_events( ){

	vector<robustGeometry::BOEvent>::iterator it;
	for ( it = result_e.begin(); it != result_e.end(); it++)
	{
	  cout << "result_e: x " <<it->getX() << "  y " << it->getY();
	  cout << endl;
	}
}

void MakeHobby::printHOResult_lines( ){

	set<robustGeometry::BOLine>::iterator it;
	for ( it = result_line.begin(); it != result_line.end(); it++)
	{
	  cout << it->getX1() << " " << it->getY1() << " ";
	  cout << it->getX2() << " " << it->getY2();
	  cout << endl;
	}
}

int MakeHobby::intersect(double sx1, double sy1, double ex2, double ey2,
		double sx3, double sy3, double ex4, double ey4) {
//return =0 OK, 1 lines parallel, 2 no intersection point
//x,y intersection point

	double a1 = 0.0;
	double a2 = 0.0;
	double b1 = 0.0;
	double b2 = 0.0;
	double c1 = 0.0;
	double c2 = 0.0;
	double m = 0.0;
	double x, y;

	a1 = ey2 - sy1;
	b1 = sx1 - ex2;
	c1 = ex2 * sy1 - sx1 * ey2;

	a2 = ey4 - sy3;
	b2 = sx3 - ex4;
	c2 = ex4 * sy3 - sx3 * ey4;

	m = a1 * b2 - a2 * b1;
	if (m == 0)
		return 1;

	x = (b1 * c2 - b2 * c1) / m;
//intersection range

	if (x < sx1 || x < sx3 || x > ex2 || x > ex4)
		return 2;

	y = (a2 * c1 - a1 * c2) / m;

	return 0;
}

void MakeHobby::updateMainActive(double von, double bis )
{
	robustGeometry::BOEvent bo_tmp;
	mainActive.clear();
	set<robustGeometry::BOEvent, robustGeometry::CompBOEventXY>::iterator
	bo_it = events_org.begin();

		for (; bo_it != events_org.end(); bo_it++) {
			if ((bo_it->getX()
					>= von)
					&& (bo_it->getX() < bis)) {
				bo_tmp = *bo_it;
				mainActive.insert(bo_tmp);
			}
			else if (bo_it->getX() > bis )	break;
		}
}
void MakeHobby::locateValue(robustGeometry::ToleranceSquare & ts)
{
	robustGeometry::BOEvent boEv;
	set<robustGeometry::BOEvent, robustGeometry::CompBOEventXY>::iterator
	ma_it = mainActive.begin();

	for (; ma_it != mainActive.end(); ++ma_it)
	{
		boEv = *ma_it;
		double y_org = boEv.getY();
		double x_org = boEv.getX();
		if ( (y_org < ts.getY11() ) &&
		     (y_org >= ts.getY21() ) &&
		     (x_org >= ts.getX11() ) &&
		     (x_org < ts.getX12() ))
		{
			snapSegment(boEv,ts);
		}
	}
}

void MakeHobby::snapSegment(robustGeometry::BOEvent & boEv,
		robustGeometry::ToleranceSquare & ts)
{
	double x = 0.0;
	double y = 0.0;

	x = ts.getSnapX();
	y = ts.getSnapY();

	boEv.setX(x);
	boEv.setY(y);

	result_e.push_back( boEv );

	if ( boEv.getPointType() == robustGeometry::intersect){ return; }

	vector<robustGeometry::BOLine>::iterator sg_it;

	sg_it = find_if(segments.begin(), segments.end(),
			FindSegment(boEv.getLine()) );

	if ( sg_it != segments.end()){
		if ( boEv.getPointType() == robustGeometry::start){
		 	sg_it->setX1_round(x);
		    sg_it->setY1_round(y);
		}
		else {
		    sg_it->setX2_round(x);
		 	sg_it->setY2_round(y);
		};

	}

}

void MakeHobby::splitIntersectedSegments(){

	sort(result_e.begin(),result_e.end(),sortEvents);
	vector<robustGeometry::BOEvent>::iterator re_it = result_e.begin();
	for (; re_it != result_e.end(); re_it++){
		if ( re_it->getPointType( )
		!= robustGeometry::intersect ) continue;

		robustGeometry::BOEvent boEv = * re_it;
//if there isn't a start or endpunkt from linie 1
		if ( (boEv.getOrigX() != boEv.getLine1().getX1() ) &&
		     (boEv.getOrigY() != boEv.getLine1().getY1() ) &&
			 (boEv.getOrigX() != boEv.getLine1().getX2() ) &&
			 (boEv.getOrigY() != boEv.getLine1().getY2() ) ){

			re_it->getLine1().setX2(boEv.getOrigX() );
			re_it->getLine1().setY2(boEv.getOrigY() );

			vector<robustGeometry::BOLine>::iterator
		    sg_it = find_if(segments.begin(),
		    segments.end(),FindSegment(boEv.getLine1()) );

			if ( sg_it != segments.end()){
				robustGeometry::BOLine sg_new = *sg_it;
				sg_it->setX2(boEv.getOrigX());
				sg_it->setY2(boEv.getOrigY());
				sg_it->setX2_round(boEv.getX());
				sg_it->setY2_round(boEv.getY());
//new segment
				sg_new.setX1(boEv.getOrigX());
				sg_new.setY1(boEv.getOrigY());
				sg_new.setX1_round(boEv.getX());
				sg_new.setY1_round(boEv.getY());
				addSegment(sg_new);
//update all other intersections
			vector<robustGeometry::BOEvent>::iterator re_it_z;

			robustGeometry::BOEvent boEv_z;
			for ( re_it_z = result_e.begin();
				  re_it_z != result_e.end();
				  re_it_z++){

			if ( ( re_it_z->getPointType( )
				!= robustGeometry::intersect ) ) continue;

			boEv_z = *re_it_z;
			if ( boEv_z.getLine1() ==  boEv.getLine1() ){
				re_it_z->getLine1( ).setX1(boEv.getOrigX());
				re_it_z->getLine1( ).setY1(boEv.getOrigY());

			} else if ( re_it_z->getLine2() ==  boEv.getLine1() ){
			re_it_z->getLine2( ).setX1(boEv.getOrigX());
			re_it_z->getLine2( ).setY1(boEv.getOrigY());
			}
			 }
			}
		}
		if ( (boEv.getOrigX() != boEv.getLine2().getX1() ) &&
		   	 (boEv.getOrigY() != boEv.getLine2().getY1() ) &&
		   	 (boEv.getOrigX() != boEv.getLine2().getX2() ) &&
			 (boEv.getOrigY() != boEv.getLine2().getY2() ) ){
			re_it->getLine2().setX2(boEv.getOrigX() );
			re_it->getLine2().setY2(boEv.getOrigY() );
			vector<robustGeometry::BOLine>::iterator
			sg_it = find_if(segments.begin(), segments.end(),
					FindSegment(boEv.getLine2()) );
			if ( sg_it != segments.end()){
				robustGeometry::BOLine sg_new = *sg_it;
			   sg_it->setX2(boEv.getOrigX());
			   sg_it->setY2(boEv.getOrigY());
			   sg_it->setX2_round(boEv.getX());
			   sg_it->setY2_round(boEv.getY());
//new segment
			   sg_new.setX1(boEv.getOrigX());
			   sg_new.setY1(boEv.getOrigY());
			   sg_new.setX1_round(boEv.getX());
			   sg_new.setY1_round(boEv.getY());
			   addSegment(sg_new);
//update all other intersections
			   vector<robustGeometry::BOEvent>::iterator re_it_z;
			   robustGeometry::BOEvent boEv_z;
			   for (re_it_z = result_e.begin();
					re_it_z != result_e.end();
					re_it_z++){
				    if ( re_it_z->getPointType( )
				    != robustGeometry::intersect ) continue;
				    boEv_z = *re_it_z;
			if ( boEv_z.getLine1() ==  boEv.getLine2() ){
				re_it_z->getLine1( ).setX1(boEv.getOrigX());
				re_it_z->getLine1( ).setY1(boEv.getOrigY());
			} else if ( re_it_z->getLine2() ==  boEv.getLine2() ){
				re_it_z->getLine2( ).setX1(boEv.getOrigX());
				re_it_z->getLine2( ).setY1(boEv.getOrigY());

			}
			   }
			}
		}
	}
}

/*
tolerance square is open along right.
It's sufficient to check intersection:
  * with the segment and any hot pixel edge
  * between the segment and both the left and bottom edges
*/
bool MakeHobby::computeAllIntersections(robustGeometry::BOEvent & boEv,
		robustGeometry::ToleranceSquare & tolS) {
	double x1 = 0.0;
	double y1 = 0.0;
	double x2 = 0.0;
	double y2 = 0.0;
	double i11 = 0.0;
	double j11 = 0.0;
	double i12 = 0.0;
	double j12 = 0.0;
	double i21 = 0.0;
	double j21 = 0.0;
	double i22 = 0.0;
	double j22 = 0.0;
	int isIntersected = 1;

	x1 = boEv.getLine().getX1();
	y1 = boEv.getLine().getY1();
	x2 = boEv.getLine().getX2();
	y2 = boEv.getLine().getY2();
	i11 = tolS.getX11();
	j11 = tolS.getY11();
	i12 = tolS.getX12();
	j12 = tolS.getY12();
	i21 = tolS.getX21();
	j21 = tolS.getY21();
	i22 = tolS.getX22();
	j22 = tolS.getY22();
//top
	isIntersected = intersect(x1, y1, x2, y2, i11,
			j11, i12,j12);
	if (isIntersected == 0)
		return true;
//bottom
	isIntersected = intersect(x1, y1, x2, y2, i21,
			j21, i22, j22);
	if (isIntersected == 0)
		return true;
//left
	isIntersected = intersect(x1, y1, x2, y2, i11,
			j11, i21, j21);
	if (isIntersected == 0)
		return true;
//right
	isIntersected = intersect(x1, y1, x2, y2, i12,
			j12 , i22, j22 );
	if (isIntersected == 0)
		return true;


	return false;
}
void MakeHobby::intersectsToleranceSquare(robustGeometry::BOEvent & boEv) {
	bool isIntersected = false;
	double y_org = 0.0;

//search corresponding tolerance square
	y_org = boEv.getY();
	set<robustGeometry::ToleranceSquare,
	robustGeometry::CompToleranceSquareY>::
	iterator ts_it = tolSquare.begin();

	for (; ts_it != tolSquare.end(); ++ts_it) {
		if ((y_org >= (ts_it->getY11() - robustGeometry::ScaleFactor))
			&& (y_org <= (ts_it->getY12() +
			robustGeometry::ScaleFactor))) {

			robustGeometry::ToleranceSquare tolSq = *ts_it;
			isIntersected = computeAllIntersections(boEv, tolSq);
			if (isIntersected == true)
			{
				snapSegment(boEv,tolSq);
			}
		}
	}
}

void MakeHobby::addBatch(robustGeometry::BOEvent & boEv) {

	double x = 0.0;
	double y = 0.0;

	x = getRoundFactor( boEv.getX());
	y = getRoundFactor( boEv.getY());

	robustGeometry::HOBatch ba = robustGeometry::HOBatch(x, y);
	batch.insert(ba);
}
;
void MakeHobby::computeToleranceSq() {

	double x11 = 0.0;
	double y11 = 0.0;
	double x12 = 0.0;
	double y12 = 0.0;
	double x21 = 0.0;
	double y21 = 0.0;
	double x22 = 0.0;
	double y22 = 0.0;
	double snap_x = 0.0;
	double snap_y = 0.0;

	robustGeometry::BOEvent boEv;

	set<robustGeometry::BOEvent, robustGeometry::CompBOEventXY >::iterator
	ev_it1 = events_org.begin();
	for (; ev_it1 != events_org.end(); ++ev_it1) {
		robustGeometry::BOEvent boEv = *ev_it1;

		snap_x = getRoundFactor(boEv.getX());
		snap_y = getRoundFactor(boEv.getY());

		x11 = snap_x - robustGeometry::ScaleFactor
				-robustGeometry::Epsilon;

		y11 = snap_y + robustGeometry::ScaleFactor;
		x12 = snap_x + robustGeometry::ScaleFactor;
		y12 = y11;
		x21 = x11;
		y21 = snap_y - robustGeometry::ScaleFactor
				-robustGeometry::Epsilon;

		x22 = x12;
		y22 = y21;

		robustGeometry::ToleranceSquare ts =
		robustGeometry::ToleranceSquare(x11, y11,
		x12, y12, x21, y21, x22, y22,snap_x, snap_y, boEv);

		tolSquare.insert(ts);
	}

};

void MakeBO::IntersectionBO(const Line& line1, const Line& line2,
		Line& result) {
	// Initialisation
	result.Clear();

	result.SetDefined(true);
	if (line1.Size() == 0 || line2.Size() == 0) {
		return; //empty line
	}

	//Initialize event queue: all segment endpoints
	//Sort x by increasing x and y
	priority_queue<avlseg::ExtendedHalfSegment, vector<
			avlseg::ExtendedHalfSegment> ,
			greater<avlseg::ExtendedHalfSegment> >
			q1;
	priority_queue<avlseg::ExtendedHalfSegment, vector<
			avlseg::ExtendedHalfSegment> ,
			greater<avlseg::ExtendedHalfSegment> >
			q2;

	avltree::AVLTree<avlseg::AVLSegment> sss;
	avlseg::ownertype owner;

	int pos1 = 0;
	int pos2 = 0;
	avlseg::ExtendedHalfSegment nextExtHs;
	int src = 0;

	avlseg::AVLSegment left1, right1, common1,
	left2, right2;
	avlseg::AVLSegment tmpL, tmpR;
	robustGeometry::BOOwnerType boOwner;

	MakeHobby* makeHO = new MakeHobby();

	result.StartBulkLoad();

	while ((owner
			= selectNext(line1, pos1, line2,
					pos2, q1, q2, nextExtHs, src))
			!= avlseg::none) {

		if (owner == avlseg::first)
			boOwner = robustGeometry::first;
		else
			boOwner = robustGeometry::second;

		if ( owner == 1 ) boOwner = robustGeometry::first;
			   else boOwner = robustGeometry::second;

		robustGeometry::BOLine boLine = robustGeometry::BOLine(
				nextExtHs.GetLeftPoint().GetX(),
				nextExtHs.GetLeftPoint().GetY(),
				nextExtHs.GetRightPoint().GetX(),
				nextExtHs.GetRightPoint().GetY(), boOwner);

		robustGeometry::BOEvent boEvent_s = robustGeometry::BOEvent(
				nextExtHs.GetLeftPoint().GetX(),
				nextExtHs.GetLeftPoint().GetY(),
				robustGeometry::start,boOwner, boLine);
		addInitialEvent(boEvent_s);

		robustGeometry::BOEvent boEvent_e = robustGeometry::BOEvent(
				nextExtHs.GetRightPoint().GetX(),
				nextExtHs.GetRightPoint().GetY(),
				robustGeometry::end, boOwner, boLine);
		addInitialEvent(boEvent_e);

		makeHO->addSegment(boLine);

	};

	//printInitialEvents();

	doBOCases();

    printBOEvents();
	makeHO->set_BOEvents( get_BOEvents());
	makeHO->printHOInitialEvents();

    makeHO->doHO( );

    makeHO->printHOEvents();
   	makeHO->printHOResult_events();
   	makeHO->printHOResult_lines();
    makeHO->sortRoundedSegments();

	vector<robustGeometry::BOLine> segments = makeHO->get_result_segments();

	vector<robustGeometry::BOLine>::iterator it = segments.begin();
	robustGeometry::BOLine boL = *it;
	it++;
  int edgeno = 0;

	for (; it != segments.end(); ++it ){
		if ( ( boL.getOwner() != it->getOwner() ) &&
			 ( boL.getX1_round() == it->getX1_round() ) &&
			 ( boL.getY1_round() == it->getY1_round() ) &&
			 ( boL.getX2_round() == it->getX2_round() ) &&
			 ( boL.getY2_round() == it->getY2_round() ) ){

			Point p,q;

			p.Set(it->getX1(), it->getY1());
			q.Set(it->getX2(), it->getY2());
			HalfSegment hs1(true,p,q);
			HalfSegment hs2(false,p,q);
      hs1.attr.edgeno = edgeno;
      hs2.attr.edgeno = edgeno;
      edgeno++;
			result += hs1;
			result += hs2;


cout << " Intersected Line 1 Owner " <<  boL.getOwner() <<
" Original X1 " <<	boL.getX1() << " Y1 " << boL.getY1() <<
" x2 " << boL.getX2() << " y2 " << boL.getY2() <<
" Roundedl X1 " <<	boL.getX1_round() << " Y1 " <<
boL.getY1_round() << " x2 " << boL.getX2_round() <<
" y2 " << boL.getY2_round() << endl;
cout << "             Line 2 Owner " <<  it->getOwner() <<
" Original X1 " <<	it->getX1() << " Y1 " << it->getY1() <<
" x2 " << it->getX2() << " y2 " << it->getY2() <<
" Roundedl X1 " <<	it->getX1_round() << " Y1 " <<
it->getY1_round() << " x2 " << it->getX2_round()
<< " y2 " << it->getY2_round() << endl;


		}

		boL = *it;

	}
//	result.EndBulkLoad(true, false);
	result.EndBulkLoad(true, true);
}

/*
 ~Intersection~ operator for objects line -line

*/

static int intersectionBO_ll(Word* args, Word& result, int message,
		Word& local, Supplier s) {

	result = qp->ResultStorage(s);
	Line *line1 = ((Line*) args[0].addr);
	Line *line2 = ((Line*) args[1].addr);

	cerr << "line1 = ";
	line1->Print(cerr);
	cerr << endl;
	cerr << "line2 = ";
	line2->Print(cerr);
	cerr << endl;

	if (line1->IsDefined() && line2->IsDefined()) {
		if (line1->IsEmpty() || line2->IsEmpty()) {
			cout << __PRETTY_FUNCTION__ << ": "
					"lines are empty!" << endl;
			((Line *) result.addr)->SetDefined(false);
			return (0);
		} else if (line1->BoundingBox().IsDefined()
				&& line2->BoundingBox().IsDefined()) {
			if (line1->BoundingBox().
					Intersects(line2->BoundingBox())) {
				MakeBO bo;
				cout << __PRETTY_FUNCTION__ << ": "
						"BoundingBox Intersects!"
						<< endl;
				bo.IntersectionBO(*line1, *line2,
				*static_cast<Line*> (result.addr));

				return (0);
			} else {
				cout << __PRETTY_FUNCTION__ << ": "
					"BoundingBox don't intersects!" << endl;
				((Line *) result.addr)->Clear();
				return (0);
			}
		} else {
			MakeBO bo;
			cout << __PRETTY_FUNCTION__ << ": "
					"BoundingBox isn't defined!"
					<< endl;
			bo.IntersectionBO(*line1, *line2,
					*static_cast<Line*> (result.addr));
			return (0);
		}
	} else {
		cout << __PRETTY_FUNCTION__ << ": "
				"lines aren't defined!" << endl;
		cout << __PRETTY_FUNCTION__ << ": value mapping "
				"implemented only for lines."
		         << endl;
		((Line *) result.addr)->Clear();
		((Line *) result.addr)->SetDefined(false);
		//assert(false); // TODO: Implement IntersectionBOGeneric.

		return (0);
	}
}

/*
 Operator Description

*/

struct intersectionInfo: OperatorInfo {
	intersectionInfo() {
		name = "intersectionBO";
		signature = Line::BasicType()
		+ " x " + Line::BasicType() + " -> "
				+ Line::BasicType();
		syntax = "_ intersectionBO _";
		meaning = "Intersection predicate for two Lines.";
	}
};

/*
10.3 Selection functions

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

10.1.1 Selection function ~RGSetOpSelect~

This select function is used for the ~intersectionBO~ operator

*/

int RGSetOpSelect(ListExpr args) {
	string a1 = nl->SymbolValue(nl->First(args));
	string a2 = nl->SymbolValue(nl->Second(args));

	if (a1 == Line::BasicType()) {
		if (a2 == Line::BasicType())
			return 0;
		return -1;
	}

	return -1;
}

/*
10.4 Value mapping functions

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators there are
several value mapping functions, one for each possible combination of input
parameter types.

10.4.1 Value mapping functions of operator ~intersection~

*/

ValueMapping intersectionVM[] = { intersectionBO_ll };

/*
 Definition of the operators

*/
Operator test("intersectionBO", intersectionSpec, 1, intersectionVM,
		RGSetOpSelect, intersectionTM);

/*
5 Creating the Algebra

*/

class RobustGeometryAlgebra: public Algebra {

public:
	RobustGeometryAlgebra() :
		Algebra() {
		AddOperator(&test);
	}
	~RobustGeometryAlgebra() {
	}
	;
};

/*
12 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C" Algebra*
InitializeRobustGeometryAlgebra(NestedList* nlRef, QueryProcessor* qpRef) {
	nl = nlRef;
	qp = qpRef;
	return (new RobustGeometryAlgebra());
};

