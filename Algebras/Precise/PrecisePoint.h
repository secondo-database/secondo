

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
#ifndef PRECISE_POINT
#define PRECISE_POINT


#include "PreciseCoordinate.h"


class MPrecPoint;

class PPrecPoint {

  public:

     PPrecPoint() {}

     PPrecPoint(const PPrecCoordinate& _x, const PPrecCoordinate& _y):
       x(_x), y(_y) {}

     PPrecPoint (const PPrecPoint& src): x(src.x), y(src.y) {}

     PPrecPoint( const MPrecPoint& src);

     PPrecPoint& operator=(const PPrecPoint& src){
       x = src.x;
       y = src.y;
       return *this;
     }

     PPrecCoordinate getX() const { return x; }
     PPrecCoordinate getY() const { return y; }

     size_t getHash() const {
        return x.getIntPart() + y.getIntPart();
     }

     void set(const PPrecCoordinate _x, const PPrecCoordinate _y){
        x = _x;
        y = _y; 
     }

  private:
     PPrecCoordinate x;
     PPrecCoordinate y; 
};




ostream& operator<<(ostream& o, const PPrecPoint& p);
ostream& operator<<(ostream& o, const MPrecPoint& p);


class MPrecPoint {
   public:


      MPrecPoint(const MPrecCoordinate& _x,
                 const MPrecCoordinate& _y): x(_x), y(_y) {
          unifyScale();
      }

      MPrecPoint(const PPrecPoint& point, const DbArray<uint32_t>* fst, 
                 const uint32_t _scale):
        x(point.getX(),fst,_scale), y(point.getY(),fst,_scale)
      {
         assert(_scale>0);
      }

      MPrecPoint(const MPrecPoint& src) : x(src.x), y(src.y)
      { }

      MPrecPoint& operator=(const MPrecPoint&  src){
            x = src.x;
            y = src.y;
            return *this;
      }

      const MPrecCoordinate& getX()const { return x; }
      const MPrecCoordinate& getY()const { return y; }

      int compareTo(const MPrecPoint& rhs) const{
         int cmp = x.compare(rhs.x);
         if(cmp!=0){
           return cmp;
         }
         return y.compare(rhs.y);
      }

      bool operator==(const MPrecPoint& rhs) const{
         return compareTo(rhs)==0;
      }
      
      bool operator!=(const MPrecPoint& rhs) const{
         return compareTo(rhs)!=0;
      }


      bool operator<(const MPrecPoint& rhs) const{
         return compareTo(rhs)<0;
      }
      bool operator>(const MPrecPoint& rhs) const{
         return compareTo(rhs)>0;
      }
      bool operator<=(const MPrecPoint& rhs) const{
         return compareTo(rhs)<=0;
      }
      bool operator>=(const MPrecPoint& rhs) const{
         return compareTo(rhs)>=0;
      }


      void compScale(const MPrecCoordinate& sf){
          x *= sf;
          y *= sf;
      }
      
      void compScale(const MPrecCoordinate& sx, const MPrecCoordinate& sy){
         x*=sx;
         y*=sy;
      }
  
      void compTranslate(const MPrecCoordinate& tx,const MPrecCoordinate& ty){
         x+=tx;
         y+=ty;
      }


      void set(const MPrecCoordinate& _x, const MPrecCoordinate& _y){
         x = _x;
         y = _y;
         unifyScale();
      }
      
      void set(const double _x, const double _y, uint32_t _scale){
         x = MPrecCoordinate(_x,_scale);
         y = MPrecCoordinate(_y,_scale);
      }

      uint32_t getScale() const{
          return x.getScale();
      }

      MPrecCoordinate QDistance( MPrecPoint& p2){
         MPrecCoordinate dx = p2.x - x;
         MPrecCoordinate dy = p2.y - y;
         return dx*dx + dy*dy;
      }

      std::string toString(bool includeScale=true) const {
        stringstream ss;
        if(includeScale){
            ss << x.getScale() << ":: ";
        } 
        ss << "( " << x.toString(false) << ", " << y.toString(false) << ")";
        return ss.str();
      }    

      size_t getHash() const{
         return x.getHash() + y.getHash();
      }

      void appendFractional(DbArray<uint32_t>* fracStorage){
         x.appendFractional(fracStorage);
         y.appendFractional(fracStorage);
      }

      PPrecPoint getPersistent() const{
         return PPrecPoint(x,y);
      }

      void changeScaleTo(uint32_t newScale) const{
        assert(newScale>0);
        x.changeScaleTo(newScale);
        y.changeScaleTo(newScale);
      }

      ListExpr toListExpr(const bool includeScale=false)const{
         ListExpr value = nl->TwoElemList(x.toListExpr(false),
                                          y.toListExpr(false));
         if(includeScale){
            value = nl->TwoElemList(nl->IntAtom(x.getScale()), value);
         }
         return value;
      }
 

/*
  Debugging function

*/    
   void getFractionals(void** result) const{
       x.getFractional(result[0]);
       y.getFractional(result[1]);
    }

    bool checkScale() const{
       return x.getScale()==y.getScale();
    }

    void swap(MPrecPoint& b){
       x.swap(b.x);
       y.swap(b.y);
    }

    void bringToMemory() const{
      x.bringToMemory();
      y.bringToMemory();
    }

   private:
     MPrecCoordinate x;
     MPrecCoordinate y;

     MPrecPoint():x(0),y(0){ assert(false); }
    
    void unifyScale(){
       if(x.getScale()==y.getScale()){
          return;
       }
       if(x.getScale()>y.getScale()){
          y.changeScaleTo(x.getScale());
       } else {
         x.changeScaleTo(y.getScale());
       }
    }


};


namespace std{
  template<>
  inline void swap(MPrecPoint& a, MPrecPoint& b){
     a.swap(b);
  }
}


#endif

