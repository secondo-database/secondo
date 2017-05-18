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

[1] Header of a column oriented spatial algebra

[2] May 2017 by Sascha Radke for bachelor thesis

*/

// TODO: remove relative paths. they are only needed by qt-creator.
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
//#include "../CRel/LongInts.h"                 // type for id result list
//#include "../CRel/TypeConstructors/LongIntsTI.h"
//#include "../CRel/AttrArray.h"              // column oriented relations

using std::vector;
// using namespace CRelAlgebra;


extern NestedList *nl;
extern QueryProcessor *qp;

namespace col {
  ColPoint::ColPoint() {}  // standard constructor doing nothing
  ColPoint::ColPoint(sPoint* newArray, long newCount) {  // main constructor
    aPoint = newArray;
    count = newCount;
  }
  ColPoint::ColPoint(int min) {  // constructor with minimum initialized array
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
          cmsg.inFunError("not enough memory (??? allocBytes needed)!");
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
    Word answer(static_cast<void*>(0));
    answer.addr = new ColPoint(inArray, inCount);  // create new point object
    // the following two lines are only for debugging mode
    // ColPoint* cPoint = static_cast<ColPoint*>(answer.addr);
    // cPoint->showArray("ColPoint after In-function");
    correct = true;
    return answer;  // return its adress
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
    sPoint* aPoint = NULL;  // array which contains the input point values
    long count;           // amount of points
    double x, y;          // actual read coordinates
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
    return ok;
  }

  bool ColPoint::Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value) {
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
      cmsg.inFunError("not enough memory (??? allocBytes needed)!");
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
3.3 Class ColLine for column-oriented representation of lines

*/
  ColLine::ColLine() {}  // standard constructor doing nothing

  ColLine::ColLine(sLine *newLine, sSegment *newSegment,
                   long newCountLine, long newCountSegment) {
    aLine = newLine;
    aSegment = newSegment;
    countLine = newCountLine;
    countSegment = newCountSegment;
  }

  ColLine::ColLine(int min) {  // constructor with minimum initialized arrays
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
    for(int i = 0; i < lCopy->Size(); i++) {
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
    // next line is only for debugging cases - should be commented out
    // showArrays("Nach append aller Linien");
    cout << countLine * sizeof(sLine) + countSegment * sizeof(sSegment)
         << " bytes used.\n";
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

    // the following two lines are only for debugging mode
    // ColLine* cLine = static_cast<ColLine*>(answer.addr);
    // cLine->showArrays("ColLine after In-function");

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

    return ok;
  }



  // Saves an array of lines into a SmiRecord.
  bool ColLine::Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value) {
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



  Word ColLine::Clone(const ListExpr typeInfo, const Word& w) {
    ColLine* cLine = static_cast<ColLine*>(w.addr);
    cLine->showArrays("before Line Clone...");
    long tmpCL = cLine->countLine;
    sLine* tmpLine = NULL;
    tmpLine = static_cast<sLine*>(realloc(tmpLine, sizeof(sLine) * tmpCL));
    if (tmpLine == NULL) {    // exit on memory overflow
      cmsg.inFunError("not enough memory (??? allocBytes needed)!");
      return (void*)0;
    }

    long tmpCS = cLine->countSegment;
    sSegment* tmpSeg = NULL;
    tmpSeg = static_cast<sSegment*>(realloc(tmpSeg, sizeof(sSegment) * tmpCS));
    if (tmpSeg == NULL) {    // exit on memory overflow
      cmsg.inFunError("not enough memory (??? allocBytes needed)!");
      return (void*)0;
    }

    cout << "sizeof(tmpline) = " << sizeof(sLine) * tmpCL << endl
         << "sizeof(tmpSeg) = " << sizeof(sSegment) * tmpCS << endl;

    for (long cl = 0; cl < tmpCL; cl++) {
      tmpLine[cl] = cLine->aLine[cl];
    cout << tmpLine[cl].index << endl;

    }

    for (long cs = 0; cs < tmpCS; cs++) {

      tmpSeg[cs] = cLine->aSegment[cs];
    cout << tmpSeg[cs].x1 << endl
   << tmpSeg[cs].y1 << endl
      << tmpSeg[cs].x2 << endl
         << tmpSeg[cs].y2 << endl;






    }

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
Class ColRegion for column-oriented representation of Regions

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
  ColRegion::ColRegion(int min) {
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
  ColRegion::~ColRegion() {   // destructor - free allocated menory
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

  long ColRegion::getCount() {
    return countRegion - 1;
  }

  // returns the number of points of a given region
  long ColRegion::getCountPoints(const long index) {
    return aRegion[index + 1].indexPoint - aRegion[index].indexPoint - 1;
  }

  // extracts the region with given index to a region object of the
  // spatial algebra.
  bool ColRegion::createRegion(Region* region, const long index) {
    vector<vector<Point> > cycles;  // initialize all cycles
    long firstCyc = aRegion[index].indexCycle;
    long lastCyc  = aRegion[index + 1].indexCycle;
    // scan all cycles of the region
    for (long indexCyc = firstCyc; indexCyc < lastCyc; indexCyc++) {
      vector<Point> cycle;            // initialize the actual cycle
      long firstPnt = abs(aCycle[indexCyc].index);
      long lastPnt  = abs(aCycle[indexCyc + 1].index);
      // append all points of the actual cycle to the vector
      for (long indexPnt = firstPnt; indexPnt < lastPnt; indexPnt++) {
        Point p(true, aPoint[indexPnt].x, aPoint[indexPnt].y);
        cycle.push_back(p);
      }
      // append first point to close the cycle
      Point p(true, aPoint[firstPnt].x, aPoint[firstPnt].y);
      cycle.push_back(p);
      if (aCycle[indexCyc].index < 0) {  // if the actual cycle is a hole
        if (getDir(cycle))               // and clockwise
          reverseCycle(cycle);           // then set cycle to counterclockwise
      }
      cycles.push_back(cycle); //  append the complete cycle to all cycles
    }
    // finally build the region from all cycles
    region = buildRegion(cycles);
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
    // adjust mbr coordinates if neccessary
    if (x < aRegion[countRegion].mbrX1) aRegion[countRegion].mbrX1 = x;
    if (x > aRegion[countRegion].mbrX2) aRegion[countRegion].mbrX2 = x;
    if (y < aRegion[countRegion].mbrY1) aRegion[countRegion].mbrY1 = y;
    if (y > aRegion[countRegion].mbrY2) aRegion[countRegion].mbrY2 = y;
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
    // mbr set to extremes, so they will be adapted in any case
    aRegion[countRegion].mbrX1 = 999999;
    aRegion[countRegion].mbrY1 = 999999;
    aRegion[countRegion].mbrX2 = -999999;
    aRegion[countRegion].mbrY2 = -999999;
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
    aRegion[countRegion].mbrX1 = 0;
    aRegion[countRegion].mbrY1 = 0;
    aRegion[countRegion].mbrX2 = 0;
    aRegion[countRegion].mbrY2 = 0;
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
    // next lines is only for debugging cases - should be commented out
    // showArrays("arrays after appending regions (no points)", false);
    // cout << countRegion * sizeof(sRegion) + countCycle * sizeof(sCycle)
    //       + countPoint * sizeof(sPoint) << " bytes used.\n";
  }

  // test output of the arrays aRegion, aCycle and aPoint and there parameters
  void ColRegion::showArrays(string title, bool showPoints) {
    // example output of a nested list: cout << nl->ToString(list) << endl;
    cout << "\n--------------------------------------------\n" << title << "\n";
    // output of the array aRegion
    cout << "aRegion (" << countRegion << "):\n";
    cout << "Bytes allocated: " << countRegion * sizeof(aRegion[0]) << "\n";
    cout << "index\tcycle\tpoint\tmbr-l\tmbr-b\tmbr-r\tmbr-t\n";
    for (long cr = 0; cr < countRegion; cr++) {
      cout << cr << ":\t" << aRegion[cr].indexCycle << "\t"
           << aRegion[cr].indexPoint << "\t"
           << aRegion[cr].mbrX1 << "\t"
           << aRegion[cr].mbrY1 << "\t"
           << aRegion[cr].mbrX2 << "\t"
           << aRegion[cr].mbrY2 << "\n";
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

  inline bool ColRegion::intersects() {
    return true;
  }

  void ColRegion::pointsInside(ColPoint* cPoint) {
    const long cp1 = cPoint->getCount();  // index of point
    const long cr = countRegion;     // index of region
    long cc = 0;                   // index of region cycle
    long cp = 0;                   // index of region point
    double idPx = 0;
    double idPy = 0;
    long isCount = 0;
    // result array of matching indices
    //LongInts* id = new LongInts();
    for (int64_t idP = 0; idP < cp1; idP++) {
      idPx = cPoint->getX(idP);
      idPy = cPoint->getY(idP);
      for (long idReg = 0; idReg < cr; idReg++) {
        // TODO: create a line to a point outside the region
        // check if actual point is inside the bounding box of actual region
        if ((idPx < aRegion[idReg].mbrX1) || (idPx > aRegion[idReg].mbrX2) ||
            (idPy < aRegion[idReg].mbrY1) || (idPy > aRegion[idReg].mbrY2))
          break;  // if not inside bbox then continue with next region
        cc = aRegion[idReg + 1].indexCycle;
        isCount = 0;
        for (long idCyc = aRegion[idReg].indexCycle; idCyc < cc; idCyc++) {
          cp = aCycle[idCyc + 1].index;
          for (long idPnt = aCycle[idCyc].index; idPnt < cp; idPnt++) {
            // TODO: check intersection between actual point and region segment
            // TODO: change type of indices to int64_t
            if (intersects()) isCount++;
          }
        }
        // if bit 1 of isCount is odd then point is inside region -> append
        //if (isCount & 1) id->Append(idP);
      }
    }
    //return id;
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
      inRegion[cr].mbrX1 = 999999;
      inRegion[cr].mbrY1 = 999999;
      inRegion[cr].mbrX2 = -999999;
      inRegion[cr].mbrY2 = -999999;
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
            // calculate the mbr
            if (x < inRegion[cr].mbrX1) inRegion[cr].mbrX1 = x;
            if (x > inRegion[cr].mbrX2) inRegion[cr].mbrX2 = x;
            if (y < inRegion[cr].mbrY1) inRegion[cr].mbrY1 = y;
            if (y > inRegion[cr].mbrY2) inRegion[cr].mbrY2 = y;

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
    // add terminator entries to arrays. they are needed for the Out-function.
    inPoint[cp].x = 0;
    inPoint[cp].y = 0;
    inCycle[cc].index = cp;
    inRegion[cr].indexCycle = cc;
    inRegion[cr].indexPoint = cp;
    inRegion[cr].mbrX1 = 0;
    inRegion[cr].mbrY1 = 0;
    inRegion[cr].mbrX2 = 0;
    inRegion[cr].mbrY2 = 0;
    // truncate oversized memory to allocate only the real used memory
    inRegion = static_cast<sRegion*>(realloc(inRegion, ++cr * sizeof(sRegion)));
    inCycle = static_cast<sCycle*>(realloc(inCycle, ++cc * sizeof(sCycle)));
    inPoint = static_cast<sPoint*>(realloc(inPoint, ++cp * sizeof(sPoint)));
    cout << cr * sizeof(sRegion) + cc * sizeof(sCycle) + cp * sizeof(sPoint)
         << " bytes used\n";
    Word answer(static_cast<void*>(0));
    // create new region object with the just read in arrays
    answer.addr = new ColRegion(inRegion, inCycle, inPoint, cr, cc, cp);
    // the following two lines are only for debugging mode
    // ColRegion* cRegion = static_cast<ColRegion*>(answer.addr);
    // cRegion->showArrays("ColRegion after In-function", true);
    correct = true;
    return answer;  // return the object
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
    // initialize arrays
    sRegion* region = NULL;
    sCycle* cycle = NULL;
    sPoint* point = NULL;
    // initialize counters
    long cr;
    long cc;
    long cp;
    long valueL;
    double valueD;          // actual read coordinates
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
      cmsg.inFunError("not enough memory (??? Bytes needed)!");
      return false;
    }
    cycle = static_cast<sCycle*>(malloc(cc * sizeof(sCycle)));
    if (cycle == NULL) {    // exit on memory overflow
      cmsg.inFunError("not enough memory (??? Bytes needed)!");
      return false;
    }
    point = static_cast<sPoint*>(malloc(cp * sizeof(sPoint)));
    if (point == NULL) {    // exit on memory overflow
      cmsg.inFunError("not enough memory (??? Bytes needed)!");
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
      region[i].mbrX1 = valueD;
      offset += sizeD;
      ok = ok && (valueRecord.Read(&valueD, sizeD, offset) == sizeD);
      region[i].mbrY1 = valueD;
      offset += sizeD;
      ok = ok && (valueRecord.Read(&valueD, sizeD, offset) == sizeD);
      region[i].mbrX2 = valueD;
      offset += sizeD;
      ok = ok && (valueRecord.Read(&valueD, sizeD, offset) == sizeD);
      region[i].mbrY2 = valueD;
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
    cout << "ColRegion object generated with read data from disk!\n";
    return ok;  // ok can be true or false
  }

  bool ColRegion::Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value) {
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
      valueD = cRegion->aRegion[i].mbrX1;
      ok = ok && valueRecord.Write(&valueD, sizeD, offset);
      offset += sizeD;
      valueD = cRegion->aRegion[i].mbrY1;
      ok = ok && valueRecord.Write(&valueD, sizeD, offset);
      offset += sizeD;
      valueD = cRegion->aRegion[i].mbrX2;
      ok = ok && valueRecord.Write(&valueD, sizeD, offset);
      offset += sizeD;
      valueD = cRegion->aRegion[i].mbrY2;
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
    cout << "Data of ColRegion object written to disk!\n";
    return ok;
  }

  void ColRegion::Close(const ListExpr typeInfo, Word& w) {
    delete static_cast<ColRegion*>(w.addr);
    w.addr = 0;
  }

  Word ColRegion::Clone(const ListExpr typeInfo, const Word& w) {
    ColRegion* cRegion = static_cast<ColRegion*>(w.addr);
    Word res;
    res.addr = new ColRegion(*cRegion);
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

// Type Mapping

ListExpr insideTM(ListExpr args) {
    string err = "{apoint | aline} x aregion expected\n";
    if (!nl->HasLength(args, 2)) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if ((ColPoint::checkType(nl->First(args)) ||
         ColLine::checkType(nl->First(args))) &&
         ColRegion::checkType(nl->Second(args))) {
      // return LongIntsTI(false).GetTypeExpr();  // list of long ints for id
      return listutils::basicSymbol<ColPoint>();
    }
    return listutils::typeError(err);
}

ListExpr mapTM(ListExpr args) {
  if (!nl->HasLength(args,2)) {
     return listutils::typeError("Wrong number of arguments!");
  }
  // check for tuple stream
  if (Stream<Tuple>::checkType(nl->First(args))) {
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
    } else {
      return listutils::typeError("Attribute type point, line or region"
                                  "not found!");
    }
  }
  // check for column spatial object and return corresponding spatial type
  if (ColPoint::checkType(nl->First(args)))
    return listutils::basicSymbol<Point>();
  if (ColLine::checkType(nl->First(args)))
    return listutils::basicSymbol<Line>();
  if (ColRegion::checkType(nl->First(args)))
    return listutils::basicSymbol<Region>();
  // if neither a tuple stream nor a column spatial object is detected as
  // input then throw an error!
  return listutils::typeError("First arg is no tuple stream and no column"
                              " spatial type!");
}

/*
checks for each point of the apoint array if it's inside each region
of a aregion array. If only one point or one region should be checked,
then the arrays have to contain only one element. But the strength of
this function lies within the bulk processing. The more elements are
processed the more efficient it will be.

*/

int insidePointVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {
  ColPoint* cPoint = (ColPoint*) args[0].addr;
  ColRegion* cRegion = (ColRegion*) args[1].addr;
  result = qp->ResultStorage(s);
  //LongInts id = cRegion->pointsInside(cPoint);
  cRegion->pointsInside(cPoint);
  return 0;

}

int insideLineVM (Word* args, Word& result, int message,
                  Word& local, Supplier s) {
  return 0;
}

int insideRegionVM (Word* args, Word& result, int message,
                    Word& local, Supplier s) {
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
  result.addr = new ColPoint(1);
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
      return false;
    }
    tuple->DeleteIfAllowed();            // remove tuple from stream
   }
  stream.close();                        // close the stream
  cPoint->finalize();
  return true;
}

/*
The operator ~mapLineVM~ expects a stream of tuples as input parameter.
It extracts the ~line~ attribute of each tuple and hands it over to the
append function of the column spatial ~aline~ object.

*/
int mapLineVM (Word* args, Word& result, int message,
               Word& local, Supplier s) {
  result = qp->ResultStorage(s);         // use result storage for the result
  result.addr = new ColLine(1);
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
      return false;
    }
    tuple->DeleteIfAllowed();            // remove tuple from stream
   }
  stream.close();                        // close the stream
  cLine->finalize();
  return true;
}

/*
The operator ~mapRegionVM~ expects a stream of tuples as input parameter.
It extracts the ~region~ attribute of each tuple and hands it over to the
append function of the column spatial ~aregion~ object.

*/
int mapRegionVM (Word* args, Word& result, int message,
                 Word& local, Supplier s) {
  result = qp->ResultStorage(s);         // use result storage for the result
  result.addr = new ColRegion(1);        // result set to cRegion object
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
      return false;
    }
    tuple->DeleteIfAllowed();            // remove tuple from stream
   }
  stream.close();                        // close the stream
  cRegion->finalize();
  return true;
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
    return false;
  }
  result.addr = new Point(true, cPoint->getX(index), cPoint->getY(index));
  return true;
}

// maps the column spatial type ~aline~ to a single spatial object ~line~.
int mapColLineVM (Word* args, Word& result, int message,
                  Word& local, Supplier s) {
  cout << "map ColLine to Line\n";
  result = qp->ResultStorage(s);
  ColLine* cLine = static_cast<ColLine*> (args[0].addr);
  int index = ((CcInt*) args[1].addr)->GetValue();  // append attributes index
  if ((index < 0) || (index >= cLine->getCount())) {
    cout << "Error: aline index " << index << " out of bounds!" << endl;
    return false;
  }
  long segCount = cLine->getSegments(index);
  result.addr = new Line(segCount);
  Line* line = static_cast<Line*> (result.addr);
  if (!cLine->createLine(line, index)) {
    cout << "Error mapping aline to line!" << endl;
    return false;
  }
  return true;
}

// maps column spatial type ~aregion~ to a single spatial object ~region~.
// needs an index.
int mapColRegionVM (Word* args, Word& result, int message,
                 Word& local, Supplier s) {
  cout << "map ColRegion to Region\n";
  result = qp->ResultStorage(s);
  ColRegion* cRegion = static_cast<ColRegion*> (args[0].addr);
  int index = ((CcInt*) args[1].addr)->GetValue();  // append attributes index
  cRegion->showArrays("before mapping", true);
  cout << "countRegion = " << cRegion->getCount() << endl;
  cout << "countPoints = " << cRegion->getCountPoints(index) << endl;
  if ((index < 0) || (index >= cRegion->getCount())) {
    cout << "Error: aregion index " << index << " out of bounds!" << endl;
    return 0;
  }
  // create a new region object with space for all halfs segments
  result.addr = new Region(cRegion->getCountPoints(index) * 2);
  Region* region = static_cast<Region*> (result.addr);
  if (!cRegion->createRegion(region, index)) {
    cout << "Error mapping aregion to region!" << endl;
    return false;
  }
  return true;
}

/*
Selection Function


*/
int insideSelect(ListExpr args) {
  if (ColPoint::checkType(nl->First(args))) { return 0; }
  if (ColLine::checkType(nl->First(args))) { return 1; }
  return 2; // ColRegion
}

/*
There are six different mappings: three for spatial objects within a relation
and three for column spatial objects.
In the first case a tuple stream is generated and the spatial attribute
is processed. e. g. 'query City feed mp[Area]' extracts the attribute ~Area~
of all tuples within the relation ~City~ and convert them to a column spatial
object.
In the second case a column spatial object is searched for a given index
and one single spatial object is generated. e.g. 'query colRiver mp[3]'
returns the third entry within the object ~colRiver~ and converts it into
a spatial object.

*/
int mapSelect(ListExpr args) {
  // first check for column spatial objects
  ListExpr arg1 = nl->First( args );
  if (ColPoint::checkType(arg1)) return 3;   // mapColLineVM
  if (ColLine::checkType(arg1)) return 4;    // mapColLineVM
  if (ColRegion::checkType(arg1)) return 5;  // mapColRegionVM
  // second check for tuple stream
  if (Stream<Tuple>::checkType(arg1)) {
    // extract the attribute list
    ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
    string name = nl->SymbolValue(nl->Second(args));
    ListExpr type;
    listutils::findAttribute(attrList, name, type);
    if (Point::checkType(type)) return 0;    // mapPointVM
    if (Line::checkType(type)) return 1;     // mapLineVM
    if (Region::checkType(type)) return 2;   // mapLineVM
  }
  return 0;  // if no match then return dummy, type mapping will be done later
}

}  // namespace col
