/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#ifndef SECONDO_LCOMPOSE_H
#define SECONDO_LCOMPOSE_H

#include <AlgebraTypes.h>
#include <Operator.h>
#include <QueryProcessor.h>

//Uncomment this if you want no logging
#define LCOMPOSE_LOG

//#define USE_CGAL_LIBRARY

namespace routeplanningalgebra {
  /*
   * Logger class used in LCompose
   */
  class Logger {
  public:
    Logger(unsigned level) : lvl(level) {}

    template<typename T>
    Logger &operator<<(const T &x) {
      if (level >= lvl) {
        std::cout << x;
      }
      return *this;
    }

    const unsigned lvl;
#ifdef LCOMPOSE_LOG
    const static unsigned level = 2;
#else
    const static unsigned level = 0;
#endif
  };

  /*
  Logger types / levels
  */
  extern Logger MSG;
  extern Logger ERROR;
  extern Logger WARN;
  extern Logger INFO;
  extern Logger DEBUG;
  extern Logger DETAIL;
  extern Logger DETAIL2;

  /*
  ~lcompose~ operator

  Calculates a height function of an sline over a surface given by a stream of
  pointclouds.
  It therefore reads the stream and reduces the whole set of points to a minimum
  by first filtering the points that are close to the sline. It then
  pre-processes that point set, further removing redundant points while trying
  to keep quality of the topography.
  It then builds a TIN from those points and maps the sline (2D) on the TIN (3D)
  This results in a mapping of a set of intervals to a set of linear functions
  which is the returned lreal object.

  The operator takes one other parameter, which has to be a real 0<=p<=1.
  This parameter dictates how many points are kept when reducing the point set.
  A higher value means less points removed, thus (possibly) decreasing the
  granularity and also increasing the resolution of the resulting lreal
  functions.

  Unfortunately there are many possible sources for errors:
  1. If the stream of Pointclouds and the slines do not or barely overlap,
     there is no way of calculating accurate height functions. The user
     therefore has to provide fitting data.
     If the sline overlaps the region of the point set partially, heights
     outside will be estimated, resulting in a valid lreal, yet the values are
     by no means accurate.
  2. In the present state of this and the TinAlgebra, if you choose the
     precision parameter too big (> 0.1) points may be too close together which
     results in maths errors in the calculation of the Tin, thus throwing an
     error that can't be recovered and leaving memory allocated until the
     termination of the program.
     !! This may cause your system to be out of memory, since lcompose is
     usually run a lot of times (on the sline of each tuple of a relation)
     !! If you choose then to terminate the program, it may leave Secondo or
     your database in an invalid state.

     To check run lcompose of a subset of the tuples by appending 'head[100]' to
     the tuple stream.

  There are plenty of (developer) options in the .cpp file that alter the
  functionality of the operator. You can activate them with the definition of
  the following guards:
  1. USE_CGAL_LIBRARY (in this file (LCompose.h))
     Use better pre-processing on the point set.
  2. DEBUG_MODE
     Use randomly generated points instead the pointcloud stream.
  2.1STATIC_NET (requires DEBUG_MODE defined)
     Use hardcoded set of points.
  3. EXAMPLE_DATA
     Instead of constructing the Tin, just calculate a lreal value for the sline
     depending on the (x, y) position of the sline's segments's points.
  4. ADJUST_PRECISION
     Adjust the precision parameter for low point density regions.

  sline x stream(pointcloud) x real -> lreal
  */
  class LCompose {
  public:
    /*
    Type Mapping
    */
    static ListExpr lcomposeTM(ListExpr args);

    /*
    Value Mapping
    */
    static int lcomposeVM(Word *args,
                          Word &result,
                          int message,
                          Word &local,
                          Supplier s);

    /*
    Specification
    */
    static OperatorSpec lcomposeSpec;

    /*
    Operator instance
    */
    static Operator lcompose;

    /*
    Auxiliary operator to export a Tin based on Pointcloud data
    */
    class PointcloudToTin {
    public:
      /*
      Type Mapping
      */
      static ListExpr pointcloud2TinTM(ListExpr args);

      /*
      Value Mapping
      */
      static int pointcloud2TinVM(Word *args,
                                  Word &result,
                                  int message,
                                  Word &local,
                                  Supplier s);

      /*
      Specification
      */
      static OperatorSpec pointcloud2TinSpec;

      /*
      Operator instance
      */
      static Operator pointcloud2Tin;
    };
  };
}


#endif //SECONDO_LCOMPOSE_H