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
#include "tree.cpp"
#include "vertex.cpp"
#include "edge.cpp"
#include "graph.cpp"
#include "path.cpp"

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
     
     if (nl->IsEqual(arg1, "graph") && nl->IsEqual(arg2, "int"))
       return nl->SymbolAtom("vertex");
     if ((nl->AtomType(arg1) == SymbolType) &&
         (nl->AtomType(arg2) == SymbolType))
       ErrorReporter::ReportError(
                "Type mapping function got parameters of type "+
                nl->SymbolValue(arg1)+" and "+nl->SymbolValue(arg2));
     else
       ErrorReporter::ReportError(
                "Type mapping function got wrong types as parameters.");
   }
   else
     ErrorReporter::ReportError(
                "Type mapping function got a parameter of length != 2.");
   
   return nl->SymbolAtom("typeerror");
}


/*
4.1.2 Type mapping function for operator ~maxDegree~ and ~minDegree~

graph $\to$ int

*/
ListExpr minMaxDegreeMap ( ListExpr args ) {

   if (nl->ListLength(args) == 2) {
   
     ListExpr arg1 = nl->First(args);
     ListExpr arg2 = nl->Second(args);
     
     if (nl->IsEqual(arg1, "graph") && nl->IsEqual(arg2, "bool"))
       return nl->SymbolAtom("int");
     if ((nl->AtomType(arg1) == SymbolType) &&
         (nl->AtomType(arg2) == SymbolType))
       ErrorReporter::ReportError(
              "Type mapping function got parameters of type "+
              nl->SymbolValue(arg1)+" and "+nl->SymbolValue(arg2));
     else
       ErrorReporter::ReportError(
              "Type mapping function got wrong types as parameters.");
   }
   else
     ErrorReporter::ReportError(
              "Type mapping function got a parameter of length != 2.");
   
   return nl->SymbolAtom("typeerror");
}


/*
4.1.3 Type mapping function for operator ~connectedComponents~

graph $\to$ stream(tuple([Graph: graph]))

*/
ListExpr connectedCompMap ( ListExpr args ) {

   if (nl->ListLength(args) == 1) {
   
     ListExpr arg1 = nl->First(args);
     
     if (nl->IsEqual(arg1, "graph"))
       return nl->TwoElemList(
                   nl->SymbolAtom("stream"),
                   nl->TwoElemList(
                        nl->SymbolAtom("tuple"),
                        nl->OneElemList(
                             nl->TwoElemList(
                                  nl->SymbolAtom("Graph"),
                                  nl->SymbolAtom("graph")
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
   
   return nl->SymbolAtom("typeerror");
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

        if (!nl->IsEqual(arg1, "graph")){
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
        if( (arg2s != "vertex") && (arg2s!="int")){
            ErrorReporter::ReportError(
                 "graph x {int, vertex} x {int, vertex} expected");
            return nl->TypeError();
        }
        if( (arg3s != "vertex") && (arg3s!="int")){
            ErrorReporter::ReportError(
                 "graph x {int, vertex} x {int, vertex} expected");
            return nl->TypeError();
        }
        return nl->SymbolAtom("path");


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

        if (nl->IsEqual(arg1, "path") || nl->IsEqual(arg1, "graph"))
            return nl->TwoElemList(
                nl->SymbolAtom("stream"),
                nl->TwoElemList(
                    nl->SymbolAtom("tuple"),
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

   return nl->SymbolAtom("typeerror");
}


/*
4.1.6 Type mapping function for operator ~vertices~

graph $\to$ stream(tuple([Vertex: vertex]))

path $\to$ stream(tuple([Vertex: vertex]))

*/
ListExpr verticesMap ( ListExpr args ) {
    if (nl->ListLength(args) == 1) {

        ListExpr arg1 = nl->First(args);

        if (nl->IsEqual(arg1, "path") || nl->IsEqual(arg1, "graph"))
            return nl->TwoElemList(
                nl->SymbolAtom("stream"),
                nl->TwoElemList(
                    nl->SymbolAtom("tuple"),
                    nl->OneElemList(
                        nl->TwoElemList(
                            nl->SymbolAtom("Vertex"),
                            nl->SymbolAtom("vertex")
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

   return nl->SymbolAtom("typeerror");
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
      CHECK_COND(nl->ListLength(args) == 3,
      "Operator constGraph expects a list of length three.");    
      inTuple = nl->First(args);
      attidx1  = nl->Second(args);
      attidx2  = nl->Third(args);
      break;
  case 1:    
      CHECK_COND(nl->ListLength(args) == 4,
      "Operator constGraph expects a list of length four.");    
      inTuple = nl->First(args);
      attidx1  = nl->Second(args);
      attidx2  = nl->Third(args);
       func  = nl->Fourth(args);
       break;
    case 2:    
      CHECK_COND(nl->ListLength(args) == 5,
      "Operator constGraph expects a list of length five.");    
      inTuple = nl->First(args);
      attidx1  = nl->Second(args);
      attidx2  = nl->Third(args);
       pointidx1  = nl->Fourth(args);
       pointidx2  = nl->Fifth(args);
       break;
    case 3:
      CHECK_COND(nl->ListLength(args) == 6,
      "Operator constGraph expects a list of length six.");    
      inTuple = nl->First(args);
      attidx1  = nl->Second(args);
      attidx2  = nl->Third(args);
       func  = nl->Fourth(args);
       pointidx1  = nl->Fifth(args);
       pointidx2= nl->Sixth(args);
       break;
    }
  nl->WriteToString(argstr, inTuple);
  CHECK_COND(nl->ListLength(inTuple) == 2  &&
         (TypeOfRelAlgSymbol(nl->First(inTuple)) == stream) &&
    (nl->ListLength(nl->Second(inTuple)) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(nl->Second(inTuple))) == tuple) &&
         (nl->ListLength(nl->Second(inTuple)) == 2) &&
         (IsTupleDescription(nl->Second(nl->Second(inTuple)))),
      "Operator constGraph expects as first argument a list with structure "
      "(stream (tuple ((a1 t1)...(an tn))))\n"
      "Operator constgraph gets as first argument '" + argstr + "'." );


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
          argstr +", "+argstr2+
          "' as attributenames.\n"
          "Atrribute name may not be the name of a Secondo object!");
        return nl->SymbolAtom("typeerror");
    }  
    j = FindAttribute(nl->Second(nl->Second(inTuple)), attrname1, attrtype);
    if (j)  
    {
      nl->WriteToString(argstr, attrtype);       
      CHECK_COND(argstr=="int",
        "Attribute "+attrname1+" is of type "+argstr+" but schould be int."); 
    }
    else
    {
        nl->WriteToString( argstr, nl->Second(nl->Second(inTuple)) );
        ErrorReporter::ReportError(
          "Attributename '" + attrname1 + "' is not known.\n"
          "Known Attribute(s): " + argstr);
        return nl->SymbolAtom("typeerror");
    }
     k = FindAttribute(nl->Second(nl->Second(inTuple)), attrname2, attrtype);
    if (k)     
    {
      nl->WriteToString(argstr, attrtype);       
      CHECK_COND(argstr=="int",
        "Attribute "+attrname1+" is of type "+argstr+" but schould be int.");
    }
    else
    {
        nl->WriteToString( argstr, nl->Second(nl->Second(inTuple)) );
        ErrorReporter::ReportError(
          "Attributename '" + attrname2 + "' is not known.\n"
          "Known Attribute(s): " + argstr);
        return nl->SymbolAtom("typeerror");
    }
    if(characteristic==1||characteristic==3)
    {         
      if ( nl->IsAtom(func)
          || !nl->ListLength(func) == 3
           || !nl->IsEqual(nl->First(func), "map")
           || !nl->IsEqual(nl->Third(func), "real") )
      {
          nl->WriteToString(argstr, func);
          ErrorReporter::ReportError("Operator filter expects a "
               "(map tuple real) as its second argument. "
               "The second argument provided "
               "has type '" + argstr + "' instead.");
          return nl->SymbolAtom("typeerror");
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
         nl->WriteToString(argstr, attrtype);       
         CHECK_COND(argstr=="point",
          "Attribute "+attrname1+" is of type "+
           argstr+" but schould be point.");                  
      }
      else
      {
          nl->WriteToString( argstr, nl->Second(nl->Second(inTuple)) );
          ErrorReporter::ReportError(
            "Attributename '" + attrname1 + "' is not known.\n"
            "Known Attribute(s): " + argstr);
          return nl->SymbolAtom("typeerror");
      }
       m = FindAttribute(nl->Second(nl->Second(inTuple)), attrname2, attrtype);
      if (m)     
      {
        nl->WriteToString(argstr, attrtype);       
         CHECK_COND(argstr=="point",
          "Attribute "+attrname1+" is of type "+argstr+
          " but schould be point.");
      }
      else
      {
          nl->WriteToString( argstr, nl->Second(nl->Second(inTuple)) );
          ErrorReporter::ReportError(
            "Attributename '" + attrname2 + "' is not known.\n"
            "Known Attribute(s): " + argstr);
          return nl->SymbolAtom("typeerror");
      }
      return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
           nl->FourElemList(nl->IntAtom(j),nl->IntAtom(k),
                            nl->IntAtom(l),nl->IntAtom(m)),           
            nl->SymbolAtom("graph"));  
  }        
   return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
     nl->TwoElemList(nl->IntAtom(j),nl->IntAtom(k)),           
        nl->SymbolAtom("graph"));
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
     
     if (nl->IsEqual(arg1, "graph") &&
         nl->IsEqual(arg2, "vertex") && 
         nl->IsEqual(arg3, "real"))
       return nl->SymbolAtom("graph");
     
     if ((nl->AtomType(arg1) == SymbolType) && 
         (nl->AtomType(arg2) == SymbolType) && 
         (nl->AtomType(arg3) == SymbolType))
       ErrorReporter::ReportError(
              "Type mapping function got parameters of type "
              +nl->SymbolValue(arg1)+
              ", " +nl->SymbolValue(arg2)+ " and "+nl->SymbolValue(arg3));
     else
       ErrorReporter::ReportError(
              "Type mapping function got wrong types as parameters.");
   }
   else
     ErrorReporter::ReportError(
             "Type mapping function got a parameter of length != 3.");
   
   return nl->SymbolAtom("typeerror");
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
    if (nl->IsEqual(arg, "graph"))
    {
      return nl->SymbolAtom("graph");
    }
    else
    {
      ErrorReporter::ReportError(
        "Type mapping function got paramater of type " +
        nl->SymbolValue(arg));
    }
  }
  else
  {
    ErrorReporter::ReportError(
      "Type mapping function got a parameter of length != 1.");
  }
  return nl->SymbolAtom("typeerror");
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
    if (nl->IsEqual(arg1, "graph") && nl->IsEqual(arg2, "graph"))
    {
      return nl->SymbolAtom("graph");
    }
    else
    {
      ErrorReporter::ReportError(
        "Type mapping function got paramaters of type " +
        nl->SymbolValue(arg1) + " and " + nl->SymbolValue(arg2));
    }
  }
  else
  {
    ErrorReporter::ReportError(
      "Type mapping function got a parameter of length != 2.");
  }
  return nl->SymbolAtom("typeerror");
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
    if (nl->IsEqual(arg1, "graph") && nl->IsEqual(arg2, "graph"))
    {
      return nl->SymbolAtom("bool");
    }
    else
    {
      ErrorReporter::ReportError(
        "Type mapping function got paramaters of type " +
        nl->SymbolValue(arg1) + " and " + nl->SymbolValue(arg2));
    }
  }
  else
  {
    ErrorReporter::ReportError(
        "Type mapping function got a parameter of length != 2.");
  }
  return nl->SymbolAtom("typeerror");
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
   
   if (nl->IsEqual(arg,"path"))
     return (0);
   if (nl->IsEqual(arg,"graph"))
     return (1);
   return (-1);
}


/*
4.2.2 Selection function for the ~shortestPath~ Operator 

*/
int shortestPathSelect(ListExpr args){
   string arg2 = nl->SymbolValue(nl->Second(args));
   string arg3 = nl->SymbolValue(nl->Third(args));
   if(arg2=="vertex" && arg3=="vertex") return 0;
   if(arg2=="vertex" && arg3=="int") return 1;
   if(arg2=="int" && arg3=="vertex") return 2;
   if(arg2=="int" && arg3=="int") return 3;
   return -1;
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
     
        localInfo = ((SccStruct*)local.addr);
        delete localInfo->tType;
        delete localInfo;
        
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

            localInfo = ((EdgeStruct*) local.addr);
            delete localInfo->tType;
            delete localInfo;

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

            localInfo = ((VertexStruct*) local.addr);
            delete localInfo->tType;
            delete localInfo;

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
    res =new Graph(true);    
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
    result =SetWord(res);
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
4.3.13 Value mapping function of operator ~equal~

*/
int equalfun(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Graph* pGraph1 = (Graph*)(args[0].addr);
  Graph* pGraph2 = (Graph*)(args[1].addr);
  result = qp->ResultStorage(s);
  ((CcBool*)result.addr)->Set(true,pGraph1->EqualsWith(pGraph2));          

  //todo
  
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
     "( <text>graph -> stream graph</text---> "
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
     "( <text>path -> stream edge, graph -> stream edge</text---> "
       "<text>edges ( _ )</text--->"
       "<text>Returns a tuple stream of all edges of the path or"
       " graph in ascending order.</text--->"
       "<text>query edges(p1) consume</text--->"
       ") )";

const string SpecVertices  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>path -> stream vertex, graph -> stream edge</text---> "
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

const string equalSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
   "\"Example\" ) "
    "( <text>(graph x graph) -> bool </text---> "
    "<text>g1 equal g2</text--->"
    "<text>Check if two graphes are equal</text--->"
    "<text> query g1 equal g1; results TRUE</text--->"
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

Operator equals(
  "equal", 
  equalSpec, 
  equalfun, 
  Operator::SimpleSelect, 
  GraphGraphBoolTypeMap
);
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
    vertexCon.AssociateKind("DATA");  
    edgeCon.AssociateKind("DATA"); 
    graphCon.AssociateKind("DATA");   
    pathCon.AssociateKind("DATA");
    
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
    AddOperator(&equals);
  }
  ~GraphAlgebra() {};
};

GraphAlgebra graphAlgebra;

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
  return (&graphAlgebra);
}
