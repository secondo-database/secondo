/*
----
This file is part of SECONDO.

Copyright (C) 2007, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title:  [{\Large \bf ]  [}]
//[->] [\ensuremath{\rightarrow}]

[1] Secondo SimulationAlgebra


April 04, 2007 C. Duentgen created the algebra

\begin{center}
\footnotesize
\tableofcontents
\end{center}


1 Overview

This algebra provides operators to create movement data for simulated vehicles.


2 Includes

We have to include some GSL headers and the GSLAlgebra.

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "DateTime.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <math.h>
#include <unistd.h>
#include <errno.h>

#include <NList.h>

#include "RelationAlgebra.h"
#include "TemporalAlgebra.h"
#include "GSLAlgebra.h"

#ifndef SECONDO_WIN32
#define HAVE_INLINE
#endif
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>



/*
3. Static Objects and Parameter Settings

This algebra uses its own random number generator to stay independent from
other random generator states.

You can change most of the parameters at runtime using operators
~sim\_set\_rng~, ~sim\_set\_dest\_params~ and  ~sim\_set\_event\_params~.

*/

// setting up the RNG
unsigned long simRngSeed = 0; // 0 = use standard random seed
int           simRngType = 0; // index of random number generator to use
static GslRandomgen simRNG( simRngType, simRngSeed );

// parameter set for waiting at destination node
double sim_dest_param_mu = 15000; // mean for exponential [ms]
                                  //   distribution [ms]
double sim_dest_param_ss =  0.33; // probabilities for forced stops
double sim_dest_param_sm =  0.66; //   at crossings...
double sim_dest_param_sf =  1.00;
double sim_dest_param_ms =  0.33;
double sim_dest_param_mm =  0.50;
double sim_dest_param_mf =  0.66;
double sim_dest_param_fs =  0.05;
double sim_dest_param_fm =  0.33;
double sim_dest_param_ff =  0.10;

// maximum allowed velocities for different street types:
double sim_vmax_sidestreet = 30.0;
double sim_vmax_mainstreet = 50.0;
double sim_vmax_freeway    = 70.0;

// parameter set for subsegment events
double sim_event_param_subsegmentlength = 5.0; // maximum length of subsections
                                               //   in [m]
double sim_event_param_propconst        = 1.0; // constant c of proportionality
                                               //   p(event) = c/v_max 
double sim_event_param_probstop         = 0.1; // prob.(event == forced stop)
                                               //   balanced by deceleration
double sim_event_param_acceleration     =12.0; // acceleration constant:
                                               //   increase step [m/s]
                                               //   within a single subsegment

// further parameters
double sim_startpoint_tolerance = 0.5; // tolerance for search of a line's
                                       // starting point. Set to negative value
                                       // for exact matching.

/*
4. General Typemapping Functions

*/

// real^N -> bool
template<int N>
    ListExpr sim_realN2bool_TM ( ListExpr args )
{
  ListExpr arg1, args2 = args;
  std::ostringstream oss;
  oss << N;
  if ( nl->ListLength( args ) == N )
  {
    for(int i=0; i<N; i++)
    {
      arg1 = nl->First( args2 );
      args2 = nl->Rest( args );
      if ( !(nl->AtomType( arg1 ) == SymbolType) || 
             !(nl->SymbolValue( arg1 ) == "real") 
         )
      {
        ErrorReporter::ReportError("SimulationAlgebra::sim_set_dest_params "
            "expects (real)^" + oss.str() + "." );
        return (nl->SymbolAtom( "typeerror" ));
      }
    }
    return (nl->SymbolAtom( "bool" ));
  }
  ErrorReporter::ReportError("SimulationAlgebra::sim_set_dest_params "
      "expects a list of length " + oss.str() + ".");
  return (nl->SymbolAtom( "typeerror" ));
}


/*
5. Operators

We will define the following operators:

----

 sim_set_rng:                int x int -> bool
 sim_set_dest_params:        real x real x real x real x real x real
                                         x real x real x real x real -> bool
 sim_set_event_params:       real x real x real x real -> bool
 sim_create_trip:            stream(tuple( ... (a_m line) ... (a_n real) ... ))
                                  x instant -> mpoint

----

*/

/*
5.1 Operator ~sim\_set\_rng~

Initialize the RNG with the passed type and random seed

*/

// int x int -> bool
ListExpr sim_set_rng_TM ( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( nl->AtomType( arg1 ) == SymbolType && 
         nl->SymbolValue( arg1 ) == "int"  && 
         nl->AtomType( arg2 ) == SymbolType && 
         nl->SymbolValue( arg2 ) == "int" )
          
    {
      return (nl->SymbolAtom( "bool" ));
    }
  }
  ErrorReporter::
      ReportError("SimulationAlgebra: sim_set_rng expected int x int");
  return (nl->SymbolAtom( "typeerror" ));
}

int sim_set_rng_VM ( Word* args, Word& result, 
                        int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool *res = ((CcBool*)result.addr);
  CcInt *arg1 = ((CcInt*)args[0].addr);
  CcInt *arg2 = ((CcInt*)args[1].addr);
  if ( arg1->IsDefined() && arg2->IsDefined() )
  {
    // overflow save implementation
    long  type = arg1->GetIntval();
    unsigned long seed = arg2->GetIntval();
    if( type >= 0 && type < GslRandomgen::rngType_getNoOfGenerators() )
    {
      simRNG = GslRandomgen(type, seed);
      res->Set(true, true);
    }
    else
      res->Set(true, false);
  }
  else
    res->Set(true, false);
  return (0);
}

const string sim_set_rng_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>   int x int -> bool</text--->"
    "<text>sim_set_rng( Type , Seed )</text--->"
    "<text>Initialize the random number generator to be "
    "of type 'Type' and use a random seed 'Seed'.</text--->"
    "<text>query sim_set_rng( 5, 54677 )</text--->"
    ") )";

Operator sim_set_rng( 
    "sim_set_rng",
    sim_set_rng_Spec,
    sim_set_rng_VM,
    Operator::SimpleSelect,
    sim_set_rng_TM) ;

/*
5.2 Operator ~sim\_set\_dest\_params~

Set parameters for stops at the destination node

*/

int sim_set_dest_params_VM ( Word* args, Word& result, 
                     int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool *res = ((CcBool*)result.addr);
  CcReal* cArgs[10];
  double dArgs[10];
  for(int i=0; i<10; i++)
  {
    cArgs[i] = (CcReal*) args[i].addr;
    if ( !cArgs[i]->IsDefined() )
    {
      res->Set(true, false);
      ErrorReporter::ReportError("SimulationAlgebra::sim_set_dest_params "
          "expected defined param.");
      return 0;
    }
    dArgs[i] = cArgs[i]->GetRealval();
    if ( i>0 && (dArgs[i] < 0.0 || dArgs[i] > 1.0 ) )
    {
      res->Set(true, false);
      ErrorReporter::ReportError("SimulationAlgebra::sim_set_dest_params: "
          "Probability out of bounds");
      return 0;
    }
  }
  sim_dest_param_mu = dArgs[0]/86400000.0;
  sim_dest_param_ss = dArgs[1]; 
  sim_dest_param_sm = dArgs[2];
  sim_dest_param_sf = dArgs[3];
  sim_dest_param_ms = dArgs[4];
  sim_dest_param_mm = dArgs[5];
  sim_dest_param_mf = dArgs[6];
  sim_dest_param_fs = dArgs[7];
  sim_dest_param_fm = dArgs[8];
  sim_dest_param_ff = dArgs[9];
  res->Set(true, true);
  return 0;
}

const string sim_set_dest_params_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> real x real x real x real x real x real x "
    " real x real x real x real -> bool</text--->"
    "<text>sim_set_dest_params( ExpMu, SS, SM, SF, MS, MM, MF, FS, FM, FF)"
    "</text--->"
    "<text>Set the parameters for stops at destination nodes: Set mean of "
    "exponential distribution of waiting times to 'ExpMu' (in milliseconds). "
    "Set probabilities "
    "for forced stops when transitions between street types. 'XY' means "
    "transition X -> Y, where S= small street, M= main street F= freeway. "
    "Observe 0.0 <= p <= 1.0 for all probabilities p. Returns 'TRUE', iff "
    "the parameters have been set correctly.</text--->"
    "<text>query sim_set_dest_params( 32.2, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, "
    "0.7, 0.8, 0.9 )</text--->"
    ") )";

Operator sim_set_dest_params( 
    "sim_set_dest_params",
    sim_set_dest_params_Spec,
    sim_set_dest_params_VM,
    Operator::SimpleSelect,
    sim_realN2bool_TM<10>) ;

/*
5.3 Operator ~sim\_set\_event\_params~

Set parameters for enroute-event generation.

*/

int sim_set_event_params_VM ( Word* args, Word& result, 
                             int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool *res = ((CcBool*)result.addr);
  CcReal* cArgs[4];
  double dArgs[4];
  for(int i=0; i<4; i++)
  {
    cArgs[i] = (CcReal*) args[i].addr;
    if ( !cArgs[i]->IsDefined() )
    {
      res->Set(true, false);
      ErrorReporter::ReportError("SimulationAlgebra::sim_set_event_params "
          "expected defined param.");
      return 0;
    }
    dArgs[i] = cArgs[i]->GetRealval();
    if ( (i==0 || i==4) && dArgs[i] <= 0.0 )
    {
      res->Set(true, false);
      ErrorReporter::ReportError("SimulationAlgebra::sim_set_event_params: "
          "1st parameter 'Length' must be positive!");
      return 0;
    }
    if ( i==1 && dArgs[i] < 0.0 )
    {
      res->Set(true, false);
      ErrorReporter::ReportError("SimulationAlgebra::sim_set_event_params: "
          "2nd parameter 'Proportionality constant' must be <= 0.0!");
      return 0;
    }
    if ( i==2 && (dArgs[i] < 0.0 || dArgs[i] > 1.0 ) )
    {
      res->Set(true, false);
      ErrorReporter::ReportError("SimulationAlgebra::sim_set_event_params: "
          "3rd parameter 'Probability' out of bounds!");
      return 0;
    }
  }
  sim_event_param_subsegmentlength = dArgs[0]; // maximum length of subsections
  sim_event_param_propconst = dArgs[1];        // constant of proportionality
  sim_event_param_probstop  = dArgs[2];        // prob. for event = forced stop
  sim_event_param_acceleration = dArgs[3];     // acceleration rate
                                    // [km/h/sim_event_param_subsegmentlength]
  res->Set(true, true);
  return 0;
}

const string sim_set_event_params_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> real x real x real x real -> bool</text--->"
    "<text>sim_set_event_params( Length, C, P, Acc)"
    "</text--->"
    "<text>Set the parameters for enroute-events: Routes will be divided into "
    "subsegments of maximum length 'Length'. The probability of an event is "
    "proprtional to C / Vmax. The probability for an event being a forced "
    "stop is given by 0.0 <= P <= 1.0 (the balance, 1-P, is meant to trigger "
    "deceleration events). Acceleration is set to 'Acc'."
    "Returns 'TRUE', iff the parameters have been set correctly.</text--->"
    "<text>sim_set_event_params( 32.2, 1.0, 0.2, 15.3 )</text--->"
    ") )";

Operator sim_set_event_params( 
    "sim_set_event_params",
    sim_set_event_params_Spec,
    sim_set_event_params_VM,
    Operator::SimpleSelect,
    sim_realN2bool_TM<4>) ;


/*
5.3 Operator ~sim\_create\_trip~

Create a moving point from a stream of lines and a startung instant.

*/

ListExpr sim_create_trip_TM ( ListExpr args )
{
  int len = nl->ListLength(args);
  if(len != 5 && len != 6 )
  {
    ErrorReporter::ReportError("five or six arguments expected");
    return nl->SymbolAtom("typeerror");
  }
  // extract the attribute names
  ListExpr a1list = nl->Second(args);
  ListExpr a2list = nl->Third(args);

  if(nl->AtomType(a1list)!=SymbolType)
  {
    ErrorReporter::ReportError("the second argument has to be a symbol");
    return nl->SymbolAtom("typeerror");
  }
  if(nl->AtomType(a2list)!=SymbolType)
  {
    ErrorReporter::ReportError("the third argument has to be a symbol");
    return nl->SymbolAtom("typeerror");
  }
  string a1 = nl->SymbolValue(a1list);
  string a2 = nl->SymbolValue(a2list);

  string restype="";
  int a1index = -1;
  int a2index = -1;

  ListExpr stype = nl->First(args);
  if(nl->AtomType(stype)!=NoAtom)
  {
    ErrorReporter::ReportError("stream(tuple(...))"
        " expected as the first argument");
    return nl->SymbolAtom("typeerror");
  }

  if((nl->ListLength(stype)!=2) || 
      (!nl->IsEqual(nl->First(stype),"stream" )))
  {
    ErrorReporter::ReportError("stream(tuple(...))"
        " expected as the first argument");
    return nl->SymbolAtom("typeerror");
  }

  ListExpr ttype = nl->Second(stype);

  if((nl->ListLength(ttype)!=2) || 
      (!nl->IsEqual(nl->First(ttype),"tuple" )))
  {
    ErrorReporter::ReportError("stream(tuple(...))"
        " expected as the first argument");
    return nl->SymbolAtom("typeerror");
  }

  ListExpr attributes = nl->Second(ttype);
  if(nl->AtomType(attributes)!=NoAtom)
  {
    ErrorReporter::ReportError("invalid tuple type");
    return nl->SymbolAtom("typeerror");
  }
  int pos = 0;
  while(!nl->IsEmpty(attributes))
  {
    ListExpr attr = nl->First(attributes);
    if( (nl->AtomType(attr)!=NoAtom) ||
         (nl->ListLength(attr)!=2))
    {
      ErrorReporter::ReportError("invalid tuple type");
      return nl->SymbolAtom("typeerror");
    }
    ListExpr anl = nl->First(attr);
    ListExpr atl = nl->Second(attr);
    if( (nl->AtomType(anl)!=SymbolType) ||
         (nl->AtomType(atl)!=SymbolType))
    {
      ErrorReporter::ReportError("invalid tuple type");
      return nl->SymbolAtom("typeerror");
    }

    string aname = nl->SymbolValue(anl);
    if(aname==a1)
    {
      if(a1index>=0)
      {
        ErrorReporter::ReportError("attr name occurs twice");
        return nl->SymbolAtom("typeerror");
      }
      if(!nl->IsEqual(atl,"line"))
      {
        ErrorReporter::
            ReportError("first attr (Trajectory) has to be of type 'line'");
        return nl->SymbolAtom("typeerror");
      }
      a1index = pos;
    }

    if(aname==a2)
    {
      if(a2index >= 0)
      {
        ErrorReporter::ReportError("attr name occurs twice");
        return nl->SymbolAtom("typeerror");
      }
      string a2type = nl->SymbolValue(atl);
      if(a2type=="real")
      {
        restype = "mpoint";
      } 
      else 
      {
        ErrorReporter::
            ReportError("seocond attr (maximimum velocity) has to be of"
            " type 'real'");
        return nl->SymbolAtom("typeerror");
      }
      a2index = pos;
    }
    pos++; 
    attributes = nl->Rest(attributes);
  }

  if(a1index<0)
  {
    ErrorReporter::ReportError("first attr name does"
        " not occur in the typle");
    return nl->SymbolAtom("typeerror");
  }

  if(a2index<0)
  {
    ErrorReporter::ReportError("second attr name does"
        " not occur in the typle");
    return nl->SymbolAtom("typeerror");
  }

  ListExpr arg5 = nl->Fifth(args);
  if ( !(nl->AtomType( arg5 ) == SymbolType) ||
         !(nl->SymbolValue( arg5 ) == "point")
     )
  {
    ErrorReporter::ReportError("fifth argument must be of type 'point'" );
    return (nl->SymbolAtom( "typeerror" ));
  }

  
  ListExpr ind;

  if (len == 6 )
  {// explicit starting velocity
    ListExpr arg6 = nl->Sixth(args);
    if ( !(nl->AtomType( arg6 ) == SymbolType) ||
           !(nl->SymbolValue( arg6 ) == "real")
       )
    {
      ErrorReporter::
          ReportError("optional sixth argument must be of type 'real'" );
      return (nl->SymbolAtom( "typeerror" ));
    }
    ind = nl->TwoElemList(nl->IntAtom(a1index),
                          nl->IntAtom(a2index));
  }
  else
  {// all is correct, starting velocity == 0.0
    ind = nl->ThreeElemList(nl->RealAtom(0.0),
                            nl->IntAtom(a1index),
                            nl->IntAtom(a2index));
  }

  return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                           ind,
                           nl->SymbolAtom(restype));
}


struct Subsegment
{
  Point start;
  Point end;
};



int sim_create_trip_VM ( Word* args, Word& result, 
                         int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MPoint*        res = ((MPoint*)result.addr);
  CcInt*  cLineIndex = (CcInt*) args[6].addr;
  CcInt*  cVmaxIndex = (CcInt*) args[7].addr;
  Instant* instStart = (Instant*) args[3].addr;
  Point*  pointStart = (Point*) args[4].addr;
  CcReal*    cVstart = (CcReal*) args[5].addr;
  long    tuplesReceived = 0;
  long    tuplesAccepted = 0;

  cout << "sim_create_trip_VM called with" << endl;
  cout << "cLineIndex = \t"; cLineIndex->Print(cout); cout << endl;
  cout << "cVmaxIndex = \t"; cVmaxIndex->Print(cout); cout << endl;
  cout << "instStart = \t" << instStart->ToString() << endl;
  cout << "pointStart = \t"; pointStart->Print(cout); cout << endl;
  cout << "cVstart = \t"; cVstart->Print(cout); cout << endl;

  res->Clear();
  res->SetDefined( true );
  res->StartBulkLoad();

  if( instStart->IsDefined() && pointStart->IsDefined() &&
      cVstart->IsDefined() &&
      cLineIndex->IsDefined() && cVmaxIndex->IsDefined() )
  {
    cout << "sim_create_trip_VM: Arguments defined." << endl;
    vector<Subsegment> subsegments(0);
    int          lineIndex = cLineIndex->GetIntval();
    int          VmaxIndex = cVmaxIndex->GetIntval();
    Instant    currentInst = *instStart;
    double     currentVmax = 0.0;
    Instant    currentTime = *instStart;
    Point  currentPosition = *pointStart;
    double    currentSpeed = cVstart->GetRealval();
    double       localVmax = 0.0;
    double    lastMaxSpeed = -1.0;
    double       lastAlpha = 0.0;
    bool     stopAfterThis = false;
    Point       startPoint(true,0,0);
    Point         endPoint(true,0,0);
    DateTime dummyDuration(0,0,durationtype);
    Line*      currentLine = NULL;
    Word      wActualTuple;
    qp->Open(args[0].addr);
    qp->Request(args[0].addr, wActualTuple);
    while (qp->Received(args[0].addr))
    {
      tuplesReceived++;
      cout << "sim_create_trip_VM: Received tuple "
          << tuplesReceived
          << ":" << endl;
      // speed == 0.0 signals to wait for a random duration before the
      // movement may be continued!
      Tuple* tuple         = (Tuple*) (wActualTuple.addr);
      currentLine          = (Line*)  (tuple->GetAttribute(lineIndex));
      CcReal *CcurrentVmax = (CcReal*)(tuple->GetAttribute(VmaxIndex));
      cout << " currentLine = "; currentLine->Print(cout); cout << endl;

      // search for the first segment (starts with point currentPosition)
      assert( currentPosition.IsDefined() );
      if( currentLine->IsDefined()  &&
          CcurrentVmax->IsDefined() &&
          !currentLine->IsEmpty()   &&
          currentLine->SelectInitialSegment(currentPosition,
                                            sim_startpoint_tolerance) )
      {// all args defined and current position found in currentLine
        tuplesAccepted++;
        cout << "New Tuple OK!" << tuplesAccepted << endl;
        currentVmax = CcurrentVmax->GetRealval();
        cout << " currentVmax = " << currentVmax << endl;
        cout << " current position (= " ;
        currentPosition.Print(cout);
        cout << " ) found in currentLine" << endl;
        startPoint = currentPosition;
        endPoint = currentPosition;
        while( currentLine->getWaypoint(endPoint) )
        { // for each line segment:
          assert( endPoint.IsDefined() );
          Subsegment s;
          double l =  startPoint.Distance(endPoint);
          double incrX = sim_event_param_subsegmentlength
                * (endPoint.GetX() - startPoint.GetX()) / l;
          double incrY = sim_event_param_subsegmentlength
                * (endPoint.GetY() - startPoint.GetY()) / l;
          if(AlmostEqual(incrX, 0.0) && AlmostEqual(incrY, 0.0))
          {
            cout << "  X- and Y-Increment == 0.0! -> BREAK" << endl;
            break;
          }
          Point interimStart = startPoint;
          Point interimEnd   = startPoint;
          while( interimEnd.Distance(endPoint) >=
                 sim_event_param_subsegmentlength )
          { // divide the remaining segment into subsegments
            interimEnd.Translate(incrX,incrY);
            s.start    = interimStart;
            s.end      = interimEnd;
            subsegments.push_back(s);
            interimStart = interimEnd;
          }
          // add the last subsegment
          s.start    = interimStart;
          s.end      = endPoint;
          assert( endPoint.IsDefined() );
          subsegments.push_back(s);
          startPoint = endPoint;
          currentLine->SelectSubsequentSegment();
        }
        // iterate vector to determine the speed at the end of each subsegment:
        for(unsigned int i=0 ; i<subsegments.size(); i++)
        {
          localVmax = currentVmax; // base for further calculations
          if ( lastMaxSpeed >= 0.0 )
          { // Special Case:
            // Handle last subsegment of preceeding line:
            cout << "\tAt Crossing: ";
            localVmax = lastMaxSpeed; // recall lastMaxSpeed for
                                      // reenqueued subsegment
            // check whether to stop at the crossing
            double pWait = 0.0;
            if ( AlmostEqual(lastMaxSpeed,sim_vmax_sidestreet) )
            {
              if ( AlmostEqual(currentVmax,sim_vmax_sidestreet) )
              { cout << " S->S "; pWait = sim_dest_param_ss;}
              else if ( AlmostEqual(currentVmax,sim_vmax_mainstreet) )
              { cout << " S->M "; pWait = sim_dest_param_sm;}
              else if ( AlmostEqual(currentVmax,sim_vmax_freeway) )
              { cout << " S->F "; pWait = sim_dest_param_sf;}
              else
              { cout << " S->??? "; pWait = 0.0;}
            }
            else if ( AlmostEqual(lastMaxSpeed,sim_vmax_mainstreet) )
            {
              if ( AlmostEqual(currentVmax,sim_vmax_sidestreet) )
              { cout << " M->S "; pWait = sim_dest_param_ms;}
              else if ( AlmostEqual(currentVmax,sim_vmax_mainstreet) )
              { cout << " M->M "; pWait = sim_dest_param_mm;}
              else if ( AlmostEqual(currentVmax,sim_vmax_freeway) )
              { cout << " M->F "; pWait = sim_dest_param_mf;}
              else
              { cout << " M->? "; pWait = 0.0;}
            }
            else if ( AlmostEqual(lastMaxSpeed,sim_vmax_freeway) )
            {
              if ( AlmostEqual(currentVmax,sim_vmax_sidestreet) )
              { cout << " F->S "; pWait = sim_dest_param_fs;}
              else if ( AlmostEqual(currentVmax,sim_vmax_mainstreet) )
              { cout << " F->M "; pWait = sim_dest_param_fm;}
              else if ( AlmostEqual(currentVmax,sim_vmax_freeway) )
              { cout << " F->F "; pWait = sim_dest_param_ff;}
              else
              { cout << " F->? "; pWait = 0.0; }
            }
            else
            {
              cout << " ?->? ";
              pWait = 0.0;
            }
            if(simRNG.NextReal() <= pWait)
            { // Force waiting at the crossing...
              cout << "\tStopping (after next)."<< endl;
              stopAfterThis = true;
            }
            else
            {
              cout << "\tPassing (after next)."<< endl;
            }
            lastMaxSpeed = -1.0; // avoid double waits
          }
          if ( AlmostEqual(currentSpeed, 0.0) )
          { // speed == 0.0 indicates, that we have to wait,
            // before we may continue the voyage:
            // Determine waiting duration using exponential distribution:
            double waittime =
                  gsl_ran_exponential(simRNG.GetGenerator(),
                                      sim_dest_param_mu/86400000.0);
            dummyDuration.ReadFrom(waittime);
            if( dummyDuration > DateTime(0,0,durationtype) )
            {
              const Interval<Instant>
                  interval( currentInst,
                            currentInst + dummyDuration,
                            true,
                            false );
              UPoint up( interval, subsegments[i].start, subsegments[i].start);
              res->MergeAdd(up);
              currentInst += dummyDuration;
              cout << "\tWaiting for " 
                   << dummyDuration.GetAllMilliSeconds()/1000 << " sec."
                   << endl;
            }
          }
          if (i<subsegments.size()-1)
          { // This is not the last subsegment for this line
            if( simRNG.NextReal() <= (sim_event_param_propconst/currentVmax) )
            {// An event occurrs
              if( simRNG.NextReal() <= (sim_event_param_probstop) )
              { // forced stop after this sub-segment
                cout << "\tEvent: Forced stop!";
                stopAfterThis = true;
              }
              else
              { // forced deceleration. Use binomial distrib.
                // to determine amount of speed loss
                localVmax =
                    localVmax * gsl_ran_binomial(simRNG.GetGenerator(),
                                                0.5,
                                                20)
                    /20.0;
                localVmax = MAX(localVmax, 1.0);
                cout << "\tEvent: Decelerate to "
                    << localVmax;
              }
            }
            else
            { // no event: accelerate up to localVmax
              localVmax =
                  MIN(currentSpeed+sim_event_param_acceleration, currentVmax);
              cout << "\tNo event: Accelerate to " << localVmax;
            }
            // calculate steepness of curves
            double alpha = subsegments[i].end.Direction(subsegments[i+1].end);
            double curveMax =
                  (1.0-( fmod(fabs(lastAlpha-alpha), 180.0) )/180.0)
                  * currentVmax;
            localVmax = MIN( localVmax, curveMax );
            lastAlpha = alpha;
            currentSpeed = localVmax;
            cout << "\t alpha=" << alpha
                << " curveMax=" << curveMax
                << " currentSpeed = " << currentSpeed << endl;
          }
          else if (i == subsegments.size()-1 )
          { // This is the last subsegment. Delete all subsegments,
            // but the last one
            Subsegment lastSubSeg = subsegments[i];
            subsegments.clear();
            subsegments.push_back(lastSubSeg);
            lastMaxSpeed = currentVmax; // prepare special treatment
                                        // of this subsegment
            currentPosition = subsegments.back().end; // required to find next
                                            // subsegment within the next line
            break; // this exits the for()-loop
          }
          // Create a unit for the current subsegment
          double dist = subsegments[i].start.Distance(subsegments[i].end);
          dummyDuration.ReadFrom(dist/currentSpeed/24000L);
          // ^^^^ time used to travel the subsegment t=s/v ^^^
          const Interval<Instant>
                interval( currentInst,
                          currentInst + dummyDuration,
                          true,
                          false );
          UPoint up( interval, subsegments[i].start, subsegments[i].end);
          res->MergeAdd(up);
          currentInst += dummyDuration;
          currentPosition = subsegments[i].end;
          if (stopAfterThis)
          {
            currentSpeed = 0.0;
            stopAfterThis = false;
          }
        } // endfor(unsigned int i=0 ; i<subsegments.size(); i++)
        currentPosition.Print(cout);
        cout << endl;
        assert( currentPosition.IsDefined() );
      } // endif( currentLine->SelectInitialSegment(currentPosition) )
      else
      { // undef/empty/complex/cyclic line or undef vmax or cuppentPos
        //            not found: do nothing
        cout << "current position (= " ;
        currentPosition.Print(cout);
        cout << " ) NOT found in currentLine" << endl;
      }
      tuple->DeleteIfAllowed();
      qp->Request(args[0].addr, wActualTuple);
    }
    // create and append the final unit
    if( subsegments.size() == 1 )
    { // there is a subsegment left. Nothing happens here.
      currentSpeed = MAX(currentSpeed, 5.0);
      double dist = subsegments[0].start.Distance(subsegments[0].end);
      dummyDuration.ReadFrom(dist/currentSpeed/24000); // time used to travel
                                                     // the subsegment t=s/v
      const Interval<Instant>
          interval( currentInst,
                    currentInst + dummyDuration,
                    true,
                    false );
      UPoint up( interval, subsegments[0].start, subsegments[0].end);
      res->MergeAdd(up);
      currentInst += dummyDuration;
      currentPosition = subsegments.back().end; // set currentPosition to
    } // endwhile (qp->Received(args[0].addr))
    else
    { // ( subsegments.size() != 1 )
      cout << "sim_create_trip_VM: Something's wrong: subsegments.size() = "
           << subsegments.size() << "." << endl;
    }
  }
  else
  { // some undef argument:
    cout << "sim_create_trip_VM: undefined Argument." << endl;
    res->SetDefined( false );
  }
  res->EndBulkLoad();
  qp->Close(args[0].addr);
  cout << "sim_create_trip_VM: Finished!" << endl;
  cout << "  tuplesReceived = " << tuplesReceived << endl;
  cout << "  tuplesAccepted = " << tuplesAccepted << endl;
  cout << "  units created  = " << res->GetNoComponents() << endl;
  cout << *res << endl;
  return 0;
}

const string sim_create_trip_Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>stream(tuple(a1: t1) ... (an,tn) x ai x aj x instant x point -> "
    "mpoint, "
    "for ti = line, tj = real, tk = point</text--->"
    "<text>_ sim_create_trip [LineAttr, VmaxAttr, StartInst, StartPoint ] \n"
    "<_ sim_create_trip [LineAttr, VmaxAttr, StartInst, StartPoint, Vstart ] \n"
    "</text--->"
    "<text>Creates a mpoint value representing a simulated vehicle, "
    "starting at instant 'StartInst' and 'StartPoint' and moving along a "
    "trajectory formed by the stream of lines received from the stream "
    "argument. The stream contains tuples describing subsequent parts of the "
    "trajectory and the maximum allowed speed for that street section. The "
    "velocity of the vehicle reflects street topology and random events. "
    "The sixth argument 'Vstart' is optional and sets the vehicle's initial "
    "velocity. If omitted, Vmax is set to 0.0. </text--->"
    "<text>query _ sim_create_trip[ _ ]</text--->"
    ") )";

Operator sim_create_trip( 
    "sim_create_trip",
    sim_create_trip_Spec,
    sim_create_trip_VM,
    Operator::SimpleSelect,
    sim_create_trip_TM) ;

/*
6 Class ~SimulationAlgebra~

The last steps in adding an algebra to the Secondo system are

  * Associating value mapping functions with their operators

  * ``Bunching'' all
type constructors and operators in one instance of class ~Algebra~.

Therefore, a new subclass ~GSLAlgebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all type constructors and operators are registered at the actual
algebra.

After declaring the new class, its only instance ~ccalgebra1~ is defined.

*/


class SimulationAlgebra : public Algebra
{
 public:
  SimulationAlgebra() : Algebra()
    {
      AddOperator( &sim_set_rng );
      AddOperator( &sim_set_event_params );
      AddOperator( &sim_set_dest_params );
      AddOperator( &sim_create_trip );
    }
  ~SimulationAlgebra() {};

  private:
    bool initialized;
    const gsl_rng_type **simalg_randomgeneratorTable;
    // number of generator types available:
    int sim_randomgeneratorTableSize;
    int sim_rng_defaultIndex;
};


SimulationAlgebra simulationAlgebra;

/*
7 Initialization

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
InitializeSimulationAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&simulationAlgebra);
}
