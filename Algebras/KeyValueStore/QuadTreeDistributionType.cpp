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

#include "QuadTreeDistributionType.h"

namespace KVS {

QuadNodeType::QuadNodeType(QuadNode* parent, double x, double y, double width,
                           double height)
    : QuadNode(parent, x, y, width, height) {}

QuadNodeType::QuadNodeType(double x, double y, double width, double height)
    : QuadNode(x, y, width, height) {}

QuadNodeType::QuadNodeType(const QuadNodeType& node) : QuadNode(node) {}

QuadNodeType::~QuadNodeType() {}

const std::string QuadNodeType::BasicType() { return "qtnode"; }

const bool QuadNodeType::checkType(const ListExpr type) {
  return listutils::isSymbol(type, BasicType());
}

Word QuadNodeType::In(const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct) {
  correct = false;

  Word w = SetWord(Address(0));

  if (!nl->IsAtom(instance)) {
    if (nl->ListLength(instance) == 11) {
      ListExpr x = nl->First(instance);
      ListExpr y = nl->Second(instance);
      ListExpr width = nl->Third(instance);
      ListExpr height = nl->Fourth(instance);
      ListExpr weight = nl->Fifth(instance);
      ListExpr serverId = nl->Sixth(instance);
      ListExpr maxGlobalId = nl->Seventh(instance);

      if (nl->IsAtom(x) && nl->AtomType(x) == RealType && nl->IsAtom(y) &&
          nl->AtomType(y) == RealType && nl->IsAtom(width) &&
          nl->AtomType(width) == RealType && nl->IsAtom(height) &&
          nl->AtomType(height) == RealType && nl->IsAtom(weight) &&
          nl->AtomType(weight) == IntType && nl->IsAtom(serverId) &&
          nl->AtomType(serverId) == IntType && nl->IsAtom(maxGlobalId) &&
          nl->AtomType(maxGlobalId) == IntType) {
        QuadNodeType* temp =
            new QuadNodeType(0, nl->RealValue(x), nl->RealValue(y),
                             nl->RealValue(width), nl->RealValue(height));

        temp->weight = nl->IntValue(weight);
        temp->serverId = nl->IntValue(serverId);
        temp->maxGlobalId = nl->IntValue(maxGlobalId);

        bool childCorrect = false;
        Word tempChild = SetWord(Address(0));

        for (unsigned int childIdx = 0; childIdx < 4; ++childIdx) {
          childCorrect = false;

          tempChild =
              QuadNodeType::In(typeInfo, nl->Nth(8 + childIdx, instance),
                               errorPos, errorInfo, childCorrect);
          if (childCorrect) {
            temp->children[childIdx] = (QuadNode*)tempChild.addr;
            temp->children[childIdx]->parent = temp;
          } else {
            temp->children[childIdx] = 0;
          }
        }

        correct = true;
        w.addr = temp;
        return w;
      } else {
        cmsg.inFunError("(QuadNode) Expecting integer atoms");
      }
    } else {
      cmsg.inFunError("(QuadNode) Expecting a list with 11 elements");
    }
  }

  return w;
}

ListExpr QuadNodeType::Out(ListExpr typeInfo, Word value) {
  QuadNodeType* node = static_cast<QuadNodeType*>(value.addr);

  if (node == 0) {
    return nl->IntAtom(0);
  } else {
    return nl->TwoElemList(
        nl->Cons(
            nl->RealAtom(node->x),
            nl->SixElemList(
                nl->RealAtom(node->y), nl->RealAtom(node->width),
                nl->RealAtom(node->height), nl->IntAtom(node->weight),
                nl->IntAtom(node->serverId), nl->IntAtom(node->maxGlobalId))),
        nl->FourElemList(Out(typeInfo, SetWord(node->children[0])),
                         Out(typeInfo, SetWord(node->children[1])),
                         Out(typeInfo, SetWord(node->children[2])),
                         Out(typeInfo, SetWord(node->children[3]))));
  }
}

bool QuadNodeType::Open(SmiRecord& valueRecord, size_t& offset,
                        const ListExpr typeInfo, Word& value) {
  double x = 0, y = 0, width = 0, height = 0;
  int weight = 0, serverId = 0;
  bool children[4] = {false, false, false, false};
  unsigned int maxGlobalId = 0;

  bool ok = true;
  ok = ok && valueRecord.Read(&x, sizeof(double), offset);
  offset += sizeof(double);
  ok = ok && valueRecord.Read(&y, sizeof(double), offset);
  offset += sizeof(double);
  ok = ok && valueRecord.Read(&width, sizeof(double), offset);
  offset += sizeof(double);
  ok = ok && valueRecord.Read(&height, sizeof(double), offset);
  offset += sizeof(double);
  ok = ok && valueRecord.Read(&weight, sizeof(int), offset);
  offset += sizeof(int);
  ok = ok && valueRecord.Read(&serverId, sizeof(int), offset);
  offset += sizeof(int);
  ok = ok && valueRecord.Read(&maxGlobalId, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);
  ok = ok && valueRecord.Read(&children[0], sizeof(bool), offset);
  offset += sizeof(bool);
  ok = ok && valueRecord.Read(&children[1], sizeof(bool), offset);
  offset += sizeof(bool);
  ok = ok && valueRecord.Read(&children[2], sizeof(bool), offset);
  offset += sizeof(bool);
  ok = ok && valueRecord.Read(&children[3], sizeof(bool), offset);
  offset += sizeof(bool);

  if (ok) {
    QuadNode* temp = new QuadNode(0, x, y, width, height);
    temp->weight = weight;
    temp->serverId = serverId;
    temp->maxGlobalId = maxGlobalId;
    for (int i = 0; i < 4; ++i) {
      temp->children[i] = 0;
      if (children[i]) {
        Word tempChild;
        if (Open(valueRecord, offset, typeInfo, tempChild)) {
          temp->children[i] = (QuadNode*)tempChild.addr;
          temp->children[i]->parent = temp;
        }
      }
    }

    value.addr = temp;
  }

  return ok;
}

bool QuadNodeType::Save(SmiRecord& valueRecord, size_t& offset,
                        const ListExpr typeInfo, Word& value) {
  QuadNodeType* node = static_cast<QuadNodeType*>(value.addr);

  size_t dsize = sizeof(double);
  size_t isize = sizeof(int);
  size_t bsize = sizeof(bool);

  bool ok = true;
  ok = ok && valueRecord.Write(&node->x, dsize, offset);
  offset += dsize;
  ok = ok && valueRecord.Write(&node->y, dsize, offset);
  offset += dsize;
  ok = ok && valueRecord.Write(&node->width, dsize, offset);
  offset += dsize;
  ok = ok && valueRecord.Write(&node->height, dsize, offset);
  offset += dsize;
  ok = ok && valueRecord.Write(&node->weight, isize, offset);
  offset += isize;
  ok = ok && valueRecord.Write(&node->serverId, isize, offset);
  offset += isize;
  ok =
      ok && valueRecord.Write(&node->maxGlobalId, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);

  for (int i = 0; i < 4; ++i) {
    bool temp = node->children[i];
    ok = ok && valueRecord.Write(&temp, bsize, offset);
    offset += bsize;
  }

  for (int i = 0; i < 4; ++i) {
    if (node->children[i]) {
      Word tempChild = SetWord(node->children[i]);
      QuadNodeType::Save(valueRecord, offset, typeInfo, tempChild);
    }
  }

  return ok;
}

/*
 2.4 Support Functions for Persistent Storage

*/

Word QuadNodeType::Create(const ListExpr typeInfo) {
  return (SetWord(new QuadNodeType(0, 0, 0, 0)));
}

void QuadNodeType::Delete(const ListExpr typeInfo, Word& w) {
  delete static_cast<QuadNodeType*>(w.addr);
  w.addr = 0;
}

void QuadNodeType::Close(const ListExpr typeInfo, Word& w) {
  delete static_cast<QuadNodeType*>(w.addr);
  w.addr = 0;
}

Word QuadNodeType::Clone(const ListExpr typeInfo, const Word& w) {
  QuadNodeType* p = static_cast<QuadNodeType*>(w.addr);
  return SetWord(new QuadNodeType(*p));
}
/*
 Here, a clone simply calls the copy constructor, but for other
 types, which may have also a disk part, some code for copying
 the disk parts would be needed also. Often this is implemented
 in a special member function "Clone()".

*/

int QuadNodeType::SizeOfObj() { return sizeof(QuadNodeType); }

ListExpr QuadNodeType::Property() {
  return (nl->TwoElemList(
      nl->FiveElemList(
          nl->StringAtom("Signature"), nl->StringAtom("Example Type List"),
          nl->StringAtom("List Rep"), nl->StringAtom("Example List"),
          nl->StringAtom("Remarks")),
      nl->FiveElemList(
          nl->StringAtom("-> DATA"), nl->StringAtom(QuadNodeType::BasicType()),
          nl->StringAtom("(<x> <y> <width> <height>"
                         " <weight/nr objects> <assigned server id>"
                         " <max global object id> <child1> <child2> <child3>"
                         " <child4>)"),
          nl->StringAtom("(128.0 128.0 64.0 64.0 100 1 140 0 0 0 0)"),
          nl->StringAtom(
              "Max global object id is automatically set"
              "during restructure phases and used to limit transfer"
              "of previously transferred data. Child nodes also use"
              "qtnode type but can also be 0 if no child exists."))));
}

bool QuadNodeType::KindCheck(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual(type, QuadNodeType::BasicType()));
}

QuadTreeDistributionType::QuadTreeDistributionType(int initialWidth,
                                                   int initialHeight,
                                                   int nrServers)
    : QuadTreeDistribution(initialWidth, initialHeight, nrServers) {}

QuadTreeDistributionType::QuadTreeDistributionType(
    const QuadTreeDistributionType& qtd)
    : QuadTreeDistribution(qtd) {}

QuadTreeDistributionType::~QuadTreeDistributionType() {}

const std::string QuadTreeDistributionType::BasicType() {
  return "qtdistribution";
}

const bool QuadTreeDistributionType::checkType(const ListExpr type) {
  return listutils::isSymbol(type, BasicType());
}

Word QuadTreeDistributionType::In(const ListExpr typeInfo,
                                  const ListExpr instance, const int errorPos,
                                  ListExpr& errorInfo, bool& correct) {
  correct = false;

  Word w = SetWord(Address(0));

  if (nl->ListLength(instance) == 4) {
    ListExpr First = nl->First(instance);
    ListExpr Second = nl->Second(instance);
    ListExpr Third = nl->Third(instance);

    if (nl->IsAtom(First) && nl->AtomType(First) == IntType &&
        nl->IsAtom(Second) && nl->AtomType(Second) == IntType &&
        !nl->IsAtom(Third)) {
      if (nl->ListLength(Third) == 0) {
        cmsg.inFunError(
            "Third parameter should be a list with at least one element.");
        return w;
      } else {
        QuadTreeDistributionType* temp = new QuadTreeDistributionType(
            nl->IntValue(First), nl->IntValue(Second), nl->ListLength(Third));

        bool childCorrect = false;
        ListExpr errorInfoNode;
        Word tempChild = QuadNodeType::In(0, nl->Fourth(instance), 0,
                                          errorInfoNode, childCorrect);

        if (childCorrect) {
          temp->root = (QuadNode*)tempChild.addr;
        } else {
          temp->root = 0;
        }

        // serverIdOrder list
        for (int i = 0; i < nl->ListLength(Third); ++i) {
          ListExpr tempElem = nl->Nth(i + 1, Third);
          if (nl->IsAtom(tempElem) && nl->AtomType(tempElem) == IntType) {
            temp->serverIdOrder[i] = nl->IntValue(tempElem);
          } else {
            cmsg.inFunError("Third parameter should be a  list of int atoms.");
            return w;
          }
        }

        // weight
        temp->updateWeightVector();

        correct = true;
        w.addr = temp;
        return w;
      }
    }
  }

  cmsg.inFunError(
      "Expecting a list of with 4 elements (int,int, list of ints, "
      "QuadNode/int!");
  return w;
}

ListExpr QuadTreeDistributionType::Out(ListExpr typeInfo, Word value) {
  QuadTreeDistributionType* qtd =
      static_cast<QuadTreeDistributionType*>(value.addr);

  ListExpr idMappingList = nl->TheEmptyList();

  for (unsigned int i = qtd->serverIdOrder.size(); i > 0; i--) {
    idMappingList =
        nl->Cons(nl->IntAtom(qtd->serverIdOrder[i - 1]), idMappingList);
  }

  return nl->FourElemList(nl->IntAtom(qtd->initialWidth),
                          nl->IntAtom(qtd->initialHeight), idMappingList,
                          QuadNodeType::Out(0, SetWord(qtd->root)));
}

bool QuadTreeDistributionType::Open(SmiRecord& valueRecord, size_t& offset,
                                    const ListExpr typeInfo, Word& value) {
  int initialWidth = 0, initialHeight = 0, nrServers = 0;
  bool root = false;

  bool ok = true;
  ok = ok && valueRecord.Read(&initialWidth, sizeof(int), offset);
  offset += sizeof(int);
  ok = ok && valueRecord.Read(&initialHeight, sizeof(int), offset);
  offset += sizeof(int);
  ok = ok && valueRecord.Read(&nrServers, sizeof(int), offset);
  offset += sizeof(int);

  QuadTreeDistributionType* temp =
      new QuadTreeDistributionType(initialWidth, initialHeight, nrServers);

  for (int i = 0; i < nrServers; ++i) {
    int elem = 0;
    ok = ok && valueRecord.Read(&elem, sizeof(int), offset);
    offset += sizeof(int);
    temp->serverIdOrder[i] = elem;
  }

  ok = ok && valueRecord.Read(&root, sizeof(bool), offset);
  offset += sizeof(bool);

  if (root) {
    Word childValue = SetWord(temp->root);
    if (QuadNodeType::Open(valueRecord, offset, typeInfo, childValue)) {
      temp->root = (QuadNode*)childValue.addr;
    }
  }
  value.addr = temp;
  return ok;
}

bool QuadTreeDistributionType::Save(SmiRecord& valueRecord, size_t& offset,
                                    const ListExpr typeInfo, Word& value) {
  QuadTreeDistributionType* qtd =
      static_cast<QuadTreeDistributionType*>(value.addr);
  unsigned int nrServers = qtd->serverIdOrder.size();
  bool root = (qtd->root != 0);
  size_t size = sizeof(int);

  bool ok = true;
  ok = ok && valueRecord.Write(&qtd->initialWidth, size, offset);
  offset += size;
  ok = ok && valueRecord.Write(&qtd->initialHeight, size, offset);
  offset += size;
  ok = ok && valueRecord.Write(&nrServers, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);

  // serverIdOrder list
  for (unsigned int i = 0; i < nrServers; ++i) {
    int temp = qtd->serverIdOrder[i];
    ok = ok && valueRecord.Write(&temp, size, offset);
    offset += size;
  }

  ok = ok && valueRecord.Write(&root, sizeof(bool), offset);
  offset += sizeof(bool);

  if (root) {
    Word rootValue = SetWord(qtd->root);
    QuadNodeType::Save(valueRecord, offset, 0, rootValue);
  }

  return ok;
}

/*
 2.4 Support Functions for Persistent Storage

*/

Word QuadTreeDistributionType::Create(const ListExpr typeInfo) {
  return (SetWord(new QuadTreeDistributionType(1000, 1000, 1)));
}

void QuadTreeDistributionType::Delete(const ListExpr typeInfo, Word& w) {
  delete static_cast<QuadTreeDistributionType*>(w.addr);
  w.addr = 0;
}

void QuadTreeDistributionType::Close(const ListExpr typeInfo, Word& w) {
  delete static_cast<QuadTreeDistributionType*>(w.addr);
  w.addr = 0;
}

Word QuadTreeDistributionType::Clone(const ListExpr typeInfo, const Word& w) {
  QuadTreeDistributionType* p = static_cast<QuadTreeDistributionType*>(w.addr);
  return SetWord(new QuadTreeDistributionType(*p));
}
/*
 Here, a clone simply calls the copy constructor, but for other
 types, which may have also a disk part, some code for copying
 the disk parts would be needed also. Often this is implemented
 in a special member function "Clone()".

*/

int QuadTreeDistributionType::SizeOfObj() {
  return sizeof(QuadTreeDistributionType);
}

ListExpr QuadTreeDistributionType::Property() {
  return (nl->TwoElemList(
      nl->FiveElemList(
          nl->StringAtom("Signature"), nl->StringAtom("Example Type List"),
          nl->StringAtom("List Rep"), nl->StringAtom("Example List"),
          nl->StringAtom("Remarks")),
      nl->FiveElemList(
          nl->StringAtom("-> DATA"),
          nl->StringAtom(QuadTreeDistributionType::BasicType()),
          nl->StringAtom("(<initialWidth> <initialHeight> "
                         "(serverIdx1 ... serverIdxN) <root node>)"),
          nl->StringAtom("(1000 1000 (0 1 2 3 4) 0)"),
          nl->StringAtom("Initial width and height are only used"
                         " when no root node is specified. Nodes use the qtnode"
                         " type, but when no node is specified 0 can be used"
                         " as a placeholder/NULL-pointer. The server id list"
                         " specifies the order in which servers are distributed"
                         " along the distribution path inside the data"
                         " structure."))));
}

bool QuadTreeDistributionType::KindCheck(ListExpr type, ListExpr& errorInfo) {
  return (nl->IsEqual(type, QuadTreeDistributionType::BasicType()));
}
}
