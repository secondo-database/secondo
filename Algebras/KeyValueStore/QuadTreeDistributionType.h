/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
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

*/

#ifndef QUADTREEDISTRIBUTIONTYPE_H_
#define QUADTREEDISTRIBUTIONTYPE_H_

#include "RectangleAlgebra.h"

#include "QuadTreeDistribution.h"

namespace KVS {

class QuadTreeDistributionType : public QuadTreeDistribution {
 public:
  QuadTreeDistributionType(int initialWidth, int initialHeight, int nrServers);
  QuadTreeDistributionType(const QuadTreeDistributionType& rhs);
  ~QuadTreeDistributionType();

  QuadTreeDistributionType* Clone();

  static Word In(const ListExpr typeInfo, const ListExpr instance,
                 const int errorPos, ListExpr& errorInfo, bool& correct);

  static ListExpr Out(ListExpr typeInfo, Word value);

  static Word Create(const ListExpr typeInfo);

  static void Delete(const ListExpr typeInfo, Word& w);

  static void Close(const ListExpr typeInfo, Word& w);

  static Word Clone(const ListExpr typeInfo, const Word& w);

  static bool KindCheck(ListExpr type, ListExpr& errorInfo);

  static int SizeOfObj();

  static ListExpr Property();

  static bool Open(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value);

  static bool Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& w);

  static const string BasicType();

  static const bool checkType(const ListExpr type);

  static QuadNode* OpenNode(SmiRecord& valueRecord, size_t& offset,
                            QuadNode* parent);
  static void SaveNode(SmiRecord& valueRecord, size_t& offset, QuadNode* node);

 private:
  inline QuadTreeDistributionType() {}
  friend class ConstructorFunctions<QuadTreeDistributionType>;

  static QuadNode* parseQuadNode(const ListExpr instance, QuadNode* parent);
  static ListExpr serializeQuadNode(QuadNode* node);
};

struct qtdistributionInfo : ConstructorInfo {
  qtdistributionInfo() {
    name = QuadTreeDistributionType::BasicType();
    signature = "-> " + Kind::SIMPLE();
    typeExample = QuadTreeDistributionType::BasicType();
    listRep =
        "(<initialWidth> <initialHeight> (serverIdx1 ... serverIdxN) <root>)";
    valueExample = "(1000 1000 (0 1 2 3 4) 0)";
    remarks = "all ints";
  }
};

struct qtdistributionFunctions
    : ConstructorFunctions<QuadTreeDistributionType> {
  qtdistributionFunctions() {
    create = QuadTreeDistributionType::Create;
    in = QuadTreeDistributionType::In;
    out = QuadTreeDistributionType::Out;
    open = QuadTreeDistributionType::Open;
    save = QuadTreeDistributionType::Save;
  }
};
}

#endif /* QUADTREEDISTRIBUTIONTYPE_H_ */
