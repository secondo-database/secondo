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



enum OWNER{FIRST,SECOND,BOTH, NONE};

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

    size_t getHash() const{
       return lp.getHash() + rp.getHash();
    }

    bool getLeftDomPoint() const{ 
      return ldp;
    }

    const PPrecPoint& getLeftPoint() const {
      return lp;
    }
    
    const PPrecPoint& getRightPoint() const {
      return rp;
    }

    bool ldp;
    PPrecPoint lp;
    PPrecPoint rp;
    AttrType attributes;

};

class MPrecHalfSegment;
ostream& operator<<(ostream& os, const MPrecHalfSegment& hs);
ostream& operator<<(ostream& os, const PPrecHalfSegment& hs);

class MPrecHalfSegment{

  public:

     MPrecHalfSegment():ldp(true),lp(0,0),rp(1,1), attributes(1) {}

     MPrecHalfSegment(const PPrecHalfSegment& _phs, 
                      const DbArray<uint32_t>* _fracStorage,
                      uint32_t _scale, const OWNER _owner = FIRST,
                      bool orderOK = false):
      ldp(_phs.ldp), 
      lp(_phs.lp, _fracStorage, _scale), 
      rp(_phs.rp, _fracStorage, _scale),
      owner(_owner),
      attributes(_phs.attributes) {
       assert(_scale>0);
       if(!orderOK){
          correctOrder();
       }
      }

     MPrecHalfSegment(const MPrecHalfSegment& src):
        ldp(src.ldp), lp(src.lp), rp(src.rp), owner(src.owner),
        attributes(src.attributes) {}

     MPrecHalfSegment(const MPrecPoint& _lp, const MPrecPoint _rp, 
                      const bool _ldp, AttrType _attributes,
                      const OWNER _owner = FIRST):
        ldp(_ldp), lp(_lp), rp(_rp), owner(_owner),
        attributes(_attributes) {
        if(lp.getScale()!=rp.getScale()){
           rp.changeScaleTo(lp.getScale());
        }
        correctOrder();
     }
                      


     MPrecHalfSegment& operator=(const MPrecHalfSegment& src){
        ldp = src.ldp;
        lp = src.lp;
        rp = src.rp;
        owner = src.owner;
        attributes = src.attributes;
        return *this;
     }
/*
Check for exact equality of the halfsegment's geometry.
The attributes and the owners are not checked.

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


     const MPrecPoint& getLeftPoint() const{ return lp; }
     const MPrecPoint& getRightPoint() const{ return rp; }
     const MPrecPoint& getDomPoint() const {return ldp?lp:rp;}
     const MPrecPoint& getSecPoint() const {return ldp?rp:lp;}
     bool isLeftDomPoint() const { return ldp; }
     OWNER getOwner() const { return owner; }


     void appendTo(DbArray<PPrecHalfSegment>* gridStorage,
                   DbArray<uint32_t>* fracStorage){
          lp.changeScaleTo(getScale());
          rp.changeScaleTo(getScale());
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
             const OWNER _owner,
             const AttrType _attributes){
       assert(lp!=rp);
       ldp = _ldp;
       lp = _lp;
       rp = _rp;
       owner = _owner;
       attributes = _attributes;
       correctOrder();
    }

    void set(const OWNER _owner, const MPrecHalfSegment& hs){
         operator=(hs);
         owner = _owner;
    }

    void setOwner(const OWNER _owner){
       owner = _owner;
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
      const MPrecCoordinate& x = p.getX();
      const MPrecCoordinate& y = p.getY();
      const MPrecCoordinate& x1 = lp.getX();
      const MPrecCoordinate& x2 = rp.getX();
      const MPrecCoordinate& y1 = lp.getY();
      const MPrecCoordinate& y2 = rp.getY();

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
       const MPrecCoordinate& x1 = lp.getX();
       const MPrecCoordinate& y1 = lp.getY();
       const MPrecCoordinate& x2 = rp.getX();
       const MPrecCoordinate& y2 = rp.getY();

       const MPrecCoordinate& hsx1 = hs.lp.getX();
       const MPrecCoordinate& hsy1 = hs.lp.getY();
       const MPrecCoordinate& hsx2 = hs.rp.getX();
       const MPrecCoordinate& hsy2 = hs.rp.getY();

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

      const MPrecCoordinate& x1 = lp.getX();
      const MPrecCoordinate& x2 = rp.getX();
      const MPrecCoordinate& y1 = lp.getY();
      const MPrecCoordinate& y2 = rp.getY();
      const MPrecCoordinate& hsx1 = hs.lp.getX();
      const MPrecCoordinate& hsx2 = hs.rp.getX();
      const MPrecCoordinate& hsy1 = hs.lp.getY();
      const MPrecCoordinate& hsy2 = hs.rp.getY();
      const MPrecCoordinate& y = hsy1-hsy2;
      const MPrecCoordinate& u = x2-x1;
      const MPrecCoordinate& v = hsx1-hsx2;
      const MPrecCoordinate& x = y2-y1;
      const MPrecCoordinate& k = y*u-v*x;

      if(k==0){  // segments are parallel, no crossing possible
         return false;
      }   
      
      const MPrecCoordinate& w = x1-hsx1;
      const MPrecCoordinate& z = y1-hsy1;

      const MPrecCoordinate& delta2 = (w*x-z*u) / k;
      const MPrecCoordinate& delta1 = abs(u) > abs(x) 
                                    ? ((w+delta2*v)/u)*-1
                                    : ((z+delta2*y)/x)*-1;

      if(delta1<=0u || delta2 <= 0u){ // cross point left of one segment
         return false;
      }   
      if(delta1>=1u || delta2 >= 1u){ // cross point right of segment
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
      const MPrecCoordinate& x1 = lp.getX();
      const MPrecCoordinate& x2 = rp.getX();
      const MPrecCoordinate& y1 = lp.getY();
      const MPrecCoordinate& y2 = rp.getY();
      const MPrecCoordinate& hsx1 = hs.lp.getX();
      const MPrecCoordinate& hsx2 = hs.rp.getX();
      const MPrecCoordinate& hsy1 = hs.lp.getY();
      const MPrecCoordinate& hsy2 = hs.rp.getY();
      const MPrecCoordinate& y = hsy1-hsy2;
      const MPrecCoordinate& u = x2-x1;
      const MPrecCoordinate& v = hsx1-hsx2;
      const MPrecCoordinate& x = y2-y1;
      const MPrecCoordinate& k = y*u-v*x;
      if(k==0){  // segments are parallel
         return 0;
      }   
      
      const MPrecCoordinate& w = x1-hsx1;
      const MPrecCoordinate& z = y1-hsy1;

      const MPrecCoordinate& delta2 = (w*x-z*u) / k;
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
      const MPrecCoordinate& xp = x1 + delta1*(x2-x1);
      const MPrecCoordinate& yp = y1 + delta1*(y2-y1);
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
       if(isVertical()){
          if(hs.isVertical()){
              return     (rp.getY() <= hs.lp.getY())   // below hs
                      || (lp.getY() >= hs.rp.getY());  // above hs
          }
          if(lp.getX() <= hs.lp.getX() || // left hs 
             lp.getX() >= hs.rp.getX()){  // right hs
             return true;
          }
          MPrecCoordinate y = hs.getY(lp.getX());
          return ((y<lp.getY()) || (y>rp.getY()));
       } 
       if(hs.isVertical()){
          if( hs.lp.getX() <= lp.getX() ||
              hs.lp.getX() >= rp.getX()){
            return true;
          }
          MPrecCoordinate y = getY(hs.lp.getX());
          return y<hs.lp.getY() || y>hs.rp.getY();
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
       const MPrecCoordinate&  dx = (p.getX() - lp.getX()) /
                                    (rp.getX() - lp.getX());
       if(dx <= 0 || dx>=1){
           return false;
       }
       const MPrecCoordinate& y = lp.getY() + dx*(rp.getY()-lp.getY());
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

/*
compares the slopes of this halfsegment and ~hs~. 
The slope is computed undirected, meaning always from left to right point.

*/
    int compareSlope(const MPrecHalfSegment& hs)const{
       if(isVertical()){
          if(hs.isVertical()){
             return 0;
          }
          return 1;
       }
       if(hs.isVertical()){
           return -1;
       }

        
       

       
       const MPrecCoordinate& slope = (lp.getY()-rp.getY()) /
                                      (lp.getX() - rp.getX());

       const MPrecCoordinate& hsslope = (hs.lp.getY()-hs.rp.getY()) /
                                 (hs.lp.getX() - hs.rp.getX());
       return slope.compare(hsslope);
    }

/*
~getY~

This function returns the y value for the line defined by this segment 
for a given x-coordinate. If the segment is vertical and the given x 
value corresponds to the x value of this segment. the smallest y -value
of this segment is returned.
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
      if(x == lp.getX()) return lp.getY();
      if(x == rp.getX()) return rp.getY();

      // non vertical segment
      const MPrecCoordinate& delta = (x - lp.getX()) / (rp.getX() - lp.getX());
      const MPrecCoordinate& y = lp.getY() + delta * (rp.getY() - lp.getY());
      return y;
    }


    uint32_t getScale() const{
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


  ListExpr toListExpr(const bool includeScale = false) const {
    ListExpr value = nl->TwoElemList(lp.toListExpr(false),
                                     rp.toListExpr(false));
    if(includeScale){
       value = nl->TwoElemList( nl->IntAtom(lp.getScale()), value);
    }
    return value;
  }

  ListExpr toLineList() const{
    return nl->TwoElemList( nl->SymbolAtom("precLine"),
               nl->TwoElemList( nl->IntAtom(lp.getScale()),
                   nl->OneElemList(toListExpr())));
  }

  const MPrecCoordinate& getMinY() const{
     return rp.getY() < lp.getY() ? rp.getY() : lp.getY() ;
  }

  const MPrecCoordinate& getMaxY() const{
     return rp.getY() > lp.getY() ? rp.getY() : lp.getY() ;
  }

  void changeScaleTo(uint32_t newScale){
    assert(newScale>0);
    lp.changeScaleTo(newScale);
    rp.changeScaleTo(newScale);
  }

  void swap(MPrecHalfSegment& h){
    bool l = ldp;
    OWNER o = owner;
    AttrType a = attributes;

    lp.swap(h.lp);
    rp.swap(h.rp);

    owner = h.owner;
    ldp = h.ldp;
    attributes = h.attributes;
    h.owner = o;
    h.ldp = l;
    h.attributes = a;
  }

     
  private:
     bool ldp;
     MPrecPoint lp;
     MPrecPoint rp;
     OWNER owner;
  public:
     AttrType attributes;
  private: 
  



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

  HalfSegmentComparator(){}

  int operator()(const MPrecHalfSegment& hs1, const MPrecHalfSegment& hs2){
      // compare domination points
      const MPrecPoint& dp1 = hs1.getDomPoint();
      const MPrecPoint& dp2 = hs2.getDomPoint();
      int cmp = dp1.compareTo(dp2);
      if(cmp!=0) return cmp;

      // compare ldp
      if( hs1.isLeftDomPoint()  != hs2.isLeftDomPoint() ) {
           if( !hs1.isLeftDomPoint() ){
              return -1;
           } else {
              return 1;
           }
      }

      // compare slopes
      cmp = hs1.compareSlope(hs2);
      return cmp;

  }
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

template<class Obj, class Comp>
class IsGreater{
  public:
     bool operator()(const Obj& o1, const Obj& o2){
        return comp(o1,o2)>0;
     }

  private:
     Comp comp;

};


namespace std{
 template<>
 inline void swap(MPrecHalfSegment& a, MPrecHalfSegment& b){
    a.swap(b);
 }
}



#endif
