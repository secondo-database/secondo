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

#ifndef TILEALGEBRA_MT_H
#define TILEALGEBRA_MT_H

#include "TypeConstructor.h"
#include "mtProperties.h"
#include "Attribute.h"
#include "../grid/mtgrid.h"
#include "../../Tools/Flob/Flob.h"
#include "../Index.h"

namespace TileAlgebra
{

/*
declaration of template class t

*/

template <typename Type, typename Properties = mtProperties<Type> >
class mt : public Attribute
{
  /*
  constructors

  */

  protected:

  mt();

  public:

  mt(bool bDefined);
  mt(const mt& rmt);

  /*
  destructor

  */

  virtual ~mt();

  /*
  operators

  */

  mt& operator=(const mt& rmt);

  /*
  TileAlgebra operator methods

  */

  mtgrid GetGrid() const;
  Type GetMinimum() const;
  Type GetMaximum() const;
  // Rectangle<3> GetBoundingBox() const;

  protected:

  /*
  internal methods

  */

  bool SetGrid(const double& rX, const double& rY, const double& rLength,
               const datetime::DateTime& rDuration);
  void SetMinimum(const Type& rValue);
  void SetMaximum(const Type& rValue);
  Type GetValue(const Index<3>& rIndex);
  bool SetValue(const Index<3>& rIndex, const Type& rValue);

  public:

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

  protected:

  /*
  members

  */

  mtgrid m_Grid;
  Type m_Minimum;
  Type m_Maximum;
  Flob m_Flob;
};

/*
implementation of template class t

*/

template <typename Type, typename Properties>
mt<Type, Properties>::mt()
                     :Attribute()
{

}

template <typename Type, typename Properties>
mt<Type, Properties>::mt(bool bDefined)
                     :Attribute(bDefined),
                      m_Grid(false),
                      m_Minimum(Properties::TypeProperties::
                                GetUndefinedValue()),
                      m_Maximum(Properties::TypeProperties::
                                GetUndefinedValue()),
                      m_Flob(Properties::GetFlobSize())
{
  int dimensionSize = Properties::GetDimensionSize();
  Type undefinedValue = Properties::TypeProperties::GetUndefinedValue();
  
  for(int time = 0; time < dimensionSize; time++)
  {
    for(int row = 0; row < dimensionSize; row++)
    {
      for(int column = 0; column < dimensionSize; column++)
      {
        Index<3> indexes = (int[]){column, row, time};
        SetValue(indexes, undefinedValue);
      }
    }
  }
}

template <typename Type, typename Properties>
mt<Type, Properties>::mt(const mt<Type, Properties>& rmt)
                     :Attribute(rmt.IsDefined()),
                      m_Grid(rmt.m_Grid),
                      m_Minimum(rmt.m_Minimum),
                      m_Maximum(rmt.m_Maximum),
                      m_Flob(rmt.m_Flob)
{
  
}

template <typename Type, typename Properties>
mt<Type, Properties>::~mt()
{

}

template <typename Type, typename Properties>
mt<Type, Properties>& mt<Type, Properties>::operator=
                                            (const mt<Type, Properties>& rmt)
{
  if(this != &rmt)
  {
    Attribute::operator=(rmt);
    m_Grid = rmt.m_Grid;
    m_Minimum = rmt.m_Minimum;
    m_Maximum = rmt.m_Maximum;

    bool bOK = false;
    bOK = m_Flob.clean();
    assert(bOK);
    bOK = m_Flob.copyFrom(rmt.m_Flob);
    assert(bOK);
  }

  return *this;
}

template <typename Type, typename Properties>
mtgrid mt<Type, Properties>::GetGrid() const
{
  return m_Grid;
}

template <typename Type, typename Properties>
Type mt<Type, Properties>::GetMinimum() const
{
  return m_Minimum;
}

template <typename Type, typename Properties>
Type mt<Type, Properties>::GetMaximum() const
{
  return m_Maximum;
}

/*
template <typename Type, typename Properties>
Rectangle<3> mt<Type, Properties>::GetBoundingBox() const
{

}

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::SetGrid(const double& rX,
                                   const double& rY,
                                   const double& rLength,
                                   const datetime::DateTime& rDuration)
{
  bool bRetVal = true;

  m_Grid.SetDefined(true);
  bRetVal &= m_Grid.SetX(rX);
  bRetVal &= m_Grid.SetY(rY);
  bRetVal &= m_Grid.SetLength(rLength);
  bRetVal &= m_Grid.SetDuration(rDuration);

  return bRetVal;
}

template <typename Type, typename Properties>
void mt<Type, Properties>::SetMinimum(const Type& rValue)
{
  m_Minimum = rValue;
}

template <typename Type, typename Properties>
void mt<Type, Properties>::SetMaximum(const Type& rValue)
{
  m_Maximum = rValue;
}

template <typename Type, typename Properties>
Type mt<Type, Properties>::GetValue(const Index<3>& rIndex)
{
  Type value = Properties::TypeProperties::GetUndefinedValue();

  int dimensionSize = Properties::GetDimensionSize();

  if(IsDefined() &&
     rIndex[0] >= 0 &&
     rIndex[0] < dimensionSize &&
     rIndex[1] >= 0 &&
     rIndex[1] < dimensionSize &&
     rIndex[2] >= 0 &&
     rIndex[2] < dimensionSize)
  {
    int flobIndex = rIndex[2] * dimensionSize * dimensionSize +
                    rIndex[1] * dimensionSize + rIndex[0];

    bool bOK = m_Flob.read(reinterpret_cast<const char*>(&value),
                           sizeof(Type),
                           flobIndex * sizeof(Type));
    assert(bOK);
  }

  return value;
}

template <typename Type, typename Properties>
bool mt<Type, Properties>::SetValue(const Index<3>& rIndex,
                                    const Type& rValue)
{
  bool bRetVal = false;

  int dimensionSize = Properties::GetDimensionSize();

  if(IsDefined() &&
     rIndex[0] >= 0 &&
     rIndex[0] < dimensionSize &&
     rIndex[1] >= 0 &&
     rIndex[1] < dimensionSize &&
     rIndex[2] >= 0 &&
     rIndex[2] < dimensionSize)
  {
    int flobIndex = rIndex[2] * dimensionSize * dimensionSize +
                    rIndex[1] * dimensionSize + rIndex[0];
    
    bRetVal = m_Flob.write(reinterpret_cast<const char*>(&rValue),
                           sizeof(Type),
                           flobIndex * sizeof(Type));
  }

  return bRetVal;
}

template <typename Type, typename Properties>
bool mt<Type, Properties>::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

template <typename Type, typename Properties>
Attribute* mt<Type, Properties>::Clone() const
{
  Attribute* pAttribute = new mt<Type, Properties>(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

template <typename Type, typename Properties>
int mt<Type, Properties>::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;

  if(pAttribute != 0)
  {
    const mt<Type, Properties>* pmt = dynamic_cast<const mt<Type, Properties>*>
                                      (pAttribute);

    if(pmt != 0)
    {
      bool bIsDefined = IsDefined();
      bool btIsDefined = pmt->IsDefined();

      if(bIsDefined == true)
      {
        if(btIsDefined == true) // defined x defined
        {
          nRetVal = m_Grid.Compare(&(pmt->m_Grid));

          if(nRetVal == 0)
          {
            SmiSize flobSize = Properties::GetFlobSize();
            
            char buffer1[flobSize];
            m_Flob.read(buffer1, flobSize, 0);

            char buffer2[flobSize];
            pmt->m_Flob.read(buffer2, flobSize, 0);

            nRetVal = memcmp(buffer1, buffer2, flobSize);
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
void mt<Type, Properties>::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const mt<Type, Properties>* pmt = dynamic_cast<const mt<Type, Properties>*>
                                      (pAttribute);

    if(pmt != 0)
    {
      *this = *pmt;
    }
  }
}

template <typename Type, typename Properties>
Flob* mt<Type, Properties>::GetFLOB(const int i)
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
size_t mt<Type, Properties>::HashValue() const
{
  size_t hashValue = 0;

  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }

  return hashValue;
}

template <typename Type, typename Properties>
int mt<Type, Properties>::NumOfFLOBs() const
{ 
  return 1;
}

template <typename Type, typename Properties>
size_t mt<Type, Properties>::Sizeof() const
{
  return sizeof(mt<Type, Properties>);
}

template <typename Type, typename Properties>
const std::string mt<Type, Properties>::BasicType()
{
  return Properties::GetTypeName();
}

template <typename Type, typename Properties>
void* mt<Type, Properties>::Cast(void* pVoid)
{
  return new(pVoid)mt<Type, Properties>;
}

template <typename Type, typename Properties>
Word mt<Type, Properties>::Clone(const ListExpr typeInfo,
                                 const Word& rWord)
{
  Word word;

  mt<Type, Properties>* pmt = static_cast<mt<Type, Properties>*>(rWord.addr);

  if(pmt != 0)
  {
    word.addr = new mt<Type, Properties>(*pmt);
    assert(word.addr != 0);
  }

  return word;
}

template <typename Type, typename Properties>
void mt<Type, Properties>::Close(const ListExpr typeInfo,
                                Word& rWord)
{
  mt<Type, Properties>* pmt = static_cast<mt<Type, Properties>*>(rWord.addr);

  if(pmt != 0)
  {
    delete pmt;
    rWord.addr = 0;
  }
}

template <typename Type, typename Properties>
Word mt<Type, Properties>::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new mt<Type, Properties>(true);
  assert(word.addr != 0);

  return word;
}

template <typename Type, typename Properties>
void mt<Type, Properties>::Delete(const ListExpr typeInfo,
                                  Word& rWord)
{
  mt<Type, Properties>* pmt = static_cast<mt<Type, Properties>*>(rWord.addr);

  if(pmt != 0)
  {
    delete pmt;
    rWord.addr = 0;
  }
}

template <typename Type, typename Properties>
TypeConstructor mt<Type, Properties>::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    mt<Type, Properties>::BasicType(), // type name function
    mt<Type, Properties>::Property,    // property function describing signature
    mt<Type, Properties>::Out,         // out function
    mt<Type, Properties>::In,          // in function
    0,                                // save to list function
    0,                                // restore from list function
    mt<Type, Properties>::Create,      // create function
    mt<Type, Properties>::Delete,      // delete function
    mt<Type, Properties>::Open,        // open function
    mt<Type, Properties>::Save,        // save function
    mt<Type, Properties>::Close,       // close function
    mt<Type, Properties>::Clone,       // clone function
    mt<Type, Properties>::Cast,        // cast function
    mt<Type, Properties>::SizeOfObj,   // sizeofobj function
    mt<Type, Properties>::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

template <typename Type, typename Properties>
Word mt<Type, Properties>::In(const ListExpr typeInfo,
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

    if(gridList.length() == 4)
    {
      if(gridList.isReal(1) &&
         gridList.isReal(2) &&
         gridList.isReal(3) &&
         gridList.isReal(4))
      {
        mt<Type, Properties>* pmt = new mt<Type, Properties>(true);

        if(pmt != 0)
        {
          datetime::DateTime duration(gridList.elem(4).realval());
          duration.SetType(datetime::durationtype);

          bool bOK = pmt->SetGrid(gridList.elem(1).realval(),
                                  gridList.elem(2).realval(),
                                  gridList.elem(3).realval(),
                                  duration);

          if(bOK == true)
          {
            instanceList.rest();

            if(instanceList.isEmpty() == false)
            {
              NList sizeList = instanceList.elem(1);

              if(sizeList.length() == 3)
              {
                if(sizeList.isInt(1) &&
                   sizeList.isInt(2) &&
                   sizeList.isInt(3) &&
                   sizeList.elem(1).intval() > 0 &&
                   sizeList.elem(2).intval() > 0 &&
                   sizeList.elem(3).intval() > 0)
                {
                  int sizeX = sizeList.elem(1).intval();
                  int sizeY = sizeList.elem(2).intval();
                  int sizeT = sizeList.elem(3).intval();
                  Cardinal valueListLength = static_cast<Cardinal>
                                             (sizeX * sizeY * sizeT);

                  instanceList.rest();

                  while(bOK &&
                        instanceList.isEmpty() == false)
                  {
                    NList pageList = instanceList.first();

                    if(pageList.length() == 4)
                    {
                      if(pageList.isInt(1) &&
                         pageList.isInt(2) &&
                         pageList.isInt(3))
                      {
                        int indexX = pageList.elem(1).intval();
                        int indexY = pageList.elem(2).intval();
                        int indexT = pageList.elem(3).intval();
                        int dimensionSize = Properties::GetDimensionSize();

                        if(indexX >= 0 &&
                           indexX <= dimensionSize - sizeX &&
                           indexY >= 0 &&
                           indexY <= dimensionSize - sizeY &&
                           indexT >= 0 &&
                           indexT <= dimensionSize - sizeT)
                        {
                          pageList.rest();
                          pageList.rest();
                          pageList.rest();

                          NList valueList = pageList.first();

                          if(valueList.length() == valueListLength)
                          {
                            int listIndex = 0;

                            for(int time = 0; time < sizeT; time++)
                            {
                              for(int row = 0; row < sizeY; row++)
                              {
                                for(int column = 0; column < sizeX; column++)
                                {
                                  listIndex++;

                                  Index<3> index = (int[]){(indexX + column),
                                                           (indexY + row),
                                                           (indexT + time)};
                                  Type value = Properties::TypeProperties::
                                               GetUndefinedValue();

                                  if(valueList.elem(listIndex).
                                     isSymbol(Symbol::UNDEFINED()) == false)
                                  {
                                    if(Properties::TypeProperties::
                                       IsValidValueType
                                       (valueList.elem(listIndex)))
                                    {
                                      value = Properties::TypeProperties::
                                              GetValue(
                                              valueList.elem(listIndex));

                                      Type minimum = pmt->GetMinimum();

                                      if(Properties::TypeProperties::
                                         IsUndefinedValue(minimum) ||
                                         value < minimum)
                                      {
                                        pmt->SetMinimum(value);
                                      }

                                      Type maximum = pmt->GetMaximum();

                                      if(Properties::TypeProperties::
                                         IsUndefinedValue(maximum) ||
                                         value > maximum)
                                      {
                                        pmt->SetMaximum(value);
                                      }
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

                                  pmt->SetValue(index, value);
                                }
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
                                        "with three integers.");
                      }
                    }

                    else
                    {
                      bOK = false;
                      cmsg.inFunError("Type mismatch: "
                                      "partial grid content must contain "
                                      "four elements.");
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
                cmsg.inFunError("Size list must have a length of 3.");
              }
            }
          }

          if(bOK)
          {
            word.addr = pmt;
            rCorrect = true;
          }

          else
          {
            delete pmt;
            pmt = 0;
            rCorrect = false;
          }
        }
      }

      else
      {
        cmsg.inFunError("Type mismatch: expected 4 reals as mtgrid sublist.");
      }
    }

    else
    {
      cmsg.inFunError("Type mismatch: list for mtgrid is too short "
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
bool mt<Type, Properties>::KindCheck(ListExpr type,
                                     ListExpr& rErrorInfo)
{
  bool bRetVal = false;

  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, mt<Type, Properties>::BasicType());
  }

  return bRetVal;
}

template <typename Type, typename Properties>
bool mt<Type, Properties>::Open(SmiRecord& rValueRecord,
                                size_t& rOffset,
                                const ListExpr typeInfo,
                                Word& rValue)
{
  bool bRetVal = OpenAttribute<mt<Type, Properties> >(rValueRecord,
                                                      rOffset,
                                                      typeInfo,
                                                      rValue);

  return bRetVal;
}

template <typename Type, typename Properties>
ListExpr mt<Type, Properties>::Out(ListExpr typeInfo,
                                   Word value)
{
  ListExpr pListExpr = 0;

  if(nl != 0)
  {
    mt<Type, Properties>* pmt = static_cast<mt<Type, Properties>*>(value.addr);

    if(pmt != 0)
    {
      if(pmt->IsDefined() == true)
      {
        NList instanceList;

        NList gridList;
        gridList.append(pmt->m_Grid.GetX());
        gridList.append(pmt->m_Grid.GetY());
        gridList.append(pmt->m_Grid.GetLength());
        gridList.append(pmt->m_Grid.GetDuration().ToDouble());
        instanceList.append(gridList);

        NList sizeList;
        sizeList.append(Properties::GetDimensionSize());
        sizeList.append(Properties::GetDimensionSize());
        sizeList.append(Properties::GetDimensionSize());
        instanceList.append(sizeList);

        NList tintList;
        tintList.append(0);
        tintList.append(0);
        tintList.append(0);

        Type undefinedValue = Properties::TypeProperties::GetUndefinedValue();
        NList valueList;

        for(int i = 0; i < Properties::GetFlobElements(); i++)
        {
          Type value = undefinedValue;

          bool bOK = pmt->m_Flob.read(reinterpret_cast<char*>(&value),
                                      sizeof(Type),
                                      i * sizeof(Type));
          assert(bOK);

          valueList.append(Properties::TypeProperties::ToNList(value));
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
ListExpr mt<Type, Properties>::Property()
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
               (std::string("((x y l t) (szx szy szt) ((ix iy it (v*)))*)"),
                true));
  values.append(NList
               (std::string("((0.0 0.0 1.0 1.0) (1 1 1) ((0 0 0 (0))))"),
                true));
  values.append(NList(std::string(""), true));

  propertyList = NList(names, values);

  return propertyList.listExpr();
}

template <typename Type, typename Properties>
bool mt<Type, Properties>::Save(SmiRecord& rValueRecord,
                                size_t& rOffset,
                                const ListExpr typeInfo,
                                Word& rValue)
{
  bool bRetVal = SaveAttribute<mt<Type, Properties> >(rValueRecord,
                                                      rOffset,
                                                      typeInfo,
                                                      rValue);

  return bRetVal;
}

template <typename Type, typename Properties>
int mt<Type, Properties>::SizeOfObj()
{
  return sizeof(mt<Type, Properties>);
}

}

#endif // TILEALGEBRA_MT_H
