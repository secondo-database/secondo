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

1.1 Attribute relations

The essential point of nested relations is, that relations are allowed as
attributes. Such attribute relations are implemented in the class "ARel"[2].

*/

#ifndef ALGEBRAS_NESTEDRELATION2_AREL_H_
#define ALGEBRAS_NESTEDRELATION2_AREL_H_

#include "Algebra.h"
#include "Attribute.h"
#include "../Tools/Flob/DbArray.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"


namespace nr2a {

//class NestedRelation2Algebra;
class ARel;

/*
To iterate attribute relations the "ARelIterator"[2] stores the current
position for reading values in the "ARel"[2]s "Flob"[2].

*/
class ARelIterator
{
  public:
    ARelIterator(const ARel * const arel, 
                 TupleType* tupleType,
                 ListExpr tupleTypeList,
                 const TupleId id = 0);
    Tuple * getNextTuple();

    ~ARelIterator(){
       if(tupleType){
          tupleType->DeleteIfAllowed();
       }
    }

  private:
    const ARel * m_arel;
    unsigned long int m_index;
    SmiSize m_offset;
    TupleType* tupleType;
    ListExpr tupleTypeList;
};

/*
SECONDOs class to represent relations attributes is called "Attribute"[2] and
"ARel"[2] inherits from it. Beneath the usual functions needed by SECONDO the
class provides functions to handle its "Flob"[2]s and functions similar to
those of the class "NRel"[2].

For the implementation -- contrary to "NRel"[2] -- has no underlying data
structure hiding its complexity. The class has some private methods dealing
with storing and requesting data and meta data.

*/
class ARel : public Attribute
{
    friend ARelIterator;

  public:
    struct Info : ConstructorInfo
    {
      Info();
    };
    struct Functions;
    class FlobWriter;
    class FlobReader;

    ARel();
    ARel(const ListExpr typeInfo);
    virtual ~ARel();

    static Word In(const ListExpr typeInfo, const ListExpr instance,
        const int errorPos, ListExpr& errorInfo, bool& correct);
    static ListExpr Out(ListExpr typeInfo, Word value);
    static Word Create(const ListExpr typeInfo);
    static void Delete(const ListExpr typeInfo, Word& w);
    static Word Clone(const ListExpr typeInfo, const Word& w);
    static bool CheckType(ListExpr type, ListExpr& errorInfo);
    static int SizeOfObj();
    static const std::string BasicType();

    static TypeConstructor GetTypeConstructor();

    int NumOfFLOBs() const;
    Flob *GetFLOB(const int i);

    virtual size_t Sizeof() const;
    virtual bool Adjacent(const Attribute *attrib) const;
    virtual int Compare(const Attribute *rhs) const;
    virtual Attribute *Clone() const;
    virtual size_t HashValue() const;
    virtual void CopyFrom(const Attribute* right);
    inline virtual size_t SerializedSize() const;
    inline virtual void Rebuild(char* storage, size_t sz);
    inline virtual void Serialize(char* storage, size_t sz,
        size_t offset) const;

    virtual void Imitate(const ARel * const prototype);
    unsigned long int GetTupleCount() const;
    ListExpr ToList(ListExpr typeInfo) const;
    void AppendTuple(Tuple *tuple);
    Tuple * GetTuple(const TupleId tid) const;
    ListExpr GetTupleType() const;
    ListExpr GetType() const;
    void Clear();

  private:
    void Init(const ListExpr typeInfo);
    Tuple * GetTupleByOffset(const SmiSize & offset,
        SmiSize & tupleSize, TupleType* tupleType, 
        ListExpr tupleTypeList) const;
    size_t CalculateHash(const Tuple * const tuple) const;
    void WriteTuple(Tuple *tuple);
    void ReadTuple(Tuple *tuple, const SmiSize & offset,
        const SmiSize & length, ListExpr tupleType) const;
    void WriteType(const ListExpr type);
    ListExpr ReadType() const;

    DbArray<SmiSize> m_indexStore;
    Flob m_tupleStore;
    Flob m_dataStore;
    unsigned long int m_tupleCount;
    size_t m_hashValue;

};

/*
The typical structures to declare a type in SECONDO.

*/
struct ARel::Functions : ConstructorFunctions<ARel>
{
    Functions()
    {
      create = ARel::Create;
      deletion = ARel::Delete;
      in = ARel::In;
      out = ARel::Out;
      kindCheck = ARel::CheckType;
    }
};

/*
To access a "Flob"[2] even more easily, two helper classes are implemented
below.

The "FlobWriter"[2] class is useful for appending values to a flob with
minimum effort. It can also be used to copy another "Flob"[2]s data and
features a type safe interface as well as an automatic internal pointer.

*/
class ARel::FlobWriter
{
  public:
    FlobWriter(Flob &flob, const SmiSize offset);

    template <typename T>
    void Write(const T &var);
    void Write(const char *buffer, const SmiSize length);
    void CopyFrom(FlobReader &source, const SmiSize length);
    void SetFrom(FlobReader &source, const SmiSize length);
    void Seek(const SmiSize offset);

  private:
    Flob &m_flob;
    SmiSize m_offset;
};

/*
The "FlobReader"[2] class is useful for reading values from a flob with
minimum effort. As its counterpart does, it features a type safe interface and
an automatic internal pointer, too.

*/
class ARel::FlobReader
{
  public:
    FlobReader(const Flob &flob, const SmiSize offset);

    template <typename T>
    void Read(T &var);
    void Read(char *buffer, const SmiSize length);
    void Seek(const SmiSize offset);

  private:
    const Flob &m_flob;
    SmiSize m_offset;
};

} /* namespace nr2a*/

#endif /* ALGEBRAS_NESTEDRELATION2_AREL_H_*/
