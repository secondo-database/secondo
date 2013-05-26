/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

#ifndef TILEALGEBRA_T_H
#define TILEALGEBRA_T_H

#include "tProperties.h"
#include "Attribute.h"
#include "../Grid/Grid2.h"
#include "../../Tools/Flob/Flob.h"

namespace TileAlgebra
{

/*
declaration of template class t

*/

template <typename Type, typename Properties = tProperties<Type> >
class t : public Attribute
{
  /*
  constructors

  */

  private:

  t();

  public:

  t(bool bDefined);
  t(const t& rt);

  /*
  destructor

  */

  virtual ~t();

  /*
  operators

  */

  t& operator=(const t& rt);

  /*
  methods

  */

  bool Load();
  bool SetGrid(const double& rX, const double& rY, const double& rLength);
  bool SetValue(int nIndex, Type nValue);

  /*
  override functions from base class Attribute

  */

  virtual bool Adjacent(const Attribute* pAttribute) const;
  virtual Attribute* Clone() const;
  virtual int Compare(const Attribute* pAttribute) const;
  virtual void CopyFrom(const Attribute* pAttribute);
  virtual Flob* GetFLOB(const int i);
  virtual size_t HashValue() const;
  virtual int NumOfFLOBs() const;
  virtual size_t Sizeof() const;

  /*
  The following functions are used to integrate the ~t~
  datatype into secondo.

  */

  static const std::string BasicType();
  static void* Cast(void* pVoid);
  static Word Clone(const ListExpr typeInfo,
                    const Word& rWord);
  static void Close(const ListExpr typeInfo,
                    Word& rWord);
  static Word Create(const ListExpr typeInfo);
  static void Delete(const ListExpr typeInfo,
                     Word& rWord);
  static TypeConstructor GetTypeConstructor();
  static Word In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& rErrorInfo,
                 bool& rCorrect);
  static bool KindCheck(ListExpr type,
                        ListExpr& rErrorInfo);
  static bool Open(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);
  static ListExpr Out(ListExpr typeInfo,
                      Word value);
  static ListExpr Property();
  static bool Save(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);
  static int SizeOfObj();

  private:

  /*
  members

  */

  Grid2 m_Grid;
  Flob m_Flob;
};

/*
implementation of template class t

*/

template <typename Type, typename Properties>
t<Type, Properties>::t()
                    :Attribute()
{

}

template <typename Type, typename Properties>
t<Type, Properties>::t(bool bDefined)
                    :Attribute(bDefined),
                     m_Grid(false),
                     m_Flob(Properties::GetFlobSize())
{
  int flobElements = Properties::GetFlobElements();
  Type undefinedValue = Properties::GetUndefinedValue();
  
  for(int i = 0; i < flobElements; i++)
  {
    SetValue(i, undefinedValue);
  }
}

template <typename Type, typename Properties>
t<Type, Properties>::t(const t<Type, Properties>& rt)
                    :Attribute(rt.IsDefined()),
                     m_Grid(rt.m_Grid),
                     m_Flob(rt.m_Flob)
{
  
}

template <typename Type, typename Properties>
t<Type, Properties>::~t()
{

}

template <typename Type, typename Properties>
t<Type, Properties>& t<Type, Properties>::operator=
                                          (const t<Type, Properties>& rt)
{
  if(this != &rt)
  {
    SetDefined(rt.IsDefined());
    m_Grid = rt.m_Grid;

    bool bOK = false;
    bOK = m_Flob.clean();
    assert(bOK);
    bOK = m_Flob.copyFrom(rt.m_Flob);
    assert(bOK);
  }

  return *this;
}

template <typename Type, typename Properties>
bool t<Type, Properties>::Load()
{
  bool bRetVal = true;

  /*
  According to Prof. Dr. Gueting open method of an attribute type loads
  only root record in memory. Flob data will be read immediately before
  an operator tries to access flob object and will be stored in a read cache.

  */

  const int flobSize = Properties::GetFlobSize();

  char buffer[flobSize];
  memset(buffer, 0, flobSize);
  bRetVal = m_Flob.read(buffer, flobSize, 0);

  return bRetVal;
}

template <typename Type, typename Properties>
bool t<Type, Properties>::SetGrid(const double& rX,
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

template <typename Type, typename Properties>
bool t<Type, Properties>::SetValue(int nIndex,
                                   Type nValue)
{
  bool bRetVal = false;

  if(IsDefined() &&
     nIndex >= 0 &&
     nIndex < Properties::GetFlobElements())
  {
    bRetVal = m_Flob.write(reinterpret_cast<const char*>(&nValue),
                           sizeof(Type),
                           nIndex * sizeof(Type));
  }

  return bRetVal;
}

template <typename Type, typename Properties>
bool t<Type, Properties>::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

template <typename Type, typename Properties>
Attribute* t<Type, Properties>::Clone() const
{
  Attribute* pAttribute = new t<Type, Properties>(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

template <typename Type, typename Properties>
int t<Type, Properties>::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;

  if(pAttribute != 0)
  {
    const t<Type, Properties>* pt = dynamic_cast<const t<Type, Properties>*>
                                    (pAttribute);

    if(pt != 0)
    {
      bool bIsDefined = IsDefined();
      bool btIsDefined = pt->IsDefined();

      if(bIsDefined == true)
      {
        if(btIsDefined == true) // defined x defined
        {
          if(m_Flob == pt->m_Flob)
          {
            nRetVal = 0;
          }
        }

        else // defined x undefined
        {
          nRetVal = 1;
        }
      }

      else
      {
        if(btIsDefined == true) // undefined x defined
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

template <typename Type, typename Properties>
void t<Type, Properties>::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const t<Type, Properties>* pt = dynamic_cast<const t<Type, Properties>*>
                                    (pAttribute);

    if(pt != 0)
    {
      *this = *pt;
    }
  }
}

template <typename Type, typename Properties>
Flob* t<Type, Properties>::GetFLOB(const int i)
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

template <typename Type, typename Properties>
size_t t<Type, Properties>::HashValue() const
{
  size_t hashValue = 0;

  if(IsDefined())
  {
    hashValue = (size_t)&m_Flob;
  }

  return hashValue;
}

template <typename Type, typename Properties>
int t<Type, Properties>::NumOfFLOBs() const
{ 
  return 1;
}

template <typename Type, typename Properties>
size_t t<Type, Properties>::Sizeof() const
{
  return sizeof(t<Type, Properties>);
}

template <typename Type, typename Properties>
const std::string t<Type, Properties>::BasicType()
{
  return Properties::GetTypeName();
}

template <typename Type, typename Properties>
void* t<Type, Properties>::Cast(void* pVoid)
{
  return new(pVoid)t<Type, Properties>;
}

template <typename Type, typename Properties>
Word t<Type, Properties>::Clone(const ListExpr typeInfo,
                                const Word& rWord)
{
  Word word;

  t<Type, Properties>* pt = static_cast<t<Type, Properties>*>(rWord.addr);

  if(pt != 0)
  {
    word.addr = new t<Type, Properties>(*pt);
    assert(word.addr != 0);
  }

  return word;
}

template <typename Type, typename Properties>
void t<Type, Properties>::Close(const ListExpr typeInfo,
                                Word& rWord)
{
  t<Type, Properties>* pt = static_cast<t<Type, Properties>*>(rWord.addr);

  if(pt != 0)
  {
    delete pt;
    rWord.addr = 0;
  }
}

template <typename Type, typename Properties>
Word t<Type, Properties>::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new t<Type, Properties>(true);
  assert(word.addr != 0);

  return word;
}

template <typename Type, typename Properties>
void t<Type, Properties>::Delete(const ListExpr typeInfo,
                                 Word& rWord)
{
  t<Type, Properties>* pt = static_cast<t<Type, Properties>*>(rWord.addr);

  if(pt != 0)
  {
    delete pt;
    rWord.addr = 0;
  }
}

template <typename Type, typename Properties>
TypeConstructor t<Type, Properties>::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    t<Type, Properties>::BasicType(), // type name function
    t<Type, Properties>::Property,    // property function describing signature
    t<Type, Properties>::Out,         // out function
    t<Type, Properties>::In,          // in function
    0,                                // save to list function
    0,                                // restore from list function
    t<Type, Properties>::Create,      // create function
    t<Type, Properties>::Delete,      // delete function
    t<Type, Properties>::Open,        // open function
    t<Type, Properties>::Save,        // save function
    t<Type, Properties>::Close,       // close function
    t<Type, Properties>::Clone,       // clone function
    t<Type, Properties>::Cast,        // cast function
    t<Type, Properties>::SizeOfObj,   // sizeofobj function
    t<Type, Properties>::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

template <typename Type, typename Properties>
Word t<Type, Properties>::In(const ListExpr typeInfo,
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
        t<Type, Properties>* pt = new t<Type, Properties>(true);

        if(pt != 0)
        {
          bool bOK = pt->SetGrid(gridList.elem(1).realval(),
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
                        int dimensionSize = Properties::GetDimensionSize();

                        if(indexX >= 0 &&
                           indexX <= dimensionSize - sizeX &&
                           indexY >= 0 &&
                           indexY <= dimensionSize - sizeY)
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
                                                 dimensionSize +
                                                (indexX + column);
                                Type value = Properties::GetUndefinedValue();

                                if(valueList.elem(listIndex).
                                   isSymbol(Symbol::UNDEFINED()) == false)
                                {
                                  if(Properties::IsValidValueType
                                    (valueList.elem(listIndex)))
                                  {
                                    value = Properties::GetValue(
                                            valueList.elem(listIndex));
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

                                pt->SetValue(flobIndex, value);
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
            word.setAddr(pt);
            rCorrect = true;
          }

          else
          {
            delete pt;
            pt = 0;
            rCorrect = false;
          }
        }
      }

      else
      {
        cmsg.inFunError("Type mismatch: expected 3 reals as grid2 sublist.");
      }
    }

    else
    {
      cmsg.inFunError("Type mismatch: list for grid2 is too short "
                      "or too long.");
    }
  }

  else
  {
    cmsg.inFunError("Expected list as first element, got an atom.");
  }

  return word;
}

template <typename Type, typename Properties>
bool t<Type, Properties>::KindCheck(ListExpr type,
                                    ListExpr& rErrorInfo)
{
  bool bRetVal = false;

  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, t<Type, Properties>::BasicType());
  }

  return bRetVal;
}

template <typename Type, typename Properties>
bool t<Type, Properties>::Open(SmiRecord& rValueRecord,
                               size_t& rOffset,
                               const ListExpr typeInfo,
                               Word& rValue)
{
  bool bRetVal = OpenAttribute<t<Type, Properties> >(rValueRecord,
                                                     rOffset,
                                                     typeInfo,
                                                     rValue);

  return bRetVal;
}

template <typename Type, typename Properties>
ListExpr t<Type, Properties>::Out(ListExpr typeInfo,
                                  Word value)
{
  ListExpr pListExpr = 0;

  if(nl != 0)
  {
    t<Type, Properties>* pt = static_cast<t<Type, Properties>*>(value.addr);

    if(pt != 0)
    {
      if(pt->IsDefined() == true)
      {
        NList instanceList;

        NList gridList;
        gridList.append(pt->m_Grid.GetX());
        gridList.append(pt->m_Grid.GetY());
        gridList.append(pt->m_Grid.GetLength());
        instanceList.append(gridList);

        NList sizeList;
        sizeList.append(Properties::GetDimensionSize());
        sizeList.append(Properties::GetDimensionSize());
        instanceList.append(sizeList);

        NList tintList;
        tintList.append(0);
        tintList.append(0);

        Type undefinedValue = Properties::GetUndefinedValue();
        NList valueList;

        for(int i = 0; i < Properties::GetFlobElements(); i++)
        {
          Type value = undefinedValue;

          bool bOK = pt->m_Flob.read(reinterpret_cast<char*>(&value),
                                     sizeof(Type),
                                     i * sizeof(Type));
          assert(bOK);

          valueList.append(Properties::ToNList(value));
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

template <typename Type, typename Properties>
ListExpr t<Type, Properties>::Property()
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

template <typename Type, typename Properties>
bool t<Type, Properties>::Save(SmiRecord& rValueRecord,
                               size_t& rOffset,
                               const ListExpr typeInfo,
                               Word& rValue)
{
  bool bRetVal = SaveAttribute<t<Type, Properties> >(rValueRecord,
                                                     rOffset,
                                                     typeInfo,
                                                     rValue);

  return bRetVal;
}

template <typename Type, typename Properties>
int t<Type, Properties>::SizeOfObj()
{
  return sizeof(t<Type, Properties>);
}

}

#endif // TILEALGEBRA_T_H
