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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Ordered Relation Algebra

Winter 2009 Nicolai Voget

[TOC]

1 Overview

The Ordered Relational Algebra implements the type constructor ~orel~.

The Implementation can be found in the
RelationAlgebra.cpp file.

2 Defines, includes, and constants

*/
#ifndef __ORDEREDRELATIONALGEBRA_H__
#define __ORDEREDRELATIONALGEBRA_H__
//#include "Algebra.h"
#include "RelationAlgebra.h"
#include "IndexableAttribute.h"

const string OREL = "orel";

/*
3 Declaration of classes
3.1 CompositeKey
CompositeKey is used for transmitting complex keyData to the BtreeFile.

*/
class CompositeKey : public IndexableAttribute {
  public:
    inline CompositeKey(char* src, SmiSize length) {
      data = (char*)malloc(length);
      memcpy(data,src,length);
      charsize = length;
    }
    inline void WriteTo(char* dest) const {
      memcpy(dest, data, charsize);
    }
    inline void ReadFrom(const char* src) {
      free(data);
      data = (char*)malloc(charsize);
      memcpy(data,src,charsize);
    }
    inline SmiSize SizeOfChars() const {
      return charsize;
    };
    inline size_t Sizeof() const {
      return sizeof(this);
    }
    inline int Compare(const Attribute* attr) const {
      const CompositeKey* other = static_cast<const CompositeKey*>(attr);
      if(other->charsize<charsize) {
        return 1;
      } else if(other->charsize>charsize) {
        return -1;
      } else {
        return memcmp(data,other->data,charsize);
      }
    }
    inline bool Adjacent(const Attribute* attr) const {
      return false;
    }
    inline CompositeKey* Clone() const {
      return new CompositeKey(data,charsize);
    }
    inline size_t HashValue() const {
      return charsize;
    }
    inline void CopyFrom(const Attribute* attr) {
      const CompositeKey* other = static_cast<const CompositeKey*>(attr);
      free(data);
      charsize = other->charsize;
      data = (char*)malloc(charsize);
      memcpy(data,other->data,charsize);
    }
    
    inline ~CompositeKey() {
      free(data);
    }
  
  private:
    SmiSize charsize;
    char* data;
};

/*
3.2 Ordered RelationIterator
Forward declaration of class OrderedRelation

*/
class OrderedRelation;

class OrderedRelationIterator : public GenericRelationIterator {
  public:
    OrderedRelationIterator(const OrderedRelation* orel, TupleType* newType=0);
    
    ~OrderedRelationIterator();
    
    Tuple* GetNextTuple();
    Tuple* GetNextTuple(const list<int>& attrList);
    
    TupleId GetTupleId() const;
    
  private:
    SmiKeyedFileIterator it;
    TupleType* tupleType;
    TupleType* outtype;
    SmiBtreeFile* tupleFile;
    SmiFileId lobFileId;
    
};

/*
3.3 OrderedRelation

*/
class OrderedRelation : public GenericRelation {

  public:
    
    OrderedRelation(ListExpr typeInfo, bool createFiles = true);

    ~OrderedRelation();

//OK
    static ListExpr Out(const ListExpr typeInfo, Word value);

//OK
    static Word In(const ListExpr typeInfo, const ListExpr value,
                    const int errorPos, ListExpr& errorInfo, bool& correct);

    //to be deleted? EDIT:Check
    static ListExpr SaveToList(const ListExpr typeInfo, const Word value);

    //to be deleted? EDIT:Check
    static Word RestoreFromList(const ListExpr typeInfo, const ListExpr value,
                   const int errorPos, ListExpr& errorInfo, bool& correct);

//OK
    static Word Create(const ListExpr typeInfo);

//OK
    static void Delete(const ListExpr typeInfo, Word& value);

//OK
    static bool Open(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value);

//OK
    static bool Save(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value);

//OK
    static void Close(const ListExpr typeInfo, Word& value);

//OK
    static Word Clone(const ListExpr typeInfo, const Word& value);

//??
    static void* Cast(void* addr);

    static inline int SizeOf() { return 0; };

    static bool CheckKind(const ListExpr typeInfo, ListExpr& errorInfo);


    virtual int GetNoTuples() const;
    
    virtual double GetTotalRootSize() const;
    
    virtual double GetTotalRootSize(int i) const;
    
    virtual double GetTotalExtSize() const;

    virtual double GetTotalExtSize(int i) const;
    
    virtual double GetTotalSize() const;
    
    virtual double GetTotalSize(int) const;
    
    virtual void Clear();
    
    virtual void AppendTuple(Tuple* t);
    
    virtual Tuple* GetTuple(const TupleId& id) const;
    
    virtual Tuple *GetTuple( const TupleId& id,
                     const int attrIndex,
                     const vector< pair<int, int> >& intervals ) const;
    
    virtual GenericRelationIterator* MakeScan() const;
    
    virtual GenericRelationIterator* MakeScan(TupleType* tt) const;
    
    virtual bool GetTupleFileStats(SmiStatResultType&);
    
    virtual bool GetLOBFileStats(SmiStatResultType&);
    
    //should work
    static void GetKey(Tuple* tuple, SmiKey& key,
                       const SmiKey::KeyDataType& keyType,
                       const vector<int>& keyElement,
                       const vector<SmiKey::KeyDataType>& keyElemType);
    
    //should work
    static bool GetKeytype (ListExpr typeInfo, SmiKey::KeyDataType& keyType,
                            vector<int>& keyElement,
                            vector<SmiKey::KeyDataType>& keyElemType);
    
    static bool ValidKeyElements(ListExpr tupleInfo, ListExpr keyInfo);

  private:
    OrderedRelation();
    SmiBtreeFile* tupleFile;
    SmiRecordFile* lobFile;

    SmiFileId tupleFileId;
    SmiFileId lobFileId;
    bool hasLobs;
    
    TupleType* tupleType;
    int noTuples;
    double totalExtSize;
    vector<double> attrExtSize;
    double totalSize;
    vector<double> attrSize;

    SmiKey::KeyDataType keyType;
    vector<SmiKey::KeyDataType> keyElemType;
    vector<int> keyElement;

  friend class OrderedRelationIterator;
};

#endif
