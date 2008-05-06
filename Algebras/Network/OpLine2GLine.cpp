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

1.1 Implementation of Operator line2gline

The operator translates a line into a network based gline

March 2008 Simone Jandt

Defines, includes, and constants

*/

#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DBArray.h"
#include "TupleIdentifier.h"

#include "StandardTypes.h"

#include "TemporalAlgebra.h"
#include "NetworkAlgebra.h"
#include "GPoint.h"
#include "GLine.h"
#include "UGPoint.h"
#include "MGPoint.h"
#include "TemporalNetAlgebra.h"
#include "Network.h"
#include "NetworkManager.h"
#include "OpPoint2GPoint.h"
#include "OpLine2GLine.h"
struct RITree {

  RITree(){};

  RITree( int ri,double pos1, double pos2, RITree *left = 0, RITree *right = 0){
    m_iRouteId = ri;
    m_dStart = pos1;
    m_dEnd = pos2;
    m_left = left;
    m_right = right;
  };

  ~RITree();

  double checkTree(RITree& father, int rid, double pos1, double pos2,
                   bool bleft) {
    if (rid < this->m_iRouteId) {
      if (this->m_left != 0) {
        return this->m_left->checkTree(*this, rid, pos1, pos2, bleft);
      } else {
        if (bleft) return pos1;
        else return pos2;
      }
    } else {
      if (rid > this->m_iRouteId) {
        if (this->m_right != 0) {
          return this->m_right->checkTree(*this, rid, pos1, pos2,bleft);
        } else {
          if (bleft) return pos1;
          else return pos2;
        }
      } else {
        if (pos2 < this->m_dStart) {
          if (this->m_left != 0) {
            return this->m_left->checkTree(*this, rid, pos1, pos2,bleft);
          } else {
            if (bleft) return pos1;
            else return pos2;
          }
        } else {
          if (pos1 > this->m_dEnd) {
            if (this->m_right != 0 ) {
              return this->m_right->checkTree(*this, rid, pos1, pos2,bleft);
            } else {
              if (bleft) return pos1;
              else return pos2;
            }
          } else {
/*
Overlapping interval found. Rebuild Tree and return new interval limit.

*/
//             double result;
//             if (bleft) {
//               if (this->m_dStart <= pos1) {
//                 result = this->m_dStart;
//                 if (father.m_left == this) {
//                   father.m_left = this->m_left;
//                 } else {
//                   father.m_right = this->m_left;
//                 }
//                 return result;
//               }
//             } else {
//               if (this->m_dEnd >= pos2) {
//                 result = this->m_dEnd;
//                 if (father.m_right == this) {
//                   father.m_right = this->m_right;
//                 } else {
//                   father.m_left = this->m_right;
//                 }
//                 return result;
//               }
//             }
            if (bleft) {
              if (this->m_dStart <= pos1) {
                  pos1 = this->m_dStart;
              }
              if (father.m_left == this) {
                father.m_left = this->m_left;
              } else {
                father.m_right = this->m_left;
              }
              if (father.m_left != 0) {
                //delete this;
                return father.m_left->checkTree(father, rid, pos1, pos2, bleft);
              } else {
                return pos1;
              }
            } else {
              if (this->m_dEnd >= pos2) {
                pos2 = this->m_dEnd;
              }
              if (father.m_left == this) {
                father.m_left = this->m_right;
              } else {
                father.m_right = this->m_right;
              }
              if (father.m_right != 0 ) {
                //delete this;
                return father.m_right->checkTree(father, rid, pos1, pos2,bleft);
              } else {
                return pos2;
              }
            }
          }
        }
      }
    }
    if (bleft) return pos1;
    else return pos2;
  };


  void insert (int rid, double pos1, double pos2) {
    double test;
    if (rid < this->m_iRouteId) {
      if (this->m_left != 0) {
        this->m_left->insert(rid, pos1, pos2);
      } else {
        this->m_left = new RITree(rid, pos1, pos2,0,0);
      }
    } else {
      if (rid > this->m_iRouteId) {
        if (this->m_right != 0) {
          this->m_right->insert(rid, pos1, pos2);
        } else {
          this->m_right = new RITree(rid, pos1, pos2,0,0);
        }
      }else{
        if(rid == this->m_iRouteId) {
          if (pos2 < this->m_dStart) {
            if (this->m_left != 0) {
               this->m_left->insert(rid, pos1, pos2);
            } else {
                this->m_left = new RITree(rid, pos1, pos2,0,0);
            }
          } else {
            if (pos1 > this->m_dEnd) {
              if (this->m_right != 0) {
                this->m_right->insert(rid, pos1, pos2);
              } else {
                this->m_right =
                    new RITree(rid, pos1, pos2,0,0);
              }
            } else {
/*
Overlapping route intervals merge and check sons if they need to be corrected.

*/
              if (this->m_dStart > pos1) {
                this->m_dStart = pos1;
                if (this->m_left != 0) {
                  test = this->m_left->checkTree(*this, rid, this->m_dStart,
                                                this->m_dEnd, true);
                  if (this->m_dStart > test) {
                    this->m_dStart = test;
                  }
                }
              }
              if (this->m_dEnd < pos2) {
                this->m_dEnd = pos2;
                if (this->m_right != 0) {
                  test = this->m_right->checkTree(*this, rid, this->m_dStart,
                                                  this->m_dEnd, false);
                  if (this->m_dEnd < test) {
                    this->m_dEnd = test;
                  }
                }
              }
            }
          }
        } // endif rid=rid
      }
    }
  };

  void treeToGLine (GLine *gline) {
    if (this->m_left != 0) {
      this->m_left->treeToGLine (gline);
    }
    gline->AddRouteInterval(this->m_iRouteId, this->m_dStart, this->m_dEnd);
    if (this->m_right != 0) {
      this->m_right->treeToGLine (gline);
    }
  };



  void tree2tree (RITree *tnew) {
    if (this->m_left != 0) {
      this->m_left->tree2tree (tnew);
    }
    tnew->insert(this->m_iRouteId, this->m_dStart, this->m_dEnd);
    if (this->m_right != 0) {
      this->m_right->tree2tree (tnew);
    }
  };

  RITree* initNewTree() {
    RITree *akt = this;
    while (akt->m_left != 0) {
      akt = akt->m_left;
    }
    RITree *tnew =
        new RITree(akt->m_iRouteId, akt->m_dStart, akt->m_dEnd, 0, 0);
    return tnew;
  };

  int m_iRouteId;
  double m_dStart, m_dEnd;
  RITree *m_left, *m_right;
};

bool checkPoint (Line *route, Point point, bool startSmaller, double &pos){
  bool result = false;
  const HalfSegment *hs;
  double k1, k2;
  Point left, right;
  for (int i = 0; i < route->Size()-1; i++) {
    route->Get(i, hs);
    left = hs->GetLeftPoint();
    right = hs->GetRightPoint();
    Coord xl = left.GetX(),
          yl = left.GetY(),
          xr = right.GetX(),
          yr = right.GetY(),
          x = point.GetX(),
          y = point.GetY();
    if ((fabs(x-xr) < 0.01 && fabs (y-yr) < 0.01) ||
       (fabs(x-xl) < 0.01 && fabs (y-yl) < 0.01)){
      result = true;
    } else {
      if (xl != xr && xl != x) {
        k1 = (y - yl) / (x - xl);
        k2 = (yr - yl) / (xr - xl);
        if ((fabs(k1-k2) < 0.01) &&
           ((xl < xr && (x > xl || fabs(x-xl) < 0.01) &&
           (x < xr || fabs(x-xr) < 0.01)) || (xl > xr &&  (x < xl ||
           fabs(x-xl)<0.01)  && ( x > xr || fabs(x-xr)))) && (((yl < yr ||
           fabs(yl-yr)<0.01) && (y > yl || fabs(y-yl)<0.01 )&& (y < yr ||
           fabs(y-yr)<0.01)) || (yl > yr && (y < yl || fabs(y-yl) <0.01) &&
           (y > yr || fabs(y-yr)<0.01)))) {
              result = true;
        } else {result = false;}
      } else {
        if (( fabs(xl - xr) < 0.01 && fabs(xl -x) < 0.01) &&
           (((yl < yr|| fabs(yl-yr)<0.01) && (yl < y || fabs(yl-y) <0.01)&&
           (y < yr ||fabs(y-yr)<0.01))|| (yl > yr && (yl > y ||
           fabs(yl-y)<0.01)&& (y > yr ||fabs(y-yr)<0.01)))) {
              result = true;
        } else {result = false;}
      }
    }
    if (result) {
      if (!(route->IsSimple())) {
        return false;
      } else {
        const LRS *lrs;
        route->Get( hs->attr.edgeno, lrs );
        route->Get( lrs->hsPos, hs );
        pos = lrs->lrsPos + point.Distance( hs->GetDomPoint() );
        if( startSmaller != route->GetStartSmaller())
          pos = route->Length() - pos;
        if( fabs(pos-0.0) < 0.01)
          pos = 0.0;
        else if (fabs(pos-route->Length())<0.01)
              pos = route->Length();
        return result;
      }
    }
  }
  return result;
};


/*
Typemap function of the operator

*/
ListExpr OpLine2GLine::TypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 ){
    sendMessage("Expects a list of length 2.");
    return (nl->SymbolAtom( "typeerror" ));
  }

  ListExpr xNetworkIdDesc = nl->First(in_xArgs);
  ListExpr xLineDesc = nl->Second(in_xArgs);

  if( (!nl->IsAtom(xNetworkIdDesc)) ||
      !nl->IsEqual(xNetworkIdDesc, "network"))
  {
    sendMessage("First element must be of type network.");
    return (nl->SymbolAtom("typeerror"));
  }

  if( (!nl->IsAtom( xLineDesc )) ||
      nl->AtomType( xLineDesc ) != SymbolType ||
      nl->SymbolValue( xLineDesc ) != "line" )
  {
    sendMessage("Second element must be of type line.");
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom( "gline" );
}

/*
Value mapping function of operator ~line2gline~

*/
int OpLine2GLine::ValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
/*
Initialize return value

*/
  GLine* pGLine = (GLine*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pGLine );
/*
Get and check input values.

*/
  Network* pNetwork = (Network*)args[0].addr;
  if (pNetwork == 0 || !pNetwork->isDefined()) {
    string strMessage = "Network is not defined.";
    cerr << strMessage << endl;
    sendMessage(strMessage);
    pGLine->SetDefined(false);
    return 0;
  }
  pGLine->SetNetworkId(pNetwork->GetId());
  Line* pLine = (Line*)args[1].addr;
  if(pLine == NULL || !pLine->IsDefined()) {
    string strMessage = "Line does not exist.";
    cerr << strMessage << endl;
    sendMessage(strMessage);
    pGLine->SetDefined(false);
    return 0;
  }
  if (pLine->Size() == 0) {
    pGLine->SetDefined(true);
    return 0;
  }
  const HalfSegment *hs;
  Relation *pRoutes = pNetwork->GetRoutes();
  RITree *tree;
  pLine->Get(0,hs);
  Tuple *pCurrentRoute;
  int iRouteTid, iRouteId;
  CcInt *pRouteId;
  Line *pRouteCurve;
  bool bLeftFound, bRightFound;
  double leftPos, rightPos;
  GenericRelationIterator* pRoutesIt = pRoutes->MakeScan();
  bool found = false;
  while( (pCurrentRoute = pRoutesIt->GetNextTuple()) != 0 && !found) {
    iRouteTid = -1;
    pRouteId = (CcInt*) pCurrentRoute->GetAttribute(ROUTE_ID);
    iRouteId = pRouteId->GetIntval();
    pRouteCurve = (Line*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
    bLeftFound = checkPoint(pRouteCurve, hs->GetLeftPoint(), true, leftPos);
    if (bLeftFound) {
      bRightFound =
          checkPoint(pRouteCurve, hs->GetRightPoint(), true, rightPos);
      if (bRightFound) {
        found = true;
        if (leftPos > rightPos) {
          tree = new RITree(iRouteId, rightPos, leftPos, 0,0);
        } else {
          tree = new RITree(iRouteId, leftPos, rightPos, 0,0);
        }
      }
    }
    pCurrentRoute->DeleteIfAllowed();
  }
  delete pRoutesIt;
  for (int i = 1 ; i < pLine->Size(); i++) {
    pLine->Get(i, hs);
    GenericRelationIterator* pRoutesIt = pRoutes->MakeScan();
    found = false;
    while( (pCurrentRoute = pRoutesIt->GetNextTuple()) != 0 && !found) {
      iRouteTid = -1;
      pRouteId = (CcInt*) pCurrentRoute->GetAttribute(ROUTE_ID);
      iRouteId = pRouteId->GetIntval();
      pRouteCurve = (Line*) pCurrentRoute->GetAttribute(ROUTE_CURVE);
      bLeftFound = checkPoint(pRouteCurve, hs->GetLeftPoint(), true, leftPos);
      if (bLeftFound) {
        bRightFound =
            checkPoint(pRouteCurve, hs->GetRightPoint(), true, rightPos);
        if (bRightFound) {
          found = true;
          if (leftPos > rightPos) {
            tree->insert(iRouteId, rightPos, leftPos);
          } else {
            tree->insert(iRouteId, leftPos, rightPos);
          }
        }
      }
      pCurrentRoute->DeleteIfAllowed();
    }
    delete pRoutesIt;
  } // end for pLine-Components
  GLine *resG = new GLine(0);
//   RITree *tnew;
//   tnew = tree->initNewTree();
//   tree->tree2tree(tnew);
//   tnew->treeToGLine(resG);
  tree->treeToGLine(resG);
  result =  SetWord(resG);
  resG->SetDefined(true);
  resG->SetNetworkId(pNetwork->GetId());
  resG->SetSorted(true);
  return 0;
} //end ValueMapping


/*
Specification of operator ~line2gline~

*/
const string OpLine2GLine::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>network x line -> gline" "</text--->"
  "<text>line2gline(_,_)</text--->"
  "<text>Translates a line to a gline value.</text--->"
  "<text>line2gline(B_NETWORK, line)</text--->"
  ") )";

/*
Sending a message via the message-center

*/
void OpLine2GLine::sendMessage(string in_strMessage)
{
  // Get message-center and initialize message-list
  static MessageCenter* xMessageCenter= MessageCenter::GetInstance();
  NList xMessage;
  xMessage.append(NList("error"));
  xMessage.append(NList().textAtom(in_strMessage));
  xMessageCenter->Send(xMessage);
}
