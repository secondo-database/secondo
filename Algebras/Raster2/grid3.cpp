 
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

#include <cmath>

#include <TypeConstructor.h>

#include "grid3.h"

namespace raster2
{

grid3::grid3()
      : grid2(), m_Duration(durationtype)
{
  
}

grid3::grid3(const double& rOriginX,
             const double& rOriginY,
             const double& rLength,
             const DateTime& rDuration)
      :grid2(rOriginX, rOriginY, rLength),
       m_Duration(rDuration)
{
    m_Duration.SetType(durationtype);
}

grid3::~grid3()
{
  
}

const DateTime& grid3::getDuration() const
{
  return m_Duration;
}

RasterIndex<3> grid3::getIndex(double xcoord, double ycoord, double t) const {
    return RasterIndex<3>((int[]){std::floor((xcoord - x)/length),
                                  std::floor((ycoord - y)/length),
                                  std::floor(t/m_Duration.ToDouble())});
}

grid3::region_type grid3::getRegion(const Rectangle<3>& bbox) const {
    return region_type(
        index_type((int[]){std::floor((bbox.MinD(0) - x)/length),
                           std::floor((bbox.MinD(1) - y)/length),
                           std::floor(bbox.MinD(2)/m_Duration.ToDouble())}),
        index_type((int[]){std::floor((bbox.MaxD(0) - x)/length),
                           std::floor((bbox.MaxD(1) - y)/length),
                           std::floor(bbox.MaxD(2)/m_Duration.ToDouble())})
    );
}

Rectangle<3> grid3::getCell(const index_type& i) const {
  return Rectangle<3>(true,
      i[0] * length + x,            (1 + i[0]) * length + x,
      i[1] * length + y,            (1 + i[1]) * length + y,
      i[2] * m_Duration.ToDouble(), (1 + i[2]) * m_Duration.ToDouble() );
}

Rectangle<3> grid3::getBBox(const region_type& r) const {
  return Rectangle<3>(true,
      r.Min[0] * length + x,            (1 + r.Max[0]) * length + x,
      r.Min[1] * length + y,            (1 + r.Max[1]) * length + y,
      r.Min[2] * m_Duration.ToDouble(), (1 + r.Max[2]) * m_Duration.ToDouble());
}

Rectangle<3> grid3::getBBox(const index_type& from, const index_type& to) const
{
  return Rectangle<3>(true,
      from[0] * length + x,            (1 + to[0]) * length + x,
      from[1] * length + y,            (1 + to[1]) * length + y,
      from[2] * m_Duration.ToDouble(), (1 + to[2]) * m_Duration.ToDouble());
}

bool grid3::matches(const grid3& g2) const{
  if(m_Duration != g2.m_Duration){
      return false;
  }
  return grid2::matches(g2);
}

bool grid3::matches(const grid2& g2) const{
  return grid2::matches(g2);
}

grid2 grid3::project() const{
  return *this;
}


void grid3::set(const double ax, const double ay, const double alength, 
         const datetime::DateTime& dt){

   grid2::set(ax,ay,alength);
   m_Duration = dt;
}


std::string grid3::BasicType()
{
  return "grid3";
}

bool grid3::checkType(ListExpr t){
  return nl->IsEqual(t,BasicType());
}


void* grid3::Cast(void* pVoid)
{
  return new (pVoid) grid3;
}

Word grid3::Clone(const ListExpr typeInfo,
                  const Word& w)
{
  Word word = SetWord(Address(0));
  
  grid3* pgrid3 = static_cast<grid3*>(w.addr);
  
  if(pgrid3 != 0)
  {
    word = SetWord(new grid3(*pgrid3));
  }
  
  return word;
}

void grid3::Close(const ListExpr typeInfo, Word& w)
{
  grid3* pgrid3 = static_cast<grid3*>(w.addr);
    
  if(pgrid3 != 0)
  {
    delete pgrid3;
    w = SetWord(Address(0));
  }
}

Word grid3::Create(const ListExpr typeInfo)
{
  Word w = SetWord(Address(0));
  
  grid3* pgrid3 = new grid3(0.0, 0.0, 1.0, 1.0);
  
  if(pgrid3 != 0)
  {
    w = SetWord(pgrid3);
  }
  
  return w;
}

void grid3::Delete(const ListExpr typeInfo,
                   Word& w)
{
  grid3* pgrid3 = static_cast<grid3*>(w.addr);
    
  if(pgrid3 != 0)
  {
    delete pgrid3;
    w = SetWord(Address(0));
  }
}

TypeConstructor grid3::getTypeConstructor()
{
  TypeConstructor typeConstructorgrid3(
    grid3::BasicType(),
    grid3::Property,
    grid3::Out,
    grid3::In,
    0,
    0,
    grid3::Create,
    grid3::Delete,
    0,
    0,
    grid3::Close,
    grid3::Clone,
    grid3::Cast,
    grid3::SizeOfObj,
    grid3::KindCheck);
  
  typeConstructorgrid3.AssociateKind(Kind::SIMPLE());
  
  return typeConstructorgrid3;
}

/*
List expression of ~grid3~ is (x y l duration) where ~x~ is the first coordinate
of the origin, ~y~ is the second coordinate of the origin, ~l~ is the size of
a cell and ~duration~ is the duration.

~l~ must be >= 0.

*/

Word grid3::In(const ListExpr typeInfo,
               const ListExpr instance,
               const int errorPos,
               ListExpr& errorInfo,
               bool& correct)
{ 
  Word w = SetWord(Address(0));
  correct = false;

  NList nlist(instance);

  if(nlist.length() == 4)
  {
    if(nlist.isReal(1) &&
       nlist.isReal(2) &&
       nlist.isReal(3) &&
       nlist.isList(4))
    {
      grid3* pgrid3 = new grid3();
      
      if(pgrid3 != 0)
      {
        pgrid3->x = nlist.elem(1).realval();
        pgrid3->y = nlist.elem(2).realval();
        pgrid3->length = nlist.elem(3).realval();

        if(pgrid3->length <= 0.0)
        {
          delete pgrid3;
          pgrid3 = 0;
          cmsg.inFunError("Length must be larger than zero.");
        }

        else
        {
          bool bOK = pgrid3->m_Duration.ReadFrom(nlist.elem(4).listExpr(),
                                                 true);

          if(bOK == true && pgrid3->m_Duration.IsDefined())
          {
            correct = true;
            w = SetWord(pgrid3);
          }

          else
          {
            delete pgrid3;
            pgrid3 = 0;
            cmsg.inFunError("Duration must be (duration (days milliseconds))");
          }
        }
      }
    }

    else
    {
        cmsg.inFunError("Type mismatch.");
    }
  } else {
      cmsg.inFunError("List length must be 4.");
  }
  
  return w;
}

bool grid3::KindCheck(ListExpr type, ListExpr& errorInfo)
{
  return NList(type).isSymbol(grid3::BasicType());
}




ListExpr grid3::Out(ListExpr typeInfo, Word value)
{ 
  NList outList;
  
  grid3* pgrid3 = static_cast<grid3*>(value.addr);
  
  if(pgrid3 != 0)
  {
    outList.append(NList(pgrid3->x));
    outList.append(NList(pgrid3->y));
    outList.append(NList(pgrid3->length));
    outList.append(NList(pgrid3->m_Duration.ToListExpr(true)));
  }

  return outList.listExpr();
}

ListExpr grid3::Property()
{
  NList property;

  NList names;
  names.append(NList(std::string("Signature"), true));
  names.append(NList(std::string("Example Type List"), true));
  names.append(NList(std::string("ListRep"), true));
  names.append(NList(std::string("Example List"), true));
  names.append(NList(std::string("Remarks"), true));

  NList values;
  values.append(NList(std::string("-> DATA"), true));
  values.append(NList(BasicType(), true));
  values.append(NList(std::string("(<x> <y> <length> <duration>)"), true));
  values.append(NList(std::string("(3.1415 2.718 12.0 (duration (0 60000))"),
                true));
  values.append(NList(std::string("length must be positive"), true));

  property = NList(names, values);

  return property.listExpr();
}

int grid3::SizeOfObj()
{
  return sizeof(grid3);
}

}
