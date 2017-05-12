/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

#include "Strings.h"

#include <algorithm>
#include <cstring>
#include <functional>
#include "FTextAlgebra.h"
#include <limits>
#include "StandardTypes.h"

using namespace CRelAlgebra;

using std::hash;
using std::min;
using std::numeric_limits;
using std::string;
using std::strncmp;

//StringEntry-------------------------------------------------------------------

template<bool text>
StringEntry<text>::StringEntry()
{
}

template<bool text>
StringEntry<text>::StringEntry(const char *data, size_t size) :
  data(data),
  size(size)
{
}

template<bool text>
bool StringEntry<text>::IsDefined() const
{
  return size > 0;
}

template<bool text>
int StringEntry<text>::Compare(const StringEntry<text> &value) const
{
  const size_t sizeA = size,
    sizeB = value.size;

  if (sizeA > sizeB)
  {
    return sizeB == 0 ? 1 : strcmp(data, value.data);
  }
  else
  {
    return sizeA == 0 ? sizeB == 0 ? 0 : -1 : strcmp(data, value.data);
  }
}

template<bool text>
int StringEntry<text>::Compare(Attribute &value) const
{
  CcString *stringValue;
  FText *textValue;

  if (text)
  {
    textValue = (FText*)&value;
  }
  else
  {
    stringValue = (CcString*)&value;
  }

  const bool defined = text ? textValue->IsDefined() : stringValue->IsDefined();

  if (defined)
  {
    if (size == 0)
    {
      return -1;
    }

    return strcmp(data, text ? textValue->Get() : *stringValue->GetStringval());
  }

  return size;
}

template<bool text>
bool StringEntry<text>::Equals(const StringEntry<text> &value) const
{
  return Compare(value) == 0;
}

template<bool text>
bool StringEntry<text>::Equals(Attribute &value) const
{
  return Compare(value) == 0;
}

template<bool text>
size_t StringEntry<text>::GetHash() const
{
  size_t h = 0;

  for (size_t i = 0; i < size; ++i)
  {
    h = 5 * h + data[i];
  }

  return h;
}

template<bool text>
size_t StringEntry<text>::GetSize() const
{
  return size;
}

template<bool text>
const char *StringEntry<text>::GetData() const
{
  return data;
}

template<bool text>
StringEntry<text>::operator const char * () const
{
  return data;
}

//Strings----------------------------------------------------------------------

template<bool text>
Strings<text>::Strings()
{
}

template<bool text>
Strings<text>::Strings(Reader &source) :
  SimpleVSAttrArray<StringEntry<text>>(source)
{
}

template<bool text>
Strings<text>::Strings(Reader &source, size_t rowCount) :
  SimpleVSAttrArray<StringEntry<text>>(source, rowCount)
{
}

template<bool text>
void Strings<text>::Append(Attribute &value)
{
  CcString *stringValue;
  FText *textValue;

  if (text)
  {
    textValue = (FText*)&value;
  }
  else
  {
    stringValue = (CcString*)&value;
  }

  if (text ? textValue->IsDefined() : stringValue->IsDefined())
  {
    const char *data = text ? textValue->Get() : *stringValue->GetStringval();
    const size_t size = text ? textValue->Length() : stringValue->Length();

    Append(StringEntry<text>(data, size + 1));
  }
  else
  {
    Append(StringEntry<text>(nullptr, 0));
  }
}

template<bool text>
Attribute *Strings<text>::GetAttribute(size_t row, bool clone) const
{
  const StringEntry<text> &entry = this->GetAt(row);

  if (text)
  {
    return entry.IsDefined() ? new FText(true, entry.data) : new FText(false);
  }

  return entry.IsDefined() ? new CcString(true, entry.data) :
                             new CcString(false);
}

template class Strings<true>;
template class Strings<false>;