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

[2] June 2017 by Sascha Radke for bachelor thesis

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

// TODO: remove relative paths
// TODO: change type of indices to int64_t

#include "../../include/Symbols.h"          // predefined strings
#include "../../include/ListUtils.h"        // useful functions for nested lists
#include "../../include/NestedList.h"       // required at many places
#include "../../include/QueryProcessor.h"   // needed for value mappings
#include "../../include/TypeConstructor.h"  // constructor for Secondo Types
#include "../Spatial/SpatialAlgebra.h"      // spatial types and operators
#include "../Stream/Stream.h"               // wrapper for secondo streams
#include "../CRel/Ints.h"                   // type for id result list

using std::vector;
using std::string;
using namespace CRelAlgebra;

namespace col {

class ColPoint;
class ColLine;
class ColRegion;


/*
3 Implementation of column-oriented spatial types

In this chapter the former mentioned spatial types are implemented.

3.1 Global variables

In the array ~allocBytes~ are listed several steps of memory allocation as
they are used in the ~In~ function of each spatial type.

*/
long allocBytes[20] = {     0,   //   initial value
                         8192,   // fit in L1 data cache
                        16384,   // fit in L1 data cache
                       245760,   // 256 KB L2 cache
                      1032192,   // fit in L3 cache
                      2080768,   // fit in L3 cache
                      3129344,   // fit in L3 cache
                      6275072,   //   6 MB L3 cache
                      8257536,   //   8 MB L3 cache
                     12566528,   //  12 MB L3 cache
                     16760832,   //  16 MB L3 cache
                    134201344,   // 128 MB
                   1073725440,   //   1 GB
                   2147467264,   //   2 GB
                   3221209088,   //   3 GB
                   4294950912,   //   4 GB
                   7516176384,   //   7 GB
                   8589918208,   //   8 GB
                  16106110976,   //  15 GB
                  17179852800};  //  16 GB maximum, more memory = error

/*
The global inline function ~getCycles~ calls the cpu instruction ~tsc~
of x86/x64 machines and returns the number of passed cycles
since system start.

For it is an unsigned long integer, it can grow very big without
any overflow. E. g. on a system with 3 GHz core clock speed it lasts
more than 233 years before the counter turns to 0.

Clock cycles are useful to compare systems with different clock speeds
and can be used for low level benchmarking.

Another result of this function is the time in nanoseconds passed
since system start.

*/
inline void benchmark(long &cycles, long &ns);

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
The array ~aPoint~ contains all points. It's size in bytes is set in both the
~In~ and the ~Out~ function. The variable ~count~ holds the amount
of points in the array. It is set in the ~In~ Function by counting the incoming
points and in the ~Open~ function by reading it from disc.

*/
 private:

  typedef struct {
    double x;
    double y;
  } sPoint;
  sPoint* aPoint;  // array of points
  long count;  // number of points
  long step;  // index to ~allocBytes~ amount of next allocated memory

/*
The public section provides constructors and a destructor of the class
and contains all static functions for the neccessary class operations.

*/
 public:

/*
  Standard constructor

*/
  ColPoint();

/*
This is the main constructor. it initializes the internal point array
and the counter given by parameters.

*/
  ColPoint(sPoint* newArray, long newCount);

/*
Another constructor with minimum initialized array size and counters set
to 0. It is need for the map operator. The parameter ~min~ defines a
new signature regardless of it's value.

*/
  ColPoint(bool min);

/*
Destructor:

*/
  ~ColPoint();

/*
~BasicType~ returns "apoint":

*/
  static const string BasicType();

/*
~checkType~ compares the type of the given object with the class type:

*/
  static const bool checkType(const ListExpr list);

/*
Description of the Secondo type ~apoint~ for the user:

*/
  static ListExpr Property();

/*
~getCount~ returns the number of elements in the point array:

*/
  long getCount();

/*
~getX~ returns the x coordinate of the indexed point entry:

*/
  double getX(long index);

/*
~getY~ returns the y coordinate of the indexed point entry:

*/
  double getY(long index);

/*
The function ~getArrayAddress~ returns the address of the ~aPoint~ array.
It is used for the block copy in the ~merge~ function.

*/

  void* getArrayAddress();

/*
The following function appends one point of the spatial algebra to the
array apoint of the column spatial algebra:

*/
  bool append(Point* point);

/*
The function ~finalize~ reallocates the memory used for the array ~apoint~
to the real needed bytes:

*/
  void finalize();

/*
The ~merge~ function expects two ~apoint~ objects as input parameters
and creates a new ~apoint~ object with the contents of both input objects.
It is called by the ~+~-operator.

*/
  bool merge(ColPoint* cPoint1, ColPoint* cPoint2);

/*
Shows the content of the array aPoint. This function is useful for debugging:

*/
  void showArray(string title);

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
After all values are safely stored, the oversized memory is truncated.

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
~Clone~ makes a copy of a ColPoint object. Therefore it allocates memory
for the clone object and scans the array of the source object and copies
each entry to the clone object.

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
This function is quite similar to the ~checkType~ function.

*/
  static bool TypeCheck(ListExpr type, ListExpr& errorInfo);

/*
The function ~SizeOf~ is excpected by the type constructor below. Because there
is no access to a special object, it only returns the size of an empty ~apoint~.

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

This class handles an array of lines and their internal
representation. A line consists of consecutive coherent segments.
Therefore two arrays are needed. The first array contains all segments
of all lines. The segments of one line has to be consecutive. The second
array contains the indizes of the first segment of each line.

*/
class ColLine {

private:
  typedef struct {
    double x1;
    double y1;
    double x2;
    double y2;
  } sSegment;
  typedef struct {
    long index;
  } sLine;

/*
Definition of the line- and segment arrays and their counters.
The real used memory is allocated in the ~In~, ~append~ and ~Clone~ functions.

*/
  sLine* aLine;
  sSegment* aSegment;

  long countLine;
  long countSegment;

/*
The following two variables are indices for the array ~allocBytes~.
They mark which amount of memory is allocated so far (see before).

*/
  long stepLine;
  long stepSegment;


 public:

/*
  Standard constructor

*/
  ColLine();

/*
Main constructor initializes internal arrays ~aLine~ and ~aSegment~ as well
as their counters.

*/
  ColLine(sLine* newLine, sSegment* newSegment,
          long newCountLine, long newCountSegment);
/*
Another constructor with minimum initialized array size and counters set
to 0. It is need for the map operator. the paramerter ~min~ is used to
create a different signature. It's value doesn't matter.

*/
  ColLine(bool min);

/*
Destructor

*/
  ~ColLine();

/*
Returns the corresponding basic type:

*/
  static const string BasicType();

/*
Compares the type of the given object with class type:

*/
  static const bool checkType(const ListExpr list);

/*
Description of the Secondo type ~aline~ for the user:

*/
  static ListExpr Property();

/*
returns the number of elements in the line array:

*/
  long getCount();

/*
Returns the total number of segments in the segments array:

*/
  long getSegments();

/*
Returns the number of segmets for a single line:

*/
  long getSegments(long index);

/*
Returns the indices of the first and last segment of a single line:

*/
  void getLineSegments(long index, long &first, long &last);

/*
Returns the coordinates of a single segment:

*/
  void getSegmentPoints(long index,
                        double &x1, double &y1, double &x2, double &y2);

/*
Returns the address of the first element of the ~aLine~ array.
This is needed for the block copy of the ~merge~ funtion.

*/
  void *getLineAddress();

/*
Returns the address of the first element of the ~aLine~ array.
This is needed for the block copy of the ~merge~ funtion.

*/
  void* getSegmentAddress();

/*
The function ~createLine~ extracts a line from the aline and
stores it in a standard spatial type,
which is returned as parameter. Fills the given spatial type line with
the data found in the column spatial type aline at the given index.

*/
  bool createLine(Line* line, long index);

/*
The ~append~ function appends a line datatype of the spatial algebra
to an attrarray of lines of the column spatial algebra.
It needs the source ~line~ as input parameter.

*/
  bool append(Line* line);

/*
The ~finalize~ function appends a terminator to each array of the aregion type.
The result is fully generated ColLine object that can be used further on.

*/
  void finalize();

/*
The ~merge~ function expects two ~aline~ objects as input parameters
and creates a new ~aline~ object with the contents of both input objects.
It is called by the ~+~-operator.

*/
  bool merge(ColLine* cLine1, ColLine* cLine2);

/*
The auxiliary function ~showArrays~ prints the contents of the internal arrays,
there sizes and counters to the screen.
It is useful during the debugging phase.

*/
  void showArrays(string title);

/*
Scans a nested-list and converts it into an array of lines:

*/
  static Word In(const ListExpr typeInfo, ListExpr instance,
                 const int errorPos, ListExpr &errorInfo, bool &correct);

/*
Converts a line array into a nested list format:

*/
  static ListExpr Out(ListExpr typeInfo, Word value);

/*
the standard funtions for the ~ColPoint~ object (analogical with ~ColPoint~)

*/
  static Word Create(const ListExpr typeInfo);

  static void Delete(const ListExpr typeInfo, Word& w);

  static bool Open(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value);

  static bool Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value);

  static void Close(const ListExpr typeInfo, Word& w);

  static Word Clone(const ListExpr typeInfo, const Word& w);

  static void* Cast(void* addr);

  static bool TypeCheck(ListExpr type, ListExpr& errorInfo);

/*
The function ~SizeOf~ is excpected by the type constructor below. Because there
is no access to a special object, it only returns the size of an empty ~aline~.

*/
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
The three arrays ~aRegion~, ~aPoint~ and ~aCycle~ are declared.
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
Finally the array ~aRegion~ contains the start indices of complete regions
both within the array ~aPoint~ to identify the first point within a region
and the array ~aCycle~ to identify the first cycle of a region.
Additional the minimum bounding rectangle of a region is stored.
For each array there is also a counter to provide fast access to the size
of this array.

To ensure that a set of regions is processed correctly by the  ~In~ and
~Out~ functions, any region has to satisfy the following conditions:

  1 Any two cycles of the same region must be disconnected,
    which means that no edges of different cycles can intersect each other

  2 Edges of the same cycle can only intersect in their endpoints,
    but not in their middle points

  3 For a certain face, the holes must be inside the outer cycle

  4 For a certain face, any two holes can not contain each other

  5 Faces must have the outer cycle, but they can have no holes

  6 For a certain cycle, any two vertex can not be the same

  7 Any cycle must be made up of at least 3 edges

  8 It is allowed that one face is inside another
    provided that their edges do not intersect.

The list representation of a set of regions is

  * (region\_1 region\_2 ... region\_n)

  * where each region is (face\_1  face\_2 ... face\_n)

  * where each face is   (outercycle holecycle\_1 ... holecycle\_n)

  * where each cycle is  (vertex\_1 vertex\_2 ... vertex\_n)

  * where each vertex is a point.

*/
class ColRegion {
 private:

/*
Defining the structure for attribute arrays:

*/
  typedef struct {    // defines one region - head data
    long indexPoint;  // index of region's first point in the point array
    long indexCycle;  // index of region's first cycle in the cycle array
    double mbrX1;     // minimum bounding rectangle left x coordinate
    double mbrY1;     // minimum bounding rectangle lower y coordinate
    double mbrX2;     // minimum bounding rectangle right x coordinate
    double mbrY2;     // minimum bounding rectangle upper y coordinate
  } sRegion;
  typedef struct {    // one cycle
    long index;       // index of cycle's first point in the point array
  } sCycle;
  typedef struct {    // one point
    double x;
    double y;
  } sPoint;

/*
These arrays build the main internal representation of regions:

*/
  sRegion* aRegion;  // array of regions
  sCycle* aCycle;    // array of cycles
  sPoint* aPoint;    // array of points

/*
The counters indicate the size of the corresponding arrays:

*/
  long countRegion;
  long countCycle;
  long countPoint;

/*
The following step - variables mark the indices in the ~allocBytes~ - array
(see before).

*/
  long stepRegion;
  long stepCycle;
  long stepPoint;
  // TODO: calculate step - variables by evaluating the counters!

 public:

/*
3.6 Constructors and destructor

Standard constructor doing nothing:

*/
  ColRegion();

/*
Non-standard constructor initializing the object with given parameters:

*/
  ColRegion(sRegion* newTuple, sCycle* newCycle, sPoint* newPoint,
            long newCountTuple, long newCountCycle, long newCountPoint);

/*
Non-standard constructor initializing the object with minimum data.
The parameter ~min~ is only used to define a seperate signature
regardless of it's value.

*/
  ColRegion(bool min);

/*
Destructor frees allocated memory:

*/
  ~ColRegion();

/*
Returns the corresponding basic type "aregion":

*/
  static const string BasicType();

/*
Compares the type of the given object with basic type:

*/
  static const bool checkType(const ListExpr list);

/*
Description of the Secondo type for the user:

*/
  static ListExpr Property();

/*
3.7 Auxiliary functions

In this section there are defined some useful functions which can be used in
different contextes. But there are also functions which are used in a special
context, e. g. the append- and finalize functions make only sense
if used together with a constructor.

The ~getCount~ function returns the number of regions within the region array
without the terminator.

*/
  long getCount();

/*
Returns the number of cycles including terminator:

*/
  long getCountCycles();

/*
Returns the number of points including terminator:

*/
  long getCountPoints();

/*
~getCountPoints~ returns the number of points of a single region.
it is used to calculate the needed halfsegments for a new region object.

*/
  long getCountPoints(const long index);


/*
The next three functions returns single elements of the aregion's arrays.

*/
  void getRegion(const long index, long &indexCycle, long &indexPoint,
                 double &mbrX1, double &mbrY1,
                 double &mbrX2, double &mbrY2);

  long getCycle(const long index);

  void getPoint(const long index, double &x, double &y);


/*
The next three functions return the address of the specified array.
They are used for block copy of the ~merge~ function.

*/
  void* getRegionAddress();

  void* getCycleAddress();

  void* getPointAddress();

/*
The function ~createRegion~ extracts the region with given index to a
region object of the spatial algebra.

*/
  bool createRegion(Region* region, const long index);

/*
The ~appendPoint~ function allocates memory for the point array
and appends the given point.

*/
  int appendPoint(const Point p, long &stepPoint);

/*
The ~appendCycle~ function allocates memory for the cycle array
and appends the given cycle.

*/
  int appendCycle(long cp, long &stepCycle);

/*
This function appends one region of the spatial algebra
to an existing aregion of the column spatial algebra.
It's algorithm is similar to the ~OutRegion~ - function of the spatial algebra.

*/
  bool append(Region* region);

/*
After all regions from the spatial alegbra are appended to the
arrays of the column spatial algebra, this function adds
terminator entries to the arrays and set the counters to correct values,
so they can be used correctly in the ~Out~ - function.
Additional the arrays are allocated to their real used memory.

*/
  void finalize();

/*
The ~merge~ function expects two ~aregion~ objects as input parameters
and creates a new ~aregion~ object with the contents of both input objects.
It is called by the ~+~-operator.

*/
  bool merge(ColRegion* cRegion1, ColRegion* cRegion2);


/*
The auxiliary function ~showArrays~ prints the contents of the internal arrays,
there sizes and counters to the screen. It can be accessed by the
Secondo-operator ~showarray~.

*/
  void showArrays(string title, bool showPoints);

/*
The ~inside~ function accepts a Secondo spatial datatype ~apoint~, ~aline~,
~aregion~ as input and checks for each element whether it is
inside the region. if the check is true, the index of this element
is appended to an attribute array of integer. The attribute array is returned.

*/
  inline bool intersects(double x1, double y1, double x2, double y2,
                         double x3, double y3, double x4, double y4);


/*
The following function checks whether a single point is inside a region
Needs x and y of the point and the index of aRegion.

*/
  inline bool pointInsideRegion(double x, double y, long idReg);



/*
The function ~pointsInside~ checks for each point of the ~apoint~ type
whether it is inside the set of regions of the actual ~aregion~ type.
if a point is inside one or more regions its index is stored
in a list of type ~longints~.

Precondition:
- cycle points must be in clockwise or in counterclockwise order,
  no matter if they are faces or holes. this precondition is always true.
- the first point of a region must be the leftmost point.
  this precondition is always true.
Complexity: $O(m . n)$,
where ~m~ is the number of points and ~n~ is the number of regions.

This function processes the complete ~aregion~ array, so if it is
only needed to compare points to a single region, the array ~aRegion~ should
contain only this single region.

known weakness:
if the region overlaps the 180th longitude then the function fails.

*/
  LongInts* pointsInside(ColPoint* cPoint);


/*
The function ~linesInside~ checks for each line of an ~aline~ type
whether it is inside one of the regions of the actual ~aregion~ type.
To do so, the first point of the line is checked whether it is inside or
outside the region. If it is outside, then the whole line
can not be inside the region and the next line is processed.
If it is inside, the number of intersections are counted. if the
number is even then the line is completely inside one of the regions
and the line index will be stored in a ~longints~ result type.
In any other case the line only crosses the region.

*/
  LongInts* linesInside(ColLine* cLine);

/*
The ~contain~ function is similar to the inside function except that not
the indices of the elements within are returned by result but the indices
of the containing regions are returned as a ~longints~ type.

*/
  LongInts* containsPoints(ColPoint* cPoint);

/*
3.8 Standard functions

The ~In~ function scans a nested-list (parameter ~instance~) and converts it
into an internal array representation.
One way to acquire the length of a nested list
would be to call nl->ListLength(instance), but for a large number of regions
this method is very expensive due to a full scan.
An alternative way is implemented in this function. It provides an algorithm
to process an unknown number of elements:
Memory is allocated on a logarithmic scale from 16 KB up to 16 GB trying to
consider known cache-sizes as well as common memory sizes with subtracting
16 KB for control data. The possible allocation sizes are stored in the array
~allocBytes~ which is desfined in the global section of this file.
After all region values are safely read in, the oversized memory is truncated.

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
to the corresponding list element. Minimum bounding boxes aren't stored
separately - they will be computed just in time in the ~In~ and
~append~ functions.

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
Therefore it allocates memory
for the clone object and scans the arrays of the source object and copies
each entry to the clone object.
If big objects are cloned, then this function needs a lot of time.

*/
  static Word Clone(const ListExpr typeInfo, const Word& w);

/*
The ~Cast~ function creates a new ColRegion object and lets it point to
the given address using a special call of new operator.
The argument points to a memory block which is to cast to the object.

*/
  static void* Cast(void* addr);

/*
The function ~TypeCheck~ tests if a given list corresponds to the class type.
This function is quite similar to the ~checkType~ function.

*/
  static bool TypeCheck(ListExpr type, ListExpr& errorInfo);

/*
The function ~SizeOf~ is excpected by the type constructor below. Because there
is no access to a special object, it only returns the size of one region.

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
ListExpr containsTM(ListExpr args);
ListExpr mapTM(ListExpr args);
ListExpr mapColTM(ListExpr args);
ListExpr countTM(ListExpr args);
ListExpr plusTM(ListExpr args);
ListExpr showarrayTM(ListExpr args);

/*
4.2 Value mapping functions

The ~inside~ functions check for each point of the ~apoint~ type,
each line of the ~aline~ type or each region of the ~aregion~ type
whether it is inside each region of an ~aregion~ type.
The strength of these functions lies within the bulk processing.
The more elements are processed the more efficient they will be.
But it is also possible to check single elements only. To achieve this,
the types should only contain single elements.

*/
int pointsInsideVM (Word* args, Word& result, int message,
                    Word& local, Supplier s);
int linesInsideVM (Word* args, Word& result, int message,
                   Word& local, Supplier s);
// not implemented yet
int regionsInsideVM (Word* args, Word& result, int message,
                     Word& local, Supplier s);

/*
Checks on each region whether it contains an object.

*/
int containsPointsVM (Word* args, Word& result, int message,
                    Word& local, Supplier s);

/*
The ~map~ operators convert streams of the standard types points,
lines or regions into their corresponding column-oriented types apoint,
aline or aregion respectively and vice versa.

*/
int mapPointVM (Word* args, Word& result, int message,
                Word& local, Supplier s);
int mapLineVM (Word* args, Word& result, int message,
               Word& local, Supplier s);
int mapRegionVM (Word* args, Word& result, int message,
                 Word& local, Supplier s);

/*
The ~mapCol~ operators convert an column spatial type ~apoint~, ~aline~
or ~aregion~ into their corresponding spatial types.

*/
int mapColPointVM (Word* args, Word& result, int message,
                   Word& local, Supplier s);
int mapColLineVM (Word* args, Word& result, int message,
                  Word& local, Supplier s);
int mapColRegionVM (Word* args, Word& result, int message,
                 Word& local, Supplier s);

/*
The ~count~ operators returns the number of elements of a column spatial type

*/
int countPointVM (Word* args, Word& result, int message,
                   Word& local, Supplier s);
int countLineVM (Word* args, Word& result, int message,
                 Word& local, Supplier s);
int countRegionVM (Word* args, Word& result, int message,
                 Word& local, Supplier s);


/*
The ~+~ Operators merges two arrays of the same type into a new array

*/
int plusPointVM (Word* args, Word& result, int message,
                   Word& local, Supplier s);
int plusLineVM (Word* args, Word& result, int message,
                 Word& local, Supplier s);
int plusRegionVM (Word* args, Word& result, int message,
                  Word& local, Supplier s);

/*
The ~showarray~ operators makes the content and structure of the internal
arrays visible. This is useful to see what's going on at the lower level.

*/
int showarrayPointVM (Word* args, Word& result, int message,
                   Word& local, Supplier s);
int showarrayLineVM (Word* args, Word& result, int message,
                 Word& local, Supplier s);
int showarrayRegionVM (Word* args, Word& result, int message,
                  Word& local, Supplier s);
/*
4.3 Value Mapping Array and Selection Function

*/
ValueMapping insideVM[] = {pointsInsideVM, linesInsideVM, regionsInsideVM};
ValueMapping containsVM[] = {containsPointsVM};
ValueMapping mapVM[]    = {mapPointVM, mapLineVM, mapRegionVM};
ValueMapping mapColVM[] = {mapColPointVM, mapColLineVM, mapColRegionVM};
ValueMapping countVM[]  = {countPointVM, countLineVM, countRegionVM};
ValueMapping plusVM[]  = {plusPointVM, plusLineVM, plusRegionVM};
ValueMapping showarrayVM[]  = {showarrayPointVM, showarrayLineVM,
                               showarrayRegionVM};

int insideSelect(ListExpr args);
int containsSelect(ListExpr args);
int mapSelect(ListExpr args);
int mapColSelect(ListExpr args);
int countSelect(ListExpr args);
int plusSelect(ListExpr args);
int showarraySelect(ListExpr args);

/*
4.4 Specification

arguments of the ~OperatorSpec~ constructor:
description of the type mapping
the syntax of the operator
the operator's meaning
example query.
remark (optional)

*/
OperatorSpec insideSpec("obj x aregion -> longints, obj={apoint,aline,aregion}",
                        "_ inside _ ",
                        "checks each element of obj whether in region",
                        "query cFluss inside cKreis ");

OperatorSpec containsSpec("aregion x apoint -> longints ",
                        "_ contains _ ",
                        "checks which region contains points",
                        "query cKreis contains CStadt ");
OperatorSpec mapSpec("stream(tuple) -> {apoint, aline, aregion} ",
                        "mp [_] ",
                        "maps a stream of standard type to attribute array",
                        "query Kreis feed mp[Gebiet] ");
OperatorSpec mapColSpec("{apoint, aline, aregion} x int "
                        "-> {point, line, region} ",
                        "mp [_] ",
                        "maps one entry of attribute array to standard type",
                        "query cKreis mp[1] ");
OperatorSpec countSpec("{apoint, aline, aregion} -> int ",
                        "count ",
                        "returns the number of elements of the spatial type",
                        "query cKreis count ");
OperatorSpec plusSpec("obj x obj -> obj, obj={apoint,aline,aregion} ",
                        "_ + _ ",
                        "merges two array types into a new array type",
                        "query cSee + cWald ");
OperatorSpec showarraySpec("{apoint,aline,aregion} -> array of atype",
                        "_ showarray ",
                        "shows the internal attribut arrays of the type",
                        "query cSee showarray ");

/*
4.5 Operator Instance

operator's name
specification
number of Value Mappings
value mapping array
selection function
type mapping

*/


Operator insideOp("inside", insideSpec.getStr(), 3,
                  insideVM, insideSelect, insideTM);

Operator containsOp("contains", containsSpec.getStr(), 1,
                  containsVM, containsSelect, containsTM);

Operator mapOp("mp", mapSpec.getStr(), 3,
               mapVM, mapSelect, mapTM);

Operator mapColOp("mp", mapColSpec.getStr(), 3,
                  mapColVM, mapColSelect, mapColTM);

Operator countOp("count", countSpec.getStr(), 3,
                  countVM, countSelect, countTM);

Operator plusOp("+", plusSpec.getStr(), 3,
                  plusVM, plusSelect, plusTM);

Operator showarrayOp("showarray", showarraySpec.getStr(), 3,
                    showarrayVM, showarraySelect, showarrayTM);


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
   AddOperator(&containsOp);
   AddOperator(&mapOp);
   AddOperator(&mapColOp);
   AddOperator(&countOp);
   AddOperator(&plusOp);
   AddOperator(&showarrayOp);
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
