/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Some commonly used types for the BTree2 Algebra 

[TOC]

0 Overview

*/
#ifndef BTREE2_TYPES_H
#define BTREE2_TYPES_H


#include "SecondoSMI.h"
#include "StandardTypes.h"

#include "TupleIdentifier.h"
#include "Attribute.h"
#include "IndexableAttribute.h"

namespace BTree2Algebra {

typedef SmiRecordId NodeId;
typedef void* NoneType;

struct StatisticStruct {
  struct {
    int NumberOfNodes;
    int UnderflowNodes;
    int Entries;
    int MissingEntries;
    int BytesWasted;
  } Internal, Leaf;
};

template <typename X>
inline Attribute* entry2Attribute(const X& value) {
  return 0;
}

template <>
inline Attribute* entry2Attribute<double>(const double& value) {
  return new CcReal(value);
}

template <>
inline Attribute* entry2Attribute<int>(const int& value) {
  return new CcInt(value);
}

template <>
inline Attribute* entry2Attribute<bool>(const bool& value) {
  return new CcBool(true,value);
}

template <>
inline Attribute* entry2Attribute<TupleId>(const TupleId& value) {
  TupleIdentifier* n = new TupleIdentifier(true);
  n->SetTid(value);
  return n;
}

template <>
inline Attribute* entry2Attribute<std::string>(const std::string& value) {
  std::string cop = value;
  return new CcString(true,cop);
}

template <>
inline Attribute* entry2Attribute<Attribute*>(Attribute * const & value) {
  return value->Copy();
}

template <>
inline Attribute* entry2Attribute<IndexableAttribute*>(
                                      IndexableAttribute* const & value) {
  return value->Copy();
}

// ----
//

template <typename X>
inline X Attribute2Entry(Attribute* a) {
  return 0;
}

template <>
inline double Attribute2Entry<double>(Attribute* a) {
  return ((CcReal*) a)->GetRealval();
}

template <>
inline int Attribute2Entry<int>(Attribute* a) {
  return ((CcInt*) a)->GetIntval();
}

template <>
inline bool Attribute2Entry<bool>(Attribute* a) {
  return ((CcBool*) a)->GetBoolval();
}

template <>
inline std::string Attribute2Entry<std::string>(Attribute* a) {
  return (char*) ((CcString*) a)->GetStringval();
}

template <>
inline TupleId Attribute2Entry<TupleId>(Attribute* a) {
  return ((TupleIdentifier*) a)->GetTid();
}

template <>
inline Attribute* Attribute2Entry<Attribute*>(Attribute* a) {
  return a;
}

template <>
inline IndexableAttribute* Attribute2Entry<IndexableAttribute*>(Attribute* a) {
  return (IndexableAttribute*) a;
}

} // end namespace 

#endif
