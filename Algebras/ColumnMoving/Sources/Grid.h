/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

1 Grid.h

*/

#pragma once

#include "Array.h"

namespace ColumnMovingAlgebra {

/* 
1.1 Declaration of the class ~Grid~

~Grid~ provides a grid index for the units of moving points.

*/

  template<int dim, int blockSize, class Value>
  class Grid {
  public:

/*
~Mbr~ represents a minimum bounding box for an object or a search. The limits
are given as coordinates.

*/
    struct Mbr {
      double l[dim], h[dim];
      Mbr unite(const Mbr & b);
      bool contains(const Mbr & b);
    };

/*
~PositionMbr~ represents a minimum bounding box for an object or a search.
The limits are given as indices of the grid cells which the object or
search overlaps.

*/
    struct PositionMbr {
      int l[dim], h[dim];
      bool operator != (const PositionMbr & b) const;
    };

/*
~IVector~ is a vector with integer coordinates. It is used for iterating
over grid cells.

*/
    struct IVector {
      int p[dim];
      IVector() = default;
      IVector(PositionMbr & pmbr);
      bool operator != (const IVector & b) const;
    };

/*
~DVector~ is a vector with double coordinates. 

*/
    struct DVector {
      double s[dim];
      DVector operator - (const DVector & b) const;
    };


/*
constructors for a new grid or reading a grid from persistant storage.

*/
    Grid(DVector offset, DVector cellSize, IVector splits);
    Grid(CRelAlgebra::Reader& source);

/*
~save~ saves the grid to persistant storage

*/
    void save(CRelAlgebra::Writer &target);
/*
~savedSize~ returns the needed space for persistant storage

*/
    int savedSize();

/*
~clear~ resets the grid

*/
    void clear();
/*
~addRow~ adds a new object which units shall be indexed

*/
    void addRow();
/*
~removeRow~ removes the last added row

*/
    void removeRow();
/*
~add~ indexes a unit to the last added row

*/
    void add(Mbr mbr, Value value);
/*
~selection~ returns the list of units for the given search Mbr

*/
    std::list<Value> selection(Mbr mbr);
/*
~offset~ returns the offset of the grid

*/
    DVector offset();
/*
~cellSize~ returns the cell size of the grid

*/
    DVector cellSize();
/*
~splits~ returns the number of splits of the in each dimension 

*/
    IVector splits();

  private:
/*
each grid cell corresponds to one or many ~Block~s which contain the indexed
units overlapping the grid cell

*/
    struct Block {
      int nextBlock;
      int count;
      Value entries[blockSize];
    };

/*
~mBlocks~ is the array of all blocks

*/
    Array<Block> m_Blocks;
/*
~Offset~ is the lower boundary of the grid

*/
    DVector m_Offset;
/*
~CellSize~ is the size of the grid cells in each dimension

*/
    DVector m_CellSize;
/*
~Splits~ is the number of splits of the grid volume in cells in each dimension

*/
    IVector m_Splits;
/*
~Log~ contains information about the last added units and is needed by
~remove~

*/
    Array<int> m_Log;
/*
~Row~ contains the index of the first entry in ~mLog~ corresponding to
each row.

*/
    Array<int> m_Row;

/*
~translateMbrToPositionMbr~ determines the grid cells which ~mbr~ 
overlaps and returns them in ~positionMbr~ 

*/
    void translateMbrToPositionMbr(Mbr & mbr, PositionMbr & positionMbr);
/*
~blockIndex~ returns the index of the first block that belongs to the
grid cell at ~position~

*/
    int blockIndex(IVector position);
/*
~createSelection~ is a helper function for ~selection~

*/
    void createSelection(PositionMbr & pmbr, IVector & position, int iDim, 
                         std::list<Value> & selection);
/*
~add~ is a helper function for adding indexed objects

*/
    void add(PositionMbr & pmbr, IVector & position, int iDim, Value & value);
  };
  
/* 
1.1 Implementation of the class ~Grid~

~unite~ expands the Mbr to contain b

*/

  template<int dim, int blockSize, class Value>
  inline typename Grid<dim, blockSize, Value>::Mbr 
  Grid<dim, blockSize, Value>::Mbr::unite(const Mbr & b)
  {
    Mbr r;

    for (int i = 0; i < dim; i++) {
      r.l[i] = std::min(l[i], b.l[i]);
      r.h[i] = std::max(h[i], b.h[i]);
    }

    return r;
  }
/*
~contains~ returns true iff b is inside the Mbr

*/
  template<int dim, int blockSize, class Value>
  inline bool 
  Grid<dim, blockSize, Value>::Mbr::contains(const Mbr & b)
  {
    for (int i = 0; i < dim; i++) 
      if (l[i] > b.l[i] || h[i] < b.h[i])
        return false;

    return true;
  }
/*
this operator checks for inequality of PositionMbr

*/
  template<int dim, int blockSize, class Value>
  inline bool Grid<dim, blockSize, Value>::PositionMbr::operator!=(
    const PositionMbr & b) const
  {
    for (int i = 0; i < dim; i++)
      if (l[i] != b.l[i] || h[i] != b.h[i])
        return false;

    return true;
  }
/*
this operator checks for inequality of IVector

*/
  template<int dim, int blockSize, class Value>
  inline bool Grid<dim, blockSize, Value>::IVector::operator!=(
    const IVector & b) const
  {
    for (int i = 0; i < dim; i++)
      if (p[i] != b.p[i])
        return false;

    return true;
  }
/*
this constructor loads the start position of pmbr into the IVector

*/
  template<int dim, int blockSize, class Value>
  inline Grid<dim, blockSize, Value>::IVector::IVector(PositionMbr & pmbr)
  {
    for (int i = 0; i < dim; i++)
      p[i] = pmbr.l[i];
  }
/*
subtraction of two DVector

*/
  template<int dim, int blockSize, class Value>
  inline typename Grid<dim, blockSize, Value>::DVector 
  Grid<dim, blockSize, Value>::DVector::operator-(
    const DVector & b) const
  {
    DVector r;
    
    for (int i = 0; i < dim; i++)
      r.s[i] = s[i] - b.s[i];

    return r;
  }
/*
constructor for a new grid. ~offset~ determines the lower boundary of the
area covered by the grid in each dimension. ~cellSize~ is the size of a
grid cell. ~splits~ is the number columns
the grid has in each dimension, so the total number of cells is the product
of all components of splits. 

*/
  template<int dim, int blockSize, class Value>
  inline Grid<dim, blockSize, Value>::Grid(DVector offset, DVector cellSize, 
    IVector splits) :
    m_Offset(offset),
    m_CellSize(cellSize),
    m_Splits(splits)
  {
    int cells = 1;
    for (int i = 0; i < dim; i++) 
      cells *= m_Splits.p[i];
    
    m_Blocks.resize(cells);
    for (int i = 0; i < cells; i++) {
      m_Blocks[i].nextBlock = -1;
      m_Blocks[i].count = 0;
    }
  }
/*
constructor reads the grid from persistent storage.

*/
  template<int dim, int blockSize, class Value>
  inline Grid<dim, blockSize, Value>::Grid(CRelAlgebra::Reader& source)
  {
    m_Blocks.load(source);
    source.ReadOrThrow(reinterpret_cast<char*>(&m_Offset), 
                       sizeof(m_Offset));
    source.ReadOrThrow(reinterpret_cast<char*>(&m_CellSize), 
                       sizeof(m_CellSize));
    source.ReadOrThrow(reinterpret_cast<char*>(&m_Splits), 
                       sizeof(m_Splits));
    m_Row.load(source);
    m_Log.load(source);
  }
/*
~save~ saves the grid to persistant storage

*/
  template<int dim, int blockSize, class Value>
  inline void Grid<dim, blockSize, Value>::save(CRelAlgebra::Writer &target)
  {
    m_Blocks.save(target);
    target.WriteOrThrow(reinterpret_cast<char*>(&m_Offset), 
                        sizeof(m_Offset));
    target.WriteOrThrow(reinterpret_cast<char*>(&m_CellSize), 
                        sizeof(m_CellSize));
    target.WriteOrThrow(reinterpret_cast<char*>(&m_Splits), 
                        sizeof(m_Splits));
    m_Row.save(target);
    m_Log.save(target);
  }
/*
~savedSize~ returns the needed space for persistant storage

*/
  template<int dim, int blockSize, class Value>
  inline int Grid<dim, blockSize, Value>::savedSize() 
  {
    return m_Blocks.savedSize() +
        sizeof(m_Offset) +
        sizeof(m_CellSize) +
        sizeof(m_Splits) +
        m_Row.savedSize() +
        m_Log.savedSize();
  }
/*
~clear~ resets the grid

*/
  template<int dim, int blockSize, class Value>
  inline void Grid<dim, blockSize, Value>::clear()
  {
    m_Blocks.clear();
    m_Log.clear();
    m_Row.clear();
  }
/*
~add~ indexes a unit for the last added row

*/
  template<int dim, int blockSize, class Value>
  inline void Grid<dim, blockSize, Value>::add(Mbr mbr, Value value)
  {
    PositionMbr pmbr;
    translateMbrToPositionMbr(mbr, pmbr);
    IVector p(pmbr);
    add(pmbr, p, 0, value);
  }
/*
~addRow~ adds a new object which units shall be indexed

*/
  template<int dim, int blockSize, class Value>
  inline void Grid<dim, blockSize, Value>::addRow()
  {
    m_Row.push_back(m_Log.size());
  }
/*
~removeRow~ removes the last added row

*/
  template<int dim, int blockSize, class Value>
  inline void Grid<dim, blockSize, Value>::removeRow()
  {
    while (static_cast<int>(m_Log.size()) > m_Row.back()) {
      int bi = m_Log.back();
      Block & b = m_Blocks[bi];

      if (b.count > 1) {
        b.count--;
      } else {
        check(b.nextBlock == static_cast<int>(m_Blocks.size()) - 1, 
                                              "rollback error");
        b = m_Blocks[b.nextBlock];
        m_Blocks.pop_back();
      }

      m_Log.pop_back();
    }

    m_Row.pop_back();
  }
/*
~selection~ returns the list of units for the given search Mbr

*/
  template<int dim, int blockSize, class Value>
  inline std::list<Value> Grid<dim, blockSize, Value>::selection(Mbr mbr)
  {
    PositionMbr pmbr;
    translateMbrToPositionMbr(mbr, pmbr);

    IVector p = IVector();
    std::list<Value> s;
    createSelection(pmbr, p, 0, s);

    return s;
  }
/*
~offset~ returns the offset of the grid. the offset determines the lower 
boundary of the
area covered by the grid in each dimension

*/
  template<int dim, int blockSize, class Value>
  inline typename Grid<dim, blockSize, Value>::DVector 
  Grid<dim, blockSize, Value>::offset()
  {
    return m_Offset;
  }
/*
~cellSize~ returns the cell size of the grid

*/
  template<int dim, int blockSize, class Value>
  inline typename Grid<dim, blockSize, Value>::DVector 
  Grid<dim, blockSize, Value>::cellSize()
  {
    return m_CellSize;
  }
/*
~splits~ is the number columns
the grid has in each dimension, so the total number of cells is the product
of all components of splits. 

*/
  template<int dim, int blockSize, class Value>
  inline typename Grid<dim, blockSize, Value>::IVector 
  Grid<dim, blockSize, Value>::splits()
  {
    return m_Splits;
  }
/*
~translateMbrToPositionMbr~ determines the grid cells which ~mbr~ 
overlaps and returns them in ~positionMbr~ 

*/
  template<int dim, int blockSize, class Value>
  inline void Grid<dim, blockSize, Value>::translateMbrToPositionMbr(
    Mbr & mbr, PositionMbr & positionMbr)
  {
    for (int i = 0; i < dim; i++) {
      int s = static_cast<int>((mbr.l[i] - m_Offset.s[i]) / m_CellSize.s[i]);
      int e = static_cast<int>((mbr.h[i] - m_Offset.s[i]) / m_CellSize.s[i]);
      
      positionMbr.l[i] = (s % m_Splits.p[i] + m_Splits.p[i]) % m_Splits.p[i];

      for (int j = 0; j < m_Splits.p[i] - 1 && s != e; j++)
        s++;

      positionMbr.h[i] = (s % m_Splits.p[i] + m_Splits.p[i]) % m_Splits.p[i];
    }
  }
/*
~blockIndex~ returns the index of the first block that belongs to the
grid cell at ~position~

*/
  template<int dim, int blockSize, class Value>
  inline int Grid<dim, blockSize, Value>::blockIndex(IVector position)
  {
    int p = 0;
    for (int i = 0; i < dim; i++)
      p = p * m_Splits.p[i] + position.p[i];

    return p;
  }
/*
~createSelection~ is a helper function for ~selection~. the grid cells
covered by the search are determined by ~pmbr~. 
~createSelection~ calls itself for each dimension increasing ~iDim~ in each 
level of recursion. 
in each level of recursion, ~createSelection~ increases the component 
~position~ with the index ~iDim~ one by one and calls itsself for this 
~position~ and ~iDim~ + 1.
in the last level of recursion, all indexed objects found 
in the grid cell are added to ~selection~.

*/
  template<int dim, int blockSize, class Value>
  inline void Grid<dim, blockSize, Value>::createSelection(PositionMbr & pmbr, 
    IVector & position, int iDim, std::list<Value> & selection)
  {
    if (iDim < dim) {
      position.p[iDim] = pmbr.l[iDim];
      do {
        createSelection(pmbr, position, iDim + 1, selection);

        if (position.p[iDim] == pmbr.h[iDim])
          return;

        position.p[iDim] = (position.p[iDim] + 1) % m_Splits.p[iDim];
      } while (true);
    } else {
      int iB = blockIndex(position);

      while (iB > -1) {
        Block & b = m_Blocks[iB];

        for (int i = 0; i < b.count; i++)
          selection.push_back(b.entries[i]);

        iB = b.nextBlock;
      }
    }
  }
/*
~add~ is a helper function for adding indexed objects. the grid cells
covered by the object are determined by ~pmbr~. 
~add~ calls itself for each dimension increasing ~iDim~ in each level
of recursion. 
in each level of recursion, ~add~ increases the component ~position~ with the
index ~iDim~ one by one and calls itsself for this ~position~ and ~iDim~ + 1.
in the last level of recursion, the object is added to the grid cell 
corresponding to ~position~

*/
  template<int dim, int blockSize, class Value>
  inline void Grid<dim, blockSize, Value>::add(PositionMbr & pmbr, 
    IVector & position, int iDim, Value & value)
  {
    if (iDim < dim) {
      position.p[iDim] = pmbr.l[iDim]; 
      do {
        add(pmbr, position, iDim + 1, value);

        if (position.p[iDim] == pmbr.h[iDim])
          return;

        position.p[iDim] = (position.p[iDim] + 1) % m_Splits.p[iDim];
      } while (true);
    } else {
      int iB = blockIndex(position);
      Block & b = m_Blocks[iB];

      if (b.count > 0 && (b.entries[b.count - 1]) == value)
        return;

      if (b.count == blockSize) {
        m_Blocks.push_back(b);
        Block & nb = m_Blocks[iB];
        nb.count = 1;
        nb.entries[0] = value;
        nb.nextBlock = m_Blocks.size() - 1;
      } else {
        b.entries[b.count] = value;
        b.count++;
      }

      m_Log.push_back(iB);
    }
  }
}
