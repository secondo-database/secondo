/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 CreateRectangles Operators

The ~createRectangles1D/2D/3D~ operators provide a stream of tuples with a
single attribute "Bbox" of type Rectangle with the respective dimension.
All rectangles are inside the interval CREATE\_RECTANGLES\_RANGE\_MIN..\_MAX
given below.

Rather than creating completely random rectangles, the canvas rectangle is
recursively narrowed by a factor in [shrinkMin .. shrinkMax] to create local
clusters of rectangles. Parameters:

----
createRectangle2D(int base, int exp, real shrinkMin, real shrinkMax,
int rndSeed)

----

The parameters are

  * *base*: the number of rectangles created in each recursion

  * *exp*: the recursive depth. For instance, base = 10, exp = 3 will create
    1.000 rectangles:

    * in depth 0, 10 large areas are defined on the canvas;

    * in depth 1, 10 medium areas are defined within each large area;

    * in depth 2, 10 small rectangles are drawn within each medium area.

    exp = 1 (no recursion) can be used to avoid clusters, i.e. to get a
    simple random stream of (tuples with) rectangles

  * *shrinkMin*: the minimum factor by which a given area shrinks to the area
    of the next recursion

  * *shrinkMax*: the maximum factor by which a given area shrinks to the area
    of the next recursion. If shrinkMin == shrinkMax, this exact factor will
    always be used, and all resulting rectangles will have equal size.
    The more shrinkMin differs from shrinkMax, the more different the resulting
    rectangles will be in size.

  * *rndSeed*: 0 to get non-reproducible random rectangles; a positive value to
    get reproducible results. Note that two streams with the exact same
    parameters and equal, positive rndSeed values will provide the exact
    same rectangles in the same order, even if the stream is re-opened.

To use ~createRectangle2D~ for testing the ~CDACSpatialJoin~ operator, a query
may look like this:

----
query createRectangles2D(10, 2, 0.3, 0.4, 1) {a}
      createRectangles2D(10, 2, 0.3, 0.4, 2) {b}
      cdacspatialjoin[Bbox_a, Bbox_b] count;

----


1.1 Headers

*/

#pragma once

#include <memory>
#include <stack>
#include <string>
#include <iostream>
#include <sstream>

#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "Algebras/Stream/Stream.h"
#include "Operator.h"

namespace cdacspatialjoin {

/*
1.2 Constants

*/
constexpr double CREATE_RECTANGLES_RANGE_MIN = 0.0;
constexpr double CREATE_RECTANGLES_RANGE_MAX = 1000.0;

/*
1.3 CreateRectangles operator class

*/
template<unsigned dim>
class CreateRectangles {
   static ListExpr TypeMapping(ListExpr args);

   static int ValueMapping(Word *args, Word &result, int message,
                           Word &local, Supplier s);

   std::string getOperatorSpec();

public:
   explicit CreateRectangles() = default;

   ~CreateRectangles() = default;

   std::shared_ptr<Operator> getOperator();
};

/*
1.4 CreateRectanglesElem class

The ~CreateRectanglesElem~ class represents an area at a given recursion of
rectangle creation. Instances of this class are saved on a stack to simulate
the call stack of a recursive implementation (which is not possible here since
one tuple is requested at a time).

*/
template<unsigned dim>
struct CreateRectanglesElem {
   /* the area in which rectangles (or sub-areas) can be created */
   const Rectangle<dim> rect;
   /* the minimum extent (in each dimension) for rectangles or sub-areas */
   double extentMin;
   /* the maximum extent (in each dimension) for rectangles or sub-areas */
   double extentMax;
   /* the iterator which runs from 0 to the (base) parameter */
   unsigned i;

   CreateRectanglesElem(Rectangle<dim> rect_,
           double shrinkMin_, double shrinkMax_);

   ~CreateRectanglesElem() = default;
};

/*
1.5 CreateRectanglesLocalInfo class

The CreateRectanglesLocalInfo class is instantiated with the parameters
passed to the createRectangleXD operator. Its ~getNext()~ function returns
the next Tuple of the stream, or nullptr if the stream is completed.

*/
template<unsigned dim>
class CreateRectanglesLocalInfo {
   /* the number of rectangles created in each recursion */
   const unsigned base;
   /* the recursive depth */
   const unsigned exp;
   /* the minimum factor by which a given area shrinks to the area
    of the next recursion */
   const double shrinkMin;
   /* the maximum factor by which a given area shrinks to the area
    of the next recursion */
   const double shrinkMax;
   /* the tuple type of the tuples to be created by the stream */
   const ListExpr resultType;

   /* the standard mersenne twister engine */
   std::mt19937 rndGenerator;
   /* the random distribution between 0.0 and 1.0 */
   std::uniform_real_distribution<double> distribution;
   /* the stack to simulate a call stack */
   std::stack<CreateRectanglesElem<dim>> elemStack;

public:
   CreateRectanglesLocalInfo(unsigned base_, unsigned exp_,
           double shrinkMin_, double shrinkMax_, unsigned long rndSeed_,
           ListExpr resultType_);

   ~CreateRectanglesLocalInfo() = default;

   /* returns the next Tuple of the stream, or nullptr if the stream is
    * completed */
   Tuple* getNext();

private:

   /* returns a random number in the given interval, using uniform real
    * distribution */
   double getRnd(double min, double max);
};

// ========================================================

/*
2 CreateRectangles implementation

2.1 CreateRectangles operator class

2.1.1 Type Mapping

*/
template<unsigned dim>
ListExpr CreateRectangles<dim>::TypeMapping(ListExpr args) {
   std::stringstream st;
   st << "createRectangles" << dim << "D";
   st << "(int base, int exp, real shrinkMin, real shrinkMax, int rndSeed) ";
   st << "expected";
   std::string err = st.str();

   const int expectedArgsLength = 5;
   int argsLength = nl->ListLength(args);
   if (argsLength != expectedArgsLength) {
      std::stringstream st2;
      st2 << "wrong number of arguments: got " << argsLength
         << " but expected " << expectedArgsLength << ": " << err;
      return listutils::typeError(st2.str());
   }

   ListExpr rest = args;
   std::stringstream wrongArgTypes;
   wrongArgTypes << "wrong argument type in argument ";
   size_t wrongCount = 0;
   for (int i = 1; i <= expectedArgsLength; ++i) {
      ListExpr arg = nl->First(rest);
      bool expectedInt = (i <= 2 || i >= 5);
      bool expectedReal = !expectedInt;

      if ((   expectedInt  && !CcInt::checkType(arg))
          || (expectedReal && !CcReal::checkType(arg))) {
         if (wrongCount > 0)
            wrongArgTypes << ", ";
         ++wrongCount;
         std::string typeName = expectedInt ? "int" : "real";
         wrongArgTypes << i << " ( " << typeName << " expected)";
      }
      rest = nl->Rest(rest);
   }
   if (wrongCount > 0) {
      wrongArgTypes << ": " << err;
      return listutils::typeError(wrongArgTypes.str());
   }

   // (Bbox Rectangle<dim>)
   const ListExpr bboxAttr = nl->TwoElemList(
           nl->SymbolAtom("Bbox"),
           nl->SymbolAtom(Rectangle<dim>::BasicType()));

   // ((Bbox Rectangle<dim>))
   const ListExpr attrList = nl->OneElemList(bboxAttr);

   // (tuple ((Bbox Rectangle<dim>)))
   const ListExpr tupleExpr = nl->TwoElemList(
           nl->SymbolAtom(Tuple::BasicType()), attrList);

   // (stream (tuple ((Bbox Rectangle<dim>))))
   const ListExpr streamExpr = nl->TwoElemList(
           nl->SymbolAtom(Symbol::STREAM()), tupleExpr);

   // If in Value Mapping, the rndSeed parameter is 0, a stream of
   // non-reproducible random rectangles should be created; however, if this
   // same stream is opened twice or several times (as the "inner loop" of two
   // large streams used by CDACSpatialJoin), we need to ensure that the same
   // rectangles will be returned as the first time. Therefore, we determine a
   // random value that will be passed to Value Mapping and remains fixed
   // during the whole lifetime of this operator instance (we cannot store
   // this value in the LocalInfo class since that will be deleted at every
   // close call). If the caller sets rndSeed to a non-zero value, the
   // fixedRndSeed will be ignored
   std::random_device rd;
   unsigned long fixedRndSeed = rd();

   // use the append mechanism to return fixedRndSeed and the stream type
   return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
           nl->OneElemList(nl->IntAtom(fixedRndSeed)), streamExpr);
}

/*
2.1.2 Value Mapping

*/
template<unsigned dim>
int CreateRectangles<dim>::ValueMapping(Word* args, Word& result, int message,
                                   Word& local, Supplier s) {

   auto li = static_cast<CreateRectanglesLocalInfo<dim>*>(local.addr);
   switch (message) {
      case OPEN: {
         ListExpr resultType = GetTupleResultType(s);
         auto base = static_cast<unsigned>(
                 (static_cast<CcInt*>(args[0].addr))->GetValue());
         auto exp = static_cast<unsigned>(
                 (static_cast<CcInt*>(args[1].addr))->GetValue());
         auto shrinkMin = (static_cast<CcReal*>(args[2].addr))->GetValue();
         auto shrinkMax = (static_cast<CcReal*>(args[3].addr))->GetValue();
         auto rndSeed = static_cast<unsigned long>(
                 (static_cast<CcInt*>(args[4].addr))->GetValue());
         if (rndSeed == 0) {
            // use the fixedRndSeed created in Type Mapping
            // (cp. the explanation there)
            rndSeed = static_cast<unsigned long>(
                    (static_cast<CcInt*>(args[5].addr))->GetValue());
         }
         local.addr = new CreateRectanglesLocalInfo<dim>(base, exp,
                 shrinkMin, shrinkMax, rndSeed, nl->Second(resultType));
         return 0;
      }
      case REQUEST: {
         result.addr = li ? li->getNext() : nullptr;
         return result.addr ? YIELD : CANCEL;
      }
      case CLOSE: {
         if (li) {
            delete li;
            local.addr = nullptr;
         }
         return 0;
      }
      default:
         assert(false);
   }
   return 0;
}

/*
2.1.3 Operator Specifications

*/
template<unsigned dim>
std::string CreateRectangles<dim>::getOperatorSpec(){
   std::string rectName = "Rectangle<" + std::to_string(dim) + ">";
   std::string opName = "createRectangles" + std::to_string(dim) + "D";
   return OperatorSpec(
           "int x int x double x double x int -> stream(" + rectName + ")",
           opName + "(base, exp, shrinkMin, shrinkMax, rndSeed) ",
           "creates a reproducible stream of base^exp rectangles ",
           "query " + opName + "(10, 4, 0.3, 0.4, 1) count;"
   ).getStr();
}

template<unsigned dim>
std::shared_ptr<Operator> CreateRectangles<dim>::getOperator(){
   std::string opName = "createRectangles" + std::to_string(dim) + "D";
   return std::make_shared<Operator>(opName,
                                     getOperatorSpec(),
                                     &CreateRectangles::ValueMapping,
                                     Operator::SimpleSelect,
                                     &CreateRectangles::TypeMapping);
}

/*
2.2 CreateRectanglesElem class

*/
template<unsigned dim>
CreateRectanglesElem<dim>::CreateRectanglesElem(Rectangle<dim> rect_,
        double shrinkMin_, double shrinkMax_) :
        rect(rect_) {
   double minEdge = rect.MaxD(0) - rect.MinD(0);
   for (unsigned d = 1; d < dim; ++d)
      minEdge = std::min(minEdge, rect.MaxD(d) - rect.MinD(d));
   extentMin = minEdge * shrinkMin_;
   extentMax = minEdge * shrinkMax_;
   i = 0;
}

/*
2.3 CreateRectanglesLocalInfo class

2.3.1 Constructor

*/
template<unsigned dim>
CreateRectanglesLocalInfo<dim>::CreateRectanglesLocalInfo(
        unsigned base_, unsigned exp_, double shrinkMin_, double shrinkMax_,
        unsigned long rndSeed_, ListExpr resultType_) :
        base(base_), exp(exp_),
        shrinkMin(shrinkMin_),
        shrinkMax(shrinkMax_),
        resultType(resultType_),
        rndGenerator(0),
        distribution(0.0, 1.0) {

   rndGenerator.seed(rndSeed_);

   double min[dim];
   double max[dim];
   for (unsigned d = 0; d < dim; ++d) {
      min[d] = CREATE_RECTANGLES_RANGE_MIN;
      max[d] = CREATE_RECTANGLES_RANGE_MAX;
   }
   Rectangle<dim> frame = Rectangle<dim>(true, min, max);
   CreateRectanglesElem<dim> firstElem { frame, shrinkMin, shrinkMax };
   elemStack.push(firstElem);
}

/*
2.3.2 getNext() function

*/
template<unsigned dim>
Tuple* CreateRectanglesLocalInfo<dim>::getNext() {
   if (elemStack.empty())
      return nullptr;

   auto result = new Tuple(resultType);

   do {
      CreateRectanglesElem<dim>& elem = elemStack.top();
      ++elem.i;

      const Rectangle<dim>& frame = elem.rect;
      double min[dim];
      double max[dim];
      for (unsigned d = 0; d < dim; ++d) {
         double extent = getRnd(elem.extentMin, elem.extentMax);
         min[d] = getRnd(frame.MinD(d), frame.MaxD(d) - extent);
         max[d] = min[d] + extent;
      }
      if (elemStack.size() >= exp) {
         result->PutAttribute(0, new Rectangle<dim>(true, min, max));
         break;
      }
      elemStack.push({ Rectangle<dim>(true, min, max), shrinkMin, shrinkMax });
   } while(true);

   // remove completed elements from the stack
   while (!elemStack.empty() && elemStack.top().i >= base)
      elemStack.pop();

   return result;
}

/*
2.3.3 getRnd() function

*/
template<unsigned dim>
double CreateRectanglesLocalInfo<dim>::getRnd(double min, double max) {
   return min + (max - min) * distribution(rndGenerator);
}


template class CreateRectangles<1>;
template class CreateRectangles<2>;
template class CreateRectangles<3>;
template class CreateRectanglesElem<1>;
template class CreateRectanglesElem<2>;
template class CreateRectanglesElem<3>;
template class CreateRectanglesLocalInfo<1>;
template class CreateRectanglesLocalInfo<2>;
template class CreateRectanglesLocalInfo<3>;
} // end namespace

