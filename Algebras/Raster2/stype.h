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

#ifndef RASTER2_STYPE_H
#define RASTER2_STYPE_H

/*
1 Overview

This file defines a class template ~stype$<$T, Helper$>$~ for spatial raster
data. The template arguments are

  * *T* -- the type of data that is stored in the raster cells,

  * *Helper* -- a helper class that maps general functionality on objects of
                type ~T~ to their concrete implementation within Secondo.

*/
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <string>

#include <utility>

#include <SecondoSMI.h>
#include <StandardTypes.h>
#include <RTreeAlgebra.h>
#include <LogMsg.h>
#include <FTextAlgebra.h>
#include <Stream.h>

#include "grid2.h"
#include "Defines.h"
#include "RasterStorage.h"
#include "./Import/Import.h"
#include "util/noncopyable.h"
#include "util/parse_error.h"

namespace raster2
{
/*
2 Class Template ~stype\_helper~

The class template ~stype\_helper~ defines the requirements of the ~Helper~
template argument to the ~stype$<$T, Helper$>$~ class template.

*/
    template <class T>
    struct stype_helper
    {
      typedef T implementation_type;
      typedef T wrapper_type;
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

  * ~name~ -- is the name under which the data type ~stype$<$T, Helper$>$~
    should be known to Secondo, i. e. the return value of
    ~stype$<$T, Helper$>$::BasicType()~,

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

3 Definition of the ~stype$<$T, Helper$>$~ Class Template

*/
    template <typename T, typename Helper = stype_helper<T> > class stype;

    template <typename T, typename Helper>
        void swap(stype<T, Helper>&, stype<T, Helper>&);

    template <typename T, typename Helper>
    class stype : util::noncopyable
    {
/*
~stype$<$T, Helper$>$~ makes heavy use of other classes that are implemented as
templates. To ease usage of these classes, they are typedefed to more convenient
names.

*/
      public:
        typedef stype<T, Helper> this_type;
        typedef T cell_type;
        typedef typename Helper::wrapper_type wrapper_type;
        typedef RasterStorage<T, 2, Helper::isUndefined> storage_type;
        typedef RasterIndex<2> index_type;
        typedef RasterStorageIterator<T, 2, Helper::isUndefined> iter_type;
        typedef RasterStorageRegionIterator<T, 2, Helper::isUndefined>
          riter_type;
        typedef typename Helper::moving_type moving_type;
        typedef typename Helper::unit_type unit_type;

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
~stype$<$T, Helper$>$~ objects can either be constructed by passing two
~SmiFileId~ objects, or by passing a pointer to a ~RasterStorage~ object. In the
second case, ~stype$<$T, Helper$>$~ will take ownership of the object and
release it by calling delete on it in the destructor.

*/
      public:
        stype();
        stype(const grid2&, SmiFileId, SmiFileId, const T&, const T&);
        ~stype();

        riter_type begin_regions() const;
        riter_type end_regions() const;
        riter_type iterate_regions(const index_type& from,
                                   const index_type& to) const;

        void set(const index_type& i, const T& value);
        T get(const index_type& i) const;

        void setCacheSize(size_t size);
        void flushCache();


/*
~stype$<$T, Helper$>$~ provides some convenience functions that are used by
Secondo operators.

*/
      public:
        void destroy();
        storage_type& getStorage();
        T atlocation(double x, double y) const;
        void setatlocation(double x, double y, const T& value);
        this_type* atrange(const Rectangle<2>& rect) const;
        Rect bbox() const;
        T getMinimum() const;
        T getMaximum() const;
        grid2 getGrid() const;
        void setGrid(const grid2& rGrid);

        void clear();

        void setDefined(const bool defined);
        bool isDefined() const;

        int importHgtFile(const char *currentHGTFile,
                          RasterData *HGTRasterData,
                          bool init, storage_type& rs);
        bool checkHGTConsistency(storage_type& rs,
                                long xOffset,
                                long yOffset, int extend,
                                RasterData *HGTRasterData,
                                int16_t* data);
        int importEsriGridFile(RasterData *HGTRasterData);
        void processEsriGridByRTileType(
            ifstream *esriGridDataFile,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint32_t tileLimitCount);
        void processConstantBlockData(
            ifstream *dataFile,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint8_t rMinSize);
        void processEsriTilePixelValueData(
            ifstream *dataFile,
            const uint8_t *rTileType,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint32_t tileLimitCount,
            const uint8_t rMinSize);
        void processEsriTileLiteralRunsData(
            ifstream *dataFile,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint32_t tileLimitCount,
            const uint8_t rMinSize);
        void processEsriTile16BitLiteralRunsData(
            ifstream *dataFile,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint32_t tileLimitCount,
            const uint8_t rMinSize);
        void processEsriTileRMinRunsData(
            ifstream *dataFile,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint32_t tileLimitCount,
            const uint8_t rMinSize);
        void processEsriTileCountLengthData(
            ifstream *dataFile,
            const uint8_t *rTileType,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint32_t tileLimitCount,
            const uint8_t rMinSize);
        void processEsriTileRMin1BitData(
            ifstream *dataFile,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint32_t tileLimitCount,
            const uint8_t rMinSize);
        void processEsriTileRMinCCITTRLEData(
            ifstream *dataFile,
            RasterData *esriRasterData,
            const uint32_t tileColIdx,
            const uint32_t tileRowIdx,
            const size_t maxRasterIdxY,
            const uint32_t cellColCount,
            const uint32_t cellRowCount,
            const uint32_t tileLimitCount,
            const uint16_t rTileSize,
            const uint8_t rMinSize);
        int importEsriRasterFile(
            const char *currentEsriFile,
            RasterData *EsriRasterData,
            bool init, storage_type& rs);
/*
Internally, an ~stype$<$T, Helper$>$~ object consists of the grid definition and
~RasterStorage~ object for convenient access to the ~SmiFile~s. Additional
member variables are provided for convenience.

*/
      private:
        friend void swap<T, Helper>(this_type&, this_type&);
        bool tmp;
        grid2 grid;
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
    };

/*
4 Implementation of the ~stype$<$T, Helper$>$~ Class Template

4.1 Construction, Destruction and Initialization

*/
    template <typename T, typename Helper>
    stype<T, Helper>::stype
      (const grid2& g, SmiFileId r, SmiFileId t, const T& min, const T& max)
      : tmp(false), grid(g), minimum(min), maximum(max),
        storage(new storage_type(Helper::getUndefined(), r, t)),
        defined(true)
    {

    }

    template <typename T, typename Helper>
    stype<T, Helper>::stype()
      : tmp(false), grid(0.0, 0.0, 1.0),
        minimum(Helper::getUndefined()),
        maximum(Helper::getUndefined()),
        storage(new storage_type(Helper::getUndefined())),
        defined(true)
    {

    }

    template <typename T, typename Helper>
    stype<T, Helper>::~stype() {
        if (tmp) {
            storage->remove();
        }
        delete storage;
    }

    template <typename T, typename Helper>
    void stype<T, Helper>::destroy()
    {
        tmp = true;
    }

    template <typename T, typename Helper>
    typename stype<T, Helper>::riter_type
    stype<T, Helper>::begin_regions() const
    {
        assert(storage != 0);
        return storage->begin_regions();
    }

    template <typename T, typename Helper>
    typename stype<T, Helper>::riter_type
    stype<T, Helper>::end_regions() const
    {
        assert(storage != 0);
        return storage->end_regions();
    }

    template <typename T, typename Helper>
    typename stype<T, Helper>::riter_type
    stype<T, Helper>::iterate_regions(const index_type& from,
                                      const index_type& to) const
    {
        return storage->iterate_regions(from, to);
    }


    template <typename T, typename Helper>
    void stype<T, Helper>::set(const index_type& i, const T& value) {
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
    T stype<T, Helper>::get(const index_type& i) const {
        assert(storage != 0);
        return (*storage)[i];
    }

    template <typename T, typename Helper>
    void stype<T, Helper>::setCacheSize(size_t size) {
        storage->setCacheSize(size);
    }

    template <typename T, typename Helper>
    void stype<T, Helper>::flushCache() {
        storage->flushCache();
    }

/*
4.2 Member Functions

*/
    template <typename T, typename Helper>
    inline RasterStorage<T, 2, Helper::isUndefined>&
    stype<T, Helper>::getStorage()
    {
        assert(storage != 0);
        return *storage;
    }

    template <typename T, typename Helper>
    inline T stype<T, Helper>::atlocation(double x, double y) const {
        return (*storage)[grid.getIndex(x, y)];
    }

    template <typename T, typename Helper>
    inline void stype<T, Helper>::setatlocation
      (double x, double y, const T& value)
    {
        (*storage)[grid.getIndex(x, y)] = value;
    }

    template <typename T, typename Helper>
    stype<T, Helper>*
    stype<T, Helper>::atrange(const Rectangle<2>& rect) const {
        this_type* result = new this_type();
        result->grid = grid;

        index_type rfrom = grid.getIndex(rect.MinD(0), rect.MinD(1));
        index_type rto = grid.getIndex(rect.MaxD(0), rect.MaxD(1));

        for (riter_type rit = iterate_regions(rfrom, rto),
                        re  = end_regions();
             rit != re; ++rit)
        {
            index_type from = *rit;
            index_type to = *rit + riter_type::region_size;

            for (index_type i = from; i < to; i.increment(from, to)) {
                if ((rect.MinD(0) <=
                      (i[0] * grid.getLength() + grid.getOriginX()) &&
                     rect.MaxD(0) >=
                      (i[0] * grid.getLength() + grid.getOriginX())) &&
                    (rect.MinD(1) <=
                      (i[1] * grid.getLength() + grid.getOriginY()) &&
                     rect.MaxD(1) >=
                      (i[1] * grid.getLength() + grid.getOriginY())))
                 result->set(i, get(i));
            }
        }

        return result;
    }

    template <typename T, typename Helper>
    inline Rect stype<T, Helper>::bbox() const {
        if(!isDefined()){
           return Rect(false,0,0,0,0);
        } 

        RasterRegion<2> bbox = (*storage).bbox();
        double min[2] = {
            bbox.Min[0] * grid.getLength() + grid.getOriginX(),
            bbox.Min[1] * grid.getLength() + grid.getOriginY()
        };
        double max[2] = {
            bbox.Max[0] * grid.getLength() + grid.getOriginX(),
            bbox.Max[1] * grid.getLength() + grid.getOriginY()
        };
        return Rect(true, min, max);
    }

    template <typename T, typename Helper>
    inline T stype<T, Helper>::getMinimum() const { 
      if(isDefined()){
          return minimum; 
      } else {
          return Helper::getUndefined();
      }
    };

    template <typename T, typename Helper>
    inline T stype<T, Helper>::getMaximum() const { 
       if(isDefined()){
          return maximum; 
       } else {
          return Helper::getUndefined();
       }
    };

    template <typename T, typename Helper>
    inline grid2 stype<T, Helper>::getGrid() const { return grid; };

    template <typename T, typename Helper>
    void stype<T, Helper>::setGrid(const grid2& rGrid)
    {
      grid = rGrid;
    }



    template <typename T, typename Helper>
    void stype<T, Helper>::clear()
    {
       storage->clear();
       minimum = Helper::getUndefined();
       maximum = Helper::getUndefined();
       defined = true; 
    }

    template <typename T, typename Helper>
    void stype<T, Helper>::setDefined(const bool _defined){
       if(defined != _defined){
          defined = _defined;
          if(!defined){
            clear();
            defined = false;
          }
       }
    }

   
    template <typename T, typename Helper>
    bool stype<T, Helper>::isDefined() const{
      return defined;
    }



/*
4.3 Static Member Functions for use by Secondo

*/
    template <typename T, typename Helper>
    TypeConstructor stype<T, Helper>::getTypeConstructor() {
        TypeConstructor tc_stype(
                stype<T, Helper>::BasicType(),
                stype<T, Helper>::Property,
                stype<T, Helper>::Out,
                stype<T, Helper>::In,
                0,
                0,
                stype<T, Helper>::Create,
                stype<T, Helper>::Delete,
                stype<T, Helper>::Open,
                stype<T, Helper>::Save,
                stype<T, Helper>::Close,
                stype<T, Helper>::Clone,
                stype<T, Helper>::Cast,
                stype<T, Helper>::SizeOfObj,
                stype<T, Helper>::KindCheck);

        tc_stype.AssociateKind(Kind::SIMPLE());
        return tc_stype;
    }

    template <typename T, typename Helper>
    std::string stype<T, Helper>::BasicType() {
        return Helper::name;
    }

    template <typename T, typename Helper>
    bool stype<T, Helper>::checkType(const ListExpr e) {
        return listutils::isSymbol(e,BasicType());
    }

    template <typename T, typename Helper>
    bool stype<T, Helper>::Open( SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value )
    {
      valueRecord.SetPos(offset);

      grid2 grid;
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

      this_type* p_stype =
              new this_type(grid, idraster, idtree, minimum, maximum);
      value.setAddr(p_stype);

      return true;
    }

    template <typename T, typename Helper>
    bool stype<T, Helper>::Save( SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value )
    {
      this_type* p_stype = static_cast<this_type*>(value.addr);

      SmiFileId raster = p_stype->storage->getRasterFileId(),
                tree   = p_stype->storage->getTreeFileId();

      valueRecord.SetPos(offset);

      valueRecord.Write(p_stype->grid);
      valueRecord.Write(raster);
      valueRecord.Write(tree);
      valueRecord.Write(p_stype->minimum);
      valueRecord.Write(p_stype->maximum);

      offset = valueRecord.GetPos();

      return true;
    }

    template <typename T, typename Helper>
    Word stype<T, Helper>::In(const ListExpr typeInfo, const ListExpr instance,
                   const int errorPos, ListExpr& errorInfo,
                   bool& correct )
    {


          
        NList nlist(instance);

        grid2 grid;
        std::pair<int, int> sizes;
        this_type* p_stype = new this_type();

        if(listutils::isSymbolUndefined(instance)){
            p_stype->setDefined(false);
            correct = true;
            return Word(p_stype);
        }


        try {
            if (nlist.isAtom()) {
                throw util::parse_error
                  ("Expected list as first element, got an atom.");
            }
            NList gridlist = nlist.elem(1);
            nlist.rest();
            if (gridlist.length() != 3) {
                throw util::parse_error
                  ("Type mismatch: list for grid2 is too short or too long.");
            }
            if (!gridlist.isReal(1)
             || !gridlist.isReal(2)
             || !gridlist.isReal(3))
            {
                throw util::parse_error(
                  "Type mismatch: expected 3 reals as grid2 sublist.");
            }

            if (gridlist.elem(3).realval() <= 0) {
                throw util::parse_error(
                  "The length in a grid2 must be larger than 0.");
            }

            grid = grid2(
                    gridlist.elem(1).realval(),
                    gridlist.elem(2).realval(),
                    gridlist.elem(3).realval()
            );
            p_stype->setGrid(grid);

            if (!nlist.isEmpty()) {
                NList sizelist = nlist.elem(1);
                nlist.rest();
                if (sizelist.length() != 2) {
                    throw util::parse_error(
                      "Type mismatch: list for grid2 is too short.");
                }
                if ( sizelist.isInt(1)
                  && sizelist.isInt(2)
                  && sizelist.elem(1).intval() > 0
                  && sizelist.elem(2).intval() > 0)
                {
                    sizes.first = sizelist.elem(1).intval();
                    sizes.second = sizelist.elem(2).intval();
                } else {
                    throw util::parse_error("Type mismatch: "
                      "partial grid size must contain two positive integers.");
                }
            }

            while (!nlist.isEmpty()) {
                index_type root;
                NList pagelist = nlist.first();
                nlist.rest();
                if (pagelist.length() != 3) {
                    throw util::parse_error("Type mismatch: "
                      "partial grid content must contain three elements.");
                }
                if (pagelist.isInt(1) && pagelist.isInt(2)) {
                    root[0] = pagelist.elem(1).intval();
                    root[1] = pagelist.elem(2).intval();
                } else {
                    throw util::parse_error("Type mismatch: "
                      "partial grid content must start with two integers.");
                }
                pagelist.rest();
                pagelist.rest();
                NList valuelist = pagelist.first();
                if (   valuelist.length()
                    != Cardinal(sizes.first) * Cardinal(sizes.second))
                {
                    throw util::parse_error("Type mismatch: "
                      "list for partial grid values is too short or too long.");
                }
                for (int r = 0; r < sizes.second; ++r) {
                    for (int c = 0; c < sizes.first; ++c) {
                        T value;
                        int i = r * sizes.first + c + 1;
                        if (valuelist.elem(i).isSymbol(Symbol::UNDEFINED())) {
                            value = Helper::getUndefined();
                        } else if (Helper::check(valuelist.elem(i))) {
                            value = Helper::parse(valuelist.elem(i));
                        } else {
                            throw util::parse_error("Type mismatch: "
                              "list value in partial grid has wrong type.");
                        }
                        index_type index((int[]){root[0] + c, root[1] + r});
                        p_stype->set(index, value);
                    }
                }
            }
        } catch (util::parse_error& e) {
            p_stype->destroy();
            delete p_stype;
            cmsg.inFunError(e.what());
            correct = false;
            return Word();
        }

        correct = true;
        return SetWord(p_stype);
    }

    template <typename T, typename Helper>
    ListExpr stype<T, Helper>::Out(ListExpr typeInfo, Word value)
    {
        this_type* p_stype = static_cast<this_type*>(value.addr);


        if(!p_stype->isDefined()){

           cout << "undefined stype " << endl; 
           return nl->SymbolAtom(Symbol::UNDEFINED());
        }

        NList result;

        NList gridlist;
        gridlist.append(p_stype->grid.getOriginX());
        gridlist.append(p_stype->grid.getOriginY());
        gridlist.append(p_stype->grid.getLength());
        result.append(gridlist);

        NList tilesizelist;
        const index_type& size = riter_type::region_size;
        index_type index;
        T element;

        storage_type& storage = p_stype->getStorage();
        RasterRegion<2> bb = storage.bbox();
        index_type sz = bb.Max - bb.Min;

        if (sz[0] <= size[0] && sz[1] <= size[1]) {
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
                NList valuelist;
                valuelist.append(Helper::print(element));
                partiallist.append(valuelist);
                result.append(partiallist);
            }
        } else {
            tilesizelist.append(size[0]);
            tilesizelist.append(size[1]);
            result.append(tilesizelist);

            for (riter_type rit = storage.begin_regions(),
                            re  = storage.end_regions();
                 rit != re; ++rit)
            {
                NList partiallist;
                partiallist.append((*rit)[0]);
                partiallist.append((*rit)[1]);
                NList valuelist;
                for (int r = 0; r < size[0]; ++r) {
                    for (int c = 0; c < size[1]; ++c) {
                        index = *rit + index_type((int[]){c, r});
                        element = storage[index];
                        valuelist.append(Helper::print(element));
                    }
                }
                partiallist.append(valuelist);

                result.append(partiallist);
            }
        }

        return result.listExpr();
    }

    template <typename T, typename Helper>
    Word stype<T, Helper>::Create(const ListExpr typeInfo) {
        this_type* p_stype = new this_type();
        return SetWord(p_stype);
    }

    template <typename T, typename Helper>
    void stype<T, Helper>::Delete(const ListExpr typeInfo, Word& w) {
        this_type* p_stype = static_cast<this_type*>(w.addr);
        p_stype->destroy();
        delete p_stype;

        w.addr = 0;
    }

    template <typename T, typename Helper>
    void stype<T, Helper>::Close(const ListExpr typeInfo, Word& w) {
        delete static_cast<this_type*>(w.addr);
        w.addr = 0;
    }

    template <typename T, typename Helper>
    Word stype<T, Helper>::Clone(const ListExpr typeInfo, const Word& w)
    {
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
    bool stype<T, Helper>::KindCheck(ListExpr type, ListExpr& errorInfo) {
        return NList(type).isSymbol(stype<T, Helper>::BasicType());
    }

    template <typename T, typename Helper>
    void* stype<T, Helper>::Cast(void* placement) {
        return new(placement) this_type;
    }

    template <typename T, typename Helper>
    int stype<T, Helper>::SizeOfObj() {
        return sizeof(this_type);
    }

    template <typename T, typename Helper>
    ListExpr stype<T, Helper>::Property() {
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
                  "((x y l) (szx szy) ((ix iy (v*)))*)"),
                true));
        values.append(NList(
                std::string(
                  "((0.0 0.0 1.0) (2 2) ((-32 -32 (1 2 3 4))))"),
                true));
        values.append(NList(std::string(""), true));

        property = NList(names, values);

        return property.listExpr();
    }

    template <typename T, typename Helper>
    void swap(raster2::stype<T, Helper>& a, raster2::stype<T, Helper>& b)
    {
        std::swap(a.tmp, b.tmp);
        std::swap(a.grid, b.grid);
        std::swap(a.minimum, b.minimum);
        std::swap(a.maximum, b.maximum);
        std::swap(a.storage, b.storage);
    }
}

#endif /* #ifndef RASTER2_STYPE_H */
