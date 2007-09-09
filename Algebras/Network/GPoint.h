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

1 Declaration of datatype for type-constructor GPoint

March 2004 Victor Almeida

Mai-Oktober 2007 Martin Scheppokat


1.1 Overview


This file contains the implementation of ~gline~


2.1 Defines, includes, and constants

*/
#ifndef GPOINT_H_
#define GPOINT_H_

// TODO: Error-Message is not working. TupleIdentifier.h has no define.
//#ifndef TUPLE_IDENTIFIER_H
//#error TupleIdentifier.h by NetworkAlgebra.h. Please include in *.cpp-File.
//#endif



enum Side { Down, Up, None };

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
    GPoint( bool in_bDefined,
            int in_iNetworkId = 0, 
            TupleId in_xRid = 0, 
            double in_dLocation = 0.0, 
            Side in_xSide = None ):
    m_iNetworkId( in_iNetworkId ), 
    m_xRouteLocation( in_xRid, in_dLocation, in_xSide ),
    m_bDefined( in_bDefined )
    {}
/*
The constructor.

*/
    GPoint( const GPoint& in_xOther ):
    m_iNetworkId( in_xOther.m_iNetworkId ), 
    m_xRouteLocation( in_xOther.m_xRouteLocation ), 
    m_bDefined( in_xOther.m_bDefined )
    {}
/*
The copy constructor.

*/
    GPoint& operator=( const GPoint& in_xOther )
    {
      m_bDefined = in_xOther.m_bDefined;
      if( m_bDefined )
      {
        m_iNetworkId = in_xOther.m_iNetworkId; 
        m_xRouteLocation = in_xOther.m_xRouteLocation; 
      }
      return *this;
    }
/*
The assignement operator redefinition.

*/

    int GetNetworkId() const
    {
      return m_iNetworkId;
    }
/*
Returns the network id.

*/
    TupleId GetRouteId() const
    {
      return m_xRouteLocation.rid;
    }
/*
Returns the route id.

*/
    double GetPosition() const
    {
      return m_xRouteLocation.d;
    }
/*
Returns the relative position of the graph point in the route.

*/
    Side GetSide() const
    {
      return m_xRouteLocation.side;
    }
/*
Returns the side on the route of the graph point.

*/
    

    bool IsDefined() const
    {
      return m_bDefined;
    }

    void SetDefined( bool in_bDefined )
    {
      m_bDefined = in_bDefined;
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
    
    int m_iNetworkId;
/*
The network id.

*/
    RLoc m_xRouteLocation;
/*
The route location.

*/
    bool m_bDefined;
/*
A flag indicating whether the instance is defined or not.

*/ 
};


#endif /*GPOINT_H_*/
