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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Source File of the Spatiotemporal Pattern Algebra

June, 2009 Mahmoud Sakr

[TOC]

1 Overview

This source file essentially contains the necessary implementations for
evaluating the spatiotemporal pattern predicates (STP).

2 Defines and includes

*/
#include "TimeSequenceAlgebra.h"
#include "Symbols.h"

using namespace temporalalgebra;

namespace TSeq{
#define Min(X,Y) ((X) < (Y) ? (X) : (Y))

ListExpr
ConstTemporalConstTemporalTypeMapReal( ListExpr args )
{
  string argstr;
  if ( nl->ListLength( args ) == 4 )
  {
    ListExpr arg1 = nl->First( args ), //first argument
    arg2 = nl->Second( args ),//second argument
    arg3 = nl->Third( args ),//second argument
    map  = nl->Fourth( args ); //the comparison function
    if(nl->ListLength(map) == 4)
    {
      ListExpr map1 = nl->First( map ),
      map2 = nl->Second( map ),
      map3 = nl->Third( map ),
      map4 = nl->Fourth( map );
      if(nl->IsEqual( map1, Symbol::MAP() ) &&
        nl->IsEqual( map4, CcBool::BasicType() ))
      {
        if( (nl->IsEqual( arg1, MInt::BasicType() ) &&
            nl->IsEqual( arg2, MInt::BasicType() )  &&
            nl->IsEqual( map2, CcInt::BasicType()  ) &&
            nl->IsEqual( map3,  CcInt::BasicType() ))
            ||
            (nl->IsEqual( arg1, "mset" ) && nl->IsEqual( arg2, "mset" )  &&
            nl->IsEqual( map2, "intset"  ) && nl->IsEqual( map3,  "intset" )))
          return nl->SymbolAtom( CcReal::BasicType() );
      }
    }
    else
    {
      ErrorReporter::ReportError("similarity operator expects "
          "a (map data data bool) as last parameter, but got: " + argstr);
      return nl->SymbolAtom( Symbol::TYPEERROR() );
    }
  }
  nl->WriteToString(argstr, args);
  ErrorReporter::ReportError("typemap error in operator similarity. "
      "Operator  received: " + argstr);
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

template <class Mapping, class Unit, class Static>
double TWED(Mapping *arg1, Mapping *arg2, Instant* tMax, Word &map)
{
  return 0;
}

double TWEDMSet(MSet *arg1, MSet *arg2, Instant* tMax, Word &map)
{
  bool debugme= true;
  ArgVectorPointer funargs = qp->Argument(map.addr);
  int n= arg1->GetNoComponents(),m= arg2->GetNoComponents();
  vector< vector<double> > DTW(n, vector<double>(m, 10000));
  USetRef unit1(false), unit2(false);
  Word mapRes;
  int cost=0;

  DTW[0][0]= 0;

  for (int i = 1 ; i< n; ++i)
  {
    arg1->Get(i, unit1);
    USet utmp1(false);
    unit1.GetUnit(arg1->data, utmp1);
    (*funargs)[0] = SetWord(&(utmp1.constValue));
    for (int j = 1; j< m; ++j)
    {
      arg2->Get(j, unit2);
      USet utmp2(false);
      unit2.GetUnit(arg2->data, utmp2);
      (*funargs)[1] = SetWord(&(utmp2.constValue));

      if( ((unit1.timeInterval.start > unit2.timeInterval.start) &&
          (unit1.timeInterval.start - unit2.timeInterval.start < *tMax)) ||
          ((unit2.timeInterval.start >= unit1.timeInterval.start) &&
          (unit2.timeInterval.start - unit1.timeInterval.start < *tMax)))
      {
         qp->Request(map.addr, mapRes);
         CcBool* mr= static_cast<CcBool*>(mapRes.addr);
         cost =(mr->IsDefined() && mr->GetBoolval())? 0: 1;
      }
      else
        cost= 10000;
      DTW[i][j] = Min(Min(
            // insertion
            DTW[i-1][j] + 1 ,
            // deletion
            DTW[i][j-1] + 1),
            // match
            DTW[i-1][j-1] + cost) ;
      if(debugme)
      {
        cerr<<"\n Cost = "<< cost;
        cerr<<"\n DTW[i-1][j] + 1 = "<< DTW[i-1][j] + 1;
        cerr<<"\n DTW[i][j-1] + 1 = "<< DTW[i][j-1] + 1;
        cerr<<"\n DTW[i-1][j-1] + cost = "<< DTW[i-1][j-1] + cost;
        cerr<<"\n DTW[i][j] = "<< DTW[i][j];
      }
    }
  }
  return DTW[n-1][m-1];
}
template <class Mapping, class Unit, class Static>
int TWEDValueMap(
    Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  CcReal* res = static_cast<CcReal*>(result.addr);
  res->SetDefined(true);

  Mapping *arg1= static_cast<Mapping*>(args[0].addr),
          *arg2= static_cast<Mapping*>(args[1].addr);
  Instant *dMax= static_cast<Instant*>(args[2].addr);
  Word map= args[4];


  double dist= TWED<Mapping, Unit, Static>(arg1, arg2, dMax, map);
  res->Set(true, dist);
  return 0;
}

int TWEDMSetValueMap(
    Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  CcReal* res = static_cast<CcReal*>(result.addr);
  res->SetDefined(true);

  MSet *arg1= static_cast<MSet*>(args[0].addr),
          *arg2= static_cast<MSet*>(args[1].addr);
  Instant *dMax= static_cast<Instant*>(args[2].addr);
  Word map= args[3];


  double dist= TWEDMSet(arg1, arg2, dMax, map);
  res->Set(true, dist);
  return 0;
}

const string TWEDInfo  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>mset x mset x map(intset intset bool) -> real"
      "mint x mint x map(int int bool) -> real </text--->"
  "<text>twed(_, _, _)</text--->"
  "<text>measures the Time Wrap Edit Distance between two "
  "mapping(constunit(x))</text--->"
  "<text>query mi equal mi2</text--->"
  ") )";

ValueMapping TWEDValueMaps[] = { TWEDMSetValueMap,
                                TWEDValueMap<MInt, UInt, CcInt>};

int TWEDSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == "mset")
    return 0;

  if( nl->SymbolValue( arg1 ) == MInt::BasicType() )
    return 1;

  return -1; // This point should never be reached
}

Operator twed( "twed",
               TWEDInfo,
               2,
               TWEDValueMaps,
               TWEDSelect,
               ConstTemporalConstTemporalTypeMapReal );

class TimeSequenceAlgebra : public Algebra
{
public:
  TimeSequenceAlgebra() : Algebra()
  {

    AddOperator(&twed);
  }
  ~TimeSequenceAlgebra() {};
};


};

/*
5 Initialization

*/



extern "C"
Algebra*
InitializeTimeSequenceAlgebra( NestedList* nlRef,
    QueryProcessor* qpRef )
{
  // The C++ scope-operator :: must be used to qualify the full name
  return new TSeq::TimeSequenceAlgebra;
}
