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

4 Enumeration State

5 Class RationalPoint3DExt

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
6 Class RationalPoint3DExtSet

*/     
    RationalPoint3DExtSet::RationalPoint3DExtSet(){
    }// Konstruktor

    void RationalPoint3DExtSet::insert(const RationalPoint3DExt& point){
      this->points.insert(point);
    }// insert

    size_t RationalPoint3DExtSet::size() const{
      return this->points.size();
    }// size

    bool RationalPoint3DExtSet::getIntersectionSegment(
        RationalSegment3D& result)const{
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
      result = RationalSegment3D(point2, point3);
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
7 Class RationalPlane3D

*/      
    RationalPlane3D::RationalPlane3D():normalVector(),pointOnPlane(){  
    }// Konstruktor
   
    RationalPlane3D::RationalPlane3D(const RationalPlane3D& plane){
      set(plane);
    }// konstruktor
   
    RationalPlane3D::RationalPlane3D(const PFace& pf){
      RationalPoint3D a = pf.getA().getR();
      RationalPoint3D b = pf.getB().getR();
      RationalPoint3D c = pf.getC().getR();
      RationalPoint3D d = pf.getD().getR();
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
      // The vector w is either the normalized cross product 
      // of the normal vector and the t-unit-vector, or it's opposite.
      // This depends on the kind of set-operation, we want to perform.
      //
      // wVector = Vector3D(GetNormalVector() ^ Vector3D(0.0, 0.0, 1.0);
      // wVector.Normalize();
      // This can be simplified to:
      wVector = RationalVector3D( normalVector.getY(), 
                                - normalVector.getX(),
                                  0.0);
      wVector.normalize();
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
      return NumericUtil::nearlyEqual(distance2ToPlane(plane.pointOnPlane),
                                      0.0);
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
    
    bool RationalPlane3D::isLeftAreaInner(const RationalSegment3D segment,
                                          const RationalPlane3D other)const{
      RationalVector3D segmentVector(segment.getHead() - segment.getTail());
      segmentVector.normalize();
      RationalVector3D vector = this->normalVector ^ segmentVector;
      if((vector * other.normalVector) < 0) return true;
      return false;
    }// isLeftAreaInner
                       
    Point2D RationalPlane3D::transform(const RationalPoint3D& point) const{
      // check point d on plane
      if(!NumericUtil::nearlyEqual(distance2ToPlane(point),0.0)){
           NUM_FAIL("Point isn,t located on plane.");
      }// if
      mpq_class w = point.getX() * wVector.getX() +
                    point.getY() * wVector.getY();
      return Point2D(w.get_d(),point.getZ().get_d());  
    }// transform
      
    Segment2D RationalPlane3D::transform(
        const RationalSegment3D& segment) const{  
      return Segment2D(transform(segment.getTail()),
                       transform(segment.getHead())); 
    }// transform
/*
8 Class IntersectionPoint

*/   
    IntersectionPoint::IntersectionPoint():x(0),y(0),z(0),w(0){
    }// Konstruktor
   
    IntersectionPoint::IntersectionPoint(const IntersectionPoint& point){
      set(point);
    }// Konstruktor
    
    IntersectionPoint::IntersectionPoint(const Point3D& point3D, 
                                         const Point2D& point2D){
      if(!(point3D.getZ() == point2D.getY())){
        NUM_FAIL("Point3D and Point2D don't discribe the same.");
      }// if
      this->x = point3D.getX();
      this->y = point3D.getY();
      this->z = point3D.getZ();
      this->w = point2D.getX();
    }// Konstruktor
    
    IntersectionPoint::IntersectionPoint(double x, double y, double z, 
                                         double w){
      this->x = x;
      this->y = y;
      this->z = z;
      this->w = w; 
    }// Konstruktor
    
    void IntersectionPoint::set(const IntersectionPoint& point){
      this->x = point.x;
      this->y = point.y;
      this->z = point.z;
      this->w = point.w;      
    }// set
    Point3D IntersectionPoint::getPoint3D() const{
      return Point3D(x,y,z);
    }// getPoint3D
      
    Point2D IntersectionPoint::getPoint2D() const{
      return Point2D(w,z);
    }// getPoint2D
      
    double IntersectionPoint::getX()const{
      return x;
    }// getX
      
    double IntersectionPoint::getY()const{
      return y;
    }// getY
      
    double IntersectionPoint::getZ()const{
      return z;
    }// getZ
      
    double IntersectionPoint::getW()const{
      return w;        
    }// getW
      
    double IntersectionPoint::getT()const{
      return z;    
    }// getT
          
    std::ostream& operator <<(std::ostream& os, 
                              const IntersectionPoint& point){
      os << "IntersectionPoint (" << point.x << ", " << point.y << ", ";
      os << point.z <<", " <<point.w <<")";
      return os; 
    }// OPerator <<
    
    IntersectionPoint& IntersectionPoint::operator =(
        const IntersectionPoint& point){
      set(point);
      return *this;
    }// Operator =
    
    bool IntersectionPoint::operator ==(const IntersectionPoint& point) const{
      return NumericUtil::nearlyEqual(this->x, point.x) && 
             NumericUtil::nearlyEqual(this->y, point.y) && 
             NumericUtil::nearlyEqual(this->z, point.z) &&
             NumericUtil::nearlyEqual(this->w, point.w);
    }// Operator ==
/*
9 Class IntersectionSegment

*/  
    IntersectionSegment::IntersectionSegment(){
      this->leftAreaIsInner = false;   
    }// Konstruktor
    
    IntersectionSegment::IntersectionSegment(
        const IntersectionSegment& segment){
      set(segment);    
    }// konstruktor    
    
    IntersectionSegment::IntersectionSegment(const IntersectionPoint& tail,
                                             const IntersectionPoint& head,
                                             bool leftAreaIsInner){
      this->tail = tail;
      this->head = head;
      this->leftAreaIsInner = leftAreaIsInner; 
    }// konstruktor
    
    IntersectionSegment::IntersectionSegment(const Segment3D& segment3D, 
                                             const Segment2D& segment2D,
                                             bool leftAreaIsInner){
      this->head = IntersectionPoint(segment3D.getHead(),segment2D.getHead());
      this->tail = IntersectionPoint(segment3D.getTail(),segment2D.getTail());
      this->leftAreaIsInner = leftAreaIsInner;   
    }// Konstruktor
      
    void IntersectionSegment::set(const IntersectionSegment& segment){
      this->tail = segment.tail;
      this->head = segment.head;
      this->leftAreaIsInner = segment.leftAreaIsInner; 
    }// set
      
    Segment3D IntersectionSegment::getSegment3D()const{
      return Segment3D(tail.getPoint3D(),head.getPoint3D());
    }// getSegment3D
    
    Segment2D IntersectionSegment::getSegment2D()const{
      return Segment2D(tail.getPoint2D(),head.getPoint2D());
    }// getSegment2D
    
    IntersectionPoint IntersectionSegment::getTail() const{
      return tail;
    }// getTail
    
    IntersectionPoint IntersectionSegment::getHead() const{
      return head;   
    }// getHead
    
    bool IntersectionSegment::isleftAreaInner()const{
      return leftAreaIsInner;
    }// getSegment3D
    
    std::ostream& operator <<(
        std::ostream& os, const IntersectionSegment& segment){
      os << "IntersectionSegment (" << segment.tail << ", " << segment.head;
      if(segment.leftAreaIsInner) os << ", Left is inner.)";
      else  os << ", Right is inner.)";
      return os;  
    }// Operator <<
    
    IntersectionSegment& IntersectionSegment::operator =(
        const IntersectionSegment& segment){
       set(segment); 
       return *this;
    }// OPerator =
    
    bool IntersectionSegment::operator ==(
        const IntersectionSegment& segment) const{
      return this->head == segment.head && 
             this->tail == segment.tail &&
             this->leftAreaIsInner == segment.leftAreaIsInner;
    }// Operator ==   
/*
10 Struct IntSegCompare

*/
    bool IntSegCompare::operator()(const IntersectionSegment* const& segment1,
                                   const IntersectionSegment* const& segment2)
        const{
      IntersectionPoint tail1 = segment1->getTail();
      IntersectionPoint tail2 = segment2->getTail();
      IntersectionPoint head1 = segment1->getHead();
      IntersectionPoint head2 = segment2->getHead();
      // We sort by (t_start, w_start, IsLeft())
      // Precondition:  tail1.getT() < haed1.getT() &&
      //                tail2.getT() < head2.getT() 
      if (NumericUtil::lower(  tail1.getT(), tail2.getT())) return true;
      if (NumericUtil::greater(tail1.getT(), tail2.getT())) return false;
      // tail1.getT() == tail2.getT()
      if (NumericUtil::lower(  tail1.getW(), tail2.getW())) return true;
      if (NumericUtil::greater(tail1.getW(), tail2.getW())) return false;
      // tail1.getW() == tail2.GetW()  
      if (head1.getPoint2D() == head2.getPoint2D()) return true;    
      return segment2->getSegment2D().isLeft(head1.getPoint2D());
    }// IntSegCompare
    
   
    IntSegContainer::IntSegContainer(){
      
    }// Konstrktor

    IntSegContainer::~IntSegContainer(){  
      set<IntersectionSegment*>::iterator iter;
      for (iter = intSegs.begin(); iter != intSegs.end(); ++iter){
        delete *iter;
      }// for  
    }// Destruktor
 
    void IntSegContainer::addIntSeg(IntersectionSegment* seg){
       intSegs.insert(seg);
    }// addIntSeg
  
    std::ostream& operator <<(std::ostream& os, 
                              const IntSegContainer& container){
      os << "IntSegContainer (";
      if (container.intSegs.empty()) {  
        os << "is empty)" << endl;
      }// if
      else {        
        set<IntersectionSegment*>::iterator iter;
        for (iter = container.intSegs.begin(); 
             iter != container.intSegs.end(); ++iter) {
          os << endl << "  "<< *(*iter);
        }// for
        os << ")" <<endl;
      }// else
      return os;
    }// operator 
     
    bool IntSegContainer::operator ==(const IntSegContainer& container){
      if(this->intSegs.size() != container.intSegs.size()) return false;
      set<IntersectionSegment*>::iterator iter1, iter2;
      for(iter1  = this->intSegs.begin(), iter2 =  container.intSegs.begin();
          iter1 != this->intSegs.end() && iter2 != container.intSegs.end();
          ++iter1, ++iter2){
        if(!(*(*iter1) == *(*iter2))) return false;
      }// for
      return true;
    }// Operator ==  
   
/*
12 Class PFace

*/            
    PFace::PFace(const Point3D& a, const Point3D& b, const Point3D& c, 
                 const Point3D& d){
      this->state = UNKNOWN;
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
      return this->a;
    }// getA
    
    Point3D PFace::getB() const{
      return this->b;
    }// getB
    
    Point3D PFace::getC() const{
      return this->c;
    }// getC
    
    Point3D PFace::getD() const{
      return this->d;
    }// getD    
    
    State PFace::getState() const{
      return state;
    }// getState
    
    string PFace::toString(State state){
      switch(state){
        case UNKNOWN:               return "UNKNOWN";
        case ENTIRELY_INSIDE:       return "ENTIRELY_INSIDE";
        case ENTIRELY_OUTSIDE:      return "ENTIRELY_OUTSIDE";
        case RELEVANT_NOT_CRITICAL: return "RELEVANT_NOT_CRITICAL";
        case RELEVANT_CRITICAL:     return "RELEVANT_CRITICAL";
        case NOT_RELEVANT:          return "NOT_RELEVANT";
        default: return "";
      }// switch
    }// toString
    
    std::ostream& operator <<(std::ostream& os, const PFace& pf){
      os << "PFace ("<< pf.a << ", " << pf.b << ", " << pf.c << ", " << pf.d; 
      os << "," << PFace::toString(pf.getState()) << "," <<endl;
      os << " " << pf.intSegContainer <<")" << endl;
      return os;   
    }// Operator <<
    
    void PFace::addIntSeg(const RationalPlane3D &planeSelf, 
                          const RationalPlane3D &planeOther,
                          const RationalSegment3D &intSeg){
      bool result = planeSelf.isLeftAreaInner(intSeg,planeOther);
      Segment2D segment = planeSelf.transform(intSeg);
      this->state = RELEVANT_NOT_CRITICAL;
      this->intSegContainer.addIntSeg(
        new IntersectionSegment(intSeg,segment,result));              
    }// addIntSeg
  
    bool PFace::intersection(PFace& other){
      // check bounding rectangles
      if(!(this->boundingRect.Intersects(other.boundingRect))){
        // cout << "No intersect bounding rectangles found." << endl;  
        return false; 
      }// if
      // create planes
      RationalPlane3D planeSelf(*this);
      RationalPlane3D planeOther(other);
      // check planes
      if (planeSelf.isParallelTo(planeOther)) {
        if(planeSelf.isCoplanarTo(planeOther)) {
          this->state = RELEVANT_CRITICAL;
          other.state = RELEVANT_CRITICAL;
          // cout << "Coplanar plane pair found." << endl;            
        }// if 
        else {
          // cout << "Parallel plane pair found." << endl;
        }// else
        return false;
      }// if
      RationalPoint3DExtSet intPointSet;
      planeSelf.intersection(other, PFACE_A, intPointSet);
      // We need exactly two intersection points.
      if (intPointSet.size() != 2) return false; 
      planeOther.intersection(*this, PFACE_B, intPointSet);  
      // There is no intersection
      RationalSegment3D intSeg;
      if(!intPointSet.getIntersectionSegment(intSeg)) return false;  
      // create and save result segments
      this->addIntSeg(planeSelf,planeOther,intSeg);
      other.addIntSeg(planeOther,planeSelf,intSeg);
      
//       bool result = planeSelf.isLeftAreaInner(intSeg,planeOther);
//       Segment2D segment = planeSelf.transform(intSeg);
//       this->state = RELEVANT_NOT_CRITICAL;
//       this->intSegContainer.addIntSeg(
//         new IntersectionSegment(intSeg,segment,result));
//       
//       result  = planeOther.isLeftAreaInner(intSeg,planeSelf);
//       segment = planeOther.transform(intSeg);
//       other.state = RELEVANT_NOT_CRITICAL;
//       other.intSegContainer.addIntSeg(
//         new IntersectionSegment(intSeg,segment,result));
      
      return true;    
    }// intersection    
  } // end of namespace mregionops3
} // end of namespace temporalalgebra
