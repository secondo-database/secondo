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
[1] Header File of the RobustGeometry Algebra


[TOC]
1 Overview
2 Defines and includes

*/
#ifndef __ROBUSTGEOMETRY_ALGEBRA_H__
#define __ROBUSTGEOMETRY_ALGEBRA_H__

#include "HalfSegment.h"
#include "AVLSegment.h"
#include "Coord.h"


namespace robustGeometry{


enum boPointType{nothing, start, end, intersect};
//enum boOwnertype{none, first, second, both};


class BOEvent{
public:

/*
Constructors
~Standard Constructor~

*/
  BOEvent();


  BOEvent(const avlseg::ExtendedHalfSegment& hs,
		  const double x, const double y,
  const boPointType pointtype , const avlseg::ownertype owner);

  BOEvent(const avlseg::ExtendedHalfSegment& ahs ,
		  const avlseg::ExtendedHalfSegment& bhs,
	  const double x, const double y,
	  const boPointType pointtype );


  BOEvent(const double x, const double y,
		  const boPointType pointType);


   ~BOEvent() {}

  void setX( double x) 	{ this->x = x; };
  void setY( double y) 	{ this->y = y; };
  void setExtededHalfSegment
  (const avlseg::ExtendedHalfSegment& extendedHalfSegment )
  { this->extendedHalfSegment = extendedHalfSegment; };

  void setAboveEHS
  (const avlseg::ExtendedHalfSegment& extendedHalfSegment )

  { this->aboveEHS = extendedHalfSegment; };

  void setBelowEHS(const avlseg::ExtendedHalfSegment&
		  extendedHalfSegment )
  { this->belowEHS = extendedHalfSegment; };

  void setOwner( avlseg::ownertype owner)
  { this->owner = owner; };

  void setPointType( boPointType pointType)
  { this->pointType = pointType; };

  double getX( ) const { return x; };
  double getY( ) const { return y; };

  avlseg::ExtendedHalfSegment getExtendedHalfSegment( )
  const   { return extendedHalfSegment; };

  avlseg::ExtendedHalfSegment getAboveEHS( ) const
  { return aboveEHS; };

  avlseg::ExtendedHalfSegment getBelowEHS( ) const
  { return belowEHS; };

  avlseg::ownertype getOwner( ) const
  { return owner; };
  boPointType getPointType( ) const
  { return pointType; };

  void Print(ostream& out)const;



private :

  double x;
  double y;
  boPointType pointType;
  avlseg::ownertype owner;
  avlseg::ExtendedHalfSegment extendedHalfSegment;
  avlseg::ExtendedHalfSegment aboveEHS;
  avlseg::ExtendedHalfSegment belowEHS;
}; //end of BOEvent



}; //end of namespaces



#endif // __ROBUSTGEOMETRY_ALGEBRA_H__












