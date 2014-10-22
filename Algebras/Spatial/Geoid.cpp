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
//[_] [\_]

[1] Implementation of Class Geoid

March 2011 Christian Duentgen: First implementation


1 Overview

The ~Geoid~ class represents geoids used for spherical geometry.

2 Defines and Includes

*/


#include <string>
#include <iostream>
#include <cstring>

#include "NestedList.h"
#include "ListUtils.h"

#include "Attribute.h"
#include "StandardTypes.h"
#include "Geoid.h"

/*
3 Class Definition

Standard constructor does nothing, because the class inherits from ~Attribute~!

*/
Geoid::Geoid() {}

Geoid::Geoid(bool _defined): Attribute(_defined), radius(6378137.000),
                           flattening(1.0/298.257223563){
      strcpy(name, string("WGS1984").c_str());
}

/*
Constructor for arbitrary geoids. ~radius~ (positive) and ~flattening~
(non-negative). Otherwise, the an UNDEF geoid is returned!

*/
Geoid::Geoid(const string& _name,
          const double _radius,
          const double _flattening) : Attribute(true),  radius(_radius),
                                      flattening(_flattening)
{
      if(    (_flattening >= 0)                   // non-negative flattening
          && (_radius>0)                          // positive radius
          && ( _flattening <= 1.0)                // equatorial r >= axial r
          && (_name.length()<=MAX_STRINGSIZE)) {
        strcpy(name, _name.c_str());
      } else { // use UNDEFINED as standard geoid
        strcpy(name, string("UNDEFINED").c_str());
        radius = 1.0;
        flattening = 0.0;
        SetDefined(false);
      }
}

/*
Constructor for special predefined Geoids.
Unknown ~GeoidName~ results in an UNDEFINED Geoid!

*/
Geoid::Geoid(const GeoidName n) : Attribute(true) {
      setPredefinedGeoid(n);
}

/*
Copy constructor

*/
Geoid::Geoid(const Geoid& g): Attribute(true), radius(g.radius),
                           flattening(g.flattening){
      strcpy(name,g.name);
}

Geoid::~Geoid(){}

Geoid& Geoid::operator=(const Geoid& g) {
      SetDefined(g.IsDefined());
      strcpy(name, g.name);
      radius = g.radius;
      flattening = g.flattening;
      return *this;
}

/*
Set the Geoid to a predined set of data.

*/
void Geoid::setPredefinedGeoid(const GeoidName n){
      SetDefined(true);
      switch(n)
      {
        case UnitSphere:
        { strcpy(name,string("UnitSphere").c_str());
          radius = 1.0;
          flattening = 0.0;
          break;
        }
        case WGS1984:
        { strcpy(name,string("WGS1984").c_str());
          radius = 6378137.000;
          flattening = 1.0/298.257223563;
          break;
        }
        case Bessel1841:
        { strcpy(name,string("Bessel1841").c_str());
          radius = 6377397.155;
          flattening = 1.0/299.1528434;
          break;
        }
        case Krasovsky1940:
        { strcpy(name,string("Krasovsky1940").c_str());
          radius = 6378245.0;
          flattening = 1.0/298.3;
          break;
        }
        case International1924 :
        { strcpy(name,string("International1924").c_str());
          radius = 6378388.0;
          flattening = 1.0/297.0;
          break;
        }
        case GRS1980 :
        { strcpy(name,string("GRS1980").c_str());
          radius = 6378137;
          flattening = 1.0/298.257222101;
          break;
        }
        default :
        {
          strcpy(name,string("UNDEFINED").c_str());
          radius = 1.0;
          flattening = 0.0;
          SetDefined(false);
        }
      }
}


/*
Function returns a string with the names of all pre-defined geoids. Can be used
in operator specs.

*/
string Geoid::getGeoIdNames(){
  return "{UnitSphere, WGS1984, Bessel1841, Krasovsky1940, International1924, "
         "GRS1980}";
}

/*
Get the ~GeoidName~ code for a given geoid name. If the name is unknown,
UnitSphere is returned and return parameter ~valid~ is set to ~false~.

*/
Geoid::GeoidName Geoid::getGeoIdNameFromString(const string& s, bool& valid){
      valid = true;
      if( s == "UnitSphere"){
        return UnitSphere;
      }
      if (s == "WGS1984" ){
        return WGS1984;
      }
      if (s == "Bessel1841" ){
        return Bessel1841;
      }
      if (s == "Krasovsky1940" ){
        return Krasovsky1940;
      }
      if (s == "International1924" ){
        return International1924;
      }
      if (s == "GRS1980" ){
        return GRS1980;
      }
      valid = false;
      return UnitSphere;
}

/*
Functions overwriting Attribute functions

*/
size_t Geoid::Sizeof() const {return sizeof(*this);}

/*
Compare function.
A g1 greater g2 if g1's radius is larger OR the radius is equal but the flattening is
smaller OR both are equal, but the name is lexicographically larger:

*/
int Geoid::Compare( const Geoid *rhs ) const {
    if(!IsDefined() && !rhs->IsDefined()){ return 0; }
    if(!IsDefined() && rhs->IsDefined()){ return -1; }
    if(IsDefined() && !rhs->IsDefined()){ return 1; }
    if(radius > rhs->radius) { return 1; }
    if(radius < rhs->radius) { return -1; }
    if(flattening < rhs->flattening) { return 1; }
    if(flattening > rhs->flattening) { return -1; }
    int cmp = strcmp(name, rhs->name);
    if(cmp < -1) { return -1; }
    if(cmp >  1) { return 1; }
    return cmp;
}

int Geoid::Compare( const Attribute *rhs ) const {
    return Compare((Geoid*)rhs);
}

/*
Operators for GenericCompare

*/
bool Geoid::operator==(const Geoid& other) const {
    return (Compare(&other) == 0);
}

bool Geoid::operator!=(const Geoid& other) const {
    return (Compare(&other) != 0);
}

bool Geoid::operator<(const Geoid& other) const {
    return (Compare(&other) < 0);
}

bool Geoid::operator<=(const Geoid& other) const {
    return (Compare(&other) <= 0);
}

bool Geoid::operator>(const Geoid& other) const {
    return (Compare(&other) > 0);
}

bool Geoid::operator>=(const Geoid& other) const {
    return (Compare(&other) >= 0);
}

/*
Instances are NEVER adjacent:

*/
bool Geoid::Adjacent( const Attribute *attrib ) const { return false; }

/*
Print function

*/
inline ostream& Geoid::Print( ostream& os ) const {
    if(IsDefined()){
      return os << "(Geoid: Name=" << name << ", radius=" << radius
                << ", flattening=" << flattening << ")";
    } else {
      return os << "(Geoid: UNDEFINED)";
    }
}

/*
Clone function

*/
Geoid* Geoid::Clone() const {
    Geoid* res = new Geoid(false);
    res->SetDefined(this->IsDefined());
    if(IsDefined()){
      strcpy(res->name, name);
      res->radius = radius;
      res->flattening = flattening;
    }
    return res;
}

/*
Hash function

*/
size_t Geoid::HashValue() const {
    if(!IsDefined()) {
      return (size_t) (0);
    }
    unsigned long h = 0;
    char* s = (char*)&radius;
    for(unsigned int i = 1; i <= sizeof(double) / sizeof(char); i++)
    {
      h = 5 * h + *s;
      s++;
    }
    return (size_t)(h);
}

void Geoid::CopyFrom(const Attribute* arg) {
    *this = *((Geoid*)(arg));
}

/*
Functions required for using ~GenericTC.h~

*/
ListExpr Geoid::ToListExpr(const ListExpr typeInfo ) const {
    if(!IsDefined()) {
      return nl->SymbolAtom("UNDEF");
    }
    return nl->ThreeElemList(nl->StringAtom(string(name)),
                              nl->RealAtom(radius),
                              nl->RealAtom(flattening));
}

bool Geoid::ReadFrom(const ListExpr instance, const ListExpr typeInfo) {
    SetDefined(false);
    strcpy(name,string("UNDEFINED").c_str());
    radius = 1.0;
    flattening = 0.0;
    if(    nl->IsEqual(instance,"undef")
        || nl->IsEqual(instance,"UNDEF")
        || nl->IsEqual(instance,"undefined")
        || nl->IsEqual(instance,"UNDEFINED") ){
      return true;
    }
    string geoidnameStr = "";
    if(nl->IsAtom(instance)){ // single symbolAtom: predefined Geoid
      if(nl->AtomType(instance)==StringType) {
        geoidnameStr = nl->StringValue(instance);
      } else if(nl->AtomType(instance)==SymbolType){
        geoidnameStr = nl->SymbolValue(instance);
      } else {
        return false;
      }
      bool valid = false;
      GeoidName geoidname = getGeoIdNameFromString(geoidnameStr, valid);
      if(valid){
        setPredefinedGeoid(geoidname);
      } else {
        SetDefined(false);
      }
      return valid;
    }
    if(nl->ListLength(instance)==3){ // user defined geoid
      string _name = "UNDEFINED";
      double _radius = 1.0;
      double _flattening = 0.0;
      // 1. element: name (accepts StringAtom or SymbolAtom)
      ListExpr first = nl->First(instance);
      if(!nl->IsAtom(first)) { return false; }
      if(listutils::isSymbol(first)){
        _name = nl->SymbolValue(first);
      } else if(nl->IsAtom(first) && (nl->AtomType(first)==StringType)){
        _name =  nl->StringValue(first);
      } else { // error
        return false;
      }
      // 2. element: radius (accepts IntAtom or RealAtom)
      ListExpr second = nl->Second(instance);
      if(!listutils::isNumeric(second)){
        return false;
      }
      _radius = listutils::getNumValue(second);
      // 3. element: inverse flattening (accepts IntAtom or RealAtom)
      ListExpr third = nl->Third(instance);
      if(!listutils::isNumeric(third)){
        return false;
      }
      _flattening = listutils::getNumValue(third);
      // check values
      if((_radius <= 0.0) || (_flattening < 0.0)){
        return false;
      }
      SetDefined(true);
      strcpy(name, _name.c_str());
      radius = _radius;
      flattening = _flattening;
      return true;
    } // else: list has wrong length -> error
    SetDefined(false);
    return false;
}

/*
4 Overloaded output operator

Implementation of stream output operator for ~Geoid~ objects

*/
ostream& operator<<( ostream& o, const Geoid& g ){
  ios_base::fmtflags oldOptions = o.flags();
  o.setf(ios_base::fixed,ios_base::floatfield);
  o.precision(16);
  g.Print(o);
  o.flags(oldOptions);
  return o;
}

