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

#ifndef TILEALGEBRA_MT_H
#define TILEALGEBRA_MT_H

#include "TypeConstructor.h"
#include "mtProperties.h"
#include "Attribute.h"
#include "../grid/mtgrid.h"
#include "../../Tools/Flob/Flob.h"
#include "../Index.h"
#include "RectangleAlgebra.h"
#include "TemporalAlgebra.h"

namespace TileAlgebra
{

/*
declaration of template class t

*/

template <typename Type, typename Properties = mtProperties<Type> >
class mt : public Attribute
{
  /*
  constructors

  */

  protected:

  mt();

  public:

  mt(bool bDefined);
  mt(const mt& rmt);

  /*
  destructor

  */

  virtual ~mt();

  /*
  operators

  */

  mt& operator=(const mt& rmt);

  /*
  TileAlgebra operator methods

  */

  void atlocation(const double& rX,
                  const double& rY,
                  typename Properties::atlocationType& rValues) const;
  void atlocation(const double& rX,
                  const double& rY,
                  const double& rInstant,
                  typename Properties::TypeProperties::WrapperType& rValue)
                  const;
  void atinstant(const Instant& rInstant,
                 typename Properties::itType& rit) const;
  void atperiods(const Periods& rPeriods,
                 typename Properties::PropertiesType& rmt) const;
  void atrange(const Rectangle<2>& rRectangle,
               typename Properties::PropertiesType& rmt) const;
  void atrange(const Rectangle<2>& rRectangle,
               const double& rInstant1,
               const double& rInstant2,
               typename Properties::PropertiesType& rmt) const;
  void deftime(Periods& rPeriods) const;
  void bbox(typename Properties::bboxType& rBoundingBox) const;
  Type minimum() const;
  Type maximum() const;
  void getgrid(mtgrid& rmtgrid) const;

  /*
  methods

  */

  Index<2> GetLocationIndex(const double& rX, const double& rY) const;
  Index<3> GetLocationIndex(const double& rX, const double& rY,
                            const double& rInstant) const;
  Type GetValue(const Index<3>& rIndex) const;
  bool SetGrid(const mtgrid& rmtgrid);
  bool SetGrid(const double& rX,
               const double& rY,
               const double& rLength,
               const datetime::DateTime& rDuration);
  bool SetValue(const Index<3>& rIndex,
                const Type& rValue,
                bool bSetExtrema);

  protected:

  /*
  internal methods

  */

  
  bool IsValidIndex(const Index<3>& rIndex) const;
  bool IsValidLocation(const double& rX,
                       const double& rY) const;
  bool IsValidLocation(const double& rX,
                       const double& rY,
                       const double& rInstant) const;

  public:

  /*
  override functions from base class Attribute

  */

  virtual bool Adjacent(const Attribute* pAttribute) const;
  virtual Attribute* Clone() const;
  virtual int Compare(const Attribute* pAttribute) const;
  virtual void CopyFrom(const Attribute* pAttribute);
  virtual Flob* GetFLOB(const int i);
  virtual size_t HashValue() const;
  virtual int NumOfFLOBs() const;
  virtual size_t Sizeof() const;

  /*
  The following functions are used to integrate the ~t~
  datatype into secondo.

  */

  static const std::string BasicType();
  static void* Cast(void* pVoid);
  static Word Clone(const ListExpr typeInfo,
                    const Word& rWord);
  static void Close(const ListExpr typeInfo,
                    Word& rWord);
  static Word Create(const ListExpr typeInfo);
  static void Delete(const ListExpr typeInfo,
                     Word& rWord);
  static TypeConstructor GetTypeConstructor();
  static Word In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& rErrorInfo,
                 bool& rCorrect);
  static bool KindCheck(ListExpr type,
                        ListExpr& rErrorInfo);
  static bool Open(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);
  static ListExpr Out(ListExpr typeInfo,
                      Word value);
  static ListExpr Property();
  static bool Save(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);
  static int SizeOfObj();

  protected:

  /*
  members

  */

  mtgrid m_Grid;
  Type m_Minimum;
  Type m_Maximum;
  Flob m_Flob;
};

/*
implementation of template class t

*/

template <typename Type, typename Properties>
mt<Type, Properties>::mt()
                     :Attribute()
{

}

template <typename Type, typename Properties>
mt<Type, Properties>::mt(bool bDefined)
                     :Attribute(bDefined),
                      m_Grid(false),
                      m_Minimum(Properties::TypeProperties::
                                GetUndefinedValue()),
                      m_Maximum(Properties::TypeProperties::
                                GetUndefinedValue()),
                      m_Flob(Properties::GetFlobSize())
{
  int xDimensionSize = Properties::GetXDimensionSize();
  int yDimensionSize = Properties::GetYDimensionSize();
  int tDimensionSize = Properties::GetTDimensionSize();
  Type undefinedValue = Properties::TypeProperties::GetUndefinedValue();
  
  for(int time = 0; time < tDimensionSize; time++)
  {
    for(int row = 0; row < yDimensionSize; row++)
    {
      for(int column = 0; column < xDimensionSize; column++)
      {
        Index<3> indexes = (int[]){column, row, time};
        SetValue(indexes, undefinedValue, false);
      }
    }
  }
}

template <typename Type, typename Properties>
mt<Type, Properties>::mt(const mt<Type, Properties>& rmt)
                     :Attribute(rmt.IsDefined()),
                      m_Grid(rmt.m_Grid),
                      m_Minimum(rmt.m_Minimum),
                      m_Maximum(rmt.m_Maximum),
                      m_Flob(rmt.m_Flob)
{
  
}

template <typename Type, typename Properties>
mt<Type, Properties>::~mt()
{

}

/*
operators

*/

template <typename Type, typename Properties>
mt<Type, Properties>& mt<Type, Properties>::operator=
                                            (const mt<Type, Properties>& rmt)
{
  if(this != &rmt)
  {
    Attribute::operator=(rmt);
    m_Grid = rmt.m_Grid;
    m_Minimum = rmt.m_Minimum;
    m_Maximum = rmt.m_Maximum;

    bool bOK = false;
    bOK = m_Flob.clean();
    assert(bOK);
    bOK = m_Flob.copyFrom(rmt.m_Flob);
    assert(bOK);
  }

  return *this;
}

/*
TileAlgebra operator methods

*/

template <typename Type, typename Properties>
void mt<Type, Properties>::atlocation(const double& rX,
                                      const double& rY,
                                      typename Properties::atlocationType&
                                      rValues) const
{
  rValues.SetDefined(false);

  if(IsValidLocation(rX, rY))
  {
    rValues.SetDefined(true);

    Rectangle<3> boundingBox;
    bbox(boundingBox);
    Instant minimumTime = boundingBox.MinD(2);
    Instant maximumTime = boundingBox.MaxD(2);
    datetime::DateTime duration = m_Grid.GetDuration();

    rValues.StartBulkLoad();

    for(Instant currentTime = minimumTime;
        currentTime < maximumTime;
        currentTime += duration)
    {
      typename Properties::TypeProperties::WrapperType value;
      atlocation(rX, rY, currentTime.ToDouble(), value);

      if(value.IsDefined())
      {
        Interval<Instant> interval(currentTime, currentTime + duration,
                                   true, false);
        rValues.Add(typename Properties::unitType(interval, value, value));
      }
    }

    rValues.EndBulkLoad();

    if(rValues.IsEmpty())
    {
      rValues.SetDefined(false);
    }
  }
}

template <typename Type, typename Properties>
void mt<Type, Properties>::atlocation(const double& rX,
                                      const double& rY,
                                      const double& rInstant,
                                      typename Properties::TypeProperties::
                                      WrapperType& rValue) const
{
  rValue.SetDefined(false);

  if(IsValidLocation(rX, rY, rInstant))
  {
    Index<3> index = GetLocationIndex(rX, rY, rInstant);
    Type value = GetValue(index);
    rValue = Properties::TypeProperties::GetWrappedValue(value);
  }
}

template <typename Type, typename Properties>
void mt<Type, Properties>::atinstant(const Instant& rInstant,
                                     typename Properties::itType& rit) const
{
  rit.SetDefined(true);

  typename Properties::tType t(true);
  t.SetGrid(m_Grid);

  int xDimensionSize = Properties::GetXDimensionSize();
  int yDimensionSize = Properties::GetYDimensionSize();
  int tDimensionSize = Properties::GetTDimensionSize();
  double gridDuration = m_Grid.GetDuration().ToDouble();
  int time = static_cast<int>(rInstant.ToDouble() / gridDuration);
  bool btDefined = false;
  
  if(time < tDimensionSize)
  {
    for(int row = 0; row < yDimensionSize; row++)
    {
      for(int column = 0; column < xDimensionSize; column++)
      {
        Index<3> index3 = (int[]){column, row, time};
        Type value = GetValue(index3);

        if(Properties::TypeProperties::IsUndefinedValue(value) == false)
        {
          btDefined = true;
          Index<2> index2 = (int[]){column, row};
          t.SetValue(index2, value, true);
        }
      }
    }
  }

  if(btDefined == false)
  {
    t.SetDefined(false);
  }

  rit.SetInstant(rInstant);
  rit.SetValues(t);
}

template <typename Type, typename Properties>
void mt<Type, Properties>::atperiods(const Periods& rPeriods,
                                     typename Properties::PropertiesType& rmt)
                                     const
{
  rmt.SetDefined(false);

  if(rPeriods.IsDefined())
  {
    rmt.SetDefined(true);
    bool bOK = rmt.SetGrid(m_Grid);

    if(bOK == true)
    {
      int xDimensionSize = Properties::GetXDimensionSize();
      int yDimensionSize = Properties::GetYDimensionSize();
      int tDimensionSize = Properties::GetTDimensionSize();
      datetime::DateTime gridDuration = m_Grid.GetDuration();
      double duration = gridDuration.ToDouble();

      for(int time = 0; time < tDimensionSize; time++)
      {
        for(int row = 0; row < yDimensionSize; row++)
        {
          for(int column = 0; column < xDimensionSize; column++)
          {
            Index<3> index = (int[]){column, row, time};
            Type value = GetValue(index);

            if(Properties::TypeProperties::IsUndefinedValue(value) == false)
            {
              datetime::DateTime startTime = time * duration;
              datetime::DateTime endTime = (time + 1) * duration;

              Interval<DateTime> timeInterval(startTime, endTime, true, false);

              if(rPeriods.Contains(timeInterval))
              {
                bOK = rmt.SetValue(index, value, true);
              }

              else
              {
                if(rPeriods.Intersects(timeInterval) ||
                   rPeriods.Inside(timeInterval))
                {
                  Range<datetime::DateTime> range(2);
                  rPeriods.Intersection(timeInterval, range);

                  Interval<DateTime> rangeValue(startTime, endTime,
                                                true, false);
                  range.Get(0, rangeValue);
                  datetime::DateTime rangeLength = rangeValue.end -
                                                   rangeValue.start;

                  if(rangeLength >= (gridDuration * 0.5))
                  {
                    bOK = rmt.SetValue(index, value, true);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

template <typename Type, typename Properties>
void mt<Type, Properties>::atrange(const Rectangle<2>& rRectangle,
                                   typename Properties::PropertiesType& rmt)
                                   const
{
  rmt.SetDefined(false);

  if(rRectangle.IsDefined())
  {
    double instant1 = 0;
    double instant2 = (Properties::GetTDimensionSize() - 1) *
                       m_Grid.GetDuration().ToDouble();

    atrange(rRectangle, instant1, instant2, rmt);
  }
}

template <typename Type, typename Properties>
void mt<Type, Properties>::atrange(const Rectangle<2>& rRectangle,
                                   const double& rInstant1,
                                   const double& rInstant2,
                                   typename Properties::PropertiesType& rmt)
                                   const
{
  rmt.SetDefined(false);

  if(rRectangle.IsDefined())
  {
    if(IsValidLocation(rRectangle.MinD(0), rRectangle.MinD(1), rInstant1) &&
       IsValidLocation(rRectangle.MaxD(0), rRectangle.MaxD(1), rInstant2))
    {
      rmt.SetDefined(true);

      double x = m_Grid.GetX();
      double y = m_Grid.GetY();
      double length = m_Grid.GetLength();
      datetime::DateTime gridDuration = m_Grid.GetDuration();
      double duration = gridDuration.ToDouble();
      rmt.SetGrid(m_Grid);

      Index<3> startIndex = GetLocationIndex(rRectangle.MinD(0),
                                             rRectangle.MinD(1),
                                             rInstant1);
      Index<3> endIndex = GetLocationIndex(rRectangle.MaxD(0),
                                           rRectangle.MaxD(1),
                                           rInstant2);

      for(int time = startIndex[2]; time <= endIndex[2]; time++)
      {
        for(int row = startIndex[1]; row <= endIndex[1]; row++)
        {
          for(int column = startIndex[0]; column <= endIndex[0]; column++)
          {
            if(rRectangle.MinD(0) <= (x + column * length) &&
               rRectangle.MaxD(0) >= (x + column * length) &&
               rRectangle.MinD(1) <= (y + row * length) &&
               rRectangle.MaxD(1) >= (y + row * length)&&
               rInstant1 <= (time * duration) &&
               rInstant2 >= (time * duration))
            {
              Index<3> index = (int[]){column, row, time};
              Type value = GetValue(index);

              if(Properties::TypeProperties::IsUndefinedValue(value) == false)
              {
                rmt.SetValue(index, value, true);
              }
            }
          }
        }
      }
    }
  }
}

template <typename Type, typename Properties>
void mt<Type, Properties>::deftime(Periods& rPeriods) const
{
  int xDimensionSize = Properties::GetXDimensionSize();
  int yDimensionSize = Properties::GetYDimensionSize();
  int tDimensionSize = Properties::GetTDimensionSize();
  double duration = m_Grid.GetDuration().ToDouble();
  Periods periods(true);

  periods.StartBulkLoad();

  for(int time = 0; time < tDimensionSize; time++)
  {
    bool bDefined = false;

    for(int row = 0; row < yDimensionSize; row++)
    {
      for(int column = 0; column < xDimensionSize; column++)
      {
        Index<3> indexes = (int[]){column, row, time};
        Type value = GetValue(indexes);

        if(Properties::TypeProperties::IsUndefinedValue(value) == false)
        {
          bDefined = true;
          break;
        }
      }
      
      if(bDefined == true)
      {
        break;
      }
    }

    if(bDefined == true)
    {
      Instant startTime(time * duration);
      Instant endTime((time + 1) * duration);
      periods.Add(Interval<DateTime>(startTime, endTime, true, false));
    }
  }

  periods.EndBulkLoad();
  periods.Merge(rPeriods);
}

template <typename Type, typename Properties>
void mt<Type, Properties>::bbox(typename Properties::bboxType& rBoundingBox)
                           const
{
  double minima[3] = { 0.0, 0.0, 0.0 };
  double maxima[3] = { 0.0, 0.0, 0.0 };

  int xDimensionSize = Properties::GetXDimensionSize();
  int yDimensionSize = Properties::GetYDimensionSize();
  int tDimensionSize = Properties::GetTDimensionSize();
  Type value = Properties::TypeProperties::GetUndefinedValue();

  for(int column = 0; column < xDimensionSize; column++)
  {
    bool bbreak = false;

    for(int row = 0; row < yDimensionSize; row++)
    {
      for(int time = 0; time < tDimensionSize; time++)
      {
        Index<3> indexes = (int[]){column, row, time};
        value = GetValue(indexes);

        if(Properties::TypeProperties::IsUndefinedValue(value) == false)
        {
          minima[0] = m_Grid.GetX() + column * m_Grid.GetLength();
          bbreak = true;
          break;
        }
      }

      if(bbreak == true)
      {
        break;
      }
    }

    if(bbreak == true)
    {
      break;
    }
  }

  for(int column = xDimensionSize - 1; column >= 0; column--)
  {
    bool bbreak = false;

    for(int row = 0; row < yDimensionSize; row++)
    {
      for(int time = 0; time < tDimensionSize; time++)
      {
        Index<3> indexes = (int[]){column, row, time};
        value = GetValue(indexes);

        if(Properties::TypeProperties::IsUndefinedValue(value) == false)
        {
          maxima[0] = m_Grid.GetX() + (column + 1) * m_Grid.GetLength();
          bbreak = true;
          break;
        }
      }

      if(bbreak == true)
      {
        break;
      }
    }

    if(bbreak == true)
    {
      break;
    }
  }

  for(int row = 0; row < yDimensionSize; row++)
  {
    bool bbreak = false;

    for(int column = 0; column < xDimensionSize; column++)
    {
      for(int time = 0; time < tDimensionSize; time++)
      {
        Index<3> indexes = (int[]){column, row, time};
        value = GetValue(indexes);

        if(Properties::TypeProperties::IsUndefinedValue(value) == false)
        {
          minima[1] = m_Grid.GetY() + row * m_Grid.GetLength();
          bbreak = true;
          break;
        }
      }

      if(bbreak == true)
      {
        break;
      }
    }

    if(bbreak == true)
    {
      break;
    }
  }

  for(int row = yDimensionSize - 1; row >= 0; row--)
  {
    bool bbreak = false;

    for(int column = 0; column < xDimensionSize; column++)
    {
      for(int time = 0; time < tDimensionSize; time++)
      {
        Index<3> indexes = (int[]){column, row, time};
        value = GetValue(indexes);

        if(Properties::TypeProperties::IsUndefinedValue(value) == false)
        {
          maxima[1] = m_Grid.GetY() + (row + 1) * m_Grid.GetLength();
          bbreak = true;
          break;
        }
      }

      if(bbreak == true)
      {
        break;
      }
    }

    if(bbreak == true)
    {
      break;
    }
  }

  for(int time = 0; time < tDimensionSize; time++)
  {
    bool bbreak = false;

    for(int column = 0; column < xDimensionSize; column++)
    {
      for(int row = 0; row < yDimensionSize; row++)
      {
        Index<3> indexes = (int[]){column, row, time};
        value = GetValue(indexes);

        if(Properties::TypeProperties::IsUndefinedValue(value) == false)
        {
          minima[2] = time * m_Grid.GetDuration().ToDouble();
          bbreak = true;
          break;
        }
      }

      if(bbreak == true)
      {
        break;
      }
    }

    if(bbreak == true)
    {
      break;
    }
  }

  for(int time = tDimensionSize - 1; time >= 0; time--)
  {
    bool bbreak = false;

    for(int column = 0; column < xDimensionSize; column++)
    {
      for(int row = 0; row < yDimensionSize; row++)
      {
        Index<3> indexes = (int[]){column, row, time};
        value = GetValue(indexes);

        if(Properties::TypeProperties::IsUndefinedValue(value) == false)
        {
          maxima[2] = (time + 1) * m_Grid.GetDuration().ToDouble();
          bbreak = true;
          break;
        }
      }

      if(bbreak == true)
      {
        break;
      }
    }

    if(bbreak == true)
    {
      break;
    }
  }

  rBoundingBox.Set(true, minima, maxima);
}

template <typename Type, typename Properties>
Type mt<Type, Properties>::minimum() const
{
  return m_Minimum;
}

template <typename Type, typename Properties>
Type mt<Type, Properties>::maximum() const
{
  return m_Maximum;
}

template <typename Type, typename Properties>
void mt<Type, Properties>::getgrid(mtgrid& rmtgrid) const
{
  rmtgrid = m_Grid;
}

/*
methods

*/

template <typename Type, typename Properties>
Index<2> mt<Type, Properties>::GetLocationIndex(const double& rX,
                                                const double& rY) const
{
  int xDimensionSize = Properties::GetXDimensionSize();
  int yDimensionSize = Properties::GetYDimensionSize();
  double gridX = m_Grid.GetX();
  double gridY = m_Grid.GetY();
  double gridLength = m_Grid.GetLength();

  int indexX = static_cast<int>((rX - gridX) / gridLength);
  int indexY = static_cast<int>((rY - gridY) / gridLength);

  /*
  special cases for bounding boxes

  */

  if(AlmostEqual((rX - gridX) / gridLength, xDimensionSize * gridLength))
  {
    indexX--;
  }

  if(AlmostEqual((rY - gridY) / gridLength, yDimensionSize * gridLength))
  {
    indexY--;
  }

  assert(indexX < xDimensionSize);
  assert(indexY < yDimensionSize);

  Index<2> locationIndex = (int[]){indexX, indexY};
  return locationIndex;
}

template <typename Type, typename Properties>
Index<3> mt<Type, Properties>::GetLocationIndex(const double& rX,
                                                const double& rY,
                                                const double& rInstant) const
{
  int tDimensionSize = Properties::GetTDimensionSize();
  double gridDuration = m_Grid.GetDuration().ToDouble();
  Index<2> index2 = GetLocationIndex(rX, rY);

  int indexT = static_cast<int>(rInstant / gridDuration);

  /*
  special case for bounding boxes

  */

  if(AlmostEqual(rInstant / gridDuration, tDimensionSize * gridDuration))
  {
    indexT--;
  }

  assert(indexT < tDimensionSize);

  Index<3> locationIndex = (int[]){index2[0], index2[1], indexT};
  return locationIndex;
}

template <typename Type, typename Properties>
Type mt<Type, Properties>::GetValue(const Index<3>& rIndex) const
{
  Type value = Properties::TypeProperties::GetUndefinedValue();

  if(IsDefined() &&
     IsValidIndex(rIndex))
  {
    int xDimensionSize = Properties::GetXDimensionSize();
    int yDimensionSize = Properties::GetYDimensionSize();
    int flobIndex = rIndex[2] * xDimensionSize * yDimensionSize +
                    rIndex[1] * yDimensionSize + rIndex[0];

    bool bOK = m_Flob.read(reinterpret_cast<char*>(&value),
                           sizeof(Type),
                           flobIndex * sizeof(Type));
    assert(bOK);
  }

  return value;
}

template <typename Type, typename Properties>
bool mt<Type, Properties>::SetGrid(const mtgrid& rmtgrid)
{
  bool bRetVal = false;

  if(rmtgrid.IsDefined())
  {
    m_Grid = rmtgrid;
    bRetVal = true;
  }

  return bRetVal;
}

template <typename Type, typename Properties>
bool mt<Type, Properties>::SetGrid(const double& rX,
                                   const double& rY,
                                   const double& rLength,
                                   const datetime::DateTime& rDuration)
{
  bool bRetVal = true;

  m_Grid.SetDefined(true);
  bRetVal &= m_Grid.SetX(rX);
  bRetVal &= m_Grid.SetY(rY);
  bRetVal &= m_Grid.SetLength(rLength);
  bRetVal &= m_Grid.SetDuration(rDuration);

  return bRetVal;
}

template <typename Type, typename Properties>
bool mt<Type, Properties>::SetValue(const Index<3>& rIndex,
                                    const Type& rValue,
                                    bool bSetExtrema)
{
  bool bRetVal = false;

  if(IsDefined() &&
     IsValidIndex(rIndex))
  {
    int xDimensionSize = Properties::GetXDimensionSize();
    int yDimensionSize = Properties::GetYDimensionSize();
    int flobIndex = rIndex[2] * xDimensionSize * yDimensionSize +
                    rIndex[1] * yDimensionSize + rIndex[0];

    bRetVal = m_Flob.write(reinterpret_cast<const char*>(&rValue),
                           sizeof(Type),
                           flobIndex * sizeof(Type));

    if(bSetExtrema == true)
    {
      if(Properties::TypeProperties::IsUndefinedValue(m_Minimum) ||
         rValue < m_Minimum)
      {
        m_Minimum = rValue;
      }

      if(Properties::TypeProperties::IsUndefinedValue(m_Maximum) ||
         rValue > m_Maximum)
      {
        m_Maximum = rValue;
      }
    }
  }

  return bRetVal;
}

/*
internal methods

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::IsValidIndex(const Index<3>& rIndex) const
{
  bool bIsValidIndex = false;

  int xDimensionSize = Properties::GetXDimensionSize();
  int yDimensionSize = Properties::GetYDimensionSize();
  int tDimensionSize = Properties::GetTDimensionSize();

  if(rIndex[0] >= 0 &&
     rIndex[0] < xDimensionSize &&
     rIndex[1] >= 0 &&
     rIndex[1] < yDimensionSize &&
     rIndex[2] >= 0 &&
     rIndex[2] < tDimensionSize)
  {
    bIsValidIndex = true;
  }

  return bIsValidIndex;
}

template <typename Type, typename Properties>
bool mt<Type, Properties>::IsValidLocation(const double& rX,
                                           const double& rY) const
{
  bool bIsValidLocation = false;

  int xDimensionSize = Properties::GetXDimensionSize();
  int yDimensionSize = Properties::GetYDimensionSize();
  double gridX = m_Grid.GetX();
  double gridY = m_Grid.GetY();
  double gridLength = m_Grid.GetLength();

  if(rX >= gridX &&
     rX < (gridX + xDimensionSize * gridLength) &&
     rY >= gridY &&
     rY < (gridY + yDimensionSize * gridLength))
  {
    bIsValidLocation = true;
  }

  return bIsValidLocation;
}

template <typename Type, typename Properties>
bool mt<Type, Properties>::IsValidLocation(const double& rX,
                                           const double& rY,
                                           const double& rInstant) const
{
  bool bIsValidLocation = IsValidLocation(rX, rY);

  if(bIsValidLocation == true)
  {
    int tDimensionSize = Properties::GetTDimensionSize();
    double gridDuration = m_Grid.GetDuration().ToDouble();

    if(rInstant > 0.0 &&
       rInstant < (tDimensionSize * gridDuration))
   {
      bIsValidLocation = true;
   }
  }

  return bIsValidLocation;
}

/*
override functions from base class Attribute

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

template <typename Type, typename Properties>
Attribute* mt<Type, Properties>::Clone() const
{
  Attribute* pAttribute = new mt<Type, Properties>(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

template <typename Type, typename Properties>
int mt<Type, Properties>::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;

  if(pAttribute != 0)
  {
    const mt<Type, Properties>* pmt = dynamic_cast<const mt<Type, Properties>*>
                                      (pAttribute);

    if(pmt != 0)
    {
      bool bIsDefined = IsDefined();
      bool btIsDefined = pmt->IsDefined();

      if(bIsDefined == true)
      {
        if(btIsDefined == true) // defined x defined
        {
          nRetVal = m_Grid.Compare(&(pmt->m_Grid));

          if(nRetVal == 0)
          {
            SmiSize flobSize = Properties::GetFlobSize();
            
            char buffer1[flobSize];
            m_Flob.read(buffer1, flobSize, 0);

            char buffer2[flobSize];
            pmt->m_Flob.read(buffer2, flobSize, 0);

            nRetVal = memcmp(buffer1, buffer2, flobSize);
          }
        }

        else // defined x undefined
        {
          nRetVal = 1;
        }
      }

      else
      {
        if(btIsDefined == true) // undefined x defined
        {
          nRetVal = -1;
        }

        else // undefined x undefined
        {
          nRetVal = 0;
        }
      }
    }
  }

  return nRetVal;
}

template <typename Type, typename Properties>
void mt<Type, Properties>::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const mt<Type, Properties>* pmt = dynamic_cast<const mt<Type, Properties>*>
                                      (pAttribute);

    if(pmt != 0)
    {
      *this = *pmt;
    }
  }
}

template <typename Type, typename Properties>
Flob* mt<Type, Properties>::GetFLOB(const int i)
{ 
  Flob* pFlob = 0;
  int nFlobs = NumOfFLOBs();
  
  if(i >= 0 &&
     i < nFlobs)
  {
    switch(i)
    {
      case 0:   pFlob = &m_Flob;
                break;
                
      default:  break;
    }
  }
  
  return pFlob;
}

template <typename Type, typename Properties>
size_t mt<Type, Properties>::HashValue() const
{
  size_t hashValue = 0;

  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }

  return hashValue;
}

template <typename Type, typename Properties>
int mt<Type, Properties>::NumOfFLOBs() const
{ 
  return 1;
}

template <typename Type, typename Properties>
size_t mt<Type, Properties>::Sizeof() const
{
  return sizeof(mt<Type, Properties>);
}

/*
The following functions are used to integrate the ~t~
datatype into secondo.

*/

template <typename Type, typename Properties>
const std::string mt<Type, Properties>::BasicType()
{
  return Properties::GetTypeName();
}

template <typename Type, typename Properties>
void* mt<Type, Properties>::Cast(void* pVoid)
{
  return new(pVoid)mt<Type, Properties>;
}

template <typename Type, typename Properties>
Word mt<Type, Properties>::Clone(const ListExpr typeInfo,
                                 const Word& rWord)
{
  Word word;

  mt<Type, Properties>* pmt = static_cast<mt<Type, Properties>*>(rWord.addr);

  if(pmt != 0)
  {
    word.addr = new mt<Type, Properties>(*pmt);
    assert(word.addr != 0);
  }

  return word;
}

template <typename Type, typename Properties>
void mt<Type, Properties>::Close(const ListExpr typeInfo,
                                Word& rWord)
{
  mt<Type, Properties>* pmt = static_cast<mt<Type, Properties>*>(rWord.addr);

  if(pmt != 0)
  {
    delete pmt;
    rWord.addr = 0;
  }
}

template <typename Type, typename Properties>
Word mt<Type, Properties>::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new mt<Type, Properties>(true);
  assert(word.addr != 0);

  return word;
}

template <typename Type, typename Properties>
void mt<Type, Properties>::Delete(const ListExpr typeInfo,
                                  Word& rWord)
{
  mt<Type, Properties>* pmt = static_cast<mt<Type, Properties>*>(rWord.addr);

  if(pmt != 0)
  {
    delete pmt;
    rWord.addr = 0;
  }
}

template <typename Type, typename Properties>
TypeConstructor mt<Type, Properties>::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    mt<Type, Properties>::BasicType(), // type name function
    mt<Type, Properties>::Property,    // property function describing signature
    mt<Type, Properties>::Out,         // out function
    mt<Type, Properties>::In,          // in function
    0,                                // save to list function
    0,                                // restore from list function
    mt<Type, Properties>::Create,      // create function
    mt<Type, Properties>::Delete,      // delete function
    mt<Type, Properties>::Open,        // open function
    mt<Type, Properties>::Save,        // save function
    mt<Type, Properties>::Close,       // close function
    mt<Type, Properties>::Clone,       // clone function
    mt<Type, Properties>::Cast,        // cast function
    mt<Type, Properties>::SizeOfObj,   // sizeofobj function
    mt<Type, Properties>::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

template <typename Type, typename Properties>
Word mt<Type, Properties>::In(const ListExpr typeInfo,
                              const ListExpr instance,
                              const int errorPos,
                              ListExpr& rErrorInfo,
                              bool& rCorrect)
{
  Word word;

  NList instanceList(instance);
  rCorrect = false;

  if(instanceList.isAtom() == false)
  {
    NList gridList = instanceList.elem(1);

    if(gridList.length() == 4)
    {
      if(gridList.isReal(1) &&
         gridList.isReal(2) &&
         gridList.isReal(3) &&
         gridList.isReal(4))
      {
        mt<Type, Properties>* pmt = new mt<Type, Properties>(true);

        if(pmt != 0)
        {
          datetime::DateTime duration(gridList.elem(4).realval());
          duration.SetType(datetime::durationtype);

          bool bOK = pmt->SetGrid(gridList.elem(1).realval(),
                                  gridList.elem(2).realval(),
                                  gridList.elem(3).realval(),
                                  duration);

          if(bOK == true)
          {
            instanceList.rest();

            if(instanceList.isEmpty() == false)
            {
              NList sizeList = instanceList.elem(1);

              if(sizeList.length() == 3)
              {
                if(sizeList.isInt(1) &&
                   sizeList.isInt(2) &&
                   sizeList.isInt(3) &&
                   sizeList.elem(1).intval() > 0 &&
                   sizeList.elem(2).intval() > 0 &&
                   sizeList.elem(3).intval() > 0)
                {
                  int sizeX = sizeList.elem(1).intval();
                  int sizeY = sizeList.elem(2).intval();
                  int sizeT = sizeList.elem(3).intval();
                  Cardinal valueListLength = static_cast<Cardinal>
                                             (sizeX * sizeY * sizeT);

                  instanceList.rest();

                  while(bOK &&
                        instanceList.isEmpty() == false)
                  {
                    NList pageList = instanceList.first();

                    if(pageList.length() == 4)
                    {
                      if(pageList.isInt(1) &&
                         pageList.isInt(2) &&
                         pageList.isInt(3))
                      {
                        int indexX = pageList.elem(1).intval();
                        int indexY = pageList.elem(2).intval();
                        int indexT = pageList.elem(3).intval();
                        int xDimensionSize = Properties::GetXDimensionSize();
                        int yDimensionSize = Properties::GetYDimensionSize();
                        int tDimensionSize = Properties::GetTDimensionSize();

                        if(indexX >= 0 &&
                           indexX <= xDimensionSize - sizeX &&
                           indexY >= 0 &&
                           indexY <= yDimensionSize - sizeY &&
                           indexT >= 0 &&
                           indexT <= tDimensionSize - sizeT)
                        {
                          pageList.rest();
                          pageList.rest();
                          pageList.rest();

                          NList valueList = pageList.first();

                          if(valueList.length() == valueListLength)
                          {
                            int listIndex = 0;

                            for(int time = 0; time < sizeT; time++)
                            {
                              for(int row = 0; row < sizeY; row++)
                              {
                                for(int column = 0; column < sizeX; column++)
                                {
                                  listIndex++;

                                  Index<3> index = (int[]){(indexX + column),
                                                           (indexY + row),
                                                           (indexT + time)};
                                  Type value = Properties::TypeProperties::
                                               GetUndefinedValue();

                                  if(valueList.elem(listIndex).
                                     isSymbol(Symbol::UNDEFINED()) == false)
                                  {
                                    if(Properties::TypeProperties::
                                       IsValidValueType
                                       (valueList.elem(listIndex)))
                                    {
                                      value = Properties::TypeProperties::
                                              GetValue(
                                              valueList.elem(listIndex));
                                    }

                                    else
                                    {
                                      bOK = false;
                                      cmsg.inFunError("Type mismatch: "
                                                      "list value in "
                                                      "partial grid has "
                                                      "wrong type.");
                                    }
                                  }

                                  pmt->SetValue(index, value, true);
                                }
                              }
                            }

                            instanceList.rest();
                          }

                          else
                          {
                            bOK = false;
                            cmsg.inFunError("Type mismatch: "
                                            "list for partial grid values "
                                            "is too short or too long.");
                          }
                        }

                        else
                        {
                          bOK = false;
                          cmsg.inFunError("Type mismatch: "
                                          "page list index is "
                                          "out of valid range.");
                        }
                      }

                      else
                      {
                        bOK = false;
                        cmsg.inFunError("Type mismatch: "
                                        "partial grid content must start "
                                        "with three integers.");
                      }
                    }

                    else
                    {
                      bOK = false;
                      cmsg.inFunError("Type mismatch: "
                                      "partial grid content must contain "
                                      "four elements.");
                    }
                  }

                }

                else
                {
                  bOK = false;
                  cmsg.inFunError("Type mismatch: "
                                  "partial grid size must contain "
                                  "two positive integers.");
                }
              }

              else
              {
                bOK = false;
                cmsg.inFunError("Size list must have a length of 3.");
              }
            }
          }

          if(bOK)
          {
            word.addr = pmt;
            rCorrect = true;
          }

          else
          {
            delete pmt;
            pmt = 0;
            rCorrect = false;
          }
        }
      }

      else
      {
        cmsg.inFunError("Type mismatch: expected 4 reals as mtgrid sublist.");
      }
    }

    else
    {
      cmsg.inFunError("Type mismatch: list for mtgrid is too short "
                      "or too long.");
    }
  }

  else
  {
    cmsg.inFunError("Expected list as first element, got an atom.");
  }

  return word;
}

template <typename Type, typename Properties>
bool mt<Type, Properties>::KindCheck(ListExpr type,
                                     ListExpr& rErrorInfo)
{
  bool bRetVal = false;

  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, mt<Type, Properties>::BasicType());
  }

  return bRetVal;
}

template <typename Type, typename Properties>
bool mt<Type, Properties>::Open(SmiRecord& rValueRecord,
                                size_t& rOffset,
                                const ListExpr typeInfo,
                                Word& rValue)
{
  bool bRetVal = OpenAttribute<mt<Type, Properties> >(rValueRecord,
                                                      rOffset,
                                                      typeInfo,
                                                      rValue);

  return bRetVal;
}

template <typename Type, typename Properties>
ListExpr mt<Type, Properties>::Out(ListExpr typeInfo,
                                   Word value)
{
  ListExpr pListExpr = 0;

  if(nl != 0)
  {
    mt<Type, Properties>* pmt = static_cast<mt<Type, Properties>*>(value.addr);

    if(pmt != 0)
    {
      if(pmt->IsDefined() == true)
      {
        NList instanceList;

        NList gridList;
        gridList.append(pmt->m_Grid.GetX());
        gridList.append(pmt->m_Grid.GetY());
        gridList.append(pmt->m_Grid.GetLength());
        gridList.append(pmt->m_Grid.GetDuration().ToDouble());
        instanceList.append(gridList);

        NList sizeList;
        sizeList.append(Properties::GetXDimensionSize());
        sizeList.append(Properties::GetYDimensionSize());
        sizeList.append(Properties::GetTDimensionSize());
        instanceList.append(sizeList);

        NList tintList;
        tintList.append(0);
        tintList.append(0);
        tintList.append(0);

        Type undefinedValue = Properties::TypeProperties::GetUndefinedValue();
        NList valueList;

        for(int i = 0; i < Properties::GetFlobElements(); i++)
        {
          Type value = undefinedValue;

          bool bOK = pmt->m_Flob.read(reinterpret_cast<char*>(&value),
                                      sizeof(Type),
                                      i * sizeof(Type));
          assert(bOK);

          valueList.append(Properties::TypeProperties::ToNList(value));
        }

        tintList.append(valueList);
        instanceList.append(tintList);

        pListExpr = instanceList.listExpr();
      }

      else
      {
        pListExpr = nl->SymbolAtom(Symbol::UNDEFINED());
      }
    }
  }

  return pListExpr;
}

template <typename Type, typename Properties>
ListExpr mt<Type, Properties>::Property()
{
  NList propertyList;

  NList names;
  names.append(NList(std::string("Signature"), true));
  names.append(NList(std::string("Example Type List"), true));
  names.append(NList(std::string("ListRep"), true));
  names.append(NList(std::string("Example List"), true));
  names.append(NList(std::string("Remarks"), true));

  NList values;
  values.append(NList(std::string("-> DATA"), true));
  values.append(NList(BasicType(), true));
  values.append(NList
               (std::string("((x y l t) (szx szy szt) ((ix iy it (v*)))*)"),
                true));
  values.append(NList
               (std::string("((0.0 0.0 1.0 1.0) (1 1 1) ((0 0 0 (0))))"),
                true));
  values.append(NList(std::string(""), true));

  propertyList = NList(names, values);

  return propertyList.listExpr();
}

template <typename Type, typename Properties>
bool mt<Type, Properties>::Save(SmiRecord& rValueRecord,
                                size_t& rOffset,
                                const ListExpr typeInfo,
                                Word& rValue)
{
  bool bRetVal = SaveAttribute<mt<Type, Properties> >(rValueRecord,
                                                      rOffset,
                                                      typeInfo,
                                                      rValue);

  return bRetVal;
}

template <typename Type, typename Properties>
int mt<Type, Properties>::SizeOfObj()
{
  return sizeof(mt<Type, Properties>);
}

}

#endif // TILEALGEBRA_MT_H
