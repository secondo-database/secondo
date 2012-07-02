/*

//[_] [\_]
//[TOC] [\tableofcontents]
//[Title] [ \title{RobustGeometry} \author{Katja Koch} \maketitle]
//[times] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]



[1]  Header File of the RobustGeometryAlgebra

[TOC]

1 Overview

This header file essentially contains the definition of the classes ~BOLine~ and
~BOEvent~ used for Bentley-Ottmann algorithm and for Hobby algorithm.
~HOBatch~ and ~ToleranceSquare~ used for Hobby algorithm.


1 Preliminaries

1.1 Includes and global declarations

*/


#ifndef __ROBUSTGEOMETRY_ALGEBRA_H__
#define __ROBUSTGEOMETRY_ALGEBRA_H__
#include "HalfSegment.h"
#include "AVLSegment.h"
#include "Coord.h"
#include <iosfwd>

using std::ostream;

namespace robustGeometry {

/*
stay away of this infinitesimal value to on side of the border,
used to build and ask the borders of ToleranceSquare

*/

double const Epsilon = 0.000001;

/*
ScaleFactor used for variable grid size

*/
double const ScaleFactor = 0.5;

/*
valid values for BOEvents

*/
enum BOPointType{start, end, intersect};

/*
valid values for BOOwner

*/
enum BOOwnerType{first, second, both};


/*
1 Class  ~BOLine~

This class implements the line object

*/
class BOLine
{
public:
/*
1.1 Constructors and Destructors

*/
	BOLine(){};
	BOLine(const BOLine& line);
	BOLine( const double x1, const double y1,
			const double x2, const double y2,
			const BOOwnerType owner);
	~BOLine(void){};
/*
1.2 Functions for reading and setting values

*/
	void setX1( const double x ) const ;
	void setX2( const double x ) const ;
	void setY1( const double y ) const ;
	void setY2( const double y ) const ;
	void setX1_round( const double x );
	void setX2_round( const double x );
	void setY1_round( const double y );
	void setY2_round( const double y );
	void setOwner( const BOOwnerType owner );

	double getX1( ) const { return x1; };
	double getX2( ) const { return x2; };
	double getY1( ) const { return y1; };
	double getY2( ) const { return y2; };
	double getX1_round( ) const { return x1_r; };
	double getX2_round( ) const { return x2_r; };
	double getY1_round( ) const { return y1_r; };
	double getY2_round( ) const { return y2_r; };
	BOOwnerType getOwner( ) const { return owner; };

	friend bool operator < (const BOLine& l1,
			const BOLine& l2) {
	if ( l1.getX1() < l2.getX1() )
			return true;
		else if ( (l1.getX1() == l2.getX1() ) &&
					  (l1.getY1() <  l2.getY1() ) )
				return true;
			else if ( (l1.getX1() == l2.getX1() ) &&
					  (l1.getY1() == l2.getY1() ) &&
					  (l1.getX2() < l2.getX2() ) )
				return true;
			else if ( (l1.getX1() == l2.getX1() ) &&
					  (l1.getY1() == l2.getY1() ) &&
					  (l1.getX2() == l2.getX2() ) &&
					  (l1.getY2() <  l2.getY2() ) )
				return true;
			else if ( (l1.getX1() == l2.getX1() ) &&
					  (l1.getY1() == l2.getY1() ) &&
					  (l1.getX2() == l2.getX2() ) &&
					  (l1.getY2() == l2.getY2() ) &&
					  (l1.getOwner() < l2.getOwner()))
						return true;
			else
				return false;

	   }
	friend bool operator == (const BOLine& l1,
			const BOLine& l2) {
	if ( (l1.getX1() == l2.getX1() ) &&
		 (l1.getY1() == l2.getY1() ) &&
		 (l1.getX2() == l2.getX2() ) &&
		 (l1.getY2() == l2.getY2() ) &&
		 (l1.getOwner() == l2.getOwner()))
		return true;
	else
		return false;
	}

private:
	mutable double x1;
	mutable double y1;
	mutable double x2;
	mutable double y2;
	BOOwnerType owner;
	double x1_r;
	double y1_r;
	double x2_r;
	double y2_r;

};

/*
3.1 Class  ~CompBOLine~

for finding and sorting the BOLine by increasing x and y

*/

class CompBOLine{
public:
	bool operator()(BOLine l1, BOLine l2) {
		if ( l1.getX1() < l2.getX1() )
			return true;
		else if ( (l1.getX1() == l2.getX1() ) &&
				  (l1.getY1() <  l2.getY1() ) )
			return true;
		else if ( (l1.getX1() == l2.getX1() ) &&
				  (l1.getY1() == l2.getY1() ) &&
				  (l1.getX2() < l2.getX2() ) )
			return true;
		else if ( (l1.getX1() == l2.getX1() ) &&
				  (l1.getY1() == l2.getY1() ) &&
				  (l1.getX2() == l2.getX2() ) &&
				  (l1.getY2() <  l2.getY2() ) )
			return true;
		else

			return false;
	};
	friend bool operator == (const BOLine l1, const BOLine l2) {
		if ( (l1.getX1() == l2.getX1() ) &&
			 (l1.getY1() == l2.getY1() ) &&
			 (l1.getX2() == l2.getX2() ) &&
			 (l1.getY2() == l2.getY2() ) &&
			 (l1.getOwner() == l2.getOwner()))
			return true;
		else
			return false;
	};

};
/*
3.1 Class  ~CompBOLineXYOwn~

for finding and sorting the BOLine by increasing x and y and owner

*/

class CompBOLineXYOwn{
public:
	bool operator()(BOLine l1, BOLine l2) {
		if ( l1.getX1() < l2.getX1() )
			return true;
		else if ( (l1.getX1() == l2.getX1() ) &&
				  (l1.getY1() <  l2.getY1() ) )
			return true;
		else if ( (l1.getX1() == l2.getX1() ) &&
				  (l1.getY1() == l2.getY1() ) &&
			      (l1.getOwner() < l2.getOwner() ) )
			return true;
		else if ( (l1.getX1() == l2.getX1() ) &&
				  (l1.getY1() == l2.getY1() ) &&
				  (l1.getOwner() == l2.getOwner() ) &&
				  (l1.getX2() < l2.getX2() ) )
			return true;
		else if ( (l1.getX1() == l2.getX1() ) &&
				  (l1.getY1() == l2.getY1() ) &&
				  (l1.getOwner() == l2.getOwner() ) &&
				  (l1.getX2() == l2.getX2() ) &&
				  (l1.getY2() <  l2.getY2() ) )
			return true;
		else
			return false;
	};

};

/*
3 Class ~BOEvent~

This class is implementing the priority of events used in the
plane-sweep-algorithm.

*/
class BOEvent
{
public:
/*
3.1 Constructors and Destructors

*/
	BOEvent(){};
	BOEvent(const double x, const double y,
		const BOPointType pType, const BOOwnerType owner);
	BOEvent(const double x, const double y,
		const BOPointType pType, const BOOwnerType owner,
		const BOLine& line);
	BOEvent(const double x, const double y,
		const BOPointType pType, const BOOwnerType owner,
		const BOLine& line1,const BOLine& line2 );
	~BOEvent(void){};
/*
3.2 Functions for reading and setting values

*/
	void setX( const double x );
	void setY( const double y );
	void setPointType( const BOPointType pointType );
	void setOwner( const BOOwnerType owner );
	void setLine( const BOLine& line );
	void setLine1( const BOLine& line1 );
	void setLine2( const BOLine& line2 );
	double getX( ) const { return x; };
	double getY( ) const { return y; };
	double getOrigX( ) const { return origX; };
	double getOrigY( ) const { return origY; };

	BOLine& getLine( ) { return line; };
	BOLine& getLine1( ) { return line1; };
	BOLine& getLine2( ) { return line2; };
	BOPointType getPointType( )const { return pointType; };
	BOOwnerType getOwner( )const { return owner; };
	void Print(ostream& out) const;
	BOLine& getLineAbove( ){ return lineAbove; };
	BOLine& getLineBelow( ){ return lineBelow; };

private :
  double x;
  double y;
  double origX;
  double origY;
  BOPointType pointType;
  BOOwnerType owner;
  BOLine line,line1,line2,lineAbove, lineBelow;
};

/*
3.1 Class ~CompBOEventXY~

for finding and sorting the BOEvents by increasing x and y

*/


class CompBOEventXY{
public:
	bool operator()(BOEvent e1, BOEvent e2) {
	if ( e1.getX() < e2.getX() )
		return true;
	else if ( (e1.getX() == e2.getX() ) &&
			  (e1.getY() <  e2.getY() ) )
		return true;
	else if ( (e1.getX() == e2.getX() ) &&
			  (e1.getY() ==  e2.getY() ) &&
			  (e1.getPointType() < e2.getPointType()))
		return true;
	else if ( (e1.getX() == e2.getX() ) &&
			  (e1.getY() ==  e2.getY() ) &&
			  (e1.getPointType() == e2.getPointType()) &&
			  (e1.getOwner() < e2.getOwner() ) )
		return true;
	else if ( (e1.getX() == e2.getX() ) &&
			  (e1.getY() ==  e2.getY() ) &&
			  (e1.getPointType() == e2.getPointType()) &&
			  (e1.getOwner() == e2.getOwner() ) ){
		if ( e1.getPointType() == intersect ){
			if ( e1.getLine1() < e2.getLine1() )
				return true;
			if ( ( e1.getLine1() == e2.getLine1()) &&
				 ( e1.getLine2() < e2.getLine2()))
				return true;
			else return false;
		}
		else if ( e1.getLine() < e2.getLine() )
			return true;
		else
			return false;

	}

		else
			return false;
	};
	friend bool operator == (const BOEvent e1, const BOEvent e2) {
		if ( (e1.getX() == e2.getX() ) &&
			 (e1.getY() == e2.getY() ) &&
			 (e1.getPointType() == e2.getPointType()) &&
			 (e1.getOwner() == e2.getOwner()) ){

				return true;

		}
		else
			return false;
	};

};


/*
3 Class ~HOBatch~

This class is implementing the batch
[->]sweep-line for hobby-algorithm.

*/
class HOBatch
{
public:
/*
3.1 Constructors and Destructors

*/
	HOBatch(){};
	HOBatch(const double x, const double y);
	~HOBatch(void){};
/*
3.2 Functions for reading and setting values

*/
	void setX( const double x );
	void setY( const double y );
	double getX( ) const { return x; };
	double getY( ) const { return y; };
	void Print(ostream& out) const {};
private :
  double x;
  double y;
};
/*
3.1 Class  ~CompBatch~

for sorting the batch by increasing x

*/

class CompBatch{
public:
	bool operator()(HOBatch e1, HOBatch e2) {
		if ( e1.getX() < e2.getX() )
			return true;
		else
			return false;
	};
};


/*
3 Class  ~ToleranceSquare~

implements a "tolerance square" as used in the Snap Rounding algorithm.
a tolerance square contains the boundary of a grid corresponding to the scaleFactor

*/
class ToleranceSquare
	{

	public:
/*
3.1 Constructors and Destructors

*/
	ToleranceSquare(){};
	ToleranceSquare(const double x11, const double y11,
		const double x12, const double y12,
		const double x21, const double y21,
		const double x22, const double y22,
		const double snap_x, const double snap_y,
		const BOEvent& boEv)
	{
		setX11( x11 );
		setY11( y11 );
		setX12( x12 );
		setY12( y12 );
		setX21( x21 );
		setY21( y21 );
		setX22( x22 );
		setY22( y22 );
		setSnapX( snap_x );
		setSnapY( snap_y );

		setBOEvent( boEv );
	};
	~ToleranceSquare(void){};
/*
3.1.2 Functions for reading and setting values

*/
		void setX11( const double x11 );
		void setY11( const double y11 );
		void setX12( const double x12 );
		void setY12( const double y12 );
		void setX21( const double x21 );
		void setY21( const double y21 );
		void setX22( const double x22 );
		void setY22( const double y22 );
		void setSnapX( const double snap_x );
		void setSnapY( const double snap_y );


		void setBOEvent( const BOEvent& boEv );

		double getX11( ) const { return x11; };
		double getY11( ) const { return y11; };
		double getX12( ) const { return x12; };
		double getY12( ) const { return y12; };
		double getX21( ) const { return x21; };
		double getY21( ) const { return y21; };
		double getX22( ) const { return x22; };
		double getY22( ) const { return y22; };
		double getSnapX( ) const { return snap_x; };
		double getSnapY( ) const { return snap_y; };

		const BOEvent& getBOEvent( ){ return boEv; };
		void Print(ostream& out)const;

	private :
	  double x11;
	  double y11;
	  double x12;
	  double y12;
	  double x21;
	  double y21;
	  double x22;
	  double y22;
	  double snap_x;
	  double snap_y;

	  BOEvent boEv;
	  double scale(double value)
	  {
		return (double) (value * ScaleFactor);
	  };

	};

/*
3.5 Class  ~CompToleranceSquareY~

implements a "tolerance square" as used in the Snap Rounding algorithm.
a tolerance square contains the boundary of a grid corresponding to the scaleFactor

*/
class CompToleranceSquareY{
public:
	bool operator()(ToleranceSquare t1, ToleranceSquare t2) {
		if ( t1.getY21() < t2.getY21() )
			return true;
		else if ( ( t1.getY21() == t2.getY21() ) &&
				  ( t1.getX21() < t2.getX21()  ) )
			return true;
		else
			return false;
	};

};

class Point
	{
	public:
		Point(){};
		Point(const double x, const double y)
		{
			setX( x );
			setY( y );
		};


		~Point(void){};

		void setX( const double x );
		void setY( const double y );
		double getX( ) const { return x; };
		double getY( ) const { return y; };

	private :
	  double x;
	  double y;

	};




}
; //end of namespaces


#endif // __ROBUSTGEOMETRY_ALGEBRA_H__

