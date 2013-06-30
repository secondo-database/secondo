
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

#include "../Index.h"
#include "../grid/mtgrid.h"

namespace TileAlgebra
{

/*
declaration of template class mtCellIterator

*/

template <typename Type>
class mtCellIterator
{
  public:

  /*
  constructors

  */

  mtCellIterator(const Type& rType,
                 const double& rX1,
                 const double& rY1,
                 const double& rX2,
                 const double& rY2,
                 const double& rT1,
                 const double& rT2);

  /*
  destructor

  */

  virtual ~mtCellIterator();

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
  void ComputeNextT();
  double GetDV(int index);
  void ComputeNextDelta();
  void ShiftX();
  void ShiftY();
  void ShiftT();

  /*
  members

  */

  mtgrid m_Grid;
  double m_dX1;
  double m_dY1;
  double m_dX2;
  double m_dY2;
  double m_dT1;
  double m_dT2;
  bool m_bHasNextX;
  bool m_bHasNextY;
  bool m_bHasNextT;
  double m_dNextX;
  double m_dNextY;
  double m_dNextT;
  double m_dLastDelta;
  double m_dCurrentDelta;
  Index<3> m_CurrentCellIndex;
  Index<3> m_LastCellIndex;
  double m_dDeltaX;
  double m_dDeltaY;
  double m_dDeltaT;
};

/*
implementation of template class mtCellIterator

*/

template <typename Type>
mtCellIterator<Type>::mtCellIterator(const Type& rType,
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
  rType.getgrid(m_Grid);
  m_CurrentCellIndex = rType.GetLocationIndex(m_dX1, m_dY1, m_dT1);
  m_LastCellIndex = rType.GetLocationIndex(m_dX2, m_dY2, m_dT2);

  Init();
}

template <typename Type>
mtCellIterator<Type>::~mtCellIterator()
{

}

template <typename Type>
bool mtCellIterator<Type>::HasNext()
{
  return m_dLastDelta < 1.0;
}

template <typename Type>
pair<double,double> mtCellIterator<Type>::Next()
{
  pair<double,double> retVal(m_dLastDelta, m_dCurrentDelta);

  ComputeNextDelta();

  return retVal;
}

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

template <typename Type>
double mtCellIterator<Type>::GetDV(int index)
{
  double dRetVal = -1.0;

  switch(index)
  {
    case 0:  dRetVal = m_dNextX;
             break;
    case 1:  dRetVal = m_dNextY;
             break;
    case 2:  dRetVal = m_dNextT;
             break;
    default: assert(false);
             break;
  }

  return dRetVal;
}

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
           minimum = GetDV(i);
        }

        else
        {
          double dv = GetDV(i);

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
