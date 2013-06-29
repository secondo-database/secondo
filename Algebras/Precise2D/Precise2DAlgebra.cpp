/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2009, University in Hagen, Faculty of Mathematics and
 Computer Science, Database Systems for New Applications.

 SECONDO is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published
 by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 SECONDO is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with SECONDO; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 ----

 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[TOC] [\tableofcontents]
 //[->] [$\rightarrow$]

 [TOC]

 0 Overview

 1 Includes and defines

*/

#include "Precise2DAlgebra.h"


extern NestedList* nl;
extern QueryProcessor* qp;

namespace p2d {

/*
 1.1.1 Function ~textTypeToGmpType()~

 Reads from inList and stores its representation as mpq\_class in outValue.

 This function was first implemented by Stefanie Renner in the
 MovingObjects2Algebra and is now adapted for negative values.
 Additionally there are now some more limitations for valid numbers.

*/
bool textTypeToGmpType(const ListExpr& inList, mpq_class& preciseValue,
  bool negValueAllowed) {

 TextScan theScan = nl->CreateTextScan(inList);
 stringstream theStream;
 int noSlash = 0;

 char lastChar = '*'; //just a random initialization...
 for (unsigned int i = 0; i < nl->TextLength(inList); i++) {
  string str = "";
  nl->GetText(theScan, 1, str);

  if ((int) str[0] == 47){
   noSlash++;
  }

  //Checking for valid character
  if (!(i == 0 && (int) str[0] == 240)) {
   if ((int) str[0] < 45 || (int) str[0] == 46 || (int) str[0] > 57
     || (i == 0 && (int) str[0] == 48 && nl->TextLength(inList) > 1)
     || (i == 0 && (int) str[0] == 47)
     || (lastChar == '/' && (int) str[0] == 48)
     || (noSlash>1)
     || (((int) str[0] == 45) && (!negValueAllowed))
     || ((int) str[0] == 45 && negValueAllowed && (!((lastChar == '/')
     ||(i==0))))
     || ((lastChar == '-') && ((int) str[0] == 47))) {
    stringstream message;
    message << "Precise coordinate not valid: " << nl->ToString(inList) << endl
      << "Only characters 1, 2, 3, 4, 5, "
        "6, 7, 8, 9, 0 and / allowed." << endl << "0 mustn't be leading "
        "character when "
        "more than one character "
        "in total are given" << endl << ", and 0 is "
        "not allowed directly after /";
    //throw invalid_argument(message.str());
    cerr << message.str()<<endl;
    return false;
   }
  }

  theStream.put(str[0]);
  lastChar = str[0];
 }

 theStream >> preciseValue;

 preciseValue.canonicalize();
 //Checking the value - must be between 0 and 1
 if (preciseValue >= 1 || (preciseValue < 0 && (!negValueAllowed))) {
  stringstream message;
  message << "Precise coordinate not valid: " << nl->ToString(inList) << endl
    << "Resulting value "
      "is not between 0 and 1, "
      "where 0 is allowed, but 1 is not.";
  //throw invalid_argument(message.str());
  cerr << message.str() << endl;
  return false;
 }
 return true;
}

/*
 ~gmpTypeToTextType()~

 Reads from ~value~ and stores its representation as TextType in ~resultList~.

*/
void gmpTypeToTextType(const mpq_class& value, ListExpr& resultList) {
 string str = value.get_str();
 resultList = nl->TextAtom(str);
}

/*
 ~prepareValue~

 A negative value is internally stored with a grid value and a precise value
 like the positive values. But the internal representation has to be changed
 from the real value for correct computations with positive and negative values:
 P.e. if we have a real value -4.3, the internal representation will be
 -5 for the grid value and 7/10 for the precise value. Before the real value
 is printed, he has to be computed from the internal value.

*/
void internalValueToOutputValue(int& gValue, mpq_class& pValue){

 if ((gValue < 0) && ((cmp(pValue, 0 ) != 0))){
  pValue = 1 - pValue ;
  gValue++;

  if (gValue==0){
   pValue = pValue * (-1);
  }
 }

}

/*
 ~outputValueToInternalValue~

 The real value ~gValue~ + ~pValue~ is already split into two parts.
 If the real value is smaller 0, the internal value must be adjusted.
 If the real value is p. e. -1 1/3, the internal value is gValue=-2 and
 pValue=2/3.

*/
void outputValueToInternalValue(int& gValue, mpq_class& pValue){

 if ((gValue < 0) && (cmp(pValue, 0) != 0)){
  pValue = 1 - pValue ;
  gValue--;

  if (gValue==0){
   pValue = pValue * (-1);
  }
 }
 if ((gValue == 0) && (cmp(pValue, 0) < 0)){
  gValue--;
  pValue = pValue + 1;
 }
}



/*
 2.1 List Representation of point2

 The list representation of a point2 is

 ---- (x y (preciseX preciseY))
 where x and y are the values of the grid-point and
 preciseX is the difference between x and the x-coordinate
 of the real point (preciseY analog). x and y are integers,
 preciseX and preciseY are of type text

 or
 undefined
 ----

 2.1.1 ~OutPoint2~-function

*/
ListExpr Point2::OutPoint2(ListExpr typeInfo, Word value) {
 Point2* point = (Point2*) (value.addr);

 if (point->IsDefined()) {
  int gX = point->getGridX();
  int gY = point->getGridY();
  mpq_class pX = point->getPreciseX();
  mpq_class pY = point->getPreciseY();
  internalValueToOutputValue(gX, pX);
  internalValueToOutputValue(gY, pY);
  ListExpr preciseX, preciseY;
  gmpTypeToTextType(pX, preciseX);
  gmpTypeToTextType(pY, preciseY);
  return nl->ThreeElemList(nl->IntAtom(gX),
    nl->IntAtom(gY), nl->TwoElemList(preciseX, preciseY));
 } else {
  return nl->SymbolAtom(Symbol::UNDEFINED());
 }
}

/*
 2.1.2 ~InPoint2~-function

*/
Word Point2::InPoint2(const ListExpr typeInfo, const ListExpr instance,
  const int errorPos, ListExpr& errorInfo, bool& correct) {
 correct = true;
 string message = "";
 if (nl->ListLength(instance)==2){
  ListExpr first = nl->First(instance);
  ListExpr second = nl->Second(instance);
  if (nl->IsAtom(first) && nl->IsAtom(second)){
   mpq_class preciseX, preciseY;
   int x, y;
   bool firstIsCorrect = true;
   bool secondIsCorrect = true;
   firstIsCorrect = createCoordinate(first, x, preciseX);
   secondIsCorrect = createCoordinate(second, y, preciseY);
   if (firstIsCorrect && secondIsCorrect){
   Point2* p = new Point2(true, x, y, preciseX, preciseY);
   return SetWord(p);
   }
  }
 }
 if (nl->ListLength(instance) == 3) {
  ListExpr first = nl->First(instance);
  ListExpr second = nl->Second(instance);
  ListExpr third = nl->Third(instance);

  if (nl->IsAtom(first) && nl->AtomType(first) == IntType && nl->IsAtom(second)
    && nl->AtomType(second) == IntType) {
   if (nl->ListLength(third) == 2) {
    ListExpr fourth = nl->First(third);
    ListExpr fifth = nl->Second(third);
    if (nl->IsAtom(fourth) && nl->AtomType(fourth) == TextType
      && nl->IsAtom(fifth) && nl->AtomType(fifth) == TextType) {
     int gX, gY;
     mpq_class pX, pY;
     gX = nl->IntValue(first);
     gY = nl->IntValue(second);
     if (
     textTypeToGmpType(fourth, pX, gX==0) &&
     textTypeToGmpType(fifth, pY, gY==0)){
     outputValueToInternalValue(gX, pX);
     outputValueToInternalValue(gY, pY);
     Point2* p = new Point2(true, gX, gY, pX, pY);
     return SetWord(p);
     } else {
      cerr << "The false value: "<<endl;
      cerr << nl->ToString(instance)<<endl;
      cerr << "is not suitable for the gmp-Type."<< endl;
      correct = false;
      return SetWord(Address(0));
     }
    } else {
     message = "The third and fourth argument "
       "have to be of type TextType";
    }
   } else {
    message = "There has to be only "
      "2 arguments in the list "
      "with the precise data";
   }
  } else {
   message = "The first and the second "
     "argument have to be of type int.";
  }
 } else {
  if (nl->ListLength(instance)<2){
   message = nl->ListLength(instance) + "are not enough "
     "arguments.";
  } else {
   message = "There are too many arguments.";
  }
  if (listutils::isSymbolUndefined(instance)) {
   return SetWord(new Point2(false));
  }
 }
 cerr << "The false value: "<<endl;
 cerr << nl->ToString(instance)<<endl;
 cerr << message << endl;
 correct = false;
 return SetWord(Address(0));
}

/*
 2.2 List Representation of line2

 The list representation of a line2 is

 ----  $(segm_1 segm_2 ... segm_n)$, where

 $segm_i$ = ((xl yl (preciseXl preciseYl))(xr yr (preciseXr preciseYr)))
 for all i $\in \{1,...,n\}$.
 x and y are the values of the grid-point and
 preciseX is the difference between x and the x-coordinate
 of the real point (preciseY analog). x and y are integers,
 preciseX and preciseY are of type text

 or
 undefined
 ----

 2.2.1 ~OutLine2~-function

*/
ListExpr Line2::OutLine2(ListExpr typeInfo, Word value) {
 Line2* line = (Line2*) (value.addr);

 if (!line->IsDefined()) {
  return nl->SymbolAtom(Symbol::UNDEFINED());
 }
 ListExpr segment, result, last;

 result = nl->TheEmptyList();

 int gLeftX, gLeftY, gRightX, gRightY;
 mpq_class pLeftX, pLeftY, pRightX, pRightY;
 ListExpr preciseLeftX, preciseLeftY, preciseRightX, preciseRightY;

 bool firstSegment = true;
 const int sz = line->Size();

 for (int i = 0; i < sz; i++) {
  if (line->IsLeftDomPoint(i)) {
   // read data of segment no ~i~
   gLeftX = line->getLeftGridX(i);
   gLeftY = line->getLeftGridY(i);
   gRightX = line->getRightGridX(i);
   gRightY = line->getRightGridY(i);
   pLeftX = line->getPreciseLeftX(i);
   pLeftY = line->getPreciseLeftY(i);
   pRightX = line->getPreciseRightX(i);
   pRightY = line->getPreciseRightY(i);
   internalValueToOutputValue(gLeftX, pLeftX);
   internalValueToOutputValue(gLeftY, pLeftY);
   internalValueToOutputValue(gRightX, pRightX);
   internalValueToOutputValue(gRightY, pRightY);
   gmpTypeToTextType(pLeftX, preciseLeftX);
   gmpTypeToTextType(pLeftY, preciseLeftY);
   gmpTypeToTextType(pRightX, preciseRightX);
   gmpTypeToTextType(pRightY, preciseRightY);
   // create ListExpr
   segment = nl->TwoElemList(
     nl->ThreeElemList(nl->IntAtom(gLeftX), nl->IntAtom(gLeftY),
       nl->TwoElemList(preciseLeftX, preciseLeftY)),
     nl->ThreeElemList(nl->IntAtom(gRightX), nl->IntAtom(gRightY),
       nl->TwoElemList(preciseRightX, preciseRightY)));
   // add ~segment~ to ~result~
   if (firstSegment) {
    last = nl->OneElemList(segment);
    result = last;
    firstSegment = false;
   } else {
    last = nl->Append(last, segment);
   }
  }
 }
 return result;
}

/*
 2.2.2 ~InLine2~-function

*/
Word Line2::InLine2(const ListExpr typeInfo, const ListExpr instance,
  const int errorPos, ListExpr& errorInfo, bool& correct) {
 correct = true;

 if (listutils::isSymbolUndefined(instance)) {
  return SetWord(new Line2(false));
 }
 Line2* line = new Line2(true);
 line->StartBulkLoad();
 ListExpr rest = instance;
 ListExpr segment, leftPoint, rightPoint;
 int edgeno = 0;
 Point2* lp = new Point2(false);
 Point2* rp = new Point2(false);
 while (!nl->IsEmpty(rest)) {
  correct = true;
  segment = nl->First(rest);
  rest = nl->Rest(rest);
  if (nl->ListLength(segment) == 4) {
   ListExpr first = nl->First(segment);
   ListExpr second = nl->Second(segment);
   ListExpr third = nl->Third(segment);
   ListExpr fourth = nl->Fourth(segment);
   if (nl->IsAtom(first) && nl->IsAtom(second)
     && nl->IsAtom(third) && nl->IsAtom(fourth)){
    mpq_class preciseLX, preciseLY, preciseRX, preciseRY;
    int lx, ly, rx, ry;
    bool firstIsCorrect = true;
    bool secondIsCorrect = true;
    bool thirdIsCorrect = true;
    bool fourthIsCorrect = true;
    firstIsCorrect = createCoordinate(first, lx, preciseLX);
    secondIsCorrect = createCoordinate(second, ly, preciseLY);
    thirdIsCorrect = createCoordinate(third, rx, preciseRX);
    fourthIsCorrect = createCoordinate(fourth, ry, preciseRY);
    if (firstIsCorrect && secondIsCorrect && thirdIsCorrect && fourthIsCorrect){
    Point2* lp = new Point2(true, lx, ly, preciseLX, preciseLY);
    Point2* rp = new Point2(true, rx, ry, preciseRX, preciseRY);

    if (*lp == *rp) {
     correct = false;
     cerr << nl->ToString(segment) << " contains an error." << endl
       << "One segment consists of "
         "2 different points. " << endl;
     line->DeleteIfAllowed();
     delete lp;
     delete rp;
     return SetWord(Address(0));
    } else {
     if (*lp < *rp) {
      line->addSegment(true, lp, rp, edgeno);
      line->addSegment(false, lp, rp, edgeno);
     } else {
      line->addSegment(true, rp, lp, edgeno);
      line->addSegment(false, rp, lp, edgeno);
     }
     edgeno++;
    }
    }
   }
  }
  if (nl->ListLength(segment) == 2) {
   leftPoint = nl->First(segment);
   Word lpWord = Point2::InPoint2(typeInfo, leftPoint, errorPos, errorInfo,
     correct);
   lp = (Point2*) (lpWord.addr);
   if (correct) {
    rightPoint = nl->Second(segment);
    Word rpWord = Point2::InPoint2(typeInfo, rightPoint, errorPos, errorInfo,
      correct);
    rp = (Point2*) (rpWord.addr);
   }
   if (correct) {
    if (*lp == *rp) {

     correct = false;
     cerr << nl->ToString(segment) << " contains an error." << endl
       << "One segment consists of "
         "2 different points. " << endl;
     line->DeleteIfAllowed();
     delete lp;
     delete rp;
     return SetWord(Address(0));
    } else {
     if (*lp < *rp) {
      line->addSegment(true, lp, rp, edgeno);
      line->addSegment(false, lp, rp, edgeno);
     } else {
      line->addSegment(true, rp, lp, edgeno);
      line->addSegment(false, rp, lp, edgeno);
     }
     edgeno++;
    }
   } else {
    cerr << nl->ToString(segment) << " contains an error." << endl;
    line->DeleteIfAllowed();
    delete lp;
    delete rp;
    return SetWord(Address(0));
   }
  }
  if (!((nl->ListLength(segment) == 2) || (nl->ListLength(segment) == 4))){
   cerr << nl->ToString(segment) << " has a false list length. "
     << "One segment consists of 2 points or 4 numbers." << endl;
   line->DeleteIfAllowed();
   return SetWord(Address(0));
  }

 }
 line->EndBulkLoad(true, true, true);
 delete lp;
 delete rp;
 return SetWord(line);
}

/*
 2.3 List Representation of points2

 The list representation of a points2-object is

 ----  ($point_1$ $point_2$ ... $point_n$), where

 $point_i$ = (x y (preciseX preciseY))
 for all i $\in$ $\{1,...,n\}$.
 x and y are the values of the grid-point and
 preciseX is the difference between x and the x-coordinate
 of the real point (preciseY analogous). x and y are integers,
 preciseX and preciseY are of type text

 or
 undefined
 ----

 2.3.1 ~OutPoints2~-function

*/
ListExpr Points2::OutPoints2(ListExpr typeInfo, Word value) {
 Points2* points = (Points2*) (value.addr);

 if (!points->IsDefined()) {
  return nl->SymbolAtom(Symbol::UNDEFINED());
 }

 ListExpr point, result, last;
 result = nl->TheEmptyList();
 int gX, gY;
 mpq_class pX, pY;
 ListExpr preciseX, preciseY;
 bool firstPoint = true;
 const int sz = points->Size();
 for (int i = 0; i < sz; i++) {
  // Daten von point i lesen
  gX = points->getGridX(i);
  gY = points->getGridY(i);
  pX = points->getPreciseX(i);
  pY = points->getPreciseY(i);

  internalValueToOutputValue(gX, pX);
  internalValueToOutputValue(gY, pY);
  //preciseX = nl->TextAtom();
  //nl->AppendText(preciseX, points->getPreciseXAsString(i));
  //preciseY = nl->TextAtom();
  //nl->AppendText(preciseY, points->getPreciseYAsString(i));
  gmpTypeToTextType(pX, preciseX);
  gmpTypeToTextType(pY, preciseY);
  // ListExpr point erstellen
  point = nl->ThreeElemList(nl->IntAtom(gX), nl->IntAtom(gY),
    nl->TwoElemList(preciseX, preciseY));
  // point an result anhaengen
  if (firstPoint) {
   last = nl->OneElemList(point);
   result = last;
   firstPoint = false;
  } else {
   last = nl->Append(last, point);
  }

 }
 return result;
}

/*
 2.3.2 ~InPoints2~-function

*/
Word Points2::InPoints2(const ListExpr typeInfo, const ListExpr instance,
  const int errorPos, ListExpr& errorInfo, bool& correct) {
 correct = true;

 if (listutils::isSymbolUndefined(instance)) {
  return SetWord(new Points2(false));
 }
 Points2* points = new Points2(true);
 points->StartBulkLoad();
 ListExpr rest = instance;
 ListExpr point;
 Point2* p = new Point2(false);
 while (!nl->IsEmpty(rest)) {
  correct = true;
  point = nl->First(rest);
  rest = nl->Rest(rest);
  Word pointWord = Point2::InPoint2(typeInfo, point, errorPos, errorInfo,
    correct);
  p = (Point2*) (pointWord.addr);

  if (correct) {
   points->addPoint(p);
  } else {
   points->DeleteIfAllowed();
   delete p;
   return SetWord(Address(0));
  }
 }
 points->EndBulkLoad(true, true, true);
 delete p;
 return SetWord(points);
}

/*
 ~operator<<~ for Point2

*/
ostream& operator<<(ostream& o, const Point2& p) {
 if (p.IsDefined())
  o << "(" << p.getGridX() << ", " << p.getGridY() << " (" << p.getPreciseX()
    << ", " << p.getPreciseY() << "))";
 else
  o << Symbol::UNDEFINED();
 return o;
}

/*
 ~operator<<~ for Points2

*/
ostream& operator<<(ostream& o, const Points2& p) {
 if (p.IsDefined()) {
  o << "(";
  int index = 0;
  while (index < p.Size()) {
   o << "(" << p.getGridX(index) << ", " << p.getGridY(index) << " ("
     << p.getPreciseX(index) << ", " << p.getPreciseY(index) << ")) ";
  }
  o << ")";
 } else
  o << Symbol::UNDEFINED();
 return o;
}

/*
 ~operator<<~ for Line2

*/
ostream& operator<<(ostream& o, const Line2& l) {
 if (l.IsDefined()) {
  o << "(";
  int index = 0;
  while (index < l.Size()) {
   o << "((" << l.getLeftGridX(index) << ", " << l.getLeftGridY(index) << " ("
     << l.getPreciseLeftX(index) << ", " << l.getPreciseLeftY(index) << ")) ("
     << l.getRightGridX(index) << ", " << l.getRightGridY(index) << " ("
     << l.getPreciseRightX(index) << ", " << l.getPreciseRightY(index) << ")))";
  }
  o << ")";
 } else
  o << Symbol::UNDEFINED();
 return o;
}



/*
 3 Value-mapping-functions

*/

/*
 ~union\_LLL~

 ~line2~ x ~line2~ [->] ~line2~

 ~result~ contains all segments of both ~line2~-objects. If there are
 overlapping segments, ~result~ will contain only one of them.

 If one of the ~line2~-objects is not defined, ~result~ will be undefined too.

*/
int union_LLL(Word* args, Word& result, int message, Word& local, Supplier s) {

 result = qp->ResultStorage(s);
 Line2* l1 = static_cast<Line2*>(args[0].addr);
 Line2* l2 = static_cast<Line2*>(args[1].addr);
 Line2* res = static_cast<Line2*>(result.addr);
 l1->unionOP(*l2, *res);
 return 0;

}


/*
 ~intersection\_LLL~

 ~line2~ x ~line2~ [->] ~line2~

 ~result~ contains all overlapping segments of both ~line2~-objects.
 If one of the ~line2~-objects is not defined, ~result~ will be undefined too.

*/
int intersection_LLL(Word* args, Word& result, int message, Word& local,
  Supplier s) {

 result = qp->ResultStorage(s);
 Line2* l1 = static_cast<Line2*>(args[0].addr);
 Line2* l2 = static_cast<Line2*>(args[1].addr);
 Line2* res = static_cast<Line2*>(result.addr);
 l1->intersection(*l2, *res);

 return 0;

}

/*
 ~minus\_LLL~

 ~line2~ x ~line2~ [->] ~line2~

 ~result~ contains all segments of the first argument, unless
 they are part of the second argument
 If one of the ~line2~-objects is not defined, ~result~ will be undefined too.

*/
int minus_LLL(Word* args, Word& result, int message, Word& local, Supplier s) {

 result = qp->ResultStorage(s);
 Line2* l1 = static_cast<Line2*>(args[0].addr);
 Line2* l2 = static_cast<Line2*>(args[1].addr);
 Line2* res = static_cast<Line2*>(result.addr);
 l1->minus(*l2, *res);

 return 0;

}

/*
 ~intersects\_LLB~

 ~line2~ x ~line2~ [->] ~bool~

 ~result~ is true, if 2 segments of both arguments intersect, false otherwise.

*/
int intersects_LLB(Word* args, Word& result, int message, Word& local,
  Supplier s) {

 Line2* l1 = static_cast<Line2*>(args[0].addr);
 Line2* l2 = static_cast<Line2*>(args[1].addr);

 result = qp->ResultStorage(s);
 CcBool* b = static_cast<CcBool*>(result.addr);

 bool defined = (l1->IsDefined() && l2->IsDefined());
 bool res = l1->intersect(*l2);

 b->Set(defined, res);

 return 0;
}

/*
 ~crossings\_LLP~

 ~line2~ x ~line2~ [->] ~points2~

 ~result~ contains all intersection points of the first and the second argument.
 Overlapping parts are not considered.

*/
int crossings_LLP(Word* args, Word& result, int message, Word& local,
  Supplier s) {

 result = qp->ResultStorage(s);
 Line2* l1 = static_cast<Line2*>(args[0].addr);
 Line2* l2 = static_cast<Line2*>(args[1].addr);
 Points2* res = static_cast<Points2*>(result.addr);
 l1->crossings(*l2, *res);

 return 0;

}

/*
 ~union\_RRR~

 ~region2~ x ~region2~ [->] ~region2~

 ~result~ contains the union-set of the first and the second argument.


 If one of the ~region2~-objects is not defined, ~result~ will be undefined too.

*/
int union_RRR(Word* args, Word& result, int message, Word& local, Supplier s) {

 result = qp->ResultStorage(s);
 Region2* r1 = static_cast<Region2*>(args[0].addr);
 Region2* r2 = static_cast<Region2*>(args[1].addr);
 Region2* res = static_cast<Region2*>(result.addr);
 //r1->Union(*r2, *res);
 p2d::SetOp(*r1, *r2, *res, union_op);
 return 0;

}

/*
 ~intersection\_RRR~

 ~region2~ x ~region2~ [->] ~region2~

 ~result~ contains the intersection-set of both ~region2~-objects.
 If one of the ~region2~-objects is not defined, ~result~
 will be undefined too.

*/
int intersection_RRR(Word* args, Word& result, int message, Word& local,
  Supplier s) {

 result = qp->ResultStorage(s);
 Region2* r1 = static_cast<Region2*>(args[0].addr);
 Region2* r2 = static_cast<Region2*>(args[1].addr);
 Region2* res = static_cast<Region2*>(result.addr);
 //r1->Intersection(*r2, *res);
 p2d::SetOp(*r1, *r2, *res, intersection_op);
 return 0;

}

/*
 ~minus\_RRR~

 ~region2~ x ~region2~ [->] ~region2~

 ~result~ contains the face of the first argument, reduced to the face
 of the secondo argument
 If one of the ~line2~-objects is not defined, ~result~ will be
 undefined too.

*/
int minus_RRR(Word* args, Word& result, int message, Word& local, Supplier s) {

 result = qp->ResultStorage(s);
 Region2* r1 = static_cast<Region2*>(args[0].addr);
 Region2* r2 = static_cast<Region2*>(args[1].addr);
 Region2* res = static_cast<Region2*>(result.addr);
 //r1->Minus(*r2, *res);
 p2d::SetOp(*r1, *r2, *res, difference_op);
 return 0;

}

/*
 ~intersects\_RRB~

 ~region2~ x ~region2~ [->] ~bool~

 ~result~ is true, if the 2 given region2-objects intersect, false otherwise.

*/
int intersects_RRB(Word* args, Word& result, int message, Word& local,
  Supplier s) {

 Region2* r1 = static_cast<Region2*>(args[0].addr);
 Region2* r2 = static_cast<Region2*>(args[1].addr);

 result = qp->ResultStorage(s);
 CcBool* b = static_cast<CcBool*>(result.addr);

 bool defined = (r1->IsDefined() && r2->IsDefined());
 bool res = intersects(*r1, *r2, 0);

 b->Set(defined, res);

 return 0;
}

/*
 ~overlaps\_RRB~

 ~region2~ x ~region2~ [->] ~bool~

 ~result~ is true, if the 2 given region2-objects overlap, false otherwise.

*/
int overlaps2_RRB(Word* args, Word& result, int message, Word& local,
  Supplier s) {

 Region2* r1 = static_cast<Region2*>(args[0].addr);
 Region2* r2 = static_cast<Region2*>(args[1].addr);

 result = qp->ResultStorage(s);
 CcBool* b = static_cast<CcBool*>(result.addr);

 bool defined = (r1->IsDefined() && r2->IsDefined());
 bool res = overlaps(*r1, *r2, 0);

 b->Set(defined, res);

 return 0;
}

/*
 ~inside\_RRB~

 ~region2~ x ~region2~ [->] ~bool~

 ~result~ is true, if the first one of the given region2-objects is
 completely inside the second region2-object, false otherwise.

*/
int inside_RRB(Word* args, Word& result, int message, Word& local,
  Supplier s) {
 Region2* r1 = static_cast<Region2*>(args[0].addr);
 Region2* r2 = static_cast<Region2*>(args[1].addr);

 result = qp->ResultStorage(s);
 CcBool* b = static_cast<CcBool*>(result.addr);

 bool defined = (r1->IsDefined() && r2->IsDefined());
 bool res = inside(*r1, *r2, 0);
 b->Set(defined, res);

 return 0;
}

/*
 ~lineTOLine2~

 ~line~ [->] ~line2~

 This function converts a ~line~-object into a ~line2~-object.

*/
int lineToLine2(Word* args, Word& result, int message, Word& local,
  Supplier s) {

 result = qp->ResultStorage(s);
 Line* l1 = static_cast<Line*>(args[0].addr);
 Line2* res = static_cast<Line2*>(result.addr);
 convertLineToLine2(*l1, *res);
 assert(res->IsDefined()&&res->BoundingBox().IsDefined());

 return 0;

}

/*
 ~coarseRegion2~

 ~region2~ [->] ~region~

 This function coarses a ~region2~-object to a ~region~object.

*/
int coarse(Word* args, Word& result, int message, Word& local,
  Supplier s) {

 result = qp->ResultStorage(s);
 Region2* r = static_cast<Region2*>(args[0].addr);
 Region* res = static_cast<Region*>(result.addr);
 p2d::coarseRegion2(*r, *res);

 return 0;

}

int coarse2(Word* args, Word& result, int message, Word& local,
  Supplier s) {

 result = qp->ResultStorage(s);
 Region2* r = static_cast<Region2*>(args[0].addr);
 Region* res = static_cast<Region*>(result.addr);
 p2d::coarseRegion2b(*r, *res);

 return 0;

}

/*
 The next operator are test-operators. They do the same as the analogous
 operators above. But the operator, who should return a lin2-, points2- or
 region2-object, return if the object is defined. The other operator, who
 returns only a value of type bool, don't change. Additionally all following
 operators give some information, like how often decisions could be made
 without using the precise values.

*/

/*
 ~testUnion\_LLB~

*/
int testUnion_LLB(Word* args, Word& result, int message,
  Word& local, Supplier s) {

 result = qp->ResultStorage(s);
 const Line2* l1 = static_cast<Line2*>(args[0].addr);
 const Line2* l2 = static_cast<Line2*>(args[1].addr);
 CcBool* b = static_cast<CcBool*>(result.addr);

 Line2* res = new Line2(0);

 p2d_test::TestStruct t;
 p2d_test::SetOp(*l1, *l2, *res, p2d_test::union_op, t, 0);

 t.noSegmentsIn = (l1->Size()/2) + (l2->Size()/2);
 t.noSegmentsOut = res->Size()/2;
 t.print();

 bool defined = (l1->IsDefined() && l2->IsDefined());
 b->Set(defined, res->IsDefined());

 delete res;
 return 0;
}

/*
 ~testIntersection\_LLB~

*/
int testIntersection_LLB(Word* args, Word& result, int message,
  Word& local, Supplier s) {

 result = qp->ResultStorage(s);
 const Line2* l1 = static_cast<Line2*>(args[0].addr);
 const Line2* l2 = static_cast<Line2*>(args[1].addr);
 CcBool* b = static_cast<CcBool*>(result.addr);

 Line2* res = new Line2(0);

 p2d_test::TestStruct t;
 p2d_test::SetOp(*l1, *l2, *res, p2d_test::intersection_op, t, 0);

 t.noSegmentsIn = (l1->Size()/2) + (l2->Size()/2);
 t.noSegmentsOut = res->Size()/2;
 t.print();

 bool defined = (l1->IsDefined() && l2->IsDefined());
 b->Set(defined, res->IsDefined());

 delete res;
 return 0;

}

/*
 ~testMinus\_LLB~

*/
int testMinus_LLB(Word* args, Word& result, int message,
  Word& local, Supplier s) {

 result = qp->ResultStorage(s);
 Line2* l1 = static_cast<Line2*>(args[0].addr);
 Line2* l2 = static_cast<Line2*>(args[1].addr);
 CcBool* b = static_cast<CcBool*>(result.addr);

 Line2* res = new Line2(0);

 p2d_test::TestStruct t;
 p2d_test::SetOp(*l1, *l2, *res, p2d_test::difference_op, t, 0);

 t.noSegmentsIn = (l1->Size()/2) + (l2->Size()/2);
 t.noSegmentsOut = res->Size()/2;
 t.print();

 bool defined = (l1->IsDefined() && l2->IsDefined());
 b->Set(defined, res->IsDefined());

 delete res;
 return 0;

}

/*
 ~testIntersects\_LLB~

*/
int testIntersects_LLB(Word* args, Word& result, int message, Word& local,
  Supplier s) {

 result = qp->ResultStorage(s);
 Line2* l1 = static_cast<Line2*>(args[0].addr);
 Line2* l2 = static_cast<Line2*>(args[1].addr);
 CcBool* b = static_cast<CcBool*>(result.addr);

 bool defined = (l1->IsDefined() && l2->IsDefined());

 p2d_test::TestStruct t;
 bool res = p2d_test::intersects(*l1, *l2, t);

 t.noSegmentsIn = (l1->Size()/2) + (l2->Size()/2);
 t.print();
 b->Set(defined, res);

 return 0;
}

/*
 ~testUnion\_RRB~

*/
int testUnion_RRB(Word* args, Word& result, int message,
  Word& local, Supplier s) {

 result = qp->ResultStorage(s);
 Region2* r1 = static_cast<Region2*>(args[0].addr);
 Region2* r2 = static_cast<Region2*>(args[1].addr);
 CcBool* b = static_cast<CcBool*>(result.addr);

 Region2* res = new Region2(0);
 p2d_test::TestStruct t;
 p2d_test::SetOp(*r1, *r2, *res, p2d_test::union_op, t);

 t.noSegmentsIn = (r1->Size()/2) + (r2->Size()/2);
 t.noSegmentsOut = res->Size()/2;
 t.print();

  bool defined = (r1->IsDefined() && r2->IsDefined());
  b->Set(defined, res->IsDefined());

  delete res;
 return 0;

}

/*
 ~testIntersection\_RRB~

*/
int testIntersection_RRB(Word* args, Word& result, int message, Word& local,
  Supplier s) {
 result = qp->ResultStorage(s);
 Region2* r1 = static_cast<Region2*>(args[0].addr);
 Region2* r2 = static_cast<Region2*>(args[1].addr);
 result = qp->ResultStorage(s);
 CcBool* b = static_cast<CcBool*>(result.addr);

 Region2* res = new Region2(0);
 p2d_test::TestStruct t;
 //r1->Union(*r2, *res);
 p2d_test::SetOp(*r1, *r2, *res, p2d_test::intersection_op, t);

 t.noSegmentsIn = (r1->Size()/2) + (r2->Size()/2);
 t.noSegmentsOut = res->Size()/2;
 t.print();

  bool defined = (r1->IsDefined() && r2->IsDefined());
  b->Set(defined, res->IsDefined());

  delete res;
 return 0;
}

/*
 ~testMinus\_RRB~

*/
int testMinus_RRB(Word* args, Word& result, int message,
  Word& local, Supplier s) {
 result = qp->ResultStorage(s);
 Region2* r1 = static_cast<Region2*>(args[0].addr);
 Region2* r2 = static_cast<Region2*>(args[1].addr);
 result = qp->ResultStorage(s);
 CcBool* b = static_cast<CcBool*>(result.addr);

 Region2* res = new Region2(0);
 p2d_test::TestStruct t;
 //r1->Union(*r2, *res);
 p2d_test::SetOp(*r1, *r2, *res, p2d_test::difference_op, t);

 t.noSegmentsIn = (r1->Size()/2) + (r2->Size()/2);
 t.noSegmentsOut = res->Size()/2;
 t.print();

  bool defined = (r1->IsDefined() && r2->IsDefined());
  b->Set(defined, res->IsDefined());

  delete res;
 return 0;
}

/*
 ~intersects\_RRB~

 ~region2~ x ~region2~ [->] ~bool~

 ~result~ is true, if the 2 given region2-objects intersect, false otherwise.

*/
int testIntersects_RRB(Word* args, Word& result, int message, Word& local,
  Supplier s) {

 Region2* r1 = static_cast<Region2*>(args[0].addr);
 Region2* r2 = static_cast<Region2*>(args[1].addr);

 result = qp->ResultStorage(s);
 CcBool* b = static_cast<CcBool*>(result.addr);

 p2d_test::TestStruct t;

 bool defined = (r1->IsDefined() && r2->IsDefined());
 bool res = p2d_test::intersects(*r1, *r2, t);

 t.noSegmentsIn = (r1->Size()/2) + (r2->Size()/2);
 t.print();
 b->Set(defined, res);

 return 0;
}


/*
 4 Type-mapping-function

*/

/*
 4.1 ~LLL\_TypeMap~

 Signature is
 ~line2~ x ~line2~ [->] ~line2~

*/
ListExpr LLL_TypeMap(ListExpr args) {
 string err = "line2 x line2 expected.";
 if (nl->ListLength(args) != 2) {
  return listutils::typeError(err + ": wrong number of arguments");
 }
 ListExpr arg1 = nl->First(args);
 ListExpr arg2 = nl->Second(args);
 if (!listutils::isSymbol(arg1)) {
  return listutils::typeError(err + ": first arg not a spatial type");
 }
 if (!listutils::isSymbol(arg2)) {
  return listutils::typeError(err + ": second arg not a spatial type");
 }
 string a1 = nl->SymbolValue(arg1);
 string a2 = nl->SymbolValue(arg2);

 if (a1 == Line2::BasicType() && a2 == Line2::BasicType()) {
  return nl->SymbolAtom(Line2::BasicType());
 }

 return listutils::typeError(err);
}


/*
 4.2 ~LLB\_TypeMap~

 Signature is ~line2~ x ~line2~ [->] bool

*/
ListExpr LLB_TypeMap(ListExpr args) {
 string err = "line2 x line2 expected.";
 if (nl->ListLength(args) != 2) {
  return listutils::typeError(err + ": wrong number of arguments");
 }
 ListExpr arg1 = nl->First(args);
 ListExpr arg2 = nl->Second(args);
 if (!listutils::isSymbol(arg1)) {
  return listutils::typeError(err + ": first arg not a spatial type");
 }
 if (!listutils::isSymbol(arg2)) {
  return listutils::typeError(err + ": second arg not a spatial type");
 }
 string a1 = nl->SymbolValue(arg1);
 string a2 = nl->SymbolValue(arg2);

 if (a1 == Line2::BasicType() && a2 == Line2::BasicType()) {
  return NList(CcBool::BasicType()).listExpr();
 }
 return listutils::typeError(err);
}

/*
 4.3 ~LLP\_TypeMap~

 Signature is line2 x line2 [->] points2

*/
ListExpr LLP_TypeMap(ListExpr args) {
 string err = "line2 x line2 expected.";
 if (nl->ListLength(args) != 2) {
  return listutils::typeError(err + ": wrong number of arguments");
 }
 ListExpr arg1 = nl->First(args);
 ListExpr arg2 = nl->Second(args);
 if (!listutils::isSymbol(arg1)) {
  return listutils::typeError(err + ": first arg not a spatial type");
 }
 if (!listutils::isSymbol(arg2)) {
  return listutils::typeError(err + ": second arg not a spatial type");
 }
 string a1 = nl->SymbolValue(arg1);
 string a2 = nl->SymbolValue(arg2);

 if (a1 == Line2::BasicType() && a2 == Line2::BasicType()) {
  return nl->SymbolAtom(Points2::BasicType());
 }
 return listutils::typeError(err);
}

/*
 4.4 ~LL2\_TypeMap~

 Signature is line2 x line2 [->] line2

*/
ListExpr LL2_TypeMap(ListExpr args) {
 string err = "line expected.";
 if (nl->ListLength(args) != 1) {
  return listutils::typeError(err + ": wrong number of arguments");
 }
 ListExpr arg1 = nl->First(args);
 if (!listutils::isSymbol(arg1)) {
  return listutils::typeError(err + ": first arg not a spatial type");
 }

 string a1 = nl->SymbolValue(arg1);

 if (a1 == Line::BasicType()) {
  return nl->SymbolAtom(Line2::BasicType());
 }

 return listutils::typeError(err);
}

/*
 4.5 ~RRR\_TypeMap~

 Signature is
 ~region2~ x ~region2~ [->] ~region2~

*/
ListExpr RRR_TypeMap(ListExpr args) {
 string err = "region2 x region2 expected. ";
 if (nl->ListLength(args) != 2) {
  return listutils::typeError(err + ": wrong number of arguments");
 }
 ListExpr arg1 = nl->First(args);
 ListExpr arg2 = nl->Second(args);
 if (!listutils::isSymbol(arg1)) {
  return listutils::typeError(err + ": first arg not a spatial type");
 }
 if (!listutils::isSymbol(arg2)) {
  return listutils::typeError(err + ": second arg not a spatial type");
 }
 string a1 = nl->SymbolValue(arg1);
 string a2 = nl->SymbolValue(arg2);

 if (a1 == Region2::BasicType() && a2 == Region2::BasicType()) {
  return nl->SymbolAtom(Region2::BasicType());
 }
 return listutils::typeError(err);
}

/*
 4.6 ~RRB\_TypeMap~

 Signature is ~region2~ x ~region2~ [->] bool

*/
ListExpr RRB_TypeMap(ListExpr args) {
 string err = "region2 x region2 expected.";
 if (nl->ListLength(args) != 2) {
  return listutils::typeError(err + ": wrong number of arguments");
 }
 ListExpr arg1 = nl->First(args);
 ListExpr arg2 = nl->Second(args);
 if (!listutils::isSymbol(arg1)) {
  return listutils::typeError(err + ": first arg not a spatial type");
 }
 if (!listutils::isSymbol(arg2)) {
  return listutils::typeError(err + ": second arg not a spatial type");
 }
 string a1 = nl->SymbolValue(arg1);
 string a2 = nl->SymbolValue(arg2);

 if (a1 == Region2::BasicType() && a2 == Region2::BasicType()) {
  return NList(CcBool::BasicType()).listExpr();
 }
 return listutils::typeError(err);
}

/*
 4.7 ~R2R\_TypeMap~

 Signature is ~region2~ [->] region

*/
ListExpr R2R_TypeMap(ListExpr args) {
 string err = "line expected.";
 if (nl->ListLength(args) != 1) {
  return listutils::typeError(err + ": wrong number of arguments");
 }
 ListExpr arg1 = nl->First(args);
 if (!listutils::isSymbol(arg1)) {
  return listutils::typeError(err + ": first arg not a spatial type");
 }

 string a1 = nl->SymbolValue(arg1);

 if (a1 == Region2::BasicType()) {
  return nl->SymbolAtom(Region::BasicType());
 }

 return listutils::typeError(err);
}

/*
 5 operator information

*/

/*
 5.1 ~union\_LLLInfo~

 The operator information for union of 2 line2-objects

*/
struct union_LLLInfo: OperatorInfo {

 union_LLLInfo() :
   OperatorInfo() {
  name = "union";
  signature = Line2::BasicType() + " x " + Line2::BasicType() + " -> "
    + Line2::BasicType();
  syntax = "query arg1 union arg2";
  meaning = "union of two line2 objects";
 }

};

/*
 5.2 ~intersection\_LLLInfo~

 The operator information for intersection of 2 line2-objects

*/
struct intersection_LLLInfo: OperatorInfo {

 intersection_LLLInfo() :
   OperatorInfo() {
  name = "intersection";
  signature = Line2::BasicType() + " x " + Line2::BasicType() + " -> "
    + Line2::BasicType();
  syntax = "query intersection (arg1, arg2)";
  meaning = "intersection of two line2 objects";
 }

};

/*
 5.3 ~minus\_LLLInfo~

 The operator information for the difference of 2 line2-objects

*/
struct minus_LLLInfo: OperatorInfo {

 minus_LLLInfo() :
   OperatorInfo() {
  name = "minus";
  signature = Line2::BasicType() + " x " + Line2::BasicType() + " -> "
    + Line2::BasicType();
  syntax = "query arg1 minus arg2";
  meaning = "difference of two line2 objects";
 }

};

/*
 5.4 ~intersects\_LLBInfo~

 The operator information for the test if 2 line2-objects intersect

*/
struct intersects_LLBInfo: OperatorInfo {

 intersects_LLBInfo() :
   OperatorInfo() {
  name = "intersects";
  signature = Line2::BasicType() + " x " + Line2::BasicType() + " -> bool";
  syntax = "query arg1 intersects arg2";
  meaning = "returns true, if both line2 "
    "objects intersect, false otherwise.";
 }

};

/*
 5.5 ~crossings\_LLPInfo~

*/
struct crossings_LLPInfo: OperatorInfo {

 crossings_LLPInfo() :
   OperatorInfo() {
  name = "crossings";
  signature = Line2::BasicType() + " x "
    + Line2::BasicType() + " -> "+ Points2::BasicType();
  syntax = "query crossings(arg1, arg2)";
  meaning = "intersection-points of two line2-objects ";
 }

};


/*
 5.6 ~union\_RRRInfo~

 The operator information for union of 2 region2-objects

*/
struct union_RRRInfo: OperatorInfo {

 union_RRRInfo() :
   OperatorInfo() {
  name = "union";
  signature = Region2::BasicType() + " x " + Region2::BasicType() + " -> "
    + Region2::BasicType();
  syntax = "query arg1 union arg2";
  meaning = "union of two region2 objects";
 }

};

/*
 5.7 ~intersection\_RRRInfo~

 The operator information for intersection of 2 region2-objects

*/
struct intersection_RRRInfo: OperatorInfo {

 intersection_RRRInfo() :
   OperatorInfo() {
  name = "intersection";
  signature = Region2::BasicType() + " x " + Region2::BasicType() + " -> "
    + Region2::BasicType();
  syntax = "query intersection (arg1, arg2)";
  meaning = "intersection of two region2 objects";
 }

};

/*
 5.8 ~minus\_RRRInfo~

 The operator information for the difference of 2 region2-objects

*/
struct minus_RRRInfo: OperatorInfo {

 minus_RRRInfo() :
   OperatorInfo() {
  name = "minus";
  signature = Region2::BasicType() + " x " + Region2::BasicType() + " -> "
    + Region2::BasicType();
  syntax = "query arg1 minus arg2";
  meaning = "difference of two region2 objects";
 }

};

/*
 5.9 ~lineToLine2Info~

 The operator information for union of 2 line2-objects

*/
struct lineToLine2Info: OperatorInfo {

 lineToLine2Info() :
   OperatorInfo() {
  name = "lineToLine2";
  signature = Line::BasicType() + " -> " + Line2::BasicType();
  syntax = "query lineToLine2(arg)";
  meaning = "converts a line-object into a line2-object";
 }

};

/*
 5.10 ~intersects\_RRBInfo~

 The operator information for the test if 2 region2-objects intersect

*/
struct intersects2_RRBInfo: OperatorInfo {

 intersects2_RRBInfo() :
   OperatorInfo() {
  name = "intersects2";
  signature = Region2::BasicType() + " x " + Region2::BasicType() + " -> bool";
  syntax = "query arg1 intersects2 arg2";
  meaning = "returns true, if both region2 "
    "objects intersect, false otherwise.";
 }

};

/*
 5.11 ~overlaps\_RRBInfo~

 The operator information for the test if 2 region2-objects intersect

*/
struct overlaps2_RRBInfo: OperatorInfo {

 overlaps2_RRBInfo() :
   OperatorInfo() {
  name = "overlaps2";
  signature = Region2::BasicType() + " x " + Region2::BasicType() + " -> bool";
  syntax = "query arg1 overlaps2 arg2";
  meaning = "returns true, if both region2 "
    "objects overlap, false otherwise.";
 }

};

/*
 5.12 ~inside\_RRBInfo~

 The operator information for the test whether a region2-object is completely
 contained in a second region2-object.

*/
struct inside2_RRBInfo: OperatorInfo {

 inside2_RRBInfo() :
   OperatorInfo() {
  name = "inside2";
  signature = Region2::BasicType() + " x " + Region2::BasicType() + " -> bool";
  syntax = "query arg1 inside2 arg2";
  meaning = "returns true, if the first region2- "
    "object is completely contained in the second region2-object, false "
    "otherwise.";
 }

};

/*
 5.13 ~coarseRegion2Info~

 The operator a ~region2~-object to a ~region~-object

*/
struct coarseInfo: OperatorInfo {

 coarseInfo() :
   OperatorInfo() {
  name = "coarse";
  signature = Region2::BasicType() + " -> " + Region::BasicType();
  syntax = "query coarse (arg)";
  meaning = "coarses a region2-object to a region-object";
 }

};

/*
 5.14 ~coarseRegion2Info~

 The operator a ~region2~-object to a ~region~-object

*/
struct coarse2Info: OperatorInfo {

 coarse2Info() :
   OperatorInfo() {
  name = "coarse2";
  signature = Region2::BasicType() + " -> " + Region::BasicType();
  syntax = "query coarse2 (arg)";
  meaning = "coarses a region2-object to a region-object. The difference"
     "between coarse and coarse2 is that the intermediate results"
     "of coarse2 are of type region2. coarse2 might last a little bit longer"
     "but it uses only the set operation of region2.";
 }

};


/*
 The operator information for the test-operators

*/
struct testUnion_LLBInfo: OperatorInfo {

 testUnion_LLBInfo() :
   OperatorInfo() {
  name = "testUnion";
  signature = Line2::BasicType() + " x " + Line2::BasicType() + " -> bool";
  syntax = "query arg1 testUnion arg2";
  meaning = "only for tests";
 }

};

struct testIntersection_LLBInfo: OperatorInfo {

 testIntersection_LLBInfo() :
   OperatorInfo() {
  name = "testIntersection";
  signature = Line2::BasicType() + " x " + Line2::BasicType() + " -> bool";
  syntax = "query testIntersection (arg1, arg2)";
  meaning = "only for tests";
 }

};

struct testMinus_LLBInfo: OperatorInfo {

 testMinus_LLBInfo() :
   OperatorInfo() {
  name = "testMinus";
  signature = Line2::BasicType() + " x " + Line2::BasicType() + " -> bool";
  syntax = "query arg1 testMinus arg2";
  meaning = "only for tests";
 }

};

struct testIntersects_LLBInfo: OperatorInfo {

 testIntersects_LLBInfo() :
   OperatorInfo() {
  name = "testIntersects";
  signature = Line2::BasicType() + " x " + Line2::BasicType() + " -> bool";
  syntax = "query arg1 testIntersects arg2";
  meaning = "only for tests";
 }

};

struct testUnion_RRBInfo: OperatorInfo {

 testUnion_RRBInfo() :
   OperatorInfo() {
  name = "testUnion";
  signature = Region2::BasicType() + " x " + Region2::BasicType()
  + " -> bool";
  syntax = "query arg1 testUnion arg2";
  meaning = "only for tests";
 }

};

struct testIntersection_RRBInfo: OperatorInfo {

 testIntersection_RRBInfo() :
   OperatorInfo() {
  name = "testIntersection";
  signature = Region2::BasicType() + " x " + Region2::BasicType()
  + " -> bool";
  syntax = "query testIntersection (arg1, arg2)";
  meaning = "only for tests";
 }

};

struct testMinus_RRBInfo: OperatorInfo {

 testMinus_RRBInfo() :
   OperatorInfo() {
  name = "testMinus";
  signature = Region2::BasicType() + " x " + Region2::BasicType() + " -> bool";
  syntax = "query arg1 testMinus arg2";
  meaning = "only for tests";
 }

};

struct testIntersects2_RRBInfo: OperatorInfo {

 testIntersects2_RRBInfo() :
   OperatorInfo() {
  name = "testIntersects2";
  signature = Region2::BasicType() + " x " + Region2::BasicType() + " -> bool";
  syntax = "query arg1 testIntersects2 arg2";
  meaning = "only for tests";
 }

};

/*
 4.12 Creation of the type constructor instance

*/
TypeConstructor point(Point2::BasicType(), Point2::Point2Property,
  Point2::OutPoint2, Point2::InPoint2, 0, 0, Point2::CreatePoint2,
  Point2::DeletePoint2, OpenAttribute<Point2>, SaveAttribute<Point2>,
  Point2::ClosePoint2, Point2::ClonePoint2, Point2::CastPoint2,
  Point2::SizeOfPoint2, Point2::CheckPoint2);

TypeConstructor line(Line2::BasicType(), Line2::Line2Property, Line2::OutLine2,
  Line2::InLine2, 0, 0, Line2::CreateLine2, Line2::DeleteLine2,
  OpenAttribute<Line2>, SaveAttribute<Line2>, Line2::CloseLine2,
  Line2::CloneLine2, Line2::CastLine2, Line2::SizeOfLine2, Line2::CheckLine2);

TypeConstructor points(Points2::BasicType(), Points2::Points2Property,
  Points2::OutPoints2, Points2::InPoints2, 0, 0, Points2::CreatePoints2,
  Points2::DeletePoints2, OpenAttribute<Points2>, SaveAttribute<Points2>,
  Points2::ClosePoints2, Points2::ClonePoints2, Points2::CastPoints2,
  Points2::SizeOfPoints2, Points2::CheckPoints2);


} // end of namespace p2d

class Precise2DAlgebra: public Algebra {
public:
 Precise2DAlgebra() :
   Algebra() {
  AddTypeConstructor(&p2d::point);
  AddTypeConstructor(&p2d::line);
  AddTypeConstructor(&p2d::points);

  p2d::point.AssociateKind(Kind::DATA());
  p2d::point.AssociateKind(Kind::SPATIAL2D());

  p2d::line.AssociateKind(Kind::DATA());
  p2d::line.AssociateKind(Kind::SPATIAL2D());

  p2d::points.AssociateKind(Kind::DATA());
  p2d::points.AssociateKind(Kind::SPATIAL2D());


  AddOperator(p2d::union_LLLInfo(), p2d::union_LLL, p2d::LLL_TypeMap);

  AddOperator(p2d::intersection_LLLInfo(), p2d::intersection_LLL,
    p2d::LLL_TypeMap);

  AddOperator(p2d::minus_LLLInfo(), p2d::minus_LLL, p2d::LLL_TypeMap);

  AddOperator(p2d::intersects_LLBInfo(), p2d::intersects_LLB, p2d::LLB_TypeMap);

  AddOperator(p2d::lineToLine2Info(), p2d::lineToLine2, p2d::LL2_TypeMap);

  AddOperator(p2d::crossings_LLPInfo(), p2d::crossings_LLP, p2d::LLP_TypeMap);

  AddOperator(p2d::union_RRRInfo(), p2d::union_RRR, p2d::RRR_TypeMap);

  AddOperator(p2d::intersection_RRRInfo(), p2d::intersection_RRR,
    p2d::RRR_TypeMap);

  AddOperator(p2d::minus_RRRInfo(), p2d::minus_RRR, p2d::RRR_TypeMap);

  AddOperator(p2d::intersects2_RRBInfo(), p2d::intersects_RRB,
    p2d::RRB_TypeMap);

  AddOperator(p2d::overlaps2_RRBInfo(), p2d::overlaps2_RRB, p2d::RRB_TypeMap);

  AddOperator(p2d::inside2_RRBInfo(), p2d::inside_RRB, p2d::RRB_TypeMap);

  AddOperator(p2d::coarseInfo(), p2d::coarse, p2d::R2R_TypeMap);

  AddOperator(p2d::coarse2Info(), p2d::coarse2, p2d::R2R_TypeMap);

  AddOperator(p2d::testUnion_LLBInfo(), p2d::testUnion_LLB, p2d::LLB_TypeMap);

  AddOperator(p2d::testIntersection_LLBInfo(), p2d::testIntersection_LLB,
    p2d::LLB_TypeMap);

  AddOperator(p2d::testMinus_LLBInfo(), p2d::testMinus_LLB, p2d::LLB_TypeMap);

  AddOperator(p2d::testIntersects_LLBInfo(), p2d::testIntersects_LLB,
    p2d::LLB_TypeMap);

  AddOperator(p2d::testUnion_RRBInfo(), p2d::testUnion_RRB, p2d::RRB_TypeMap);

  AddOperator(p2d::testIntersection_RRBInfo(), p2d::testIntersection_RRB,
    p2d::RRB_TypeMap);

  AddOperator(p2d::testMinus_RRBInfo(), p2d::testMinus_RRB, p2d::RRB_TypeMap);

  AddOperator(p2d::testIntersects2_RRBInfo(), p2d::testIntersects_RRB,
    p2d::RRB_TypeMap);

 }

 ~Precise2DAlgebra() {
 }

};

extern "C" Algebra*
InitializePrecise2DAlgebra(NestedList* nlRef, QueryProcessor* qpRef) {
 nl = nlRef;
 qp = qpRef;
 return (new Precise2DAlgebra());
}

