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

*/

#pragma once

#include "Array.h"

namespace ColumnMovingAlgebra {

  template<int dim, int blockSize, class Value>
  class Grid {
  public:

    struct Mbr {
      double l[dim], h[dim];
      Mbr unite(const Mbr & b);
    };

    struct PositionMbr {
      int l[dim], h[dim];
      bool operator != (const PositionMbr & b) const;
    };

    struct IVector {
      int p[dim];
      IVector() = default;
      IVector(PositionMbr & pmbr);
      bool operator != (const IVector & b) const;
    };

    struct DVector {
      double s[dim];
    };

    Grid(IVector splits, DVector cellSize);
    Grid(CRelAlgebra::Reader& source);

    void save(CRelAlgebra::Writer &target);
    int savedSize();

    void clear();
    void addRow();
    void removeRow();
    void add(Mbr mbr, Value value);
    std::set<Value> selection(Mbr mbr);
    Mbr mbr();

  private:
    struct Block {
      int nextBlock;
      int count;
      Value entries[blockSize];
    };

    Array<Block> m_Blocks;
    DVector m_CellSize;
    IVector m_Splits;
    Array<int> m_Log;
    Array<int> m_Row;
    Mbr m_Mbr;
    bool m_MbrValid = false;

    void translateMbrToPositionMbr(Mbr & mbr, PositionMbr & positionMbr);
    int blockIndex(IVector position);
    void createSelection(PositionMbr & pmbr, IVector & position, int iDim, 
                         std::set<Value> & selection);
    void add(PositionMbr & pmbr, IVector & position, int iDim, Value & value);
  };



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



  template<int dim, int blockSize, class Value>
  inline bool Grid<dim, blockSize, Value>::PositionMbr::operator!=(
    const PositionMbr & b) const
  {
    for (int i = 0; i < dim; i++)
      if (l[i] != b.l[i] || h[i] != b.h[i])
        return false;

    return true;
  }



  template<int dim, int blockSize, class Value>
  inline bool Grid<dim, blockSize, Value>::IVector::operator!=(
    const IVector & b) const
  {
    for (int i = 0; i < dim; i++)
      if (p[i] != b.p[i])
        return false;

    return true;
  }



  template<int dim, int blockSize, class Value>
  inline Grid<dim, blockSize, Value>::IVector::IVector(PositionMbr & pmbr)
  {
    for (int i = 0; i < dim; i++)
      p[i] = pmbr.l[i];
  }




  template<int dim, int blockSize, class Value>
  inline Grid<dim, blockSize, Value>::Grid(IVector splits, DVector cellSize) :
    m_CellSize(cellSize),
    m_Splits(splits)
  {
    int cells = 1;
    for (int i = 0; i < dim; i++) {
      cells *= m_Splits.p[i];
      m_Mbr.l[i] = m_Mbr.h[i] = 0.0;
    }
    
    m_Blocks.resize(cells);
    for (int i = 0; i < cells; i++) {
      m_Blocks[i].nextBlock = -1;
      m_Blocks[i].count = 0;
    }
  }

  template<int dim, int blockSize, class Value>
  inline Grid<dim, blockSize, Value>::Grid(CRelAlgebra::Reader& source)
  {
    m_Blocks.load(source);
    source.ReadOrThrow(reinterpret_cast<char*>(&m_Splits), 
                       sizeof(m_Splits));
    source.ReadOrThrow(reinterpret_cast<char*>(&m_CellSize), 
                       sizeof(m_CellSize));
    source.ReadOrThrow(reinterpret_cast<char*>(&m_Mbr), 
                       sizeof(m_Mbr));
    m_Row.load(source);
    m_Log.load(source);
  }

  template<int dim, int blockSize, class Value>
  inline void Grid<dim, blockSize, Value>::save(CRelAlgebra::Writer &target)
  {
    m_Blocks.save(target);
    target.WriteOrThrow(reinterpret_cast<char*>(&m_Splits), 
                        sizeof(m_Splits));
    target.WriteOrThrow(reinterpret_cast<char*>(&m_CellSize), 
                        sizeof(m_CellSize));
    target.WriteOrThrow(reinterpret_cast<char*>(&m_Mbr), 
                        sizeof(m_Mbr));
    m_Row.save(target);
    m_Log.save(target);
  }

  template<int dim, int blockSize, class Value>
  inline int Grid<dim, blockSize, Value>::savedSize() 
  {
    return m_Blocks.savedSize() +
        sizeof(m_Splits) +
        sizeof(m_CellSize) +
        sizeof(m_Mbr) +
        m_Row.savedSize() +
        m_Log.savedSize();
  }

  template<int dim, int blockSize, class Value>
  inline void Grid<dim, blockSize, Value>::clear()
  {
    m_Blocks.clear();
    m_Log.clear();
    m_Row.clear();
    m_MbrValid = false;
  }
  
  template<int dim, int blockSize, class Value>
  inline void Grid<dim, blockSize, Value>::add(Mbr mbr, Value value)
  {
    if (!m_MbrValid)
      m_Mbr = mbr;
    else
      m_Mbr = m_Mbr.unite(mbr);

    m_MbrValid = true;

    PositionMbr pmbr;
    translateMbrToPositionMbr(mbr, pmbr);
    IVector p(pmbr);
    add(pmbr, p, 0, value);
  }

  template<int dim, int blockSize, class Value>
  inline void Grid<dim, blockSize, Value>::addRow()
  {
    m_Row.push_back(m_Log.size());
  }

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

  template<int dim, int blockSize, class Value>
  inline std::set<Value> Grid<dim, blockSize, Value>::selection(Mbr mbr)
  {
    PositionMbr pmbr;
    translateMbrToPositionMbr(mbr, pmbr);

    IVector p = IVector();
    std::set<Value> s;
    createSelection(pmbr, p, 0, s);

    return s;
  }

  template<int dim, int blockSize, class Value>
  inline typename Grid<dim, blockSize, Value>::Mbr 
  Grid<dim, blockSize, Value>::mbr()
  {
    return m_Mbr;
  }

  template<int dim, int blockSize, class Value>
  inline void Grid<dim, blockSize, Value>::translateMbrToPositionMbr(
    Mbr & mbr, PositionMbr & positionMbr)
  {
    for (int i = 0; i < dim; i++) {
      int s = static_cast<int>(mbr.l[i] / m_CellSize.s[i]);
      int e = static_cast<int>(mbr.h[i] / m_CellSize.s[i]);
      
      positionMbr.l[i] = (s % m_Splits.p[i] + m_Splits.p[i]) % m_Splits.p[i];

      for (int j = 0; j < m_Splits.p[i] - 1 && s != e; j++)
        s++;

      positionMbr.h[i] = (s % m_Splits.p[i] + m_Splits.p[i]) % m_Splits.p[i];
    }
  }

  template<int dim, int blockSize, class Value>
  inline int Grid<dim, blockSize, Value>::blockIndex(IVector position)
  {
    int p = 0;
    for (int i = 0; i < dim; i++)
      p = p * m_Splits.p[i] + position.p[i];

    return p;
  }

  template<int dim, int blockSize, class Value>
  inline void Grid<dim, blockSize, Value>::createSelection(PositionMbr & pmbr, 
    IVector & position, int iDim, std::set<Value> & selection)
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
          selection.insert(b.entries[i]);

        iB = b.nextBlock;
      }
    }
  }

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
