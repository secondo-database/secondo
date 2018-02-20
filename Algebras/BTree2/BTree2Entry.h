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

This class represents a single entry of a btree's node. We make heavily use
of template specialization. We first introduce the default implementation
for generic types. These are stored directly (via memcpy), so no indirect
types can be used here, e.g. string, Attribute. Latter are implemented
below within special versions of this class.

The class BTreeEntry is inherited from BTreeEntryBase. This is a trick to
save some programming effort. Basically, the BTreeEntryBase class deals with
the storage of the key part, while the BTreeEntry class here mainly deals
with storing the valie part.

However, Read/Write methods, which store both (key+value), are provided here.

*/
#ifndef _BTREE2_ENTRY_H_
#define _BTREE2_ENTRY_H_

#include "BTree2Node.h"
#include "Attribute.h"
#include "SecondoSMI.h"

#include "BTree2EntryBase.h"

#include <string>

namespace BTree2Algebra {

/*
2 Class ~BTreeEntry~ - Default implementation for generic type VALUETYPE

*/
template<typename KEYTYPE, typename VALUETYPE>
class BTreeEntry : public BTreeEntryBase<KEYTYPE> {
  protected:
  VALUETYPE value;
/*
The only variable of this entry: keep memory usage small.

*/

  public:
  VALUETYPE const GetValue() const { return value; }
/*
Returns ~value~.

*/

  void SetValue(const VALUETYPE v) { value = v; }
/*
Sets ~value~ to ~v~.

*/

  bool valueEquals(const VALUETYPE v) { return value == v; }
/*
Returns true, if ~v~ is equal to ~value~.

*/

  static bool needExtendedValueFile() { return false; }
/*
This specialization does not use the extended value file.
(so no file have to be created)

*/

  static int GetValueSizeInRecord(unsigned int maxValuesize) { 
        return sizeof(VALUETYPE); 
  }
/*
Returns the size of this entry on disc.

*/

  int GetValueSizeInMemory() { return sizeof(VALUETYPE); }
/*
Returns the size of this entry in memory (the same as on disc).

*/

  void ReadValue(char *buffer, int& offset) {
    memcpy(&value, buffer+offset, sizeof(VALUETYPE));
    offset += sizeof(VALUETYPE);
  }
/*
Read the value from disc.

*/

  void Read(char *buffer, int& offset,
             const ListExpr& keytype,
             const ListExpr& valuetype,
             SmiRecordFile* extendedKeysFile, 
             SmiRecordFile* extendedValueFile) {
    BTreeEntryBase<KEYTYPE>::ReadKey(buffer, offset, keytype, extendedKeysFile);
    ReadValue(buffer, offset);
  }
/*
Read the entry from disc: read key first, then value.

*/

  void WriteValue(char *buffer, int& offset) {
    memcpy( buffer+offset, &(this->value), sizeof(VALUETYPE) );
    offset += sizeof(VALUETYPE);
  }
/*
Writes the value to disc: direct memory copy.

*/
  
  void Write(char *buffer, int& offset, 
              const ListExpr& keytype,
              const ListExpr& valuetype,
              SmiRecordFile* extendedKeysFile, 
              SmiRecordFile* extendedValueFile, unsigned char maxKeysize,
               unsigned char maxValuesize) {
    BTreeEntryBase<KEYTYPE>::WriteKey(buffer, offset,keytype, extendedKeysFile, 
                                       maxKeysize);
    WriteValue(buffer, offset);
  }
/*
Writes the entry to disc: key first, then entry

*/

};


/*
3 Class ~BTreeEntry~ - Implementation for VALUETYPE=NoneType

Although we do not have any data stored, we have to provide access methods
and comparison methods for this type as well. 

*/

template<typename KEYTYPE>
class BTreeEntry<KEYTYPE,NoneType> : public BTreeEntryBase<KEYTYPE> {
  public:
  const NoneType GetValue() const { return 0; }
/*
Must return something.

*/

  void SetValue(const NoneType& v) { }
/*
Do not have to set anything, so this method is empty.

*/

  bool valueEquals(const NoneType& v) { return true; }
/*
Two non-existent values are always equal.

*/

  static bool needExtendedValueFile() { return false; }
/*
This specialization does not use the extended value file.
(so no file have to be created)

*/

  static int GetValueSizeInRecord(unsigned int maxValuesize) { return 0; }
/*
No data = 0

*/
  int GetValueSizeInMemory() { return 0; }
/*
No data = 0

*/
  void Read(char *buffer, int& offset,
             const ListExpr& keytype,
             const ListExpr& valuetype,
             SmiRecordFile* extendedKeysFile, 
             SmiRecordFile* extendedValueFile) {
    BTreeEntryBase<KEYTYPE>::ReadKey(buffer, offset,keytype, extendedKeysFile);
  }
/*
Read the entry: only have to read the key

*/

  void Write(char *buffer, int& offset,
              const ListExpr& keytype,
              const ListExpr& valuetype,
              SmiRecordFile* extendedKeysFile, 
              SmiRecordFile* extendedValueFile, unsigned char maxKeysize, 
              unsigned char maxValuesize) {
    BTreeEntryBase<KEYTYPE>::WriteKey(buffer, offset,keytype, extendedKeysFile, 
         maxKeysize);
  }
/*
Write the entry: only have to write the key

*/

};


/*
4 Class ~BTreeEntry~ - Implementation for VALUETYPE=string

Storage is more complicated, as we might have to write the key to 
the extendedValueFile.

*/

template<typename KEYTYPE>
class BTreeEntry<KEYTYPE,std::string> : public BTreeEntryBase<KEYTYPE> {
  protected:
  std::string value;
  SmiRecordId extendedValueId;
/*
We have to store the record id of this entry in the extendedValueFile,
otherwise we would not be able to distinguish between adding a new entry
or overwriting the old one during writing of this entry.

*/

  public:
  BTreeEntry<KEYTYPE,std::string>() { extendedValueId = 0; }
/*
Constructor: Mark ~extendedValueId~ as unset.

*/

  std::string const& GetValue() const { return value; }
/*
Returns a reference to the string.

*/

  void SetValue(const std::string& v) { value=v; }
/*
Set the value string.

*/

  bool valueEquals(const std::string& v) { return v == value; }
/*
Returns true, if ~v~ is equal to ~value~.

*/

  static bool needExtendedValueFile() { return true; }
/*
This specialization uses the extended value file.

*/

  static int GetValueSizeInRecord(size_t maxValuesize) { 
     return std::max(maxValuesize,sizeof(SmiRecordId))+sizeof(unsigned char); 
  }
/*
The size of an entry on disc is given by one byte giving the size of
the string if stored directly + the maximum of ~maxValuesize~ (so that
there is enough space for direct storage) and ~SmiRecordId~ (so that
there is enough space for indirect storage reference).

*/

  int GetValueSizeInMemory() { return value.capacity() + 
                              sizeof(SmiRecordId)+sizeof(std::string); }
/*
Returns the memory usage of this entry.

*/

  void Read(char *buffer, int& offset,
             const ListExpr& keytype,
             const ListExpr& valuetype,
             SmiRecordFile* extendedKeysFile, 
             SmiRecordFile* extendedValuesFile) {
    BTreeEntryBase<KEYTYPE>::ReadKey(buffer, offset, keytype, extendedKeysFile);
    unsigned char valuesize;
    memcpy(&valuesize, buffer+offset, sizeof(unsigned char));
    offset += sizeof(unsigned char);
    
    if (valuesize != 0) { // Read direct 
      char buf2[valuesize+1];
      buf2[valuesize] = '\0';
      memcpy(buf2, buffer+offset, valuesize);
      offset += valuesize;
      value = buf2;
    } else {  // Read indirect
      SmiRecordId recid;

      memcpy(&recid, buffer+offset, sizeof(SmiRecordId));
      offset += sizeof(SmiRecordId);
 
      SmiRecord record;
      int RecordSelected = extendedValuesFile->SelectRecord(recid, record, 
                                 SmiRecordFile::ReadOnly );
      assert(RecordSelected);

      char* buf2 = new char[record.Size()];
      int RecordRead = record.Read(buf2, record.Size());
      assert(RecordRead);

      value = buf2;
      delete[] buf2;
      extendedValueId = recid;
    }
  }
/*
Read the value from disc. 

*/

  void Write(char *buffer, int& offset,
               const ListExpr& keytype,
               const ListExpr& valuetype,
               SmiRecordFile* extendedKeysFile,
               SmiRecordFile* extendedValuesFile, unsigned char maxKeysize, 
               unsigned char maxValuesize) {
    BTreeEntryBase<KEYTYPE>::WriteKey(buffer, offset,keytype, extendedKeysFile, 
         maxKeysize);
    int valuelength = value.length();
    if ((valuelength > 0) && (value.length() < maxValuesize)) { 
      unsigned char vl = valuelength;
      memcpy(buffer+offset, &vl, sizeof(unsigned char));
      offset += sizeof(unsigned char);

      memcpy(buffer+offset, value.c_str(), valuelength);
      offset += valuelength;
    } else {  // Write indirect
      SmiRecordId recid;
      SmiRecord record;
      if (extendedValueId == 0) {
        int RecordSelected = extendedValuesFile->AppendRecord(recid, record);
        extendedValueId = recid;
        assert(RecordSelected);
      } else {
        recid = extendedValueId;
        int RecordSelected = extendedValuesFile->SelectRecord(recid, record, 
                                                SmiRecordFile::Update);
        assert(RecordSelected);
      }
      int RecordWrite = record.Write(value.c_str(), valuelength+1); // incl \0
      assert(RecordWrite == valuelength+1);
     
      int zeroMarker = 0; 
      memcpy(buffer+offset, &zeroMarker, sizeof(unsigned char));
      offset += sizeof(unsigned char);

      memcpy(buffer+offset, &recid, sizeof(SmiRecordId));
      offset += sizeof(SmiRecordId);
    }
  }
};
/*
Write the value to disc. If the string length is below maxValuesize,
write it directly into the node. Otherwise write it in the extended value
file and store the record id pointing to it in the node.

*/


/*
4 Class ~BTreeEntry~ - Implementation for VALUETYPE=Attribute

Storage is more complicated, as we might have to write the value to 
the extendedValueFile. Secondo uses multiple methods for storing and
retrieving Attributes, we utilize two methods here: explicit
serialization and Open/Save methods.

Explicit serialization is used, when it is implemented by the data type,
and when the size is below ~maxValuesize~. Otherwise Open/Save will be
used, as this should always work (in special cases, the developer
of a data type would have to provide its own Open/Save method if
storage is more complicated). With explicit serialization, data is 
stored in the btree's node, whereas Open/Save can only store into a
separate record (here within the extended values file).

Using explicit serialization is therefore supposed to be more efficient.

*/

template<typename KEYTYPE>
class BTreeEntry<KEYTYPE,Attribute*> : public BTreeEntryBase<KEYTYPE> {
  protected:
  Attribute* value;
  SmiRecordId extendedValueId;
/*
We have to store the record id of this entry in the extendedValueFile,
otherwise we would not be able to distinguish between adding a new entry
or overwriting the old one during writing of this entry.

*/

  public:
  BTreeEntry<KEYTYPE,Attribute*>() { extendedValueId = 0; value = 0; }
/*
Constructor: Mark ~extendedValueId~ as unset.

*/
  ~BTreeEntry<KEYTYPE,Attribute*>() { 
      if (value != 0) value->DeleteIfAllowed(); 
  }
/*
Destructor: Removes reference to Attribute.

*/

  Attribute * const GetValue() const { return value; }
/*
The size of an entry on disc is given by one byte giving the size of
the serialization data size if stored directly + the maximum of 
~maxValuesize~ (so that there is enough space for direct storage)
 and ~SmiRecordId~ (so that there is enough space to store a reference
pointer for indirect storage).

*/

  void SetValue(Attribute* const v) { 
     if (value != 0) { value->DeleteIfAllowed(); } 
     value=v->Copy(); 
  }
/*
Set the value string.

*/

  bool valueEquals(Attribute* v) { return value->Compare(v) == 0; }
/*
Returns true, if ~v~ is equal to ~value~.

*/

  static bool needExtendedValueFile() { return true; }
/*
This specialization uses the extended value file.

*/

  static int GetValueSizeInRecord(size_t maxValuesize) { 
     return std::max(maxValuesize,sizeof(SmiRecordId))+sizeof(unsigned char); 
  }
  int GetValueSizeInMemory() { return value->Sizeof()+ 
                                        sizeof(SmiRecordId); }

  void Read(char *buffer, 
            int& offset, 
            const ListExpr& keytype,
            const ListExpr& valuetype,
            SmiRecordFile* extendedKeysFile, 
            SmiRecordFile* extendedValueFile) {
    BTreeEntryBase<KEYTYPE>::ReadKey(buffer, offset, keytype, extendedKeysFile);

    unsigned char valuesize;
    memcpy(&valuesize, buffer+offset, sizeof(unsigned char));
    offset += sizeof(unsigned char);

    if (valuesize == 0) {
      memcpy(&extendedValueId, buffer+offset, sizeof(SmiRecordId));
      offset += sizeof(SmiRecordId);
 
      SmiRecord record;
      int RecordSelected = extendedValueFile->SelectRecord(extendedValueId, 
                                 record, 
                                 SmiRecordFile::ReadOnly );
      assert(RecordSelected);

      size_t y = 0;
      value = Attribute::Open(record,y,valuetype);
    } else {
      size_t sss = valuesize;
      value = Attribute::Create(buffer+offset,sss,valuetype);
      offset += valuesize;
    }
  }
/*
Read the value from disc. 

*/


  void Write(char *buffer, int& offset,
               const ListExpr& keytype,
               const ListExpr& valuetype,
               SmiRecordFile* extendedKeysFile,
               SmiRecordFile* extendedValueFile, unsigned char maxKeysize, 
               unsigned char maxValuesize) {
    BTreeEntryBase<KEYTYPE>::WriteKey(buffer, offset, keytype,
                                      extendedKeysFile, maxKeysize);
    unsigned int ssize = value->SerializedSize();
    if ((ssize < maxValuesize) && (ssize > 0)) {
      // using explicit serialization and direct storage
      unsigned char sc = ssize;
      memcpy(buffer+offset, &sc, sizeof(unsigned char));
      offset += sizeof(unsigned char);

      value->Serialize(buffer+offset, ssize, 0);
      offset += sc;
    } else {  // Write indirect with Save
      SmiRecord record;
      if (extendedValueId == 0) {  
        int RecordSelected = 
                 extendedValueFile->AppendRecord(extendedValueId, record);
        assert(RecordSelected);
      } else { 
        int RecordSelected = extendedValueFile->SelectRecord(extendedValueId, 
                                                record, 
                                                SmiRecordFile::Update);
        assert(RecordSelected);
      }

      int zeroMarker = 0; 
      memcpy(buffer+offset, &zeroMarker, sizeof(unsigned char));
      offset += sizeof(unsigned char);

      memcpy(buffer+offset, &extendedValueId, sizeof(SmiRecordId));
      offset += sizeof(SmiRecordId);

      size_t dummy_offset = 0;
      Attribute::Save(record,dummy_offset,valuetype,value);
    }
  }
};
/*
Write the value to disc. If explicit serialization is not implemented for
this type or the data size would exceed ~maxValuesize~, data is stored via
calling the Save method with a new record in the extended values file.

*/

} // end namespace BTree2Algebra

#endif
