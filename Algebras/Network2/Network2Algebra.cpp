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

1 Implementation of Algebra Network


March 2004 Victor Almeida

Mai-Oktober 2007 Martin Scheppokat

February 2008 -  Simone Jandt

October 2008 - Jianqiu Xu

April-Mai 2011 - Nina Wrede

1.1 Defines, includes, and constants

*/

#include <sstream>
#include <time.h>
#include <map>
#include <iterator>

#include "TupleIdentifier.h"
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "RTreeAlgebra.h"
#include "../../Tools/Flob/DbArray.h"
#include "../../Tools/Flob/Flob.h"
#include "SpatialAlgebra.h"
#include "Network2Algebra.h"
#include "RectangleAlgebra.h"
#include "StandardTypes.h"
#include "Algebra.h"
#include "Messages.h"
#include "Network2Manager.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "../../include/ListUtils.h"
#include "../../include/TypeMapUtils.h"


using namespace network2;

extern NestedList* nl;
extern QueryProcessor* qp;
static map<int,string> *netList;

/*
2 Helping structs, methods and classes

Sending a message via the message-center

*/
void sendMessage ( string in_strMessage )
{
  // Get message-center and initialize message-list
  static MessageCenter* xMessageCenter= MessageCenter::GetInstance();
  NList xMessage;
  xMessage.append ( NList ( "error" ) );
  xMessage.append ( NList().textAtom ( in_strMessage ) );
  xMessageCenter->Send ( xMessage );
}

/*
Computes a spatial BoundingBox of a RouteInterval

*/

Rectangle<2> RouteInterval::BoundingBox ( Network* pNetwork,
                                          const Geoid* geoid /*=0*/ ) const
{
  if(geoid){
    cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
    << endl;
    assert( !geoid ); // TODO: implement spherical geometry case
  }
  if ( AlmostEqual ( m_dStart , m_dEnd ) )
  {
    Point *p = ( GPoint ( true, pNetwork->GetId(), m_iRouteId,
                          m_dStart ) ).ToPoint ( pNetwork );
    Rectangle<2> bbox = Rectangle<2> ( true,
                                       p->GetX(), p->GetX(),
                                       p->GetY(), p->GetY() );
    p->DeleteIfAllowed();
    return bbox;
  }
  else
  {
    SimpleLine *line = new SimpleLine ( 0 );
    pNetwork->GetLineValueOfRouteInterval ( this, line );
    if ( !line->IsEmpty() )
    {
      Rectangle<2> res = line->BoundingBox();
      line->DeleteIfAllowed();
      return res;
    }
    else
    {
      line->DeleteIfAllowed();
      Point *p1 = ( GPoint ( true, pNetwork->GetId(), m_iRouteId,
                             m_dStart ) ).ToPoint ( pNetwork );
      Point *p2 = ( GPoint ( true, pNetwork->GetId(), m_iRouteId,
                             m_dEnd ) ).ToPoint ( pNetwork );
      Rectangle<2> bbox = Rectangle<2> ( true,
                                         min ( p1->GetX(), p2->GetX() ),
                                         max ( p1->GetX(), p2->GetX() ),
                                         min ( p1->GetY(), p2->GetY() ),
                                         max ( p1->GetY(), p2->GetY() ) );
      p1->DeleteIfAllowed();
      p2->DeleteIfAllowed();
      return bbox;
    }
  }
}

/*
~searchRouteInterval~

Method for binary search after a route interval in a sorted ~GLine~. O(log n).
Used for example by operator ~inside~.

*/

bool searchRouteInterval ( GPoint *pGPoint, GLine *&pGLine, size_t low,
                           size_t high )
{
  RouteInterval rI;
  if ( low <= high )
  {
    size_t mid = ( high + low ) / 2;
    int imid = mid;
    if ( ( imid < 0 ) || ( imid >= pGLine->NoOfComponents() ) )
    {
      return false;
    }
    else
    {
      pGLine->Get ( mid, rI );
      if ( rI.GetRouteId() < pGPoint->GetRouteId() )
      {
        return searchRouteInterval ( pGPoint, pGLine, mid+1, high );
      }
      else
      {
        if ( rI.GetRouteId() > pGPoint->GetRouteId() )
        {
          return searchRouteInterval ( pGPoint, pGLine, low, mid-1 );
        }
        else
        {
          if ( fabs ( pGPoint->GetPosition() - rI.GetStartPos() ) < 0.01 ||
                  fabs ( pGPoint->GetPosition() - rI.GetEndPos() ) < 0.01 )
          {
            return true;
          }
          else
          {
            if ( rI.GetStartPos() > pGPoint->GetPosition() )
            {
              return searchRouteInterval ( pGPoint, pGLine, low, mid-1 );
            }
            else
            {
              if ( rI.GetEndPos() < pGPoint->GetPosition() )
              {
                return searchRouteInterval ( pGPoint, pGLine, mid+1, high );
              }
              else
              {
                return true;
              }
            }
          }
        }
      }
    }
  }
  return false;
}

/*
Helper struct to store sections which have been visited before. Used in
shortest path implementation of Dijkstras Algorithm.

*/
struct SectTree
{

  SectTree() {};

  SectTree ( SectTreeEntry *nEntry, SectTree *l = 0, SectTree *r = 0 )
  {
    value = *nEntry;
    left = l;
    right = r;
  };

  ~SectTree() {};

  /*
  Inserts a section into the tree. If the section is already in the insert is
  ignored.

  */
  void Insert ( SectTreeEntry *nEntry )
  {
    if ( nEntry->secttid < value.secttid )
    {
      if ( right != 0 ) right->Insert ( nEntry );
      else right = new SectTree ( nEntry,0,0 );
    }
    else
    {
      if ( nEntry->secttid > value.secttid )
      {
        if ( left != 0 ) left->Insert ( nEntry );
        else left = new SectTree ( nEntry,0,0 );
      }
    }
  };

  /*
  Returns a pointer to the node with the given id or 0 if not found.

  */
  void Find ( TupleId n, SectTree *result, bool &found )
  {
    if ( n < value.secttid )
    {
      if ( right != 0 ) right->Find ( n, result, found );
      else
      {
        found = false;
        result = 0;
      }
    }
    else
    {
      if ( n > value.secttid )
      {
        if ( left != 0 ) left->Find ( n, result, found );
        else
        {
          found = false;
          result = 0;
        }
      }
      else
      {
        found = true;
        result = this;
      }
    }
  };

  void CheckSection ( Network *pNetwork, SectTreeEntry n, GPoints &result )
  {
    vector<DirectedSection> sectList;
    sectList.clear();
    SectTree *pSectTree = 0;
    if ( n.startbool || n.endbool )
    {
      if ( n.startbool )
      {
        pNetwork->GetAdjacentSections ( n.secttid, false, sectList );
        size_t j = 0;
        bool found = true;
        while ( j < sectList.size() && found )
        {
          DirectedSection actSection = sectList[j];
          Find ( actSection.GetSectionTid(), pSectTree, found );
        }
        if ( !found )
          result += GPoint ( true, pNetwork->GetId(), n.rid, n.start, None );
      }
      sectList.clear();
      if ( n.endbool )
      {
        pNetwork->GetAdjacentSections ( n.secttid, true, sectList );
        size_t j = 0;
        bool found = true;
        while ( j < sectList.size() && found )
        {
          DirectedSection actSection = sectList[j];
          Find ( actSection.GetSectionTid(), pSectTree, found );
        }
        if ( !found ) result+= GPoint ( true, pNetwork->GetId(), n.rid, n.end,
None );
      }
      sectList.clear();
    }
    else result += GPoint ( true, pNetwork->GetId(), n.rid, n.end, None );
  }

  void WriteResult ( Network* pNetwork, GPoints &result, SectTree &secTr )
  {
    if ( left != 0 ) left->WriteResult ( pNetwork, result, secTr );
    if ( right != 0 ) right ->WriteResult ( pNetwork, result, secTr );
    secTr.CheckSection ( pNetwork, value, result );
    //TODO: Remove duplicate GPoint from Result, which are caused by junction
    //problematic
  };

  void Remove()
  {
    if ( left != 0 ) left->Remove();
    if ( right != 0 ) right->Remove();
    delete this;
  };

  SectTreeEntry value;
  SectTree *left, *right;
};

/*
Almost similar to operator ~checkPoint~ but additional returning a difference
value if the point is not exactly on the ~sline~.

Used by operator ~point2gpoint~

*/

bool chkPoint ( SimpleLine *&route, Point point, bool startSmaller, double &pos,
                double &difference )
{
  bool result = false;
  HalfSegment hs;
  double k1, k2;
  Point left, right;
  for ( int i = 0; i < route->Size()-1; i++ )
  {
    route->Get ( i, hs );
    left = hs.GetLeftPoint();
    right = hs.GetRightPoint();
    Coord xl = left.GetX(),
          yl = left.GetY(),
          xr = right.GetX(),
          yr = right.GetY(),
          x = point.GetX(),
          y = point.GetY();
    if ( ( fabs ( x-xr ) < 0.01 && fabs ( y-yr ) < 0.01 ) ||
         ( fabs ( x-xl ) < 0.01 && fabs ( y-yl ) < 0.01 ) )
    {
      difference = 0.0;
      result = true;
    }
    else
    {
      if ( xl != xr && xl != x )
      {
        k1 = ( y - yl ) / ( x - xl );
        k2 = ( yr - yl ) / ( xr - xl );
        if ( ( fabs ( k1-k2 ) < 0.004 ) &&
             ( ( xl < xr &&
                  ( x > xl || fabs ( x-xl ) < 0.01 ) &&
                  ( x < xr || fabs ( x-xr ) < 0.01 ) ) ||
               ( xl > xr &&
                  ( x < xl || fabs ( x-xl ) <0.01 )  &&
                  ( x > xr || fabs ( x-xr ) < 0.01 ) ) ) &&
             ((( yl <= yr  &&
                   ( y > yl || fabs ( y-yl ) <0.01 ) &&
                   ( y < yr || fabs ( y-yr ) <0.01 ) ) ||
               ( yl > yr &&
                   ( y < yl || fabs ( y-yl ) <0.01 ) &&
                   ( y > yr || fabs ( y-yr ) <0.01 ) ) ) ))
        {
          difference = fabs ( k1-k2 );
          result = true;
        }
        else {result = false;}
      }
      else
      {
        if ( ( fabs ( xl - xr ) < 0.01 && fabs ( xl -x ) < 0.01 ) &&
            ( ( ( yl <= yr &&
                  ( yl < y || fabs (yl-y ) < 0.01 ) &&
                  ( y < yr ||fabs ( y-yr ) <0.01 ) ) ||
                ( yl > yr &&
                  ( yl > y || fabs ( yl-y ) < 0.01) &&
                  ( y > yr ||fabs ( y-yr ) <0.01 ) ) ) ))
        {
          difference = 0.0;
          result = true;
        }
        else {result = false;}
      }
    }
    if ( result )
    {
      LRS lrs;
      route->Get ( hs.attr.edgeno, lrs );
      route->Get ( lrs.hsPos, hs );
      pos = lrs.lrsPos + point.Distance ( hs.GetDomPoint() );
      if ( startSmaller != route->GetStartSmaller() )
        pos = route->Length() - pos;
      if ( fabs ( pos-0.0 ) < 0.01 )
        pos = 0.0;
      else if ( fabs ( pos-route->Length() ) <0.01 )
        pos = route->Length();
      return result;
    }
  }
  return result;
}

/*
Almost similar to operator ~chkPoint~ but allowing a greater difference if the
point is not exactly on the ~sline~.

Used by operator ~point2gpoint~

*/

bool chkPoint03 ( SimpleLine *&route, Point point, bool startSmaller,
                  double &pos, double &difference )
{
  bool result = false;
  HalfSegment hs;
  double k1, k2;
  Point left, right;
  for ( int i = 0; i < route->Size()-1; i++ )
  {
    route->Get ( i, hs );
    left = hs.GetLeftPoint();
    right = hs.GetRightPoint();
    Coord xl = left.GetX(),
          yl = left.GetY(),
          xr = right.GetX(),
          yr = right.GetY(),
          x = point.GetX(),
          y = point.GetY();
    if ( ( fabs ( x-xl ) < 0.01 && fabs ( y-yl ) < 0.01 ) ||
         ( fabs ( x-xr ) < 0.01 && fabs ( y-yr ) < 0.01 ) )
    {
      difference = 0.0;
      result = true;
    }
    else
    {
      if ( xl != xr && xl != x )
      {
        k1 = ( y - yl ) / ( x - xl );
        k2 = ( yr - yl ) / ( xr - xl );
        if ( ( fabs ( k1-k2 ) < 1.2 ) &&
             ( ( xl < xr &&
                  ( x > xl || fabs ( x-xl ) < 0.01 ) &&
                  ( x < xr || fabs ( x-xr ) < 0.01 ) ) ||
               ( xl > xr &&
                  ( x < xl || fabs ( x-xl ) < 0.01 )  &&
                  ( x > xr || fabs ( x-xr ) < 0.01 ) ) ) &&
             ( ( ( yl < yr || fabs ( yl-yr ) < 0.01 ) &&
                  ( y > yl || fabs (y-yl ) <0.01 ) &&
                  ( y < yr || fabs ( y-yr ) <0.01 ) ) ||
               ( yl > yr &&
                  ( y < yl || fabs ( y-yl ) <0.01 ) &&
                  ( y > yr || fabs ( y-yr ) <0.01 ) ) ) )
        {
          difference = fabs ( k1-k2 );
          result = true;
        }
        else {result = false;}
      }
      else
      {
        if ( ( fabs ( xl - xr ) < 0.01 && fabs ( xl -x ) < 0.01 ) &&
             ( ( ( yl < yr|| fabs ( yl-yr ) <0.01 ) &&
                    ( yl < y || fabs (yl-y ) <0.01 ) &&
                    ( y < yr ||fabs ( y-yr ) <0.01 ) ) ||
                 ( yl > yr  &&
                    ( yl > y || fabs ( yl-y ) < 0.01) &&
                    ( y > yr ||fabs ( y-yr ) <0.01 ) ) ) )
        {
          difference = 0.0;
          result = true;
        }
        else {result = false;}
      }
    }
    if ( result )
    {
      LRS lrs;
      route->Get ( hs.attr.edgeno, lrs );
      route->Get ( lrs.hsPos, hs );
      pos = lrs.lrsPos + point.Distance ( hs.GetDomPoint() );
      if ( startSmaller != route->GetStartSmaller() )
        pos = route->Length() - pos;
      if ( fabs ( pos-0.0 ) < 0.01 )
        pos = 0.0;
      else if ( fabs ( pos-route->Length() ) <0.01 )
        pos = route->Length();
      return result;
    }
  }
  return result;
}

bool lastchkPoint03 ( SimpleLine *&route, Point point, bool startSmaller,
                      double &pos, double &difference )
{
  bool result = false;
  HalfSegment hs;
  double k1, k2;
  Point left, right;
  for ( int i = 0; i < route->Size()-1; i++ )
  {
    route->Get ( i, hs );
    left = hs.GetLeftPoint();
    right = hs.GetRightPoint();
    Coord xl = left.GetX(),
          yl = left.GetY(),
          xr = right.GetX(),
          yr = right.GetY(),
          x = point.GetX(),
          y = point.GetY();
    if ( ( fabs ( x-xl ) < 0.01 && fabs ( y-yl ) < 0.01 ) ||
         ( fabs ( x-xr ) < 0.01 && fabs ( y-yr ) < 0.01 ) )
    {
      difference = 0.0;
      result = true;
    }
    else
    {
      if ( xl != xr && xl != x )
      {
        k1 = ( y - yl ) / ( x - xl );
        k2 = ( yr - yl ) / ( xr - xl );
        if ( ( ( xl < xr &&
                  ( x > xl || fabs ( x-xl ) < 0.1 ) &&
                  ( x < xr || fabs ( x-xr ) < 0.01 ) ) ||
               ( xl > xr &&
                  ( x < xl || fabs ( x-xl ) <0.01 ) &&
                  ( x > xr || fabs ( x-xr ) < 0.01) ) ) &&
             ( ( ( yl < yr || fabs ( yl-yr ) < 0.01 ) &&
                  ( y > yl || fabs (y-yl ) < 0.01 ) &&
                  ( y < yr || fabs ( y-yr ) < 0.01 ) ) ||
               ( yl > yr &&
                  ( y < yl || fabs ( y-yl ) <0.01 ) &&
                  ( y > yr || fabs ( y-yr ) <0.01 ) ) ) )
        {
          difference = fabs ( k1-k2 );
          result = true;
        }
        else {result = false;}
      }
      else
      {
        if ( ( fabs ( xl - xr ) < 0.01 && fabs ( xl -x ) < 0.01 ) &&
             ( ( ( yl < yr|| fabs ( yl-yr ) <0.01 ) &&
                    ( yl < y || fabs ( yl-y ) <0.01 ) &&
                    ( y < yr ||fabs ( y-yr ) <0.01 ) ) ||
                 ( yl > yr &&
                    ( yl > y || fabs ( yl-y ) < 0.01 ) &&
                    ( y > yr ||fabs ( y-yr ) <0.01 ) ) ) )
        {
          difference = 0.0;
          result = true;
        }
        else {result = false;}
      }
    }
    if ( result )
    {
      LRS lrs;
      route->Get ( hs.attr.edgeno, lrs );
      route->Get ( lrs.hsPos, hs );
      pos = lrs.lrsPos + point.Distance ( hs.GetDomPoint() );
      if ( startSmaller != route->GetStartSmaller() )
        pos = route->Length() - pos;
      if ( pos < 0.0 || fabs ( pos - 0.0 ) < 0.01 )
        pos = 0.0;
      else if ( pos > route->Length() || fabs ( pos - route->Length() ) < 0.01 )
        pos = route->Length();
      return result;
    }
  }
  return result;
}

/*
Returns true if a ~point~ is part of a ~sline~, false elsewhere. If the point
is part of the sline his distance from the start is computed also. Used by
operator ~line2gline~.

*/
bool checkPoint ( SimpleLine *&route, Point point, bool startSmaller,
                  double &pos )
{
  bool result = false;
  HalfSegment hs;
  double k1, k2;
  Point left, right;
  for ( int i = 0; i < route->Size()-1; i++ )
  {
    route->Get ( i, hs );
    left = hs.GetLeftPoint();
    right = hs.GetRightPoint();
    Coord xl = left.GetX(),
          yl = left.GetY(),
          xr = right.GetX(),
          yr = right.GetY(),
          x = point.GetX(),
          y = point.GetY();
    if ( ( fabs ( x-xr ) < 0.01 && fabs ( y-yr ) < 0.01 ) ||
         ( fabs ( x-xl ) < 0.01 && fabs ( y-yl ) < 0.01 ) )
    {
      result = true;
    }
    else
    {
      if ( xl != xr && xl != x )
      {
        k1 = ( y - yl ) / ( x - xl );
        k2 = ( yr - yl ) / ( xr - xl );
        if ( ( fabs ( k1-k2 ) < 0.004 ) &&
             ( ( xl < xr &&
                  ( x > xl || fabs ( x-xl ) < 0.01 ) &&
                  ( x < xr || fabs ( x-xr ) < 0.01 ) ) ||
               ( xl > xr &&
                  ( x < xl || fabs ( x-xl ) < 0.01 )  &&
                  ( x > xr || fabs ( x-xr ) < 0.01 ) ) ) &&
             ( ( ( yl < yr || fabs ( yl-yr ) < 0.01 ) &&
                  ( y > yl || fabs ( y-yl ) < 0.01 ) &&
                  ( y < yr || fabs ( y-yr ) < 0.01 ) ) ||
               ( yl > yr &&
                  ( y < yl || fabs ( y-yl ) < 0.01 ) &&
                  ( y > yr || fabs ( y-yr ) < 0.01 ) ) ) )
        {
          result = true;
        }
        else {result = false;}
      }
      else
      {
        if ( ( fabs ( xl - xr ) < 0.01 && fabs ( xl -x ) < 0.01 ) &&
             ( ( ( yl < yr|| fabs ( yl-yr ) < 0.01 ) &&
                    ( yl < y || fabs ( yl-y ) <0.01 ) &&
                    ( y < yr ||fabs ( y-yr ) <0.01 ) ) ||
                 ( yl > yr &&
                    ( yl > y || fabs ( yl-y ) < 0.01) &&
                    ( y > yr ||fabs ( y-yr ) <0.01 ) ) ) )
        {
          result = true;
        }
        else {result = false;}
      }
    }
    if ( result )
    {
      LRS lrs;
      route->Get ( hs.attr.edgeno, lrs );
      route->Get ( lrs.hsPos, hs );
      pos = lrs.lrsPos + point.Distance ( hs.GetDomPoint() );
      if ( startSmaller != route->GetStartSmaller() )
        pos = route->Length() - pos;
      if ( fabs ( pos-0.0 ) < 0.01 )
        pos = 0.0;
      else if ( fabs ( pos-route->Length() ) <0.01 )
        pos = route->Length();
      return result;

    }
  }
  return result;
};

/*
Precondition: ~GLine~ is sorted.

Returns true if there is a ~RouteInterval~ in the sorted ~GLine~ which
intersects with the given ~RouteInterval~ false elsewhere.

Used by operator ~intersects~

*/

bool searchUnit ( GLine *pGLine, size_t low, size_t high,
                  RouteInterval pRi )
{
  assert ( pGLine->IsSorted() );
  RouteInterval rI;
  if ( low <= high )
  {
    size_t mid = ( high + low ) / 2;
    int imid = mid;
    if ( ( mid < 0 ) || ( imid >= pGLine->NoOfComponents() ) )
    {
      return false;
    }
    else
    {
      pGLine->Get ( mid, rI );
      if ( rI.GetRouteId() < pRi.GetRouteId() )
      {
        return searchUnit ( pGLine, mid+1, high, pRi );
      }
      else
      {
        if ( rI.GetRouteId() > pRi.GetRouteId() )
        {
          return searchUnit ( pGLine, low, mid-1, pRi );
        }
        else
        {
          if ( rI.GetStartPos() > pRi.GetEndPos() )
          {
            return searchUnit ( pGLine, low, mid-1, pRi );
          }
          else
          {
            if ( rI.GetEndPos() < pRi.GetStartPos() )
            {
              return searchUnit ( pGLine, mid+1, high, pRi );
            }
            else
            {
              return true;
            }
          }
        }
      }
    }
  }
  else
  {
    return false;
  }
  return false;
}


/*
class ~GPointList~

Used by the operator ~polygpoint~. Computes and stores the resulting ~GPoints~
for the resulting ~stream~ of ~GPoint~.

*/

class GPointList
{
  public:
    /*
    The constructor creates a GPointList from a given gpoint.

    */
    GPointList ( GPoint *gp, Network *pNetwork ) :
        aliasGP ( 0 )
    {
      lastPos = 0;
      aliasGP.clean();
      aliasGP.Append ( *gp );
      vector<JunctionSortEntry> xJunctions;
      xJunctions.clear();
      if ( pNetwork != 0 )
      {
        CcInt iRouteId ( true, gp->GetRouteId() );
        pNetwork->GetJunctionsOnRoute ( &iRouteId, xJunctions );
        bool found = false;
        JunctionSortEntry pCurrJunction;
        size_t i = 0;
        while ( !found && i < xJunctions.size() )
        {
          pCurrJunction = xJunctions[i];
          if ( fabs ( pCurrJunction.GetRouteMeas() - gp->GetPosition() ) < 0.01
)
          {
            found = true;
            GPoint test ( true, gp->GetNetworkId(),
                          pCurrJunction.GetOtherRouteId(),
                          pCurrJunction.GetOtherRouteMeas(),
                          None );
            aliasGP.Append ( test );
          }
          i++;
        }
        while ( found && i < xJunctions.size() )
        {
          pCurrJunction = xJunctions[i];
          if ( fabs ( pCurrJunction.GetRouteMeas() - gp->GetPosition() ) <0.01 )
          {
            GPoint test ( true, gp->GetNetworkId(),
                          pCurrJunction.GetOtherRouteId(),
                          pCurrJunction.GetOtherRouteMeas(),
                          None );
            aliasGP.Append ( test );
          }
          else
          {
            found = false;
          }
          i++;
        }
        for ( unsigned int i=0;i<xJunctions.size();i++ )
        {
          xJunctions[i].m_pJunction->DeleteIfAllowed();
        }
        xJunctions.clear();
      }
    }

    ~GPointList() {}

    /*
    ~NextGPoint~

    This function returns the next GPoint from the GPointList.
    If there is no more GPoint in the  List the result will be
    0. This function creates a new GPoint instance via the new
    operator. The caller of this function has to ensure the
    deletion of this object.

    */
    const GPoint NextGPoint()
    {
      GPoint pAktGPoint;
      if ( lastPos >= aliasGP.Size() || lastPos < 0 )
      {
        pAktGPoint.SetDefined ( false );
      }
      else
      {
        aliasGP.Get ( lastPos, pAktGPoint );
        lastPos++;
      }
      return pAktGPoint;
    }

  private:

    DbArray<GPoint> aliasGP;
    int lastPos;

};

/*
class ~RectangleList~

Almost similar to ~GPointList~. But storing rectangles representing route
intervals of a ~gline~. Used by operator ~routeintervals~ to create a ~stream~
of ~rectangles~.

*/

class RectangleList
{

  public:

    /*
    ~Constructor~

    Creates a RectangleList from a given gline.

    */

    RectangleList ( GLine *gl ) :
        aliasRectangleList ( 0 )
    {
      RouteInterval ri;
      lastPos = 0;
      aliasRectangleList.clean();
      for ( int i = 0 ; i < gl->NoOfComponents(); i++ )
      {
        gl->Get ( i, ri );
        Rectangle<2> elem ( true,
                            ( double ) ri.GetRouteId(),
                            ( double ) ri.GetRouteId(),
                            min ( ri.GetStartPos(),ri.GetEndPos() ),
                            max ( ri.GetStartPos(), ri.GetEndPos() ) );
        aliasRectangleList.Append ( elem );
      }
    }

    ~RectangleList() {}

    /*
    ~NextRectangle~

    This function returns the next rectangle from the RectangleList.
    If there is no more route interval in the  List the result will be
    0. This function creates a new Rectangle instance via the new
    operator. The caller of this function has to ensure the
    deletion of this object.

    */
    const Rectangle<2> NextRectangle()
    {
      Rectangle<2> pAktRectangle;
      if ( lastPos >= aliasRectangleList.Size() || lastPos < 0 )
      {
        pAktRectangle.SetDefined ( false );
      }
      else
      {
        aliasRectangleList.Get ( lastPos, pAktRectangle );
        lastPos++;
      }
      return pAktRectangle;
    }

  private:

    DbArray<Rectangle<2> > aliasRectangleList;
    int lastPos;

};

/*
~struct RIStack~

Used to build compressed shortestpath ~gline~ values.
The ~RouteIntervals~ of the shortest path computation are pushed to a stack to
first the end section. Last the first section of the path. So they can be
returned in the sequence they are needed in the path.

When the stack is returned to the resulting ~gline~ it is checked if the
~RouteInterval~s on the stack can be connected to bigger parts. This is always
possible if the shortest path follows the same route for more than one section.

*/

struct RIStack
{

  RIStack() {};

  RIStack ( int ri,double pos1, double pos2, RIStack* next = 0 )
  {
    m_iRouteId = ri;
    m_dStart = pos1;
    m_dEnd = pos2;
    m_next = next;
  };

  ~RIStack() {};

  void Push ( int rid, double pos1, double pos2, RIStack *&first )
  {
    RIStack *actElem = new RIStack ( rid, pos1, pos2, this );
    first = actElem;
  };

  void StackToGLine ( GLine *gline )
  {
    int actRId = m_iRouteId;
    double actStartPos = m_dStart;
    double actEndPos = m_dEnd;
    RIStack *actElem = this->m_next;
    while ( actElem != 0 )
    {
      if ( actRId == actElem->m_iRouteId &&
              AlmostEqual ( actEndPos,actElem->m_dStart ) )
      {
        actEndPos = actElem->m_dEnd;
      }
      else
      {
        gline->AddRouteInterval ( actRId, actStartPos, actEndPos );
        actRId = actElem->m_iRouteId;
        actStartPos = actElem->m_dStart;
        actEndPos = actElem->m_dEnd;
      }
      actElem = actElem->m_next;
    }
    gline->AddRouteInterval ( actRId, actStartPos, actEndPos );
  };

  void RemoveStack()
  {
    if ( m_next != 0 ) m_next->RemoveStack();
    delete this;
  };

  int m_iRouteId;
  double m_dStart, m_dEnd;
  RIStack *m_next;
};

/*
Class PQEntry used for priority Queue in Dijkstras Algorithm for shortest path
computing between two gpoint.

*/

class PQEntry
{
  public:
    PQEntry() {}

    PQEntry ( TupleId aktID, double distance, bool upDown,
              TupleId beforeID )
    {
      sectID = aktID;
      distFromStart = distance;
      upDownFlag = upDown;
      beforeSectID = beforeID;
      meas1 =0;
      meas2 =0;
      rid = 0;
      costFromStart = 0.0;
    }
    PQEntry(TupleId in_aktID, double in_distance, bool in_upDown,
            TupleId in_beforeID, double in_meas1, double in_meas2,
            int in_rid, double in_costs=0.0 ):
      sectID(in_aktID),
      distFromStart(in_distance),
      upDownFlag(in_upDown),
      beforeSectID(in_beforeID),
      meas1(in_meas1),
      meas2(in_meas2),
      rid(in_rid),
      costFromStart(in_costs)
    {
    }

    PQEntry ( PQEntry &e )
    {
      sectID = e.sectID;
      distFromStart = e.distFromStart;
      upDownFlag = e.upDownFlag;
      beforeSectID = e.beforeSectID;
      meas1 = e.meas1;
      meas2 = e.meas2;
      rid = e.rid;
      costFromStart = e.costFromStart;
    }
    PQEntry ( const PQEntry &e )
    {
      sectID = e.sectID;
      distFromStart = e.distFromStart;
      upDownFlag = e.upDownFlag;
      beforeSectID = e.beforeSectID;
      meas1 = e.meas1;
      meas2 = e.meas2;
      rid = e.rid;
      costFromStart = e.costFromStart;
    }

    ~PQEntry() {}

    TupleId sectID;
    double distFromStart;
    bool upDownFlag;
    TupleId beforeSectID;
    double meas1;
    double meas2;
    int rid;
    double costFromStart;
};

/*
struct sectIDTree stores the PQEntrys by section ID with identification flag
for the PrioQueue-Array.
An arrayIndex from max integer means not longer in PrioQ and already visited.

*/

struct SectIDTree
{
  SectIDTree() {};

  SectIDTree ( TupleId sectIdent, TupleId beforeSectIdent, bool upDown,
               int arrayIndex,
               SectIDTree *l = 0,SectIDTree *r = 0 )
  {
    sectID = sectIdent;
    beforeSectId = beforeSectIdent;
    upDownFlag = upDown;
    index = arrayIndex;
    left = l;
    right = r;

  };

  ~SectIDTree() {};

  SectIDTree* Find ( TupleId sectIdent )
  {
    if ( sectID > sectIdent )
    {
      if ( left != 0 ) return left->Find ( sectIdent );
      else
      {
        return this;
      }
    }
    else
    {
      if ( sectID < sectIdent )
      {
        if ( right != 0 ) return right->Find ( sectIdent );
        else
        {
          return this;
        }
      }
      else
      {
        return this;
      }
    }
  };

  void Remove()
  {
    if ( left != 0 ) left->Remove();
    if ( right != 0 ) right->Remove();
    delete this;
  };

  bool Insert ( TupleId sectIdent, TupleId beforeSectIdent, bool upDownFlag,
                int arrayIndex,SectIDTree *&pointer )
  {
    pointer = Find ( sectIdent );
    if ( pointer->sectID != sectIdent )
    {
      if ( pointer->sectID > sectIdent )
      {
        pointer->left = new SectIDTree ( sectIdent, beforeSectIdent, upDownFlag,
                                         arrayIndex );
        pointer = pointer->left;
        return true;
      }
      else
      {
        if ( pointer->sectID < sectIdent )
        {
          pointer->right = new SectIDTree ( sectIdent, beforeSectIdent,
                                            upDownFlag, arrayIndex );
          pointer = pointer->right;
          return true;
        }
        else
        {
          //should never be reached
          return false;
        }
      }
    }
    else
    {
      return false;
    }
  };

  void SetIndex ( TupleId sectIdent, int arrayIndex )
  {
    Find ( sectIdent )->index = arrayIndex;
  };

  void SetIndex ( int arrayIndex )
  {
    index = arrayIndex;
  };

  int GetIndex ( TupleId sectIdent )
  {
    return Find ( sectIdent )->index;
  };

  void SetBeforeSectId ( TupleId sectIdent, TupleId before )
  {
    Find ( sectIdent )->beforeSectId = before;
  };

  void SetBeforeSectId ( TupleId before )
  {
    beforeSectId = before;
  };

  TupleId sectID;
  TupleId beforeSectId;
  bool upDownFlag;
  int index;
  SectIDTree *left, *right;
};

/*
struct Priority Queue for Dijkstras Algorithm of shortest path computing between
two gpoint.

*/

struct PrioQueue
{

  PrioQueue() {};

  PrioQueue ( int n ) : prioQ ( 0 ) {firstFree = 0;};

  ~PrioQueue() {};

  /*
  If a point is reached second time and the distance of the second way is
  smaller than on the path he has been found before. Its distance value and its
  position in the  priority queue must be corrected.

  */
  void CorrectPosition ( int checkX, const PQEntry nElem, SectIDTree *pSection,
                         SectIDTree *sectTree, bool calcCost=false )
  {
    int act = checkX;
    PQEntry test;
    bool found = false;
    while ( checkX >= 0 && !found )
    {
      if ( ( act % 2 ) == 0 ) checkX = ( act-2 ) / 2;
      else checkX = ( act -1 ) / 2;
      if ( checkX >= 0 )
      {
        prioQ.Get ( checkX, test );
        if(calcCost){
            if ( test.costFromStart > nElem.costFromStart )
            {
              PQEntry help = test;
              prioQ.Put ( checkX, nElem );
              pSection->SetIndex ( checkX );
              prioQ.Put ( act, help );
              SectIDTree *thelp = sectTree->Find ( help.sectID );
              thelp->SetIndex ( act );
              act = checkX;
            }
            else
            {
              found = true;
            }
        }
        else {
            if ( test.distFromStart > nElem.distFromStart )
            {
              PQEntry help = test;
              prioQ.Put ( checkX, nElem );
              pSection->SetIndex ( checkX );
              prioQ.Put ( act, help );
              SectIDTree *thelp = sectTree->Find ( help.sectID );
              thelp->SetIndex ( act );
              act = checkX;
            }
            else
            {
              found = true;
            }
        }
      }
      else
      {
        found = true;
      }
    }
  };

  void Insert ( PQEntry nElem, SectIDTree *sectTree, bool calcCost=false )
  {
    SectIDTree *pSection = sectTree->Find ( nElem.sectID );
    PQEntry old;
    if ( pSection->sectID == nElem.sectID )
    {
      if ( pSection->index != numeric_limits<int>::max() )
      {
        prioQ.Get ( pSection->index, old );
        if(calcCost)
        {
            if ( nElem.costFromStart < old.costFromStart )
            {
              prioQ.Put ( pSection->index, nElem );
              pSection->SetBeforeSectId ( nElem.beforeSectID );
              CorrectPosition( pSection->index, nElem, pSection, sectTree,true);
            }
        }
        else
        {
            if ( nElem.distFromStart < old.distFromStart )
            {
              prioQ.Put ( pSection->index, nElem );
              pSection->SetBeforeSectId ( nElem.beforeSectID );
              CorrectPosition(pSection->index, nElem, pSection, sectTree,false);
            }
        }
      }
    }
    else
    {
      prioQ.Put ( firstFree, nElem );
      sectTree->Insert ( nElem.sectID, nElem.beforeSectID, nElem.upDownFlag,
                         firstFree, pSection );
      CorrectPosition ( firstFree, nElem, pSection ,sectTree,calcCost );
      firstFree++;
    }
  }


  PQEntry* GetAndDeleteMin ( SectIDTree *sectTree, bool calcCost=false)
  {
    if ( firstFree > 0 )
    {
      PQEntry result, last, test1, test2;
      prioQ.Get ( 0,result );
      PQEntry *retValue = new PQEntry ( result.sectID,
                                        result.distFromStart,
                                        result.upDownFlag,
                                        result.beforeSectID,
                                        result.meas1,
                                        result.meas2,
                                        result.rid,
                                        result.costFromStart);
      SectIDTree *tRet = sectTree->Find ( result.sectID );
      tRet->SetIndex ( numeric_limits<int>::max() );
      prioQ.Get ( firstFree-1, last );
      prioQ.Put ( 0, last );
      firstFree = firstFree--;
      SectIDTree *pSection = sectTree->Find ( last.sectID );
      pSection->SetIndex ( 0 );
      int act = 0;
      int checkX = 0;
      bool found = false;
      while ( checkX < firstFree && !found )
      {
        checkX = 2*act + 1;
        if ( checkX < firstFree-1 )
        {
          prioQ.Get ( checkX, test1 );
          prioQ.Get ( checkX+1, test2 );
          if(calcCost)
          {
              if ( test1.costFromStart < last.costFromStart ||
                      test2.costFromStart < last.costFromStart )
              {
                if ( test1.costFromStart <= test2.costFromStart )
                {
                  PQEntry help = test1;
                  prioQ.Put ( checkX, last );
                  pSection->SetIndex ( checkX );
                  prioQ.Put ( act, help );
                  SectIDTree *thelp = sectTree->Find ( help.sectID );
                  thelp->SetIndex ( act );
                  act = checkX;
                }
                else
                {
                  PQEntry help = test2;
                  prioQ.Put ( checkX+1, last );
                  pSection->SetIndex ( checkX+1 );
                  prioQ.Put ( act, help );
                  SectIDTree *thelp = sectTree->Find ( help.sectID );
                  thelp->SetIndex ( act );
                  act = checkX+1;
                }
              }
              else
              {
                found = true;
              }
          }
          else
          {
              if ( test1.distFromStart < last.distFromStart ||
                      test2.distFromStart < last.distFromStart )
              {
                if ( test1.distFromStart <= test2.distFromStart )
                {
                  PQEntry help = test1;
                  prioQ.Put ( checkX, last );
                  pSection->SetIndex ( checkX );
                  prioQ.Put ( act, help );
                  SectIDTree *thelp = sectTree->Find ( help.sectID );
                  thelp->SetIndex ( act );
                  act = checkX;
                }
                else
                {
                  PQEntry help = test2;
                  prioQ.Put ( checkX+1, last );
                  pSection->SetIndex ( checkX+1 );
                  prioQ.Put ( act, help );
                  SectIDTree *thelp = sectTree->Find ( help.sectID );
                  thelp->SetIndex ( act );
                  act = checkX+1;
                }
              }
              else
              {
                found = true;
              }
          }
        }
        else
        {
          if ( checkX != 0 && checkX == firstFree-1 )
          {
            prioQ.Get ( checkX, test1 );
            if(calcCost){
                if ( test1.costFromStart < last.costFromStart )
                {
                  PQEntry help = test1;
                  prioQ.Put ( checkX, last );
                  pSection->SetIndex ( checkX );
                  prioQ.Put ( act, help );
                  SectIDTree *thelp = sectTree->Find ( help.sectID );
                  thelp->SetIndex ( act );
                  act = checkX;
                }
                else
                {
                  found = true;
                }
            }
            else
            {
                if ( test1.distFromStart < last.distFromStart )
                {
                  PQEntry help = test1;
                  prioQ.Put ( checkX, last );
                  pSection->SetIndex ( checkX );
                  prioQ.Put ( act, help );
                  SectIDTree *thelp = sectTree->Find ( help.sectID );
                  thelp->SetIndex ( act );
                  act = checkX;
                }
                else
                {
                  found = true;
                }
            }
          }
        }
      }
      return retValue;
    }
    else
    {
      return 0;
    }
  }

  void Clear()
  {
    prioQ.clean();
    firstFree = 0;
  }

  bool IsEmpty()
  {
    if ( firstFree == 0 ) return true;
    else return false;
  }

  void Destroy()
  {
    prioQ.Destroy();
  }

  DbArray<PQEntry> prioQ;
  int firstFree;

};

/*
3 Class Definitions

3.1 class ~Network~

3.1.1 Network relations

*/
string Network::routesTypeInfo =
    "(rel (tuple ((Id int) (Length real) (Curve sline) "
    "(Dual bool) (StartsSmaller bool))))";

string Network::routesInternalTypeInfo =
    "(rel (tuple ((Id int) (Length real) (Curve sline) "
    "(Dual bool) (StartsSmaller bool) (Startposid int))))";

string Network::routesBTreeTypeInfo =
    "(btree (tuple ((Id int) (Length real) (Curve sline) "
    "(Dual bool) (StartsSmaller bool) (Startposid int) )) int)";

string Network::routesRTreeTypeInfo =
    "(rtree (tuple((Id int)(Length real)(Curve sline)(Dual bool)"
    "(StartsSmaller bool) (Startposid int) )) sline FALSE)";

string Network::junctionsTypeInfo =
    "(rel (tuple ((R1id int) (Meas1 real) (R2id int) "
    "(Meas2 real) (Cc int))))";

/*string Network::junctionsInternalTypeInfo =
    "(rel (tuple ((r1id int) (meas1 real) (r2id int) "
    "(meas2 real) (cc int) (pos point) (r1rc tid) (r2rc tid) "
    "(sauprc tid) (sadownrc tid)(sbuprc tid) (sbdownrc tid))))";*/
string Network::junctionsInternalTypeInfo =
    "(rel (tuple ((R1id int) (Meas1 real) (R2id int) "
    "(Meas2 real) (Cc int) (Loc point) (R1rc tid) (R2rc tid) "
    "(Sauprc tid) (Sadownrc tid)(Sbuprc tid) (Sbdownrc tid))))";

string Network::junctionsBTreeTypeInfo =
    "(btree (tuple ((R1id int) (Meas1 real) (R2id int) "
    "(Meas2 real) (Cc int) (Loc point) (R1rc tid) (R2rc tid) "
    "(Sauprc tid) (Sadownrc tid)(Sbuprc tid) (Sbdownrc tid))) int)";
string Network::sectionsInternalTypeInfo =
    "(rel (tuple ((Rid int) (Meas1 real) (Meas2 real) (Dual bool)"
    "(Curve sline)(CurveStartsSmaller bool) (Rrc tid) (Sid int) "
    "(Pnoup int) (Pnodown int) (Slotup int) (Slotdown int) "
    "(Cost real) (Time real))))";
string Network::sectionsBTreeTypeInfo =
    "(btree (tuple ((Rid int) (Meas1 real) (Meas2 real) (Dual bool)"
    "(Curve sline)(CurveStartsSmaller bool) (Rrc tid) (Sid int) "
    "(Pnoup int) (Pnodown int) (Slotup int) (Slotdown int) "
    "(Cost real) (Time real) )) int)";
string Network::distancestorageTypeInfo =
    "(rel (tuple((J1 tid)(J2 tid)(Dist real)(Sp gline))))";

/*
2.1.2 Constructors and destructors class ~Network~

*/

Network::Network() :
    m_iId ( 0 ),
    m_bDefined ( false ),
    m_pRoutes ( 0 ),
    m_pJunctions ( 0 ),
    m_pSections ( 0 ),
    m_pBTreeRoutes ( 0 ),
    m_pBTreeRoutesByStartposId( 0 ),
    m_pRTreeRoutes ( 0 ),
    m_pBTreeJunctionsByRoute1 ( 0 ),
    m_pBTreeJunctionsByRoute2 ( 0 ),
    m_xAdjacencyList ( 0 ),
    m_xSubAdjacencyList ( 0 ),
    m_pBTreeSectionsByRoute ( 0 ),
    m_pBTreeSections (0)
    /*alldistance(0)*/  //only for experimental use with network distances
{
}

Network::Network ( SmiRecord& in_xValueRecord,
                   size_t& inout_iOffset,
                   const ListExpr in_xTypeInfo ):
m_xAdjacencyList(0),
m_xSubAdjacencyList(0)
{

  // Read network id
  in_xValueRecord.Read ( &m_iId, sizeof ( int ), inout_iOffset );
  inout_iOffset += sizeof ( int );

  // Open routes
  ListExpr xType;
  nl->ReadFromString ( routesInternalTypeInfo, xType );
  ListExpr xNumericType = SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pRoutes = Relation::Open ( in_xValueRecord,
                               inout_iOffset,
                               xNumericType );
  if ( !m_pRoutes )
  {
    return;
  }

  // Open junctions
  nl->ReadFromString ( junctionsInternalTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pJunctions = Relation::Open ( in_xValueRecord,
                                  inout_iOffset,
                                  xNumericType );
  if ( !m_pJunctions )
  {
    m_pRoutes->Delete();
    return;
  }

  // Open sections
  nl->ReadFromString ( sectionsInternalTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pSections = Relation::Open ( in_xValueRecord,
                                 inout_iOffset,
                                 xNumericType );
  if ( !m_pSections )
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    return;
  }

  // Open btree for routes
  nl->ReadFromString ( routesBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pBTreeRoutes = BTree::Open ( in_xValueRecord,
                                 inout_iOffset,
                                 xNumericType );

  if ( !m_pBTreeRoutes )
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    return;
  }
  // Open btree for routesByStartposId
  nl->ReadFromString ( routesBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pBTreeRoutesByStartposId = BTree::Open ( in_xValueRecord,
                                 inout_iOffset,
                                 xNumericType );

  if ( !m_pBTreeRoutesByStartposId )
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    return;
  }

  //Open  rtree for routes
  Word xValue;

  if ( ! ( m_pRTreeRoutes->Open ( in_xValueRecord,
                                  inout_iOffset,
                                  routesRTreeTypeInfo,
                                  xValue ) ) )
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    delete m_pBTreeRoutesByStartposId;
    return;
  }

  m_pRTreeRoutes = ( R_Tree<2,TupleId>* ) xValue.addr;

  // Open first btree for junctions
  nl->ReadFromString ( junctionsBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pBTreeJunctionsByRoute1 = BTree::Open ( in_xValueRecord,
                              inout_iOffset,
                              xNumericType );
  if ( !m_pBTreeJunctionsByRoute1 )
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    delete m_pBTreeRoutesByStartposId;
    delete m_pRTreeRoutes;
    return;
  }

  // Open second btree for junctions
  nl->ReadFromString ( junctionsBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pBTreeJunctionsByRoute2 = BTree::Open ( in_xValueRecord,
                              inout_iOffset,
                              xNumericType );
  if ( !m_pBTreeJunctionsByRoute2 )
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    delete m_pBTreeRoutesByStartposId;
    delete m_pRTreeRoutes;
    delete m_pBTreeJunctionsByRoute1;
    return;
  }

// m_xAdjacencyList.OpenFromRecord(in_xValueRecord, inout_iOffset);
// m_xSubAdjacencyList.OpenFromRecord(in_xValueRecord,inout_iOffset);
   size_t bufsize = sizeof(FlobId) + sizeof(SmiSize) + 2*sizeof(int);
   SmiSize offset = 0;
   char* buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   assert(buf != NULL);
   m_xAdjacencyList.restoreHeader(buf,offset);
   free(buf);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   assert(buf != NULL);
   m_xSubAdjacencyList.restoreHeader(buf,offset);
   inout_iOffset += bufsize;
   free(buf);

  // Open btree for sections by Routes
  nl->ReadFromString ( sectionsBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pBTreeSectionsByRoute = BTree::Open ( in_xValueRecord,
                                          inout_iOffset,
                                          xNumericType );
  if ( !m_pBTreeSectionsByRoute )
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    delete m_pBTreeRoutesByStartposId;
    delete m_pRTreeRoutes;
    delete m_pBTreeJunctionsByRoute1;
    delete m_pBTreeJunctionsByRoute2;
    return;
  }

  // Open btree for sections
  nl->ReadFromString ( sectionsBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pBTreeSections = BTree::Open ( in_xValueRecord,
                                          inout_iOffset,
                                          xNumericType );
  if ( !m_pBTreeSections )
  {
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    delete m_pBTreeRoutesByStartposId;
    delete m_pRTreeRoutes;
    delete m_pBTreeJunctionsByRoute1;
    delete m_pBTreeJunctionsByRoute2;
    delete m_pBTreeSectionsByRoute;
    return;
  }

  //Open distance storage
  /*
  nl->ReadFromString(distancestorageTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  alldistance = Relation::Open(in_xValueRecord,inout_iOffset,xNumericType);

  if(!alldistance){
    m_pRoutes->Delete();
    m_pJunctions->Delete();
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    delete m_pRTreeRoutes;
    delete m_pBTreeJunctionsByRoute1;
    delete m_pBTreeJunctionsByRoute2;
    return;
  }
  */
  m_bDefined = true;
}

Network::Network ( ListExpr in_xValue,
                   int in_iErrorPos,
                   ListExpr& inout_xErrorInfo,
                   bool& inout_bCorrect ) :
    m_iId ( 0 ),
    m_bDefined ( false ),
    m_pRoutes ( 0 ),
    m_pJunctions ( 0 ),
    m_pSections ( 0 ),
    m_pBTreeRoutes ( 0 ),
    m_pBTreeRoutesByStartposId ( 0 ),
    m_pRTreeRoutes ( 0 ),
    m_pBTreeJunctionsByRoute1 ( 0 ),
    m_pBTreeJunctionsByRoute2 ( 0 ),
    m_xAdjacencyList ( 0 ),
    m_xSubAdjacencyList ( 0 ),
    m_pBTreeSectionsByRoute ( 0 ),
    m_pBTreeSections (0)
    /*alldistance(0)*/
{

  // Check the list
  if ( ! ( nl->ListLength ( in_xValue ) == 3 ) )
  {
    string strErrorMessage = "Network(): List length must be 3.";
    inout_xErrorInfo =
        nl->Append ( inout_xErrorInfo, nl->StringAtom ( strErrorMessage ) );
    inout_bCorrect = false;
    return;
  }

  // Get type-info for temporary table
  ListExpr xType;
  nl->ReadFromString ( routesTypeInfo, xType );
  ListExpr xRoutesNumType = SecondoSystem::GetCatalog()->NumericType ( xType );
  nl->ReadFromString ( junctionsTypeInfo, xType );
  ListExpr xJunctionsNumType = SecondoSystem::GetCatalog()->NumericType ( xType
);

  // Split into the three parts
  ListExpr xIdList = nl->First ( in_xValue );
  ListExpr xRouteList = nl->Second ( in_xValue );
  ListExpr xJunctionList = nl->Third ( in_xValue );
  // Sections will be calculated in the load-method

  // Read Id
  if ( !nl->IsAtom ( xIdList ) ||
          nl->AtomType ( xIdList ) != IntType )
  {
    string strErrorMessage = "Network(): Id is missing.";
    inout_xErrorInfo = nl->Append ( inout_xErrorInfo,
                                    nl->StringAtom ( strErrorMessage ) );
    inout_bCorrect = false;
    return;
  }
  m_iId = nl->IntValue ( xIdList );

  // Create new temporary relations.
  Relation* pRoutes = new Relation ( xRoutesNumType, false );
  Relation* pJunctions = new Relation ( xJunctionsNumType, false );

  // Iterate over all routes
  while ( !nl->IsEmpty ( xRouteList ) )
  {
    ListExpr xCurrentRoute = nl->First ( xRouteList );
    xRouteList = nl->Rest ( xRouteList );

    // Create tuple for internal table
    Tuple* pNewRoute = new Tuple ( nl->Second ( xRoutesNumType ) );

    // Check this part of the list
    if ( nl->ListLength ( xCurrentRoute ) != 5 ||
            ( !nl->IsAtom ( nl->First ( xCurrentRoute ) ) ) ||
            nl->AtomType ( nl->First ( xCurrentRoute ) ) != IntType ||
            ( !nl->IsAtom ( nl->Second ( xCurrentRoute ) ) ) ||
            nl->AtomType ( nl->Second ( xCurrentRoute ) ) != RealType ||
            ( nl->IsAtom ( nl->Third ( xCurrentRoute ) ) ) ||
            ( !nl->IsAtom ( nl->Fourth ( xCurrentRoute ) ) ) ||
            nl->AtomType ( nl->Fourth ( xCurrentRoute ) ) != BoolType ||
            ( !nl->IsAtom ( nl->Fifth ( xCurrentRoute ) ) ) ||
            nl->AtomType ( nl->Fifth ( xCurrentRoute ) ) != BoolType )
    {
      delete pRoutes;
      delete pRoutes;

      string strErrorMessage = "Network(): Error while reading out routes.";
      inout_xErrorInfo = nl->Append ( inout_xErrorInfo,
                                      nl->StringAtom ( strErrorMessage ) );
      inout_bCorrect = false;
      return;
    }

    // Read attributes from list
    // Read values from table
    int iRouteId = nl->IntValue ( nl->First ( xCurrentRoute ) );
    double dLength  = nl->RealValue ( nl->Second ( xCurrentRoute ) );
    Word xLineWord = InSimpleLine ( nl->TheEmptyList(),
                                    nl->Third ( xCurrentRoute ),
                                    in_iErrorPos,
                                    inout_xErrorInfo,
                                    inout_bCorrect );
    SimpleLine* pLine = ( SimpleLine* ) ( xLineWord.addr );
    bool bDual= nl->BoolValue ( nl->Fourth ( xCurrentRoute ) );
    bool bStartsSmaller  = nl->BoolValue ( nl->Fifth ( xCurrentRoute ) );

    // Set all necessary attributes
    pNewRoute->PutAttribute ( ROUTE_ID, new CcInt ( true, iRouteId ) );
    pNewRoute->PutAttribute ( ROUTE_LENGTH, new CcReal ( true, dLength ) );
    pNewRoute->PutAttribute ( ROUTE_CURVE, pLine );
    pNewRoute->PutAttribute ( ROUTE_DUAL, new CcBool ( true, bDual ) );
    pNewRoute->PutAttribute ( ROUTE_STARTSSMALLER, new CcBool ( true,
                              bStartsSmaller ) );

    // Append new junction
    pRoutes->AppendTuple ( pNewRoute );
    if ( pNewRoute )
    {
      pNewRoute->DeleteIfAllowed();
      pNewRoute=0;
    }
  }

  // Iterate over all junctions
  while ( !nl->IsEmpty ( xJunctionList ) )
  {
    ListExpr xCurrentJunction = nl->First ( xJunctionList );
    xJunctionList = nl->Rest ( xJunctionList );

    // Create tuple for internal table
    Tuple* pNewJunction = new Tuple ( nl->Second ( xJunctionsNumType ) );

    // Check this part of the list
    if ( nl->ListLength ( xCurrentJunction ) != 6 ||
            ( !nl->IsAtom ( nl->First ( xCurrentJunction ) ) ) ||
            nl->AtomType ( nl->First ( xCurrentJunction ) ) != IntType ||
            ( !nl->IsAtom ( nl->Second ( xCurrentJunction ) ) ) ||
            nl->AtomType ( nl->Second ( xCurrentJunction ) ) != RealType ||
            ( !nl->IsAtom ( nl->Third ( xCurrentJunction ) ) ) ||
            nl->AtomType ( nl->Third ( xCurrentJunction ) ) != IntType ||
            ( !nl->IsAtom ( nl->Fourth ( xCurrentJunction ) ) ) ||
            nl->AtomType ( nl->Fourth ( xCurrentJunction ) ) != RealType ||
            ( !nl->IsAtom ( nl->Fifth ( xCurrentJunction ) ) ) ||
            nl->AtomType ( nl->Fifth ( xCurrentJunction ) ) != IntType )
    {
      delete pRoutes;
      delete pJunctions;

      string strErrorMessage = "Network(): Error while reading out junctions.";
      inout_xErrorInfo = nl->Append ( inout_xErrorInfo,
                                      nl->StringAtom ( strErrorMessage ) );
      inout_bCorrect = false;
      return;
    }

    // Read attributes from list
    int iRoute1Id = nl->IntValue ( nl->First ( xCurrentJunction ) );
    double dMeas1 = nl->RealValue ( nl->Second ( xCurrentJunction ) );
    int iRoute2Id = nl->IntValue ( nl->Third ( xCurrentJunction ) );
    double dMeas2 = nl->RealValue ( nl->Fourth ( xCurrentJunction ) );
    int iConnectivityCode= nl->IntValue ( nl->Fifth ( xCurrentJunction ) );
    // The location of the junction "Point" is calculated in the load-method

    // Set all necessary attributes
    pNewJunction->PutAttribute ( JUNCTION_ROUTE1_ID,
                                 new CcInt ( true, iRoute1Id ) );
    pNewJunction->PutAttribute ( JUNCTION_ROUTE1_MEAS,
                                 new CcReal ( true, dMeas1 ) );
    pNewJunction->PutAttribute ( JUNCTION_ROUTE2_ID,
                                 new CcInt ( true, iRoute2Id ) );
    pNewJunction->PutAttribute ( JUNCTION_ROUTE2_MEAS,
                                 new CcReal ( true, dMeas2 ) );
    pNewJunction->PutAttribute ( JUNCTION_CC,
                                 new CcInt ( true, iConnectivityCode ) );

    // Append new junction
    pJunctions->AppendTuple ( pNewJunction );
    if ( pNewJunction )
    {
      pNewJunction->DeleteIfAllowed();
      pNewJunction=0;
    }
  }

  Load ( m_iId,
         pRoutes,
         pJunctions );

  delete pRoutes;
  delete pJunctions;


  m_bDefined = true;
}

/*
Destructor

*/
Network::~Network()
{
  delete m_pRoutes;

  delete m_pJunctions;

  delete m_pSections;

  delete m_pBTreeRoutes;

  delete m_pBTreeRoutesByStartposId;

  delete m_pRTreeRoutes;

  delete m_pBTreeJunctionsByRoute1;

  delete m_pBTreeJunctionsByRoute2;

  delete m_pBTreeSectionsByRoute;

  delete m_pBTreeSections;

//  delete alldistance;
}

/*
2.1.3 Methods of class ~network~

Method ~Destroy~

Removing a network from the database

*/
void Network::Destroy()
{
  assert ( m_pRoutes != 0 );
  m_pRoutes->Delete(); m_pRoutes = 0;

  assert ( m_pJunctions != 0 );
  m_pJunctions->Delete(); m_pJunctions = 0;

  assert ( m_pSections != 0 );
  m_pSections->Delete(); m_pSections = 0;

  assert ( m_pBTreeRoutes != 0 );
  m_pBTreeRoutes->DeleteFile();
  delete m_pBTreeRoutes; m_pBTreeRoutes = 0;

  assert ( m_pBTreeRoutesByStartposId != 0 );
  m_pBTreeRoutesByStartposId->DeleteFile();
  delete m_pBTreeRoutesByStartposId; m_pBTreeRoutesByStartposId = 0;

  assert ( m_pRTreeRoutes != 0 );
  //m_pRTreeRoutes->DeleteFile();
  delete m_pRTreeRoutes; m_pRTreeRoutes = 0;

  m_pBTreeJunctionsByRoute1->DeleteFile();
  delete m_pBTreeJunctionsByRoute1; m_pBTreeJunctionsByRoute1 = 0;

  m_pBTreeJunctionsByRoute2->DeleteFile();
  delete m_pBTreeJunctionsByRoute2; m_pBTreeJunctionsByRoute2 = 0;

//  m_xAdjacencyList.Destroy();
//  m_xSubAdjacencyList.Destroy();
  assert ( m_pBTreeSectionsByRoute != 0 );
  m_pBTreeSectionsByRoute->DeleteFile();
  delete m_pBTreeSectionsByRoute;
  m_pBTreeSectionsByRoute = 0;

  assert ( m_pBTreeSections != 0 );
  m_pBTreeSections->DeleteFile();
  delete m_pBTreeSections;
  m_pBTreeSections = 0;

  /*
  assert(alldistance != 0);
  delete alldistance;
  */
}

/*
Method ~Load~

Create a network from two external relations

*/

void Network::Load ( int in_iId,
                     const Relation* in_pRoutes,
                     const Relation* in_pJunctions )
{
  m_iId = in_iId;
  FillRoutes ( in_pRoutes );
  FillJunctions ( in_pJunctions );
  FillSections();
  FillAdjacencyLists();
//FillDistanceStorage();//store distance
  m_bDefined = true;
}

/*
Method ~OptimizeNetwork~

optimizes the order of the ~routes~, after that all ~sections~ are generated new
and  all structures based on these ~sections~ are adjusted

*/
bool Network::OptimizeNetwork(){

    //sort routes
    OptimizationRoutes();

    //DEBUG
    //cout << "output of all routes"<< endl;
    //for (int i =1; i<=m_pRoutes->GetNoTuples();i++)
    //{
        //Tuple *n = m_pRoutes->GetTuple(i,false);
        //cout <<"("<< ((CcInt*)n->GetAttribute(ROUTE_ID))->GetIntval() << ","
        //       << ((CcInt*)n->GetAttribute(ROUTE_STARTPOS_ID))->GetIntval()
        //       << ")"<<endl;
        //n->DeleteIfAllowed();
    //}

    //update the BTree for routes by startPos_id
    delete m_pBTreeRoutesByStartposId;
    m_pBTreeRoutesByStartposId=0;
    ostringstream xThisRoutesPtrStream2;
    xThisRoutesPtrStream2 << ( long ) m_pRoutes;
    string strQuery = "(createbtree (" + routesInternalTypeInfo +
             " (ptr " + xThisRoutesPtrStream2.str() + "))" + " Startposid)";
    Word xResult;
    int QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
    assert ( QueryExecuted ); // no query with side effects, please!
    m_pBTreeRoutesByStartposId = ( BTree* ) xResult.addr;

    //Fillsections new
    delete m_pBTreeSections;
    delete m_pBTreeSectionsByRoute;
    int anz = m_pSections->GetNoTuples();
    for(int z=1;z <= anz ;z++){
        m_pSections->DeleteTuple(m_pSections->GetTuple(z,false));
    }

    m_pSections->Clear();
    delete m_pSections;
    m_pSections = 0;
    FillSections();

    //Update the Adjacencylist
    //DEBUG cout << "update AdjacencList mit "<< m_xAdjacencyList.Size()<< endl;
    m_xAdjacencyList.clean();
    m_xAdjacencyList = 0;
    FillAdjacencyLists();
    return true;
}

/*
Mehtod ~OptimizationRoutes~

determined for all ~routes~ their lexicographic start point, stores them in an
array and calls the sort method ~QuicksortRoutes~

*/
bool Network::OptimizationRoutes(){
  Tuple *route;
  GPoint *gp;
  Point *p = new Point();
  int anzElemente = m_pRoutes->GetNoTuples();
  int testarray[anzElemente][2];
  bool opt= false;

  for (int i =1; i<=m_pRoutes->GetNoTuples();i++)
    {
        route = m_pRoutes->GetTuple(i,false);

        int id = ((CcInt*) route->GetAttribute(ROUTE_ID))->GetIntval();
        //bool smaller=
        //   ((CcBool*) route->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();

        // dermine points
        gp = new GPoint(true, GetId(),id, 0.0);
        gp->ToPoint(p);
        if (p->IsDefined()) opt = true; else return false;
        testarray[i-1][0] = id;
        testarray[i-1][1] =  p->GetX();
    }
    if(opt)
    {
        //DEBUG for testing
        //cout << "vor dem Sortieren" << endl;
        //for (int k =0; k< anzElemente; k++)
        //{
            //cout << testarray[k][0] <<","  <<testarray[k][1]<< endl;
        //}
        QuicksortRoutes(testarray,0,anzElemente-1);

        //DEBUG for testing
        //cout << "nach dem Sortieren" << endl;
        //for (int k =0; k< anzElemente; k++)
        //{
            //cout << testarray[k][0] <<","  <<testarray[k][1]<< endl;
        //}

        // update startposId  in m_pRoutes
        for (int k =0; k< anzElemente; k++)
        {
            route = m_pRoutes->GetTuple(testarray[k][0],false);

            vector<int> xIndices;
            vector<Attribute*> xAttrs;
            xIndices.push_back ( ROUTE_STARTPOS_ID );
            xAttrs.push_back ( new CcInt ( true, k+1 ) );
            m_pRoutes->UpdateTuple(route, xIndices, xAttrs);
        }
    }
    p->DeleteIfAllowed();
    gp->DeleteIfAllowed();
    route->DeleteIfAllowed();
    return true;
}



/*
Method ~QuicksortRoutes~

calculates the pivot-element-index with the method
~QuicksortRoutesPartition~. After that ~QuicksortRoutes~ is recursively called
for the upper and lower part of the array

*/

void Network::QuicksortRoutes(int arr[][2], int low, int high)
{
    int pivotIndex;
    if(low < high)
    {   //split the array and calculate the pivot-element
        pivotIndex = QuicksortRoutesPartition(arr, low, high);
        //sort the bottom part
        QuicksortRoutes(arr, low, pivotIndex-1);
        //sort the top part
        QuicksortRoutes(arr, pivotIndex+1,high);
    }
}

/*
Method ~QuicksortRoutesPartition~

split the given array and sorts the array so that all elements before the
pivot-element are smaller and all elements that are above the pivot element
are larger.

*/
int Network::QuicksortRoutesPartition(int arr[][2], int low, int high)
{
    //DEBUG cout << "QuicksortPartiton:  mit lowindex: " << low
    //             << " und highindex: " << high << endl;
    int high_vac[2]={0},low_vac[2]={0},pivot[2]={0};
    pivot[0] = arr[low][0];
    pivot[1] = arr[low][1];

    // while the lowindex is less than the highIndex
    while(high > low)
    {   //DEBUG cout << "while high: "<< high
        //             << " ist groesser als low: " << low << endl;
        high_vac[0] = arr[high][0];
        high_vac[1] = arr[high][1];
        //DEBUG cout << "highWert " << high_vac[1] << endl;

        //while the value of the high-element is greater than
        //or equal the pivot-element
        while(pivot[1] <= high_vac[1])
        {   //DEBUG cout << "while  pivot : "<< pivot[1]
            //          << " ist kleiner als hight_vac: " << high_vac[1]<< endl;
            // if hightindex less then or equal with
            // the lowindex BREAK the while-loop
            if(high <= low) break;
            high--;
            high_vac[0] = arr[high][0];
            high_vac[1] = arr[high][1];
        }
        arr[low][0] = high_vac[0];
        arr[low][1] = high_vac[1];
        low_vac[0] = arr[low][0];
        low_vac[1] = arr[low][1];

        //while the value of the pivot-element is greater than the low-element
        while(pivot[1] > low_vac[1])
        {   //cout << "while  pivot : "<< pivot[1]
            //       << " ist groesser als low_vac: " << low_vac[1]<< endl;
            // if hightindex less then or equal with
            // the lowindex BREAK the while-loop
            if(high<=low) break;
            low++;
            low_vac[0] = arr[low][0];
            low_vac[1] = arr[low][1];
        }
        arr[high][0] = low_vac[0];
        arr[high][1] = low_vac[1];
    }
    arr[low][0] = pivot[0];
    arr[low][1] = pivot[1];

    //return the lowindex value
    return low;
}



/*
Method ~FillRoutes~

Fill routes relation of network

*/
void Network::FillRoutes ( const Relation *routes )
{
    //Create new table for the routes
    ListExpr xTypeInfo;
    nl->ReadFromString ( routesInternalTypeInfo, xTypeInfo );
    ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType ( xTypeInfo );
    Relation *pIntRoutes = new Relation ( xNumType, true );

    // Iterator for the input-table with routes
    GenericRelationIterator* pRoutesIter = routes->MakeScan();
    Tuple* pCurrentRoute;
    int routeCounter =1;
    while ( ( pCurrentRoute = pRoutesIter->GetNextTuple() ) != 0 )
    {
        // Create tuple for internal table and copy all attributes from input
        Tuple* pNewRoute = new Tuple ( nl->Second ( xNumType ) );
        for ( int i = 0; i < pCurrentRoute->GetNoAttributes(); i++ )
        {
            pNewRoute->CopyAttribute ( i, pCurrentRoute, i );
        }
        // Fill other fields of the table
        pNewRoute->PutAttribute(ROUTE_STARTPOS_ID,new CcInt(true,routeCounter));
        // Append new route
        pIntRoutes->AppendTuple ( pNewRoute );

        //clean up
        pCurrentRoute->DeleteIfAllowed();
        pNewRoute->DeleteIfAllowed();
        // increment routeCounter for next route
        routeCounter++;
    }
    delete pRoutesIter;

    ostringstream xRoutesPtrStream;
    xRoutesPtrStream << ( long ) pIntRoutes;

    string strQuery = "(consume (sort (feed (" + routesInternalTypeInfo +
                        " (ptr " + xRoutesPtrStream.str() + ")))))";

    Word xResult;
    int QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
    assert ( QueryExecuted );
    m_pRoutes = ( Relation * ) xResult.addr;

    delete pIntRoutes;



  // Create B-Tree for the routes
  ostringstream xThisRoutesPtrStream;
  xThisRoutesPtrStream << ( long ) m_pRoutes;
  strQuery = "(createbtree (" + routesInternalTypeInfo +
             " (ptr " + xThisRoutesPtrStream.str() + "))" + " Id)";

  QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted ); // no query with side effects, please!
  m_pBTreeRoutes = ( BTree* ) xResult.addr;

  // Create B-Tree for the routes by startPos_id
  ostringstream xThisRoutesPtrStream2;
  xThisRoutesPtrStream2 << ( long ) m_pRoutes;
  strQuery = "(createbtree (" + routesInternalTypeInfo +
             " (ptr " + xThisRoutesPtrStream2.str() + "))" + " Startposid)";

  QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted ); // no query with side effects, please!
  m_pBTreeRoutesByStartposId = ( BTree* ) xResult.addr;

  //Create R-Tree for the routes
  ostringstream xNetRoutes;
  xNetRoutes << ( long ) m_pRoutes;

  strQuery = "(bulkloadrtree(sortby(addid(feed (" + routesInternalTypeInfo +
         " (ptr " + xThisRoutesPtrStream.str() + "))))((Curve asc))) Curve)";
  QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted );
  m_pRTreeRoutes = ( R_Tree<2,TupleId>* ) xResult.addr;
}


/*
Method ~FillJunctions~

Fill junctions relation of network

*/
void Network::FillJunctions ( const Relation *in_pJunctions )
{
  /////////////////////////////////////////////////////////////////////
  //
  // Create new table for the junctions
  //
  ListExpr xTypeInfo;
  nl->ReadFromString ( junctionsInternalTypeInfo, xTypeInfo );
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType ( xTypeInfo );
  Relation *pIntJunctions = new Relation ( xNumType, true );

  /////////////////////////////////////////////////////////////////////
  //
  // Iterator for the input-table with junctions
  //
  GenericRelationIterator* pJunctionsIter = in_pJunctions->MakeScan();
  Tuple* pCurrentJunction;
  while ( ( pCurrentJunction = pJunctionsIter->GetNextTuple() ) != 0 )
  {
    /////////////////////////////////////////////////////////////////////
    //
    // Create tuple for internal table and copy all attributes from input
    //
    Tuple* pNewJunction = new Tuple ( nl->Second ( xNumType ) );
    for ( int i = 0; i < pCurrentJunction->GetNoAttributes(); i++ )
    {
      pNewJunction->CopyAttribute ( i, pCurrentJunction, i );
    }


    /////////////////////////////////////////////////////////////////////
    //
    // Fill other fields of the table
    //

    // Store Pointer to the first route in the new relation.
    CcInt* pR1Id = ( CcInt* ) pCurrentJunction->GetAttribute (
JUNCTION_ROUTE1_ID );
    BTreeIterator* pRoutesIter = m_pBTreeRoutes->ExactMatch ( pR1Id );
    int NextIter = pRoutesIter->Next();
    assert ( NextIter );
    TupleIdentifier *pR1RC = new TupleIdentifier ( true, pRoutesIter->GetId() );
    pNewJunction->PutAttribute ( JUNCTION_ROUTE1_RC, pR1RC );

    // Calculate and store the exakt location of the junction.
    Tuple* pRoute = m_pRoutes->GetTuple ( pRoutesIter->GetId(), false );
    assert ( pRoute != 0 );
    SimpleLine* pLine = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
    assert ( pLine != 0 );
    CcReal* pMeas = ( CcReal* ) pNewJunction->GetAttribute (
JUNCTION_ROUTE1_MEAS );
    Point* pPoint = new Point ( false );
    pLine->AtPosition ( pMeas->GetRealval(), true, *pPoint );
    pNewJunction->PutAttribute ( JUNCTION_POS, pPoint );

    pRoute->DeleteIfAllowed();
    delete pRoutesIter;

    // Store Pointer to the second route in the new relation.
    CcInt* pR2Id = ( CcInt* ) pCurrentJunction->GetAttribute (
JUNCTION_ROUTE2_ID );
    pRoutesIter = m_pBTreeRoutes->ExactMatch ( pR2Id );
    NextIter = pRoutesIter->Next();
    assert ( NextIter ); // no query with side effects, please!
    TupleIdentifier *pR2RC = new TupleIdentifier ( true, pRoutesIter->GetId() );
    pNewJunction->PutAttribute ( JUNCTION_ROUTE2_RC, pR2RC );
    delete pRoutesIter;

    // Pointers to sections are filled in FillSections
    pNewJunction->PutAttribute ( JUNCTION_SECTION_AUP_RC,
                                 new TupleIdentifier ( false ) );
    pNewJunction->PutAttribute ( JUNCTION_SECTION_ADOWN_RC,
                                 new TupleIdentifier ( false ) );
    pNewJunction->PutAttribute ( JUNCTION_SECTION_BUP_RC,
                                 new TupleIdentifier ( false ) );
    pNewJunction->PutAttribute ( JUNCTION_SECTION_BDOWN_RC,
                                 new TupleIdentifier ( false ) );

    /////////////////////////////////////////////////////////////////////
    //
    // Append new junction
    //
    pIntJunctions->AppendTuple ( pNewJunction );

    pCurrentJunction->DeleteIfAllowed();
    pNewJunction->DeleteIfAllowed();
  }
  delete pJunctionsIter;

  /////////////////////////////////////////////////////////////////////
  //
  // Sort the table which is now containing all junctions
  //
  ostringstream xJunctionsStream;
  xJunctionsStream << ( long ) pIntJunctions;
  string strQuery = "(consume (sortby (feed (" + junctionsInternalTypeInfo +
                    " (ptr " + xJunctionsStream.str() +
                    "))) ((R1id asc)(Meas1 asc))))";


  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted );
  m_pJunctions = ( Relation * ) xResult.addr;

  // Delete internal table
  pIntJunctions->Delete();

  /////////////////////////////////////////////////////////////////////
  //
  // Create two b-trees for the junctions sorted by first and second id
  //
  ostringstream xThisJunctionsPtrStream;
  xThisJunctionsPtrStream << ( long ) m_pJunctions;
  strQuery = "(createbtree (" + junctionsInternalTypeInfo +
             " (ptr " + xThisJunctionsPtrStream.str() + "))" + " R1id)";
  QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted ); // no query with side effects, please!
  m_pBTreeJunctionsByRoute1 = ( BTree* ) xResult.addr;

  ostringstream xThisJunctionsPtrStream2;
  xThisJunctionsPtrStream2 << ( long ) m_pJunctions;
  strQuery = "(createbtree (" + junctionsInternalTypeInfo +
             " (ptr " + xThisJunctionsPtrStream2.str() + "))" + " R2id)";
  QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted ); // no query with side effects, please!
  m_pBTreeJunctionsByRoute2 = ( BTree* ) xResult.addr;
}

/*
Method ~FillSections~

Fill section relation of network

*/
void Network::FillSections()
{
  // The method will iterate over routes by StartposIndex
  BTreeIterator* pRoutesIt = m_pBTreeRoutesByStartposId->SelectAll();

  // Create relation for sections
  ListExpr xType;
  nl->ReadFromString ( sectionsInternalTypeInfo, xType );
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType ( xType );
  m_pSections = new Relation ( xNumType );
  /////////////////////////////////////////////////////////////////////
  //
  // Iterate over all Routes
  //
  Tuple* pRoute;
  TupleId iSectionTid = 0;
  while ( pRoutesIt->Next() )
    {
      pRoute = m_pRoutes->GetTuple(pRoutesIt->GetId(),false);
      //DEBUG cout << "Rid "
      //          <<((CcInt*)pRoute->GetAttribute(ROUTE_ID))->GetIntval()<<endl;
    // Current position on route - starting at the beginning of the route
    double dCurrentPosOnRoute = 0;
    SimpleLine* pRouteCurve = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE
);
    TupleId iTupleId = pRoute->GetTupleId();
    CcInt* xRouteId = ( CcInt* ) pRoute->GetAttribute ( ROUTE_ID );
    int iRouteId = xRouteId->GetIntval();
    bool bDual = ( ( CcBool* ) pRoute->GetAttribute ( ROUTE_DUAL )
)->GetBoolval();
    /////////////////////////////////////////////////////////////////////
    //
    // We need to find all junctions belonging to this route
    //
    vector<JunctionSortEntry> xJunctions;
    GetJunctionsOnRoute ( xRouteId,
                          xJunctions );

    /////////////////////////////////////////////////////////////////////
    //
    // Now that we found all relevant junctions we can iterate over them.
    //
    JunctionSortEntry xCurrentEntry;
    xCurrentEntry.m_pJunction = 0;
    xCurrentEntry.m_bFirstRoute = false;
    for ( size_t i = 0; i < xJunctions.size(); i++ )
    {
      // Get next junction
      xCurrentEntry = xJunctions[i];

      // Find values for the new section
      double dStartPos = dCurrentPosOnRoute;
      double dEndPos = xCurrentEntry.GetRouteMeas();

      // If the first junction is at the very start of the route, no
      // section will be added
      if ( xCurrentEntry.GetRouteMeas() == 0 )
      {
        vector<int> xIndices;
        vector<Attribute*> xAttrs;
        if ( xCurrentEntry.m_bFirstRoute )
        {
          xIndices.push_back ( JUNCTION_SECTION_ADOWN_RC );
          xAttrs.push_back ( new TupleIdentifier ( true, 0 ) );
        }
        else
        {
          xIndices.push_back ( JUNCTION_SECTION_BDOWN_RC );
          xAttrs.push_back ( new TupleIdentifier ( true, 0 ) );
        }
        m_pJunctions->UpdateTuple ( xCurrentEntry.m_pJunction,
                                    xIndices,
                                    xAttrs );
        continue;
      }

      /////////////////////////////////////////////////////////////////////
      //
      // Create a new section
      //
      // Section will only be created if the length is > 0. Otherwise the
      // one before remains valid.
      if ( dEndPos - dStartPos > 0.01 )
      {
        // A sline for the section
        SimpleLine* pLine = new SimpleLine ( 0 );

        // Take start from the route
        bool bStartSmaller = ( ( CcBool* ) pRoute->GetAttribute (
                                   ROUTE_STARTSSMALLER ) )->GetBoolval();

        pRouteCurve->SubLine ( dStartPos,
                               dEndPos,
                               bStartSmaller,
                               *pLine );

        // Find out, if the orientation of the subline differs from the position
        // of the line. If so, the direction has to be changed.
        bool bLineStartsSmaller;
        Point pStartPoint ( false );
        pRouteCurve->AtPosition ( dStartPos, bStartSmaller, pStartPoint );
        Point pEndPoint ( false );
        pRouteCurve->AtPosition ( dEndPos, bStartSmaller, pEndPoint );
        if ( pStartPoint.GetX() < pEndPoint.GetX() ||
                (
                    pStartPoint.GetX() == pEndPoint.GetX() &&
                    pStartPoint.GetY() < pEndPoint.GetY()
                )
           )
        {
          // Normal orientation
          bLineStartsSmaller = true;
        }
        else
        {
          // Opposite orientation
          bLineStartsSmaller = false;
        }

        // The new section
        Tuple* pNewSection = new Tuple ( nl->Second ( xNumType ) );
        pNewSection->PutAttribute ( SECTION_RID, new CcInt ( true, iRouteId ) );
        pNewSection->PutAttribute ( SECTION_DUAL, new CcBool ( true, bDual ) );
        pNewSection->PutAttribute ( SECTION_MEAS1, new CcReal ( true, dStartPos
) );
        pNewSection->PutAttribute ( SECTION_MEAS2, new CcReal ( true, dEndPos )
);
        pNewSection->PutAttribute ( SECTION_RRC, new TupleIdentifier ( true,
                                    iTupleId ) );
        pNewSection->PutAttribute ( SECTION_CURVE, pLine );
        pNewSection->PutAttribute ( SECTION_CURVE_STARTS_SMALLER,
                                    new CcBool ( true, bLineStartsSmaller ) );
        pNewSection->PutAttribute ( SECTION_SID,
                                    new CcInt ( true, m_pSections->GetNoTuples()
+1 ) );

        //new Attributes inserted
        pNewSection->PutAttribute (SECTION_PNO_UP, new CcInt(true,0));
        pNewSection->PutAttribute (SECTION_PNO_DOWN, new CcInt(true,0));
        pNewSection->PutAttribute (SECTION_SLOT_UP, new CcInt(true,0));
        pNewSection->PutAttribute (SECTION_SLOT_DOWN, new CcInt(true,0));
        pNewSection->PutAttribute (SECTION_COST, new CcReal(true,1.0));
        pNewSection->PutAttribute (SECTION_DURATION, new CcReal(true,1.0));

        m_pSections->AppendTuple ( pNewSection );
        iSectionTid++;
        pNewSection->DeleteIfAllowed();
        // Update position for next loop
        dCurrentPosOnRoute = dEndPos;
      }

      /////////////////////////////////////////////////////////////////////
      //
      // Store ID of new section in junction behind that section.
      //
      vector<int> xIndices;
      vector<Attribute*> xAttrs;
      if ( xCurrentEntry.m_bFirstRoute )
      {
        xIndices.push_back ( JUNCTION_SECTION_ADOWN_RC );
        xAttrs.push_back ( new TupleIdentifier ( true, iSectionTid ) );
      }
      else
      {
        xIndices.push_back ( JUNCTION_SECTION_BDOWN_RC );
        xAttrs.push_back ( new TupleIdentifier ( true, iSectionTid ) );
      }
      m_pJunctions->UpdateTuple ( xCurrentEntry.m_pJunction,
                                  xIndices,
                                  xAttrs );
      if ( pRouteCurve->Length() - xCurrentEntry.GetRouteMeas() < 0.01 )
      {
        vector<int> xIndices;
        vector<Attribute*> xAttrs;
        if ( xCurrentEntry.m_bFirstRoute )
        {
          xIndices.push_back ( JUNCTION_SECTION_AUP_RC );
          xAttrs.push_back ( new TupleIdentifier ( true, 0 ) );
        }
        else
        {
          xIndices.push_back ( JUNCTION_SECTION_BUP_RC );
          xAttrs.push_back ( new TupleIdentifier ( true, 0 ) );
        }
        m_pJunctions->UpdateTuple ( xCurrentEntry.m_pJunction,
                                    xIndices,
                                    xAttrs );
      }

    } // End junctions-loop



    /////////////////////////////////////////////////////////////////////
    //
    // The last section of the route is still missing, if the last
    // junction is not at the end of the route.
    //
    if ( pRouteCurve->Length() - dCurrentPosOnRoute > 0.01 ||
            dCurrentPosOnRoute == 0.0 )
    {
      // Find values for the new section
      int iRouteId = ( ( CcInt* ) pRoute->GetAttribute ( ROUTE_ID )
)->GetIntval();
      bool bDual = ( ( CcBool* ) pRoute->GetAttribute ( ROUTE_DUAL )
)->GetBoolval();
      double dStartPos = dCurrentPosOnRoute;
      double dEndPos = pRouteCurve->Length();
      TupleId iTupleId = pRoute->GetTupleId();

      // Calculate line
      SimpleLine* pLine = new SimpleLine ( 0 );
      bool bStartSmaller = ( ( CcBool* ) pRoute->GetAttribute (
                                 ROUTE_STARTSSMALLER ) )->GetBoolval();
      pRouteCurve->SubLine ( dStartPos,
                             dEndPos,
                             bStartSmaller,
                             *pLine );

      // Find out, if the orientation of the subline differs from the position
      // of the sline. If so, the direction has to be changed.
      bool bLineStartsSmaller;
      Point pStartPoint ( false );
      pRouteCurve->AtPosition ( dStartPos, bStartSmaller, pStartPoint );
      Point pEndPoint ( false );
      pRouteCurve->AtPosition ( dEndPos, bStartSmaller, pEndPoint );
      if ( pStartPoint.GetX() < pEndPoint.GetX() ||
              (
                  pStartPoint.GetX() == pEndPoint.GetX() &&
                  pStartPoint.GetY() < pEndPoint.GetY()
              )
         )
      {
        // Normal orientation
        bLineStartsSmaller = true;
      }
      else
      {
        // Opposite orientation
        bLineStartsSmaller = false;
      }

      // Create a new Section
      Tuple* pNewSection = new Tuple ( nl->Second ( xNumType ) );
      pNewSection->PutAttribute ( SECTION_RID, new CcInt ( true, iRouteId ) );
      pNewSection->PutAttribute ( SECTION_DUAL, new CcBool ( true, bDual ) );
      pNewSection->PutAttribute ( SECTION_MEAS1, new CcReal ( true, dStartPos )
);
      pNewSection->PutAttribute ( SECTION_MEAS2, new CcReal ( true, dEndPos ) );
      pNewSection->PutAttribute ( SECTION_RRC,
                                  new TupleIdentifier ( true, iTupleId ) );
      pNewSection->PutAttribute ( SECTION_CURVE, pLine );
      pNewSection->PutAttribute ( SECTION_CURVE_STARTS_SMALLER,
                                  new CcBool ( true, bLineStartsSmaller ) );
      pNewSection->PutAttribute ( SECTION_SID,
                                  new CcInt ( true, m_pSections->GetNoTuples()
+1 ) );
      //new Attributes inserted
      pNewSection->PutAttribute (SECTION_PNO_UP, new CcInt(true,0));
      pNewSection->PutAttribute (SECTION_PNO_DOWN, new CcInt(true,0));
      pNewSection->PutAttribute (SECTION_SLOT_UP, new CcInt(true,0));
      pNewSection->PutAttribute (SECTION_SLOT_DOWN, new CcInt(true,0));
      pNewSection->PutAttribute (SECTION_COST, new CcReal(true,1.0));
      pNewSection->PutAttribute (SECTION_DURATION, new CcReal(true,1.0));


      m_pSections->AppendTuple ( pNewSection );
      iSectionTid++;
      pNewSection->DeleteIfAllowed();
      // Store ID of new section in Junction
      if ( xCurrentEntry.m_pJunction != 0 )
      {
        vector<int> xIndicesLast;
        vector<Attribute*> xAttrsLast;
        if ( xCurrentEntry.m_bFirstRoute )
        {
          xIndicesLast.push_back ( JUNCTION_SECTION_AUP_RC );
          xAttrsLast.push_back ( new TupleIdentifier ( true,
                                 iSectionTid ) );
        }
        else
        {
          xIndicesLast.push_back ( JUNCTION_SECTION_BUP_RC );
          xAttrsLast.push_back ( new TupleIdentifier ( true,
                                 iSectionTid ) );
        }
        m_pJunctions->UpdateTuple ( xCurrentEntry.m_pJunction,
                                    xIndicesLast,
                                    xAttrsLast );
      }
    } // end if
    ////////////////////////////////////////////////////////////////////
    //
    // Fill Up-Pointers of all sections but the last
    //
    vector<JunctionSortEntry> yJunctions;
    GetJunctionsOnRoute ( xRouteId,
                          yJunctions );
    if ( yJunctions.size() > 2 )
    {
      for ( int i = yJunctions.size()-2; i >= 0; i-- )
      {
        // Get next junction
        JunctionSortEntry xEntry = yJunctions[i];
        JunctionSortEntry xEntryBehind = yJunctions[i + 1];

        vector<int> xIndices;
        if ( xEntry.m_bFirstRoute )
        {
          xIndices.push_back ( JUNCTION_SECTION_AUP_RC );
        }
        else
        {
          xIndices.push_back ( JUNCTION_SECTION_BUP_RC );
        }
        vector<Attribute*> xAttrs;
        if ( xEntryBehind.GetRouteMeas() - xEntry.GetRouteMeas() < 0.01 )
        {
          // Two junctions at the same place. In this case they do have
          // the same up-pointers
          if ( xEntryBehind.m_bFirstRoute )
          {
            TupleId iTid = xEntryBehind.GetUpSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
          else
          {
            TupleId iTid = xEntryBehind.GetUpSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
        }
        else
        {
          // Junctions not on the same place. The down-pointer of the second is
          // the up-pointer of the first.
          if ( xEntryBehind.m_bFirstRoute )
          {
            TupleId iTid = xEntryBehind.GetDownSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
          else
          {
            TupleId iTid = xEntryBehind.GetDownSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
        }
        m_pJunctions->UpdateTuple ( xEntry.m_pJunction,
                                    xIndices,
                                    xAttrs );
      }
    }
    else
    {
      if ( yJunctions.size() == 2 )
      {
        JunctionSortEntry xEntry = yJunctions[0];
        JunctionSortEntry xEntryBehind = yJunctions[1];
        vector<int> xIndices;
        if ( xEntry.m_bFirstRoute )
        {
          xIndices.push_back ( JUNCTION_SECTION_AUP_RC );
        }
        else
        {
          xIndices.push_back ( JUNCTION_SECTION_BUP_RC );
        }
        vector<Attribute*> xAttrs;
        if ( fabs ( xEntry.GetRouteMeas() - xEntryBehind.GetRouteMeas() ) < 0.01
)
        {
          if ( xEntryBehind.m_bFirstRoute )
          {
            TupleId iTid = xEntryBehind.GetUpSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
          else
          {
            TupleId iTid = xEntryBehind.GetUpSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
          m_pJunctions->UpdateTuple ( xEntry.m_pJunction,
                                      xIndices,
                                      xAttrs );
        }
        else
        {
          if ( xEntryBehind.m_bFirstRoute )
          {
            TupleId iTid = xEntryBehind.GetDownSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
          else
          {
            TupleId iTid = xEntryBehind.GetDownSectionId();
            xAttrs.push_back ( new TupleIdentifier ( true, iTid ) );
          }
          m_pJunctions->UpdateTuple ( xEntry.m_pJunction,
                                      xIndices,
                                      xAttrs );
        }
      }
    }
    pRoute->DeleteIfAllowed();

    // delete Tuples from xJunctions
    for ( unsigned int i=0;i<xJunctions.size();i++ )
    {
      if ( xJunctions[i].m_pJunction )
      {
        xJunctions[i].m_pJunction->DeleteIfAllowed();
      }
    }
    xJunctions.clear();
    // delete Tuples from yJunctions
    for ( unsigned int i=0;i<yJunctions.size();i++ )
    {
      if ( yJunctions[i].m_pJunction )
      {
        yJunctions[i].m_pJunction->DeleteIfAllowed();
      }
    }
    yJunctions.clear();

  } // End while Routes
  delete pRoutesIt;

  // Create B-Tree for the sections by Routes
  Word xResult;
  ostringstream xThisSectionsPtrStream;
  xThisSectionsPtrStream << ( long ) m_pSections;
  string strQuery = "(createbtree (" + sectionsInternalTypeInfo +
                    " (ptr " + xThisSectionsPtrStream.str() + "))" + " Rid)";
  int QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted ); // no query with side effects, please!
  m_pBTreeSectionsByRoute = ( BTree* ) xResult.addr;
  //Create B-Tree for the sections

  Word xResultNewBtree;
  ostringstream xStream2;
  xStream2 << (long) m_pSections;
  string query = "(createbtree (" + sectionsInternalTypeInfo +
                    " (ptr " + xStream2.str() + "))" + " Sid)";
  int QueryEx = QueryProcessor::ExecuteQuery ( query, xResultNewBtree );
  assert ( QueryEx ); // no query with side effects, please!
  m_pBTreeSections = ( BTree* ) xResultNewBtree.addr;
}

/*
Method ~FillAdjacencyLists~

Fill adjacency list of network

*/
void Network::FillAdjacencyLists()
{
    ////////////////////////////////////////////////////////////////////
    //In a first step all pairs of adjacent sections will be collected
    GenericRelationIterator* pJunctionsIt = m_pJunctions->MakeScan();
    Tuple* pCurrentJunction;
    vector<DirectedSectionPair> xList;

    while ( ( pCurrentJunction = pJunctionsIt->GetNextTuple() ) != 0 )
    {
        // Retrieve the connectivity code
        int iCc =
       (( CcInt* ) pCurrentJunction->GetAttribute ( JUNCTION_CC ))->GetIntval();
        ConnectivityCode xCc ( iCc );

        // Retrieve the four sections - if they exist
        TupleId tidpAUp, tidpADown, tidpBUp, tidpBDown;
        tidpAUp = (( TupleIdentifier* )pCurrentJunction
                    ->GetAttribute ( JUNCTION_SECTION_AUP_RC ))->GetTid();
        tidpADown =(( TupleIdentifier* )pCurrentJunction
                    ->GetAttribute ( JUNCTION_SECTION_ADOWN_RC ))->GetTid();
        tidpBUp = ( ( TupleIdentifier* )pCurrentJunction
                    ->GetAttribute ( JUNCTION_SECTION_BUP_RC ))->GetTid();
        tidpBDown = ( ( TupleIdentifier* )pCurrentJunction
                    ->GetAttribute ( JUNCTION_SECTION_BDOWN_RC ))->GetTid();

        // If a section is existing and the transition is possible
        // it will be added to the list.
        FillAdjacencyPair ( tidpAUp, false, tidpAUp, true,
                            xCc, AUP_AUP, xList );
        FillAdjacencyPair ( tidpAUp, false, tidpADown, false,
                            xCc, AUP_ADOWN, xList);
        FillAdjacencyPair ( tidpAUp, false, tidpBUp, true,
                            xCc, AUP_BUP, xList );
        FillAdjacencyPair ( tidpAUp, false, tidpBDown, false,
                            xCc, AUP_BDOWN, xList);

        FillAdjacencyPair ( tidpADown, true, tidpAUp, true,
                            xCc, ADOWN_AUP, xList );
        FillAdjacencyPair ( tidpADown, true, tidpADown, false,
                            xCc,ADOWN_ADOWN,xList);
        FillAdjacencyPair ( tidpADown, true, tidpBUp, true,
                            xCc, ADOWN_BUP, xList );
        FillAdjacencyPair ( tidpADown, true, tidpBDown, false,
                            xCc,ADOWN_BDOWN,xList);

        FillAdjacencyPair ( tidpBUp, false, tidpAUp, true,
                            xCc, BUP_AUP, xList );
        FillAdjacencyPair ( tidpBUp, false, tidpADown, false,
                            xCc, BUP_ADOWN, xList);
        FillAdjacencyPair ( tidpBUp, false, tidpBUp, true,
                            xCc, BUP_BUP, xList );
        FillAdjacencyPair ( tidpBUp, false, tidpBDown, false,
                            xCc, BUP_BDOWN, xList);

        FillAdjacencyPair ( tidpBDown, true, tidpAUp, true,
                            xCc, BDOWN_AUP, xList );
        FillAdjacencyPair ( tidpBDown, true, tidpADown, false,
                            xCc,BDOWN_ADOWN,xList);
        FillAdjacencyPair ( tidpBDown, true, tidpBUp, true,
                            xCc,BDOWN_BUP, xList );
        FillAdjacencyPair ( tidpBDown, true, tidpBDown, false,
                            xCc,BDOWN_BDOWN,xList);

        pCurrentJunction->DeleteIfAllowed();
    }
    delete pJunctionsIt;

    ////////////////////////////////////////////////////////////////////
    // Now - as the second step the adjacency lists are filled.

    // Sort the list by the first directed section
    sort ( xList.begin(), xList.end() );

    //m_pSections contains a sorted List of all sections
    //xList contains all neightbours of one section, if a section has no
    //neighbours, there is no Entry in xList for this section

    //search all neighbours of a section
    DirectedSectionPair xLastPair;  //stored the last Entry
    TupleId actTid = 1; //actual TupleId of the section to be processed
    int counterN = 0; //counts the neighbours of one section
    bool flag = true; //toggelt after a pageEntry for section without neighbours
    DirectedSection actDS = DirectedSection (actTid, flag);
    DirectedSection lastDS = DirectedSection(m_pSections->GetNoTuples(),false);

    PageRecord actPage = PageRecord(0);
    int actPageNum = 1;
    int actSlotNum = 1;

    m_xAdjacencyList.clean();

    //*****************************************************************
    // First step to fill the adjacencylist is to identify the number of
    // neighbours to calculate the necessery space for a section.
    for ( size_t i = 0; i < xList.size(); i++ ){
        // Get next entry
        DirectedSectionPair xPair = xList[i];
        //Special treatment for the first Entry
        if(i==0){
            //set last Element
            xLastPair = xPair;
            //collect all Sections without neighbours,
            //there are listed befor the first Entry
            while(xPair.m_iFirstSectionTid != actDS.GetSectionTid()
                        || xPair.m_bFirstUpDown != actDS.GetUpDownFlag()){
                // if there is a section without neighbours put it in pageRecord
                if(xPair.m_iFirstSectionTid != actDS.GetSectionTid()
                        || xPair.m_bFirstUpDown != actDS.GetUpDownFlag()){
                    //get sid, meas1, meas2 and rid
                    Tuple *pSect =
                           m_pSections->GetTuple (actDS.GetSectionTid(),false );
                    double actMeas1 =
                    ((CcReal*)pSect->GetAttribute(SECTION_MEAS1))->GetRealval();
                    double actMeas2 =
                    ((CcReal*)pSect->GetAttribute(SECTION_MEAS2))->GetRealval();
                    int actRid =
                    (( CcInt* )pSect->GetAttribute(SECTION_RID))->GetIntval();
                    double actCost =
                   (( CcReal* )pSect->GetAttribute(SECTION_COST))->GetRealval();
                    double actDuration =
                 ((CcReal*)pSect->GetAttribute(SECTION_DURATION))->GetRealval();
                    int actSid =
                    ((CcInt* )pSect->GetAttribute(SECTION_SID))->GetIntval();
                    // stored Section-Information in page
                    bool success = actPage.SetSectionContent(
                         actDS.GetSectionTid(),actSid, actDS.GetUpDownFlag(),
                         actMeas1,actMeas2,actRid,actCost, actDuration, 0,
                         actSlotNum);
                    if(!success){
                        // page was full, store the pagerecord in adjacencylist
                        m_xAdjacencyList.Append(actPage);
                        // reset actSlotNum to 1 and increment actPageNum
                        actSlotNum = 1;
                        actPageNum++;
                        //create a new Page and stored section information there
                        actPage = PageRecord(0);
                        actPage.SetSectionContent(
                            actDS.GetSectionTid(),actSid, actDS.GetUpDownFlag(),
                            actMeas1,actMeas2, actRid, actCost, actDuration , 0,
                            actSlotNum);
                    }
                    //stored PageNum and SlotNum in Section
                    vector<int> xIndices;
                    vector<Attribute*> xAttrs;
                    if(actDS.GetUpDownFlag()){
                        xIndices.push_back (SECTION_PNO_UP);
                        xIndices.push_back (SECTION_SLOT_UP);
                    }else{
                        xIndices.push_back (SECTION_PNO_DOWN);
                        xIndices.push_back (SECTION_SLOT_DOWN);
                    }
                    xAttrs.push_back ( new CcInt(true,actPageNum) );
                    xAttrs.push_back ( new CcInt(true,actSlotNum) );
                    m_pSections->UpdateTuple(pSect, xIndices, xAttrs);
                    pSect->DeleteIfAllowed();

                    //increment actSlotNum
                    actSlotNum++;
                }
                //set next actDirectedSection to be processed
                if(flag){
                    flag = false;
                } else {
                    flag= true;
                    actTid++;
                }
                actDS = DirectedSection(actTid,flag);
            }
        }

        // Entry in adjacency list if all sections adjacent to one section have
        // been found. This is the case every time the first section changes.
        // Never at the first entry and always at the last.
        if (i == xList.size() -1 ||
            (i != 0 &&
                (xLastPair.m_iFirstSectionTid != xPair.m_iFirstSectionTid
                 || xLastPair.m_bFirstUpDown != xPair.m_bFirstUpDown))){
            if(i==xList.size() -1){
                counterN++; //last entry, the counter must increment
            }
            //all neighbours of a section are found,
            //so put the section on the pageRecord
            //get sid, meas1, meas2 and rid
            Tuple *pSect =
                   m_pSections->GetTuple (xLastPair.m_iFirstSectionTid,false );
            double actMeas1 =
            ((CcReal*) pSect->GetAttribute (SECTION_MEAS1))->GetRealval();;
            double actMeas2 =
            ((CcReal*) pSect->GetAttribute (SECTION_MEAS2))->GetRealval();
            int actRid =
            (( CcInt* )pSect->GetAttribute(SECTION_RID))->GetIntval();
            double actCost =
            (( CcReal* )pSect->GetAttribute(SECTION_COST))->GetRealval();
            double actDuration =
            (( CcReal* )pSect->GetAttribute(SECTION_DURATION))->GetRealval();
            int actSid =
            ((CcInt*)pSect->GetAttribute(SECTION_SID))->GetIntval();
            // stored Section-Information in page
            bool success = actPage.SetSectionContent(
                 xLastPair.m_iFirstSectionTid,actSid, xLastPair.m_bFirstUpDown,
                 actMeas1,actMeas2,actRid,actCost, actDuration, counterN,
                 actSlotNum);
            if(!success){
                // page was full, store the pagerecord in adjacencylist
                m_xAdjacencyList.Append(actPage);
                // reset actSlotNum to 1 and increment actPageNum
                actSlotNum = 1;
                actPageNum++;
                //create a new Page and stored the section information there
                actPage = PageRecord(0);
                actPage.SetSectionContent(
                        xLastPair.m_iFirstSectionTid,actSid,
                        xLastPair.m_bFirstUpDown,actMeas1, actMeas2, actRid,
                        actCost, actDuration , counterN, actSlotNum);
            }
            //stored PageNum and SlotNum in Section
            vector<int> xIndices;
            vector<Attribute*> xAttrs;
            if(xLastPair.m_bFirstUpDown){
                xIndices.push_back (SECTION_PNO_UP);
                xIndices.push_back (SECTION_SLOT_UP);
            }else{
                xIndices.push_back (SECTION_PNO_DOWN);
                xIndices.push_back (SECTION_SLOT_DOWN);
            }
            xAttrs.push_back ( new CcInt(true,actPageNum) );
            xAttrs.push_back ( new CcInt(true,actSlotNum) );
            m_pSections->UpdateTuple(pSect, xIndices, xAttrs);
            pSect->DeleteIfAllowed();

            //increment actSlotNum
            actSlotNum++;

            //check, if sections are between the last and the actual Entry
            //exist, which have no neighbours
            while(xPair.m_iFirstSectionTid != actDS.GetSectionTid()
                    || xPair.m_bFirstUpDown != actDS.GetUpDownFlag()){
               //set next actDirectedSection to be processed
               if(flag){
                    flag = false;
                } else {
                    flag= true;
                    actTid++;
                }
                actDS = DirectedSection(actTid,flag);
                //if there is a section without neighbours put it in pageRecord
                if(xPair.m_iFirstSectionTid != actDS.GetSectionTid()
                    || xPair.m_bFirstUpDown != actDS.GetUpDownFlag()){

                    //get sid, meas1, meas2 and rid
                    Tuple *pSect =
                          m_pSections->GetTuple (actDS.GetSectionTid(),false );
                    double actMeas1 =
                    ((CcReal*)pSect->GetAttribute(SECTION_MEAS1))->GetRealval();
                    double actMeas2 =
                    ((CcReal*)pSect->GetAttribute(SECTION_MEAS2))->GetRealval();
                    int actRid =
                    (( CcInt* )pSect->GetAttribute(SECTION_RID))->GetIntval();
                    double actCost =
                    ((CcReal* )pSect->GetAttribute(SECTION_COST))->GetRealval();
                    double actDuration =
                 ((CcReal*)pSect->GetAttribute(SECTION_DURATION))->GetRealval();
                    int actSid =
                    ((CcInt*)pSect->GetAttribute(SECTION_SID))->GetIntval();
                    // stored Section-Information in page
                    bool success = actPage.SetSectionContent(
                         actDS.GetSectionTid(),actSid, actDS.GetUpDownFlag(),
                         actMeas1, actMeas2,actRid, actCost,actDuration , 0,
                         actSlotNum);
                    if(!success){
                        // page was full, store the pagerecord in adjacencylist
                        m_xAdjacencyList.Append(actPage);
                        // reset actSlotNum to 1 and increment actPageNum
                        actSlotNum = 1;
                        actPageNum++;
                        //create a new Page and stored section information there
                        actPage = PageRecord(0);
                        actPage.SetSectionContent(
                            actDS.GetSectionTid(),actSid, actDS.GetUpDownFlag(),
                            actMeas1, actMeas2,actRid, actCost, actDuration , 0,
                            actSlotNum);
                    }
                    //stored PageNum and SlotNum in Section
                    vector<int> xIndices;
                    vector<Attribute*> xAttrs;
                    if(actDS.GetUpDownFlag()){
                        xIndices.push_back (SECTION_PNO_UP);
                        xIndices.push_back (SECTION_SLOT_UP);
                    }else{
                        xIndices.push_back (SECTION_PNO_DOWN);
                        xIndices.push_back (SECTION_SLOT_DOWN);
                    }
                    xAttrs.push_back ( new CcInt(true,actPageNum) );
                    xAttrs.push_back ( new CcInt(true,actSlotNum) );
                    m_pSections->UpdateTuple(pSect, xIndices, xAttrs);
                    pSect->DeleteIfAllowed();

                    //increment actSlotNum
                    actSlotNum++;
                }
            }
            // reset the counter of neighbours after the Entry in the pageRecord
            counterN =0;
        }

        // Check if entry allready exists in list. As the list is sorted it
        // has to be the entry before.
        if ( i == 0 ||
            xLastPair.m_iFirstSectionTid != xPair.m_iFirstSectionTid ||
            xLastPair.m_bFirstUpDown != xPair.m_bFirstUpDown ||
            xLastPair.m_iSecondSectionTid != xPair.m_iSecondSectionTid ||
            xLastPair.m_bSecondUpDown != xPair.m_bSecondUpDown )
        {
            counterN++; // the counter of neighbours must increment
        }

        //check, if sections after the last Entry exist, that have no neighbours
        if (i == xList.size() -1){

            while(lastDS.GetSectionTid() != actDS.GetSectionTid()
                  || lastDS.GetUpDownFlag() != actDS.GetUpDownFlag()){
                //set next actDirectedSection to be processed
                if(flag){
                    flag = false;
                } else {
                    flag= true;
                    actTid++;
                }
                actDS = DirectedSection(actTid,flag);
                // if there is a section without neighbours put it in pageRecord
                if(lastDS.GetSectionTid() != actDS.GetSectionTid()
                    || lastDS.GetUpDownFlag() != actDS.GetUpDownFlag()){
                    //get sid, meas1,meas2and rid
                    Tuple *pSect =
                           m_pSections->GetTuple (actDS.GetSectionTid(),false );
                    double actMeas1 =
                    ((CcReal*)pSect->GetAttribute(SECTION_MEAS1))->GetRealval();
                    double actMeas2 =
                    ((CcReal*)pSect->GetAttribute(SECTION_MEAS2))->GetRealval();
                    int actRid =
                    (( CcInt* )pSect->GetAttribute(SECTION_RID))->GetIntval();
                    double actCost =
                    (( CcReal*)pSect->GetAttribute(SECTION_COST))->GetRealval();
                    double actDuration =
                 ((CcReal*)pSect->GetAttribute(SECTION_DURATION))->GetRealval();
                    int actSid =
                    ((CcInt*)pSect->GetAttribute(SECTION_SID))->GetIntval();
                    // stored Section-Information in page
                    bool success = actPage.SetSectionContent(
                         actDS.GetSectionTid(),actSid, actDS.GetUpDownFlag(),
                         actMeas1, actMeas2, actRid, actCost, actDuration , 0,
                         actSlotNum);
                    if(!success){
                        // page was full, store the pagerecord in adjacencylist
                        m_xAdjacencyList.Append(actPage);
                        // reset actSlotNum to 1 and increment actPageNum
                        actSlotNum = 1;
                        actPageNum++;
                        //create a new Page and stored section information there
                        actPage = PageRecord(0);
                        actPage.SetSectionContent(
                            actDS.GetSectionTid(),actSid, actDS.GetUpDownFlag(),
                            actMeas1, actMeas2, actRid, actCost, actDuration ,0,
                            actSlotNum);
                    }
                    //stored PageNum and SlotNum in Section
                    vector<int> xIndices;
                    vector<Attribute*> xAttrs;
                    if(actDS.GetUpDownFlag()){
                        xIndices.push_back (SECTION_PNO_UP);
                        xIndices.push_back (SECTION_SLOT_UP);
                    }else{
                        xIndices.push_back (SECTION_PNO_DOWN);
                        xIndices.push_back (SECTION_SLOT_DOWN);
                    }
                    xAttrs.push_back ( new CcInt(true,actPageNum) );
                    xAttrs.push_back ( new CcInt(true,actSlotNum) );
                    m_pSections->UpdateTuple(pSect, xIndices, xAttrs);
                    pSect->DeleteIfAllowed();
                    //increment actSlotNum
                    actSlotNum++;

                }
            }
        }
        // set last Entry
        xLastPair = xPair;
    }
    if(xList.size()!= 0){
        // store last Page in the AdjacencyList
        m_xAdjacencyList.Append(PageRecord(actPage));
    }
    m_xAdjacencyList.TrimToSize();

    //
    //*****************************************************************
    // Second step to fill the adjacencylist is to set all pagenumbers and
    // Slotindex for the neighbours of a section. Because that information
    // is collected in the first Step.

    actPageNum =0;              // reset old used variable for actual PageNumber
    actSlotNum =0;              // reset old used variable for actual SlotNumber
    actPage = PageRecord(0);    // reset old used variable for actual Page
    DirectedSectionPair actPair;// actual read sectionPair from xList
    DirectedSectionPair lastPair;
    int nCounter=0;// counts the proceeded neighbours start even by 0
    int actNPage,actNSlot;     // actual PageNR and SlotNR of a Neighboursection
    TupleId nSTid;              // sectionid and UpDownFlag of a neighbour
    bool nUpD;
    size_t actIndex = 0;           // actual Index of the xList

    while(actIndex < xList.size()){
        //get actual element
        actPair = xList[actIndex];
        // get actual Tuple from section relation
        Tuple *pSect = m_pSections->GetTuple (actPair.m_iFirstSectionTid,false);
        // get page and slotnumber of this DirectionPair
        if (actPair.m_bFirstUpDown){
        actPageNum =((CcInt*)pSect->GetAttribute(SECTION_PNO_UP))->GetIntval();
        actSlotNum =((CcInt*)pSect->GetAttribute(SECTION_SLOT_UP))->GetIntval();
        }else{
        actPageNum=((CcInt*)pSect->GetAttribute(SECTION_PNO_DOWN))->GetIntval();
       actSlotNum=((CcInt*)pSect->GetAttribute(SECTION_SLOT_DOWN))->GetIntval();
        }
        // get page, where the actual section is stored
        m_xAdjacencyList.Get(actPageNum-1, actPage);
        nCounter =0;

        // at first element strored this as last element
        if(actIndex == 0){
            lastPair = actPair;
        }
        while(actIndex==0 ||
             (actPair.m_iFirstSectionTid== lastPair.m_iFirstSectionTid
              && actPair.m_bFirstUpDown==lastPair.m_bFirstUpDown)){
          if(actIndex !=0 &&
          !(lastPair.m_iSecondSectionTid == xList[actIndex].m_iSecondSectionTid
          && lastPair.m_bSecondUpDown ==xList[actIndex].m_bSecondUpDown)){
            // read nSid and nUpD form xList
            nSTid = lastPair.m_iSecondSectionTid;
            nUpD = lastPair.m_bSecondUpDown;

            // for each Entry stored Information
            Tuple *pNSect = m_pSections->GetTuple (nSTid,false );
            if (nUpD){
            actNPage =
              (( CcInt* )pNSect->GetAttribute(SECTION_PNO_UP))->GetIntval();
            actNSlot =
              (( CcInt* )pNSect->GetAttribute(SECTION_SLOT_UP))->GetIntval();
            }else{
            actNPage =
              (( CcInt* )pNSect->GetAttribute(SECTION_PNO_DOWN))->GetIntval();
            actNSlot =
              (( CcInt* )pNSect->GetAttribute(SECTION_SLOT_DOWN))->GetIntval();
            }
            pNSect->DeleteIfAllowed();

             // Update the Information on the page
             actPage.SetSectionNeighbour(actSlotNum,nCounter,actNPage,actNSlot);
             nCounter++;
            }
            lastPair = xList[actIndex];
            actIndex++;
        }

        // Delete section Tupel if allowed
        pSect->DeleteIfAllowed();

        //page stored back in the adjacencyList
        // get page, where the actual section is stored
        m_xAdjacencyList.Put(actPageNum-1, actPage);

        //Reset all used values
        actPageNum=0;
        actSlotNum=0;
        actPage=PageRecord(0);
    }
   // test output
   //of the content from xList, m_xAdjacencyList, m_xSubAdjacencyList
   // and all neighbourSection which are issued from GetAdjacentSections()
  /* cout << "Output of xList:" <<endl;
  DirectedSectionPair actDSP;
  for(size_t i=0; i< xList.size();i++){
      cout << "[(" << xList[i].m_iFirstSectionTid << ","
           << xList[i].m_bFirstUpDown << ")"
           << "(" << xList[i].m_iSecondSectionTid << ","
           << xList[i].m_bSecondUpDown << ")]" <<endl;
  }
  //print all information which are stored in the PageRecord
  PrintAdjacentSectionInfo();


  cout << "output all neighbours with GetAdjacentSections:" << endl;
  vector<DirectedSection> actVDS;
  for (int i=1; i<= m_pSections->GetNoTuples();i++){
      actVDS.clear();
      cout << "Sec:(" << i <<",0): [";
      GetAdjacentSections(i,false,actVDS);
      for(size_t k=0; k<actVDS.size();k++){
        cout <<"("<< actVDS[k].GetSectionTid() << ","
             << actVDS[k].GetUpDownFlag() <<")";
      }
      cout << "]" << endl;

      actVDS.clear();
      cout << "Sec:(" << i <<",1): [";
      GetAdjacentSections(i,true,actVDS);
      for(size_t k=0; k<actVDS.size();k++){
        cout <<"("<< actVDS[k].GetSectionTid() << ","
             << actVDS[k].GetUpDownFlag() <<")";
      }
      cout << "]" << endl;
  }

  //statistic output of the numbers of neighbours
  CountNeighbours(xList); */
}

/*
Method ~FillAdjacencyPair~

Build vector of directed section pairs.

*/

void Network::FillAdjacencyPair ( TupleId in_pFirstSection,
                                  bool in_bFirstUp,
                                  TupleId in_pSecondSection,
                                  bool in_bSecondUp,
                                  ConnectivityCode in_xCc,
                                  Transition in_xTransition,
                                  vector<DirectedSectionPair> &inout_xPairs )
{
  if ( in_pFirstSection != 0 &&
          in_pSecondSection != 0 &&
          in_xCc.IsPossible ( in_xTransition ) )
  {
    inout_xPairs.push_back ( DirectedSectionPair ( in_pFirstSection,
                             in_bFirstUp,
                             in_pSecondSection,
                             in_bSecondUp ) );
  }
}


/*
Method ~PrintAdjacentSectionInfo~

prints informations of ~sections~ which are stored in a ~PageRecord~.

first output: all ~sections~ with there sectionId and cost-value

second output: all ~routes~ iterated over the startposId

third output: all neighbours-information of the ~sections~

Sec (SectionTupelId,UpDownFlag):[(NeighborSectionTId, UpDownFlag, Meas1, Meas2,
RId, Cost, Time)(..)]

*/
bool Network::PrintAdjacentSectionInfo(){

    cout << "output of all Sections:  (Section_SId,Section_Cost)"<< endl;
    for (int i =1; i<=m_pSections->GetNoTuples();i++){
        Tuple *n = m_pSections->GetTuple(i,false);
        cout <<"("<< ((CcInt*)n->GetAttribute(SECTION_SID))->GetIntval() << ","
          << ((CcReal*)n->GetAttribute(SECTION_COST))->GetRealval()<< ")"<<endl;
        n->DeleteIfAllowed();
    }

    cout << "ROUTES over startposid: (Routes_Id, Routes_StartposId)" << endl;
    BTreeIterator* pRoutesIt = m_pBTreeRoutesByStartposId->SelectAll();
    Tuple* pRoute;
    while ( pRoutesIt->Next() )
    {
      pRoute = m_pRoutes->GetTuple(pRoutesIt->GetId(),false);
      cout <<"("<< ((CcInt*)pRoute->GetAttribute(ROUTE_ID))->GetIntval() << ","
        << ((CcInt*)pRoute->GetAttribute(ROUTE_STARTPOS_ID))->GetIntval()<< ")"
        <<endl;
      pRoute->DeleteIfAllowed();
    }
    delete pRoutesIt;


  cout << "output all neighbours with GetAdjacentSectionsInfo" << endl;
  cout << "Sec:(SectionTupelId,UpDownFlag):[(NeighborSectionTId,"
       << " UpDownFlag, Meas1, Meas2, RId, Cost, Time)(..)]" << endl;
  vector<DirectedSectionInfo> actVDSI;
  BTreeIterator* pSectionIt;
  pSectionIt = m_pBTreeSections->SelectAll();
  while ( pSectionIt->Next() )
  {
    actVDSI.clear();
    TupleId sTid = pSectionIt->GetId();
    cout << "Sec:(" << sTid <<",0): [";
    GetAdjacentSectionsInfo(sTid,false,actVDSI);
    for(size_t k=0; k<actVDSI.size();k++){
      cout <<"("<< actVDSI[k].GetSectionTid()<< ","<< actVDSI[k].GetUpDownFlag()
           <<","<< actVDSI[k].GetMeas1() << "," << actVDSI[k].GetMeas2() << ","
           << actVDSI[k].GetRid()<<","<< actVDSI[k].GetCost() <<","
           << actVDSI[k].GetDuration()<< ")";
    }
    cout << "]" << endl;
    actVDSI.clear();
    cout << "Sec:(" << sTid <<",1): [";
    GetAdjacentSectionsInfo(sTid,true,actVDSI);
    for(size_t k=0; k<actVDSI.size();k++){
      cout <<"("<< actVDSI[k].GetSectionTid()<< ","<< actVDSI[k].GetUpDownFlag()
           <<","<< actVDSI[k].GetMeas1() << "," << actVDSI[k].GetMeas2() << ","
           << actVDSI[k].GetRid()<<","<< actVDSI[k].GetCost() <<","
           << actVDSI[k].GetDuration()  <<")";
    }
    cout << "]" << endl;

  }
  delete pSectionIt;
  return true;
}

/*
Method ~CountNeighbours~

calculate the numbers of neighbours of all ~sections~. Only used for internal
debug informations.

*/
void Network::CountNeighbours(vector<DirectedSectionPair> in_xList){
    cout << "count all neighbours of all sections:" << endl;
    //Counter for neighbours, if more then nine, the counter a10 is increased
    int a0=0,a1=0,a2=0,a3=0,a4=0,a5=0,a6=0,a7=0,a8=0,a9=0,a10=0;
    int actCounter=0;
    DirectedSectionPair xLastPair;

    //determin the number of neighbours of the sections
    actCounter=0;
    //pass through all pairs
    for (size_t i=0; i<in_xList.size(); i++){
        //get next
        DirectedSectionPair xPair = in_xList[i];

        //at the first element,remember it as the last pair
        if(i==0){
            xLastPair = xPair;
            actCounter++;
        }
        if ( i == in_xList.size() -1 ||
            (i != 0 &&
                ( xLastPair.m_iFirstSectionTid != xPair.m_iFirstSectionTid ||
                  xLastPair.m_bFirstUpDown != xPair.m_bFirstUpDown
                )
            )
        ){
            switch(actCounter){
                case 1: a1++;break;
                case 2: a2++;break;
                case 3: a3++;break;
                case 4: a4++;break;
                case 5: a5++;break;
                case 6: a6++;break;
                case 7: a7++;break;
                case 8: a8++;break;
                case 9: a9++;break;
                default: a10++;
            }
            actCounter = 0;
        }
        if ( i == 0 ||

            xLastPair.m_iFirstSectionTid != xPair.m_iFirstSectionTid ||
            xLastPair.m_bFirstUpDown != xPair.m_bFirstUpDown ||
            xLastPair.m_iSecondSectionTid != xPair.m_iSecondSectionTid ||
            xLastPair.m_bSecondUpDown != xPair.m_bSecondUpDown ){
                actCounter++;
        }
        xLastPair = xPair;
    }

    //determine sections without neighbours
    a0 = (m_pSections->GetNoTuples() *2) -(a1+a2+a3+a4+a5+a6+a7+a8+a9+a10);

    // output of the results
    cout << "sections without neighbours: " << a0 << endl;
    cout << "sections with one neighbour: " << a1 << endl;
    cout << "sections with two neighbours: " << a2 << endl;
    cout << "sections with three neighbours: " << a3 << endl;
    cout << "sections with four neighbours: " << a4<< endl;
    cout << "sections with five neighbours: " << a5 << endl;
    cout << "sections with six neighbours: " << a6 << endl;
    cout << "sections with seven neighbours: " << a7 << endl;
    cout << "sections with eight neighbours: " << a8 << endl;
    cout << "sections with nine neighbours: " << a9 << endl;
    cout << "sections with more than nine neighbours : " << a10 << endl;

}





/*
Method ~InShortestPath~

*/
bool Network::InShortestPath ( GPoint*start,GPoint *to, GLine *result )
{
  GPoint* end = new GPoint ( *to );//copy the gpoint
  result->Clear();
  if ( start == 0 || end == 0 || !start->IsDefined() ||
          !end->IsDefined() )
  {
    sendMessage ( "Both gpoints must exist and be defined." );
    result->SetDefined ( false );
    end->DeleteIfAllowed();
    return false;
  }
  // Check wether both points belong to the same network
  if ( start->GetNetworkId() != end->GetNetworkId() )
  {
    sendMessage ( "Both gpoints belong to different networks." );
    result->SetDefined ( false );
    end->DeleteIfAllowed();
    return false;
  }

  result->SetNetworkId ( GetId() );

  // Get sections where the path should start or end
  Tuple* startSection = GetSectionOnRoute ( start );
  if ( startSection == 0 )
  {
    sendMessage ( "Starting GPoint not found in network." );
    result->SetDefined ( false );
    end->DeleteIfAllowed();
    return false;
  }
  Tuple* endSection = GetSectionOnRoute ( end );
  if ( endSection == 0 )
  {
    sendMessage ( "End GPoint not found in network." );
    startSection->DeleteIfAllowed();
    result->SetDefined ( false );
    end->DeleteIfAllowed();
    return false;
  }
////////////////////////////////////////////////////
  bool junctionpoint = false;
  Point* endp = new Point();
  GetPointOnRoute ( end,endp ); //end point
  vector<JunctionSortEntry> juns;
  CcInt* routeid = new CcInt ( true,end->GetRouteId() );
  GetJunctionsOnRoute ( routeid,juns );
  for ( unsigned int i = 0;i < juns.size();i++ )
  {
    Tuple* t = juns[i].m_pJunction;
    if ( ( ( CcInt* ) t->GetAttribute ( JUNCTION_ROUTE1_ID ) )->GetIntval() ==
            end->GetRouteId() &&
            fabs ( ( ( CcReal* ) t->GetAttribute ( JUNCTION_ROUTE1_MEAS )
)->GetRealval()-
                   end->GetPosition() ) < 0.1 )
      junctionpoint = true;
    if ( ( ( CcInt* ) t->GetAttribute ( JUNCTION_ROUTE2_ID ) )->GetIntval() ==
            end->GetRouteId() &&
            fabs ( ( ( CcReal* ) t->GetAttribute ( JUNCTION_ROUTE2_MEAS )
)->GetRealval()-
                   end->GetPosition() ) < 0.1 )
      junctionpoint = true;
  }
  vector<TupleId> secjunid;
  if ( junctionpoint )  //it is a junction point
  {
    vector<DirectedSection> sectionlist;
    if ( fabs ( end->GetPosition()-
                ( ( CcReal* ) endSection->GetAttribute ( SECTION_MEAS1 )
)->GetRealval() ) < 0.1 )
      GetAdjacentSections ( endSection->GetTupleId(),false,sectionlist );
    else
      GetAdjacentSections ( endSection->GetTupleId(),true,sectionlist );
    for ( unsigned int i = 0;i < sectionlist.size();i++ )
    {
      if ( sectionlist[i].GetSectionTid() != endSection->GetTupleId() )
        secjunid.push_back ( sectionlist[i].GetSectionTid() );
    }
  }
  endp->DeleteIfAllowed();
  routeid->DeleteIfAllowed();
/////////////////////////////////////////////////////
// Calculate the shortest path using dijkstras algorithm.



  TupleId startSectTID = startSection->GetTupleId();
  TupleId lastSectTID = endSection->GetTupleId();

  if ( startSectTID == lastSectTID )
  {
    result->AddRouteInterval ( start->GetRouteId(), start->GetPosition(),
                               end->GetPosition() );
  }
  else
  {

//Initialize PriorityQueue

    PrioQueue *prioQ = new PrioQueue ( 0 );
    SectIDTree *visitedSect = 0;
    double sectMeas1 =
        ( ( CcReal* ) startSection->GetAttribute ( SECTION_MEAS1 )
)->GetRealval();
    double sectMeas2 =
        ( ( CcReal* ) startSection->GetAttribute ( SECTION_MEAS2 )
)->GetRealval();
    double dist = 0.0;
    vector<DirectedSection> adjSectionList;
    adjSectionList.clear();
    if ( start->GetSide() == 0 )
    {
      dist = start->GetPosition() - sectMeas1;
      GetAdjacentSections ( startSectTID, false, adjSectionList );
      SectIDTree *startTree = new SectIDTree ( startSectTID,
              ( TupleId ) numeric_limits<long>::max(),
              false,
              numeric_limits<int>::max() );
      visitedSect = startTree;
      for ( size_t i = 0;  i < adjSectionList.size(); i++ )
      {
        DirectedSection actNextSect = adjSectionList[i];
        if ( actNextSect.GetSectionTid() != startSectTID )
        {
          PQEntry *actEntry = new PQEntry ( actNextSect.GetSectionTid(), dist,
                                            actNextSect.GetUpDownFlag(),
                                            startSectTID );
          prioQ->Insert ( *actEntry, visitedSect ) ;
          delete actEntry;
        }
      }
      adjSectionList.clear();
    }
    else
    {
      if ( start->GetSide() == 1 )
      {
        dist = sectMeas2 - start->GetPosition();
        SectIDTree *startTree = new SectIDTree ( startSectTID,
                ( TupleId ) numeric_limits<long>::max(),
                true,
                numeric_limits<int>::max() );
        visitedSect = startTree;
        GetAdjacentSections ( startSectTID, true, adjSectionList );
        for ( size_t i = 0;  i < adjSectionList.size(); i++ )
        {
          DirectedSection actNextSect = adjSectionList[i];
          if ( actNextSect.GetSectionTid() != startSectTID )
          {
            PQEntry *actEntry = new PQEntry ( actNextSect.GetSectionTid(), dist,
                                              actNextSect.GetUpDownFlag(),
                                              startSectTID );
            prioQ->Insert ( *actEntry, visitedSect );
            delete actEntry;
          }
        }
        adjSectionList.clear();
      }
      else
      {
        dist = start->GetPosition() - sectMeas1;
        GetAdjacentSections ( startSectTID, false, adjSectionList );
        bool first = true;
        for ( size_t i = 0;  i < adjSectionList.size(); i++ )
        {
          DirectedSection actNextSect = adjSectionList[i];
          if ( actNextSect.GetSectionTid() != startSectTID )
          {
            if ( first )
            {
              first = false;
              SectIDTree *startTree = new SectIDTree ( startSectTID,
                      ( TupleId ) numeric_limits<long>::max(),
                      false,
                      numeric_limits<int>::max() );
              visitedSect = startTree;
            }
            PQEntry *actEntry = new PQEntry ( actNextSect.GetSectionTid(), dist,
                                              actNextSect.GetUpDownFlag(),
                                              startSectTID );
            prioQ->Insert ( *actEntry, visitedSect );
            delete actEntry;
          }
        }
        adjSectionList.clear();
        dist = sectMeas2 -start->GetPosition();
        GetAdjacentSections ( startSectTID, true, adjSectionList );
        for ( size_t i = 0;  i < adjSectionList.size(); i++ )
        {
          DirectedSection actNextSect = adjSectionList[i];
          if ( actNextSect.GetSectionTid() != startSectTID )
          {
            if ( first )
            {
              first = false;
              SectIDTree *startTree = new SectIDTree ( startSectTID,
                      ( TupleId ) numeric_limits<long>::max(),
                      true,
                      numeric_limits<int>::max() );
              visitedSect = startTree;
            }
            PQEntry *actEntry = new PQEntry ( actNextSect.GetSectionTid(), dist,
                                              actNextSect.GetUpDownFlag(),
                                              startSectTID );
            prioQ->Insert ( *actEntry, visitedSect );
            delete actEntry;
          }
        }
        adjSectionList.clear();
      }
    }
// Use priorityQueue to find shortestPath.

    PQEntry *actPQEntry;
    bool found = false;
    while ( !prioQ->IsEmpty() && !found )
    {
      actPQEntry = prioQ->GetAndDeleteMin ( visitedSect );
      Tuple *actSection = GetSection ( actPQEntry->sectID );
      sectMeas1 =
          ( ( CcReal* ) actSection->GetAttribute ( SECTION_MEAS1 )
)->GetRealval();
      sectMeas2 =
          ( ( CcReal* ) actSection->GetAttribute ( SECTION_MEAS2 )
)->GetRealval();
      dist = actPQEntry->distFromStart + fabs ( sectMeas2 - sectMeas1 );

//////////////////////////////////////
      if ( junctionpoint )   //end point is a junction point
      {
        for ( unsigned int i = 0;i < secjunid.size();i++ )
        {
          if ( secjunid[i] == actPQEntry->sectID )
          {
            lastSectTID = actPQEntry->sectID;
            Tuple* sect = GetSection ( lastSectTID );
            double m1 =
                ( ( CcReal* ) sect->GetAttribute ( SECTION_MEAS1 )
)->GetRealval();
            double m2 =
                ( ( CcReal* ) sect->GetAttribute ( SECTION_MEAS2 )
)->GetRealval();
            if ( actPQEntry->upDownFlag )
            {
              GPoint* temp = new GPoint ( true,end->GetNetworkId(),
                                          end->GetRouteId(),m2,None );
              *end = *temp;
              temp->DeleteIfAllowed();
            }
            else
            {
              GPoint* temp = new GPoint ( true,end->GetNetworkId(),
                                          end->GetRouteId(),m1,None );
              *end = *temp;
              temp->DeleteIfAllowed();
            }
            sect->DeleteIfAllowed();
            break;
          }
        }
      }
////////////////////////////////////

      if ( actPQEntry->sectID != lastSectTID )
      {
//Search further in the network unitl reached last section.
//Get adjacent sections and insert into priority Queue.

        adjSectionList.clear();
        GetAdjacentSections ( actPQEntry->sectID,
                              actPQEntry->upDownFlag,
                              adjSectionList );
        for ( size_t i = 0; i <adjSectionList.size();i++ )
        {
          DirectedSection actNextSect = adjSectionList[i];
          if ( actNextSect.GetSectionTid() != actPQEntry->sectID )
          {
            PQEntry *actEntry = new PQEntry ( actNextSect.GetSectionTid(),
                                              dist,
                                              actNextSect.GetUpDownFlag(),
                                              actPQEntry->sectID );
            prioQ->Insert ( *actEntry, visitedSect );
            delete actEntry;
          }
        }
        delete actPQEntry;
        actSection->DeleteIfAllowed();
      }
      else
      {

// Shortest Path found.
// Compute last route interval and resulting gline.

        found = true;
        double startRI, endRI;
        int actRouteId =
            ( ( CcInt* ) actSection->GetAttribute ( SECTION_RID )
)->GetIntval();
        if ( actPQEntry->upDownFlag == true )
        {
          startRI = sectMeas1;
          endRI = end->GetPosition();
        }
        else
        {
          startRI = sectMeas2;
          endRI = end->GetPosition();
        }

        actSection->DeleteIfAllowed();

//Get the sections used for shortest path and write them in right
//order (from start to end ) in the resulting gline. Because dijkstra gives
//the sections from end to start we first have to put the result sections on a
//stack to turn in right order.

        RIStack *riStack = new RIStack ( actRouteId, startRI, endRI );
        TupleId lastSectId = actPQEntry->sectID;
        SectIDTree *pElem = visitedSect->Find ( actPQEntry->beforeSectID );
        bool end = false;
        bool upDown;
        //   if (startRI >= endRI) upDown = false;
        if ( startRI > endRI || fabs ( startRI-endRI ) < 0.1 ) upDown = false;
        else upDown = true;
        while ( !end )
        {
          //GetSection
          actSection = GetSection ( pElem->sectID );
          actRouteId =
              ( ( CcInt* ) actSection->GetAttribute ( SECTION_RID )
)->GetIntval();
          sectMeas1 =
              ( ( CcReal* ) actSection->GetAttribute ( SECTION_MEAS1 )
)->GetRealval();
          sectMeas2 =
              ( ( CcReal* ) actSection->GetAttribute ( SECTION_MEAS2 )
)->GetRealval();
          upDown = pElem->upDownFlag;
          if ( pElem->sectID != startSectTID )
          {
            if ( upDown )
              riStack->Push ( actRouteId, sectMeas1, sectMeas2, riStack );
            else
              riStack->Push ( actRouteId, sectMeas2, sectMeas1, riStack );
            lastSectId = pElem->sectID;
            pElem = visitedSect->Find ( pElem->beforeSectId );
          }
          else
          {
            end = true;
            GetAdjacentSections ( startSectTID, true, adjSectionList );
            size_t i = 0;
            bool stsectfound = false;
            while ( i < adjSectionList.size() && !stsectfound )
            {
              DirectedSection adjSection = adjSectionList[i];
              if ( adjSection.GetSectionTid() == lastSectId )
              {
                if ( fabs ( start->GetPosition()-sectMeas2 ) > 0.1 )
                {
                  stsectfound = true;
                  riStack->Push ( actRouteId, start->GetPosition(), sectMeas2,
                                  riStack );
                  end = true;
                }
              }
              i++;
            }
            adjSectionList.clear();
            if ( !stsectfound )
            {
              GetAdjacentSections ( startSectTID, false,
                                    adjSectionList );
              i = 0;
              while ( i < adjSectionList.size() && !stsectfound )
              {
                DirectedSection adjSection = adjSectionList[i];
                if ( adjSection.GetSectionTid() == lastSectId )
                {
                  if ( fabs ( start->GetPosition() - sectMeas1 ) > 0.1 )
                  {
                    stsectfound = true;
                    riStack->Push ( actRouteId, start->GetPosition(), sectMeas1,
                                    riStack );
                    end = true;
                  }
                }
                i++;
              }
              adjSectionList.clear();
            }
          }
        }
        // Cleanup and return result
        riStack->StackToGLine ( result );
        riStack->RemoveStack();
        delete actPQEntry;
      }
    }
    visitedSect->Remove();
    prioQ->Destroy();
    delete prioQ;
  }
  startSection->DeleteIfAllowed();
  endSection->DeleteIfAllowed();
  result->SetSorted ( false );
  result->SetDefined ( true );
  result->TrimToSize();
  end->DeleteIfAllowed();
  return true;
};

/*
Method ~FindSP~

*/
void Network::FindSP ( TupleId j1,TupleId j2,double& length,GLine* res )
{
  res->SetNetworkId ( GetId() );
  for ( int i = 1; i <= alldistance->GetNoTuples();i++ )
  {
    Tuple* tuple = alldistance->GetTuple ( i, false );
    TupleId jun1 = ( ( CcInt* ) tuple->GetAttribute ( 0 ) )->GetIntval();
    TupleId jun2 = ( ( CcInt* ) tuple->GetAttribute ( 1 ) )->GetIntval();
    if ( ( jun1 == j1 && jun2 == j2 ) ||
            ( jun1 ==j2 && jun2 == j1 ) )
    {
      length = ( ( CcReal* ) tuple->GetAttribute ( 2 ) )->GetRealval();
      GLine* gline = ( GLine* ) tuple->GetAttribute ( 3 );
      *res = *gline;
      tuple->DeleteIfAllowed();
      break;
    }
    tuple->DeleteIfAllowed();
  }
  res->TrimToSize();
}

/*
Method ~FillDistanceStorage~

*/

void Network::FillDistanceStorage()
{
  ListExpr xType;
  nl->ReadFromString ( distancestorageTypeInfo,xType );
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType ( xType );
  alldistance = new Relation ( xNumType );
  //store the distance between each two junction points

  for ( int i = 1;i <= m_pJunctions->GetNoTuples();i++ )
  {
    for ( int j = i+1; j <= m_pJunctions->GetNoTuples();j++ )
    {
      Tuple* jun1 = m_pJunctions->GetTuple ( i, false );
      Tuple* jun2 = m_pJunctions->GetTuple ( j , false);
      int rid1 = ( ( CcInt* ) jun1->GetAttribute ( JUNCTION_ROUTE1_ID )
)->GetIntval();
      int rid2 = ( ( CcInt* ) jun2->GetAttribute ( JUNCTION_ROUTE1_ID )
)->GetIntval();
      float pos1 =
          ( ( CcReal* ) jun1->GetAttribute ( JUNCTION_ROUTE1_MEAS )
)->GetRealval();
      float pos2 =
          ( ( CcReal* ) jun2->GetAttribute ( JUNCTION_ROUTE1_MEAS )
)->GetRealval();
      Side side = None;
      Point* p1 = ( Point* ) jun1->GetAttribute ( JUNCTION_POS );
      Point* p2 = ( Point* ) jun2->GetAttribute ( JUNCTION_POS );
      if ( fabs ( p1->GetX()-p2->GetX() ) < 0.1 &&
              fabs ( p1->GetY()-p2->GetY() ) < 0.1 ) //different junction point
        continue;
      GPoint* gp1 = new GPoint ( true,GetId(),rid1,pos1,side );
      GPoint* gp2 = new GPoint ( true,GetId(),rid2,pos2,side );
      Tuple* tuple = new Tuple ( nl->Second ( xNumType ) );
      tuple->PutAttribute ( 0,new TupleIdentifier ( true,i ) );
      tuple->PutAttribute ( 1,new TupleIdentifier ( true,j ) );
      GLine* gline = new GLine ( 0 );
      assert ( InShortestPath ( gp1,gp2,gline ) );
      tuple->PutAttribute ( 2,new CcReal ( true,gline->GetLength() ) );
      GLine* temp = new GLine ( 0 );
      temp->SetNetworkId ( gline->GetNetworkId() );
      RouteInterval ri;
      gline->Get ( 0,ri );
      temp->AddRouteInterval ( ri );//head
      gline->Get ( gline->Size()-1,ri );
      temp->AddRouteInterval ( ri );//tail
      tuple->PutAttribute ( 3,new GLine ( temp ) );
      temp->DeleteIfAllowed();
      gline->DeleteIfAllowed();
      alldistance->AppendTuple ( tuple );
      tuple->DeleteIfAllowed();
      gp1->DeleteIfAllowed();
      gp2->DeleteIfAllowed();
    }
  }
}

/*
Method ~UpdateSectionCost~

update the cost of a given ~section~

returns ~true~, if the update was successful, otherwise ~false~

*/
bool Network::UpdateSectionCost(int sectId,double newCost){
    BTreeIterator* pSectIter;
    CcInt* ciSectId = new CcInt(true,sectId);
    pSectIter = m_pBTreeSections->ExactMatch (ciSectId);
    delete ciSectId;
    if(pSectIter->Next()){
        //get act Tuple
        Tuple *actSect = 0;
        actSect = m_pSections->GetTuple(pSectIter->GetId(), false);

        //save cost in the section relation
        vector<int> xIndices;
        vector<Attribute*> xAttrs;
        xIndices.push_back ( SECTION_COST );
        xAttrs.push_back ( new CcReal ( true, newCost ) );
        m_pSections->UpdateTuple(actSect, xIndices, xAttrs);



        //save cost in the PageRecord of the section
        // read pages und Slots
        int pageUp =
          ((CcInt* )actSect->GetAttribute(SECTION_PNO_UP))->GetIntval();
        int slotUp =
          ((CcInt* )actSect->GetAttribute(SECTION_SLOT_UP))->GetIntval();
        int pageDown =
          ((CcInt* )actSect->GetAttribute(SECTION_PNO_DOWN))->GetIntval();
        int slotDown =
          ((CcInt* )actSect->GetAttribute(SECTION_SLOT_DOWN))->GetIntval();

        //get the page and save cost for both entries
        PageRecord actPage =PageRecord(0);
        m_xAdjacencyList.Get(pageUp-1, actPage);
        actPage.SetSectionCost(slotUp, newCost);
        if(pageDown == pageUp){
            actPage.SetSectionCost(slotDown, newCost);
            m_xAdjacencyList.Put(pageUp-1, actPage);
        }else{
            m_xAdjacencyList.Put(pageUp-1, actPage);
            m_xAdjacencyList.Get(pageDown-1, actPage);
            actPage.SetSectionCost(slotDown, newCost);
            m_xAdjacencyList.Put(pageDown-1, actPage);
        }


        //clean up
        actSect->DeleteIfAllowed();
        delete pSectIter;
        return true;
    }
    else {
        //errormessage
        sendMessage ( "the section with the given SectId must exist." );
        cout << "the section " <<sectId <<"  doesn't exist." <<endl;
        //clean up
        delete pSectIter;
        return false;
    }
}

/*
Method ~UpdateMoreSectionCost~

update the cost of some ~sections~ which are in the given relation.

returns ~true~, if the update was successful, otherwise ~false~

*/
bool Network::UpdateMoreSectionCost(const Relation* in_pCosts){
    //DEBUG PrintAdjacentSectionInfo();
    // check input
    if(in_pCosts->GetNoTuples() >0){
        //for each tuple of the Relation
        for(int i=1; i<=in_pCosts->GetNoTuples();i++){
            //Get sid and cost
            Tuple *actTuple = 0;
            actTuple = in_pCosts->GetTuple(i, false);
            int sid = ((CcInt*)actTuple->GetAttribute(0))->GetIntval();
            double cost = ((CcReal*)actTuple->GetAttribute(1))->GetRealval();

            //call the function UpdateSectionCost(sid,cost)
            UpdateSectionCost(sid,cost);

            // clean up
            actTuple->DeleteIfAllowed();
        }
        //DEBUG PrintAdjacentSectionInfo();
        return true;
    }else{
        sendMessage ( "the relation is empty." );
        cout << "the relation is empty." <<endl;
        return false;
    }

}

/*
Method ~UpdateSectionDuration~

update the duration of a given ~section~

returns ~true~, if the update was successful, otherwise ~false~

*/
bool Network::UpdateSectionDuration(int sectId,double newDuration){
    BTreeIterator* pSectIter;
    CcInt* ciSectId = new CcInt(true,sectId);
    pSectIter = m_pBTreeSections->ExactMatch (ciSectId);
    delete ciSectId;
    if(pSectIter->Next()){
        //get act Tuple
        Tuple *actSect = 0;
        actSect = m_pSections->GetTuple(pSectIter->GetId(), false);

        //save duration in the section relation
        vector<int> xIndices;
        vector<Attribute*> xAttrs;
        xIndices.push_back ( SECTION_DURATION );
        xAttrs.push_back ( new CcReal ( true, newDuration ) );
        m_pSections->UpdateTuple(actSect, xIndices, xAttrs);

        //save duration in the PageRecord of the section
        // read pages und Slots
        int pageUp =
          ((CcInt* )actSect->GetAttribute(SECTION_PNO_UP))->GetIntval();
        int slotUp =
          ((CcInt* )actSect->GetAttribute(SECTION_SLOT_UP))->GetIntval();
        int pageDown =
          ((CcInt* )actSect->GetAttribute(SECTION_PNO_DOWN))->GetIntval();
        int slotDown =
          ((CcInt* )actSect->GetAttribute(SECTION_SLOT_DOWN))->GetIntval();

        //get the page and save duration for both entries
        PageRecord actPage;
        m_xAdjacencyList.Get(pageUp-1, actPage);
        actPage.SetSectionDuration(slotUp, newDuration);

        if(pageDown == pageUp){
            actPage.SetSectionDuration(slotDown, newDuration);
            m_xAdjacencyList.Put(pageUp-1, actPage);
        }else{
            m_xAdjacencyList.Put(pageUp-1, actPage);
            m_xAdjacencyList.Get(pageDown-1, actPage);
            actPage.SetSectionDuration(slotDown, newDuration);
            m_xAdjacencyList.Put(pageDown-1, actPage);
        }

        //clean up
        actSect->DeleteIfAllowed();
        delete pSectIter;
        return true;
    }
    else {
        //errormessage
        sendMessage ( "the section with the given SectId must exist." );
        cout << "the section " <<sectId <<"  doesn't exist." <<endl;
        //clean up
        delete pSectIter;
        return false;
    }
}

/*
Method ~UpdateMoreSectionDuration~

update the cost of some ~sections~ which are in the given relation.

returns ~true~, if the update was successful, otherwise ~false~

*/
bool Network::UpdateMoreSectionDuration(const Relation* in_pDurations){
    //DEBUG PrintAdjacentSectionInfo();
    // check input
    if(in_pDurations->GetNoTuples() >0){
        //for each tuple of the Relation
        for(int i=1; i<=in_pDurations->GetNoTuples();i++){
            //Get sid and duration
            Tuple *actTuple = 0;
            actTuple = in_pDurations->GetTuple(i, false);
            int sid = ((CcInt*)actTuple->GetAttribute(0))->GetIntval();
            double duration =((CcReal*)actTuple->GetAttribute(1))->GetRealval();

            //call the function UpdateSectionDuration(sid,duration)
            UpdateSectionDuration(sid,duration);

            // clean up
            actTuple->DeleteIfAllowed();
        }
        //DEBUG PrintAdjacentSectionInfo();
        return true;
    }else{
        sendMessage ( "the relation is empty." );
        cout << "the relation is emplty." <<endl;
        return false;
    }
}


/*
Method ~chkStartEndA~

Returns the route Interval between the two points

*/
void chkStartEndA ( double &StartPos, double &EndPos )
{
  double help;
  if ( StartPos > EndPos )
  {
    help = StartPos;
    StartPos = EndPos;
    EndPos = help;
  }
};

/*
Method ~ShorterConnection~

*/
bool Network::ShorterConnection ( Tuple *route, double &start,
                                  double &end, double &dpos, double &dpos2,
                                  int &rid, int &ridt, Point p1, Point p2 )
{
  if ( AlmostEqual ( p1.Distance ( p2 ), fabs ( end-start ) ) ) return false;
  double difference;
  GPoint *gp = new GPoint ( true, GetId(), route->GetTupleId(), end - 0.01 );
  TupleId pSection1 = GetTupleIdSectionOnRoute ( gp );
  gp->DeleteIfAllowed();
  vector<DirectedSection> pAdjSect1;
  vector<DirectedSection> pAdjSect2;
  pAdjSect1.clear();
  pAdjSect2.clear();
  GetAdjacentSections ( pSection1,true, pAdjSect1 );
  GetAdjacentSections ( pSection1,false, pAdjSect2 );
  if ( pAdjSect1.size() == 0 || pAdjSect2.size() == 0 )
  {
    pAdjSect1.clear();
    pAdjSect2.clear();
    return false;
  }
  else
  {
    size_t j = 0;
    while ( j < pAdjSect1.size() )
    {
      DirectedSection actSection = pAdjSect1[j];
      Tuple *pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(),
                                                 false );
      ridt = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID )
)->GetIntval();
      pCurrSect->DeleteIfAllowed();
      if ( ridt != rid )
      {
        Tuple *pRoute = GetRoute ( ridt );
        SimpleLine *pCurve =
            ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
        if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
                ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )
        {
          pAdjSect1.clear();
          pAdjSect2.clear();
          chkStartEndA ( dpos, dpos2 );
          pRoute->DeleteIfAllowed();
          if ( fabs ( dpos2 - dpos ) < fabs ( end - start ) ) return  true;
          else return false;
        }
        pRoute->DeleteIfAllowed();
      }
      j++;
    }
    pAdjSect1.clear();
    j = 0;
    while ( j < pAdjSect2.size() )
    {
      DirectedSection actSection = pAdjSect2[j];
      Tuple *pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(),
                                                 false );
      ridt = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID )
)->GetIntval();
      pCurrSect->DeleteIfAllowed();
      if ( ridt != rid )
      {
        Tuple *pRoute = GetRoute ( ridt );
        SimpleLine *pCurve =
            ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
        if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
                ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )
        {
          pAdjSect2.clear();
          chkStartEndA ( dpos, dpos2 );
          pRoute->DeleteIfAllowed();
          if ( fabs ( dpos2-dpos ) < fabs ( end - start ) ) return true;
          else return false;
        }
        pRoute->DeleteIfAllowed();
      }
      j++;
    }
    return false;
  }
}

/*
Method ~ShorterConnection2~

*/
bool Network::ShorterConnection2 ( Tuple *route, double &start,
                                   double &end, double &dpos, double &dpos2,
                                   int &rid, int &ridt, Point p1, Point p2 )
{
  if ( AlmostEqual ( p1.Distance ( p2 ), fabs ( end-start ) ) ) return false;
  double difference = 0.0;
  if ( start < end && end > 0.01 ) difference = end - 0.01;
  else
    if ( start < end && end <= 0.01 ) difference = 0.01;
    else
      if ( start > end ) difference = end + 0.01;
      else difference = end; //start == end
  GPoint *gp = new GPoint ( true, GetId(), route->GetTupleId(), difference );
  TupleId pSection1 = GetTupleIdSectionOnRoute ( gp );
  gp->DeleteIfAllowed();
  vector<DirectedSection> pAdjSect1;
  vector<DirectedSection> pAdjSect2;
  pAdjSect1.clear();
  pAdjSect2.clear();
  GetAdjacentSections ( pSection1,true, pAdjSect1 );
  GetAdjacentSections ( pSection1,false, pAdjSect2 );
  if ( pAdjSect1.size() == 0 || pAdjSect2.size() == 0 )
  {
    pAdjSect1.clear();
    pAdjSect2.clear();
    return false;
  }
  else
  {
    size_t j = 0;
    while ( j < pAdjSect1.size() )
    {
      DirectedSection actSection = pAdjSect1[j];
      Tuple *pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(),
                                                 false );
      ridt = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID )
)->GetIntval();
      pCurrSect->DeleteIfAllowed();
      if ( ridt != rid )
      {
        Tuple *pRoute = GetRoute ( ridt );
        SimpleLine *pCurve =
            ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
        if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
                ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )
        {
          pAdjSect1.clear();
          pAdjSect2.clear();
          pRoute->DeleteIfAllowed();
          if ( fabs ( dpos2 - dpos ) < fabs ( end - start ) ) return  true;
          else return false;
        }
        pRoute->DeleteIfAllowed();
      }
      j++;
    }
    pAdjSect1.clear();
    j = 0;
    while ( j < pAdjSect2.size() )
    {
      DirectedSection actSection = pAdjSect2[j];
      Tuple *pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(),
                                                 false );
      ridt = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID )
)->GetIntval();
      pCurrSect->DeleteIfAllowed();
      if ( ridt != rid )
      {
        Tuple *pRoute = GetRoute ( ridt );
        SimpleLine *pCurve =
            ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
        if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
                ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )
        {
          pAdjSect2.clear();
          pRoute->DeleteIfAllowed();
          if ( fabs ( dpos2-dpos ) < fabs ( end - start ) ) return true;
          else return false;
        }
        pRoute->DeleteIfAllowed();
      }
      j++;
    }
    return false;
  }
}

/*
Method ~Find~

Searches the route interval between the two given point values.

*/
RouteInterval* Network::Find ( Point p1, Point p2 )
{
  GPoint *gpp1 = GetNetworkPosOfPoint ( p1 );
  GPoint *gpp2 = GetNetworkPosOfPoint ( p2 );
  assert ( gpp1->IsDefined() && gpp2->IsDefined() );
  int rid, ridt;
  double start, end, dpos, dpos2, difference;
  if ( gpp1->GetRouteId() == gpp2->GetRouteId() )
  {
    rid = gpp1->GetRouteId();
    start = gpp1->GetPosition();
    end = gpp2->GetPosition();
    chkStartEndA ( start,end );
    Tuple *pRoute = GetRoute ( rid );
    if ( ShorterConnection ( pRoute, start, end, dpos, dpos2, rid, ridt, p1, p2
) )
    {
      gpp1->DeleteIfAllowed();
      gpp2->DeleteIfAllowed();
      pRoute->DeleteIfAllowed();
      return new RouteInterval ( ridt, dpos, dpos2 );
    }
    else
    {
      gpp1->DeleteIfAllowed();
      gpp2->DeleteIfAllowed();
      pRoute->DeleteIfAllowed();
      return new RouteInterval ( rid, start, end );
    }
  }
  else   // different RouteIds
  {
    Tuple *pRoute = GetRoute ( gpp1->GetRouteId() );
    SimpleLine *pCurve =
        ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
    if ( chkPoint ( pCurve, p2, true, dpos, difference ) )
    {
      rid =
          ( ( CcInt* ) pRoute->GetAttribute ( ROUTE_ID ) )->GetIntval();
      start = gpp1->GetPosition();
      end = dpos;
      chkStartEndA ( start, end );
      if ( ShorterConnection ( pRoute, start, end, dpos, dpos2, rid, ridt, p1,
                               p2 ) )
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( ridt, dpos, dpos2 );
      }
      else
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( rid, start, end );
      }
    }
    pRoute->DeleteIfAllowed();
    pRoute = GetRoute ( gpp2->GetRouteId() );
    pCurve = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
    if ( chkPoint ( pCurve, p1, true, dpos, difference ) )
    {
      rid =
          ( ( CcInt* ) pRoute->GetAttribute ( ROUTE_ID ) )->GetIntval();
      start = gpp2->GetPosition();
      end = dpos;
      chkStartEndA ( start, end );
      if ( ShorterConnection ( pRoute, start, end, dpos, dpos2, rid, ridt, p1,
                               p2 ) )
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( ridt, dpos, dpos2 );
      }
      else
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( rid, start, end );
      }
    }
    pRoute->DeleteIfAllowed();
    TupleId pSection = GetTupleIdSectionOnRoute ( gpp1 );
    vector<DirectedSection> pAdjSect1;
    vector<DirectedSection> pAdjSect2;
    pAdjSect1.clear();
    pAdjSect2.clear();
    GetAdjacentSections ( pSection,true, pAdjSect1 );
    GetAdjacentSections ( pSection,false, pAdjSect2 );
    size_t j = 0;
    Tuple *pCurrSect;
    while ( j < pAdjSect1.size() )
    {
      DirectedSection actSection = pAdjSect1[j];
      pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(),
                                          false );
      rid = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID ) )->GetIntval();
      pCurrSect->DeleteIfAllowed();
      pRoute = GetRoute ( rid );
      pCurve = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
      if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
              ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )
      {
        start = dpos;
        end = dpos2;
        chkStartEndA ( start, end );
        pAdjSect1.clear();
        pAdjSect2.clear();
        if ( ShorterConnection ( pRoute, start, end, dpos, dpos2,
                                 rid, ridt, p1, p2 ) )
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( ridt, dpos, dpos2 );
        }
        else
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( rid, start, end );
        }
      }
      j++;
      pRoute->DeleteIfAllowed();
    }
    j = 0;
    pAdjSect1.clear();
    while ( j < pAdjSect2.size() )
    {
      DirectedSection actSection = pAdjSect2[j];
      pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(),
                                          false );
      rid = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID ) )->GetIntval();
      pCurrSect->DeleteIfAllowed();
      pRoute = GetRoute ( rid );
      pCurve = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
      if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
              ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )
      {
        start = dpos;
        end = dpos2;
        chkStartEndA ( start, end );
        pAdjSect2.clear();
        if ( ShorterConnection ( pRoute, start, end, dpos, dpos2,
                                 rid, ridt, p1, p2 ) )
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( ridt, dpos, dpos2 );
        }
        else
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( rid, start, end );
        }
      }
      j++;
      pRoute->DeleteIfAllowed();
    }//should never be reached
    pAdjSect2.clear();
  }//should never be reached
  gpp1->DeleteIfAllowed();
  gpp2->DeleteIfAllowed();
  return 0;
}


/*
Method ~FindInterval~

Returns the route interval for the connection from p1 to p2

*/
RouteInterval* Network::FindInterval ( Point p1, Point p2 )
{
  GPoint *gpp1 = GetNetworkPosOfPoint ( p1 );
  GPoint *gpp2 = GetNetworkPosOfPoint ( p2 );
  assert ( gpp1->IsDefined() && gpp2->IsDefined() );
  int rid, ridt;
  double start, end, dpos, dpos2, difference;
  if ( gpp1->GetRouteId() == gpp2->GetRouteId() )
  {
    rid = gpp1->GetRouteId();
    start = gpp1->GetPosition();
    end = gpp2->GetPosition();
    Tuple *pRoute = GetRoute ( rid );
    if ( ShorterConnection2 ( pRoute, start, end, dpos, dpos2, rid, ridt,
         p1, p2) )
    {
      gpp1->DeleteIfAllowed();
      gpp2->DeleteIfAllowed();
      pRoute->DeleteIfAllowed();
      return new RouteInterval ( ridt, dpos, dpos2 );
    }
    else
    {
      gpp1->DeleteIfAllowed();
      gpp2->DeleteIfAllowed();
      pRoute->DeleteIfAllowed();
      return new RouteInterval ( rid, start, end );
    }
  }
  else
  {
    Tuple *pRoute = GetRoute ( gpp1->GetRouteId() );
    SimpleLine *pCurve =
        ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
    if ( chkPoint ( pCurve, p2, true, dpos, difference ) )
    {
      rid =
          ( ( CcInt* ) pRoute->GetAttribute ( ROUTE_ID ) )->GetIntval();
      start = gpp1->GetPosition();
      end = dpos;
      if ( ShorterConnection2 ( pRoute, start, end, dpos, dpos2,
                                rid, ridt, p1, p2 ) )
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( ridt, dpos, dpos2 );
      }
      else
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( rid, start, end );
      }
    }
    pRoute->DeleteIfAllowed();
    pRoute = GetRoute ( gpp2->GetRouteId() );
    pCurve = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
    if ( chkPoint ( pCurve, p1, true, dpos, difference ) )
    {
      rid =
          ( ( CcInt* ) pRoute->GetAttribute ( ROUTE_ID ) )->GetIntval();
      start = dpos;
      end = gpp2->GetPosition();
      if ( ShorterConnection2 ( pRoute, start, end, dpos, dpos2,
                                rid, ridt, p1, p2 ) )
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( ridt, dpos, dpos2 );
      }
      else
      {
        gpp1->DeleteIfAllowed();
        gpp2->DeleteIfAllowed();
        pRoute->DeleteIfAllowed();
        return new RouteInterval ( rid, start, end );
      }
    }
    pRoute->DeleteIfAllowed();
    TupleId pSection = GetTupleIdSectionOnRoute ( gpp1 );
    vector<DirectedSection> pAdjSect1;
    vector<DirectedSection> pAdjSect2;
    pAdjSect1.clear();
    pAdjSect2.clear();
    GetAdjacentSections ( pSection,true, pAdjSect1 );
    GetAdjacentSections ( pSection,false, pAdjSect2 );
    size_t j = 0;
    Tuple *pCurrSect;
    while ( j < pAdjSect1.size() )
    {
      DirectedSection actSection = pAdjSect1[j];
      pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(), false );
      rid = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID ) )->GetIntval();
      pCurrSect->DeleteIfAllowed();
      pRoute = GetRoute ( rid );
      pCurve = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
      if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
              ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )
      {
        start = dpos;
        end = dpos2;
        pAdjSect1.clear();
        pAdjSect2.clear();
        if ( ShorterConnection2 ( pRoute, start, end, dpos, dpos2,
                                  rid, ridt, p1, p2 ) )
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( ridt, dpos, dpos2 );
        }
        else
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( rid, start, end );
        }
      }
      j++;
      pRoute->DeleteIfAllowed();
    }
    j = 0;
    pAdjSect1.clear();
    while ( j < pAdjSect2.size() )
    {
      DirectedSection actSection = pAdjSect2[j];
      pCurrSect = m_pSections->GetTuple ( actSection.GetSectionTid(), false );
      rid = ( ( CcInt* ) pCurrSect->GetAttribute ( SECTION_RID ) )->GetIntval();
      pCurrSect->DeleteIfAllowed();
      pRoute = GetRoute ( rid );
      pCurve = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
      if ( ( chkPoint ( pCurve, p1, true, dpos, difference ) ) &&
              ( chkPoint ( pCurve, p2, true, dpos2, difference ) ) )
      {
        start = dpos;
        end = dpos2;
        pAdjSect2.clear();
        if ( ShorterConnection2 ( pRoute, start, end, dpos, dpos2,
                                  rid, ridt, p1, p2 ) )
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( ridt, dpos, dpos2 );
        }
        else
        {
          gpp1->DeleteIfAllowed();
          gpp2->DeleteIfAllowed();
          pRoute->DeleteIfAllowed();
          return new RouteInterval ( rid, start, end );
        }
      }
      j++;
      pRoute->DeleteIfAllowed();
    }//should never be reached
    pAdjSect2.clear();
  }//should never be reached
  gpp1->DeleteIfAllowed();
  gpp2->DeleteIfAllowed();
  return 0;
}

/*
Get-Methodes of network parameters.

~GetId~, ~GetRoutes~, ~GetJunctions~, ~GetJunctionsOnRoute~, ~GetSection~,
~GetTupleIdSectionOnRoute~, ~GetTupleIdSectionOnRoute~, ~GetSectionOnRoute~,
~GetRoute~, ~GetSectionsOfRouteInterval~, ~GetSectionsOfRoutInterval~,
~GetPointOnRoute~, ~GetSectionsInternal~, ~GetSections~,
~GetAdjacentSectionsInfo~, ~GetAdjacentSections~, ~GetRouteCurve~, ~GetDual~,
~GetTupleIdSectionOnRouteJun~, ~GetNetworkPosOfPoint~,
~GetJunctionMeasForRoutes~, ~GetLineValueOfRouteInterval~

*/

int Network::GetId()
{
  return m_iId;
}

Relation *Network::GetRoutes()
{
  return m_pRoutes;
/*  ostringstream strRoutesPtr;
  strRoutesPtr << ( long ) m_pRoutes;

  string querystring = "(consume (feed (" + routesTypeInfo +
                       " (ptr " + strRoutesPtr.str() + "))))";

  Word resultWord;
  int QueryExecuted = QueryProcessor::ExecuteQuery ( querystring, resultWord );
  assert ( QueryExecuted ); // no ASSERT with side effects, please
  return ( Relation * ) resultWord.addr;*/
}


Relation *Network::GetJunctions()
{
  ostringstream strJunctionsPtr;
  strJunctionsPtr << ( long ) m_pJunctions;

  string querystring = "(consume (feed (" + junctionsInternalTypeInfo +
                       " (ptr " + strJunctionsPtr.str() + "))))";

  Word resultWord;
  int QueryExecuted = QueryProcessor::ExecuteQuery ( querystring, resultWord );
  assert ( QueryExecuted ); // no ASSERT with side effects, please
  return ( Relation * ) resultWord.addr;
}


void Network::GetJunctionsOnRoute ( CcInt* in_pRouteId,
                                    vector<JunctionSortEntry> &inout_xJunctions
)
{
  BTreeIterator* pJunctionsIt;
  pJunctionsIt = m_pBTreeJunctionsByRoute1->ExactMatch ( in_pRouteId );
  while ( pJunctionsIt->Next() )
  {
    // Get next junction
    Tuple* pCurrentJunction = m_pJunctions->GetTuple ( pJunctionsIt->GetId(),
                                                       false );
    inout_xJunctions.push_back ( JunctionSortEntry ( true, pCurrentJunction ) );
  }
  delete pJunctionsIt;

  // Now we look up the second b-tree
  pJunctionsIt = m_pBTreeJunctionsByRoute2->ExactMatch ( in_pRouteId );
  while ( pJunctionsIt->Next() )
  {
    Tuple* pCurrentJunction = m_pJunctions->GetTuple ( pJunctionsIt->GetId(),
                                                       false );
    inout_xJunctions.push_back ( JunctionSortEntry ( false, pCurrentJunction )
);
  }
  delete pJunctionsIt;

  // The junctions will be sorted by their mesure on the relevant route.
  sort ( inout_xJunctions.begin(),
         inout_xJunctions.end() );
}

Tuple* Network::GetSection ( TupleId n )
{
  return m_pSections->GetTuple ( n, false );
}


TupleId Network::GetTupleIdSectionOnRoute ( GPoint* in_xGPoint )
{
  CcInt *ciRouteId = new CcInt ( true, in_xGPoint->GetRouteId() );
  BTreeIterator* pSectionIter =
      m_pBTreeSectionsByRoute->ExactMatch ( ciRouteId );
  ciRouteId->DeleteIfAllowed();
  Tuple *actSect = 0;
  TupleId result;
  while ( pSectionIter->Next() )
  {
    result = pSectionIter->GetId();
    actSect =
        m_pSections->GetTuple ( pSectionIter->GetId(), false );
    if ( actSect != 0 )
    {
      double start =
          ( ( CcReal* ) actSect->GetAttribute ( SECTION_MEAS1 ) )->GetRealval();
      double end =
          ( ( CcReal* ) actSect->GetAttribute ( SECTION_MEAS2 ) )->GetRealval();
      if ( in_xGPoint->GetPosition() >= start&&in_xGPoint->GetPosition() <= end
)
      {
        delete pSectionIter;
        actSect->DeleteIfAllowed();
        return result;
      }
      else
      {
        if ( fabs ( in_xGPoint->GetPosition() - start ) <= 0.01 )
        {
          delete pSectionIter;
          actSect->DeleteIfAllowed();
          return result;
        }
        else
        {
          if ( fabs ( in_xGPoint->GetPosition() - end ) <= 0.01 )
          {
            Tuple *pRoute = GetRoute ( ( ( TupleIdentifier* )
                                         actSect->GetAttribute ( SECTION_RRC )
)->GetTid() );
            if ( fabs ( ( ( CcReal* )
                          pRoute->GetAttribute ( ROUTE_LENGTH ) )->GetRealval()
                        - end ) <= 0.01 )
            {
              pRoute->DeleteIfAllowed();
              delete pSectionIter;
              actSect->DeleteIfAllowed();
              return result;
            }
            else
            {
              pRoute->DeleteIfAllowed();
            }
          }
        }
      }
      actSect->DeleteIfAllowed();
    }
  }
  delete pSectionIter;
  return 0;
}

Tuple* Network::GetSectionOnRoute ( GPoint* in_xGPoint )
{
  return GetSection ( GetTupleIdSectionOnRoute ( in_xGPoint ) );
  /*
  New implementation using sectionsBTree



  CcInt *ciRouteId = new CcInt(true, in_xGPoint->GetRouteId());
  BTreeIterator* pSectionIter =
      m_pBTreeSectionsByRoute->ExactMatch(ciRouteId);
  ciRouteId->DeleteIfAllowed();
  Tuple *actSect = 0;
  while (pSectionIter->Next()){
    actSect =
      m_pSections->GetTuple(pSectionIter->GetId());
    if(actSect != 0){
      double start =
        ((CcReal*)actSect->GetAttribute(SECTION_MEAS1))->GetRealval();
      double end =
        ((CcReal*) actSect->GetAttribute(SECTION_MEAS2))->GetRealval();
      if (in_xGPoint->GetPosition() >= start&&in_xGPoint->GetPosition() <= end)
      {
        delete pSectionIter;
        return actSect;
      }
      else {
        if (fabs(in_xGPoint->GetPosition() - start) <= 0.01) {
          delete pSectionIter;
          return actSect;
        }
        else {
          if (fabs(in_xGPoint->GetPosition() - end) <= 0.01) {
            Tuple *pRoute = GetRoute(((TupleIdentifier*)
                actSect->GetAttribute(SECTION_RRC))->GetTid());
            if (fabs(((CcReal*)
                pRoute->GetAttribute(ROUTE_LENGTH))->GetRealval()
                - end) <= 0.01){
              pRoute->DeleteIfAllowed();
              delete pSectionIter;
              return actSect;
            } else {
              pRoute->DeleteIfAllowed();
            }
          }
        }
      }
      actSect->DeleteIfAllowed();
    }
  }
  delete pSectionIter;
  return 0;



  vector<JunctionSortEntry> xJunctions;
  CcInt xRouteId(true, in_xGPoint->GetRouteId());
  GetJunctionsOnRoute(&xRouteId,
                      xJunctions);

  // Now that we found all relevant junctions we can iterate over them.
  int iSectionId = 0;
  double juncpos;
  for(size_t i = 0; i < xJunctions.size(); i++)
  {
    // Get next junction
    JunctionSortEntry xCurrentEntry = xJunctions[i];
    iSectionId = xCurrentEntry.GetDownSectionId();
    juncpos = xCurrentEntry.GetRouteMeas();
    if(juncpos > in_xGPoint->GetPosition())
    {
      break;
    }
    if (juncpos != 0 && fabs(juncpos - in_xGPoint->GetPosition()) < 0.01){
      break;
    }
    iSectionId = xCurrentEntry.GetUpSectionId();
  }
  for(size_t i = 0; i < xJunctions.size(); i++)
  {
    // Get next junction
    JunctionSortEntry xCurrentEntry = xJunctions[i];
    xCurrentEntry.m_pJunction->DeleteIfAllowed();

  }

  if(iSectionId == 0) return 0;
  else return m_pSections->GetTuple(iSectionId);
  */
}

Tuple* Network::GetRoute ( int in_RouteId )
{
  CcInt* pRouteId = new CcInt ( true, in_RouteId );
  BTreeIterator *pRoutesIter = m_pBTreeRoutes->ExactMatch ( pRouteId );
  pRouteId->DeleteIfAllowed();
  Tuple *pRoute = 0;
  if ( pRoutesIter->Next() )
    pRoute = m_pRoutes->GetTuple ( pRoutesIter->GetId() , false);
  assert ( pRoute != 0 );
  delete pRoutesIter;
  return pRoute;

}

Tuple* Network::GetRoute ( TupleId in_routeTID )
{
  return m_pRoutes->GetTuple ( in_routeTID, false );
}

void Network::GetSectionsOfRouteInterval ( const RouteInterval *ri,
        DbArray<SectTreeEntry> *io_SectionIds )
{
  double ristart = min ( ri->GetStartPos(), ri->GetEndPos() );
  double riend = max ( ri->GetStartPos(), ri->GetEndPos() );
  CcInt* ciRouteId = new CcInt ( true, ri->GetRouteId() );
  BTreeIterator* pSectionIter =
      m_pBTreeSectionsByRoute->ExactMatch ( ciRouteId );
  ciRouteId->DeleteIfAllowed();
  Tuple *actSect;
  TupleId actSectTID;
  bool bsectstart = true;
  bool bsectend = true;
  while ( pSectionIter->Next() )
  {
    actSectTID = pSectionIter->GetId();
    actSect = m_pSections->GetTuple ( actSectTID, false );
    assert ( actSect != 0 );
    double start =
        ( ( CcReal* ) actSect->GetAttribute ( SECTION_MEAS1 ) )->GetRealval();
    double end =
        ( ( CcReal* ) actSect->GetAttribute ( SECTION_MEAS2 ) )->GetRealval();
    if ( ( ristart <= start && riend >= end ) ||
            ( start <= ristart && end >= ristart ) ||
            ( start <= riend && end >= riend ) )
    {
      if ( start <= ristart )
      {
        start = ristart;
        bsectstart = false;
      }
      if ( riend <= end )
      {
        end = riend;
        bsectend = false;
      }
      SectTreeEntry *sect =
          new SectTreeEntry ( actSect->GetTupleId(), ri->GetRouteId(), start,
end,
                              bsectstart, bsectend );
      io_SectionIds->Append ( *sect );
      delete sect;
      if ( riend <= end )
      {
        actSect->DeleteIfAllowed();
        break;
      }
    }
    actSect->DeleteIfAllowed();
  }
  io_SectionIds->TrimToSize();
  delete pSectionIter;
};

void Network::GetSectionsOfRoutInterval ( const RouteInterval *ri,
        vector<TupleId> &res )
{
  res.clear();
  double ristart = min ( ri->GetStartPos(), ri->GetEndPos() );
  double riend = max ( ri->GetStartPos(), ri->GetEndPos() );
  CcInt* ciRouteId = new CcInt ( true, ri->GetRouteId() );
  BTreeIterator* pSectionIter =
      m_pBTreeSectionsByRoute->ExactMatch ( ciRouteId );
  ciRouteId->DeleteIfAllowed();
  Tuple *actSect;
  TupleId actSectTID;
  //bool bsectstart = true;
  //bool bsectend = true;
  while ( pSectionIter->Next() )
  {
    actSectTID = pSectionIter->GetId();
    actSect = m_pSections->GetTuple ( actSectTID, false );
    assert ( actSect != 0 );
    double start =
        ( ( CcReal* ) actSect->GetAttribute ( SECTION_MEAS1 ) )->GetRealval();
    double end =
        ( ( CcReal* ) actSect->GetAttribute ( SECTION_MEAS2 ) )->GetRealval();
    if ( fabs ( ristart - riend ) <= 0.01 &&
            ( fabs ( ristart - start ) <= 0.01 || fabs ( ristart - end ) <= 0.01
) )
    {
      res.push_back ( actSectTID );
      actSect->DeleteIfAllowed();
      break;
    }
    else
    {
      if ( ( ( ristart <= start && end <= riend ) ||
              ( start <= ristart && end > ristart ) ||
              ( start < riend && riend <= end ) ) &&
              ( ! ( fabs ( ristart - end ) <= 0.01 || fabs ( start - riend ) <=
0.01 ) ) )
        res.push_back ( actSectTID );
    }
    actSect->DeleteIfAllowed();
  }
  delete pSectionIter;
}

void Network::GetPointOnRoute ( const GPoint* in_pGPoint, Point*& res )
{
  /*Point *res = new Point(false);*/
  CcInt* pRouteId = new CcInt ( true, in_pGPoint->GetRouteId() );
  BTreeIterator* pRoutesIter = m_pBTreeRoutes->ExactMatch ( pRouteId );
  pRouteId->DeleteIfAllowed();
  Tuple *pRoute = 0;
  if ( pRoutesIter->Next() )
    pRoute = m_pRoutes->GetTuple ( pRoutesIter->GetId(), false );
  assert ( pRoute != 0 );
  SimpleLine* pLine = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
  assert ( pLine != 0 );
  pLine->AtPosition ( in_pGPoint->GetPosition(),true, *res );
  pRoute->DeleteIfAllowed();
  delete pRoutesIter;
  /*return res;*/
}

Relation* Network::GetSectionsInternal()
{
  return m_pSections;
}

Relation* Network::GetSections()
{
  ostringstream strSectionsPtr;
  strSectionsPtr << ( long ) m_pSections;

  string querystring = "(consume (feed (" + sectionsInternalTypeInfo +
                       " (ptr " + strSectionsPtr.str() + "))))";

  Word resultWord;
  int QueryExecuted = QueryProcessor::ExecuteQuery ( querystring, resultWord );
  assert ( QueryExecuted ); // No ASSERT with side effect, please!
  return ( Relation * ) resultWord.addr;
}


void Network::GetAdjacentSectionsInfo ( TupleId in_iSectionTId,
                                  bool in_bUpDown,
                                  vector<DirectedSectionInfo> &inout_xSections )
{
  inout_xSections.clear();
  Tuple *pSect = GetSection ( in_iSectionTId );
  if ( pSect != 0 )
  {
    // get Sid, SectionPage and SectionSlot
    int iSectionId, iSectionPage, iSectionSlot;
    iSectionId =( ( CcInt* ) pSect->GetAttribute ( SECTION_SID ) )->GetIntval();
    if (in_bUpDown){
     iSectionPage=((CcInt*)pSect->GetAttribute(SECTION_PNO_UP))->GetIntval();
     iSectionSlot=((CcInt*)pSect->GetAttribute(SECTION_SLOT_UP))->GetIntval();
    }else{
     iSectionPage=((CcInt*)pSect->GetAttribute(SECTION_PNO_DOWN))->GetIntval();
     iSectionSlot=((CcInt*)pSect->GetAttribute(SECTION_SLOT_DOWN))->GetIntval();
    }
    pSect->DeleteIfAllowed();
    if(iSectionPage != 0 && iSectionSlot !=0){
        //getSectionPage
        PageRecord xPage;
        m_xAdjacencyList.Get ( iSectionPage-1, xPage );

        //get Number of Neighbours
        int iNeighbours=0;
        xPage.GetSectionNoNeighbours(iSectionSlot, iNeighbours);
        //Cache for Pages and Slots defined
        int **iPagesSlots;
        iPagesSlots = new int* [iNeighbours];
        for (int i=0; i<iNeighbours;i++){
            iPagesSlots[i] = new int[2];
        }

        for(int i =0; i<iNeighbours ;i++){
            xPage.GetSectionNeighbourPage(iSectionSlot,i,iPagesSlots[i][0]);
            xPage.GetSectionNeighbourSlot(iSectionSlot,i,iPagesSlots[i][1]);
        }

        // store all needet attributes of the Neighbour in DirectedSectionInfo
        int actPageNum = iSectionPage;
        bool bUpDownFlag;
        int iSectionId, irid;
        double dcost, dduration;
        TupleId sectionTid;
        double dmeas1, dmeas2;

        for(int i=0; i< iNeighbours; i++){
            if(actPageNum != iPagesSlots[i][0]){
                // get new Page
                actPageNum = iPagesSlots[i][0];
                m_xAdjacencyList.Get ( actPageNum-1, xPage );
            }
            xPage.GetSectionId(iPagesSlots[i][1],iSectionId);
            xPage.GetSectionUpdown(iPagesSlots[i][1],bUpDownFlag);
            xPage.GetSectionMeas1(iPagesSlots[i][1],dmeas1);
            xPage.GetSectionMeas2(iPagesSlots[i][1],dmeas2);
            xPage.GetSectionRid(iPagesSlots[i][1],irid);
            xPage.GetSectionCost(iPagesSlots[i][1],dcost);
            xPage.GetSectionDuration(iPagesSlots[i][1],dduration);
            xPage.GetSectionTid(iPagesSlots[i][1],sectionTid);

            inout_xSections.push_back (
                 DirectedSectionInfo(sectionTid,bUpDownFlag,dmeas1,dmeas2,irid,
                                     dcost,dduration));
        }
        //Delete Array
        for (int i=0; i<iNeighbours;i++){
            delete [] iPagesSlots[i];
        }
        delete [] iPagesSlots;
    }
  }
}


void Network::GetAdjacentSections ( TupleId in_iSectionTId,
                                    bool in_bUpDown,
                                    vector<DirectedSection> &inout_xSections )
{
  inout_xSections.clear();
  Tuple *pSect = GetSection ( in_iSectionTId );
  if ( pSect != 0 )
  {
    // get Sid, SectionPage and SectionSlot
    int iSectionId, iSectionPage, iSectionSlot;
    iSectionId = ( ( CcInt* )pSect->GetAttribute ( SECTION_SID ) )->GetIntval();
    if (in_bUpDown){
     iSectionPage =((CcInt*)pSect->GetAttribute(SECTION_PNO_UP))->GetIntval();
     iSectionSlot =((CcInt*)pSect->GetAttribute(SECTION_SLOT_UP))->GetIntval();
    }else{
     iSectionPage =((CcInt*)pSect->GetAttribute(SECTION_PNO_DOWN))->GetIntval();
     iSectionSlot=((CcInt*)pSect->GetAttribute(SECTION_SLOT_DOWN))->GetIntval();
    }
    pSect->DeleteIfAllowed();
    if(iSectionPage != 0 && iSectionSlot !=0){
        //getSectionPage
        PageRecord xPage;
        m_xAdjacencyList.Get ( iSectionPage-1, xPage );

        //get Number of Neighbours
        int iNeighbours=0;

        xPage.GetSectionNoNeighbours(iSectionSlot, iNeighbours);
        //Cache for Pages and Slots defined
        int **iPagesSlots;
        iPagesSlots = new int* [iNeighbours];
        for (int i=0; i<iNeighbours;i++){
            iPagesSlots[i] = new int[2];
        }

        for(int i =0; i<iNeighbours ;i++){
            xPage.GetSectionNeighbourPage(iSectionSlot,i,iPagesSlots[i][0]);
            xPage.GetSectionNeighbourSlot(iSectionSlot,i,iPagesSlots[i][1]);
        }

        // store the Sid and UpDownFlag of the Neighbour in DirectedSection
        int actPageNum = iSectionPage;
        bool bUpDownFlag;
        int iSectionId;
        TupleId sectionTid;

        for(int i=0; i< iNeighbours; i++){

            if(actPageNum != iPagesSlots[i][0]){
                // get new Page
                actPageNum = iPagesSlots[i][0];
                m_xAdjacencyList.Get ( actPageNum-1, xPage );
            }
            xPage.GetSectionId(iPagesSlots[i][1],iSectionId);
            xPage.GetSectionUpdown(iPagesSlots[i][1],bUpDownFlag);
            xPage.GetSectionTid(iPagesSlots[i][1],sectionTid);
            inout_xSections.push_back (DirectedSection(sectionTid,bUpDownFlag));
        }
        //Delete Array
        for (int i=0; i<iNeighbours;i++){
            delete [] iPagesSlots[i];
        }
        delete [] iPagesSlots;

    }
  }
}

SimpleLine Network::GetRouteCurve ( int in_iRouteId )
{
  Tuple *pRoute = GetRoute ( in_iRouteId );
  SimpleLine sl = * ( ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE ) );
  pRoute->DeleteIfAllowed();
  return sl;
}

bool Network::GetDual ( int in_iRouteId )
{
  Tuple *pRoute = GetRoute ( in_iRouteId );
  bool dual = ( ( CcBool* ) pRoute->GetAttribute ( ROUTE_DUAL ) )->GetBoolval();
  pRoute->DeleteIfAllowed();
  return dual;
}


void Network::GetTupleIdSectionOnRouteJun(GPoint* in_xGPoint,
vector<TupleId>& res)
{

  CcInt *ciRouteId = new CcInt ( true, in_xGPoint->GetRouteId() );
  BTreeIterator* pSectionIter =
      m_pBTreeSectionsByRoute->ExactMatch ( ciRouteId );
  delete ciRouteId;
  Tuple *actSect = 0;
  TupleId result;
  while ( pSectionIter->Next() )
  {
    result = pSectionIter->GetId();
    actSect = m_pSections->GetTuple ( pSectionIter->GetId(), false );
    if ( actSect != 0 )
    {
      double start =
          ((CcReal*)actSect->GetAttribute(SECTION_MEAS1))->GetRealval();
      double end =
          ((CcReal*)actSect->GetAttribute(SECTION_MEAS2))->GetRealval();
      if(in_xGPoint->GetPosition() >= start&&in_xGPoint->GetPosition() <= end)
      {
//        delete pSectionIter;
//        actSect->DeleteIfAllowed();
//        return result;
        res.push_back(result);
      }
      else
      {
        if ( fabs ( in_xGPoint->GetPosition() - start ) <= 0.01 )
        {
//          delete pSectionIter;
//          actSect->DeleteIfAllowed();
//          return result;
          res.push_back(result);
        }
        else
        {
          if ( fabs ( in_xGPoint->GetPosition() - end ) <= 0.01 )
          {
           Tuple *pRoute = GetRoute(((TupleIdentifier* )
                          actSect->GetAttribute ( SECTION_RRC ))->GetTid() );
            if ( fabs ( ( ( CcReal* )
                          pRoute->GetAttribute ( ROUTE_LENGTH ) )->GetRealval()
                        - end ) <= 0.01 )
            {
 //             pRoute->DeleteIfAllowed();
//              delete pSectionIter;
//              actSect->DeleteIfAllowed();
                res.push_back(result);
//              return result;
            }
            pRoute->DeleteIfAllowed();
          }
        }
      }
//      actSect->DeleteIfAllowed();
    }
    actSect->DeleteIfAllowed();
  }
  delete pSectionIter;

}

GPoint* Network::GetNetworkPosOfPoint ( Point p )
{
  const Rectangle<2> orig = p.BoundingBox();
  const Rectangle<2> bbox = Rectangle<2> ( true,
                            orig.MinD ( 0 ) - 1.0,
                            orig.MaxD ( 0 ) + 1.0,
                            orig.MinD ( 1 ) - 1.0,
                            orig.MaxD ( 1 ) + 1.0 );
  R_TreeLeafEntry<2,TupleId> res;
  Tuple *pCurrRoute = 0;
  if ( m_pRTreeRoutes->First ( bbox, res ) )
  {
    pCurrRoute = m_pRoutes->GetTuple ( res.info, false );
    // pCurrRoute->PutAttribute(0, new TupleIdentifier(true, res.info));
  }
  else
  {
    GPoint *result = new GPoint ( false );
    pCurrRoute->DeleteIfAllowed();
    return result;
  }
  double dpos, difference;
  SimpleLine* pRouteCurve = ( SimpleLine* ) pCurrRoute->GetAttribute (
ROUTE_CURVE );
  if ( chkPoint ( pRouteCurve, p, true, dpos, difference ) )
  {
    int rid = ( ( CcInt* ) pCurrRoute->GetAttribute ( ROUTE_ID ) )->GetIntval();
    GPoint *result = new GPoint ( true, GetId(), rid, dpos, None );
    pCurrRoute->DeleteIfAllowed();
    return result;
  }
  else
  {
    pCurrRoute->DeleteIfAllowed();
    while ( m_pRTreeRoutes->Next ( res ) )
    {
      pCurrRoute = m_pRoutes->GetTuple ( res.info, false );
      pRouteCurve = ( SimpleLine* ) pCurrRoute->GetAttribute ( ROUTE_CURVE );
      if ( chkPoint ( pRouteCurve, p, true, dpos, difference ) )
      {
        int rid = ( ( CcInt* ) pCurrRoute->GetAttribute ( ROUTE_ID )
)->GetIntval();
        GPoint *result = new GPoint ( true, GetId(),
                                      rid,
                                      dpos, None );
        pCurrRoute->DeleteIfAllowed();
        return result;
      }
      pCurrRoute->DeleteIfAllowed();
    }
    /*
    If the point exact hits a route the route should be found here. If the point
    value is not exact on the route curve we try to map it in the next step with
    bigger tolerance for the hit of the route curve.

    */
    if ( m_pRTreeRoutes->First ( bbox, res ) )
      pCurrRoute = m_pRoutes->GetTuple ( res.info, false );
    pRouteCurve = ( SimpleLine* ) pCurrRoute->GetAttribute ( ROUTE_CURVE );
    if ( chkPoint03 ( pRouteCurve, p, true, dpos, difference ) )
    {
      int rid = ( ( CcInt* ) pCurrRoute->GetAttribute ( ROUTE_ID )
)->GetIntval();
      GPoint *result = new GPoint ( true, GetId(),
                                    rid,
                                    dpos, None );
      pCurrRoute->DeleteIfAllowed();
      return result;
    }
    else
    {
      pCurrRoute->DeleteIfAllowed();
      while ( m_pRTreeRoutes->Next ( res ) )
      {
        pCurrRoute = m_pRoutes->GetTuple ( res.info , false);
        pRouteCurve = ( SimpleLine* ) pCurrRoute->GetAttribute ( ROUTE_CURVE );
        if ( chkPoint03 ( pRouteCurve, p, true, dpos, difference ) )
        {
          int rid = ( ( CcInt* ) pCurrRoute->GetAttribute ( ROUTE_ID )
)->GetIntval();
          GPoint *result = new GPoint ( true, GetId(),
                                        rid,
                                        dpos, None );
          pCurrRoute->DeleteIfAllowed();
          return result;
        }
        pCurrRoute->DeleteIfAllowed();
      }
    }

    if ( m_pRTreeRoutes->First ( bbox, res ) )
      pCurrRoute = m_pRoutes->GetTuple ( res.info, false );
    pRouteCurve = ( SimpleLine* ) pCurrRoute->GetAttribute ( ROUTE_CURVE );
    if ( lastchkPoint03 ( pRouteCurve, p, true, dpos, difference ) )
    {
      int rid = ( ( CcInt* ) pCurrRoute->GetAttribute ( ROUTE_ID )
)->GetIntval();
      GPoint *result = new GPoint ( true, GetId(),
                                    rid,
                                    dpos, None );
      pCurrRoute->DeleteIfAllowed();
      return result;
    }
    else
    {
      pCurrRoute->DeleteIfAllowed();
      while ( m_pRTreeRoutes->Next ( res ) )
      {
        pCurrRoute = m_pRoutes->GetTuple ( res.info, false );
        pRouteCurve = ( SimpleLine* ) pCurrRoute->GetAttribute ( ROUTE_CURVE );
        if ( lastchkPoint03 ( pRouteCurve, p, true, dpos, difference ) )
        {
          int rid = ( ( CcInt* ) pCurrRoute->GetAttribute ( ROUTE_ID )
)->GetIntval();
          GPoint *result = new GPoint ( true, GetId(),
                                        rid,
                                        dpos, None );
          pCurrRoute->DeleteIfAllowed();
          return result;
        }
        pCurrRoute->DeleteIfAllowed();
      }
    } // should not be reached
    GPoint *result = new GPoint ( false );
    pCurrRoute->DeleteIfAllowed();
    return result;
  }
}

void Network::GetJunctionMeasForRoutes ( CcInt *pRoute1Id, CcInt *pRoute2Id,
        double &rid1meas, double &rid2meas )
{
  CcInt *pCurrJuncR2id, *pCurrJuncR1id;
  int iCurrJuncTupleR2id, iCurrJuncR1id, iRoute1Id, iRoute2Id;
  CcReal *pRid1Meas, *pRid2Meas;
  bool r1smallerr2, found;
  BTreeIterator *pJunctionsIt;
  if ( pRoute1Id->GetIntval() <= pRoute2Id->GetIntval() )
  {
    pJunctionsIt = m_pBTreeJunctionsByRoute1->ExactMatch ( pRoute1Id );
    iRoute1Id = pRoute1Id->GetIntval();
    iRoute2Id = pRoute2Id->GetIntval();
    r1smallerr2 = true;
  }
  else
  {
    pJunctionsIt = m_pBTreeJunctionsByRoute1->ExactMatch ( pRoute2Id );
    iRoute1Id = pRoute2Id->GetIntval();
    iRoute2Id = pRoute1Id->GetIntval();
    r1smallerr2 = false;
  }
  found = false;
  while ( !found && pJunctionsIt->Next() )
  {
    Tuple *pCurrJuncTuple = m_pJunctions->GetTuple ( pJunctionsIt->GetId(),
                                                     false );
    pCurrJuncR2id = ( CcInt* ) pCurrJuncTuple->GetAttribute ( JUNCTION_ROUTE2_ID
);
    iCurrJuncTupleR2id = pCurrJuncR2id->GetIntval();
    pCurrJuncR1id = ( CcInt* ) pCurrJuncTuple->GetAttribute ( JUNCTION_ROUTE1_ID
);
    iCurrJuncR1id = pCurrJuncR1id->GetIntval();
    if ( iCurrJuncTupleR2id == iRoute2Id && iCurrJuncR1id == iRoute1Id )
    {
      found = true;
      if ( r1smallerr2 )
      {
        pRid1Meas = ( CcReal* ) pCurrJuncTuple->GetAttribute (
JUNCTION_ROUTE1_MEAS );
        rid1meas = pRid1Meas->GetRealval();
        pRid2Meas = ( CcReal* ) pCurrJuncTuple->GetAttribute (
JUNCTION_ROUTE2_MEAS );
        rid2meas = pRid2Meas->GetRealval();
      }
      else
      {
        pRid1Meas = ( CcReal* ) pCurrJuncTuple->GetAttribute (
JUNCTION_ROUTE2_MEAS );
        rid1meas = pRid1Meas->GetRealval();
        pRid2Meas = ( CcReal* ) pCurrJuncTuple->GetAttribute (
JUNCTION_ROUTE1_MEAS );
        rid2meas = pRid2Meas->GetRealval();
      }
    }
    pCurrJuncTuple->DeleteIfAllowed();
  }
  delete pJunctionsIt;
  if ( !found )
  {
    rid1meas = numeric_limits<double>::max();
    rid2meas = numeric_limits<double>::max();
  }
}

void Network::GetLineValueOfRouteInterval ( const RouteInterval *in_ri,
        SimpleLine *out_Line )
{
  CcInt* pRouteId = new CcInt ( true, in_ri->GetRouteId() );
  BTreeIterator *pRoutesIter = m_pBTreeRoutes->ExactMatch ( pRouteId );
  pRouteId->DeleteIfAllowed();
  Tuple *pRoute = 0;
  if ( pRoutesIter->Next() ) pRoute = m_pRoutes->GetTuple (
                                pRoutesIter->GetId(), false);
  assert ( pRoute != 0 );
  SimpleLine* pLine = ( SimpleLine* ) pRoute->GetAttribute ( ROUTE_CURVE );
  assert ( pLine != 0 );
  CcBool* pStSm = ( CcBool* ) pRoute->GetAttribute ( ROUTE_STARTSSMALLER );
  bool startSmaller = pStSm->GetBoolval();
  pLine->SubLine ( min ( in_ri->GetStartPos(), in_ri->GetEndPos() ),
                   max ( in_ri->GetStartPos(), in_ri->GetEndPos() ),
                   startSmaller, *out_Line );
  pRoute->DeleteIfAllowed();
  delete pRoutesIter;
}

/*
3.1.4 Support functions of type constructor ~network~

~Out~-function of type constructor ~network~

*/
ListExpr Network::Out ( ListExpr typeInfo )
{
//cout << "NetworkOut" << endl;
  ///////////////////////
  // Output of all routes
  GenericRelationIterator *pRoutesIter = m_pRoutes->MakeScan();
  Tuple *pCurrentRoute = 0;
  ListExpr xLast = nl->TheEmptyList();
  ListExpr xNext = nl->TheEmptyList();
  ListExpr xRoutes = nl->TheEmptyList();
  bool bFirst = true;

  while ( ( pCurrentRoute = pRoutesIter->GetNextTuple() ) != 0 )
  {
    // Read values from table
    CcInt* pRouteId = ( CcInt* ) pCurrentRoute->GetAttribute ( ROUTE_ID );
    int iRouteId = pRouteId->GetIntval();
    CcReal* pLength = ( CcReal* ) pCurrentRoute->GetAttribute ( ROUTE_LENGTH );
    double dLength  = pLength->GetRealval();
    SimpleLine *pCurve = ( SimpleLine* ) pCurrentRoute->GetAttribute (
ROUTE_CURVE );
    // The list for the curve contains all segments of the curve.
    ListExpr xCurve = OutSimpleLine ( nl->TheEmptyList(), SetWord ( pCurve ) );
    CcBool* pDual = ( CcBool* ) pCurrentRoute->GetAttribute ( ROUTE_DUAL );
    bool bDual= pDual->GetBoolval();
    CcBool* pStartsSmaller;
    pStartsSmaller = ( CcBool* ) pCurrentRoute->GetAttribute (
ROUTE_STARTSSMALLER );
    bool bStartsSmaller = pStartsSmaller->GetBoolval();

    // Build list
    xNext = nl->FiveElemList ( nl->IntAtom ( iRouteId ),
                               nl->RealAtom ( dLength ),
                               xCurve,
                               nl->BoolAtom ( bDual ),
                               nl->BoolAtom ( bStartsSmaller ) );

    // Create new list or append element to existing list
    if ( bFirst )
    {
      xRoutes = nl->OneElemList ( xNext );
      xLast = xRoutes;
      bFirst = false;
    }
    else
    {
      xLast = nl->Append ( xLast, xNext );
    }
    pCurrentRoute->DeleteIfAllowed();
  }
  delete pRoutesIter;

  ///////////////////////
  // Output of all junctions
  GenericRelationIterator *pJunctionsIter = m_pJunctions->MakeScan();
  Tuple *pCurrentJunction;
  ListExpr xJunctions = nl->TheEmptyList();
  bFirst = true;

  while ( ( pCurrentJunction = pJunctionsIter->GetNextTuple() ) != 0 )
  {
    // Read values from table
    CcInt* pRoute1Id;
    pRoute1Id = ( CcInt* ) pCurrentJunction->GetAttribute ( JUNCTION_ROUTE1_ID
);
    int iRoute1Id = pRoute1Id->GetIntval();
    CcReal* pMeas1;
    pMeas1 = ( CcReal* ) pCurrentJunction->GetAttribute ( JUNCTION_ROUTE1_MEAS
);
    double dMeas1 = pMeas1->GetRealval();
    CcInt* pRoute2Id;
    pRoute2Id = ( CcInt* ) pCurrentJunction->GetAttribute ( JUNCTION_ROUTE2_ID
);
    int iRoute2Id = pRoute2Id->GetIntval();
    CcReal* pMeas2;
    pMeas2 = ( CcReal* ) pCurrentJunction->GetAttribute ( JUNCTION_ROUTE2_MEAS
);
    double dMeas2 = pMeas2->GetRealval();
    CcInt* pConnectivityCode;
    pConnectivityCode = ( CcInt* ) pCurrentJunction->GetAttribute ( JUNCTION_CC
);
    int iConnectivityCode= pConnectivityCode->GetIntval();
    Point* pPoint = ( Point* ) pCurrentJunction->GetAttribute ( JUNCTION_POS );
    ListExpr xPoint = OutPoint ( nl->TheEmptyList(), SetWord ( pPoint ) );

    // Build list
    xNext = nl->SixElemList ( nl->IntAtom ( iRoute1Id ),
                              nl->RealAtom ( dMeas1 ),
                              nl->IntAtom ( iRoute2Id ),
                              nl->RealAtom ( dMeas2 ),
                              nl->IntAtom ( iConnectivityCode ),
                              xPoint );

    // Create new list or append element to existing list
    if ( bFirst )
    {
      xJunctions= nl->OneElemList ( xNext );
      xLast = xJunctions;
      bFirst = false;
    }
    else
    {
      xLast = nl->Append ( xLast, xNext );
    }
    pCurrentJunction->DeleteIfAllowed();
  }

  delete pJunctionsIter;

  return nl->ThreeElemList ( nl->IntAtom ( m_iId ),
                             xRoutes,
                             xJunctions );
}



/*
~Save~-function of type constructor ~network~

*/
ListExpr Network::Save ( SmiRecord& in_xValueRecord,
                         size_t& inout_iOffset,
                         const ListExpr in_xTypeInfo )
{
  // Save id of the network
  int iId = m_iId;
  in_xValueRecord.Write ( &iId,
                          sizeof ( int ),
                          inout_iOffset );
  inout_iOffset += sizeof ( int );

  // Save routes
  ListExpr xType;
  nl->ReadFromString ( routesInternalTypeInfo, xType );
  ListExpr xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pRoutes->Save ( in_xValueRecord,
                          inout_iOffset,
                          xNumericType ) )
  {
    return false;
  }
  // Save junctions
  nl->ReadFromString ( junctionsInternalTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pJunctions->Save ( in_xValueRecord,
                             inout_iOffset,
                             xNumericType ) )
  {
    return false;
  }
  // Save sections
  nl->ReadFromString ( sectionsInternalTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pSections->Save ( in_xValueRecord,
                            inout_iOffset,
                            xNumericType ) )
  {
    return false;
  }
  // Save btree for routes
  nl->ReadFromString ( routesBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pBTreeRoutes->Save ( in_xValueRecord,
                               inout_iOffset,
                               xNumericType ) )
  {
    return false;
  }
  // Save btree for routesByStartposId
  nl->ReadFromString ( routesBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pBTreeRoutesByStartposId->Save ( in_xValueRecord,
                               inout_iOffset,
                               xNumericType ) )
  {
    return false;
  }
  // Save rtree for routes

  if ( !m_pRTreeRoutes->Save ( in_xValueRecord,
                               inout_iOffset ) )
  {
    return false;
  }
  // Save first btree for junctions
  nl->ReadFromString ( junctionsBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pBTreeJunctionsByRoute1->Save ( in_xValueRecord,
                                          inout_iOffset,
                                          xNumericType ) )
  {
    return false;
  }
  // Save second btree for junctions
  nl->ReadFromString ( junctionsBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pBTreeJunctionsByRoute2->Save ( in_xValueRecord,
                                          inout_iOffset,
                                          xNumericType ) )
  {
    return false;
  }

   //SmiFileId fileId = 0;
  //m_xAdjacencyList.SaveToRecord(in_xValueRecord, inout_iOffset, fileId);
  //m_xSubAdjacencyList.SaveToRecord(in_xValueRecord, inout_iOffset, fileId);
    //save m_xAdjacencyLlist
   //Flob *tmpAdjList = &m_xAdjacencyList;

 /*  cout << "before storing:" << endl;
   cout << "adjList" << ((Flob)m_xAdjacencyList) << endl;
   cout << "adjList" << ((Flob)m_xSubAdjacencyList) << endl;
  cout << "----" << endl;*/

   SecondoCatalog *ctlg = SecondoSystem::GetCatalog();
   SmiRecordFile *rf = ctlg->GetFlobFile();
   m_xAdjacencyList.saveToFile(rf, m_xAdjacencyList);
   SmiSize offset = 0;
   size_t bufsize = m_xAdjacencyList.headerSize()+ 2*sizeof(int);
   char* buf = (char*) malloc(bufsize);
   m_xAdjacencyList.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   free(buf);
    //save m_xSubAdjacencyList
   //Flob *tmpSubAdjList = &m_xSubAdjacencyList;
   // SmiRecordFile *rf1 = ctlg->GetFlobFile();
   m_xSubAdjacencyList.saveToFile(rf, m_xSubAdjacencyList);
   offset = 0;
   buf = (char*) malloc(bufsize);
   m_xSubAdjacencyList.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf,bufsize, inout_iOffset);
   free(buf);
   inout_iOffset += bufsize;


  // Save btree for sections
  nl->ReadFromString ( sectionsBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pBTreeSectionsByRoute->Save ( in_xValueRecord,
                                        inout_iOffset,
                                        xNumericType ) )
  {

    //cout << "cannot store BTree_sections" << endl;

    return false;
  }

  nl->ReadFromString ( sectionsBTreeTypeInfo, xType );
  xNumericType =SecondoSystem::GetCatalog()->NumericType ( xType );
  if ( !m_pBTreeSections->Save ( in_xValueRecord,
                                 inout_iOffset,
                                 xNumericType ) )
  {

    //cout << "cannot store BTree_sections" << endl;

    return false;
  }

  //save distance storage
  /*
  nl->ReadFromString(distancestorageTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!alldistance->Save(in_xValueRecord,
                                      inout_iOffset,
                                      xNumericType))
  {
    return false;
  }
  */

  return true;
}


/*
~Open~-function of type constructor ~network~

*/
Network *Network::Open ( SmiRecord& in_xValueRecord,
                         size_t& inout_iOffset,
                         const ListExpr in_xTypeInfo )
{
//cout << "NetworkOpen" << endl;
  // Create network
  return new Network ( in_xValueRecord,
                       inout_iOffset,
                       in_xTypeInfo );
}

/*
~NetworkProp~-funtion of the type constructor ~network~

*/
ListExpr Network::NetworkProp()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText ( examplelist,
                   "thenetwork(<id>,<routes-relation>, <junctions-relation>)" );

  return ( nl->TwoElemList (
               nl->TwoElemList ( nl->StringAtom ( "Creation" ),
                                 nl->StringAtom ( "Example Creation" ) ),
               nl->TwoElemList ( examplelist,
                      nl->StringAtom ( "(let n = thenetwork(id, r,j))" ) ) ));
}

/*
~OutNetwork~-funtion of the type constructor ~network~

*/
ListExpr Network::OutNetwork ( ListExpr typeInfo, Word value )
{
  Network *n = ( Network* ) value.addr;
  return n->Out ( typeInfo );
}

/*
~InNetwork~-funtion of the type constructor ~network~

*/
Word Network::InNetwork ( ListExpr in_xTypeInfo,
                          ListExpr in_xValue,
                          int in_iErrorPos,
                          ListExpr& inout_xErrorInfo,
                          bool& inout_bCorrect )
{
//cout << "inNetwork" << endl;
  Network* pNetwork = new Network ( in_xValue,
                                    in_iErrorPos,
                                    inout_xErrorInfo,
                                    inout_bCorrect );

  if ( inout_bCorrect )
  {
    return SetWord ( pNetwork );
  }
  else
  {
    delete pNetwork;
    return SetWord ( Address ( 0 ) );
  }
}

/*
~CreateNetwork~-funtion of the type constructor ~network~

*/
Word Network::CreateNetwork ( const ListExpr typeInfo )
{
  return SetWord ( new Network() );
}

/*
~CloseNetwork~-funtion of the type constructor ~network~

*/
void Network::CloseNetwork ( const ListExpr typeInfo, Word& w )
{
  delete static_cast<Network*> ( w.addr );
  w.addr = 0;

}

/*
~Clone~-function of type constructor ~network~

Not implemented yet.

*/
Word Network::CloneNetwork ( const ListExpr typeInfo, const Word& w )
{
  return SetWord ( Address ( 0 ) );
}

/*
~DeleteNetwork~-funtion of the type constructor ~network~

*/
void Network::DeleteNetwork ( const ListExpr typeInfo, Word& w )
{
  Network* n = ( Network* ) w.addr;
  //n->Destroy();
  delete n;
  w.addr = 0;
}

/*
~CheckNetwork~-funtion of the type constructor ~network~

*/
bool Network::CheckNetwork ( ListExpr type, ListExpr& errorInfo )
{
  return ( nl->IsEqual ( type, "network" ) );
}

/*
~CastNetwork~-funtion of the type constructor ~network~

*/
void* Network::CastNetwork ( void* addr )
{
  return ( 0 );
}

/*
~SaveNetwork~-funtion of the type constructor ~network~

*/
bool Network::SaveNetwork ( SmiRecord& valueRecord,
                            size_t& offset,
                            const ListExpr typeInfo,
                            Word& value )
{
// cout << "Save Network" << endl;
  Network *n = ( Network* ) value.addr;
  return n->Save ( valueRecord, offset, typeInfo );
}

/*
~OpenNetwork~-funtion of the type constructor ~network~

*/
bool Network::OpenNetwork ( SmiRecord& valueRecord,
                            size_t& offset,
                            const ListExpr typeInfo,
                            Word& value )
{
  value.addr = Network::Open ( valueRecord, offset, typeInfo );
  return value.addr != 0;
}

/*
~SizeOfNetwork~-funtion of the type constructor ~network~

*/
int Network::SizeOfNetwork()
{
  return 0;
}

/*
~IsDefined~-funtion of the type constructor ~network~

*/
int Network::IsDefined()
{
  return m_bDefined;
}


/*
Secondo TypeConstructor for class ~Network~

*/

TypeConstructor network ( "network",          Network::NetworkProp,
                          Network::OutNetwork,           Network::InNetwork,
                          0,                    0,
                          Network::CreateNetwork,        Network::DeleteNetwork,
                          Network::OpenNetwork,          Network::SaveNetwork,
                          Network::CloseNetwork,         Network::CloneNetwork,
                          Network::CastNetwork,          Network::SizeOfNetwork,
                          Network::CheckNetwork );


/*
3.2 class ~GLine~

3.2.1 Constructors

The simple constructor. Should not be used.

*/
GLine::GLine():Attribute()
{}

GLine::GLine ( int in_iSize ) :
    Attribute(true),
    m_xRouteIntervals ( in_iSize )
{
  SetDefined(true);
  m_bSorted = false;
  m_dLength = 0.0;
}

GLine::GLine ( const GLine* in_xOther ) :
    Attribute(in_xOther->IsDefined()),
    m_xRouteIntervals ( 0 )
{
  SetDefined(in_xOther->IsDefined());
  m_bSorted = in_xOther->m_bSorted;
  m_iNetworkId = in_xOther->m_iNetworkId;
  m_dLength = 0.0;
  // Iterate over all RouteIntervalls
  for ( int i = 0; i < in_xOther->m_xRouteIntervals.Size(); i++ )
  {
    // Get next Interval
    RouteInterval pCurrentInterval;
    in_xOther->m_xRouteIntervals.Get ( i, pCurrentInterval );

    int iRouteId = pCurrentInterval.GetRouteId();
    double dStart = pCurrentInterval.GetStartPos();
    double dEnd = pCurrentInterval.GetEndPos();
    AddRouteInterval ( iRouteId,
                       dStart,
                       dEnd );
  }
  TrimToSize();
}

GLine::GLine ( ListExpr in_xValue,
               int in_iErrorPos,
               ListExpr& inout_xErrorInfo,
               bool& inout_bCorrect ):
    Attribute(true)
{
  // Check the list
  if ( ! ( nl->ListLength ( in_xValue ) == 2 ) )
  {
    string strErrorMessage = "GLine(): List length must be 2.";
    inout_xErrorInfo = nl->Append ( inout_xErrorInfo,
                                    nl->StringAtom ( strErrorMessage ) );
    inout_bCorrect = false;
    SetDefined(false);
    m_bSorted = false;
    return;
  }

  // Split into the two parts
  ListExpr xNetworkIdList = nl->First ( in_xValue );
  ListExpr xRouteIntervalList = nl->Second ( in_xValue );

  // Check the parts
  if ( !nl->IsAtom ( xNetworkIdList ) ||
          nl->AtomType ( xNetworkIdList ) != IntType )
  {
    string strErrorMessage = "GLine(): Error while reading network-id.";
    inout_xErrorInfo = nl->Append ( inout_xErrorInfo,
                                    nl->StringAtom ( strErrorMessage ) );
    SetDefined(false);
    m_bSorted = false;
    inout_bCorrect = false;
    return;
  }

  m_iNetworkId = nl->IntValue ( xNetworkIdList );
  m_dLength = 0.0;
  if ( !nl->IsEmpty ( xRouteIntervalList ) )
  {
    // Iterate over all routes

    while ( !nl->IsEmpty ( xRouteIntervalList ) )
    {
      ListExpr xCurrentRouteInterval = nl->First ( xRouteIntervalList );
      xRouteIntervalList = nl->Rest ( xRouteIntervalList );

      if ( nl->ListLength ( xCurrentRouteInterval ) != 3 ||
              ( !nl->IsAtom ( nl->First ( xCurrentRouteInterval ) ) ) ||
              nl->AtomType ( nl->First ( xCurrentRouteInterval ) ) != IntType ||
              ( !nl->IsAtom ( nl->Second ( xCurrentRouteInterval ) ) ) ||
              nl->AtomType ( nl->Second ( xCurrentRouteInterval ) ) != RealType
||
              ( !nl->IsAtom ( nl->Third ( xCurrentRouteInterval ) ) ) ||
              nl->AtomType ( nl->Third ( xCurrentRouteInterval ) ) != RealType )
      {
        string strErrorMessage = "GLine(): Error while reading route-interval.";
        inout_xErrorInfo = nl->Append ( inout_xErrorInfo,
                                        nl->StringAtom ( strErrorMessage ) );
        inout_bCorrect = false;
        SetDefined(false);
        m_bSorted = false;
        return;
      }

      // Read attributes from list
      // Read values from table
      int iRouteId = nl->IntValue ( nl->First ( xCurrentRouteInterval ) );
      double dStart = nl->RealValue ( nl->Second ( xCurrentRouteInterval ) );
      double dEnd  = nl->RealValue ( nl->Third ( xCurrentRouteInterval ) );

      AddRouteInterval ( iRouteId,
                         dStart,
                         dEnd );

    }
    inout_bCorrect = true;
    SetDefined(true);
    m_bSorted = false;
  }
  else
  {
    SetDefined(false);
    m_bSorted = false;
    inout_bCorrect = true;
  }
  TrimToSize();
  return;
}

/*
2.2.2 Methods of class ~GLine~

*/
void GLine::SetNetworkId ( int in_iNetworkId )
{
  m_iNetworkId = in_iNetworkId;
  SetDefined(true);
}

void GLine::AddRouteInterval ( RouteInterval ri )
{
  m_xRouteIntervals.Append ( ri );
  m_dLength = m_dLength + fabs ( ri.GetEndPos() - ri.GetStartPos() );
}

void GLine::AddRouteInterval ( int in_iRouteId,
                               double in_dStart,
                               double in_dEnd )
{
  RouteInterval *ri = new RouteInterval ( in_iRouteId,
                                          in_dStart,
                                          in_dEnd );
  AddRouteInterval ( *ri );
  delete ri;
}


bool GLine::IsSorted()
{
  return m_bSorted;
}

void GLine::SetSorted ( bool in_bSorted )
{
  m_bSorted = in_bSorted;
}

/*
Secondo Integration

*/
Word GLine::In ( const ListExpr typeInfo, const ListExpr instance,
                 const int errorPos, ListExpr& errorInfo, bool& correct )
{
  GLine* pGline = new GLine ( 0 );
  if ( nl->ListLength ( instance ) == 0 )
  {
    correct = true;
    pGline->SetDefined ( false );
    return SetWord ( pGline );
  }
  if ( nl->ListLength ( instance ) != 2 )
  {
    correct = false;
    pGline->DeleteIfAllowed();
    cmsg.inFunError ( "Expecting (networkid (list of routeintervals))" );
    return SetWord ( Address ( 0 ) );
  }
  ListExpr FirstElem = nl->First ( instance );
  ListExpr SecondElem = nl->Second ( instance );
  if ( !nl->IsAtom ( FirstElem ) || !nl->AtomType ( FirstElem ) == IntType )
  {
    correct = false;
    pGline->DeleteIfAllowed();
    cmsg.inFunError ( "Networkadress is not evaluable" );
    return SetWord ( Address ( 0 ) );
  }
  pGline->SetNetworkId ( nl->IntValue ( FirstElem ) );
  if ( nl->IsEmpty ( SecondElem ) )
  {
    correct = false;
    pGline->DeleteIfAllowed();
    return SetWord ( Address ( 0 ) );
  }
  while ( !nl->IsEmpty ( SecondElem ) )
  {
    ListExpr start = nl->First ( SecondElem );
    SecondElem = nl->Rest ( SecondElem );
    if ( nl->ListLength ( start ) != 3 )
    {
      correct = false;
      pGline->DeleteIfAllowed();
      cmsg.inFunError ( "Routeinterval incorrect.Expected list of 3 Elements."
);
      return SetWord ( Address ( 0 ) );
    }
    ListExpr lrid = nl->First ( start );
    ListExpr lpos1 = nl->Second ( start );
    ListExpr lpos2 = nl->Third ( start );
    if ( !nl->IsAtom ( lrid ) || !nl->AtomType ( lrid ) == IntType ||
            !nl->IsAtom ( lpos1 ) || !nl->AtomType ( lpos1 ) == RealType ||
            !nl->IsAtom ( lpos2 ) || !nl->AtomType ( lpos2 ) == RealType )
    {
      correct = false;
      pGline->DeleteIfAllowed();
      cmsg.inFunError ( "Routeinterval should be list int, real, real." );
      return SetWord ( Address ( 0 ) );
    }
    pGline->AddRouteInterval ( nl->IntValue ( lrid ),
                               nl->RealValue ( lpos1 ),
                               nl->RealValue ( lpos2 ) );
  }
  correct = true;
  pGline->SetDefined(true);
  pGline->TrimToSize();
  return SetWord ( pGline );
}

ListExpr GLine::Out ( ListExpr in_xTypeInfo,
                      Word in_xValue )
{
  GLine *pGline = ( GLine* ) in_xValue.addr;

  if ( pGline == 0 || !pGline->IsDefined() )
  {
    return nl->SymbolAtom ( "undef" );
  }

  ListExpr xLast = nl->TheEmptyList();
  ListExpr xNext = nl->TheEmptyList();
  bool bFirst = true;
  ListExpr xNetworkId = nl->IntAtom ( pGline->m_iNetworkId );
  ListExpr xRouteIntervals = nl->TheEmptyList();
  // Iterate over all RouteIntervalls
  for ( int i = 0; i < pGline->m_xRouteIntervals.Size(); ++i )
  {
    // Get next Interval
    RouteInterval pCurrentInterval;
    pGline->m_xRouteIntervals.Get ( i, pCurrentInterval );

    int iRouteId = pCurrentInterval.GetRouteId();
    double dStart = pCurrentInterval.GetStartPos();
    double dEnd = pCurrentInterval.GetEndPos();
    // Build list
    xNext = nl->ThreeElemList ( nl->IntAtom ( iRouteId ),
                                nl->RealAtom ( dStart ),
                                nl->RealAtom ( dEnd ) );

    // Create new list or append element to existing list
    if ( bFirst )
    {
      xRouteIntervals = nl->OneElemList ( xNext );
      xLast = xRouteIntervals;
      bFirst = false;
    }
    else
    {
      xLast = nl->Append ( xLast, xNext );
    }
  }
  if ( pGline->m_xRouteIntervals.Size() == 0 )
  {
    xRouteIntervals = nl->TheEmptyList();
  }
  return nl->TwoElemList ( xNetworkId,
                           xRouteIntervals );
}

Word GLine::Create ( const ListExpr typeInfo )
{
  return SetWord ( new GLine ( 0 ) );
}

void GLine::Clear()
{
  m_xRouteIntervals.clean();
  SetSorted ( false );
  m_dLength = 0.0;
}

void GLine::Delete ( const ListExpr typeInfo,
                     Word& w )
{
  GLine *l = ( GLine* ) w.addr;
  //if (l->del.refs == 1) l->m_xRouteIntervals.Destroy();
  l->DeleteIfAllowed();
  w.addr = 0;
}

void GLine::Close ( const ListExpr typeInfo,
                    Word& w )
{
  ( ( GLine* ) w.addr )->DeleteIfAllowed();
  w.addr = 0;
}

Word GLine::CloneGLine ( const ListExpr typeInfo,
                         const Word& w )
{
  return SetWord ( ( ( GLine* ) w.addr )->Clone() );
}

GLine* GLine::Clone() const
{
  GLine *xOther = new GLine ( Size() );
  xOther->SetDefined ( IsDefined() );
  xOther->SetSorted ( m_bSorted );
  xOther->SetNetworkId ( m_iNetworkId );
  /*RouteInterval ri;
  for ( int i = 0; i < Size(); i++ )
  {
    Get ( i, ri );
    int rid = ri.GetRouteId();
    double start = ri.GetStartPos();
    double end = ri.GetEndPos();
    xOther->AddRouteInterval ( rid, start, end );
  }*/
  xOther->m_xRouteIntervals.copyFrom(m_xRouteIntervals);
  return xOther;
}

void* GLine::Cast ( void* addr )
{
  return new ( addr ) GLine;
}

int GLine::Size() const
{
  return m_xRouteIntervals.Size();
}

int GLine::SizeOf()
{
  return sizeof ( GLine );
}

size_t GLine::Sizeof() const
{
  return sizeof ( *this );
}

ostream& GLine::Print ( ostream& os ) const
{
  os << "GLine: NetworkId: " << m_iNetworkId << endl;
  for ( int i = 0; i < m_xRouteIntervals.Size() ; i++ )
  {
    RouteInterval ri;
    Get ( i, ri );
    os << "RouteInterval: " << i << " rid: " << ri.GetRouteId();
    os << " from: " << ri.GetStartPos() << " to: " << ri.GetEndPos();
    os << endl;
  }
  os << " end gline";
  return os;
};

bool GLine::Adjacent ( const Attribute* arg ) const
{
  return false;
}

/*
Compare

*/
int GLine::Compare ( const Attribute* arg ) const
{
  GLine *gl2 = ( GLine* ) arg;
  if ( IsDefined() && !gl2->IsDefined() ) return 1;
  else
    if ( !IsDefined() && gl2->IsDefined() ) return -1;
    else
      if ( !IsDefined() && !gl2->IsDefined() ) return 0;
      else
        if ( m_dLength < gl2->m_dLength ) return -1;
        else
          if ( m_dLength > gl2->m_dLength ) return 1;
          else
            if ( m_xRouteIntervals.Size() < gl2->m_xRouteIntervals.Size() )
              return -1;
            else
              if ( m_xRouteIntervals.Size() > gl2->m_xRouteIntervals.Size() )
                return 1;
              else
                if ( *this == *gl2 ) return 0;
                else
                {
                  RouteInterval ri1, ri2;
                  int i = 0;
                  while ( i < m_xRouteIntervals.Size() )
                  {
                    Get ( i,ri1 );
                    gl2->Get ( i,ri2 );
                    if ( ri1.GetRouteId() < ri2.GetRouteId() ) return -1;
                    else
                      if ( ri1.GetRouteId() > ri2.GetRouteId() ) return 1;
                      else
                        if ( ri1.GetStartPos() < ri2.GetStartPos() ) return -1;
                        else
                          if ( ri1.GetStartPos() > ri2.GetStartPos() ) return 1;
                          else
                            if ( ri1.GetEndPos() < ri2.GetEndPos() ) return -1;
                            else
                              if ( ri1.GetEndPos() > ri2.GetEndPos() ) return 1;
                    i++;
                  }
                }
  return 0;
}

GLine& GLine::operator= ( const GLine& l )
{
  m_xRouteIntervals.copyFrom(l.m_xRouteIntervals);
  m_bSorted = l.m_bSorted;
  SetDefined(l.IsDefined());
  m_iNetworkId = l.m_iNetworkId;
  TrimToSize();
  return *this;
}

bool GLine::operator== ( const GLine& l ) const
{
  if ( !IsDefined() || !l.IsDefined())
  {
    return false;
  }
  else
  {
    RouteInterval rIt, rIl;
    if ( m_xRouteIntervals.Size() == l.m_xRouteIntervals.Size() &&
            AlmostEqual ( m_dLength, l.m_dLength ) )
    {
      if ( m_bSorted && l.m_bSorted )
      {
        for ( int i=0; i < m_xRouteIntervals.Size(); i++ )
        {
          Get ( i,rIt );
          l.Get ( i,rIl );
          if ( ! ( rIt.GetRouteId() == rIl.GetRouteId() &&
                   rIt.GetStartPos() == rIl.GetStartPos() &&
                   rIt.GetEndPos() == rIl.GetEndPos() ) ) return false;
        }
        return true;
      }
      else
      {
        for ( int i=0; i < m_xRouteIntervals.Size(); i++ )
        {
          Get ( i,rIt );
          for ( int j = 0; j < m_xRouteIntervals.Size(); j++ )
          {
            l.Get ( i,rIl );
            if ( ! ( rIt.GetRouteId() == rIl.GetRouteId() &&
                     rIt.GetStartPos() == rIl.GetStartPos() &&
                     rIt.GetEndPos() == rIl.GetEndPos() ) ) return false;
          }
        }
        return true;
      }
    }
    else return false;
  }
}

size_t GLine::HashValue() const
{
  size_t xHash = m_iNetworkId;

  // Iterate over all RouteIntervalls
  for ( int i = 0; i < m_xRouteIntervals.Size(); ++i )
  {
    // Get next Interval
    RouteInterval pCurrentInterval;
    m_xRouteIntervals.Get ( i, pCurrentInterval );

    // Add something for each entry
    int iRouteId = pCurrentInterval.GetRouteId();
    double dStart = pCurrentInterval.GetStartPos();
    double dEnd = pCurrentInterval.GetEndPos();
    xHash += iRouteId + ( size_t ) dStart + ( size_t ) dEnd;
  }
  return xHash;
}

int GLine::NumOfFLOBs() const
{
  return 1;
}

Flob* GLine::GetFLOB ( const int i )
{
  if ( i == 0 ) return &m_xRouteIntervals;
  return 0;
}

DbArray<RouteInterval>* GLine::GetRouteIntervals()
{
  if ( IsDefined() ) return &m_xRouteIntervals;
  else return 0;
};

void GLine::CopyFrom ( const Attribute* right )
{
  *this = * ( ( const GLine* ) right );
  //Clear();
  /*
  GLine *src = (GLine*) right;
  SetDefined(src->IsDefined());
  SetSorted(src->IsSorted());
  const RouteInterval* ri;
  for (int i = 0; i < src->Size(); i++) {
    src->Get(i,ri);
    int rid = ri.GetRouteId();
    double start = ri.GetStartPos();
    double end = ri.GetEndPos();
    AddRouteInterval(rid, start, end);
  }
  */
}

double GLine::GetLength()
{
  return m_dLength;
}

int GLine::GetNetworkId()
{
  return  m_iNetworkId;
};

/*
~Get~ returns the route interval at position i in the route intervals ~DbArray~.

*/

void GLine::Get ( const int i, RouteInterval &ri ) const
{
  m_xRouteIntervals.Get ( i, ri );
};

int GLine::NoOfComponents()
{
  return m_xRouteIntervals.Size();
};


ListExpr GLine::Property()
{
  return ( nl->TwoElemList (
               nl->FourElemList ( nl->StringAtom ( "Signature" ),
                                  nl->StringAtom ( "Example Type List" ),
                                  nl->StringAtom ( "List Rep" ),
                                  nl->StringAtom ( "Example List" ) ),
               nl->FourElemList (
                  nl->StringAtom ( "-> DATA" ),
                  nl->StringAtom ( "gline" ),
                  nl->StringAtom ( "(<nid> ((<rid> <startpos><endpos>)...))"),
                  nl->StringAtom ( "(1 ((1 1.5 2.5)(2 1.5 2.0)))" ) ) ) );
}

bool GLine::Check ( ListExpr type, ListExpr& errorInfo )
{
  return ( nl->IsEqual ( type, "gline" ) );
}

/*
Netdistance method computes the network distance between two glines. Uses
network distance method of ~GPoint~.

*/

double GLine::Netdistance ( GLine* pgl2 )
{
  //GLine* pgl1 = (GLine*) this;
  double minDist = numeric_limits<double>::max();
  double aktDist = numeric_limits<double>::max();
  if ( GetNetworkId() != pgl2->GetNetworkId() )
  {
    cmsg.inFunError ( "Both glines must belong to the network." );
    return minDist;
  }
  GPoints *bGPgl1 = GetBGP();
  GPoints *bGPgl2 = pgl2->GetBGP();
  GPoint gp1, gp2;
  for ( int i = 0; i < bGPgl1->Size(); i++ )
  {
    bGPgl1->Get ( i,gp1 );
    if ( gp1.Inside ( pgl2 ) )
    {
      bGPgl1->DeleteIfAllowed();
      bGPgl2->DeleteIfAllowed();
      return 0.0;
    }
    for ( int j = 0; j < bGPgl2->Size(); j++ )
    {
      bGPgl2->Get ( j, gp2 );
      if ( gp2.Inside ( this ) )
      {
        bGPgl1->DeleteIfAllowed();
        bGPgl2->DeleteIfAllowed();
        return 0.0;
      }
      aktDist = gp1.Netdistance ( &gp2 );
      if ( aktDist < minDist ) minDist = aktDist;
      if ( minDist <= 0.0 )
      {
        bGPgl1->DeleteIfAllowed();
        bGPgl2->DeleteIfAllowed();
        return 0.0;
      }
    }
  }
  bGPgl1->DeleteIfAllowed();
  bGPgl2->DeleteIfAllowed();
  return minDist;
}

/*
Distance method computes the Euclidean Distance between two glines. Uses
distance method of ~GPoint~.

*/

double GLine::Distance ( GLine* pgl2 )
{
  Line *l1 = new Line ( 0 );
  Line *l2 = new Line ( 0 );
  Gline2line ( l1 );
  pgl2->Gline2line ( l2 );
  if ( l1->IsDefined() && l2->IsDefined() )
  {
    double res = l1->Distance ( *l2 );
    l1->DeleteIfAllowed();
    l2->DeleteIfAllowed();
    return res;
  }
  else return numeric_limits<double>::max();
}


void GLine::Uniongl ( GLine *pgl2, GLine *res )
{
  RouteInterval pRi1, pRi2;
  if ( !IsDefined() || NoOfComponents() == 0 )
  {
    if ( pgl2->IsDefined() && pgl2->NoOfComponents() > 0 )
    {
      if ( pgl2->IsSorted() )
      {
        for ( int j = 0; j < pgl2->NoOfComponents(); j++ )
        {
          pgl2->Get ( j,pRi2 );
          res->AddRouteInterval ( pRi2.GetRouteId(),
                                  pRi2.GetStartPos(),
                                  pRi2.GetEndPos() );
        }
      }
      else
      {
        pgl2->Get ( 0,pRi2 );
        RITree *ritree = new RITree ( pRi2.GetRouteId(),
                                      pRi2.GetStartPos(), pRi2.GetEndPos(),0,0
);
        for ( int j = 1; j < pgl2->NoOfComponents(); j++ )
        {
          pgl2->Get ( j,pRi2 );
          ritree->Insert ( pRi2.GetRouteId(), pRi2.GetStartPos(),
                           pRi2.GetEndPos() );
        }
        ritree->TreeToGLine ( res );
        ritree->RemoveTree();
      }
      res->SetDefined ( true );
      res->SetSorted ( true );
      res->SetNetworkId ( pgl2->GetNetworkId() );
    }
    else
    {
      res->SetDefined ( false );
      res->SetSorted ( false );
    }
  }
  else
  {
    if ( !pgl2->IsDefined() || pgl2->NoOfComponents() == 0 )
    {
      if ( IsDefined() && NoOfComponents() >0 )
      {
        if ( IsSorted() )
        {
          for ( int i = 0; i < NoOfComponents(); i++ )
          {
            Get ( i,pRi1 );
            res->AddRouteInterval ( pRi1.GetRouteId(),
                                    pRi1.GetStartPos(),
                                    pRi1.GetEndPos() );
          }
        }
        else
        {
          Get ( 0,pRi1 );
          RITree *ritree = new RITree ( pRi1.GetRouteId(),
                                        pRi1.GetStartPos(), pRi1.GetEndPos(),0,0
);
          for ( int i = 1; i < NoOfComponents(); i++ )
          {
            Get ( i,pRi1 );
            ritree->Insert ( pRi1.GetRouteId(), pRi1.GetStartPos(),
                             pRi1.GetEndPos() );
          }
          ritree->TreeToGLine ( res );
          ritree->RemoveTree();
        }
        res->SetDefined ( true );
        res->SetSorted ( true );
        res->SetNetworkId ( GetNetworkId() );
      }
      else
      {
        res->SetDefined ( false );
        res->SetSorted ( false );
      }
    }
    else
    {
      if ( GetNetworkId() != pgl2->GetNetworkId() )
      {
        res->SetDefined ( false );
        res->SetSorted ( false );
      }
      else
      {
        res->SetNetworkId ( GetNetworkId() );
        if ( IsSorted() && pgl2->IsSorted() )
        {
          int i=0;
          int j=0;
          bool newroute = false;
          int iRouteId;
          double start, end;
          while ( i < NoOfComponents() && j < pgl2->NoOfComponents() )
          {
            Get ( i, pRi1 );
            pgl2->Get ( j, pRi2 );
            if ( pRi1.GetRouteId() < pRi2.GetRouteId() )
            {
              res->AddRouteInterval ( pRi1.GetRouteId(),
                                      pRi1.GetStartPos(),
                                      pRi1.GetEndPos() );
              i++;
            }
            else
            {
              if ( pRi1.GetRouteId() > pRi2.GetRouteId() )
              {
                res->AddRouteInterval ( pRi2.GetRouteId(), pRi2.GetStartPos(),
                                        pRi2.GetEndPos() );
                j++;
              }
              else
              {
                if ( pRi1.GetEndPos() < pRi2.GetStartPos() )
                {
                  res->AddRouteInterval ( pRi1.GetRouteId(), pRi1.GetStartPos(),
                                          pRi1.GetEndPos() );
                  i++;
                }
                else
                {
                  if ( pRi2.GetEndPos() < pRi1.GetStartPos() )
                  {
                    res->AddRouteInterval ( pRi2.GetRouteId(),
                                            pRi2.GetStartPos(),
                                            pRi2.GetEndPos() );
                    j++;
                  }
                  else
                  {
                    iRouteId = pRi1.GetRouteId();
                    start = min ( pRi1.GetStartPos(), pRi2.GetStartPos() ),
                            end = max ( pRi1.GetEndPos(), pRi2.GetEndPos() );
                    i++;
                    j++;
                    newroute = false;
                    while ( i < NoOfComponents() && !newroute )
                    {
                      Get ( i,pRi1 );
                      if ( pRi1.GetRouteId() == iRouteId )
                      {
                        if ( pRi1.GetStartPos() <= end )
                        {
                          end = max ( pRi1.GetEndPos(), end );
                          i++;
                        }
                        else newroute = true;
                      }
                      else newroute = true;
                    }
                    newroute = false;
                    while ( j < pgl2->NoOfComponents() && !newroute )
                    {
                      pgl2->Get ( j,pRi2 );
                      if ( pRi2.GetRouteId() == iRouteId )
                      {
                        if ( pRi2.GetStartPos() <= end )
                        {
                          end = max ( pRi2.GetEndPos(), end );
                          j++;
                        }
                        else newroute = true;
                      }
                      else newroute = true;
                    }
                    res->AddRouteInterval ( iRouteId, start, end );
                  }
                }
              }
            }
          }
          while ( i < NoOfComponents() )
          {
            Get ( i,pRi1 );
            res->AddRouteInterval ( pRi1.GetRouteId(),
                                    pRi1.GetStartPos(),
                                    pRi1.GetEndPos() );
            i++;
          }
          while ( j < pgl2->NoOfComponents() )
          {
            pgl2->Get ( j,pRi2 );
            res->AddRouteInterval ( pRi2.GetRouteId(), pRi2.GetStartPos(),
                                    pRi2.GetEndPos() );
            j++;
          }
          res->SetDefined ( true );
          res->SetSorted ( true );
        }
        else
        {
          RITree *ritree;
          Get ( 0,pRi1 );
          ritree = new RITree ( pRi1.GetRouteId(),
                                pRi1.GetStartPos(), pRi1.GetEndPos(),0,0 );
          for ( int i = 1; i < NoOfComponents(); i++ )
          {
            Get ( i,pRi1 );
            ritree->Insert ( pRi1.GetRouteId(), pRi1.GetStartPos(),
                             pRi1.GetEndPos() );
          }
          for ( int j = 0; j < pgl2->NoOfComponents(); j++ )
          {
            pgl2->Get ( j,pRi2 );
            ritree->Insert ( pRi2.GetRouteId(), pRi2.GetStartPos(),
                             pRi2.GetEndPos() );
          }
          ritree->TreeToGLine ( res );
          ritree->RemoveTree();
          res->SetDefined ( true );
          res->SetSorted ( true );
        }
      }
    }
  }
  res->TrimToSize();
}

void GLine::Gline2line ( Line* res )
{
  res->Clear();
  if ( IsDefined() && NoOfComponents() > 0 )
  {
    //Network* pNetwork = NetworkManager::GetNetwork(GetNetworkId());
    Network* pNetwork = NetworkManager::GetNetworkNew ( GetNetworkId(), netList
);
    RouteInterval rI;
    Line *l = new Line ( 0 );
    Line *x = l;
    for ( int i=0; i < this->NoOfComponents(); i++ )
    {
      this->Get ( i,rI );
      SimpleLine *pSubline = new SimpleLine ( 0 );
      pNetwork->GetLineValueOfRouteInterval ( &rI, pSubline );
      if ( pSubline->IsDefined() )
      {
        Line *partLine = new Line ( 0 );
        pSubline->toLine ( *partLine );
        pSubline->DeleteIfAllowed();
        x = SetOp ( *l, *partLine, avlseg::union_op );
        partLine->DeleteIfAllowed();
        l->DeleteIfAllowed();
        l = x;
      }
    }
    NetworkManager::CloseNetwork ( pNetwork );
    ( *res ) = *x;
    x->DeleteIfAllowed();
    res->SetDefined ( true );
  }
  else
  {
    if ( IsDefined() && NoOfComponents() == 0 )
    {
      res->SetDefined ( true );
    }
    else
    {
      res->SetDefined ( false );
    }
  }
  res->TrimToSize();
}

bool GLine::Intersects ( GLine *pgl )
{
  RouteInterval pRi1, pRi2;
  if ( !IsSorted() )
  {
    for ( int i = 0; i < NoOfComponents(); i++ )
    {
      Get ( i, pRi1 );
      if ( pgl->IsSorted() )
      {
        if ( searchUnit ( pgl, 0, pgl->NoOfComponents()-1, pRi1 ) )
        {
          return true;
        };
      }
      else
      {
        for ( int j = 0 ; j < pgl->NoOfComponents(); j ++ )
        {
          pgl->Get ( j,pRi2 );
          if ( pRi1.GetRouteId() == pRi2.GetRouteId() &&
                  ( ! ( pRi1.GetEndPos() < pRi2.GetStartPos() ||
                        pRi2.GetStartPos() > pRi1.GetEndPos() ) ) )
          {
            return true;
          }
        }
      }
    }
  }
  else
  {
    if ( pgl->IsSorted() )
    {
      int i = 0;
      int j = 0;
      while ( i<NoOfComponents() && j < pgl->NoOfComponents() )
      {
        Get ( i,pRi1 );
        pgl->Get ( j,pRi2 );
        if ( pRi1.GetRouteId() < pRi2.GetRouteId() ) i++;
        else
          if ( pRi1.GetRouteId() > pRi2.GetRouteId() ) j++;
          else
            if ( pRi1.GetStartPos() > pRi2.GetEndPos() ) j++;
            else
              if ( pRi1.GetEndPos() < pRi2.GetStartPos() ) i++;
              else return true;
      }
    }
    else
    {
      for ( int i = 0; i < pgl->NoOfComponents(); i++ )
      {
        pgl->Get ( i, pRi2 );
        if ( searchUnit ( this, 0, NoOfComponents()-1, pRi2 ) ) return true;
      }
    }
  }
  return false;
}


/*
Secondo Type Constructor for class ~GLine~

*/
TypeConstructor gline (
    "gline",                       //name
    GLine::Property,               //property function
    GLine::Out, GLine::In,         //Out and In functions
    0, 0,                          //SaveToList and
    //RestoreFromList functions
    GLine::Create, GLine::Delete,  //object creation and deletion
    OpenAttribute<GLine>, SaveAttribute<GLine>,  //open and save functions
    GLine::Close, GLine::CloneGLine,    //object close, and clone
    GLine::Cast,                   //cast function
    GLine::SizeOf,                 //sizeof function
    GLine::Check );                //kind checking function


/*
3.3 class ~GPoint~

3.3.1 Constructors

See ~network.h~ class definition of ~GPoint~

3.3.2 Methods of class ~GPoint~

*/
Word GPoint::InGPoint ( const ListExpr typeInfo,
                        const ListExpr instance,
                        const int errorPos,
                        ListExpr& errorInfo,
                        bool& correct )
{
  if ( nl->ListLength ( instance ) == 4 )
  {
    if ( nl->IsAtom ( nl->First ( instance ) ) &&
            nl->AtomType ( nl->First ( instance ) ) == IntType &&
            nl->IsAtom ( nl->Second ( instance ) ) &&
            nl->AtomType ( nl->Second ( instance ) ) == IntType &&
            nl->IsAtom ( nl->Third ( instance ) ) &&
            nl->AtomType ( nl->Third ( instance ) ) == RealType &&
            nl->IsAtom ( nl->Fourth ( instance ) ) &&
            nl->AtomType ( nl->Fourth ( instance ) ) == IntType )
    {
      GPoint *gp = new GPoint (
          true,
          nl->IntValue ( nl->First ( instance ) ),
          nl->IntValue ( nl->Second ( instance ) ),
          nl->RealValue ( nl->Third ( instance ) ),
          ( Side ) nl->IntValue ( nl->Fourth ( instance ) ) );
      correct = true;
      return SetWord ( gp );
    }
  }

  correct = false;
  return SetWord ( Address ( 0 ) );
}

ListExpr GPoint::OutGPoint ( ListExpr typeInfo, Word value )
{
  GPoint *gp = ( GPoint* ) value.addr;

  if ( gp->IsDefined() )
  {
    return nl->FourElemList (
               nl->IntAtom ( gp->GetNetworkId() ),
               nl->IntAtom ( gp->GetRouteId() ),
               nl->RealAtom ( gp->GetPosition() ),
               nl->IntAtom ( gp->GetSide() ) );
  }
  return nl->SymbolAtom ( "undef" );
}

Word GPoint::CreateGPoint ( const ListExpr typeInfo )
{
  return SetWord ( new GPoint ( false ) );
}

void GPoint::DeleteGPoint ( const ListExpr typeInfo, Word& w )
{
  GPoint *gp = ( GPoint* ) w.addr;
  if (gp->DeleteIfAllowed()) w.addr = 0;
}

void GPoint::CloseGPoint ( const ListExpr typeInfo, Word& w )
{
  GPoint *gp = ( GPoint* ) w.addr;
  if(gp->DeleteIfAllowed()) w.addr = 0;
}

Word GPoint::CloneGPoint ( const ListExpr typeInfo, const Word& w )
{
  return SetWord ( ( ( GPoint* ) w.addr )->Clone() );
}

void* GPoint::CastGPoint ( void* addr )
{
  return new ( addr ) GPoint;
}

int GPoint::SizeOfGPoint()
{
  return sizeof ( GPoint );
}

ListExpr GPoint::GPointProperty()
{
  return ( nl->TwoElemList (
               nl->FourElemList ( nl->StringAtom ( "Signature" ),
                                  nl->StringAtom ( "Example Type List" ),
                                  nl->StringAtom ( "List Rep" ),
                                  nl->StringAtom ( "Example List" ) ),
               nl->FourElemList ( nl->StringAtom ( "-> DATA" ),
                                  nl->StringAtom ( "gpoint" ),
                                  nl->StringAtom ( "(<network_id> <route_id> "
                                                   "<position> <side>)" ),
                                  nl->StringAtom ( "(1 1 0.0 0)" ) ) ) );
}

bool GPoint::CheckGPoint ( ListExpr type, ListExpr& errorInfo )
{
  return ( nl->IsEqual ( type, "gpoint" ) );
}

/*
Netdistance function computes the network distance between two ~GPoint~s.
Using Dijkstras-Algorithm for shortest path computing

*/

double GPoint::Netdistance ( GPoint* pToGPoint )
{
  GLine* pGLine = new GLine ( 0 );
  double res;
  if ( ShortestPath ( pToGPoint, pGLine ) )
  {
    res = pGLine->GetLength();
    pGLine->DeleteIfAllowed();
    return res;
  }
  else
  {
    pGLine->DeleteIfAllowed();
    return 0;
  }
}
double GPoint::NewNetdistance ( GPoint* pToGPoint,GLine* gline )
{
  GPoint* gp1 = new GPoint ( true,GetNetworkId(),GetRouteId(),GetPosition() );
  GPoint* gp2 = new GPoint ( true,pToGPoint->GetNetworkId(),
                             pToGPoint->GetRouteId(),pToGPoint->GetPosition() );

  double res;
  if ( gp1->ShortestPath ( gp2, gline ) )
  {
    res = gline->GetLength();

    gp1->DeleteIfAllowed();
    gp2->DeleteIfAllowed();
    return res;
  }
  else
  {

    gp1->DeleteIfAllowed();
    gp2->DeleteIfAllowed();
    return 0;
  }

}

/*
Distance function computes the Euclidean Distance between two ~GPoint~s.

*/

double GPoint::Distance ( GPoint* pToGPoint )
{
  map<int,string>::iterator it = netList->begin();
  if ( IsDefined() && pToGPoint->IsDefined() &&
          GetNetworkId() == pToGPoint->GetNetworkId() )
  {
    //Network* pNetwork=NetworkManager::GetNetwork(GetNetworkId());
    Network* pNetwork = NetworkManager::GetNetworkNew ( GetNetworkId(),
                        netList );
    Point *from = new Point ( false );
    pNetwork->GetPointOnRoute ( this, from );
    Point *to = new Point ( false );
    pNetwork->GetPointOnRoute ( pToGPoint,to );
    double res = from->Distance ( *to );
    from->DeleteIfAllowed();
    to->DeleteIfAllowed();
    NetworkManager::CloseNetwork ( pNetwork );
    return res;
  }
  else return numeric_limits<double>::max();
}

/*
Returns true if the gpoint is inside the gline false elsewhere.

*/
bool GPoint::Inside ( GLine *gl )
{
  if ( ! ( gl->IsDefined() ) || !IsDefined() ||
          gl->NoOfComponents() < 1 ) return false;
  if ( GetNetworkId() != gl->GetNetworkId() ) return false;
  RouteInterval pCurrRInter;
  if ( gl->IsSorted() )
    return ( searchRouteInterval ( this, gl, 0, gl->NoOfComponents()-1 ) );
  else
  {
    int i = 0;
    while ( i < gl->NoOfComponents() )
    {
      gl->Get ( i, pCurrRInter );
      if ( pCurrRInter.GetRouteId() == GetRouteId() )
      {
        if ( pCurrRInter.GetStartPos() < GetPosition() &&
                GetPosition() < pCurrRInter.GetEndPos() )
          return true;
        if ( pCurrRInter.GetStartPos() > GetPosition() &&
                GetPosition() > pCurrRInter.GetEndPos() )
          return true;
        if ( fabs ( pCurrRInter.GetStartPos() - GetPosition() ) < 0.1 )
          return true;
        if ( fabs ( pCurrRInter.GetEndPos() - GetPosition() ) < 0.1 )
          return true;
      }
      i++;
    }
    return false;
  }
  return false;
}

bool GPoint::operator== ( const GPoint& p ) const
{
  if ( !IsDefined() || !p.IsDefined() ) return false;
  else
  {
    if ( m_iNetworkId == p.GetNetworkId() &&
            m_xRouteLocation.rid == p.GetRouteId() &&
            m_xRouteLocation.d == p.GetPosition() &&
            ( m_xRouteLocation.side == p.GetSide() || m_xRouteLocation.side == 2
||
              p.GetSide() == 2 ) )
    {
      return true;
    }
    else return false;
  }
}

bool GPoint::operator!= ( const GPoint& p ) const
{
  return ! ( *this == p );
}

/*
Computes the shortest path between start and end in the network. Using
Dijkstras Algorithm. The path is returned as gline value.

*/

bool GPoint::ShortestPath ( GPoint *to, GLine *result )
{

  GPoint* start =
       new GPoint ( true,GetNetworkId(),GetRouteId(),GetPosition(), GetSide() );
  GPoint* end = new GPoint ( true,to->GetNetworkId(),to->GetRouteId(),
                             to->GetPosition(),to->GetSide() );//copy the gpoint
  result->Clear();
  if ( start == 0 || end == 0 || !start->IsDefined() ||
          !end->IsDefined() )
  {
    sendMessage ( "Both gpoints must exist and be defined." );
    result->SetDefined ( false );
    start->DeleteIfAllowed();
    end->DeleteIfAllowed();
    return false;
  }
  // Check wether both points belong to the same network
  if ( start->GetNetworkId() != end->GetNetworkId() )
  {
    sendMessage ( "Both gpoints belong to different networks." );
    result->SetDefined ( false );
    start->DeleteIfAllowed();
    end->DeleteIfAllowed();
    return false;
  }
  result->SetNetworkId ( start->GetNetworkId() );
  // Load the network
  //Network* pNetwork = NetworkManager::GetNetwork(start->GetNetworkId());
  Network* pNetwork = NetworkManager::GetNetworkNew ( start->GetNetworkId(),
                      netList );
  if ( pNetwork == 0 )
  {
    sendMessage ( "Network not found." );
    result->SetDefined ( false );
    start->DeleteIfAllowed();
    end->DeleteIfAllowed();
    return false;
  }
  // Get sections where the path should start or end
  TupleId startSectTID = pNetwork->GetTupleIdSectionOnRoute ( start );
  Tuple* startSection = pNetwork->GetSection ( startSectTID );
  if ( startSection == 0 )
  {
    sendMessage ( "Starting GPoint not found in network." );
    NetworkManager::CloseNetwork ( pNetwork );
    result->SetDefined ( false );
    start->DeleteIfAllowed();
    end->DeleteIfAllowed();
    return false;
  }
  TupleId lastSectTID = pNetwork->GetTupleIdSectionOnRoute ( end );
  Tuple* endSection = pNetwork->GetSection ( lastSectTID );
  if ( endSection == 0 )
  {
    sendMessage ( "End GPoint not found in network." );
    startSection->DeleteIfAllowed();
    NetworkManager::CloseNetwork ( pNetwork );
    result->SetDefined ( false );
    start->DeleteIfAllowed();
    end->DeleteIfAllowed();
    return false;
  }
////////////////////////////////////////////////////
  /* bool junctionpoint = false;
  Point* endp = new Point();
  pNetwork->GetPointOnRoute(end,endp); //end point
  Point* startp = new Point();
  pNetwork->GetPointOnRoute(start,startp);
  if(fabs(endp->GetX() - startp->GetX()) < 0.1 &&
     fabs(endp->GetY() - startp->GetY()) < 0.1){
  endp->DeleteIfAllowed();
  startp->DeleteIfAllowed();
  start->DeleteIfAllowed();
  end->DeleteIfAllowed();
    startSection->DeleteIfAllowed();
    endSection->DeleteIfAllowed();
    NetworkManager::CloseNetwork(pNetwork);
    return false;
  }

  vector<JunctionSortEntry> juns;
  CcInt* routeid = new CcInt(true,end->GetRouteId());
  pNetwork->GetJunctionsOnRoute(routeid,juns);
  for(unsigned int i = 0;i < juns.size();i++){
    Tuple* t = juns[i].m_pJunction;
    if(((CcInt*)t->GetAttribute(JUNCTION_ROUTE1_ID))->GetIntval() ==
        end->GetRouteId() &&
      fabs(((CcReal*)t->GetAttribute(JUNCTION_ROUTE1_MEAS))->GetRealval()-
      end->GetPosition()) < 0.1)
      junctionpoint = true;
    if(((CcInt*)t->GetAttribute(JUNCTION_ROUTE2_ID))->GetIntval() ==
        end->GetRouteId() &&
      fabs(((CcReal*)t->GetAttribute(JUNCTION_ROUTE2_MEAS))->GetRealval()-
      end->GetPosition()) < 0.1)
      junctionpoint = true;
  }
  vector<TupleId> secjunid;
  if(junctionpoint){//it is a junction point
    vector<DirectedSection> sectionlist;
    if(fabs(end->GetPosition()-
      ((CcReal*)endSection->GetAttribute(SECTION_MEAS1))->GetRealval())< 0.1)
      pNetwork->GetAdjacentSections(endSection->GetTupleId(),false,sectionlist);
    else
      pNetwork->GetAdjacentSections(endSection->GetTupleId(),true,sectionlist);
    for(unsigned int i = 0;i < sectionlist.size();i++){
      if(sectionlist[i].GetSectionTid() != endSection->GetTupleId())
        secjunid.push_back(sectionlist[i].GetSectionTid());
    }
  }
  endp->DeleteIfAllowed();
  routeid->DeleteIfAllowed();*/
/////////////////////////////////////////////////////
  /*
  Calculate the shortest path using dijkstras algorithm.

  */

  if ( startSectTID == lastSectTID  ||
          GetRouteId() == to->GetRouteId() )
  {
    result->AddRouteInterval ( start->GetRouteId(), start->GetPosition(),
                               end->GetPosition() );
  }
  else
  {
    /*
    Initialize PriorityQueue

    */
    PrioQueue *prioQ = new PrioQueue ( 0 );
    SectIDTree *visitedSect = 0;
    double sectMeas1 =
        ( ( CcReal* ) startSection->GetAttribute ( SECTION_MEAS1 )
)->GetRealval();
    double sectMeas2 =
        ( ( CcReal* ) startSection->GetAttribute ( SECTION_MEAS2 )
)->GetRealval();

    double dist = 0.0;
    vector<DirectedSectionInfo> adjSectionList;
    adjSectionList.clear();
    if ( start->GetSide() == 0 )
    {
      dist = start->GetPosition() - sectMeas1;
      pNetwork->GetAdjacentSectionsInfo ( startSectTID, false, adjSectionList );
      SectIDTree *startTree = new SectIDTree ( startSectTID,
              ( TupleId ) numeric_limits<long>::max(),
              false,
              numeric_limits<int>::max() );
      visitedSect = startTree;
      for ( size_t i = 0;  i < adjSectionList.size(); i++ )
      {
        DirectedSectionInfo actNextSect = adjSectionList[i];
        if ( actNextSect.GetSectionTid() != startSectTID )
        {
          PQEntry *actEntry = new PQEntry ( actNextSect.GetSectionTid(), dist,
                                            actNextSect.GetUpDownFlag(),
                                            startSectTID,
                                            actNextSect.GetMeas1(),
                                            actNextSect.GetMeas2(),
                                            actNextSect.GetRid());
          prioQ->Insert ( *actEntry, visitedSect ) ;
          delete actEntry;
        }
      }
      adjSectionList.clear();
    }
    else
    {
      if ( start->GetSide() == 1 )
      {
        dist = sectMeas2 - start->GetPosition();
        SectIDTree *startTree = new SectIDTree ( startSectTID,
                ( TupleId ) numeric_limits<long>::max(),
                true,
                numeric_limits<int>::max() );
        visitedSect = startTree;
        pNetwork->GetAdjacentSectionsInfo ( startSectTID, true, adjSectionList);
        for ( size_t i = 0;  i < adjSectionList.size(); i++ )
        {
          DirectedSectionInfo actNextSect = adjSectionList[i];
          if ( actNextSect.GetSectionTid() != startSectTID )
          {
            PQEntry *actEntry = new PQEntry ( actNextSect.GetSectionTid(), dist,
                                              actNextSect.GetUpDownFlag(),
                                              startSectTID,
                                              actNextSect.GetMeas1(),
                                              actNextSect.GetMeas2(),
                                              actNextSect.GetRid() );
            prioQ->Insert ( *actEntry, visitedSect );
            delete actEntry;
          }
        }
        adjSectionList.clear();
      }
      else
      {

       dist = start->GetPosition() - sectMeas1;
       pNetwork->GetAdjacentSectionsInfo ( startSectTID, false, adjSectionList);
        bool first = true;
        for ( size_t i = 0;  i < adjSectionList.size(); i++ )
        {
          DirectedSectionInfo actNextSect = adjSectionList[i];
          if ( actNextSect.GetSectionTid() != startSectTID )
          {
            if ( first )
            {
              first = false;
              SectIDTree *startTree = new SectIDTree ( startSectTID,
                      ( TupleId ) numeric_limits<long>::max(),
                      false,
                      numeric_limits<int>::max() );
              visitedSect = startTree;
            }
            PQEntry *actEntry = new PQEntry ( actNextSect.GetSectionTid(), dist,
                                              actNextSect.GetUpDownFlag(),
                                              startSectTID,
                                              actNextSect.GetMeas1(),
                                              actNextSect.GetMeas2(),
                                              actNextSect.GetRid() );

            prioQ->Insert ( *actEntry, visitedSect );
            delete actEntry;
          }
        }
        adjSectionList.clear();
        dist = sectMeas2 -start->GetPosition();
        pNetwork->GetAdjacentSectionsInfo ( startSectTID, true, adjSectionList);
        for ( size_t i = 0;  i < adjSectionList.size(); i++ )
        {
          DirectedSectionInfo actNextSect = adjSectionList[i];
          if ( actNextSect.GetSectionTid() != startSectTID )
          {
            if ( first )
            {
              first = false;
              SectIDTree *startTree = new SectIDTree ( startSectTID,
                      ( TupleId ) numeric_limits<long>::max(),
                      true,
                      numeric_limits<int>::max() );
              visitedSect = startTree;
            }
            PQEntry *actEntry = new PQEntry ( actNextSect.GetSectionTid(), dist,
                                              actNextSect.GetUpDownFlag(),
                                              startSectTID,
                                              actNextSect.GetMeas1(),
                                              actNextSect.GetMeas2(),
                                              actNextSect.GetRid() );

            prioQ->Insert ( *actEntry, visitedSect );
            delete actEntry;
          }
        }
        adjSectionList.clear();
      }
    }
    /*
    Use priorityQueue to find shortestPath.

    */

    PQEntry *actPQEntry;
    bool found = false;
    vector<PQEntry> candidate;
    while ( !prioQ->IsEmpty() && !found )
    {
      actPQEntry = prioQ->GetAndDeleteMin ( visitedSect );
      sectMeas1 = actPQEntry->meas1;
      sectMeas2 = actPQEntry->meas2;
      dist = actPQEntry->distFromStart + fabs ( sectMeas2 - sectMeas1 );


//////////////////////////////////////
      /*if(junctionpoint){ //end point is a junction point
        for(unsigned int i = 0;i < secjunid.size();i++){
          if(secjunid[i] == actPQEntry->sectID){
            lastSectTID = actPQEntry->sectID;
            Tuple* sect = pNetwork->GetSection(lastSectTID);
            double m1 =
                  ((CcReal*)sect->GetAttribute(SECTION_MEAS1))->GetRealval();
            double m2 =
                  ((CcReal*)sect->GetAttribute(SECTION_MEAS2))->GetRealval();

            if(actPQEntry->upDownFlag){
              GPoint* temp = new GPoint(true,end->GetNetworkId(),
                        end->GetRouteId(),m2,None);
              *end = *temp;
      temp->DeleteIfAllowed();
            }else{
              GPoint* temp = new GPoint(true,end->GetNetworkId(),
                        end->GetRouteId(),m1,None);
              *end = *temp;
      temp->DeleteIfAllowed();
            }
            sect->DeleteIfAllowed();
            break;
          }
        }
      }*/
////////////////////////////////////

      if ( actPQEntry->sectID != lastSectTID )
      {
        /*
        Search further in the network unitl reached last section.
        Get adjacent sections and insert into priority Queue.

        */
        adjSectionList.clear();
        pNetwork->GetAdjacentSectionsInfo ( actPQEntry->sectID,
                                        actPQEntry->upDownFlag,
                                        adjSectionList );
        for ( size_t i = 0; i <adjSectionList.size();i++ )
        {
          DirectedSectionInfo actNextSect = adjSectionList[i];
          if ( actNextSect.GetSectionTid() != actPQEntry->sectID )
          {
            PQEntry *actEntry = new PQEntry ( actNextSect.GetSectionTid(),
                                              dist,
                                              actNextSect.GetUpDownFlag(),
                                              actPQEntry->sectID,
                                              actNextSect.GetMeas1(),
                                              actNextSect.GetMeas2(),
                                              actNextSect.GetRid() );
            prioQ->Insert ( *actEntry, visitedSect );
            delete actEntry;
          }
        }
        delete actPQEntry;
        actPQEntry=0;
      }
      else
      {
        /*
        Shortest Path found.

        Compute last route interval and resulting gline.

        */
//actually not really found
        /*candidate.push_back(*actPQEntry);

        while(!prioQ->IsEmpty()){
          PQEntry* temp = prioQ->GetAndDeleteMin(visitedSect);
        Tuple *act = pNetwork->GetSection(temp->sectID);
        //double m1 =
        //  ((CcReal*) act->GetAttribute(SECTION_MEAS1))->GetRealval();
        //double m2 =
        //  ((CcReal*) act->GetAttribute(SECTION_MEAS2))->GetRealval();

        act->DeleteIfAllowed();
          if(temp->distFromStart >=
            (actPQEntry->distFromStart+fabs(sectMeas2-sectMeas1))){
        temp->DeleteIfAllowed();
            break;
          }
        if(junctionpoint){ //end point is a junction point
        for(unsigned int i = 0;i < secjunid.size();i++){
          if(secjunid[i] == temp->sectID){

            Tuple* sect = pNetwork->GetSection(temp->sectID);
            double m1 =
                  ((CcReal*)sect->GetAttribute(SECTION_MEAS1))->GetRealval();
            double m2 =
                  ((CcReal*)sect->GetAttribute(SECTION_MEAS2))->GetRealval();
            if(temp->distFromStart+m2-m1 >
              actPQEntry->distFromStart+fabs(sectMeas2-sectMeas1)){
              sect->DeleteIfAllowed();
              continue;
            }
            lastSectTID = temp->sectID;
            if(actPQEntry->upDownFlag){
              GPoint* temp = new GPoint(true,end->GetNetworkId(),
                        end->GetRouteId(),m2,None);
              *end = *temp;
        temp->DeleteIfAllowed();
            }else{
              GPoint* temp = new GPoint(true,end->GetNetworkId(),
                        end->GetRouteId(),m1,None);
              *end = *temp;
        temp->DeleteIfAllowed();
            }
            sect->DeleteIfAllowed();
            break;
          }
        }
        }

          if(temp->sectID == lastSectTID){
            candidate.push_back(*temp);
          }else{
            adjSectionList.clear();
            pNetwork->GetAdjacentSections(temp->sectID,
                                      temp->upDownFlag,
                                      adjSectionList);
            Tuple *tempsec = pNetwork->GetSection(temp->sectID);
            double meas1 =
                ((CcReal*) tempsec->GetAttribute(SECTION_MEAS1))->GetRealval();
            double meas2 =
               ((CcReal*) tempsec->GetAttribute(SECTION_MEAS2))->GetRealval();
            dist = temp->distFromStart + fabs(meas2 - meas1);
            for (size_t i = 0; i <adjSectionList.size();i++){
              DirectedSection actNextSect = adjSectionList[i];
            if (actNextSect.GetSectionTid() != temp->sectID) {
                PQEntry *actEntry = new PQEntry(actNextSect.GetSectionTid(),
                                        dist,
                                        actNextSect.GetUpDownFlag(),
                                        temp->sectID);
              prioQ->Insert(*actEntry, visitedSect);
              delete actEntry;
              }
            }
            tempsec->DeleteIfAllowed();
          }
        temp->DeleteIfAllowed();

        }

        //double length;
        delete actPQEntry;
        actSection->DeleteIfAllowed();
        double tempdist = numeric_limits<double>::max();
        actPQEntry = &candidate[0];
        for(unsigned int i = 0; i < candidate.size();i++){
            Tuple *sec = pNetwork->GetSection(candidate[i].sectID);
            double m1 =
              ((CcReal*)sec->GetAttribute(SECTION_MEAS1))->GetRealval();
            double m2 =
              ((CcReal*)sec->GetAttribute(SECTION_MEAS2))->GetRealval();
            if(candidate[i].upDownFlag == true){//UP
              double dist = candidate[i].distFromStart + m2-m1;
              if(dist < tempdist){
                  actPQEntry = &candidate[i];
                  tempdist = dist;
              }
             }else{//DOWN
              double dist = candidate[i].distFromStart + m2-m1;
              if(dist < tempdist){
                  actPQEntry = &candidate[i];
                  tempdist = dist;
                }
            }
            sec->DeleteIfAllowed();
        }

        actSection = pNetwork->GetSection(actPQEntry->sectID);
        sectMeas1 =
        ((CcReal*) actSection->GetAttribute(SECTION_MEAS1))->GetRealval();
        sectMeas2 =
        ((CcReal*) actSection->GetAttribute(SECTION_MEAS2))->GetRealval();*/
///////////////////////////////////////////////

        found = true;
        double startRI, endRI;
        int actRouteId =actPQEntry->rid;
        if ( actPQEntry->upDownFlag == true )
        {
          startRI = sectMeas1;
          endRI = end->GetPosition();
        }
        else
        {
          startRI = sectMeas2;
          endRI = end->GetPosition();
        }
        /*
        Get the sections used for shortest path and write them in right
        order (from start to end gpoint) in the resulting gline. Because
dijkstra gives
        the sections from end to start we first have to put the result sections
on a
        stack to turn in right order.

        */
        RIStack *riStack = new RIStack ( actRouteId, startRI, endRI );
        TupleId lastSectId = actPQEntry->sectID;
        SectIDTree *pElem = visitedSect->Find ( actPQEntry->beforeSectID );
        bool end = false;
        bool upDown;
        //   if (startRI >= endRI) upDown = false;
        if ( startRI > endRI || fabs ( startRI-endRI ) < 0.1 ) upDown = false;
        else upDown = true;
        while ( !end )
        {
          //GetSection
          Tuple *actSection = pNetwork->GetSection ( pElem->sectID );
          actRouteId =
              ( ( CcInt* ) actSection->GetAttribute ( SECTION_RID )
)->GetIntval();
          sectMeas1 =
              ( ( CcReal* ) actSection->GetAttribute ( SECTION_MEAS1 )
)->GetRealval();
          sectMeas2 =
              ( ( CcReal* ) actSection->GetAttribute ( SECTION_MEAS2 )
)->GetRealval();
          upDown = pElem->upDownFlag;
          if ( pElem->sectID != startSectTID )
          {
            if ( upDown )
              riStack->Push ( actRouteId, sectMeas1, sectMeas2, riStack );
            else
              riStack->Push ( actRouteId, sectMeas2, sectMeas1, riStack );
            lastSectId = pElem->sectID;
            pElem = visitedSect->Find ( pElem->beforeSectId );
          }
          else
          {
            end = true;
            pNetwork->GetAdjacentSectionsInfo (startSectTID, true,adjSectionList
);
            size_t i = 0;
            bool stsectfound = false;
            while ( i < adjSectionList.size() && !stsectfound )
            {
              DirectedSectionInfo adjSection = adjSectionList[i];
              if ( adjSection.GetSectionTid() == lastSectId )
              {
                if ( fabs ( start->GetPosition()-sectMeas2 ) > 0.1 )
                {
                  stsectfound = true;
                  riStack->Push ( actRouteId, start->GetPosition(), sectMeas2,
                                  riStack );
                  end = true;
                }
              }
              i++;
            }
            adjSectionList.clear();
            if ( !stsectfound )
            {
              pNetwork->GetAdjacentSectionsInfo ( startSectTID, false,
                                              adjSectionList );
              i = 0;
              while ( i < adjSectionList.size() && !stsectfound )
              {
                DirectedSectionInfo adjSection = adjSectionList[i];
                if ( adjSection.GetSectionTid() == lastSectId )
                {
                  if ( fabs ( start->GetPosition()-sectMeas1 ) > 0.1 )
                  {
                    stsectfound = true;
                    riStack->Push ( actRouteId, start->GetPosition(), sectMeas1,
                                    riStack );
                    end = true;
                  }
                }
                i++;
              }
              adjSectionList.clear();
            }
          }
          if ( actSection )
          {
            actSection->DeleteIfAllowed();
            actSection = 0;
          }
        }
        // Cleanup and return result
        riStack->StackToGLine ( result );
        riStack->RemoveStack();
      }
      if ( actPQEntry )
      {
        delete actPQEntry;
        actPQEntry = 0;
      }
    }
    if ( visitedSect != 0 ) visitedSect->Remove();
    prioQ->Destroy();
    delete prioQ;
  }
  startSection->DeleteIfAllowed();
  endSection->DeleteIfAllowed();
  NetworkManager::CloseNetwork ( pNetwork );
  result->SetSorted ( false );
  result->SetDefined ( true );
  result->TrimToSize();
  start->DeleteIfAllowed();
  end->DeleteIfAllowed();
  return true;
};

/*
Method ~ShortestPathAStarPlus~

Computes the shortest path between start- and endpoint in the network. Using
AStar-algorithm. The path is returned as gline value.

*/

bool GPoint::ShortestPathAStarPlus(GPoint *pToGPoint,GLine *result,Word func){
    bool withfunc = false;
    Word funcResult;
    ArgVectorPointer funargs;

    if (func.addr != 0)
    {
        withfunc = true;
        funargs = qp->Argument(func.addr);
    }

    //reset the result
    result->Clear();
    //******************************************************************
    // Check the Inputs whether they exist and are defined
    //1. generate the startpoint of the search, corresponds with the
    //   pToGPoint, because we want to have the right sequence at the end
    GPoint *end = new GPoint(true, pToGPoint->GetNetworkId(),
        pToGPoint->GetRouteId(), pToGPoint->GetPosition(),pToGPoint->GetSide());

    //2. generate the endpoint of the search,
    //   corresponts with the startpoint of the path
    GPoint *start =
       new GPoint(true, GetNetworkId(), GetRouteId(), GetPosition(), GetSide());

    //3. check whether both Points are not 0
    if ( start == 0 || end == 0 || !start->IsDefined() ||
          !end->IsDefined() )
    {
        sendMessage ( "Both gpoints must exist and be defined." );
        result->SetDefined ( false );
        start->DeleteIfAllowed();
        end->DeleteIfAllowed();
        return false;
    }

    //4. check whether both Points belong to the same network
    if ( start->GetNetworkId() != end->GetNetworkId() )
    {
        sendMessage ( "Both gpoints belong to different networks." );
        result->SetDefined ( false );
        start->DeleteIfAllowed();
        end->DeleteIfAllowed();
        return false;
    }

    //*************************************************
    // Get sections where the path should start or end
    //1. get the netwwork and check whether its not null
    Network* pNetwork =
        NetworkManager::GetNetworkNew ( start->GetNetworkId(),netList );
    if ( pNetwork == 0 )
    {
        sendMessage ( "Network not found." );
        result->SetDefined ( false );
        start->DeleteIfAllowed();
        end->DeleteIfAllowed();
        return false;
    }
    //2. set networkId to the result-gline
    TupleId netId = start->GetNetworkId();
    result->SetNetworkId ( start->GetNetworkId() );
    //3. determin the startsection and check whether its not null
    TupleId startSectTID = pNetwork->GetTupleIdSectionOnRoute(start);
    Tuple* startSection = pNetwork->GetSection(startSectTID);
    if ( startSection == 0 )
    {
        sendMessage ( "Starting GPoint not found in network." );
        NetworkManager::CloseNetwork ( pNetwork );
        result->SetDefined ( false );
        start->DeleteIfAllowed();
        end->DeleteIfAllowed();
        return false;
    }
    //4. determin the endsection and check whether its not null
    TupleId endSectTID = pNetwork->GetTupleIdSectionOnRoute(end);
    Tuple* endSection = pNetwork->GetSection(endSectTID);
    if ( endSection == 0 )
    {
        sendMessage ( "End GPoint not found in network." );
        startSection->DeleteIfAllowed();
        NetworkManager::CloseNetwork ( pNetwork );
        result->SetDefined ( false );
        start->DeleteIfAllowed();
        end->DeleteIfAllowed();
        return false;
    }

    //******************************************************************
    // CAlCULATE THE SHORTEST PATH WITH A*

    //1. check whether both Points on the same Route
    //   if true, then the path can be directly calculated
    if ( startSectTID == endSectTID  ||
         start->GetRouteId() == end->GetRouteId())
    {
        result->AddRouteInterval ( start->GetRouteId(), start->GetPosition(),
                                   end->GetPosition());
    }
    //   else calculate the shortespath with the A* algorithem
    else{
        //1. Generate the openlist,which save all known Points on the searchpath
        //   with the costs and a pointer to its predecessor used a PrioQueue
        PrioQueue *openList = new PrioQueue(0);
        //2. Generate the ClosedList, which save alle Points with the sid
        //   to which the shortest path is known
        SectIDTree *closeList = 0;
        //3. other variables, they are needed
        double dist = 0.0;                  //distance
        double totalcost =0.0;                //totalcost of the section
        double heuristic = 0.0;             //heuristical part of the totalcost
        vector<DirectedSectionInfo> adjSectionList;
        adjSectionList.clear();
        //4. Initialized the OpenList
        //Determin the Side of the section, where the point is located
        //DEBUG cout << start->GetSide()<<endl;
        if(start->GetSide() == 0) // Down-Side of the section
        {
            //4a. Calculate the distance
            double sectMeas1 =
             ((CcReal*)startSection->GetAttribute(SECTION_MEAS1))->GetRealval();
            dist = start->GetPosition() - sectMeas1;
            //4b. get the adjacent sections
           pNetwork->GetAdjacentSectionsInfo(startSectTID,false,adjSectionList);
            //4c. get startsection and put it in the closelist
            SectIDTree *startTree = new SectIDTree ( startSectTID,
                                        ( TupleId ) numeric_limits<long>::max(),
                                        false, numeric_limits<int>::max() );
            closeList = startTree;
            //4d. put all adjacent sections in the openlist
            for ( size_t i = 0;  i < adjSectionList.size(); i++ )
            {
                DirectedSectionInfo actNextSect = adjSectionList[i];
                if (actNextSect.GetSectionTid() != startSectTID)
                {
                    //calculate the totalcost
                    GPoint *sectEP =
                      new GPoint(true,netId,
                                 actNextSect.GetRid(),actNextSect.GetMeas1());
                    heuristic = sectEP->Distance(end);
                    if(withfunc)
                    { //funcargs[0]=cost,funcargs[1]=duration,funcargs[2]=laenge
                      (*funargs)[0] =new CcReal(true,actNextSect.GetCost());
                      (*funargs)[1] =new CcReal(true,actNextSect.GetDuration());
                      (*funargs)[2] =new CcReal(true,actNextSect.GetLength());
                        qp->Request(func.addr, funcResult);
                        double fr= ((CcReal*)(funcResult.addr))->GetRealval();
                        totalcost = fr + heuristic;
                        //DEBUG cout << "ERGEBNIS:" <<totalcost<<endl;
                    }
                    else
                    {
                        totalcost = actNextSect.GetCost() + heuristic;
                    }
                    //DEBUG
                    //cout << "section: "<< actNextSect.GetSectionTid()<<"."
                    //     << actNextSect.GetUpDownFlag() << " heuristic: "
                    //     << heuristic<< "+ kosten: "<< actNextSect.GetCost()
                    //     <<"=totalcost: "<< totalcost <<endl;
                    sectEP->DeleteIfAllowed();
                    PQEntry *actEntry = new PQEntry(
                                            actNextSect.GetSectionTid(),
                                            dist,
                                            actNextSect.GetUpDownFlag(),
                                            startSectTID,
                                            actNextSect.GetMeas1(),
                                            actNextSect.GetMeas2(),
                                            actNextSect.GetRid(),
                                            totalcost);
                    openList->Insert (*actEntry, closeList,true);
                    delete actEntry;
                }
            }
            adjSectionList.clear();
        }//END IF (DownSide)
        else if(start->GetSide() == 1) //Up-Side of the section
        {
            //4a. Calculate the distance
            double sectMeas2 =
            ((CcReal*)startSection->GetAttribute(SECTION_MEAS2))->GetRealval();
            dist = sectMeas2 - start->GetPosition();
            //4b. get the adjacent sections
            pNetwork->GetAdjacentSectionsInfo(startSectTID,true,adjSectionList);
            //4c. get startsection and put it in the closelist
            SectIDTree *startTree = new SectIDTree ( startSectTID,
                                        (TupleId) numeric_limits<long>::max(),
                                        true, numeric_limits<int>::max() );
            closeList = startTree;
            //4d. put all adjacent sections in the openlist
            for ( size_t i = 0;  i < adjSectionList.size(); i++ )
            {
                DirectedSectionInfo actNextSect = adjSectionList[i];
                if ( actNextSect.GetSectionTid() != startSectTID )
                {
                    //calculate the totalcost
                    GPoint *sectEP =
                        new GPoint(true,netId, actNextSect.GetRid(),
                                   actNextSect.GetMeas2());
                    heuristic = sectEP->Distance(end);
                    if(withfunc)
                    { //funcargs[0]=cost,funcargs[1]=duration,funcargs[2]=laenge
                      (*funargs)[0] =new CcReal(true,actNextSect.GetCost());
                      (*funargs)[1] =new CcReal(true,actNextSect.GetDuration());
                      (*funargs)[2] =new CcReal(true,actNextSect.GetLength());
                        qp->Request(func.addr, funcResult);
                        double fr= ((CcReal*)(funcResult.addr))->GetRealval();
                        totalcost = fr + heuristic;
                        //DEBUG cout << "ERGEBNIS:" <<totalcost<<endl;
                    }
                    else
                    {
                        totalcost = actNextSect.GetCost() + heuristic;
                    }
                    sectEP->DeleteIfAllowed();
                    //DEBUG
                    //cout << "section: "<< actNextSect.GetSectionTid()<<"."
                    //     << actNextSect.GetUpDownFlag() << " heuristic: "
                    //     << heuristic << "+ kosten: " << actNextSect.GetCost()
                    //     <<"=totalcost: "<< totalcost <<endl;
                    PQEntry *actEntry = new PQEntry (
                                            actNextSect.GetSectionTid(),
                                            dist,
                                            actNextSect.GetUpDownFlag(),
                                            startSectTID,
                                            actNextSect.GetMeas1(),
                                            actNextSect.GetMeas2(),
                                            actNextSect.GetRid(),
                                            totalcost);
                    openList->Insert ( *actEntry, closeList, true );
                    delete actEntry;
                }
            }
            adjSectionList.clear();

        }//END ELSE IF (UpSide)
        else // NONE, not defined both ways must be calculate
        {
            bool first = true;
            //Entries for the DownSide
            //4a. Calculate the totalcost
            double sectMeas1 =
             ((CcReal*)startSection->GetAttribute(SECTION_MEAS1))->GetRealval();
            dist = start->GetPosition() - sectMeas1;
            //4b. get the adjacent sections
           pNetwork->GetAdjacentSectionsInfo(startSectTID,false,adjSectionList);
            //4c. put all adjacent sections in the openlist
            for ( size_t i = 0;  i < adjSectionList.size(); i++ )
            {
                DirectedSectionInfo actNextSect = adjSectionList[i];
                if ( actNextSect.GetSectionTid() != startSectTID )
                {
                    if ( first )
                    {   //4d. get startsection and put it in the closelist
                        first = false;
                        SectIDTree *startTree = new SectIDTree ( startSectTID,
                                        ( TupleId ) numeric_limits<long>::max(),
                                        false, numeric_limits<int>::max() );
                        closeList = startTree;
                    }
                    //calculate the totalcost
                    GPoint *sectEP =
                      new GPoint(true,netId, actNextSect.GetRid(),
                                 actNextSect.GetMeas1());
                    heuristic = sectEP->Distance(end);
                    if(withfunc)
                    { //funcargs[0]=cost,funcargs[1]=duration,funcargs[2]=laenge
                      (*funargs)[0] =new CcReal(true,actNextSect.GetCost());
                      (*funargs)[1] =new CcReal(true,actNextSect.GetDuration());
                      (*funargs)[2] = new CcReal(true,actNextSect.GetLength());
                        qp->Request(func.addr, funcResult);
                        double fr= ((CcReal*)(funcResult.addr))->GetRealval();
                        totalcost = fr + heuristic;
                        //DEBUG cout << "ERGEBNIS:" <<totalcost<<endl;
                    }
                    else
                    {
                        totalcost = actNextSect.GetCost() + heuristic;
                    }
                    sectEP->DeleteIfAllowed();
                    //DEBUG
                    //cout << "section: "<< actNextSect.GetSectionTid()<<"."
                    //     << actNextSect.GetUpDownFlag() << " heuristic: "
                    //     << heuristic << "+ kosten: " << actNextSect.GetCost()
                    //     <<"=totalcost: "<< totalcost <<endl;
                    PQEntry *actEntry = new PQEntry (
                                            actNextSect.GetSectionTid(),
                                            dist,
                                            actNextSect.GetUpDownFlag(),
                                            startSectTID,
                                            actNextSect.GetMeas1(),
                                            actNextSect.GetMeas2(),
                                            actNextSect.GetRid(),
                                            totalcost );
                    openList->Insert ( *actEntry, closeList, true );
                    delete actEntry;
                }
            }
            adjSectionList.clear();

            //Entries for the UpSide
            //4a. Calculate the distance
            double sectMeas2 =
             ((CcReal*)startSection->GetAttribute(SECTION_MEAS2))->GetRealval();
            dist = sectMeas2 - start->GetPosition();
            //4b. get the adjacent sections
            pNetwork->GetAdjacentSectionsInfo(startSectTID,true,adjSectionList);
            //4c. put all adjacent sections in the openlist
            for ( size_t i = 0;  i < adjSectionList.size(); i++ )
            {
                DirectedSectionInfo actNextSect = adjSectionList[i];
                if ( actNextSect.GetSectionTid() != startSectTID )
                {
                    if ( first )
                    {
                        //4d. get startsection and put it in the closelist
                        first = false;
                        SectIDTree *startTree = new SectIDTree ( startSectTID,
                                        ( TupleId ) numeric_limits<long>::max(),
                                        true, numeric_limits<int>::max() );
                        closeList = startTree;
                    }
                    //calculate the totalcost
                    GPoint *sectEP =
                         new GPoint(true,netId, actNextSect.GetRid(),
                                    actNextSect.GetMeas2());
                    heuristic = sectEP->Distance(end);
                    if(withfunc)
                    { //funcargs[0]=cost,funcargs[1]=duration,funcargs[2]=laenge
                      (*funargs)[0] =new CcReal(true,actNextSect.GetCost());
                      (*funargs)[1] =new CcReal(true,actNextSect.GetDuration());
                      (*funargs)[2] =new CcReal(true,actNextSect.GetLength());
                        qp->Request(func.addr, funcResult);
                        double fr= ((CcReal*)(funcResult.addr))->GetRealval();
                        totalcost = fr + heuristic;
                        //DEBUG cout << "ERGEBNIS:" <<totalcost<<endl;
                    }
                    else
                    {
                        totalcost = actNextSect.GetCost() + heuristic;
                    }
                    sectEP->DeleteIfAllowed();
                    //DEBUG
                    //cout << "section: "<< actNextSect.GetSectionTid()<<"."
                    //     << actNextSect.GetUpDownFlag() << " heuristic: "
                    //     << heuristic << "+ kosten: " << actNextSect.GetCost()
                    //     <<"=totalcost: "<< totalcost <<endl;
                    PQEntry *actEntry = new PQEntry (
                                            actNextSect.GetSectionTid(),
                                            dist,
                                            actNextSect.GetUpDownFlag(),
                                            startSectTID,
                                            actNextSect.GetMeas1(),
                                            actNextSect.GetMeas2(),
                                            actNextSect.GetRid(),
                                            totalcost );
                    openList->Insert ( *actEntry, closeList, true );
                    delete actEntry;
                }
            }
            adjSectionList.clear();
        }//END ELSE (BothSide)

        //5. Use openList to find shortestPath.

        PQEntry *actPQEntry;
        bool found = false;
        vector<PQEntry> candidate;
        while ( !openList->IsEmpty() && !found )
        {
            //5a get the minimum of the openList
            actPQEntry = openList->GetAndDeleteMin ( closeList, true );
            //5b calculate the totalcost of this element
            double sectMeas1 = actPQEntry->meas1;
            double sectMeas2 = actPQEntry->meas2;
            dist = actPQEntry->distFromStart + fabs ( sectMeas2 - sectMeas1 );

            if ( actPQEntry->sectID != endSectTID )
            {
                // Search further in the network unitl reached last section.
                // Get adjacent sections and insert into priority Queue.
                adjSectionList.clear();
                //5c get adjacent section of the actual element
                pNetwork->GetAdjacentSectionsInfo ( actPQEntry->sectID,
                                                    actPQEntry->upDownFlag,
                                                    adjSectionList );
                for ( size_t i = 0; i <adjSectionList.size();i++ )
                {
                    DirectedSectionInfo actNextSect = adjSectionList[i];
                        if ( actNextSect.GetSectionTid() != actPQEntry->sectID )
                        {
                            //calculate the totalcost
                            GPoint *sectEP;
                            if(actPQEntry->upDownFlag)
                                sectEP = new GPoint(true,netId,
                                  actNextSect.GetRid(), actNextSect.GetMeas2());
                            else
                                sectEP = new GPoint(true,netId,
                                  actNextSect.GetRid(), actNextSect.GetMeas1());
                            heuristic = sectEP->Distance(end);
                            if(withfunc)
                           {//fcargs[0]=cost,fcargs[1]=duration,fcargs[2]=laenge
                             (*funargs)[0]=
                                     new CcReal(true,actNextSect.GetCost());
                             (*funargs)[1]=
                                     new CcReal(true,actNextSect.GetDuration());
                             (*funargs)[2] =
                                     new CcReal(true,actNextSect.GetLength());
                                qp->Request(func.addr, funcResult);
                                double fr=
                                    ((CcReal*)(funcResult.addr))->GetRealval();
                                totalcost = fr + heuristic;
                                //DEBUG cout << "ERGEBNIS:" <<totalcost<<endl;
                            }
                            else
                            {
                                totalcost = actNextSect.GetCost() + heuristic;
                            }
                            sectEP->DeleteIfAllowed();
                            //DEBUG
                            //cout << "section: "<< actNextSect.GetSectionTid()
                            //     <<"."<< actNextSect.GetUpDownFlag()
                            //     << " heuristic: "<< heuristic<<"+ vorkosten "
                            //     << actPQEntry->costFromStart << "+ kosten: "
                            //     << actNextSect.GetCost() <<"=totalcost: "
                            //     << totalcost <<endl;
                            PQEntry *actEntry = new PQEntry (
                                                actNextSect.GetSectionTid(),
                                                dist,
                                                actNextSect.GetUpDownFlag(),
                                                actPQEntry->sectID,
                                                actNextSect.GetMeas1(),
                                                actNextSect.GetMeas2(),
                                                actNextSect.GetRid(),
                                                totalcost );
                            openList->Insert ( *actEntry, closeList, true );
                            delete actEntry;
                        }
                }
                delete actPQEntry;
                actPQEntry=0;
            }//END IF the end is not reached
            else //shortespath has been found
            {   //Compute last route interval and resulting gline.
                found = true;
                double startRI, endRI;
                int actRouteId =actPQEntry->rid;
                if ( actPQEntry->upDownFlag == true )
                {
                    startRI = sectMeas1;
                    endRI = end->GetPosition();
                }
                else
                {
                    startRI = sectMeas2;
                    endRI = end->GetPosition();
                }

                //6. Get the sections used for shortest path and write them in
                //   right order (from start to end gpoint) in the resulting
                //   gline. Because Astar gives the sections from end to start
                //   we first have to put the result sections on a stack to turn
                //   in right order.

                RIStack *riStack = new RIStack ( actRouteId, startRI, endRI );
                TupleId endSectId = actPQEntry->sectID;
                SectIDTree *pElem = closeList->Find ( actPQEntry->beforeSectID);
                bool end = false;
                bool upDown;

                if ( startRI > endRI
                     || fabs ( startRI-endRI ) < 0.1 ) upDown = false;
                else upDown = true;
                while ( !end )
                {
                   //GetSection
                   Tuple *actSection = pNetwork->GetSection ( pElem->sectID );
                   actRouteId =
                   ((CcInt*)actSection->GetAttribute(SECTION_RID))->GetIntval();
                   sectMeas1 =
                    ((CcReal*)actSection->GetAttribute(SECTION_MEAS1))
                                        ->GetRealval();
                   sectMeas2 =
                    (( CcReal*)actSection->GetAttribute(SECTION_MEAS2))
                                         ->GetRealval();
                    upDown = pElem->upDownFlag;
                    if ( pElem->sectID != startSectTID )
                    {
                        if ( upDown )
                          riStack->Push(actRouteId,sectMeas1,sectMeas2,riStack);
                        else
                          riStack->Push(actRouteId,sectMeas2,sectMeas1,riStack);
                        endSectId = pElem->sectID;
                        pElem = closeList->Find ( pElem->beforeSectId );
                    }
                    else
                    {
                        end = true;
                        pNetwork->GetAdjacentSectionsInfo(
                                  startSectTID,true, adjSectionList);
                        size_t i = 0;
                        bool stsectfound = false;
                        while ( i < adjSectionList.size() && !stsectfound )
                        {
                            DirectedSectionInfo adjSection = adjSectionList[i];
                            if ( adjSection.GetSectionTid() == endSectId )
                            {
                                if (fabs(start->GetPosition()-sectMeas2 ) > 0.1)
                                {
                                  stsectfound = true;
                                  riStack->Push(
                                   actRouteId,start->GetPosition(),
                                   sectMeas2,riStack );
                                  end = true;
                                }
                            }
                            i++;
                        }
                        adjSectionList.clear();
                        if ( !stsectfound )
                        {
                            pNetwork->
                             GetAdjacentSectionsInfo(
                               startSectTID,false,adjSectionList);
                            i = 0;
                            while ( i < adjSectionList.size() && !stsectfound )
                            {
                             DirectedSectionInfo adjSection = adjSectionList[i];
                              if ( adjSection.GetSectionTid() == endSectId )
                              {
                                if (fabs(start->GetPosition()-sectMeas1) > 0.1)
                                  {
                                    stsectfound = true;
                                    riStack->Push (
                                      actRouteId, start->GetPosition(),
                                      sectMeas1,riStack );
                                    end = true;
                                  }
                              }
                                i++;
                            }
                            adjSectionList.clear();
                        }
                    }
                    if ( actSection )
                    {
                        actSection->DeleteIfAllowed();
                        actSection = 0;
                    }
                }
                // Cleanup and return result
                riStack->StackToGLine ( result );
                riStack->RemoveStack();
            }
            if ( actPQEntry )
            {
                delete actPQEntry;
                actPQEntry = 0;
            }



        }// END ELSE shortesPath has been found

        //clean up
        if ( closeList != 0 ) closeList->Remove();
        openList->Destroy();
        delete openList;

    }//END ELSE Calculate the shortes path with A*

     //clean up
     endSection->DeleteIfAllowed();
     startSection->DeleteIfAllowed();
     NetworkManager::CloseNetwork ( pNetwork );
     result->SetSorted ( false );
     result->SetDefined ( true );
     result->TrimToSize();
     start->DeleteIfAllowed();
     end->DeleteIfAllowed();
     return true;
}


/*
Returns the x,y point represented by gpoint.

*/

void GPoint::ToPoint ( Point *&res )
{
  //Network *pNetwork = NetworkManager::GetNetwork(GetNetworkId());
  Network* pNetwork = NetworkManager::GetNetworkNew ( GetNetworkId(), netList );
  if ( pNetwork != 0 )
  {
    pNetwork->GetPointOnRoute ( this, res );
  }
  else
  {
    res->SetDefined ( false );
  }
  NetworkManager::CloseNetwork ( pNetwork );
};

Point* GPoint::ToPoint() const
{
  Point *res = new Point ( false );
  //Network *pNetwork = NetworkManager::GetNetwork(GetNetworkId());
  Network* pNetwork = NetworkManager::GetNetworkNew ( GetNetworkId(), netList );
  if ( pNetwork != 0 ) pNetwork->GetPointOnRoute ( this, res );
  NetworkManager::CloseNetwork ( pNetwork );
  return res;
};

Point* GPoint::ToPoint ( Network*& pNetwork ) const
{
  Point *res = new Point ( false );
  if ( pNetwork != 0 ) pNetwork->GetPointOnRoute ( this, res );
  return res;
};

/*
Returns the bounding GPoints of the given GLine.

*/

GPoints* GLine::GetBGP ()
{
  GPoints *result = new GPoints ( 0 );
  if ( !IsDefined() || NoOfComponents() == 0 ) return result;
  else
  {
    SectTree *sectionTree = 0;
    RouteInterval ri;
    DbArray<SectTreeEntry> *actSections = new DbArray<SectTreeEntry> ( 0 );
    //Network *pNetwork = NetworkManager::GetNetwork(GetNetworkId());
    Network* pNetwork = NetworkManager::GetNetworkNew ( GetNetworkId(), netList
);
    bool first = true;
    SectTreeEntry nEntry;
    for ( int i = 0; i < Size(); i++ )
    {
      Get ( i,ri );
      pNetwork->GetSectionsOfRouteInterval ( &ri, actSections );
      for ( int j = 0; j < actSections->Size(); j++ )
      {
        actSections->Get ( j, nEntry );
        if ( first )
        {
          first = false;
          sectionTree = new SectTree ( &nEntry );
        }
        else
        {
          sectionTree->Insert ( &nEntry );
        }
      }
      actSections->clean();
    }
    delete actSections;
    sectionTree->WriteResult ( pNetwork, *result, *sectionTree );
    sectionTree->Remove();
    NetworkManager::CloseNetwork ( pNetwork );
    result->TrimToSize();
    return result;
  }
};

/*
Secondo Type Constructor for class ~GPoint~

*/

TypeConstructor gpoint (
    "gpoint",                                   //name
    GPoint::GPointProperty,                     //property function
    GPoint::OutGPoint, GPoint::InGPoint,        //Out and In functions
    0,                   0,                     //SaveToList and
    //RestoreFromList functions
    GPoint::CreateGPoint, GPoint::DeleteGPoint, //object creation/deletion
    OpenAttribute<GPoint>,SaveAttribute<GPoint>,//open and save functions
    GPoint::CloseGPoint, GPoint::CloneGPoint,   //object close, and clone
    GPoint::CastGPoint,                         //cast function
    GPoint::SizeOfGPoint,                       //sizeof function
    GPoint::CheckGPoint );                      //kind checking function

/*
3.4 Class GPoints implemented by Jianqiu Xu

*/

string edistjoinpointlist = "(rel(tuple((pid int)(p point))))";
enum edistjoinpointlistrelation {POINTSID = 0,POINTSOBJECT};

/*
3.4.1 Constructors

*/

GPoints::GPoints():Attribute()
{}
GPoints::GPoints ( int in_iSize ) :Attribute(true),m_xGPoints ( in_iSize )
{}
GPoints::GPoints ( GPoints* in_xOther ) :
    Attribute(in_xOther->IsDefined()),
    m_xGPoints ( 0 )
{
  GPoint pCurrentInterval;
  for ( int i = 0; i < in_xOther->m_xGPoints.Size(); i++ )
  {
    // Get next Interval
    in_xOther->m_xGPoints.Get ( i, pCurrentInterval );
    m_xGPoints.Append ( pCurrentInterval );
  }
  SetDefined(in_xOther->IsDefined());
  TrimToSize();
}
/*
3.4.2 Methods of class ~GPoints~

*/

bool GPoints::IsEmpty() const
{
  return m_xGPoints.Size() == 0;
}
ostream& GPoints::Print ( ostream& os ) const
{
  for ( int i = 0;i < m_xGPoints.Size();i++ )
  {
    GPoint pGP;
    m_xGPoints.Get ( i,pGP );
    os<<"GPoint:"<<i<<" rid "<<pGP.GetRouteId();
    os<<" Position "<<pGP.GetPosition();
    os<<" Side "<< ( int ) pGP.GetSide() <<endl;
  }
  return os;
}
GPoints& GPoints::operator= ( const GPoints& gps )
{
  m_xGPoints.copyFrom(gps.m_xGPoints);
  TrimToSize();
  return *this;
}

int GPoints::NumOfFLOBs() const
{
  return 1;
}

Flob* GPoints::GetFLOB ( const int i )
{
  assert ( i >= 0 && i < NumOfFLOBs() );
  return &m_xGPoints;
}

size_t GPoints::Sizeof() const
{
  return sizeof ( *this );
}

int GPoints::SizeOf()
{
  return sizeof ( GPoints );
}
int GPoints::Size() const
{
  return m_xGPoints.Size();
}
GPoints& GPoints::operator+= ( const GPoint& gp )
{
  m_xGPoints.Append ( gp );
  TrimToSize();
  return *this;
}

GPoints& GPoints::operator-= ( const GPoint& gp )
{
  GPoints *nGPs = new GPoints ( 0 );
  GPoint actGP;
  for ( int i = 0; i < m_xGPoints.Size(); i++ )
  {
    m_xGPoints.Get ( i, actGP );
    if ( gp != actGP ) nGPs->m_xGPoints.Append ( actGP );
  }
  nGPs->TrimToSize();
  return *nGPs;
}

void GPoints::Get ( int i, GPoint& pgp ) const
{
  assert ( i >= 0 && i < m_xGPoints.Size() );
  m_xGPoints.Get ( i,pgp );
}
ListExpr GPoints:: Property()
{
  return ( nl->TwoElemList (
               nl->FourElemList ( nl->StringAtom ( "Signature" ),
                                  nl->StringAtom ( "Example Type List" ),
                                  nl->StringAtom ( "List Rep" ),
                                  nl->StringAtom ( "Example List" ) ),
               nl->FourElemList ( nl->StringAtom ( "-> DATA" ),
                                  nl->StringAtom ( "gpoints" ),
                                  nl->StringAtom (
"(<gpoint1><gpoint2>...<gpointN>))" ),
                                  nl->StringAtom ( "((1 2.0 0)(2 3.0 0))" ) ) )
);

}

ListExpr GPoints::Out ( ListExpr in_xTypeInfo,
                        Word in_xValue )
{
  GPoints *pGPS = ( GPoints* ) in_xValue.addr;

  if ( pGPS->IsEmpty() )
  {
    return nl->TheEmptyList();
  }
  GPoint pgp;
  pGPS->Get ( 0,pgp );
  ListExpr result =
      nl->OneElemList ( GPoint::OutGPoint ( nl->TheEmptyList(),SetWord ( ( void*
) &pgp ) ) );
  ListExpr last = result;
  for ( int i = 1; i < pGPS->Size();i++ )
  {
    pGPS->Get ( i,pgp );
    last =
        nl->Append ( last,GPoint::OutGPoint ( nl->TheEmptyList(),
                                              SetWord ( ( void* ) &pgp ) ) );
  }
  return result;
}
Word GPoints::In ( const ListExpr typeInfo, const ListExpr instance,
                   const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->ListLength ( instance ) == 0 )
  {
    correct = false;
    cmsg.inFunError ( "Empty List" );
    return SetWord ( Address ( 0 ) );
  }
  GPoints* pGPS = new GPoints ( nl->ListLength ( instance ) );
  ListExpr rest = instance;
  while ( !nl->IsEmpty ( rest ) )
  {
    ListExpr first = nl->First ( rest );
    rest = nl->Rest ( rest );
    if ( nl->ListLength ( first ) != 4 )
    {
      correct = false;
      cmsg.inFunError ( "GPoint incorrect.Expected list of 4 Elements." );
      return SetWord ( Address ( 0 ) );
    }
    GPoint* pgp =
        ( GPoint* ) GPoint::InGPoint (
nl->TheEmptyList(),first,0,errorInfo,correct ).addr;
    if ( correct )
    {
      ( *pGPS ) += ( *pgp );
      pgp->DeleteIfAllowed();
    }
    else
    {
      pgp->DeleteIfAllowed();
      return SetWord ( Address ( 0 ) );
    }
  }
  correct = true;
  pGPS->SetDefined ( true );
  pGPS->TrimToSize();
  return SetWord ( pGPS );
}
bool GPoints::OpenGPoints ( SmiRecord& valueRecord,size_t& offset,
                            const ListExpr typeInfo, Word& value )
{
  GPoints* pGPS = ( GPoints* ) Attribute::Open ( valueRecord,offset,typeInfo );
  value = SetWord ( pGPS );
  pGPS->TrimToSize();
  return true;
}
bool GPoints::SaveGPoints ( SmiRecord& valueRecord,size_t& offset,
                            const ListExpr typeInfo, Word& value )
{
  GPoints* pGPS = ( GPoints* ) value.addr;
  Attribute::Save ( valueRecord,offset,typeInfo,pGPS );
  return true;
}
Word GPoints::Create ( const ListExpr typeInfo )
{
  return SetWord ( new GPoints ( 0 ) );
}
void GPoints::Delete ( const ListExpr typeInfo,
                       Word& w )
{
  GPoints *gp = ( GPoints * ) w.addr;
  // if (l->del.refs == 1) { l->m_xRouteIntervals.Destroy();}
  if ( gp->DeleteIfAllowed() == true )
    w.addr = 0;
}
void GPoints::Close ( const ListExpr typeInfo,
                      Word& w )
{
  if ( ( ( GPoints* ) w.addr )->DeleteIfAllowed() == true )
    w.addr = 0;
}
Word GPoints::CloneGPoints ( const ListExpr typeInfo,
                             const Word& w )
{
  return SetWord ( new GPoints ( * ( ( GPoints* ) w.addr ) ) );
}
void* GPoints::Cast ( void* addr )
{
  return new ( addr ) GPoints();
}
bool GPoints::Check ( ListExpr type, ListExpr& errorInfo )
{
  return ( nl->IsEqual ( type, "gpoints" ) );
}

int GPoints::Compare ( const Attribute* ) const
{
  return false;

}
bool GPoints::Adjacent ( const Attribute* ) const
{
  return false;
}

GPoints* GPoints::Clone() const
{
  GPoints* res = new GPoints ( *this );
  return res;
}
size_t GPoints::HashValue() const
{
  size_t xHash = 0;
  // Iterate over all GPoint objects
  for ( int i = 0; i < m_xGPoints.Size(); ++i )
  {
    // Get next Interval
    GPoint pCurrentInterval;
    m_xGPoints.Get ( i, pCurrentInterval );

    // Add something for each entry
    int iNetworkId = pCurrentInterval.GetNetworkId();
    int iRouteId = pCurrentInterval.GetRouteId();
    double iPosition = pCurrentInterval.GetPosition();
    int iSide = ( int ) pCurrentInterval.GetSide();
    xHash += iNetworkId + iRouteId + ( size_t ) iPosition + iSide;
  }
  return xHash;
}
void GPoints::CopyFrom ( const Attribute* right )
{
  *this = * ( ( const GPoints * ) right );
}

TypeConstructor gpoints ( "gpoints",
                          GPoints::Property,
                          GPoints::Out, GPoints::In,
                          0, 0,
                          GPoints::Create, GPoints::Delete,
                          GPoints::OpenGPoints, GPoints::SaveGPoints,
                          GPoints::Close, GPoints::CloneGPoints,
                          GPoints::Cast, GPoints::SizeOf,
                          GPoints::Check );



/*
4 Secondo Operators

4.1 Operator ~netdistance~

Returns the network distance between two ~Gpoints~ or two ~GLines~. Using
Dijkstras Algorithm for computation of the shortest paths.

*/
ListExpr OpNetNetdistanceTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second ( args );
  //ListExpr param3 = nl->Third(args);

  if ( ( nl->IsAtom ( param1 ) && nl->AtomType ( param1 ) == SymbolType &&
          nl->IsAtom ( param2 ) && nl->AtomType ( param2 ) == SymbolType &&
          ( ( nl->SymbolValue ( param1 ) == "gpoint" &&
              nl->SymbolValue ( param2 ) == "gpoint" ) ||
            ( nl->SymbolValue ( param1 ) == "gline" &&
              nl->SymbolValue ( param2 ) == "gline" ) ) ) )
  {
    return nl->SymbolAtom ( CcReal::BasicType() );
  }
  return nl->SymbolAtom ( Symbol::TYPEERROR() );
}

int OpNetNetdistance_gpgp ( Word* args, Word& result, int message,
                            Word& local, Supplier in_pSupplier )
{
  GPoint* pFromGPoint = ( GPoint* ) args[0].addr;
  GPoint* pToGPoint = ( GPoint* ) args[1].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcReal* pResult = ( CcReal* ) result.addr;
  if ( ! ( pFromGPoint->IsDefined() ) || ! ( pToGPoint->IsDefined() ) )
  {
    cmsg.inFunError ( "Both gpoint must be defined!" );
    return 0;
  };
  double dist =  pFromGPoint->Netdistance ( pToGPoint );
  if ( dist != numeric_limits<double>::max() ) pResult->Set ( true,dist );
  else pResult->Set ( false, dist );
  return 1;
};

int OpNetNetdistance_glgl ( Word* args, Word& result, int message,
                            Word& local, Supplier in_pSupplier )
{
  GLine* pGLine1 = ( GLine* ) args[0].addr;
  GLine* pGLine2 = ( GLine* ) args[1].addr;
  CcReal* pResult = ( CcReal* ) qp->ResultStorage ( in_pSupplier ).addr;
  result = SetWord ( pResult );
  if ( ! ( pGLine1->IsDefined() ) || ! ( pGLine2->IsDefined() ) )
  {
    cmsg.inFunError ( "Both gpoint must be defined!" );
    return 0;
  };
  double dist = pGLine1->Netdistance ( pGLine2 );
  if ( dist != numeric_limits<double>::max() ) pResult->Set ( true, dist );
  else pResult->Set ( false, dist );
  return 1;
};

ValueMapping OpNetNetdistancemap[] =
{
  OpNetNetdistance_gpgp,
  OpNetNetdistance_glgl
};

int OpNetNetdistanceselect ( ListExpr args )
{
  ListExpr arg1 = nl->First ( args );
  ListExpr arg2 = nl->Second ( args );
  if ( nl->SymbolValue ( arg1 ) == "gpoint" &&
          nl->SymbolValue ( arg2 ) == "gpoint" )
    return 0;
  if ( nl->SymbolValue ( arg1 ) == "gline" &&
          nl->SymbolValue ( arg2 ) == "gline" )
    return 1;
  return -1; // This point should never be reached
};

const string OpNetNetdistanceSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>A x A x network-> real with A = gpoint or gline" "</text--->"
    "<text>netdistance(GPOINT1,GPOINT2, NETWORK)</text--->"
  "<text>Calculates the network distance of two gpoints resp. glines.</text--->"
    "<text>query netdistance(gp1,gp2, B_NETWORK)</text--->"
     ") )";

Operator networknetdistance (
    "netdistance",               // name
    OpNetNetdistanceSpec,          // specification
    2,
    OpNetNetdistancemap,  // value mapping
    OpNetNetdistanceselect,        // selection function
    OpNetNetdistanceTypeMap        // type mapping
);

/*
4.2 Operator ~gpoint2rect~

Returns a rectangle degenerated to a point with coordinates rid, rid, pos, pos.

*/

ListExpr OpGPoint2RectTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 1 )
  {
    sendMessage ( "Expects a list of length 1." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xsource = nl->First ( in_xArgs );

  if ( ( !nl->IsAtom ( xsource ) ) ||
          !nl->IsEqual ( xsource, "gpoint" ) )
  {
    sendMessage ( "Element must be of type gpoint." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return ( nl->SymbolAtom ( Rectangle<2>::BasicType() ) );
}

int OpGPoint2RectValueMapping ( Word* args,
                                Word& result,
                                int message,
                                Word& local,
                                Supplier in_xSupplier )
{
  result = qp->ResultStorage ( in_xSupplier );
  Rectangle<2>* box = static_cast<Rectangle<2>* > ( result.addr );
  GPoint* arg = static_cast<GPoint*> ( args[0].addr );
  if ( !arg->IsDefined() )
  {
    box->SetDefined ( false );
  }
  else
  {
    ( *box ) = arg->NetBoundingBox();
  }
  return 0;
} //end ValueMapping

const string OpGPoint2RectSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>gpoint -> rect" "</text--->"
    "<text>gpoint2rect(GPOINT)</text--->"
    "<text>Returns a rectangle representing the gpoint.</text--->"
    "<text> gpoint2rect (gpoint) </text--->"
    ") )";

Operator networkgpoint2rect (
    "gpoint2rect",               // name
    OpGPoint2RectSpec,          // specification
    OpGPoint2RectValueMapping,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpGPoint2RectTypeMap        // type mapping
);

/*
4.3 Operator ~inside~

Returns true if ths ~GPoint~ is inside the ~GLine~ false elsewhere.

*/
ListExpr OpInsideTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr gpoint = nl->First ( args );
  ListExpr gline = nl->Second ( args );
  if ( !nl->IsAtom ( gpoint ) || nl->AtomType ( gpoint ) != SymbolType ||
          nl->SymbolValue ( gpoint ) != "gpoint" || !nl->IsAtom ( gline ) ||
          nl->AtomType ( gline ) != SymbolType || nl->SymbolValue ( gline ) !=
"gline" )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return nl->SymbolAtom ( CcBool::BasicType() );
}

int OpInsideValueMap ( Word* args, Word& result, int message,
                       Word& local, Supplier in_pSupplier )
{
  GPoint* pGPoint = ( GPoint* ) args[0].addr;
  GLine* pGLine = ( GLine* ) args[1].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcBool* pResult = ( CcBool* ) result.addr;
  if ( pGPoint->GetNetworkId() != pGLine->GetNetworkId() ||
          !pGLine->IsDefined() || !pGPoint->IsDefined() )
  {
    pResult->Set ( false, false );
  }
  pResult->Set ( true, pGPoint->Inside ( pGLine ) );
  return 0;
}

const string OpInsideSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>gpoint x gline -> bool" "</text--->"
    "<text> _ inside _</text--->"
    "<text>Returns true if gpoint is inside gline false elsewhere.</text--->"
    "<text>GPOINT inside GLINE</text--->"
    ") )";

Operator networkinside (
    "inside",               // name
    OpInsideSpec,          // specification
    OpInsideValueMap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpInsideTypeMap        // type mapping
);

/*
4.4 Operator ~length~

Returns the length of a ~GLine~.

*/
ListExpr OpLengthTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr gline = nl->First ( args );
  if ( !nl->IsAtom ( gline ) || nl->AtomType ( gline ) != SymbolType ||
          nl->SymbolValue ( gline ) != "gline" )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return nl->SymbolAtom ( CcReal::BasicType() );
}

int OpLengthValueMap ( Word* args, Word& result, int message,
                       Word& local, Supplier in_pSupplier )
{
  GLine* pGline = ( GLine* ) args[0].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcReal* pResult = ( CcReal* ) result.addr;
  if ( ! ( pGline->IsDefined() ) )
  {
    cmsg.inFunError ( "gline is not defined!" );
    return 0;
  };
  pResult-> Set ( true, pGline->GetLength() );
  return 1;
}

const string OpLengthSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>gline -> real" "</text--->"
    "<text>length(_)</text--->"
    "<text>Calculates the length of the gline.</text--->"
    "<text>query length(gline)</text--->"
    ") )";

Operator networklength (
    "length",               // name
    OpLengthSpec,          // specification
    OpLengthValueMap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpLengthTypeMap        // type mapping
);

/*
4.5 Operator ~line2gline~

Translates a spatial ~line~ value into a network ~GLine~ value if possible.

*/

ListExpr OpLine2GLineTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 2 )
  {
    sendMessage ( "Expects a list of length 2." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xNetworkIdDesc = nl->First ( in_xArgs );
  ListExpr xLineDesc = nl->Second ( in_xArgs );

  if ( ( !nl->IsAtom ( xNetworkIdDesc ) ) ||
          !nl->IsEqual ( xNetworkIdDesc, "network" ) )
  {
    sendMessage ( "First element must be of type network." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  if ( ( !nl->IsAtom ( xLineDesc ) ) ||
          nl->AtomType ( xLineDesc ) != SymbolType ||
          nl->SymbolValue ( xLineDesc ) != Line::BasicType() )
  {
    sendMessage ( "Second element must be of type sline." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  return nl->SymbolAtom ( "gline" );
}

int OpLine2GLineValueMapping ( Word* args,
                               Word& result,
                               int message,
                               Word& local,
                               Supplier in_xSupplier )
{
  //Initialize return value
  GLine* pGLine = ( GLine* ) qp->ResultStorage ( in_xSupplier ).addr;
  result = SetWord ( pGLine );
  // Get and check input values.
  Network* pNetwork = ( Network* ) args[0].addr;
  GLine *res = new GLine ( 0 );
  if ( pNetwork == 0 || !pNetwork->IsDefined() )
  {
    string strMessage = "Network is not defined.";
    cerr << strMessage << endl;
    sendMessage ( strMessage );
    res->SetDefined ( false );
    pGLine->CopyFrom ( res );
    res->DeleteIfAllowed();
    return 0;
  }
  res->SetNetworkId ( pNetwork->GetId() );
  Line* pLine = ( Line* ) args[1].addr;
  if ( pLine == NULL || !pLine->IsDefined() )
  {
    string strMessage = "line does not exist.";
    cerr << strMessage << endl;
    sendMessage ( strMessage );
    res->SetDefined ( false );
    pGLine->CopyFrom ( res );
    res->DeleteIfAllowed();
    return 0;
  }
  if ( pLine->Size() <= 0 )
  {
    string strMessage = "line is empty";
    cerr <<strMessage << endl;
    sendMessage ( strMessage );
    res->SetDefined ( true );
    pGLine->CopyFrom ( res );
    res->DeleteIfAllowed();
    return 0;
  }

  HalfSegment hs;
  pLine->Get ( 0,hs );
  RouteInterval *ri =
      pNetwork->Find ( hs.GetLeftPoint(), hs.GetRightPoint() );
  if ( ri!= 0 )
  {
    RITree *riTree = new RITree ( ri->GetRouteId(), ri->GetStartPos(),
                                  ri->GetEndPos() );
    delete ri;
    for ( int i = 1; i < pLine->Size();i++ )
    {
      pLine->Get ( i,hs );
      ri = pNetwork->Find ( hs.GetLeftPoint(), hs.GetRightPoint() );
      if ( ri!=0 )
      {
        riTree->Insert ( ri->GetRouteId(), ri->GetStartPos(), ri->GetEndPos() );
        delete ri;
      }
    }
    riTree->TreeToGLine ( res );
    riTree->RemoveTree();
    res->SetDefined ( true );
    res->SetSorted ( true );
    pGLine->CopyFrom ( res );
    res->DeleteIfAllowed();
  }
  else
  {
    res->SetDefined ( false );
    pGLine->CopyFrom ( res );
    res->DeleteIfAllowed();
  }
  return 0;
} //end ValueMapping

const string OpLine2GLineSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x line -> gline" "</text--->"
    "<text>sline2gline(_,_)</text--->"
    "<text>Translates a line to a gline value.</text--->"
    "<text>line2gline(B_NETWORK, sline)</text--->"
    ") )";

Operator sline2gline (
    "line2gline",               // name
    OpLine2GLineSpec,          // specification
    OpLine2GLineValueMapping,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpLine2GLineTypeMap        // type mapping
);


/*
4.6 Operator ~=~

Returns true if two ~GPoint~s respectively ~GLine~s are identical false
elsewhere.

*/

ListExpr OpNetEqualTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr first = nl->First ( args );
  ListExpr second = nl->Second ( args );
  if ( ( nl->IsAtom ( first ) && nl->AtomType ( first ) == SymbolType &&
          nl->IsAtom ( second ) && nl->AtomType ( second ) == SymbolType ) &&
          ( ( nl->SymbolValue ( first ) == "gpoint" &&
              nl->SymbolValue ( second ) == "gpoint" ) ||
            ( nl->SymbolValue ( first ) == "gline" &&
              nl->SymbolValue ( second ) == "gline" ) ) )
  {
    return nl->SymbolAtom ( CcBool::BasicType() );
  }
  return nl->SymbolAtom ( Symbol::TYPEERROR() );
}

int OpNetEqual_gpgp ( Word* args, Word& result, int message,
                      Word& local, Supplier in_pSupplier )
{
  GPoint* p1 = ( GPoint* ) args[0].addr;
  GPoint* p2 = ( GPoint* ) args[1].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcBool* pResult = ( CcBool* ) result.addr;
  if ( ! ( p1->IsDefined() ) || !p2->IsDefined() )
  {
    cmsg.inFunError ( "Both gpoints must be defined!" );
    pResult->Set ( false, false );
    return 0;
  };
  pResult-> Set ( true, *p1 == *p2 );
  return 1;
}


int OpNetEqual_glgl ( Word* args, Word& result, int message,
                      Word& local, Supplier in_pSupplier )
{
  GLine* l1 = ( GLine* ) args[0].addr;
  GLine* l2 = ( GLine* ) args[1].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcBool* pResult = ( CcBool* ) result.addr;
  if ( ! ( l1->IsDefined() ) || !l2->IsDefined() )
  {
    cmsg.inFunError ( "Both glines must be defined!" );
    pResult->Set ( false, false );
    return 0;
  };
  pResult-> Set ( true, *l1 == *l2 );
  return 1;
}

ValueMapping OpNetEqualmap[] =
{
  OpNetEqual_gpgp,
  OpNetEqual_glgl
};

int OpNetEqualselect ( ListExpr args )
{
  ListExpr arg1 = nl->First ( args );
  ListExpr arg2 = nl->Second ( args );
  if ( nl->SymbolValue ( arg1 ) == "gpoint" &&
          nl->SymbolValue ( arg2 ) == "gpoint" )
    return 0;
  if ( nl->SymbolValue ( arg1 ) == "gline" &&
          nl->SymbolValue ( arg2 ) == "gline" )
    return 1;
  return -1; // This point should never be reached
};

const string OpNetEqualSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>A x A -> bool for GPoint and GLine" "</text--->"
    "<text> _ = _</text--->"
    "<text>Returns if two gpoints are equal.</text--->"
    "<text>query gpoint1 = gpoint2</text--->"
    ") )";

Operator netgpequal (
    "=",               // name
    OpNetEqualSpec,          // specification
    2,
    OpNetEqualmap,  // value mapping
    OpNetEqualselect,        // selection function
    OpNetEqualTypeMap        // type mapping
);


/*
4.7 Operator ~intersects~

Returns true if two ~Gline~ intersect false elsewhere.

*/

ListExpr OpNetIntersectsTypeMap ( ListExpr in_xArgs )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength ( in_xArgs ) == 2 )
  {
    arg1 = nl->First ( in_xArgs );
    arg2 = nl->Second ( in_xArgs );
    if ( nl->IsAtom ( arg1 ) && nl->AtomType ( arg1 ) == SymbolType &&
            nl->SymbolValue ( arg1 ) == "gline" && nl->IsAtom ( arg2 ) &&
            nl->AtomType ( arg2 ) == SymbolType &&
            nl->SymbolValue ( arg2 ) == "gline" )
    {
      return ( nl->SymbolAtom ( CcBool::BasicType() ) );
    }
  }
  return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
}

int OpNetIntersectsValueMapping ( Word* args,
                                  Word& result,
                                  int message,
                                  Word& local,
                                  Supplier in_xSupplier )
{
  // Get (empty) return value
  CcBool* pResult = ( CcBool* ) qp->ResultStorage ( in_xSupplier ).addr;
  result = SetWord ( pResult );
  pResult->Set ( false, false );
  // Get input values
  GLine* pGLine1 = ( GLine* ) args[0].addr;
  if ( pGLine1 == NULL || !pGLine1->IsDefined() )
  {
    cerr << "First gline does not exist." << endl;
    pResult->Set ( false, false );
    return 0;
  }
  GLine* pGLine2 = ( GLine* ) args[1].addr;
  if ( pGLine2 == NULL || !pGLine2->IsDefined() )
  {
    cerr << "Second gline does not exist." << endl;
    pResult->Set ( false, false );
    return 0;
  }
//  const RouteInterval *pRi1, *pRi2;
  if ( pGLine1->GetNetworkId() != pGLine2->GetNetworkId() )
  {
    cerr << "glines belong to different networks." << endl;
    pResult->Set ( true, false );
    return 0;
  }
  pResult->Set ( true, pGLine1->Intersects ( pGLine2 ) );
  return 0;
}

const string OpNetIntersectsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>gline x gline -> bool" "</text--->"
    "<text>intersects(gline, gline)</text--->"
 "<text>Returns true if the both glines intersects false elsewhere.</text--->"
    "<text>intersects(gline, gline)</text--->"
    ") )";

Operator networkintersects (
    "intersects",               // name
    OpNetIntersectsSpec,          // specification
    OpNetIntersectsValueMapping,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpNetIntersectsTypeMap        // type mapping
);


/*
4.8 Operator ~junctions~

Returns the junctions relation of the network.

*/
ListExpr OpNetworkJunctionsTypeMap ( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength ( args ) == 1 )
  {
    arg1 = nl->First ( args );
    if ( nl->IsAtom ( arg1 ) &&
            nl->AtomType ( arg1 ) == SymbolType &&
            nl->SymbolValue ( arg1 ) == "network" )
    {
      ListExpr xType;
      nl->ReadFromString ( Network::junctionsInternalTypeInfo, xType );
      return xType;
    }
  }
  return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
}

int OpNetworkJunctionsValueMapping ( Word* args, Word& result, int message,
                                     Word& local, Supplier s )
{
  Network *network = ( Network* ) args[0].addr;
  result = SetWord ( network->GetJunctions() );

  Relation *resultSt = ( Relation* ) qp->ResultStorage ( s ).addr;
  resultSt->Close();
  qp->ChangeResultStorage ( s, result );

  return 0;
}

const string OpNetworkJunctionsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network -> rel" "</text--->"
    "<text>junctions(_)</text--->"
    "<text>Return the junctions of a network.</text--->"
    "<text>query junctions(network)</text--->"
    ") )";

Operator networkjunctions (
    "junctions",                // name
    OpNetworkJunctionsSpec,          // specification
    OpNetworkJunctionsValueMapping,  // value mapping
    Operator::SimpleSelect,            // trivial selection function
    OpNetworkJunctionsTypeMap        // type mapping
);


/*
4.9 Operator ~routes~

Returns the routes relation of the network.

*/
ListExpr OpNetworkRoutesTypeMap ( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength ( args ) == 1 )
  {
    arg1 = nl->First ( args );
    if ( nl->IsAtom ( arg1 ) &&
            nl->AtomType ( arg1 ) == SymbolType &&
            nl->SymbolValue ( arg1 ) == "network" )
    {
      ListExpr xType;
      nl->ReadFromString ( Network::routesInternalTypeInfo, xType );
      return xType;
    }
  }
  return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
}

int OpNetworkRoutesValueMapping ( Word* args, Word& result, int message,
                                  Word& local, Supplier s )
{
  Network *network = ( Network* ) args[0].addr;
  Relation* pRoute = network->GetRoutes();
  result =  SetWord ( pRoute->Clone() );
  Relation *resultSt = ( Relation* ) qp->ResultStorage ( s ).addr;
  resultSt->Close();
  qp->ChangeResultStorage ( s, result );

  return 0;
}

const string OpNetworkRoutesSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network -> rel" "</text--->"
    "<text>routes(_)</text--->"
    "<text>Return the routes of a network.</text--->"
    "<text>query routes(network)</text--->"
    ") )";

Operator networkroutes (
    "routes",                // name
    OpNetworkRoutesSpec,              // specification
    OpNetworkRoutesValueMapping,      // value mapping
    Operator::SimpleSelect,               // trivial selection function
    OpNetworkRoutesTypeMap            // type mapping
);

/*
4.10 Operator ~sections~

Returns the sections relation of the network.

*/

ListExpr OpNetworkSectionsTypeMap ( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength ( args ) == 1 )
  {
    arg1 = nl->First ( args );
    if ( nl->IsAtom ( arg1 ) &&
            nl->AtomType ( arg1 ) == SymbolType &&
            nl->SymbolValue ( arg1 ) == "network" )
    {
      ListExpr xType;
      nl->ReadFromString ( Network::sectionsInternalTypeInfo, xType );
      return xType;
    }
  }
  return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
}

int OpNetworkSectionsValueMapping ( Word* args, Word& result, int message,
                                    Word& local, Supplier s )
{
  Network *network = ( Network* ) args[0].addr;
  result = SetWord ( network->GetSections() );

  Relation *resultSt = ( Relation* ) qp->ResultStorage ( s ).addr;
  resultSt->Close();
  qp->ChangeResultStorage ( s, result );

  return 0;
}

const string OpNetworkSectionsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network -> rel" "</text--->"
    "<text>sections(_)</text--->"
    "<text>Return the sections of a network.</text--->"
    "<text>query sections(network)</text--->"
    ") )";

Operator networksections (
    "sections",                       // name
    OpNetworkSectionsSpec,          // specification
    OpNetworkSectionsValueMapping,  // value mapping
    Operator::SimpleSelect,           // trivial selection function
    OpNetworkSectionsTypeMap        // type mapping
);


/*
4.11 Operator ~thenetwork~

Creates a network with the given id, from the given routes and junctions
relations.

*/
ListExpr OpNetworkTheNetworkTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 3 )
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );

  ListExpr xIdDesc = nl->First ( in_xArgs );
  ListExpr xRoutesRelDesc = nl->Second ( in_xArgs );
  ListExpr xJunctionsRelDesc = nl->Third ( in_xArgs );

  if ( !nl->IsEqual ( xIdDesc, CcInt::BasicType() ) )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  if ( !IsRelDescription ( xRoutesRelDesc ) ||
          !IsRelDescription ( xJunctionsRelDesc ) )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xType;
  nl->ReadFromString ( Network::routesTypeInfo, xType );
  if ( !CompareSchemas ( xRoutesRelDesc, xType ) )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  nl->ReadFromString ( Network::junctionsTypeInfo, xType );
  if ( !CompareSchemas ( xJunctionsRelDesc, xType ) )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  return nl->SymbolAtom ( "network" );
}

int OpNetworkTheNetworkValueMapping ( Word* args, Word& result,
                                      int message, Word& local, Supplier s )
{
  Network* pNetwork = ( Network* ) qp->ResultStorage ( s ).addr;
  CcInt* pId = ( CcInt* ) args[0].addr;
  int iId = pId->GetIntval();
  map<int,string>::iterator it = netList->find ( iId );
  if ( it != netList->end() )
  {
    cerr << "NetworkId used before" << iId << endl;
    while ( it != netList->end() )
    {
      it = netList->find ( ++iId );
    }
    cerr << "NetworkId changed to: " << iId << endl;
  }
  netList->insert ( pair<int,string> ( iId, "" ) );
  Relation* pRoutes = ( Relation* ) args[1].addr;
  Relation* pJunctions = ( Relation* ) args[2].addr;
  pNetwork->Load ( iId,
                   pRoutes,
                   pJunctions );
  result = SetWord ( pNetwork );

  return 0;
}

const string OpNetworkTheNetworkSpec  =
    "((\"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\") "
    "(<text>int x rel x rel -> network" "</text--->"
    "<text>thenetwork(_, _, _)</text--->"
    "<text>Creates network with id or next free id.</text--->"
    "<text>let n = thenetwork(int, routes, junctions)</text--->"
    "))";

Operator networkthenetwork (
    "thenetwork",                // name
    OpNetworkTheNetworkSpec,              // specification
    OpNetworkTheNetworkValueMapping,      // value mapping
    Operator::SimpleSelect,               // trivial selection function
    OpNetworkTheNetworkTypeMap            // type mapping
);

/*
4.12 Operator ~no\_components~

Returns the number of ~RouteIntervals~ of the ~GLine~.

*/

ListExpr OpNoComponentsTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr gline = nl->First ( args );
  if ( !nl->IsAtom ( gline ) || nl->AtomType ( gline ) != SymbolType ||
          nl->SymbolValue ( gline ) != "gline" )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return nl->SymbolAtom ( CcInt::BasicType() );
}

int OpNoComponentsValueMapping ( Word* args, Word& result, int message,
                                 Word& local, Supplier in_pSupplier )
{
  GLine* pGline = ( GLine* ) args[0].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcInt* pResult = ( CcInt* ) result.addr;
  if ( ! ( pGline->IsDefined() ) )
  {
    pResult->Set ( false, 0 );
    return 0;
  };
  pResult-> Set ( true, pGline->NoOfComponents() );
  return 1;
}

const string OpNoComponentsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>gline -> int" "</text--->"
    "<text>no_components(_)</text--->"
    "<text>Returns the number of route intervals.</text--->"
    "<text>query no_components(gline)</text--->"
    ") )";

Operator networknocomponents (
    "no_components",               // name
    OpNoComponentsSpec,          // specification
    OpNoComponentsValueMapping,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpNoComponentsTypeMap        // type mapping
);


/*
4.13 Operator ~point2gpoint~

Translates a spatial ~Point~ value into a network ~GPoint~ value if possible.

*/
ListExpr OpPoint2GPointTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 2 )
  {
    sendMessage ( "Expects a list of length 2." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xNetworkIdDesc = nl->First ( in_xArgs );
  ListExpr xMPointDesc = nl->Second ( in_xArgs );

  if ( ( !nl->IsAtom ( xNetworkIdDesc ) ) ||
          !nl->IsEqual ( xNetworkIdDesc, "network" ) )
  {
    sendMessage ( "First element must be of type network." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  if ( ( !nl->IsAtom ( xMPointDesc ) ) ||
          nl->AtomType ( xMPointDesc ) != SymbolType ||
          nl->SymbolValue ( xMPointDesc ) != Point::BasicType() )
  {
    sendMessage ( "Second element must be of type point." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  return nl->SymbolAtom ( "gpoint" );
}

int OpPoint2GPointValueMapping ( Word* args,
                                 Word& result,
                                 int message,
                                 Word& local,
                                 Supplier in_xSupplier )
{
  GPoint* pGPoint = ( GPoint* ) qp->ResultStorage ( in_xSupplier ).addr;
  result = SetWord ( pGPoint );
  Network* pNetwork = ( Network* ) args[0].addr;
  if ( pNetwork == 0 || !pNetwork->IsDefined() )
  {
    string strMessage = "Network is not defined.";
    cerr << strMessage << endl;
    sendMessage ( strMessage );
    pGPoint->SetDefined ( false );
    return 0;
  }

  Point* pPoint = ( Point* ) args[1].addr;
  if ( pPoint == NULL || !pPoint->IsDefined() )
  {
    string strMessage = "Point does not exist.";
    cerr << strMessage << endl;
    sendMessage ( strMessage );
    pGPoint->SetDefined ( false );
    return 0;
  }
  GPoint *res = pNetwork->GetNetworkPosOfPoint ( *pPoint );
  pGPoint->CopyFrom ( res );
  res->DeleteIfAllowed();
  /*GPoint *res = pNetwork->GetNetworkPosOfPoint(*pPoint);
  qp->ChangeResultStorage(in_xSupplier, res);*/
  return 0;
} //end ValueMapping

const string OpPoint2GPointSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x point -> gpoint" "</text--->"
    "<text>point2gpoint(networkobject, point)</text--->"
    "<text>Translates a point to a gpoint.</text--->"
    "<text>point2gpoint(b_network, point)</text--->"
    ") )";

Operator point2gpoint (
    "point2gpoint",               // name
    OpPoint2GPointSpec,          // specification
    OpPoint2GPointValueMapping,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpPoint2GPointTypeMap        // type mapping
);

/*
4.14 Operator ~gpoint2point~

Translates a ~gpoint~ into a spatial ~Point~ value.

*/

ListExpr OpGPoint2PointTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 1 )
  {
    sendMessage ( "Expects a list of length 1." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xMPointDesc = nl->First ( in_xArgs );

  if ( ( !nl->IsAtom ( xMPointDesc ) ) ||
          nl->AtomType ( xMPointDesc ) != SymbolType ||
          nl->SymbolValue ( xMPointDesc ) != "gpoint" )
  {
    sendMessage ( "Element must be of type gpoint." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  return nl->SymbolAtom ( Point::BasicType() );
}

int OpGPoint2PointValueMapping ( Word* args,
                                 Word& result,
                                 int message,
                                 Word& local,
                                 Supplier in_xSupplier )
{
  Point* pPoint = ( Point* ) qp->ResultStorage ( in_xSupplier ).addr;
  result = SetWord ( pPoint );
  GPoint* pGPoint = ( GPoint* ) args[0].addr;
  if ( pGPoint == NULL || !pGPoint->IsDefined() )
  {
    string strMessage = "Point does not exist.";
    cerr << strMessage << endl;
    sendMessage ( strMessage );
    pPoint->SetDefined ( false );
    return 0;
  }
  pGPoint->ToPoint ( pPoint );
  return 0;
} //end ValueMapping

const string OpGPoint2PointSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>gpoint -> point" "</text--->"
    "<text>gpoint2point(gpoint)</text--->"
    "<text>Translates a gpoint to a point.</text--->"
    "<text>gpoint2point(gpoint)</text--->"
    ") )";

Operator gpoint2point (
    "gpoint2point",               // name
    OpGPoint2PointSpec,          // specification
    OpGPoint2PointValueMapping,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpGPoint2PointTypeMap        // type mapping
);


/*
4.15 Operator ~polygpoint~

Returns a  stream of all ~GPoint~ values which are at the same network position
as the given ~GPoint~. Including the given  ~GPoint~.

*/

ListExpr OpPolyGPointTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 2 )
  {
    sendMessage ( "Expects a list of length 1." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xsource = nl->First ( in_xArgs );
  ListExpr xnetwork = nl->Second ( in_xArgs );

  if ( ( !nl->IsAtom ( xsource ) ) ||
          !nl->IsEqual ( xsource, "gpoint" ) )
  {
    sendMessage ( "First Element must be of type gpoint." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  if ( ( !nl->IsAtom ( xnetwork ) ) ||
          !nl->IsEqual ( xnetwork, "network" ) )
  {
    sendMessage ( "Second Element must be of type network." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  return nl->TwoElemList ( nl->SymbolAtom ( Symbol::STREAM() ),
                           nl->SymbolAtom ( "gpoint" ) );
}

int OpPolyGPointValueMapping ( Word* args,
                               Word& result,
                               int message,
                               Word& local,
                               Supplier in_xSupplier )
{
  GPointList* localinfo;
  GPoint res;
  result = qp->ResultStorage ( in_xSupplier );
  switch ( message )
  {
    case OPEN:
      local = SetWord ( new GPointList ( ( GPoint* ) args[0].addr,
                                         ( Network* ) args[1].addr ) );
      return 0;
    case REQUEST:
      localinfo = ( GPointList* ) local.addr;
      res = localinfo->NextGPoint();
      if ( !res.IsDefined() )
      {
        return CANCEL;
      }
      else
      {
        result = SetWord ( new GPoint ( res ) );
        return YIELD;
      }
    case CLOSE:
      if ( local.addr != 0 )
      {
        delete ( GPointList* ) local.addr;
        local = SetWord ( Address ( 0 ) );
      }
      return 0;
  }
  return 0; // ignore unknown message
} //end ValueMapping


const string OpPolyGPointSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>gpoint x network -> stream(gpoint)" "</text--->"
    "<text>gpoint polygpoint</text--->"
    "<text>Returns the gpoint and gpoints with the same network position"
    " if the gpoint is a junction.</text--->"
    "<text> polygpoints (gpoint, network) </text--->"
    ") )";

Operator polygpoints (
    "polygpoints",               // name
    OpPolyGPointSpec,          // specification
    OpPolyGPointValueMapping,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpPolyGPointTypeMap        // type mapping
);


/*
4.16 Operator ~routeintervals~

Returns a stream of rectangles representing the ~RouteIntervals~ of the ~GLine~.

*/

ListExpr OpRouteIntervalsTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 1 )
  {
    sendMessage ( "Expects a list of length 1." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xsource = nl->First ( in_xArgs );

  if ( ( !nl->IsAtom ( xsource ) ) ||
          !nl->IsEqual ( xsource, "gline" ) )
  {
    sendMessage ( "First Element must be of type gline." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return nl->TwoElemList ( nl->SymbolAtom ( Symbol::STREAM() ),
                           nl->SymbolAtom ( Rectangle<2>::BasicType() ) );
}

int OpRouteIntervalsValueMapping ( Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier )
{
  RectangleList* localinfo;
  Rectangle<2> res;
  result = qp->ResultStorage ( in_xSupplier );
  switch ( message )
  {
    case OPEN:
      local = SetWord ( new RectangleList ( ( GLine* ) args[0].addr ) );
      return 0;
    case REQUEST:
      localinfo = ( RectangleList* ) local.addr;
      res = localinfo->NextRectangle();
      if ( !res.IsDefined() ) return CANCEL;
      else
      {
        result = SetWord ( new Rectangle<2> ( res ) );
        return YIELD;
      }
    case CLOSE:
      if ( local.addr != 0 )
      {
        delete ( RectangleList* ) local.addr;
        local = SetWord ( Address ( 0 ) );
      }
      return 0;
  }
  return 0; // ignore unknown message
} //end ValueMapping

const string OpRouteIntervalsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>gline -> stream(rect)" "</text--->"
    "<text>routeintervals(GLINE)</text--->"
 "<text>Returns a stream of rectangles representing the route intervals of the"
    " gline.</text--->"
    "<text> routeintervals (gline) </text--->"
    ") )";

Operator networkrouteintervals (
    "routeintervals",               // name
    OpRouteIntervalsSpec,          // specification
    OpRouteIntervalsValueMapping,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpRouteIntervalsTypeMap        // type mapping
);


/*
4.17 Operator ~shortest\_path~

Returns the shortest path in the ~Network~ between two ~GPoint~. Using Dijkstra
Algorithm to compute the shortest path.

*/

ListExpr OpShortestPathTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 2 )
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );

  ListExpr xGPoint1Desc = nl->First ( in_xArgs );
  ListExpr xGPoint2Desc = nl->Second ( in_xArgs );
// ListExpr xGPoint3Desc = nl->Third(in_xArgs);

  if ( ( !nl->IsAtom ( xGPoint1Desc ) ) ||
          nl->AtomType ( xGPoint1Desc ) != SymbolType ||
          nl->SymbolValue ( xGPoint1Desc ) != "gpoint" )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  if ( ( !nl->IsAtom ( xGPoint2Desc ) ) ||
          nl->AtomType ( xGPoint2Desc ) != SymbolType ||
          nl->SymbolValue ( xGPoint2Desc ) != "gpoint" )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  //if((!nl->IsAtom(xGPoint3Desc))|| (!nl->IsEqual(xGPoint3Desc,"network"))) {
  //  return (nl->SymbolAtom(Symbol::TYPEERROR()));
  //}

  return nl->SymbolAtom ( "gline" );
}

int OpShortestPathValueMapping ( Word* args,
                                 Word& result,
                                 int message,
                                 Word& local,
                                 Supplier in_xSupplier )
{

  GPoint *pFromGPoint = ( GPoint* ) args[0].addr;
  GPoint *pToGPoint = ( GPoint* ) args[1].addr;
  GLine* pGLine = ( GLine* ) qp->ResultStorage ( in_xSupplier ).addr;
  result = SetWord ( pGLine );

  pGLine->SetSorted ( false );
  pGLine->SetDefined ( pFromGPoint->ShortestPath ( pToGPoint, pGLine ) );
  return 0;
}

const string OpShortestPathSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>gpoint x gpoint -> gline" "</text--->"
    "<text>shortest_path( _ , _)</text--->"
    "<text>Calculates the shortest path between two gpoints.</text--->"
    "<text>query shortest_path(x, y)</text--->"
    ") )";

Operator shortest_path (
    "shortest_path",               // name
    OpShortestPathSpec,          // specification
    OpShortestPathValueMapping,  // value mapping
    Operator::SimpleSelect,        // trivial selection function
    OpShortestPathTypeMap        // type mapping
);

/*
4.18 Operator ~gline2line~

Returns the ~line~ value of the given ~GLine~.

*/

ListExpr OpGLine2LineTypeMap ( ListExpr in_xArgs )
{
  if ( nl->ListLength ( in_xArgs ) != 1 )
  {
    sendMessage ( "Expects a list of length 1." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  ListExpr xsource = nl->First ( in_xArgs );

  if ( ( !nl->IsAtom ( xsource ) ) ||
          !nl->IsEqual ( xsource, "gline" ) )
  {
    sendMessage ( "Element must be of type gline." );
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return nl->SymbolAtom ( Line::BasicType() );
}

int OpGLine2LineValueMapping ( Word* args,
                               Word& result,
                               int message,
                               Word& local,
                               Supplier in_xSupplier )
{
  Line* pLine = ( Line* ) qp->ResultStorage ( in_xSupplier ).addr;
  result = SetWord ( pLine );
  GLine* pGLine = ( GLine* ) args[0].addr;
  if ( pGLine == NULL || !pGLine->IsDefined() )
  {
    sendMessage ( "GLine must be defined!" );
    pLine->SetDefined ( false );
    return 0;
  }
  pGLine->Gline2line ( pLine );
  return 0;
}

const string OpGLine2LineSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>gline -> line" "</text--->"
    "<text>gline2line(GLINE)</text--->"
    "<text>Returns the line value of the gline.</text--->"
    "<text> gline2line(gline) </text--->"
    ") )";

Operator networkgline2line (
    "gline2line",               // name
    OpGLine2LineSpec,          // specification
    OpGLine2LineValueMapping,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpGLine2LineTypeMap        // type mapping
);

/*
4.19 Operator ~isempty~

Returns if the ~GLine~. is empty.

*/
ListExpr OpNetIsEmptyTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr gline = nl->First ( args );
  if ( !nl->IsAtom ( gline ) || nl->AtomType ( gline ) != SymbolType ||
          nl->SymbolValue ( gline ) != "gline" )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return nl->SymbolAtom ( CcBool::BasicType() );
}

int OpNetIsEmptyValueMap ( Word* args, Word& result, int message,
                           Word& local, Supplier in_pSupplier )
{
  GLine* pGline = ( GLine* ) args[0].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcBool* pResult = ( CcBool* ) result.addr;
  if ( ( ! ( pGline->IsDefined() ) ) || pGline->NoOfComponents() == 0 )
  {
    pResult->Set ( true, true );
    return 0;
  }
  else
  {
    pResult->Set ( true,false );
    return 0;
  }
}

const string OpNetIsEmptySpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>gline -> bool" "</text--->"
    "<text>isemtpy(_)</text--->"
    "<text>Returns true if the gline is empty.</text--->"
    "<text>query isempty(gline)</text--->"
    ") )";

Operator networkisempty (
    "isempty",               // name
    OpNetIsEmptySpec,          // specification
    OpNetIsEmptyValueMap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpNetIsEmptyTypeMap        // type mapping
);

/*
4.20 Operator ~union~

Builds the union of the two given glines as sorted gline.

*/

ListExpr OpNetUnionTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr gline1 = nl->First ( args );
  ListExpr gline2 = nl->Second ( args );

  if ( !nl->IsAtom ( gline1 ) || nl->AtomType ( gline1 ) != SymbolType ||
          nl->SymbolValue ( gline1 ) != "gline" )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }

  if ( !nl->IsAtom ( gline2 ) || nl->AtomType ( gline2 ) != SymbolType ||
          nl->SymbolValue ( gline2 ) != "gline" )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  return nl->SymbolAtom ( "gline" );
}

int OpNetUnionValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  GLine *pGL1 = ( GLine* ) args[0].addr;
  GLine *pGL2 = ( GLine* ) args[1].addr;
  GLine *pGLine = ( GLine* ) qp->ResultStorage ( in_pSupplier ).addr;
  result = SetWord ( pGLine );
  pGL1->Uniongl ( pGL2, pGLine );
  return 0;
}

const string OpNetUnionSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>gline x gline -> gline" "</text--->"
    "<text>_ union _ </text--->"
    "<text>Returns the sorted gline resulting of the union of both "
    "gline.</text--->"
    "<text>query gline1 union gline2</text--->"
    ") )";

Operator networkunion (
    "union",               // name
    OpNetUnionSpec,          // specification
    OpNetUnionValueMap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpNetUnionTypeMap        // type mapping
);

/*
4.21 Operator ~distance~

Returns the Euclidean Distance between two ~Gpoints~ or two ~GLines~.

*/

ListExpr OpNetDistanceTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( Symbol::TYPEERROR() ) );
  }
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second ( args );

  if ( ( nl->IsAtom ( param1 ) && nl->AtomType ( param1 ) == SymbolType &&
          nl->IsAtom ( param2 ) && nl->AtomType ( param2 ) == SymbolType &&
          ( ( nl->SymbolValue ( param1 ) == "gpoint" &&
              nl->SymbolValue ( param2 ) == "gpoint" ) ||
            ( nl->SymbolValue ( param1 ) == "gline" &&
              nl->SymbolValue ( param2 ) == "gline" ) ) ) )
  {
    return nl->SymbolAtom ( CcReal::BasicType() );
  }
  return nl->SymbolAtom ( Symbol::TYPEERROR() );
}

int OpNetDistance_gpgp ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  GPoint* pFromGPoint = ( GPoint* ) args[0].addr;
  GPoint* pToGPoint = ( GPoint* ) args[1].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcReal* pResult = ( CcReal* ) result.addr;
  if ( ! ( pFromGPoint->IsDefined() ) || ! ( pToGPoint->IsDefined() ) )
  {
    cmsg.inFunError ( "Both gpoint must be defined!" );
    return 0;
  };
  pResult-> Set ( true, pFromGPoint->Distance ( pToGPoint ) );
  return 0;
};

int OpNetDistance_glgl ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  GLine* pGLine1 = ( GLine* ) args[0].addr;
  GLine* pGLine2 = ( GLine* ) args[1].addr;
  result = qp->ResultStorage ( in_pSupplier );
  CcReal* pResult = ( CcReal* ) result.addr;
  if ( ! ( pGLine1->IsDefined() ) || ! ( pGLine2->IsDefined() ) ||
          pGLine1->NoOfComponents() == 0 || pGLine2->NoOfComponents() == 0 )
  {
    cmsg.inFunError ( "Both gline must be defined! And have at least 1 interval"
);
    return 0;
  };
  pResult-> Set ( true, pGLine1->Distance ( pGLine2 ) );
  return 1;
};

ValueMapping OpNetDistancemap[] =
{
  OpNetDistance_gpgp,
  OpNetDistance_glgl
};

int OpNetDistanceselect ( ListExpr args )
{
  ListExpr arg1 = nl->First ( args );
  ListExpr arg2 = nl->Second ( args );
  if ( nl->SymbolValue ( arg1 ) == "gpoint" &&
          nl->SymbolValue ( arg2 ) == "gpoint" )
    return 0;
  if ( nl->SymbolValue ( arg1 ) == "gline" &&
          nl->SymbolValue ( arg2 ) == "gline" )
    return 1;
  return -1; // This point should never be reached
};

const string OpNetDistanceSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>A x A -> real with A = gpoint or gline" "</text--->"
    "<text>distance(GPOINT1,GPOINT2)</text--->"
    "<text>Calculates the Euclidean Distance of gpoints resp. glines.</text--->"
    "<text>query distance(gp1,gp2)</text--->"
    ") )";

Operator networkdistance (
    "distance",               // name
    OpNetDistanceSpec,          // specification
    2,
    OpNetDistancemap,  // value mapping
    OpNetDistanceselect,        // selection function
    OpNetDistanceTypeMap        // type mapping
);

/*
4.22 Operator ~update\_sectioncost~

Updates the costs of one or more sections.

The oprator expects two or three arguments. The first argument should be a
network. Is the number of arguments two. The second agument should be a relation
like (rel (tuple ((sid int) (cost real)))). If the number of arguments is three,
then the second argument should be an sectionid (int) and the third argument
should be the cost of the section (real).

*/
ListExpr OpUpdateSectionCostTypeMap ( ListExpr in_xArgs ){

    NList param(in_xArgs);
    //check the number of arguments
    if(param.length() <2 || param.length() >3)
    {
        return listutils::typeError(
               "update_sectioncost expects either 2 or 3 arguments.");
    }
    ListExpr xNetworkDesc = nl->First(in_xArgs);
    //check the type of the first arguments
    if ( ( !nl->IsAtom ( xNetworkDesc ) ) ||
          !nl->IsEqual ( xNetworkDesc, "network" ) )
    {
        return listutils::typeError("1.argument should be a network");
    }


    if (param.length() == 2)
    {   //read the arguments
        ListExpr xRelDesc = nl->Second(in_xArgs);

        if ( !IsRelDescription ( xRelDesc ) )
        {
            return listutils::typeError( "2.agument should be a relation" );
        }

        ListExpr xType;
        string costsTypeInfo = "(rel (tuple ((sid int) (cost real))))";

        nl->ReadFromString ( costsTypeInfo, xType );
        if ( !CompareSchemas ( xRelDesc, xType ) )
        {
           return listutils::typeError("the pattern of the relation ist wrong");
        }
    }
    if ( param.length() == 3 )
    {   //read the arguments
        ListExpr xIdDesc = nl->Second(in_xArgs);
        ListExpr xCostDesc = nl->Third(in_xArgs);
        if (!nl->IsEqual(xIdDesc, CcInt::BasicType()))
        {
            return listutils::typeError("2.argument should be an int.");
        }
        if(!nl->IsEqual (xCostDesc, CcReal::BasicType()))
        {
            return listutils::typeError("3.argument should be an real.");
        }

    }

    return nl->SymbolAtom ( CcBool::BasicType() );
}

int OpUpdateSectionCost_One ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
    Network* pNetwork = ( Network* ) args[0].addr;
    int secId = (( CcInt* ) args[1].addr)->GetIntval();
    double newCost = (( CcReal* ) args[2].addr)->GetRealval();
    result = qp->ResultStorage ( in_pSupplier );
    CcBool* pResult = ( CcBool* ) result.addr;

    if ( pNetwork == 0 || !pNetwork->IsDefined() )
    {
        string strMessage = "Network is not defined.";
        cerr << strMessage << endl;
        sendMessage ( strMessage );
        pResult->Set ( false, false );
        return 0;
    }
   qp->SetModified(qp->GetSon(in_pSupplier,0));
   pResult->Set ( true, pNetwork->UpdateSectionCost(secId,newCost));
   return 0;
}

int OpUpdateSectionCost_More ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
    Network* pNetwork = ( Network* ) args[0].addr;
    Relation* pCosts = ( Relation* ) args[1].addr;
    result = qp->ResultStorage ( in_pSupplier );
    CcBool* pResult = ( CcBool* ) result.addr;

    if ( pNetwork == 0 || !pNetwork->IsDefined() )
    {
        string strMessage = "Network is not defined.";
        cerr << strMessage << endl;
        sendMessage ( strMessage );
        pResult->Set ( false, false );
        return 0;
    }
   qp->SetModified(qp->GetSon(in_pSupplier,0));
   pResult->Set ( true, pNetwork->UpdateMoreSectionCost(pCosts));
   return 0;
}

ValueMapping OpUpdateSectionCostMap[]=
{   OpUpdateSectionCost_One,
    OpUpdateSectionCost_More
};

int OpUpdateSectionCostSelect(ListExpr in_xArgs){
     NList param(in_xArgs);
    //check the number of arguments
    if ( param.length() == 3 ){
        return 0;
    }
    if(param.length() == 2){
        return 1;
    }
    return -1;
}

const string OpUpdateSectionCostSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text> network x A [x real] -> bool with A as int or as rel </text--->"
    "<text> update_sectioncost(_,_,_) </text--->"
    "<text>Returns true, when the Update was successful.</text--->"
    "<text>query update_sectioncost(B_NETWORK,1,5.0)</text--->"
    ") )";

Operator update_sectioncost (
    "update_sectioncost",       // name
    OpUpdateSectionCostSpec,    // specification
    2,
    OpUpdateSectionCostMap,     // value mapping
    OpUpdateSectionCostSelect,  // selection function
    OpUpdateSectionCostTypeMap  // type mapping
);

/*
4.23 Operator ~update\_sectionduration~

Updates the duration of one or more sections.

The oprator expects two or three arguments. The first argument should be a
network. Is the number of arguments two. The second agument should be a relation
like (rel (tuple ((sid int) (duration real)))). If the number of arguments is
three, then the second argument should be an sectionid (int) and the third
argument should be the duration of the section (real).

*/
ListExpr OpUpdateSectionDurationTypeMap ( ListExpr in_xArgs ){

    NList param(in_xArgs);
    //check the number of arguments
    if(param.length() <2 || param.length() >3){
        return listutils::typeError(
               "update_sectionduration expects either 2 or 3 arguments.");
    }
    ListExpr xNetworkDesc = nl->First(in_xArgs);
    //check the type of the first arguments
    if ( ( !nl->IsAtom ( xNetworkDesc ) ) ||
          !nl->IsEqual ( xNetworkDesc, "network" ) )
    {
        return listutils::typeError("1.argument should be a network");
    }


    if (param.length() == 2)
    {   //read the arguments
        ListExpr xRelDesc = nl->Second(in_xArgs);

        if ( !IsRelDescription ( xRelDesc ) )
        {
            return listutils::typeError( "2.agument should be a relation" );
        }

        ListExpr xType;
        string durationsTypeInfo = "(rel (tuple ((sid int) (duration real))))";

        nl->ReadFromString ( durationsTypeInfo, xType );
        if ( !CompareSchemas ( xRelDesc, xType ) )
        {
           return listutils::typeError("the pattern of the relation ist wrong");
        }
    }
    if ( param.length() == 3 )
    {   //read the arguments
        ListExpr xIdDesc = nl->Second(in_xArgs);
        ListExpr xDurationDesc = nl->Third(in_xArgs);
        if (!nl->IsEqual(xIdDesc, CcInt::BasicType()))
        {
            return listutils::typeError("2.argument should be an int.");
        }
        if(!nl->IsEqual (xDurationDesc, CcReal::BasicType()))
        {
            return listutils::typeError("3.argument should be an real.");
        }

    }

    return nl->SymbolAtom ( CcBool::BasicType() );
}

int OpUpdateSectionDuration_One ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
    Network* pNetwork = ( Network* ) args[0].addr;
    int secId = (( CcInt* ) args[1].addr)->GetIntval();
    double newDuration = (( CcReal* ) args[2].addr)->GetRealval();
    result = qp->ResultStorage ( in_pSupplier );
    CcBool* pResult = ( CcBool* ) result.addr;

    if ( pNetwork == 0 || !pNetwork->IsDefined() )
    {
        string strMessage = "Network is not defined.";
        cerr << strMessage << endl;
        sendMessage ( strMessage );
        pResult->Set ( false, false );
        return 0;
    }
   qp->SetModified(qp->GetSon(in_pSupplier,0));
   pResult->Set ( true, pNetwork->UpdateSectionDuration(secId,newDuration));
   return 0;
}
int OpUpdateSectionDuration_More ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
    Network* pNetwork = ( Network* ) args[0].addr;
    Relation* pDurations = ( Relation* ) args[1].addr;
    result = qp->ResultStorage ( in_pSupplier );
    CcBool* pResult = ( CcBool* ) result.addr;

    if ( pNetwork == 0 || !pNetwork->IsDefined() )
    {
        string strMessage = "Network is not defined.";
        cerr << strMessage << endl;
        sendMessage ( strMessage );
        pResult->Set ( false, false );
        return 0;
    }
   qp->SetModified(qp->GetSon(in_pSupplier,0));
   pResult->Set ( true, pNetwork->UpdateMoreSectionDuration(pDurations));
   return 0;
}

ValueMapping OpUpdateSectionDurationMap[]=
{   OpUpdateSectionDuration_One,
    OpUpdateSectionDuration_More
};

int OpUpdateSectionDurationSelect(ListExpr in_xArgs){
     NList param(in_xArgs);
    //check the number of arguments
    if ( param.length() == 3 ){
        return 0;
    }
    if(param.length() == 2){
        return 1;
    }
    return -1;
}

const string OpUpdateSectionDurationSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text> network x A [x real] -> bool with A as int or as rel </text--->"
    "<text> update_sectionduration(_,_,_) </text--->"
    "<text>Returns true, when the Update was successful.</text--->"
    "<text>query update_sectionduration(B_NETWORK,1,5.0)</text--->"
    ") )";

Operator update_sectionduration (
    "update_sectionduration",       // name
    OpUpdateSectionDurationSpec,    // specification
    2,
    OpUpdateSectionDurationMap,     // value mapping
    OpUpdateSectionDurationSelect,  // selection function
    OpUpdateSectionDurationTypeMap  // type mapping
);

/*
4.24 Operator ~shortespath\_astarplus~

Returns the shortest path in the ~Network~ between two ~GPoint~. Using
AStar-Algorithm to compute the shortest path.

The operator expects either 2 or 3 arguments. The first and the second argument
should be a gpoint. If the number of arguments ist three, then the third
argument should be a function with three parameters.

*/

ListExpr OpShortestPathAStarPlusTypeMap ( ListExpr in_xArgs )
{
    //check the number of arguments
    if(nl->ListLength ( in_xArgs )  <2 || nl->ListLength ( in_xArgs ) >3){
        return listutils::typeError(
               "shortestpath_astarplus expects either 2 or 3 arguments.");
    }

    ListExpr xGPoint1Desc = nl->First (in_xArgs);
    ListExpr xGPoint2Desc = nl->Second (in_xArgs);

    if ( ( !nl->IsAtom ( xGPoint1Desc ) ) ||
            nl->AtomType ( xGPoint1Desc ) != SymbolType ||
            nl->SymbolValue ( xGPoint1Desc ) != "gpoint" )
    {
        return listutils::typeError("the first parameter should be a gpoint.");
    }
    if ( ( !nl->IsAtom ( xGPoint2Desc ) ) ||
            nl->AtomType ( xGPoint2Desc ) != SymbolType ||
            nl->SymbolValue ( xGPoint2Desc ) != "gpoint" )
    {
        return listutils::typeError("the second parameter should be a gpoint.");
    }

    if(nl->ListLength ( in_xArgs ) == 3)
    {
        ListExpr map = nl->Third(in_xArgs);
        if (  nl->IsAtom( map ) || !( nl->IsEqual(nl->First(map),
                                                  Symbol::MAP()) ) )
        {
            return listutils::typeError(
                   "the third parameter should be a function.");
        }
        else
        {
            if ( nl->ListLength(map) != 5)
            {
              return NList::typeError(
              "the third parameter should be a function with three parameter.");
            }
            if(!( nl->IsEqual(nl->Second(map), CcReal::BasicType()))
               || !( nl->IsEqual(nl->Third(map), CcReal::BasicType()))
               || !( nl->IsEqual(nl->Fourth(map), CcReal::BasicType())) ){
             return listutils::typeError(
             "one or more parameters has the wrong type, it should be a real.");
            }
        }
    }

    return nl->SymbolAtom ( "gline" );
}

int OpShortestPathAStarPlusValueMapping ( Word* args,
                                 Word& result,
                                 int message,
                                 Word& local,
                                 Supplier in_xSupplier )
{
  GPoint *pFromGPoint = ( GPoint* ) args[0].addr;
  GPoint *pToGPoint = ( GPoint* ) args[1].addr;
  Word func = args[2].addr;

  GLine* pGLine = ( GLine* ) qp->ResultStorage ( in_xSupplier ).addr;
  result = SetWord ( pGLine );

  pGLine->SetSorted ( false );
  pGLine->SetDefined (
          pFromGPoint->ShortestPathAStarPlus ( pToGPoint, pGLine, func ) );
  return 0;
}

const string OpShortestPathAStarPlusSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>gpoint x gpoint [x A] -> gline with A as (map real real real real)"
    "</text--->"
    "<text>shortestpath_astarplus( _ , _ ,_)</t1ext--->"
    "<text>Calculates the shortest path between two "
    "gpoints with the astar-algorithm.</text--->"
    "<text>query shortestpath_astarplus([const gpoint value(1 2 0.0 0)],"
    "[const gpoint value(1 10 5.0 1)]);</text--->"
    ") )";

Operator shortestpath_astarplus (
    "shortestpath_astarplus",    // name
    OpShortestPathAStarPlusSpec,          // specification
    OpShortestPathAStarPlusValueMapping,  // value mapping
    Operator::SimpleSelect,        // trivial selection function
    OpShortestPathAStarPlusTypeMap        // type mapping
);

/*
4.25 Operator ~optimize\_network~

Optimizes the given ~network~ by all routes contained sorted by lexicographic
starting point and all sections are rearranged.

*/

ListExpr OpOptimizeNetworkTypeMap ( ListExpr in_xArgs )
{
    //check the number of arguments
    if(nl->ListLength ( in_xArgs )  !=1){
        return listutils::typeError(
               "optimize_network expects only one argument.");
    }
    ListExpr xNetworkDesc = nl->First(in_xArgs);
    //check the type of the first arguments
    if ( ( !nl->IsAtom ( xNetworkDesc ) ) ||
          !nl->IsEqual ( xNetworkDesc, "network" ) )
    {
        return listutils::typeError("1.argument should be a network");
    }
    return nl->SymbolAtom ( CcBool::BasicType() );
}

int OpOptimizeNetworkValueMapping ( Word* args,
                                 Word& result,
                                 int message,
                                 Word& local,
                                 Supplier in_xSupplier )
{
    Network* pNetwork = ( Network* ) args[0].addr;
    result = qp->ResultStorage ( in_xSupplier );
    CcBool* pResult = ( CcBool* ) result.addr;

    if ( pNetwork == 0 || !pNetwork->IsDefined() )
    {
        string strMessage = "Network is not defined.";
        cerr << strMessage << endl;
        sendMessage ( strMessage );
        pResult->Set ( false, false );
        return 0;
    }
   qp->SetModified(qp->GetSon(in_xSupplier,0));
   pResult->Set ( true, pNetwork->OptimizeNetwork());
   return 0;
}

const string OpOptimizeNetworkSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network -> bool)" "</text--->"
    "<text>optimizenet( _ )</t1ext--->"
    "<text>Optimized the storage of the sections.</text--->"
    "<text>query optimize_network(B_NETWORK)]);</text--->"
    ") )";

Operator optimizenet (
    "optimizenet",    // name
    OpOptimizeNetworkSpec,         // specification
    OpOptimizeNetworkValueMapping,  // value mapping
    Operator::SimpleSelect,        // trivial selection function
    OpOptimizeNetworkTypeMap        // type mapping
);

/*
4.26 Operator ~print\_adjacencylist~

Prints infromations of routes, sections and their neighbours of the
given ~network~.

*/

ListExpr OpPrintAdjacencyListTypeMap ( ListExpr in_xArgs )
{
    //check the number of arguments
    if(nl->ListLength ( in_xArgs )  !=1){
        return listutils::typeError(
               "optimize_network expects only one argument.");
    }
    ListExpr xNetworkDesc = nl->First(in_xArgs);
    //check the type of the first arguments
    if ( ( !nl->IsAtom ( xNetworkDesc ) ) ||
          !nl->IsEqual ( xNetworkDesc, "network" ) )
    {
        return listutils::typeError("1.argument should be a network");
    }
    return nl->SymbolAtom ( CcBool::BasicType() );
}

int OpPrintAdjacencyListValueMapping ( Word* args,
                                 Word& result,
                                 int message,
                                 Word& local,
                                 Supplier in_xSupplier )
{
    Network* pNetwork = ( Network* ) args[0].addr;
    result = qp->ResultStorage ( in_xSupplier );
    CcBool* pResult = ( CcBool* ) result.addr;

    if ( pNetwork == 0 || !pNetwork->IsDefined() )
    {
        string strMessage = "Network is not defined.";
        cerr << strMessage << endl;
        sendMessage ( strMessage );
        pResult->Set ( false, false );
        return 0;
    }
   pResult->Set ( true, pNetwork->PrintAdjacentSectionInfo());
   return 0;
}

const string OpPrintAdjacencyListSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network -> bool)" "</text--->"
    "<text>print_adjacencylist( _ )</t1ext--->"
    "<text>print the informations about the adjacent sections.</text--->"
    "<text>query print_adjacencylist(B_NETWORK)]);</text--->"
    ") )";

Operator print_adjacencylist (
    "print_adjacencylist",    // name
    OpPrintAdjacencyListSpec,         // specification
    OpPrintAdjacencyListValueMapping,  // value mapping
    Operator::SimpleSelect,        // trivial selection function
    OpPrintAdjacencyListTypeMap        // type mapping
);



/*
5 Creating the ~NetworkAlgebra~

*/

class NetworkAlgebra : public Algebra
{
  public:
    NetworkAlgebra() : Algebra()
    {
      AddTypeConstructor ( &network );
      AddTypeConstructor ( &gpoint );
      AddTypeConstructor ( &gline );
      AddTypeConstructor ( &gpoints );

      gpoint.AssociateKind ( Kind::DATA() );
      gline.AssociateKind ( Kind::DATA() );
      network.AssociateKind ( Kind::NETWORK() );
      gpoints.AssociateKind ( Kind::DATA() );


      AddOperator ( &networkthenetwork );
      AddOperator ( &networkroutes );
      AddOperator ( &networkjunctions );
      AddOperator ( &networksections );
      AddOperator ( &shortest_path );
      AddOperator ( &networklength );
      AddOperator ( &networknetdistance );
      AddOperator ( &point2gpoint );
      AddOperator ( &gpoint2point );
      AddOperator ( &netgpequal );
      AddOperator ( &sline2gline );
      AddOperator ( &networkinside );
      AddOperator ( &networknocomponents );
      AddOperator ( &polygpoints );
      AddOperator ( &networkrouteintervals );
      AddOperator ( &networkintersects );
      AddOperator ( &networkgpoint2rect );
      AddOperator ( &networkgline2line );
      AddOperator ( &networkisempty );
      AddOperator ( &networkunion );
      AddOperator ( &networkdistance );
      AddOperator ( &update_sectioncost);
      AddOperator ( &update_sectionduration);
      AddOperator ( &shortestpath_astarplus);
      AddOperator ( &optimizenet);
      AddOperator ( &print_adjacencylist);

    }
    ~NetworkAlgebra() {delete netList;};
};

/*
Initialization

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

extern "C"
  Algebra*
  InitializeNetworkAlgebra ( NestedList* nlRef,
                             QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  netList = new map<int,string>();
  return ( new NetworkAlgebra() );
}
