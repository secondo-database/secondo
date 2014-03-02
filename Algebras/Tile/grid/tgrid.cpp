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

#include "tgrid.h"
#include "../Constants.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Constructor tgrid does not initialize any members and
should only be used in conjunction with Cast method.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

tgrid::tgrid()
      :Attribute()
{

}

/*
Constructor tgrid sets defined flag of base class Attribute and
initializes all members of the class with default values.

author: Dirk Zacher
parameters: bDefined - defined flag of base class Attribute
return value: -
exceptions: -

*/

tgrid::tgrid(bool bDefined)
      :Attribute(bDefined)
{
  Reset(); 
}

/*
Constructor tgrid sets defined flag of base class Attribute to true and
initializes all members of the class with corresponding parameter values.

author: Dirk Zacher
parameters: rX - reference to the x origin of the grid
            rY - reference to the y origin of the grid
            rLength - reference to the length of a grid cell
return value: -
exceptions: -

*/

tgrid::tgrid(const double& rX,
             const double& rY,
             const double& rLength)
      :Attribute(true),
       m_dX(rX),
       m_dY(rY),
       m_dLength(rLength)
{

}

/*
Constructor tgrid sets defined flag of base class Attribute to defined flag
of rtgrid object and initializes all members of the class with corresponding
values of rtgrid object.

author: Dirk Zacher
parameters: rtgrid - reference to a tgrid object
return value: -
exceptions: -

*/

tgrid::tgrid(const tgrid& rtgrid)
      :Attribute(rtgrid.IsDefined())
{
  *this = rtgrid;
}

/*
Destructor deinitializes a tgrid object.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

tgrid::~tgrid()
{
  
}

/*
Operator= assigns all member values of a given tgrid object
to the corresponding member values of this object.

author: Dirk Zacher
parameters: rtgrid - reference to a tgrid object
return value: reference to this object
exceptions: -

*/

const tgrid& tgrid::operator=(const tgrid& rtgrid)
{
  if(this != &rtgrid)
  {
    Attribute::operator=(rtgrid);
    m_dX = rtgrid.m_dX;
    m_dY = rtgrid.m_dY;
    m_dLength = rtgrid.m_dLength;
  }

  return *this;
}

/*
Operator== compares this object with given tgrid object.

author: Dirk Zacher
parameters: rtgrid - reference to a tgrid object
return value: true, if this object equals rtgrid, otherwise false
exceptions: -

*/

bool tgrid::operator==(const tgrid& rtgrid) const
{
  bool bIsEqual = false;

  if(this != &rtgrid)
  {
    if(m_dX == rtgrid.m_dX &&
       m_dY == rtgrid.m_dY &&
       m_dLength == rtgrid.m_dLength)
    {
      bIsEqual = true;
    }
  }

  return bIsEqual;
}

/*
Method GetX returns the x origin of the tgrid.

author: Dirk Zacher
parameters: -
return value: x origin of the tgrid
exceptions: -

*/

const double& tgrid::GetX() const
{
  return m_dX;
}

/*
Method GetY returns the y origin of the tgrid.

author: Dirk Zacher
parameters: -
return value: y origin of the tgrid
exceptions: -

*/

const double& tgrid::GetY() const
{
  return m_dY;
}

/*
Method GetLength returns the length of the a tgrid cell.

author: Dirk Zacher
parameters: -
return value: length of the a tgrid cell
exceptions: -

*/

const double& tgrid::GetLength() const
{
  return m_dLength;
}

/*
Method SetX sets the x origin of the tgrid.

author: Dirk Zacher
parameters: rX - reference to x origin
return value: true, if x origin successfully set, otherwise false
exceptions: -

*/

bool tgrid::SetX(const double& rX)
{
  bool bRetVal = true;

  m_dX = rX;

  return bRetVal;
}

/*
Method SetY sets the y origin of the tgrid.

author: Dirk Zacher
parameters: rY - reference to y origin
return value: true, if y origin successfully set, otherwise false
exceptions: -

*/

bool tgrid::SetY(const double& rY)
{
  bool bRetVal = true;

  m_dY = rY;

  return bRetVal;
}

/*
Method SetLength sets the length of a grid cell.

author: Dirk Zacher
parameters: rLength - reference to length of grid cell
return value: true, if length of grid cell successfully set, otherwise false
exceptions: -

*/

bool tgrid::SetLength(const double& rLength)
{
  bool bRetVal = false;

  m_dLength = rLength;

  if(m_dLength > 0.0)
  {
    bRetVal = true;
  }

  return bRetVal;
}

/*
Method IsEqualGrid checks if this object equals given tgrid object.

author: Dirk Zacher
parameters: rtgrid - reference to a tgrid object
return value: true, if this object equals rtgrid object, otherwise false
exceptions: -

*/

bool tgrid::IsEqualGrid(const tgrid& rtgrid) const
{
  bool bIsEqualGrid = false;

  if(AlmostEqual(m_dX, rtgrid.m_dX) &&
     AlmostEqual(m_dY, rtgrid.m_dY) &&
     AlmostEqual(m_dLength, rtgrid.m_dLength))
  {
    bIsEqualGrid = true;
  }

  return bIsEqualGrid;
}

/*
Method IsMatchingGrid checks if this object matches given tgrid object.

author: Dirk Zacher
parameters: rtgrid - reference to a tgrid object
return value: true, if this object matches rtgrid object, otherwise false
exceptions: -

*/

bool tgrid::IsMatchingGrid(const tgrid& rtgrid) const
{
  bool bIsMatchingGrid = false;

  if(m_dLength == rtgrid.m_dLength)
  {
    double fx = (rtgrid.m_dX - m_dX) / m_dLength;
    double fy = (rtgrid.m_dY - m_dY) / m_dLength;

    if(AlmostEqual(fx, round(fx)) && 
       AlmostEqual(fy, round(fy)))
    {
      bIsMatchingGrid = true;
    }
  }

  return bIsMatchingGrid;
}

/*
Method Reset resets all members of the class to the default values.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

void tgrid::Reset()
{
  m_dX = 0.0;
  m_dY = 0.0;
  m_dLength = 1.0;
}

/*
Method Adjacent checks if this object is adjacent to given Attribute object.

author: Dirk Zacher
parameters: pAttribute - a pointer to an Attribute object
return value: true, if this object is adjacent to pAttribute, otherwise false
exceptions: -

*/

bool tgrid::Adjacent(const Attribute* pAttribute) const
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

Attribute* tgrid::Clone() const
{
  Attribute* pAttribute = new tgrid(*this);
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

int tgrid::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;
  
  if(pAttribute != 0)
  {
    const tgrid* ptgrid = dynamic_cast<const tgrid*>(pAttribute);
    
    if(ptgrid != 0)
    {
      bool bIsDefined = IsDefined();
      bool btgridIsDefined = ptgrid->IsDefined();
      
      if(bIsDefined == true)
      {
        if(btgridIsDefined == true) // defined x defined
        {
          if(m_dX < ptgrid->m_dX)
          {
            nRetVal = -1;
          }

          else
          {
            if(m_dX > ptgrid->m_dX)
            {
              nRetVal = 1;
            }

            else // m_dX == ptgrid->m_dX
            {
              if(m_dY < ptgrid->m_dY)
              {
                nRetVal = -1;
              }

              else
              {
                if(m_dY > ptgrid->m_dY)
                {
                  nRetVal = 1;
                }

                else // m_dY == ptgrid->m_dY
                {
                  if(m_dLength < ptgrid->m_dLength)
                  {
                    nRetVal = -1;
                  }

                  else
                  {
                    if(m_dLength > ptgrid->m_dLength)
                    {
                      nRetVal = 1;
                    }

                    else // m_dLength == ptgrid->m_dLength
                    {
                      nRetVal = 0;
                    }
                  }
                }
              }
            }
          }
        }
        
        else // defined x undefined
        {
          nRetVal = 1;
        }
      }
      
      else
      {
        if(btgridIsDefined == true) // undefined x defined
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

void tgrid::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const tgrid* ptgrid = dynamic_cast<const tgrid*>(pAttribute);
    
    if(ptgrid != 0)
    {
      *this = *ptgrid;
    }
  }
}

/*
Method HashValue returns the hash value of the tgrid object.

author: Dirk Zacher
parameters: -
return value: hash value of the tgrid object
exceptions: -

*/

size_t tgrid::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }
  
  return hashValue;
}

/*
Method Sizeof returns the size of tgrid datatype.

author: Dirk Zacher
parameters: -
return value: size of tgrid object
exceptions: -

*/

size_t tgrid::Sizeof() const
{
  return sizeof(tgrid);
}

/*
Method BasicType returns the typename of tgrid datatype.

author: Dirk Zacher
parameters: -
return value: typename of tgrid datatype
exceptions: -

*/

const std::string tgrid::BasicType()
{
  return TYPE_NAME_TGRID;
}

/*
Method Cast casts a void pointer to a new tgrid object.

author: Dirk Zacher
parameters: pVoid - a pointer to a memory address
return value: a pointer to a new tgrid object
exceptions: -

*/

void* tgrid::Cast(void* pVoid)
{
  return new(pVoid)tgrid;
}

/*
Method Clone clones an existing tgrid object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing tgrid object
return value: a word that references a new tgrid object
exceptions: -

*/

Word tgrid::Clone(const ListExpr typeInfo,
                  const Word& rWord)
{
  Word word;

  tgrid* ptgrid = static_cast<tgrid*>(rWord.addr);

  if(ptgrid != 0)
  {
    word.addr = new tgrid(*ptgrid);
    assert(word.addr != 0);
  }

  return word;
}

/*
Method Close closes an existing tgrid object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing tgrid object
return value: -
exceptions: -

*/

void tgrid::Close(const ListExpr typeInfo,
                  Word& rWord)
{
  tgrid* ptgrid = static_cast<tgrid*>(rWord.addr);
  
  if(ptgrid != 0)
  {
    delete ptgrid;
    rWord.addr = 0;
  }
}

/*
Method Create creates a new tgrid object.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of the new tgrid object to create
return value: a word that references a new tgrid object
exceptions: -

*/

Word tgrid::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new tgrid(0.0, 0.0, 1.0);
  assert(word.addr != 0);

  return word;
}

/*
Method Delete deletes an existing tgrid object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing tgrid object
return value: -
exceptions: -

*/

void tgrid::Delete(const ListExpr typeInfo,
                   Word& rWord)
{
  tgrid* ptgrid = static_cast<tgrid*>(rWord.addr);

  if(ptgrid != 0)
  {
    delete ptgrid;
    rWord.addr = 0;
  }
}

/*
Method GetTypeConstructor returns the TypeConstructor of class tgrid.

author: Dirk Zacher
parameters: -
return value: TypeConstructor of class tgrid
exceptions: -

*/

TypeConstructor tgrid::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    tgrid::BasicType(), // type name function
    tgrid::Property,    // property function describing signature
    tgrid::Out,         // out function
    tgrid::In,          // in function
    0,                  // save to list function
    0,                  // restore from list function
    tgrid::Create,      // create function
    tgrid::Delete,      // delete function
    tgrid::Open,        // open function
    tgrid::Save,        // save function
    tgrid::Close,       // close function
    tgrid::Clone,       // clone function
    tgrid::Cast,        // cast function
    tgrid::SizeOfObj,   // sizeofobj function
    tgrid::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

/*
Method In creates a new tgrid object on the basis of a given ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object to create on the basis of instance
            instance - ListExpr of the tgrid object to create
            errorPos - error position
            rErrorInfo - reference to error information
            rCorrect - flag that indicates if tgrid object correctly created
return value: TypeConstructor of tgrid class
exceptions: -

*/

Word tgrid::In(const ListExpr typeInfo,
               const ListExpr instance,
               const int errorPos,
               ListExpr& rErrorInfo,
               bool& rCorrect)
{
  Word word;

  rCorrect = false;
  NList instanceList(instance);
  
  if(instanceList.length() == 3)
  {
    if(instanceList.isReal(1) &&
       instanceList.isReal(2) &&
       instanceList.isReal(3))
    {
      tgrid* ptgrid = new tgrid(instanceList.elem(1).realval(),
                                instanceList.elem(2).realval(),
                                instanceList.elem(3).realval());
      
      if(ptgrid != 0)
      { 
        if(ptgrid->m_dLength <= 0)
        {
          cmsg.inFunError("Length of tgrid object must be larger than zero.");
          delete ptgrid;
          ptgrid = 0;
          rCorrect = false;
        }

        else
        {
          word.addr = ptgrid;
          rCorrect = true;
        }
      }
    }

    else
    {
      cmsg.inFunError("Types of nested list expression elements"
                      "of tgrid object must be reals.");
    }
  }

  else
  {
    cmsg.inFunError("Length of nested list expression of tgrid object"
                    "must be 3.");
  }

  return word;
}

/*
Method KindCheck checks if given type is tgrid type.

author: Dirk Zacher
parameters: type - ListExpr of type to check
            rErrorInfo - reference to error information
return value: true, if type is tgrid type, otherwise false
exceptions: -

*/

bool tgrid::KindCheck(ListExpr type,
                      ListExpr& rErrorInfo)
{
  return NList(type).isSymbol(tgrid::BasicType());
}

/*
Method Open opens a tgrid object from a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord containing tgrid object to open
            rOffset - Offset to the tgrid object in SmiRecord
            typeInfo - TypeInfo of tgrid object to open
            rValue - reference to a Word referencing the opened tgrid object
return value: true, if tgrid object was successfully opened, otherwise false
exceptions: -

*/

bool tgrid::Open(SmiRecord& rValueRecord,
                 size_t& rOffset,
                 const ListExpr typeInfo,
                 Word& rValue)
{
  bool bRetVal = OpenAttribute<tgrid>(rValueRecord,
                                      rOffset,
                                      typeInfo,
                                      rValue);

  return bRetVal;
}

/*
Method Out writes out an existing tgrid object in form of a ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of tgrid object to write out
            value - reference to a Word referencing the tgrid object to write
return value: ListExpr of tgrid object referenced by value
exceptions: -

*/

ListExpr tgrid::Out(ListExpr typeInfo,
                    Word value)
{
  NList nlist;

  tgrid* ptgrid = static_cast<tgrid*>(value.addr);

  if(ptgrid != 0)
  {
    nlist.append(NList(ptgrid->m_dX));
    nlist.append(NList(ptgrid->m_dY));
    nlist.append(NList(ptgrid->m_dLength));
  }

  return nlist.listExpr();
}

/*
Method Property returns all properties of tgrid datatype.

author: Dirk Zacher
parameters: -
return value: properties of tgrid datatype in form of a ListExpr
exceptions: -

*/

ListExpr tgrid::Property()
{
  NList nlist;

  NList names;
  names.append(NList(std::string("Signature"), true));
  names.append(NList(std::string("Example Type List"), true));
  names.append(NList(std::string("ListRep"), true));
  names.append(NList(std::string("Example List"), true));
  names.append(NList(std::string("Remarks"), true));

  NList values;
  values.append(NList(std::string("-> DATA"), true));
  values.append(NList(BasicType(), true));
  values.append(NList(std::string("(<X> <Y> <Length>)"), true));
  values.append(NList(std::string("(3.1415 2.718 12.0)"), true));
  values.append(NList(std::string("length must be positive"), true));

  nlist = NList(names, values);

  return nlist.listExpr();
}

/*
Method Save saves an existing tgrid object in a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord to save existing tgrid object
            rOffset - Offset to save position of tgrid object in SmiRecord
            typeInfo - TypeInfo of tgrid object to save
            rValue - reference to a Word referencing the tgrid object to save
return value: true, if tgrid object was successfully saved, otherwise false
exceptions: -

*/

bool tgrid::Save(SmiRecord& rValueRecord,
                 size_t& rOffset,
                 const ListExpr typeInfo,
                 Word& rValue)
{
  bool bRetVal = SaveAttribute<tgrid>(rValueRecord,
                                      rOffset,
                                      typeInfo,
                                      rValue);

  return bRetVal;
}

/*
Method SizeOfObj returns the size of tgrid object.

author: Dirk Zacher
parameters: -
return value: size of tgrid object
exceptions: -

*/

int tgrid::SizeOfObj()
{
  return sizeof(tgrid);
}

}
