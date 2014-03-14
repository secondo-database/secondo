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

#include "tintFlob.h"
#include "../Constants.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Constructor tintFlob does not initialize any members and
should only be used in conjunction with Cast method.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

tintFlob::tintFlob()
         :Attribute()
{
  
}

/*
Constructor tintFlob sets defined flag of base class Attribute and
initializes all members of the class with default values.

author: Dirk Zacher
parameters: bDefined - defined flag of base class Attribute
return value: -
exceptions: -

*/
  
tintFlob::tintFlob(bool bDefined)
         :Attribute(bDefined),
          m_Grid(false),
          m_Flob(TINTFLOB_SIZE)
{
  for(int i = 0; i < TINTFLOB_ELEMENTS; i++)
  {
    SetValue(i, UNDEFINED_INT);
  }
}

/*
Constructor tintFlob sets defined flag of base class Attribute to defined flag
of rtintFlob object and initializes all members of the class with corresponding
values of rtintFlob object.

author: Dirk Zacher
parameters: rtintFlob - reference to a tintFlob object
return value: -
exceptions: -

*/

tintFlob::tintFlob(const tintFlob& rtintFlob)
                  :Attribute(rtintFlob.IsDefined()),
                   m_Grid(rtintFlob.m_Grid),
                   m_Flob(rtintFlob.m_Flob.getSize())
{
  m_Flob.copyFrom(rtintFlob.m_Flob);
}

/*
Destructor deinitializes a tintFlob object.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

tintFlob::~tintFlob()
{
  
}

/*
Operator= assigns all member values of a given tintFlob object
to the corresponding member values of this object.

author: Dirk Zacher
parameters: rtintFlob - reference to a tintFlob object
return value: reference to this object
exceptions: -

*/

tintFlob& tintFlob::operator=(const tintFlob& rtintFlob)
{
  if(this != &rtintFlob)
  {
    SetDefined(rtintFlob.IsDefined());
    m_Grid=rtintFlob.m_Grid;

    bool bOK = false;

    bOK = m_Flob.clean();
    assert(bOK);
    bOK = m_Flob.copyFrom(rtintFlob.m_Flob);
    assert(bOK);
  }
  
  return *this;
}

/*
Operator== checks if this object equals rtintFlob object.

author: Dirk Zacher
parameters: rtintFlob - reference to a tintFlob object
return value: true, if this object equals rtintFlob object, otherwise false
exceptions: -

*/

bool tintFlob::operator==(const tintFlob& rtintFlob) const
{
  bool bIsEqual = false;

  if(this != &rtintFlob)
  {
    if(m_Grid == rtintFlob.m_Grid &&
       m_Flob == rtintFlob.m_Flob)
    {
      bIsEqual = true;
    }
  }

  return bIsEqual;
}

/*
TileAlgebra operator load loads all values of the tintFlob object.

author: Dirk Zacher
parameters: -
return value: true, if all values of the tintFlob object successfully loaded,
              otherwise false
exceptions: -

*/

bool tintFlob::load()
{
  bool bRetVal = true;

  /*
  According to Prof. Dr. Gueting open method of an attribute type loads
  only root record in memory. Flob data will be read immediately before
  an operator tries to access flob object and will be stored in a read cache.

  */

  char buffer[TINTFLOB_SIZE];
  bRetVal = m_Flob.read(buffer, TINTFLOB_SIZE, 0);

  return bRetVal;
}

/*
Method Destroy destroys tintFlob object.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

void tintFlob::Destroy()
{
  m_Flob.destroy();
}

/*
Method SetGrid sets the tgrid properties of tintFlob object.

author: Dirk Zacher
parameters: rX - reference to the x origin of the grid
            rY - reference to the y origin of the grid
            rLength - reference to the length of a grid cell
return value: true, if tgrid properties of tintFlob object successfully set,
              otherwise false
exceptions: -

*/

bool tintFlob::SetGrid(const double& rX,
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
Method SetValue sets a value of tintFlob object at given index.

author: Dirk Zacher
parameters: nIndex - index of tintFlob value
            nValue - value
return value: true, if nValue was successfully set at nIndex, otherwise false
exceptions: -

*/

bool tintFlob::SetValue(int nIndex,
                        int nValue)
{
  bool bRetVal = false;

  if(IsDefined() &&
     nIndex >= 0 &&
     nIndex < TINTFLOB_ELEMENTS)
  {
    bRetVal = m_Flob.write(reinterpret_cast<const char*>(&nValue),
                           sizeof(int),
                           nIndex * sizeof(int));
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

bool tintFlob::Adjacent(const Attribute* pAttribute) const
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

Attribute* tintFlob::Clone() const
{
  Attribute* pAttribute = new tintFlob(*this);
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

int tintFlob::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;
  
  if(pAttribute != 0)
  {
    const tintFlob* ptintFlob = dynamic_cast<const tintFlob*>(pAttribute);
    
    if(ptintFlob != 0)
    {
      bool bIsDefined = IsDefined();
      bool btintFlobIsDefined = ptintFlob->IsDefined();
      
      if(bIsDefined == true)
      {
        if(btintFlobIsDefined == true) // defined x defined
        {
          char buffer1[TINTFLOB_SIZE];
          m_Flob.read(buffer1, TINTFLOB_SIZE, 0);

          char buffer2[TINTFLOB_SIZE];
          ptintFlob->m_Flob.read(buffer2, TINTFLOB_SIZE, 0);

          nRetVal = memcmp(buffer1, buffer2, TINTFLOB_SIZE);
        }
        
        else // defined x undefined
        {
          nRetVal = 1;
        }
      }
      
      else
      {
        if(btintFlobIsDefined == true) // undefined x defined
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

void tintFlob::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const tintFlob* ptintFlob = dynamic_cast<const tintFlob*>(pAttribute);
    
    if(ptintFlob != 0)
    {
      *this = *ptintFlob;
    }
  }
}

/*
Method GetFLOB returns a pointer to the Flob with given index.

author: Dirk Zacher
parameters: i - index of Flob
return value: a pointer to the Flob with given index
exceptions: -

*/

Flob* tintFlob::GetFLOB(const int i)
{ 
  Flob* pFlob = 0;
  int nFlobs = NumOfFLOBs();
  
  if(i >= 0 &&
     i < nFlobs)
  {
    switch(i)
    {
      case 0:   pFlob = &m_Flob;
                break;
                
      default:  break;
    }
  }
  
  return pFlob;
}

/*
Method HashValue returns the hash value of the tintFlob object.

author: Dirk Zacher
parameters: -
return value: hash value of the tintFlob object
exceptions: -

*/

size_t tintFlob::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }
  
  return hashValue;
}

/*
Method NumOfFLOBs returns the number of Flobs of a tintFlob object.

author: Dirk Zacher
parameters: -
return value: number of Flobs of a tintFlob object
exceptions: -

*/

int tintFlob::NumOfFLOBs() const
{ 
  return 1;
}

/*
Method Sizeof returns the size of tintFlob datatype.

author: Dirk Zacher
parameters: -
return value: size of tintFlob datatype
exceptions: -

*/

size_t tintFlob::Sizeof() const
{
  return sizeof(tintFlob);
}

/*
Method BasicType returns the typename of tintFlob datatype.

author: Dirk Zacher
parameters: -
return value: typename of tintFlob datatype
exceptions: -

*/

const std::string tintFlob::BasicType()
{
  return "tintFlob";
}

/*
Method Cast casts a void pointer to a new tintFlob object.

author: Dirk Zacher
parameters: pVoid - a pointer to a memory address
return value: a pointer to a new tintFlob object
exceptions: -

*/

void* tintFlob::Cast(void* pVoid)
{
  return new(pVoid)tintFlob;
}

/*
Method Clone clones an existing tintFlob object
given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing tintFlob object
return value: a Word that references a new tintFlob object
exceptions: -

*/

Word tintFlob::Clone(const ListExpr typeInfo,
                     const Word& rWord)
{
  Word word;
  
  tintFlob* ptintFlob = static_cast<tintFlob*>(rWord.addr);
                     
  if(ptintFlob != 0)
  {
    word.addr = new tintFlob(*ptintFlob);
    assert(word.addr != 0);
  }
  
  return word;
}

/*
Method Close closes an existing tintFlob object
given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing tintFlob object
return value: -
exceptions: -

*/

void tintFlob::Close(const ListExpr typeInfo,
                           Word& rWord)
{
  tintFlob* ptintFlob = static_cast<tintFlob*>(rWord.addr);
  
  if(ptintFlob != 0)
  {
    delete ptintFlob;
    rWord.addr = 0;
  }
}

/*
Method Create creates a new tintFlob object.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of the new tintFlob object to create
return value: a Word that references a new tintFlob object
exceptions: -

*/

Word tintFlob::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new tintFlob(true);
  assert(word.addr != 0);

  return word;
}

/*
Method Delete deletes an existing tintFlob object
given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing tintFlob object
return value: -
exceptions: -

*/

void tintFlob::Delete(const ListExpr typeInfo,
                      Word& rWord)
{
  tintFlob* ptintFlob = static_cast<tintFlob*>(rWord.addr);
  
  if(ptintFlob != 0)
  {
    delete ptintFlob;
    rWord.addr = 0;
  }
}

/*
Method GetTypeConstructor returns the TypeConstructor of class tintFlob.

author: Dirk Zacher
parameters: -
return value: TypeConstructor of class tintFlob
exceptions: -

*/

TypeConstructor tintFlob::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    tintFlob::BasicType(), // type name function    
    tintFlob::Property,    // property function describing signature
    tintFlob::Out,         // out function
    tintFlob::In,          // in function
    0,                     // save to list function
    0,                     // restore from list function
    tintFlob::Create,      // create function
    tintFlob::Delete,      // delete function
    tintFlob::Open,        // open function
    tintFlob::Save,        // save function
    tintFlob::Close,       // close function
    tintFlob::Clone,       // clone function
    tintFlob::Cast,        // cast function
    tintFlob::SizeOfObj,   // sizeofobj function
    tintFlob::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());
  
  return typeConstructor;
}

/*
Method In creates a new tintFlob object on the basis of a given ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object to create on the basis of instance
            instance - ListExpr of the tintFlob object to create
            errorPos - error position
            rErrorInfo - reference to error information
            rCorrect - flag that indicates if tintFlob object
                       correctly created
return value: a Word that references a new tintFlob object
exceptions: -

*/

Word tintFlob::In(const ListExpr typeInfo,
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
        tintFlob* ptintFlob = new tintFlob(true);

        if(ptintFlob != 0)
        {
          bool bOK = ptintFlob->SetGrid(gridList.elem(1).realval(),
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
                           indexX <= TINTFLOB_DIMENSION_SIZE - sizeX &&
                           indexY >= 0 &&
                           indexY <= TINTFLOB_DIMENSION_SIZE - sizeY)
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
                                int flobIndex = (indexY + row) *
                                                TINTFLOB_DIMENSION_SIZE +
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

                                ptintFlob->SetValue(flobIndex, value);
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
            word.setAddr(ptintFlob);
            rCorrect = true;
          }

          else
          {
            delete ptintFlob;
            ptintFlob = 0;
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
Method KindCheck checks if given type is tintFlob type.

author: Dirk Zacher
parameters: type - ListExpr of type to check
            rErrorInfo - reference to error information
return value: true, if type is tintFlob type, otherwise false
exceptions: -

*/

bool tintFlob::KindCheck(ListExpr type,
                         ListExpr& rErrorInfo)
{
  bool bRetVal = false;
  
  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, tintFlob::BasicType());
  }
  
  return bRetVal;
}

/*
Method Open opens a tintFlob object from a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord containing tintFlob object to open
            rOffset - Offset to the tintFlob object in SmiRecord
            typeInfo - TypeInfo of tintFlob object to open
            rValue - reference to a Word referencing the opened
                     tintFlob object
return value: true, if tintFlob object was successfully opened,
              otherwise false
exceptions: -

*/

bool tintFlob::Open(SmiRecord& rValueRecord,
                    size_t& rOffset,
                    const ListExpr typeInfo,
                    Word& rValue)
{ 
  bool bRetVal = OpenAttribute<tintFlob>(rValueRecord,
                                         rOffset,
                                         typeInfo,
                                         rValue);

  return bRetVal;
}

/*
Method Out writes out an existing tintFlob object in the form of a ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of tintFlob object to write out
            value - reference to a Word referencing the tintFlob object
return value: ListExpr of tintFlob object referenced by value
exceptions: -

*/

ListExpr tintFlob::Out(ListExpr typeInfo,
                       Word value)
{ 
  ListExpr pListExpr = 0;

  if(nl != 0)
  {  
    tintFlob* ptintFlob = static_cast<tintFlob*>(value.addr);
    
    if(ptintFlob != 0)
    {
      if(ptintFlob->IsDefined() == true)
      {
        NList instanceList;

        NList gridList;
        gridList.append(ptintFlob->m_Grid.GetX());
        gridList.append(ptintFlob->m_Grid.GetY());
        gridList.append(ptintFlob->m_Grid.GetLength());
        instanceList.append(gridList);

        NList sizeList;
        sizeList.append(TINTFLOB_DIMENSION_SIZE);
        sizeList.append(TINTFLOB_DIMENSION_SIZE);
        instanceList.append(sizeList);

        NList tintList;
        tintList.append(0);
        tintList.append(0);

        NList valueList;

        for(int i = 0; i < TINTFLOB_ELEMENTS; i++)
        {
          int nValue = UNDEFINED_INT;

          bool bOK = ptintFlob->m_Flob.read(reinterpret_cast<char*>(&nValue),
                                            sizeof(int),
                                            i * sizeof(int));
          assert(bOK);

          if(nValue == UNDEFINED_INT)
          {
            valueList.append(NList(Symbol::UNDEFINED()));
          }

          else
          {
            valueList.append(nValue);
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
Method Property returns all properties of tintFlob datatype.

author: Dirk Zacher
parameters: -
return value: properties of tintFlob datatype in the form of a ListExpr
exceptions: -

*/

ListExpr tintFlob::Property()
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
Method Save saves an existing tintFlob object in a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord to save existing tintFlob object
            rOffset - Offset to save position of tintFlob object in SmiRecord
            typeInfo - TypeInfo of tintFlob object to save
            rValue - reference to a Word referencing
                     the tintFlob object to save
return value: true, if tintFlob object was successfully saved,
              otherwise false
exceptions: -

*/

bool tintFlob::Save(SmiRecord& rValueRecord,
                    size_t& rOffset,
                    const ListExpr typeInfo,
                    Word& rValue)
{ 
  bool bRetVal = SaveAttribute<tintFlob>(rValueRecord,
                                         rOffset,
                                         typeInfo,
                                         rValue);

  return bRetVal;
}

/*
Method SizeOfObj returns the size of a tintFlob object.

author: Dirk Zacher
parameters: -
return value: size of a tintFlob object
exceptions: -

*/

int tintFlob::SizeOfObj()
{
  return sizeof(tintFlob);
}

}
