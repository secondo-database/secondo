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

1.1 Implementation of Operator Trajectory

The operator finds the trajectory of a moving points

Oktober 2007 Martin Scheppokat

March 2008 Simone Jandt ValueMapping added to reduce the number of parts of the
resulting gline.

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

#include "OpTrajectory.h"


struct RITree {

  RITree(){};

  ~RITree();

  RITree( int ri,double pos1, double pos2, RITree *left = 0, RITree *right = 0){
    m_iRouteId = ri;
    m_dStart = pos1;
    m_dEnd = pos2;
    m_left = left;
    m_right = right;
  };

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
            double result;
            if (bleft) {
              if (this->m_dStart <= pos1) {
                result = this->m_dStart;
                if (father.m_left == this) {
                  father.m_left = this->m_left;
                } else {
                  father.m_right = this->m_left;
                }
                return result;
              }
            } else {
              if (this->m_dEnd >= pos2) {
                result = this->m_dEnd;
                if (father.m_right == this) {
                  father.m_right = this->m_right;
                } else {
                  father.m_left = this->m_right;
                }
                return result;
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
    cout << "insert: " << rid << ", " <<pos1 << ", " << pos2 << endl;
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

/*
Check if StartPos <= EndPos if not change StartPos and EndPos.

*/
void chkStartEnd(double &StartPos, double &EndPos){
  double help;
  if (StartPos > EndPos) {
    help = StartPos;
    StartPos = EndPos;
    EndPos = help;
  }
};


/*
Typemap function of the operator

*/
ListExpr OpTrajectory::TypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom( "gline" );
}

/*
Value mapping function of operator ~trajectory~

*/
int OpTrajectory::ValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  // Get (empty) return value
  GLine* pGLine = (GLine*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pGLine);
  // Get input values
  MGPoint* pMGPoint = (MGPoint*)args[0].addr;
  if(pMGPoint == NULL || pMGPoint->GetNoComponents() < 1 ||
      !pMGPoint->IsDefined()) {
    cerr << "MGPoint does not exist." << endl;
    pGLine->SetDefined(false);
    return 0;
  }
  const UGPoint *pCurrentUnit;
  pMGPoint->Get(0, pCurrentUnit);
  int iNetworkId = pCurrentUnit->p0.GetNetworkId();
  pGLine->SetNetworkId(iNetworkId);
  pGLine->SetDefined(true);
  int aktRouteId = pCurrentUnit->p0.GetRouteId();
  double aktStartPos = pCurrentUnit->p0.GetPosition();
  double aktEndPos = pCurrentUnit->p1.GetPosition();
  chkStartEnd(aktStartPos, aktEndPos);
  double curStartPos, curEndPos;
  RITree *tree;
  tree = new RITree(aktRouteId, aktStartPos, aktEndPos,0,0);
  int curRoute;
  for (int i = 1; i < pMGPoint->GetNoComponents(); i++)
  {
    // Get start and end of current unit
    pMGPoint->Get(i, pCurrentUnit);
    curRoute = pCurrentUnit->p0.GetRouteId();
    curStartPos = pCurrentUnit->p0.GetPosition();
    curEndPos = pCurrentUnit->p1.GetPosition();
    chkStartEnd(curStartPos, curEndPos);
    if (curRoute != aktRouteId) {
      tree->insert(aktRouteId, aktStartPos, aktEndPos);
      aktRouteId = curRoute;
      aktStartPos = curStartPos;
      aktEndPos = curEndPos;
    } else { // curRoute == aktRouteId concat pieces if possible
      if (AlmostEqual(aktStartPos,curEndPos)) {
        aktStartPos = curStartPos;
      } else {
        if (AlmostEqual(aktEndPos,curStartPos)) {
          aktEndPos = curEndPos;
        } else { //concat impossible start new routeInterval for gline.
          tree->insert(aktRouteId, aktStartPos, aktEndPos);
          aktRouteId = curRoute;
          aktStartPos = curStartPos;
          aktEndPos = curEndPos;
        }
      }
    }
  }
  tree->insert(aktRouteId, aktStartPos, aktEndPos);
  GLine *resG = new GLine(0);
//   RITree *tnew;
//   tnew = tree->initNewTree();
//   tree->tree2tree(tnew);
//   tnew->treeToGLine(resG);
  tree->treeToGLine(resG);
  result = SetWord(resG);
  resG->SetDefined(true);
  resG->SetNetworkId(iNetworkId);
  return 0;
}


/*
Specification of operator ~trajectory~

*/
const string OpTrajectory::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> gline" "</text--->"
  "<text>trajectory(mgpoint)</text--->"
  "<text>Calculates the trajectory for a moving gpoint.</text--->"
  "<text>trajectory(mgpoint)</text--->"
  ") )";

