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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of GLine in Module Network Algebra

March 2004 Victor Almeida

Mai-Oktober 2007 Martin Scheppokat

[TOC]

1 Overview


This file contains the implementation of ~gline~


2 Defines, includes, and constants

*/
#ifndef GPOINT_H_
#define GPOINT_H_

// TODO: Error-Message is not working. TupleIdentifier.h has no define.
//#ifndef TUPLE_IDENTIFIER_H
//#error TupleIdentifier.h by NetworkAlgebra.h. Please include in *.cpp-File.
//#endif



enum Side { Up, Down, None };

struct RLoc
{
  RLoc() {}
/*
The simple constructor. Should not be used.

*/
  RLoc( const TupleId rid, 
        const double d, 
        const Side side ):
    rid( rid ), d( d ), side( side )
  {}
/*
The constructor.

*/
  RLoc( const RLoc& rloc ):
  rid( rloc.rid ), d( rloc.d ), side( rloc.side )
  {}
/*
The copy constructor.

*/
  RLoc& operator=( const RLoc& rloc )
  {
    rid = rloc.rid; d = rloc.d; side = rloc.side;
    return *this;
  }

  TupleId rid;
/*
The route id.

*/
  double d;
/*
The distance in the route.

*/
  Side side;
/*
The side in the route.

*/
};

class GPoint : public StandardAttribute
{
  public:
    GPoint() {}
/*
The simple constructor.

*/
    GPoint( bool defined,
              SmiRecordId nid = 0, 
              TupleId rid = 0, 
              double d = 0.0, 
              Side side = None ):
    nid( nid ), rloc( rid, d, side ),
    defined( defined )
    {}
/*
The constructor.

*/
    GPoint( const GPoint& gp ):
    nid( gp.nid ), rloc( gp.rloc ), defined( gp.defined )
    {}
/*
The copy constructor.

*/
    GPoint& operator=( const GPoint& gp )
    {
      defined = gp.defined;
      if( defined )
      {
        nid = gp.nid; 
        rloc = gp.rloc; 
      }
      return *this;
    }
/*
The assignement operator redefinition.

*/

    SmiRecordId GetNetworkId() const
    {
      return nid;
    }
/*
Returns the network id.

*/
    TupleId GetRouteId() const
    {
      return rloc.rid;
    }
/*
Returns the route id.

*/
    double GetPosition() const
    {
      return rloc.d;
    }
/*
Returns the relative position of the graph point in the route.

*/
    Side GetSide() const
    {
      return rloc.side;
    }
/*
Returns the side on the route of the graph point.

*/
    

    bool IsDefined() const
    {
      return defined;
    }

    void SetDefined( bool d )
    {
      defined = d;
    }

    size_t Sizeof() const
    {
      return sizeof(GPoint);
    }

    size_t HashValue() const
    {
      return 0;
    }

    void CopyFrom( const StandardAttribute* right )
    {
      const GPoint* gp = (const GPoint*)right;
      *this = *gp;
    }

    int Compare( const Attribute* arg ) const
    {
      return 0;
    }

    bool Adjacent( const Attribute *arg ) const
    {
      return false;
    }

    GPoint *Clone() const
    {
      return new GPoint( *this );
    }

    ostream& Print( ostream& os ) const
    {
      os << "GPoint::Print" << endl;
      return os;
    }

    static ListExpr OutGPoint( ListExpr typeInfo, Word value );

    static Word InGPoint( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct );

    static Word CreateGPoint( const ListExpr typeInfo );

    static void DeleteGPoint( const ListExpr typeInfo, Word& w );

    static void CloseGPoint( const ListExpr typeInfo, Word& w );

    static Word CloneGPoint( const ListExpr typeInfo, const Word& w );

    static void* CastGPoint( void* addr );

    static int SizeOfGPoint();

    static ListExpr GPointProperty();

    static bool CheckGPoint( ListExpr type, ListExpr& errorInfo );



  private:
    
    SmiRecordId nid;
/*
The network id.

*/
    RLoc rloc;
/*
The route location.

*/
    bool defined;
/*
A flag indicating whether the instance is defined or not.

*/ 
};


#endif /*GPOINT_H_*/
