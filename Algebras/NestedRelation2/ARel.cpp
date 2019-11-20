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

#include "Symbols.h"
#include "TypeConstructor.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

#include "../FText/FTextAlgebra.h"

#include <limits>
#include <limits.h>

//#define DEBUG_AREL

using namespace std;

namespace nr2a {

#ifdef DEBUG_AREL
  static int arelsCreated = 0;
  static int arelsDeleted = 0;
  static int arelsDestroyed = 0;
  static int arelsExisting = 0;
#endif

 //const int cTss = sizeof(u_int16_t); // Tuple size's size
 //const int cBss = sizeof(u_int32_t); // Block size's size
const int cFss = sizeof(SmiSize);   // Flob size's size

ARel::Info::Info()
{
  name = ARel::BasicType();
  signature = "-> " + Kind::DATA();
  typeExample = ARel::BasicType();
  listRep = "(nrel2(tuple([Keyword:string, Occurences: "
      "arel2(tuple([Page: int, Line: int]))])))";
  valueExample = "((\"database\" ((1 1) (2 10))) "
      "(\"system\" ((1 3) (3 14) (8 15))))";
  remarks =
      "Represents a nested relations subrelation. It can contain "
          "further attribute relations as attributes (arel2).";
}

/*
Constructor used for SECONDOs casting technique.

*/
ARel::ARel()
{
  // Do not use!

#ifdef DEBUG_AREL
  cout << "ARels  -  : " << arelsExisting << "  " << arelsCreated
      << "  "  <<  this << " " << "XXXXXX" << endl;
#endif
}

/*
Constructor for building an empty attribute relation.

*/
ARel::ARel(const ListExpr typeInfo)
    : Attribute(false), m_indexStore(0), m_tupleStore(0), m_dataStore(0)
{
#ifdef DEBUG_AREL
  arelsCreated++;
  arelsExisting++;
  cout << "ARels  -> : " << arelsExisting << "  " << arelsCreated
      << "  "  <<  this << " " << nl->ToString(typeInfo) << endl;
#endif
  Init(typeInfo);
}

/*virtual*/ARel::~ARel()
{
#ifdef DEBUG_AREL
  arelsDeleted++;
  arelsExisting--;
  cout << "ARels <-  : " << arelsExisting << "  " << arelsDeleted
      << "  "  << this << " " << "---" << endl;
#endif
}

/*
Initialisation function writing an empty relation, which contains only a
header.

*/
void ARel::Init(const ListExpr typeInfo)
{
  AutoWrite(typeInfo);
  bool def = Nr2aHelper::IsNestedRelation(typeInfo);
  SetDefined(def);
  if (def)
  {
    WriteType(typeInfo);
  }
  m_tupleCount = 0;
  m_hashValue = 0;
}

/*
This functions writes the relations type into the "Flob"[2]. It is represented
in nested list format as a string.

*/
void ARel::WriteType(const ListExpr type)
{
  string typeString = nl->ToString(nl->Second(nl->Second(type)));
  int typeLen = typeString.length();
  m_tupleStore.resize(typeLen);
  m_tupleStore.write(typeString.c_str(), typeLen, 0);
}

/*
Read the headers content from the "Flob"[2] into a variable. The nested list
expression is parsed from the string stored.

*/
ListExpr ARel::ReadType() const
{
  WriteFlob(&m_indexStore, 0, 100);
  WriteFlob(&m_tupleStore, 0, 250);
  ListExpr result = nl->TheEmptyList();
  SmiSize lenOfType = 0;
  if (m_indexStore.getSize() != 0)
  {
    m_indexStore.Get(0, lenOfType);
    if (lenOfType == 0)
    {
      lenOfType = m_tupleStore.getSize();
    }
  }
  else
  {
    lenOfType = m_tupleStore.getSize();
  }
  char * buffer = new char[lenOfType+1];
  memset(buffer, 0, lenOfType);
  m_tupleStore.read(buffer, lenOfType, 0);
  string typeString(buffer, lenOfType);
  delete[] buffer;
  nl->ReadFromString(typeString, result);
  return result;
}

/*
Function to write a tuple's data into the "Flob"[2] at the given position.

*/
void ARel::WriteTuple(Tuple *tuple)
{
  AlgebraManager * algMan = SecondoSystem::GetAlgebraManager();
/*
Prepare the tuple's attribute's Flob data one after another.

*/
  m_indexStore.Put(m_tupleCount, m_tupleStore.getSize());
  FlobWriter tupleWriter(m_tupleStore, m_tupleStore.getSize());
  FlobWriter dataWriter(m_dataStore, 0);
  for(int attrNo = 0; attrNo < tuple->GetNoAttributes(); attrNo++)
  {
    Attribute * attr = tuple->GetAttribute(attrNo);
    SmiSize attrSize = attr->SerializedSize();
    if (attrSize == 0)
    {
      const AttributeType &attrType =
          tuple->GetTupleType()->GetAttributeType(attrNo);
      attrSize = algMan->SizeOfObj(attrType.algId, attrType.typeId)
          ();
    }
    int numFlobs = attr->NumOfFLOBs();
    tupleWriter.Write(attrSize);
    tupleWriter.Write(numFlobs);
    char * bufferAttr = new char[attrSize];
    attr->Serialize(bufferAttr, attrSize, 0);
    tupleWriter.Write(bufferAttr, attrSize);
    delete[] bufferAttr;
    if (numFlobs > 0)
    {
      for(int flobNo = 0; flobNo < numFlobs; flobNo++)
      {
        Flob *flob = attr->GetFLOB(flobNo);
        const SmiSize flobSize = flob->getSize();
        SmiSize offsetData = m_dataStore.getSize();
        tupleWriter.Write(offsetData);
        char *buffer = new char[flobSize];
        memset(buffer, 0, flobSize);
        flob->read(buffer, flobSize, 0);
        dataWriter.Seek(offsetData);
        dataWriter.Write(flobSize);
        dataWriter.Write(buffer, flobSize);
        delete[] buffer;
      }
    }
  }
}

/*
Read a tuple's data from the "ARel"[2]s "Flob"[2]s. For the "Flob"[2]s of the
attributes of the "ARel"[2] are stored in the "ARel"[2]'s "Flob"[2].

*/
void ARel::ReadTuple(Tuple* tuple, const SmiSize & offset,
    const SmiSize & length, ListExpr tupleType) const
{
  AlgebraManager * algMan = SecondoSystem::GetAlgebraManager();
  SmiSize flobOffset = 0;
  FlobReader tupleReader(m_tupleStore, offset);
  FlobReader dataReader(m_dataStore, 0);
  if(nl->IsEmpty(tupleType)){
    tupleType = GetTupleType(); 
  }
  ListExpr attrList = nl->Second(tupleType);
  for(int attrNo = 0; attrNo < tuple->GetNoAttributes(); attrNo++)
  {
    size_t attrSize = 0;
    tupleReader.Read(attrSize);
    int numFlobs = 0;
    tupleReader.Read(numFlobs);

    const AttributeType &attrType =
        tuple->GetTupleType()->GetAttributeType(attrNo);

    ListExpr attrTypeList = nl->Second(nl->First(attrList));
    attrList = nl->Rest(attrList);

    const Word &w = algMan->CreateObj(attrType.algId, attrType.typeId)
        (attrTypeList);
    Attribute *attr = (Attribute*) w.addr;




    assert(numFlobs == attr->NumOfFLOBs()); //number of flobs is fixed per type

    std::vector<Flob> realFlobs;
    if (numFlobs > 0)
    {
      realFlobs.reserve(numFlobs);
      for(int flobNo = 0; flobNo < numFlobs; flobNo++)
      {
        realFlobs.push_back(*attr->GetFLOB(flobNo));
      }
    }

    char * attrBuffer = new char[attrSize];
    tupleReader.Read(attrBuffer, attrSize);
    attr->Rebuild(attrBuffer, attrSize);
    delete[] attrBuffer;
    attr = (Attribute*) (algMan->Cast(attrType.algId, attrType.typeId)(attr));

    for(int flobNo = 0; flobNo < numFlobs; flobNo++)
    {
      tupleReader.Read(flobOffset);
      dataReader.Seek(flobOffset);
      SmiSize flobSize;
      dataReader.Read(flobSize);
      flobOffset+=cFss;
      Flob *flob = attr->GetFLOB(flobNo);
      memset((char*)flob, 0, sizeof(Flob));
      *flob = realFlobs[flobNo];
      FlobWriter attrFlobWriter(*flob, 0);
      attrFlobWriter.SetFrom(dataReader, flobSize);
    }
    tuple->PutAttribute(attrNo, attr);
  }
}

/*
The following function appends a tuple to the attribute relation. It writes the
tuple's data and updates the relation's meta data (tuple count and hash value).

The hashing is based on combining all attributes' hashes using XOR. This shall
preserve a good distribution of ones and zeroes and will not lead to overflows.
To avoid similar or equal hashes compensating each other, the hash is rotated.

The code used for rotating is inspired by John Regehr's thoughts on a "Safe,
Efficient, and Portable Rotate in C/C++"[9], which are published at
~http://blog.regehr.org/archives/1063~.

*/
void ARel::AppendTuple(Tuple * tuple)
{
  const size_t n = 1;
  const size_t c = (sizeof(size_t)*CHAR_BIT)-1;

  WriteTuple(tuple);
  size_t tupleIndex = ++m_tupleCount;

  const int attrCount = tuple->GetNoAttributes();
  for(int attrIndex = 0; attrIndex < attrCount; attrIndex++)
  {
    m_hashValue ^= tuple->GetAttribute(attrIndex)->HashValue();
    m_hashValue = (m_hashValue<<n) | (m_hashValue>>(-n&c)); //Rotate left by n
  }
  m_hashValue ^= tupleIndex;
  m_hashValue = (m_hashValue<<n) | (m_hashValue>>(-n&c)); //Rotate left by n
  tuple->DeleteIfAllowed();
}

/*
Use this function to request a tuple in its object representation by its
"TupleId"[2]. It needs the tuples type to interpret the read data.

*/
Tuple * ARel::GetTuple(const TupleId tid) const
{
  Tuple * result = NULL;
  long idx = tid-1;
  ARelIterator iter(this, 0,idx);
  result = iter.getNextTuple();
  return result;
}

/*
Returns the tuples' type as a nested list.

*/
ListExpr ARel::GetTupleType() const
{
  return Nr2aHelper::TupleOf(ReadType());
}

/*
Returns the "ARel"[2]'s type as a nested list.

*/
ListExpr ARel::GetType() const
{
  return nl->TwoElemList(nl->SymbolAtom(BasicType()), GetTupleType());
}

/*
This function is used by the "ARelIterator"[2] to request a tuple in its object
representation by its position in the "Flob"[2]. It needs the tuples type to
interpret the read data.

*/
Tuple *
ARel::GetTupleByOffset(const SmiSize & offset, SmiSize & tupleSize,
                       TupleType* tupleType,
                       ListExpr tupleTypeList) const
{
  Tuple *newTuple;
  assert(tupleType);
  newTuple = new Tuple(tupleType);
  ReadTuple(newTuple, offset, tupleSize, tupleTypeList);
  return newTuple;
}

/*
Resets the attribute relation's content to these of an empty attribute
relation, keeping the object and its "Flob"[2]s.

*/
void
ARel::Clear()
{
  Init(GetType());
}

/*
The "In"[2] function mainly maps the given arguments to "Tuple"[2]s "In"[2]
function and then appends the read tuple to its storage.

*/
/*static*/Word ARel::In(const ListExpr typeInfo, const ListExpr instance,
    const int errorPos, ListExpr& errorInfo, bool& correct)
{
  Word result;
  result.addr = NULL;
  try
  {
    {
      ARel * arel = new ARel(typeInfo);

      //Tuple::In expects the first element (not the whole list) to be the
      //tuple's description
      ListExpr tupleTypeInfo = nl->OneElemList(nl->Second(typeInfo));

      int errorPosTuple = 0;
      correct = true;
      if (!nl->IsAtom(instance))
      {
        listForeach(instance, current)
        {
          Tuple *currentTuple = Tuple::In(tupleTypeInfo, current,
              ++errorPosTuple, errorInfo, correct);
          if (!correct)
          {
            break;
          }
          arel->AppendTuple(currentTuple);
        }
      }
      else
      {
        if (listutils::isSymbolUndefined(instance))
        {
          arel->SetDefined(false);
        }
        else
        {
          correct = false;
        }
      }
      if (!correct)
      {
        arel->DeleteIfAllowed();
        arel = new ARel(typeInfo);
      }
      result.addr = arel;
    }
  }
  catch (Nr2aException &e)
  {
    correct = false;
    result.addr = NULL;
    cmsg.inFunError(e.what());
  }
  catch (std::bad_alloc& e)
  {
    correct = false;
    result.addr = NULL;
    string s(e.what());
    Write(true, s);
  }
  return result;
}

/*
Output of an attribute relation in nested list format.

*/
/*static*/ListExpr ARel::Out(ListExpr typeInfo, Word value)
{
  ARel* arel = static_cast<ARel*>(value.addr);
  ListExpr result = arel->ToList(typeInfo);
  return result;
}

/*
Creation, deletion and cloning of attribute relations is pretty straight
forward.

*/
/*static*/Word ARel::Create(const ListExpr typeInfo)
{
  ARel * arel = new ARel(typeInfo);
#ifdef DEBUG_AREL
  cout << "ARels  ^c : " << arelsExisting << "  " << arelsCreated
      << "  "  <<  arel << " " << nl->ToString(typeInfo) << endl;
#endif
  AutoWrite(typeInfo);
  return (SetWord(arel));
}

/*static*/void ARel::Delete(const ListExpr typeInfo, Word& w)
{
  AutoWrite(typeInfo);
  if (w.addr != NULL)
  {
    ARel* val = static_cast<ARel*>(w.addr);
    val->m_indexStore.Destroy();
    val->m_tupleStore.destroy();
    val->m_dataStore.destroy();
    delete val;
    w.addr = 0;
#ifdef DEBUG_AREL
    arelsDestroyed++;
    cout << "ARels XXX : " << arelsExisting << "  "  << arelsDestroyed
        << "  "  <<  &val << " " << nl->ToString(typeInfo) << endl;
#endif
  }
}

/*static*/Word ARel::Clone(const ListExpr typeInfo, const Word& w)
{
  ARel *p = static_cast<ARel*>(w.addr);
  ARel *result = new ARel(typeInfo);
  result->Imitate(p);
  return SetWord(result);
}

/*
The type of an attribute relation is any valid tuple type wrapped by "arel2"[2].

*/
/*static*/bool ARel::CheckType(ListExpr type, ListExpr& errorInfo)
{
  bool result = false;
  if (nl->HasLength(type, 2) &&
      listutils::isSymbol(nl->First(type), BasicType()))
  {
    result = am->CheckKind(Kind::TUPLE(), nl->Second(type), errorInfo);
  }
  else
  {
    errorInfo = nl->Append(nl->End(errorInfo),
      nl->ThreeElemList(
        nl->IntAtom(ERR_IN_TYPE_EXPRESSION),
        nl->IntAtom(1),
        nl->IsAtom(type)?type:nl->First(type)));
    result = false;
  }
  return result;
}

/*
This function returns the objects's size.

*/
/*static*/int ARel::SizeOfObj()
{
  return sizeof(ARel);
}

/*
Attribute relations have exactly one "Flob"[2].

*/
int ARel::NumOfFLOBs() const
{
  return 3;
}

/*
Apart from error handling the object's only "Flob"[1] is returned here.

*/
Flob *ARel::GetFLOB(const int i)
{
  Flob *result = NULL;
  switch(i)
  {
    case 0:
      result = &m_indexStore;
      break;
    case 1:
      result = &m_tupleStore;
      break;
    case 2:
      result = &m_dataStore;
      break;
    default:
    {
      std::string msg =
          "GetFLOB must be called with an integer value in the range 0<=x<"
          + Nr2aHelper::IntToString(NumOfFLOBs());
      throw Nr2aException(msg);
      result = &m_tupleStore;
    }
  }
  return result;
}

/*
Returns an attribute relations basic type "arel2"[2].

*/
/*static*/const string ARel::BasicType()
{
  return "arel2";
}

/*
Returns wether two attribute relations are equal or defines their order
for strict sorting.
Unfortunately the function forces to determine the order of the objects. Using
the hash is no option, because it will not distinguish betweeen 1 and -1.
The relations are compared analogously to comparing a text in lexicographical
order. As the letters are compared one by one, here the attributes are
compared ordered by tuples. As shorter words are considered to be ordered
before longer ones here smaller relations go before larger ones.

*/
/*virtual*/int ARel::Compare(const Attribute *rhs) const
{
  const int cUndecided = -2;
  const int cLeftSmaller = -1;
  const int cBothEqual = 0;
  const int cRightSmaller = 1;

  int result = cUndecided;

  if(this != rhs) // Identity means equality (Avoids full scan)
  {
    const ARel * rightRel = static_cast<const ARel*>(rhs);
    ARelIterator *leftIter = new ARelIterator(this,0,0);
    ARelIterator *rightIter = new ARelIterator(rightRel,0,0);
    // Iterate tuples
    while(result == cUndecided)
    {
      const Tuple *leftTuple = leftIter->getNextTuple();
      const Tuple *rightTuple = rightIter->getNextTuple();
      if (leftTuple != NULL)
      {
        if (rightTuple != NULL)
        {
          const int attributeCount = leftTuple->GetNoAttributes();
          assert(rightTuple->GetNoAttributes() == attributeCount);
          int attributeIndex = 0;
          //Iterate tuple's attributes
          while((attributeIndex < attributeCount) && (result == cUndecided))
          {
            const Attribute *leftAttribute =
                leftTuple->GetAttribute(attributeIndex);
            const Attribute *rightAttribute =
                rightTuple->GetAttribute(attributeIndex);
            const int attrComp = leftAttribute->Compare(rightAttribute);
            if(attrComp == 0)
              result = cUndecided;
            else if (attrComp < 0)
              result = cLeftSmaller;
            else
              result = cRightSmaller;
            ++attributeIndex;
          }
        }
        else
        {
          result = cRightSmaller;
        }
      }
      else
      {
        if (rightTuple != NULL)
        {
          result = cLeftSmaller;
        }
        else
        {
          result = cBothEqual;
        }
      }
      delete leftIter;
      delete rightIter;
    }
  }
  return result;
}

/*
The attribute relation's hash value.

*/
/*virtual*/size_t ARel::HashValue() const
{
  return m_hashValue;
}

/*
The concept of being adjacent to each other is not usable for relations, so
they are never "adjacent" to each other.

*/
/*virtual*/bool ARel::Adjacent(const Attribute *attrib) const
{
  return false;
}

/*
Discards the object's own values and resembles the ones of the given other
object.

*/
/*virtual*/void ARel::Imitate(const ARel * const prototype)
{
  SetDefined(prototype->IsDefined());
  m_tupleCount = prototype->m_tupleCount;
  m_hashValue = prototype->m_hashValue;
  if (IsDefined())
  {
    m_indexStore.copyFrom(prototype->m_indexStore);
    m_tupleStore.copyFrom(prototype->m_tupleStore);
    m_dataStore.copyFrom(prototype->m_dataStore);
  }
  else
  {
    m_indexStore.clean();
    m_tupleStore.clean();
    m_dataStore.clean();
  }
#ifdef DEBUG_AREL
  cout << "ARels  ^i : " << arelsExisting << "  " << arelsCreated
      << "  "  <<  this << " " << nl->ToString(this->GetType()) << endl;
#endif
}

/*
Returns the tuple count, which is stored in memory for performance reasons.
For persistency it is stored in the objects "Flob"[2].

*/
unsigned long int ARel::GetTupleCount() const
{
  return IsDefined() ? m_tupleCount : 0;
}

/*
To return a nested list representation of the attribute relation call this
method.

*/
ListExpr ARel::ToList(ListExpr typeInfo) const
{
  ListExpr result = nl->TheEmptyList();
  ListBuilder list;

  if (IsDefined())
  {
    ListExpr tupleTypeFull = nl->Second(typeInfo);
    Tuple * tuple = NULL;
    ListExpr tupleList;

    ARelIterator *iter = new ARelIterator(this,0,0);

    while ((tuple = iter->getNextTuple()) != NULL)
    {
      tupleList = tuple->Out(
          nl->TwoElemList(tupleTypeFull, nl->TheEmptyList()));
      tuple->DeleteIfAllowed();
      list.Append(tupleList);
    }

    delete iter;

    result = list.GetList();
  }
  else
  {
    result = nl->SymbolAtom(Symbol::UNDEFINED());
  }

  return result;
}

/*
This function returns the attribute relation's size.

*/
/*virtual*/size_t ARel::Sizeof() const
{
  return SizeOfObj();
}

/*
"Clone"[2] returns a copy of the object it was called on.

*/
/*virtual*/Attribute *ARel::Clone() const
{
  // tp Generic copy method works for classes not containing references
  ARel* p = new ARel(nl->TheEmptyList());
  p->Imitate(static_cast<const ARel * const >(this));
  return (Attribute*) p;
}

/*
"Copy"[2] works the other way round and makes the current object act like the
given one.

*/
/*virtual*/void ARel::CopyFrom(const Attribute* right)
{
  this->Imitate(static_cast<const ARel * const >(right));
}

inline /*virtual*/ size_t ARel::SerializedSize() const
{
  return sizeof(ARel);
}

inline /*virtual*/ void ARel::Rebuild(char* storage, size_t sz)
{
  Attribute::Rebuild(storage, sz);

#ifdef DEBUG_AREL
  cout << "ARels  >  : " << arelsExisting << "  " << arelsCreated
      << "  "  <<  this << " " << "---" << endl;
#endif
}

inline /*virtual*/ void ARel::Serialize(char* storage, size_t sz,
    size_t offset) const
{
#ifdef DEBUG_AREL
  cout << "ARels  <  : " << arelsExisting << "  " << arelsCreated
      << "  "  <<  this << " " << "---" << endl;
#endif

  Attribute::Serialize(storage, sz, offset);
}


/*
Iterators should not be constructed explicitly, but by the "getIterator"[2]
function of the attribute relation.

*/
ARelIterator::ARelIterator(const ARel * const arel,
    TupleType* _tupleType, ListExpr _tupleTypeList,
    const TupleId id /*= 0*/)
{
  if(_tupleType){
     tupleTypeList = _tupleTypeList;
     tupleType = _tupleType;
     tupleType->IncReference();
  } else {
      tupleTypeList = arel->GetTupleType();
      tupleType = new TupleType(tupleTypeList);
  }
  m_arel = arel;
  m_index = id;
  if (m_arel->m_indexStore.getSize() >= sizeof(m_offset))
  {
    m_arel->m_indexStore.Get(m_index, m_offset);
  }
  else
  {
    m_offset = m_arel->m_tupleStore.getSize();
  }
}

/*
Call this method to request a new tuple from the iterator.

*/
Tuple * ARelIterator::getNextTuple()
{
  Tuple * result = NULL;
  SmiSize tupleSize = 0;

  if (m_index < m_arel->m_tupleCount)
  {
    ++m_index;
    SmiSize nextOffset;
    if(m_index < m_arel->m_tupleCount)
    {
      m_arel->m_indexStore.Get(m_index, nextOffset);
    }
    else
    {
      nextOffset = m_arel->m_tupleStore.getSize();
    }
    tupleSize = nextOffset-m_offset;
    result = m_arel->GetTupleByOffset(m_offset, tupleSize, tupleType,
                                      tupleTypeList);
    m_offset=nextOffset;
  }

  return result;
}

/*
Creates a new "FlobWriter"[2].

*/
ARel::FlobWriter::FlobWriter(Flob &flob, SmiSize offset)
: m_flob(flob), m_offset(offset)
{
  // intentionally empty
}

/*
To write data into a flob the following two functions can be used. The first
can be used to write an object or simple types value. Its size is determined
automatically, by its type. Note, that the internal pointer is advanced,
which allows consecutive calls to "Write"[2] for writing blocks of values.

*/
template <typename T>
void ARel::FlobWriter::Write(const T &var)
{
  m_flob.write((char*)&var, sizeof(T), m_offset);
  m_offset += sizeof(T);
}

/*
To write a byte array into a flob use the following function.

*/
void ARel::FlobWriter::Write(const char *buffer, const SmiSize length)
{
  m_flob.write(buffer, length, m_offset);
  m_offset += length;
}

/*
"CopyFrom"[2] reads a byte sequence from the given "source"[2] and writes it to
this object's buffer. Both "Flob"[2]s use its own current internal pointer.

*/
void ARel::FlobWriter::CopyFrom(FlobReader &source,
    const SmiSize length)
{
  char * buffer = new char[length];
  source.Read(buffer, length);
  this->Write(buffer, length);
  delete[] buffer;
}

/*
"SetFrom"[2] resizes its "Flob"[2] to the given "length"[2] and fills it with a
byte sequence of equal length, read from the given "FlobReader"[2].

*/
void ARel::FlobWriter::SetFrom(FlobReader &source,
    const SmiSize length)
{
  m_flob.resize(length);
  m_offset = 0;
  CopyFrom(source, length);
}

/*
Use "Seek"[2] to set the internal pointer to the given "offset"[2].

*/
void ARel::FlobWriter::Seek(const SmiSize offset)
{
  m_offset = offset;
}

/*
"FlobReader"[2]s contructor.

*/
ARel::FlobReader::FlobReader(const Flob &flob, const SmiSize offset)
: m_flob(flob), m_offset(offset)
{
  // intentionally empty
}

/*
"Read"[2] sets the value of the given "variable"[2] to the byte value read
from the current position or fills a given "buffer"[2] with a byte sequence of
given "length"[2].

*/
template <typename T>
void ARel::FlobReader::Read(T &variable)
{
  m_flob.read((char*)&variable, sizeof(T), m_offset);
  m_offset += sizeof(T);
}

void ARel::FlobReader::Read(char *buffer, const SmiSize length)
{
  memset(buffer, 0, length);
  m_flob.read(buffer, length, m_offset);
  m_offset += length;
}

/*
Use "Seek"[2] to set the internal pointer to the given "offset"[2].

*/
void ARel::FlobReader::Seek(const SmiSize offset)
{
  m_offset = offset;
}

} // namespace nr2a

