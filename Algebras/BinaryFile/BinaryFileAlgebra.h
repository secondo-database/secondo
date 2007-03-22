
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

#include "StandardAttribute.h"
#include "FLOB.h"

class BinaryFile : public StandardAttribute
{
  public:

    inline BinaryFile() {};
/*
This constructor should not be used.

*/
    BinaryFile( const int size );
    inline ~BinaryFile();
    inline void Destroy();

    inline bool IsDefined() const;
    inline void SetDefined( bool Defined);
    inline size_t Sizeof() const;
    inline size_t HashValue() const;
    void CopyFrom(const StandardAttribute* right);
    inline int Compare(const Attribute * arg) const;
    inline bool Adjacent(const Attribute * arg) const;
    BinaryFile* Clone() const;
    ostream& Print( ostream &os ) const;
    inline int NumOfFLOBs() const;
    inline FLOB *GetFLOB(const int i);

    void Encode( string& textBytes ) const;
    void Decode( const string& textBytes );
    bool SaveToFile( const char *fileName ) const;

    int GetSize() const;
    void Get(size_t offset,const char** bytes) const;
    void Resize(int newSize);
    void Put(int offset, int size, const char* bytes);



  private:

    FLOB binData;
    bool canDelete;
};

#endif
