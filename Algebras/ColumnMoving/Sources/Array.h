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

1 Array.h

*/

#pragma once

#include "Algebras/CRel/ReadWrite.h"
#include <vector>

namespace ColumnMovingAlgebra {

/*
1.1 Declaration of class ~Array~

This class extends ~std::vector~ by functions needed by attribut arrays for
persistant storage.

*/

  template<class T>
  class Array : public std::vector<T> {
  public:
    Array() = default;
    Array(size_t count);
    Array(CRelAlgebra::Reader& source);
    Array(CRelAlgebra::Reader& source, size_t count);

    void load(CRelAlgebra::Reader& source);
    void save(CRelAlgebra::Writer &target, bool includeHeader = true);
    int savedSize();
    int savedSize(int count);
  };
  
/*
1.1 Implementation of class ~Array~

Constructor with size argument

*/

  template<class T>
  inline Array<T>::Array(size_t count) : std::vector<T>(count) 
  {
  }

/*
constructor loading the array from persistant storage.

*/

  template<class T>
  inline Array<T>::Array(CRelAlgebra::Reader& source)
  {
    size_t count;
    source.ReadOrThrow(reinterpret_cast<char*>(&count), sizeof(size_t));
    std::vector<T>::resize(count);
    source.ReadOrThrow(reinterpret_cast<char*>(std::vector<T>::data()), 
                                               count * sizeof(T));
  }

/*
constructor loading the array with known size from persistant storage-

*/

  template<class T>
  inline Array<T>::Array(CRelAlgebra::Reader& source, size_t count)
  {
    std::vector<T>::resize(count);
    source.ReadOrThrow(reinterpret_cast<char*>(std::vector<T>::data()), 
                                               count * sizeof(T));
  }

/*
function loading the array from persistant storage.

*/

  template<class T>
  inline void Array<T>::load(CRelAlgebra::Reader& source)
  {
    size_t count;
    source.ReadOrThrow(reinterpret_cast<char*>(&count), sizeof(size_t));
    std::vector<T>::resize(count);
    source.ReadOrThrow(reinterpret_cast<char*>(std::vector<T>::data()), 
                                               count * sizeof(T));
  }

/*
saving the array from persistant storage.

*/

  template<class T>
  inline void Array<T>::save(CRelAlgebra::Writer &target, 
    bool includeHeader)
  {
    if (includeHeader) {
      size_t count = std::vector<T>::size();
      target.WriteOrThrow(reinterpret_cast<char*>(&count), sizeof(size_t));
    }

    target.WriteOrThrow(reinterpret_cast<char*>(std::vector<T>::data()), 
      std::vector<T>::size() * sizeof(T));
  }

/*
returns the size needed for persistant storage of this instance

*/

  template<class T>
  inline int Array<T>::savedSize() {
    return sizeof(size_t) + sizeof(T) * std::vector<T>::size();
  }

/*
returns the size needed for persistant storage for an array with containing
~count~ elements

*/

  template<class T>
  inline int Array<T>::savedSize(int count) {
    return sizeof(size_t) + sizeof(T) * count;
  }


}
