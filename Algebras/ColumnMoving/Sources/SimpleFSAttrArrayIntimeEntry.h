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

namespace ColumnMovingAlgebra
{
  template<class InternalType, class ExternalType>
  class SimpleFSAttrArrayIntimeEntry
  {
  public:
    typedef ExternalType AttributeType;

    static const bool isPrecise = true;

    SimpleFSAttrArrayIntimeEntry() = default;
    SimpleFSAttrArrayIntimeEntry(bool defined, int64_t time, 
                                 InternalType value);

    bool IsDefined() const;
    int64_t GetTime() const;
    InternalType GetValue() const;

  protected:

    bool m_Defined;
    int64_t m_Time;
    InternalType m_Value;
  };



  template<class InternalType, class ExternalType>
  inline SimpleFSAttrArrayIntimeEntry<InternalType, ExternalType>::
  SimpleFSAttrArrayIntimeEntry(bool defined, int64_t time, 
                               InternalType value) :
    m_Defined(defined),
    m_Time(time),
    m_Value(value)
  {
  }

  template<class InternalType, class ExternalType>
  bool SimpleFSAttrArrayIntimeEntry<InternalType, ExternalType>::
  IsDefined() const
  {
    return m_Defined;
  }

  template<class InternalType, class ExternalType>
  int64_t SimpleFSAttrArrayIntimeEntry<InternalType, ExternalType>::
  GetTime() const {
    return m_Time;
  }

  template<class InternalType, class ExternalType>
  InternalType SimpleFSAttrArrayIntimeEntry<InternalType, ExternalType>::
  GetValue() const {
    return m_Value;
  }

}
