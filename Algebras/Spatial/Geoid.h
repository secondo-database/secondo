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

[1] Class Geoid

March 2011 Christian Duentgen: First implementation


1 Overview

The ~Geoid~ class represents geoids used for spherical geometry.

2 Defines and Includes

*/

#ifndef __GEOID_H__
#define __GEOID_H__


#include <string>
#include <iostream>

#include "NestedList.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "NestedList.h"
#include "GenericTC.h"



/*
3 Class Definition

*/
class Geoid : public Attribute {
  // stream output operator. Implementation in file SpatialAlgebra.cpp
  friend ostream& operator<< (ostream&, const Geoid&);

  public:
    enum GeoidName {UnitSphere, WGS1984, Bessel1841, Krasovsky1940,
                    International1924, GRS1980};
/*
Standard constructor does nothing. DO NOT USE!

*/
    Geoid();

/*
Constructor creates a WGS1984 geoid with given definedness:

*/
    explicit Geoid(bool _defined);

/*
Constructor for arbitrary geoids. ~radius~ (must be positive) and ~flattening~
(must be non-negative).
Otherwise, the an UNDEF geoid is returned!

*/
    Geoid(const string& _name,
          const double _radius,
          const double _flattening);

/*
Constructor for special predefined Geoids.
Unknown ~GeoidName~ results in an UNDEFINED Geoid!

*/
    explicit Geoid(const GeoidName n);

/*
Copy constructor

*/
    Geoid(const Geoid& g);
    ~Geoid();

    Geoid& operator=(const Geoid& g);

/*
Set the Geoid to a predined set of data.

*/
    void setPredefinedGeoid(const GeoidName n);

/*
Functions providing copies of the private data.

*/
    inline string getName() const { return string(name); }
    inline double getR() const { return radius; }
    inline double getF() const { return flattening; }

/*
Function returns a string with the names of all pre-defined geoids. Can be used
in operator specs.

*/
  static string getGeoIdNames();

/*
Get the ~GeoidName~ code for a given geoid name. If the name is unknown, WGS1984
is returned and return parameter ~valid~ is set to ~false~.

*/
  static GeoidName getGeoIdNameFromString(const string& s, bool& valid);

/*
Functions overwriting Attribute functions

*/
  size_t Sizeof() const;

/*
Compare function.
A g1 > g2 if g1's radius is larger OR the radius is equal but the flattening is
smaller OR both are equal, but the name is lexicographically larger:

*/
  int Compare( const Geoid *rhs ) const;
  int Compare( const Attribute *rhs ) const;

/*
Operators for GenericCompare

*/
  bool operator==(const Geoid& other) const;
  bool operator!=(const Geoid& other) const;
  bool operator<(const Geoid& other) const;
  bool operator<=(const Geoid& other) const;
  bool operator>(const Geoid& other) const;
  bool operator>=(const Geoid& other) const;

/*
Instances are NEVER adjacent:

*/
  bool Adjacent( const Attribute *attrib ) const;

/*
Geoid contains no Flobs:

*/
  inline int NumOfFLOBs() const { return 0; }
  inline Flob* GetFLOB( const int i ) { assert(false); return 0; }

/*
Print function

*/
  virtual ostream& Print( ostream& os ) const;

/*
Clone function

*/
  Geoid* Clone() const;

/*
Hash function

*/
  size_t HashValue() const;
  void CopyFrom(const Attribute* arg);

/*
Functions required for using ~GenericTC.h~

*/
  static string BasicType() { return "geoid"; }
  static const bool checkType(const ListExpr type){
     return listutils::isSymbol(type, BasicType());
  }


  static ListExpr Property() {
    return gentc::GenProperty(
      "-> DATA",
      BasicType(),
      "(Name Radius InverseFlattening) | GeoidName | UNDEF",
      "(\"WSG1984\" 6378137.000 298.257223563) | \"Bessel1841\" "
      "| Bessel1841 | UNDEF", "valid GeoidNames are: " + getGeoIdNames() + ". "
      "If GeoidName is unknown, or Radius or InverseFlattening are "
      "non-positive the geoid's value is set to UNDEF. Name and "
      "GeoidName may be SymbolAtom or StringAtom; both Radius and "
      "InverseFlattening may be IntAtom or RealAtom; UNDEF must be "
      "a SymbolAtom with value UNDEF, undef, UNDEFINED or undefined.");
}

  static bool CheckKind(ListExpr type, ListExpr& errorInfo) {
    return nl->IsEqual(type,BasicType());
  }

  ListExpr ToListExpr(const ListExpr typeInfo ) const;
  bool ReadFrom(const ListExpr instance, const ListExpr typeInfo);

  private:
    double radius;        // Equatorial axis (m)
    double flattening;    // Flattenig
    STRING_T name;        // Name of the geoid (cannot use string)
};

#endif // __GEOID_H__
