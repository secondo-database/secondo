/*
----
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
----

*/

#ifndef TILEALGEBRA_INDEX_H
#define TILEALGEBRA_INDEX_H

namespace TileAlgebra
{

template <int Dimension>
class Index
{
  public:

  /*
  constructors

  */

  Index();
  Index(const int (&)[Dimension]);

  /*
  destructor

  */

  virtual ~Index();

  /*
  operators

  */

  const int& operator[] (int) const;
  bool operator< (const Index<Dimension>& rIndex) const;
  bool operator> (const Index<Dimension>& rIndex) const;
  bool operator==(const Index<Dimension>& rIndex) const;
  bool operator!=(const Index<Dimension>& rIndex) const;
  bool operator<=(const Index<Dimension>& rIndex) const;
  bool operator>=(const Index<Dimension>& rIndex) const;
  Index<Dimension>& operator+=(const Index<Dimension>& rIndex);
  Index<Dimension>& operator-=(const Index<Dimension>& rIndex);

  private:

  /*
  members

  */

  int m_Index[Dimension];
};

template <int Dimension>
Index<Dimension>::Index()
{
  for(int i = 0; i < Dimension; i++)
  {
    m_Index[i] = 0;
  }
}

template <int Dimension>
Index<Dimension>::Index(const int (&rIndex)[Dimension])
{
  for(int i = 0; i < Dimension; i++)
  {
    m_Index[i] = rIndex[i];
  }
}

template <int Dimension>
Index<Dimension>::~Index()
{

}

template <int Dimension>
const int& Index<Dimension>::operator[] (int i) const
{
  assert(0 <= i && i < Dimension);

  return m_Index[i];
}

template <int Dimension>
bool Index<Dimension>::operator<(const Index<Dimension>& rIndex) const
{
  bool bRetVal = false;

  if(this != &rIndex)
  {
    for(int i = 0; i < Dimension; i++)
    {
      if(m_Index[i] < rIndex.m_Index[i])
      {
        bRetVal = true;
        break;
      }

      if(m_Index[i] > rIndex.m_Index[i])
      {
        break;
      }
    }
  }

  return bRetVal;
}

template <int Dimension>
bool Index<Dimension>::operator>(const Index<Dimension>& rIndex) const
{
  bool bRetVal = false;

  if(this != &rIndex)
  {
    for(int i = 0; i < Dimension; i++)
    {
      if(m_Index[i] > rIndex.m_Index[i])
      {
        bRetVal = true;
        break;
      }

      if(m_Index[i] < rIndex.m_Index[i])
      {
        break;
      }
    }
  }

  return bRetVal;
}

template <int Dimension>
bool Index<Dimension>::operator==(const Index<Dimension>& rIndex) const
{
  bool bRetVal = true;

  if(this != &rIndex)
  {
    for(int i = 0; i < Dimension; i++)
    {
      if(m_Index[i] != rIndex[i])
      {
        bRetVal = false;
        break;
      }
    }
  }

  return bRetVal;
}

template <int Dimension>
bool Index<Dimension>::operator!=(const Index<Dimension>& rIndex) const
{
  bool bRetVal = false;

  if(this != &rIndex)
  {
    bRetVal = (!(*this == rIndex));
  }

  return bRetVal;
}

template <int Dimension>
bool Index<Dimension>::operator<=(const Index<Dimension>& rIndex) const
{
  bool bRetVal = false;

  bRetVal |= (*this < rIndex);
  bRetVal |= (*this == rIndex);

  return bRetVal;
}

template <int Dimension>
bool Index<Dimension>::operator>=(const Index<Dimension>& rIndex) const
{
  bool bRetVal = false;

  bRetVal |= (*this > rIndex);
  bRetVal |= (*this == rIndex);

  return bRetVal;
}

template <int Dimension>
Index<Dimension>& Index<Dimension>::operator+=(const Index<Dimension>& rIndex)
{
  for(int i = 0; i < Dimension; i++)
  {
    m_Index[i] += rIndex.m_Index[i];
  }

  return *this;
}

template <int Dimension>
Index<Dimension>& Index<Dimension>::operator-=(const Index<Dimension>& rIndex)
{
  for(int i = 0; i < Dimension; i++)
  {
    m_Index[i] -= rIndex.m_Index[i];
  }

  return *this;
}

}

#endif /* TILEALGEBRA_INDEX_H */
