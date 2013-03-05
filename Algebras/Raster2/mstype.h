/*
----
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
----

*/

#ifndef RASTER2_MTYPE_H
#define RASTER2_MTYPE_H

/*
1 Overview

This file defines a class template ~mstype$<$T, Helper$>$~ for spatial raster
data. The template arguments are

  * *T* -- the type of data that is stored in the raster cells,

  * *Helper* -- a helper class that maps general functionality on objects of
                type ~T~ to their concrete implementation within Secondo.

*/
#include <cassert>
#include <stdexcept>
#include <vector>
#include <string>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <utility>

#include <SecondoSMI.h>
#include <StandardTypes.h>
#include <RTreeAlgebra.h>
#include <LogMsg.h>
#include <FTextAlgebra.h>
#include <Stream.h>

#include "grid3.h"
#include "Defines.h"
#include "RasterStorage.h"
#include "util/noncopyable.h"
#include "util/parse_error.h"

namespace raster2
{
/*
2 Class Template ~mstype\_helper~

The class template ~mstype\_helper~ defines the requirements of the ~Helper~
template argument to the ~mstype$<$T, Helper$>$~ class template.

*/
    template <class T>
    struct mstype_helper
    {
        typedef T implementation_type;
        typedef T wrapper_type;
        typedef T moving_type;
        typedef T spatial_type;
        static const char* name;
        static bool check(const NList& nl);
        static T parse(const NList& nl);
        static NList print(const T& i);
        static bool isUndefined(const T& t);
        static T getUndefined();
        static std::string BasicType();
        static wrapper_type wrap(const T& i);
        static T unwrap(const wrapper_type& i);
    };
/*
In this class template,

  * ~wrapper\_type~ -- is the data type that is used by Secondo to represent
    values of type ~T~,

  * ~name~ -- is the name under which the data type ~mstype$<$T, Helper$>$~
    should be known to Secondo, i. e. the return value of
    ~mstype$<$T, Helper$>$::BasicType()~,

  * ~check(const NList\& nl)~ tests whether an NList object represents a value
    of type ~T~,

  * ~parse(const NList\& nl)~ -- parses an NList to yield an object of type ~T~,

  * ~getUndefined()~ -- retrieves the undefined value of type ~T~,

  * ~checkUndefined(const T\&)~ -- tests whether the given value is undefined.

The ~wrapper\_type~ must provide two constructors:

  * ~wrapper\_type(const T\& v)~ or ~wrapper\_type(T v)~ -- must create a
    defined objects with value v,

  * ~wrapper\_type(bool b)~ -- must create an uninitialized object that is
    defined iff ~b == true~.

Examples of wrapper types are CcInt, CcReal.

3 Definition of the ~mstype$<$T, Helper$>$~ Class Template

*/
    template <typename T, typename Helper = mstype_helper<T> > class mstype;

    template <typename T, typename Helper>
        void swap(mstype<T, Helper>&, mstype<T, Helper>&);

    template <typename T, typename Helper>
    class mstype : util::noncopyable {
/*
~mstype$<$T, Helper$>$~ makes heavy use of other classes that are implemented as
templates. To ease usage of these classes, they are typedefed to more convenient
names.

*/
      public:
        typedef mstype<T, Helper> this_type;
        typedef T cell_type;
        typedef typename Helper::wrapper_type wrapper_type;
        typedef typename Helper::moving_type moving_type;
        typedef typename Helper::spatial_type spatial_type;
        typedef RasterStorage<T, 3, Helper::isUndefined> storage_type;
        typedef RasterIndex<3> index_type;
        typedef RasterStorageIterator<T, 3, Helper::isUndefined> iter_type;
        typedef RasterStorageRegionIterator<T, 3, Helper::isUndefined>
          riter_type;
        typedef grid3 grid_type;

        static bool isUndefined(const T& t) {
          return Helper::isUndefined(t);
        }

        static wrapper_type wrap(const T& t) {
          return Helper::wrap(t);
        }

        static T unwrap(const wrapper_type& w) {
          return Helper::unwrap(w);
        }
/*
~mstype$<$T, Helper$>$~ objects can either be constructed by passing two
~SmiFileId~ objects, or by passing a pointer to a ~RasterStorage~ object. In the
second case, ~mstype$<$T, Helper$>$~ will take ownership of the object and
release it by calling delete on it in the destructor.

*/
      public:
        mstype();
        mstype(const grid3&, SmiFileId, SmiFileId, const T&, const T&);
        ~mstype();

/*
~mstype$<$T, Helper$>$~ provides some convenience functions that are used by
Secondo operators.

*/
      public:
        void destroy();
        storage_type& getStorage();
        void getDefinedPeriods(Periods& result) const;
        this_type* atperiods(const Periods& periods);
        this_type* atrange(const Rect& pRect, const Instant& start,
                           const Instant& end);
        T atlocation(double x, double y, double t) const;
        typename Helper::moving_type* atlocation(double x, double y) const;
        void setatlocation(double x, double y, double t, const T& value);
//        this_type* atrange(const Rectangle<2>&,
//                           const Instant&,
//                           const Instant&) const;
        Rectangle<3> bbox() const;
        const T& getMinimum() const;
        const T& getMaximum() const;
        grid3 getGrid() const;
        void setGrid(const grid3& g);

        riter_type begin_regions();
        riter_type end_regions();
        riter_type iterate_regions(const index_type& from,
                                   const index_type& to);

        void set(const index_type& i, const T& value);
        T get(const index_type& i) const;

        void setCacheSize(size_t size);
        void flushCache();

        void clear();

        bool isDefined() const;

        void setDefined(const bool _defined);


/*
Internally, an ~mstype$<$T, Helper$>$~ object consists of the grid definition
and ~RasterStorage~ object for convenient access to the ~SmiFile~s. Additional
member variables are provided for convenience.

*/
      private:
        friend void swap<T, Helper>(this_type&, this_type&);
        bool tmp;
        grid3 grid;
        T minimum;
        T maximum;
        storage_type* storage;
        bool defined;

      public:
        static TypeConstructor getTypeConstructor();
        static std::string BasicType();
        static bool checkType(const ListExpr e);
        static bool Open(SmiRecord& valueRecord, size_t& offset,
                         const ListExpr typeInfo, Word& value );
        static bool Save(SmiRecord& valueRecord, size_t& offset,
                         const ListExpr typeInfo, Word& w );
        static Word In(const ListExpr typeInfo, const ListExpr instance,
                       const int errorPos, ListExpr& errorInfo,
                       bool& correct);
        static ListExpr Out(ListExpr typeInfo, Word value);
        static Word Create(const ListExpr typeInfo);
        static void Delete(const ListExpr typeInfo, Word& w);
        static void Close(const ListExpr typeInfo, Word& w);
        static Word Clone(const ListExpr typeInfo, const Word& w);
        static bool KindCheck(ListExpr type, ListExpr& errorInfo);
        static void* Cast(void* placement);
        static int SizeOfObj();
        static ListExpr Property();

      private:
        void Delete();
    };

/*
4 Implementation of the ~mstype$<$T, Helper$>$~ Class Template

4.1 Construction, Destruction and Initialization

*/
    template <typename T, typename Helper>
    mstype<T, Helper>::mstype
      (const grid3& g, SmiFileId r, SmiFileId t, const T& min, const T& max)
      : tmp(false), grid(g), minimum(min), maximum(max),
        storage(new storage_type (Helper::getUndefined(),r , t)),
        defined(true)
    {}

    template <typename T, typename Helper>
    mstype<T, Helper>::mstype()
      : tmp(false), grid(0.0, 0.0, 1.0, 1.0),
        minimum(Helper::getUndefined()),
        maximum(Helper::getUndefined()),
        storage(new storage_type(Helper::getUndefined())),
        defined(true)
    {}

    template <typename T, typename Helper>
    mstype<T, Helper>::~mstype() {
        if (tmp) {
            storage->remove();
        }
        delete storage;
    }

/*
4.2 Member Functions

*/

    template <typename T, typename Helper>
    void mstype<T, Helper>::destroy()
    {
        tmp = true;
    }

    template <typename T, typename Helper>
    inline RasterStorage<T, 3, Helper::isUndefined>&
    mstype<T, Helper>::getStorage()
    {
        assert(storage != 0);
        return *storage;
    }

    template <typename T, typename Helper>
    inline void mstype<T, Helper>::getDefinedPeriods(Periods& result) const {
        Periods p(storage_type::tile_size);

        const index_type& size = riter_type::region_size;
        index_type index;
        T element;

        double duration = grid.getDuration().ToDouble();

        // Initially empty. Eventually contains the time components of all
        // region indices
        std::tr1::unordered_set<int> done;

        // For each region (identified by the time component of its index), this
        // will contain the set of time offsets, at which no defined value has
        // been found.
        std::tr1::unordered_map<int, std::tr1::unordered_set<int> > gaps;

        // Whether a valid entry has been found at the current row and column.
        // This allows advancing to the next time index without iterating over
        // all rows and columns.
        bool found;

        // The way we iterate over the raster elements does not guarantee that
        // intervals are added in order.
        p.StartBulkLoad();

        for (riter_type rit = this->storage->begin_regions(),
                        re  = this->storage->end_regions();
             rit != re; ++rit)
        {
            if (done.find((*rit)[2]) != done.end()) {
                // This time index has already been dealt with and contains
                // no gaps. Skip it.
                continue;
            }

            if (gaps.find((*rit)[2]) == gaps.end()) {
                // This is the first region at the given time index. Mark all
                // offsets as gaps.
                for (int i = 0; i < size[2]; ++i) {
                    gaps[(*rit)[2]].insert(i);
                }
            }


            // Iterate over the gaps, that the current region can fill.
            std::tr1::unordered_set<int>::iterator it = gaps[(*rit)[2]].begin();
            std::tr1::unordered_set<int>::iterator  e = gaps[(*rit)[2]].end();
            while (it != e)
            {
                found = false;
                for (int c = 0; c < size[0]; ++c) {
                    for (int r = 0; r < size[1]; ++r) {
                        index = *rit + index_type((int[]){c, r, *it});
                        element = (*this->storage)[index];
                        if (!Helper::isUndefined(element)) {
                            // Add the interval to the periods.
                            DateTime start(index[2] * duration);
                            DateTime end((index[2] + 1) * duration);
                            p.Add(Interval<DateTime>(start, end, true, false));

                            found = true;
                            break; /* for (int r = 0; r < size[1]; ++r) */
                        }
                    }

                    if (found) {
                        break; /* for (int c = 0; c < size[0]; ++c) */
                    }
                }

                // Bookkeeping and iterator advancement.
                if (found) {
                    it = gaps[(*rit)[2]].erase(it); // Value found => Gap closed
                    if (gaps[(*rit)[2]].empty()) {
                        // There are no more gaps in regions with the current
                        // time index. Remember that, so we can skip other
                        // regions with the same time index.
                        done.insert((*rit)[2]);
                        it = e; // This breaks out of the while loop
                    }
                } else {
                    ++it;
                }
            }
        }

        // Sort the found intervals
        p.EndBulkLoad();

        // Merge adjacent intervals into larger intervals
        p.Merge(result);
        p.Destroy();

        return;
    }
    
    template <typename T, typename Helper>
    inline mstype<T, Helper>* mstype<T, Helper>::atperiods(
        const Periods& periods)
    {
        this_type* result = new this_type();
        
        grid3 copy(grid.getOriginX(),
                   grid.getOriginY(),
                   grid.getLength(),
                   grid.getDuration());
        result->grid = copy;

        const index_type& size = riter_type::region_size;
        index_type index;
        T element;

        for (riter_type rit = this->storage->begin_regions(),
                        re  = this->storage->end_regions();
             rit != re; ++rit)
        {
            for (int r = 0; r < size[0]; ++r) {
                for (int c = 0; c < size[1]; ++c) {
                    for (int t = 0; t < size[2]; ++t) {
                        index = *rit + index_type((int[]){r, c, t});
                        element = (*this->storage)[index];
                        if (!Helper::isUndefined(element)) {
                            double duration = grid.getDuration().ToDouble();
                            
                            DateTime start(index[2] * duration);
                            DateTime end((index[2] + 1) * duration);

                            Interval<DateTime> value(start, end, true, false);
                            
                            if (periods.Contains(value))
                            {
                                RasterIndex<3> ri = (int[]){index[0],
                                                            index[1],
                                                            index[2]};
                                result->set(ri, element);
                            }
                            else
                            {
                                // check, whether there is a period, for which
                                // at least half of the period is in interval
                                if (periods.Intersects(value)
                                    || periods.Inside(value))
                                {
                                    Range<DateTime> range(2);
                                    periods.Intersection(value, range);
                                    
                                    Interval<DateTime> rangeVal(start,
                                                                  end,
                                                                 true,
                                                                 false);
                                    range.Get(0, rangeVal);
                                    
                                    DateTime l = rangeVal.end - rangeVal.start;
                                    bool b = l >= grid.getDuration() * 0.5;
                                    
                                    if (b)
                                    {
                                        RasterIndex<3> ri = (int[]){index[0],
                                                                    index[1],
                                                                    index[2]};
                                        result->set(ri, element);
                                    }
                                }
                            }
                        } 
                    }
                }
            }
        }

        return result;
    }
    
    template <typename T, typename Helper>
    inline mstype<T, Helper>* mstype<T, Helper>::atrange(
            const Rect& pRect, const Instant& start,
            const Instant& end)
    {
        this_type* result = new this_type();

        double instFrom = start.ToDouble();
        double instTo = end.ToDouble();

        grid3 copy(grid.getOriginX(),
                   grid.getOriginY(),
                   grid.getLength(),
                   grid.getDuration());
        result->grid = copy;

        index_type rfrom =
             grid.getIndex(pRect.MinD(0), pRect.MinD(1), instFrom);
        index_type rto =
             grid.getIndex(pRect.MaxD(0), pRect.MaxD(1), instTo);

        for (riter_type rit = iterate_regions(rfrom, rto),
                        re  = end_regions();
                        rit != re; ++rit)
        {
              index_type from = *rit;
              index_type to = *rit + riter_type::region_size;

              for (index_type i = from; i < to; i.increment(from, to)) {
                  if((pRect.MinD(0) <=
                          (i[0] * grid.getLength() + grid.getOriginX()) &&
                       pRect.MaxD(0) >=
                          (i[0] * grid.getLength() + grid.getOriginX())) &&
                      (pRect.MinD(1) <=
                          (i[1] * grid.getLength() + grid.getOriginY()) &&
                       pRect.MaxD(1) >=
                          (i[1] * grid.getLength() + grid.getOriginY())) &&
                      (instFrom <= i[2] && instTo >= i[2]))
                      result->set(i, get(i));
              }
        }

        return result;

    }

    template <typename T, typename Helper>
    inline T mstype<T, Helper>::atlocation(double x, double y, double t) const {
        return (*storage)[grid.getIndex(x, y, t)];
    }
    
    template <typename T, typename Helper>
    inline typename Helper::moving_type*
    mstype<T, Helper>::atlocation(double x, double y) const {
      assert(grid.getDuration() > DateTime(durationtype, 0));

      typename Helper::moving_type* ret = new typename Helper::moving_type(0);

      Rectangle<3> bound = bbox();
      Instant tmax = bbox().MaxD(2);
      Instant tmin = bbox().MinD(2);

      ret->StartBulkLoad();
      for (Instant tact = tmin; tact < tmax; tact += grid.getDuration())
      {
        T value = (*storage)[grid.getIndex(x, y, tact.ToDouble())];
        if (value != Helper::getUndefined()) {
            Interval<Instant> iv(
                    tact, tact + grid.getDuration(), true, true);
            wrapper_type v(true,value);
            ret->Add(typename Helper::unit_type(iv,v,v));
        }
      }
      ret->EndBulkLoad();

      return ret;
    }

    template <typename T, typename Helper>
    inline void mstype<T, Helper>::setatlocation
      (double x, double y, double t, const T& value)
    {
        (*storage)[grid.getIndex(x, y, t)] = value;
    }
    
    template <typename T, typename Helper>
    inline Rectangle<3> mstype<T, Helper>::bbox() const {
        if(!isDefined()){
           return Rectangle<3>(false,0,0,0,0,0,0);
        }
        RasterRegion<3> bbox = (*storage).bbox();
        double min[3] = {
            bbox.Min[0] * grid.getLength() + grid.getOriginX(),
            bbox.Min[1] * grid.getLength() + grid.getOriginY(),
            bbox.Min[2] * grid.getDuration().ToDouble()
        };
        double max[3] = {
            bbox.Max[0] * grid.getLength() + grid.getOriginX(),
            bbox.Max[1] * grid.getLength() + grid.getOriginY(),
            bbox.Max[2] * grid.getDuration().ToDouble()
        };
        return Rectangle<3>(true, min, max);
    }
    
    template <typename T, typename Helper>
    inline const T& mstype<T, Helper>::getMinimum() const { 
       if(isDefined()){
         return minimum;
       } else {
         static T undef = Helper::getUndefined();
         return undef;
       }
    };
    
    template <typename T, typename Helper>
    inline const T& mstype<T, Helper>::getMaximum() const { 
       if(isDefined()){
          return maximum;
       } else {
          static T undef = Helper::getUndefined();
          return undef;
       }
    };
    
    template <typename T, typename Helper>
    inline grid3 mstype<T, Helper>::getGrid() const { return grid; };

    template <typename T, typename Helper>
    inline void mstype<T, Helper>::setGrid(const grid3& g) { grid = g; }

    template <typename T, typename Helper>
    typename mstype<T, Helper>::riter_type mstype<T, Helper>::begin_regions() {
        assert(storage != 0);
        return storage->begin_regions();
    }

    template <typename T, typename Helper>
    typename mstype<T, Helper>::riter_type mstype<T, Helper>::end_regions() {
        assert(storage != 0);
        return storage->end_regions();
    }

    template <typename T, typename Helper>
    typename mstype<T, Helper>::riter_type mstype<T, Helper>::iterate_regions
        (const index_type& from, const index_type& to)
    {
        assert(storage != 0);
        return storage->iterate_regions(from, to);
    }

    template <typename T, typename Helper>
    void mstype<T, Helper>::set(const index_type& i, const T& value) {
        assert(storage != 0);
        (*storage)[i] = value;

        if (Helper::isUndefined(minimum)) {
            minimum = value;
        } else if (!Helper::isUndefined(value)) {
            if (value < minimum) {
                minimum = value;
            }
        }
        if (Helper::isUndefined(maximum)) {
            maximum = value;
        } else if (!Helper::isUndefined(value)) {
            if (value > maximum) {
                maximum = value;
            }
        }
    }

    template <typename T, typename Helper>
    T mstype<T, Helper>::get(const index_type& i) const {
        assert(storage != 0);
        return (*storage)[i];
    }

    template <typename T, typename Helper>
    void mstype<T, Helper>::setCacheSize(size_t size) {
        storage->setCacheSize(size);
    }

    template <typename T, typename Helper>
    void mstype<T, Helper>::flushCache() {
        storage->flushCache();
    }

    template <typename T, typename Helper>
    void mstype<T, Helper>::clear() {
        storage->clear();
        minimum = maximum = Helper::getUndefined();
        defined = true; 
    }

    template <typename T, typename Helper>
    bool mstype<T, Helper>::isDefined() const {
      return defined;
    }
    

    template <typename T, typename Helper>
    void mstype<T, Helper>::setDefined(const bool _defined) {
      if(defined != _defined){
         defined = _defined;
         if(!defined){
            clear();
            defined = false;
         }
      }
    }


/*
4.3 Static Member Functions for use by Secondo

*/
    template <typename T, typename Helper>
    TypeConstructor mstype<T, Helper>::getTypeConstructor() {
        TypeConstructor tc_mstype(
                mstype<T, Helper>::BasicType(),
                mstype<T, Helper>::Property,
                mstype<T, Helper>::Out,
                mstype<T, Helper>::In,
                0,
                0,
                mstype<T, Helper>::Create,
                mstype<T, Helper>::Delete,
                mstype<T, Helper>::Open,
                mstype<T, Helper>::Save,
                mstype<T, Helper>::Close,
                mstype<T, Helper>::Clone,
                mstype<T, Helper>::Cast,
                mstype<T, Helper>::SizeOfObj,
                mstype<T, Helper>::KindCheck);

        tc_mstype.AssociateKind(Kind::SIMPLE());
        return tc_mstype;
    }

    template <typename T, typename Helper>
    std::string mstype<T, Helper>::BasicType() {
        return Helper::name;
    }
    
    template <typename T, typename Helper>
    bool mstype<T, Helper>::checkType(const ListExpr e) {
        return listutils::isSymbol(e,BasicType());
    }

    template <typename T, typename Helper>
    bool mstype<T, Helper>::Open(SmiRecord& valueRecord, size_t& offset,
                                 const ListExpr typeInfo, Word& value )
    {
      valueRecord.SetPos(offset);
    
      grid3 grid;
      SmiFileId idraster;
      SmiFileId idtree;
      T minimum;
      T maximum;

      valueRecord.Read(grid);
      valueRecord.Read(idraster);
      valueRecord.Read(idtree);
      valueRecord.Read(minimum);
      valueRecord.Read(maximum);
      
      offset = valueRecord.GetPos();

      this_type* p_mstype =
              new this_type(grid, idraster, idtree, minimum, maximum);
      value.setAddr(p_mstype);

      return true;
    }

    template <typename T, typename Helper>
    bool mstype<T, Helper>::Save( SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value )
    {
      this_type* p_mstype = static_cast<this_type*>(value.addr);

      SmiFileId raster = p_mstype->storage->getRasterFileId(),
                tree   = p_mstype->storage->getTreeFileId();

      valueRecord.SetPos(offset);

      valueRecord.Write(p_mstype->grid);
      valueRecord.Write(raster);
      valueRecord.Write(tree);
      valueRecord.Write(p_mstype->minimum);
      valueRecord.Write(p_mstype->maximum);

      offset = valueRecord.GetPos();

      return true;
    }

    template <typename T, typename Helper>
    Word mstype<T, Helper>::In(const ListExpr typeInfo, const ListExpr instance,
                   const int errorPos, ListExpr& errorInfo,
                   bool& correct )
    {

        if(listutils::isSymbolUndefined(instance)){
           this_type* p_mstype = new this_type();
           p_mstype->setDefined(false);
           correct = true;
           return Word(p_mstype);
        }


        NList nlist(instance);

        grid3 grid;
        index_type sizes((int[]){0, 0, 0});
        this_type* p_mstype = new this_type();

        try {
            if (nlist.length() < 1) {
                throw util::parse_error
                  ("Type mismatch: list for mstype is too short.");
            }
            NList gridlist = nlist.elem(1);
            nlist.rest();
            if (gridlist.length() != 4) {
                throw util::parse_error
                  ("Type mismatch: list for grid3 is too short or too long.");
            }
            if (!gridlist.isReal(1)
             || !gridlist.isReal(2)
             || !gridlist.isReal(3)
             || !gridlist.isReal(4))
            {
                throw util::parse_error(
                  "Type mismatch: expected 4 reals as grid3 sublist.");
            }

            if (gridlist.elem(3).realval() <= 0.0
             || gridlist.elem(4).realval() <= 0.0)
            {
                throw util::parse_error(
                  "Length and duration in grid3 must be larger than 0.");
            }

            grid = grid3(
                    gridlist.elem(1).realval(),
                    gridlist.elem(2).realval(),
                    gridlist.elem(3).realval(),
                    DateTime(gridlist.elem(4).realval())
            );
            p_mstype->setGrid(grid);

            if (!nlist.isEmpty()) {
                NList sizelist = nlist.elem(1);
                nlist.rest();
                if (sizelist.length() != 3) {
                    throw util::parse_error(
                      "Type mismatch: list for tile size is too short.");
                }
                if ( sizelist.isInt(1)
                  && sizelist.isInt(2)
                  && sizelist.isInt(3)
                  && sizelist.elem(1).intval() > 0
                  && sizelist.elem(2).intval() > 0
                  && sizelist.elem(3).intval() > 0)
                {
                    sizes[0] = sizelist.elem(1).intval();
                    sizes[1] = sizelist.elem(2).intval();
                    sizes[2] = sizelist.elem(3).intval();
                } else {
                    throw util::parse_error("Type mismatch: "
                      "tile size list must contain three positive integers.");
                }
            }

            while (!nlist.isEmpty()) {
                index_type root;
                NList pagelist = nlist.first();
                nlist.rest();
                if (pagelist.length() != 4) {
                    throw util::parse_error("Type mismatch: "
                      "tile content must contain four elements.");
                }
                if (pagelist.isInt(1) && pagelist.isInt(2) && pagelist.isInt(3))
                {
                    root[0] = pagelist.elem(1).intval();
                    root[1] = pagelist.elem(2).intval();
                    root[2] = pagelist.elem(3).intval();
                } else {
                    throw util::parse_error("Type mismatch: "
                      "tile content must start with three integers.");
                }
                pagelist.rest();
                pagelist.rest();
                pagelist.rest();
                NList valuelist = pagelist.first();
                if (   valuelist.length()
                    != Cardinal(sizes[0]) *
                       Cardinal(sizes[1]) *
                       Cardinal(sizes[2]))
                {
                    throw util::parse_error("Type mismatch: "
                      "list for tile values is too short or too long.");
                }
                int i = 0;
                for (int t = 0; t < sizes[2]; ++t) {
                    for (int r = 0; r < sizes[1]; ++r) {
                        for (int c = 0; c < sizes[0]; ++c) {
                            T value;
                            i = i + 1;
                            if (valuelist.elem(i).isSymbol(Symbol::UNDEFINED()))
                            {
                                value = Helper::getUndefined();
                            } else if (Helper::check(valuelist.elem(i))) {
                                value = Helper::parse(valuelist.elem(i));
                            } else {
                                throw util::parse_error("Type mismatch: "
                                  "list value in tile has wrong type.");
                            }
                            index_type index =
                                 (int[]){root[0] + c, root[1] + r, root[2] + t};
                            p_mstype->set(index, value);
                        }
                    }
                }
            }
        } catch (util::parse_error& e) {
            p_mstype->destroy();
            delete p_mstype;
            cmsg.inFunError(e.what());
            correct = false;
            return Word();
        }

        correct = true;
        return SetWord(p_mstype);
    }

    template <typename T, typename Helper>
    ListExpr mstype<T, Helper>::Out(ListExpr typeInfo, Word value)
    {
        this_type* p_mstype = static_cast<this_type*>(value.addr);

        if(!p_mstype->isDefined()){
           return nl->SymbolAtom(Symbol::UNDEFINED());
        } 

        NList result;

        NList gridlist;
        gridlist.append(p_mstype->grid.getOriginX());
        gridlist.append(p_mstype->grid.getOriginY());
        gridlist.append(p_mstype->grid.getLength());
        gridlist.append(p_mstype->grid.getDuration().ToDouble());
        result.append(gridlist);

        NList tilesizelist;
        const index_type& size = riter_type::region_size;
        index_type index;
        T element;

        storage_type& storage = p_mstype->getStorage();
        RasterRegion<3> bb = storage.bbox();
        index_type sz = bb.Max - bb.Min;

        if (sz[0] <= size[0] && sz[1] <= size[1] && sz[2] <= size[2]) {
            tilesizelist.append(1);
            tilesizelist.append(1);
            tilesizelist.append(1);
            result.append(tilesizelist);

            for (iter_type it = storage.begin(),
                            e = storage.end();
                 it != e; ++it)
            {
                element = *it;
                NList partiallist;
                partiallist.append(it.getIndex()[0]);
                partiallist.append(it.getIndex()[1]);
                partiallist.append(it.getIndex()[2]);
                NList valuelist;
                valuelist.append(Helper::print(element));
                partiallist.append(valuelist);
                result.append(partiallist);
            }
        } else {
            tilesizelist.append(size[0]);
            tilesizelist.append(size[1]);
            tilesizelist.append(size[2]);
            result.append(tilesizelist);

            for (riter_type rit = p_mstype->storage->begin_regions(),
                            re  = p_mstype->storage->end_regions();
                 rit != re; ++rit)
            {
                NList partiallist;
                partiallist.append((*rit)[0]);
                partiallist.append((*rit)[1]);
                partiallist.append((*rit)[2]);
                NList valuelist;
                for (int t = 0; t < size[2]; ++t) {
                    for (int r = 0; r < size[1]; ++r) {
                        for (int c = 0; c < size[0]; ++c) {
                            index = *rit + index_type((int[]){c, r, t});
                            element = (*p_mstype->storage)[index];
                            valuelist.append(Helper::print(element));
                        }
                    }
                }
                partiallist.append(valuelist);

                result.append(partiallist);
            }
        }

        return result.listExpr();
    }

    template <typename T, typename Helper>
    Word mstype<T, Helper>::Create(const ListExpr typeInfo) {
        this_type* p_mstype = new this_type();
        return SetWord(p_mstype);
    }

    template <typename T, typename Helper>
    void mstype<T, Helper>::Delete(const ListExpr typeInfo, Word& w) {
        this_type* p_mstype = static_cast<this_type*>(w.addr);

        p_mstype->destroy();
        delete p_mstype;

        w.addr = 0;
    }

    template <typename T, typename Helper>
    void mstype<T, Helper>::Close(const ListExpr typeInfo, Word& w) {
        delete static_cast<this_type*>(w.addr);
        w.addr = 0;
    }

    template <typename T, typename Helper>
    Word mstype<T, Helper>::Clone(const ListExpr typeInfo, const Word& w) {
        this_type* source = static_cast<this_type*>(w.addr);

        this_type* clone = new this_type();
        clone->setGrid(source->getGrid());

        for (riter_type rit = source->storage->begin_regions(),
                        re  = source->storage->end_regions();
             rit != re; ++rit)
        {
            index_type from = *rit;
            index_type to = from + storage_type::region_size;
            for (index_type i = from,
                            e = to;
                  i < e; i.increment(from, to))
            {
              clone->set(i, (*source->storage)[i]);
            }
        }

        return SetWord(clone);
    }

    template <typename T, typename Helper>
    bool mstype<T, Helper>::KindCheck(ListExpr type, ListExpr& errorInfo) {
        return NList(type).isSymbol(mstype<T, Helper>::BasicType());
    }

    template <typename T, typename Helper>
    void* mstype<T, Helper>::Cast(void* placement) {
        return new(placement) this_type;
    }

    template <typename T, typename Helper>
    int mstype<T, Helper>::SizeOfObj() {
        return sizeof(this_type);
    }

    template <typename T, typename Helper>
    ListExpr mstype<T, Helper>::Property() {
        NList property;

        NList names;
        names.append(NList(std::string("Signature"), true));
        names.append(NList(std::string("Example Type List"), true));
        names.append(NList(std::string("ListRep"), true));
        names.append(NList(std::string("Example List"), true));
        names.append(NList(std::string("Remarks"), true));

        NList values;
        values.append(NList(std::string("-> DATA"), true));
        values.append(NList(BasicType(), true));
        values.append(NList(
                std::string(
                  "((x y l t) (szx szy szt) ((ix iy it (v*)))*)"),
                true));
        values.append(NList(
                std::string("((0.0 0.0 1.0 1.0) "
                             "(1 1 1) ((-3 -2 4 (1))))"),
                true));
        values.append(NList(std::string(""), true));

        property = NList(names, values);

        return property.listExpr();
    }

    template <typename T, typename Helper>
    void swap(raster2::mstype<T, Helper>& a, raster2::mstype<T, Helper>& b)
    {
        std::swap(a.tmp, b.tmp);
        std::swap(a.grid, b.grid);
        std::swap(a.minimum, b.minimum);
        std::swap(a.maximum, b.maximum);
        std::swap(a.storage, b.storage);
    }
}

#endif /* #ifndef RASTER2_MTYPE_H */
