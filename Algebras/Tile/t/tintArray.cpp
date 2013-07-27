 
/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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

*/ 

/*
SECONDO includes

*/

#include "Symbols.h"
#include "TypeConstructor.h"

/*
TileAlgebra includes

*/

#include "tintArray.h"
#include "../Constants.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Constructor tintArray does not initialize any members and
should only be used in conjunction with Cast method.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

tintArray::tintArray()
          :Attribute()
{
  
}

/*
Constructor tintArray sets defined flag of base class Attribute and
initializes all members of the class with default values.

author: Dirk Zacher
parameters: bDefined - defined flag of base class Attribute
return value: -
exceptions: -

*/
  
tintArray::tintArray(bool bDefined)
          :Attribute(bDefined),
           m_Grid(false)
{
  for(int i = 0; i < TINTARRAY_SIZE; i++)
  {
    m_Array[i] = UNDEFINED_INT;
  }
}

/*
Constructor tintArray sets defined flag of base class Attribute to defined flag
of rtintArray object and initializes all members of the class with corresponding
values of rtintArray object.

author: Dirk Zacher
parameters: rtintArray - reference to a tintArray object
return value: -
exceptions: -

*/

tintArray::tintArray(const tintArray& rtintArray)
          :Attribute(rtintArray.IsDefined())
{
  m_Grid = rtintArray.m_Grid;
  memcpy(m_Array, rtintArray.m_Array, TINTARRAY_SIZE * sizeof(int));
}

/*
Destructor deinitializes a tintArray object.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

tintArray::~tintArray()
{
  
}

/*
Operator= assigns all member values of a given tintArray object
to the corresponding member values of this object.

author: Dirk Zacher
parameters: rtintArray - reference to a tintArray object
return value: reference to this object
exceptions: -

*/

tintArray& tintArray::operator=(const tintArray& rtintArray)
{
  if(this != &rtintArray)
  {
    SetDefined(rtintArray.IsDefined());
    m_Grid = rtintArray.m_Grid;
    memcpy(m_Array, rtintArray.m_Array, TINTARRAY_SIZE * sizeof(int));
  }
  
  return *this;
}

/*
Operator== checks if this object equals rtintArray object.

author: Dirk Zacher
parameters: rtintArray - reference to a tintArray object
return value: true, if this object equals rtintArray object, otherwise false
exceptions: -

*/

bool tintArray::operator==(const tintArray& rtintArray) const
{
  bool bIsEqual = false;

  if(this != &rtintArray)
  {
    if(m_Grid == rtintArray.m_Grid &&
       memcmp(m_Array, rtintArray.m_Array, TINTARRAY_SIZE * sizeof(int)) == 0)
    {
      bIsEqual = true;
    }
  }

  return bIsEqual;
}

/*
TileAlgebra operator load loads all values of the tintArray object.

author: Dirk Zacher
parameters: -
return value: true, if all values of the tintArray object successfully loaded,
              otherwise false
exceptions: -

*/

bool tintArray::load()
{
  bool bRetVal = true;

  return bRetVal;
}

/*
Method SetGrid sets the tgrid properties of tintArray object.

author: Dirk Zacher
parameters: rX - reference to the x origin of the grid
            rY - reference to the y origin of the grid
            rLength - reference to the length of a grid cell
return value: true, if tgrid properties of tintArray object successfully set,
              otherwise false
exceptions: -

*/

bool tintArray::SetGrid(const double& rX,
                        const double& rY,
                        const double& rLength)
{
  bool bRetVal = true;

  m_Grid.SetDefined(true);
  bRetVal &= m_Grid.SetX(rX);
  bRetVal &= m_Grid.SetY(rY);
  bRetVal &= m_Grid.SetLength(rLength);

  return bRetVal;
}

/*
Method SetValue sets a value of tintArray object at given index.

author: Dirk Zacher
parameters: nIndex - index of tintArray value
            nValue - value
return value: true, if nValue was successfully set at nIndex, otherwise false
exceptions: -

*/

bool tintArray::SetValue(int nIndex, int nValue)
{
  bool bRetVal = false;

  if(IsDefined() &&
     nIndex >= 0 &&
     nIndex < TINTARRAY_SIZE)
  {
    bRetVal = true;
    m_Array[nIndex] = nValue;
  }

  return bRetVal;
}

/*
Method Adjacent checks if this object is adjacent to given Attribute object.

author: Dirk Zacher
parameters: pAttribute - a pointer to an Attribute object
return value: true, if this object is adjacent to pAttribute, otherwise false
exceptions: -

*/

bool tintArray::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

/*
Method Clone returns a copy of this object.

author: Dirk Zacher
parameters: -
return value: a pointer to a copy of this object
exceptions: -

*/

Attribute* tintArray::Clone() const
{
  Attribute* pAttribute = new tintArray(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

/*
Method Compare compares this object with given Attribute object.

author: Dirk Zacher
parameters: pAttribute - a pointer to an Attribute object
return value: -1 if this object < pAttribute object or
                 this object is undefined and pAttribute object is defined,
               0 if this object equals pAttribute object or
                 this object and pAttribute object are undefined,
               1 if this object > pAttribute object or
                 this object is defined and pAttribute object is undefined
exceptions: -

*/

int tintArray::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;
  
  if(pAttribute != 0)
  {
    const tintArray* ptintArray = dynamic_cast<const tintArray*>(pAttribute);
    
    if(ptintArray != 0)
    {
      bool bIsDefined = IsDefined();
      bool btintArrayIsDefined = ptintArray->IsDefined();
      
      if(bIsDefined == true)
      {
        if(btintArrayIsDefined == true) // defined x defined
        {
          nRetVal = memcmp(m_Array, ptintArray->m_Array,
                           TINTARRAY_SIZE * sizeof(int));
        }
        
        else // defined x undefined
        {
          nRetVal = 1;
        }
      }
      
      else
      {
        if(btintArrayIsDefined == true) // undefined x defined
        {
          nRetVal = -1;
        }
        
        else // undefined x undefined
        {
          nRetVal = 0;
        }
      }
    }
  }
  
  return nRetVal;
}

/*
Method CopyFrom assigns all member values of pAttribute object
to the corresponding member values of this object.

author: Dirk Zacher
parameters: pAttribute - a pointer to an Attribute object
return value: -
exceptions: -

*/

void tintArray::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const tintArray* ptintArray = dynamic_cast<const tintArray*>(pAttribute);
    
    if(ptintArray != 0)
    {
      *this = *ptintArray;
    }
  }
}

/*
Method HashValue returns the hash value of the tintArray object.

author: Dirk Zacher
parameters: -
return value: hash value of the tintArray object
exceptions: -

*/

size_t tintArray::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }
  
  return hashValue;
}

/*
Method Sizeof returns the size of tintArray datatype.

author: Dirk Zacher
parameters: -
return value: size of tintArray datatype
exceptions: -

*/

size_t tintArray::Sizeof() const
{
  return sizeof(tintArray);
}

/*
Method BasicType returns the typename of tintArray datatype.

author: Dirk Zacher
parameters: -
return value: typename of tintArray datatype
exceptions: -

*/

const std::string tintArray::BasicType()
{
  return "tintArray";
}

/*
Method Cast casts a void pointer to a new tintArray object.

author: Dirk Zacher
parameters: pVoid - a pointer to a memory address
return value: a pointer to a new tintArray object
exceptions: -

*/

void* tintArray::Cast(void* pVoid)
{
  return new(pVoid)tintArray;
}

/*
Method Clone clones an existing tintArray object
given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing tintArray object
return value: a Word that references a new tintArray object
exceptions: -

*/

Word tintArray::Clone(const ListExpr typeInfo,
                      const Word& rWord)
{
  Word word;
  
  tintArray* ptintArray = static_cast<tintArray*>(rWord.addr);
                     
  if(ptintArray != 0)
  {
    word.addr = new tintArray(*ptintArray);
    assert(word.addr != 0);
  }
  
  return word;
}

/*
Method Close closes an existing tintArray object
given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing tintArray object
return value: -
exceptions: -

*/

void tintArray::Close(const ListExpr typeInfo,
                            Word& rWord)
{
  tintArray* ptintArray = static_cast<tintArray*>(rWord.addr);
  
  if(ptintArray != 0)
  {
    delete ptintArray;
    rWord.addr = 0;
  }
}

/*
Method Create creates a new tintArray object.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of the new tintArray object to create
return value: a Word that references a new tintArray object
exceptions: -

*/

Word tintArray::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new tintArray(true);
  assert(word.addr != 0);

  return word;
}

/*
Method Delete deletes an existing tintArray object
given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing tintArray object
return value: -
exceptions: -

*/

void tintArray::Delete(const ListExpr typeInfo,
                       Word& rWord)
{
  tintArray* ptintArray = static_cast<tintArray*>(rWord.addr);
  
  if(ptintArray != 0)
  {
    delete ptintArray;
    rWord.addr = 0;
  }
}

/*
Method GetTypeConstructor returns the TypeConstructor of class tintArray.

author: Dirk Zacher
parameters: -
return value: TypeConstructor of class tintArray
exceptions: -

*/

TypeConstructor tintArray::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    tintArray::BasicType(), // type name function    
    tintArray::Property,    // property function describing signature
    tintArray::Out,         // out function
    tintArray::In,          // in function
    0,                      // save to list function
    0,                      // restore from list function
    tintArray::Create,      // create function
    tintArray::Delete,      // delete function
    tintArray::Open,        // open function
    tintArray::Save,        // save function
    tintArray::Close,       // close function
    tintArray::Clone,       // clone function
    tintArray::Cast,        // cast function
    tintArray::SizeOfObj,   // sizeofobj function
    tintArray::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());
  
  return typeConstructor;
}

/*
Method In creates a new tintArray object on the basis of a given ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object to create on the basis of instance
            instance - ListExpr of the tintArray object to create
            errorPos - error position
            rErrorInfo - reference to error information
            rCorrect - flag that indicates if tintArray object
                       correctly created
return value: a Word that references a new tintArray object
exceptions: -

*/

Word tintArray::In(const ListExpr typeInfo,
                   const ListExpr instance,
                   const int errorPos,
                   ListExpr& rErrorInfo,
                   bool& rCorrect)
{ 
  Word word;
  
  NList instanceList(instance);
  rCorrect = false;

  if(instanceList.isAtom() == false)
  {
    NList gridList = instanceList.elem(1);

    if(gridList.length() == 3)
    {
      if(gridList.isReal(1) &&
         gridList.isReal(2) &&
         gridList.isReal(3))
      {
        tintArray* ptintArray = new tintArray(true);

        if(ptintArray != 0)
        {
          bool bOK = ptintArray->SetGrid(gridList.elem(1).realval(),
                                         gridList.elem(2).realval(),
                                         gridList.elem(3).realval());

          if(bOK == true)
          {
            instanceList.rest();

            if(instanceList.isEmpty() == false)
            {
              NList sizeList = instanceList.elem(1);

              if(sizeList.length() == 2)
              {
                if(sizeList.isInt(1) &&
                   sizeList.isInt(2) &&
                   sizeList.elem(1).intval() > 0 &&
                   sizeList.elem(2).intval() > 0)
                {
                  int sizeX = sizeList.elem(1).intval();
                  int sizeY = sizeList.elem(2).intval();
                  Cardinal valueListLength = static_cast<Cardinal>
                                             (sizeX * sizeY);

                  instanceList.rest();

                  while(bOK &&
                        instanceList.isEmpty() == false)
                  {
                    NList pageList = instanceList.first();

                    if(pageList.length() == 3)
                    {
                      if(pageList.isInt(1) &&
                         pageList.isInt(2))
                      {
                        int indexX = pageList.elem(1).intval();
                        int indexY = pageList.elem(2).intval();

                        if(indexX >= 0 &&
                           indexX <= TINTARRAY_DIMENSION_SIZE - sizeX &&
                           indexY >= 0 &&
                           indexY <= TINTARRAY_DIMENSION_SIZE - sizeY)
                        {
                          pageList.rest();
                          pageList.rest();

                          NList valueList = pageList.first();

                          if(valueList.length() == valueListLength)
                          {
                            for(int row = 0; row < sizeY; row++)
                            {
                              for(int column = 0; column < sizeX; column++)
                              {
                                int listIndex = row * sizeX + column + 1;
                                int arrayIndex = (indexY + row) *
                                                 TINTARRAY_DIMENSION_SIZE +
                                                 (indexX + column);
                                int value = UNDEFINED_INT;

                                if(valueList.elem(listIndex).
                                   isSymbol(Symbol::UNDEFINED()) == false)
                                {
                                  if(valueList.elem(listIndex).isInt())
                                  {
                                    value = valueList.elem(listIndex).intval();
                                  }

                                  else
                                  {
                                    bOK = false;
                                    cmsg.inFunError("Type mismatch: "
                                                    "list value in "
                                                    "partial grid has "
                                                    "wrong type.");
                                  }
                                }

                                ptintArray->SetValue(arrayIndex, value);
                              }
                            }

                            instanceList.rest();
                          }

                          else
                          {
                            bOK = false;
                            cmsg.inFunError("Type mismatch: "
                                            "list for partial grid values "
                                            "is too short or too long.");
                          }
                        }

                        else
                        {
                          bOK = false;
                          cmsg.inFunError("Type mismatch: "
                                          "page list index is "
                                          "out of valid range.");
                        }
                      }

                      else
                      {
                        bOK = false;
                        cmsg.inFunError("Type mismatch: "
                                        "partial grid content must start "
                                        "with two integers.");
                      }
                    }

                    else
                    {
                      bOK = false;
                      cmsg.inFunError("Type mismatch: "
                                      "partial grid content must contain "
                                      "three elements.");
                    }
                  }

                }

                else
                {
                  bOK = false;
                  cmsg.inFunError("Type mismatch: "
                                  "partial grid size must contain "
                                  "two positive integers.");
                }
              }

              else
              {
                bOK = false;
                cmsg.inFunError("Size list must have a length of 2.");
              }
            }

            else
            {
              bOK = false;
              cmsg.inFunError("Expected list as second element, "
                              "got an empty list.");
            }
          }

          if(bOK)
          {
            word.setAddr(ptintArray);
            rCorrect = true;
          }

          else
          {
            delete ptintArray;
            ptintArray = 0;
            rCorrect = false;
          }
        }
      }

      else
      {
        cmsg.inFunError("Type mismatch: expected 3 reals as tgrid sublist.");
      }
    }

    else
    {
      cmsg.inFunError("Type mismatch: list for tgrid is too short "
                      "or too long.");
    }
  }

  else
  {
    cmsg.inFunError("Expected list as first element, got an atom.");
  }

  return word;
}

/*
Method KindCheck checks if given type is tintArray type.

author: Dirk Zacher
parameters: type - ListExpr of type to check
            rErrorInfo - reference to error information
return value: true, if type is tintArray type, otherwise false
exceptions: -

*/

bool tintArray::KindCheck(ListExpr type,
                          ListExpr& rErrorInfo)
{
  bool bRetVal = false;
  
  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, tintArray::BasicType());
  }
  
  return bRetVal;
}

/*
Method Open opens a tintArray object from a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord containing tintArray object to open
            rOffset - Offset to the tintArray object in SmiRecord
            typeInfo - TypeInfo of tintArray object to open
            rValue - reference to a Word referencing the opened
                     tintArray object
return value: true, if tintArray object was successfully opened,
              otherwise false
exceptions: -

*/

bool tintArray::Open(SmiRecord& rValueRecord,
                     size_t& rOffset,
                     const ListExpr typeInfo,
                     Word& rValue)
{ 
  bool bRetVal = OpenAttribute<tintArray>(rValueRecord,
                                          rOffset,
                                          typeInfo,
                                          rValue);

  return bRetVal;
}

/*
Method Out writes out an existing tintArray object in the form of a ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of tintArray object to write out
            value - reference to a Word referencing the tintArray object
return value: ListExpr of tintArray object referenced by value
exceptions: -

*/

ListExpr tintArray::Out(ListExpr typeInfo,
                        Word value)
{ 
  ListExpr pListExpr = 0;
  
  if(nl != 0)
  {  
    tintArray* ptintArray = static_cast<tintArray*>(value.addr);
    
    if(ptintArray != 0)
    {
      if(ptintArray->IsDefined() == true)
      {
        NList instanceList;

        NList gridList;
        gridList.append(ptintArray->m_Grid.GetX());
        gridList.append(ptintArray->m_Grid.GetY());
        gridList.append(ptintArray->m_Grid.GetLength());
        instanceList.append(gridList);

        NList sizeList;
        sizeList.append(TINTARRAY_DIMENSION_SIZE);
        sizeList.append(TINTARRAY_DIMENSION_SIZE);
        instanceList.append(sizeList);

        NList tintList;
        tintList.append(0);
        tintList.append(0);

        NList valueList;

        for(int i = 0; i < TINTARRAY_SIZE; i++)
        {
          if(ptintArray->m_Array[i] == UNDEFINED_INT)
          {
            valueList.append(NList(Symbol::UNDEFINED()));
          }

          else
          {
            valueList.append(ptintArray->m_Array[i]);
          }
        }

        tintList.append(valueList);
        instanceList.append(tintList);

        pListExpr = instanceList.listExpr();
      }
      
      else
      {
        pListExpr = nl->SymbolAtom(Symbol::UNDEFINED());
      }
    }
  }
  
  return pListExpr;
}

/*
Method Property returns all properties of tintArray datatype.

author: Dirk Zacher
parameters: -
return value: properties of tintArray datatype in the form of a ListExpr
exceptions: -

*/

ListExpr tintArray::Property()
{
  NList propertyList;

  NList names;
  names.append(NList(std::string("Signature"), true));
  names.append(NList(std::string("Example Type List"), true));
  names.append(NList(std::string("ListRep"), true));
  names.append(NList(std::string("Example List"), true));
  names.append(NList(std::string("Remarks"), true));

  NList values;
  values.append(NList(std::string("-> DATA"), true));
  values.append(NList(BasicType(), true));
  values.append(NList
               (std::string("((x y l) (szx szy) ((ix iy (v*)))*)"),
                true));
  values.append(NList
               (std::string("((0.0 0.0 1.0) (2 2) ((-32 -32 (1 2 3 4))))"),
                true));
  values.append(NList(std::string(""), true));

  propertyList = NList(names, values);

  return propertyList.listExpr();
}

/*
Method Save saves an existing tintArray object in a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord to save existing tintArray object
            rOffset - Offset to save position of tintArray object in SmiRecord
            typeInfo - TypeInfo of tintArray object to save
            rValue - reference to a Word referencing
                     the tintArray object to save
return value: true, if tintArray object was successfully saved,
              otherwise false
exceptions: -

*/

bool tintArray::Save(SmiRecord& rValueRecord,
                     size_t& rOffset,
                     const ListExpr typeInfo,
                     Word& rValue)
{ 
  bool bRetVal = SaveAttribute<tintArray>(rValueRecord,
                                          rOffset,
                                          typeInfo,
                                          rValue);

  return bRetVal;
}

/*
Method SizeOfObj returns the size of a tintArray object.

author: Dirk Zacher
parameters: -
return value: size of a tintArray object
exceptions: -

*/

int tintArray::SizeOfObj()
{
  return sizeof(tintArray);
}

}
