
/*
----
This file is part of SECONDO.

Copyright (C) 2013,
Faculty of Mathematics and Computer Science,
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

1 Includes and global variables

*/


#include "PrecSecTypes.h"
#include "SpatialAlgebra.h"

#include "StringUtils.h"

#include "HsTools.h"
#include "PrecTools.h"

class MPrecPointComp{
  public:
     bool operator()(const MPrecPoint& p1, const MPrecPoint& p2){
        return p1.compareTo(p2) < 0;
    }
};


class LogicCompare{
   public:
      bool operator()(const MPrecHalfSegment& hs1, const MPrecHalfSegment& hs2){
         AttrType a1 = hs1.attributes;
         AttrType a2 = hs2.attributes;
         if(a1.faceno < a2.faceno) return true;
         if(a1.faceno > a2.faceno) return false;
         if(a1.cycleno < a2.cycleno) return true;
         if(a1.cycleno > a2.cycleno) return false;
         if(a1.edgeno < a2.edgeno) return true;
         if(a1.edgeno > a2.edgeno) return false;
         return false; 
      }    
};

/*
2 Auxiliary functions


*/

ListExpr toListExpr(const MPrecCoordinate& x){
   if(!x.hasFracPart()){
      return listutils::getInt64List(x.getGridCoord());
   }
   return nl->TwoElemList(
                listutils::getInt64List(x.getGridCoord()),
                nl->TextAtom(x.getFracAsText()));
}


ListExpr toListExpr(const MPrecPoint& p){
   return nl->TwoElemList( toListExpr(p.getX()), 
                           toListExpr(p.getY()));
}


ListExpr toListExpr(const MPrecHalfSegment& hs){
   return nl->TwoElemList( toListExpr(hs.getLeftPoint()),
                           toListExpr(hs.getRightPoint()));

}



bool readFrom(ListExpr le, MPrecCoordinate& result, bool includeScale, 
              const uint32_t _scale){
    int64_t intPart=0;

    uint32_t scale = _scale;
    if(includeScale){
      if(!nl->HasLength(le,2)){
         return false;
      }
      ListExpr sl = nl->First(le);
      le = nl->Rest(le);
      if(nl->AtomType(sl)!=IntType){
         return false;
      }
      scale = nl->IntValue(sl);
      if(scale <=0){
         return false;
      }
    }


    if(nl->AtomType(le)==IntType){
       result = MPrecCoordinate(nl->IntValue(le), scale);
       return true; 
    }

    if(nl->HasLength(le,1)){
       if(!listutils::decodeInt64(nl->First(le),intPart)){
          return false;
       }  
       result = MPrecCoordinate(intPart, scale);
       return true;
    }

    if(!nl->HasLength(le,2)){
       return false;
    }
    if(!listutils::decodeInt64(nl->First(le),intPart)){
       return false;
    }
    if(nl->AtomType(nl->Second(le)) != TextType){
       return false;
    }
    try{
      result.set(intPart, nl->Text2String(nl->Second(le)), scale);
      return true;
    } catch(...){
       return  false;
    } 
}

bool readPoint(ListExpr le, MPrecPoint& result, 
               bool includeScale, const uint32_t _scale){


  uint32_t scale = _scale;
  if(includeScale){
     if(!nl->HasLength(le,2)){
       return false;
     }
     ListExpr sl = nl->First(le);
     le = nl->Second(le);
     if(nl->AtomType(sl)!=IntType){
       return false;
     }
     scale = nl->IntValue(sl);
     if(scale <=0){
         return false;
     }
  }

  if(!nl->HasLength(le,2)){
    return false;
  }
  MPrecCoordinate x(0);
  MPrecCoordinate y(0);
  if(!readFrom(nl->First(le),x,false,scale) || 
     !readFrom(nl->Second(le),y,false,scale)){
     return  false;
  }
  result.set(x,y);
  return true;
}


void enlarge(Rectangle<2>& box, const MPrecPoint& p){
   double x = p.getX().toDouble();
   double y = p.getY().toDouble();
   if(!box.IsDefined()){
     double min[] = {x,y};
     double max[] = {x,y};
     box.Set(true,min,max);
     return;
   } 
   double MIN[] = {min(x,box.MinD(0)), min(y,box.MinD(1))};
   double MAX[] = {max(x,box.MaxD(0)), max(y,box.MaxD(1))};
   box.Set(true,MIN,MAX);
}


double getQDistance(const Rectangle<2>& rect,
                    const MPrecPoint& p    )  {
    double x = p.getX().toDouble();
    double y = p.getY().toDouble();
    double x1 = rect.MinD(0);
    double x2 = rect.MaxD(0);      
    double y1 = rect.MinD(0);
    double y2 = rect.MaxD(0);
    if(x<x1){
       if(y<y1){
          return ( (x1-x)*(x1-x) + (y1-y)*(y1-y));
       }
       if(y>y2){
          return ( (x1-x)*(x1-x) + (y2-y)*(y2-y));
       }
       return (x1-x) * (x1-x);
    } 
    
    if(x>x2){
       if(y<y1){
          return ( (x2-x)*(x2-x) + (y1-y)*(y1-y));
       }
       if(y>y2){
          return ( (x2-x)*(x2-x) + (y2-y)*(y2-y));
       }
       return (x-x2)*(x-x2);
    }
    if(y<y1){
      return (y1-y)*(y1-y);
    }
    if(y>y2){
       return (y - y2)*(y-y2);
    }
    return 0;
} 

bool intersects(const Rectangle<2>& rect, 
                const MPrecPoint& p){

  double x1 = rect.MinD(0);
  double y1 = rect.MinD(1);

  MPrecCoordinate x = p.getX();
  MPrecCoordinate y = p.getY();

  if(x<x1 || y < y1){
     return false;
  }
  double x2 = rect.MaxD(0);
  double y2 = rect.MaxD(1);
  if(x>x2 || y > y2){
      return false;
  }
  return true;

}



ListExpr getCycleList(const vector<MPrecHalfSegment>& v, size_t& pos){

  assert(pos < v.size()-2); // we need at leat 3 segments to build a cycle

  MPrecHalfSegment hs1 = v[pos];
  int ffaceno = hs1.attributes.faceno;  // facno of first segment
  int fcycleno = hs1.attributes.cycleno; // facno of second segment
  MPrecHalfSegment hs2 = v[pos+1];

  assert(ffaceno == hs2.attributes.faceno);
  assert(fcycleno == hs2.attributes.cycleno);

  MPrecPoint p1l = hs1.getLeftPoint();
  MPrecPoint p1r = hs1.getRightPoint();
  MPrecPoint p2l = hs2.getLeftPoint();
  MPrecPoint p2r = hs2.getRightPoint();

  ListExpr cycle;
  ListExpr last;
  MPrecPoint lastPoint(0,0);

  if(p1l==p2l){
     cycle = nl->OneElemList( toListExpr(p1r));
     last = cycle;
     last = nl->Append( last, toListExpr(p1l));
     last = nl->Append( last, toListExpr(p2r));
     lastPoint = p2r;
  } else if(p1l == p2r){
     cycle = nl->OneElemList( toListExpr(p1r));
     last = cycle;
     last = nl->Append( last, toListExpr(p1l));
     last = nl->Append( last, toListExpr(p2l));
     lastPoint = p2l;
  } else if(p1r == p2l){
     cycle = nl->OneElemList( toListExpr(p1l));
     last = cycle;
     last = nl->Append( last, toListExpr(p1r));
     last = nl->Append( last, toListExpr(p2r));
     lastPoint = p2r;
  } else if(p1r==p2r){
     cycle = nl->OneElemList( toListExpr(p1l));
     last = cycle;
     last = nl->Append( last, toListExpr(p1r));
     last = nl->Append( last, toListExpr(p2l));
     lastPoint = p2l;
  }

  pos = pos + 2;
  hs1 = v[pos];

  while( pos < v.size() && hs1.attributes.cycleno == fcycleno &&
                           hs1.attributes.faceno==ffaceno){
     p1l = hs1.getLeftPoint();
     p1r = hs1.getRightPoint();
     if(p1l==lastPoint){
        lastPoint = p1r;
     } else {
        assert(p1r == lastPoint);
        lastPoint = p1l; 
     }
     last = nl->Append(last, toListExpr(lastPoint));
     pos++;
     if(pos<v.size()){
       hs1 = v[pos];
     }
  }
  return cycle;
}

ListExpr getFaceList(const vector<MPrecHalfSegment>& v, size_t& pos){
  assert(pos < v.size());
  MPrecHalfSegment hs = v[pos];
  int  ffaceno = hs.attributes.faceno;
  bool first = true;
  int faceno = ffaceno;
  ListExpr cycles;
  ListExpr last;
  

  while(pos < v.size() &&   faceno==ffaceno){
     ListExpr cycle = getCycleList(v,pos);
     if(first){
         cycles = nl->OneElemList(cycle);
         last = cycles;
         first = false;
     } else {
         last = nl->Append(last,cycle);
     }
     if(pos<v.size()){
          faceno = v[pos].attributes.faceno;
     }
  }
  return cycles;
}

ListExpr getRegionList(vector<MPrecHalfSegment>& v){
   if(v.empty()){
      return nl->TheEmptyList();
   }
   LogicCompare cmp;
   sort(v.begin(), v.end(), cmp);

   ListExpr faces;
   ListExpr last;
   size_t pos = 0;
   bool first = true;
   while(pos < v.size()){
     ListExpr fl = getFaceList(v,pos);
     if(first){
       faces = nl->OneElemList(fl);
       last = faces;
       first = false;
     } else {
       last = nl->Append(last,fl);
     }
   }   
   return faces;
}

/*
~getCoord~

converts a double into aMPrecCoordinate

*/
MPrecCoordinate getCoord(double d, int scale, bool useStr){
  if(!useStr){
     MPrecCoordinate res(d,scale);
     res *= scale;
     return res;
  }
  MPrecCoordinate res(0);

  res.readFromString(stringutils::double2str(d, 16),scale);
  return res;
}


/*
~getPrecPoint~

Converts a point into a precise point

*/
MPrecPoint getPrecPoint( const Point& p, int scale, bool useStr){
   assert(scale>0);
   MPrecCoordinate x = getCoord(p.GetX(),scale,useStr);
   MPrecCoordinate y = getCoord(p.GetY(), scale,useStr);
   MPrecPoint res(x,y);
   return res;
    
}

/*
~getMPrecHsExact~

Converts a usual halfsegment into a precise one.

*/
MPrecHalfSegment getMPrecHsExact(const HalfSegment& hs, int scale){
   assert(scale>0);
   MPrecPoint lp( MPrecCoordinate(hs.GetLeftPoint().GetX(),scale),
                  MPrecCoordinate( hs.GetLeftPoint().GetY(), scale));
   MPrecPoint rp( MPrecCoordinate(hs.GetRightPoint().GetX(),scale),
                  MPrecCoordinate( hs.GetRightPoint().GetY(), scale));
   lp.compScale(scale);
   rp.compScale(scale);
   MPrecHalfSegment mhs(lp,rp,hs.IsLeftDomPoint(), hs.attr);
   return mhs;
}

/*
~getMPrecHs~

Converts a usual halfsegment into a precise one using the given precision.

*/
MPrecHalfSegment getMPrecHs(const HalfSegment& hs, 
                            int scale, uint32_t precision){
   assert(scale>0);

   MPrecCoordinate x(0);
   MPrecCoordinate y(0);
   
   if(!x.readFromString(stringutils::double2str(hs.GetLeftPoint().GetX(), 
                        precision),scale) ||
      !y.readFromString(stringutils::double2str( hs.GetLeftPoint().GetY(), 
                        precision), scale)){
      assert(false);
   }
   MPrecPoint lp(x,y);
   if(!x.readFromString(stringutils::double2str(hs.GetRightPoint().GetX(), 
                                     precision),scale) ||
      !y.readFromString(stringutils::double2str( hs.GetRightPoint().GetY(), 
                                     precision), scale)){
      assert(false);
   }
   MPrecPoint rp(x,y);
   MPrecHalfSegment mhs(lp,rp,hs.IsLeftDomPoint(), hs.attr);
   return mhs;
}


bool readHalfSegment(ListExpr le, MPrecHalfSegment& result, 
                     const bool includeScale, int scale){

  if(includeScale){
     if(!nl->HasLength(le,2)){
       return false;
     }
     ListExpr sc = nl->First(le);
     le = nl->Second(le);
     if(nl->AtomType(sc)!=IntType){
        return false;
     }
     scale = nl->IntValue(sc);
     if(scale<=0){
         return false;
     }
  }
  if(!nl->HasLength(le,2)){
    return false;
  }
  MPrecPoint p1(0,0);
  MPrecPoint p2(0,0);
  if(   !readPoint(nl->First(le),p1,false,scale) 
     || !readPoint(nl->Second(le),p2,false,scale)){
    return false;
  } 
  if(p1==p2){
     return false;
  }
  AttrType dummy(0);
  result = MPrecHalfSegment(p1,p2,true, dummy);
  return true;
}



/*
3 Implementation of complex class methods

3.1 Precise Coordinate

*/

 ListExpr PrecCoord::ToListExpr(ListExpr typeInfo) const{
        if(!IsDefined()){
             return listutils::getUndefined();
        }
        if(!coord.hasFracPart()){
           return nl->TwoElemList(
                      nl->IntAtom(scale),
                      nl->OneElemList(
                              listutils::getInt64List(coord.getIntPart())));
        }
        MPrecCoordinate pc(coord,&fracStorage, scale);
        return nl->TwoElemList(
                     nl->IntAtom(scale),
                       nl->TwoElemList( 
                         listutils::getInt64List(pc.getIntPart()),
                         nl->TextAtom(pc.getFracAsText())));
    }

bool PrecCoord::ReadFrom(ListExpr LE, const ListExpr typeInfo){

       if(listutils::isSymbolUndefined(LE)){
         SetDefined(false);
         fracStorage.clean();
         return true;
       }

       int64_t intPart=0;

       if(!nl->HasLength(LE,2)){
          return false;
       } 
       ListExpr sl = nl->First(LE);
       LE = nl->Second(LE);
       int sc = nl->IntValue(sl);
       if(sc<=0){
          return false;
       }


       if(nl->HasLength(LE,1)){
          if(!listutils::decodeInt64(nl->First(LE),intPart)){
              return false;
          }   
          coord = PPrecCoordinate(intPart,0);
          fracStorage.clean();
          scale = sc;
          return true;
       }

       if(!nl->HasLength(LE,2)){
           return false;
       }
       if(!listutils::decodeInt64(nl->First(LE),intPart)){
          return false;
       }
       if(nl->AtomType(nl->Second(LE)) != TextType){
          return false;
       }

       try{
         MPrecCoordinate pc(intPart, nl->Text2String(nl->Second(LE)),1);
         pc.appendFractional(&fracStorage);
         coord = pc;
         SetDefined(true);
         scale = sc;
         return true;
       } catch(...){
          return  false;
       } 
    }


/*
3.2 Precise Point

*/
 bool PrecPoint::Intersects(const Rectangle<2>& rect,
                            const Geoid* geoid/*=0*/ ) const {
       if(!IsDefined()){
          return false;
       }

       CType p = GetValue();
       double x = p.getX().toDouble();
       double y = p.getY().toDouble();
       double x1 = rect.MinD(0);
       double x2 = rect.MaxD(0);      
       double y1 = rect.MinD(0);
       double y2 = rect.MaxD(0);
       return x>=x1 && x<=x2 && y>=y1 && y<=y2; 
    }


bool PrecPoint::ReadFrom(ListExpr LE, ListExpr typeInfo){

        if(listutils::isSymbolUndefined(LE)){
           SetDefined(false);
           fracStorage.clean();
           return true;
        }
        if(!nl->HasLength(LE,2)){
           return false;
        }

        ListExpr sl = nl->First(LE);
        LE = nl->Second(LE);
        if(nl->AtomType(sl)!=IntType){
          return false;
        }

        int32_t sc = nl->IntValue(sl);
        if(sc <= 0){
           return false;
        }
        scale = sc;

        if(!nl->HasLength(LE,2)){
           return false;
        }

        MPrecCoordinate x(0);
        MPrecCoordinate y(0);
        if(!::readFrom(nl->First(LE),x,false, scale)){
           return false;
        }

        if(!::readFrom(nl->Second(LE),y,false, scale)){
           return false;
        }

        fracStorage.clean();
        x.appendFractional(&fracStorage);
        y.appendFractional(&fracStorage); 
        pos.set(x,y);

        SetDefined(true);
        return true;
     }

     void PrecPoint::readFrom(const Point& p, int scale, bool useStr){
         if(!p.IsDefined() || scale<=0){
            SetDefined(false);
            return;
         }
         MPrecPoint pp = getPrecPoint(p,scale,useStr);
         set(pp);
     }


/*
3.3 PrecPoints

*/

ListExpr PrecPoints::ToListExpr (ListExpr typeInfo){
         if(!IsDefined()){
            return listutils::getUndefined();
         }

         if(gridPoints.Size()==0){
            return nl->TheEmptyList();
         }

         ListExpr result = nl->OneElemList(
             toListExpr(getPointAt(0))
         );
         ListExpr last = result;
         for(size_t i=1;i<Size();i++){
           last = nl->Append(last, toListExpr(getPointAt(i)));
         }  
         return nl->TwoElemList( nl->IntAtom(scale), result);
     }


int PrecPoints::compareTo(const PrecPoints& rhs)const{
        if(!IsDefined()){
           return rhs.IsDefined()?-1:0;
        }
        if(!rhs.IsDefined()){
            return 1;
        }
        size_t ms = min(Size(), rhs.Size());
        for(size_t i=0;i<ms;i++){
           int cmp = getPointAt(i).compareTo(rhs.getPointAt(i));
           if(cmp!=0) {
              return cmp;
           }
        }
        if(Size()>ms){
           return 1;
        }
        if(rhs.Size()>ms){
           return -1;
        }
        return 0;
     }
 

std::ostream& PrecPoints::Print(std::ostream &os) const{
        if(!IsDefined()){
            os << "undef";
            return os;
        };
        os << "(";
        for(size_t i=0;i<Size();i++){
          if(i>0) os << ", ";
          os << getPointAt(i);
        }
        os << ")";
        return os;
     }


 double PrecPoints::Distance(const Rectangle<2>& rect,
                            const Geoid* geoid/*=0*/) const {
       assert(geoid==0);
       if(!IsDefined()){
          return -1;
       } 
       if(Size()==0){
          return -1;
       }
       double dist = getQDistance(rect,getPointAt(0));
       for(size_t i=1;i<Size() && dist > 0;i++){
          dist = min(dist, getQDistance(rect,getPointAt(i)));
       }
       return sqrt(dist);
    } 


 bool PrecPoints::Intersects(const Rectangle<2>& rect,
                            const Geoid* geoid/*=0*/ ) const {
       if(!IsDefined()){
          return false;
       }
       if(!bbox.Intersects(rect)){
         return false;
       }
       for(size_t i=0;i<Size();i++){
          if(::intersects(rect, getPointAt(i))){
              return true; 
          }
       }
       return false;
    }



bool PrecPoints::ReadFrom(ListExpr LE, ListExpr typeInfo){
        if(listutils::isSymbolUndefined(LE)){
           SetDefined(false);
           clear();
           return true;
        }
        uint32_t sc=1;
        if(!nl->HasLength(LE,2)){
          return false;
        }
        ListExpr sl = nl->First(LE);
        LE = nl->Second(LE);
        if(nl->AtomType(sl)!=IntType){
          return false;
        }

        int32_t a = nl->IntValue(sl);
        if(a<=0){
           return  false;
        }
        sc = a;

        if(nl->AtomType(LE)!=NoAtom){
           return false;
        }
        clear();

        startBulkLoad(sc);
        while(!nl->IsEmpty(LE)){
           ListExpr first = nl->First(LE);
           LE = nl->Rest(LE);
           MPrecPoint p(0,0);
           if(!readPoint(first, p,false,sc)){
              clear();
              endBulkLoad(false);
              return false;
           }
           append(p);
        }
        endBulkLoad(true);
        SetDefined(true);
        return true;
     }


void PrecPoints::endBulkLoad(bool sort/*=true*/){
        assert(bulkloadStorage);
        if(sort){
           MPrecPointComp cmp; 
           std::sort(bulkloadStorage->begin(), bulkloadStorage->end(), cmp); 
        }
        
        // make vector persistent
        bbox.SetDefined(false);
        if(bulkloadStorage->size() == 0){
           delete bulkloadStorage;
           bulkloadStorage = 0;
           return;
        } 
        MPrecPoint lp(0,0);
        for(size_t i=0;i<bulkloadStorage->size();i++){
            MPrecPoint cp = bulkloadStorage->at(i);
            if(i==0 || lp!=cp){
               cp.appendFractional(&fracStorage);
               PPrecPoint pcp = cp.getPersistent();
               gridPoints.Append(pcp);
               enlarge(bbox,cp);  
               lp = cp;
            }
        } 
        delete bulkloadStorage;
        bulkloadStorage = 0;
        SetDefined(true);
     }


void PrecPoints::compScale(const MPrecCoordinate& s1, 
                const MPrecCoordinate& s2,
                 PrecPoints& result) const{

         result.clear();
         if(!IsDefined()){
           result.SetDefined(false);
           return;
         }
         result.startBulkLoad(scale);
         for(size_t i=0;i<Size();i++){
           MPrecPoint p = getPointAt(i);
           p.compScale(s1,s2);
           result.append(p);
         }
         result.endBulkLoad(false);
     }


void PrecPoints::compTranslate(const MPrecCoordinate& t1, 
                    const MPrecCoordinate& t2,
                    PrecPoints& result) const{
         result.clear();
         if(!IsDefined()){
           result.SetDefined(false);
           return;
         }
         result.startBulkLoad( scale);
         for(size_t i=0;i<Size();i++){
           MPrecPoint p = getPointAt(i);
           p.compTranslate(t1,t2);
           result.append(p);
         }
         result.endBulkLoad(false);
     }


void PrecPoints::contains(const PrecPoints& ps, CcBool& result) const{
        if(!IsDefined() || !ps.IsDefined()){
            result.SetDefined(false);
            return;
        }
        if(ps.Size()==0){ // the empty set is part of each set
           result.Set(true,true);
           return; 
        }
        if(Size() < ps.Size()){
           result.Set(true,false);
           return;
        }
        size_t pos1=0; // position here
        size_t pos2=0; // position in ps
        while(pos1<Size() && pos2<ps.Size()){
           MPrecPoint p1 =  getPointAt(pos1);
           MPrecPoint p2 = ps.getPointAt(pos2);
           int cmp = p1.compareTo(p2);
           if(cmp<0){
              pos1++;
           } else if (cmp==0){
              pos1++;
              pos2++;
           } else {
              result.Set(true,false);
              return; 
           }
        }
        result.Set(true, pos2==ps.Size()); 
     }



/*
~contains~

Checks whether p is contained in this set.

*/

void PrecPoints::contains(const PrecPoint& p, CcBool& result) const{
        if(!IsDefined() || !p.IsDefined()){
           result.SetDefined(false);
           return;
        }
        if(Size()==0){
            result.Set(true,false);
            return;
        } 
        MPrecPoint mp = p.GetValue();
        size_t min = 0;
        size_t max = Size();
        while(min<max){
           size_t mid = (min + max) / 2;
           MPrecPoint tp = getPointAt(mid);
           int cmp = mp.compareTo(tp);
           if(cmp==0){
              result.Set(true,true);
              return;
           } 
           if(cmp < 0){
               max = mid - 1;
           } else {
               min = mid + 1;
           }
        }
        if(min==max){
           MPrecPoint tp = getPointAt(min);
           int cmp = mp.compareTo(tp);
           if(cmp==0){
              result.Set(true,true);
              return;
           } 
        } 
        result.Set(true,false);
     }


 void PrecPoints::intersects(const PrecPoints& ps, CcBool& result)const{
        if(!IsDefined() || !ps.IsDefined()){
           result.SetDefined(false);
           return;
        }
        if(Size()==0 || ps.Size()==0){
           result.Set(true,false);
           return;
        }
        size_t pos1 = 0;
        size_t pos2 = 0;
        while( pos1<Size() && pos2<ps.Size()){
            MPrecPoint p1 = getPointAt(pos1);
            MPrecPoint p2 = ps.getPointAt(pos2);
            int cmp = p1.compareTo(p2);
            if(cmp<0){
               pos1++;
            } else if(cmp>0){
               pos2++;
            } else {
                result.Set(true,true);
                return;
            }
        } 
        result.Set(true,false);
     }



 void PrecPoints::intersection(const PrecPoints& ps, PrecPoints& result) const{
        result.clear();
        if(!IsDefined() || !ps.IsDefined()){
           result.SetDefined(false);
           return;
        }
        result.startBulkLoad( scale);
        size_t pos1 = 0;
        size_t pos2 = 0;
        while( pos1<Size() && pos2<ps.Size()){
            MPrecPoint p1 = getPointAt(pos1);
            MPrecPoint p2 = ps.getPointAt(pos2);
            int cmp = p1.compareTo(p2);
            if(cmp<0){
               pos1++;
            } else if(cmp>0){
               pos2++;
            } else {
               result.append(p1);
               pos1++;
               pos2++;
            }
        } 
        result.endBulkLoad(false);
    }


/*
~union~

compute the union of this and ps

*/    
void PrecPoints::compUnion(const PrecPoints& ps, PrecPoints& result)const{
        result.clear();
        if(!IsDefined() || !ps.IsDefined()){
           result.SetDefined(false);
           return;
        }

        result.startBulkLoad(scale);

        size_t pos1 = 0;
        size_t pos2 = 0;
        while( pos1<Size() && pos2<ps.Size()){
            MPrecPoint p1 = getPointAt(pos1);
            MPrecPoint p2 = ps.getPointAt(pos2);
            int cmp = p1.compareTo(p2);
            if(cmp<0){
               result.append(p1);
               pos1++;
            } else if(cmp>0){
               result.append(p2);
               pos2++;
            } else {
               result.append(p1);
               pos1++;
               pos2++;
            }
        } 
        while(pos1<Size()){
           MPrecPoint p1 = getPointAt(pos1);
           result.append(p1);
           pos1++;
        }
        while(pos2<ps.Size()){
           MPrecPoint p2 = ps.getPointAt(pos2);
           result.append(p2);
           pos2++;
        }
        result.endBulkLoad(false);
    }



/*
~difference~

compute the difference of this and ps

*/    
void PrecPoints::difference(const PrecPoints& ps, PrecPoints& result)const{
        result.clear();
        if(!IsDefined() || !ps.IsDefined()){
           result.SetDefined(false);
           return;
        }
        result.startBulkLoad(scale);
        size_t pos1 = 0;
        size_t pos2 = 0;
        while( pos1<Size() && pos2<ps.Size()){
            MPrecPoint p1 = getPointAt(pos1);
            MPrecPoint p2 = ps.getPointAt(pos2);
            int cmp = p1.compareTo(p2);
            if(cmp<0){
               result.append(p1);
               pos1++;
            } else if(cmp>0){
               pos2++;
            } else {
               pos1++;
               pos2++;
            }
        } 
        while(pos1<Size()){
           MPrecPoint p1 = getPointAt(pos1);
           result.append(p1);
           pos1++;
        }
        result.endBulkLoad(false);
    }

/*
~readFrom~

Converts the argument into a precise value

*/
void PrecPoints::readFrom(const Points& points, int scale, bool useString){
    clear();
    if(!points.IsDefined() || scale<=0){
      SetDefined(false);
      return;
    }
    SetDefined(true);
    if(points.Size()==0){
      return;
    }
    startBulkLoad(scale);
    Point p;
    MPrecPoint pp(0,0);
    for(int i=0;i<points.Size();i++){
       points.Get(i,p);
       pp = getPrecPoint(p,scale,useString);
       append(pp);
    }
    endBulkLoad();
}
 

/*
3.3 precLine

*/

int PrecLine::compareTo(const PrecLine& rhs)const{
        if(!IsDefined()){
           return rhs.IsDefined()?-1:0;
        }
        if(!rhs.IsDefined()){
           return 1;
        }
        int cbb = bbox.Compare(&rhs.bbox);
        if(cbb!=0){
          return cbb;
        }
        if(Size() < rhs.Size()){
            return -1;
        }
        if(Size()> rhs.Size()){
            return 1;
        }
        for(size_t i =0; i< Size(); i++){
           MPrecHalfSegment hs1 = getHalfSegment(i);
           MPrecHalfSegment hs2 = rhs.getHalfSegment(i);
           int cmp = hs1.compareTo(hs2);
           if(cmp!=0){
              return cmp;
           }
        }
        return 0;
    }



ListExpr PrecLine::ToListExpr(ListExpr typeInfo) const{
      if(!IsDefined()){
         return listutils::getUndefined();
      }

       ListExpr hsList = nl->TheEmptyList();
       ListExpr last;
       bool first = true;
       for(size_t i=0;i<Size();i++){
          MPrecHalfSegment hs = getHalfSegment(i);
          if(hs.isLeftDomPoint()){
              if(first){
                  hsList = nl->OneElemList( toListExpr(hs));
                  last = hsList;
                  first = false;
              } else {
                 last = nl->Append(last, toListExpr(hs));
              }
          }
       }
       return nl->TwoElemList( nl->IntAtom(scale), 
                               hsList); 
    }



void PrecLine::readFrom(const Line& line, int scale, bool useString){
    if(!line.IsDefined() || scale<=0){
      clear();
      SetDefined(false);
      return;
    }
    vector<MPrecHalfSegment> hsv1;
    HalfSegment hs;

    for(int i=0;i<line.Size();i++){
       line.Get(i,hs);
       if(useString){
          hsv1.push_back(getMPrecHs(hs, scale, 16));
       } else {
          hsv1.push_back(getMPrecHsExact(hs, scale));
       }
    }
    vector<MPrecHalfSegment> hsv;

    

    hstools::realminize(hsv1,hsv);

    gridData.resize(hsv.size());
    for(size_t i=0;i<hsv.size(); i++){
       MPrecHalfSegment mhs = hsv[i];
       mhs.appendTo(&gridData, &fracStorage);
       enlarge(bbox, mhs.getLeftPoint());
       enlarge(bbox, mhs.getRightPoint()); 
    }
    this->scale = scale; 
    SetDefined(true);
}

bool PrecLine::ReadFrom(ListExpr value, ListExpr typeInfo){
   if(listutils::isSymbolUndefined(value)){
     clear();
     SetDefined(false);
     return true;
   }

   if(!nl->HasLength(value,2)){
      return false;
   }
   ListExpr sc = nl->First(value);
   value = nl->Second(value);
   if(nl->AtomType(sc)!=IntType ||
      nl->AtomType(value)!=NoAtom){
     return false;
   }
   int scale = nl->IntValue(sc);
   if(scale<=0){
      return false;
   }
   
   startBulkLoad();
   AttrType dummy(0);
   MPrecHalfSegment hs(MPrecPoint(0,0),MPrecPoint(1,0),true,dummy);
   while(!nl->IsEmpty(value)){
      ListExpr first = nl->First(value);
      value = nl->Rest(value);
      if(!readHalfSegment(first, hs,false,scale)){
          clear();
          SetDefined(false);
          delete bulkloadStorage;
          bulkloadStorage=0;
          return false;
      }
      append(hs);
   }
   endBulkLoad();
   this->scale = scale;
   SetDefined(true);
   return true;
}

void PrecLine::endBulkLoad(bool realminize){
   assert(bulkloadStorage);
   if(bulkloadStorage->empty()){
     delete bulkloadStorage;
     bulkloadStorage=0;
     scale = 1;
     return;
   }

   hstools::sort(*bulkloadStorage);
   vector<MPrecHalfSegment> v2;
   if(realminize){
      hstools::realminize(*bulkloadStorage,v2);
   } else {
      v2 = *bulkloadStorage;
   }

   hstools::setPartnerNumbers(v2);
   bbox.SetDefined(false);
   scale = v2[0].getScale(); 
   for(size_t i=0;i<v2.size();i++){
      v2[i].appendTo(&gridData, &fracStorage);
      enlarge(bbox, v2[i].getLeftPoint());
      enlarge(bbox, v2[i].getRightPoint());
   }
   delete bulkloadStorage;
   bulkloadStorage=0;
}


void PrecLine::compUnion(const PrecLine& l2, PrecLine& result) const{
  result.clear();
  if(!IsDefined() || !l2.IsDefined()){
    result.SetDefined(false);
    return;
  }
  result.SetDefined(true);
  result.startBulkLoad();
  for(size_t i=0;i<Size();i++){
     MPrecHalfSegment hs = getHalfSegment(i);
     if(hs.isLeftDomPoint()){
        result.append(hs);
     }
  }
  for(size_t i=0;i<l2.Size();i++){ 
     MPrecHalfSegment hs = l2.getHalfSegment(i);
     if(hs.isLeftDomPoint()){
       result.append(hs);
     }
  }
  bool realmrequired = true;
  if(Size()==0 || l2.Size()==0){
    realmrequired = false;
  } else {
    realmrequired = bbox.Intersects(l2.bbox);
  }
     
  result.endBulkLoad(realmrequired);
}


void PrecLine::intersection(const PrecLine& l2, PrecLine& result) const{
  result.clear();
  if(!IsDefined() || !l2.IsDefined()){
    result.SetDefined(false);
    return;
  }
  if(!bbox.Intersects(l2.bbox)){ // intersection will be empty
     return;
  }
  result.SetDefined(true);
  vector<MPrecHalfSegment> v1;
  vector<MPrecHalfSegment> v2;
  for(size_t i=0;i<Size();i++){
     MPrecHalfSegment hs = getHalfSegment(i);
     v1.push_back(hs);
  }
  for(size_t i=0;i<l2.Size();i++){ 
     MPrecHalfSegment hs = l2.getHalfSegment(i);
       v2.push_back(hs);
  }
  vector<MPrecHalfSegment> rv;
  hstools::setOP(v1,v2,rv,hstools::INTERSECTION);  
  result.startBulkLoad();
  for(size_t i=0;i< rv.size();i++){
     if(rv[i].isLeftDomPoint()){
        result.append(rv[i]);
     }
  }
  result.endBulkLoad(false);

}

void PrecLine::difference(const PrecLine& l2, PrecLine& result) const{
  result.clear();
  if(!IsDefined() || !l2.IsDefined()){
    result.SetDefined(false);
    return;
  }
  if(!bbox.Intersects(l2.bbox)){ // intersection will be empty
     return;
  }
  result.SetDefined(true);
  vector<MPrecHalfSegment> v1;
  vector<MPrecHalfSegment> v2;
  for(size_t i=0;i<Size();i++){
     MPrecHalfSegment hs = getHalfSegment(i);
     v1.push_back(hs);
  }
  for(size_t i=0;i<l2.Size();i++){ 
     MPrecHalfSegment hs = l2.getHalfSegment(i);
       v2.push_back(hs);
  }
  vector<MPrecHalfSegment> rv;
  hstools::setOP(v1,v2,rv,hstools::DIFFERENCE);  
  result.startBulkLoad();
  for(size_t i=0;i< rv.size();i++){
     if(rv[i].isLeftDomPoint()){
        result.append(rv[i]);
     }
  }
  result.endBulkLoad(false);

}


/*
3.4 precRegion

*/

int PrecRegion::compareTo(const PrecRegion& rhs)const{
        if(!IsDefined()){
           return rhs.IsDefined()?-1:0;
        }
        if(!rhs.IsDefined()){
           return 1;
        }
        int cbb = bbox.Compare(&rhs.bbox);
        if(cbb!=0){
          return cbb;
        }
        if(Size() < rhs.Size()){
            return -1;
        }
        if(Size()> rhs.Size()){
            return 1;
        }
        for(size_t i =0; i< Size(); i++){
           MPrecHalfSegment hs1 = getHalfSegment(i);
           MPrecHalfSegment hs2 = rhs.getHalfSegment(i);
           int cmp = hs1.compareTo(hs2);
           if(cmp!=0){
              return cmp;
           }
        }
        return 0;
    }



ListExpr PrecRegion::ToListExpr(ListExpr typeInfo) const{
      if(!IsDefined()){
         return listutils::getUndefined();
      }

       vector<MPrecHalfSegment> hsvec;
       for(size_t i=0;i<Size();i++){
          MPrecHalfSegment hs = getHalfSegment(i);
          if(hs.isLeftDomPoint()){
             hsvec.push_back(hs);
          }
       }
       return nl->TwoElemList( nl->IntAtom(scale), 
                               getRegionList(hsvec)); 
}

bool PrecRegion::endBulkLoad( bool sort,
                  bool setCoverageNo,
                  bool setPartnerNo,
                  bool computeRegion){

   assert(bulkloadStorage);
   if(sort){
     hstools::sort(*bulkloadStorage);
   }

   if(!hstools::checkRealm(*bulkloadStorage)){
     delete bulkloadStorage;
     bulkloadStorage=0;
     return false;
   }
   if(setCoverageNo){
     hstools::setCoverage(*bulkloadStorage);
   }
   if(setPartnerNo){
     hstools::setPartnerNumbers(*bulkloadStorage);
   }
   if(computeRegion){
     cerr << "computeRegion not implemented yet" << endl;
   }
   for(size_t i=0;i<bulkloadStorage->size();i++){
      (*bulkloadStorage)[i].appendTo(&gridData, &fracStorage);
      enlarge(bbox, (*bulkloadStorage)[i].getLeftPoint());
      enlarge(bbox, (*bulkloadStorage)[i].getRightPoint());
   }
   delete bulkloadStorage;
   bulkloadStorage = 0;
   return true; 
} 

void PrecRegion::cancelBulkLoad(){
  assert(bulkloadStorage);
  delete bulkloadStorage;
  bulkloadStorage=0;
}



bool PrecRegion::addCycle(ListExpr cycle, int faceNo, int cycleNo, int& edgeNo){
   if(nl->ListLength(cycle) < 3) {
     return false;
   }
   vector<MPrecPoint> points;
   MPrecPoint p(0,0);
   while(!nl->IsEmpty(cycle)){
     if(!readPoint(nl->First(cycle), p, false, scale)){
       return false;
     }
     cycle = nl->Rest(cycle);
     points.push_back(p);
   }
   // open cycle if closed 
   if(points[0]==points[points.size()-1]){
     points.pop_back();
   } 
   bool clockwise = precisetools::isClockwise(points);
   bool isRight = cycleNo==0?clockwise:!clockwise;
   // create halfsegments
 
   for(size_t i = 0;i<points.size();i++){
     MPrecPoint p1 = points[i];
     MPrecPoint p2 = points[ (i+1) % points.size() ];
     if(p1==p2){
       return false;
     }
     AttrType a;
     a.faceno = faceNo;
     a.cycleno = cycleNo;
     a.edgeno = edgeNo;
     if(p1.getX()==p2.getX()){
       a.insideAbove = p1.getY() < p2.getY()?!isRight:isRight;
     } else {
       a.insideAbove = p1.getX() < p2.getX()? !isRight:isRight;
     }
     MPrecHalfSegment hs1(p1,p2,true,a);
     append(hs1);
     edgeNo++;
   }
   return true;
    
}



bool PrecRegion::addFace(ListExpr face, 
        int faceNo, int& edgeNo ){

   int cycleNo = 0;
   if(nl->AtomType(face)!=NoAtom){
      return  false;
   }
   while(!nl->IsEmpty(face)){
      if(!addCycle(nl->First(face), faceNo, cycleNo, edgeNo)){
          return false;
      }
      face = nl->Rest(face);
   }
   return true;
} 


bool PrecRegion::ReadFrom(ListExpr value,  ListExpr type){
   clear();
   SetDefined(false);
   if(listutils::isSymbolUndefined(value)){
     return true;
   }

   if(!nl->HasLength(value,2)){
     return false;
   }
  
   ListExpr scale = nl->First(value);
   value = nl->Second(value);
   if(nl->AtomType(scale)!=IntType){
     return false;
   }
   int sc = nl->IntValue(scale);
   if(sc < 1){
     return false;
   }
   setScale(sc);


   if(nl->AtomType(value)!=NoAtom){
     return false;
   }

   SetDefined(true);
   // liststructure is
   // ( face_1, face_2 ,...)
   // face_i = ( cycle_1, cycle_2 ,...)
   // cycle_i = ( point_1, point_2 , ... )
   startBulkLoad();
   int faceNo = 0;
   int edgeNo = 0;
   while(!nl->IsEmpty(value)){
     if(!addFace(nl->First(value),faceNo, edgeNo )) {
        cancelBulkLoad();
        SetDefined(false);
        return false;
     }
     faceNo++;
     value = nl->Rest(value);
   }
   return endBulkLoad();
}



bool correctHs(vector<MPrecHalfSegment>& v){

  // check order
  if(! hstools::isSorted(v)){
    return false;
  }

  if(!hstools::checkRealm(v)){
     return false;
  }

  return true;
 
  // todo : implement this function
  // 1. if not ordered : reorder and recompute partnernumbers   
  // 2. Check whether cycles are connected 


}


void PrecRegion::readFrom(const Region& reg, int scale, bool useString){
    if(!reg.IsDefined() || scale<=0){
      clear();
      SetDefined(false);
      return;
    }
    vector<MPrecHalfSegment> hsv;
    HalfSegment hs;
    
    for(int i=0;i<reg.Size();i++){
       reg.Get(i,hs);
       if(useString){
          hsv.push_back(getMPrecHs(hs, scale, 16));
       } else {
          hsv.push_back(getMPrecHsExact(hs, scale));
       }
    }
    if(!correctHs(hsv)){
      clear();
      SetDefined(false);
      return;
    }
    gridData.resize(hsv.size());
    for(size_t i=0;i<hsv.size(); i++){
       MPrecHalfSegment mhs = hsv[i];
       mhs.appendTo(&gridData, &fracStorage);
       enlarge(bbox, mhs.getLeftPoint());
       enlarge(bbox, mhs.getRightPoint()); 
    }
    this->scale = scale; 
    SetDefined(true);
}


