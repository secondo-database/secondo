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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[&] [\&]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]
//[ast] [\ensuremath{\ast}]

*/

/*
[1] ExtRelation2AlgebraCostEstimation 

Mai, 2012. Jan Kristof Nidzwetzki

[TOC]

0 Description

This file provides some CostEstimationClasses for the ExtRelation2Algebra. 

Mai 2012, JKN, First version of this file

*/

/*
0.1 Defines

*/

#ifndef COST_EST_EXT_RELATION_ALG_H
#define COST_EST_EXT_RELATION_ALG_H

/*
1.0 Prototyping

Local info for operator

*/

class ItHashJoinDInfo;

/*
1.1 The class ~ItHashJoinCostEstimation~ provides cost estimation
    capabilities for the operator itHashJoin

*/
class ItHashJoinCostEstimation : public CostEstimation 
{

public:
    ItHashJoinCostEstimation()
    {    
       pli = new ProgressLocalInfo();
    }    

/*
1.0 Free local datastructures

*/
  virtual ~ItHashJoinCostEstimation() {
     if(pli) {
        delete pli;
     }
  };

  virtual int requestProgress(Word* args, ProgressInfo* pRes, void* localInfo, 
    bool argsAvialable) {

     // no progress info available => cancel
     if(! argsAvialable) {
         cout << "No args available, cancel" << endl;
         return CANCEL;
     }

     if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2)) 
      {     
        pli->SetJoinSizes(p1, p2);
        
        pRes->Card = p1.Card  * p2.Card;
        pRes->Progress = p1.Progress;

        pRes->Time = p1.Time + p1.Card * p2.Time;
        pRes->CopySizes(pli);

        cout << "Send Progress info" << endl;
        return YIELD;
      }

     // default: send cancel
     return CANCEL;
  }


  void setStream1Exhausted(bool exhausted) {
      stream1Exhausted = exhausted;
  }

/*
1.1.0 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
  }

private:
  ProgressLocalInfo *pli;
  ProgressInfo p1, p2;
  bool stream1Exhausted;
};


#endif
