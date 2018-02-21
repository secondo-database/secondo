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


//[$][\$]
//[_][\_]

*/
#ifndef ALGEBRAS_RELATION_C___OPERATORCONSUME_H_
#define ALGEBRAS_RELATION_C___OPERATORCONSUME_H_

#include "Algebra.h"
#include "Operator.h"
#include "Algebras/Stream/Stream.h"
#include "RelationAlgebra.h"
#include "Algebras/OrderedRelation/OrderedRelationAlgebra.h"

class OperatorConsume
{
public:
/*
5.6 Operator ~consume~

Collects objects from a stream into a relation.

5.6.1 Type mapping function of operator ~consume~

Operator ~consume~ accepts a stream of tuples and returns a relation.


----    (stream  x)                 -> ( rel x)
----

*/

    template<bool isOrel>
    static ListExpr ConsumeTypeMap(ListExpr args)
    {
        std::string expected =
                isOrel ? "(stream(tuple(...)) (sortby_Ids)) expected" :
                        "stream(tuple(...)) expected";
      if(nl->ListLength(args) != (isOrel?2:1)){
        ErrorReporter::ReportError(expected);
        return nl->TypeError();
      }

      ListExpr first = nl->First(args);

      if(!Stream<Tuple>::checkType(first)){
        return listutils::typeError(expected);
      }

      ListExpr attrlist = nl->Second(nl->Second(first));

      if(!listutils::checkAttrListForNamingConventions(attrlist)){
        return listutils::typeError("Some of the attributes does "
                          "not fit to Secondo's naming conventions");
      }

      // do not allow an arel  attribute for standard consume
      while(!nl->IsEmpty(attrlist)){
        ListExpr attr = nl->First(attrlist);
        attrlist = nl->Rest(attrlist);
        ListExpr type = nl->Second(attr);
        if( (nl->ListLength(type)==2) &&
            (nl->IsAtom(nl->First(type))) &&
            ( (nl->SymbolValue(nl->First(type))) == "arel")){
          ErrorReporter::ReportError("arel attributes cannot be processed"
                                      " with standard consume");
          return nl->TypeError();
        }
      }

      if(!isOrel) {
        return nl->Cons(nl->SymbolAtom(Relation::BasicType()), nl->Rest(first));
      } else {
        if(!listutils::isKeyDescription(nl->Second(first),nl->Second(args))) {
          ErrorReporter::ReportError("all identifiers of second argument must "
                                      "appear in the first argument");
          return nl->TypeError();
        }
        ListExpr result = nl->ThreeElemList(nl->SymbolAtom(OREL),
                                            nl->Second(first),
                                            nl->Second(args));
        return result;
      }
    }

    static ListExpr tconsume_tm(ListExpr args);
/*
5.6.2 Value mapping function of operator ~consume~

*/

    static int
    Consume(Word* args, Word& result, int message,
            Word& local, Supplier s);

#ifdef USE_PROGRESS
    static CostEstimation* ConsumeCostEstimationFunc();
#endif

};

#endif /* ALGEBRAS_RELATION_C___OPERATORCONSUME_H_ */
