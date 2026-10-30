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

//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[Contents] [\tableofcontents]

1 Header File: Progress View

*/

#ifndef PROGRESSVIEW_H
#define PROGRESSVIEW_H

#include "NList.h"
#include "Messages.h"          // msg->Send(msgList)
#include "Profiles.h"          // SmiProfile::GetParameter
#include "Progress.h"
#include "CharTransform.h"
#include "SystemTables.h"     // ProgressInfoRel (SEC2PROGRESS)


class ProgressView
{
  public:

  ProgressView(ProgressInfoRel* rel) : currentProgress(0.0),
                   initialized(false), startClock(0), countedPoints(-1),
                   countedBase(0.0), msgList(), msg(0),
                   progressRel(rel), cmdNr(0),
                   PROGRESS_NORM(50), 
                   PROGTYPECARD(0), PROGTYPEPROG(0)
  {

    std::string parmFile = expandVar("$(SECONDO_CONFIG)");
    PROGRESS_NORM = SmiProfile::GetParameter("ProgressEstimation", 
      "ProgNorm", 50, parmFile);
    PROGTYPECARD = SmiProfile::GetParameter("ProgressEstimation", 
      "ProgTypeCard", 0, parmFile);
    PROGTYPEPROG = SmiProfile::GetParameter("ProgressEstimation", 
      "ProgTypeProg", 0, parmFile);
  }

  void
  InitProgressView()
  {

    if (PROGTYPEPROG)
    {
      initialized  = true;
       startClock  = clock();
      countedPoints = -1;
      msg = MessageCenter::GetInstance();

      msgList = NList(NList("progress"), 
                         NList(NList(-1), 
                         NList(PROGRESS_NORM)));
      msg->Send(nl,msgList.listExpr(),-1);
    }
  }

  // the command number the following progress updates are recorded under
  void
  SetCommandNr(int nr)
  {
    cmdNr = nr;
    countedBase = 0.0;
    if (progressRel) progressRel->dropOldCommands(cmdNr);
  }

  // ~evalShare~ is the part of the bar the operator tree's estimate fills,
  // when writing the result to the client follows (QueryProcessor::
  // UpdateProgress); the write fills the rest, see ModifyCountedProgress.
  // progress.Time already includes the write.
  void
  ModifyProgressView(ProgressInfo progress, double evalShare = 1.0)
  {
    if (PROGTYPEPROG)
    {
      if (!initialized) InitProgressView();

       currentProgress = progress.Progress;
       if(currentProgress < 0){
         currentProgress = 0;
       } 
       if(currentProgress>1){
         currentProgress= 1;
       }
       evalShare = std::min(1.0, std::max(0.0, evalShare));
       currentProgress *= evalShare;
       double bProgress = progress.BProgress;
       bProgress = std::min(1.0,std::max(0.0,bProgress));

       int cp = (int) (currentProgress*PROGRESS_NORM);
       if(cp<0){  // found invalid value
          return;
       }

       msgList = NList( NList("progress"),
              NList( NList(cp), 
                     NList(PROGRESS_NORM)));
        msg->Send(nl,msgList.listExpr(),-1);
       countedBase = evalShare;

      uint64_t clocks = clock() - startClock;
      // convert clocks to milliseconds
      clocks = (clocks * 1000) / CLOCKS_PER_SEC;

      // The estimates behind the bar, for clients that chart them (the
      // WebUI): (progressdetail (ms card time progress)). A message of its
      // own, because clients check (progress (k n)) for exactly two ints.
      msgList = NList( NList("progressdetail"),
              NList( NList((double) clocks),
                     NList(progress.Card),
                     NList(progress.Time),
                     NList(currentProgress)));
      msg->Send(nl,msgList.listExpr(),-1);

      // keep them for later analysis in the system table SEC2PROGRESS
      if (progressRel)
      {
        progressRel->append(new ProgressInfoTuple(cmdNr, (double) clocks,
                              progress.Card, progress.Time, currentProgress,
                              progress.BTime, bProgress), false);
      }
    }
  }


  // A phase whose share done is counted rather than estimated -- writing the
  // result to the client (Relation::Out, Relation::OutStreamed). It moves the
  // bar only: there is no estimate behind it to send as progressdetail or to
  // keep in SEC2PROGRESS, where it would read as a perfect one. It fills the
  // part of the bar the operator tree's estimate left over, so the bar goes on
  // from there instead of starting again; all of it when there was none.
  void
  ModifyCountedProgress(double share)
  {
    if (PROGTYPEPROG)
    {
      if (!initialized) InitProgressView();

      share = std::min(1.0, std::max(0.0, share));
      share = countedBase + (1.0 - countedBase) * share;
      int cp = (int) (share*PROGRESS_NORM);
      if (cp == countedPoints) {
        return;
      }
      countedPoints = cp;

      msgList = NList( NList("progress"),
              NList( NList(cp),
                     NList(PROGRESS_NORM)));
      msg->Send(nl,msgList.listExpr(),-1);
    }
  }


  // ~complete~ is false when the command did not get to the end -- a result
  // write that failed -- so the bar is closed without claiming 100 %.
  void
  FinishProgressView(bool complete = true)
  {
    if (PROGTYPEPROG && initialized)
    {
      initialized = false;

      // set progress to 100%
      if (complete)
      {
        msgList = NList( NList("progress"),
                             NList(NList(PROGRESS_NORM), 
                             NList(PROGRESS_NORM)));
        msg->Send(nl,msgList.listExpr(), -1);
      }
    

      // mark end of progress messages
      msgList = NList(NList("progress"), NList(NList(0), NList(-1)));
      msg->Send(nl,msgList.listExpr(),-1);
    }
  }

  private:
    double currentProgress;
    bool initialized;
    clock_t startClock;
    int countedPoints;  // the bar's last value from ModifyCountedProgress
    double countedBase; // where ModifyCountedProgress starts: the share the
                        // operator tree's estimate filled, 0 if there was none
    NList msgList;
    MessageCenter* msg;
    ProgressInfoRel* progressRel;
    int cmdNr;
    int PROGRESS_NORM;
    int PROGTYPECARD;
    int PROGTYPEPROG;
};

#endif
