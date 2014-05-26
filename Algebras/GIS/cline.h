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

#ifndef GIS_CLINE_H
#define GIS_CLINE_H

#include "DLine.h"

namespace GISAlgebra 
{
  class CLine : public Attribute
  {
    public:
      CLine(){}
     
      CLine(bool d): def(d), level(0), segments(0), bbox(false,0,0,0,0) {}

      CLine(const CLine& s): def(s.IsDef()), level(s.getLevel()), 
                             segments(s.segments.getSize()), 
                             bbox(s.BoundingBox())
      {
        segments.copyFrom(s.segments);
      }


      ~CLine(){}

      bool IsDef() const{ return def; }

      void SetDef(bool b){ def = b; }

      void clear()
      {
         SetDef(true);
         segments.clean();
         bbox.SetDefined(false);
      }

      void append(const SimpleSegment& s) 
      {
        segments.Append(s);
        bbox.Extend(s.getBox());
      }

      void get(size_t index, SimpleSegment& s) const
      {
        segments.Get(index,s);
      }
 
      int getLevel() const
      {
        return level;
      }

      void set(size_t index, const SimpleSegment& s) 
      {
        segments.Put(index,s);
        bbox.Extend(s.getBox());
      }    

      void setLevel(int l) 
      {
        level = l;
      }

      void resize(size_t newSize)
      {
        segments.resize(newSize);
      }
     
      virtual const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const
      {
        return bbox;
      }

      void setBoundingBox(Rectangle<2> bb)
      {
        bbox = bb;
      }

      int Compare(const Attribute* rhs) const
      {
        if(!IsDefined())
        {
          return rhs->IsDefined()?-1:0; 
        }
        if(!rhs->IsDefined())
        {
          return 1;
        }
       
        CLine* dl = (CLine*) rhs;
        if(segments.Size() < dl->segments.Size())
        {
          return -1;
        }
        if(segments.Size() > dl->segments.Size())
        {
          return 1;
        }
        SimpleSegment ts;
        SimpleSegment ds;
        for(int i=0;i<segments.Size();i++)
        {
          segments.Get(i,ts);
          dl->segments.Get(i,ds);
          int cmp = ts.compare(ds);
          if(cmp!=0)
          {
            return cmp;
          }
        }
        return 0;
      }

      int NumOfFLOBs() const
      {
        return 1;
      }

      Flob* GetFLOB(int i)
      {
        return &segments;
      }
    
      bool Adjacent(const Attribute*) const {return false;}

      size_t HashValue() const { return segments.Size(); }

      void CopyFrom(const Attribute* arg) 
      {
        segments.clean();

        if(!arg->IsDefined())
        {
          SetDefined(false);
        } 
        else 
        { 
          SetDefined(true);
        }

        CLine* d = (CLine*) arg;
        level = d->level;
        segments.copyFrom(d->segments); 
        bbox = d->bbox;
      }

      void CopyTo(CLine& arg) 
      {
        arg.clear();

        if(!IsDef())
        {
          arg.SetDef(false);
        } 
        else 
        { 
          arg.SetDef(true);
        }

        arg.setLevel(level);
        arg.setBoundingBox(bbox);
        arg.segments.copyFrom(segments); 
      }


      size_t Sizeof() const { return sizeof(*this); }

      virtual ostream& Print( ostream& os ) const
      { 
        if(!IsDef())
        {
           os << "Undefined";
           return os;
        }

        os << level << endl;
        SimpleSegment s;
        for(int i=0;i<segments.Size();i++)
        {
           segments.Get(i,s);
           s.print(os);
           os << endl;
        }
        return os;
      }

      static string BasicType()
      {
         return "cline";
      }
     
      static ListExpr Property()
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
               (std::string("(level s_i s_i ..) with s_i=(x1 y1 x2 y2)"), 
                                                                  true));
        values.append(NList
               (std::string("(10 (3.5 5.0 12.2 1.1))"), true));
        values.append(NList(std::string(""), true));

        propertyList = NList(names, values);

        return propertyList.listExpr();
      }    

      static bool KindCheck(ListExpr type, ListExpr& errorInfo)
      {
        return nl->IsEqual(type,BasicType());
      }   

      int getSize() const
      {
        return segments.Size();
      }

      static TypeConstructor GetTypeConstructor()
      {
        TypeConstructor typeConstructor
        (
          BasicType(), // type name function
          Property,    // property function describing signature
          Out,         // out function
          In,          // in function
          0,                  // save to list function
          0,                  // restore from list function
          Create,      // create function
          Delete,      // delete function
          Open,        // open function
          Save,        // save function
          Close,       // close function
          Clone,       // clone function
          Cast,        // cast function
          SizeOfObj,   // sizeofobj function
          KindCheck    // kindcheck function
        );

        typeConstructor.AssociateKind(Kind::DATA());

        return typeConstructor;
      }

      static ListExpr Out(ListExpr typeInfo, Word value)
      {
        ListExpr pListExpr = 0;

        if(nl != 0)
        {
          CLine* pt = static_cast<CLine*>(value.addr);

          if(pt != 0)
          {
            if(pt->IsDefined() == true)
            {
              NList instanceList;

              NList level;
              level.append(pt->getLevel());
              instanceList.append(level);

              /*int s = pt.segments.Size();

              NList segmentList;

              SimpleSegment out;
              for (int i=0; i<s; i++)
              {
                pt.segments.get(i,out);
                segmentList.append(out);
              }

              instanceList.append(segmentList);*/

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

      static Word In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& rErrorInfo, bool& rCorrect)
      {
        Word word;

        NList instanceList(instance);

        return word;
      }

      static Word Create(const ListExpr typeInfo)
      {
        Word word;
        word.addr = new CLine(true);
        assert(word.addr != 0);
      
        return word;
      }

      static void Delete(const ListExpr typeInfo, Word& rWord)
      {
        CLine* pt = static_cast<CLine*>(rWord.addr);
      
        if(pt != 0)
        {
          delete pt;
          rWord.addr = 0;
        }
      }

      static bool Open(SmiRecord& rValueRecord, size_t& rOffset,
                             const ListExpr typeInfo, Word& rValue)
      {
        bool bRetVal = OpenAttribute<CLine>(rValueRecord, rOffset, 
                                            typeInfo, rValue);

        return bRetVal;
      }

      static bool Save(SmiRecord& rValueRecord, size_t& rOffset,
                       const ListExpr typeInfo, Word& rValue)
      {
        bool bRetVal = SaveAttribute<CLine>(rValueRecord, rOffset,
                                            typeInfo, rValue);

        return bRetVal;
      }

      static void Close(const ListExpr typeInfo, Word& rWord)
      {
        CLine* pt = static_cast<CLine*>(rWord.addr);

        if(pt != 0)
        {
          delete pt;
          rWord.addr = 0;
        }
      }

      static void* Cast(void* pVoid)
      {
        return new(pVoid)CLine;
      }

      static Word Clone(const ListExpr typeInfo, const Word& rWord)
      {
        Word word;
        CLine* pt = static_cast<CLine*>(rWord.addr);

        if(pt != 0)
        {
          word.addr = new CLine(*pt);
          assert(word.addr != 0);
        }

        return word;
      }

      virtual Attribute* Clone() const
      {
        Attribute* pAttribute = new CLine(*this);
        assert(pAttribute != 0);

        return pAttribute;
      }


      static int SizeOfObj() { return sizeof(CLine); }

  private:
    bool def;
    int level;
    DbArray<SimpleSegment> segments; 
    Rectangle<2> bbox;

  };

}

#endif /* #define GIS_CLINE_H */

