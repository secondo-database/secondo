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

#ifndef TILEALGEBRA_MTCELLITERATOR_H
#define TILEALGEBRA_MTCELLITERATOR_H

/*
TileAlgebra includes

*/

#include "../grid/mtgrid.h"
#include "../Index/Index.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Template class mtCellIterator represents an iteration class
for mt datatype cells.

author: Dirk Zacher

*/

template <typename Type>
class mtCellIterator
{
  public:

  /*
  Constructor mtCellIterator initializes some members of the class
  with corresponding parameter values and all the other members
  of the class with default values.

  author: Dirk Zacher
  parameters: rmt - reference to a mt datatype object
              rX1 - reference to X1 coordinate
              rY1 - reference to Y1 coordinate
              rX2 - reference to X2 coordinate
              rY2 - reference to Y2 coordinate
              rT1 - reference to T1 coordinate
              rT2 - reference to T2 coordinate
  return value: -
  exceptions: -

  */

  mtCellIterator(const Type& rmt,
                 const double& rX1,
                 const double& rY1,
                 const double& rX2,
                 const double& rY2,
                 const double& rT1,
                 const double& rT2);

  /*
  Destructor ~mtCellIterator deinitializes a mtCellIterator object.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  virtual ~mtCellIterator();

  /*
  Method HasNext checks if mtCellIterator object has a next cell to iterate.

  author: Dirk Zacher
  parameters: -
  return value: true, if mtCellIterator object has a next cell to iterate,
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

  pair<double, double> Next();

  private:

  /*
  Method Init initializes mtCellIterator.

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
  Method ComputeNextT calculates next time value.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  void ComputeNextT();

  /*
  Method GetNextValue returns next value of given index.

  author: Dirk Zacher
  parameters: index - index of next value to return.
  return value: next value of given index
  exceptions: -

  */

  double GetNextValue(int index);

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
  Method ShiftT decrements or increments time dimension of current cell index
  dependent on current delta time value.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  void ShiftT();

  /*
  Member m_Grid contains the mtgrid object.

  */

  mtgrid m_Grid;

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
  Member m_dT1 contains the T1 coordinate.

  */

  double m_dT1;

  /*
  Member m_dT2 contains the T2 coordinate.

  */

  double m_dT2;

  /*
  Member m_bHasNextX is true if mtCellIterator has a next cell in x dimension,
  otherwise m_bHasNextX is false.

  */

  bool m_bHasNextX;

  /*
  Member m_bHasNextY is true if mtCellIterator has a next cell in y dimension,
  otherwise m_bHasNextY is false.

  */

  bool m_bHasNextY;

  /*
  Member m_bHasNextT is true if mtCellIterator has a next cell
  in time dimension, otherwise m_bHasNextT is false.

  */

  bool m_bHasNextT;

  /*
  Member m_dNextX contains the next value in x dimension.

  */

  double m_dNextX;

  /*
  Member m_dNextY contains the next value in y dimension.

  */

  double m_dNextY;

  /*
  Member m_dNextT contains the next value in time dimension.

  */

  double m_dNextT;

  /*
  Member m_dLastDelta contains the last delta value.

  */

  double m_dLastDelta;

  /*
  Member m_dCurrentDelta contains the current delta value.

  */

  double m_dCurrentDelta;

  /*
  Member m_LastCellIndex contains the last 3-dimensional cell index.

  */

  Index<3> m_LastCellIndex;

  /*
  Member m_CurrentCellIndex contains the current 3-dimensional cell index.

  */

  Index<3> m_CurrentCellIndex;

  /*
  Member m_dDeltaX contains the delta x value.

  */
  
  double m_dDeltaX;

  /*
  Member m_dDeltaY contains the delta y value.

  */

  double m_dDeltaY;

  /*
  Member m_dDeltaT contains the delta time value.

  */

  double m_dDeltaT;
};

/*
Constructor mtCellIterator inintializes some members of the class
with corresponding parameter values and all the other members
of the class with default values.

author: Dirk Zacher
parameters: rmt - reference to a mt object
            rX1 - reference to X1 coordinate
            rY1 - reference to Y1 coordinate
            rX2 - reference to X2 coordinate
            rY2 - reference to Y2 coordinate
            rT1 - reference to T1 coordinate
            rT2 - reference to T2 coordinate
return value: -
exceptions: -

*/

template <typename Type>
mtCellIterator<Type>::mtCellIterator(const Type& rmt,
                                     const double& rX1,
                                     const double& rY1,
                                     const double& rX2,
                                     const double& rY2,
                                     const double& rT1,
                                     const double& rT2)
                     :m_dX1(rX1),
                      m_dY1(rY1),
                      m_dX2(rX2),
                      m_dY2(rY2),
                      m_dT1(rT1),
                      m_dT2(rT2),
                      m_bHasNextX(false),
                      m_bHasNextY(false),
                      m_bHasNextT(false),
                      m_dNextX(0.0),
                      m_dNextY(0.0),
                      m_dNextT(0.0),
                      m_dLastDelta(0.0),
                      m_dCurrentDelta(0.0)
{
  rmt.getgrid(m_Grid);

  if(rmt.IsValidLocation(m_dX2, m_dY2, m_dT2))
  {
    m_LastCellIndex = rmt.GetLocationIndex(m_dX2, m_dY2, m_dT2);
  }

  if(rmt.IsValidLocation(m_dX1, m_dY1, m_dT1))
  {
    m_CurrentCellIndex = rmt.GetLocationIndex(m_dX1, m_dY1, m_dT1);
  }

  Init();
}

/*
Destructor deinitializes a mtCellIterator object.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

template <typename Type>
mtCellIterator<Type>::~mtCellIterator()
{

}

/*
Method HasNext checks if mtCellIterator object has a next cell to iterate.

author: Dirk Zacher
parameters: -
return value: true, if mtCellIterator object has a next cell to iterate,
              otherwise false
exceptions: -

*/

template <typename Type>
bool mtCellIterator<Type>::HasNext()
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
pair<double, double> mtCellIterator<Type>::Next()
{
  pair<double, double> retVal(m_dLastDelta, m_dCurrentDelta);

  ComputeNextDelta();

  return retVal;
}

/*
Method Init initializes mtCellIterator.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

template <typename Type>
void mtCellIterator<Type>::Init()
{
  if(m_CurrentCellIndex == m_LastCellIndex)
  {
     m_dCurrentDelta = 1.0;
  }

  else
  {
    m_dDeltaX = m_dX2 - m_dX1;
    m_dDeltaY = m_dY2 - m_dY1;
    m_dDeltaT = m_dT2 - m_dT1;

    ComputeNextX();
    ComputeNextY();
    ComputeNextT();
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
void mtCellIterator<Type>::ComputeNextX()
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
void mtCellIterator<Type>::ComputeNextY()
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
Method ComputeNextT calculates next time value.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

template <typename Type>
void mtCellIterator<Type>::ComputeNextT()
{
  m_bHasNextT = m_CurrentCellIndex[2] != m_LastCellIndex[2];

  if(m_bHasNextT)
  {
    double horizontalT = 0.0;

    if(m_dDeltaT < 0.0)
    {
      // bottom border of cell
      horizontalT = m_CurrentCellIndex[2] *
                    m_Grid.GetDuration().ToDouble();
    }

    else
    {
      // right border of cell
      horizontalT = (m_CurrentCellIndex[2] + 1) *
                     m_Grid.GetDuration().ToDouble();
    }

    m_dNextT = (horizontalT - m_dT1) / m_dDeltaT;
  }
}

/*
Method GetNextValue returns next value of given index.

author: Dirk Zacher
parameters: index - index of next value to return.
return value: next value of given index
exceptions: -

*/

template <typename Type>
double mtCellIterator<Type>::GetNextValue(int index)
{
  double dNextValue = -1.0;

  switch(index)
  {
    case 0:  dNextValue = m_dNextX;
             break;
    case 1:  dNextValue = m_dNextY;
             break;
    case 2:  dNextValue = m_dNextT;
             break;
    default: assert(false);
             break;
  }

  return dNextValue;
}

/*
Method ComputeNextDelta calculates next delta values.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

template <typename Type>
void mtCellIterator<Type>::ComputeNextDelta()
{
  m_dLastDelta = m_dCurrentDelta;

  if(m_CurrentCellIndex == m_LastCellIndex)
  {
     m_dCurrentDelta = 1.0;
  }

  else
  {
    assert(m_bHasNextX || m_bHasNextY || m_bHasNextT);

    bool cand[3];
    cand[0] = m_bHasNextX;
    cand[1] = m_bHasNextY;
    cand[2] = m_bHasNextT;

    double minimum = 0.0;
    bool first = true;

    for(int i = 0; i < 3; i++)
    {
      if(cand[i])
      {
        if(first)
        {
           first = false;
           minimum = GetNextValue(i);
        }

        else
        {
          double dv = GetNextValue(i);

          if(dv < minimum)
          {
            // new minimum found
            minimum = dv;

            for(int j = 0; j < i; j++)
            {
              cand[j] = false;
            }
          }
        }
      }
    }

    m_dCurrentDelta = minimum;

    if(cand[0] == true)
    {
      ShiftX();
      ComputeNextX();
    }

    if(cand[1] == true)
    {
      ShiftY();
      ComputeNextY();
    }

    if(cand[2] == true)
    {
      ShiftT();
      ComputeNextT();
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
void mtCellIterator<Type>::ShiftX()
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
void mtCellIterator<Type>::ShiftY()
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

/*
Method ShiftT decrements or increments time dimension of current cell index
dependent on current delta time value.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

template <typename Type>
void mtCellIterator<Type>::ShiftT()
{
  if(m_dDeltaT > 0.0)
  {
    m_CurrentCellIndex.Increment(2);
  }

  else if(m_dDeltaT < 0.0)
  {
    m_CurrentCellIndex.Decrement(2);
  }
}

}

#endif // TILEALGEBRA_MTCELLITERATOR_H
