/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Relation Algebra

[1] Separate part of persistent data representation

[1] Using Storage Manager Berkeley DB

June 1996 Claudia Freundorfer

May 2002 Frank Hoffmann port to C++

November 7, 2002 RHG Corrected the type mapping of ~tcount~.

November 30, 2002 RHG Introduced a function ~RelPersistValue~ instead of
~DefaultPersistValue~ which keeps relations that have been built in memory in a
small cache, so that they need not be rebuilt from then on.

[TOC]

*/
#ifdef RELALG_PERSISTENT  

using namespace std;

#include "RelationAlgebra.h"

class TupleAttributesInfo
{

    TupleAttributes* tupleType;
    AttributeType* attrTypes;

  public:

    TupleAttributesInfo (ListExpr typeInfo, ListExpr value);

};

TupleAttributesInfo::TupleAttributesInfo (ListExpr typeInfo, ListExpr value)
{
  ListExpr attrlist, valuelist,first,firstvalue, errorInfo;
  Word attr;
  int algebraId, typeId, noofattrs;
  attrTypes = new AttributeType[nl->ListLength(value)];
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  bool valueCorrect;

  attrlist = nl->Second(typeInfo);
  valuelist = value;
  noofattrs = 0;

  while (!nl->IsEmpty(attrlist))
  {
    first = nl->First(attrlist);
    attrlist = nl->Rest(attrlist);

    algebraId = nl->IntValue(nl->First(nl->Second(first)));
    typeId = nl->IntValue(nl->Second(nl->Second(first)));

    firstvalue = nl->First(valuelist);
    valuelist = nl->Rest(valuelist);
    attr = (algM->InObj(algebraId, typeId))(nl->Second(first),
              firstvalue, 0, errorInfo, valueCorrect);
    if (valueCorrect)
    {
      AttributeType attrtype = { algebraId, typeId, ((Attribute*)attr.addr)->Sizeof() };
      attrTypes[noofattrs] = attrtype;
      noofattrs++;
    }
  }
  tupleType = new TupleAttributes(noofattrs, attrTypes);
};

#endif

