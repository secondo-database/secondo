/*
----
This file is part of SECONDO.

Copyright (C) 2016,
Faculty of Mathematics and Computer Science,
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
//paragraph [2] Centered: [\begin{center}] [\end{center}]
//[_] [\_]

//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

[1] Header of a column oriented spatial algebra

[2] May 2017 by Sascha Radke for bachelor thesis

\setcounter{tocdepth}{2}
\tableofcontents

1 Overview

This is the implementation of the classes ~ColPoint~, ~ColLine~ and ~ColRegion~.
They are column orientated memory representations of point arrays,
line arrays and region arrays. The corresponding Secondo types are
~apoint~, ~aline~ and ~aregion~.

Also the operators on these types are defined.

2 Includes and global variables and classes

*/

// TODO: remove relative path

#include "../../include/Symbols.h"          // predefined strings
#include "../../include/ListUtils.h"        // useful functions for nested lists
#include "../../include/NestedList.h"       // required at many places
#include "../../include/QueryProcessor.h"   // needed for value mappings
#include "../../include/TypeConstructor.h"  // constructor for Secondo Types
#include "../Spatial/SpatialAlgebra.h"      // spatial types and operators
#include "../Stream/Stream.h"               // wrapper for secondo streams
//#include "../CRel/AttrArray.h"              // column oriented relations

class ColPoint;
class ColLine;
class ColRegion;

using std::string;

namespace col {

/*
3 Implementation of column-oriented spatial types

In this chapter the former mentioned spatial types are implemented.

3.1 Global variables

In the array ~allocBytes~ are listed several steps of memory allocation as
they are used in the ~In~ function of each spatial type.

*/
long allocBytes[19] = {     0,   //   [0]  initial value
                        16384,   //   [1]  32 KB L1 data cache
                       245760,   //   [2] 256 KB L2 cache
                      1032192,   //   [3]   1 MB L3 cache
                      2080768,   //   [4]   2 MB L3 cache
                      3129344,   //   [5]   3 MB L3 cache
                      6275072,   //   [6]   6 MB L3 cache
                      8257536,   //   [7]   8 MB L3 cache
                     12566528,   //   [8]  12 MB L3 cache
                     16760832,   //   [9]  16 MB L3 cache
                    134201344,   //   [1] 128 MB
                   1073725440,   //  [11]   1 GB
                   2147467264,   //  [12]   2 GB
                   3221209088,   //  [13]   3 GB
                   4294950912,   //  [14]   4 GB
                   7516176384,   //  [15]   7 GB
                   8589918208,   //  [16]   8 GB
                  16106110976,   //  [17]  15 GB
                  17179852800};  //  [18]  16 GB;
/*
3.2 Class ColPoint for column-oriented representation of points

This class handles an array of points to reduce costs of processing.
A point is the smallest unit and is considered as an attribute.
The size of a point is 16 Bytes (each coordinate is a double value of 8 bytes).
All points are stored dense and ascending in an array.

*/
class ColPoint {
/*
In the private section the internal data structure is defined:
The array ~array~ contains all points. It's size in bytes is set in both the
~In~ and the ~Out~ function. The variable ~count~ holds the amount
of points in the array. It is set in the ~In~ Function by counting the incoming
points and in the ~Open~ function by reading it from disc.

*/
 private:
  typedef struct {
    double x;
    double y;
  } point;
  point* array;
  long count;

/*
The public section provides constructors and a destructor of the class
and contains all static functions for the neccessary class operations.

*/
 public:

/*
  constructors AND destructor

*/
  ColPoint();
  ColPoint(point* newArray, long newCount);
  ~ColPoint();
/*
returns the corresponding basic type

*/
  static const string BasicType();

/*
compares the type of the given object with the class type

*/
  static const bool checkType(const ListExpr list);

/*
returns the number of elements in the point array

*/
  long getCount();

/*
returns the address of the point array for external access

*/
void* getArray();

/*
description of the Secondo type for the user

*/
  static ListExpr Property();

/*
The ~In~ function scans a nested-list (parameter ~instance~) and converts it
into an array of points. One way to acquire the length of a nested list
would be to call nl->ListLength(instance), but for a large number of points
this method is very expensive due to a full scan.
An alternative way is implemented in this function. It provides an algorithm
to process an unknown number of elements:
Memory is allocated on a logarithmic scale from 16 KB up to 16 GB trying to
consider known cache-sizes as well as common memory sizes with subtracting
16 KB for control data. The possible allocation sizes are stored in the array
~allocBytes~ which is desfined in the global section of this file.

*/
  static Word In(const ListExpr typeInfo, ListExpr instance,
                 const int errorPos, ListExpr &errorInfo, bool &correct);

/*
The ~Out~ function is the counter part of the ~In~ function. It converts the
internal array of points into the Secondo nested list format.

*/
  static ListExpr Out(ListExpr typeInfo, Word value);

/*
This function creates an object instance having an arbitrary value.
For example it is needed in the operator tree.

*/
  static Word Create(const ListExpr typeInfo);

/*
The ~Delete~ function removes the complete object including disc parts
if there are any.

*/
  static void Delete(const ListExpr typeInfo, Word& w);

/*
The ~Open~ function reads an ~array~ from disc via an ~SmiRecord~. Then it
creates a ColPoint object with the array as parameter.
Because the number of points is also retrieved from the ~SmiRecord~ it
is possible to allocate exact the amount of memory that is really needed.

*/
  static bool Open(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value);

/*
The ~Save~ function stores an array of points into a ~SmiRecord~.
This function has to be symmetrically to the ~Open~ function.

*/
  static bool Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value);

/*
The ~Close~ function removes the main memory part of an object.
In contrast to ~Delete~, the disc part of the object is untouched
(if there is one).

*/
  static void Close(const ListExpr typeInfo, Word& w);

/*
The ~Clone~ function creates a depth copy (inclusive disc parts) of an object.

*/
  static Word Clone(const ListExpr typeInfo, const Word& w);

/*
The ~Cast~ function crerates a new ColPoint object and lets it point to
the given address using a special call of new operator.
The argument points to a memory block which is to cast to the object.

*/
  static void* Cast(void* addr);

/*
The function ~TypeCheck~ tests if a given list corresponds to the class type.
This function is quite similar to the ~checkType~ function the class.

*/
  static bool TypeCheck(ListExpr type, ListExpr& errorInfo);

/*
The function ~SizeOf~ is excpected by the type constructor below. Because there
is no standard size of a ColPoint object, it doesn't return any meaningful
value.

*/
  static int SizeOf();
};  // class ColPoint

/*
Creating a Secondo type feeded with the class functions.

*/
TypeConstructor ColPointTC(
  ColPoint::BasicType(), ColPoint::Property,
  ColPoint::Out, ColPoint::In, 0, 0,
  ColPoint::Create, ColPoint::Delete,
  ColPoint::Open, ColPoint::Save,
  ColPoint::Close, ColPoint::Clone,
  ColPoint::Cast, ColPoint::SizeOf,
  ColPoint::TypeCheck);


/*
3.3 Class ColLine for column-oriented representation of lines

This class handles an array of lines and their column-wise internal
representation. Because the functions are similar to the functions of the
class ColPoint, they are explained only if they diverge.

*/
class ColLine {
 private:
    typedef struct {
    double x1;
    double y1;
    double x2;
    double y2;
  } line;
  line* array;  // contains all lines. Size is set in functions In and Open
  long count;    // amount of lines stored in the array

 public:
  // constructors
  ColLine();
  ColLine(line* newArray, long newCount);
  // destructor
  ~ColLine();

  // returns the corresponding basic type
  static const string BasicType();

  // compares the type of the given object with class type
  static const bool checkType(const ListExpr list);

  // description of the Secondo type for the user
  static ListExpr Property();

  // scans a nested-list and converts it into an array of lines.
  static Word In(const ListExpr typeInfo, ListExpr instance,
                 const int errorPos, ListExpr &errorInfo, bool &correct);

  // converts a line array into a nested list format
  static ListExpr Out(ListExpr typeInfo, Word value);

  // creates a new line object and returns its address
  static Word Create(const ListExpr typeInfo);

  // Removes the complete object including disc parts if there are any
  static void Delete(const ListExpr typeInfo, Word& w);

  // Reads an array from disc via an ~SmiRecord~.
  static bool Open(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value);

  // Saves an array of lines into a SmiRecord.
  static bool Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value);

  static void Close(const ListExpr typeInfo, Word& w);

  static Word Clone(const ListExpr typeInfo, const Word& w);

  static void* Cast(void* addr);

  static bool TypeCheck(ListExpr type, ListExpr& errorInfo);

  static int SizeOf();
};  // class ColLine

/*
Creating a Secondo type feeded with the class functions.

*/
TypeConstructor ColLineTC(
  ColLine::BasicType(), ColLine::Property,
  ColLine::Out, ColLine::In, 0, 0,
  ColLine::Create, ColLine::Delete,
  ColLine::Open, ColLine::Save,
  ColLine::Close, ColLine::Clone,
  ColLine::Cast, ColLine::SizeOf,
  ColLine::TypeCheck);


/*
3.4 Class ColRegion for column-oriented representation of Regions

The Class ~ColRegion~ implements the Secondo datatype ~attrarray(region)~,
in short ~aregion~. It is used to deal with a big amount of regions.
Special attention is paid to an efficient internal representation of data,
so it will be sparse with memory on one hand and allows a quick mass evaluation
by ordering the data dense and ascending on the other hand.
The internal representation differs widely from the datatype ~region~.
And so it works:
The three arrays ~aTuple~, ~aPoint~ and ~aCycle~ are declared.
The array ~aPoint~ contains all points of all regions and the points are stored
in a special order. First come all points of an outer cycle, also called face,
followed by all points of corresponding inner cycles, called holes.
In the sequel there can follow further regions by storing further
outer and inner cycles. The order of points within a cycle is always clockwise.
To mark whether a cycle is a face or a hole, the values in the array ~aCycle~
are signed.
The array ~aCycle~ contains the indices within the array ~aPoint~ at which a
face or a hole starts. Faces are stored with positive, holes with
negative indices. Therefore only the sign decides whether a cycle is a
face or a hole.
Finally the array ~aTuple~ contains the start indices of complete regions
both within the array ~aPoint~ to identify the first point within a region
and the array ~aCycle~ to identify the first cycle of a region.
For each array there is also a counter to provide fast access to the size
of this array.

To ensure that a set of regions is processed correctly by the  ~In~ and
~Out~ functions, any region has to satisfy the following conditions:

  1) Any two cycles of the same region must be disconnected,
     which means that no edges of different cycles can intersect each other
  2) Edges of the same cycle can only intersect in their endpoints,
     but not in their middle points
  3) For a certain face, the holes must be inside the outer cycle
  4) For a certain face, any two holes can not contain each other
  5) Faces must have the outer cycle, but they can have no holes
  6) For a certain cycle, any two vertex can not be the same
  7) Any cycle must be made up of at least 3 edges
  8) It is allowed that one face is inside another
     provided that their edges do not intersect.

  The list representation of a set of regions is

----
  (region1 region2 ... regionn)
  where each region is (face1  face2 ... facen)
  where each face is   (outercycle holecycle1 ... holecyclen)
  where each cycle is  (vertex1 vertex2 ... vertexn)
  where each vertex is a point.

  or

  undef
----

*/
class ColRegion {
 private:

/*
3.5 Defining the structure for attribute arrays:

*/
  typedef struct {    // defines one region - head data
    long indexPoint;  // index of region's first point in the point array
    long indexCycle;  // index of region's first cycle in the cycle array
    double mbrX1;     // minimum bounding rectangle left x coordinate
    double mbrY1;     // minimum bounding rectangle lower y coordinate
    double mbrX2;     // minimum bounding rectangle right x coordinate
    double mbrY2;     // minimum bounding rectangle upper y coordinate
  } sTuple;
  typedef struct {    // one cycle
    long index;       // index of cycle's first point in the point array
  } sCycle;
  typedef struct {    // one point
    double x;
    double y;
  } sPoint;

/*
these arrays build the main internal representation of regions:

*/
  sTuple* aTuple;  // array of regions
  sCycle* aCycle;  // array of cycles
  sPoint* aPoint;  // array of points

/*
the counters indicate the size of the corresponding arrays

*/
  long countTuple;
  long countCycle;
  long countPoint;

 public:

/*
3.6 constructors and destructor

*/
  ColRegion();  // standard constructor doing nothing
  // non-standard constructor initializing the object with parameters
  ColRegion(sTuple* newTuple, sCycle* newCycle, sPoint* newPoint,
            long newCountTuple, long newCountCycle, long newCountPoint);
  ~ColRegion();   // destructor - free allocated menory

/*
returns the corresponding basic type

*/
  static const string BasicType();

/*
compares the type of the given object with class type

*/
  static const bool checkType(const ListExpr list);

/*
description of the Secondo type for the user

*/
  static ListExpr Property();

/*
3.7 Auxiliary functions

The ~clear~ function frees possible former allocated memory to assure a
clean object for the following mapping.

*/
  void clear();

/*
This function appends a region datatype of the spatial algebra
to an attrarray of regions of the column spatial algebra.
It needs the source ~region~ and the destiniation ~aregion~ as parameters.

*/
  void append(Region* region);

/*
The auxiliary function ~showArrays~ prints the contents of the internal arrays,
there sizes and counters to the screen. It is useful during the debugging phase.

*/
  void showArrays(string title);

/*
3.8 Standard functions

The ~inside~ function accepts a Secondo spatial datatype ~apoint~, ~aline~,
~aregion~ as input and checks for each element whether it is
inside the region. if the check is true, the index of this element
is appended to an attribute array of integer. The attribute array is returned.

*/
  int p_inside(const Word value, const long count, Word& attrarray) const;


/*
The ~In~ function scans a nested-list (parameter ~instance~) and converts it
into an internal array representation. Memory is allocated on a logarithmic
scale from 16 KB up to 16 GB, trying to consider known cache-sizes as well as
common memory sizes with subtracting 16 KB for control data. The possible
allocation sizes are stored in the array ~allocBytes~ which is defined in the
global section of this file.

*/
  static Word In(const ListExpr typeInfo, const ListExpr instance,
                 const int errorPos, ListExpr& errorInfo, bool& correct);

/*
The ~Out~ function is the counter part of the ~In~ function. It converts the
internal arrays into the Secondo nested list format.
To do this, a loop over the point array is performed. On every step of
this loop it is checked whether a new cycle, face, hole or region starts,
which can be done by comparing the actual point index to the
next entries in the tuple array and in the cycle array respectively.
if there is a match, the resulting nested list is extended
to the corresponding list element.

*/
  static ListExpr Out(ListExpr typeInfo, Word value);

/*
The ~Create~ function creates an object instance having an arbitrary value.
The ~typeInfo~ argument represents the type of the object and is required for
nested types like tuples or in an operator tree.

*/
  static Word Create(const ListExpr typeInfo);

/*
The ~Delete~ function removes the complete object including disc parts
if there are any.

*/
  static void Delete(const ListExpr typeInfo, Word& w);

/*
The ~Open~ function reads three arrays from disc via an ~SmiRecord~. Then it
creates a ColRegion object with the arrays as parameter.
Because the number of tuples, points and cycles are also retrieved from the
~SmiRecord~ it is possible to allocate exact the amount of memory that is
really needed.

*/
  static bool Open(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value);

/*
The ~Save~ function stores an array of points into a ~SmiRecord~.
This function has to be symmetrically to the ~Open~ function.
The order of counters and arrays are tuples - cycles - points, short "tcp".

*/
  static bool Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value);

/*
The ~Close~ function removes the main memory part of an object.
In contrast to ~Delete~, the disc part of the object is untouched
(if there is one).

*/
  static void Close(const ListExpr typeInfo, Word& w);

/*
The ~Clone~ function creates a depth copy (inclusive disc parts) of an object.

*/
  static Word Clone(const ListExpr typeInfo, const Word& w);

/*
The ~Cast~ function crerates a new ColRegion object and lets it point to
the given address using a special call of new operator.
The argument points to a memory block which is to cast to the object.

*/
  static void* Cast(void* addr);

/*
The function ~TypeCheck~ tests if a given list corresponds to the class type.
This function is quite similar to the ~checkType~ function the class.

*/
  static bool TypeCheck(ListExpr type, ListExpr& errorInfo);

/*
The function ~SizeOf~ is excpected by the type constructor below. Because there
is no standard size of a ColRegion object, it doesn't return any meaningful
value.

*/
  static int SizeOf();
};  // class Colregion

/*
Creating a Secondo type feeded with the class functions.

*/
TypeConstructor ColRegionTC(
  ColRegion::BasicType(), ColRegion::Property,
  ColRegion::Out, ColRegion::In, 0, 0,
  ColRegion::Create, ColRegion::Delete,
  ColRegion::Open, ColRegion::Save,
  ColRegion::Close, ColRegion::Clone,
  ColRegion::Cast, ColRegion::SizeOf,
  ColRegion::TypeCheck);


/*
4 Operator Implementation

Each operator implementation in Secondo contains of a type mapping,
a set of value mappings, a selection function,
a description, and  a creation of an operator instance.

The syntax of the operator is described in the file ~AlgebraName.spec~.
Examples must be given in the file ~AlgebraName.examples~.
Without an example, the operator will be switched off by the Secondo framework.

4.1 Type Mapping

*/
ListExpr insideTM(ListExpr args);

ListExpr mapTM(ListExpr args);


/*
4.2 Value mapping functions

~inside~
This operator checks for each element of an array of points, lines or regions
whether they are inside a given region and returns an AttrArray with the
indizes of all matching points, lines or regions.

*/
int insidePoint (Word* args, Word& result, int message,
                 Word& local, Supplier s);

int insideLine (Word* args, Word& result, int message,
                Word& local, Supplier s);

int insideRegion (Word* args, Word& result, int message,
                  Word& local, Supplier s);

/*
~map~
This operator converts a stream of standard types points, lines or regions
into its corresponding column-oriented type apoint, aline or a region
respectively.

*/

int mapPoint (Word* args, Word& result, int message, Word& local, Supplier s);
int mapLine (Word* args, Word& result, int message, Word& local, Supplier s);
int mapRegion (Word* args, Word& result, int message, Word& local, Supplier s);

/*
4.3 Value Mapping Array and Selection Function

*/
ValueMapping insideVM[] = {insidePoint, insideLine, insideRegion};
ValueMapping mapVM[] = {mapPoint, mapLine, mapRegion};

int insideSelect(ListExpr args);
int mapSelect(ListExpr args);

/*
4.4 Specification

arguments of the ~OperatorSpec~ constructor
description of the type mapping
the syntax of the operator
the operator's meaning
example query.
remark (optional)

*/
OperatorSpec insideSpec(" obj x region -> ints, obj={apoint,aline,aregion}",
                        " _ inside _ ",
                        "attrarray(obj) within region",
                        "query cp1 inside r1");

OperatorSpec mapSpec(" stream(obj) -> aobj, obj={point,line,region}",
                        " mp _ ",
                        "maps a stream of standard type to attributarray",
                        "query r feed mp");

/*
4.5 Operator Instance

*/
Operator insideOp(
  "inside",            // operator's name
  insideSpec.getStr(), // specification
  3,                   // number of Value Mappings
  insideVM,            // value mapping array
  insideSelect,        // selection function
  insideTM);           // type mapping

Operator mapOp(
  "mp",                // operator's name
  mapSpec.getStr(),    // specification
  3,                   // number of Value Mappings
  mapVM,               // value mapping array
  mapSelect,           // selection function
  mapTM);              // type mapping


/*
5 Definition of the Algebra

In this step, a new algebra -- a class derived from the ~Algebra~ class  --
is created. Within the constructor of the algebra, we add the type constructors
and assign the corresponding kinds to the types.
Furthermore, all operators are added to the algebra.

*/
class ColumnSpatialAlgebra : public Algebra {
 public:
  ColumnSpatialAlgebra() : Algebra() {
    AddTypeConstructor(&ColPointTC);
    ColPointTC.AssociateKind(Kind::SIMPLE());

    AddTypeConstructor(&ColLineTC);
    ColLineTC.AssociateKind(Kind::SIMPLE());

    AddTypeConstructor(&ColRegionTC);
    ColRegionTC.AssociateKind(Kind::SIMPLE());

   AddOperator(&insideOp);
   AddOperator(&mapOp);
  }
};

}  // namespace col

/*
6 Initialization of the Algebra

*/
extern "C"
Algebra*
  InitializeColumnSpatialAlgebra(NestedList* nlRef, QueryProcessor* qpRef) {
  return new col::ColumnSpatialAlgebra;
}
