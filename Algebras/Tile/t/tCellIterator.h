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

/*
TileAlgebra includes

*/

#include "../grid/tgrid.h"
#include "../Index/Index.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Template class tCellIterator represents an iteration class
for t datatype cells.

author: Dirk Zacher

*/

template <typename Type>
class tCellIterator
{
  public:

  /*
  Constructor tCellIterator initializes some members of the class
  with corresponding parameter values and all the other members
  of the class with default values.

  author: Dirk Zacher
  parameters: rt - reference to a t datatype object
              rX1 - reference to X1 coordinate
              rY1 - reference to Y1 coordinate
              rX2 - reference to X2 coordinate
              rY2 - reference to Y2 coordinate
  return value: -
  exceptions: -

  */

  tCellIterator(const Type& rt,
                const double& rX1,
                const double& rY1,
                const double& rX2,
                const double& rY2);

  /*
  Destructor ~tCellIterator deinitializes a tCellIterator object.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  virtual ~tCellIterator();

  /*
  Method HasNext checks if tCellIterator object has a next cell to iterate.

  author: Dirk Zacher
  parameters: -
  return value: true, if tCellIterator object has a next cell to iterate,
                otherwise false
  exceptions: -

  */

  bool HasNext();

  /*
  Method Next returns a pair of last delta value and current delta value
  and calculates next delta values.

  author: Dirk Zacher
  parameters: -
  return value: a pair of last delta value and current delta value
  exceptions: -

  */

  pair<double,double> Next();

  private:

  /*
  Method Init initializes tCellIterator.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  void Init();

  /*
  Method ComputeNextX calculates next x value.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  void ComputeNextX();

  /*
  Method ComputeNextY calculates next y value.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  void ComputeNextY();

  /*
  Method ComputeNextDelta calculates next delta values.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  void ComputeNextDelta();

  /*
  Method ShiftX decrements or increments x dimension of current cell index
  dependent on current delta x value.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  void ShiftX();

  /*
  Method ShiftY decrements or increments y dimension of current cell index
  dependent on current delta y value.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  void ShiftY();

  /*
  Member m_Grid contains the tgrid object.

  */

  tgrid m_Grid;

  /*
  Member m_dX1 contains the X1 coordinate.

  */

  double m_dX1;

  /*
  Member m_dY1 contains the Y1 coordinate.

  */

  double m_dY1;

  /*
  Member m_dX2 contains the X2 coordinate.

  */

  double m_dX2;

  /*
  Member m_dY2 contains the Y2 coordinate.

  */

  double m_dY2;

  /*
  Member m_bHasNextX is true if tCellIterator has a next cell in x dimension,
  otherwise m_bHasNextX is false.

  */

  bool m_bHasNextX;

  /*
  Member m_bHasNextY is true if tCellIterator has a next cell in y dimension,
  otherwise m_bHasNextY is false.

  */

  bool m_bHasNextY;

  /*
  Member m_dNextX contains the next value in x dimension.

  */

  double m_dNextX;

  /*
  Member m_dNextY contains the next value in y dimension.

  */

  double m_dNextY;

  /*
  Member m_dLastDelta contains the last delta value.

  */

  double m_dLastDelta;

  /*
  Member m_dCurrentDelta contains the current delta value.

  */

  double m_dCurrentDelta;

  /*
  Member m_LastCellIndex contains the last 2-dimensional cell index.

  */

  Index<2> m_LastCellIndex;

  /*
  Member m_CurrentCellIndex contains the current 2-dimensional cell index.

  */

  Index<2> m_CurrentCellIndex;

  /*
  Member m_dDeltaX contains the delta x value.

  */

  double m_dDeltaX;

  /*
  Member m_dDeltaY contains the delta y value.

  */

  double m_dDeltaY;
};

/*
Constructor tCellIterator initializes some members of the class
with corresponding parameter values and all the other members
of the class with default values.

author: Dirk Zacher
parameters: rt - reference to a t datatype object
            rX1 - reference to X1 coordinate
            rY1 - reference to Y1 coordinate
            rX2 - reference to X2 coordinate
            rY2 - reference to Y2 coordinate
return value: -
exceptions: -

*/

template <typename Type>
tCellIterator<Type>::tCellIterator(const Type& rt,
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
  rt.getgrid(m_Grid);

  if(rt.IsValidLocation(m_dX1, m_dY1))
  {
    m_CurrentCellIndex = rt.GetLocationIndex(m_dX1, m_dY1);
  }

  if(rt.IsValidLocation(m_dX2, m_dY2))
  {
    m_LastCellIndex = rt.GetLocationIndex(m_dX2, m_dY2);
  }

  Init();
}

/*
Destructor deinitializes a tCellIterator object.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

template <typename Type>
tCellIterator<Type>::~tCellIterator()
{

}

/*
Method HasNext checks if tCellIterator object has a next cell to iterate.

author: Dirk Zacher
parameters: -
return value: true, if tCellIterator object has a next cell to iterate,
              otherwise false
exceptions: -

*/

template <typename Type>
bool tCellIterator<Type>::HasNext()
{
  return m_dLastDelta < 1.0;
}

/*
Method Next returns a pair of last delta value and current delta value
and calculates next delta values.

author: Dirk Zacher
parameters: -
return value: a pair of last delta value and current delta value
exceptions: -

*/

template <typename Type>
pair<double,double> tCellIterator<Type>::Next()
{
  pair<double,double> retVal(m_dLastDelta, m_dCurrentDelta);

  ComputeNextDelta();

  return retVal;
}

/*
Method Init initializes tCellIterator.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

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

/*
Method ComputeNextX calculates next x value.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

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

/*
Method ComputeNextY calculates next y value.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

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

/*
Method ComputeNextDelta calculates next delta values.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

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

/*
Method ShiftX decrements or increments x dimension of current cell index
dependent on current delta x value.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

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

/*
Method ShiftY decrements or increments y dimension of current cell index
dependent on current delta y value.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

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
