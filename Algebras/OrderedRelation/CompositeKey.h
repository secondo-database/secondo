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
#ifndef __COMPOSITEKEY_H__
#define __COMPOSITEKEY_H__

#include "IndexableAttribute.h"
#include "ListUtils.h"

#include "Algebras/TupleIdentifier/TupleIdentifier.h"

class CompositeKey : public IndexableAttribute {
  public:
    CompositeKey(const Tuple* t, const std::vector<int>& keyElements,
                 const std::vector<SmiKey::KeyDataType>& keyElemTypes,
                 const bool appendNumber = false, const TupleId appendix = -1);
    
    CompositeKey(const std::vector<void*>& attributes,
                 const std::vector<SmiKey::KeyDataType>& attrTypes,
                 const bool upperRange = false);
    
    CompositeKey(SmiKey& key);
    CompositeKey(SmiRecord& record);
    CompositeKey(SmiRecord& record, SmiSize& offset);
    CompositeKey(PrefetchingIterator*);
    CompositeKey(PrefetchingIterator*, SmiSize& offset);
    CompositeKey();
    CompositeKey(const CompositeKey& src);


    CompositeKey& operator=(const CompositeKey& src);
    
    void WriteTo(char* dest) const;
    
    void ReadFrom(const char* src);
    
    SmiSize SizeOfChars() const;
    
    size_t Sizeof() const;
    
    int Compare(const Attribute* attr) const;
    
    bool Adjacent(const Attribute* attr) const;
    
    CompositeKey* Clone() const;
    
    size_t HashValue() const;
    
    void CopyFrom(const Attribute* attr);
    
    SmiKey::KeyDataType GetType() const;
    
    bool WriteToRecord(SmiRecord& record, SmiSize& offset) const;
    
    SmiKey GetSmiKey() const;
    
    TupleId GetAppendix() const;
    
    ~CompositeKey();
    
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

    static void* Cast(void* addr);

    static int SizeOf();

    static bool CheckKind(const ListExpr typeInfo, ListExpr& errorInfo);
    
    const bool operator==(const CompositeKey& other) const;
    
  private:
    enum Mode { none, appendNumber, upperRange};
    void init(const std::vector<void*>& attributes,
         const std::vector<SmiKey::KeyDataType>& attrTypes,
         const Mode mode = none, const TupleId appendix = -1);
    
    void init(SmiRecord& record, SmiSize& offset);
    
    void init(PrefetchingIterator* iter, SmiSize& offset);
    
    SmiKey::KeyDataType kdt;
    SmiSize charsize;
    void* data;
};

#endif


