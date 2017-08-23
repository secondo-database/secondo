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

[1] Implementation of a column oriented spatial algebra

[2] May 2017 by Sascha Radke for bachelor thesis



1 Includes and global settings

*/

// TODO: remove relative paths and int64_t.
// they are only needed for a better syntax hilighting with qt-creator.
#include "ColumnSpatialAlgebra.h"             // header for this algebra
#include "../../include/Symbols.h"            // predefined strings
#include "../../include/ListUtils.h"          // useful nested list functions
#include "../../include/NestedList.h"         // required at many places
#include "../../include/QueryProcessor.h"     // needed for value mappings
#include "../../include/TypeConstructor.h"    // constructor for Secondo Types
#include "../../include/StandardTypes.h"
#include "../Spatial/SpatialAlgebra.h"        // spatial types and operators
#include "../Spatial/RegionTools.h"           // cycles and region building
#include "../Relation-C++/RelationAlgebra.h"  // use of tuples
#include "../Stream/Stream.h"                 // wrapper for secondo streams
#include "../CRel/Ints.h"                     // type for id result list
#include "../CRel/TypeConstructors/LongIntsTC.h"
#include <ctime>
#include <fstream>
#include <cstring>

using std::vector;
using std::fstream;

using namespace CRelAlgebra;

extern NestedList *nl;
extern QueryProcessor *qp;

typedef long int64_t;

namespace col {



/*
Benchmark cycles and nanoseconds.

*/
inline void benchmark(long &cycles, long &ns) {
  struct timespec ttime;
  long lo, hi;
  // get nanoseconds from timer
  clock_gettime(CLOCK_MONOTONIC, &ttime);
  ns = (long)ttime.tv_sec * 1.0e9 + ttime.tv_nsec;
  // get timestamp counter (tsc) from cpu
  asm( "rdtsc" : "=a" (lo), "=d" (hi) );
  cycles = lo | (hi << 32);
}



/*
2 Class ColPoint for column-oriented representation of points

*/
  ColPoint::ColPoint() {}  // standard constructor doing nothing

  ColPoint::ColPoint(sPoint* newArray, long newCount) {  // main constructor
    aPoint = newArray;
    count = newCount;
  }

  ColPoint::ColPoint(bool min) {  // constructor with minimum array
    aPoint = NULL;
    aPoint = static_cast<sPoint*>(calloc(1, sizeof(sPoint)));
    count = 0;
    step = 0;
  }

  ColPoint::~ColPoint() {  // destructor
    free(aPoint);
  }



  // returns the corresponding basic type
  const string ColPoint::BasicType() { return "apoint"; }



  // compares the type of the given object with the class type
  const bool ColPoint::checkType(const ListExpr list) {
    return listutils::isSymbol(list, BasicType());
  }



  // description of the Secondo type for the user
  ListExpr ColPoint::Property() {
    return (nl->TwoElemList(
    nl->FiveElemList(
      nl->StringAtom("Signature"),
      nl->StringAtom("Example Type List"),
      nl->StringAtom("List Rep"),
      nl->StringAtom("Example List"),
      nl->StringAtom("Remarks")),
    nl->FiveElemList(
      nl->StringAtom("-> SIMPLE"),
      nl->StringAtom(BasicType()),
      nl->StringAtom("((real real) .. (real real))"),
      nl->StringAtom("((3.5 -6.0) (1.0 2) (-9.1 5))"),
      nl->StringAtom("created by nested list or stream of points."))));
  }



  // returns the number of elements in the point array
  long ColPoint::getCount() {
    return count;
  }



  // returns the x coordinate of the indexed point entry
  double ColPoint::getX(long index) {
    return aPoint[index].x;
  }



  // returns the x coordinate of the indexed point entry
  double ColPoint::getY(long index) {
    return aPoint[index].y;
  }



  // returns the address of the first element of the apoint array
  void* ColPoint::getArrayAddress() {
    return &aPoint[0].x;
  }



  // appends one point to the point array
  bool ColPoint::append(Point* point) {
    if(!point->IsDefined()) {
      cout << "point is undefined!" << endl;
      return false;
    }

    if(point->IsEmpty()) {
      cout << "point is empty" << endl;
      return false;
    }

    // calculate and allocate sufficient memory for the tuple array
    while ((long) (count * sizeof(sPoint)) >= allocBytes[step]) step++;
    aPoint = static_cast<sPoint*> (realloc(aPoint, allocBytes[step]));
    if (aPoint == NULL) {  // exit on memory overflow
      cmsg.inFunError("not enough memory for all lines!");
      return false;
    }

    // initialize tuple array with next line
    aPoint[count].x = point->GetX();
    aPoint[count].y = point->GetY();
    count++;

    return true;
  }



  // add terminator entries to arrays and finalize counters
  void ColPoint::finalize() {
    aPoint = static_cast<sPoint*>(realloc(aPoint, count * sizeof(sPoint)));
    cout << count * sizeof(sPoint) << " bytes used.\n";
  }



/*
The ~merge~ - function concatenates two apoints and returns a single apoint.
It uses the memcpy - function of the string-library

*/
  // merges two apoints into one apoint
  bool ColPoint::merge(ColPoint* cPoint1, ColPoint* cPoint2) {

    // calculate and allocate sufficient memory for the result array
    long cc1 = cPoint1->getCount();
    long cc2 = cPoint2->getCount();
    aPoint = static_cast<sPoint*>
             (realloc(aPoint, (cc1 + cc2) * sizeof(sPoint)));
    if (aPoint == NULL) {  // exit on memory overflow
      cmsg.inFunError("not enough memory for all points!");
      return false;
    }

    memcpy(&aPoint[0].x, cPoint1->getArrayAddress(), cc1 * sizeof(sPoint));
    memcpy(&aPoint[cc1].x, cPoint2->getArrayAddress(), cc2 * sizeof(sPoint));

    count = cc1 + cc2;

    return true;
  }



  // test output of the array aPoint and there parameters
  void ColPoint::showArray(string title) {
    cout << "\n--------------------------------------------\n" << title << "\n";
    // output of the array aPoint
    cout << "aPoint (" << count << "):\n";
    cout << "Bytes allocated: " << count * sizeof(sPoint) << "\n";
    cout << "index\tx\ty\n";
    for (long cp = 0; cp < count; cp++) {
      cout << cp << ":\t" << aPoint[cp].x << "\t" << aPoint[cp].y << "\n";
    }
    cout << "--------------------------------------------\n";
  }



  // reads a nested list and stores it into the array format
  Word ColPoint::In(const ListExpr typeInfo, ListExpr instance,
                 const int errorPos, ListExpr &errorInfo, bool &correct) {
    correct = false;
    sPoint* inArray = NULL;  // array which contains the input point values
    long inCount = 0;       // counter of input points, grows with each point
    long step = 0;          // index of actual allocated memory (allocBytes)
    Word error(static_cast<void*>(0));  // create an error result pointing at 0
    errorInfo = listutils::typeError("Error in ColPoint.In!!!");

    if (listutils::isSymbolUndefined(instance)) {
      cmsg.inFunError("Symbol is undefined!");
      return error;
    }

    if (nl->IsAtom(instance)) {  // exit when the nested list is an atom
      cmsg.inFunError("Nested list must not be an atom!");
      return error;
    }

    while (!nl->IsEmpty(instance)) {  // as long as there are points left
      ListExpr pointLeaf = nl->First(instance);  // get the point
      if ((!nl->HasLength(pointLeaf, 2)) ||
          (!listutils::isNumeric(nl->First(pointLeaf))) ||
          (!listutils::isNumeric(nl->Second(pointLeaf)))) {
          // exit on invalid point
        cmsg.inFunError("expected an atom with two numeric values!");
        return error;
      }

      // allocate more memory if the actual allocated memory is insufficient
      if ((long)((inCount + 1) * sizeof(sPoint)) >= allocBytes[step]) {
        // allocate more memory - in C++ a type casting is necessary unlike in C
        inArray = static_cast<sPoint*>(realloc(inArray, allocBytes[++step]));
        if (inArray == NULL) {    // exit on memory overflow
          cmsg.inFunError("out of memory");
          return error;
        }
      }

      // append the x- and y-Coordinates from the actual point to the array
      inArray[inCount].x = listutils::getNumValue((nl->First(pointLeaf)));
      inArray[inCount].y = listutils::getNumValue((nl->Second(pointLeaf)));
      inCount++;
      instance = nl->Rest(instance);  // move to next entry
    }

    // truncate oversized memory to allocate the real used memory
    inArray = static_cast<sPoint*>(realloc(inArray, inCount * sizeof(sPoint)));
    cout << inCount * sizeof(sPoint) << " bytes used\n";

    Word answer(static_cast<void*>(0));
    answer.addr = new ColPoint(inArray, inCount);  // create new point object
    correct = true;
    return answer;
  }

  // converts the internal array structure int a nested list
  ListExpr ColPoint::Out(ListExpr typeInfo, Word value) {
    ColPoint* cPoint = static_cast<ColPoint*>(value.addr);

    if (cPoint->count == 0) {
      return listutils::emptyErrorInfo();
    }

    ListExpr res = nl->OneElemList(nl->TwoElemList(  // init first point
      nl->RealAtom(cPoint->aPoint[0].x),
      nl->RealAtom(cPoint->aPoint[0].y)));
    ListExpr last = res;  // set actual list element

    for (long i = 1; i < cPoint->count; i++) {  // read all aPoint elements
      last = nl->Append(last,  // append the point to the nested list
        nl->TwoElemList(
          nl->RealAtom(cPoint->aPoint[i].x),
          nl->RealAtom(cPoint->aPoint[i].y)));
    }

    return res;  // pointer to the beginning of the nested list
  }



  Word ColPoint::Create(const ListExpr typeInfo) {
    Word answer(static_cast<void*>(0));
    sPoint* inArray = NULL;
    answer.addr = new ColPoint(inArray, 0);  // create a new point object
    return answer;  // return its adress
  }



  void ColPoint::Delete(const ListExpr typeInfo, Word& w) {
    ColPoint* cPoint = static_cast<ColPoint*>(w.addr);
    delete cPoint;
    w.addr = 0;
  }



  bool ColPoint::Open(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value) {

    cout << "open apoint...\n";


    sPoint* aPoint = NULL;  // array which contains the input point values
    long count;             // amount of points
    double x, y;            // actual read coordinates
    size_t sizeL = sizeof(long);
    size_t sizeD = sizeof(double);
    bool ok = (valueRecord.Read(&count, sizeL, offset) == sizeL);
    offset += sizeL;

    // allocate memory depending on the ammount of points
    aPoint = static_cast<sPoint*>(malloc(count * sizeof(sPoint)));
    if (aPoint == NULL) {    // exit on memory overflow
      cmsg.inFunError("not enough memory (??? Bytes needed)!");
      return false;
    }

    // read each point and store it into the array
    for (long i = 0; i < count; i++) {
      ok = ok && (valueRecord.Read(&x, sizeD, offset) == sizeD);
      offset += sizeD;
      aPoint[i].x = x;
      ok = ok && (valueRecord.Read(&y, sizeD, offset) == sizeD);
      offset += sizeD;
      aPoint[i].y = y;
      if (!ok) { break; }
    }

    if (ok) {  // create a new ColPoint and store the read values in it
      value.addr = new ColPoint(aPoint, count);
    } else {  // error
      value.addr = 0;
    }

    cout << count * sizeof(sPoint) << " bytes used." << endl;

    return ok;
  }

  bool ColPoint::Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value) {

    cout << "save apoint...\n";

    ColPoint* cPoint = static_cast<ColPoint*>(value.addr);
    size_t sizeL = sizeof(long);
    size_t sizeD = sizeof(double);
    long c = cPoint->count;
    double x, y;

    bool ok = valueRecord.Write(&c, sizeL, offset);
    offset += sizeL;

    for (long i = 0; i < cPoint->count; i++) {
      x = cPoint->aPoint[i].x;
      y = cPoint->aPoint[i].y;
      ok = ok && valueRecord.Write(&x, sizeL, offset);
      offset += sizeD;
      ok = ok && valueRecord.Write(&y, sizeL, offset);
      offset += sizeD;
    }

    return ok;
  }



  void ColPoint::Close(const ListExpr typeInfo, Word& w) {
    delete static_cast<ColPoint*>( w.addr);
    w.addr = 0;
  }



/*
~Clone~ makes a copy of a ColPoint object. Therefore it allocates memory
for the clone object and scans the array of the source object and copies
each entry to the clone object.

*/
  Word ColPoint::Clone(const ListExpr typeInfo, const Word& w) {

    ColPoint* cPoint = static_cast<ColPoint*>(w.addr);

    long tmpCount = cPoint->count;
    sPoint* tmpPoint = NULL;
    tmpPoint = static_cast<sPoint*>(realloc(tmpPoint,sizeof(sPoint)*tmpCount));

    if (tmpPoint == NULL) {    // exit on memory overflow
      cmsg.inFunError("out of memory");
      return (void*)0;
    }

    for (long cp = 0; cp < tmpCount; cp++)
      tmpPoint[cp] = cPoint->aPoint[cp];
    Word res((void*)0);
    res.addr = new ColPoint(tmpPoint, tmpCount);

    return res;
  }



  void* ColPoint::Cast(void* addr) {
    return (new (addr) ColPoint);
  }



  bool ColPoint::TypeCheck(ListExpr type, ListExpr& errorInfo) {
    return nl->IsEqual(type, BasicType());
  }



  int ColPoint::SizeOf() {
    return sizeof(sPoint);
  }

//------------------------------------------------------------------------------

/*
3 Class ColLine for column-oriented representation of lines

*/
  ColLine::ColLine() {}  // standard constructor doing nothing

  ColLine::ColLine(sLine *newLine, sSegment *newSegment,
                   long newCountLine, long newCountSegment) {
    aLine = newLine;
    aSegment = newSegment;
    countLine = newCountLine;
    countSegment = newCountSegment;
  }

  ColLine::ColLine(bool min) {  // constructor with minimum initialized arrays
    aLine = NULL;
    aSegment = NULL;
    aLine = static_cast<sLine*>(calloc(1, sizeof(sLine)));
    aSegment = static_cast<sSegment*>(calloc(1, sizeof(sSegment)));
    countLine = 0;
    countSegment = 0;
    stepLine = 0;
    stepSegment = 0;
  }

  ColLine::~ColLine() {   // destructor
    free(aLine);
    free(aSegment);
  }



  // returns the corresponding basic type
  const string ColLine::BasicType() { return "aline"; }



  // compares the type of the given object with class type
  const bool ColLine::checkType(const ListExpr list) {
    return listutils::isSymbol(list, BasicType());
  }



  // description of the Secondo type for the user
  ListExpr ColLine::Property() {
    return (nl->TwoElemList(
    nl->FiveElemList(
      nl->StringAtom("Signature"),
      nl->StringAtom("Example Type List"),
      nl->StringAtom("List Rep"),
      nl->StringAtom("Example List"),
      nl->StringAtom("Remarks")),
    nl->FiveElemList(
      nl->StringAtom("-> SIMPLE"),
      nl->StringAtom(BasicType()),
      nl->StringAtom("((x1 y1 x2 y2) (x2 y2 x3 y3) ... ) ...)"),
      nl->StringAtom("(((3.5 -6.0 5 3) (5 3 5 2) (5 3 4 7)))"),
      nl->StringAtom("all coordinates must be of type real"))));
  }



  // returns the number of elements in the line array
  long ColLine::getCount() {
    return countLine - 1;  // subtract the terminator element
  }



  // returns the total number of segments in the segments array
  long ColLine::getSegments() {
    return countSegment - 1;  // subtract the terminator element
  }



  // returns the number of segmets for a single line
  long ColLine::getSegments(long index) {
      return aLine[index + 1].index - aLine[index].index;
  }



  // returns the indices of the first and last segment of a single line
  void ColLine::getLineSegments(long index, long &first, long &last) {
    first = aLine[index].index;
    last = aLine[index + 1].index;
  }



  // returns the coordinates of a single segment
  void ColLine::getSegmentPoints(long index,
                                 double &x1, double &y1,
                                 double &x2, double &y2) {
    x1 = aSegment[index].x1;
    y1 = aSegment[index].y1;
    x2 = aSegment[index].x2;
    y2 = aSegment[index].y2;
  }



  void* ColLine::getLineAddress() {
    return &aLine[0].index;
  }

  void* ColLine::getSegmentAddress() {
    return &aSegment[0].x1;
  }


  // returns the line found at index of aline
  bool ColLine::createLine(Line* line, long index) {
    long start = aLine[index].index;
    long stop  = aLine[index + 1].index;
    double x1, y1, x2, y2;  // work variables
    Point* lp = new Point(true, 0, 0);  // actual point
    Point* rp = new Point(true, 1, 1);  // actual point
    HalfSegment* hs = new HalfSegment(true, *lp, *rp);  // actual halfsegment

    line->StartBulkLoad();  // bulk load for easy filling of line type

    for (long i = start; i < stop; i++) {  // scan all segments
      x1 = aSegment[i].x1;
      y1 = aSegment[i].y1;
      x2 = aSegment[i].x2;
      y2 = aSegment[i].y2;
      lp->Set(x1, y1);
      rp->Set(x2, y2);
      hs->Set(true, *lp, *rp);
      line->Put(i - start, *hs);  // insert segment to line
    }

    line->EndBulkLoad(true, true, true);

    return true;
  }



  // appends a complete line to an aline object
  bool ColLine::append(Line* line) {

    if(!line->IsDefined()) {
      cout << "line is undefined!" << endl;
      return false;
    }

    if(line->IsEmpty()) {
      cout << "line is empty" << endl;
      return false;
    }

    // calculate and allocate sufficient memory for the tuple array
    while ((long)
          ((countLine + 2) * sizeof(sLine)) >= allocBytes[stepLine]) {
      stepLine++;
    }
    aLine = static_cast<sLine*> (realloc(aLine, allocBytes[stepLine]));
    if (aLine == NULL) {  // exit on memory overflow
      cmsg.inFunError("not enough memory for all lines!");
      return false;
    }

    // initialize tuple array with next line
    aLine[countLine].index = countSegment;
    Point lp, rp;
    HalfSegment hs;

    // make a copy of the input line to avoid modifying the original data
    Line *lCopy = new Line(*line);

    for (int i = 0; i < lCopy->Size(); i++) {
      lCopy->Get(i, hs);  // extract actual halfsegment

      if(hs.IsLeftDomPoint() == true) {
        // allocates memory for the point array and appends the given segment
        if ((long)
           ((countSegment + 2) * sizeof(sSegment)) >= allocBytes[stepSegment]) {
          // allocate more memory - in C++ a type casting is necessary
          aSegment = static_cast<sSegment*>
            (realloc(aSegment, allocBytes[++stepSegment]));
          if (aSegment == NULL) {  // exit on memory overflow
            cmsg.inFunError("not enough memoryfor new segment!");
            return false;
          }
        }

        lp = hs.GetLeftPoint();
        rp = hs.GetRightPoint();
        aSegment[countSegment].x1 = lp.GetX();
        aSegment[countSegment].y1 = lp.GetY();
        aSegment[countSegment].x2 = rp.GetX();
        aSegment[countSegment].y2 = rp.GetY();
        countSegment++;
      }
   }

   countLine++;
   return true;
  }



  // add terminator entries to arrays and finalize counters
  void ColLine::finalize() {
    aLine[countLine].index = countSegment;
    aSegment[countSegment].x1 = 0;
    aSegment[countSegment].y1 = 0;
    aSegment[countSegment].x2 = 0;
    aSegment[countSegment].y2 = 0;

    countLine++;
    countSegment++;

    aLine = static_cast<sLine*>
             (realloc(aLine, countLine * sizeof(sLine)));
    aSegment = static_cast<sSegment*>
             (realloc(aSegment, countSegment * sizeof(sSegment)));

    cout << countLine * sizeof(sLine) + countSegment * sizeof(sSegment)
         << " bytes used.\n";
  }



  // merges two alines into one aline
  bool ColLine::merge(ColLine* cLine1, ColLine* cLine2) {

    // calculate and allocate sufficient memory for the result array
    long ccl1 = cLine1->getCount();
    long ccl2 = cLine2->getCount();
    long ccs1 = cLine1->getSegments();
    long ccs2 = cLine2->getSegments();

    aLine = static_cast<sLine*>
            (realloc(aLine, (ccl1 + ccl2 + 1) *  sizeof(sLine)));
    if (aLine == NULL) {  // exit on memory overflow
      cmsg.inFunError("not enough memory for all lines!");
      return false;
    }

    aSegment = static_cast<sSegment*>
               (realloc(aSegment, (ccs1 + ccs2 + 1) * sizeof(sSegment)));
    if (aSegment == NULL) {  // exit on memory overflow
      cmsg.inFunError("not enough memory for all segments!");
      return false;
    }

    // fast block copy
    memcpy(&aLine[0].index, cLine1->getLineAddress(),
           ccl1 * sizeof(sLine));
    memcpy(&aLine[ccl1].index, cLine2->getLineAddress(),
           (ccl2 + 1) * sizeof(sLine));

    memcpy(&aSegment[0].x1, cLine1->getSegmentAddress(),
           ccs1 * sizeof(sSegment));
    memcpy(&aSegment[ccs1].x1, cLine2->getSegmentAddress(),
           (ccs2 + 1) * sizeof(sSegment));

    // correct indices of second aline
    for (long i = ccl1; i < (ccl1 + ccl2 + 1); i++)
      aLine[i].index += ccs1;

    countLine = ccl1 + ccl2 + 1;
    countSegment = ccs1 + ccs2 + 1;

    return true;
  }



  // test output of the arrays aLine and aSegment and there parameters
  void ColLine::showArrays(string title) {
    cout << "\n--------------------------------------------\n" << title << "\n";

    // output of the array aLine
    cout << "aLine (" << countLine << "):\n";
    cout << "Bytes allocated: " << countLine * sizeof(sLine) << "\n";
    cout << "index\tsegment\n";
    for (long cl = 0; cl < countLine; cl++) {
      cout << cl << ":\t" << aLine[cl].index << "\n";
    }

    // output of the array aSegment
    cout << "aSegment (" << countSegment << "):\n";
    cout << "Bytes allocated: " << countSegment * sizeof(sSegment) << "\n";
    cout << "index\tx1\ty1\tx2\ty2\n";
    for (long cs = 0; cs < countSegment; cs++) {
      cout << cs << ":\t"
           << aSegment[cs].x1 << "\t"
           << aSegment[cs].y1 << "\t"
           << aSegment[cs].x2 << "\t"
           << aSegment[cs].y2 << "\n";
    }

    cout << "--------------------------------------------\n";
  }



  // scans a nested-list and converts it into an array of lines.
  Word ColLine::In(const ListExpr typeInfo, ListExpr instance,
                 const int errorPos, ListExpr &errorInfo, bool &correct) {
    correct = false;
    Word error(static_cast<void*>(0));  // create an error result pointing at 0
    errorInfo = listutils::typeError("Error in ColLine.In!!!");

    if (listutils::isSymbolUndefined(instance)) {
      cmsg.inFunError("Symbol is undefined!");
      return error;
    }

    if (nl->IsAtom(instance)) {  // exit when the nested list is an atom
      cmsg.inFunError("Nested list must not be an atom!");
      return error;
    }

    sLine* inLine = NULL;  // array which contains the input line values
    sSegment* inSeg = NULL;  // array which contains the input line values
    long cl = 0;    // counter of input lines, grows with each line
    long cs = 0;    // counter of input lines, grows with each line
    // index to the array allocbytes of actual allocated memory for each array
    long stepLine = 0;
    long stepSegment = 0;

    // allocate memory for the input arrays
    inLine = static_cast<sLine*>(realloc(inLine, allocBytes[stepLine]));
    inSeg = static_cast<sSegment*>(realloc(inSeg, allocBytes[stepSegment]));
    ListExpr lineNL = instance;  // get all lines
    while (!nl->IsEmpty(lineNL)) {  // as long as there are lines left

      // allocate more memory if the actual allocated memory is insufficient
      if ((long)((cl + 2) * sizeof(sLine)) >= allocBytes[stepLine]) {
        // allocate more memory - C++ needs type casting unlike C
        inLine = static_cast<sLine*>(realloc(inLine, allocBytes[++stepLine]));
        if (inLine == NULL) {  // exit on memory overflow
          cmsg.inFunError("not enough memory for line indices!");
          return error;
        }
      }

      inLine[cl].index = cs;
      cl++;

      ListExpr segmentNL = nl->First(lineNL);  // get first segment
      if (nl->IsAtom(segmentNL)) {
        cmsg.inFunError("expected a nested list of segments for each line!");
        return error;
      }

      while(!nl->IsEmpty(segmentNL)) {
        ListExpr segmentLeaf = nl->First(segmentNL);  // get first segment
        if ((!nl->HasLength(segmentLeaf, 4)) ||
            (!listutils::isNumeric(nl->First(segmentLeaf))) ||
            (!listutils::isNumeric(nl->Second(segmentLeaf))) ||
            (!listutils::isNumeric(nl->Third(segmentLeaf))) ||
            (!listutils::isNumeric(nl->Fourth(segmentLeaf)))) {
           // exit on invalid segment
          cmsg.inFunError("expected an atom with four numeric values!");
          return error;
        }

        // allocate more memory if the actual allocated memory is insufficient
        if ((long)((cs + 2) * sizeof(sSegment)) >= allocBytes[stepSegment]) {
          // allocate more memory - C++ needs type casting unlike C
          inSeg = static_cast<sSegment*>
                      (realloc(inSeg, allocBytes[++stepSegment]));
          if (inSeg == NULL) {  // exit on memory overflow
            cmsg.inFunError("not enough memory for line segments!");
            return error;
          }
        }

        // append the coordinates from the actual line to the array
        inSeg[cs].x1 = listutils::getNumValue((nl->First(segmentLeaf)));
        inSeg[cs].y1 = listutils::getNumValue((nl->Second(segmentLeaf)));
        inSeg[cs].x2 = listutils::getNumValue((nl->Third(segmentLeaf)));
        inSeg[cs].y2 = listutils::getNumValue((nl->Fourth(segmentLeaf)));
        cs++;
        segmentNL = nl->Rest(segmentNL);
      }
      lineNL = nl->Rest(lineNL);  // move to next line
    }

    // add terminator entries to arrays. they are needed for the Out-function.
    inLine[cl].index = cs;
    inSeg[cs].x1 = 0;
    inSeg[cs].y1 = 0;
    inSeg[cs].x2 = 0;
    inSeg[cs].y2 = 0;

    // truncate oversized memory to allocate the real used memory
    inLine = static_cast<sLine*>(realloc(inLine, ++cl * sizeof(sLine)));
    inSeg = static_cast<sSegment*>(realloc(inSeg, ++cs * sizeof(sSegment)));
    cout << cl * sizeof(sLine) + cs * sizeof(sSegment) << " bytes used\n";
    Word answer(static_cast<void*>(0));
    answer.addr = new ColLine(inLine, inSeg, cl, cs);  // create new line object

    correct = true;
    return answer;  // return the object
  }



  // converts a line array into a nested list format
  ListExpr ColLine::Out(ListExpr typeInfo, Word value) {
    ColLine* cLine = static_cast<ColLine*>(value.addr);
    if (cLine->countLine == 0) {
      return listutils::emptyErrorInfo();
    }

    long cl = 1;  // counter for index array aLine starting at second entry

    // initialize heads and tails for working lists
    ListExpr lineNL     = nl->TheEmptyList();
    ListExpr lineNLLast = lineNL;

    // init tuple with first segment
    ListExpr tupleNL = nl->OneElemList(nl->FourElemList(
                           nl->RealAtom(cLine->aSegment[0].x1),
                           nl->RealAtom(cLine->aSegment[0].y1),
                           nl->RealAtom(cLine->aSegment[0].x2),
                           nl->RealAtom(cLine->aSegment[0].y2)));
    ListExpr tupleNLLast  = tupleNL;

    // main loop: traverse all segments in the segment array
    for (long cs = 1; cs < cLine->countSegment; cs++) {  // read all segments
      // check whether a new line appears
      if (cs == cLine->aLine[cl].index) {
        if (nl->IsEmpty(lineNL)) {  // append tuple to first line
          lineNL = nl->OneElemList(tupleNL);
          lineNLLast = lineNL;
        } else {  // append tuple to line
          lineNLLast = nl->Append(lineNLLast, tupleNL);
        }

        tupleNL = nl->OneElemList(nl->FourElemList(
                                   nl->RealAtom(cLine->aSegment[cs].x1),
                                   nl->RealAtom(cLine->aSegment[cs].y1),
                                   nl->RealAtom(cLine->aSegment[cs].x2),
                                   nl->RealAtom(cLine->aSegment[cs].y2)));
        tupleNLLast  = tupleNL;
        cl++;
      } else { // append new segment to segment list
        tupleNLLast = nl->Append(tupleNLLast, nl->FourElemList(
                                   nl->RealAtom(cLine->aSegment[cs].x1),
                                   nl->RealAtom(cLine->aSegment[cs].y1),
                                   nl->RealAtom(cLine->aSegment[cs].x2),
                                   nl->RealAtom(cLine->aSegment[cs].y2)));
      }
    }

    return lineNL;
  }



  Word ColLine::Create(const ListExpr typeInfo) {
    Word answer(static_cast<void*>(0));
    sLine* inLine = NULL;
    sSegment* inSeg = NULL;
    answer.addr = new ColLine(inLine, inSeg, 0, 0);  // create new line object
    return answer;  // return its adress
  }



  // Removes the complete object including disc parts if there are any
  void ColLine::Delete(const ListExpr typeInfo, Word& w) {
    ColLine* cLine = static_cast<ColLine*>(w.addr);
    delete cLine;
    w.addr = 0;
    }



  // Reads an array from disc via an ~SmiRecord~.
  bool ColLine::Open(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value) {

    cout << "open aline...\n";

    sLine* inLine = NULL;  // array which contains the input line values
    sSegment* inSegment = NULL;  // array which contains the input line values
    long cl;           // number of lines
    long cs;  // number  of segments
    long valueL;
    double x1, y1, x2, y2;          // actual read coordinates
    size_t sizeL = sizeof(long);
    size_t sizeD = sizeof(double);

    // allocate memory depending on the ammount of lines
    bool ok = (valueRecord.Read(&cl, sizeL, offset) == sizeL);
    offset += sizeL;
    inLine = static_cast<sLine*>(malloc(cl * sizeof(sLine)));
    if (inLine == NULL) {    // exit on memory overflow
      cmsg.inFunError("not enough memory for line indices!");
      return false;
    }

    // allocate memory depending on the ammount of segments
    ok = (valueRecord.Read(&cs, sizeL, offset) == sizeL);
    offset += sizeL;
    inSegment = static_cast<sSegment*>(malloc(cs * sizeof(sSegment)));
    if (inSegment == NULL) {    // exit on memory overflow
      cmsg.inFunError("not enough memory for segments!");
      return false;
    }

    // read each line and store it into the array
    for (long i = 0; i < cl; i++) {
      ok = ok && (valueRecord.Read(&valueL, sizeL, offset) == sizeL);
      offset += sizeL;
      inLine[i].index = valueL;
      if (!ok) break;
    }

    // read each segment and store it into the array
    for (long i = 0; i < cs; i++) {
      ok = ok && (valueRecord.Read(&x1, sizeD, offset) == sizeD);
      offset += sizeD;
      ok = ok && (valueRecord.Read(&y1, sizeD, offset) == sizeD);
      offset += sizeD;
      ok = ok && (valueRecord.Read(&x2, sizeD, offset) == sizeD);
      offset += sizeD;
      ok = ok && (valueRecord.Read(&y2, sizeD, offset) == sizeD);
      offset += sizeD;
      inSegment[i].x1 = x1;
      inSegment[i].y1 = y1;
      inSegment[i].x2 = x2;
      inSegment[i].y2 = y2;
      if (!ok) break;
    }

    if (ok) {  // create a new ColLine and store the read values in it
      value.addr = new ColLine(inLine, inSegment, cl, cs);
    } else {  // error
      value.addr = 0;
    }

    cout << cl * sizeof(sLine) + cs * sizeof(sSegment)
         << " bytes used." << endl;
    return ok;
  }



  // Saves an array of lines into a SmiRecord.
  bool ColLine::Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value) {

    cout << "save aline...\n";

    ColLine* cLine = static_cast<ColLine*>(value.addr);

    size_t sizeL = sizeof(long);
    size_t sizeD = sizeof(double);
    long cl = cLine->countLine;
    long cs = cLine->countSegment;
    long valueL;
    double x1, y1, x2, y2;

    bool ok = valueRecord.Write(&cl, sizeL, offset);
    offset += sizeL;
    ok = valueRecord.Write(&cs, sizeL, offset);
    offset += sizeL;

    // write all line indices to smi record
    for (long i = 0; i < cLine->countLine; i++) {
      valueL = cLine->aLine[i].index;
      ok = ok && valueRecord.Write(&valueL, sizeL, offset);
      offset += sizeL;
    }

    // write all segments to smi record
    for (long i = 0; i < cLine->countSegment; i++) {
      x1 = cLine->aSegment[i].x1;
      ok = ok && valueRecord.Write(&x1, sizeL, offset);
      offset += sizeD;
      y1 = cLine->aSegment[i].y1;
      ok = ok && valueRecord.Write(&y1, sizeL, offset);
      offset += sizeD;
      x2 = cLine->aSegment[i].x2;
      ok = ok && valueRecord.Write(&x2, sizeL, offset);
      offset += sizeD;
      y2 = cLine->aSegment[i].y2;
      ok = ok && valueRecord.Write(&y2, sizeL, offset);
      offset += sizeD;
    }

    return ok;
  }



  void ColLine::Close(const ListExpr typeInfo, Word& w) {
    delete static_cast<ColLine*>(w.addr);
    w.addr = 0;
  }



/*
~Clone~ makes a copy of a ColLine object. Therefore it allocates memory
for the clone object and scans the arrays of the source object and copies
each entry to the clone object.

*/
  Word ColLine::Clone(const ListExpr typeInfo, const Word& w) {

    ColLine* cLine = static_cast<ColLine*>(w.addr);

    long tmpCL = cLine->countLine;
    sLine* tmpLine = NULL;
    tmpLine = static_cast<sLine*>(realloc(tmpLine, sizeof(sLine) * tmpCL));
    if (tmpLine == NULL) {    // exit on memory overflow
      cmsg.inFunError("out of memory");
      return (void*)0;
    }

    long tmpCS = cLine->countSegment;
    sSegment* tmpSeg = NULL;
    tmpSeg = static_cast<sSegment*>(realloc(tmpSeg, sizeof(sSegment) * tmpCS));
    if (tmpSeg == NULL) {    // exit on memory overflow
      cmsg.inFunError("out of memory");
      return (void*)0;
    }

    for (long cl = 0; cl < tmpCL; cl++)
      tmpLine[cl] = cLine->aLine[cl];

    for (long cs = 0; cs < tmpCS; cs++)
      tmpSeg[cs] = cLine->aSegment[cs];

    Word res((void*)0);
    res.addr = new ColLine(tmpLine, tmpSeg, tmpCL, tmpCS);
    return res;
  }



  void* ColLine::Cast(void* addr) {
    return (new (addr) ColLine);
  }

  bool ColLine::TypeCheck(ListExpr type, ListExpr& errorInfo) {
    return nl->IsEqual(type, BasicType());
  }

  int ColLine::SizeOf() {
    return sizeof(sLine) + sizeof(sSegment);  // the result isn't meaningful
  }

//-----------------------------------------------------------------------------

/*
4 Class ColRegion for column-oriented representation of Regions

*/
  ColRegion::ColRegion() {}  // standard constructor doing nothing

  // non-standard constructor initializing the object with parameters
  ColRegion::ColRegion(sRegion* newTuple, sCycle* newCycle, sPoint* newPoint,
            long newCountTuple, long newCountCycle, long newCountPoint) {
    aRegion = newTuple;
    aCycle = newCycle;
    aPoint = newPoint;
    countRegion = newCountTuple;
    countCycle = newCountCycle;
    countPoint = newCountPoint;
  }

  // non-standard constructor initializing the object with minimum data
  ColRegion::ColRegion(bool min) {

    aRegion = NULL;
    aCycle = NULL;
    aPoint = NULL;

    aRegion = static_cast<sRegion*>(calloc(1, sizeof(sRegion)));
    aCycle = static_cast<sCycle*>(calloc(1, sizeof(sCycle)));
    aPoint = static_cast<sPoint*>(calloc(1, sizeof(sPoint)));

    countRegion = 0;
    countCycle = 0;
    countPoint = 0;
    stepRegion = 0;
    stepCycle = 0;
    stepPoint = 0;
  }

  // destructor - free allocated menory
  ColRegion::~ColRegion() {
    free(aRegion);
    free(aCycle);
    free(aPoint);
  }



  // returns the corresponding basic type
  const string ColRegion::BasicType() { return "aregion"; }



  // compares the type of the given object with class type
  const bool ColRegion::checkType(const ListExpr list) {
    return listutils::isSymbol(list, BasicType());
  }



  // description of the Secondo type for the user
  ListExpr ColRegion::Property() {
    return (nl->TwoElemList(
    nl->FiveElemList(
      nl->StringAtom("Signature"),
      nl->StringAtom("Example Type List"),
      nl->StringAtom("List Rep"),
      nl->StringAtom("Example List"),
      nl->StringAtom("Remarks")),
    nl->FiveElemList(
      nl->StringAtom("-> SIMPLE"),
      nl->StringAtom(BasicType()),
      nl->StringAtom("((real real) .. (real real))"),
      nl->StringAtom("((3.5 -6.0) (1.0 2) (-9.1 5))"),
      nl->StringAtom("aregion is defined by a list of regions."))));
  }



  // returns the number of regions without terminator
  long ColRegion::getCount() {
    return countRegion - 1;
  }



  // returns the number of cycles within the aCycle array
  long ColRegion::getCountCycles() {
    return countCycle - 1;  // subtract the terminator element
  }



  // returns the number of points within the aPoint array
  long ColRegion::getCountPoints() {
    return countPoint - 1;  // subtract the terminator element
  }



  // returns the number of points of a given region
  long ColRegion::getCountPoints(const long index) {
    return aRegion[index + 1].indexPoint - aRegion[index].indexPoint - 1;
  }



  void ColRegion::getRegion(const long index,
                            long &indexCycle, long &indexPoint,
                            double &mbbX1, double &mbbY1,
                            double &mbbX2, double &mbbY2) {
    indexCycle = aRegion[index].indexCycle;
    indexPoint = aRegion[index].indexPoint;
    mbbX1 = aRegion[index].mbbX1;
    mbbY1 = aRegion[index].mbbY1;
    mbbX2 = aRegion[index].mbbX2;
    mbbY2 = aRegion[index].mbbY2;
  }



  long ColRegion::getCycle(const long index) {
    return aCycle[index].index;
  }



  void ColRegion::getPoint(const long index, double &x, double &y) {
    x = aPoint[index].x;
    y = aPoint[index].y;
  }



  void* ColRegion::getRegionAddress() {
    return &aRegion[0].indexCycle;
  }



  void* ColRegion::getCycleAddress() {
    return &aCycle[0].index;
  }



  void* ColRegion::getPointAddress() {
    return &aPoint[0].x;
  }



  // extracts one region from aregion to region of the spatial algebra.
  bool ColRegion::createRegion(Region* region, const long index) {
    vector<vector<Point> > cycles;
    long firstCyc = aRegion[index].indexCycle;
    long lastCyc  = aRegion[index + 1].indexCycle; // first cycle of next region

    // scan all cycles of the region
    for (long indexCyc = firstCyc; indexCyc < lastCyc; indexCyc++) {
      vector<Point> cycle;
      long firstPnt = abs(aCycle[indexCyc].index);
      long lastPnt  = abs(aCycle[indexCyc + 1].index);

      // append all points of the actual cycle to the vector
      for (long indexPnt = firstPnt; indexPnt < lastPnt; indexPnt++) {
        Point p(true, aPoint[indexPnt].x, aPoint[indexPnt].y);
        cycle.push_back(p);
      }

      // append first point to close the cycle as demanded in RegionTools.h
      Point p(true, aPoint[firstPnt].x, aPoint[firstPnt].y);
      cycle.push_back(p);

      // ensure that faces are clockwise and holes are counter-clockwise
      if (((aCycle[indexCyc].index  < 0) &&  getDir(cycle)) ||
          ((aCycle[indexCyc].index >= 0) && !getDir(cycle)))
        reverseCycle(cycle);

      cycles.push_back(cycle);
    }

    region = buildRegion(cycles);

    // region->Print(cout);  // returns a valid value

    return true;
  }



  // allocates memory for the point array and appends the given point
  int ColRegion::appendPoint(const Point p, long &stepPoint) {
    double x = p.GetX();
    double y = p.GetY();

    // allocate more memory if actual allocated memory is insufficient
    if ((long) ((countPoint + 2) * sizeof(sPoint)) >= allocBytes[stepPoint]) {
      // allocate more memory - in C++ a type casting is necessary
      stepPoint++;
      aPoint = static_cast<sPoint*> (realloc(aPoint, allocBytes[stepPoint]));
      if (aPoint == NULL) {  // exit on memory overflow
        cmsg.inFunError("not enough memory for all points!");
        return false;
      }
    }

    aPoint[countPoint].x = x;
    aPoint[countPoint].y = y;

    // adjust mbb coordinates if neccessary
    if (x < aRegion[countRegion].mbbX1) aRegion[countRegion].mbbX1 = x;
    if (x > aRegion[countRegion].mbbX2) aRegion[countRegion].mbbX2 = x;
    if (y < aRegion[countRegion].mbbY1) aRegion[countRegion].mbbY1 = y;
    if (y > aRegion[countRegion].mbbY2) aRegion[countRegion].mbbY2 = y;

    countPoint++;  // increase index of point array

    return true;
  }



  // allocates memory for the cycle array and appends the give cycle
  int ColRegion::appendCycle(long cp, long &stepCycle) {

    // allocate more memory if actual allocated memory is insufficient
    if ((long)
      ((countCycle + 2) * sizeof(sCycle)) >= allocBytes[stepCycle]) {
      // allocate more memory - in C++ a type casting is necessary
      aCycle =static_cast<sCycle*>
               (realloc(aCycle, allocBytes[++stepCycle]));
      if (aCycle == NULL) {  // exit on memory overflow
        cmsg.inFunError("not enough memory for all cycles!");
        return false;
      }
    }

    aCycle[countCycle].index = cp;
    countCycle++;

    return true;
  }



  // This function appends a region datatype of the spatial algebra
  // to an attrarray of regions of the column spatial algebra.
  bool ColRegion::append(Region* region) {

    if(!region->IsDefined()) {
      cout << "region is undefined!" << endl;
      return false;
    }

    if(region->IsEmpty()) {
      cout << "region is empty" << endl;
      return false;
    }

    // calculate and allocate sufficient memory for the tuple array
    while ((long)
          ((countRegion + 2) * sizeof(sRegion)) >= allocBytes[stepRegion]) {
      stepRegion++;
    }
    aRegion = static_cast<sRegion*> (realloc(aRegion, allocBytes[stepRegion]));
    if (aRegion == NULL) {  // exit on memory overflow
      cmsg.inFunError("not enough memory for all regions!");
      return false;
    }

    // initialize tuple array with region
    aRegion[countRegion].indexCycle = countCycle;
    aRegion[countRegion].indexPoint = countPoint;

    // mbb set to extremes, so they will be adapted in any case
    aRegion[countRegion].mbbX1 = 999999;
    aRegion[countRegion].mbbY1 = 999999;
    aRegion[countRegion].mbbX2 = -999999;
    aRegion[countRegion].mbbY2 = -999999;

    Point outputP, leftoverP;
    HalfSegment hs, hsnext;

    // make a copy of the input region to avoid modifying the original data
    Region *rCopy=new Region(*region, true);

    rCopy->LogicSort();  // sort is important for consecutive iterations
    rCopy->Get(0, hs);  // extract first hs

    int currFace = hs.attr.faceno;     // set actual face to face of first hs
    int currCycle = hs.attr.cycleno;   // set actual cycle to cycle of first hs

    // calculate coordinates of first point in region
    rCopy->Get(1, hsnext);  // extract second hs
    if ((hs.GetLeftPoint() == hsnext.GetLeftPoint()) ||
       ((hs.GetLeftPoint() == hsnext.GetRightPoint()))) {
      outputP = hs.GetRightPoint();
      leftoverP = hs.GetLeftPoint();
    } else if ((hs.GetRightPoint() == hsnext.GetLeftPoint()) ||
              ((hs.GetRightPoint() == hsnext.GetRightPoint()))) {
      outputP = hs.GetLeftPoint();
      leftoverP = hs.GetRightPoint();
    } else {
      cmsg.inFunError("Wrong data format: discontiguous segments!");
      return false;
    }

    // append first point to aPoint[]
    if (!appendPoint(outputP, stepPoint)) {
      cmsg.inFunError("error on appending first point!");
      return false;
    }

      // append first cycle (face) to aCycle[]
    if (!appendCycle(countPoint - 1, stepCycle)) {
      cmsg.inFunError("error on appending first face!");
      return false;
    }

    // main loop: scan all remaining halfsegments
    for (int i = 1; i < rCopy->Size(); i++) {
      rCopy->Get(i, hs);  // extract actual hs

      if (hs.attr.faceno == currFace) {  // hs belongs to actual face

        if (hs.attr.cycleno == currCycle) {  // hs belongs to actual cycle
          // calculate coordinates of actual point in region
          outputP=leftoverP;
          if (hs.GetLeftPoint() == leftoverP)
            leftoverP = hs.GetRightPoint();
          else if (hs.GetRightPoint() == leftoverP) {
            leftoverP = hs.GetLeftPoint();
          } else {
            cmsg.inFunError("Wrong data format: discontiguous segments!");
            return false;
          }

          // append actual point to aPoint[]
          if (!appendPoint(outputP, stepPoint)) {
            cmsg.inFunError("error on appending point!");
            return false;
          }

        } else {  // hs belongs to new cycle = hole
          currCycle = hs.attr.cycleno;
          rCopy->Get(i+1, hsnext);

          if ((hs.GetLeftPoint() == hsnext.GetLeftPoint()) ||
             ((hs.GetLeftPoint() == hsnext.GetRightPoint()))) {
            outputP = hs.GetRightPoint();
            leftoverP = hs.GetLeftPoint();
          } else if ((hs.GetRightPoint() == hsnext.GetLeftPoint()) ||
                    ((hs.GetRightPoint() == hsnext.GetRightPoint()))) {
            outputP = hs.GetLeftPoint();
            leftoverP = hs.GetRightPoint();
          } else {
            cmsg.inFunError("Wrong data format: discontiguous segments!");
            return false;
          }

          // append new point to aPoint[]
          if (!appendPoint(outputP, stepPoint)) {
            cmsg.inFunError("error on appending point!");
            return false;
          }

          // append new cycle (hole = -index) to aCycle[]
          if (!appendCycle((countPoint - 1) * (-1), stepCycle)) {
            cmsg.inFunError("error on appending hole!");
            return false;
          }
        }
      } else {  // hs belongs to new face
        currFace = hs.attr.faceno;
        currCycle = hs.attr.cycleno;
        rCopy->Get( i+1, hsnext );
        if ((hs.GetLeftPoint() == hsnext.GetLeftPoint()) ||
           ((hs.GetLeftPoint() == hsnext.GetRightPoint()))) {
          outputP = hs.GetRightPoint();
          leftoverP = hs.GetLeftPoint();
        } else if ((hs.GetRightPoint() == hsnext.GetLeftPoint()) ||
                  ((hs.GetRightPoint() == hsnext.GetRightPoint()))) {
          outputP = hs.GetLeftPoint();
          leftoverP = hs.GetRightPoint();
        } else {
          cmsg.inFunError("Wrong data format: discontiguous segments!");
          return false;
        }

        if (!appendPoint(outputP, stepPoint)) {
          cmsg.inFunError("error on appending point!");
          return false;
        }

        if (!appendCycle(countPoint - 1, stepCycle)) {
          cmsg.inFunError("error on appending face!");
          return false;
        }
      }
    }

    countRegion++;

    return true;
  }



/*
The ~finalize~ function appends a terminator to each array of the aregion type.

*/
  void ColRegion::finalize() {
    aPoint[countPoint].x = 0;
    aPoint[countPoint].y = 0;
    aCycle[countCycle].index = countPoint;
    aRegion[countRegion].indexCycle = countCycle;
    aRegion[countRegion].indexPoint = countPoint;
    aRegion[countRegion].mbbX1 = 0;
    aRegion[countRegion].mbbY1 = 0;
    aRegion[countRegion].mbbX2 = 0;
    aRegion[countRegion].mbbY2 = 0;

    countRegion++;
    countCycle++;
    countPoint++;

    // truncate oversized memory to allocate only the real used memory
    aRegion = static_cast<sRegion*>
              (realloc(aRegion, countRegion * sizeof(sRegion)));
    aCycle = static_cast<sCycle*>
             (realloc(aCycle, countCycle * sizeof(sCycle)));
    aPoint = static_cast<sPoint*>
             (realloc(aPoint, countPoint * sizeof(sPoint)));
  }



/*
merges two aregions into one aregion.

*/
  bool ColRegion::merge(ColRegion* cRegion1, ColRegion* cRegion2) {
    long ccr1 = cRegion1->getCount();
    long ccr2 = cRegion2->getCount();
    long ccc1 = cRegion1->getCountCycles();
    long ccc2 = cRegion2->getCountCycles();
    long ccp1 = cRegion1->getCountPoints();
    long ccp2 = cRegion2->getCountPoints();

    // allocate memory for the input arrays
    aRegion = static_cast<sRegion*>
              (realloc(aRegion, (ccr1 + ccr2 + 1) * sizeof(sRegion)));
    aCycle = static_cast<sCycle*>
             (realloc(aCycle, (ccc1 + ccc2 + 1) * sizeof(sCycle)));
    aPoint = static_cast<sPoint*>
             (realloc(aPoint, (ccp1 + ccp2 + 1) * sizeof(sPoint)));

/*
The following instructions are commented out because there is still
a problem with memory allocation. Instead the loops below are used.

----
    // fast block copy
    memcpy(&aRegion[0].indexCycle, cRegion1->getRegionAddress(),
           ccr1 * sizeof(sRegion));
    memcpy(&aRegion[ccr1].indexCycle, cRegion2->getRegionAddress(),
           (ccr1 + 1) * sizeof(sRegion));

    memcpy(&aCycle[0].index, cRegion1->getCycleAddress(),
           ccc1 * sizeof(sCycle));
    memcpy(&aCycle[ccc1].index, cRegion2->getCycleAddress(),
           (ccc1 + 1) * sizeof(sCycle));

    memcpy(&aPoint[0].x, cRegion1->getPointAddress(),
           ccp1 * sizeof(sPoint));
    memcpy(&aPoint[ccp1].x, cRegion2->getPointAddress(),
           (ccp1 + 1) * sizeof(sPoint));

    // correct indices of second aRegion
    for (long i = ccr1; i < (ccr1 + ccr2 + 1); i++) {
      aRegion[i].indexCycle += ccc1;
      aRegion[i].indexPoint += ccp1;
    }

    // correct indices of second aCycle
    for (long i = ccc1; i < (ccc1 + ccc2 + 1); i++)
      aCycle[i].index += ccp1;
----

*/

    // scan all regions of first aregion
    for (long i = 0; i < ccr1; i++)
      cRegion1->getRegion(i, aRegion[i].indexCycle, aRegion[i].indexPoint,
                          aRegion[i].mbbX1, aRegion[i].mbbY1,
                          aRegion[i].mbbX2, aRegion[i].mbbY2);

    // scan all regions of second aregion including terminator
    long iCycle, iPoint;
    for (long i = 0; i <= ccr2; i++) {
      cRegion2->getRegion(i, iCycle, iPoint,
                          aRegion[i+ccr1].mbbX1, aRegion[i+ccr1].mbbY1,
                          aRegion[i+ccr1].mbbX2, aRegion[i+ccr1].mbbY2);
      aRegion[i+ccr1].indexCycle = iCycle + ccc1;
      aRegion[i+ccr1].indexPoint = iPoint + ccp1;
    }

    // scan all cycles of first aregion
    for (long i = 0; i < ccc1; i++)
      aCycle[i].index = cRegion1->getCycle(i);

      // scan all cycles of second aregion including terminator
    for (long i = 0; i <= ccc2; i++)
      aCycle[i+ccc1].index = cRegion2->getCycle(i) + ccp1;

    // scan all points of first aregion
    for (long i = 0; i < ccp1; i++)
      cRegion1->getPoint(i, aPoint[i].x, aPoint[i].y);

      // scan all points of second aregion including terminator
    for (long i = 0; i <= ccp2; i++)
      cRegion2->getPoint(i, aPoint[i+ccp1].x, aPoint[i+ccp1].y);


    countRegion = ccr1 + ccr2 + 1;
    countCycle = ccc1 + ccc2 + 1;
    countPoint = ccp1 + ccp2 + 1;

    return true;
  }



  // test output of the arrays aRegion, aCycle and aPoint and there parameters
  void ColRegion::showArrays(string title, bool showPoints) {

    cout << "\n--------------------------------------------\n" << title << "\n";

    // output of the array aRegion
    cout << "aRegion (" << countRegion << "):\n";
    cout << "Bytes allocated: " << countRegion * sizeof(aRegion[0]) << "\n";
    cout << "index\tcycle\tpoint\tmbb-l\tmbb-b\tmbb-r\tmbb-t\n";
    for (long cr = 0; cr < countRegion; cr++) {
      cout << cr << ":\t" << aRegion[cr].indexCycle << "\t"
           << aRegion[cr].indexPoint << "\t"
           << aRegion[cr].mbbX1 << "\t"
           << aRegion[cr].mbbY1 << "\t"
           << aRegion[cr].mbbX2 << "\t"
           << aRegion[cr].mbbY2 << "\n";
    }

    // output of the array aCycle
    cout << "aCycle (" << countCycle << "):\n";
    cout << "Bytes allocated: " << countCycle * sizeof(aCycle[0]) << "\n";
    cout << "index\tpoint\n";
    for (long cc = 0; cc < countCycle; cc++) {
      cout << cc << ":\t" << aCycle[cc].index << "\n";
    }

    // output of the array aPoint
    cout << "aPoint (" << countPoint << "):\n";
    cout << "Bytes allocated: " << countPoint * sizeof(aPoint[0]) << "\n";
    if (showPoints) {
      cout << "index\tx\ty\n";
      for (long cp = 0; cp < countPoint; cp++) {
        cout << cp << ":\t" << aPoint[cp].x << "\t" << aPoint[cp].y << "\n";
      }
    }

    cout << "--------------------------------------------\n";
  }



/*
Checks whether two segments intersect. It uses the vector equation of two
lines, which are derived from the two
segments s1 = (x1, y1, x2, y2) and s2 = (x3, y3, x4, y4).

*/
  inline bool ColRegion::intersects(double x1, double y1,
                                    double x2, double y2,
                                    double x3, double y3,
                                    double x4, double y4) {
    double s1x = x2-x1;
    double s1y = y2-y1;
    double s2x = x4-x3;
    double s2y = y4-y3;

    // vector equations of both straight lines solved for scalars t and s
    double s = (s1x*(y1-y3) - s1y*(x1-x3)) / (s1x*s2y - s2x*s1y);
    double t = (s2x*(y1-y3) - s2y*(x1-x3)) / (s1x*s2y - s2x*s1y);

    // return true if a point lies on both segments, otherwise return false
    return (s >= 0 && s <= 1 && t >= 0 && t <= 1);
  }



/*
The following function checks whether a single point is inside a region
Needs x and y of the point and the index of aRegion.

*/
inline bool ColRegion::pointInsideRegion(double x, double y, long idReg) {

  // define a x coordinate outside the region
  long x2 = aRegion[idReg].mbbX1 - 1;
  // first and last cycle within the region
  long ccStart = aRegion[idReg].indexCycle;
  long ccStop = aRegion[idReg + 1].indexCycle;
  long cpStart, cpStop;
  // number of intersections
  long isCount = 0;
  double x3, y3, x4, y4;

  // scan all cycles of the actual region
  for (long idCyc = ccStart; idCyc < ccStop; idCyc++) {
    // set first and last point of the actual cycle
    cpStart = abs(aCycle[idCyc].index);
    cpStop = abs(aCycle[idCyc + 1].index);

    // scan all points
    for (long idPnt = cpStart; idPnt < cpStop; idPnt++) {

      x3 = aPoint[idPnt].x;
      y3 = aPoint[idPnt].y;

      // set next point of cycle
      if ((idPnt + 1) < cpStop) {
        x4 = aPoint[idPnt + 1].x;
        y4 = aPoint[idPnt + 1].y;
      } else {
        // if cycle's last point, then set first point to close the cycle
        x4 = aPoint[cpStart].x;
        y4 = aPoint[cpStart].y;
      }

      // avoid double counting of vertexes on vertical segments
      // the intersection counts only if the second vertex of the segment
      // lies below the tested point
      if ((y == y3 && y3 > y4) || (y == y4 && y4 > y3))
        continue;

      if (intersects(x, y, x2, y, x3, y3, x4, y4))
        isCount++;
    }
  }

  // if isCount is odd then segment intersects with region
  return isCount & 1;
}



/*
checks for each point of the ~ColPoint~ object whether it is inside
one of the regions of the actual ~ColRegions~ object.
Precondition:
- cycle points must be in clockwise or in counterclockwise order,
  no matter if they are faces or holes. this precondition is always true.
- the first point of a region must be the leftmost/bottom point.
  this precondition is always true.
Complexity: $O(m . n)$,
where ~m~ is the number of points and ~n~ is the number of regions.
known weakness:
if a tested region overlaps the 180th longitude then the function fails.

*/
  LongInts* ColRegion::pointsInside(ColPoint* cPoint) {

    const long cp1 = cPoint->getCount();  // number of points
    long cr = countRegion - 1;  // number of regions without terminator
    double idPx1;
    double idPy1;

    // result array of matching indices
    LongInts* id = new LongInts();

    // scan all points
    for (long idP = 0; idP < cp1; idP++) {

      // this is the actual point to be checked
      idPx1 = cPoint->getX(idP);
      idPy1 = cPoint->getY(idP);

      // scan all regions
      for (long idReg = 0; idReg < cr; idReg++) {
        // check if actual point is inside the bounding box of actual region
        if ((idPx1 < aRegion[idReg].mbbX1) || (idPx1 > aRegion[idReg].mbbX2) ||
            (idPy1 < aRegion[idReg].mbbY1) || (idPy1 > aRegion[idReg].mbbY2))
          continue;  // if not inside bbox then continue with next region

        if (pointInsideRegion(idPx1, idPy1, idReg)) {
          id->Append(idP);
          // no need to check further regions, continue with next point
          break;
        }
      }
    }

    return id;
  }



/*
Checks for each line of the ~ColLine~ object whether it is inside
one of the regions of the actual ~ColRegions~ object.
To do so, the first point of the line is checked whether it is inside or
outside the boundig box of the region. If it is outside the line
can not be inside the region and the next line is processed.
If it is inside, the number of intersections are countes. if the
number is even then the line is completely inside one of the regions.
any other case the line only crosses the region.

*/
  LongInts* ColRegion::linesInside(ColLine* cLine) {

    const long cl1 = cLine->getCount();  // number of lines
    long cr = countRegion -1;      // number of regions without terminator
    long cc, cp, idSegStart, idSegStop;
    cc = cp = idSegStart = idSegStop = 0;
    double x1, y1, x2, y2, x4, y4;
    x1 = y1 = x2 = y2 = x4 = y4 = 42;
    bool intersect_flag = false;  // controls the loops

    // result array of matching indices
    LongInts* id = new LongInts();

    // scan all lines
    for (long idL = 0; idL < cl1; idL++) {

      // set first and last segment of the line
      cLine->getLineSegments(idL, idSegStart, idSegStop);

      // scan all regions
      for (long idReg = 0; idReg < cr; idReg++) {

        // if first point is outside region -> continue with next region
        cLine->getSegmentPoints(idSegStart, x1, y1, x2, y2);
        if (!pointInsideRegion(x1, y1, idReg)) break;

        intersect_flag = false;

        // scan all line segments
        for (long idLS = idSegStart; ((idLS < idSegStop) && !intersect_flag);
             idLS++) {

          // get points of actual line segment
          cLine->getSegmentPoints(idSegStart, x1, y1, x2, y2);
          // number of cycles within the actual region
          cc = aRegion[idReg + 1].indexCycle;

          // scan all cycles of the actual region
          for (long idCyc = aRegion[idReg].indexCycle;
               (idCyc < cc) && !intersect_flag; idCyc++) {
            // set last point of the actual cycle
            cp = aCycle[idCyc + 1].index;

            // scan all points
            for (long idPnt = aCycle[idCyc].index; idPnt < cp; idPnt++) {

              // set next point of cycle
              if ((idPnt + 1) < cp) {  // next point
                x4 = aPoint[idPnt + 1].x;
                y4 = aPoint[idPnt + 1].y;
              } else {  // first point to close the cycle
                x4 = aPoint[aCycle[idCyc].index].x;
                y4 = aPoint[aCycle[idCyc].index].y;
              }

              // if intersection between line segment and region segment
              // -> continue with next region
              if (intersects(x1, y1, x2, y2,
                             aPoint[idPnt].x, aPoint[idPnt].y, x4, y4)) {
                intersect_flag = true;
                break;
              }
            }
          }
        }
        // if no intersection has occured -> append
        if (!intersect_flag) id->Append(idL);
      }
    }

    return id;
  }



/*
checks for each point of the ~ColPoint~ object whether it is inside
one of the regions of the actual ~ColRegions~ object
and returns the region indices.

*/
  LongInts* ColRegion::containsPoints(ColPoint* cPoint) {

    const long cp1 = cPoint->getCount();  // number of points
    long cr = countRegion - 1;  // number of regions without terminator
    double idPx1;
    double idPy1;

    // result array of matching indices
    LongInts* id = new LongInts();
    // scan all regions
    for (long idReg = 0; idReg < cr; idReg++) {

      // scan all points
      for (long idP = 0; idP < cp1; idP++) {

        // this is the actual point to be checked
        idPx1 = cPoint->getX(idP);
        idPy1 = cPoint->getY(idP);

        // check if actual point is inside the bounding box of actual region
        if ((idPx1 < aRegion[idReg].mbbX1) || (idPx1 > aRegion[idReg].mbbX2) ||
            (idPy1 < aRegion[idReg].mbbY1) || (idPy1 > aRegion[idReg].mbbY2))
          continue;  // if not inside bbox then continue with next point

        if (pointInsideRegion(idPx1, idPy1, idReg)) {
          id->Append(idReg);
          // no need to check further points, continue with next region
          break;
        }
      }
    }

    return id;
  }



  Word ColRegion::In(const ListExpr typeInfo, const ListExpr instance,
                 const int errorPos, ListExpr& errorInfo, bool& correct) {
    correct = false;        // assume that there will be an error
    Word error(static_cast<void*>(0));  // create an error result pointing at 0
    errorInfo = listutils::typeError("Error in ColRegion::In!!!");

    if (listutils::isSymbolUndefined(instance)) {
      cmsg.inFunError("Symbol is undefined!");
      return error;
    }

    if (nl->IsAtom(instance)) {  // exit when the nested list is an atom
      cmsg.inFunError("Nested list must not be an atom!");
      return error;
    }

    // initialize the arrays which contain the latter input values
    sRegion* inRegion = NULL;
    sCycle* inCycle = NULL;
    sPoint* inPoint = NULL;

    long cr = 0;  // index counter of the tuple array
    long cc = 0;  // index counter of the cycle array
    long cp = 0;  // index counter of the point array

    // index to the array allocbytes of actual allocated memory for each array
    long stepRegion = 0;
    long stepCycle = 0;
    long stepPoint = 0;

    // allocate memory for the input arrays
    inRegion = static_cast<sRegion*>(realloc(inRegion, allocBytes[stepRegion]));
    inCycle = static_cast<sCycle*>(realloc(inCycle, allocBytes[stepCycle]));
    inPoint = static_cast<sPoint*>(realloc(inPoint, allocBytes[stepPoint]));
    int cycleSign = 1;  // 1 marks an outer cycle and -1 marks an inner cycle
    // loops to parse a nested list of regions
    ListExpr regionNL = instance;
    while (!nl->IsEmpty(regionNL)) {  // as long as there are regions left
      // allocate more memory if the actual allocated memory is insufficient
      if ((long)
        ((cr + 2) * sizeof(sRegion)) >= allocBytes[stepRegion]) {
        // allocate more memory - in C++ a type casting is necessary
        inRegion = static_cast<sRegion*>
                  (realloc(inRegion, allocBytes[++stepRegion]));
        if (inRegion == NULL) {  // exit on memory overflow
          cmsg.inFunError("not enough memory!");
          return error;
        }
      }

      inRegion[cr].indexCycle = cc;
      inRegion[cr].indexPoint = cp;

      // values for the actual minimum bounding rectangle per region
      inRegion[cr].mbbX1 = 999999;
      inRegion[cr].mbbY1 = 999999;
      inRegion[cr].mbbX2 = -999999;
      inRegion[cr].mbbY2 = -999999;

      ListExpr tupleNL = nl->First(regionNL);

      if (nl->IsAtom(tupleNL)) {
        cmsg.inFunError("expected a nested list for regions!");
        return error;
      }

      while (!nl->IsEmpty(tupleNL)) {  // faces
        ListExpr faceNL = nl->First(tupleNL);

        if (nl->IsAtom(faceNL)) {
          cmsg.inFunError("expected a nested list for faces!");
          return error;
        }

        cycleSign = 1;  // the first cycle of a face is always an outer cycle

        while (!nl->IsEmpty(faceNL)) {  // cycles
          // allocate more memory if actual allocated memory is insufficient
          if ((long)
            ((cc + 2) * sizeof(sCycle)) >= allocBytes[stepCycle]) {
            // allocate more memory - in C++ a type casting is necessary
            inCycle =static_cast<sCycle*>
                     (realloc(inCycle, allocBytes[++stepCycle]));
            if (inCycle == NULL) {  // exit on memory overflow
              cmsg.inFunError("not enough memory!");
              return error;
            }
          }

          inCycle[cc++].index = cycleSign * cp;
          cycleSign = -1;  // any other cycles within a face are holes
          ListExpr cycleNL = nl->First(faceNL);
          if (nl->IsAtom(cycleNL)) {
            cmsg.inFunError("expected a nested list for cycles!");
            return error;
          }

          while (!nl->IsEmpty(cycleNL)) {  // points
            ListExpr pointNL = nl->First(cycleNL);
            if ((!nl->HasLength(pointNL, 2)) ||
                (!listutils::isNumeric(nl->First(pointNL))) ||
                (!listutils::isNumeric(nl->Second(pointNL)))) {
              cmsg.inFunError("expected an atom with two numeric values!");
              return error;  // exit on invalid point
            }

            // allocate more memory if actual allocated memory is insufficient
            if ((long) ((cp + 2) * sizeof(sPoint)) >= allocBytes[stepPoint]) {
              // allocate more memory - in C++ a type casting is necessary
              inPoint = static_cast<sPoint*>
                (realloc(inPoint, allocBytes[++stepPoint]));
              if (inPoint == NULL) {  // exit on memory overflow
                cmsg.inFunError("not enough memory!");
                return error;
              }
            }

            double x = listutils::getNumValue((nl->First(pointNL)));
            double y = listutils::getNumValue((nl->Second(pointNL)));
            inPoint[cp].x = x;
            inPoint[cp].y = y;

            // calculate the mbb
            if (x < inRegion[cr].mbbX1) inRegion[cr].mbbX1 = x;
            if (x > inRegion[cr].mbbX2) inRegion[cr].mbbX2 = x;
            if (y < inRegion[cr].mbbY1) inRegion[cr].mbbY1 = y;
            if (y > inRegion[cr].mbbY2) inRegion[cr].mbbY2 = y;

            cp++;

            cycleNL = nl->Rest(cycleNL);  // "move" to next point
          }
          faceNL = nl->Rest(faceNL);      // "move" to next cycle
        }
        tupleNL = nl->Rest(tupleNL);      // "move" to next face
      }

      cr++;

      regionNL = nl->Rest(regionNL);      // "move" to next region
    }

    // add terminator entries to arrays.
    inPoint[cp].x = 0;
    inPoint[cp].y = 0;
    inCycle[cc].index = cp;
    inRegion[cr].indexCycle = cc;
    inRegion[cr].indexPoint = cp;
    inRegion[cr].mbbX1 = 0;
    inRegion[cr].mbbY1 = 0;
    inRegion[cr].mbbX2 = 0;
    inRegion[cr].mbbY2 = 0;

    // truncate oversized memory to allocate only the real used memory
    inRegion = static_cast<sRegion*>(realloc(inRegion, ++cr * sizeof(sRegion)));
    inCycle = static_cast<sCycle*>(realloc(inCycle, ++cc * sizeof(sCycle)));
    inPoint = static_cast<sPoint*>(realloc(inPoint, ++cp * sizeof(sPoint)));
    cout << cr * sizeof(sRegion) + cc * sizeof(sCycle) + cp * sizeof(sPoint)
         << " bytes used\n";
    Word answer(static_cast<void*>(0));

    // create new region object with the just read in arrays
    answer.addr = new ColRegion(inRegion, inCycle, inPoint, cr, cc, cp);
    correct = true;

    return answer;
  }



  ListExpr ColRegion::Out(ListExpr typeInfo, Word value) {
    ColRegion* cRegion = static_cast<ColRegion*>(value.addr);

    if (cRegion->countRegion == 0) {  // no elements lead to error
      return listutils::emptyErrorInfo();
    }

    // counters for index arrays aRegion and aCycle starting at second entries
    long cr = 1;
    long cc = 1;

    // initialize heads and tails for working lists
    ListExpr regionNL     = nl->TheEmptyList();
    ListExpr regionNLLast = regionNL;
    ListExpr tupleNL      = nl->TheEmptyList();
    ListExpr tupleNLLast  = tupleNL;
    ListExpr faceNL       = nl->TheEmptyList();
    ListExpr faceNLLast   = faceNL;
    ListExpr cycleNL      = nl->OneElemList(nl->TwoElemList(
                              nl->RealAtom(cRegion->aPoint[0].x),
                              nl->RealAtom(cRegion->aPoint[0].y)));
    ListExpr cycleNLLast  = cycleNL;

    // main loop: traverse all points in the point array
    for (long cp = 1; cp < cRegion->countPoint; cp++) {

      // check whether a new cycle appears
      if (cp == abs(cRegion->aCycle[cc].index)) {

        // check whether the last cycle was an outer cycle
        if (cRegion->aCycle[cc - 1].index >= 0) {  // positive value = face
          faceNL = nl->OneElemList(cycleNL);
          faceNLLast = faceNL;
        } else {  // negative value = hole = append to last face
          faceNLLast = nl->Append(faceNLLast, cycleNL);
        }

        // check whether the actual cycle is an outer cycle = new face
        if (cRegion->aCycle[cc].index >= 0) {

          if (nl->IsEmpty(tupleNL)) {  // append last face to new tuple
            tupleNL = nl->OneElemList(faceNL);
            tupleNLLast = tupleNL;
          } else {  // append last face to last tuple
            tupleNLLast = nl->Append(tupleNLLast, faceNL);
          }

          // check whether a new tuple appears
          if (cp == cRegion->aRegion[cr].indexPoint) {

            if (nl->IsEmpty(regionNL)) {  // append last tuple to new region
              regionNL = nl->OneElemList(tupleNL);
              regionNLLast = regionNL;
            } else {  // append last tuple to last region
              regionNLLast = nl->Append(regionNLLast, tupleNL);
            }

            // clear tuple and increase region counter
            tupleNL = nl->TheEmptyList();
            tupleNLLast = tupleNL;

            cr++;
          }
        }

        // initialize new cycle and increase cycle counter
        // note: terminator will be created too, but not appended
        cycleNL = nl->OneElemList(nl->TwoElemList(
                    nl->RealAtom(cRegion->aPoint[cp].x),
                    nl->RealAtom(cRegion->aPoint[cp].y)));
        cycleNLLast = cycleNL;

        cc++;
      } else {  // append new point to actual cycle
        cycleNLLast = nl->Append(cycleNLLast, nl->TwoElemList(
                        nl->RealAtom(cRegion->aPoint[cp].x),
                        nl->RealAtom(cRegion->aPoint[cp].y)));
      }
    }
    return regionNL;
  }



  Word ColRegion::Create(const ListExpr typeInfo) {
    Word answer(static_cast<void*>(0));
    sPoint *inPoint = NULL;
    sCycle *inCycle = NULL;
    sRegion *inRegion = NULL;
    // create a new region object

    answer.addr = new ColRegion(inRegion, inCycle, inPoint, 0, 0, 0);
    return answer;  // return its adress
  }



  void ColRegion::Delete(const ListExpr typeInfo, Word& w) {
    ColRegion* cRegion = static_cast<ColRegion*>(w.addr);
    delete cRegion;
    w.addr = 0;
  }



  bool ColRegion::Open(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value) {

    cout << "open aregion...\n";
    // initialize arrays
    sRegion* region = NULL;
    sCycle* cycle = NULL;
    sPoint* point = NULL;

    // declare counters
    long cr;
    long cc;
    long cp;

    // declare read in values
    long valueL;
    double valueD;

    size_t sizeL = sizeof(long);
    size_t sizeD = sizeof(double);

    // 1. read the counters from disc
    bool ok = (valueRecord.Read(&cr, sizeL, offset) == sizeL);
    offset += sizeL;

    ok = ok && (valueRecord.Read(&cc, sizeL, offset) == sizeL);
    offset += sizeL;

    ok = ok && (valueRecord.Read(&cp, sizeL, offset) == sizeL);
    offset += sizeL;

    // allocate memory depending on the counters
    region = static_cast<sRegion*>(malloc(cr * sizeof(sRegion)));
    if (region == NULL) {    // exit on memory overflow
      cmsg.inFunError("out of memory!");
      return false;
    }

    cycle = static_cast<sCycle*>(malloc(cc * sizeof(sCycle)));
    if (cycle == NULL) {    // exit on memory overflow
      cmsg.inFunError("out of memory!");
      return false;
    }

    point = static_cast<sPoint*>(malloc(cp * sizeof(sPoint)));
    if (point == NULL) {    // exit on memory overflow
      cmsg.inFunError("out of memory!");
      return false;
    }

    // 2. read the tuples and store them into the array tuple
    for (long i = 0; i < cr; i++) {
      ok = ok && (valueRecord.Read(&valueL, sizeL, offset) == sizeL);
      region[i].indexCycle = valueL;
      offset += sizeL;

      ok = ok && (valueRecord.Read(&valueL, sizeL, offset) == sizeL);
      region[i].indexPoint = valueL;
      offset += sizeL;

      ok = ok && (valueRecord.Read(&valueD, sizeD, offset) == sizeD);
      region[i].mbbX1 = valueD;
      offset += sizeD;

      ok = ok && (valueRecord.Read(&valueD, sizeD, offset) == sizeD);
      region[i].mbbY1 = valueD;
      offset += sizeD;

      ok = ok && (valueRecord.Read(&valueD, sizeD, offset) == sizeD);
      region[i].mbbX2 = valueD;
      offset += sizeD;

      ok = ok && (valueRecord.Read(&valueD, sizeD, offset) == sizeD);
      region[i].mbbY2 = valueD;
      offset += sizeD;

      if (!ok) { break; }
    }

    // 3. read the cycles and store them into the array tuple
    for (long i = 0; i < cc; i++) {
      ok = ok && (valueRecord.Read(&valueL, sizeL, offset) == sizeL);
      offset += sizeL;
      cycle[i].index = valueL;
      if (!ok) { break; }
    }

    // 4. read the points and store them into the array tuple
    for (long i = 0; i < cp; i++) {
      ok = ok && (valueRecord.Read(&valueD, sizeD, offset) == sizeD);
      point[i].x = valueD;
      offset += sizeD;
      ok = ok && (valueRecord.Read(&valueD, sizeD, offset) == sizeD);
      point[i].y = valueD;
      offset += sizeD;
      if (!ok) { break; }
    }

    if (ok) {  // create a new ColRegion and store the read values in it
      value.addr = new ColRegion(region, cycle, point, cr, cc, cp);
    } else {  // error
      value.addr = 0;
    }

    cout << cr * sizeof(sRegion) + cc * sizeof(sCycle) + cp * sizeof(sPoint)
         << " bytes used." << endl;

    return ok;  // ok can be true or false
  }



  bool ColRegion::Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value) {

    cout << "save aregion...\n";

    ColRegion* cRegion = static_cast<ColRegion*>(value.addr);
    size_t sizeL = sizeof(long);
    size_t sizeD = sizeof(double);

    // order of counters and arrays are tuples - cycles - points
    long cr = cRegion->countRegion;
    long cc = cRegion->countCycle;
    long cp = cRegion->countPoint;
    long valueL;
    double valueD;

    // 1. write the counters to disc
    bool ok = valueRecord.Write(&cr, sizeL, offset);
    offset += sizeL;

    ok = ok && valueRecord.Write(&cc, sizeL, offset);
    offset += sizeL;

    ok = ok && valueRecord.Write(&cp, sizeL, offset);
    offset += sizeL;

    // 2. write the tuple array to disc
    for (long i = 0; i < cr; i++) {
      valueL = cRegion->aRegion[i].indexCycle;

      ok = ok && valueRecord.Write(&valueL, sizeL, offset);
      offset += sizeL;
      valueL = cRegion->aRegion[i].indexPoint;

      ok = ok && valueRecord.Write(&valueL, sizeL, offset);
      offset += sizeL;
      valueD = cRegion->aRegion[i].mbbX1;

      ok = ok && valueRecord.Write(&valueD, sizeD, offset);
      offset += sizeD;
      valueD = cRegion->aRegion[i].mbbY1;

      ok = ok && valueRecord.Write(&valueD, sizeD, offset);
      offset += sizeD;
      valueD = cRegion->aRegion[i].mbbX2;

      ok = ok && valueRecord.Write(&valueD, sizeD, offset);
      offset += sizeD;
      valueD = cRegion->aRegion[i].mbbY2;

      ok = ok && valueRecord.Write(&valueD, sizeD, offset);
      offset += sizeD;
    }
    // 3. write the cycle array to disc
    for (long i = 0; i < cc; i++) {
      valueL = cRegion->aCycle[i].index;
      ok = ok && valueRecord.Write(&valueL, sizeL, offset);
      offset += sizeL;
    }
    // 4. write the point array to disc
    for (long i = 0; i < cp; i++) {
      valueD = cRegion->aPoint[i].x;
      ok = ok && valueRecord.Write(&valueD, sizeD, offset);
      offset += sizeD;

      valueD = cRegion->aPoint[i].y;
      ok = ok && valueRecord.Write(&valueD, sizeD, offset);
      offset += sizeD;
    }

    return ok;
  }



  void ColRegion::Close(const ListExpr typeInfo, Word& w) {
    delete static_cast<ColRegion*>(w.addr);
    w.addr = 0;
  }



/*
~Clone~ makes a copy of a ~ColRegion~ object. Therefore it allocates memory
for the clone object and scans the arrays of the source object and copies
each entry to the clone object.

*/
  Word ColRegion::Clone(const ListExpr typeInfo, const Word& w) {
    ColRegion* cRegion = static_cast<ColRegion*>(w.addr);

    long tmpCR = cRegion->getCount() + 1; // add 1 for terminator
    sRegion* tmpRegion = NULL;
    tmpRegion = static_cast<sRegion*>
                (realloc(tmpRegion, sizeof(sRegion) * tmpCR));
    if (tmpRegion == NULL) {    // exit on memory overflow
      cmsg.inFunError("out of memory");
      return (void*)0;
    }

    long tmpCC = cRegion->getCountCycles() + 1;  // add terminator entry
    sCycle* tmpCycle = NULL;
    tmpCycle = static_cast<sCycle*>
               (realloc(tmpCycle, sizeof(sCycle) * tmpCC));
    if (tmpCycle == NULL) {    // exit on memory overflow
      cmsg.inFunError("out of memory");
      return (void*)0;
    }

    long tmpCP = cRegion->getCountPoints() + 1;  // add terminator entry
    sPoint* tmpPoint = NULL;
    tmpPoint = static_cast<sPoint*>
               (realloc(tmpPoint, sizeof(sPoint) * tmpCP));
    if (tmpPoint == NULL) {    // exit on memory overflow
      cmsg.inFunError("out of memory");
      return (void*)0;
    }

    cout << "sizeof(tmpRegion) = " << sizeof(sRegion) * tmpCR << endl
         << "sizeof(tmpCycle)  = " << sizeof(sCycle) * tmpCC << endl
         << "sizeof(tmppoint)  = " << sizeof(sPoint) * tmpCP << endl;

    for (long cr = 0; cr < tmpCR; cr++)
      tmpRegion[cr] = cRegion->aRegion[cr];

    for (long cc = 0; cc < tmpCC; cc++)
      tmpCycle[cc] = cRegion->aCycle[cc];

    for (long cp = 0; cp < tmpCP; cp++)
      tmpPoint[cp] = cRegion->aPoint[cp];

    Word res((void*)0);
    res.addr = new ColRegion(tmpRegion, tmpCycle, tmpPoint,
                             tmpCR, tmpCC, tmpCP);

    return res;
  }


  void* ColRegion::Cast(void* addr) {
    return (new (addr) ColRegion);
  }



  bool ColRegion::TypeCheck(ListExpr type, ListExpr& errorInfo) {
    return nl->IsEqual(type, BasicType());
  }



  int ColRegion::SizeOf() {
    return sizeof(sRegion) + sizeof(sCycle) + sizeof(sPoint);
  }

//------------------------------ Operators ------------------------------------

/*
5 Operators

5.1 Type Mapping

*/
ListExpr insideTM(ListExpr args) {
    string err = "{apoint | aline} x aregion expected\n";

    if (!nl->HasLength(args, 2)) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    if ((ColPoint::checkType(nl->First(args)) ||
         ColLine::checkType(nl->First(args))) &&
         ColRegion::checkType(nl->Second(args))) {
      return LongIntsTI(false).GetTypeExpr();  // list of long ints for id
    }

    return listutils::typeError(err);
}



ListExpr containsTM(ListExpr args) {
    string err = "aregion x apoint expected\n";

    if (!nl->HasLength(args, 2)) {
      return listutils::typeError(err + " (wrong number of arguments)");
    }

    if (ColRegion::checkType(nl->First(args)) &&
        ColPoint::checkType(nl->Second(args))) {
      return LongIntsTI(false).GetTypeExpr();  // list of long ints for id
    }
    return listutils::typeError(err);
}



ListExpr mapTM(ListExpr args) {

  if (!nl->HasLength(args,2)) {
     return listutils::typeError("Wrong number of arguments!");
  }

  // check for tuple stream
  if (!Stream<Tuple>::checkType(nl->First(args))) {
    return listutils::typeError("tuple stream "
                                "with spatial attribute expected!");
  }

  if (nl->AtomType(nl->Second(args))!=SymbolType) {
    return listutils::typeError("Second arg is no valid attribute name!");
  }

  // extract the attribute list
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr type;
  string name = nl->SymbolValue(nl->Second(args));
  int j = listutils::findAttribute(attrList, name, type);
  if (j==0) {
    return listutils::typeError("Attribute " + name + " not found!");
  };

  // check type and return corresponding attribute number
  // and result type using append mechanism
  ListExpr returnType = nl->Empty();

  if (Point::checkType(type))
    returnType = listutils::basicSymbol<ColPoint>();

  if (Line::checkType(type))
    returnType = listutils::basicSymbol<ColLine>();

  if (Region::checkType(type))
    returnType = listutils::basicSymbol<ColRegion>();

  if (!nl->IsEmpty(returnType)) {
    cout << "'" << name << "' is attribute " << j << ", type is '"
         << nl->ToString(type) << "'." << endl;
    return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
           nl->OneElemList(nl->IntAtom(j)), returnType);
  };

  return listutils::typeError("Attribute type point, line or region"
                               "not found!");
}



ListExpr mapColTM(ListExpr args) {

  if (!nl->HasLength(args,2)) {
     return listutils::typeError("Wrong number of arguments!");
  }

  // check for column spatial object and return corresponding spatial type
  if (ColPoint::checkType(nl->First(args)))
    return listutils::basicSymbol<Point>();

  if (ColLine::checkType(nl->First(args)))
    return listutils::basicSymbol<Line>();

  if (ColRegion::checkType(nl->First(args)))
    return listutils::basicSymbol<Region>();

  return listutils::typeError("First arg is no valid column spatial type!");
}



ListExpr countTM(ListExpr args) {

  if (!nl->HasLength(args,1)) {
     return listutils::typeError("Wrong number of arguments!");
  }

  // check for column spatial object and return corresponding spatial type
  if (ColPoint::checkType(nl->First(args)) ||
      ColLine::checkType(nl->First(args)) ||
      ColRegion::checkType(nl->First(args)))
    return listutils::basicSymbol<CcInt>();

  return listutils::typeError("First arg is no valid column spatial type!");
}



ListExpr plusTM(ListExpr args) {

  if (!nl->HasLength(args,2)) {
     return listutils::typeError("Expected two column spatial types!");
  }

  // check for column spatial object and return corresponding spatial type
  if (ColPoint::checkType(nl->First(args)) &&
      ColPoint::checkType(nl->Second(args)))
    return listutils::basicSymbol<ColPoint>();

  if (ColLine::checkType(nl->First(args)) &&
       ColLine::checkType(nl->Second(args)))
    return listutils::basicSymbol<ColLine>();

  if (ColRegion::checkType(nl->First(args)) &&
       ColRegion::checkType(nl->Second(args)))
    return listutils::basicSymbol<ColRegion>();

  return listutils::typeError("Expected two column spatial types!");
}



ListExpr showarrayTM(ListExpr args) {

  if (!nl->HasLength(args,1)) {
     return listutils::typeError("Expected a column spatial type!");
  }

  // check for column spatial object and return corresponding spatial type
  if ((ColPoint::checkType(nl->First(args))) ||
      (ColLine::checkType(nl->First(args))) ||
      (ColRegion::checkType(nl->First(args))))
    return listutils::basicSymbol<CcBool>();

  return listutils::typeError("Expected a column spatial type!");
}



/*

5.2 Value Mapping

checks for each point of the apoint array if it's inside each region
of a aregion array. If only one point or one region should be checked,
then the arrays have to contain only one element.

*/
int pointsInsideVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {

  long cStart, cStop, tStart, tStop;
  benchmark(cStart, tStart);

  ColPoint* cPoint = (ColPoint*) args[0].addr;
  ColRegion* cRegion = (ColRegion*) args[1].addr;

  cout << "check " << cPoint->getCount() << " points "
       << "and " << cRegion->getCount() << " regions..." << endl;

  result = qp->ResultStorage(s);
  LongInts* id = cRegion->pointsInside(cPoint);
  result.addr = id;

  benchmark(cStop, tStop);
  cout << "Benchmark: " << (long)(cStop - cStart) << " cycles / "
       << (long) tStop - tStart << " nanoseconds." << endl;
    fstream f;
    f.open("benchmark.dat", std::fstream::out|std::fstream::app);
    f  << "Benchmark: " << (long)(cStop - cStart) << " cycles / "
       << (long) tStop - tStart << " nanoseconds." << endl;
    f.close();

  return 0;
}



int linesInsideVM (Word* args, Word& result, int message,
                  Word& local, Supplier s) {

  long cStart, cStop, tStart, tStop;
  benchmark(cStart, tStart);

  ColLine* cLine = (ColLine*) args[0].addr;
  ColRegion* cRegion = (ColRegion*) args[1].addr;

  cout << "check " << cLine->getCount() << " lines "
       << "and " << cRegion->getCount() << " regions..." << endl;

  result = qp->ResultStorage(s);
  LongInts* id = cRegion->linesInside(cLine);
  result.addr = id;

  benchmark(cStop, tStop);
  cout << "Benchmark: " << (long)(cStop - cStart) << " cycles / "
       << (long) tStop - tStart << " nanoseconds." << endl;

  return 0;
}



int regionsInsideVM (Word* args, Word& result, int message,
                    Word& local, Supplier s) {
  result = qp->ResultStorage(s);
  // initialized and empty result array
  LongInts* id = new LongInts();
  result.addr = id;
  return 0;
}


/*
~contain~ functions

*/
int containsPointsVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {
  long cStart, cStop, tStart, tStop;
  benchmark(cStart, tStart);

  ColRegion* cRegion = (ColRegion*) args[0].addr;
  ColPoint* cPoint = (ColPoint*) args[1].addr;

  cout << "check " << cPoint->getCount() << " points "
       << "and " << cRegion->getCount() << " regions..." << endl;

  result = qp->ResultStorage(s);
  LongInts* id = cRegion->containsPoints(cPoint);
  result.addr = id;

  benchmark(cStop, tStop);
  cout << "Benchmark: " << (long)(cStop - cStart) << " cycles / "
       << (long) tStop - tStart << " nanoseconds." << endl;

  return 0;
}



/*
The operator ~mapPointVM~ expects a stream of tuples as input parameter.
It extracts the ~point~ attribute of each tuple and hands it over to the
append function of the column spatial ~apoint~ object.

*/
int mapPointVM (Word* args, Word& result, int message,
                Word& local, Supplier s) {
  result = qp->ResultStorage(s);         // use result storage for the result
  result.addr = new ColPoint(true);

  ColPoint* cPoint = static_cast<ColPoint*> (result.addr);

  int index = ((CcInt*) args[2].addr)->GetValue();  // append attributes index

  Stream<Tuple> stream(args[0]);         // get the tuples
  stream.open();                         // open the stream

  Tuple* tuple;                          // actual tuple element
  Point* point;                          // actual line attribute

  while( (tuple = stream.request()) ) {  // if exists, get next tuple

    // extract point from tuple
    point = (Point*) tuple->GetAttribute(index - 1);

    if (!cPoint->append(point)) {        // append line to aline
      cout << "Error in mapping stream(point) to apoint!" << endl;
      return 0;
    }

    tuple->DeleteIfAllowed();            // remove tuple from stream
   }

  stream.close();                        // close the stream
  cPoint->finalize();

  return 0;
}



/*
The operator ~mapLineVM~ expects a stream of tuples as input parameter.
It extracts the ~line~ attribute of each tuple and hands it over to the
append function of the column spatial ~aline~ object.

*/
int mapLineVM (Word* args, Word& result, int message,
               Word& local, Supplier s) {
  result = qp->ResultStorage(s);         // use result storage for the result
  result.addr = new ColLine(true);

  ColLine* cLine = static_cast<ColLine*> (result.addr);


  int index = ((CcInt*) args[2].addr)->GetValue();  // append attributes index
  Stream<Tuple> stream(args[0]);         // get the tuples
  stream.open();                         // open the stream

  Tuple* tuple;                          // actual tuple element
  Line* line;                            // actual line attribute

  while( (tuple = stream.request()) ) {  // if exists, get next tuple

    // extract line from tuple
    line = (Line*) tuple->GetAttribute(index - 1);

    if (!cLine->append(line)) {          // append line to aline
      cout << "Error in mapping stream(line) to aline!" << endl;
      return 0;
    }

    tuple->DeleteIfAllowed();            // remove tuple from stream
   }

  stream.close();                        // close the stream
  cLine->finalize();

  return 0;
}



/*
The operator ~mapRegionVM~ expects a stream of tuples as input parameter.
It extracts the ~region~ attribute of each tuple and hands it over to the
append function of the column spatial ~aregion~ object.

*/
int mapRegionVM (Word* args, Word& result, int message,
                 Word& local, Supplier s) {
  result = qp->ResultStorage(s);         // use result storage for the result
  result.addr = new ColRegion(true);     // result set to cRegion object

  ColRegion* cRegion = static_cast<ColRegion*> (result.addr);

  int index = ((CcInt*) args[2].addr)->GetValue();  // append attributes index

  Stream<Tuple> stream(args[0]);         // get the tuples
  stream.open();                         // open the stream

  Tuple* tuple;                          // actual tuple element
  Region* region;                        // actual region attribute

  while( (tuple = stream.request()) ) {  // if exists, get next tuple
    // extract region from tuple
    region = (Region*) tuple->GetAttribute(index - 1);

    if (!cRegion->append(region)) {      // append region to aregion
      cout << "Error in mapping stream(region) to aregion!" << endl;
      return 0;
    }

    tuple->DeleteIfAllowed();            // remove tuple from stream
   }

  stream.close();                        // close the stream
  cRegion->finalize();

  return 0;
}



/*
The operator ~mapColPointVM~ expects an apoint type of the column spatial
algebra and an index. Then it extracts the entry specified by index and
converts it into the spatial object ~point~.

*/
int mapColPointVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {
  result = qp->ResultStorage(s);
  ColPoint* cPoint = static_cast<ColPoint*> (args[0].addr);
  int index = ((CcInt*) args[1].addr)->GetValue();  // append attributes index

  if ((index < 0) || (index >= cPoint->getCount())) {
    cout << "Error: apoint index " << index << " out of bounds!" << endl;
    return 0;
  }
  cout << "map apoint[" << index << "] to point.\n";

  result.addr = new Point(true, cPoint->getX(index), cPoint->getY(index));
  return 0;
}


/*
maps the column spatial type ~aline~ to a single spatial type ~line~.
needs an index.

*/
int mapColLineVM (Word* args, Word& result, int message,
                  Word& local, Supplier s) {
  result = qp->ResultStorage(s);
  ColLine* cLine = static_cast<ColLine*> (args[0].addr);
  int index = ((CcInt*) args[1].addr)->GetValue();  // append attributes index

  if ((index < 0) || (index >= cLine->getCount())) {
    cout << "Error: aline index " << index << " out of bounds!" << endl;
    return false;
  }
  cout << "map aline[" << index << "] to line\n";

  long segCount = cLine->getSegments(index);
  result.addr = new Line(segCount);
  Line* line = static_cast<Line*> (result.addr);

  if (!cLine->createLine(line, index)) {
    cout << "Error mapping aline to line!" << endl;
    return 0;
  }

  return 0;
}


/*
maps column spatial type ~aregion~ to a single spatial type ~region~.
needs an index.

*/
int mapColRegionVM (Word* args, Word& result, int message,
                 Word& local, Supplier s) {
  result = qp->ResultStorage(s);
  ColRegion* cRegion = static_cast<ColRegion*> (args[0].addr);
  int index = ((CcInt*) args[1].addr)->GetValue();  // append attributes index

  if ((index < 0) || (index >= cRegion->getCount())) {
    cout << "Error: aregion index " << index << " out of bounds!" << endl;
    return 0;
  }
  cout << "map aregion[" << index << "] to region.\n";

  long segCount = cRegion->getCountPoints(index) * 2;
  result.addr = new Region(segCount);
  Region* region = static_cast<Region*> (result.addr);

  // Region* region = static_cast<Region*> (result.addr);
  // region = new Region(cRegion->getCountPoints(index)* 2);

  if (!cRegion->createRegion(region, index)) {
    cout << "Error mapping aregion to region!" << endl;
    //region->SetDefined(false);
    return 0;
  }

  //region->Print(cout);
  // cout << region->getMaxX() << endl;        returns 0 !?!?!

  //region->SetDefined(true);
  //result.addr = region;
  return 0;
}



/*
value mapping of the count functions

*/
int countPointVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {

  long cStart, cStop, tStart, tStop;
  benchmark(cStart, tStart);

  result = qp->ResultStorage(s);
  ColPoint* cPoint = static_cast<ColPoint*> (args[0].addr);
  CcInt* res = (CcInt*) result.addr;
  res->Set(true, cPoint->getCount());

  benchmark(cStop, tStop);
  cout << "Benchmark: " << (long)(cStop - cStart) << " cycles / "
       << (long) tStop - tStart << " nanoseconds." << endl;

  return 0;
}

int countLineVM (Word* args, Word& result, int message,
                 Word& local, Supplier s) {

  long cStart, cStop, tStart, tStop;
  benchmark(cStart, tStart);

  result = qp->ResultStorage(s);
  ColLine* cLine = static_cast<ColLine*> (args[0].addr);
  CcInt* res = (CcInt*) result.addr;
  res->Set(true, cLine->getCount());

  benchmark(cStop, tStop);
  cout << "Benchmark: " << (long)(cStop - cStart) << " cycles / "
       << (long) tStop - tStart << " nanoseconds." << endl;

  return 0;
}

int countRegionVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {

  long cStart, cStop, tStart, tStop;
  benchmark(cStart, tStart);

  result = qp->ResultStorage(s);
  ColRegion* cRegion = static_cast<ColRegion*> (args[0].addr);
  CcInt* res = (CcInt*) result.addr;
  res->Set(true, cRegion->getCount());

  benchmark(cStop, tStop);
  cout << "Benchmark: " << (long)(cStop - cStart) << " cycles / "
       << (long) tStop - tStart << " nanoseconds." << endl;

  return 0;
}

/*
value mappings of the plus functions

*/
int plusPointVM (Word* args, Word& result, int message,
                 Word& local, Supplier s) {

  long cStart, cStop, tStart, tStop;
  benchmark(cStart, tStart);

  result = qp->ResultStorage(s);
  result.addr = new ColPoint(true);
  ColPoint* cPoint = static_cast<ColPoint*> (result.addr);

  ColPoint* cPoint1 = (ColPoint*) (args[0].addr);
  ColPoint* cPoint2 = (ColPoint*) (args[1].addr);

  if (!cPoint->merge(cPoint1, cPoint2)) {
    cout << "Error in merging two apoint types!" << endl;
    return 0;
  }

  benchmark(cStop, tStop);
  cout << "Benchmark: " << (long)(cStop - cStart) << " cycles / "
       << (long) tStop - tStart << " nanoseconds." << endl;

  return 0;
}



int plusLineVM (Word* args, Word& result, int message,
                Word& local, Supplier s) {

  long cStart, cStop, tStart, tStop;
  benchmark(cStart, tStart);

  result = qp->ResultStorage(s);
  result.addr = new ColLine(true);
  ColLine* cLine = static_cast<ColLine*> (result.addr);

  ColLine* cLine1 = (ColLine*) (args[0].addr);
  ColLine* cLine2 = (ColLine*) (args[1].addr);

  if (!cLine->merge(cLine1, cLine2)) {
    cout << "Error in merging two aline types!" << endl;
    return 0;
  }

  benchmark(cStop, tStop);

  cout << "Benchmark: " << (long)(cStop - cStart) << " cycles / "
       << (long) tStop - tStart << " nanoseconds." << endl;

  return 0;
}



int plusRegionVM (Word* args, Word& result, int message,
                  Word& local, Supplier s) {

  long cStart, cStop, tStart, tStop;
  benchmark(cStart, tStart);


  result = qp->ResultStorage(s);
  result.addr = new ColRegion(true);     // result set to cRegion object
  ColRegion* cRegion = static_cast<ColRegion*> (result.addr);

  ColRegion* cRegion1 = (ColRegion*) (args[0].addr);
  ColRegion* cRegion2 = (ColRegion*) (args[1].addr);

  if (!cRegion->merge(cRegion1, cRegion2)) {
    cout << "Error in merging two aregion types!" << endl;
    return 0;
  }

  benchmark(cStop, tStop);
  cout << "Benchmark: " << (long)(cStop - cStart) << " cycles / "
       << (long) tStop - tStart << " nanoseconds." << endl;

  return 0;
}



int showarrayPointVM (Word* args, Word& result, int message,
                       Word& local, Supplier s) {
  ColPoint* cPoint = (ColPoint*) (args[0].addr);

  cPoint->showArray("show internal array of apoint");

  result.addr = new CcBool();
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  res->Set(true, true);
  return 0;
}



int showarrayLineVM (Word* args, Word& result, int message,
                       Word& local, Supplier s) {

  ColLine* cLine = (ColLine*) (args[0].addr);

  cLine->showArrays("show internal arrays of aregion");

  result.addr = new CcBool();
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  res->Set(true, true);
  return 0;
}



int showarrayRegionVM (Word* args, Word& result, int message,
                       Word& local, Supplier s) {

  ColRegion* cRegion = (ColRegion*) (args[0].addr);

  cRegion->showArrays("show internal arrays of aregion", true);

  result.addr = new CcBool();
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  res->Set(true, true);
  return 0;
}



/*
5.3 Selection Function

*/
int insideSelect(ListExpr args) {
  if (ColPoint::checkType(nl->First(args))) { return 0; }
  if (ColLine::checkType(nl->First(args))) { return 1; }
  return 2; // ColRegion
}



int containsSelect(ListExpr args) {
  if (ColPoint::checkType(nl->Second(args))) { return 0; }
  if (ColLine::checkType(nl->Second(args))) { return 1; }
  return 2; // ColRegion
}



/*
If the first argument is a tuple type then there are three different mappings
of spatial types within a relation to column spatial types.

*/
int mapSelect(ListExpr args) {

  ListExpr arg1 = nl->First( args );
  // check for tuple stream
  if (Stream<Tuple>::checkType(arg1)) {
    // extract the attribute list
    ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
    string name = nl->SymbolValue(nl->Second(args));
    ListExpr type;
    listutils::findAttribute(attrList, name, type);
    if (Point::checkType(type)) return 0;    // mapPointVM
    if (Line::checkType(type)) return 1;     // mapLineVM
    if (Region::checkType(type)) return 2;   // mapRegionVM
  }

  return 0;  // if no match then return dummy, type mapping will be done later
}



/*
If the first argument is a colum spatial type then there are three mappings
of column spatial types to spatial types.

*/
int mapColSelect(ListExpr args) {

  // first check for column spatial objects
  ListExpr arg1 = nl->First( args );
  if (ColPoint::checkType(arg1)) return 0;   // mapColPointVM
  if (ColLine::checkType(arg1)) return 1;    // mapColLineVM
  if (ColRegion::checkType(arg1)) return 2;  // mapColRegionVM

  return 0;  // if no match then return dummy, type mapping will be done later
}



int countSelect(ListExpr args) {

  // first check for column spatial objects
  ListExpr arg1 = nl->First( args );
  if (ColPoint::checkType(arg1)) return 0;   // countPointVM
  if (ColLine::checkType(arg1)) return 1;    // countLineVM
  if (ColRegion::checkType(arg1)) return 2;  // countRegionVM

  return 0;  // if no match then return dummy, type mapping will be done later
}



int plusSelect(ListExpr args) {
  // first check for column spatial objects
  ListExpr arg1 = nl->First( args );
  if (ColPoint::checkType(arg1)) return 0;   // plusColPointVM
  if (ColLine::checkType(arg1)) return 1;    // plusColLineVM
  if (ColRegion::checkType(arg1)) return 2;  // plusColRegionVM

  return 0;  // if no match then return dummy, type mapping will be done later

}


int showarraySelect(ListExpr args) {
  // first check for column spatial objects
  ListExpr arg1 = nl->First( args );
  if (ColPoint::checkType(arg1)) return 0;   // showarrayColPointVM
  if (ColLine::checkType(arg1)) return 1;    // showarrayColLineVM
  if (ColRegion::checkType(arg1)) return 2;  // showarrayColRegionVM

  return 0;  // if no match then return dummy, type mapping will be done later

}

}  // namespace col
