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
#include "ColumnSpatialAlgebra.h"           // header for this algebra
#include "../../include/Symbols.h"          // predefined strings
#include "../../include/ListUtils.h"        // useful functions for nested lists
#include "../../include/NestedList.h"       // required at many places
#include "../../include/QueryProcessor.h"   // needed for value mappings
#include "../../include/TypeConstructor.h"  // constructor for Secondo Types
#include "../Spatial/SpatialAlgebra.h"      // spatial types and operators
#include "../Stream/Stream.h"               // wrapper for secondo streams
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
      nl->StringAtom("An apoint is created by a nested list of points."))));
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

  // clear variables if neccessary
  void ColRegion::clear(Word& arg) {
    ColRegion* cr = static_cast<ColRegion*>(arg.addr);
    // free all arrays
    free(cr->aTuple);
    free(cr->aCycle);
    free(cr->aPoint);
    cout << "clear... done";
  }

/*
This function appends one region datatype of the spatial algebra
to an attrarray of regions of the column spatial algebra.
It needs the source ~region~ and the destiniation ~aregion~ as parameters.

*/
  void ColRegion::append(ColRegion* cRegion, Region* region) {
    //ColRegion* cRegion = static_cast<ColRegion*>(arg.addr);
    cout << "append: " << region->BasicType() << " -> " << cRegion->BasicType();
    int hSegCount = region->Size();
    HalfSegment hSeg;
    for (int i = 0; i < hSegCount; i++) {
      if (region->Get(i, hSeg)){
        cout << "x=" << hSeg.GetDomPoint().GetX() << "/ty="
             << hSeg.GetDomPoint().GetY();
      }
    }
  }

  // test output of the arrays aRegion, aCycle and aPoint and there parameters
  void ColRegion::showArrays(string title) {
    cout << "\n--------------------------------------------\n" << title << "\n";
    cout << "aRegion (" << countTuple << "):\n";
    cout << "Bytes allocated: " << countTuple * sizeof(aTuple[0]) << "\n";
    cout << "index\tcycle\tpoint\n";
    for (long ct = 0; ct < countTuple; ct++) {
      cout << ct << ":\t" << aTuple[ct].indexCycle << "\t"
           << aTuple[ct].indexPoint << "\n";
    }
    cout << "aCycle (" << countCycle << "):\n";
    cout << "Bytes allocated: " << countCycle * sizeof(aCycle[0]) << "\n";
    cout << "index\tpoint\n";
    for (long cc = 0; cc < countCycle; cc++) {
      cout << cc << ":\t" << aCycle[cc].index << "\n";
    }
    cout << "aPoint (" << countPoint << "):\n";
    cout << "Bytes allocated: " << countPoint * sizeof(aPoint[0]) << "\n";
    cout << "index\tx\ty\n";
    for (long cp = 0; cp < countPoint; cp++) {
      cout << cp << ":\t" << aPoint[cp].x << "\t" << aPoint[cp].y << "\n";
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
    while (!nl->IsEmpty(regionNL)) {  // regions
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
      // temp values for the actual minimum bounding rectangle per region
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
            inCycle = (sCycle*) realloc(inCycle, allocBytes[++stepCycle]);
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
              inTuple[ct].mbrX2 = inPoint[cp].x;
            if (inPoint[cp].y > inTuple[ct].mbrY2)
              inTuple[ct].mbrY2 = inPoint[cp].x;

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
    // truncate oversized memory to allocate the real used memory
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
    // cRegion->showArrays("ColRegion after In-function");
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
        } else {  // negative value = append hole to last face
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
      return listutils::basicSymbol<CcBool>();
    }
    return listutils::typeError(err);
}

ListExpr mapTM(ListExpr args) {
  string err = "stream({point|line|region}) x "
               "{apoint|aline|aregion} expected\n";
  // check for correct number of arguments
  if (!nl->HasLength(args, 2)) {
      return listutils::typeError(err + " (wrong number of arguments)");
  }
  // first argument must be a stream of points, lines or regions
  // second argument must be a single column spatial type apoint, aline, aregion

  if( Stream<CcInt>::checkType(nl->First(args))
  && CcInt::checkType(nl->Second(args))){
  }

  if ((Point::checkType(nl->Second(args)) &&
       ColPoint::checkType(nl->First(args))) ||
      (Line::checkType(nl->Second(args)) &&
       ColLine::checkType(nl->First(args))) ||
      (Region::checkType(nl->Second(args)) &&
       ColRegion::checkType(nl->First(args)))) {
    // result is a bool
    return listutils::basicSymbol<CcBool>();
  }
  return listutils::typeError(err + " (wrong type of arguments)");
}

/*
Value mapping functions
TODO: replace by type mapping

*/
int insidePoint (Word* args, Word& result, int message,
                 Word& local, Supplier s) {

//ColPoint* point = (ColPoint*) args[0].addr;
//Region* region = (Region*) args[1].addr;
//result = qp->ResultStorage(s);
//(AttrArray*) result.addr;
//region->p_inside(point->getArray(), point->getCount(), result.addr);

  return 0;
}

int insideLine (Word* args, Word& result, int message,
                Word& local, Supplier s) {

//ColLine* line = (ColLine*) args[0].addr;
//Region* region = (Region*) args[1].addr;
//result = qp->ResultStorage(s);
//(AttrArray*) result.addr;
//region->l_inside(line->getArray(), line->getCount(), result.addr);
  return 0;
}

int insideRegion (Word* args, Word& result, int message,
                  Word& local, Supplier s) {
//ColRegion* region1 = (ColRegion*) args[0].addr;
//Region* region2 = (Region*) args[1].addr;
//result = qp->ResultStorage(s);
//(AttrArray*) result.addr;
//region->r_inside(region->getArray(), region->getCount(), result.addr);
  return 0;
}

int mapPoint (Word* args, Word& result, int message, Word& local, Supplier s) {
 // ColPoint* point = (ColPoint*) args[0].addr;

  return 0;

}
int mapLine (Word* args, Word& result, int message, Word& local, Supplier s) {
  return 0;

}

/*
converts region to aregion by using the ~out~ function of the stream object
and an append function of the aregion object. this is just a quick
implementation
and should be modiefied for abetter performance.

TODO (Radke): direct mapping of the stream object

*/
int mapRegion (Word* args, Word& result, int message, Word& local, Supplier s) {
  // result object is ColRegion
  ColRegion* cRegion = static_cast<ColRegion*> (result.addr);
  cRegion->clear(result);               // clear variables of instance
  result = qp->ResultStorage(s);        // use result storage for the result
  Stream<Region> stream(args[0]);       // wrap the stream
  stream.open();                        // open the stream
  Region* elem;                         // actual region element
  while( (elem = stream.request()) ){   // get next region, if not empty
    cRegion->append(cRegion, elem);              // append region to
    //array(region)
    elem->DeleteIfAllowed();            // remove element
  }
  //Region* res = (Region*) result.addr;  // initialize res with result
  //res->Set(true, cRegion);              // set result to the array(region)
  stream.close();                       // close the stream
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
    if (ColPoint::checkType(nl->First(args))) { return 0; }
    if (ColLine::checkType(nl->First(args))) { return 1; }
    return 2; // ColRegion
}

}  // namespace col
