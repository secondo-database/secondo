/*
----
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Fre PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

1 Template Header File: BTree2Entry

Jan. 2010 M.Klocke, O.Feuer, K.Teufel

1.1 Overview

This class represents the key part of a single entry of a btree's node. 
We make heavily use of template specialization. 
We first introduce the default implementation
for generic types. These are stored directly (via memcpy), so no indirect
types can be used here, e.g. string, Attribute. Latter are implemented
below within special versions of this class.

The class BTreeEntry is inherited from BTreeEntryBase. This is a trick to
save some programming effort. Basically, the BTreeEntryBase class deals with
the storage of the key part, while the BTreeEntry class here mainly deals
with storing the value part.

However, Read/Write methods, which store both (key+value), are provided only
in the BTreeEntry Class.

*/
#ifndef _BTREE2_ENTRYBASE_H_
#define _BTREE2_ENTRYBASE_H_

#include "BTree2Node.h"
#include "Attribute.h"
#include "SecondoSMI.h"
#include "IndexableAttribute.h"

#include "AlgebraManager.h"

#include <string>

namespace BTree2Algebra {

/*
2 Class ~BTreeEntryBase~ - Default implementation for generic type VALUETYPE

*/

template<class KEYTYPE>  // Template has specializations
class BTreeEntryBase {
  protected:
  KEYTYPE key;
/*
The only variable of this entry: keep memory usage small.

*/

  public:
  KEYTYPE const& GetKey() const { return key; }
/*
Returns ~key~.

*/

  void SetKey(const KEYTYPE& x) { key = x; }
/*
Sets ~key~ to ~x~.

*/

  bool keyLessThan(const KEYTYPE& x) const { return key < x; }
  bool keyEquals(const KEYTYPE& x) const { return key == x; }
  bool keyGreaterThan(const KEYTYPE& x) const { return key > x; }
/*
Comparison methods for keys.

*/
  static bool needExtendedKeyFile() { return false; }
/*
This specialization does not use the extended key file.
(so no file have to be created)

*/

  static int GetKeySizeInRecord(unsigned int maxKeysize) { 
       return sizeof(KEYTYPE); 
  }
/*
Returns the size of this entry on disc.

*/

  int GetKeySizeInMemory() { return sizeof(KEYTYPE); }
/*
Returns the size of this entry in memory (the same as on disc).

*/


  void ReadKey(char *buffer, int& offset,
                 const ListExpr& keytype,
                 SmiRecordFile* extendedKeysFile) {
    memcpy(&key, buffer+offset, sizeof(KEYTYPE));
    offset += sizeof(KEYTYPE);
  }
/*
Read the key from disc.

*/

  void WriteKey(char *buffer, int& offset, 
                   const ListExpr& keytype,
                   SmiRecordFile* extendedKeysFile, 
                   unsigned int maxKeysize) {
    memcpy(buffer+offset, &key, sizeof(KEYTYPE));
    offset += sizeof(KEYTYPE);
  }
/*
Writes the key to disc: direct memory copy.

*/
  
};

/*
3 Class ~BTreeEntryBase~ - Implementation for KEYTYPE=string

Storage is more complicated, as we might have to write the key to 
the extendedKeyFile.

*/

template<>
class BTreeEntryBase<std::string> {
  protected:
  std::string key;
  SmiRecordId extendedId;
/*
We have to store the record id of this entry in the extendedKeyFile,
otherwise we would not be able to distinguish between adding a new entry
or overwriting the old one during writing of this entry.

*/


  public:
  BTreeEntryBase<std::string>() { extendedId = 0; }
/*
Constructor: Mark ~extendedValueId~ as unset.

*/

  const std::string& GetKey() const { return key; }
/*
Returns a reference to the string.

*/

  void SetKey(const std::string& x) { key = x; }
/*
Set the value string.

*/


  bool keyLessThan(const std::string& x) const { return key < x; }
  bool keyEquals(const std::string& x) const { return key == x; }
  bool keyGreaterThan(const std::string& x) const { return key > x; } 

  static bool needExtendedKeyFile() { return true; }
/*
This specialization might use the extended key file.

*/

  static int GetKeySizeInRecord(unsigned char maxKeysize) { 
     return maxKeysize+sizeof(unsigned char); 
  }
/*
The size of an entry on disc is given by one byte giving the size of
the string if stored directly + the maximum of ~maxValuesize~ (so that
there is enough space for direct storage) and ~SmiRecordId~ (so that
there is enough space for indirect storage reference).

*/

  int GetKeySizeInMemory() { return key.capacity()+sizeof(std::string)+
                                      sizeof(SmiRecordId); }
/*
Returns the memory usage of this entry.

*/

  void ReadKey(char *buffer, int& offset, 
                const ListExpr& keytype,
                SmiRecordFile* extendedKeysFile) {
    unsigned char keysize;
    memcpy(&keysize, buffer+offset, sizeof(unsigned char));
    offset += sizeof(unsigned char);
    
    if (keysize != 0) { // Read direct 
      char buf2[keysize+1];
      buf2[keysize] = '\0';
      memcpy(buf2, buffer+offset, keysize);
      offset += keysize;
      key = buf2;
    } else {  // Read indirect
      SmiRecordId recid;

      memcpy(&recid, buffer+offset, sizeof(SmiRecordId));
      offset += sizeof(SmiRecordId);
 
      SmiRecord record;
      int RecordSelected = extendedKeysFile->SelectRecord(recid, record, 
                                 SmiRecordFile::ReadOnly );
      assert(RecordSelected);

      char* buf2 = new char[record.Size()];
      int RecordRead = record.Read(buf2, record.Size());
      assert(RecordRead);

      key = buf2;
      delete[] buf2;
      extendedId = recid;
    }
  }
/*
Read the value from disc. 

*/


  void WriteKey(char *buffer, int& offset, 
                   const ListExpr& keytype,
                   SmiRecordFile* extendedKeysFile, 
                   unsigned int maxKeysize) {
    char keylength = key.length();
    if ((keylength > 0) && (key.length() < maxKeysize)) { 
      memcpy(buffer+offset, &keylength, sizeof(char));
      offset += sizeof(char);

      memcpy(buffer+offset, key.c_str(), keylength);
      offset += keylength;
    } else {  // Write indirect
      SmiRecordId recid;
      SmiRecord record;
      if (extendedId == 0) {
        int RecordSelected = extendedKeysFile->AppendRecord(recid, record);
        extendedId = recid;
        assert(RecordSelected);
      } else {
        recid = extendedId;
        int RecordSelected = extendedKeysFile->SelectRecord(recid, record, 
                                                SmiRecordFile::Update);
        assert(RecordSelected);
      }
      int RecordWrite = record.Write(key.c_str(), keylength+1); // including \0
      assert(RecordWrite == keylength+1);
     
      int zeroMarker = 0; 
      memcpy(buffer+offset, &zeroMarker, sizeof(unsigned char));
      offset += sizeof(unsigned char);

      memcpy(buffer+offset, &recid, sizeof(SmiRecordId));
      offset += sizeof(SmiRecordId);
    }
  }
/*
Write the value to disc. If the string length is below maxValuesize,
write it directly into the node. Otherwise write it in the extended value
file and store the record id pointing to it in the node.

*/
};


/*
4 Class ~BTreeEntryBase~ - Implementation for KEYTYPE=IndexableAttribute

Storage is more complicated, as we might have to write the value to 
the extendedKeyFile. However, it is quite similar to the case of
string storage, as the IndexableAttribute provides a special method
for serialization which returns a string.

*/

template<>
class BTreeEntryBase<IndexableAttribute*> {
  protected:
  IndexableAttribute* key;
  SmiRecordId extendedId;
/*
We have to store the record id of this entry in the extendedValueFile,
otherwise we would not be able to distinguish between adding a new entry
or overwriting the old one during writing of this entry.

*/

  public:
  BTreeEntryBase<IndexableAttribute*>() { key = 0; }
/*
Constructor: Mark ~extendedValueId~ as unset.

*/

  ~BTreeEntryBase<IndexableAttribute*>() { 
      if (key != 0) key->DeleteIfAllowed(); 
  }
/*
Destructor: Removes reference to Attribute.

*/

  bool keyLessThan(IndexableAttribute* x) const{ return key->Compare(x) < 0; }
  bool keyEquals(IndexableAttribute* x) const { return key->Compare(x) == 0; }
  bool keyGreaterThan(IndexableAttribute* x) const { 
    return key->Compare(x) > 0; 
  }

  IndexableAttribute* const& GetKey() const { return key; }
  void SetKey(IndexableAttribute* const x) { 
      if (key != 0) { key->DeleteIfAllowed(); }
      key = (IndexableAttribute*) x->Copy();
  }

  static bool needExtendedKeyFile() { return true; }
  static int GetKeySizeInRecord(unsigned int maxKeysize) {
     return maxKeysize + sizeof(unsigned char); 
  }
  int GetKeySizeInMemory() { return key->Sizeof(); }

  void ReadKey(char *buffer, int& offset, 
                const ListExpr& keytype,
                SmiRecordFile* extendedKeysFile) {
    unsigned char keysize;
    memcpy(&keysize, buffer+offset, sizeof(unsigned char));
    offset += sizeof(unsigned char);
    
    if (keysize != 0) { // Read direct
      char buf2[keysize+1];
      buf2[keysize] = '\0';
      memcpy(buf2, buffer+offset, keysize);
      offset += keysize;
      int algId = nl->IntValue(nl->First(keytype));
      int typeId = nl->IntValue(nl->Second(keytype));
      // CreateObj returns a function pointer; the function is then
      // called with argument keytype. It returns a structure, from
      // which addr is taken. Not my idea! Copied from Attribute.cpp
      key = (IndexableAttribute*) 
            (SecondoSystem::GetAlgebraManager()->CreateObj(algId, typeId))
            (keytype).addr;

      key->ReadFrom(buf2);
    } else {  // Read indirect
      SmiRecordId recid;

      memcpy(&recid, buffer+offset, sizeof(SmiRecordId));
      offset += sizeof(SmiRecordId);
 
      SmiRecord record;
      int RecordSelected = extendedKeysFile->SelectRecord(recid, record, 
                                                     SmiRecordFile::ReadOnly );
      assert(RecordSelected);

      char* buf2 = new char[record.Size()];
      int RecordRead = record.Read(buf2, record.Size());
      assert(RecordRead);

      key->ReadFrom(buf2);
      delete[] buf2;
      extendedId = recid;
    }
  }

  void WriteKey(char *buffer, int& offset, 
                   const ListExpr& keytype,
                   SmiRecordFile* extendedKeysFile, 
                   unsigned int maxKeysize) {
    unsigned int keylength = key->SizeOfChars();
    if ((keylength > 0) && (keylength < maxKeysize)) { 
      unsigned char kl = keylength;
      memcpy(buffer+offset, &kl, sizeof(unsigned char));
      offset += sizeof(unsigned char);

      key->WriteTo(buffer+offset);
      offset += keylength;
    } else {  // Write indirect
      SmiRecordId recid;
      SmiRecord record;
      if (extendedId == 0) {
        int RecordSelected = extendedKeysFile->AppendRecord(recid, record);
        assert(RecordSelected);
      } else {
        recid = extendedId;
        int RecordSelected = extendedKeysFile->SelectRecord(recid, record, 
                                                       SmiRecordFile::Update);
        assert(RecordSelected);
      }
      char* buf2 = new char[keylength+1];
      key->WriteTo(buf2);
      int RecordWrite = record.Write(buf2, keylength+1); // including \0
      assert(RecordWrite);
      delete[] buf2;
     
      int zeroMarker = 0; 
      memcpy(buffer+offset, &zeroMarker, sizeof(unsigned char));
      offset += sizeof(unsigned char);

      memcpy(buffer+offset, &recid, sizeof(SmiRecordId));
      offset += sizeof(SmiRecordId);
    }
  }
};

} // end namespace 
#endif
