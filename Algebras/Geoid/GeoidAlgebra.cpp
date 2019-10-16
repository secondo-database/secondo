/*
----
This file is part of SECONDO.

Copyright (C) 2019, 
University in Hagen, 
Faculty of Mathematics and  Computer Science,
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
//[_] [\_]

1 Implementation of the Geoid Algebra

2 Defines and Includes

*/

#include <string>

#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Geoid.h"

using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;

/*
3 Type Constructor ~geoid~

*/
GenTC<Geoid> geoid_t;


/*
4 operators with geoids

*/


/*
Type Mapping: string [ x {real|int} x {real|int} ] [->] geoid
For operator: create\_geoid

*/
ListExpr geoid_create_geoid_TM(ListExpr args){
  int noargs = nl->ListLength(args);
  string errmsg = "Expected ("+CcString::BasicType()+" [ x {"
                +CcInt::BasicType()+"|"+CcReal::BasicType()
                +"} x {"+CcInt::BasicType()+"|"+CcReal::BasicType()+"} ]).";
  // allow only 1 or 3 arguments:
  if( (noargs<1)|| (noargs==2)  || (noargs>3) ){
    return listutils::typeError(errmsg);
  }
  // check first argument
  if(!listutils::isSymbol(nl->First(args), CcString::BasicType())){
    return listutils::typeError(errmsg);
  }
  // check 2. + 3. argument
  if(noargs==3){
    if(    !listutils::isNumericType(nl->Second(args))
        || !listutils::isNumericType(nl->Third(args)) ){
      return listutils::typeError(errmsg);
    }
  }
  return nl->SymbolAtom(Geoid::BasicType());
}

/*
Type Mapping: geoid [->] real
For operators: getRadius, getFlattening

*/
ListExpr geoid2real_TM(ListExpr args){
  int noargs = nl->ListLength(args);
  string errmsg = "Expected ("+Geoid::BasicType()+").";
  if(noargs!=1){
    return listutils::typeError(errmsg);
  }
  if(listutils::isSymbol(nl->First(args), Geoid::BasicType())){
    return nl->SymbolAtom(CcReal::BasicType());
  }
  return listutils::typeError(errmsg);
}


/*
Operations on geoids

*/

int geoid_getRadius_VM(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  CcReal* res = static_cast<CcReal*>(result.addr);
  Geoid* arg = static_cast<Geoid*>(args[0].addr);
  if(!arg->IsDefined()){
    res->Set(false, 0.0);
  } else {
    res->Set(true, arg->getR());
  }
  return 0;
}

int geoid_getFlattening_VM(Word* args, Word& result, int message,
                        Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  CcReal* res = static_cast<CcReal*>(result.addr);
  Geoid* arg = static_cast<Geoid*>(args[0].addr);
  if(!arg->IsDefined()){
    res->Set(false, 0.0);
  } else {
    res->Set(true, arg->getF());
  }
  return 0;
}

template<class T1, class T2>
int geoid_create_geoid_VM(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  Geoid* res = static_cast<Geoid*>(result.addr);
  CcString* nameCcStr = static_cast<CcString*>(args[0].addr);
  T1* radiusT1 = 0;
  T2* flatteningT2 = 0;
  int noargs = qp->GetNoSons(s);
  if(!nameCcStr->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  string name = nameCcStr->GetValue();
  double radius = 1.0;
  double flattening = 0.0;
  if(noargs==3){
    radiusT1 = static_cast<T1*>(args[1].addr);
    flatteningT2 = static_cast<T2*>(args[2].addr);
    if(!radiusT1->IsDefined() || !flatteningT2->IsDefined()){
      res->SetDefined(false);
      return 0;
    }
    radius = radiusT1->GetValue();
    flattening = flatteningT2->GetValue();
    if( (radius <= 0.0) || (flattening < 0.0) ){
      res->SetDefined(false);
      return 0;
    }
    Geoid g(name, radius, flattening);
    res->CopyFrom(&g);
    return 0;
  }
  bool valid = false;
  Geoid::GeoidName gc = Geoid::getGeoIdNameFromString(name, valid);
  if(!valid){
    res->SetDefined(false);
    return 0;
  }
  res->setPredefinedGeoid(gc);
  return 0;
}

ValueMapping geoid_create_geoid_vm[] = {
  geoid_create_geoid_VM<CcReal,CcReal>, // (string)
  geoid_create_geoid_VM<CcReal,CcReal>, // (string x real x real)
  geoid_create_geoid_VM<CcReal,CcInt>,  // (string x real x int)
  geoid_create_geoid_VM<CcInt,CcReal>,  // (string x int x real)
  geoid_create_geoid_VM<CcInt,CcInt>    // (string x int x int)
};

int geoid_create_geoid_SELECT(ListExpr args){
  if(nl->ListLength(args)==1){ return 0; }
  ListExpr first = nl->Second(args);
  ListExpr second = nl->Third(args);
  if(    listutils::isSymbol(first,CcReal::BasicType())
      && listutils::isSymbol(second,CcReal::BasicType())){ return 1; }
  if(    listutils::isSymbol(first,CcReal::BasicType())
      && listutils::isSymbol(second,CcInt::BasicType())){ return 2; }
  if(    listutils::isSymbol(first,CcInt::BasicType())
      && listutils::isSymbol(second,CcReal::BasicType())){ return 3; }
  if(    listutils::isSymbol(first,CcInt::BasicType())
      && listutils::isSymbol(second,CcInt::BasicType())){ return 4; }
  return -1;
}

/*
Operations on geoids

*/
const string geoid_getRadius_SPEC  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text>geoid -> real</text--->"
   "<text> getRadius( G )  </text--->"
   "<text>Returns the radius parameter of geoid G.</text--->"
   "<text>query getRadius([const geoid value WGS1984])</text--->"
   ") )";

const string geoid_getFlattening_SPEC  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text>geoid -> real</text--->"
   "<text> getFlattening( G )  </text--->"
   "<text>Returns the flattening parameter of geoid G.</text--->"
   "<text>query getFlattening([const geoid value UnitSphere]) = 0.0</text--->"
   ") )";

const string geoid_create_geoid_SPEC  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text> string [ x {real|int} x {real|int} -> geoid</text--->"
   "<text> create_geoid( Name [, Radius, Flattening] )  </text--->"
   "<text>Returns a geoid. If only the string parameter Name is used, it must "
   "be the name of a predefined geoid (available: "+Geoid::getGeoIdNames()+"). "
   "Otherwise, Radius must be the positive radius, and 0.0 <= Flattening <= 1.0"
   "is the flattening parameter (0.0 results a perfect sphere).</text--->"
   "<text>query create_geoid(\"MyGeoid\", 1.0, 0.5)</text--->"
   ") )";

Operator geoid_getRadius
(
  "getRadius",
  geoid_getRadius_SPEC,
  geoid_getRadius_VM,
  Operator::SimpleSelect,
  geoid2real_TM
);

Operator geoid_getFlattening
(
  "getFlattening",
  geoid_getFlattening_SPEC,
  geoid_getFlattening_VM,
  Operator::SimpleSelect,
  geoid2real_TM
);

Operator geoid_create_geoid
(
  "create_geoid",
  geoid_create_geoid_SPEC,
  5,
  geoid_create_geoid_vm,
  geoid_create_geoid_SELECT,
  geoid_create_geoid_TM
);


/*
5 Creating the Algebra

*/

class GeoidAlgebra : public Algebra
{
 public:
  GeoidAlgebra() : Algebra()
  {
    AddTypeConstructor(&geoid_t);
    geoid_t.AssociateKind(Kind::DATA());

    AddOperator( &geoid_getRadius );
    AddOperator( &geoid_getFlattening );
    AddOperator( &geoid_create_geoid );


  }
  ~GeoidAlgebra() {};
};

/*
12 Initialization

*/

extern "C"
Algebra*
InitializeGeoidAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new GeoidAlgebra());
}
