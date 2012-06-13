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

#include "RelationAlgebra.h"
#include "CompositeKey.h"

const string OREL = "orel";
const TupleId MIN_TUPLE_ID = 1;

/*
3 Declaration of classes
3.1 Ordered RelationIterator
Forward declaration of class OrderedRelation

*/
class OrderedRelation;

class OrderedRelationIterator : public GenericRelationIterator {
  public:
    OrderedRelationIterator(const OrderedRelation* orel, TupleType* newType=0,
                            const CompositeKey& from = CompositeKey(),
                            const CompositeKey& to = CompositeKey());

    virtual ~OrderedRelationIterator();

    virtual Tuple* GetNextTuple();
    virtual Tuple* GetNextTuple(const list<int>& attrList);

    virtual TupleId GetTupleId() const;

    virtual bool EndOfScan() const {return endOfScan;};

    const CompositeKey& GetKey() const;

  private:
    bool Advance();

    PrefetchingIterator* it;
    bool endOfScan;
    TupleType* tupleType;
    TupleType* outtype;
    SmiBtreeFile* tupleFile;
    SmiFileId lobFileId;
    CompositeKey key;
    TupleId appendix;
};

/*
3.3 OrderedRelation

*/

class OrderedRelation : public GenericRelation {

  public:

    OrderedRelation(ListExpr typeInfo, bool createFiles = true);

    ~OrderedRelation();

    static ListExpr Out(const ListExpr typeInfo, Word value);

    static Word In(const ListExpr typeInfo, const ListExpr value,
                    const int errorPos, ListExpr& errorInfo, bool& correct);

    static Word Create(const ListExpr typeInfo);

    static void Delete(const ListExpr typeInfo, Word& value);

    static bool Open(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value);

    static bool Save(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value);

    static void Close(const ListExpr typeInfo, Word& value);

    static Word Clone(const ListExpr typeInfo, const Word& value);

//??
    static void* Cast(void* addr);

//??
    static inline int SizeOf() { return 0; };

    static bool CheckKind(const ListExpr typeInfo, ListExpr& errorInfo);


    virtual int GetNoTuples() const;

    //wrong
    virtual double GetTotalRootSize() const;

    //wrong
    virtual double GetTotalRootSize(int i) const;

    virtual double GetTotalExtSize() const;

    virtual double GetTotalExtSize(int i) const;

    virtual double GetTotalSize() const;

    virtual double GetTotalSize(int) const;

    virtual void Clear();

    virtual void AppendTuple(Tuple* t);

    virtual Tuple* GetTuple(const TupleId& id,
                            const bool dontReportError) const;

    virtual Tuple* GetTuple( const TupleId& id,
                     const int attrIndex,
                     const vector< pair<int, int> >& intervals,
                     const bool dontReportError ) const;

    virtual Tuple* GetTuple(const CompositeKey& key) const;

    virtual Tuple* GetTuple(const CompositeKey& key,
                            const int attrIndex,
                            const vector< pair<int, int> >& intervals) const;

    virtual bool DeleteTuple(Tuple* t);

    bool DeleteTuple(Tuple* t, bool deleteComplete = true);

    virtual void UpdateTuple( Tuple *tuple,
                              const vector<int>& changedIndices,
                              const vector<Attribute *>& newAttrs );

    virtual GenericRelationIterator* MakeScan() const;

    virtual GenericRelationIterator* MakeScan(TupleType* tt) const;

    virtual GenericRelationIterator*
            MakeRangeScan( const CompositeKey& from=CompositeKey(),
                           const CompositeKey& to=CompositeKey()) const;

    virtual GenericRelationIterator*
            MakeRangeScan( TupleType* tt,
                           const CompositeKey& from=CompositeKey(),
                           const CompositeKey& to=CompositeKey()) const;

    virtual bool GetTupleFileStats(SmiStatResultType&);

    virtual bool GetLOBFileStats(SmiStatResultType&);

    CompositeKey GetKey(const Tuple* tuple, const bool appendNumber,
                        const TupleId appendix);

    CompositeKey GetRangeKey(Word& arg, int length, bool upper=false);
    CompositeKey GetUpperRangeKey(Word& arg, int length);
    CompositeKey GetLowerRangeKey(Word& arg, int length);

    static bool GetKeyStructure (ListExpr typeInfo, vector<int>& keyElement,
                            vector<SmiKey::KeyDataType>& keyElemType);

    const SmiBtreeFile* GetTupleFile() const;

    const TupleType* GetTupleType() const;

    static const string BasicType() { return "orel"; }

     static const bool checkType(ListExpr list){
      return listutils::isRelDescription2(list, BasicType());
     }

    ostream& Print(ostream& os) const;
  private:
    OrderedRelation();
    SmiBtreeFile* tupleFile;

    SmiFileId tupleFileId;
    SmiFileId lobFileId;
    bool hasLobs;

    TupleType* tupleType;
    int noTuples;
    double totalExtSize;
    vector<double> attrExtSize;
    double totalSize;
    vector<double> attrSize;

    vector<SmiKey::KeyDataType> keyElemType;
    vector<int> keyElement;

    TupleId maxId;

  friend class OrderedRelationIterator;
};
#endif


