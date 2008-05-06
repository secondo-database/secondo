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

1.1 Declaration of GLine

March 2004 Victor Almeida

Mai-Oktober 2007 Martin Scheppokat

Dezember 2007 Simone Jandt

March 2008 Simone Jandt added distance operation for two glines

Defines, includes, and constants

*/
#ifndef GLINE_H_
#define GLINE_H_

#include "StandardAttribute.h"
#include "DBArray.h"


struct RouteInterval
{
  RouteInterval()
  {
  }

  RouteInterval(int in_iRouteId,
                double in_dStart,
                double in_dEnd):
    m_iRouteId(in_iRouteId),
    m_dStart(in_dStart),
    m_dEnd(in_dEnd)
  {
  }

  int m_iRouteId;
/*
The route id.

*/
  double m_dStart;

  double m_dEnd;
/*
The distance interval in the route.

*/
};

class GLine : public StandardAttribute
{
/*
The simple constructor. Should not be used.

*/
  public:
    GLine();

    GLine(int in_iSize);


    GLine(GLine* in_xOther);

    ~GLine() {};


/*
The constructor.

*/
    GLine( ListExpr in_xValue,
         int in_iErrorPos,
         ListExpr& inout_xErrorInfo,
         bool& inout_bCorrect);

    void SetNetworkId(int in_iNetworkId);

    void AddRouteInterval(int in_iRouteId,
                          double in_dStart,
                          double in_dEnd);

    static ListExpr Out( ListExpr typeInfo, Word value );

    static Word In( const ListExpr typeInfo,
                    const ListExpr instance,
                    const int errorPos,
                    ListExpr& errorInfo,
                    bool& correct );

    static Word Create( const ListExpr typeInfo );

    static void Delete( const ListExpr typeInfo, Word& w );

    static void Close( const ListExpr typeInfo, Word& w );

    static Word Clone( const ListExpr typeInfo,
                       const Word& w );

    static void* Cast( void* addr );

    int NumOfFLOBs() const;

    FLOB *GetFLOB( const int i );

    static int SizeOf();

    static ListExpr Property();

    static bool Check( ListExpr type, ListExpr& errorInfo );

    bool IsDefined() const;

    void SetDefined(bool);

    size_t Sizeof() const;

    int Compare(const Attribute*) const;

    bool Adjacent(const Attribute*) const;

    Attribute* Clone() const;

    size_t HashValue() const;

    void CopyFrom(const StandardAttribute*);

    double GetLength ();

    int GetNetworkId();

    void Get(const int i, const RouteInterval* &ri) const;

    int NoOfComponents();

    double distance(GLine* pgl2);

    bool IsSorted();

    void SetSorted(bool);


  private:

    int m_iNetworkId;
/*
The network id.

*/

    bool m_bDefined;
/*
True if all members are defined

*/

    bool m_bSorted;
/*
True if route intervals of the gline are sorted.

*/

    double m_dLength;

/*
Length of the gline is stored in parameter length

*/
    DBArray<RouteInterval> m_xRouteIntervals;

/*
The array of route intervals.

*/

};

#endif /*GLINE_H_*/
