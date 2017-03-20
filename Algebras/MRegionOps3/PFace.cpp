/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]
//[x] [$\times $]
//[->] [$\rightarrow $]

[1] Implementation of the MRegionOpsAlgebra

April - November 2008, M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "PFace.h"

using namespace std;

namespace temporalalgebra {
  namespace mregionops3 {
/*
3 Enumeration SourceFlag

4 Class RationalPoint3DExt

*/      
    RationalPoint3DExt::RationalPoint3DExt():RationalPoint3D(){
    }// Konstruktor
    
    RationalPoint3DExt::RationalPoint3DExt(const mpq_class& a, 
                                           const mpq_class& b, 
                                           const mpq_class& c,
                                           SourceFlag _sourceFlag):
        RationalPoint3D(a,b,c),sourceFlag(_sourceFlag){
    }// Konstruktor
    
    void RationalPoint3DExt::setSourceFlag(SourceFlag flag){
      this->sourceFlag = flag;
    }// setSourceFlag

    SourceFlag RationalPoint3DExt::getSourceFlag()const{
      return this->sourceFlag;
    }// getSourceFlag
    
    bool RationalPoint3DExt::operator < (const RationalPoint3DExt& point)const{
      if (NumericUtil::lower(this->x,  point.x))return true;
      if (NumericUtil::greater(this->x,point.x))return false;
      if (NumericUtil::lower(this->y,  point.y))return true;
      if (NumericUtil::greater(this->y,point.y))return false;
      if (NumericUtil::lower(this->z,  point.z))return true;
      if (NumericUtil::greater(this->z,point.z))return false;
      return this->sourceFlag < point.sourceFlag;
    }// Operator <
    
    std::ostream& operator <<(std::ostream& os, 
                              const RationalPoint3DExt& point){
      os << "RationalPoint3DExt (" << point.x.get_d();
      os << ", " << point.y.get_d();
      os << ", " << point.z.get_d() <<", ";
      if(point.sourceFlag == PFACE_A) os << "PFACE_A)";
      else os << "PFACE_B)";
      return os; 
    }// operator <<
/*
5 Class RationalPoint3DExtSet

*/     
    RationalPoint3DExtSet::RationalPoint3DExtSet(){
    }// Konstruktor

    void RationalPoint3DExtSet::insert(const RationalPoint3DExt& point){
      this->points.insert(point);
    }// insert

    size_t RationalPoint3DExtSet::size() const{
      return this->points.size();
    }// size

    bool RationalPoint3DExtSet::getIntersectionSegment(Segment3D& result)const{
      if (this->points.size() != 4) return false;
      set<RationalPoint3DExt>::iterator it = this->points.begin();
      RationalPoint3DExt point1 = *it;
      it++;
      RationalPoint3DExt point2 = *it;
      if (point1.getSourceFlag() == point2.getSourceFlag()) return false;
      it++;
      RationalPoint3DExt point3 = *it;
      if (point2 == point3) {
        // The length of the intersection segment is zero.
        return false;
      }// if
      result = Segment3D(point2.get(), point3.get());
      return true;
    }// getIntersectionSegment

    std::ostream& operator <<(std::ostream& os, 
                              const RationalPoint3DExtSet& points){
      set<RationalPoint3DExt>::iterator iter;
      os << "RationalPoint3DExtSet(" << endl;
      for(iter = points.points.begin(); iter != points.points.end(); ++iter){
        os << "  " << *iter << endl;
      }// for
      os << ")" << endl;
      return os;
    }// operator <<
/*
6 Class RationalPlane3D

*/      
    RationalPlane3D::RationalPlane3D():normalVector(),pointOnPlane(){  
    }// Konstruktor
   
    RationalPlane3D::RationalPlane3D(const RationalPlane3D& plane){
      set(plane);
    }// konstruktor
   
    RationalPlane3D::RationalPlane3D(const PFace& pf){
      RationalPoint3D a = pf.getA().get();
      RationalPoint3D b = pf.getB().get();
      RationalPoint3D c = pf.getC().get();
      RationalPoint3D d = pf.getD().get();
      this->pointOnPlane = a;
      // We compute the normalvector
      if (a != b) {
        // Cross product of vector ab and ac
        this->normalVector = (b - a) ^ (c - a);
        // check point d on plane
        if(!NumericUtil::nearlyEqual(distance2ToPlane(d),0.0)){
           NUM_FAIL("Not all points from the pface are located on plane.");
        }// if
      }// if
      else { // A == B
        // Cross product of vector dc and db:
        this->normalVector = (c - d) ^ (b - d);
      }// else
      this->normalVector.normalize();
    }// Konstruktor
      
    void RationalPlane3D::set(const RationalPlane3D& plane){
      this->normalVector = plane.normalVector;
      this->pointOnPlane = plane.pointOnPlane; 
    }// set
      
    void RationalPlane3D::set(const RationalVector3D& normalVector,
                              const RationalPoint3D& pointOnPlane){
      this->normalVector = normalVector;
      this->pointOnPlane = pointOnPlane; 
    }// set
   
    RationalPoint3D RationalPlane3D::getPointOnPlane() const{
      return this->pointOnPlane;  
    }// getPointOnPlane
        
    RationalVector3D RationalPlane3D::getNormalVector() const{
      return this->normalVector;
    }// getNormalVector
        
    mpq_class RationalPlane3D::distance2ToPlane(const RationalPoint3D& point)
        const{
      mpq_class n =  - (this->normalVector * (point - this->pointOnPlane));
      mpq_class d = this->normalVector * this->normalVector;
      mpq_class b = n / d;
      RationalPoint3D result = point + b * this->normalVector;
      return result.distance2(point);
    }// distance2ToPlane
      
    bool RationalPlane3D::isParallelTo(const RationalPlane3D& plane) const{
      RationalVector3D cross = this->normalVector ^ plane.normalVector;
      return NumericUtil::nearlyEqual(cross.length2(),0.0);
    }// isParallelTo
    
    bool RationalPlane3D::isCoplanarTo(const RationalPlane3D& plane) const{
      return NumericUtil::nearlyEqual(distance2ToPlane(plane.pointOnPlane),0.0);
    }// isCoplanarTo
          
    bool RationalPlane3D::intersection(const Segment3D segment, 
                                       RationalPoint3D& result)const{
      RationalPoint3D head = segment.getHead();
      RationalPoint3D tail = segment.getTail();
      // We compute the intersection point of the plane 
      // - defined by the PFace - and the segment.
      RationalVector3D u = head - tail;
      RationalVector3D w = tail - this->pointOnPlane;
      mpq_class d = this->normalVector * u;
      mpq_class n = - this->normalVector * w;
      // Segment is parallel to plane ?
      if (NumericUtil::nearlyEqual(d, 0.0))  return false;
      mpq_class s = n / d;
      // No intersection point, if s < -eps or s > 1 + eps.
      if (NumericUtil::lower(s, 0.0) || NumericUtil::greater(s, 1.0)) 
        return false;
      // Compute segment intersection point
      result = tail + s * u;
      return true;                                
    }// intersection
                                    
    RationalPlane3D& RationalPlane3D::operator =(const RationalPlane3D& plane){
      set(plane);
      return *this;
    }// Operator =      
      
    std::ostream& operator <<(std::ostream& os, 
                              const RationalPlane3D& plane){
      os << "RationalPlane3D ( "<< plane.normalVector << ", ";
      os << plane.pointOnPlane <<")";
      return os;   
    }// operator << 
    
    void RationalPlane3D::intersection(const PFace& other,
         SourceFlag sourceFlag, RationalPoint3DExtSet& intPointSet)const{
      Point3D a = other.getA();
      Point3D b = other.getB();
      Point3D c = other.getC();
      Point3D d = other.getD();
      RationalPoint3DExt intPoint;
      vector<Segment3D> edgesPFace;
      // We store all edges of this PFace as 3DSegments in the vector 
      // edgesPFace.
      edgesPFace.push_back(Segment3D(a, c));
      edgesPFace.push_back(Segment3D(b, d));
      if (a != b){
        edgesPFace.push_back(Segment3D(a,b));
      }// if
      if (c != d){
        edgesPFace.push_back(Segment3D(c,d));
      }// if
      // Intersect the plane - defined by the other PFace - 
      // with all edges of this PFace:
      for(size_t i = 0, j = 0 ; j < 2 && i < edgesPFace.size();i++){
        if(intersection(edgesPFace[i], intPoint)){
          intPoint.setSourceFlag(sourceFlag);
          intPointSet.insert(intPoint);
          j++;
        }// if
      }// for      
    }// intersection
/*
7 Class PFace

*/            
    PFace::PFace(const Point3D& a, const Point3D& b, const Point3D& c, 
                 const Point3D& d){
      this->a = a;
      this->b = b;
      this->c = c;
      this->d = d;
      // BoundingRect bestimmen
      pair<double, double> x = NumericUtil::minMax4(a.getX(),b.getX(),
                                                    c.getX(),d.getX());
      pair<double, double> y = NumericUtil::minMax4(a.getY(),b.getY(),
                                                    c.getY(),d.getY());
      boundingRect = Rectangle<2>(true, x.first  - NumericUtil::eps2, 
                                        x.second + NumericUtil::eps2, 
                                        y.first  - NumericUtil::eps2, 
                                        y.second + NumericUtil::eps2);
    }// Konstruktor
      
    Point3D PFace::getA() const{
      return a;
    }// getA
    
    Point3D PFace::getB() const{
      return b;
    }// getB
    
    Point3D PFace::getC() const{
      return c;
    }// getC
    
    Point3D PFace::getD() const{
      return d;
    }// getD    
    
    std::ostream& operator <<(std::ostream& os, const PFace& pf){
      os << "PFace ( "<< pf.a << ", " << pf.b << ", " << pf.c << ", " << pf.d;
      os <<")";
      return os;   
    }// Operator <<
             
    bool PFace::intersection(const PFace& other,Segment3D& intSeg)const{
      // check bounding rectangles
      if(!(this->boundingRect.Intersects(other.boundingRect))){
        cout << "No intersect bounding rectangles found." << endl;  
        return false; 
      }// if
      // create planes
      RationalPlane3D planeSelf(*this);
      RationalPlane3D planeOther(other);
      // check planes
      if (planeSelf.isParallelTo(planeOther)) {
        if(planeSelf.isCoplanarTo(planeOther)) {
          // this->MarkAsCriticalRelevant();
          // other.MarkAsCriticalRelevant();
          cout << "Coplanar plane pair found." << endl;            
        }// if 
        else {
          cout << "Parallel plane pair found." << endl;
        }// else
        return false;
      }// if
      RationalPoint3DExtSet intPointSet;
      planeSelf.intersection(other, PFACE_A, intPointSet);
      // We need exactly two intersection points.
      if (intPointSet.size() != 2) return false; 
      planeOther.intersection(*this, PFACE_B, intPointSet);  
      // There is no intersection
      if(!intPointSet.getIntersectionSegment(intSeg)) return false;    
      return true;
    }// intersection    

  } // end of namespace mregionops3
} // end of namespace temporalalgebra
