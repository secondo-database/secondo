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

*/


#ifndef PRECISE_HALFSEGMENT
#define PRECISE_HALFSEGMENT

#include "PreciseCoordinate.h"

#include "PrecisePoint.h"
#include "AttrType.h"
#include "RectangleAlgebra.h"





/*
1 Implementation of a precise halfsegment


1.1 class ~PPrecHalfSegment~

This class represents the persistent part of a precise halfsegment.


*/

class PPrecHalfSegment{

  public:
    PPrecHalfSegment() {}
    PPrecHalfSegment(const PPrecHalfSegment& src):
         ldp(src.ldp), lp(src.lp), rp(src.rp), attributes(src.attributes) {}

    PPrecHalfSegment(const bool _ldp, const PPrecPoint _lp, 
                     const PPrecPoint _rp, 
                     const AttrType& _attributes):
          ldp(_ldp), lp(_lp), rp(_rp),attributes(_attributes) {}

    PPrecHalfSegment& operator=(const PPrecHalfSegment& src){
        ldp = src.ldp;
        lp = src.lp;
        rp = src.rp;
        attributes = src.attributes;
        return *this;
    }

    void switchOrder(){
       PPrecPoint tmp(lp);
       lp = rp;
       rp = tmp;
    }

    size_t getHash(){
       return lp.getHash() + rp.getHash();
    }
  
    bool ldp;
    PPrecPoint lp;
    PPrecPoint rp;
    AttrType attributes;

};



class MPrecHalfSegment{

  public:
     MPrecHalfSegment(const PPrecHalfSegment& phs, 
                      const DbArray<uint32_t>* fracStorage,
                      uint32_t _scale):
      ldp(phs.ldp), lp(phs.lp,fracStorage,_scale), 
      rp(phs.rp,fracStorage, _scale),
      attributes(phs.attributes) {
       correctOrder();
      }

     MPrecHalfSegment(const MPrecHalfSegment& src):
        ldp(src.ldp), lp(src.lp), rp(src.rp), attributes(src.attributes) {}

     MPrecHalfSegment(const MPrecPoint& _lp, const MPrecPoint _rp, 
                      const bool _ldp, AttrType _attributes):
        ldp(_ldp), lp(_lp), rp(_rp), attributes(_attributes) {}
                      


     MPrecHalfSegment& operator=(const MPrecHalfSegment& src){
        ldp = src.ldp;
        lp = src.lp;
        rp = src.rp;
        attributes = src.attributes;
        return *this;
     }

     MPrecPoint getLeftPoint() const{ return lp; }
     MPrecPoint getRightPoint() const{ return rp; }
     MPrecPoint getDomPoint() const {return ldp?lp:rp;}
     MPrecPoint getSecPoint() const {return ldp?rp:lp;}
     bool isLeftDomPoint() const { return ldp; }


     void appendTo(DbArray<PPrecHalfSegment>* gridStorage,
                   DbArray<uint32_t>* fracStorage){
          lp.appendFractional( fracStorage);
          rp.appendFractional( fracStorage);
          PPrecHalfSegment hs(ldp,lp,rp,attributes);
          gridStorage->Append(hs);
     }


     Rectangle<2> BoundingBox() const{
        double x0 = lp.getX().toDouble();
        double y0 = lp.getY().toDouble();
        double x1 = rp.getX().toDouble();
        double y1 = rp.getY().toDouble();
        double MIN[] = { min(x0,x1), min(y0,y1)};
        double MAX[] = { max(x0,x1), max(y0,y1)};
        Rectangle<2> res(true,MIN,MAX);
        return res;
     }


    MPrecCoordinate qLength(){
        return lp.QDistance(rp);
    }

    double length(){
       return sqrt(qLength().toDouble());
    }

    void set(const bool _ldp, const MPrecPoint& _lp, const MPrecPoint& _rp,
             const AttrType _attributes){
       ldp = _ldp;
       lp = _lp;
       rp = _rp;
       attributes = _attributes;
       correctOrder();
    }

    void compScale(const MPrecCoordinate& sx, const MPrecCoordinate& sy){
         lp.compScale(sx,sy);
         rp.compScale(sx,sy);
         assert(lp!=rp);
         correctOrder();
    }

    void compTranslate(const MPrecCoordinate& tx, const MPrecCoordinate& ty){
         lp.compTranslate(tx,ty);
         rp.compTranslate(tx,ty);
    }

    void switchLDP(){
        ldp = !ldp;
    }



/*
~contains~

Checks whether this halfsegment contains a certain point

*/
    bool contains(const MPrecPoint& p) const{
      MPrecCoordinate x = p.getX();
      MPrecCoordinate y = p.getY();
      MPrecCoordinate x1 = lp.getX();
      MPrecCoordinate x2 = rp.getX();
      MPrecCoordinate y1 = lp.getY();
      MPrecCoordinate y2 = rp.getY();

      // bounding box check (uses only comparision)

      if(( x1 < x2) && ( (x < x1) || (x > x2))){
         return false;
      }  
      if(( x1 > x2) && ( (x < x2) || (x > x1))){
         return false;
      }  
      if(( y1 < y2) && ( (y < y1) || (y > y2))){
         return false;
      }  
      if(( y1 > y2) && ( (y < y2) || (y > y1))){
         return false;
      }  
      
      // orthogonal segments
      // vertical
      if(x1==x2){
          if(x!=x1){
             return false;
          }
          if(y1<y2){
             return y1 <= y && y <= y2;
          }  else {
             return y2 <= y && y <= y1;
          }
      }

      // horizontal
      if(y1==y2){
         if(y!=y1){
            return false;
         }
         if(x1<x2){
            return x1 <= x && x <= x2;
         } else {
            return x2 <= x && x<= x2;
         }
      }
      // non orthogonal segment
      return (x-x1)*(y2-y1) == (y-y2)*(x2-x1);
    }
   
    bool boxIntersects(const MPrecHalfSegment& hs){
       MPrecCoordinate x1 = lp.getX();
       MPrecCoordinate y1 = lp.getY();
       MPrecCoordinate x2 = rp.getX();
       MPrecCoordinate y2 = rp.getY();

       MPrecCoordinate hsx1 = hs.lp.getX();
       MPrecCoordinate hsy1 = hs.lp.getY();
       MPrecCoordinate hsx2 = hs.rp.getX();
       MPrecCoordinate hsy2 = hs.rp.getY();

       if(hsx1 < x1 && hsx1 < x2 &&   // hs left of this
          hsx2 < x1 && hsx2 < x2){
          return false;
       }

       if(hsx1 > x1 && hsx1 > x2 &&   // hs right of this
          hsx2 > x1 && hsx2 > x2){
          return false;
       }

       if(hsy1 < y1 && hsy1 < y2 &&   // hs under this
          hsy2 < y1 && hsy2 < y2){
          return false;
       }

       if(hsy1 > y1 && hsy1 > y2 &&   // hs above this
          hsy2 > y1 && hsy2 > y2){
          return false;
       }
       return true;
    }


/*
~overlaps~

Checks whether this segment and hs have a common segment

*/
    bool overlaps(const MPrecHalfSegment& hs) {
        if(!boxIntersects(hs)){
          return false;
        }
        bool clp = contains(hs.lp);
        bool crp = contains(hs.rp);
        if(clp && crp){ // hs completely this segment
          return true;
        }
        bool lpc = hs.contains(lp);
        bool rpc = hs.contains(rp);
        if(lpc && rpc) { // this completely contained in hs
          return true;
        }
        if(clp && rpc) return true;
        if(lpc && crp) return true;
        return false; 
    } 

/*
~crosses~

This function checks whether the interior of this segment and the interior 
of hs have a common point.

*/
    bool crosses(const MPrecHalfSegment& hs){
      if(!boxIntersects(hs)){
         return false;
      }
      MPrecCoordinate x1 = lp.getX();
      MPrecCoordinate x2 = rp.getX();
      MPrecCoordinate y1 = lp.getY();
      MPrecCoordinate y2 = rp.getY();
      MPrecCoordinate hsx1 = hs.lp.getX();
      MPrecCoordinate hsx2 = hs.rp.getX();
      MPrecCoordinate hsy1 = hs.lp.getY();
      MPrecCoordinate hsy2 = hs.rp.getY();
      MPrecCoordinate y = hsy1-hsy2;
      MPrecCoordinate u = x2-x1;
      MPrecCoordinate v = hsx1-hsx2;
      MPrecCoordinate x = y2-y1;
      MPrecCoordinate k = y*u-v*x;
      if(k==0){  // segments are parallel
         return false;
      }   
      
      MPrecCoordinate w = x1-hsx1;
      MPrecCoordinate z = y1-hsy1;

      MPrecCoordinate delta2 = (w*x-z*u) / k;
      MPrecCoordinate delta1(0);
      if(abs(u) > abs(x)){
         delta1 = ((w+delta2*v)/u)*-1;
      } else {
         delta1 = ((z+delta2*y)/x)*-1;
      }   

      if(delta1<=0 || delta2 <= 0){ 
         return false;
      }   
      if(delta1>=1 || delta2 >= 1){ 
        return false;
      }  
      return true;    
    }


/*
~crossPoint~

Computes the intersectionPOint of the interiors of this and hs.
If no such intersection exists, 0 is returned

*/
    MPrecPoint* crossPoint(const MPrecHalfSegment& hs){
      if(!boxIntersects(hs)){
         return 0;
      }
      MPrecCoordinate x1 = lp.getX();
      MPrecCoordinate x2 = rp.getX();
      MPrecCoordinate y1 = lp.getY();
      MPrecCoordinate y2 = rp.getY();
      MPrecCoordinate hsx1 = hs.lp.getX();
      MPrecCoordinate hsx2 = hs.rp.getX();
      MPrecCoordinate hsy1 = hs.lp.getY();
      MPrecCoordinate hsy2 = hs.rp.getY();
      MPrecCoordinate y = hsy1-hsy2;
      MPrecCoordinate u = x2-x1;
      MPrecCoordinate v = hsx1-hsx2;
      MPrecCoordinate x = y2-y1;
      MPrecCoordinate k = y*u-v*x;
      if(k==0){  // segments are parallel
         return 0;
      }   
      
      MPrecCoordinate w = x1-hsx1;
      MPrecCoordinate z = y1-hsy1;

      MPrecCoordinate delta2 = (w*x-z*u) / k;
      MPrecCoordinate delta1(0);
      if(abs(u) > abs(x)){
         delta1 = ((w+delta2*v)/u)*-1;
      } else {
         delta1 = ((z+delta2*y)/x)*-1;
      }   

      if(delta1<0 || delta2 < 0){ 
         return 0;
      }   
      if(delta1>1 || delta2 > 1){ 
        return 0;
      }  
      MPrecCoordinate xp = x1 + delta1*(x2-x1);
      MPrecCoordinate yp = y1 + delta1*(y2-y1);
      return new MPrecPoint(xp,yp);    
    }

    int compareTo(const MPrecHalfSegment& rhs) const{
       int cmp = getDomPoint().compareTo(rhs.getDomPoint());
       if(cmp!=0) return cmp;
       return getSecPoint().compareTo(rhs.getSecPoint());
    }

    void appendFractional(DbArray<uint32_t>* fracStore){
        lp.appendFractional(fracStore);
        rp.appendFractional(fracStore);
    }

    bool isVertical() const{
       return lp.getX() == rp.getX();
    }

    int compareSlope(const MPrecHalfSegment& hs)const{
       if(isVertical()){
           if(hs.isVertical()){
             return 0;
           }
           bool hsright = hs.getDomPoint().getX() < hs.getSecPoint().getX();
           return hsright?-1:1;
       }
       if(hs.isVertical()){
           bool right = getDomPoint().getX() < getSecPoint().getX();
           return right?1:-1;
       }  
       MPrecCoordinate slope = (lp.getY()-rp.getY()) / (lp.getX() - rp.getX());
       MPrecCoordinate hsslope = (hs.lp.getY()-hs.rp.getY()) /
                                 (hs.lp.getX() - hs.rp.getX());
       return slope.compare(hsslope);
    }

     
  private:
     bool ldp;
     MPrecPoint lp;
     MPrecPoint rp;
  public:
     AttrType attributes;
  private: 
  


    void correctOrder(){
       assert(lp!=rp);

       if(lp < rp){
         MPrecPoint tmp(lp);
         lp = rp;
         rp = tmp;
       }

    }

};

ostream& operator<<(ostream& os, const MPrecHalfSegment& hs);


class HalfSegmentComparator{
  public:
  int operator()(const MPrecHalfSegment& hs1, const MPrecHalfSegment& hs2){
      int cmp = hs1.getDomPoint().compareTo(hs2.getDomPoint());
      if(cmp!=0) return cmp;
      if( hs1.isLeftDomPoint()  != hs2.isLeftDomPoint() ) {
           if( hs1.isLeftDomPoint() == false ){
              return -1;
           } else {
              return 1;
           }
      }
      // domination points are equal, compare slopes
      return hs1.compareSlope(hs2);
  }

};



#endif
