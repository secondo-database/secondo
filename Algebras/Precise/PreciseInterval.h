

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
#ifndef PRECISE_INTERVAL
#define PRECISE_INTERVAL


#include "PreciseCoordinate.h"


class MPrecInterval;

class PPrecInterval {

  public:

     PPrecInterval() {}

     PPrecInterval(const PPrecCoordinate& _start, const PPrecCoordinate& _end,
                   const bool leftClosed, const bool rightClosed):
       start(_start), end(_end), lc(leftClosed), rc(rightClosed) {
     }

     PPrecInterval (const PPrecInterval& src):
         start(src.start), end(src.end) , lc(src.lc), rc(src.rc){}

     PPrecInterval( const MPrecInterval& src);

     PPrecInterval& operator=(const PPrecInterval& src){
       start = src.start;
       end = src.end;
       lc = src.lc;
       rc = src.rc;
       return *this;
     }

     PPrecCoordinate getStart() const { return start; }
     PPrecCoordinate getEnd() const { return end; }
     bool getLC() const{ return lc; }
     bool getRC() const{ return rc; }

     size_t getHash() const {
        return start.getIntPart() + end.getIntPart();
     }

     void set(const PPrecCoordinate _start, const PPrecCoordinate _end,
              const bool _lc, const bool _rc){
        start = _start;
        end = _end; 
        lc = _lc;
        rc = _rc;
     }

  private:
     PPrecCoordinate start;
     PPrecCoordinate end; 
     bool lc;
     bool rc;
};




ostream& operator<<(ostream& o, const PPrecInterval& p);
ostream& operator<<(ostream& o, const MPrecInterval& p);


class MPrecInterval {
   public:


      MPrecInterval(const MPrecCoordinate& _start,
                 const MPrecCoordinate& _end,
                 const bool _lc, const bool _rc): 
     start(_start), end(_end), lc(_lc), rc(_rc) {
          unifyScale();
      }

      MPrecInterval(const PPrecInterval& interval, 
                    const DbArray<uint32_t>* fst, 
                 const uint32_t _scale):
        start(interval.getStart(),fst,_scale), 
        end(interval.getEnd(),fst,_scale),
        lc(interval.getLC()), rc(interval.getRC())
      {
         assert(_scale>0);
      }

      MPrecInterval(const MPrecInterval& src) : start(src.start), 
                            end(src.end), lc(src.lc), rc(src.rc)
      { }


      MPrecInterval& operator=(const MPrecInterval&  src){
            start = src.start;
            end = src.end;
            lc = src.lc;
            rc = src.rc;
            return *this;
      }

      const MPrecCoordinate& getStart()const { return start; }
      const MPrecCoordinate& getEnd()const { return end; }
      bool getLC() const{ return lc; }
      bool getRC() const{ return rc; }

      int compareTo(const MPrecInterval& rhs) const{
         int cmp = start.compare(rhs.start);
         if(cmp!=0){
           return cmp;
         }
         if(lc != rhs.lc){
           return lc?-1:1;
         }
         cmp =  end.compare(rhs.end);
         if(cmp !=0){
           return cmp;
         }
         if(rc!=rhs.rc){
           return rc?1:-1;
         }
         return 0;
      }

      bool operator==(const MPrecInterval& rhs) const{
         return compareTo(rhs)==0;
      }
      
      bool operator!=(const MPrecInterval& rhs) const{
         return compareTo(rhs)!=0;
      }


      bool operator<(const MPrecInterval& rhs) const{
         return compareTo(rhs)<0;
      }
      bool operator>(const MPrecInterval& rhs) const{
         return compareTo(rhs)>0;
      }
      bool operator<=(const MPrecInterval rhs) const{
         return compareTo(rhs)<=0;
      }
      bool operator>=(const MPrecInterval& rhs) const{
         return compareTo(rhs)>=0;
      }


      void set(const MPrecCoordinate& _start, 
               const MPrecCoordinate& _end, const bool _lc, const bool _rc){
         start = _start;
         end = _end;
         lc = _lc;
         rc = _rc;
         unifyScale();
      }
      

      uint32_t getScale() const{
          return start.getScale();
      }

     
      size_t getHash() const{
         return start.getHash() + end.getHash();
      }

      void appendFractional(DbArray<uint32_t>* fracStorage){
         start.appendFractional(fracStorage);
         end.appendFractional(fracStorage);
      }

      PPrecInterval getPersistent() const{
         return PPrecInterval(start,end, lc, rc);
      }

      void changeScaleTo(uint32_t newScale) const{
        assert(newScale>0);
        start.changeScaleTo(newScale);
        end.changeScaleTo(newScale);
      }

      ListExpr toListExpr(const bool includeScale=false)const;


    bool checkScale() const{
       return start.getScale()==end.getScale();
    }

    void swap(MPrecInterval& b){
       start.swap(b.start);
       end.swap(b.end);
       bool tmp = lc;
       lc = b.lc;
       b.lc = tmp;
       tmp = rc;
       rc = b.rc;
       b.rc = tmp;
    }

   private:
     MPrecCoordinate start;
     MPrecCoordinate end;
     bool lc;
     bool rc;


     MPrecInterval():start(0),end(0),lc(true),rc(true){ assert(false); }
    
    void unifyScale(){
       if(start.getScale()==end.getScale()){
          return;
       }
       if(start.getScale()>end.getScale()){
          end.changeScaleTo(start.getScale());
       } else {
         start.changeScaleTo(end.getScale());
       }
    }


};


namespace std{
  template<>
  inline void swap(MPrecInterval& a, MPrecInterval& b){
     a.swap(b);
  }
}


#endif

