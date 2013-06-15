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

#include "mtstring.h"

namespace TileAlgebra
{

/*
implementation of template class mtstring

*/

mtstring::mtstring()
         :mtint()
{

}

mtstring::mtstring(bool bDefined)
         :mtint(bDefined),
          m_UniqueStringArray(bDefined)
{
  
}

mtstring::mtstring(const mtint& rmtint,
                   const UniqueStringArray& rUniqueStringArray)
         :mtint(rmtint),
          m_UniqueStringArray(rUniqueStringArray)
{
  
}

mtstring::mtstring(const mtstring& rmtstring)
         :mtint(rmtstring)
{
  
}

mtstring::~mtstring()
{

}

mtstring& mtstring::operator=(const mtstring& rmtstring)
{
  if(this != &rmtstring)
  {
    const mtint& rmtint = dynamic_cast<const mtint&>(rmtstring);
    mtint::operator =(rmtint);
    m_UniqueStringArray = rmtstring.m_UniqueStringArray;
  }

  return *this;
}

 void mtstring::atlocation(const double& rX,
                           const double& rY,
                           MString& rValues) const
{
  rValues.SetDefined(false);

  if(IsValidLocation(rX, rY))
  {
    rValues.SetDefined(true);

    Rectangle<3> boundingBox;
    bbox(boundingBox);
    Instant minimumTime = boundingBox.MinD(2);
    Instant maximumTime = boundingBox.MaxD(2);
    datetime::DateTime duration = m_Grid.GetDuration();

    rValues.StartBulkLoad();

    for(Instant currentTime = minimumTime;
        currentTime < maximumTime;
        currentTime += duration)
    {
      CcString value;
      atlocation(rX, rY, currentTime.ToDouble(), value);

      if(value.IsDefined())
      {
        Interval<Instant> interval(currentTime, currentTime + duration,
                                   true, false);
        rValues.Add(UString(interval, value, value));
      }
    }

    rValues.EndBulkLoad();

    if(rValues.IsEmpty())
    {
      rValues.SetDefined(false);
    }
  }
}

void mtstring::atlocation(const double& rX,
                          const double& rY,
                          const double& rInstant,
                          CcString& rValue) const
{
  rValue.SetDefined(false);

  if(IsValidLocation(rX, rY, rInstant))
  {
    Index<3> index = GetLocationIndex(rX, rY, rInstant);
    std::string value = GetValue(index);

    if(mtProperties<std::string>::TypeProperties::
       IsUndefinedValue(value) == false)
    {
      rValue = mtProperties<std::string>::TypeProperties::
               GetWrappedValue(value);
    }
  }
}

void mtstring::atinstant(const Instant& rInstant,
                         itstring& ritstring) const
{
  ritstring.SetDefined(false);

  tstring tstring(true);
  tstring.SetGrid(m_Grid.GetX(), m_Grid.GetY(), m_Grid.GetLength());

  int dimensionSize = mtProperties<std::string>::GetDimensionSize();
  double gridDuration = m_Grid.GetDuration().ToDouble();
  int time = static_cast<int>(rInstant.ToDouble() / gridDuration);
  bool bmtstringDefined = false;

  for(int row = 0; row < dimensionSize; row++)
  {
    for(int column = 0; column < dimensionSize; column++)
    {
      Index<3> index3 = (int[]){column, row, time};
      std::string value = GetValue(index3);

      if(mtProperties<std::string>::TypeProperties::
         IsUndefinedValue(value) == false)
      {
        bmtstringDefined = true;
        Index<2> index2 = (int[]){column, row};
        tstring.SetValue(index2, value);
      }
    }
  }
  
  if(bmtstringDefined == false)
  {
    tstring.SetDefined(false);
  }

  ritstring.SetInstant(rInstant);
  ritstring.SetValues(tstring);
}

std::string mtstring::minimum() const
{
  std::string minimum;

  if(mtProperties<int>::TypeProperties::IsUndefinedValue(m_Minimum) == false)
  {
    bool bOK = m_UniqueStringArray.GetUniqueString(m_Minimum, minimum);
    assert(bOK);
  }

  return minimum;
}

std::string mtstring::maximum() const
{
  std::string maximum;

  if(mtProperties<int>::TypeProperties::IsUndefinedValue(m_Maximum) == false)
  {
    bool bOK = m_UniqueStringArray.GetUniqueString(m_Maximum, maximum);
    assert(bOK);
  }

  return maximum;
}

std::string mtstring::GetValue(const Index<3>& rIndex) const
{
  std::string value = mtProperties<std::string>::TypeProperties::
                      GetUndefinedValue();

  int intValue = mtint::GetValue(rIndex);

  if(mtProperties<int>::TypeProperties::IsUndefinedValue(intValue) == false)
  {
    bool bOK = m_UniqueStringArray.GetUniqueString(intValue, value);
    assert(bOK);
  }

  return value;
}

bool mtstring::SetValue(const Index<3>& rIndex,
                        const std::string& rValue)
{
  bool bRetVal = false;

  int dimensionSize = mtProperties<std::string>::GetDimensionSize();

  if(IsDefined() &&
     rIndex[0] >= 0 &&
     rIndex[0] < dimensionSize &&
     rIndex[1] >= 0 &&
     rIndex[1] < dimensionSize &&
     rIndex[2] >= 0 &&
     rIndex[2] < dimensionSize)
  {
    int stringIndex = m_UniqueStringArray.AddString(rValue);

    if(stringIndex != UNDEFINED_STRING_INDEX)
    {
      bRetVal = mtint::SetValue(rIndex, stringIndex, false);
    }

    if(mtProperties<int>::TypeProperties::IsUndefinedValue(m_Minimum) ||
       rValue < minimum())
    {
      m_Minimum = stringIndex;
    }
   
    if(mtProperties<int>::TypeProperties::IsUndefinedValue(m_Maximum) ||
       rValue > maximum())
    {
      m_Maximum = stringIndex;
    }
  }

  return bRetVal;
}

bool mtstring::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

Attribute* mtstring::Clone() const
{
  Attribute* pAttribute = new mtstring(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

int mtstring::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;

  if(pAttribute != 0)
  {
    const mtstring* pmtstring = dynamic_cast<const mtstring*>(pAttribute);

    if(pmtstring != 0)
    {
      bool bIsDefined = IsDefined();
      bool bmtstringIsDefined = pmtstring->IsDefined();

      if(bIsDefined == true)
      {
        if(bmtstringIsDefined == true) // defined x defined
        {
          nRetVal = mtint::Compare(pmtstring);

          if(nRetVal == 0)
          {
            nRetVal = m_UniqueStringArray.Compare(
                      &(pmtstring->m_UniqueStringArray));
          }
        }

        else // defined x undefined
        {
          nRetVal = 1;
        }
      }

      else
      {
        if(bmtstringIsDefined == true) // undefined x defined
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

void mtstring::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const mtstring* pmtstring = dynamic_cast<const mtstring*>(pAttribute);

    if(pmtstring != 0)
    {
      *this = *pmtstring;
    }
  }
}

Flob* mtstring::GetFLOB(const int i)
{ 
  Flob* pFlob = 0;
  int nFlobs = NumOfFLOBs();
  
  if(i >= 0 &&
     i < nFlobs)
  {
    switch(i)
    {
      case 0:   pFlob = mtint::GetFLOB(i);
                break;
      case 1:   pFlob = m_UniqueStringArray.GetFLOB(i - 1);
                break;
      case 2:   pFlob = m_UniqueStringArray.GetFLOB(i - 1);
                break;
      default:  break;
    }
  }
  
  return pFlob;
}

size_t mtstring::HashValue() const
{
  size_t hashValue = 0;

  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }

  return hashValue;
}

int mtstring::NumOfFLOBs() const
{ 
  int nNumberOfFLOBs = mtint::NumOfFLOBs() +
                       m_UniqueStringArray.NumOfFLOBs();

  return nNumberOfFLOBs;
}

size_t mtstring::Sizeof() const
{
  return sizeof(mtstring);
}

const std::string mtstring::BasicType()
{
  return mtProperties<std::string>::GetTypeName();
}

void* mtstring::Cast(void* pVoid)
{
  return new(pVoid)mtstring;
}

Word mtstring::Clone(const ListExpr typeInfo,
                     const Word& rWord)
{
  Word word;

  mtstring* pmtstring = static_cast<mtstring*>(rWord.addr);

  if(pmtstring != 0)
  {
    word.addr = new mtstring(*pmtstring);
    assert(word.addr != 0);
  }

  return word;
}

void mtstring::Close(const ListExpr typeInfo,
                     Word& rWord)
{
  mtstring* pmtstring = static_cast<mtstring*>(rWord.addr);

  if(pmtstring != 0)
  {
    delete pmtstring;
    rWord.addr = 0;
  }
}

Word mtstring::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new mtstring(true);
  assert(word.addr != 0);

  return word;
}

void mtstring::Delete(const ListExpr typeInfo,
                     Word& rWord)
{
  mtstring* pmtstring = static_cast<mtstring*>(rWord.addr);

  if(pmtstring != 0)
  {
    delete pmtstring;
    rWord.addr = 0;
  }
}

TypeConstructor mtstring::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    mtstring::BasicType(), // type name function
    mtstring::Property,    // property function describing signature
    mtstring::Out,         // out function
    mtstring::In,          // in function
    0,                     // save to list function
    0,                     // restore from list function
    mtstring::Create,      // create function
    mtstring::Delete,      // delete function
    mtstring::Open,        // open function
    mtstring::Save,        // save function
    mtstring::Close,       // close function
    mtstring::Clone,       // clone function
    mtstring::Cast,        // cast function
    mtstring::SizeOfObj,   // sizeofobj function
    mtstring::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

Word mtstring::In(const ListExpr typeInfo,
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
        mtstring* pmtstring = new mtstring(true);

        if(pmtstring != 0)
        {
          datetime::DateTime duration(gridList.elem(4).realval());
          duration.SetType(datetime::durationtype);

          bool bOK = pmtstring->SetGrid(gridList.elem(1).realval(),
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
                        int dimensionSize = mtProperties<std::string>::
                                            GetDimensionSize();

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
                                  std::string stringValue = mtProperties
                                                            <std::string>::
                                                            TypeProperties::
                                                            GetUndefinedValue();

                                  if(valueList.elem(listIndex).
                                     isSymbol(Symbol::UNDEFINED()) == false)
                                  {
                                    if(mtProperties<std::string>::
                                       TypeProperties::IsValidValueType
                                      (valueList.elem(listIndex)))
                                    {
                                      stringValue = mtProperties<std::string>::
                                                    TypeProperties::GetValue(
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

                                  if(mtProperties<std::string>::TypeProperties::
                                     IsUndefinedValue(stringValue) == false)
                                  {
                                    bOK = pmtstring->SetValue(index,
                                                              stringValue);
                                  }
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
            word.addr = pmtstring;
            rCorrect = true;
          }

          else
          {
            delete pmtstring;
            pmtstring = 0;
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

bool mtstring::KindCheck(ListExpr type,
                         ListExpr& rErrorInfo)
{
  bool bRetVal = false;

  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, mtstring::BasicType());
  }

  return bRetVal;
}

bool mtstring::Open(SmiRecord& rValueRecord,
                    size_t& rOffset,
                    const ListExpr typeInfo,
                    Word& rValue)
{
  bool bRetVal = OpenAttribute<mtstring>(rValueRecord,
                                         rOffset,
                                         typeInfo,
                                         rValue);

  return bRetVal;
}

ListExpr mtstring::Out(ListExpr typeInfo,
                       Word value)
{
  ListExpr pListExpr = 0;

  if(nl != 0)
  {
    mtstring* pmtstring = static_cast<mtstring*>(value.addr);

    if(pmtstring != 0)
    {
      if(pmtstring->IsDefined() == true)
      {
        NList instanceList;

        NList gridList;
        gridList.append(pmtstring->m_Grid.GetX());
        gridList.append(pmtstring->m_Grid.GetY());
        gridList.append(pmtstring->m_Grid.GetLength());
        gridList.append(pmtstring->m_Grid.GetDuration().ToDouble());
        instanceList.append(gridList);

        NList sizeList;
        sizeList.append(mtProperties<std::string>::GetDimensionSize());
        sizeList.append(mtProperties<std::string>::GetDimensionSize());
        sizeList.append(mtProperties<std::string>::GetDimensionSize());
        instanceList.append(sizeList);

        NList tintList;
        tintList.append(0);
        tintList.append(0);
        tintList.append(0);

        std::string undefinedStringValue = mtProperties<std::string>::
                                           TypeProperties::GetUndefinedValue();
        NList valueList;

        for(int i = 0; i < mtProperties<std::string>::GetFlobElements(); i++)
        {
          int stringIndex = UNDEFINED_STRING_INDEX;
          std::string stringValue = undefinedStringValue;

          bool bOK = pmtstring->m_Flob.read(reinterpret_cast<char*>
                                            (&stringIndex),
                                            sizeof(int),
                                            i * sizeof(int));
          
          if(bOK &&
             stringIndex != UNDEFINED_INT)
          {
            bOK = pmtstring->m_UniqueStringArray.GetUniqueString(stringIndex,
                                                                 stringValue);
          }

          valueList.append(mtProperties<std::string>::TypeProperties::
                           ToNList(stringValue));
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

ListExpr mtstring::Property()
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
               (std::string("((0.0 0.0 1.0 1.0) (1 1 1) ((0 0 0 (\"A\"))))"),
                true));
  values.append(NList(std::string(""), true));

  propertyList = NList(names, values);

  return propertyList.listExpr();
}

bool mtstring::Save(SmiRecord& rValueRecord,
                    size_t& rOffset,
                    const ListExpr typeInfo,
                    Word& rValue)
{
  bool bRetVal = SaveAttribute<mtstring>(rValueRecord,
                                         rOffset,
                                         typeInfo,
                                         rValue);

  return bRetVal;
}

int mtstring::SizeOfObj()
{
  return sizeof(mtstring);
}

/*
implementation of template class mtProperties<std::string>

*/

int mtProperties<std::string>::GetDimensionSize()
{
  int dimensionSize = static_cast<unsigned int>
                      (std::pow((WinUnix::getPageSize() -
                                 sizeof(mtgrid) -
                                 2 * sizeof(int)) /
                                 sizeof(int),
                                 1.0 / 3.0)
                      );

  return dimensionSize;
}

int mtProperties<std::string>::GetFlobElements()
{
  int nFlobElements = static_cast<unsigned int>
                      (std::pow(GetDimensionSize(), 3));

  return nFlobElements;
}

SmiSize mtProperties<std::string>::GetFlobSize()
{
  SmiSize flobSize = GetFlobElements() * sizeof(int);

  return flobSize;
}

std::string mtProperties<std::string>::GetTypeName()
{
  return TYPE_NAME_MTSTRING;
}

}
