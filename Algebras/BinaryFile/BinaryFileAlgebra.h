
/*
----
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Department of Computer Science,
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


2.1 Class ~BinaryFile~

*/

#ifndef BINARYFILE_H
#define BINARYFILE_H

#include <string>
#include "Attribute.h"
#include "ListUtils.h"
#include "../../Tools/Flob/Flob.h"

class BinaryFile : public Attribute
{
  public:

    BinaryFile( const int size, const bool defined=false );
    inline ~BinaryFile();
    inline void Destroy();

    inline size_t Sizeof() const;
    inline size_t HashValue() const;
    void CopyFrom(const Attribute* right);
    inline int Compare(const Attribute * arg) const;
    inline bool Adjacent(const Attribute * arg) const;
    BinaryFile* Clone() const;
    std::ostream& Print( std::ostream &os ) const;
    inline int NumOfFLOBs() const;
    inline Flob *GetFLOB(const int i);

    void Encode( std::string& textBytes ) const;
    void Decode( const std::string& textBytes );
    bool SaveToFile( const std::string& fileName ) const;

    int GetSize() const;
    void Get(const size_t offset, const size_t size, char* bytes) const;
    void Resize(int newSize);
    void Put(const size_t offset, const size_t size, const char* bytes);

    static void* Cast(void* addr);

    static const std::string BasicType() { return "binfile";}

    static const bool checkType(const ListExpr list){
      return listutils::isSymbol(list, BasicType());
    }


  private:

    inline BinaryFile() {};
/*
This constructor should not be used.

*/
    Flob binData;
    bool canDelete;
};

namespace FilePath {
  const std::string BasicType();
}
#endif
