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

// TODO: remove relative paths
#include "ColumnSpatialAlgebra.h"             // header for this algebra
#include "../../include/Symbols.h"            // predefined strings
#include "../../include/ListUtils.h"          // useful nested list functions
#include "../../include/NestedList.h"         // required at many places
#include "../../include/QueryProcessor.h"     // needed for value mappings
#include "../../include/TypeConstructor.h"    // constructor for Secondo Types
#include "../Spatial/SpatialAlgebra.h"        // spatial types and operators
#include "../Relation-C++/RelationAlgebra.h"  // use of tuples
#include "../Stream/Stream.h"                 // wrapper for secondo streams
//#include "../CRel/AttrArray.h"              // column oriented relations

extern NestedList *nl;
extern QueryProcessor *qp;

using std::string;

namespace col {
  ColPoint::ColPoint() {}  // standard constructor doing nothing
  ColPoint::ColPoint(point* newArray, long newCount) {  // non-standard
    //constructor
    array = newArray;
    count = newCount;
  }
  ColPoint::~ColPoint() {  // destructor
    free(array);
  }
  // returns the corresponding basic type
  const string ColPoint::BasicType() { return "apoint"; }
  // compares the type of the given object with the class type
  const bool ColPoint::checkType(const ListExpr list) {
    return listutils::isSymbol(list, BasicType());
  }
  // returns the number of elements in the point array
  long ColPoint::getCount() {
    return count;
  }
  // returns the address of the point array for external access
  void* ColPoint::getArray() {
    return &array;
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

  Word ColPoint::In(const ListExpr typeInfo, ListExpr instance,
                 const int errorPos, ListExpr &errorInfo, bool &correct) {
    correct = false;
    point* inArray = NULL;  // array which contains the input point values
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
      if ((long)((inCount + 1) * sizeof(point)) >= allocBytes[step]) {
        // allocate more memory - in C++ a type casting is necessary unlike in C
        inArray = static_cast<point*>(realloc(inArray, allocBytes[++step]));
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
    inArray = static_cast<point*>(realloc(inArray, inCount * sizeof(point)));
    Word answer(static_cast<void*>(0));
    answer.addr = new ColPoint(inArray, inCount);  // create new point object
    correct = true;
    return answer;  // return its adress
}

  ListExpr ColPoint::Out(ListExpr typeInfo, Word value) {
    ColPoint* cPoint = static_cast<ColPoint*>(value.addr);
    if (cPoint->count == 0) {
      return listutils::emptyErrorInfo();
    }
    ListExpr res = nl->OneElemList(nl->TwoElemList(  // init first point
      nl->RealAtom(cPoint->array[0].x),
      nl->RealAtom(cPoint->array[0].y)));
      ListExpr last = res;  // set actual list element
    for (long i = 1; i < cPoint->count; i++) {  // read all array elements
      last = nl->Append(last,  // append the point to the nested list
        nl->TwoElemList(
          nl->RealAtom(cPoint->array[i].x),
          nl->RealAtom(cPoint->array[i].y)));
    }
    return res;  // pointer to the beginning of the nested list
  }

  Word ColPoint::Create(const ListExpr typeInfo) {
    Word answer(static_cast<void*>(0));
    point* inArray = NULL;
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
    point* array = NULL;  // array which contains the input point values
    long count;           // amount of points
    double x, y;          // actual read coordinates
    size_t sizeL = sizeof(long);
    size_t sizeD = sizeof(double);
    bool ok = (valueRecord.Read(&count, sizeL, offset) == sizeL);
    offset += sizeL;
    // allocate memory depending on the ammount of points
    array = static_cast<point*>(malloc(count * sizeof(point)));
    if (array == NULL) {    // exit on memory overflow
      cmsg.inFunError("not enough memory (??? Bytes needed)!");
      return false;
    }
    // read each point and store it into the array
    for (long i = 0; i < count; i++) {
      ok = ok && (valueRecord.Read(&x, sizeD, offset) == sizeD);
      offset += sizeD;
      array[i].x = x;
      ok = ok && (valueRecord.Read(&y, sizeD, offset) == sizeD);
      offset += sizeD;
      array[i].y = y;
      if (!ok) { break; }
    }

    if (ok) {  // create a new ColPoint and store the read values in it
      value.addr = new ColPoint(array, count);
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
      x = cPoint->array[i].x;
      y = cPoint->array[i].y;
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

  Word ColPoint::Clone(const ListExpr typeInfo, const Word& w) {
    ColPoint* cPoint = static_cast<ColPoint*>(w.addr);
    Word res;
    res.addr = new ColPoint(*cPoint);
    return res;
  }

  void* ColPoint::Cast(void* addr) {
    return (new (addr) ColPoint);
  }

  bool ColPoint::TypeCheck(ListExpr type, ListExpr& errorInfo) {
    return nl->IsEqual(type, BasicType());
  }

  int ColPoint::SizeOf() {
    return sizeof(point);
  }

//------------------------------------------------------------------------------

/*
3.3 Class ColLine for column-oriented representation of lines

*/
  ColLine::ColLine() {}  // standard constructor doing nothing
  ColLine::ColLine(line* newArray, long newCount) {  // non-standard constructor
    array = newArray;
    count = newCount;
  }
  ColLine::ColLine(int min) {  // constructor with minimum initialized arrays
    array = NULL;
    array = static_cast<line*>(calloc(1, sizeof(line)));
    count = 0;
  }
  ColLine::~ColLine() {   // destructor
    free(array);
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
      nl->StringAtom("((x1 y1 x2 y2 ... xn yn) ... )"),
      nl->StringAtom("((3.5 -6.0 5 3) (1.0 2 5 2) (-9.1 5))"),
      nl->StringAtom("An aline is created by a nested list of lines"))));
  }

  // scans a nested-list and converts it into an array of lines.
  Word ColLine::In(const ListExpr typeInfo, ListExpr instance,
                 const int errorPos, ListExpr &errorInfo, bool &correct) {
    correct = false;
    line* inArray = NULL;  // array which contains the input line values
    long inCount = 0;       // counter of input lines, grows with each line
    long step = 0;          // index of actual allocated memory (allocBytes)
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
    while (!nl->IsEmpty(instance)) {  // as long as there are lines left
      ListExpr lineLeaf = nl->First(instance);  // get the line
      if ((!nl->HasLength(lineLeaf, 4)) ||
          (!listutils::isNumeric(nl->First(lineLeaf))) ||
          (!listutils::isNumeric(nl->Second(lineLeaf)))) {
          // exit on invalid point
        cmsg.inFunError("expected an atom with four numeric values!");
        return error;
      }
      // allocate more memory if the actual allocated memory is insufficient
      if ((long)((inCount + 1) * sizeof(line)) >= allocBytes[step]) {
        // allocate more memory - C++ needs type casting unlike C
        inArray = static_cast<line*>(realloc(inArray, allocBytes[++step]));
        if (inArray == NULL) {  // exit on memory overflow
          cmsg.inFunError("not enough memory (??? Bytes needed)!");
          return error;
        }
      }
      // append the coordinates from the actual line to the array
      inArray[inCount].x1 = listutils::getNumValue((nl->First(lineLeaf)));
      inArray[inCount].y1 = listutils::getNumValue((nl->Second(lineLeaf)));
      inArray[inCount].x2 = listutils::getNumValue((nl->Third(lineLeaf)));
      inArray[inCount].y2 = listutils::getNumValue((nl->Fourth(lineLeaf)));
      inCount++;
      instance = nl->Rest(instance);  // move to next entry
    }
    // truncate oversized memory to allocate the real used memory
    inArray = static_cast<line*>(realloc(inArray, inCount * sizeof(line)));
    Word answer(static_cast<void*>(0));
    answer.addr = new ColLine(inArray, inCount);  // create a new line object
    correct = true;
    return answer;  // return its adress
  }

  // converts a line array into a nested list format
  ListExpr ColLine::Out(ListExpr typeInfo, Word value) {
    ColLine* cLine = static_cast<ColLine*>(value.addr);
    if (cLine->count == 0) {
      return listutils::emptyErrorInfo();
    }
    ListExpr res = nl->OneElemList(nl->FourElemList(  // init list
      nl->RealAtom(cLine->array[0].x1),
      nl->RealAtom(cLine->array[0].y1),
      nl->RealAtom(cLine->array[0].x2),
      nl->RealAtom(cLine->array[0].y2)));
    ListExpr last = res;  // set actual list element
    for (long i = 1; i < cLine->count; i++) {  // read all array elements
      last = nl->Append(last,  // append the line to the nested list
        nl->FourElemList(
          nl->RealAtom(cLine->array[i].x1),
          nl->RealAtom(cLine->array[i].y1),
          nl->RealAtom(cLine->array[i].x2),
          nl->RealAtom(cLine->array[i].y2)));
    }
    return res;  // pointer to the beginning of the nested list
  }

  Word ColLine::Create(const ListExpr typeInfo) {
    Word answer(static_cast<void*>(0));
    line* inArray = NULL;
    answer.addr = new ColLine(inArray, 0);  // create a new line object
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
    line* array = NULL;  // array which contains the input line values
    long count;           // amount of lines
    double x1, y1, x2, y2;          // actual read coordinates
    size_t sizeL = sizeof(long);
    size_t sizeD = sizeof(double);
    bool ok = (valueRecord.Read(&count, sizeL, offset) == sizeL);
    offset += sizeL;
    // allocate memory depending on the ammount of lines
    array = static_cast<line*>(malloc(count * sizeof(line)));
    if (array == NULL) {    // exit on memory overflow
      cmsg.inFunError("not enough memory (??? Bytes needed)!");
      return false;
    }
    // read each line and store it into the array
    for (long i = 0; i < count; i++) {
      ok = ok && (valueRecord.Read(&x1, sizeD, offset) == sizeD);
      offset += sizeD;
      array[i].x1 = x1;
      ok = ok && (valueRecord.Read(&y1, sizeD, offset) == sizeD);
      offset += sizeD;
      array[i].y1 = y1;
      ok = ok && (valueRecord.Read(&x2, sizeD, offset) == sizeD);
      offset += sizeD;
      array[i].x2 = x2;
      ok = ok && (valueRecord.Read(&y2, sizeD, offset) == sizeD);
      offset += sizeD;
      array[i].y2 = y2;
      if (!ok) { break; }
    }

    if (ok) {  // create a new ColLine and store the read values in it
      value.addr = new ColLine(array, count);
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
    long c = cLine->count;
    double x1, y1, x2, y2;
    bool ok = valueRecord.Write(&c, sizeL, offset);
    offset += sizeL;
    for (long i = 0; i < cLine->count; i++) {
      x1 = cLine->array[i].x1;
      ok = ok && valueRecord.Write(&x1, sizeL, offset);
      offset += sizeD;
      y1 = cLine->array[i].y1;
      ok = ok && valueRecord.Write(&y1, sizeL, offset);
      offset += sizeD;
      x2 = cLine->array[i].x2;
      ok = ok && valueRecord.Write(&x2, sizeL, offset);
      offset += sizeD;
      y2 = cLine->array[i].y2;
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
    Word res;
    res.addr = new ColLine(*cLine);
    return res;
  }

  void* ColLine::Cast(void* addr) {
    return (new (addr) ColLine);
  }

  bool ColLine::TypeCheck(ListExpr type, ListExpr& errorInfo) {
    return nl->IsEqual(type, BasicType());
  }

  int ColLine::SizeOf() {
    return sizeof(line);  // the result isn't meaningful
  }

//-----------------------------------------------------------------------------

/*
Class ColRegion for column-oriented representation of Regions

*/
  ColRegion::ColRegion() {}  // standard constructor doing nothing
  // non-standard constructor initializing the object with parameters
  ColRegion::ColRegion(sTuple* newTuple, sCycle* newCycle, sPoint* newPoint,
            long newCountTuple, long newCountCycle, long newCountPoint) {
    aTuple = newTuple;
    aCycle = newCycle;
    aPoint = newPoint;
    countTuple = newCountTuple;
    countCycle = newCountCycle;
    countPoint = newCountPoint;
  }
  // non-standard constructor initializing the object with minimum data
  ColRegion::ColRegion(int min) {
    aTuple = NULL;
    aCycle = NULL;
    aPoint = NULL;
    aTuple = static_cast<sTuple*>(calloc(1, sizeof(sTuple)));
    aCycle = static_cast<sCycle*>(calloc(1, sizeof(sCycle)));
    aPoint = static_cast<sPoint*>(calloc(1, sizeof(sPoint)));
    countTuple = 0;
    countCycle = 0;
    countPoint = 0;
  }
  ColRegion::~ColRegion() {   // destructor - free allocated menory
    free(aTuple);
    free(aCycle);
    free(aPoint);
  }

  // returns the corresponding basic type
  const string ColRegion::BasicType() { return "aregion"; }

  // compares the type of the given object with class type
  const bool ColRegion::checkType(const ListExpr list) {
    return listutils::isSymbol(list, BasicType());
  }

  // allocates memory for the point array and appends the given point
  int ColRegion::appendPoint(Point p, long &stepPoint) {
    double x = p.GetX();
    double y = p.GetY();
    // allocate more memory if actual allocated memory is insufficient
    if ((long) ((countPoint + 2) * sizeof(sPoint)) >= allocBytes[stepPoint]) {
      // allocate more memory - in C++ a type casting is necessary
      aPoint = static_cast<sPoint*>
        (realloc(aPoint, allocBytes[++stepPoint]));  // increase alloc counter
      if (aPoint == NULL) {  // exit on memory overflow
        cmsg.inFunError("not enough memory!");
        return 0;
      }
    }
    aPoint[countPoint].x = x;
    aPoint[countPoint].y = y;
    // adjust mbr coordinates if neccessary
    if (x < aTuple[countTuple].mbrX1) aTuple[countTuple].mbrX1 = x;
    if (x > aTuple[countTuple].mbrX2) aTuple[countTuple].mbrX2 = x;
    if (y < aTuple[countTuple].mbrY1) aTuple[countTuple].mbrY1 = y;
    if (y > aTuple[countTuple].mbrY2) aTuple[countTuple].mbrY2 = y;
    countPoint++;  // increase index of point array
    return 1;
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
        cmsg.inFunError("not enough memory!");
        return 0;
      }
    }
    aCycle[countCycle].index = cp;
    countCycle++;
    return 1;
  }

/*
This function appends one region of the spatial algebra
to an existing aregion of the column spatial algebra.
It's algorithm is similar to the ~OutRegion~ - function of the spatial algebra.

*/
  bool ColRegion::append(Region* region) {
    if(!region->IsDefined()) {
      cout << "region is undefined!" << endl;
      return false;
    }
    if(region->IsEmpty()) {
      cout << "region is empty" << endl;
      return false;
    }
    long stepTuple = 0;
    long stepCycle = 0;
    long stepPoint = 0;
    // calculate and allocate sufficient memory for the tuple array
    while ((long)
          ((countTuple + 2) * sizeof(sTuple)) >= allocBytes[stepTuple]) {
      stepTuple++;
    }
    aTuple = static_cast<sTuple*> (realloc(aTuple, allocBytes[stepTuple++]));
    if (aTuple == NULL) {  // exit on memory overflow
      cmsg.inFunError("not enough memory!");
      return false;
    }
    // initialize tuple array with region. mbr set to extremes
    aTuple[countTuple].indexCycle = countCycle;
    aTuple[countTuple].indexPoint = countPoint;
    aTuple[countTuple].mbrX1 = 999999;
    aTuple[countTuple].mbrY1 = 999999;
    aTuple[countTuple].mbrX2 = -999999;
    aTuple[countTuple].mbrY2 = -999999;
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
      cmsg.inFunError("1 Wrong data format: discontiguous segments!");
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
            cmsg.inFunError("2 Wrong data format: discontiguous segments!");
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
            cmsg.inFunError("3 Wrong data format: discontiguous segments!");
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
          cmsg.inFunError("4 Wrong data format: discontiguous segments!");
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
    countTuple++;
    return true;
  }

  // add terminator entries to arrays and finalize counters
  void ColRegion::finalize() {
    aPoint[countPoint].x = 0;
    aPoint[countPoint].y = 0;
    aCycle[countCycle].index = countPoint;
    aTuple[countTuple].indexCycle = countCycle;
    aTuple[countTuple].indexPoint = countPoint;
    aTuple[countTuple].mbrX1 = 0;
    aTuple[countTuple].mbrY1 = 0;
    aTuple[countTuple].mbrX2 = 0;
    aTuple[countTuple].mbrY2 = 0;
    countTuple++;
    countCycle++;
    countPoint++;
  }

  // test output of the arrays aRegion, aCycle and aPoint and there parameters
  void ColRegion::showArrays(string title, bool showPoints) {
    // example output of a nested list: cout << nl->ToString(list) << endl;
    cout << "\n--------------------------------------------\n" << title << "\n";
    // output of the array aTuple
    cout << "aRegion (" << countTuple << "):\n";
    cout << "Bytes allocated: " << countTuple * sizeof(aTuple[0]) << "\n";
    cout << "index\tcycle\tpoint\tmbr-l\tmbr-b\tmbr-r\tmbr-t\n";
    for (long ct = 0; ct < countTuple; ct++) {
      cout << ct << ":\t" << aTuple[ct].indexCycle << "\t"
           << aTuple[ct].indexPoint << "\t"
           << aTuple[ct].mbrX1 << "\t"
           << aTuple[ct].mbrY1 << "\t"
           << aTuple[ct].mbrX2 << "\t"
           << aTuple[ct].mbrY2 << "\n";
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

  int ColRegion::p_inside(const Word value, const long count, Word& attrarray)
  const {


//      ColPoint* cPoint = static_cast<ColPoint*>(value.addr);  // Punkt-Array
    // AttrArray aarray = (AttrArray*) attrarray.addr;      // Ausgabe-Array
//    for (long i=0; i < count; i++) {
//      if (cPoint->array[i].x, cPoint->array[i].y) {
          // aarray.Append(&i);
//      }
//    }
    return 0;
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
    sTuple* inTuple = NULL;
    sCycle* inCycle = NULL;
    sPoint* inPoint = NULL;
    long ct = 0;  // index counter of the tuple array
    long cc = 0;  // index counter of the cycle array
    long cp = 0;  // index counter of the point array
    // index to the array allocbytes of actual allocated memory for each array
    long stepTuple = 0;
    long stepCycle = 0;
    long stepPoint = 0;

    // allocate memory for the input arrays
    inTuple = static_cast<sTuple*>(realloc(inTuple, allocBytes[stepTuple]));
    inCycle = static_cast<sCycle*>(realloc(inCycle, allocBytes[stepCycle]));
    inPoint = static_cast<sPoint*>(realloc(inPoint, allocBytes[stepPoint]));
    int cycleSign = 1;  // 1 marks an outer cycle and -1 marks an inner cycle
    // loops to parse a nested list of regions
    ListExpr regionNL = instance;
    while (!nl->IsEmpty(regionNL)) {  // as long as there are regions left
      // allocate more memory if the actual allocated memory is insufficient
      if ((long)
        ((ct + 2) * sizeof(sTuple)) >= allocBytes[stepTuple]) {
        // allocate more memory - in C++ a type casting is necessary
        inTuple = static_cast<sTuple*>
                  (realloc(inTuple, allocBytes[++stepTuple]));
        if (inTuple == NULL) {  // exit on memory overflow
          cmsg.inFunError("not enough memory!");
          return error;
        }
      }
      inTuple[ct].indexCycle = cc;
      inTuple[ct].indexPoint = cp;
      // values for the actual minimum bounding rectangle per region
      inTuple[ct].mbrX1 = 0;
      inTuple[ct].mbrY1 = 0;
      inTuple[ct].mbrX2 = 0;
      inTuple[ct].mbrY2 = 0;
      ct++;
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
            inPoint[cp].x = listutils::getNumValue((nl->First(pointNL)));
            inPoint[cp].y = listutils::getNumValue((nl->Second(pointNL)));

            if (inPoint[cp].x < inTuple[ct].mbrX1)
              inTuple[ct].mbrX1 = inPoint[cp].x;
            if (inPoint[cp].x > inTuple[ct].mbrX2)
              inTuple[ct].mbrX2 = inPoint[cp].x;
            if (inPoint[cp].y < inTuple[ct].mbrY1)
              inTuple[ct].mbrY1 = inPoint[cp].y;
            if (inPoint[cp].y > inTuple[ct].mbrY2)
              inTuple[ct].mbrY2 = inPoint[cp].y;

            cp++;
            cycleNL = nl->Rest(cycleNL);  // "move" to next point
          }
          faceNL = nl->Rest(faceNL);      // "move" to next cycle
        }
        tupleNL = nl->Rest(tupleNL);      // "move" to next face
      }
      regionNL = nl->Rest(regionNL);      // "move" to next region
    }
    // add terminator entries to arrays. they are needed for the Out-function.
    inPoint[cp].x = 0;
    inPoint[cp].y = 0;
    inCycle[cc].index = cp;
    inTuple[ct].indexCycle = cc;
    inTuple[ct].indexPoint = cp;
    inTuple[ct].mbrX1 = 0;
    inTuple[ct].mbrY1 = 0;
    inTuple[ct].mbrX2 = 0;
    inTuple[ct].mbrY2 = 0;
    // truncate oversized memory to allocate only the real used memory
    inTuple = static_cast<sTuple*>(realloc(inTuple, ++ct * sizeof(sTuple)));
    inCycle = static_cast<sCycle*>(realloc(inCycle, ++cc * sizeof(sCycle)));
    inPoint = static_cast<sPoint*>(realloc(inPoint, ++cp * sizeof(sPoint)));
    cout << ct * sizeof(sTuple) + cc * sizeof(sCycle) + cp * sizeof(sPoint)
         << " bytes used\n";
    Word answer(static_cast<void*>(0));
    // create new region object with the just read in arrays
    answer.addr = new ColRegion(inTuple, inCycle, inPoint, ct, cc, cp);
    // the following two lines are only for debugging mode
    // ColRegion* cRegion = static_cast<ColRegion*>(answer.addr);
    // cRegion->showArrays("ColRegion after In-function", true);
    correct = true;
    return answer;  // return the object
  }

  ListExpr ColRegion::Out(ListExpr typeInfo, Word value) {
    ColRegion* cRegion = static_cast<ColRegion*>(value.addr);
    if (cRegion->countTuple == 0) {  // no elements lead to error
      return listutils::emptyErrorInfo();
    }
    // counters for index arrays aTuple and aCycle starting at second entries
    long ct = 1;
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
          if (cp == cRegion->aTuple[ct].indexPoint) {
            if (nl->IsEmpty(regionNL)) {  // append last tuple to new region
              regionNL = nl->OneElemList(tupleNL);
              regionNLLast = regionNL;
            } else {  // append last tuple to last region
              regionNLLast = nl->Append(regionNLLast, tupleNL);
            }
            // clear tuple and increase region counter
            tupleNL = nl->TheEmptyList();
            tupleNLLast = tupleNL;
            ct++;
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
    sTuple *inTuple = NULL;
    // create a new region object
    answer.addr = new ColRegion(inTuple, inCycle, inPoint, 0, 0, 0);
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
    sTuple* tuple = NULL;
    sCycle* cycle = NULL;
    sPoint* point = NULL;
    // initialize counters
    long ct;
    long cc;
    long cp;
    long valueL;
    double x, y;          // actual read coordinates
    size_t sizeL = sizeof(long);
    size_t sizeD = sizeof(double);
    // 1. read the counters from disc
    bool ok = (valueRecord.Read(&ct, sizeL, offset) == sizeL);
    offset += sizeL;
    ok = ok && (valueRecord.Read(&cc, sizeL, offset) == sizeL);
    offset += sizeL;
    ok = ok && (valueRecord.Read(&cp, sizeL, offset) == sizeL);
    offset += sizeL;
    // allocate memory depending on the counters
    tuple = static_cast<sTuple*>(malloc(ct * sizeof(sTuple)));
    if (tuple == NULL) {    // exit on memory overflow
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
    for (long i = 0; i < ct; i++) {
      ok = ok && (valueRecord.Read(&valueL, sizeL, offset) == sizeL);
      offset += sizeL;
      tuple[i].indexCycle = valueL;
      ok = ok && (valueRecord.Read(&valueL, sizeL, offset) == sizeL);
      offset += sizeL;
      tuple[i].indexPoint = valueL;
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
      ok = ok && (valueRecord.Read(&x, sizeD, offset) == sizeD);
      offset += sizeD;
      point[i].x = x;
      ok = ok && (valueRecord.Read(&y, sizeD, offset) == sizeD);
      offset += sizeD;
      point[i].y = y;
      if (!ok) { break; }
    }
    if (ok) {  // create a new ColRegion and store the read values in it
      value.addr = new ColRegion(tuple, cycle, point, ct, cc, cp);
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
    long ct = cRegion->countTuple;
    long cc = cRegion->countCycle;
    long cp = cRegion->countPoint;
    long valueL;
    double x, y;
    // 1. write the counters to disc
    bool ok = valueRecord.Write(&ct, sizeL, offset);
    offset += sizeL;
    ok = ok && valueRecord.Write(&cc, sizeL, offset);
    offset += sizeL;
    ok = ok && valueRecord.Write(&cp, sizeL, offset);
    offset += sizeL;
    // 2. write the tuple array to disc
    for (long i = 0; i < ct; i++) {
      valueL = cRegion->aTuple[i].indexCycle;
      ok = ok && valueRecord.Write(&valueL, sizeL, offset);
      offset += sizeL;
      valueL = cRegion->aTuple[i].indexPoint;
      ok = ok && valueRecord.Write(&valueL, sizeL, offset);
      offset += sizeL;
    }
    // 3. write the cycle array to disc
    for (long i = 0; i < cc; i++) {
      valueL = cRegion->aCycle[i].index;
      ok = ok && valueRecord.Write(&valueL, sizeL, offset);
      offset += sizeL;
    }
    // 4. write the point array to disc
    for (long i = 0; i < cp; i++) {
      x = cRegion->aPoint[i].x;
      y = cRegion->aPoint[i].y;
      ok = ok && valueRecord.Write(&x, sizeD, offset);
      offset += sizeD;
      ok = ok && valueRecord.Write(&y, sizeD, offset);
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
    return sizeof(sTuple) + sizeof(sCycle) + sizeof(sPoint);
  }

//------------------------------ Operators ------------------------------------

// Type Mapping

ListExpr insideTM(ListExpr args) {
    string err = "{apoint | aline | aregion} x region expected\n";
    if (!nl->HasLength(args, 2)) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if ((ColPoint::checkType(nl->First(args)) ||
         ColLine::checkType(nl->First(args)) ||
         ColRegion::checkType(nl->First(args))) &&
         Region::checkType(nl->Second(args))) {
      return listutils::basicSymbol<CcBool>();  // replace by CRel::Ints
    }
    return listutils::typeError(err);
}

ListExpr mapTM(ListExpr args) {
  if (!nl->HasLength(args,2)) {
     return listutils::typeError("\nWrong number of arguments!");
  }
  if (!Stream<Tuple>::checkType(nl->First(args))) {
    return listutils::typeError("\nFirst arg is not a tuple stream!");
  }
  if (nl->AtomType(nl->Second(args))!=SymbolType) {
     return listutils::typeError("\nSecond arg is not a valid attribute name!");
  }
  // extract the attribute list
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr type;
  string name = nl->SymbolValue(nl->Second(args));
  int j = listutils::findAttribute(attrList, name, type);
  if (j==0) {
    return listutils::typeError("\nAttribute " + name + " not found!");
  };
  // check type and return corresponding result type using the append mechanism
  if (Point::checkType(type) ||
      Line::checkType(type) ||
      Region::checkType(type)) {
    cout << "Attribute " << j << " is " << nl->ToString(type) << endl;
    return nl->ThreeElemList(
           nl->SymbolAtom(Symbols::APPEND()),
           nl->OneElemList(nl->IntAtom(j)),
           type);
  } else {
    return listutils::typeError("\nAttribute type point, line or region"
                                "not found!");
  }
}

int insidePointVM (Word* args, Word& result, int message,
                   Word& local, Supplier s) {
//ColPoint* point = (ColPoint*) args[0].addr;
//Region* region = (Region*) args[1].addr;
//result = qp->ResultStorage(s);
//(AttrArray*) result.addr;
//region->p_inside(point->getArray(), point->getCount(), result.addr);
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
  return 0;
}

/*
The operator ~mapLineVM~ expects a stream of tuples as input parameter.
It extracts the ~line~ attribute of each tuple and hands it over to the
append function of the column spatial ~aline~ object.

*/
int mapLineVM (Word* args, Word& result, int message,
               Word& local, Supplier s) {
  result.addr = new ColLine(1);
  ColLine* cLine = static_cast<ColLine*> (result.addr);
  result = qp->ResultStorage(s);         // use result storage for the result
  CcInt* index = (CcInt*) args[2].addr;  // index of the appended attribute
  int v = index->GetValue();
  Stream<Tuple> stream(args[0]);         // get the tuples
  stream.open();                         // open the stream
  Tuple* tuple;                          // actual tuple element
  Line* line;                            // actual line attribute
  while( (tuple = stream.request()) ) {  // if exists, get next tuple
    // extract region from tuple
    region = (Line*) tuple->GetAttribute(v - 1);
    if (!cLine->append(line)) {          // append line to aline
      cout << "Error in line mapping!";
      return 0;
    }
    tuple->DeleteIfAllowed();            // remove tuple from stream
   }
  stream.close();                        // close the stream
  cRegion->finalize();
  // next line is only for debugging cases - should be commented out
  cRegion->showArrays("Nach append aller Linien", false);
  return 0;
}

/*
The operator ~mapRegionVM~ expects a stream of tuples as input parameter.
It extracts the ~region~ attribute of each tuple and hands it over to the
append function of the column spatial ~aregion~ object.

*/
int mapRegionVM (Word* args, Word& result, int message,
                 Word& local, Supplier s) {
  result.addr = new ColRegion(1);        // result set to cRegion object
  ColRegion* cRegion = static_cast<ColRegion*> (result.addr);
  result = qp->ResultStorage(s);         // use result storage for the result
  CcInt* index = (CcInt*) args[2].addr;  // index of the appended attribute
  int v = index->GetValue();
  Stream<Tuple> stream(args[0]);         // get the tuples
  stream.open();                         // open the stream
  Tuple* tuple;                          // actual tuple element
  Region* region;                        // actual region attribute
  while( (tuple = stream.request()) ) {  // if exists, get next tuple
    // extract region from tuple
    region = (Region*) tuple->GetAttribute(v - 1);
    if (!cRegion->append(region)) {      // append region to aregion
      cout << "Error in mapping!";
      return 0;
    }
    tuple->DeleteIfAllowed();            // remove tuple from stream
   }
  stream.close();                        // close the stream
  cRegion->finalize();
  // next line is only for debugging cases - should be commented out
  cRegion->showArrays("Nach append aller Regionen - Ausgabe ohne Punkte",
                      false);
  return 0;
}

/*
Selection Function

*/
int insideSelect(ListExpr args) {
    if (ColPoint::checkType(nl->First(args))) { return 0; }
    if (ColLine::checkType(nl->First(args))) { return 1; }
    return 2; // ColRegion
}

int mapSelect(ListExpr args) {
    // extract the attribute list
    ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
    ListExpr type;
    string name = nl->SymbolValue(nl->Second(args));
    listutils::findAttribute(attrList, name, type);
    if (Point::checkType(type)) return 0;  // mapPointVM
    if (Line::checkType(type)) return 1;   // mapLineVM
    // in any other case choose mapRegionVM
    // if this is wrong, it will be detected in the type mapping function
    return 2;
}


}  // namespace col
