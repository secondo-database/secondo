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

 1 Data types

 The algebra features ~nested relations~ ("NRel"[2]) being an outer border
 containing one or more ~attribute relations~ ("ARel"[2]). Both of them are
 described in this section.

 1.1 Nested relations

*/

#ifndef ALGEBRAS_NESTEDRELATION2_NREL_H_
#define ALGEBRAS_NESTEDRELATION2_NREL_H_

#include "Include.h"
#include "../Relation-C++/RelationAlgebra.h"

namespace nr2a {

class NRel;

/*
 To iterate a nested relation it is possible to request an iterator from it,
 which is declared here.

*/
class NRelIterator
{
  friend NRel;

  public:
    ~NRelIterator();
    Tuple * getNextTuple();

  private:
    NRelIterator(const NRel * const nrel,
        const ListExpr attributeDescription);
    const NRel * m_nrel;
    RelationIterator* m_relIterator;
    TupleType * m_tupleTypeObject;
};

/*
 The implementation of the nested relations contains the typical methods
 neccessary to implement a data type in SECONDO. Additionally it provides some
 specific methods for building it up and requesting data from it.

*/
class NRel
{
    friend NRelIterator;

  public:
    struct Info : ConstructorInfo
    {
      public:
        Info()
        {
          name = NRel::BasicType();
          signature = "-> " + Kind::SIMPLE();
          typeExample = NRel::BasicType();
          listRep = "(nrel2(tuple([Keyword:string, Occurences: "
              "arel2(tuple([Page: int, Line: int]))])))";
          valueExample =
              "((\"database\" ((1 1) (2 10))) "
              "(\"system\" ((1 3) (3 14) (8 15))))";
          remarks = "Represents a nested relation. It can contain"
              "nested relations as attributes (arel2).";
        }
    };
    struct Functions;

    NRel();
    NRel(const ListExpr typeInfo);
    NRel(const ListExpr typeInfo, Relation * const data);
    ~NRel();

    static Word In(const ListExpr typeInfo, const ListExpr instance,
        const int errorPos, ListExpr& errorInfo, bool& correct);
    static ListExpr Out(ListExpr typeInfo, Word value);
    static Word Create(const ListExpr typeInfo);
    static void Delete(const ListExpr typeInfo, Word& w);
    static bool Open(SmiRecord& valueRecord, size_t& offset,
        const ListExpr typeInfo, Word& value);
    static bool Save(SmiRecord& valueRecord, size_t& offset,
        const ListExpr typeInfo, Word& value);
    static void Close(const ListExpr typeInfo, Word& w);
    static Word Clone(const ListExpr typeInfo, const Word& w);
    static bool CheckType(ListExpr type, ListExpr& errorInfo);
    static int SizeOfObj();
    static const std::string BasicType();

    void Clear();
    // Tuple is untouched, but Tuple::WriteToBinStr is not const
    void AppendTuple(Tuple *tuple);
    NRelIterator* getIterator(const ListExpr attributeDescription) const;
    Tuple * GetTuple(const ListExpr tupleType, const TupleId tid) const;
    unsigned long int GetTupleCount() const;

  private:
    Relation* m_data;
};

struct NRel::Functions : ConstructorFunctions<NRel>
{
    Functions()
    {
      // re-assign some function pointers
      create = NRel::Create;
      deletion = NRel::Delete;
      in = NRel::In;
      out = NRel::Out;
      save = NRel::Save;
      open = NRel::Open;
      close = NRel::Close;
      kindCheck = NRel::CheckType;
    }
};

} /* namespace nr2a*/

#endif /* ALGEBRAS_NESTEDRELATION2_NREL_H_*/
