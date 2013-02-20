/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//[->][\ensuremath{\Rightarrow}]
//[<>][\ensuremath{\neq}]


*/
#ifndef RASTER2_RASTERSTORAGE_H
#define RASTER2_RASTERSTORAGE_H

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <tr1/unordered_map>

#include <SecondoSMI.h>
#include <RTreeAlgebra.h>
#include <WinUnix.h>

#include "util/noncopyable.h"

namespace raster2
{
/*******************************************************************************
1 Interface

1.1 Class Template ~RasterStorage~

The ~RasterStorage~ class template provides a non-technical interface to the
storage of raster objects. The template parameters are:

  * *T* -- is the data type that is stored in raster cells. values of this type
           must be

  * *dim* -- is the number of coordinates that are used to identify a raster
           cell

  * *Undef* -- is a functor or function pointer that accepts a ~const T\&~ as
           argument and yields boolean true iff the value is considered to be
           undefined.

  * *undef* -- is a ~T~ that is used when undefined values must be written.

Template parameters of the same name have in other class templates have the same
meaning as the respective template parameter in this class template.

It is forward declared, because it is a friend of many other classes of this
header file.

*/
    template <class T, int dim, bool Undef(const T&)> class RasterStorage;

/*
1.2 Class Template ~RasterIndex~

The ~RasterIndex~ class template encapsulates an array of integers that is used
to identify a cell within a raster. It mainly provides methods for calculations
in cell indices.

*/
  template <int dim> class RasterIndex
  {
    public:
      RasterIndex();
      RasterIndex(const int (&)[dim]);
      RasterIndex(const int*);

      int& operator[] (int);
      int  operator[] (int) const;

      bool operator< (const RasterIndex<dim>&) const;
      bool operator> (const RasterIndex<dim>&) const;
      bool operator<=(const RasterIndex<dim>&) const;
      bool operator>=(const RasterIndex<dim>&) const;
      bool operator==(const RasterIndex<dim>&) const;
      bool operator!=(const RasterIndex<dim>&) const;

      RasterIndex<dim>& operator+=(const RasterIndex<dim>&);
      RasterIndex<dim>& operator-=(const RasterIndex<dim>&);

      RasterIndex<dim>& increment
        (const RasterIndex<dim>&, const RasterIndex<dim>&);

    private:
      int index[dim];
  };

  template <int dim> inline RasterIndex<dim>
  operator+(const RasterIndex<dim>&, const RasterIndex<dim>&);

  template <int dim> inline RasterIndex<dim>
  operator-(const RasterIndex<dim>&, const RasterIndex<dim>&);

  template <int dim> inline std::ostream&
  operator<<(std::ostream&, const RasterIndex<dim>&);


/*
1.3 Class Template ~RasterRegion~

The ~RasterRegion~ class template represents a bounding box by specifying
the lowest cell and the highest cell.

*/
    template <int dim>
    struct RasterRegion {
      RasterRegion();
      RasterRegion(const RasterIndex<dim>& min, const RasterIndex<dim>& max);
      bool operator< (const RasterRegion<dim>& other) const;

      RasterIndex<dim> Min;
      RasterIndex<dim> Max;
  };

/*
1.4 Class Template ~RasterValueProxy~

The ~RasterValueProxy~ class template wraps the information that is needed to
write to a cell of a raster. It is meant to be transparent to the user and
can be used both as an ~lvalue~ and an ~rvalue~ for objects of type ~T~. This
means, objects of class ~RasterValueProxy$<$T, dim, Undef$>$~ are implicitly
cast to type ~T~ and values of type can be assigned to a
~RasterValueProxy$<$T, dim, Undef$>$~.

*/
    template <class T, int dim, bool Undef(const T&)>
    class RasterValueProxy {
      friend class RasterStorage<T, dim, Undef>;

      public:
        operator T() const;
        RasterValueProxy<T, dim, Undef>& operator=(const T& rhs);
        RasterValueProxy<T, dim, Undef>& operator=
            (const RasterValueProxy<T, dim, Undef>& rhs);

      private:
        RasterValueProxy (const RasterIndex<dim>&,
                          RasterStorage<T, dim, Undef>*);

        RasterIndex<dim>  index;
        RasterStorage<T, dim, Undef>* storage;
    };

/*
1.5 Class Template ~RasterStorageIterator~

The ~RasterStorageIterator~ class template is used to iterate over the cells in
a ~RasterStorage~.

*/
    template <class T, int dim, bool Undef(const T&)>
    class RasterStorageIterator {
      friend class RasterStorage<T, dim, Undef>;
      public:
        RasterStorageIterator& operator++();
        RasterStorageIterator operator++(int);
        RasterValueProxy<T, dim, Undef> operator*();
        bool operator==(const RasterStorageIterator<T, dim, Undef>& rhs);
        bool operator!=(const RasterStorageIterator<T, dim, Undef>& rhs);
        const RasterIndex<dim>& getIndex() const;

      private:
        RasterStorageIterator(const RasterIndex<dim>& from,
                              const RasterIndex<dim>& to,
                              RasterStorage<T, dim, Undef>* storage);

        RasterIndex<dim> from;
        RasterIndex<dim> to;
        RasterIndex<dim> pos;
        RasterStorage<T, dim, Undef>* storage;
        bool valid;
    };

/*
1.6 Class Template ~RasterStorageRegionIterator~

The ~RasterStorageRegionIterator~ class template is used to iterate over the
regions in a ~RasterStorage~, that likely contain values.

*/

    template <class T, int dim, bool Undef(const T&)>
    class RasterStorageRegionIterator {
      friend class RasterStorage<T, dim, Undef>;
      public:
        RasterStorageRegionIterator& operator++();
        RasterStorageRegionIterator operator++(int);
        const RasterIndex<dim>& operator*() const;
        const RasterIndex<dim>* operator->() const;
        bool operator==(const RasterStorageRegionIterator<T, dim, Undef>& rhs);
        bool operator!=(const RasterStorageRegionIterator<T, dim, Undef>& rhs);

        static const RasterIndex<dim> region_size;

      private:
        RasterStorageRegionIterator(const RasterIndex<dim>& from,
                                    const RasterIndex<dim>& to,
                                    R_Tree<dim, RasterIndex<dim> >* tree);

        R_Tree<dim, RasterIndex<dim> >* tree;
        R_TreeLeafEntry<dim, RasterIndex<dim> > current;
        bool valid;
    };

/*
1.7 Class Template ~RasterRectangle~

The class template ~RasterRectangle~ works around the problem that
~Rectangle~ objects from the RectangleAlgebra cannot be stored in a C++
map. Elements in a map must be default constructible and copyable. The
~Rectangle~ does not initialize its values when default-constructed, the copy
constructor then raises an assertion error when such an object is copied.

Casting from and to ~Rectangle~ objects is done implicitly.

*/
    template <int dim>
    class RasterRectangle {
      public:
        inline RasterRectangle();
        inline RasterRectangle(const double *i, const double *x );
        inline RasterRectangle(const RasterRectangle<dim>& r);
        inline RasterRectangle(const Rectangle<dim>& r);

        bool operator== (const RasterRectangle<dim>& rhs) const;

        operator Rectangle<dim> () const;

      private:
        void initialize(const double* i, const double* x);

        static const double* getDefault();

        double min[dim];
        double max[dim];
    };

/*
1.8 Class Template ~RasterCacheItem<T, dim, Undef>~

Class invariants are

  * dirty [->] writable

  * dirty [->] smi\_record [<>] 0

  * dirty [->] buffer [<>] 0

The constructor takes ownership of the SmiRecord-pointer, but not of the
RasterStorageFile- and RasterCache-pointers.

*/
    class RasterStorageFile;
    template <class T, int dim, bool Undef(const T&)> class RasterCache;

    template <class T, int dim, bool Undef(const T&)>
    class RasterCacheItem : util::noncopyable {
      public:
        RasterCacheItem(const T& undef, RasterIndex<dim> i);
        RasterCacheItem(const T& undef, RasterIndex<dim> i, SmiRecord* r,
                        RasterStorageFile* f, RasterCache<T, dim, Undef>* c,
                        bool w, bool n);
        ~RasterCacheItem();

        bool isWritable();

        void set(SmiSize offset, const T& value);
        T get(SmiSize offset);

        void flush();
        void discard();

      private:
        int getOffset(const RasterIndex<dim>& index);

        T undefined;
        RasterIndex<dim> index;
        SmiRecord* smi_record;
        RasterStorageFile* file;
        RasterCache<T, dim, Undef>* cache;
        bool writable;
        bool dirty;
        bool has_tree_entry;
        char* buffer;
        SmiSize length;
    };

/*
1.9 Class Template ~RasterCache<T, dim, Undef>~

The ~RasterCache<T, dim, Undef>~ class template manages the cache for a
~RasterStorage<T, dim, Undef>~.

*/
    template <class T, int dim, bool Undef(const T&)>
    class RasterCache : util::noncopyable {
      friend class RasterCacheItem<T, dim, Undef>;
      public:
        typedef RasterCacheItem<T, dim, Undef> Item;
        typedef RasterIndex<dim> Index;
        typedef RasterRectangle<dim> TreeEntry;

        typedef std::list<Index> LRU_List;
        typedef typename std::list<Index>::iterator LRU_Pointer;

        typedef std::pair<Item*, LRU_Pointer> ValuePair;
        typedef std::tr1::unordered_map<Index, ValuePair> DataCache;
        typedef std::tr1::unordered_map<Index, TreeEntry> TreeCache;

        RasterCache(size_t s, R_Tree<dim, Index>* t);
        ~RasterCache();

        bool isCached(const Index& i);
        void cacheItem(const Index&, Item*);
        Item* getItem(const Index& i);

        void flush();
        void discard();
        void setSize(size_t s);
        void ensureSize(size_t s);

        void insertRectangle(const Index& i, const TreeEntry& b);
        void removeRectangle(const Index& i, const TreeEntry& b);

      private:
        size_t size;
        DataCache cache;
        LRU_List history;
        TreeCache tree_insert;
        TreeCache tree_remove;
        R_Tree<dim, Index>* tree;
    };

/*
1.10 Class ~RasterStorageFile~

The ~RasterStorageFile~ class is derived from ~SmiKeyedFile~, because
~SmiKeyedFile~ has purely virtual functions and cannot be instantiated.

*/
      class RasterStorageFile : public SmiKeyedFile
      {
        public:
          virtual ~RasterStorageFile() {};
          RasterStorageFile(bool isTemporary) :
            SmiKeyedFile(SmiFile::KeyedHash, SmiKey::String, true, isTemporary)
          {}
      };

/*
1.10 Definition of the ~RasterStorage~ Class.

*/
    template <class T, int dim, bool Undef(const T&)>
    class RasterStorage : util::noncopyable {
      friend class RasterValueProxy<T, dim, Undef>;

      public:
        typedef RasterIndex<dim> index_type;

        static const int tile_size;
        static const int cell_count;
        static const index_type region_size;

        explicit RasterStorage(const T& undefined);
        RasterStorage(const T& undefined, SmiFileId f, SmiFileId t);
        ~RasterStorage();

        T operator[] (const index_type& index) const;

        RasterValueProxy<T, dim, Undef> operator[]
            (const index_type& index);

        void remove();
        RasterRegion<dim> bbox();
        SmiFileId getRasterFileId();
        SmiFileId getTreeFileId();

        void setCacheSize(size_t);
        void ensureCacheSize(size_t);
        void flushCache();

        RasterStorageIterator<T, dim, Undef> begin();
        RasterStorageIterator<T, dim, Undef> end();
        RasterStorageIterator<T, dim, Undef> iterate
            (const index_type& from, const index_type& to);

        RasterStorageRegionIterator<T, dim, Undef> begin_regions();
        RasterStorageRegionIterator<T, dim, Undef> end_regions();
        RasterStorageRegionIterator<T, dim, Undef> iterate_regions
            (const index_type& from, const index_type& to);
        static RasterIndex<dim> getRegion(const RasterIndex<dim>& i);

      private:
        void set(const index_type& index, const T& value);
        T get(const index_type& index) const;
        RasterCacheItem<T, dim, Undef>* getRecord(
                const SmiKey&, const index_type&, bool = false) const;
        void splitIndex(const index_type& index,
                index_type& offset, SmiSize* pos = 0,
                index_type* grid = 0, SmiKey* key = 0) const;

        T undef;
        mutable RasterStorageFile* file;
        mutable R_Tree<dim, index_type >* tree;
        mutable size_t cache_size;
        mutable RasterCache<T, dim, Undef> cache;
    };

/*******************************************************************************
2 Implementation

2.1 Logging

Operations on the ~RasterCache~ can be logged to a file. to activate logging,
define the ~RASTERSTORAGE\_LOGFILE~ macro to be the name of the log file.

The log file is overwritten on each invocation of Secondo.

*/
    #ifdef RASTERSTORAGE_LOGFILE
    extern std::ofstream rslog;
    #else
    #define rslog if (false) std::clog
    #endif

/*
2.2 Methods and Operators of class ~RasterStorage~.

2.2.1 Create new storage files

The following constructor creates a ~RasterStorage~ object by creating the
necessary files to store raster data.

  * *undefined* -- is the undefined value of type ~T~.

*/
    template <class T, int dim, bool Undef(const T&)>
    RasterStorage<T, dim, Undef>::RasterStorage(const T& undefined)
      : undef(undefined),
        file(new RasterStorageFile(false)),
        tree(new R_Tree<dim, index_type>(
                WinUnix::getPageSize()-2*dim*int(sizeof(double)))),
        cache_size(10),
        cache(cache_size, tree)
    {
        file->Create();
    }

/*
2.1.2 Read existing storage files

The following constructor creates a ~RasterStorage~ object to access existing
raster data.

  * *undefined* -- is the undefined value of type ~T~.

  * *f* -- is the ID of the record file.

  * *t* -- is the IF of the file that holds the R\_Tree.

*/
    template <class T, int dim, bool Undef(const T&)>
    RasterStorage<T, dim, Undef>::RasterStorage(
            const T& undefined, SmiFileId f, SmiFileId t)
      : undef(undefined),
        file(new RasterStorageFile(false)),
        tree(new R_Tree<dim, index_type>(t, true)),
        cache_size(10),
        cache(cache_size, tree)
    {
        file->Open(f);
    }

/*
2.1.3 Destructor

This releases the resources of a ~RasterStorage~.

*/
    template <class T, int dim, bool Undef(const T&)>
    RasterStorage<T, dim, Undef>::~RasterStorage() {
        cache.flush();
        // tree and file are zero when the files have been removed and might
        // be zero when the constructor threw an exception.
        if (file != 0) {
            file->Close(true);
            delete file;
        }
        if (tree != 0) {
            // Calling tree->CloseFile() here prevents writing the header,
            // leading to a corrupt SmiFile. Deleting the R\_Tree object
            // automatically closes the file.
            delete tree;
        }
    }

/*
2.1.4 File Management

The following functions deal with managing the files associated with a
~RasterStorage~ object.

  * *remove()* -- removes the files from the persistent storage. After this
                  function is called, the ~RasterStorage~ object is unusable and
                  the only sensible operation is to destroy it.

  * *getRasterFileId()* --

  * *getTreeFileId()* -- retrieve the ~SmiFileId~ associated with this
                         ~RasterStorage~ object.

*/
    template <class T, int dim, bool Undef(const T&)>
    void RasterStorage<T, dim, Undef>::remove() {
        cache.discard();
        file->Close(true);
        file->Drop();
        delete file;
        file = 0;
        tree->DeleteFile();
        delete tree;
        tree = 0;
    }

    template <class T, int dim, bool Undef(const T&)>
    SmiFileId RasterStorage<T, dim, Undef>::getRasterFileId() {
        if (file == 0) {
            return 0;
        }
        return file->GetFileId();
    }

    template <class T, int dim, bool Undef(const T&)>
    SmiFileId RasterStorage<T, dim, Undef>::getTreeFileId() {
        if (tree == 0) {
            return 0;
        }
        return tree->FileId();
    }

/*
2.1.7 Cache Management

*/

    template <class T, int dim, bool Undef(const T&)>
    void RasterStorage<T, dim, Undef>::setCacheSize(size_t s) {
        cache.setSize(s);
    }

    template <class T, int dim, bool Undef(const T&)>
    void RasterStorage<T, dim, Undef>::ensureCacheSize(size_t s) {
        cache.ensureSize(s);
    }

    template <class T, int dim, bool Undef(const T&)>
    void RasterStorage<T, dim, Undef>::flushCache() {
        cache.flush();
    }


/*
2.1.6 Accessing Raster Data

*/
    template <class T, int dim, bool Undef(const T&)>
    RasterRegion<dim>
    RasterStorage<T, dim, Undef>::bbox() {
        cache.flush();
        RasterRegion<dim> result;
        BBox<dim> bbox(tree->Root().BoundingBox());
        for (int i = 0; i < dim; ++i) {
            result.Min[i] = int(bbox.MinD(i));
            result.Max[i] = int(bbox.MaxD(i));
        }
        return result;
    }

    template <class T, int dim, bool Undef(const T&)>
    RasterStorageIterator<T, dim, Undef>
    RasterStorage<T, dim, Undef>::iterate
        (const index_type& from, const index_type& to)
    {
        RasterStorageIterator<T, dim, Undef> it(from, to, this);
        while (it.valid && Undef(*it)) ++it;
        return it;
    }

    template <class T, int dim, bool Undef(const T&)>
    RasterStorageIterator<T, dim, Undef>
    RasterStorage<T, dim, Undef>::begin()
    {
        RasterRegion<dim> rbb = bbox();
        RasterStorageIterator<T, dim, Undef> iter(rbb.Min, rbb.Max, this);
        if (Undef(*iter)) ++iter;
        return iter;
    }

    template <class T, int dim, bool Undef(const T&)>
    RasterStorageIterator<T, dim, Undef>
    RasterStorage<T, dim, Undef>::end()
    {
        index_type i;
        return RasterStorageIterator<T, dim, Undef>(i, i, this);
    }

    template <class T, int dim, bool Undef(const T&)>
    RasterStorageRegionIterator<T, dim, Undef>
    RasterStorage<T, dim, Undef>::iterate_regions
        (const index_type& from, const index_type& to)
    {
        return RasterStorageRegionIterator<T, dim, Undef>(from, to, this->tree);
    }

    template <class T, int dim, bool Undef(const T&)>
    RasterStorageRegionIterator<T, dim, Undef>
    RasterStorage<T, dim, Undef>::begin_regions()
    {
        RasterRegion<dim> rbb = bbox();
        return RasterStorageRegionIterator<T, dim, Undef>
            (rbb.Min, rbb.Max, this->tree);
    }

    template <class T, int dim, bool Undef(const T&)>
    RasterStorageRegionIterator<T, dim, Undef>
    RasterStorage<T, dim, Undef>::end_regions()
    {
        index_type i;
        return RasterStorageRegionIterator<T, dim, Undef>(i, i, 0);
    }

    template <class T, int dim, bool Undef(const T&)>
    RasterIndex<dim>
    RasterStorage<T, dim, Undef>::getRegion(const RasterIndex<dim>& index)
    {
        index_type result = index;
        for (int i = 0; i < dim; ++i) {
          result[i] -= index[i] >= 0 ?
                       index[i] % tile_size :
                       index[i] % tile_size + tile_size;
        }
        return result;
    }

    template <class T, int dim, bool Undef(const T&)>
    inline T RasterStorage<T, dim, Undef>::operator[]
        (const index_type& index) const
    {
        return get(index);
    }

    template <class T, int dim, bool Undef(const T&)>
    inline RasterValueProxy<T, dim, Undef>
    RasterStorage<T, dim, Undef>::operator[] (const index_type& index)
    {
        return RasterValueProxy<T, dim, Undef>(index, this);
    }

/*
2.1.7 Private Functions

~getRecord()~ is used to retrieve items from the cache.

Returns a non-null pointer to a cache item. The caller is responsible for
deleting the pointed to object.

*/
    template <class T, int dim, bool Undef(const T&)>
    RasterCacheItem<T, dim, Undef>*
    RasterStorage<T, dim, Undef>::getRecord(
            const SmiKey& key, const index_type& index, bool writable) const
    {
        RasterCacheItem<T, dim, Undef>* item =
                cache.getItem(index);
        if (item == 0 || (writable && !item->isWritable())) {

            SmiRecord* record = new SmiRecord;
            bool exists = file->SelectRecord
                (key, *record, writable ? SmiFile::Update : SmiFile::ReadOnly);

            if (exists) {
                item = new RasterCacheItem<T, dim, Undef>
                    (undef, index, record, file, &cache, writable, false);
            } else if (writable) {
                file->InsertRecord(key, *record);

                T* empty_record = new T[cell_count];
                for (int i = 0; i < cell_count; ++i) {
                    empty_record[i] = undef;
                }
                double zeros[2*dim] = {0.0};
                record->Write(zeros);
                record->Write(empty_record,
                              cell_count * sizeof(T),
                              2*dim*sizeof(double));
                delete[] empty_record;

                item = new RasterCacheItem<T, dim, Undef>
                    (undef, index, record, file, &cache, true, true);
            } else {
                item = new RasterCacheItem<T, dim, Undef>(undef, index);
            }
            cache.cacheItem(index, item);
        }
        return item;
    }

    /**
     * @pre key != 0 => grid != 0
     */
    template <class T, int dim, bool Undef(const T&)>
    void RasterStorage<T, dim, Undef>::splitIndex(const index_type& index,
            index_type& offset, SmiSize* pos,
            index_type* grid, SmiKey* key) const
    {
            for (int i = 0; i < dim; ++i) {
                offset[i] = index[i] % tile_size;
                if (offset[i] < 0) offset[i] += tile_size;
            }

            if (pos != 0) {
                *pos = 0;
                for (int i = 0, multiplier = 1; i < dim;
                     ++i, multiplier *= tile_size)
                {
                    *pos += offset[i] * sizeof(T) * multiplier;
                }
                *pos += 2*sizeof(double)*dim;
            }

            if (grid != 0) {
                *grid = index - offset;
            }

            if (key != 0) {
                assert(grid != 0);
                *key = SmiKey(std::string(reinterpret_cast<char*>(grid),
                                          sizeof(index_type)));
            }
    }

    template <class T, int dim, bool Undef(const T&)>
    void RasterStorage<T, dim, Undef>::set(
            const index_type& index, const T& value)
    {
        index_type offset, grid;
        SmiSize pos;
        SmiKey key;

        splitIndex(index, offset, &pos, &grid, &key);

        RasterCacheItem<T, dim, Undef>* item = getRecord(key, grid, true);
        assert(item != 0); // record should have been created on demand
        return item->set(pos, value);
    }

    template <class T, int dim, bool Undef(const T&)>
    T RasterStorage<T, dim, Undef>::get(const index_type& index) const
    {
        index_type offset, grid;
        SmiSize pos;
        SmiKey key;

        splitIndex(index, offset, &pos, &grid, &key);

        RasterCacheItem<T, dim, Undef>* item = getRecord(key, grid, false);
        return item->get(pos);
    }

/*
2.2 Implementation of ~RasterIndex~

*/

    template <int dim> inline
    RasterIndex<dim>::RasterIndex()
    {
      for (int i = 0; i < dim; ++i) {
          index[i] = 0;
      }
    }

    template <int dim> inline
    RasterIndex<dim>::RasterIndex(const int (&ri)[dim])
    {
      std::memcpy(&index, &ri, dim*sizeof(int));
    }

    template <int dim> inline
    RasterIndex<dim>::RasterIndex(const int* ri)
    {
      std::memcpy(&index, ri, dim*sizeof(int));
    }

    template <int dim> inline int&
    RasterIndex<dim>::operator[] (int i)
    {
      assert(0 <= i && i < dim);
      return index[i];
    }

    template <int dim> inline int
    RasterIndex<dim>::operator[] (int i) const
    {
      assert(0 <= i && i < dim);
      return index[i];
    }

    template <int dim> inline bool
    RasterIndex<dim>::operator<(const RasterIndex<dim>& rhs) const
    {
      for (int i = 0; i < dim; ++i) {
        if (index[i] < rhs.index[i]) {
          return true;
        } else if (index[i] > rhs.index[i]) {
          return false;
        }
      }
      return false;
    }

    template <int dim> inline bool
    RasterIndex<dim>::operator==(const RasterIndex<dim>& rhs) const
    {
      for (int i = 0; i < dim; ++i) {
        if (index[i] != rhs[i]) {
          return false;
        }
      }
      return true;
    }

    template <int dim> inline bool
    RasterIndex<dim>::operator>(const RasterIndex<dim>& rhs) const
    {
      return !(*this == rhs) && !(*this < rhs);
    }

    template <int dim> inline bool
    RasterIndex<dim>::operator!=(const RasterIndex<dim>& rhs) const
    {
      return !(*this == rhs);
    }

    template <int dim> inline bool
    RasterIndex<dim>::operator<=(const RasterIndex<dim>& rhs) const
    {
      return *this < rhs || *this == rhs;
    }

    template <int dim> inline bool
    RasterIndex<dim>::operator>=(const RasterIndex<dim>& rhs) const
    {
      return rhs <= *this;
    }

    template <int dim> inline RasterIndex<dim>&
    RasterIndex<dim>::operator+=(const RasterIndex<dim>& rhs)
    {
      for (int i = 0; i < dim; ++i) {
        index[i] += rhs.index[i];
      }
      return *this;
    }

    template <int dim> inline RasterIndex<dim>&
    RasterIndex<dim>::operator-=(const RasterIndex<dim>& rhs) {
      for (int i = 0; i < dim; ++i) {
        index[i] -= rhs.index[i];
      }
      return *this;
    }

    template <int dim> inline RasterIndex<dim>&
    RasterIndex<dim>::increment
        (const RasterIndex<dim>& from, const RasterIndex<dim>& to)
    {
      int i = 0;
      while (i < dim && index[i] >= to[i] - 1) {
          ++i;
      }
      if (i < dim) {
        ++index[i];
        for (int j = 0; j < i; ++j) {
          index[j] = from[j];
        }
      } else {
        *this = to;
      }
      return *this;
    }

    template <int dim>
    std::ostream& operator<<(std::ostream& os, const RasterIndex<dim>& ri) {
        os << "( ";
        for (int i = 0; i < dim; ++i) {
            os << ri[i] << " ";
        }
        os << ")";
        return os;
    }

    template <int dim>
    RasterIndex<dim> operator+(const RasterIndex<dim>& lhs,
                               const RasterIndex<dim>& rhs)
    {
        RasterIndex<dim> result = lhs;
        result += rhs;
        return result;
    }

    template <int dim>
    RasterIndex<dim> operator-(const RasterIndex<dim>& lhs,
                               const RasterIndex<dim>& rhs)
    {
        RasterIndex<dim> result = lhs;
        result -= rhs;
        return result;
    }

/*
2.2 Methods and Operators of ~RasterRegion~

*/

    template<int dim> inline
    RasterRegion<dim>::RasterRegion()
      : Min(), Max()
    {}

    template<int dim> inline
    RasterRegion<dim>::RasterRegion
      (const RasterIndex<dim>& min, const RasterIndex<dim>& max)
      : Min(min), Max(max)
    {}

    template<int dim> inline bool
    RasterRegion<dim>::operator<(const RasterRegion<dim>& rhs) const
    {
      if (Min < rhs.Min)
        return true;
      else if (rhs.Min < Min)
        return false;
      else
        return Max < rhs.Max;
    }

/*
2.2 Methods and Operators of ~RasterValueProxy~

*/
    template <class T, int dim, bool Undef(const T&)>
    RasterValueProxy<T, dim, Undef>::RasterValueProxy
      (const RasterIndex<dim>& i, RasterStorage<T, dim, Undef>* s)
      : index(i), storage(s)
    {
        assert(s != 0);
    }

    template <class T, int dim, bool Undef(const T&)>
    inline RasterValueProxy<T, dim, Undef>::operator T() const
    {
        return storage->get(index);
    };

    template <class T, int dim, bool Undef(const T&)>
    inline RasterValueProxy<T, dim, Undef>&
    RasterValueProxy<T, dim, Undef>::operator=(const T& rhs)
    {
        storage->set(index, rhs);
        return *this;
    }

    template <class T, int dim, bool Undef(const T&)>
    inline RasterValueProxy<T, dim, Undef>&
    RasterValueProxy<T, dim, Undef>::operator=
      (const RasterValueProxy<T, dim, Undef>& rhs)
    {
        storage->set(index, rhs.storage->get(rhs.index));
        return *this;
    }

/*
2.2 Methods and Operators of ~RasterRectangle~

*/
    template <int dim> inline
    RasterRectangle<dim>::RasterRectangle()
    {
        initialize(getDefault(), getDefault());
    }

    template <int dim> inline
    RasterRectangle<dim>::RasterRectangle(const double *i, const double *x )
    {
        initialize(i, x);
    }

    template <int dim> inline
    RasterRectangle<dim>::RasterRectangle(const RasterRectangle<dim>& r)
    {
        initialize(r.min, r.max);
    }

    template <int dim> inline
    RasterRectangle<dim>::RasterRectangle(const Rectangle<dim>& r)
    {
        if (r.IsDefined()) {
            for (int i = 0; i < dim; ++i) {
                min[i] = r.MinD(i);
                max[i] = r.MaxD(i);
            }
        } else {
            initialize(getDefault(), getDefault());
        }
    }

    template <int dim> inline bool
    RasterRectangle<dim>::operator== (const RasterRectangle<dim>& rhs) const
    {
        return std::memcmp(min, rhs.min, dim*sizeof(double)) == 0
            && std::memcmp(max, rhs.max, dim*sizeof(double)) == 0;
    }

    template <int dim> inline
    RasterRectangle<dim>::operator Rectangle<dim> () const
    {
        return Rectangle<dim>(true, min, max);
    }

    template <int dim> inline void
    RasterRectangle<dim>::initialize(const double* i, const double* x)
    {
        std::memcpy(&min, i, dim*sizeof(double));
        std::memcpy(&max, x, dim*sizeof(double));
    }

    template <int dim> inline const double*
    RasterRectangle<dim>::getDefault()
    {
        static double* min = 0;
        if (min == 0) {
            min = new double[dim];
            for (int i = 0; i < dim; ++i) {
                min[i] = 0;
            }
        }
        return min;
    }

/*
2.3 Methods of ~RasterCacheItem~

*/
    template <class T, int dim, bool Undef(const T&)> inline
    RasterCacheItem<T, dim, Undef>::RasterCacheItem(const T& undef,
        RasterIndex<dim> i, SmiRecord* r, RasterStorageFile* f,
        RasterCache<T, dim, Undef>* c, bool w, bool n)
      : undefined(undef), index(i), smi_record(r), file(f), cache(c),
        writable(w), dirty(false), has_tree_entry(!n),
        buffer(0), length(0)
    {
        assert(smi_record != 0);
        assert(file != 0);
        assert(cache != 0);
    }

    template <class T, int dim, bool Undef(const T&)> inline
    RasterCacheItem<T, dim, Undef>::RasterCacheItem(const T& undef,
        RasterIndex<dim> i)
      : undefined(undef), index(i), smi_record(0), file(0), cache(0),
        writable(false), dirty(false), has_tree_entry(false),
        buffer(0), length(0)
    { }

    template <class T, int dim, bool Undef(const T&)> inline
    RasterCacheItem<T, dim, Undef>::~RasterCacheItem()
    {
        flush();
        // SmiRecord::GetData() calls DB::get() with the DB_DBT_MALLOC flag set.
        std::free(buffer);
        delete smi_record;
    }

    template <class T, int dim, bool Undef(const T&)> inline bool
    RasterCacheItem<T, dim, Undef>::isWritable()
    {
        return writable;
    }

    template <class T, int dim, bool Undef(const T&)> void
    RasterCacheItem<T, dim, Undef>::set(SmiSize offset, const T& value)
    {
        assert(writable);
        if (buffer == 0) {
          buffer = smi_record->GetData(length);
          assert(buffer); // force a result
        }
        T* current = reinterpret_cast<T*>(buffer + offset);
        if ((Undef(*current) ^ Undef(value)) ||
            (!Undef(*current) && !Undef(value) && *current != value))
        {
          std::memcpy(buffer + offset, &value, sizeof(T));
          dirty = true;
        }
    }

    template <class T, int dim, bool Undef(const T&)> inline T
    RasterCacheItem<T, dim, Undef>::get(SmiSize offset)
    {
        if (smi_record == 0) {
          return undefined;
        } else if (buffer == 0) {
          buffer = smi_record->GetData(length);
        }
        return *reinterpret_cast<T*>(buffer + offset);
    }

    template <class T, int dim, bool Undef(const T&)> void
    RasterCacheItem<T, dim, Undef>::flush()
    {
        static int arg[dim] = {0};
        static RasterIndex<dim> from = RasterIndex<dim>(arg);
        static RasterIndex<dim> to = RasterStorage<T, dim, Undef>::region_size;

        if (dirty) {
            rslog << cache->tree->FileId() << " vvvvv " << index << ".\n";

            bool has_values = false;

            RasterRegion<dim> bbox;
            for (int i = 0; i < dim; ++i) {
                bbox.Min[i] = RasterStorage<T, dim, Undef>::tile_size;
                bbox.Max[i] = 0;
            }
            int offset;
            for (RasterIndex<dim> i = from; i < to; i.increment(from, to)) {
                offset = getOffset(i);
                T* value = reinterpret_cast<T*>(buffer + offset);
                if (!Undef(*value)) {
                    has_values = true;
                    for (int d = 0; d < dim; ++d) {
                        if (bbox.Min[d] > i[d]) {
                            bbox.Min[d] = i[d];
                        }
                        if (bbox.Max[d] < i[d]) {
                            bbox.Max[d] = i[d];
                        }
                    }
                }
            }
            bbox.Min += index;
            bbox.Max += index;

            double* min = reinterpret_cast<double*>(buffer);
            double* max = reinterpret_cast<double*>(buffer +
                                                    dim*sizeof(double));

            if (has_tree_entry) {
                cache->removeRectangle
                    (index, RasterRectangle<dim>(min, max));
                has_tree_entry = false;
            }

            if (has_values) {
                for (int i = 0; i < dim; ++i) {
                    min[i] = bbox.Min[i];
                    max[i] = bbox.Max[i] + 1;
                }
                RasterRectangle<dim> bbnew(min, max);
                cache->insertRectangle
                    (index, RasterRectangle<dim>(min,max));
                has_tree_entry = true;
                smi_record->Write(buffer, length, 0);
            } else {
                file->DeleteRecord(
                        smi_record->GetKey(), true, smi_record->GetId());
            }
        }
        dirty = false;
    }

    template <class T, int dim, bool Undef(const T&)> void
    RasterCacheItem<T, dim, Undef>::discard()
    {
        // SmiRecord::GetData() calls DB::get() with the DB_DBT_MALLOC flag set.
        std::free(buffer);
        delete smi_record;

        smi_record = 0;
        file = 0;
        cache = 0;
        writable = false;
        dirty = false;
        has_tree_entry = false;
        buffer = 0;
        length = 0;
    }

    template <class T, int dim, bool Undef(const T&)> int
    RasterCacheItem<T, dim, Undef>::getOffset(const RasterIndex<dim>& index)
    {
        int offset = 2*sizeof(double)*dim;
        for (int i = 0, multiplier = 1; i < dim;
             ++i, multiplier *= RasterStorage<T, dim, Undef>::tile_size)
        {
            offset += index[i] * sizeof(T) * multiplier;
        }
        return offset;
    }

/*
2.3 Methods of ~RasterCache~

*/

    template <class T, int dim, bool Undef(const T&)>
    RasterCache<T, dim, Undef>::RasterCache(size_t s, R_Tree<dim, Index>* t)
      : size(s), cache(), history(), tree_insert(), tree_remove(), tree(t)
    {}

    template <class T, int dim, bool Undef(const T&)>
    RasterCache<T, dim, Undef>::~RasterCache() {
        flush();
        for (typename DataCache::iterator it = cache.begin(),
                                                e = cache.end();
             it != e; ++it)
        {
            delete it->second.first;
        }
    }

    template <class T, int dim, bool Undef(const T&)> bool
    RasterCache<T, dim, Undef>::isCached(const Index& i) {
        return cache.find(i) != cache.end();
    }

    template <class T, int dim, bool Undef(const T&)> void
    RasterCache<T, dim, Undef>::cacheItem(const Index& i, Item* record) {
        // Let S be the size of the cache.
        typename DataCache::iterator cit = cache.find(i); // O(log(1))
        if (cit == cache.end()) { // O(1)
            if (cache.size() >= size) { // O(1)
                typename LRU_List::iterator hit = history.begin();// O(1)
                rslog << tree->FileId() << " <<<<< " << *hit << ".\n";
                delete cache[*hit].first; // O(1)
                cache.erase(*hit); // O(1)
                history.erase(hit); // O(1)
            }
            rslog << tree->FileId() << " >>>>> " << i << ".\n";
        } else {
            rslog << tree->FileId() << " ooooo " << i << ".\n";
            delete cit->second.first; // O(1)
            history.erase(cit->second.second); // O(log(1))
        }
        history.push_back(i);
        cache[i] = std::make_pair(record, --history.end()); // O(log(1))
    }

    template <class T, int dim, bool Undef(const T&)>
    RasterCacheItem<T, dim, Undef>*
    RasterCache<T, dim, Undef>::getItem(const Index& i) {
        // Let S be the size of the cache.
        typename DataCache::iterator it = cache.find(i); // O(log(1))
        if (it == cache.end()) { // O(1)
            return 0; // O(1)
        } else {
            typename LRU_List::iterator tmp = it->second.second;
            if (++tmp != history.end()) {
                rslog << tree->FileId() << " ^^^^^ " << i << ".\n";
                LRU_Pointer cnt = it->second.second; // O(1)
                history.erase(cnt); // O(1)
                history.push_back(i); // O(1)
                it->second.second = --history.end(); // O(1)
                assert(*cache.find(i)->second.second == i);
            }
            return it->second.first; // O(1)
        }
    }

    template <class T, int dim, bool Undef(const T&)> void
    RasterCache<T, dim, Undef>::flush()
    {
      if (tree == 0) {
          return;
      }

      for(typename DataCache::iterator it = cache.begin();
          it != cache.end();
          ++it)
      {
        it->second.first->flush();
      }

      for (typename TreeCache::iterator it = tree_remove.begin(),
                                              e = tree_remove.end();
           it != e; ++it)
      {
          rslog << tree->FileId() << " _|_|_ " << it->first << ".\n";
          tree->Remove(
                  R_TreeLeafEntry<dim, Index>(it->second, it->first));
      }
      tree_remove.clear();

      if (!tree_insert.empty()) {
          if (tree->NodeCount() == 1 && tree->EntryCount() == 0 &&
              tree->InitializeBulkLoad(false))
          {
              rslog << tree->FileId() << " BBBBB.\n";
              tree->InitializeBulkLoad(false);
              for (typename TreeCache::iterator
                      it = tree_insert.begin(), e = tree_insert.end();
                   it != e; ++it)
              {
                  rslog << tree->FileId() << " TTTTT " << it->first << "\n";
                  tree->InsertBulkLoad
                    (R_TreeLeafEntry<dim, Index>
                      (it->second, it->first) );
              }
              tree->FinalizeBulkLoad();
          } else {
              for (typename TreeCache::iterator
                      it = tree_insert.begin(), e = tree_insert.end();
                   it != e; ++it)
              {
                  rslog << tree->FileId() << " ttttt " << it->first << "\n";
                  tree->Insert
                    (R_TreeLeafEntry<dim, Index>
                      (it->second, it->first));
              }
          }
          tree_insert.clear();
      }
    }

    template <class T, int dim, bool Undef(const T&)> void
    RasterCache<T, dim, Undef>::discard()
    {
        for(typename DataCache::iterator it = cache.begin();
            it != cache.end();
            ++it)
        {
          it->second.first->discard();
        }
        size = 0;
        cache.clear();
        history.clear();
        tree_insert.clear();
        tree_remove.clear();
        tree = 0;
    }

    template <class T, int dim, bool Undef(const T&)> void
    RasterCache<T, dim, Undef>::setSize(size_t s) {
        if (s < size) {
            size_t actual = cache.size();
            if(actual > s) {
                typename LRU_List::iterator hit;
                for (size_t i = s; i < actual; ++i) {
                    hit = history.begin(); // O(1)
                    rslog << tree->FileId() << " <<<<< " << *hit << ".\n";
                    delete cache[*hit].first; // O(1)
                    cache.erase(*hit); // O(1)
                    history.erase(hit); // O(1)
                }
                assert(cache.size() == s);
            }
            assert(cache.size() <= s);
        }
        size = s;
    }

    template <class T, int dim, bool Undef(const T&)> void
    RasterCache<T, dim, Undef>::ensureSize(size_t s) {
        if (size < s) {
            size = s;
        }
    }

    template <class T, int dim, bool Undef(const T&)> void
    RasterCache<T, dim, Undef>::insertRectangle(const Index& i,
                         const RasterRectangle<dim>& b)
    {
        tree_insert[i] = b;
    }

    template <class T, int dim, bool Undef(const T&)> void
    RasterCache<T, dim, Undef>::removeRectangle(const Index& i,
                    const RasterRectangle<dim>& b)
    {
        typename TreeCache::iterator it = tree_insert.find(i);
        if (it == tree_insert.end()) {
            tree_remove[i] = b;
        } else {
            assert(it->second == b);
            tree_insert.erase(it);
        }
    }


/*
2.3 Methods of ~RasterStorageIterator~

The constructor of ~RasterStorageIterator~ is private, because
~RasterStorageIterator~ objects should only be created by its friend class
~RasterStorage~.

*/
    template <class T, int dim, bool Undef(const T&)>
    RasterStorageIterator<T, dim, Undef>::RasterStorageIterator(
            const RasterIndex<dim>& f,
            const RasterIndex<dim>& t,
            RasterStorage<T, dim, Undef>* s)
      : from(f), to(t), pos(f), storage(s), valid(f < t)
    {}

/*
Accessing elements is done in the same way as in the the standard library.

*/
    template <class T, int dim, bool Undef(const T&)>
    RasterValueProxy<T, dim, Undef>
    RasterStorageIterator<T, dim, Undef>::operator*() {
        return (*storage)[pos];
    }

/*
Moving to the next element is done in the same way as in the the C++ standard
library.

Prefix increment is implemented using

*/
    template <class T, int dim, bool Undef(const T&)>
    RasterStorageIterator<T, dim, Undef>&
    RasterStorageIterator<T, dim, Undef>::operator++ ()
    {
        if (!valid)  {
            return *this;
        }

        do {
            int i = 0;
            while (i < dim && pos[i] == to[i] - 1) {
                ++i;
            }
            if (i < dim) {
                ++pos[i];
                for (int j = 0; j < i; ++j) {
                    pos[j] = from[j];
                }
            } else {
                valid = false;
            }
        } while (valid && Undef((*storage)[pos]));

        return *this;
    }

/*
Postfix increment is implemented using

*/
    template <class T, int dim, bool Undef(const T&)>
    RasterStorageIterator<T, dim, Undef>
    RasterStorageIterator<T, dim, Undef>::operator++ (int)
    {
        RasterStorageIterator<T, dim, Undef> copy(*this);
        ++this;
        return copy;
    }

/*
~RasterStorageIterator~ objects can be compared for equality.

All invalid iterators are considered equal. No valid iterator is equal to an
invalid iterator. Valid iterators are equal if they come from the same
~RasterStorage~ object and point to the same element.

*/
    template <class T, int dim, bool Undef(const T&)>
    bool RasterStorageIterator<T, dim, Undef>::operator==
            (const RasterStorageIterator<T, dim, Undef>& rhs)
    {
      if (valid && rhs.valid) {
          return pos == rhs.pos && storage == rhs.storage;
      }
      return !valid && !rhs.valid;
    }

/*
~RasterStorageIterator~ are unequal iff they are not equal.

*/
    template <class T, int dim, bool Undef(const T&)> inline bool
    RasterStorageIterator<T, dim, Undef>::operator!=
      (const RasterStorageIterator<T, dim, Undef>& rhs)
    {
      return !(*this == rhs);
    }

    template <class T, int dim, bool Undef(const T&)> inline
    const RasterIndex<dim>&
    RasterStorageIterator<T, dim, Undef>::getIndex() const
    {
      return pos;
    };

/*
2.4 Methods of ~RasterStorageRegionIterator~

The constructor of ~RasterStorageRegionIterator~ is private, because
~RasterStorageRegionIterator~ objects should only be created by its friend class
~RasterStorage~.

*/
    template <class T, int dim, bool Undef(const T&)>
    RasterStorageRegionIterator<T, dim, Undef>::RasterStorageRegionIterator(
            const RasterIndex<dim>& from,
            const RasterIndex<dim>& to,
            R_Tree<dim, RasterIndex<dim> >* r)
      : tree(r), current(), valid(r != 0)
    {
        if (valid) {
            double min[dim], max[dim];
            for (int i = 0; i < dim; ++i) {
                min[i] = from[i];
                max[i] = to[i];
            }
            BBox<dim> box(true, min, max);
            valid = tree->First(box, current);
        }
    }

    template <class T, int dim, bool Undef(const T&)>
    RasterStorageRegionIterator<T, dim, Undef>&
    RasterStorageRegionIterator<T, dim, Undef>::operator++()
    {
        if (valid) {
            valid = tree->Next(current);
        }
        return *this;
    }

    template <class T, int dim, bool Undef(const T&)>
    RasterStorageRegionIterator<T, dim, Undef>
    RasterStorageRegionIterator<T, dim, Undef>::operator++(int) {
        RasterStorageRegionIterator<T, dim, Undef> copy(*this);
        ++this;
        return copy;
    }

    template <class T, int dim, bool Undef(const T&)>
    const RasterIndex<dim>&
    RasterStorageRegionIterator<T, dim, Undef>::operator*() const {
        assert(valid);
        return current.info;
    }

    template <class T, int dim, bool Undef(const T&)>
    const RasterIndex<dim>*
    RasterStorageRegionIterator<T, dim, Undef>::operator->() const {
        assert(valid);
        return &current.info;
    }

    template <class T, int dim, bool Undef(const T&)>
    bool RasterStorageRegionIterator<T, dim, Undef>::operator==
        (const RasterStorageRegionIterator<T, dim, Undef>& rhs)
    {
        if (valid && rhs.valid) {
            return current.info == rhs.current.info && tree == rhs.tree;
        }
        return !valid && !rhs.valid;
    }

    template <class T, int dim, bool Undef(const T&)>
    bool RasterStorageRegionIterator<T, dim, Undef>::operator!=
        (const RasterStorageRegionIterator<T, dim, Undef>& rhs)
    {
        return !(*this == rhs);
    }

/*
3 Auxiliary Functions and initializations.

*/
    template <class T, int dim> int calculatePartialGridSize() {
        double EffectivePageSize = WinUnix::getPageSize()
            - dim * sizeof(int) // for the key
            - 2*sizeof(double)*dim; // for the tree bbox;
        double ValsPerPage = EffectivePageSize/sizeof(T);
        double ValsPerDimension = std::pow(ValsPerPage, 1.0/dim);
        assert(ValsPerDimension >= 1);
        return int(ValsPerDimension);
    }

    template <class T, int dim, bool Undef(const T&)>
    const int RasterStorage<T, dim, Undef>::tile_size
        = calculatePartialGridSize<T, dim>();

    template <class T, int dim> RasterIndex<dim> calculateRegionSize() {
        RasterIndex<dim> result;
        int s = calculatePartialGridSize<T, dim>();
        for (int i = 0; i < dim; ++i) {
            result[i] = s;
        }
        return result;
    }

    template <class T, int dim, bool Undef(const T&)>
    const RasterIndex<dim>
    RasterStorageRegionIterator<T, dim, Undef>::region_size
        = calculateRegionSize<T, dim>();

    template <class T, int dim, bool Undef(const T&)>
    const RasterIndex<dim>
    RasterStorage<T, dim, Undef>::region_size
        = calculateRegionSize<T, dim>();

    template <class T, int dim> int calculateCellCount() {
        int count = 1;
        int tile_size = calculatePartialGridSize<T, dim>();
        for (int i = 0; i < dim; ++i) {
            count *= tile_size;
        }
        return count;
    }

    template <class T, int dim, bool Undef(const T&)>
    const int
    RasterStorage<T, dim, Undef>::cell_count = calculateCellCount<T, dim>();

}

namespace std {
    namespace tr1 {
        template<int dim>
        struct hash<raster2::RasterIndex<dim> > {
            size_t operator()(const raster2::RasterIndex<dim>& value) const {
                size_t result = 0;
                for (int i = 0; i < dim; ++i) {
                    result ^= static_cast<size_t>(value[i]);
                }
                return result;
            }
        };
    }
}

#endif /* RASTER2\_RASTERSTORAGE\_H */
