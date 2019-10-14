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

*/

#include "Include.h"

using namespace std;

namespace nr2a {

/*
The default constructor is used for casting in SECONDO and therefore empty.

*/
NRel::NRel()
{
  // Do not use!
}

/*
Constructors for building an empty relation or cloning an existing one.

*/
NRel::NRel(const ListExpr typeInfo)
    : m_data(new Relation(nl->TwoElemList(nl->SymbolAtom("rel"),
        nl->Second(typeInfo))))
{
}

NRel::NRel(const ListExpr typeInfo, Relation * const data)
    : m_data(data)
{
}

NRel::~NRel()
{
  if (m_data != NULL)
  {
    m_data->Close();
  }
}

/*
The "In"[2] function uses the functionality of the underlying "Relation"[2]
class handling the data internally.

*/
/*static*/Word NRel::In(const ListExpr typeInfo, const ListExpr instance,
    const int errorPos, ListExpr& errorInfo, bool& correct)
{
  ListExpr typeInfoRel = nl->TwoElemList(nl->SymbolAtom("rel"),
      nl->Second(typeInfo));
  Relation* data = (Relation*) Relation::In(typeInfoRel, instance, errorPos,
      errorInfo, correct);
  NRel * newNrel = new NRel(typeInfo, data);
  return Word(newNrel);
}

/*
The same is done for any other typical method of the interface to SECONDOs
core.

*/
/*static*/ListExpr NRel::Out(ListExpr typeInfo, Word value)
{
  ListExpr typeInfoRel = nl->TwoElemList(nl->SymbolAtom("rel"),
      nl->Second(typeInfo));
  NRel * nrel = (NRel*) value.addr;
  Relation* rel = nrel->m_data;
  ListExpr result = Relation::Out(typeInfoRel, rel->MakeScan());
  AutoWrite(result);
  return result;
}

/*static*/bool NRel::Open(SmiRecord& valueRecord, size_t& offset,
    const ListExpr typeInfo, Word& value)
{
  ListExpr typeInfoRel = nl->TwoElemList(nl->SymbolAtom("rel"),
      nl->Second(typeInfo));
  NRel* nrel = new NRel(typeInfo, Relation::Open(valueRecord, offset,
      typeInfoRel));
  value.addr = nrel;
  return (nrel->m_data != NULL);
}

/*static*/bool NRel::Save(SmiRecord& valueRecord, size_t& offset,
    const ListExpr typeInfo, Word& value)
{
  NRel * nrel = (NRel*) value.addr;
  bool result = nrel->m_data->Save(valueRecord, offset, typeInfo);
  return result;
}

/*static*/Word NRel::Create(const ListExpr typeInfo)
{
  return Word(new NRel(typeInfo));
}

/*static*/void NRel::Delete(const ListExpr typeInfo, Word& w)
{
  NRel * nrel = (NRel*) w.addr;
  nrel->m_data->Delete();
  nrel->m_data = NULL;
  delete nrel;
  w.addr = NULL;
}

/*static*/void NRel::Close(const ListExpr typeInfo, Word& w)
{
  NRel * nrel = (NRel*) w.addr;
  nrel->m_data->Close();
  nrel->m_data = NULL;
  delete nrel;
  w.addr = NULL;
}

/*static*/Word NRel::Clone(const ListExpr typeInfo, const Word& w)
{
  NRel * other = (NRel*) w.addr;
  return Word(new NRel(typeInfo, other->m_data->Clone()));
}

/*
While type checking the type is checked for "nrel2"[2], the rest is done by a
function of the "listutils"[2] checking for a correct definition of a tuple.

*/
/*static*/bool NRel::CheckType(ListExpr type, ListExpr& errorInfo)
{
  AutoWrite(type);
  bool result = false;
  if (nl->HasLength(type, 2) &&
      listutils::isSymbol(nl->First(type), BasicType()))
  {
    result = am->CheckKind(Kind::TUPLE(), nl->Second(type), errorInfo);
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(
        nl->IntAtom(ERR_IN_TYPE_EXPRESSION),
        nl->IntAtom(1),
        nl->IsAtom(type)?type:nl->First(type)));
    result = false;
  }
  return result;
}

/*
The size of the object is analogous to the size of "Relation"[2].

*/
/*static*/int NRel::SizeOfObj()
{
  return sizeof(NRel);
}

/*
The nested relations symbol value used in nested lists.

*/
/*static*/const string NRel::BasicType()
{
  return "nrel2";
}

/*
Purges all entries resulting in an empty relation.

*/
void NRel::Clear()
{
  m_data->Clear();
}

/*
To append a tuple to the nested relation it is appended to the internal
relation.

*/
void NRel::AppendTuple(Tuple *tuple)
{
  m_data->AppendTuple(tuple);
  tuple->DeleteIfAllowed();
}

/*
If requested the class returns an iterator pointing to itself.

*/
NRelIterator *NRel::getIterator(const ListExpr tupleType) const
{
  return new NRelIterator(this, tupleType);
}

/*
To return a specific tuple by Id the internal relation is requested for the
same.

*/
Tuple * NRel::GetTuple(const ListExpr tupleType, const TupleId tid) const
{
  return m_data->GetTuple(tid, false);
}

/*
A request for the amount of tuples is dispatched to the internal relation,
also.

*/
unsigned long int NRel::GetTupleCount() const
{
  return m_data->GetNoTuples();
}

/*
A new iterator is constructed by its belonging relation and the type thereof.
Both parameters are passed to the class for relations' iterators, which uses
a special representation of the tuple type.

*/
NRelIterator::NRelIterator(const NRel * const nrel, const ListExpr
    tupleType)
{
  m_nrel = nrel;
  ListExpr tupleTypeNumeric = SecondoSystem::GetCatalog()->NumericType(
      tupleType);
  m_tupleTypeObject = new TupleType(tupleTypeNumeric);
  m_relIterator = new RelationIterator(*(m_nrel->m_data),
      m_tupleTypeObject);
}

NRelIterator::~NRelIterator()
{
  delete m_relIterator;
  delete m_tupleTypeObject;
}

/*
Call this function to use the iterator. It requests the next tuple (if any)
and advances (or invalidates) the internal pointer.

*/
Tuple * NRelIterator::getNextTuple()
{
  Tuple* result = NULL;
  if (m_nrel->GetTupleCount() > 0)
  {
    result = m_relIterator->GetNextTuple();
  }
  return result;
}

} /* namespace nr2a*/
