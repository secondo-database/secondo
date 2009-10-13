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

*/
/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]

1.1 Declarations Necessary for Traffic Algebra

*/
#ifndef __TRAFFIC_ALGEBRA_H__
#define __TRAFFIC_ALGEBRA_H__

#include "StandardAttribute.h"
#include "TemporalAlgebra.h"
#include "DateTime.h"
#include "NestedList.h"

/*
2 class MGPSecUnit

*/
class MGPSecUnit : public StandardAttribute
{
  public:

/*
2.1 Constructors and Destructor

*/

    MGPSecUnit();

    MGPSecUnit(int secId, int direct, Interval<Instant> timeInterval);

    MGPSecUnit( const MGPSecUnit& in_xOther );

    ~MGPSecUnit();

/*
2.2 Methods of class ~mgpsecunit~

Get and Set private attributes.

*/

    int GetSecId() const;

    int GetDirect() const;

    Interval<Instant> GetTimeInterval() const;

    void SetSecId(int secId);

    void SetDirect(int dir);

    void SetTimeInterval(Interval<Instant> time);

    size_t Sizeof() const;
    
    size_t HashValue() const;
    
    void CopyFrom( const StandardAttribute* right );

    int Compare( const Attribute* arg ) const;

    bool Adjacent( const Attribute *arg ) const;

    MGPSecUnit *Clone() const;

    ostream& Print( ostream& os ) const;

/*
Functions for Secondo integration.

*/
    static ListExpr Out(ListExpr typeInfo, Word value);

    static Word In(const ListExpr typeInfo, const ListExpr instance,
                   const int errorPos, ListExpr& errorInfo, bool& correct);

    static bool CheckKind( ListExpr type, ListExpr& errorInfo );

  private:

    int m_secId;
    int m_direct;
    Interval<Instant> m_time;

};

#endif // __NETWORK_ALGEBRA_H__
