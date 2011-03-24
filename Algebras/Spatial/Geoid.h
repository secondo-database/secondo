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

#include <iostream>
#include <string>

/*
3 Class Definition

*/
class Geoid {
  public:
    enum GeoidName {WGS1984, Bessel1841, Krasovsky1940,
                    International1924, GRS1980};
/*
Standard constructor returns the  WGS84 geoid.

*/
    Geoid() : name("WGS84"), radius(6378137.000), iflat(298.257223563),
      flattening(1.0/298.257223563){}
/*
Constructor for arbitrary geoids. ~radius~ and ~iflat~ both must be positive.
Otherwise, the WGS84 geoid is returned!

*/
    Geoid(const string& _name,
          const double _radius,
          const double,
          const double _iflat) : name(_name), radius(_radius), iflat(_iflat),
          flattening(0.0)
    {
      if( (iflat > 0) && (radius>0) ) {
        flattening = 1.0/iflat;
      } else { // use WGS84 as standard geoid
        name = "WGS1984";
        radius = 6378137.000;
        iflat = 298.257223563;
        flattening = 1.0/298.257223563;
      }
    }
/*
Constructor for special, named geoids. If the passed geoid name is unknown,
the WGS84 geoid is returned instead.

*/
    Geoid(const GeoidName n) {
      switch(n)
      {
        case WGS1984:
        { name = "WGS1984";
          radius = 6378137.000;
          iflat = 298.257223563;
          flattening = 1.0/298.257223563;
          break;
        }
        case Bessel1841:
        { name = "Bessel1841";
          radius = 6377397.155;
          iflat = 299.1528434;
          flattening = 1.0/299.1528434;
          break;
        }
        case Krasovsky1940:
        { name = "Krasovsky1940";
          radius = 6378245.0;
          iflat = 298.3;
          flattening = 1.0/298.3;
          break;
        }
        case International1924 :
        { name = "International1924";
          radius = 6378388.0;
          iflat = 297.0;
          flattening = 1.0/297.0;
          break;
        }
        case GRS1980 :
        { name = "GRS1980";
          radius = 6378137;
          iflat = 298.257222101;
          flattening = 1.0/298.257222101;
          break;
        }
        default :
        {
          assert(false);
        }
      }
    }

    Geoid(const Geoid& g): name(g.name), radius(g.radius),
                           iflat(g.iflat), flattening(g.flattening){}
    ~Geoid(){}

    Geoid& operator=(const Geoid& g) {
      name = g.name;
      radius = g.radius;
      iflat = g.iflat;
      flattening = g.flattening;
      return *this;
    }
/*
Functions providing copies of the private data.

*/
    string getName() const { return name; }
    double getR() const { return radius; }
    double getIFlat() const { return iflat; }
    double getF() const { return flattening; }

/*
Function returns a string with the names of all pre-defined geoids. Can be used
in operator specs.

*/
    static string getGeoIdNames()
    {
      return "{WGS1984, Bessel1841, Krasovsky1940, International1924, GRS1980}";
    }

/*
Get the ~GeoidName~ code for a given geoid name. If the name is unknown, WGS1984
is returned and return parameer ~valid~ is set to ~false~.

*/
    static GeoidName getGeoIdNameFromString(const string& s, bool& valid){
      valid = true;
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
      return WGS1984;
    }

  private:
    string name;          // Name of the geoid
    double radius;        // Equatorial axis (m)
    double iflat;         // Inverted flattening 1/f
    double flattening;    // Flattenig
};
/*
4 Overloaded output operator

Implemented in file ~SpatialAlgebra.cpp~

*/
ostream& operator<<( ostream& o, const Geoid& g );

#endif // __GEOID_H__
