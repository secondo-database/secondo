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

#include "mtgrid.h"
#include "../Constants.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Constructor mtgrid does not initialize any members and
should only be used in conjunction with Cast method.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

mtgrid::mtgrid()
       :tgrid()
{
  
}

/*
Constructor mtgrid sets defined flag of base class Attribute and
initializes all members of the class with default values.

author: Dirk Zacher
parameters: bDefined - defined flag of base class Attribute
return value: -
exceptions: -

*/

mtgrid::mtgrid(bool bDefined)
       :tgrid(bDefined),
        m_dT(0.0),
        m_Duration(datetime::durationtype)
{

}

/*
Constructor mtgrid sets defined flag of base class Attribute to true and
initializes all members of the class with corresponding parameter values.

author: Dirk Zacher
parameters: rX - reference to the x origin of the grid
            rY - reference to the y origin of the grid
            rLength - reference to the length of a grid cell
            rDuration - reference to the duration value of time dimension
return value: -
exceptions: -

*/

mtgrid::mtgrid(const double& rX,
               const double& rY,
               const double& rLength,
               const datetime::DateTime& rDuration)
       :tgrid(rX, rY, rLength),
        m_dT(0.0),
        m_Duration(rDuration)
{
  m_Duration.SetType(datetime::durationtype);
}

/*
Constructor mtgrid sets defined flag of base class Attribute to defined flag
of rmtgrid object and initializes all members of the class with corresponding
values of rmtgrid object.

author: Dirk Zacher
parameters: rmtgrid - reference to a mtgrid object
return value: -
exceptions: -

*/

mtgrid::mtgrid(const mtgrid& rmtgrid)
       :tgrid(rmtgrid.IsDefined())
{
  *this = rmtgrid;
}

/*
Destructor deinitializes a mtgrid object.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

mtgrid::~mtgrid()
{
  
}

/*
Operator= assigns all member values of a given mtgrid object
to the corresponding member values of this object.

author: Dirk Zacher
parameters: rmtgrid - reference to a mtgrid object
return value: reference to this object
exceptions: -

*/

const mtgrid& mtgrid::operator=(const mtgrid& rmtgrid)
{
  if(this != &rmtgrid)
  {
    tgrid::operator=(rmtgrid);
    m_dT = rmtgrid.m_dT;
    m_Duration = rmtgrid.m_Duration;
  }

  return *this;
}

/*
Operator== compares this object with given mtgrid object.

author: Dirk Zacher
parameters: rmtgrid - reference to a mtgrid object
return value: true, if this object equals rmtgrid, otherwise false
exceptions: -

*/

bool mtgrid::operator==(const mtgrid& rmtgrid) const
{
  bool bIsEqual = false;

  if(this != &rmtgrid)
  {
    if(tgrid::operator==(rmtgrid) &&
       m_dT == rmtgrid.m_dT &&
       m_Duration == rmtgrid.m_Duration)
    {
      bIsEqual = true;
    }
  }

  return bIsEqual;
}

/*
Method GetT returns the time origin of the mtgrid.

author: Dirk Zacher
parameters: -
return value: time origin of the mtgrid
exceptions: -

*/

const double& mtgrid::GetT() const
{
  return m_dT;
}

/*
Method GetDuration returns the duration of the mtgrid.

author: Dirk Zacher
parameters: -
return value: duration of the mtgrid
exceptions: -

*/

const datetime::DateTime& mtgrid::GetDuration() const
{
  return m_Duration;
}

/*
Method SetT sets the time origin of the mtgrid.

author: Dirk Zacher
parameters: rT - reference to time origin
return value: true, if time origin successfully set, otherwise false
exceptions: -

*/

bool mtgrid::SetT(const double& rT)
{
  bool bRetVal = false;

  if(m_dT >= 0.0)
  {
    m_dT = rT;
    bRetVal = true;
  }

  return bRetVal;
}

/*
Method SetDuration sets the duration of the mtgrid.

author: Dirk Zacher
parameters: rDuration - reference to duration
return value: true, if duration successfully set, otherwise false
exceptions: -

*/

bool mtgrid::SetDuration(const datetime::DateTime& rDuration)
{
  bool bRetVal = false;

  m_Duration = rDuration;

  if(m_Duration.LessThanZero() == false &&
     m_Duration.IsZero() == false)
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

bool mtgrid::IsEqualGrid(const tgrid& rtgrid) const
{
  bool bIsEqualGrid = tgrid::IsEqualGrid(rtgrid);

  return bIsEqualGrid;
}

/*
Method IsEqualGrid checks if this object equals given mtgrid object.

author: Dirk Zacher
parameters: rmtgrid - reference to a mtgrid object
return value: true, if this object equals rmtgrid object, otherwise false
exceptions: -

*/

bool mtgrid::IsEqualGrid(const mtgrid& rmtgrid) const
{
  bool bIsEqualGrid = tgrid::IsEqualGrid(rmtgrid);

  if(bIsEqualGrid == true &&
     m_dT == rmtgrid.m_dT &&
     m_Duration == rmtgrid.m_Duration)
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

bool mtgrid::IsMatchingGrid(const tgrid& rtgrid) const
{
  bool bIsMatchingGrid = tgrid::IsMatchingGrid(rtgrid);

  return bIsMatchingGrid;
}

/*
Method IsMatchingGrid checks if this object matches given mtgrid object.

author: Dirk Zacher
parameters: rmtgrid - reference to a mtgrid object
return value: true, if this object matches rmtgrid object, otherwise false
exceptions: -

*/

bool mtgrid::IsMatchingGrid(const mtgrid& rmtgrid) const
{
  bool bIsMatchingGrid = tgrid::IsMatchingGrid(rmtgrid);

  if(bIsMatchingGrid == true &&
     m_dT == rmtgrid.m_dT &&
     m_Duration == rmtgrid.m_Duration)
  {
    bIsMatchingGrid = true;
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

void mtgrid::Reset()
{
  tgrid::Reset();
  m_dT = 0.0;
  m_Duration.SetToZero();
}

/*
Method Adjacent checks if this object is adjacent to given Attribute object.

author: Dirk Zacher
parameters: pAttribute - a pointer to an Attribute object
return value: true, if this object is adjacent to pAttribute, otherwise false
exceptions: -

*/

bool mtgrid::Adjacent(const Attribute* pAttribute) const
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

Attribute* mtgrid::Clone() const
{
  Attribute* pAttribute = new mtgrid(*this);
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

int mtgrid::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;
  
  if(pAttribute != 0)
  {
    const mtgrid* pmtgrid = dynamic_cast<const mtgrid*>(pAttribute);
    
    if(pmtgrid != 0)
    {
      bool bIsDefined = IsDefined();
      bool bmtgridIsDefined = pmtgrid->IsDefined();
      
      if(bIsDefined == true)
      {
        if(bmtgridIsDefined == true) // defined x defined
        {
          nRetVal = tgrid::Compare(pmtgrid);

          if(nRetVal == 0)
          {
            nRetVal = m_Duration.Compare(&(pmtgrid->m_Duration));
          }
        }
        
        else // defined x undefined
        {
          nRetVal = 1;
        }
      }
      
      else
      {
        if(bmtgridIsDefined == true) // undefined x defined
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

void mtgrid::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const mtgrid* pmtgrid = dynamic_cast<const mtgrid*>(pAttribute);
    
    if(pmtgrid != 0)
    {
      *this = *pmtgrid;
    }
  }
}

/*
Method HashValue returns the hash value of the mtgrid object.

author: Dirk Zacher
parameters: -
return value: hash value of the mtgrid object
exceptions: -

*/

size_t mtgrid::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }
  
  return hashValue;
}

/*
Method Sizeof returns the size of mtgrid datatype.

author: Dirk Zacher
parameters: -
return value: size of mtgrid datatype
exceptions: -

*/

size_t mtgrid::Sizeof() const
{
  return sizeof(mtgrid);
}

/*
Method BasicType returns the typename of mtgrid datatype.

author: Dirk Zacher
parameters: -
return value: typename of mtgrid datatype
exceptions: -

*/

const std::string mtgrid::BasicType()
{
  return TYPE_NAME_MTGRID;
}

/*
Method Cast casts a void pointer to a new mtgrid object.

author: Dirk Zacher
parameters: pVoid - a pointer to a memory address
return value: a pointer to a new mtgrid object
exceptions: -

*/

void* mtgrid::Cast(void* pVoid)
{
  return new(pVoid)mtgrid;
}

/*
Method Clone clones an existing mtgrid object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing mtgrid object
return value: a Word that references a new mtgrid object
exceptions: -

*/

Word mtgrid::Clone(const ListExpr typeInfo,
                   const Word& rWord)
{
  Word word;

  mtgrid* pmtgrid = static_cast<mtgrid*>(rWord.addr);

  if(pmtgrid != 0)
  {
    word.addr = new mtgrid(*pmtgrid);
    assert(word.addr != 0);
  }

  return word;
}

/*
Method Close closes an existing mtgrid object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing mtgrid object
return value: -
exceptions: -

*/

void mtgrid::Close(const ListExpr typeInfo,
                   Word& rWord)
{
  mtgrid* pmtgrid = static_cast<mtgrid*>(rWord.addr);
  
  if(pmtgrid != 0)
  {
    delete pmtgrid;
    pmtgrid = 0;
    rWord.addr = 0;
  }
}

/*
Method Create creates a new mtgrid object.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of the new mtgrid object to create
return value: a Word that references a new mtgrid object
exceptions: -

*/

Word mtgrid::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new mtgrid(0.0, 0.0, 1.0, 1.0);
  assert(word.addr != 0);

  return word;
}

/*
Method Delete deletes an existing mtgrid object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing mtgrid object
return value: -
exceptions: -

*/

void mtgrid::Delete(const ListExpr typeInfo,
                    Word& rWord)
{
  mtgrid* pmtgrid = static_cast<mtgrid*>(rWord.addr);

  if(pmtgrid != 0)
  {
    delete pmtgrid;
    pmtgrid = 0;
    rWord.addr = 0;
  }
}

/*
Method GetTypeConstructor returns the TypeConstructor of class mtgrid.

author: Dirk Zacher
parameters: -
return value: TypeConstructor of class mtgrid
exceptions: -

*/

TypeConstructor mtgrid::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    mtgrid::BasicType(), // type name function
    mtgrid::Property,    // property function describing signature
    mtgrid::Out,         // out function
    mtgrid::In,          // in function
    0,                   // save to list function
    0,                   // restore from list function
    mtgrid::Create,      // create function
    mtgrid::Delete,      // delete function
    mtgrid::Open,        // open function
    mtgrid::Save,        // save function
    mtgrid::Close,       // close function
    mtgrid::Clone,       // clone function
    mtgrid::Cast,        // cast function
    mtgrid::SizeOfObj,   // sizeofobj function
    mtgrid::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

/*
Method In creates a new mtgrid object on the basis of a given ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object to create on the basis of instance
            instance - ListExpr of the mtgrid object to create
            errorPos - error position
            rErrorInfo - reference to error information
            rCorrect - flag that indicates if mtgrid object correctly created
return value: a Word that references a new mtgrid object
exceptions: -

*/

Word mtgrid::In(const ListExpr typeInfo,
                const ListExpr instance,
                const int errorPos,
                ListExpr& rErrorInfo,
                bool& rCorrect)
{
  Word word;

  rCorrect = false;
  NList nlist(instance);

  if(nlist.length() == 4)
  {
    if(nlist.isReal(1) &&
       nlist.isReal(2) &&
       nlist.isReal(3) &&
       nlist.isList(4))
    {
      datetime::DateTime duration(datetime::durationtype);

      mtgrid* pmtgrid = new mtgrid(nlist.elem(1).realval(),
                                   nlist.elem(2).realval(),
                                   nlist.elem(3).realval(),
                                   duration);
      
      if(pmtgrid != 0)
      { 
        if(pmtgrid->GetLength() <= 0.0)
        {
          cmsg.inFunError("Length of mtgrid object must be larger than zero.");
          delete pmtgrid;
          pmtgrid = 0;
          rCorrect = false;
        }

        else
        {
          bool bOK = pmtgrid->m_Duration.ReadFrom(nlist.elem(4).listExpr(),
                                                  true);

          if(bOK == true &&
             pmtgrid->m_Duration.IsDefined())
          {
            word.addr = pmtgrid;
            rCorrect = true;
          }

          else
          {
            cmsg.inFunError("Duration of mtgrid object must be "
                            "(duration (days milliseconds)).");
            delete pmtgrid;
            pmtgrid = 0;
            rCorrect = false;
          }
        }
      }
    }

    else
    {
      cmsg.inFunError("Types of nested list expression elements "
                      "of mtgrid object must be 3 reals and 1 list.");
    }
  }
  
  else
  {
    cmsg.inFunError("Length of nested list expression of mtgrid object "
                    "must be 4.");
  }
  
  return word;
}

/*
Method KindCheck checks if given type is mtgrid type.

author: Dirk Zacher
parameters: type - ListExpr of type to check
            rErrorInfo - reference to error information
return value: true, if type is mtgrid type, otherwise false
exceptions: -

*/

bool mtgrid::KindCheck(ListExpr type,
                       ListExpr& rErrorInfo)
{
  return NList(type).isSymbol(mtgrid::BasicType());
}

/*
Method Open opens a mtgrid object from a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord containing mtgrid object to open
            rOffset - Offset to the mtgrid object in SmiRecord
            typeInfo - TypeInfo of mtgrid object to open
            rValue - reference to a Word referencing the opened mtgrid object
return value: true, if mtgrid object was successfully opened, otherwise false
exceptions: -

*/

bool mtgrid::Open(SmiRecord& rValueRecord,
                  size_t& rOffset,
                  const ListExpr typeInfo,
                  Word& rValue)
{
  bool bRetVal = OpenAttribute<mtgrid>(rValueRecord,
                                       rOffset,
                                       typeInfo,
                                       rValue);

  return bRetVal;
}

/*
Method Out writes out an existing mtgrid object in the form of a ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of mtgrid object to write out
            value - reference to a Word referencing the mtgrid object
return value: ListExpr of mtgrid object referenced by value
exceptions: -

*/

ListExpr mtgrid::Out(ListExpr typeInfo,
                     Word value)
{
  NList nlist;

  mtgrid* pmtgrid = static_cast<mtgrid*>(value.addr);

  if(pmtgrid != 0)
  {
    nlist.append(NList(pmtgrid->GetX()));
    nlist.append(NList(pmtgrid->GetY()));
    nlist.append(NList(pmtgrid->GetLength()));
    nlist.append(NList(pmtgrid->m_Duration.ToListExpr(true)));
  }

  return nlist.listExpr();
}

/*
Method Property returns all properties of mtgrid datatype.

author: Dirk Zacher
parameters: -
return value: properties of mtgrid datatype in the form of a ListExpr
exceptions: -

*/

ListExpr mtgrid::Property()
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
  values.append(NList(std::string("(<X> <Y> <Length> <Duration>)"), true));
  values.append(NList(std::string("(3.1415 2.718 12.0 (duration (0 60000))"),
                true));
  values.append(NList(std::string("length must be positive"), true));

  nlist = NList(names, values);

  return nlist.listExpr();
}

/*
Method Save saves an existing mtgrid object in a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord to save existing mtgrid object
            rOffset - Offset to save position of mtgrid object in SmiRecord
            typeInfo - TypeInfo of mtgrid object to save
            rValue - reference to a Word referencing the mtgrid object to save
return value: true, if mtgrid object was successfully saved, otherwise false
exceptions: -

*/

bool mtgrid::Save(SmiRecord& rValueRecord,
                  size_t& rOffset,
                  const ListExpr typeInfo,
                  Word& rValue)
{
  bool bRetVal = SaveAttribute<mtgrid>(rValueRecord,
                                       rOffset,
                                       typeInfo,
                                       rValue);

  return bRetVal;
}

/*
Method SizeOfObj returns the size of a mtgrid object.

author: Dirk Zacher
parameters: -
return value: size of a mtgrid object
exceptions: -

*/

int mtgrid::SizeOfObj()
{
  return sizeof(mtgrid);
}

}
