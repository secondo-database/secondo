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

/*
TileAlgebra includes

*/

#include "../Constants.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Template class Index represents an index with Dimension dimensions.

author: Dirk Zacher

*/

template <int Dimension>
class Index
{
  public:

  /*
  Constructor Index initializes all members of the class with default values.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  Index();

  /*
  Constructor Index initializes all members of the class
  with corresponding initial values from given int array.

  author: Dirk Zacher
  parameters: rIndex - reference to an int array
  return value: -
  exceptions: -

  */

  Index(const int (&rIndex)[Dimension]);

  /*
  Constructor Index initializes all members of the class
  with corresponding values of rIndex object.

  author: Dirk Zacher
  parameters: rIndex - reference to an Index<Dimension> object
  return value: -
  exceptions: -

  */

  Index(const Index<Dimension>& rIndex);

  /*
  Destructor ~Index deinitializes an Index object.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  virtual ~Index();

  /*
  Operator[] returns the value of the int array at given index.

  author: Dirk Zacher
  parameters: index - index of the int array 
  return value: reference to the value of the array at given index
  exceptions: -

  */

  const int& operator[] (int) const;

  /*
  Operator< checks if this object is less than rIndex object.

  author: Dirk Zacher
  parameters: rIndex - reference to an Index object
  return value: true, if this object is less than rIndex object,
                otherwise false
  exceptions: -

  */

  bool operator< (const Index<Dimension>& rIndex) const;

  /*
  Operator> checks if this object is greater than rIndex object.

  author: Dirk Zacher
  parameters: rIndex - reference to an Index object
  return value: true, if this object is greater than rIndex object,
                otherwise false
  exceptions: -

  */

  bool operator> (const Index<Dimension>& rIndex) const;

  /*
  Operator== checks if this object equals rIndex object.

  author: Dirk Zacher
  parameters: rIndex - reference to an Index object
  return value: true, if this object equals rIndex object, otherwise false
  exceptions: -

  */

  bool operator==(const Index<Dimension>& rIndex) const;

  /*
  Operator!= checks if this object unequals rIndex object.

  author: Dirk Zacher
  parameters: rIndex - reference to an Index object
  return value: true, if this object unequals rIndex object, otherwise false
  exceptions: -

  */

  bool operator!=(const Index<Dimension>& rIndex) const;

  /*
  Operator<= checks if this object is less equal than rIndex object.

  author: Dirk Zacher
  parameters: rIndex - reference to an Index object
  return value: true, if this object is less equal than rIndex object,
                otherwise false
  exceptions: -

  */

  bool operator<=(const Index<Dimension>& rIndex) const;

  /*
  Operator>= checks if this object is greater equal than rIndex object.

  author: Dirk Zacher
  parameters: rIndex - reference to an Index object
  return value: true, if this object is greater equal than rIndex object,
                otherwise false
  exceptions: -

  */

  bool operator>=(const Index<Dimension>& rIndex) const;

  /*
  Operator+= adds given rIndex object to this object.

  author: Dirk Zacher
  parameters: rIndex - reference to an Index object
  return value: reference to this object
  exceptions: -

  */

  Index<Dimension>& operator+=(const Index<Dimension>& rIndex);

  /*
  Operator-= subtracts given rIndex object from this object.

  author: Dirk Zacher
  parameters: rIndex - reference to an Index object
  return value: reference to this object
  exceptions: -

  */

  Index<Dimension>& operator-=(const Index<Dimension>& rIndex);

  public:

  /*
  Method Decrement decrements the value of the int array at given index.

  author: Dirk Zacher
  parameters: rIndex - index of the int array to decrement
  return value: true, if the value of the int array at rIndex was decremented,
                otherwise false
  exceptions: -

  */

  bool Decrement(const int& rIndex);

  /*
  Method Increment increments the value of the int array at given index.

  author: Dirk Zacher
  parameters: rIndex - index of the int array to increment
  return value: true, if the value of the int array at rIndex was incremented,
                otherwise false
  exceptions: -

  */

  bool Increment(const int& rIndex);

  /*
  Method Set sets the int array at given index to given value.

  author: Dirk Zacher
  parameters: rIndex - index of the int array
              rValue - value of the int array at rIndex.
  return value: true, if rValue was set in int array at rIndex,
                otherwise false
  exceptions: -

  */

  bool Set(const int& rIndex, const int& rValue);

  private:

  /*
  Member m_Index contains all values of all dimensions of an Index.

  */

  int m_Index[Dimension];
};

/*
Constructor Index initializes all members of the class with default values.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

template <int Dimension>
Index<Dimension>::Index()
{
  for(int i = 0; i < Dimension; i++)
  {
    m_Index[i] = 0;
  }
}

/*
Constructor Index initializes all members of the class
with corresponding initial values from given int array.

author: Dirk Zacher
parameters: rIndex - reference to an int array
return value: -
exceptions: -

*/

template <int Dimension>
Index<Dimension>::Index(const int (&rIndex)[Dimension])
{
  for(int i = 0; i < Dimension; i++)
  {
    m_Index[i] = rIndex[i];
  }
}

/*
Constructor Index initializes all members of the class
with corresponding values of rIndex object.

author: Dirk Zacher
parameters: rIndex - reference to an Index<Dimension> object
return value: -
exceptions: -

*/

template <int Dimension>
Index<Dimension>::Index(const Index<Dimension>& rIndex)
{
  for(int i = 0; i < Dimension; i++)
  {
    m_Index[i] = rIndex[i];
  }
}

/*
Destructor deinitializes an Index object.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

template <int Dimension>
Index<Dimension>::~Index()
{

}

/*
Operator[] returns the value of the int array at given index.

author: Dirk Zacher
parameters: index - index of the int array 
return value: reference to the value of the array at given index
exceptions: -

*/

template <int Dimension>
const int& Index<Dimension>::operator[] (int i) const
{
  assert(0 <= i && i < Dimension);

  return m_Index[i];
}

/*
Operator< checks if this object is less than rIndex object.

author: Dirk Zacher
parameters: rIndex - reference to an Index object
return value: true, if this object is less than rIndex object,
              otherwise false
exceptions: -

*/

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

/*
Operator> checks if this object is greater than rIndex object.

author: Dirk Zacher
parameters: rIndex - reference to an Index object
return value: true, if this object is greater than rIndex object,
              otherwise false
exceptions: -

*/

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

/*
Operator== checks if this object equals rIndex object.

author: Dirk Zacher
parameters: rIndex - reference to an Index object
return value: true, if this object equals rIndex object, otherwise false
exceptions: -

*/

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

/*
Operator!= checks if this object unequals rIndex object.

author: Dirk Zacher
parameters: rIndex - reference to an Index object
return value: true, if this object unequals rIndex object, otherwise false
exceptions: -

*/

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

/*
Operator<= checks if this object is less equal than rIndex object.

author: Dirk Zacher
parameters: rIndex - reference to an Index object
return value: true, if this object is less equal than rIndex object,
              otherwise false
exceptions: -

*/

template <int Dimension>
bool Index<Dimension>::operator<=(const Index<Dimension>& rIndex) const
{
  bool bRetVal = false;

  bRetVal |= (*this < rIndex);
  bRetVal |= (*this == rIndex);

  return bRetVal;
}

/*
Operator>= checks if this object is greater equal than rIndex object.

author: Dirk Zacher
parameters: rIndex - reference to an Index object
return value: true, if this object is greater equal than rIndex object,
              otherwise false
exceptions: -

*/

template <int Dimension>
bool Index<Dimension>::operator>=(const Index<Dimension>& rIndex) const
{
  bool bRetVal = false;

  bRetVal |= (*this > rIndex);
  bRetVal |= (*this == rIndex);

  return bRetVal;
}

/*
Operator+= adds given rIndex object to this object.

author: Dirk Zacher
parameters: rIndex - reference to an Index object
return value: reference to this object
exceptions: -

*/

template <int Dimension>
Index<Dimension>& Index<Dimension>::operator+=(const Index<Dimension>& rIndex)
{
  for(int i = 0; i < Dimension; i++)
  {
    m_Index[i] += rIndex.m_Index[i];
  }

  return *this;
}

/*
Operator-= subtracts given rIndex object from this object.

author: Dirk Zacher
parameters: rIndex - reference to an Index object
return value: reference to this object
exceptions: -

*/

template <int Dimension>
Index<Dimension>& Index<Dimension>::operator-=(const Index<Dimension>& rIndex)
{
  for(int i = 0; i < Dimension; i++)
  {
    m_Index[i] -= rIndex.m_Index[i];
  }

  return *this;
}

/*
Method Decrement decrements the value of the int array at given index.

author: Dirk Zacher
parameters: rIndex - index of the int array to decrement
return value: true, if the value of the int array at rIndex was decremented,
              otherwise false
exceptions: -

*/

template <int Dimension>
bool Index<Dimension>::Decrement(const int& rIndex)
{
  bool bRetVal = false;

  if(rIndex < Dimension)
  {
    m_Index[rIndex]--;
    bRetVal = true;
  }

  return bRetVal;
}

/*
Method Increment increments the value of the int array at given index.

author: Dirk Zacher
parameters: rIndex - index of the int array to increment
return value: true, if the value of the int array at rIndex was incremented,
              otherwise false
exceptions: -

*/

template <int Dimension>
bool Index<Dimension>::Increment(const int& rIndex)
{
  bool bRetVal = false;

  if(rIndex < Dimension)
  {
    m_Index[rIndex]++;
    bRetVal = true;
  }

  return bRetVal;
}

/*
Method Set sets the int array at given index to given value.

author: Dirk Zacher
parameters: rIndex - index of the int array
            rValue - value of the int array at rIndex.
return value: true, if rValue was set in int array at rIndex,
              otherwise false
exceptions: -

*/

template <int Dimension>
bool Index<Dimension>::Set(const int& rIndex,
                           const int& rValue)
{
  bool bRetVal = false;

  if(rIndex < Dimension)
  {
    m_Index[rIndex] = rValue;
    bRetVal = true;
  }

  return bRetVal;
}

}

#endif /* TILEALGEBRA_INDEX_H */
