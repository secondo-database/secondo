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

class MPrecHalfSegment;
ostream& operator<<(ostream& os, const MPrecHalfSegment& hs);

class MPrecHalfSegment{

  public:
     MPrecHalfSegment(const PPrecHalfSegment& phs, 
                      const DbArray<uint32_t>* fracStorage,
                      uint32_t _scale):
      ldp(phs.ldp), lp(phs.lp,fracStorage,_scale), 
      rp(phs.rp,fracStorage, _scale),
      attributes(phs.attributes) {
       assert(_scale>0);
       correctOrder();
      }

     MPrecHalfSegment(const MPrecHalfSegment& src):
        ldp(src.ldp), lp(src.lp), rp(src.rp), attributes(src.attributes) {}

     MPrecHalfSegment(const MPrecPoint& _lp, const MPrecPoint _rp, 
                      const bool _ldp, AttrType _attributes):
        ldp(_ldp), lp(_lp), rp(_rp), attributes(_attributes) {
        if(lp.getScale()!=rp.getScale()){
           rp.changeScaleTo(lp.getScale());
        }
        correctOrder();
     }
                      


     MPrecHalfSegment& operator=(const MPrecHalfSegment& src){
        ldp = src.ldp;
        lp = src.lp;
        rp = src.rp;
        attributes = src.attributes;
        return *this;
     }
/*
Check for exact equality of the halfsegment's geometry.
The dominating points as well as the attributes are not checked.

*/
    bool operator==(const MPrecHalfSegment& hs) const{
       return ldp==hs.ldp && lp==hs.lp && rp==hs.rp;
    }

/*
Checks whether this halfsegment and the argument have the same geometry,
i.e. if lp and rp are equal.

*/

    bool sameGeometry(const MPrecHalfSegment& hs) const{
       return (lp==hs.lp) && (rp==hs.rp);
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

    void setLDP(const bool _ldp){
        ldp = _ldp;
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
   
    bool boxIntersects(const MPrecHalfSegment& hs) const{
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
    bool overlaps(const MPrecHalfSegment& hs) const{
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
    bool crosses(const MPrecHalfSegment& hs) const{
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
    MPrecPoint* crossPoint(const MPrecHalfSegment& hs) const{
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

/*
~checkRealm~

This function checks whether this halfsegment and ~hs~ have at most 
end points in common.

*/
    bool checkRealm(const MPrecHalfSegment& hs) const{
       if(!boxIntersects(hs)){
          return true;
       }

       if(isVertical()) {
          if(hs.isVertical()){
             MPrecCoordinate y1 = hs.lp.getY();
             MPrecCoordinate y2 = hs.rp.getY();
             return ( y1 >= rp.getY() || y2 <= lp.getY());   
          } else {
             MPrecCoordinate y = hs.getY(lp.getX());
             if(y<lp.getY() || y>rp.getY()){
                return true;
             }
             if(y>lp.getY() && y<rp.getY()){
                return false;
             }
             // y is one of the endpoints of this segment
             return (lp.getX()==hs.lp.getX() || lp.getX()==hs.rp.getX());
          }
       } else if(hs.isVertical()){
           MPrecCoordinate y = getY(hs.lp.getX());
           if(y<hs.lp.getY() || y>hs.rp.getY()){
              return true;
           } 
           if(y>hs.lp.getX() && y<hs.rp.getY()){
              return false;
           }
           return (lp.getX() == hs.lp.getX() || rp.getX() == hs.lp.getX());
       }
       // both segments are non-vertical
       
       if(lp==hs.lp && rp==hs.rp){ // equal halfsegments
            return false;
       }  
       if(innerContains(hs.lp) || innerContains(hs.rp)){
          return false;
       } 
       if(hs.innerContains(lp) || hs.innerContains(rp)){
          return false;
       }
       return !crosses(hs);

    }

/*
~innerContains~

Checks whether ~p~ is located in the interior of this halfsegment.

*/ 
   bool innerContains(const MPrecPoint& p)const{
       if(isVertical()){
          return lp.getX() == p.getX() &&
                 p.getY()>lp.getY() && p.getY() < rp.getY();
       }
       MPrecCoordinate dx = (p.getX() - lp.getX()) / (rp.getX() - lp.getX());
       if(dx <= 0 || dx>=1){
           return false;
       }
       MPrecCoordinate y = lp.getY() + dx*(rp.getY()-lp.getY());
       return y == p.getY();
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

/*
~getY~

This function returns the y value for the line defined by this segment 
for a given x-coordinate. If the segment is vertical and the given x value corresponds 
to the x value of this segment. the smallest y -value of this segment is returned.
If the x-values differ for a vertical segment, an exception is thrown.

*/    
    MPrecCoordinate getY(const MPrecCoordinate& x) const{
      if(isVertical()){
          if(x==lp.getX()){
             return lp.getY();
          } else {
             throw 1;
          }
      }
      // non vertical segment
      MPrecCoordinate delta = (x - lp.getX()) / (rp.getX() - lp.getX());
      MPrecCoordinate y = lp.getY() + delta * (rp.getY() - lp.getY());
      return y;
    }


    int getScale() const{
       return lp.getScale();
    }

/**
  Debugging function. not for normal use 

*/    
  void getFractionals(void** result ) const{
     lp.getFractionals(result);
     rp.getFractionals(result+2);
  }

  bool checkScales() const{
     return     lp.checkScale() 
             && rp.checkScale()
             && (lp.getScale()==rp.getScale());
  }


     
  private:
     bool ldp;
     MPrecPoint lp;
     MPrecPoint rp;
  public:
     AttrType attributes;
  private: 
  

    MPrecHalfSegment():ldp(true),lp(0,0),rp(1,1), attributes(1) {}


    void correctOrder(){
       assert(lp!=rp);

       if(rp < lp){
         MPrecPoint tmp(lp);
         lp = rp;
         rp = tmp;
       }

    }

};



class HalfSegmentComparator{
  public:

  HalfSegmentComparator():print(false){}

  int operator()(const MPrecHalfSegment& hs1, const MPrecHalfSegment& hs2){

      //cout << "compare " << hs1 << endl
      //     << "with    " << hs2 << endl;

      MPrecPoint dp1 = hs1.getDomPoint();

      if(print){
        cout << "comapre halfsegments, dp1 = " << dp1 << endl; 
      }

      MPrecPoint dp2 = hs2.getDomPoint();

      if(print){
        cout << "comapre halfsegments, dp2 = " << dp2 << endl; 
      }


      int cmp = dp1.compareTo(dp2);
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

  bool print; 


};


class LogicalHalfSegmentComparator{
  public:
     int operator()(const MPrecHalfSegment& hs1, const MPrecHalfSegment& hs2){
        if(hs1.attributes.faceno < hs2.attributes.faceno) return -1;
        if(hs1.attributes.faceno > hs2.attributes.faceno) return 1;
        if(hs1.attributes.cycleno < hs2.attributes.cycleno) return -1;
        if(hs1.attributes.cycleno > hs2.attributes.cycleno) return 1;
        if(hs1.attributes.edgeno < hs2.attributes.edgeno) return -1;
        if(hs1.attributes.edgeno > hs2.attributes.edgeno) return 1;
        return 0;
     }
};


template<class Obj, class Comp>
class IsSmaller{

  public:
     bool operator()(const Obj& o1, const Obj& o2){
        return comp(o1,o2)<0;
     }

  private:
     Comp comp;

};




#endif
