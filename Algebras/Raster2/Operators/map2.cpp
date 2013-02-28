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
  ListExpr map2TypeMap(ListExpr args) {

     if(!nl->HasLength(args,3)){
       return listutils::typeError("3 arguments expected");  
     } 
     ListExpr arg1 = nl->First(args);
     ListExpr arg2 = nl->Second(args);
     ListExpr arg3 = nl->Third(args);

     //cout << "args = " << nl->ToString(args) << endl;
     //cout << "arg1 = " << nl->ToString(arg1) << endl;
     //cout << "arg2 = " << nl->ToString(arg2) << endl;
     //cout << "arg3 = " << nl->ToString(arg3) << endl;


    
     string err = "raster x raster x fun expected" ;

     if(!util::isSType(arg1) && !util::isMSType(arg1)){
        return listutils::typeError(  err 
                                    + " (first arg is not a raster type)");
     }
     if(!util::isSType(arg2) && !util::isMSType(arg2)){
        return listutils::typeError(  err
                                    + " (second arg is not a raster type)");
     }
     if(!listutils::isMap<2>(arg3)){
        return listutils::typeError(  err 
                       + " (third arg is not a function with 2 arguments)");
     }   
     bool static1 = util::isSType(arg1);
     bool static2 = util::isSType(arg2);

   //     if(static1!=static2){
   //     return listutils::typeError("mix of static and moving "
   //                                 "rasters not allowed");
   //  }

     string bt1 = util::getValueBasicType(nl->ToString(arg1));
     string bt2 = util::getValueBasicType(nl->ToString(arg2));

     ListExpr funarg1 = nl->Second(arg3);
     ListExpr funarg2 = nl->Third(arg3);
     ListExpr funres = nl->Fourth(arg3);
     if(!listutils::isSymbol(funarg1,bt1)){
       return listutils::typeError("fun argument 1 and raster 1 " 
                                   "type does not match");
     }
     if(!listutils::isSymbol(funarg2,bt2)){
       return listutils::typeError("fun argument 2 and raster 2 " 
                                   "type does not match");
     }
     string restype="";
     if(static1 && static2){
        restype=util::getSpatialBasicType(nl->ToString(funres));
     } else {
        restype = util::getMovingSpatialBasicType(nl->ToString(funres));
     }
     if(restype==""){
        return listutils::typeError("Functionresult does not corresponds"
                                    " to a raster type");
     }
     return nl->SymbolAtom(restype);
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

    if(!s1->isDefined() || !s2->isDefined()){
      r->setDefined(false);
      return 0;
    }

    grid2 g1 = s1->getGrid();
    grid2 g2 = s2->getGrid();

    if(!g1.matches(g2)){
      r->setDefined(false);
      return 0;
    }    

    r->clear();


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

    if(!g2.matches(g1)){
        r->setDefined(false);
        return 0;
    }

    r->clear();

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

    if(!g1.matches(g2)){
       r->setDefined(false);
       return 0;
    }

    r->clear();


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

    if(!g1.matches(g2)){
       r->setDefined(false);
       return 0;
    }
    r->clear();

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


}
