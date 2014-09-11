

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

#ifndef PRECSECTYPES_H
#define PRECSECTYPES_H

#include "PreciseCoordinate.h"
#include "PrecisePoint.h"
#include "PreciseInterval.h"
#include "Attribute.h"
#include "NestedList.h"
#include "GenericTC.h"
#include "DateTime.h"

#include <iostream>
#include <algorithm>
#include <vector>

#include "RectangleAlgebra.h"
#include "SpatialAlgebra.h"

#include "PreciseHalfSegment.h"

#include "HsTools.h"
#include "StringUtils.h"


extern NestedList* nl;



/*
2 Auxiliary functions


2.1 Conversion to nested lists

~toListExpr~

This function converts a precise coordinate into a nested list omitting the 
scale factor.

*/

ListExpr toListExpr(const MPrecCoordinate& x);

/*

~toListExpr~

This function converts a precise point into a nested list ommitting the 
scale factor.

*/

ListExpr toListExpr(const MPrecPoint& p);

/*

~readFrom~

This function tries to get the value of a precise coordinate from a 
nested list representation.
If include scale is set to false, the value of scale is used as the 
scale factor for this 
coordinate. Otherwise, the value given in scale is ignored and the 
scale factor is read
from the nested list representation. The return value determines the 
success of this operation.

*/
bool readFrom(ListExpr le, MPrecCoordinate& result, 
              bool includeScale, const uint32_t _scale);

/*
~readPoint~

This function tries to get the value of a precise point from a nested
 list representation.
If includeScale is set to false, the value of scale is used as the scale
 factor for the created
point. Otherwise, the value given in scale is ignored and the scale 
factor is read
from the nested list representation. The return value determines the 
success of this operation.

*/
bool readPoint(ListExpr le, MPrecPoint& result, 
               bool includeScale, const uint32_t _scale);


/*
~enlarge~

This function enlarges box that p is included in this box.

*/
void enlarge(Rectangle<2>& box, const MPrecPoint& p);


/*
~getQDistance~

This functions return the square of the distance between rect and p.

*/
double getQDistance(const Rectangle<2>& rect,
                    const MPrecPoint& p    )  ;

/*
~intersects~

Checks whether p is inside or onborder of rect.

*/
bool intersects(const Rectangle<2>& rect, 
                const MPrecPoint& p);



/*
~getRegionList~

This function takes a vector of halfsegments (only one direction)
 and computes the
nested list representation from it. It assumes that faceno, cycleno 
and edgeno are set
correctly.

*/
ListExpr getRegionList(vector<MPrecHalfSegment>& v);

/*
3 Secondo class representing a rational number with arbitrary precision

*/
class PrecCoord: public Attribute{

  public:
     PrecCoord() {}

     explicit PrecCoord(const int dummy): Attribute(false), 
                               fracStorage(0), coord(0,0), scale(1) {}

     explicit PrecCoord(const bool defined): Attribute(defined), 
                                        fracStorage(0), coord(0,0), scale(1) {}

     PrecCoord(const PrecCoord& src): Attribute(src),
           fracStorage(src.fracStorage.Size()), coord(src.coord),
           scale(src.scale){
           fracStorage.copyFrom(src.fracStorage);
     }

 
     ~PrecCoord() {}

    PrecCoord& operator=(const PrecCoord& src){
        SetDefined(src.IsDefined());
        coord = src.coord;
        fracStorage.copyFrom(src.fracStorage);
        scale = src.scale;
        return *this;
    }

    PrecCoord& operator=(const MPrecCoordinate& src){
        setValue(src);
        return *this;
    }
 

    void setValue(const MPrecCoordinate src){
         fracStorage.clean();
         src.appendFractional(&fracStorage);
         coord = src;
         scale = src.getScale();
         SetDefined(true);
    }
   
    int compareTo(const PrecCoord& rhs) const{
       MPrecCoordinate lhs1(coord,&fracStorage, scale);
       MPrecCoordinate rhs1(rhs.coord, &rhs.fracStorage, rhs.scale);
       return lhs1.compare(rhs1);
    }


    int Compare(const Attribute* arg) const{
       return compareTo(*((PrecCoord*)arg));
    }

    bool Adjacent(const Attribute* arg) const{
       return false;
    }

    size_t Sizeof() const{
       return sizeof(*this);
    }


    size_t HashValue() const{
       if(!IsDefined()){
          return 0;
       }   
       return (size_t) coord.getGridCoord();
    }

    void CopyFrom(const Attribute* arg){
       *this = *((PrecCoord*)arg);
    }

    PrecCoord* Clone() const{
       return new PrecCoord(*this);
    }

    std::string toString() const{
        if(!IsDefined()){
           return listutils::getUndefinedString();
        }
        MPrecCoordinate c(coord,&fracStorage, scale);
        return c.toString();      
    }

    std::ostream& Print(std::ostream &os) const{
       return os << toString();
    }

    ListExpr ToListExpr(ListExpr typeInfo) const;

    bool ReadFrom(ListExpr LE, const ListExpr typeInfo);

    MPrecCoordinate GetValue() const {
        MPrecCoordinate result(coord,&fracStorage, scale);
        return result;
    }

     inline virtual int NumOfFLOBs() const {
       return 1;
     }
     inline virtual Flob* GetFLOB( const int i ) {
       assert(i==0);
       return &fracStorage;   
     }

    static const string BasicType(){
       return "precise";
    }

    static bool checkType(const ListExpr e){
       return listutils::isSymbol(e, BasicType());
    }

    static bool CheckKind(ListExpr type, ListExpr& errorInfo){
      return nl->IsEqual(type,BasicType());
    } 
     
    static ListExpr Property(){
       return gentc::GenProperty("-> DATA",
                          BasicType(),
                          "( scale (int64 fraction))",
                          "( 10 (1 '1/3'))");
     }


    /* reads this intant's value form a CcInt. The 
       Arguments useStr and digits are ignored and for compatibility 
       with other calls.
     */ 
     void readFrom(const CcInt& i, int scale, bool useStr, uint8_t digits = 16){
        if(!i.IsDefined() || scale<=0){
           SetDefined(false);
           return;
        }
        SetDefined(true);
        coord = PPrecCoordinate(i.GetValue()*scale,0);
        fracStorage.clean();
        this->scale = scale;    
     }
     
     void readFrom(const CcReal& r, int scale, bool useStr, 
                   uint8_t digits = 16){
        if(!r.IsDefined() || scale<=0){
           SetDefined(false);
           return;
        }
        SetDefined(true);
        if(!useStr){
           MPrecCoordinate c(r.GetValue());
            c *= scale;
            *this = c; 
            this->scale = scale;    
        } else {
          MPrecCoordinate res(0);
          res.readFromString(stringutils::double2str(r.GetValue(), digits),
                              scale); 
          *this = res;
        }
     }
     
     void readFrom(const Rational& r, int scale, bool useStr, 
                   uint8_t digits = 16){
        if(!r.IsDefined() || scale<=0){
           SetDefined(false);
           return;
        }
        SetDefined(true);
        MPrecCoordinate c(r);
        c *= scale;
        *this = c; 
        this->scale = scale;    
     }
     
     void readFrom(const LongInt& l, int scale, bool useStr, uint8_t digits=16){
        if(!l.IsDefined() || scale<=0){
           SetDefined(false);
           return;
        }
        this->scale = scale;    
        SetDefined(true);
        coord = PPrecCoordinate(l.GetValue()*scale,0);
        fracStorage.clean();
        this->scale = scale;    
     }
     
    void readFrom(const PrecCoord& p, int scale, bool useStr, 
                  uint8_t digits=16){
        *this = p;
    }

    void clear() {
       coord = PPrecCoordinate(0,0);
       fracStorage.clean();
       scale = 1;
       SetDefined(true);
    }

    uint32_t getScale() const{
      return scale;
    }

    bool isNull() const{
        return coord.isNull(); 
    }

  private:
     DbArray<uint32_t> fracStorage;
     PPrecCoordinate coord;
     uint32_t scale;
};


/*
2 class PrecTime

A precTime value consist of a precise value and a flag indicating the
kind of the time (duration or instant)

*/
template <int type>
class PrecTime: public Attribute{

  public:
     PrecTime() {}

     explicit PrecTime(const int dummy): 
         Attribute(true), fracStorage(0), coord(0,0)
         {}

     explicit PrecTime(const bool defined): Attribute(defined), 
            fracStorage(0), coord(0,0)  {}

     PrecTime(const PrecTime<type>& src): Attribute(src),
           fracStorage(src.fracStorage.Size()), coord(src.coord) {
           fracStorage.copyFrom(src.fracStorage);
     }

 
     ~PrecTime() {}

    PrecTime<type>& operator=(const PrecTime& src){
        SetDefined(src.IsDefined());
        coord = src.coord;
        fracStorage.copyFrom(src.fracStorage);
        return *this;
    }

    PrecTime<type>& operator=(const MPrecCoordinate& src){
        setValue(src);
        return *this;
    }
 

    void setValue(const MPrecCoordinate& src){
         fracStorage.clean();
         src.appendFractional(&fracStorage);
         coord = src;
         SetDefined(true);
    }
   
    int compareTo(const PrecTime<type>& rhs) const{
       MPrecCoordinate lhs1(coord,&fracStorage, 1);
       MPrecCoordinate rhs1(rhs.coord, &rhs.fracStorage, 1);
       return lhs1.compare(rhs1);
    }

    PrecTime<type> operator+(const PrecTime<datetime::durationtype>& dur){
       PrecTime<type> res(true);
       if(!IsDefined() || !dur.IsDefined()){
          res.SetDefined(false);
          return res;
       }
       MPrecCoordinate mc(GetValue());
       mc += dur.GetValue();
       res.setValue(mc);
       return res;
    }



    int Compare(const Attribute* arg) const{
       return compareTo(*((PrecTime<type>*)arg));
    }

    bool Adjacent(const Attribute* arg) const{
       return false;
    }

    size_t Sizeof() const{
       return sizeof(*this);
    }


    size_t HashValue() const{
       if(!IsDefined()){
          return 0;
       }   
       return (size_t) coord.getGridCoord();
    }

    void CopyFrom(const Attribute* arg){
       *this = *((PrecTime<type>*)arg);
    }

    PrecTime<type>* Clone() const{
       return new PrecTime<type>(*this);
    }

    std::string toString() const{
        if(!IsDefined()){
           return listutils::getUndefinedString();
        }
        // todo convert into a readable format
        MPrecCoordinate c(coord,&fracStorage, 1);
        return c.toString();      
    }

    std::ostream& Print(std::ostream &os) const{
       return os << toString();
    }

    ListExpr ToListExpr(ListExpr typeInfo) const;

    bool ReadFrom(ListExpr LE, const ListExpr typeInfo);

    bool readFrom(const string& value);

    void readFrom(const datetime::DateTime& time){
       if(!time.IsDefined()){
          SetDefined(false);
          return;
       } else {
          SetDefined(true);
          setValue(time.millisecondsToNull());
       }
    }

   void readFrom(const datetime::DateTime& time, 
                 CcInt::inttype dummy1, bool dummy2, 
                 CcInt::inttype dummy3){
      readFrom(time);
   }


    MPrecCoordinate GetValue() const {
        MPrecCoordinate result(coord,&fracStorage, 1);
        return result;
    }

     inline virtual int NumOfFLOBs() const {
       return 1;
     }
     inline virtual Flob* GetFLOB( const int i ) {
       assert(i==0);
       return &fracStorage;   
     }

    static const string BasicType(){
      return type==datetime::instanttype?"precInstant":"precDuration";
    }

    static bool checkType(const ListExpr e){
       return listutils::isSymbol(e, BasicType());
    }

    static bool CheckKind(ListExpr typeList, ListExpr& errorInfo){
      return nl->IsEqual(typeList,BasicType());
    } 
     
    static ListExpr Property(){
       return gentc::GenProperty("-> DATA",
                          BasicType(),
                          "( scale (int64 fraction))",
                          "( 10 (1 '1/3'))");
     }


    /* reads this intant's value form a CcInt. The 
       Arguments useStr and digits are ignored and for compatibility 
       with other calls.
     */ 
     void readFrom(const CcInt& i, int scale, bool useStr, uint8_t digits = 16){
        if(!i.IsDefined() || scale<=0){
           SetDefined(false);
           return;
        }
        SetDefined(true);
        coord = PPrecCoordinate(i.GetValue()*scale,0);
        fracStorage.clean();
        this->scale = scale;    
     }
     
     void readFrom(const CcReal& r, int scale, bool useStr, 
                   uint8_t digits = 16){
        if(!r.IsDefined() || scale<=0){
           SetDefined(false);
           return;
        }
        SetDefined(true);
        if(!useStr){
           MPrecCoordinate c(r.GetValue());
            c *= scale;
            *this = c; 
            this->scale = scale;    
        } else {
          MPrecCoordinate res(0);
          res.readFromString(stringutils::double2str(r.GetValue(), digits),
                              scale); 
          *this = res;
        }
     }
     
     void readFrom(const Rational& r, int scale, bool useStr, 
                   uint8_t digits = 16){
        if(!r.IsDefined() || scale<=0){
           SetDefined(false);
           return;
        }
        SetDefined(true);
        MPrecCoordinate c(r);
        c *= scale;
        *this = c; 
        this->scale = scale;    
     }
     
     void readFrom(const LongInt& l, int scale, bool useStr, uint8_t digits=16){
        if(!l.IsDefined() || scale<=0){
           SetDefined(false);
           return;
        }
        this->scale = scale;    
        SetDefined(true);
        coord = PPrecCoordinate(l.GetValue()*scale,0);
        fracStorage.clean();
        this->scale = scale;    
     }
     
    void readFrom(const PrecCoord& p, int scale, bool useStr, 
                  uint8_t digits=16){
        *this = p;
    }

    void clear() {
       coord = PPrecCoordinate(0,0);
       fracStorage.clean();
       SetDefined(true);
    }


  private:
     DbArray<uint32_t> fracStorage;
     PPrecCoordinate coord;
};


/*
4 class PrecPoint

This class represents a precise 
point within the Secondo System.

*/

class PrecPoint: public StandardSpatialAttribute<2> {
  public:
 
      typedef MPrecPoint CType;


     PrecPoint() {}
     PrecPoint(const bool defined): 
            StandardSpatialAttribute(defined), 
             pos(0,0), fracStorage(0), scale(1) {}


    static string BasicType(){ 
      return "precPoint";
    }
   
    static bool checkType(const ListExpr e){
       return listutils::isSymbol(e, BasicType());
    }

    static bool CheckKind(ListExpr type, ListExpr& errorInfo){
      return nl->IsEqual(type,BasicType());
    } 
     
    static ListExpr Property(){
       return gentc::GenProperty("-> DATA",
                          BasicType(),
                          "(precise precise)",
                          "( (1 '1/3') (25) )");
     }

     PPrecCoordinate getX(){
         return pos.getX();
     }

     PPrecCoordinate getY() {
        return pos.getY();
     }

     CType GetValue() const{
        return CType(pos,&fracStorage, scale);
     }

     void set(const MPrecCoordinate& x, const MPrecCoordinate& y, 
              const uint32_t _scale){
         SetDefined(true);
         fracStorage.clean();
         x.appendFractional(&fracStorage);
         y.appendFractional(&fracStorage);
         pos.set(x,y);
         scale = _scale;
         assert(scale>0);
     }

     void set(CType& p){
        set(p.getX(),p.getY(), p.getScale());
     }

     
     inline virtual int NumOfFLOBs() const {
       return 1;
     }
     inline virtual Flob* GetFLOB( const int i ) {
       assert(i==0);
       return &fracStorage;   
     }

     ListExpr ToListExpr (ListExpr typeInfo){
         if(!IsDefined()){
            return listutils::getUndefined();
         }
         MPrecCoordinate x(pos.getX(),&fracStorage, scale);
         MPrecCoordinate y(pos.getY(),&fracStorage, scale);
         return nl->TwoElemList( 
                  nl->IntAtom(scale),
                  nl->TwoElemList(
                    ::toListExpr(x),
                    ::toListExpr(y)));
     }
   
     int compareTo(const PrecPoint& rhs)const{
        if(!IsDefined()){
           return rhs.IsDefined()?-1:0;
        }
        if(!rhs.IsDefined()){
            return 1;
        }
        CType t = GetValue();
        CType r = rhs.GetValue();  
        return t.compareTo(r);
     }
 
     int Compare(const Attribute* arg) const{
       return compareTo(*((PrecPoint*)arg));
     }

     bool Adjacent(const Attribute* arg) const{
        return false;
     }

     size_t Sizeof() const{
        return sizeof(*this);
     }


    size_t HashValue() const{
       if(!IsDefined()){
          return 0;
       }   
       return (size_t) pos.getHash();
    }

    void CopyFrom(const Attribute* arg){
       *this = *((PrecPoint*)arg);
    }

    PrecPoint* Clone() const{
       return new PrecPoint(*this);
    }

    std::string toString() const{
        if(!IsDefined()){
           return listutils::getUndefinedString();
        }
        CType c= GetValue();
        return c.toString();      
    }

     std::ostream& Print(std::ostream &os) const{
       return os << toString();
     }


    virtual const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const{
        assert(geoid==0);
        if(!IsDefined()){
          return Rectangle<2>(false);
        }
        CType p = GetValue();
        double min[] = {p.getX().toDouble(), p.getY().toDouble()};
        return Rectangle<2>(true,min,min);
    }

    virtual double Distance(const Rectangle<2>& rect,
                            const Geoid* geoid=0) const {
       assert(geoid==0);
       return sqrt(getQDistance(rect,GetValue()));
    } 
    
    virtual bool Intersects(const Rectangle<2>& rect,
                            const Geoid* geoid=0 ) const ;

    virtual bool IsEmpty() const {
       return !IsDefined();
     }


     bool ReadFrom(ListExpr LE, ListExpr typeInfo);

     void compScale(const MPrecCoordinate& s1,
                const MPrecCoordinate& s2,
                PrecPoint& result){
         if(!IsDefined()){
             result.SetDefined(false);
             return;
         }
         MPrecPoint p = GetValue();
         p.compScale(s1,s2);
         result.set(p);
     }
     
     void compTranslate(const MPrecCoordinate& t1,
                    const MPrecCoordinate& t2,
                    PrecPoint& result){
         if(!IsDefined()){
             result.SetDefined(false);
             return;
         }
         MPrecPoint p = GetValue();
         p.compTranslate(t1,t2);
         result.set(p);
     }

     void readFrom(const Point& p, int scale, bool useStr, uint8_t digits);
    
     void clear(){
        pos = PPrecPoint(0,0);
        fracStorage.clean(); 
        scale = 1;
     }

     size_t getNoElements() const{
       return IsDefined()?1:0;
     } 
    
     uint32_t getScale() const{
       return scale;
     }

  private:
     PPrecPoint pos;
     DbArray<uint32_t> fracStorage; 
     uint32_t scale;

};


/*
5 class PrecPoints

This class represents a set of precise points.

*/
class PrecPoints: public StandardSpatialAttribute<2>{
    
 public:

   typedef MPrecPoint ElemType;

   PrecPoints() {}
   PrecPoints(bool defined) :
      StandardSpatialAttribute(defined), gridPoints(0), 
      fracStorage(0), bbox(false), bulkloadStorage(0), scale(1) {}

   PrecPoints(int dummy) :
      StandardSpatialAttribute(false), gridPoints(0), 
      fracStorage(0), bbox(false), bulkloadStorage(0), scale(1){}

   PrecPoints(const PrecPoints& src):
     StandardSpatialAttribute(src), gridPoints(src.gridPoints.Size()),
     fracStorage(src.fracStorage.Size()), bbox(src.bbox) , 
     bulkloadStorage(0), scale(src.scale) {
          gridPoints.copyFrom(src.gridPoints);
          fracStorage.copyFrom(src.fracStorage);
          assert(src.bulkloadStorage==0);
     }

    PrecPoints& operator=(const PrecPoints& src){
       gridPoints.copyFrom(src.gridPoints);
       fracStorage.copyFrom(src.fracStorage);
       assert(bulkloadStorage==0);
       assert(src.bulkloadStorage==0);
       bbox = src.bbox; 
       scale = src.scale;
       return *this;
    }

   ~PrecPoints(){
        if(bulkloadStorage){
          bulkloadStorage->clear();
          delete bulkloadStorage;
        }
    }



   void clear(){
     gridPoints.clean();
     fracStorage.clean();
     bbox.SetDefined(false);
     if(bulkloadStorage){
        bulkloadStorage->clear();
     }
   }

  
    static string BasicType(){ 
      return "precPoints";
    }
   
    static bool checkType(const ListExpr e){
       return listutils::isSymbol(e, BasicType());
    }

    static bool CheckKind(ListExpr type, ListExpr& errorInfo){
      return nl->IsEqual(type,BasicType());
    } 
     
    static ListExpr Property(){
       return gentc::GenProperty("-> DATA",
                          BasicType(),
                          "(precPoint, precPoint, ...)",
                          "( ((1 '1/3') (25)) )");
     }

    
     size_t Size() const{
        if(bulkloadStorage){
             return bulkloadStorage->size();
        }
        return gridPoints.Size();
     }
     
     size_t size() const{
        if(bulkloadStorage){
             return bulkloadStorage->size();
        }
        return gridPoints.Size();
     }

     ElemType getPointAt(const int index) const{
        if(bulkloadStorage){
           return bulkloadStorage->at(index);
        }
        PPrecPoint p;
        gridPoints.Get(index,p);
        return MPrecPoint(p,&fracStorage, scale);
     }

     ElemType operator[](const int index) const{
        return getPointAt(index);
     }

 
     inline virtual int NumOfFLOBs() const {
       return 2;
     }

     inline virtual Flob* GetFLOB( const int i ) {
        assert(i>=0 && i<=1);
        if(i==0){
         return &gridPoints;
        } else {
          return &fracStorage;
        }
     }

     ListExpr ToListExpr (ListExpr typeInfo);
   
     int compareTo(const PrecPoints& rhs)const;
 
     int Compare(const Attribute* arg) const{
       return compareTo(*((PrecPoints*)arg));
     }

     bool Adjacent(const Attribute* arg) const{
        return false;
     }

     size_t Sizeof() const{
        return sizeof(*this);
     }


    size_t HashValue() const{
       if(!IsDefined()){
          return 0;
       }   
       int sum = 0;
       for(size_t i=0;i<min((size_t)5,Size());i++){
          sum += getPointAt(i).getHash();
       }
       return sum;
    }

    void CopyFrom(const Attribute* arg){
       *this = *((PrecPoints*)arg);
    }

    PrecPoints* Clone() const{
       return new PrecPoints(*this);
    }

     std::ostream& Print(std::ostream &os) const;

    virtual const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const{
        return bbox;
    }

    virtual double Distance(const Rectangle<2>& rect,
                            const Geoid* geoid=0) const ;
    
    virtual bool Intersects(const Rectangle<2>& rect,
                            const Geoid* geoid=0 ) const; 

    virtual bool IsEmpty() const {
       return !IsDefined() || Size()==0;
     }


     bool ReadFrom(ListExpr LE, ListExpr typeInfo);

     void startBulkLoad(const uint32_t _scale = 1){
       assert(_scale>0);
       clear();
       assert(bulkloadStorage==0);
       scale = _scale;
       bulkloadStorage = new vector<MPrecPoint>();  
     }

     void append(const MPrecPoint& p){
        assert(bulkloadStorage);
        if(scale!=p.getScale()){
          p.changeScaleTo(scale);
        }
        
        bulkloadStorage->push_back(p);
     }

     void append(const PrecPoint& p){
        assert(bulkloadStorage);
        assert(p.IsDefined());
        if(bulkloadStorage->empty()){
           scale = p.getScale();
        }
        append(p.GetValue());
     }

     void append(const PrecPoints& p){
        assert(bulkloadStorage);
        assert(p.IsDefined());
        if(bulkloadStorage->empty()){
           scale = p.getScale();
        }
        for(size_t i=0;i<p.Size();i++){
           append(p.getPointAt(i));
        }
     }



     void endBulkLoad(bool sort=true);

     void cancelBulkLoad(){
       assert(bulkloadStorage);
       delete bulkloadStorage;
       bulkloadStorage = 0;
     }


     void compScale(const MPrecCoordinate& s1, 
                const MPrecCoordinate& s2,
                 PrecPoints& result) const;

     void compTranslate(const MPrecCoordinate& t1, 
                    const MPrecCoordinate& t2,
                    PrecPoints& result) const;

/*
~contains~

Checks whether all points of ps are contained in this point set.

*/
     void contains(const PrecPoints& ps, CcBool& result) const;

/*
~contains~

Checks whether p is contained in this set.

*/

     void contains(const PrecPoint& p, CcBool& result) const;

/*
~intersects~

checks whether this point set and ps have any common points.

*/
     void intersects(const PrecPoints& ps, CcBool& result)const;

/*
~intersection~

computes the intersection between this point set and the argument

*/
    void intersection(const PrecPoints& ps, PrecPoints& result) const;

/*
~union~

compute the union of this and ps

*/    
    void compUnion(const PrecPoints& ps, PrecPoints& result)const;



/*
~difference~

compute the difference of this and ps

*/    
    void difference(const PrecPoints& ps, PrecPoints& result)const;
    

    void readFrom(const Points& points, int scale, bool useString, 
                  uint8_t digits);


    size_t getNoElements() const{
        return IsDefined()?gridPoints.Size():0;
    }

    uint32_t getScale() const{
      return scale;
    }

 private:
    DbArray<PPrecPoint> gridPoints;
    DbArray<uint32_t> fracStorage;
    Rectangle<2> bbox; 
    vector<MPrecPoint>* bulkloadStorage;
    uint32_t scale;



};


/*
5 Implementation of a precise line type


*/
class Line;

class PrecLine : public StandardSpatialAttribute<2> {

  public:
     typedef MPrecHalfSegment ElemType;


     PrecLine() {}
     PrecLine(bool defined) :
         StandardSpatialAttribute(defined), 
         bbox(false),
         scale(1), gridData(0),fracStorage(0), bulkloadStorage(0) {}

     PrecLine(int dummy) :
         StandardSpatialAttribute(false), bbox(false),
         scale(1), gridData(0), fracStorage(0), bulkloadStorage(0) {}

     PrecLine(const PrecLine& src) :
        StandardSpatialAttribute(src), bbox(src.bbox),
        scale(src.scale), gridData(src.gridData.Size()),
        fracStorage(src.fracStorage.Size()), bulkloadStorage(0){
           assert(src.bulkloadStorage==0);
           gridData.copyFrom(src.gridData);
           fracStorage.copyFrom(src.fracStorage); 
           assert(gridData.Size() == src.gridData.Size());
           assert(fracStorage.Size() == src.fracStorage.Size());  
     }


     PrecLine& operator=(const PrecLine& src){
        StandardSpatialAttribute::operator=(src);
        bbox = src.bbox;
        scale = src.scale;
        gridData.copyFrom(src.gridData);
        fracStorage.copyFrom(src.fracStorage);
        assert(bulkloadStorage==0);
        assert(src.bulkloadStorage==0);
        return *this;
     }

     size_t Size() const{
        return gridData.Size();
     }
     
     size_t size() const{
        return gridData.Size();
     }

     MPrecHalfSegment getHalfSegment(const size_t index) const{
       PPrecHalfSegment pps;
       gridData.Get(index,pps);
       MPrecHalfSegment res(pps,&fracStorage, scale, FIRST, true);
       return res;
     }

     inline MPrecHalfSegment operator[](const size_t index) const{
       return getHalfSegment(index);
     }    


     const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const{
         assert(geoid==0);
         return bbox;         
      } 

     
     bool Adjacent(const Attribute* arg) const{
       bool implemented = false;
       assert(implemented);
       return false;
     }

     size_t Sizeof() const{
        return sizeof(*this);
     }


    size_t HashValue() const{
       if(!IsDefined()){
          return 0;
       } 
       size_t sum = 0;
       for(size_t i=0;i<min((size_t)5,Size()); i++){
          PPrecHalfSegment hs;
          gridData.Get(i,hs);
          sum += hs.getHash();
       }
       return sum*scale;
    }

    int compareTo(const PrecLine& rhs)const;

    int Compare( const Attribute *rhs ) const{
      return compareTo(*((PrecLine*) rhs));
    }

    void CopyFrom(const Attribute* right) {
        *this = *( (PrecLine*) right);
    }

    PrecLine* Clone() const{
       return new PrecLine(*this);
    }

    ostream& Print(ostream& os ) const{
       os << "precLine " << endl;
       for(size_t i =0; i< Size(); i++){
           MPrecHalfSegment hs = getHalfSegment(i);
           os << hs << endl;
        }
        return os;
    }

    int NumOfFLOBs() const{
      return 2;
    } 
   
    Flob* GetFLOB( const int i ) {
      if(i==0) return &gridData;
      if(i==1) return &fracStorage;
      assert(false);
      return 0;
    }

    static string BasicType() {
       return "precLine";
    }

    static bool checkType(const ListExpr e){
       return listutils::isSymbol(e, BasicType());
    }    

    static bool CheckKind(ListExpr type, ListExpr& errorInfo){
      return nl->IsEqual(type,BasicType());
    }    
     
    static ListExpr Property(){
       return gentc::GenProperty("-> DATA",
                          BasicType(),
                          "( scale (hs1, hs2, ...))",
                          "( 100 ( (0 0 1 1 )))");
     }    


    virtual double Distance(const Rectangle<2>& rect,
                            const Geoid* geoid=0) const {
       assert(false);
       // TODO: Implement this function
       return 0;

    }; 
    
    virtual bool Intersects(const Rectangle<2>& rect,
                            const Geoid* geoid=0 ) const {
        assert(false);
        // TODO: Implement this function
        return false;
    } 

    virtual bool IsEmpty() const {
        return !IsDefined() || Size()==0;  
    } 

    ListExpr ToListExpr(ListExpr typeInfo) const;


    bool ReadFrom(ListExpr value, ListExpr typeInfo);


    void clear(){
       bbox.SetDefined(false);
       gridData.clean();
       fracStorage.clean(); 
    }


    void startBulkLoad(){
        assert(bulkloadStorage==0);
        clear(); // ensure to start from beginning
        bulkloadStorage = new vector<MPrecHalfSegment>();
    }
    
    void startBulkLoad(uint32_t newScale){
        assert(bulkloadStorage==0);
        assert(newScale>0);
        scale = newScale;
        clear(); // ensure to start from beginning
        bulkloadStorage = new vector<MPrecHalfSegment>();
    }

    void append(MPrecHalfSegment hs, bool autoTwice = true){
        assert(bulkloadStorage!=0);
        hs.bringToMemory();
        if(hs.getScale()!= scale){
             hs.changeScaleTo(scale);
        }
        if(autoTwice){
          size_t edgeno = bulkloadStorage->size()/2;
          hs.attributes.edgeno = edgeno;
        }
        hs.appendTo(&gridData,&fracStorage); 
        bulkloadStorage->push_back(hs);
        if(autoTwice){
          MPrecHalfSegment hs2(hs);
          hs2.switchLDP();
          bulkloadStorage->push_back(hs2);
        }
    }

    void append(const PrecLine& line){
      if(!IsDefined() || !line.IsDefined()){
         return;
      }
      assert(bulkloadStorage);
      for(size_t i=0;i<line.Size();i++){
        append(line.getHalfSegment(i));
      }

    }


    void endBulkLoad( bool realminize = true ); 

    void cancelBulkLoad(){
      assert(bulkloadStorage);
      delete bulkloadStorage;
      bulkloadStorage = 0;
    }


    void readFrom(const Line& line, int scale, bool useString, uint8_t digits);

    size_t getNoElements()const{
      return IsDefined()?gridData.Size()/2:0;
    }
    
    uint32_t getScale() const{
      return scale;
    }

    void compUnion(const PrecLine& l2, PrecLine& result) const;

    void intersection(const PrecLine& l2, PrecLine& result) const;

    void difference(const PrecLine& l2, PrecLine& result) const; 
    
    void intersects(const PrecLine& l2, CcBool& result);

  private:
    Rectangle<2> bbox;
    uint32_t scale;        // should be 10^x
    DbArray<PPrecHalfSegment> gridData;
    DbArray<uint32_t> fracStorage;

    vector<MPrecHalfSegment>* bulkloadStorage; // only used during bulkload
                                               // after end of bulkload 0
};





/*
6 Implementation of a precise Region

*/

class Region; 


class PrecRegion : public StandardSpatialAttribute<2> {

  public:

     typedef MPrecHalfSegment ElemType;

     PrecRegion() {}
     PrecRegion(bool defined) :
         StandardSpatialAttribute(defined), 
         bbox(false),
         scale(1), gridData(0),fracStorage(0), bulkloadStorage(0) {}

     PrecRegion(int dummy) :
         StandardSpatialAttribute(false), bbox(false),
         scale(1), gridData(0), fracStorage(0), bulkloadStorage(0) {}

     PrecRegion(const PrecRegion& src) :
        StandardSpatialAttribute(src), bbox(src.bbox),
        scale(src.scale), gridData(src.gridData.Size()),
        fracStorage(src.fracStorage.Size()), bulkloadStorage(0){
           assert(src.bulkloadStorage==0);
           gridData.copyFrom(src.gridData);
           fracStorage.copyFrom(src.fracStorage); 
     }


     PrecRegion& operator=(const PrecRegion& src){
        bbox = src.bbox;
        scale = src.scale;
        gridData.copyFrom(src.gridData);
        fracStorage.copyFrom(src.fracStorage);
        assert(fracStorage==0);
        assert(src.fracStorage==0);
        return *this;
     }

     size_t Size() const{
        return gridData.Size();
     }

     size_t size() const{
        return gridData.Size();
     }

     ElemType getHalfSegment(size_t index) const{
       PPrecHalfSegment pps;
       gridData.Get(index,pps);
       return MPrecHalfSegment(pps,&fracStorage, scale);
     }

     inline ElemType operator[](const size_t index) const{
       return getHalfSegment(index);
     }

     const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const{
         assert(geoid==0);
         return bbox;         
      } 

     
     bool Adjacent(const Attribute* arg) const{
       bool implemented = false;
       assert(implemented);
       return false;
     }

     size_t Sizeof() const{
        return sizeof(*this);
     }


    size_t HashValue() const{
       if(!IsDefined()){
          return 0;
       } 
       size_t sum = 0;
       for(size_t i=0;i<min((size_t)5,Size()); i++){
          PPrecHalfSegment hs;
          gridData.Get(i,hs);
          sum += hs.getHash();
       }
       return sum;
    }

    int compareTo(const PrecRegion& rhs)const;

    int Compare( const Attribute *rhs ) const{
      return compareTo(*((PrecRegion*) rhs));
    }

    void CopyFrom(const Attribute* right) {
        *this = *( (PrecRegion*) right);
    }

    PrecRegion* Clone() const{
       return new PrecRegion(*this);
    }

    ostream& Print(ostream& os ) const{
       os << "precRegion " << endl;
       for(size_t i =0; i< Size(); i++){
           MPrecHalfSegment hs = getHalfSegment(i);
           os << hs << endl;
        }
        return os;
    }

    int NumOfFLOBs() const{
      return 2;
    } 
   
    Flob* GetFLOB( const int i ) {
      if(i==0) return &gridData;
      if(i==1) return &fracStorage;
      assert(false);
      return 0;
    }

    static string BasicType() {
       return "precRegion";
    }

    static bool checkType(const ListExpr e){
       return listutils::isSymbol(e, BasicType());
    }    

    static bool CheckKind(ListExpr type, ListExpr& errorInfo){
      return nl->IsEqual(type,BasicType());
    }    
     
    static ListExpr Property(){
       return gentc::GenProperty("-> DATA",
                          BasicType(),
                          "( scale ( (cycle_1 hole_1_1 ...))...)",
                          "( 10 ( ( (0 0) (0 1) (1 1) (1 0))))");
     }    


    virtual double Distance(const Rectangle<2>& rect,
                            const Geoid* geoid=0) const {
       assert(false);
       // TODO: Implement this function
       return 0;

    }; 
    
    virtual bool Intersects(const Rectangle<2>& rect,
                            const Geoid* geoid=0 ) const {
        assert(false);
        // TODO: Implement this function
        return false;
    } 

    virtual bool IsEmpty() const {
        return !IsDefined() || Size()==0;  
    } 

    ListExpr ToListExpr(ListExpr typeInfo) const;


    bool ReadFrom(ListExpr value, ListExpr typeInfo);

    void clear(){
       bbox.SetDefined(false);
       gridData.clean();
       fracStorage.clean(); 
    }


    void startBulkLoad(){
        assert(bulkloadStorage==0);
        clear(); // ensure to start from beginning
        bulkloadStorage = new vector<MPrecHalfSegment>();
    }

    void append(MPrecHalfSegment& hs){
        assert(bulkloadStorage!=0);
        size_t edgeno = bulkloadStorage->size()/2;
        hs.attributes.edgeno = edgeno;
        bulkloadStorage->push_back(hs);
        hs.switchLDP();
        bulkloadStorage->push_back(hs);
    }

    bool endBulkLoad( bool sort = true,
                      bool setCoverageNo = true,
                      bool setPartnerNo = true,
                      bool computeRegion = true ); 

    void cancelBulkLoad();


    void readFrom(const Region& region, int scale, bool useString,
                  uint8_t digits);


    size_t getNoElements()const{
      return IsDefined()?gridData.Size()/2:0;
    }
    
    uint32_t getScale() const{
      return scale;
    }

    void setScale(const uint32_t scale){
      assert(scale>0);
      this->scale = scale;
    }

    void boundary(PrecLine& result){
       result.clear();
       if(!IsDefined()){
         result.SetDefined(false);
         return;
       }
       result.SetDefined(true);
       result.startBulkLoad(scale);
       for(int i=0;i<gridData.Size();i++){
           result.append(getHalfSegment(i),false);
       } 
       result.endBulkLoad(false);
    }

    void setOp(const PrecRegion& r, PrecRegion& result, 
               hstools::SETOP op) const;

    void compUnion(const PrecRegion& r, PrecRegion& result) const{
        setOp(r,result,hstools::UNION);
    }

  private:
    Rectangle<2> bbox;
    uint32_t scale;        // should be 10^x
    DbArray<PPrecHalfSegment> gridData;
    DbArray<uint32_t> fracStorage;

    vector<MPrecHalfSegment>* bulkloadStorage; // only used during bulkload
                                               // after end of bulkload 0

     /** Finds cycles in segments and sets faceno, cycleno and edgeno for 
       * ecah segment. Throws an exceptin if the halfsegments do not
       * form a valid region. 
       **/
     static void computeRegion(vector<MPrecHalfSegment>& segments);

     bool addFace(ListExpr face, int faceNo, int& edgeNo);
     bool addCycle(ListExpr cycle, int faceNo, int cycleNo, int& edgeNo);


};

/*
Implementation of the precTime class.

*/
template<int type>
ListExpr PrecTime<type>::ToListExpr(ListExpr typeInfo) const{

    int64_t ms = coord.getIntPart();
    int64_t julian = ms / MILLISECONDS; 
    ms = ms % MILLISECONDS;
    if(ms<0){
      ms=-ms;
    } 

   if(type==datetime::instanttype){
     int64_t year;
     int32_t month;
     int32_t day;
     datetime::DateTime::ToGregorian(julian,year,month,day);
      
     int32_t hour;
     int32_t minute;
     int32_t second;
     int32_t millisecond;
     millisecond = ms % 1000;
     ms = ms / 1000;
     second = ms % 60;
     ms /= 60;
     minute = ms % 60;
     ms /= 60;
     hour = ms;

     assert(hour < 24);
     stringstream ss;
     ss << year<<"-"<<month<<"-"<<day;
     if(hour>0 || minute>0 || second>0 || millisecond>0 || coord.hasFracPart()){
       ss << "-" <<  hour << ":" << minute;
       if(second>0 || millisecond>0 || coord.hasFracPart()){
         ss << ":" << second;
         if(millisecond>0 || coord.hasFracPart()){
           ss << "." << millisecond;
           if(coord.hasFracPart()){
              ss << "+" << *(GetValue().getUnscaledFrac());
           }
         }
       }  
     }
     return nl->TextAtom(ss.str());
   } else {
      if(coord.hasFracPart()){
        return nl->ThreeElemList( nl->IntAtom(julian) ,
                      nl->IntAtom(ms),
                      nl->TextAtom(GetValue().getUnscaledFrac()->get_str()));
      } else {
         return nl->TwoElemList( nl->IntAtom(julian),
                                 nl->IntAtom(ms));
      }
      
   }
}


template<int type>
bool  PrecTime<type>::ReadFrom(ListExpr instance, ListExpr typeExpr) {

     if(type==datetime::durationtype){
        int len = nl->ListLength(instance);
        if(len!=2 && len!=3) return false;
        if(nl->AtomType(nl->First(instance))!=IntType ||
           nl->AtomType(nl->Second(instance))!=IntType){
           return false;
        }
        int64_t julian = MILLISECONDS*nl->IntValue(nl->First(instance))+
                                      nl->IntValue(nl->Second(instance));
        if(len==2){
           setValue(julian);
           return true; 
        }
        if(nl->AtomType(nl->Third(instance))!=TextType){
           return false;
        }
        string frac = nl->Text2String(nl->Third(instance));
        try{
           mpq_class f(frac);
           MPrecCoordinate mc(julian,f);
           setValue(mc);
           return true;
        } catch(...){
         return false;
        }
     }  else { // instant type
       if(nl->AtomType(instance)!=TextType){
         return false;
       }
       return readFrom(nl->Text2String(instance));

    } 
}

template<int type>
bool PrecTime<type>::readFrom(const string& s){

   int sign = 1;
   int64_t year = 0;
   int64_t month = 0;
   int64_t day = 0;
   int64_t hour = 0;
   int64_t minute = 0;
   int64_t second = 0;
   int64_t millisecond = 0;

   mpq_class frac;

   int state = 0; // read year

   for(size_t i=0;i<s.length() && state < 10 ;i++){
     char c = s[i];
     switch(state){
       case 0 : if(c=='-'){ // year
                   if(i==0){
                     sign=-1;
                   } else {
                     state = 1;
                   }
                } else if(stringutils::isDigit(c)){
                   year = year*10 + stringutils::getDigit(c);
                } else {
                   return false;
                }
                break;
       case 1: if(c=='-'){ // month
                   state = 2;
               } else if(stringutils::isDigit(c)){
                  month=month*10 + stringutils::getDigit(c);
               } else {
                  return false;
               }
               break;
       case 2: if(c=='-'){ // day
                   state = 3;
               } else if(stringutils::isDigit(c)){
                  day = day*10 + stringutils::getDigit(c);
               } else {
                  return false;
               }
               break;
       case 3: if(c==':'){ // hour
                  state = 4;
               } else if(stringutils::isDigit(c)){
                  hour = hour*10 + stringutils::getDigit(c);
               } else {
                  return  false;
               }
               break;
       case 4 : if(c==':') { // minute
                  state = 5;
                } else if(stringutils::isDigit(c)){
                  minute = minute*10 + stringutils::getDigit(c);
                } else {
                   return false;
                }
                break;
       case 5 : if(c=='.'){ // second
                   state = 6;
                } else if(stringutils::isDigit(c)){
                   second = second*10 + stringutils::getDigit(c);
                } else {
                  return false;
                }
                break;
       case 6 : if(c=='+'){ // milli second
                  state = 7;
                } else if(stringutils::isDigit(c)){
                   millisecond = millisecond*10 + stringutils::getDigit(c);
                } else {
                   return  false;
                }
                break;
       case 7 : {
                string f = s.substr(i,s.size());
                if(!f.empty()){
                    try{
                      frac.set_str(f.c_str(),10);
                    } catch(...){
                       return false;
                    }
                }
                state = 10;
                }
                break;
       default : assert(false);
     }
   }


   // do some range checks
   if(    day > 31 || month>12  || hour>23 || minute>59 || second>59 
       || millisecond>999 || frac > 1){
     return false;
   }
   int64_t julian = datetime::DateTime::ToJulian(sign*year,month,day);
   julian *= MILLISECONDS;
   int64_t t = hour;
   t = t*60 + minute;
   t = t * 60 + second;
   t = t*1000 + millisecond;
   julian += t;
   if(frac>0){
      MPrecCoordinate mc(julian,frac);
      setValue(mc);
   } else {
      setValue(julian);
   }
   return true;
}

typedef PrecTime<datetime::instanttype> precInstant;
typedef PrecTime<datetime::durationtype> precDuration;



#endif


