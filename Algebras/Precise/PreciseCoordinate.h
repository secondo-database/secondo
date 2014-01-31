
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


#include <gmp.h>
#include <gmpxx.h>
#include <limits>
#include <vector>
#include <exception>
#include <string>
#include <sstream>
#include <iostream>
#include <errno.h>
#include "StringUtils.h"

#include "../../Tools/Flob/DbArray.h"

#include "RationalAttr.h"

#ifndef PRECISE_COORDINATE
#define PRECISE_COORDINATE

class overflowException: public exception{
   public:
        overflowException(const string& _msg): msg(_msg) {
          
        } 

        virtual const char* what() const throw() {
           return msg.c_str();
        }

        ~overflowException() throw(){}
   
   private:
       string msg;
  
};



/*
1 Class PPrecCoordinate

This class represents the persistent root of a precise Coordinate.
It consists of an integer representing the gridcoordinate and an
unsigned integer representing the position of the exact part within 
a DbArray.

The gridCoord represents the integer part of this component. The
position represents the position of the part within the cell within a
DbArray. A value of 0 means that there is no fractional part.

*/

class PPrecCoordinate{

  public:
     PPrecCoordinate(){}  // does nothing for storing within a DbArray 

     PPrecCoordinate(const int64_t _gridCoord, const uint32_t _precPos):
         gridCoord(_gridCoord), precPos(_precPos) {}

     PPrecCoordinate(const PPrecCoordinate& src): 
         gridCoord(src.gridCoord),precPos(src.precPos) {}

     PPrecCoordinate(const int32_t v): gridCoord(v), precPos(0) {}
     PPrecCoordinate(const int64_t v): gridCoord(v), precPos(0) {}

     PPrecCoordinate& operator=(const PPrecCoordinate& src){
         this->gridCoord = src.gridCoord;
         this->precPos = src.precPos;
         return *this;
     }

     int64_t getGridCoord() const{
        return gridCoord;
     }

     bool hasFracPart() const{
        return precPos!=0;
     }
    
     int64_t getIntPart() const{
       return gridCoord;
     }

     size_t getHash() const{
        return (size_t) gridCoord;
     }

                                                  

  protected:
     mutable int64_t  gridCoord;     // the grid part
     mutable uint32_t precPos;  // position of the exact part
};



/*
2 Class MPrecCoordinate

This class represents a precise coordinate as a main memory structure. It 
contains the The usual operations are defined on this class. It holds a 
pointer to the DbArray storing
the fractional part. This is the only class having knowlegde about the 
persistent storing of the fractional part.

Beside the information inherited from the persistent coordinate 
representation, it contains two pointers. One pointer points to the 
DbArray containing the fractional part. The other pointer points to 
the fraction itselfs. In each case, one of the pointers is null and the 
other one is not null.

The representation of the fractional part is:

length of numerator 
numerator*
length of denominator
denominator*

*/
class MPrecCoordinate;
std::ostream& operator<<(std::ostream& o, const MPrecCoordinate& x);

class MPrecCoordinate : public PPrecCoordinate{
  public:
     MPrecCoordinate(const int64_t _gridCoord, 
                     const uint32_t  _precPos,
                     const uint32_t _scale, 
                     DbArray<uint32_t>* _fracStorage):
          PPrecCoordinate(_gridCoord,_precPos), 
          fracStorage(_fracStorage), fractional(0), scale(_scale) {
        assert(_scale>0);
     }

     MPrecCoordinate(const int64_t intPart, const string& fracPart, 
                     const uint32_t _scale): 
            PPrecCoordinate(intPart,1), fracStorage(0), 
            fractional(0), scale(_scale){
         assert(_scale>0);
         fractional = new mpq_class(fracPart);
         canonicalize();
     }

     MPrecCoordinate(const PPrecCoordinate& _p, 
                     const DbArray<uint32_t>* _fracStore,
                     const uint32_t _scale):
       PPrecCoordinate(_p), fracStorage(_fracStore), 
       fractional(0), scale(_scale) {
        assert(_scale>0);
     }

     MPrecCoordinate(const int64_t _gridCoord, const mpq_class& _fractional, 
                     const int32_t _scale):
          PPrecCoordinate(_gridCoord,1),fracStorage(0), 
          fractional(new mpq_class(_fractional)),
          scale(_scale) {
       assert(_scale>0);
       canonicalize();
     }

     MPrecCoordinate(const MPrecCoordinate& src):
       PPrecCoordinate(src), fracStorage(src.fracStorage),
       fractional(0),scale(src.scale){
        fractional = src.fractional?new mpq_class(*(src.fractional)):0;
     }
     
     MPrecCoordinate(const MPrecCoordinate& src, uint32_t newScale):
       PPrecCoordinate(src), fracStorage(src.fracStorage),
       fractional(0),scale(src.scale){
        assert(newScale >0);
        fractional = src.fractional?new mpq_class(*(src.fractional)):0;
        changeScaleTo(newScale);
     }

     MPrecCoordinate(const int64_t value, uint32_t _scale=1):
         PPrecCoordinate(value,0), fracStorage(0),
         fractional(0), scale(_scale) {
         assert(_scale>0);
     }

     MPrecCoordinate(const int32_t value, const uint32_t _scale =1):
       PPrecCoordinate(value,0), fracStorage(0),fractional(0) , scale(_scale){
       assert(_scale>0);
     }
     
     MPrecCoordinate(const uint32_t value, const uint32_t _scale =1):
       PPrecCoordinate(value,0), fracStorage(0),fractional(0) , scale(_scale){
         assert(_scale>0);
     }

     MPrecCoordinate(const double& value, const uint32_t _scale=1):
         PPrecCoordinate(0,0),fracStorage(0), 
          fractional( new mpq_class(value)), scale(_scale){
         assert(_scale>0);
          canonicalize(); 
     }

     MPrecCoordinate(const Rational& value, const uint32_t _scale=1):
          PPrecCoordinate(0,0), fracStorage(0), 
          fractional(new mpq_class(value.ToString())),
          scale(_scale){
         assert(_scale>0);
          canonicalize();
     }

     MPrecCoordinate(const mpq_class& value, uint32_t _scale=1):
         PPrecCoordinate(0,0), fracStorage(0), 
         fractional(new mpq_class(value)), scale(_scale){
         assert(_scale>0);
         canonicalize();
     }

     bool readFromString(const string& str, uint32_t _scale=1){
        // the string may be in format a/b or a.b  or a where
       // a and b are in form [0-9]*
        if(_scale<=0){
           return false;
        }
        bool isFraction = false;
        string part1 =""; 
        string part2 ="";
        bool fill2=false; 
        for(size_t i=0;i<str.length();i++){
           char c = str[i];
           if(stringutils::isDigit(c)){
              if(fill2){
                part2 += c;
              }   else {
                part1 += c;
              }
           } else if(c=='.'){
               if(fill2){
                 return false;;
               }
               fill2 = true;
               isFraction = false;  
           } else if(c=='/'){
               if(fill2){
                  return false;
               }
               fill2 = true;
               isFraction = true;
           } else if(c=='-'){
              if(i==0){
                part1+=c;
              } else {
                return false;
              }
           } else  {
               return false;
           }
        }  
 
        if(part2.length()==0){
           bool correct;
           gridCoord = stringutils::str2int<int64_t>(part1, correct);
           if(!correct){
              return false;
           }
           precPos = 0;
           if(fractional){
             delete fractional;
             fractional = 0;
           }
           fracStorage = 0;
           scale = _scale;
           operator *=(_scale);
           assert(scale >0);
           return true;
        } else {
           if(isFraction){
               try{
                 gridCoord = 0;
                 if(fractional){
                    fractional->set_str(str,10); 
                 } else {
                    fractional = new mpq_class(str,10);
                 }
                 scale = _scale;
                 fracStorage = 0;
                 precPos = 1;
                 canonicalize();
                 operator*=(_scale);
                 assert(scale>0);
                 return true;
               } catch(...){
                 return false;  
               }
           }  else {
              bool correct;
              int64_t c = stringutils::str2int<int64_t>(part1, correct);

              if(!correct){
                return false;
              }
              uint64_t num = stringutils::str2int<uint64_t>(part2, correct);
              if(!correct){
                return false;
              }
              gridCoord = c;
              if(num==0){
                precPos = 0;
                if(fractional){
                  delete fractional;
                  fractional = 0;
                }
                fracStorage = 0;
                precPos = 0;
                scale = _scale;
                assert(scale >0);
                return true; 
              }
              uint64_t denom = 1;
              for(size_t i=0;i<part2.length();i++){
                denom *=10;
              }
              if(fractional){
                 mpq_class tmp(num, denom);  
                 *fractional = tmp;
              } else {
                 fractional = new mpq_class(num,denom);
              }
              fracStorage = 0;
              precPos = 1;
              scale = _scale;
              canonicalize();
              operator *=(_scale);
              assert(scale >0);
              return  true; 
           }
        }

     }


     MPrecCoordinate& operator=(const MPrecCoordinate& src){
         PPrecCoordinate::operator=(src);
         src.retrieveFractional();
         fracStorage=0;
         if(!fractional){
           if(src.fractional){
              fractional = new mpq_class(*(src.fractional));
           }
         } else {
           if(src.fractional){
              *fractional = *(src.fractional);
           } else {
              delete fractional;
              fractional = 0;
           }
         }
         scale = src.scale;
         assert(scale>0);
         return *this;
     }


     ~MPrecCoordinate(){
        if(fractional){
           delete fractional;
        }
     }

     std::string toString( bool includeScale=true) const{
         stringstream ss;
         if(includeScale){
              ss << scale << " :: ";
         }
         ss  << gridCoord;
         if(precPos==0){
             return ss.str();
         }
         retrieveFractional();
         ss << " " << fractional->get_str();
         return ss.str();
     }

     std::string getDebugString() const{
       stringstream ss;
       ss << "scale "        << scale << ", "
          << "grid "         << gridCoord << ", "
          << "precPos "      << precPos << ", "
          << "Db present : " << (fracStorage?"true":"false") << ", "
          << "fraction : " << (fractional?fractional->get_str():"null") ;
       return ss.str();
     }


     void appendFractional(DbArray<uint32_t>* storage) const{
        if(precPos==0){ // no fractional part to append
           return;
        }
        retrieveFractional(); // be sure to have the fractional part
        fracStorage = storage;
        mpz_class numerator(fractional->get_num());
        if(numerator==0){ // no fractional part found
            precPos = 0;
            return;  
        }
        mpz_class denominator(fractional->get_den());
        vector<uint32_t> vn = getVector(numerator);
        vector<uint32_t> vd = getVector(denominator);
        uint32_t ln = (uint32_t) vn.size();
        uint32_t ld = (uint32_t) vd.size();
        if(storage->Size()==0){ // append a dummy because 0 is a 
                                  // reserved position
           storage->Append( 0 );
        }
        precPos = storage->Size(); 
        storage->Append(ln);
        for(size_t i=0;i<vn.size();i++){
          storage->Append(vn[i]);
        } 
        storage->Append(ld);
        for(size_t i=0;i<vd.size();i++){
          storage->Append(vd[i]);
        } 
     } 


     int compare(const MPrecCoordinate& rhs) const {

        if(scale!=rhs.scale){
           rhs.changeScaleTo(scale);
        }


        if(gridCoord < rhs.gridCoord) return -1;
        if(gridCoord > rhs.gridCoord) return 1;
        if(precPos == 0){
           return rhs.precPos==0?0:-1;
        }
        if(rhs.precPos==0){
           return 1;
        }
        retrieveFractional();
        rhs.retrieveFractional();
        return cmp(*fractional, *(rhs.fractional));
     }


     // mathematical operators
     // unary + and -
     MPrecCoordinate operator-() const{
        MPrecCoordinate result(*this);
        if(precPos==0){
           result.gridCoord *= -1;
        } else {
           result.retrieveFractional();
           result.gridCoord = -1*gridCoord -1;
           *(result.fractional) = 1 - *fractional;
        }
        return result;
     }

     MPrecCoordinate operator+() const{
        MPrecCoordinate result(*this);
        return result;
     }

/*
~Comparisions~

*/     

     bool operator==(const MPrecCoordinate& rhs) const{
         return compare(rhs)==0;
     }

     bool operator!=(const MPrecCoordinate& rhs) const{
         return compare(rhs)!=0;
     }

     bool operator<(const MPrecCoordinate& rhs) const{
         return compare(rhs)<0;
     }

     bool operator>(const MPrecCoordinate& rhs) const{
         return compare(rhs)>0;
     }

     bool operator<=(const MPrecCoordinate& rhs) const{
         return compare(rhs)<=0;
     }

     bool operator>=(const MPrecCoordinate& rhs) const{
         return compare(rhs)>=0;
     }

/*
Binary operators

*/     
     MPrecCoordinate& operator+=(const int64_t& v) {
         if(scale==1){
            gridCoord += v;
         } else {
            MPrecCoordinate tmp( mpq_class(v,scale), scale);
            operator+=(tmp);
         }
         return *this;
     }
     
     MPrecCoordinate& operator-=(const int64_t& v) {
         if(scale == 1){
             gridCoord -= v;
         } else {
            MPrecCoordinate tmp( mpq_class(v,scale), scale);
            operator-=(tmp);
         }
         return *this;
     }

     MPrecCoordinate& operator+=(const MPrecCoordinate& v){
         if(v.scale != scale){
            v.changeScaleTo(scale);
         }
         if(v.precPos==0){
            gridCoord += v.gridCoord;
            return *this;
         }
         if(this->precPos==0){
            v.retrieveFractional();
            fractional = new mpq_class(*(v.fractional));
            fracStorage = 0;
            precPos = 1;
            gridCoord += v.gridCoord;
            return *this;
         }
         // both operands have a fractional part
         retrieveFractional();
         v.retrieveFractional();
         gridCoord += v.gridCoord;
         *fractional += *(v.fractional);
         canonicalize();
         return *this;
     }

     MPrecCoordinate& operator-=(const MPrecCoordinate& v){
         if(v.scale != scale){
           v.changeScaleTo(scale);
         }

         if(v.precPos==0){
            gridCoord -= v.gridCoord;
            return *this;
         }
         v.retrieveFractional();
         if(this->precPos==0){
            fractional = new mpq_class(*(v.fractional));
            fracStorage = 0;
            precPos = 1;
            *fractional *= -1;
            gridCoord -= v.gridCoord;
            canonicalize();
            return *this; 
         }
         // both operands have a fractional part
         retrieveFractional();
         v.retrieveFractional();
         gridCoord -= v.gridCoord;
         *fractional -= *(v.fractional);
         canonicalize();
         return *this;
     }

     MPrecCoordinate& operator*=(const int64_t v){
          if(precPos==0){
              gridCoord *= v;
              return *this;
          }
          retrieveFractional();
          gridCoord *= v;
          *fractional *= v;
          canonicalize();
          return *this;
     }
     
    MPrecCoordinate& operator*=(const MPrecCoordinate v){
       if(v==1){
          return *this;
       }
       if(v.precPos==0){
          if(precPos==0){
             gridCoord *= v.gridCoord*v.scale;
             return *this;
          } else {
             gridCoord *= v.gridCoord*v.scale;
             retrieveFractional();
             (*fractional) *= v.gridCoord*v.scale;
             canonicalize();
             return *this; 
          }
       }  
       if(precPos==0){
          v.retrieveFractional();
          fractional = new mpq_class(gridCoord * *(v.fractional) * v.scale);
          gridCoord *= v.gridCoord*v.scale;
          canonicalize();
          return *this;
       }
       retrieveFractional();
       v.retrieveFractional();
       int64_t gc = gridCoord * v.gridCoord*v.scale;
       mpq_class f = v.scale*gridCoord*(*v.fractional) +
                    *(fractional)*v.gridCoord + *(fractional)* *(v.fractional);
       gridCoord = gc;
       *fractional =  f;
       canonicalize();
       return *this;
    }

    MPrecCoordinate& operator/=(const MPrecCoordinate v){

       mpq_class t = getComplete();
       mpq_class o = v.getComplete()*v.scale;
       gridCoord = 0;
       if(!fractional){
         fractional = new mpq_class(t/o);
       } else {
         *fractional = t/o;
       }
       canonicalize();
       return *this;
    }

    MPrecCoordinate operator+(const MPrecCoordinate& rhs) const{
        MPrecCoordinate result(*this);
        return result.operator+=(rhs);
    }
    MPrecCoordinate operator-(const MPrecCoordinate& rhs) const{
        MPrecCoordinate result(*this);
        return result.operator-=(rhs);
    }
    MPrecCoordinate operator*(const MPrecCoordinate& rhs)const{
        MPrecCoordinate result(*this);
        return result.operator*=(rhs);
    }
    MPrecCoordinate operator/(const MPrecCoordinate& rhs) const{
        MPrecCoordinate result(*this);
        return result.operator/=(rhs);
    }

    string getFracAsText() const{
       if(precPos==0) return "";
       retrieveFractional();
       return fractional->get_str();
    }

    double toDouble(){
       mpq_class c = getComplete();
       c *= scale;
       return c.get_d();
    }


    uint32_t getScale() const{
      return scale;
    }
   

    void set(int64_t intPart, const string& frac, uint32_t _scale){
        if(fractional){
          fractional->set_str(frac,10);
        } else {
           fractional = new mpq_class(frac);
        }
        gridCoord = intPart;
        scale = _scale;
        assert(scale>0);
        canonicalize(); 
    }

    void changeScaleTo(uint32_t newScale) const{
        if(scale==newScale){
           return;
        }
        assert(newScale!=0);
        mpq_class v = getComplete();
        mpq_class factor(newScale,scale);
        gridCoord = 0;
        if(fractional){
           *fractional = v*factor;
        } else {
           fractional = new mpq_class(v*factor);
        }
        scale = newScale;
        canonicalize();
    } 


/*
  Debugging function

*/
  void getFractional(void*& result) const{
      result = fractional;
  }


  private:
     mutable const DbArray<uint32_t>* fracStorage;
     mutable mpq_class* fractional;
     mutable uint32_t scale;


     MPrecCoordinate(): PPrecCoordinate(0,0), 
                        fracStorage(0), fractional(0), 
                        scale(1){}


     void retrieveFractional() const{
        if(precPos==0) return; // no fractional part present
        if(fractional) return; // fractional part already read
        uint32_t length_nom;
        uint32_t length_denom;
        // read in length information
        fracStorage->Get(precPos,length_nom);
        uint32_t pos = precPos+1;
        // read numerator
        uint32_t intval;
        uint32_t factori = numeric_limits<uint32_t>::max();
        mpz_class factor(factori);
        fracStorage->Get(pos,intval);
        mpz_class numerator(intval);
        pos++;
        for(uint32_t i=1;i<length_nom;i++){
            fracStorage->Get(pos,intval);
            pos++;
            numerator = numerator +  factor * mpz_class(intval);
            factor = factor * factor;
        }
        
        fracStorage->Get(pos, length_denom);
        pos++;
        factor = mpz_class(factori);
        fracStorage->Get(pos,intval);
        pos++;
        mpz_class denominator(intval);
        for(uint32_t  i=1;i<length_denom;i++){
            fracStorage->Get(pos,intval);
            pos++;
            denominator = denominator  +  factor * mpz_class(intval);
            factor = factor * factor;
        }
        fractional = new mpq_class(numerator, denominator); 
        fracStorage = 0; // not longer usednt 
        canonicalize();
     }


/*
~Canonicalize~

This function brings the integer part of fractional to the gridCoordinate. 
After calling this function, the fractional part is in (0,1).

*/

     void canonicalize() const{
        assert(fractional);
        if(*fractional == 0){ // no fractional part
           precPos = 0;
           delete fractional;
           fractional = 0;
           return;
        }
        fractional->canonicalize();
        if(*fractional < 0){
           *fractional = abs(*fractional) + 1;
           mpz_class intPart = fractional->get_num() / fractional->get_den();
           if(!intPart.fits_slong_p()){
               throw overflowException("intpart of fraction does not"
                                       " fit into a long");
           }
           *fractional = *fractional - intPart;
           errno=0;
           gridCoord -= intPart.get_si();
           if(errno){
               throw overflowException("intpart too small");
           } 
           *fractional = 1 - *fractional;
        } 
        if(*fractional >= 1){
           mpz_class intPart = fractional->get_num() / fractional->get_den();
           *fractional = *fractional - intPart;
           if(!intPart.fits_slong_p()){
              throw overflowException("intpart of fraction does not"
                                      " fit into a long");
           } 
           errno=0;
           gridCoord += intPart.get_si();     
           if(errno){
               throw overflowException("intpart too small");
           } 
        }
        fracStorage = 0;
        precPos = *fractional!=0?1:0;
        if(*fractional==0){ // remove fractional part of 0
            delete fractional;
            fractional = 0;
        }
     }

   
     vector<uint32_t> getVector(mpz_class number) const{
        uint32_t maxi = numeric_limits<uint32_t>::max();
        mpz_class max(maxi);
        mpz_class zero(0);
        vector<uint32_t> result;
        while(number > zero){
           mpz_class part = number % max;
           number = number / max;
           result.push_back((uint32_t) part.get_ui());
        }  
        return result;
     } 


     mpq_class getComplete() const{
        if(precPos==0){
           return mpq_class(gridCoord);
        }
        retrieveFractional();
        return mpq_class(gridCoord) + *fractional;
     }


};

MPrecCoordinate abs(const MPrecCoordinate& v);




#endif



