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

\setcounter{tocdepth}{2}
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
int rndSeed, int lockX = 0, int lockY = 0)

----

The parameters are

  * *base*: the number of rectangles created in each recursion

  * *exp*: the recursive depth. For instance, base = 10, exp = 3 will create
    1.000 rectangles:

    * in depth 0, 10 large areas are defined on the canvas (but not written
      to the stream);

    * in depth 1, 10 medium areas are defined within each large area (but not
      written to the stream);

    * in depth 2, 10 small rectangles are created within each medium area
      and written to the stream (since this is the final depth).

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

  * *lockX* (optional): 0 for normal behavior; 1 for locking minX to the same
    value for all rectangles (while maxX varies); 2 for locking maxX to the
    same value for all rectangles (while minX varies); 3 for locking both
    minX and maxX (i.e. all rectangles will be on top of each other, spanning
    exactly the same x interval)

  * *lockY* (optional): 0 for normal behavior; 1 for locking minY to the same
    value for all rectangles; 2 for locking maxY to the same value for all
    rectangles; 3 for locking both minY and maxY (i.e. all rectangles will be
    next to each other, spanning exactly the same y interval)

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
#include <stack> // std::stack
#include <string>
#include <sstream> // std::stringstream
#include <random>

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
constexpr double CREATE_RECTANGLES_RANGE_MID =
        (CREATE_RECTANGLES_RANGE_MIN + CREATE_RECTANGLES_RANGE_MAX) / 2.0;
enum LOCK_TYPE : unsigned { none = 0, min = 1, max = 2, both = 3 };

/*
1.3 CreateRectangles operator class

*/
template<unsigned dim>
class CreateRectangles {
   static ListExpr TypeMapping(ListExpr args);

   static int ValueMapping(Word *args, Word &result, int message,
                           Word &local, Supplier s);

   std::string getOperatorSpec() const;

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

   CreateRectanglesElem(const Rectangle<dim>& rect_,
           double shrinkMin_, double shrinkMax_, const LOCK_TYPE lock[dim]);

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
   /* the locks used on dimension X/Y/Z: 0 = none, 1 = minimum value locked,
    * 2 = maximum value locked, 3 = both locked */
   LOCK_TYPE locks[dim];

   /* the standard mersenne twister engine */
   std::mt19937 rndGenerator;
   /* the random distribution between 0.0 and 1.0 */
   std::uniform_real_distribution<double> distribution;
   /* the stack to simulate a call stack */
   std::stack<CreateRectanglesElem<dim>> elemStack;

public:
   CreateRectanglesLocalInfo(unsigned base_, unsigned exp_,
           double shrinkMin_, double shrinkMax_, unsigned long rndSeed_,
           const LOCK_TYPE locks_[], ListExpr resultType_);

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
   st << "(int base, int exp, real shrinkMin, real shrinkMax, int rndSeed";
   st << ", int lockX = 0";
   if (dim >= 2) {
      st << ", int lockY = 0";
      if (dim >= 3) {
         st << ", int lockZ = 0";
      }
   }
   st << ") expected";
   const std::string err = st.str();

   const int expectedArgsLengthMin = 5;
   const int expectedArgsLengthMax = expectedArgsLengthMin + dim;
   const int argsLength = nl->ListLength(args);
   if (   argsLength < expectedArgsLengthMin
       || argsLength > expectedArgsLengthMax) {
      std::stringstream st2;
      st2 << "wrong number of arguments: got " << argsLength
         << " but expected " << expectedArgsLengthMin
         << " - " << expectedArgsLengthMax <<   ": " << err;
      return listutils::typeError(st2.str());
   }

   ListExpr rest = args;
   std::stringstream wrongArgTypes;
   wrongArgTypes << "wrong argument type in argument ";
   size_t wrongCount = 0;
   for (int i = 1; i <= argsLength; ++i) {
      ListExpr arg = nl->First(rest);
      const bool expectedInt = (i <= 2 || i >= 5);
      const bool expectedReal = !expectedInt;

      if ((   expectedInt  && !CcInt::checkType(nl->First(arg)))
          || (expectedReal && !CcReal::checkType(nl->First(arg)))) {
         if (wrongCount > 0)
            wrongArgTypes << ", ";
         ++wrongCount;
         const std::string typeName = expectedInt ? "int" : "real";
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
   const unsigned long fixedRndSeed = rd();

   // compile information required by Value Mapping
   // the nl->Empty() element will be omitted below:
   const ListExpr appendInfo = nl->OneElemList(nl->Empty());
   ListExpr appendEnd = appendInfo;

   // check lockX/Y/Z values if provided, otherwise append parameter "0"
   // for each omitted lock value
   rest = args;
   for (int i = 0; i < expectedArgsLengthMax; ++i) {
      if (i < expectedArgsLengthMin) {
         // skip the other parameters
         rest = nl->Rest(rest);
      } else if (i < argsLength) {
         // check provided lock value
         ListExpr arg = nl->First(rest);
         long lock = nl->IntValue(nl->Second(arg));
         if (lock < 0 || lock > 3) {
            return listutils::typeError("expected lock values: 0 = none "
                "(default), 1 = min locked, 2 = max locked, 3 = both locked");
         }
         rest = nl->Rest(rest);
      } else {
         // append default lock value 0
         appendEnd = nl->Append(appendEnd, nl->IntAtom(0));
      }
   }
   // append fixedRndSeed
   appendEnd = nl->Append(appendEnd, nl->IntAtom(fixedRndSeed));


   // use the append mechanism to return appendInfo and stream type
   return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
           nl->Rest(appendInfo), streamExpr);
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
         const auto base = static_cast<unsigned>(
                 (static_cast<CcInt*>(args[0].addr))->GetValue());
         const auto exp = static_cast<unsigned>(
                 (static_cast<CcInt*>(args[1].addr))->GetValue());
         const auto shrinkMin =
                 (static_cast<CcReal*>(args[2].addr))->GetValue();
         const auto shrinkMax =
                 (static_cast<CcReal*>(args[3].addr))->GetValue();
         auto rndSeed = static_cast<unsigned long>(
                 (static_cast<CcInt*>(args[4].addr))->GetValue());
         LOCK_TYPE locks[dim] {};
         for (unsigned d = 0; d < dim; ++d) {
            locks[d] = static_cast<LOCK_TYPE>(
                    (static_cast<CcInt*>(args[5 + d].addr))->GetValue());
         }
         if (rndSeed == 0) {
            // use the fixedRndSeed created in Type Mapping
            // (cp. the explanation there)
            rndSeed = static_cast<unsigned long>(
                    (static_cast<CcInt*>(args[5 + dim].addr))->GetValue());
         }
         local.addr = new CreateRectanglesLocalInfo<dim>(base, exp,
                 shrinkMin, shrinkMax, rndSeed, locks, nl->Second(resultType));
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
std::string CreateRectangles<dim>::getOperatorSpec() const {
   const std::string rectName = "Rectangle<" + std::to_string(dim) + ">";
   const std::string opName = "createRectangles" + std::to_string(dim) + "D";

   std::string optSign = " x int";
   std::string optList = ", lockX = 0";
   if (dim >= 2) {
      optSign += " x int";
      optList += ", lockY = 0";
      if (dim >= 3) {
         optSign += " x int";
         optList += ", lockZ = 0";
      }
   }
   return OperatorSpec(
           "int x int x double x double" + optSign +
              " -> stream(" + rectName + ")",
           opName + "(base, exp, shrinkMin, shrinkMax, rndSeed" +
              optList + ") ",
           "creates a reproducible stream of base^exp rectangles ",
           "query " + opName + "(10, 4, 0.3, 0.4, 1) count;"
   ).getStr();
}

template<unsigned dim>
std::shared_ptr<Operator> CreateRectangles<dim>::getOperator(){
   const std::string opName = "createRectangles" + std::to_string(dim) + "D";
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
CreateRectanglesElem<dim>::CreateRectanglesElem(const Rectangle<dim>& rect_,
        const double shrinkMin_, const double shrinkMax_,
        const LOCK_TYPE lock[dim]) :
        rect(rect_) {
   // get the shortest edge, ignoring edges that are both min and max locked
   double minEdge = std::numeric_limits<double>::max();
   bool found = false;
   for (unsigned d = 0; d < dim; ++d) {
      if (lock[d] != LOCK_TYPE::both) {
         minEdge = std::min(minEdge, rect.MaxD(d) - rect.MinD(d));
         found = true;
      }
   }
   // set minimum and maximum extent. If all dimensions are both min and max
   // locked, these values will never be used
   extentMin = found ? minEdge * shrinkMin_ : 0.0;
   extentMax = found ? minEdge * shrinkMax_ : 0.0;
   i = 0;
}

/*
2.3 CreateRectanglesLocalInfo class

2.3.1 Constructor

*/
template<unsigned dim>
CreateRectanglesLocalInfo<dim>::CreateRectanglesLocalInfo(
        const unsigned base_, const unsigned exp_,
        const double shrinkMin_, const double shrinkMax_,
        const unsigned long rndSeed_, const LOCK_TYPE locks_[],
        const ListExpr resultType_) :
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
      locks[d] = locks_[d];
      if (locks[d] != LOCK_TYPE::both) {
         min[d] = CREATE_RECTANGLES_RANGE_MIN;
         max[d] = CREATE_RECTANGLES_RANGE_MAX;
      } else {
         // if both locks apply (i.e. the min value and the max value should
         // be locked), create a fixed interval for this dimension,
         // using the average expected edge length
         double fac = (shrinkMin + shrinkMax) / 2.0;
         double extent = (CREATE_RECTANGLES_RANGE_MAX -
                CREATE_RECTANGLES_RANGE_MIN) * fac; // use * fac at least once
         for (unsigned i = 1; i < exp; ++i)
            extent *= fac;
         min[d] = CREATE_RECTANGLES_RANGE_MID - extent / 2.0;
         max[d] = CREATE_RECTANGLES_RANGE_MID + extent / 2.0;
      }
   }
   const Rectangle<dim> frame = Rectangle<dim>(true, min, max);
   CreateRectanglesElem<dim> firstElem { frame, shrinkMin, shrinkMax, locks };
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
         double extent = (locks[d] == LOCK_TYPE::both) ? 0.0 :
                 getRnd(elem.extentMin, elem.extentMax);
         switch (locks[d]) {
            case LOCK_TYPE::none:
               min[d] = getRnd(frame.MinD(d), frame.MaxD(d) - extent);
               max[d] = min[d] + extent;
               break;
            case LOCK_TYPE::min:
               min[d] = frame.MinD(d); // min[d] is always the same
               max[d] = min[d] + extent; // max[d] varies
               break;
            case LOCK_TYPE::max:
               max[d] = frame.MaxD(d); // max[d] is always the same
               min[d] = max[d] - extent; // min[d] varies
               break;
            default: // LOCK_TYPE::both
               min[d] = frame.MinD(d); // both min[d] ...
               max[d] = frame.MaxD(d); // ... and max[d] are always the same
               break;
         }
      }
      if (elemStack.size() >= exp) {
         result->PutAttribute(0, new Rectangle<dim>(true, min, max));
         break;
      }
      elemStack.push({ Rectangle<dim>(true, min, max), shrinkMin, shrinkMax,
                       locks });
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

