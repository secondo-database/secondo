
/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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

#ifndef TILEALGEBRA_TCELLITERATOR_H
#define TILEALGEBRA_TCELLITERATOR_H

#include "../Index/Index.h"
#include "../grid/tgrid.h"

namespace TileAlgebra
{

/*
declaration of template class tCellIterator

*/

template <typename Type>
class tCellIterator
{
  public:

  /*
    constructors

  */

  tCellIterator(const Type& rType,
                const double& rX1,
                const double& rY1,
                const double& rX2,
                const double& rY2);

  /*
  destructor

  */

  virtual ~tCellIterator();

  /*
  methods

  */

  bool HasNext();
  pair<double,double> Next();

  private:

  /*
  internal methods

  */

  void Init();
  void ComputeNextX();
  void ComputeNextY();
  void ComputeNextDelta();
  void ShiftX();
  void ShiftY();

  /*
  members

  */

  tgrid m_Grid;
  double m_dX1;
  double m_dY1;
  double m_dX2;
  double m_dY2;
  bool m_bHasNextX;
  bool m_bHasNextY;
  double m_dNextX;
  double m_dNextY;
  double m_dLastDelta;
  double m_dCurrentDelta;
  Index<2> m_CurrentCellIndex;
  Index<2> m_LastCellIndex;
  double m_dDeltaX;
  double m_dDeltaY;
};

/*
implementation of template class tCellIterator

*/

template <typename Type>
tCellIterator<Type>::tCellIterator(const Type& rType,
                                   const double& rX1,
                                   const double& rY1,
                                   const double& rX2,
                                   const double& rY2)
                    :m_dX1(rX1),
                     m_dY1(rY1),
                     m_dX2(rX2),
                     m_dY2(rY2),
                     m_bHasNextX(false),
                     m_bHasNextY(false),
                     m_dNextX(0.0),
                     m_dNextY(0.0),
                     m_dLastDelta(0.0),
                     m_dCurrentDelta(0.0)
{
  rType.getgrid(m_Grid);
  m_CurrentCellIndex = rType.GetLocationIndex(m_dX1, m_dY1);
  m_LastCellIndex = rType.GetLocationIndex(m_dX2, m_dY2);

  Init();
}

template <typename Type>
tCellIterator<Type>::~tCellIterator()
{

}

template <typename Type>
bool tCellIterator<Type>::HasNext()
{
  return m_dLastDelta < 1.0;
}

template <typename Type>
pair<double,double> tCellIterator<Type>::Next()
{
  pair<double,double> retVal(m_dLastDelta, m_dCurrentDelta);

  ComputeNextDelta();

  return retVal;
}

template <typename Type>
void tCellIterator<Type>::Init()
{
  if(m_CurrentCellIndex == m_LastCellIndex)
  {
     m_dCurrentDelta = 1.0;
  }

  else
  {
    m_dDeltaX = m_dX2 - m_dX1;
    m_dDeltaY = m_dY2 - m_dY1;

    ComputeNextX();
    ComputeNextY();
  }
}

template <typename Type>
void tCellIterator<Type>::ComputeNextX()
{
  m_bHasNextX = m_CurrentCellIndex[0] != m_LastCellIndex[0];

  if(m_bHasNextX)
  {
    double verticalX = 0.0;

    if(m_dDeltaX < 0.0)
    {
      // left border of cell
      verticalX = m_Grid.GetX() +
                  m_CurrentCellIndex[0] * m_Grid.GetLength();
    }

    else
    {
      // right border of cell
      verticalX = m_Grid.GetX() +
                  (m_CurrentCellIndex[0] + 1) * m_Grid.GetLength();
    }

    m_dNextX = (verticalX - m_dX1)/ m_dDeltaX;
  }
}

template <typename Type>
void tCellIterator<Type>::ComputeNextY()
{
  m_bHasNextY = m_CurrentCellIndex[1] != m_LastCellIndex[1];

  if(m_bHasNextY)
  {
    double horizontalY = 0.0;

    if(m_dDeltaY < 0.0)
    {
      // bottom border of cell
      horizontalY = m_Grid.GetY() +
                    m_CurrentCellIndex[1] * m_Grid.GetLength();
    }

    else
    {
      // right border of cell
      horizontalY = m_Grid.GetY() +
                    (m_CurrentCellIndex[1] + 1) * m_Grid.GetLength();
    }

    m_dNextY = (horizontalY - m_dY1) / m_dDeltaY;
  }
}

template <typename Type>
void tCellIterator<Type>::ComputeNextDelta()
{
  m_dLastDelta = m_dCurrentDelta;

  if(m_CurrentCellIndex == m_LastCellIndex)
  {
    m_dCurrentDelta = 1.0;
  }

  else
  {
    if(m_bHasNextX)
    {
      if(m_bHasNextY)
      {
        // x and y deltas are present
        if(m_dNextX == m_dNextY)
        {
          m_dCurrentDelta = m_dNextX;
          ShiftX();
          ShiftY();
          ComputeNextX();
          ComputeNextY();
        }

        else if(m_dNextX < m_dNextY)
        {
          // shift in x direction
          m_dCurrentDelta = m_dNextX;
          ShiftX();
          ComputeNextX();
        }

        else
        {
          // shift in y direction
          m_dCurrentDelta = m_dNextY;
          ShiftY();
          ComputeNextY();
        }
      }

      else
      { // only x delta is present
        m_dCurrentDelta = m_dNextX;
        ShiftX();
        ComputeNextX();
      }
    }

    else
    {
      assert(m_bHasNextY);
      m_dCurrentDelta = m_dNextY;
      ShiftY();
      ComputeNextY();
    }
  }
}

template <typename Type>
void tCellIterator<Type>::ShiftX()
{
  if(m_dDeltaX > 0.0)
  {
   m_CurrentCellIndex.Increment(0);
  }

  else if(m_dDeltaX < 0.0)
  {
   m_CurrentCellIndex.Decrement(0);
  }
}

template <typename Type>
void tCellIterator<Type>::ShiftY()
{
  if(m_dDeltaY > 0.0)
  {
    m_CurrentCellIndex.Increment(1);
  }

  else if(m_dDeltaY < 0.0)
  {
    m_CurrentCellIndex.Decrement(1);
  }
}

}

#endif // TILEALGEBRA_TCELLITERATOR_H
