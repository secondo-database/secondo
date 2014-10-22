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

[1]  Implementation of the Graph Algebra

February 2007, G. Barz, M. Stein, A. Ruloffs, A. Martin


[TOC]

1 Overview

This implementation file essentially contains the implementation of the
following operators:

  * ~constGraph~

  * ~constGraphPoints~

  * ~vertices~

  * ~edges~

  * ~equal~

  * ~partOf~

  * ~connectedComponents~

  * ~merge~

  * ~maxDegree~

  * ~minDegree~

  * ~theVertex~

  * ~shortestPath~

  * ~circle~

  * ~key~

  * ~pos~

  * ~source~

  * ~target~

  * ~cost~

Enter

----list algebra GraphAlgebra
----

in SECONDO to get more informations about these operators.


2 Defines and includes

*/

#include "GraphAlgebra.h"
#include "RelationAlgebra.h"
#include "ListUtils.h"
#include "Symbols.h"
#include "PlaceNodesHelper.h"

/*
4 Creating Operators

4.1 Type Mapping Functions

Checks whether the correct argument types are supplied for an operator; if so,
returns a list expression for the result type, otherwise the symbol
~typeerror~.

*/

/*
4.1.1 Type mapping function for operator ~theVertex~

graph x int $\to$ vertex

*/
ListExpr theVertexMap ( ListExpr args ) {

   if (nl->ListLength(args) == 2) {

     ListExpr arg1 = nl->First(args);
     ListExpr arg2 = nl->Second(args);

     if (nl->IsEqual(arg1, Graph::BasicType()) &&
         nl->IsEqual(arg2, CcInt::BasicType()))
       return nl->SymbolAtom(Vertex::BasicType());
     if ((nl->AtomType(arg1) == SymbolType) &&
         (nl->AtomType(arg2) == SymbolType))
       ErrorReporter::ReportError(
                "Type mapping function got parameters of type "+
                nl->ToString(arg1)+" and "+nl->ToString(arg2));
     else
       ErrorReporter::ReportError(
                "Type mapping function got wrong types as parameters.");
   }
   else
     ErrorReporter::ReportError(
                "Type mapping function got a parameter of length != 2.");

   return nl->SymbolAtom(Symbol::TYPEERROR());
}


/*
4.1.2 Type mapping function for operator ~maxDegree~ and ~minDegree~

graph $\to$ int

*/
ListExpr minMaxDegreeMap ( ListExpr args ) {

   if (nl->ListLength(args) == 2) {

     ListExpr arg1 = nl->First(args);
     ListExpr arg2 = nl->Second(args);

     if (nl->IsEqual(arg1, Graph::BasicType()) &&
         nl->IsEqual(arg2, CcBool::BasicType()))
       return nl->SymbolAtom(CcInt::BasicType());
     if ((nl->AtomType(arg1) == SymbolType) &&
         (nl->AtomType(arg2) == SymbolType))
       ErrorReporter::ReportError(
              "Type mapping function got parameters of type "+
              nl->ToString(arg1)+" and "+nl->ToString(arg2));
     else
       ErrorReporter::ReportError(
              "Type mapping function got wrong types as parameters.");
   }
   else
     ErrorReporter::ReportError(
              "Type mapping function got a parameter of length != 2.");

   return nl->SymbolAtom(Symbol::TYPEERROR());
}


/*
4.1.3 Type mapping function for operator ~connectedComponents~

graph $\to$ stream(tuple([Graph: graph]))

*/
ListExpr connectedCompMap ( ListExpr args ) {

   if (nl->ListLength(args) == 1) {

     ListExpr arg1 = nl->First(args);

     if (nl->IsEqual(arg1, Graph::BasicType()))
       return nl->TwoElemList(
                   nl->SymbolAtom(Symbol::STREAM()),
                   nl->TwoElemList(
                        nl->SymbolAtom(Tuple::BasicType()),
                        nl->OneElemList(
                             nl->TwoElemList(
                                  nl->SymbolAtom("Graph"),
                                  nl->SymbolAtom(Graph::BasicType())
                             )
                        )
                   )
              );

     if ((nl->AtomType(arg1) == SymbolType))
       ErrorReporter::ReportError(
             "Type mapping function got parameters of type "+
              nl->SymbolValue(arg1));
     else
       ErrorReporter::ReportError(
             "Type mapping function got wrong type as parameter.");
   }
   else
     ErrorReporter::ReportError(
             "Type mapping function got a parameter of length != 1.");

   return nl->SymbolAtom(Symbol::TYPEERROR());
}


/*
4.1.4 Type mapping function for operator ~shortestPath~

graph x vertex x real $\to$ graph

*/
ListExpr shortestPathMap ( ListExpr args ) {

    if (nl->ListLength(args) == 3) {
        ListExpr arg1 = nl->First(args);
        ListExpr arg2 = nl->Second(args);
        ListExpr arg3 = nl->Third(args);

        if (!nl->IsEqual(arg1, Graph::BasicType())){
          ErrorReporter::ReportError("first argument must be of type graph");
          return nl->TypeError();
        }
        if( (nl->AtomType(arg2)!=SymbolType) ||
            (nl->AtomType(arg3)!=SymbolType)){
          ErrorReporter::ReportError(
                 "graph x {int, vertex} x {int, vertex} expected");
          return nl->TypeError();
        }
        string arg2s = nl->SymbolValue(arg2);
        string arg3s = nl->SymbolValue(arg3);
        if( (arg2s != Vertex::BasicType()) && (arg2s!=CcInt::BasicType())){
            ErrorReporter::ReportError(
                 "graph x {int, vertex} x {int, vertex} expected");
            return nl->TypeError();
        }
        if( (arg3s != Vertex::BasicType()) && (arg3s!=CcInt::BasicType())){
            ErrorReporter::ReportError(
                 "graph x {int, vertex} x {int, vertex} expected");
            return nl->TypeError();
        }
        return nl->SymbolAtom(Path::BasicType());


    } else {
        ErrorReporter::ReportError(
                "Three arguments expected");
    }

   return nl->TypeError();
}


/*
4.1.5 Type mapping function for operator ~edges~

graph $\to$ stream(tuple([Edge: edge]))

path $\to$ stream(tuple([Edge: edge]))

*/
ListExpr edgesMap ( ListExpr args ) {

    if (nl->ListLength(args) == 1) {

        ListExpr arg1 = nl->First(args);

        if (nl->IsEqual(arg1, Path::BasicType()) ||
            nl->IsEqual(arg1, Graph::BasicType()))
            return nl->TwoElemList(
                nl->SymbolAtom(Symbol::STREAM()),
                nl->TwoElemList(
                    nl->SymbolAtom(Tuple::BasicType()),
                    nl->OneElemList(
                        nl->TwoElemList(
                            nl->SymbolAtom("Edge"),
                            nl->SymbolAtom("edge")
                        )
                    )
                )
            );

        if ((nl->AtomType(arg1) == SymbolType))
            ErrorReporter::ReportError(
                   "Type mapping function got parameter of type " +
                    nl->SymbolValue(arg1));
        else
            ErrorReporter::ReportError(
                    "Type mapping function got wrong types as parameters.");

    } else
        ErrorReporter::ReportError(
                    "Type mapping function got a parameter of length != 1.");

   return nl->SymbolAtom(Symbol::TYPEERROR());
}


/*
4.1.6 Type mapping function for operator ~vertices~

graph $\to$ stream(tuple([Vertex: vertex]))

path $\to$ stream(tuple([Vertex: vertex]))

*/
ListExpr verticesMap ( ListExpr args ) {
    if (nl->ListLength(args) == 1) {

        ListExpr arg1 = nl->First(args);

        if (nl->IsEqual(arg1, Path::BasicType()) ||
            nl->IsEqual(arg1, Graph::BasicType()))
            return nl->TwoElemList(
                nl->SymbolAtom(Symbol::STREAM()),
                nl->TwoElemList(
                    nl->SymbolAtom(Tuple::BasicType()),
                    nl->OneElemList(
                        nl->TwoElemList(
                            nl->SymbolAtom("Vertex"),
                            nl->SymbolAtom(Vertex::BasicType())
                        )
                    )
                )
            );

        if ((nl->AtomType(arg1) == SymbolType))
            ErrorReporter::ReportError(
                   "Operator 'vertices' got parameter of type " +
                   nl->SymbolValue(arg1));
        else
            ErrorReporter::ReportError(
                    "Operator 'vertices' got wrong types as parameters.");

    } else
        ErrorReporter::ReportError(
            "Operator 'vertices'  got a parameter of length != 1.");

   return nl->SymbolAtom(Symbol::TYPEERROR());
}


/*
4.1.7 Type mapping function of operator ~constGraph~ \& ~constGraphPoints~

stream(Tuple) x int\_attr x int\_attr x (Tuple $\to$ real) $\to$ graph

stream(Tuple) x int\_attr x int\_attr x (Tuple $\to$ real) x point\_attr x point\_attr $\to$ graph

*/
template<int characteristic>
ListExpr constGraphTypeMap( ListExpr args )
{
  ListExpr   inTuple,    //The listexpr for the incoming tuple
        attidx1,    //The listexpr for the first index attribute
        attidx2,     //The listexpr for the second index attribute
        func,      //The listexpr for the costs function
        pointidx1,    //The listexpr for the first point attibute
        pointidx2,     //The listexpr for the first point attibute
        errorInfo,attrtype;
  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  string argstr, argstr2,attrname1,attrname2;
  int j,k,l,m;        //The indices of the attributes in the tuple inTuple
  switch (characteristic)
  {
    case 0:
      if(nl->ListLength(args)!=3){
        return listutils::typeError("3 arguments expected");
      }
      inTuple = nl->First(args);
      attidx1  = nl->Second(args);
      attidx2  = nl->Third(args);
      break;
  case 1:
      if(nl->ListLength(args)!=4){
        return listutils::typeError("4 arguments expected");
      }
      inTuple = nl->First(args);
      attidx1  = nl->Second(args);
      attidx2  = nl->Third(args);
       func  = nl->Fourth(args);
       break;
    case 2:
      if(nl->ListLength(args)!=5){
        return listutils::typeError("5 arguments expected");
      }
      inTuple = nl->First(args);
      attidx1  = nl->Second(args);
      attidx2  = nl->Third(args);
       pointidx1  = nl->Fourth(args);
       pointidx2  = nl->Fifth(args);
       break;
    case 3:
      if(nl->ListLength(args)!=6){
        return listutils::typeError("6 arguments expected");
      }
      inTuple = nl->First(args);
      attidx1  = nl->Second(args);
      attidx2  = nl->Third(args);
       func  = nl->Fourth(args);
       pointidx1  = nl->Fifth(args);
       pointidx2= nl->Sixth(args);
       break;
    }

  if(!listutils::isTupleStream(inTuple)){
    return listutils::typeError("first argument must be a tuple stream");
  }

  nl->WriteToString(argstr, attidx1);
  nl->WriteToString(argstr2, attidx2);
  if (nl->AtomType(attidx1) == SymbolType&&
      nl->AtomType(attidx2) == SymbolType)
  {
    attrname1 = nl->SymbolValue(attidx1);
    attrname2 = nl->SymbolValue(attidx2);
  }
  else
  {
    nl->WriteToString(argstr, attidx1);
    ErrorReporter::ReportError("Operator constGraph gets '" +
            argstr +", "+argstr2+ "' as attributenames.\n"
            "Atrribute name may not be the name of a Secondo object!");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
  j = listutils::findAttribute(nl->Second(nl->Second(inTuple)),
                                attrname1, attrtype);
  if (j) {
    if(!listutils::isSymbol(attrtype,CcInt::BasicType())){
      return listutils::typeError("Attribute " + attrname1 +
                                  " must be of type int");
    }
  }
  else
  {
    return listutils::typeError("Attribute " + attrname1 +
                                " not found in tuple");
  }
  k = listutils::findAttribute(nl->Second(nl->Second(inTuple)),
                               attrname2, attrtype);
 if (k)
 {
   if(!listutils::isSymbol(attrtype,CcInt::BasicType())){
     return listutils::typeError("Attribute " + attrname1 +
                                 " must be of type int");
   }
 }
 else
 {
   return listutils::typeError("Attribute " + attrname2 +
                               " not known in the tuple");
 }
 if(characteristic==1||characteristic==3)
 {
   if ( nl->IsAtom(func)
        || (nl->ListLength(func) != 3)
        || !nl->IsEqual(nl->First(func), Symbol::MAP())
        || !nl->IsEqual(nl->Third(func), CcReal::BasicType()) )
      {
          nl->WriteToString(argstr, func);
          ErrorReporter::ReportError("Operator filter expects a "
               "(map tuple real) as its second argument. "
               "The second argument provided "
               "has type '" + argstr + "' instead.");
          return nl->SymbolAtom(Symbol::TYPEERROR());
      }
    }

  if(characteristic==2||characteristic==3)
  {
      nl->WriteToString(argstr, pointidx1);
      nl->WriteToString(argstr2, pointidx2);
      if (nl->AtomType(pointidx1) == SymbolType&&
          nl->AtomType(pointidx2) == SymbolType)
      {
        attrname1 = nl->SymbolValue(pointidx1);
          attrname2 = nl->SymbolValue(pointidx2);
      }
      else
      {
          ErrorReporter::ReportError("Operator constGraph gets '" +
            argstr +", "+argstr2+
            "' as attributenames.\n"
            "Atrribute name may not be the name of a Secondo object!");
      }
      l = FindAttribute(nl->Second(nl->Second(inTuple)), attrname1, attrtype);
      if (l)
      {
        if(!listutils::isSymbol(attrtype,Point::BasicType())){
          return listutils::typeError(" Attribute " + attrname1 +
                                      " must be of type point");
        }
      }
      else
      {
          nl->WriteToString( argstr, nl->Second(nl->Second(inTuple)) );
          ErrorReporter::ReportError(
            "Attributename '" + attrname1 + "' is not known.\n"
            "Known Attribute(s): " + argstr);
          return nl->SymbolAtom(Symbol::TYPEERROR());
      }
       m = FindAttribute(nl->Second(nl->Second(inTuple)), attrname2, attrtype);
      if (m)
      {
        if(!listutils::isSymbol(attrtype,Point::BasicType())){
          return listutils::typeError("Attribute " + attrname2 +
                                      " must be of type point");
        }
      }
      else
      {
          nl->WriteToString( argstr, nl->Second(nl->Second(inTuple)) );
          ErrorReporter::ReportError(
            "Attributename '" + attrname2 + "' is not known.\n"
            "Known Attribute(s): " + argstr);
          return nl->SymbolAtom(Symbol::TYPEERROR());
      }
      return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
           nl->FourElemList(nl->IntAtom(j),nl->IntAtom(k),
                            nl->IntAtom(l),nl->IntAtom(m)),
            nl->SymbolAtom(Graph::BasicType()));
  }
   return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
     nl->TwoElemList(nl->IntAtom(j),nl->IntAtom(k)),
        nl->SymbolAtom(Graph::BasicType()));
}


/*
4.1.8 Type mapping function for operator ~circle~

graph x vertex x real $\to$ graph

*/
ListExpr circleMap ( ListExpr args ) {

   if (nl->ListLength(args) == 3) {

     ListExpr arg1 = nl->First(args);
     ListExpr arg2 = nl->Second(args);
     ListExpr arg3 = nl->Third(args);

     if (nl->IsEqual(arg1, Graph::BasicType()) &&
         nl->IsEqual(arg2, Vertex::BasicType()) &&
         nl->IsEqual(arg3, CcReal::BasicType()))
       return nl->SymbolAtom(Graph::BasicType());

     if ((nl->AtomType(arg1) == SymbolType) &&
         (nl->AtomType(arg2) == SymbolType) &&
         (nl->AtomType(arg3) == SymbolType))
       ErrorReporter::ReportError(
              "Operator 'circle' got parameters of type "
              +nl->SymbolValue(arg1)+
              ", " +nl->SymbolValue(arg2)+ " and "+nl->SymbolValue(arg3));
     else
       ErrorReporter::ReportError(
           "Operator 'circle' got wrong types as parameters.");
   }
   else
     ErrorReporter::ReportError(
         "Operator 'circle' got a parameter of length != 3.");

   return nl->SymbolAtom(Symbol::TYPEERROR());
}


/*
4.1.9 Type mapping function for operator ~placeNodes~

graph $\to$ graph

*/
ListExpr GraphGraphTypeMap(ListExpr args)
{
  if (nl->ListLength(args) == 1)
  {
    ListExpr arg = nl->First(args);
    if (nl->IsEqual(arg, Graph::BasicType()))
    {
      return nl->SymbolAtom(Graph::BasicType());
    }
    else
    {
      ErrorReporter::ReportError(
        "Operator 'placenodes' got paramater of type " +
        nl->ToString(arg));
    }
  }
  else
  {
    ErrorReporter::ReportError(
        "Operator 'placenodes' got a parameter of length != 1.");
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}


/*
4.1.10 Type mapping function for operator ~merge~

graph x graph $\to$ graph

*/
ListExpr GraphGraphGraphTypeMap(ListExpr args)
{
  if (nl->ListLength(args) == 2)
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if (nl->IsEqual(arg1, Graph::BasicType()) &&
        nl->IsEqual(arg2, Graph::BasicType()))
    {
      return nl->SymbolAtom(Graph::BasicType());
    }
    else
    {
      ErrorReporter::ReportError(
          "GraphGraphGraphTypeMap got paramaters of type " +
        nl->ToString(arg1) + " and " + nl->ToString(arg2));
    }
  }
  else
  {
    ErrorReporter::ReportError(
        "GraphGraphGraphTypeMap got a parameter of length != 2.");
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}


/*
4.1.11 Type mapping function for operator ~equal~ and ~partOf~

graph x graph $\to$ bool

*/
ListExpr GraphGraphBoolTypeMap(ListExpr args)
{
  if (nl->ListLength(args) == 2)
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if (nl->IsEqual(arg1, Graph::BasicType()) &&
        nl->IsEqual(arg2, Graph::BasicType()))
    {
      return nl->SymbolAtom(CcBool::BasicType());
    }
    else
    {
      ErrorReporter::ReportError(
          "GraphGraphBoolTypeMap got paramaters of type " +
        nl->ToString(arg1) + " and " + nl->ToString(arg2));
    }
  }
  else
  {
    ErrorReporter::ReportError(
        "GraphGraphBoolTypeMap got a parameter of length != 2.");
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
4.1.12 Type mapping function for the '=' operator

  t x t -> bool with t in {vertex, path, edge, graph}

*/
ListExpr EqualTypeMap(ListExpr args){
 if(nl->ListLength(args)!=2){
   ErrorReporter::ReportError("EqualTypeMap: two arguments expected");
    return nl->TypeError();
 }
 ListExpr arg1 = nl->First(args);
 ListExpr arg2 = nl->Second(args);
 if( (nl->AtomType(arg1)!=SymbolType) || (nl->AtomType(arg2)!=SymbolType)){
   ErrorReporter::ReportError("EqualTypeMap: two simple types expected");
   return nl->TypeError();
 }
 string arg1s = nl->SymbolValue(arg1);
 string arg2s = nl->SymbolValue(arg2);
 if(arg1s!=arg2s){
   ErrorReporter::ReportError(
       "EqualTypeMap: both arguments must have the same type");
   return nl->TypeError();
 }
 if( (arg1s!=Vertex::BasicType()) && (arg1s!="edge") &&
     (arg1s!=Path::BasicType()) && (arg1s!=Graph::BasicType())){
   ErrorReporter::ReportError("EqualTypeMap: only arguments vertex, edge,"
                              " path, and graph are allowed");
   return nl->TypeError();
 }
 return nl->SymbolAtom(CcBool::BasicType());
}

/*
4.1.13 Type mapping for the ~equalway~ operator

*/
ListExpr EqualWayTypeMap(ListExpr args){
  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("Operator equalway: two arguments expected");
     return nl->TypeError();
  }
  if(!nl->IsEqual(nl->First(args),Path::BasicType()) ||
     !nl->IsEqual(nl->Second(args),Path::BasicType())){
    ErrorReporter::ReportError("Operator equalway: path x path expected");
     return nl->TypeError();
  }
  return nl->SymbolAtom(CcBool::BasicType());
}


/*
4.2 Selection functions

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

4.2.1 Selection function for the ~vertices~ and ~edges~ operator

*/
int edgesVerticesSelect(ListExpr args) {

   ListExpr arg = nl->First(args);

   if (nl->IsEqual(arg,Path::BasicType()))
     return (0);
   if (nl->IsEqual(arg,Graph::BasicType()))
     return (1);
   return (-1);
}


/*
4.2.2 Selection function for the ~shortestPath~ Operator

*/
int shortestPathSelect(ListExpr args){
   string arg2 = nl->SymbolValue(nl->Second(args));
   string arg3 = nl->SymbolValue(nl->Third(args));
   if(arg2==Vertex::BasicType() && arg3==Vertex::BasicType()) return 0;
   if(arg2==Vertex::BasicType() && arg3==CcInt::BasicType()) return 1;
   if(arg2==CcInt::BasicType() && arg3==Vertex::BasicType()) return 2;
   if(arg2==CcInt::BasicType() && arg3==CcInt::BasicType()) return 3;
   return -1;
}


/*
4.2.3 Selection function for the '=' operator

*/
int EqualSelect(ListExpr args){
  if(nl->IsEqual(nl->First(args),Graph::BasicType())){
     return  0;
  } else {
    return 1;
  }
}


/*
4.3 Value Mapping Functions

*/

/*
4.3.1 Value mapping function for operator ~theVertex~

*/
int theVertexFun (Word* args, Word& result, int message,
                  Word& local, Supplier s)
{
   Graph* g = ((Graph*)args[0].addr);
   int n = ((CcInt*)args[1].addr)->GetIntval();

   result = qp->ResultStorage(s);

   *((Vertex*)result.addr) = g->GetVertex(n);

   return 0;
}

/*
4.3.2 Value mapping function for operator ~maxDegree~

*/
int maxDegreeFun (Word* args, Word& result,
                  int message, Word& local, Supplier s)
{
   Graph* g = ((Graph*)args[0].addr);
   bool b = ((CcBool*)args[1].addr)->GetBoolval();

   result = qp->ResultStorage(s);

   ((CcInt*)result.addr)->Set(true,g->GetMaxDeg(b));

   return 0;
}

/*
4.3.3 Value mapping function for operator ~minDegree~

*/
int minDegreeFun (Word* args, Word& result, int message,
                  Word& local, Supplier s)
{
   Graph* g = ((Graph*)args[0].addr);
   bool b = ((CcBool*)args[1].addr)->GetBoolval();

   result = qp->ResultStorage(s);

   ((CcInt*)result.addr)->Set(true,g->GetMinDeg(b));

   return 0;
}

/*
4.3.4 Value mapping function for operator ~connectedComponents~

*/
struct SccStruct {

   vector<Graph*> scc;
   unsigned int index;
   TupleType* tType;
};


int connectedCompFun (Word* args, Word& result, int message,
                      Word& local, Supplier s) {

   SccStruct* localInfo;
   ListExpr resultType;
   Tuple* t;
   Graph* g;

   switch(message) {

     case OPEN:

        g = ((Graph*)args[0].addr);
        resultType = GetTupleResultType(s);
        localInfo = new SccStruct();

        localInfo->tType = new TupleType(nl->Second(resultType));
        localInfo->scc = g->GetStronglyConnectedComponents();
        localInfo->index = 0;
        local.addr = localInfo;

        return 0;

     case REQUEST:

        localInfo = ((SccStruct*)local.addr);

        // no more SCCs to return ?
        if (localInfo->index >= localInfo->scc.size())
          return CANCEL;

        t = new Tuple(localInfo->tType);
        t->PutAttribute(0,localInfo->scc[localInfo->index]);

        result.addr = t;
        localInfo->index++;

        return YIELD;

     case CLOSE:

       if(local.addr)
       {
          localInfo = ((SccStruct*)local.addr);
          delete localInfo->tType;
          delete localInfo;
          local.setAddr(Address(0));
       }
       return 0;
   }

   return -1;
}
/*
4.3.5 Value mapping function for operator ~circle~

*/
int circleFun (Word* args, Word& result, int message,
               Word& local, Supplier s) {

   Graph* g = ((Graph*)args[0].addr);
   Vertex* v = ((Vertex*)args[1].addr);
   float f = ((CcReal*)args[2].addr)->GetRealval();

   if (f < 0.0f)
     f = 0.0f;

   result = qp->ResultStorage(s);

   Graph* circle = g->GetCircle(v->GetKey(),f);
   ((Graph*)result.addr)->CopyFrom(circle);
   delete circle;

   return 0;
}

/*
4.3.6 Value mapping function for operator ~shortestPath~

*/

template <class T1, class T2>
int shortestPathFun (Word* args, Word& result,
                    int message, Word& local, Supplier s)
{
    Graph* g = ((Graph*)args[0].addr);
    T1* source = ((T1*)args[1].addr);
    T2* target = ((T2*)args[2].addr);
    result = qp->ResultStorage(s);

    g->GetShortestPath(source->GetIntval(),
                       target->GetIntval(),
                       (Path*)result.addr);
    return 0;
}




/*
4.3.7 Value mapping function for operator ~edges~

*/

struct EdgeStruct {

    vector<Edge>* edges;
    unsigned int index;
    TupleType* tType;
};

template<bool isGraph>
int edgesFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
    EdgeStruct* localInfo;
    ListExpr resultType;
    Tuple* t;
    Path* p;
    Graph* g;

    switch (message) {
        case OPEN:

            if (isGraph)
              g = ((Graph*)args[0].addr);
            else
              p = ((Path*)args[0].addr);

            localInfo = new EdgeStruct();
            resultType = GetTupleResultType(s);
            localInfo->tType = new TupleType(nl->Second(resultType));

            localInfo->edges = (isGraph) ? g->GetEdges() : p->GetEdges();
            localInfo->index = 0;
            local.addr = localInfo;

            return 0;

        case REQUEST:

            localInfo = ((EdgeStruct*) local.addr);

            // Are all edges be given into the stream ?
            if (localInfo->index >= localInfo->edges->size())
                return CANCEL;

            t = new Tuple(localInfo->tType);
            t->PutAttribute(0,new Edge(localInfo->edges->at(localInfo->index)));
            result.addr = t;
            localInfo->index++;

            return YIELD;

        case CLOSE:

          if(local.addr)
          {
            localInfo = ((EdgeStruct*) local.addr);
            delete localInfo->tType;
            delete localInfo->edges;
            delete localInfo;
            local.setAddr(Address(0));
          }
          return 0;
    }

    return -1;
}

/*
4.3.8 Value mapping function for operator ~vertices~

*/

struct VertexStruct {

    vector<Vertex>* vertices;
    unsigned int index;
    TupleType* tType;
};

template<bool isGraph>
int verticesFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
    VertexStruct* localInfo;
    ListExpr resultType;
    Tuple* t;
    Graph* g;
    Path* p;

    switch (message) {
        case OPEN:

            if (isGraph)
              g = ((Graph*)args[0].addr);
            else
              p = ((Path*)args[0].addr);

            p = ((Path*)args[0].addr);
            resultType = GetTupleResultType(s);
            localInfo = new VertexStruct();
            localInfo->tType = new TupleType(nl->Second(resultType));
            localInfo->vertices = (isGraph) ? g->GetVertices()
                                            : p->GetVertices();
            localInfo->index = 0;
            local.addr = localInfo;

            return 0;

        case REQUEST:

            localInfo = ((VertexStruct*) local.addr);
            // Are all edges be given into the stream ?
            if (localInfo->index >= localInfo->vertices->size())
                return CANCEL;

            t = new Tuple(localInfo->tType);
            t->PutAttribute(0,
                  new Vertex(localInfo->vertices->at(localInfo->index)));
            result.addr = t;
            localInfo->index++;

            return YIELD;

        case CLOSE:

          if(local.addr)
          {
            localInfo = ((VertexStruct*) local.addr);
            delete localInfo->tType;
            delete localInfo->vertices;
            delete localInfo;
            local.setAddr(Address(0));
          }
          return 0;
    }

    return -1;
}

/*
4.3.9 Value mapping function for operator ~partOf~

*/

int partOfFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
    Graph* part = ((Graph*)args[0].addr);
    Graph* big = ((Graph*)args[1].addr);

    result = qp->ResultStorage(s);

    ((CcBool*)result.addr)->Set(true,big->PartOf(part));

    return 0;
}

/*
4.3.10 Value mapping function of operator ~constGraph~ \& ~constGraphPoints~

This operator has 4 parameter characteristics seperated by the template ~characteristic~:

   0: ((stream X) a1 a2)

   1: ((stream X) a1 a2 (tuple$\to$real)))

   2: ((stream X) a1 a2 p1 p2)

   3: ((stream X) a1 a2 (tuple$\to$real) p1 p2)

         $\to$ (graph )

*/
template<int characteristic>
int constGraphFun(Word* args, Word& result, int message,
                 Word& local, Supplier s)
{
//     cout <<"test1 "<<((CcInt*)args[1].addr)->GetIntval() - 1 <<endl;
//     cout <<"test2 "<<((CcInt*)args[2].addr)->GetIntval() - 1 <<endl;
//     cout <<"test3 "<<((CcInt*)args[3].addr)->GetIntval() - 1 <<endl;
//    cout <<"test4 "<<((CcInt*)args[4].addr)->GetIntval() - 1 <<endl;
//     cout <<"test5 "<<((CcInt*)args[5].addr)->GetIntval() - 1 <<endl;
//     cout <<"test6 "<<((CcInt*)args[6].addr)->GetIntval() - 1 <<endl;
//     cout <<"test7 "<<((CcInt*)args[7].addr)->GetIntval() - 1 <<endl;
//     cout <<"test8 "<<((CcInt*)args[8].addr)->GetIntval() - 1 <<endl;
    Word t,funresult;
    Graph* res;       //the graph all is about ;-))
    Point p1(false);  //the point representations of a vertex,
                      //if no point attributes are given,
                      // an empty point will be used
    Point p2(false);  //the point representations of a vertex,
                      // if no point attributes are given,
                     // an empty point will be used
    Tuple* tup;
    Point* Start, *Ziel;                //temprary point
                                       //variables used to fill p1 and p2
    int attributeIndex1,attributeIndex2, //The indices of the index
                                         // attributes in the incoming tuple
      pointIndex1,pointIndex2;          //The indices of the point attributes
                                        // in the incoming tuple
    ArgVectorPointer funargs;
    double costs=1.0;          //the costs of an edge,
                               // if no cost function is given,
                               // 1.0 will be used
    Supplier supplier;
  qp->Open(args[0].addr);
    if (characteristic==0||characteristic==1)  //the typemapping operator
                                       // put theseattributes
                                       // to different places
    {
      attributeIndex1 = ((CcInt*)args[4].addr)->GetIntval() - 1;
      attributeIndex2 = ((CcInt*)args[5].addr)->GetIntval() - 1;
    }
    else
    {
      attributeIndex1 = ((CcInt*)args[6].addr)->GetIntval() - 1;
      attributeIndex2 = ((CcInt*)args[7].addr)->GetIntval() - 1;
      pointIndex1=((CcInt*)args[8].addr)->GetIntval() - 1;
      pointIndex2=((CcInt*)args[9].addr)->GetIntval() - 1;
    }

    result = qp->ResultStorage(s);
    res = (Graph*)(result.addr);
    res->SetDefined(true);
    qp->Request(args[0].addr,t);
    if(characteristic==1||characteristic==3)
    {
      supplier = args[3].addr;
        funargs = qp->Argument(supplier);  //Get the argument vector for
    }
    while (qp->Received(args[0].addr))
    {
        tup = (Tuple*)t.addr;
        int extIndex1=((CcInt*)
                       tup->GetAttribute(attributeIndex1))->GetIntval();
        int extIndex2=((CcInt*)
                       tup->GetAttribute(attributeIndex2))->GetIntval();
        if(characteristic==1||characteristic==3)
        {
          (*funargs)[0] = t;
            //Supply the argument for the
            //parameter function.
          qp->Request(supplier, funresult);
            //Ask the parameter function
            //to be evaluated.
            costs=abs(((CcReal*)funresult.addr)->GetRealval());
        }
        if(characteristic==2||characteristic==3)
        {
          Start=(Point*)(tup->GetAttribute(pointIndex1));
          p1.Set(Start->GetX(),Start->GetY());
          Ziel=(Point*)(tup->GetAttribute(pointIndex2));
          p2.Set(Ziel->GetX(),Ziel->GetY());
        }
        res->AddVertex(extIndex1,p1);
        res->AddVertex(extIndex2,p2);
        res->AddEdge(extIndex1,extIndex2,costs);
        qp->Request(args[0].addr,t);
        tup->DeleteIfAllowed();
  }
    result.setAddr(res);
    qp->Close(args[0].addr);
    return 0;
}



/*
4.3.11 Value mapping function of operator ~placeNodes~

*/
int graphplacenodes(Word* args, Word& result, int message, Word& local,
  Supplier s)
{
  Graph const * pGraph = static_cast<Graph const *>(args[0].addr);
  result = qp->ResultStorage(s);
  Graph* pRet = static_cast<Graph *>(result.addr);
  pRet->CopyFrom(pGraph);
  PlaceNodesHelper helper;
  helper.PlaceNodes(pRet);

  return 0;
}


/*
4.3.12 Value mapping function of operator ~merge~

*/
int graphmerge(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Graph const * pGraph1 = static_cast<Graph const *>(args[0].addr);
  Graph const * pGraph2 = static_cast<Graph const *>(args[1].addr);
  result = qp->ResultStorage(s);
  Graph* pRet = static_cast<Graph *>(result.addr);

  if (pGraph1 != NULL && pGraph2 != NULL && pRet != NULL)
  {
    //Add the vertices of both source graphs
    vector<Vertex> * pVertices1 = pGraph1->GetVertices(false);
    vector<Vertex> * pVertices2 = pGraph2->GetVertices(false);
    if (pVertices1 != NULL && pVertices2 != NULL)
    {
      vector<Vertex>::const_iterator it1 = pVertices1->begin();
      vector<Vertex>::const_iterator it2 = pVertices2->begin();
      vector<Vertex>::const_iterator itEnd1 = pVertices1->end();
      vector<Vertex>::const_iterator itEnd2 = pVertices2->end();
      while (it1 != itEnd1 && it2 != itEnd2)
      {
        Vertex const & v1 = *it1;
        Vertex const & v2 = *it2;
        int nCompare = v1.Compare(&v2);
        if (nCompare < 0)
        {
          pRet->Add(v1);
          ++it1;
        }
        else if (nCompare > 0)
        {
          pRet->Add(v2);
          ++it2;
        }
        else
        {
          if (v1.GetPos().IsDefined())
          {
            if (v2.GetPos().IsDefined())
            {
              double min = v1.GetPos().GetX();
              double max = v2.GetPos().GetX();
             if (min > max) {
                   min = max;
                   max = v1.GetPos().GetX();
             }
             double x = min + (abs(max-min)) / 2.0;
              min = v1.GetPos().GetY();
             max = v2.GetPos().GetY();
             if (min > max) {
                 min = max;
                 max = v1.GetPos().GetY();
             }
             double y = min + (abs(max-min)) / 2.0;
              pRet->Add(Vertex(v1.GetKey(), Point(true,x, y)));
            }
            else
            {
              pRet->Add(v1);
            }
          }
          else
          {
            pRet->Add(v2);
          }
          ++it1;
          ++it2;
        }
      }

      //Add the rest
      while (it1 != itEnd1)
      {
        pRet->Add(*it1);
        ++it1;
      }
      while (it2 != itEnd2)
      {
        pRet->Add(*it2);
        ++it2;
      }
    }
    else if (pVertices1 != NULL)
    {
      for (vector<Vertex>::const_iterator it = pVertices1->begin(),
        itEnd = pVertices1->end(); it != itEnd; ++it)
      {
        pRet->Add(*it);
      }
    }
    else if (pVertices2 != NULL)
    {
      for (vector<Vertex>::const_iterator it = pVertices2->begin(),
        itEnd = pVertices2->end(); it != itEnd; ++it)
      {
        pRet->Add(*it);
      }
    }
    delete pVertices1;
    pVertices1 = NULL;
    delete pVertices2;
    pVertices2 = NULL;

    //Add the edges of both source graphs
    vector<Edge> * pEdges1 = pGraph1->GetEdges(false);
    vector<Edge> * pEdges2 = pGraph2->GetEdges(false);
    if (pEdges1 != NULL && pEdges2 != NULL)
    {
      vector<Edge>::const_iterator it1 = pEdges1->begin();
      vector<Edge>::const_iterator it2 = pEdges2->begin();
      vector<Edge>::const_iterator itEnd1 = pEdges1->end();
      vector<Edge>::const_iterator itEnd2 = pEdges2->end();
      while (it1 != itEnd1 && it2 != itEnd2)
      {
        Edge const & e1 = *it1;
        Edge const & e2 = *it2;
        int nCompare = e1.Compare(&e2);
        if (nCompare < 0)
        {
          pRet->Add(e1);
          ++it1;
        }
        else if (nCompare > 0)
        {
          pRet->Add(e2);
          ++it2;
        }
        else
        {
          pRet->Add(e1.GetCost() < e2.GetCost() ? e1 : e2);
          ++it1;
          ++it2;
        }
      }

      //Add the rest
      while (it1 != itEnd1)
      {
        pRet->Add(*it1);
        ++it1;
      }
      while (it2 != itEnd2)
      {
        pRet->Add(*it2);
        ++it2;
      }
    }
    else if (pEdges1 != NULL)
    {
      for (vector<Edge>::const_iterator it = pEdges1->begin(),
        itEnd = pEdges1->end(); it != itEnd; ++it)
      {
        pRet->Add(*it);
      }
    }
    else if (pEdges2 != NULL)
    {
      for (vector<Edge>::const_iterator it = pEdges2->begin(),
        itEnd = pEdges2->end(); it != itEnd; ++it)
      {
        pRet->Add(*it);
      }
    }
    delete pEdges1;
    pEdges1 = NULL;
    delete pEdges2;
    pEdges2 = NULL;
  }
  else if (pRet != NULL)
  {
    if (pGraph1 != NULL)
    {
      pRet->Add(*pGraph1);
    }
    else if (pGraph2 != NULL)
    {
      pRet->Add(*pGraph2);
    }
  }

  return 0;
}

/*
4.3.13 Value mappings function of operator ~equal~

*/
int EqualFun(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Attribute* arg1 = (Attribute*)(args[0].addr);
  Attribute* arg2 = (Attribute*)(args[1].addr);
  result = qp->ResultStorage(s);
  if(!arg1->IsDefined() || !arg2->IsDefined()){
     ((CcBool*)result.addr)->SetDefined(false);
  } else {
     bool res = arg1->Compare(arg2)==0;
     ((CcBool*)result.addr)->Set(true,res);
  }
  return 0;
}

int EqualGraphFun(Word* args, Word& result,
                 int message, Word& local, Supplier s)
{
  Graph* pGraph1 = (Graph*)(args[0].addr);
  Graph* pGraph2 = (Graph*)(args[1].addr);
  result = qp->ResultStorage(s);
  ((CcBool*)result.addr)->Set(true,pGraph1->EqualsWith(pGraph2));

  //todo

  return 0;
}

/*
4.3.14 Value mapping for the ~equalway~ operator

*/

int EqualWayFun(Word* args, Word& result, int message,
                Word& local, Supplier s)
{
  Path* arg1 = (Path*)(args[0].addr);
  Path* arg2 = (Path*)(args[1].addr);
  result = qp->ResultStorage(s);
  arg1->EqualWay(arg2,*((CcBool*)result.addr));
  return 0;
}


/*
4.4 Specification of operators

*/
const string SpecTheVertex  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>graph x int -> vertex</text---> "
       "<text>thevertex ( _ , _ )</text--->"
       "<text>Returns the vertex of the graph whose key corresponds"
              " to the second argument or an undefined vertex.</text--->"
       "<text>query thevertex(g1,-10)</text--->"
       ") )";

const string SpecMaxDegree  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>graph x bool -> int</text---> "
       "<text>maxdegree ( _, _ )</text--->"
       "<text>Returns the maximum output degree of the graph if the second"
       " argument is TRUE and otherwise the maximum input degree.</text--->"
       "<text>query maxdegree(g1,true)</text--->"
       ") )";

const string SpecMinDegree  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>graph x bool -> int</text---> "
       "<text>mindegree ( _, _ )</text--->"
       "<text>Returns the minimum output degree of the graph if the second"
       " argument is TRUE and otherwise the minimum input degree.</text--->"
       "<text>query mindegree(g1,false)</text--->"
       ") )";

const string SpecConnectedComp  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>graph -> stream(tuple(Graph: graph))</text---> "
       "<text>connectedcomponents ( _ )</text--->"
       "<text>Returns all strong connected components"
       " of the graph as graph stream.</text--->"
       "<text>query connectedcomponents(g1) consume</text--->"
       ") )";

const string SpecCircle  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>graph x vertex x float -> graph</text---> "
       "<text>circle ( _, _, _ )</text--->"
       "<text>Returns all vertices and connecting edges of the graph"
       " whose network distance from the start vertex (second argument)"
       " is not greater than the third argument.</text--->"
       "<text>query circle(g1,v1,23.4)</text--->"
       ") )";

const string SpecShortestPath  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>graph x {vertex, int} x {vertex, int} -> path</text---> "
       "<text>shortestPath ( _, _, _ )</text--->"
       "<text>Returns the sportest path from the first to"
        " the second vertex.</text--->"
       "<text>query shortestpath(g1, 1, 9)</text--->"
       ") )";

const string SpecEdges  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>path -> stream(tuple(Edge: edge)), "
     "graph -> stream(tuple(Edge: edge))</text---> "
       "<text>edges ( _ )</text--->"
       "<text>Returns a tuple stream of all edges of the path or"
       " graph in ascending order.</text--->"
       "<text>query edges(p1) consume</text--->"
       ") )";

const string SpecVertices  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>path -> stream(tuple(Vertex: vertex)), "
     "graph -> stream(tuple(Vertex: vertex))</text---> "
       "<text>vertices ( _ )</text--->"
       "<text>Returns a tuple stream of all vertices of the path or "
       "graph in ascending order.</text--->"
       "<text>query vertices(p1) consume</text--->"
       ") )";

const string SpecPartOf  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>graph x graph -> bool</text---> "
       "<text>_ partof _ </text--->"
       "<text>Returns true if the small graph is a subgraph of the "
       "big graph, otherwise false.</text--->"
       "<text>query small partof big consume</text--->"
       ") )";

const string constGraphSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
   "\"Example\" ) "
    "( <text>(stream(tuple(x)) x a1 x a2 x (fun)"
    " -> graph </text---> "
    "<text>x constgraph [a1,a2,(fun)]</text--->"
    "<text>Constructs a new graph, with vertices are indexed"
    " by the attributes a1 and a2. The edges go from a1 to a2 with "
    "the costs given by the real function fun.</text--->"
    "<text>query (poi feed {a} filter [.poiShapeID_a<10] "
    "poi feed{b} symmjoin[.poiShapeID_a > ..poiShapeID_b]"
    " constgraph [poiShapeID_a"
    ",poiShapeID_b,1.0] (Works on the database OSNABRUECK)</text--->"
    ") )";

const string constGraphPointsSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
   "\"Example\" ) "
    "( <text>(stream(tuple(x)) x a1 x a2 x(tuple -> real)x p1 x p2"
    " -> graph </text---> "
    "<text>x constgraphpoints[a1,a2,(fun,p1,p2]</text--->"
    "<text>Constructs a new graph, with vertices are indexed"
    " by the attributes a1 and a2. Each vertex has the graphical"
    " representation given by the p attributes."
    " The edges go from a1 to a2 with the costs given by the real"
    " function fun.</text--->"
    "<text> query (poi feed {a} filter [.poiShapeID_a<10] poi"
    " feed{b} symmjoin"
    "[.poiShapeID_a > ..poiShapeID_b]"
    " constgraphpoints[poiShapeID_a,poiShapeID_b,"
    "distance(.geoData_a,.geoData_b),geoData_a,geoData_b]"
    "(Works on the database OSNABRUECK)</text--->"
    ") )";

const string EqualSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
   "\"Example\" ) "
    "( <text> T x T  -> bool , T in {vertex, edge, path, graph}</text---> "
    "<text>g1 =  g2</text--->"
    "<text>Check if the arguments are equal</text--->"
    "<text> query g1 equal g1; results TRUE</text--->"
    ") )";

const string EqualWaySpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
   "\"Example\" ) "
    "( <text> path x path -> bool</text---> "
    "<text>p1 equalway p2</text--->"
    "<text>Check if the arguments have the same way "
          "with repect to the key of the vertices</text--->"
    "<text> query p1 equalway p2</text--->"
    ") )";

string const placenodesSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>graph -> graph</text--->"
  "<text>placenodes ( _ )</text--->"
  "<text>places the nodes of a graph</text--->"
  "<text>placenodes(g1)</text---> ) )";


string const mergeSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>graph x graph -> graph</text--->"
  "<text>merge ( _, _ )</text--->"
  "<text>merges two graphs</text--->"
  "<text>merge(g1, g2)</text---> ) )";

/*
4.5 Definition of the operators

4.5.1 Definition of value mapping vectors

*/
ValueMapping verticesValueMap[] = { verticesFun<false>, verticesFun<true> };

ValueMapping edgesValueMap[] = { edgesFun<false>, edgesFun<true> };

ValueMapping shortestPathValueMap[] = {
            shortestPathFun<Vertex,Vertex>, shortestPathFun<Vertex,CcInt>,
            shortestPathFun<CcInt,Vertex>, shortestPathFun<CcInt,CcInt>};

ValueMapping EqualValueMap[] = {EqualGraphFun, EqualFun };



/*
4.5.2  Operator definitions

*/
Operator theVertex (
  "thevertex",
  SpecTheVertex,
  theVertexFun,
  Operator::SimpleSelect,
  theVertexMap
);

Operator maxDegree (
  "maxdegree",
  SpecMaxDegree,
  maxDegreeFun,
  Operator::SimpleSelect,
  minMaxDegreeMap
);

Operator minDegree (
  "mindegree",
  SpecMinDegree,
  minDegreeFun,
  Operator::SimpleSelect,
  minMaxDegreeMap
);

Operator connectedComp (
  "connectedcomponents",
  SpecConnectedComp,
  connectedCompFun,
  Operator::SimpleSelect,
  connectedCompMap
);

Operator circle (
  "circle",
  SpecCircle,
  circleFun,
  Operator::SimpleSelect,
  circleMap
);

Operator shortestPath (
  "shortestpath",
  SpecShortestPath,
  4,
  shortestPathValueMap,
  shortestPathSelect,
  shortestPathMap
);

Operator edges (
  "edges",
  SpecEdges,
  2,
  edgesValueMap,
  edgesVerticesSelect,
  edgesMap
);

Operator vertices (
  "vertices",
  SpecVertices,
  2,
  verticesValueMap,
  edgesVerticesSelect,
  verticesMap
);


Operator partOf (
  "partof",
  SpecPartOf,
  partOfFun,
  Operator::SimpleSelect,
  GraphGraphBoolTypeMap
);


Operator constGraph (
         "constgraph",
         constGraphSpec,
         constGraphFun<1>,
         Operator::SimpleSelect,
         constGraphTypeMap<1>
);

Operator constGraphPoints (
         "constgraphpoints",
         constGraphPointsSpec,
         constGraphFun<3>,
         Operator::SimpleSelect,
         constGraphTypeMap<3>
);


Operator graph_placenodes(
  "placenodes",
  placenodesSpec,
  graphplacenodes,
  Operator::SimpleSelect,
  GraphGraphTypeMap
);


Operator graph_merge(
  "merge",
  mergeSpec,
  graphmerge,
  Operator::SimpleSelect,
  GraphGraphGraphTypeMap
);

Operator equalway(
  "equalway",
  EqualWaySpec,
  EqualWayFun,
  Operator::SimpleSelect,
  EqualWayTypeMap
);


Operator equalop (
  "=",
  EqualSpec,
  2,
  EqualValueMap,
  EqualSelect,
  EqualTypeMap
);

/*
4 operators

*/


ListExpr EdgeIntTypeMap(ListExpr args)
{
  if (nl->ListLength(args) == 1)
  {
    ListExpr arg = nl->First(args);
    if (nl->IsEqual(arg, "edge"))
    {
      return nl->SymbolAtom(CcInt::BasicType());
    }
    else
    {
      ErrorReporter::ReportError(
        "Type mapping function got paramater of type " +
        nl->ToString(arg));
    }
  }
  else
  {
    ErrorReporter::ReportError(
      "Type mapping function got a parameter of length != 1.");
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr EdgeRealTypeMap(ListExpr args)
{
  if (nl->ListLength(args) == 1)
  {
    ListExpr arg = nl->First(args);
    if (nl->IsEqual(arg, "edge"))
    {
      return nl->SymbolAtom(CcReal::BasicType());
    }
    else
    {
      ErrorReporter::ReportError(
        "Type mapping function got paramater of type " +
        nl->ToString(arg));
    }
  }
  else
  {
    ErrorReporter::ReportError(
      "Type mapping function got a parameter of length != 1.");
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}


int graphsource(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Edge const * pEdge = static_cast<Edge const *>(args[0].addr);
  result = qp->ResultStorage(s);
  CcInt* pRet = static_cast<CcInt *>(result.addr);
  pRet->Set(true, pEdge->GetSource());

  return 0;
}


int graphtarget(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Edge const * pEdge = static_cast<Edge const *>(args[0].addr);
  result = qp->ResultStorage(s);
  CcInt* pRet = static_cast<CcInt *>(result.addr);
  pRet->Set(true, pEdge->GetTarget());

  return 0;
}

int graphcost(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Edge const * pEdge = static_cast<Edge const *>(args[0].addr);
  result = qp->ResultStorage(s);
  CcReal* pRet = static_cast<CcReal *>(result.addr);
  pRet->Set(true, pEdge->GetCost());

  return 0;
}

string const sourceSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text>edge -> int</text--->"
   "<text>get_source ( _ )</text--->"
   "<text>the source vertex of the edge</text--->"
   "<text>get_source(e1)</text---> ) )";


string const targetSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>edge -> int</text--->"
  "<text>get_target ( _ )</text--->"
  "<text>the target vertex of the edge</text--->"
  "<text>get_target(e1)</text---> ) )";


string const costSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>edge -> real</text--->"
  "<text>get_cost ( _ )</text--->"
  "<text>the cost of the edge</text--->"
  "<text>get_cost(e1)</text---> ) )";



/*
4.1 operator source

returns the key of the source vertex

*/
Operator graph_source("get_source", sourceSpec, graphsource,
  Operator::SimpleSelect, EdgeIntTypeMap);

/*
4.2 operator target

returns the key of the target vertex

*/
Operator graph_target("get_target", targetSpec, graphtarget,
  Operator::SimpleSelect, EdgeIntTypeMap);

/*
4.3 operator cost

returns the cost of the edge

*/
Operator graph_cost("get_cost", costSpec, graphcost,
  Operator::SimpleSelect, EdgeRealTypeMap);


/*
4 operators on edges

*/

ListExpr VertexIntTypeMap(ListExpr args)
{
    if (nl->ListLength(args) == 1)
    {
        ListExpr arg = nl->First(args);
        if (nl->IsEqual(arg, Vertex::BasicType()))
        {
            return nl->SymbolAtom(CcInt::BasicType());
        }
        else
        {
            ErrorReporter::ReportError(
                "Type mapping function got paramater of type " +
                nl->ToString(arg));
        }
    }
    else
    {
        ErrorReporter::ReportError(
            "Type mapping function got a parameter of length != 1.");
    }
    return nl->SymbolAtom(Symbol::TYPEERROR());
}


ListExpr VertexPointTypeMap(ListExpr args)
{
    if (nl->ListLength(args) == 1)
    {
        ListExpr arg = nl->First(args);
        if (nl->IsEqual(arg, Vertex::BasicType()))
        {
            return nl->SymbolAtom(Point::BasicType());
        }
        else
        {
            ErrorReporter::ReportError(
                "Type mapping function got paramater of type " +
                nl->ToString(arg));
        }
    }
    else
    {
        ErrorReporter::ReportError(
            "Type mapping function got a parameter of length != 1.");
    }
    return nl->SymbolAtom(Symbol::TYPEERROR());
}


int graphkey(Word* args, Word& result, int message, Word& local, Supplier s)
{
    Vertex const * pVertex = static_cast<Vertex const *>(args[0].addr);
    result = qp->ResultStorage(s);
    CcInt* pRet = static_cast<CcInt *>(result.addr);
    pRet->Set(true, pVertex->GetKey());

    return 0;
}


int graphpos(Word* args, Word& result, int message, Word& local, Supplier s)
{
    Vertex const * pVertex = static_cast<Vertex const *>(args[0].addr);
    result = qp->ResultStorage(s);
    Point* pRet = static_cast<Point *>(result.addr);
    pRet->CopyFrom(&pVertex->GetPos());

    return 0;
}


string const keySpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>vertex -> int</text--->"
    "<text>get_key ( _ )</text--->"
    "<text>the key of the vertex</text--->"
    "<text>get_key(v1)</text---> ) )";

string const posSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>vertex -> point</text--->"
    "<text>get_pos ( _ )</text--->"
    "<text>the position of the vertex</text--->"
    "<text>get_pos(v1)</text---> ) )";


/*
4.1 operator key

returns the key of the vertex

*/
Operator graph_key("get_key", keySpec, graphkey, Operator::SimpleSelect,
    VertexIntTypeMap);

/*
4.2 operator pos

returns the position of the vertex

*/

Operator graph_pos("get_pos", posSpec, graphpos, Operator::SimpleSelect,
    VertexPointTypeMap);


/*
4 Implementation of the ~graph~ type constructor

*/


/*
4.10 Function describing the signature of the type constructor

*/
ListExpr GraphProperty() {

   return (
     nl->TwoElemList(
         nl->FiveElemList(
             nl->StringAtom("Signature"),
             nl->StringAtom("Example Type List"),
             nl->StringAtom("List Rep"),
             nl->StringAtom("Example List"),
             nl->StringAtom("Remarks")
             ),
         nl->FiveElemList(
             nl->StringAtom("-> DATA"),
             nl->StringAtom("graph"),
             nl->TextAtom("((<vertex>*) (<edge>*))"),
             nl->TextAtom("(((2 (2.0 -3.5)) (5 (-1.2 5.0)))"
                          " ((2 2 4.2) (5 2 0.2)))"),
             nl->TextAtom("The vertices must have unique keys, the edges use"
                          " source and target keys from the vertex list, the"
                          " edge costs must be positive and "
                          "there are no parallel edges.")
             )
         )
     );
}


/*
4.1 List Representation

The list representation of a graph is

----  ((vertex*) (edge*))
----

conditions: vertex keys are unique, no parallel edges, source and target
keys of the edges exist in the vertex list

4.2 ~Out~-function

*/
ListExpr OutGraph( ListExpr typeInfo, Word value ) {


   Graph* graph = (Graph*)(value.addr);

   if( !graph->IsDefined() )
     return nl->SymbolAtom(Symbol::UNDEFINED());

   // create ListExpr for vertices

   vector<Vertex>* v = graph->GetVertices(true);
   ListExpr last;
   ListExpr verticesList;
   Point p;

   if ((*v).size() > 0) {
     p = (*v)[0].GetPos();
     verticesList =
       nl->OneElemList(
           nl->TwoElemList(
               nl->IntAtom((*v)[0].GetKey()),
               OutPoint( nl->TheEmptyList(), SetWord( (void*)(&p)))
           )
       );

    last = verticesList;
    for (unsigned int i=1;i<(*v).size();i++) {
       p = (*v)[i].GetPos();
       last =
         nl->Append(
             last,
             nl->TwoElemList(
                 nl->IntAtom((*v)[i].GetKey()),
                 OutPoint( nl->TheEmptyList(), SetWord( (void*)(&p)))
            )
         );
     }
   }
   else
     verticesList = nl->TheEmptyList();
   delete v;
   v = 0;

   // create ListExpr for edges

   vector<Edge>* e = graph->GetEdges(true);
   ListExpr edgesList;

   if (e->size() > 0) {

     edgesList =
       nl->OneElemList(
           nl->ThreeElemList(
               nl->IntAtom((*e)[0].GetSource()),
               nl->IntAtom((*e)[0].GetTarget()),
               nl->RealAtom((*e)[0].GetCost())
           )
       );

     last = edgesList;
     for (unsigned int i=1;i<(*e).size();i++) {
       last =
         nl->Append(
             last,
             nl->ThreeElemList(
               nl->IntAtom((*e)[i].GetSource()),
               nl->IntAtom((*e)[i].GetTarget()),
               nl->RealAtom((*e)[i].GetCost())
             )
         );
     }
   }
   else{
     edgesList = nl->TheEmptyList();
   }
   delete e;

   return (nl->TwoElemList(verticesList,edgesList));
}

/*
4.3 ~In~-function

*/
Word InGraph( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct ) {

   Graph* graph;
   graph = new Graph(true);

   correct = false;

   if (nl->ListLength(instance) == 2) {

     ListExpr verticesList = nl->First(instance);
     ListExpr edgesList = nl->Second(instance);

     if (!(nl->IsAtom(verticesList) || nl->IsAtom(edgesList))) {

       correct = true;
       Point* p;
       ListExpr first, second, third;
       ListExpr firstElem = nl->Empty();
       ListExpr rest = verticesList;

       // parse values of vertices
       while (correct && !nl->IsEmpty(rest)) {
         firstElem = nl->First(rest);
         rest = nl->Rest(rest);

         if (nl->ListLength(firstElem) != 2)
           correct = false;
         else {
           first = nl->First(firstElem);
           second = nl->Second(firstElem);

           if (!(nl->IsAtom(first) && (nl->AtomType(first) == IntType)))
             correct = false;
           else {
             p = (Point*)InPoint( nl->TheEmptyList(),
                                  second, 0, errorInfo, correct ).addr;
             if (correct) {
               correct = graph->AddVertex(nl->IntValue(first),*p);
               delete p;
             }
           }
         }
       }

       // parse values of edges
       firstElem = nl->Empty();
       rest = edgesList;

       while (correct && !nl->IsEmpty(rest)) {
         firstElem = nl->First(rest);
         rest = nl->Rest(rest);

         if (nl->ListLength(firstElem) != 3)
           correct = false;
         else {
           first = nl->First(firstElem);
           second = nl->Second(firstElem);
           third = nl->Third(firstElem);

           if (!(nl->IsAtom(first) && (nl->AtomType(first) == IntType)
                && nl->IsAtom(second) && (nl->AtomType(second) == IntType)
                && nl->IsAtom(third) && (nl->AtomType(third) == RealType) &&
                   (nl->RealValue(third) >= 0)))
             correct = false;
           else
             correct = graph->AddEdge(nl->IntValue(first),
                                      nl->IntValue(second),
                                      nl->RealValue(third));
         }
       }

       if (!correct)
         cout << "Graph is invalid!" << endl;
     }

   }
   else if (listutils::isSymbolUndefined(instance)) {

     graph->SetDefined(false);
     correct = true;
   }

   if (correct)
     return SetWord(graph);

   delete graph;
   return SetWord(Address(0));
}

/*
4.5 ~Create~-function

*/
Word CreateGraph( const ListExpr typeInfo ) {

    return SetWord(new Graph(true));
}


/*
4.6 ~Delete~-function

*/
void DeleteGraph( const ListExpr typeInfo, Word& w ) {

   Graph* graph = (Graph*)w.addr;

   graph->Destroy();
   graph->DeleteIfAllowed(false);
   w.addr = 0;
}

/*
4.7 ~Close~-function

*/
void CloseGraph( const ListExpr typeInfo, Word& w ) {

  ((Graph*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

/*
4.8 ~Clone~-function

*/
Word CloneGraph( const ListExpr typeInfo, const Word& w ) {

   return SetWord( ((Graph*)w.addr)->Clone() );
}

/*
4.9 ~SizeOf~-function

*/
int SizeOfGraph() {

   return sizeof(Graph);
}

/*
4.11 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~graph~ does not have arguments, this is trivial.

*/

bool CheckGraph( ListExpr type, ListExpr& errorInfo ) {

   return (nl->IsEqual(type,"graph"));
}
/*
4.12 ~Cast~-function

*/
void* CastGraph (void* addr) {

   return (new (addr) Graph);
}

/*
4.13 ~Open~-function

*/
bool
    OpenGraph( SmiRecord& valueRecord,
                    size_t& offset,
                    const ListExpr typeInfo,
                    Word& value )
{
  // This Open function is implemented in the Attribute class
  // and uses the same method of the Tuple manager to open objects
  Graph *bf =
      (Graph*)Attribute::Open( valueRecord, offset, typeInfo );
  value.setAddr( bf );
  return true;
}

/*
4.14 ~Save~-function

*/
bool
    SaveGraph( SmiRecord& valueRecord,
                    size_t& offset,
                    const ListExpr typeInfo,
                    Word& value )
{
  Graph *bf = (Graph *)value.addr;

  // This Save function is implemented in the Attribute class
  // and uses the same method of the Tuple manager to save objects
  Attribute::Save( valueRecord, offset, typeInfo, bf );
  return true;
}


TypeConstructor graphCon(
    "graph",              //name
    GraphProperty,               //property function describing signature
    OutGraph, InGraph,            //Out and In functions
    0, 0,                      //SaveToList and RestoreFromList functions
    CreateGraph, DeleteGraph,      //object creation and deletion
    OpenGraph, SaveGraph, CloseGraph, CloneGraph,
                                    //^^^ object open, save, close, and clone
    CastGraph,              //cast function
    SizeOfGraph,           //sizeof function
    CheckGraph                    //kind checking function
);

/*
Type constructor Path

*/

void* CastPath (void* addr)
{
    return (new (addr) Path);
}

ListExpr OutPath( ListExpr typeInfo, Word value )
{
    Path const * pPath = static_cast<Path const *>(value.addr);
    if (pPath->IsDefined())
    {
        int nCount = pPath->GetNoPathStructs();
        if (nCount == 0)
        {
            return nl->TheEmptyList();
        }
        else
        {

            pathStruct pStruct = pPath->GetPathStruct(0);
            ListExpr result;
            if (pStruct.pos.IsDefined())
            {
                result = nl->OneElemList(nl->TwoElemList(
                    nl->IntAtom(pStruct.key), nl->TwoElemList(
                    nl->RealAtom(pStruct.pos.GetX()),
                    nl->RealAtom(pStruct.pos.GetY()))));
            }
            else
            {
                result = nl->OneElemList(nl->TwoElemList(
                    nl->IntAtom(pStruct.key),
                                          nl->SymbolAtom(Symbol::UNDEFINED())));
            }

            ListExpr last = result;
            for (int i = 1; i < nCount; ++i)
            {
                last = nl->Append(last, nl->RealAtom(pStruct.cost));
                pStruct = pPath->GetPathStruct(i);
                if (pStruct.pos.IsDefined())
                {
                    last = nl->Append(last, nl->TwoElemList(
                        nl->IntAtom(pStruct.key), nl->TwoElemList(
                        nl->RealAtom(pStruct.pos.GetX()),
                        nl->RealAtom(pStruct.pos.GetY()))));
                }
                else
                {
                    last = nl->Append(last, nl->TwoElemList(
                        nl->IntAtom(pStruct.key),
                                          nl->SymbolAtom(Symbol::UNDEFINED())));
                }
            }

            return result;
        }
    }
    else
    {
        return nl->SymbolAtom(Symbol::UNDEFINED());
    }

}

Word InPath( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{
    Path* pPath = new Path(true);
    correct = true;
    if (listutils::isSymbolUndefined(instance))
    {
        pPath->SetDefined(false);
    }
    else if (!nl->IsEmpty(instance))
    {
        map<EdgeDirection, float> mapUsedEdges;
        map<int, Point> mapUsedVertices;

        pathStruct ps;
        ListExpr first = nl->First(instance);
        ListExpr rest = nl->Rest(instance);

        if (nl->ListLength(first) == 2)
        {
            ListExpr First = nl->First(first);
            ListExpr Second = nl->Second(first);
            if (nl->IsAtom(First) && nl->AtomType(First) == IntType)
            {
                ps.key = nl->IntValue(First);
                if (nl->ListLength(Second) == 2)
                {
                    First = nl->First(Second);
                    Second = nl->Second(Second);
                    if (nl->IsAtom(First)
                        && nl->AtomType(First) == RealType &&
                        nl->IsAtom(Second) &&
                        nl->AtomType(Second) == RealType)
                    {
                        ps.pos = Point(true, nl->RealValue(First),
                            nl->RealValue(Second));
                    }
                    else
                    {
                        correct = false;
                    }
                }
                else if (listutils::isSymbolUndefined(Second))
                {
                    ps.pos = Point(false);
                }
                else
                {
                    correct = false;
                }
            }
            else
            {
                correct = false;
            }
        }
        else
        {
            correct = false;
        }

        mapUsedVertices.insert(pair<int, Point>(ps.key, ps.pos));

        while (!nl->IsEmpty(rest) && correct)
        {
            first = nl->First(rest);
            rest = nl->Rest(rest);

            if (nl->IsAtom(first) && nl->AtomType(first) == RealType &&
                !nl->IsEmpty(rest))
            {
                ps.cost = nl->RealValue(first);
                pPath->Append(ps);
            }
            else
            {
                correct = false;
            }

            if (ps.cost < 0.0)
            {
                cout << "Negative costs are not allowed!" << endl;
                correct = false;
            }

            if (correct)
            {
                first = nl->First(rest);
                rest = nl->Rest(rest);
                if (nl->ListLength(first) == 2)
                {
                    ListExpr First = nl->First(first);
                    ListExpr Second = nl->Second(first);
                    if (nl->IsAtom(First) && nl->AtomType(First) == IntType)
                    {
                        int nLastKey = ps.key;
                        ps.key = nl->IntValue(First);
                        EdgeDirection edge(nLastKey, ps.key);
                        map<EdgeDirection, float>::const_iterator itEdge =
                            mapUsedEdges.find(edge);
                        if (itEdge == mapUsedEdges.end())
                        {
                            mapUsedEdges.insert(
                                pair<EdgeDirection, float>(edge, ps.cost));
                        }
                        else if (ps.cost != (*itEdge).second)
                        {
                            cout << "Parallel borders are not allowed" << endl;
                            correct = false;
                        }

                        if (nl->ListLength(Second) == 2)
                        {
                            First = nl->First(Second);
                            Second = nl->Second(Second);
                            if (nl->IsAtom(First) &&
                                nl->AtomType(First) == RealType &&
                                nl->IsAtom(Second) &&
                                nl->AtomType(Second) == RealType)
                            {
                                ps.pos = Point(true, nl->RealValue(First),
                                    nl->RealValue(Second));
                            }
                            else
                            {
                                correct = false;
                            }
                        }
                        else if (listutils::isSymbolUndefined(Second))
                        {
                            ps.pos = Point(false);
                        }
                        else
                        {
                            correct = false;
                        }

                        if (correct)
                        {
                            map<int, Point>::const_iterator itVertex =
                                mapUsedVertices.find(ps.key);
                            if (itVertex == mapUsedVertices.end())
                            {
                                mapUsedVertices.insert(
                                    pair<int, Point>(ps.key, ps.pos));
                            }
                            else
                            {
                                Point const & rPos2 = (*itVertex).second;
                                if (ps.pos.IsDefined())
                                {
                                    if (rPos2.IsDefined())
                                    {
                                        correct = ps.pos == rPos2;
                                    }
                                    else
                                    {
                                        correct = false;
                                    }
                                }
                                else if (rPos2.IsDefined())
                                {
                                    correct = false;
                                }
                                if (!correct)
                                {
                                    cout << "Vertex " << ps.key <<
                                        " mustn't have different positions!"
                                        << endl;
                                }
                            }
                        }
                    }
                    else
                    {
                        correct = false;
                    }
                }
                else
                {
                    correct = false;
                }
            }
        }

        //Append the last vertex with cost 0.0
        ps.cost = 0.0;
        pPath->Append(ps);
    }

    if (!correct)
    {
        delete pPath;
        pPath = NULL;
    }

    return SetWord(pPath);
}

ListExpr PathProperty()
{
    return (nl->TwoElemList(
        nl->FiveElemList(nl->StringAtom("Signature"),
            nl->StringAtom("Example Type List"),
            nl->StringAtom("List Rep"),
            nl->StringAtom("Example List"),
            nl->StringAtom("Remarks")),
        nl->FiveElemList(nl->StringAtom("-> DATA"),
            nl->StringAtom("path"),
            nl->StringAtom("(<fromV> <cost> <toV> ... <cost> <toV>)"),
            nl->StringAtom("((1 (1.0 2.0)) 0.5 (2 (2.0 3.0)))"),
            nl->StringAtom("fromV, toV: vertex, cost: float"))));
}

Word CreatePath( const ListExpr typeInfo )
{
    return SetWord(new Path(true));
}

void DeletePath( const ListExpr typeInfo, Word& w )
{
    Path * pPath = static_cast<Path *>(w.addr);
    pPath->Destroy();
    pPath->DeleteIfAllowed(false);
    w.addr = 0;

}

void ClosePath( const ListExpr typeInfo, Word& w )
{
    static_cast<Path *>(w.addr)->DeleteIfAllowed();
    w.addr = 0;

}

Word ClonePath( const ListExpr typeInfo, const Word& w )
{
    return SetWord((static_cast<Path const *>(w.addr))->Clone());
}

int SizeofPath()
{
    return sizeof(Path);
}


bool CheckPath( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual(type, "path"));
}

bool OpenPath(SmiRecord& valueRecord, size_t& offset,
    const ListExpr typeInfo, Word& value)
{
    value.setAddr(Attribute::Open(valueRecord, offset, typeInfo));
    return true;
}


bool SavePath( SmiRecord& valueRecord, size_t& offset,
    const ListExpr typeInfo, Word& value)
{
    Attribute::Save(valueRecord, offset, typeInfo,
        static_cast<Attribute*>(value.addr));
    return true;
}



TypeConstructor pathCon(
    "path",                 //name
    PathProperty,           //property function describing signature
    OutPath, InPath,        //Out and In functions
    0, 0,                   //SaveToList and RestoreFromList functions
    CreatePath, DeletePath, //object creation and deletion
    OpenPath, SavePath,     //object open, save
    ClosePath, ClonePath,   //object close, and clone
    CastPath,               //cast function
    SizeofPath,             //sizeof function
    CheckPath);             //kind checking function


/*
3 Type Constructor edge

*/

void* CastEdge (void* addr)
{
  return (new (addr) Edge);
}

ListExpr OutEdge( ListExpr typeInfo, Word value )
{
  Edge const * pEdge = static_cast<Edge const *>(value.addr);
  if (pEdge->IsDefined())
  {
    return nl->ThreeElemList(nl->IntAtom(pEdge->GetSource()),
      nl->IntAtom(pEdge->GetTarget()),
      nl->RealAtom(pEdge->GetCost()));
  }
  else
  {
    return nl->SymbolAtom("undef");
  }
}

Word InEdge( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if (nl->ListLength(instance) == 3)
  {
    ListExpr first = nl->First(instance);
    ListExpr second = nl->Second(instance);
    ListExpr third = nl->Third(instance);

    if (nl->IsAtom(first) && nl->AtomType(first) == IntType &&
      nl->IsAtom(second) && nl->AtomType(second) == IntType &&
      nl->IsAtom(third) && nl->AtomType(third) == RealType)
    {
      float fCost = nl->RealValue(third);
      if (fCost >= 0.0)
      {
        correct = true;
        return SetWord(new Edge(nl->IntValue(first),
          nl->IntValue(second), fCost));
      }
      else
      {
        cout << "Negative costs are not allowed!" << endl;
      }
    }
    else
    {
      correct = false;
    }
  }
  else if (nl->AtomType(instance) == SymbolType &&
    nl->SymbolValue(instance) == "undef")
  {
    correct = true;
    Edge* pEdge = new Edge;
    pEdge->SetDefined(false);
    return SetWord(pEdge);
  }

  correct = false;
  return SetWord(Address(0));
}

ListExpr EdgeProperty()
{
  return (nl->TwoElemList(
      nl->FiveElemList(nl->StringAtom("Signature"),
        nl->StringAtom("Example Type List"),
      nl->StringAtom("List Rep"),
      nl->StringAtom("Example List"),
      nl->StringAtom("Remarks")),
    nl->FiveElemList(nl->StringAtom("-> DATA"),
        nl->StringAtom("edge"),
      nl->StringAtom("(<source> <target> <cost>)"),
      nl->StringAtom("(1 2 1.0)"),
      nl->StringAtom("source, target: int; cost: float."))));
}

Word
CreateEdge( const ListExpr typeInfo )
{
    return SetWord(new Vertex(0, 0, 1.0));
}

void
DeleteEdge( const ListExpr typeInfo, Word& w )
{
  static_cast<Edge *>(w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void
CloseEdge( const ListExpr typeInfo, Word& w )
{
  DeleteEdge(typeInfo, w);
}

Word
CloneEdge( const ListExpr typeInfo, const Word& w )
{
  return SetWord((static_cast<Edge const *>(w.addr))->Clone());
}

int
SizeofEdge()
{
  return sizeof(Edge);
}


bool
CheckEdge( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual(type, "edge"));
}


bool OpenEdge( SmiRecord& valueRecord,
  size_t& offset, const ListExpr typeInfo, Word& value )
{
  value = SetWord(Attribute::Open(valueRecord, offset, typeInfo));
  return true;
}


bool SaveEdge( SmiRecord& valueRecord,
  size_t& offset, const ListExpr typeInfo, Word& value )
{
  Attribute::Save(valueRecord, offset, typeInfo,
    static_cast<Attribute*>(value.addr));
  return true;
}


TypeConstructor edgeCon(
  "edge",                 //name
  EdgeProperty,          //property function describing signature
    OutEdge, InEdge,        //Out and In functions
    0, 0,                   //SaveToList and RestoreFromList functions
  CreateEdge, DeleteEdge, //object creation and deletion
    OpenEdge, SaveEdge,     //object open, save
  CloseEdge, CloneEdge,   //object close, and clone
  CastEdge,               //cast function
    SizeofEdge,             //sizeof function
  CheckEdge);             //kind checking function


/*
Type constructor vertex

*/

void* CastVertex (void* addr)
{
    return (new (addr) Vertex);
}

ListExpr OutVertex( ListExpr typeInfo, Word value )
{
    Vertex const * pVertex = static_cast<Vertex const *>(value.addr);
    if (pVertex->IsDefined())
    {
        if (pVertex->GetPos().IsDefined())
        {
            return nl->TwoElemList(nl->IntAtom(pVertex->GetKey()),
                nl->TwoElemList(nl->RealAtom(pVertex->GetPos().GetX()),
                nl->RealAtom(pVertex->GetPos().GetY())));
        }
        else
        {
            return nl->TwoElemList(nl->IntAtom(pVertex->GetKey()),
                nl->SymbolAtom(Symbol::UNDEFINED()));

        }
    }
    else
    {
        return nl->SymbolAtom(Symbol::UNDEFINED());
    }
}

Word InVertex( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{
    if (nl->ListLength(instance) == 2)
    {
        ListExpr first = nl->First(instance);
        ListExpr second = nl->Second(instance);

        correct = true;
        int nKey = 0;
        if (nl->IsAtom(first) && nl->AtomType(first) == IntType)
        {
            nKey = nl->IntValue(first);
        }
        else
        {
            correct = false;
        }

        Coord coordX = 0.0;
        Coord coordY = 0.0;
        if (!nl->IsAtom(second) && nl->ListLength(second) == 2)
        {
            first = nl->First(second);
            second = nl->Second(second);
            if (nl->IsAtom(first) && nl->AtomType(first) == RealType &&
                nl->IsAtom(second) && nl->AtomType(second) == RealType)
            {
                coordX = nl->RealValue(first);
                coordY = nl->RealValue(second);
            }
            else
            {
                correct = false;
            }
            if (correct)
            {
                return SetWord(new Vertex(nKey, coordX, coordY));
            }
        }
        else if (listutils::isSymbolUndefined(second))
        {
            if (correct)
            {
                Point pnt = Point(false);
                return SetWord(new Vertex(nKey, pnt));
            }
        }
        else
        {
            correct = false;
        }
    }
    else if (listutils::isSymbolUndefined(instance))
    {
        correct = true;
        Vertex* pVertex = new Vertex;
        pVertex->SetDefined(false);
        return SetWord(pVertex);
    }

    correct = false;
    return SetWord(Address(0));
}

ListExpr VertexProperty()
{
    return (nl->TwoElemList(
        nl->FiveElemList(nl->StringAtom("Signature"),
            nl->StringAtom("Example Type List"),
            nl->StringAtom("List Rep"),
            nl->StringAtom("Example List"),
            nl->StringAtom("Remarks")),
        nl->FiveElemList(nl->StringAtom("-> DATA"),
            nl->StringAtom(Vertex::BasicType()),
            nl->StringAtom("(key (<x> <y>))"),
            nl->StringAtom("(1 (-3.0 15.3))"),
            nl->StringAtom("key: int; x, y: float."))));
}

Word CreateVertex( const ListExpr typeInfo )
{
    return SetWord(new Vertex(0, 0.0, 0.0));
}

void DeleteVertex( const ListExpr typeInfo, Word& w )
{
    static_cast<Vertex *>(w.addr)->DeleteIfAllowed();
    w.addr = 0;
}

void CloseVertex( const ListExpr typeInfo, Word& w )
{
    DeleteVertex(typeInfo, w);
}

Word CloneVertex( const ListExpr typeInfo, const Word& w )
{
    return SetWord((static_cast<Vertex const *>(w.addr))->Clone());
}

int SizeofVertex()
{
    return sizeof(Vertex);
}


bool CheckVertex( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual(type, Vertex::BasicType()));
}


bool OpenVertex( SmiRecord& valueRecord,
    size_t& offset, const ListExpr typeInfo, Word& value )
{
    value.setAddr(Attribute::Open(valueRecord, offset, typeInfo));
    return true;
}


bool SaveVertex( SmiRecord& valueRecord,
    size_t& offset, const ListExpr typeInfo, Word& value )
{
    Attribute::Save(valueRecord, offset, typeInfo,
        static_cast<Attribute*>(value.addr));
    return true;
}


TypeConstructor vertexCon(
    Vertex::BasicType(),                   //name
    VertexProperty,             //property function describing signature
    OutVertex, InVertex,        //Out and In functions
    0, 0,                       //SaveToList and RestoreFromList functions
    CreateVertex, DeleteVertex, //object creation and deletion
    OpenVertex, SaveVertex,     //object open and save
    CloseVertex, CloneVertex,   //object close, and clone
    CastVertex,                 //cast function
    SizeofVertex,               //sizeof function
    CheckVertex);               //kind checking function




/*
5 Creating the Algebra

*/

class GraphAlgebra : public Algebra
{
 public:
  GraphAlgebra() : Algebra()
  {
    AddTypeConstructor( &vertexCon );
    AddTypeConstructor( &edgeCon );
    AddTypeConstructor( &graphCon );
    AddTypeConstructor( &pathCon );
    vertexCon.AssociateKind(Kind::DATA());
    edgeCon.AssociateKind(Kind::DATA());
    graphCon.AssociateKind(Kind::DATA());
    pathCon.AssociateKind(Kind::DATA());

    AddOperator(&theVertex);
    AddOperator(&maxDegree);
    AddOperator(&minDegree);
    AddOperator(&circle);
    AddOperator(&connectedComp);

    AddOperator(&shortestPath);
    AddOperator(&edges);
    AddOperator(&vertices);
    AddOperator(&partOf);

    AddOperator(&graph_key);
    AddOperator(&graph_pos);
    AddOperator(&graph_source);
    AddOperator(&graph_target);
    AddOperator(&graph_cost);
    AddOperator(&graph_placenodes);
    AddOperator(&graph_merge);
    AddOperator(&constGraph);
    AddOperator(&constGraphPoints);
    AddOperator(&equalop);
    AddOperator(&equalway);

  }
  ~GraphAlgebra() {};
};

/*
6 Initialization

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
InitializeGraphAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new GraphAlgebra());
}
