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

#include <string>
#include <sstream>

#include <AlgebraTypes.h>
#include <NestedList.h>
#include <NList.h>
#include <RelationAlgebra.h>

#include "../sint.h"
#include "../sbool.h"
#include "../sreal.h"
#include "../sstring.h"
#include "../msint.h"
#include "../msbool.h"
#include "../msreal.h"
#include "../msstring.h"
#include "../util/types.h"

#include "matchgrid.h"

namespace raster2
{
  ListExpr matchgridTypeMap(ListExpr args) {
    NList types = args;
    std::string attr_name_string;
    int attr_algebraId;
    int attr_typeId;
    bool is_stype = false;

    std::string result_type_name;

    try {
      std::ostringstream oss;
      if (types.length() != 3 && types.length() != 4) {
        oss << "Expected 3 or 4 arguments, got " << types.length() << ".";
        throw util::parse_error(oss.str());
      }

      std::string raster_type = types.first().convertToString();
      is_stype = util::isSType(raster_type);
      if (is_stype) {
        if (types.second() != NList(grid2::BasicType())) {
          oss << "Expected " << grid2::BasicType() << " as argument 2.";
          throw util::parse_error(oss.str());
        }
      } else if (util::isMSType(raster_type)) {
        if (types.second() != NList(grid3::BasicType())) {
          oss << "Expected " << grid3::BasicType() << " as argument 2.";
          throw util::parse_error(oss.str());
        }
      } else {
        oss << "Expected sT or msT as argument 1.";
        throw util::parse_error(oss.str());
      }

      if (!listutils::isMap<1>(types.third().listExpr())) {
        throw util::parse_error
            ("Third argument must be a function of one parameter.");
      }

      NList function = types.third();
      if (!listutils::isRelDescription(function.second().listExpr())) {
        throw util::parse_error
            ("Argument function must accept a relation as paramter.");
      }

      NList attr_list = function.second().second().second();
      if (attr_list.length() != 1) {
        throw util::parse_error
            ("Relation must have exactly one attribute.");
      }

      NList attr_type = attr_list.first().second();
      std::string attr_type_string = attr_type.convertToString();
      if (attr_type_string != util::getValueBasicType(raster_type)) {
        oss << "Attribute must be of type "
            << util::getValueBasicType(raster_type) << ".";
        throw util::parse_error(oss.str());
      }

      SecondoCatalog& catalog = *SecondoSystem::GetCatalog();
      catalog.LookUpTypeExpr(attr_type.listExpr(),
          attr_type_string, attr_algebraId, attr_typeId);
      if (attr_type_string.empty()) {
        oss << "Cannot find type " << attr_type.convertToString() << ".";
        throw util::parse_error(oss.str());
      }

      std::string fresult_type_name = function.third().convertToString();
      result_type_name = is_stype
                       ? util::getSpatialBasicType(fresult_type_name)
                       : util::getMovingSpatialBasicType(fresult_type_name);
      if (result_type_name.empty()) {
        oss << "Cannot find raster type to store " << fresult_type_name << ".";
        throw util::parse_error(oss.str());
      }

      if (types.length() == 4 && types.fourth() != NList(CcBool::BasicType())) {
        oss << "Parameter 4 must be of type bool.";
        throw util::parse_error(oss.str());
      }
    } catch (util::parse_error& e) {
      return NList::typeError(e.what());
    }

    if (types.length() == 4) {
      return NList(
          NList(Symbol::APPEND()),
          NList(
              NList(attr_name_string, true),
              NList(attr_algebraId),
              NList(attr_typeId)),
          NList(result_type_name)
      ).listExpr();
    } else {
      return NList(
          NList(Symbol::APPEND()),
          NList(
              NList(false, false),
              NList(attr_name_string, true),
              NList(attr_algebraId),
              NList(attr_typeId)),
          NList(result_type_name)
      ).listExpr();
    }
  }

  template <typename SIn, typename SOut, typename Traits>
  int matchgridFunS(ArgVector args, Word& result, int msg,
                    Word& local, Supplier tree)
  {
    typedef typename SIn::index_type index_type;
    typedef grid2::region_type region_type;

    result = qp->ResultStorage(tree);

    SIn& src = *static_cast<SIn*>(args[0].addr);
    grid2& g_res = *static_cast<grid2*>(args[1].addr);
    SOut& res = *static_cast<SOut*>(result.addr);

    if(!src.isDefined() || (g_res.getLength()<=0)){
       res.setDefined(false);
       return 0;   
    }

    res.clear();

    Address function = args[2].addr;
    CcBool& use_weightS = *static_cast<CcBool*>(args[3].addr);
    CcInt&  attr_algebraId = *static_cast<CcInt*>(args[5].addr);
    CcInt&  attr_typeId = *static_cast<CcInt*>(args[6].addr);

    // create tuple type fpr temporary relation
    ListExpr tuple_type = NList(
        NList(
            NList("tuple"),
            NList(
                NList("Elem"),
                NList(
                    NList(attr_algebraId.GetIntval()),
                    NList(attr_typeId.GetIntval())
                )
            ).enclose()
        )
    ).listExpr();


    TupleType* tt = new TupleType(tuple_type);

    bool use_weight = false;
    if (use_weightS.IsDefined() && use_weightS.GetValue()) {
      use_weight = true;
    }

    res.setGrid(g_res);

    grid2 g_src = src.getGrid();
    // compute area of a single cell in source for weighting
    double area_src = g_src.getLength() * g_src.getLength();

    //const index_type& size = SOut::riter_type::region_size;

    Rectangle<2>  bbox_src    = src.bbox();
    index_type   start_res    = g_res.getIndex(bbox_src.MinD(0), 
                                               bbox_src.MinD(1));
    index_type   end_res      = g_res.getIndex(bbox_src.MaxD(0), 
                                               bbox_src.MaxD(1));
    index_type   current_res  = start_res;

    ArgVector& arguments = *qp->Argument(function);
    Word function_result;

    size_t maxMem = qp->GetMemorySize(tree)*1024*1024; // in byte

    // compute, how many tuples must be stored in TupleBuffer
    int noCells = ((int) ceil(g_src.getLength() / g_res.getLength()) ) + 1;
    int noTuples = noCells * noCells;
    int tupleSize = 200; 

    size_t cacheItemSize = 
             sizeof(typename SIn::storage_type::cache_type::Item) 
           + sizeof(size_t) * 8;



    size_t tupleBufferSize = tupleSize * noTuples;
    if(tupleBufferSize > maxMem/2){
       tupleBufferSize = maxMem/2; 
    } 

    // create temporarly relation
    TupleBuffer rel(tupleBufferSize);

    size_t cacheSize = (maxMem - tupleBufferSize) / cacheItemSize;

    src.setCacheSize(cacheSize / 2);
    res.setCacheSize(cacheSize / 2);

    arguments[0].setAddr(&rel);

    while (current_res <= end_res) { // iterate over all cells in g_new
                            // inside the bbox of s

      Rectangle<2> bb_current = g_res.getBBox(current_res,current_res);
      region_type reg_src =  g_src.getRegion(bb_current);
      
      // iterate over all overlaped cells from the original raster
      index_type start_src = reg_src.Min;
      index_type current_src = start_src;
      index_type end_src = reg_src.Max;
      rel.Clear();

      while(current_src <= end_src){
         typename SIn::cell_type value = src.get(current_src);
         if(!SIn::isUndefined(value)){
            // weight if required
            if (Traits::can_weight && use_weight) {
                Rectangle<2> sc_bb = g_src.getBBox(current_src,current_src);
                Rectangle<2> overlap = bb_current.Intersection(sc_bb); 
                Traits::weight(value, overlap.Area() / area_src);
            }  
            typename SIn::wrapper_type* attr =
                new typename SIn::wrapper_type(SIn::wrap(value));
            Tuple* t = new Tuple(tt);
            t->PutAttribute(0,attr);
            rel.AppendTuple(t);
            t->DeleteIfAllowed();
         }
        // goto next cell
        if(current_src[0] < end_src[0]){
           current_src[0]++;
        } else { // new row
           current_src[1]++;
           if(current_src[1] <= end_src[1]){
              current_src[0] = start_src[0]; 
           } 
        }
      }
      // evaluate function
      qp->Request(function, function_result);

      typename SOut::wrapper_type& matched =
          *static_cast<typename SOut::wrapper_type*>(function_result.addr);
      if(matched.IsDefined()){
          res.set(current_res,SOut::unwrap(matched));
      }

      // goto next cell
     if(current_res[0] < end_res[0]){
         current_res[0]++;
      } else { // new row
         current_res[1]++;
         if(current_res[1] <= end_res[1]){
            current_res[0] = start_res[0]; 
         } 
      }
    }

    tt->DeleteIfAllowed();
    return 0;
  }

  template <typename MSIn, typename MSOut, typename Traits>
  int matchgridFunM(ArgVector args, Word& result, int msg,
                    Word& local, Supplier tree)
  {
    typedef typename MSIn::index_type index_type;
    typedef grid3::region_type region_type;

    result = qp->ResultStorage(tree);

    MSIn& s = *static_cast<MSIn*>(args[0].addr);
    grid3& g_new = *static_cast<grid3*>(args[1].addr);
    Address function = args[2].addr;
    CcBool& use_weight = *static_cast<CcBool*>(args[3].addr);
    CcString& attr_name = *static_cast<CcString*>(args[4].addr);
    CcInt& attr_algebraId = *static_cast<CcInt*>(args[5].addr);
    CcInt& attr_typeId = *static_cast<CcInt*>(args[6].addr);
    MSOut& r = *static_cast<MSOut*>(result.addr);

     if(!s.isDefined()){
       r.setDefined(false);
       return 0;
     }

     r.clear();

    ListExpr relation_description = NList(
        NList("rel"),
        NList(
            NList("tuple"),
            NList(
                NList(attr_name.toText()),
                NList(
                    NList(attr_algebraId.GetIntval()),
                    NList(attr_typeId.GetIntval())
                )
            ).enclose()
        )
    ).listExpr();
    ListExpr tuple_type = NList(relation_description).second().listExpr();

    if (!use_weight.IsDefined()) {
      use_weight.Set(true, false);
    }

    r.setGrid(g_new);

    grid3 g_old = s.getGrid();
    double area = g_old.getLength()
                * g_old.getLength()
                * g_old.getDuration().ToDouble();

    const index_type& size = MSOut::riter_type::region_size;

    Rectangle<3> bbox = s.bbox();
    index_type start = g_new.getIndex(bbox.MinD(0), bbox.MinD(1), bbox.MinD(2));
    index_type   end = g_new.getIndex(bbox.MaxD(0), bbox.MaxD(1), bbox.MaxD(2));
    index_type r_start = start;

    ArgVector& arguments = *qp->Argument(function);
    Word function_result;

    while (r_start < end) {
      index_type r_end = r_start + size;
      Rectangle<3> bb_current = g_new.getBBox(r_start, r_end);
      region_type rg_current = g_old.getRegion(bb_current);
      if (s.iterate_regions(rg_current.Min, rg_current.Max) != s.end_regions())
      {
        for (index_type i = r_start; i < r_end; i.increment(r_start, r_end))
        {
          Rectangle<3> bb_cell = g_new.getCell(i);
          region_type rg_cell = g_old.getRegion(bb_cell);
          Relation rel(relation_description, true);
          for (index_type j = rg_cell.Min;
               j < rg_cell.Max;
               j.increment(rg_cell.Min, rg_cell.Max))
          {
            typename MSIn::cell_type value = s.get(j);
            if (!MSIn::isUndefined(value)) {
              if (Traits::can_weight && use_weight.GetValue()) {
                Rectangle<3> overlap = bb_cell.Intersection(g_old.getCell(j));
                Traits::weight(value, overlap.Area() / area);
              }
              Tuple* t = new Tuple(tuple_type);
              typename MSIn::wrapper_type* attr =
                  new typename MSIn::wrapper_type(MSIn::wrap(s.get(j)));

              t->PutAttribute(0, attr);
              rel.AppendTuple(t);
            }
          }
          arguments[0].setAddr(&rel);
          qp->Request(function, function_result);

          typename MSOut::wrapper_type& matched =
              *static_cast<typename MSOut::wrapper_type*>(function_result.addr);
          r.set(i, MSOut::unwrap(matched));
        }
      }

      r_start[0] += size[0];
      if (r_start[0] >= end[0]) {
        r_start[0] = start[0];
        r_start[1] += size[1];
        if (r_start[1] >= end[1]) {
          r_start[1] = start[1];
          r_start[2] += size[2];
          if (r_start[2] >= end[2]) {
            r_start[0] = end[0];
            r_start[1] = end[1];
          }
        }
      }
    }

    return 0;
  }

  template <typename T> struct matchgrid_traits {
      static const bool can_weight;
      static inline void weight(T&, double) {};
  };
  template <typename T> const bool matchgrid_traits<T>::can_weight = false;


  template <> struct matchgrid_traits<int> {
      static const bool can_weight;
      static inline void weight(int& i, double d) { i *= d; }
  };
  const bool matchgrid_traits<int>::can_weight = true;

  template <> struct matchgrid_traits<double> {
      static const bool can_weight;
      static void weight(double& r, double d) { r *= d; };
  };
  const bool matchgrid_traits<double>::can_weight = true;

  ValueMapping matchgridFuns[] = {
      matchgridFunS<sint, sint, matchgrid_traits<int> >,
      matchgridFunS<sint, sbool, matchgrid_traits<int> >,
      matchgridFunS<sint, sreal, matchgrid_traits<int> >,
      matchgridFunS<sint, sstring, matchgrid_traits<int> >,

      matchgridFunS<sbool, sint, matchgrid_traits<char> >,
      matchgridFunS<sbool, sbool, matchgrid_traits<char> >,
      matchgridFunS<sbool, sreal, matchgrid_traits<char> >,
      matchgridFunS<sbool, sstring, matchgrid_traits<char> >,

      matchgridFunS<sreal, sint, matchgrid_traits<double> >,
      matchgridFunS<sreal, sbool, matchgrid_traits<double> >,
      matchgridFunS<sreal, sreal, matchgrid_traits<double> >,
      matchgridFunS<sreal, sstring, matchgrid_traits<double> >,

      matchgridFunS<sstring, sint, matchgrid_traits<std::string> >,
      matchgridFunS<sstring, sbool, matchgrid_traits<std::string> >,
      matchgridFunS<sstring, sreal, matchgrid_traits<std::string> >,
      matchgridFunS<sstring, sstring, matchgrid_traits<std::string> >,

      matchgridFunM<msint, msint, matchgrid_traits<int> >,
      matchgridFunM<msint, msbool, matchgrid_traits<int> >,
      matchgridFunM<msint, msreal, matchgrid_traits<int> >,
      matchgridFunM<msint, msstring, matchgrid_traits<int> >,

      matchgridFunM<msbool, msint, matchgrid_traits<char> >,
      matchgridFunM<msbool, msbool, matchgrid_traits<char> >,
      matchgridFunM<msbool, msreal, matchgrid_traits<char> >,
      matchgridFunM<msbool, msstring, matchgrid_traits<char> >,

      matchgridFunM<msreal, msint, matchgrid_traits<double> >,
      matchgridFunM<msreal, msbool, matchgrid_traits<double> >,
      matchgridFunM<msreal, msreal, matchgrid_traits<double> >,
      matchgridFunM<msreal, msstring, matchgrid_traits<double> >,

      matchgridFunM<msstring, msint, matchgrid_traits<std::string> >,
      matchgridFunM<msstring, msbool, matchgrid_traits<std::string> >,
      matchgridFunM<msstring, msreal, matchgrid_traits<std::string> >,
      matchgridFunM<msstring, msstring, matchgrid_traits<std::string> >,
      0
  };

  int matchgridSelectFun(ListExpr args) {
    int offset = 0;
    int i;

    NList types(args);
    std::string in = types.first().str();
    std::string out = types.third().third().str();

    std::string idf = in.substr(in.length()-2, 2);
    std::string idr = out.substr(out.length()-2, 2);

    if (idf == "nt")      i = 0;
    else if (idf == "ol") i = 1;
    else if (idf == "al") i = 2;
    else                  i = 3;
    offset += 4 * i + (util::isSType(in) ? 0 : 16);

    if (idr == "nt")      i = 0;
    else if (idr == "ol") i = 1;
    else if (idr == "al") i = 2;
    else                  i = 3;
    offset += i;

    return offset;
  }

}
