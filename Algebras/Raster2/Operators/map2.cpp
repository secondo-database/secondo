/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

*/

#include <ListUtils.h>

#include "map2.h"
#include "../sint.h"
#include "../sreal.h"
#include "../sbool.h"
#include "../sstring.h"
#include "../msint.h"
#include "../msreal.h"
#include "../msbool.h"
#include "../msstring.h"
#include "../util/parse_error.h"
#include "../util/types.h"

namespace raster2
{
/*
The following declarations are used by the type mapping to check whether the
grids of the operator arguments are compatible.

*/
  typedef void (*GridChecker)(Word, Word);
  extern GridChecker map2CheckGridsFuns[64];
  int map2SelectCheckGridsFun(const std::string&, const std::string&);

  ListExpr map2TypeMap(ListExpr args) {
    NList types(args);
    std::string map2_result_type;
    NList first;
    NList second;

    try {
      if(types.length() != 3) {
        throw util::parse_error("Operator map2 requires three arguments.");
      }
      first = types.first().first();
      second  = types.second().first();
      NList function = types.third().first();

      std::string first_type = first.convertToString();
      if (!first.isSymbol() ||
          (!util::isMSType(first_type) && !util::isSType(first_type)))
      {
        throw util::parse_error
            ("First argument must be a (moving) spatial type.");
      };

      std::string second_type = second.convertToString();
      if (   !second.isSymbol()
          || (!util::isMSType(second_type) && !util::isSType(second_type)))
      {
        throw util::parse_error
            ("Second argument must be a (moving) spatial type.");
      }

      if (!listutils::isMap<2>(function.listExpr())) {
        throw util::parse_error
            ("Third argument must be a function of two parameters.");
      }

      NList ffirst = function.second();
      NList fsecond  = function.third();
      NList fresult  = function.fourth();

      std::string ffirst_type = ffirst.convertToString();
      if (!ffirst.isSymbol() ||
          util::getValueBasicType(first_type) != ffirst_type)
      {
        throw util::parse_error("Parameter function requires a "  +
            util::getValueBasicType(first_type) + " as first argument.");
      }

      std::string fsecond_type = fsecond.convertToString();
      if (!fsecond.isSymbol() ||
          util::getValueBasicType(second_type) != fsecond_type)
      {
        throw util::parse_error("Parameter function requires a "  +
            util::getValueBasicType(second_type) + " as second argument.");
      }

      std::string fresult_type = fresult.convertToString();
      if (util::isMSType(first_type) || util::isMSType(second_type)) {
        map2_result_type = util::getMovingSpatialBasicType(fresult_type);
      } else {
        map2_result_type = util::getSpatialBasicType(fresult_type);
      }
      if (!fresult.isSymbol() || map2_result_type.empty())
      {
        throw util::parse_error
            ("Parameter function must have int, real, bool or string result.");
      }

      Word result_first;
      Word result_second;
      bool ok = QueryProcessor::ExecuteQuery
          (types.first().second().convertToString(), result_first);
      if (!ok) {
          throw util::parse_error("Argument 1 cannot be evaluated.");
      }
      ok = QueryProcessor::ExecuteQuery
          (types.second().second().convertToString(), result_second);
      if (!ok) {
          throw util::parse_error("Argument 2 cannot be evaluated.");
      }
      int callback_id = map2SelectCheckGridsFun(first_type, second_type);
      GridChecker callback = map2CheckGridsFuns[callback_id];
      // The following call must throw util::parse_error if the grids are not
      // compatible.
      callback(result_first, result_second);

    } catch (util::parse_error& e) {
      return NList::typeError(e.what());
    }

    return NList(map2_result_type).listExpr();
  }

  int map2SelectFun(ListExpr args) {
    int offset = 0;
    int i;

    NList types(args);
    std::string first = types.first().str();
    std::string second = types.second().str();
    std::string result = types.third().fourth().str();

    std::string idf = first.substr(first.length()-2, 2);
    std::string ids = second.substr(second.length()-2, 2);
    std::string idr = result.substr(result.length()-2, 2);

    if (idf == "nt")      i = 0;
    else if (idf == "ol") i = 1;
    else if (idf == "al") i = 2;
    else                  i = 3;
    offset += util::isSType(first) ?  32 * i : 32 * i + 128;

    if (ids == "nt")      i = 0;
    else if (ids == "ol") i = 1;
    else if (ids == "al") i = 2;
    else                  i = 3;
    offset += util::isSType(second) ? 4 * i : 4 * i + 16;

    if (idr == "nt")      i = 0;
    else if (idr == "ol") i = 1;
    else if (idr == "al") i = 2;
    else                  i = 3;
    offset += i;

    return offset;
  }

  template <typename S1, typename S2, typename Result>
  int map2FunSS(Word* args, Word& result, int message, Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);

    S1* s1 = static_cast<S1*>(args[0].addr);
    S2* s2 = static_cast<S2*>(args[1].addr);
    Address function = args[2].addr;
    Result* r = static_cast<Result*>(result.addr);

    grid2 g1 = s1->getGrid();
    grid2 g2 = s2->getGrid();

    double dx = (g1.getOriginX() - g2.getOriginX()) / g1.getLength();
    double dy = (g1.getOriginY() - g2.getOriginY()) / g1.getLength();
    int dc = dx + 0.5;
    int dr = dy + 0.5;
    assert(g1.getLength() == g2.getLength());
    assert(AlmostEqual(dx, dc) && AlmostEqual(dy, dr));

    BBox<2> bb1 = s1->bbox();
    BBox<2> bb2 = s2->bbox();

    BBox<2> bb = bb1.Union(bb2);

    r->setGrid(g1);

    ArgVector& arguments = *qp->Argument(function);
    Word fresult;

    // Determine the first and last region of concern.
    const typename S1::index_type& region_size = S1::riter_type::region_size;
    typename S1::index_type from = g1.getIndex(bb.MinD(0), bb.MinD(1));
    typename S1::index_type to   = g1.getIndex(bb.MaxD(0), bb.MaxD(1));

    // Iterate over regions in the first argument.
    for (typename S1::riter_type rit = s1->iterate_regions(from, to),
                                  re = s1->end_regions();
         rit != re; ++rit)
    {
      typename S1::index_type rfrom = *rit;
      typename S1::index_type rto   = *rit + region_size;

      // Iterate over the cells in the current region.
      for (typename S1::index_type i1 = rfrom, e = rto;
           i1 != e; i1.increment(rfrom, rto))
      {
        // Calculate the corresponding cell in the second argument
        typename S2::index_type i2 = (int[]){i1[0] + dc, i1[1] + dr};
        if (!S1::isUndefined(s1->get(i1)) || !S2::isUndefined(s2->get(i2)))
        {
          typename S1::wrapper_type w1 = S1::wrap(s1->get(i1));
          typename S2::wrapper_type w2 = S2::wrap(s2->get(i2));

          // Apply function
          arguments[0].setAddr(&w1);
          arguments[1].setAddr(&w2);
          qp->Request(function, fresult);

          // Set result
          typename Result::wrapper_type* mapped =
              static_cast<typename Result::wrapper_type*>(fresult.addr);
          r->set(i1, Result::unwrap(*mapped));
        }
      }
    }

    return 0;
  }

  template <typename S, typename M, typename Result>
  int map2FunSM(Word* args, Word& result, int message, Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);

    S* sobj = static_cast<S*>(args[0].addr);
    M* mobj = static_cast<M*>(args[1].addr);
    Address function = args[2].addr;
    Result* r = static_cast<Result*>(result.addr);

    grid2 g1 = sobj->getGrid();
    grid3 g2 = mobj->getGrid();

    double dx = (g1.getOriginX() - g2.getOriginX()) / g1.getLength();
    double dy = (g1.getOriginY() - g2.getOriginY()) / g1.getLength();
    int dc = dx + 0.5;
    int dr = dy + 0.5;
    assert(g1.getLength() == g2.getLength());
    assert(AlmostEqual(dx, dc) && AlmostEqual(dy, dr));

    BBox<2> bbs = sobj->bbox();
    BBox<3> bbm = mobj->bbox();

    BBox<2> bbm_proj(true, bbm.MinD(0), bbm.MaxD(0), bbm.MinD(1), bbm.MaxD(1));
    BBox<2> bb = bbs.Union(bbm_proj);

    r->setGrid(g2);

    ArgVector& arguments = *qp->Argument(function);
    Word fresult;

    // Determine the first and last region of concern.
    const typename M::index_type& region_size = M::riter_type::region_size;
    typename M::index_type from = g2.getIndex(
        bb.MinD(0), bb.MinD(1), bb.MinD(2));
    typename M::index_type to = g2.getIndex(
        bb.MaxD(0), bb.MaxD(1), bb.MaxD(2));

    // Iterate over regions in the moving spatial argument.
    for (typename M::riter_type rit = mobj->iterate_regions(from, to),
                                 re = mobj->end_regions();
         rit != re; ++rit)
    {
      typename M::index_type rfrom = *rit;
      typename M::index_type rto   = *rit + region_size;

      // Iterate over the cells in the current region.
      for (typename M::index_type i1 = rfrom, e = rto;
           i1 != e; i1.increment(rfrom, rto))
      {
        // Calculate the corresponding cell in the second argument
        typename S::index_type i2 = (int[]){i1[0] + dc, i1[1] + dr};
        if (!M::isUndefined(mobj->get(i1)) || !S::isUndefined(sobj->get(i2)))
        {
          typename S::wrapper_type w1 = S::wrap(sobj->get(i2));
          typename M::wrapper_type w2 = M::wrap(mobj->get(i1));

          // Apply function
          arguments[0].setAddr(&w1);
          arguments[1].setAddr(&w2);
          qp->Request(function, fresult);

          // Set result
          typename Result::wrapper_type& mapped =
              *static_cast<typename Result::wrapper_type*>(result.addr);
          r->set(i1, Result::unwrap(mapped));
        }
      }
    }

    return 0;
  }

  template <typename M, typename S, typename Result>
  int map2FunMS(Word* args, Word& result, int message, Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);

    M* mobj = static_cast<M*>(args[0].addr);
    S* sobj = static_cast<S*>(args[1].addr);
    Address function = args[2].addr;
    Result* r = static_cast<Result*>(result.addr);

    grid3 g1 = mobj->getGrid();
    grid2 g2 = sobj->getGrid();

    double dx = (g1.getOriginX() - g2.getOriginX()) / g1.getLength();
    double dy = (g1.getOriginY() - g2.getOriginY()) / g1.getLength();
    int dc = dx + 0.5;
    int dr = dy + 0.5;
    assert(g1.getLength() == g2.getLength());
    assert(AlmostEqual(dx, dc) && AlmostEqual(dy, dr));

    BBox<3> bbm = mobj->bbox();
    BBox<2> bbs = sobj->bbox();

    BBox<2> bbm_proj(true, bbm.MinD(0), bbm.MaxD(0), bbm.MinD(1), bbm.MaxD(1));
    BBox<2> bb = bbs.Union(bbm_proj);

    r->setGrid(g1);

    ArgVector& arguments = *qp->Argument(function);
    Word fresult;

    // Determine the first and last region of concern.
    const typename M::index_type& region_size = M::riter_type::region_size;
    typename M::index_type from = g1.getIndex(
        bb.MinD(0), bb.MinD(1), bb.MinD(2));
    typename M::index_type to = g1.getIndex(
        bb.MaxD(0), bb.MaxD(1), bb.MaxD(2));

    // Iterate over regions in the moving spatial argument.
    for (typename M::riter_type rit = mobj->iterate_regions(from, to),
                                 re = mobj->end_regions();
         rit != re; ++rit)
    {
      typename M::index_type rfrom = *rit;
      typename M::index_type rto   = *rit + region_size;

      // Iterate over the cells in the current region.
      for (typename M::index_type i1 = rfrom, e = rto;
           i1 != e; i1.increment(rfrom, rto))
      {
        // Calculate the corresponding cell in the second argument
        typename S::index_type i2 = (int[]){i1[0] + dc, i1[1] + dr};
        if (!M::isUndefined(mobj->get(i1)) || !S::isUndefined(sobj->get(i2)))
        {
          typename M::wrapper_type w1 = M::wrap(mobj->get(i1));
          typename S::wrapper_type w2 = S::wrap(sobj->get(i2));

          // Apply function
          arguments[0].setAddr(&w1);
          arguments[1].setAddr(&w2);
          qp->Request(function, fresult);

          // Set result
          typename Result::wrapper_type& mapped =
              *static_cast<typename Result::wrapper_type*>(result.addr);
          r->set(i1, Result::unwrap(mapped));
        }
      }
    }

    return 0;
  }

  template <typename M1, typename M2, typename Result>
  int map2FunMM(Word* args, Word& result, int message, Word& local, Supplier s)
  {
    result = qp->ResultStorage(s);

    M1* m1 = static_cast<M1*>(args[0].addr);
    M2* m2 = static_cast<M2*>(args[1].addr);
    Address function = args[2].addr;
    Result* r = static_cast<Result*>(result.addr);

    grid3 g1 = m1->getGrid();
    grid3 g2 = m2->getGrid();

    double dx = (g1.getOriginX() - g2.getOriginX()) / g1.getLength();
    double dy = (g1.getOriginY() - g2.getOriginY()) / g1.getLength();
    int dc = dx + 0.5;
    int dr = dy + 0.5;
    assert(g1.getLength() == g2.getLength());
    assert(AlmostEqual(dx, dc) && AlmostEqual(dy, dr));

    // If one value is undefined, the mapped value is undefined. This means we
    // only need to consider the intersection of the two bounding boxes.
    BBox<3> bb1 = m1->bbox();
    BBox<3> bb2 = m2->bbox();

    BBox<3> bb = bb1.Union(bb2);

    // If the intersection of the bboxes is empty, we are done.
    if (bb.IsDefined()) {
      r->setGrid(g1);

      ArgVector& arguments = *qp->Argument(function);
      Word result;

      // Determine the first and last region of concern.
      const typename M1::index_type& region_size
        = M1::riter_type::region_size;
      typename M1::index_type from
        = g1.getIndex(bb.MinD(0), bb.MinD(1), bb.MinD(2));
      typename M1::index_type to
        = g1.getIndex(bb.MaxD(0), bb.MaxD(1), bb.MaxD(2));

      // Iterate over regions in the first argument.
      for (typename M1::riter_type rit = m1->iterate_regions(from, to),
                                    re = m1->end_regions();
           rit != re; ++rit)
      {
        typename M1::index_type rfrom = *rit;
        typename M1::index_type rto   = *rit + region_size;

        // Iterate over the cells in the current region.
        for (typename M1::index_type i1 = rfrom, e = rto;
             i1 != e; i1.increment(rfrom, rto))
        {
          // Calculate the corresponding cell in the second argument and check
          // whether both have defined values
          typename M2::index_type i2 = (int[]){i1[0] + dc, i1[1] + dr, i1[2]};
          if (!M1::isUndefined(m1->get(i1)) || !M2::isUndefined(m2->get(i2)))
          {
            typename M1::wrapper_type w1 = M1::wrap(m1->get(i1));
            typename M2::wrapper_type w2 = M2::wrap(m2->get(i2));

            arguments[0].setAddr(&w1);
            arguments[1].setAddr(&w2);
            qp->Request(function, result);

            typename Result::wrapper_type& mapped =
                *static_cast<typename Result::wrapper_type*>(result.addr);
            r->set(i1, Result::unwrap(mapped));
          }
        }
      }
    }

    return 0;
  }

  ValueMapping map2Funs[] = {
      map2FunSS<sint, sint, sint>,
      map2FunSS<sint, sint, sbool>,
      map2FunSS<sint, sint, sreal>,
      map2FunSS<sint, sint, sstring>,
      map2FunSS<sint, sbool, sint>,
      map2FunSS<sint, sbool, sbool>,
      map2FunSS<sint, sbool, sreal>,
      map2FunSS<sint, sbool, sstring>,
      map2FunSS<sint, sreal, sint>,
      map2FunSS<sint, sreal, sbool>,
      map2FunSS<sint, sreal, sreal>,
      map2FunSS<sint, sreal, sstring>,
      map2FunSS<sint, sstring, sint>,
      map2FunSS<sint, sstring, sbool>,
      map2FunSS<sint, sstring, sreal>,
      map2FunSS<sint, sstring, sstring>,
      map2FunSM<sint, msint, msint>,
      map2FunSM<sint, msint, msbool>,
      map2FunSM<sint, msint, msreal>,
      map2FunSM<sint, msint, msstring>,
      map2FunSM<sint, msbool, msint>,
      map2FunSM<sint, msbool, msbool>,
      map2FunSM<sint, msbool, msreal>,
      map2FunSM<sint, msbool, msstring>,
      map2FunSM<sint, msreal, msint>,
      map2FunSM<sint, msreal, msbool>,
      map2FunSM<sint, msreal, msreal>,
      map2FunSM<sint, msreal, msstring>,
      map2FunSM<sint, msstring, msint>,
      map2FunSM<sint, msstring, msbool>,
      map2FunSM<sint, msstring, msreal>,
      map2FunSM<sint, msstring, msstring>,

      map2FunSS<sbool, sint, sint>,
      map2FunSS<sbool, sint, sbool>,
      map2FunSS<sbool, sint, sreal>,
      map2FunSS<sbool, sint, sstring>,
      map2FunSS<sbool, sbool, sint>,
      map2FunSS<sbool, sbool, sbool>,
      map2FunSS<sbool, sbool, sreal>,
      map2FunSS<sbool, sbool, sstring>,
      map2FunSS<sbool, sreal, sint>,
      map2FunSS<sbool, sreal, sbool>,
      map2FunSS<sbool, sreal, sreal>,
      map2FunSS<sbool, sreal, sstring>,
      map2FunSS<sbool, sstring, sint>,
      map2FunSS<sbool, sstring, sbool>,
      map2FunSS<sbool, sstring, sreal>,
      map2FunSS<sbool, sstring, sstring>,
      map2FunSM<sbool, msint, msint>,
      map2FunSM<sbool, msint, msbool>,
      map2FunSM<sbool, msint, msreal>,
      map2FunSM<sbool, msint, msstring>,
      map2FunSM<sbool, msbool, msint>,
      map2FunSM<sbool, msbool, msbool>,
      map2FunSM<sbool, msbool, msreal>,
      map2FunSM<sbool, msbool, msstring>,
      map2FunSM<sbool, msreal, msint>,
      map2FunSM<sbool, msreal, msbool>,
      map2FunSM<sbool, msreal, msreal>,
      map2FunSM<sbool, msreal, msstring>,
      map2FunSM<sbool, msstring, msint>,
      map2FunSM<sbool, msstring, msbool>,
      map2FunSM<sbool, msstring, msreal>,
      map2FunSM<sbool, msstring, msstring>,

      map2FunSS<sreal, sint, sint>,
      map2FunSS<sreal, sint, sbool>,
      map2FunSS<sreal, sint, sreal>,
      map2FunSS<sreal, sint, sstring>,
      map2FunSS<sreal, sbool, sint>,
      map2FunSS<sreal, sbool, sbool>,
      map2FunSS<sreal, sbool, sreal>,
      map2FunSS<sreal, sbool, sstring>,
      map2FunSS<sreal, sreal, sint>,
      map2FunSS<sreal, sreal, sbool>,
      map2FunSS<sreal, sreal, sreal>,
      map2FunSS<sreal, sreal, sstring>,
      map2FunSS<sreal, sstring, sint>,
      map2FunSS<sreal, sstring, sbool>,
      map2FunSS<sreal, sstring, sreal>,
      map2FunSS<sreal, sstring, sstring>,
      map2FunSM<sreal, msint, msint>,
      map2FunSM<sreal, msint, msbool>,
      map2FunSM<sreal, msint, msreal>,
      map2FunSM<sreal, msint, msstring>,
      map2FunSM<sreal, msbool, msint>,
      map2FunSM<sreal, msbool, msbool>,
      map2FunSM<sreal, msbool, msreal>,
      map2FunSM<sreal, msbool, msstring>,
      map2FunSM<sreal, msreal, msint>,
      map2FunSM<sreal, msreal, msbool>,
      map2FunSM<sreal, msreal, msreal>,
      map2FunSM<sreal, msreal, msstring>,
      map2FunSM<sreal, msstring, msint>,
      map2FunSM<sreal, msstring, msbool>,
      map2FunSM<sreal, msstring, msreal>,
      map2FunSM<sreal, msstring, msstring>,

      map2FunSS<sstring, sint, sint>,
      map2FunSS<sstring, sint, sbool>,
      map2FunSS<sstring, sint, sreal>,
      map2FunSS<sstring, sint, sstring>,
      map2FunSS<sstring, sbool, sint>,
      map2FunSS<sstring, sbool, sbool>,
      map2FunSS<sstring, sbool, sreal>,
      map2FunSS<sstring, sbool, sstring>,
      map2FunSS<sstring, sreal, sint>,
      map2FunSS<sstring, sreal, sbool>,
      map2FunSS<sstring, sreal, sreal>,
      map2FunSS<sstring, sreal, sstring>,
      map2FunSS<sstring, sstring, sint>,
      map2FunSS<sstring, sstring, sbool>,
      map2FunSS<sstring, sstring, sreal>,
      map2FunSS<sstring, sstring, sstring>,
      map2FunSM<sstring, msint, msint>,
      map2FunSM<sstring, msint, msbool>,
      map2FunSM<sstring, msint, msreal>,
      map2FunSM<sstring, msint, msstring>,
      map2FunSM<sstring, msbool, msint>,
      map2FunSM<sstring, msbool, msbool>,
      map2FunSM<sstring, msbool, msreal>,
      map2FunSM<sstring, msbool, msstring>,
      map2FunSM<sstring, msreal, msint>,
      map2FunSM<sstring, msreal, msbool>,
      map2FunSM<sstring, msreal, msreal>,
      map2FunSM<sstring, msreal, msstring>,
      map2FunSM<sstring, msstring, msint>,
      map2FunSM<sstring, msstring, msbool>,
      map2FunSM<sstring, msstring, msreal>,
      map2FunSM<sstring, msstring, msstring>,

      map2FunMS<msint, sint, msint>,
      map2FunMS<msint, sint, msbool>,
      map2FunMS<msint, sint, msreal>,
      map2FunMS<msint, sint, msstring>,
      map2FunMS<msint, sbool, msint>,
      map2FunMS<msint, sbool, msbool>,
      map2FunMS<msint, sbool, msreal>,
      map2FunMS<msint, sbool, msstring>,
      map2FunMS<msint, sreal, msint>,
      map2FunMS<msint, sreal, msbool>,
      map2FunMS<msint, sreal, msreal>,
      map2FunMS<msint, sreal, msstring>,
      map2FunMS<msint, sstring, msint>,
      map2FunMS<msint, sstring, msbool>,
      map2FunMS<msint, sstring, msreal>,
      map2FunMS<msint, sstring, msstring>,
      map2FunMM<msint, msint, msint>,
      map2FunMM<msint, msint, msbool>,
      map2FunMM<msint, msint, msreal>,
      map2FunMM<msint, msint, msstring>,
      map2FunMM<msint, msbool, msint>,
      map2FunMM<msint, msbool, msbool>,
      map2FunMM<msint, msbool, msreal>,
      map2FunMM<msint, msbool, msstring>,
      map2FunMM<msint, msreal, msint>,
      map2FunMM<msint, msreal, msbool>,
      map2FunMM<msint, msreal, msreal>,
      map2FunMM<msint, msreal, msstring>,
      map2FunMM<msint, msstring, msint>,
      map2FunMM<msint, msstring, msbool>,
      map2FunMM<msint, msstring, msreal>,
      map2FunMM<msint, msstring, msstring>,

      map2FunMS<msbool, sint, msint>,
      map2FunMS<msbool, sint, msbool>,
      map2FunMS<msbool, sint, msreal>,
      map2FunMS<msbool, sint, msstring>,
      map2FunMS<msbool, sbool, msint>,
      map2FunMS<msbool, sbool, msbool>,
      map2FunMS<msbool, sbool, msreal>,
      map2FunMS<msbool, sbool, msstring>,
      map2FunMS<msbool, sreal, msint>,
      map2FunMS<msbool, sreal, msbool>,
      map2FunMS<msbool, sreal, msreal>,
      map2FunMS<msbool, sreal, msstring>,
      map2FunMS<msbool, sstring, msint>,
      map2FunMS<msbool, sstring, msbool>,
      map2FunMS<msbool, sstring, msreal>,
      map2FunMS<msbool, sstring, msstring>,
      map2FunMM<msbool, msint, msint>,
      map2FunMM<msbool, msint, msbool>,
      map2FunMM<msbool, msint, msreal>,
      map2FunMM<msbool, msint, msstring>,
      map2FunMM<msbool, msbool, msint>,
      map2FunMM<msbool, msbool, msbool>,
      map2FunMM<msbool, msbool, msreal>,
      map2FunMM<msbool, msbool, msstring>,
      map2FunMM<msbool, msreal, msint>,
      map2FunMM<msbool, msreal, msbool>,
      map2FunMM<msbool, msreal, msreal>,
      map2FunMM<msbool, msreal, msstring>,
      map2FunMM<msbool, msstring, msint>,
      map2FunMM<msbool, msstring, msbool>,
      map2FunMM<msbool, msstring, msreal>,
      map2FunMM<msbool, msstring, msstring>,

      map2FunMS<msreal, sint, msint>,
      map2FunMS<msreal, sint, msbool>,
      map2FunMS<msreal, sint, msreal>,
      map2FunMS<msreal, sint, msstring>,
      map2FunMS<msreal, sbool, msint>,
      map2FunMS<msreal, sbool, msbool>,
      map2FunMS<msreal, sbool, msreal>,
      map2FunMS<msreal, sbool, msstring>,
      map2FunMS<msreal, sreal, msint>,
      map2FunMS<msreal, sreal, msbool>,
      map2FunMS<msreal, sreal, msreal>,
      map2FunMS<msreal, sreal, msstring>,
      map2FunMS<msreal, sstring, msint>,
      map2FunMS<msreal, sstring, msbool>,
      map2FunMS<msreal, sstring, msreal>,
      map2FunMS<msreal, sstring, msstring>,
      map2FunMM<msreal, msint, msint>,
      map2FunMM<msreal, msint, msbool>,
      map2FunMM<msreal, msint, msreal>,
      map2FunMM<msreal, msint, msstring>,
      map2FunMM<msreal, msbool, msint>,
      map2FunMM<msreal, msbool, msbool>,
      map2FunMM<msreal, msbool, msreal>,
      map2FunMM<msreal, msbool, msstring>,
      map2FunMM<msreal, msreal, msint>,
      map2FunMM<msreal, msreal, msbool>,
      map2FunMM<msreal, msreal, msreal>,
      map2FunMM<msreal, msreal, msstring>,
      map2FunMM<msreal, msstring, msint>,
      map2FunMM<msreal, msstring, msbool>,
      map2FunMM<msreal, msstring, msreal>,
      map2FunMM<msreal, msstring, msstring>,

      map2FunMS<msstring, sint, msint>,
      map2FunMS<msstring, sint, msbool>,
      map2FunMS<msstring, sint, msreal>,
      map2FunMS<msstring, sint, msstring>,
      map2FunMS<msstring, sbool, msint>,
      map2FunMS<msstring, sbool, msbool>,
      map2FunMS<msstring, sbool, msreal>,
      map2FunMS<msstring, sbool, msstring>,
      map2FunMS<msstring, sreal, msint>,
      map2FunMS<msstring, sreal, msbool>,
      map2FunMS<msstring, sreal, msreal>,
      map2FunMS<msstring, sreal, msstring>,
      map2FunMS<msstring, sstring, msint>,
      map2FunMS<msstring, sstring, msbool>,
      map2FunMS<msstring, sstring, msreal>,
      map2FunMS<msstring, sstring, msstring>,
      map2FunMM<msstring, msint, msint>,
      map2FunMM<msstring, msint, msbool>,
      map2FunMM<msstring, msint, msreal>,
      map2FunMM<msstring, msint, msstring>,
      map2FunMM<msstring, msbool, msint>,
      map2FunMM<msstring, msbool, msbool>,
      map2FunMM<msstring, msbool, msreal>,
      map2FunMM<msstring, msbool, msstring>,
      map2FunMM<msstring, msreal, msint>,
      map2FunMM<msstring, msreal, msbool>,
      map2FunMM<msstring, msreal, msreal>,
      map2FunMM<msstring, msreal, msstring>,
      map2FunMM<msstring, msstring, msint>,
      map2FunMM<msstring, msstring, msbool>,
      map2FunMM<msstring, msstring, msreal>,
      map2FunMM<msstring, msstring, msstring>,
      0
  };

  int map2SelectCheckGridsFun(const std::string& type1,
                              const std::string& type2)
  {
    int offset = 0;
    int i;

    std::string idf = type1.substr(type1.length()-2, 2);
    std::string ids = type2.substr(type2.length()-2, 2);

    if (idf == "nt")      i = 0;
    else if (idf == "ol") i = 1;
    else if (idf == "al") i = 2;
    else                  i = 3;
    offset += util::isSType(type1) ?  8 * i : 8 * i + 32;

    if (ids == "nt")      i = 0;
    else if (ids == "ol") i = 1;
    else if (ids == "al") i = 2;
    else                  i = 3;
    offset += util::isSType(type2) ? i : i + 4;

    return offset;
  }

  template <typename S1, typename S2>
  void map2CheckGridsFunSS(Word arg1, Word arg2)
  {
    S1* s1 = static_cast<S1*>(arg1.addr);
    S2* s2 = static_cast<S2*>(arg2.addr);

    grid2 g1 = s1->getGrid();
    grid2 g2 = s2->getGrid();

    // The grid lengths need to match exactly, because otherwise the difference
    // in grid lengths will eventually become significant.
    if (g1.getLength() != g2.getLength()) {
      throw util::parse_error("Grid lengths are not compatible.");
    }

    // Calculate difference between the origins. The difference between the
    // the origins should roughly be a whole multiple of the grid length.
    double dx = (g1.getOriginX() - g2.getOriginX()) / g1.getLength();
    double dy = (g1.getOriginY() - g2.getOriginY()) / g1.getLength();

    int dc = dx + 0.5;
    int dr = dy + 0.5;

    if (!AlmostEqual(dx, dc) || !AlmostEqual(dy, dr)) {
      throw util::parse_error("Grids do not match.");
    }
  }

  template <typename S, typename M>
  void map2CheckGridsFunSM(Word arg1, Word arg2)
  {
    S* sobj = static_cast<S*>(arg1.addr);
    M* mobj = static_cast<M*>(arg2.addr);

    grid2 gs = sobj->getGrid();
    grid3 gm = mobj->getGrid();

    // The grid lengths need to match exactly, because otherwise the difference
    // in grid lengths will eventually become significant.
    if (gs.getLength() != gm.getLength()) {
      throw util::parse_error("Grid lengths are not compatible.");
    }

    // Calculate difference between the origins. The difference between the
    // the origins should roughly be a whole multiple of the grid length.
    double dx = (gs.getOriginX() - gm.getOriginX()) / gs.getLength();
    double dy = (gs.getOriginY() - gm.getOriginY()) / gs.getLength();

    int dc = dx + 0.5;
    int dr = dy + 0.5;

    if (!AlmostEqual(dx, dc) || !AlmostEqual(dy, dr)) {
      throw util::parse_error("Grids do not match.");
    }
  }

  template <typename M, typename S>
  void map2CheckGridsFunMS(Word arg1, Word arg2)
  {
    map2CheckGridsFunMS<S, M>(arg2, arg1);
  }

  template <typename M1, typename M2>
  void map2CheckGridsFunMM(Word arg1, Word arg2)
  {
    M1* m1 = static_cast<M1*>(arg1.addr);
    M2* m2 = static_cast<M2*>(arg2.addr);

    grid3 g1 = m1->getGrid();
    grid3 g2 = m2->getGrid();

    // The grid lengths need to match exactly, because otherwise the difference
    // in grid lengths will eventually become significant.
    if (g1.getLength() != g2.getLength()) {
      throw util::parse_error("Grid lengths are not compatible.");
    }

    // The duration needs to match exactly, because otherwise the difference
    // in durations will eventually become significant.
    if (g1.getDuration() != g2.getDuration()) {
      throw util::parse_error("Durations are not equal.");
    }

    // Calculate difference between the origins. The difference between the
    // the origins should roughly be a whole multiple of the grid length.
    double dx = (g1.getOriginX() - g2.getOriginX()) / g1.getLength();
    double dy = (g1.getOriginY() - g2.getOriginY()) / g1.getLength();

    int dc = dx + 0.5;
    int dr = dy + 0.5;

    if (!AlmostEqual(dx, dc) || !AlmostEqual(dy, dr)) {
      throw util::parse_error("Grids do not match.");
    }
  }

  GridChecker map2CheckGridsFuns[64] = {
      map2CheckGridsFunSS<sint, sint>,
      map2CheckGridsFunSS<sint, sbool>,
      map2CheckGridsFunSS<sint, sreal>,
      map2CheckGridsFunSS<sint, sstring>,
      map2CheckGridsFunSM<sint, msint>,
      map2CheckGridsFunSM<sint, msreal>,
      map2CheckGridsFunSM<sint, msbool>,
      map2CheckGridsFunSM<sint, msstring>,
      map2CheckGridsFunSS<sbool, sint>,
      map2CheckGridsFunSS<sbool, sbool>,
      map2CheckGridsFunSS<sbool, sreal>,
      map2CheckGridsFunSS<sbool, sstring>,
      map2CheckGridsFunSM<sbool, msint>,
      map2CheckGridsFunSM<sbool, msbool>,
      map2CheckGridsFunSM<sbool, msreal>,
      map2CheckGridsFunSM<sbool, msstring>,
      map2CheckGridsFunSS<sreal, sint>,
      map2CheckGridsFunSS<sreal, sbool>,
      map2CheckGridsFunSS<sreal, sreal>,
      map2CheckGridsFunSS<sreal, sstring>,
      map2CheckGridsFunSM<sreal, msint>,
      map2CheckGridsFunSM<sreal, msbool>,
      map2CheckGridsFunSM<sreal, msreal>,
      map2CheckGridsFunSM<sreal, msstring>,
      map2CheckGridsFunSS<sstring, sint>,
      map2CheckGridsFunSS<sstring, sbool>,
      map2CheckGridsFunSS<sstring, sreal>,
      map2CheckGridsFunSS<sstring, sstring>,
      map2CheckGridsFunSM<sstring, msint>,
      map2CheckGridsFunSM<sstring, msbool>,
      map2CheckGridsFunSM<sstring, msreal>,
      map2CheckGridsFunSM<sstring, msstring>,
      map2CheckGridsFunMS<msint, sint>,
      map2CheckGridsFunMS<msint, sbool>,
      map2CheckGridsFunMS<msint, sreal>,
      map2CheckGridsFunMS<msint, sstring>,
      map2CheckGridsFunMM<msint, msint>,
      map2CheckGridsFunMM<msint, msbool>,
      map2CheckGridsFunMM<msint, msreal>,
      map2CheckGridsFunMM<msint, msstring>,
      map2CheckGridsFunMS<msbool, sint>,
      map2CheckGridsFunMS<msbool, sbool>,
      map2CheckGridsFunMS<msbool, sreal>,
      map2CheckGridsFunMS<msbool, sstring>,
      map2CheckGridsFunMM<msbool, msint>,
      map2CheckGridsFunMM<msbool, msbool>,
      map2CheckGridsFunMM<msbool, msreal>,
      map2CheckGridsFunMM<msbool, msstring>,
      map2CheckGridsFunMS<msreal, sint>,
      map2CheckGridsFunMS<msreal, sbool>,
      map2CheckGridsFunMS<msreal, sreal>,
      map2CheckGridsFunMS<msreal, sstring>,
      map2CheckGridsFunMM<msreal, msint>,
      map2CheckGridsFunMM<msreal, msbool>,
      map2CheckGridsFunMM<msreal, msreal>,
      map2CheckGridsFunMM<msreal, msstring>,
      map2CheckGridsFunMS<msstring, sint>,
      map2CheckGridsFunMS<msstring, sbool>,
      map2CheckGridsFunMS<msstring, sreal>,
      map2CheckGridsFunMS<msstring, sstring>,
      map2CheckGridsFunMM<msstring, msint>,
      map2CheckGridsFunMM<msstring, msbool>,
      map2CheckGridsFunMM<msstring, msreal>,
      map2CheckGridsFunMM<msstring, msstring>,
  };
}
