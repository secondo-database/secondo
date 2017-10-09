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
Mai - November 2017, U. Wiesecke for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "SetOps.h"

using namespace std;

namespace temporalalgebra {
  namespace mregionops3 {
    
/*
3 Enumeration 

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
      if (NumericUtil::lower(this->x,  point.x)) return true;
      if (NumericUtil::greater(this->x,point.x)) return false;
      if (NumericUtil::lower(this->y,  point.y)) return true;
      if (NumericUtil::greater(this->y,point.y)) return false;
      if (NumericUtil::lower(this->z,  point.z)) return true;
      if (NumericUtil::greater(this->z,point.z)) return false;
      return this->sourceFlag < point.sourceFlag;
    }// Operator <
    
    std::ostream& operator <<(std::ostream& os, 
                              const RationalPoint3DExt& point){
      os << "RationalPoint3DExt(" << point.x.get_d();
      os << ", " << point.y.get_d();
      os << ", " << point.z.get_d() <<", ";
      if (point.sourceFlag == UNIT_A) os << "UNIT_A)";
      else os << "UNIT_B)";
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
      return points.print(os,"");
    }// operator <<

    std::ostream& RationalPoint3DExtSet::print(std::ostream& os, 
                                               std::string prefix)const{
      set<RationalPoint3DExt>::const_iterator iter;
      os << "RationalPoint3DExtSet(" << endl;
      for (iter = points.begin(); iter != points.end(); ++iter) {
        os << prefix << "  " << *iter << endl;
      }// for
      os << prefix << ")" << endl;
      return os;
    }// print
    
/*
6 Class RationalPlane3D

*/      
    RationalPlane3D::RationalPlane3D():normalVector(),pointOnPlane(){  
    }// Konstruktor
   
    RationalPlane3D::RationalPlane3D(const RationalPlane3D& plane){
      set(plane);
    }// konstruktor
    

    RationalPlane3D::RationalPlane3D(const PFace& pf){
      RationalPoint3D leftStart = pf.getLeftStart().getR();
      RationalPoint3D leftEnd   = pf.getLeftEnd().getR();
      RationalPoint3D rightStart= pf.getRightStart().getR();
      RationalPoint3D rightEnd  = pf.getRightEnd().getR();
      this->pointOnPlane = leftStart;
      // We compute the normalvector
      if (leftStart != rightStart) {
        // Cross product of vector ab and ac
        this->normalVector = (rightStart - leftStart) ^ (leftEnd - leftStart);
        // check point d on plane
        if (!NumericUtil::nearlyEqual(distance2ToPlane(rightEnd),0.0)) {
           NUM_FAIL("Not all points from the pface are located on plane.");
        }// if
      }// if
      else { // A == B
        // Cross product of vector dc and db:
        this->normalVector = (leftEnd - rightEnd) ^ (rightStart - rightEnd);
      }// else
      this->normalVector.normalize();
      // The vector w is either the normalized cross product 
      // of the normal vector and the t-unit-vector, or it's opposite.
      // This depends on the kind of set-operation, we want to perform.
      //
      // wVector = Vector3D(normalVector() ^ Vector3D(0.0, 0.0, -1.0);
      // wVector.normalize();
      // This can be simplified to:
      this->wVector = RationalVector3D( - normalVector.getY(), 
                                          normalVector.getX(),
                                          0.0);      
      this->wVector.normalize(); 
    }// Konstruktor
      
    void RationalPlane3D::set(const RationalPlane3D& plane){
      this->normalVector   = plane.normalVector;
      this->pointOnPlane   = plane.pointOnPlane; 
      this->wVector        = plane.wVector; 
    }// set
      
    void RationalPlane3D::set(const RationalVector3D& normalVector,
                              const RationalPoint3D& pointOnPlane){
      this->normalVector   = normalVector;
      this->pointOnPlane   = pointOnPlane; 
      this->wVector = RationalVector3D( - normalVector.getY(), 
                                          normalVector.getX(),
                                          0.0);      
      this->wVector.normalize();       
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
      if (NumericUtil::nearlyEqual(d,0.0)) NUM_FAIL("Normalvector is zerro.");
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

      mpq_class test = 0.000001;      
      if(NumericUtil::between(   -test, s, test))     s = 0.0;
      if(NumericUtil::between(1 - test, s, 1 + test)) s = 1.0; 
      
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
      os << "RationalPlane3D("<< plane.normalVector << ", ";
      os << plane.pointOnPlane <<")";
      return os;   
    }// operator << 
    
    void RationalPlane3D::intersection(const PFace& other,
         SourceFlag sourceFlag, RationalPoint3DExtSet& intPointSet)const{
      Point3D leftStart  = other.getLeftStart();
      Point3D leftEnd    = other.getLeftEnd();
      Point3D rightStart = other.getRightStart();
      Point3D rightEnd   = other.getRightEnd();
      RationalPoint3DExt intPoint;
      vector<Segment3D> edgesPFace;
      // We store all edges of this PFace as 3DSegments in the vector 
      // edgesPFace.
      edgesPFace.push_back(Segment3D(leftStart, leftEnd));
      edgesPFace.push_back(Segment3D(rightStart, rightEnd));
      if (leftStart != rightStart) {
        edgesPFace.push_back(Segment3D(leftStart,rightStart));
      }// if
      if (leftEnd != rightEnd) {
        edgesPFace.push_back(Segment3D(leftEnd,rightEnd));
      }// if
      // Intersect the plane - defined by the other PFace - 
      // with all edges of this PFace:
      RationalPoint3DExt temp;
      for (size_t i = 0, j = 0 ; j < 2 && i < edgesPFace.size();i++) {        
        if (intersection(edgesPFace[i], intPoint)) {          
          intPoint.setSourceFlag(sourceFlag);
          intPointSet.insert(intPoint); 
          if (j == 0) { 
            temp = intPoint;
            j++;
          }// if
          else if (!(temp == intPoint)){
            j++;           
          }// else if                    
        }// if
      }// for      
    }// intersection
    
    bool RationalPlane3D::isLeftSideInner(const RationalSegment3D segment,
                                          const RationalPlane3D other)const{
      RationalVector3D segmentVector(segment.getHead() - segment.getTail());
      segmentVector.normalize();
      RationalVector3D vector = this->normalVector ^ segmentVector;
      if (NumericUtil::greater(vector * other.normalVector, 0)){
        return false;
      }// if
      return true;
    }// isLeftAreaInner
                         
    RationalPoint2D RationalPlane3D::transform(
        const RationalPoint3D& point) const{
      // check point d on plane
      if (!NumericUtil::nearlyEqual(distance2ToPlane(point),0.0)) {
        cerr << "Distance to plane " << distance2ToPlane(point).get_d() << endl;
        NUM_FAIL("Point isn't located on plane.");
      }// if
      mpq_class w = point.getX() * wVector.getX() +
                    point.getY() * wVector.getY();
      return RationalPoint2D(w,point.getZ());  
    }// transform
    
    RationalSegment2D RationalPlane3D::transform(
        const RationalSegment3D& segment) const{  
      return RationalSegment2D(transform(segment.getTail()),
                               transform(segment.getHead())); 
    }// transform

/*
7 Class IntersectionPoint

*/   
    IntersectionPoint::IntersectionPoint():x(0),y(0),z(0),w(0){
    }// Konstruktor
   
    IntersectionPoint::IntersectionPoint(const IntersectionPoint& point){
      set(point);
    }// Konstruktor
    
    IntersectionPoint::IntersectionPoint(const Point3D& point3D, 
                                         const Point2D& point2D){
      if (!NumericUtil::nearlyEqual(point3D.getZ(), point2D.getY())) {
        cerr << "Point3D:=" << point3D << endl;
        cerr << "Point2D:=" << point2D << endl;
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
    
    Rectangle<3> IntersectionPoint::getBoundingBox()const{
      double array[3] = {x,y,z};
      return Rectangle<3>(true,array,array);
    }// getBoundingBox 
          
    std::ostream& operator <<(std::ostream& os, 
                              const IntersectionPoint& point){
      os << "IntersectionPoint(" << point.x << ", " << point.y << ", ";
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
8 Class IntersectionSegment

*/  
    IntersectionSegment::IntersectionSegment(){
      this->predicate  = UNDEFINED; 
    }// Konstruktor
    
    IntersectionSegment::IntersectionSegment(
        const IntersectionSegment& segment){
      set(segment);    
    }// konstruktor    
    
    IntersectionSegment::IntersectionSegment(
        const IntersectionPoint& tail,
        const IntersectionPoint& head,
        const Predicate& predicate        /* = UNDEFINED */){
      set(tail,head,predicate);
    }// konstruktor
    
    IntersectionSegment::IntersectionSegment(const Segment3D& segment3D, 
        const Segment2D& segment2D,
        const Predicate& predicate        /* = UNDEFINED */){
      IntersectionPoint tail(segment3D.getTail(),segment2D.getTail());
      IntersectionPoint head(segment3D.getHead(),segment2D.getHead());
      set(tail,head,predicate);  
    }// Konstruktor
      
    void IntersectionSegment::set(
        const IntersectionPoint& tail,
        const IntersectionPoint& head,
        const Predicate& predicate){            
      if ((NumericUtil::lower(tail.getT(), head.getT())) ||
          (NumericUtil::nearlyEqual(tail.getT(), head.getT()) &&
           NumericUtil::lower(tail.getW(), head.getW()))) {   
        this->tail = tail;
        this->head = head;
        this->predicate = predicate; 
      }// if
      else {
        this->tail = head;
        this->head = tail;
        if(predicate == LEFT_IS_INNER) this->predicate = RIGHT_IS_INNER;
        else if (predicate == RIGHT_IS_INNER) this->predicate = LEFT_IS_INNER;
        else this->predicate = predicate;
      }// else

    }// set
    
    void IntersectionSegment::set(const IntersectionSegment& segment){
      this->tail = segment.tail;
      this->head = segment.head;
      this->predicate = segment.predicate; 
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
    
    Predicate IntersectionSegment::getPredicate()const{
      return predicate;
    }// getSegment3D
        
    bool IntersectionSegment::isOrthogonalToTAxis()const{
      return NumericUtil::nearlyEqual(tail.getT(),head.getT());
    }// isOrthogonalToTAxis
    
    bool IntersectionSegment::isOutOfRange(double t)const{
      if (NumericUtil::lower(this->head.getT(),t)) return true;
      return NumericUtil::nearlyEqual(this->head.getT(),t);
    }// isOutOfRange
    
    bool IntersectionSegment::isLeftOf(const IntersectionSegment& intSeg)const{
      double tail1T = this->getTail().getT();
      double head2T = intSeg.getHead().getT();
      double tail2T = intSeg.getTail().getT();
      // Precondition: 
      // this->getTail().getT() is inside the interval 
      // [intSeg.getTail().getT(), intSeg.getHead().getT()]
      // and this and intSeg don't intersect in their interior.
      if (NumericUtil::lower(tail1T,tail2T) ||
          NumericUtil::greater(tail1T,head2T)) {
        NUM_FAIL ("t must between the t value form tail und haed.");
      }// if
      Segment2D segment2D1 =  this->getSegment2D();
      Segment2D segment2D2 = intSeg.getSegment2D();
      double sideOfStart = segment2D2.whichSide(segment2D1.getTail());   
      if (sideOfStart >   NumericUtil::eps) return true;
      if (sideOfStart < - NumericUtil::eps) return false;
      double sideOfEnd = segment2D2.whichSide(segment2D1.getHead());
      return sideOfEnd > NumericUtil::eps;
    }// bool
    
    Point3D IntersectionSegment::evaluate(double t) const {
      double headT = this->getHead().getT();
      double tailT = this->getTail().getT();
      Point3D head3D = head.getPoint3D();
      Point3D tail3D = tail.getPoint3D();
      // Precondition:
      // t is between t on tail and haed
      if(!(NumericUtil::between(tailT, t, headT))){
        NUM_FAIL ("t must between the t value form tail und haed.");
      }// if
      if (NumericUtil::nearlyEqual(t, headT)) return head3D;
      if (NumericUtil::nearlyEqual(t, tailT)) return tail3D;
      // Point3D pointInPlane(0.0, 0.0, t);
      // Vector3D normalVectorOfPlane(0.0, 0.0, 1.0);
      // Vector3D u = this->getHead().getPoint3D() - 
      //              this->getTail().getPoint3D();
      // Vector3D w = this->getTail().getPoint3D() - pointInPlane;
      // double d = normalVectorOfPlane * u;
      // double n = -normalVectorOfPlane * w;
      //
      // This can be simplified to:
      RationalVector3D u = head3D.getR() - tail3D.getR();
      mpq_class d =  mpq_class(headT) - tailT;
      mpq_class n =  mpq_class(t) - tailT;
      // this segment must not be parallel to plane
      if (NumericUtil::nearlyEqual(d, 0.0)) {
        NUM_FAIL ("Intersection segment must not be parallel to plane."); 
      }// if  
      mpq_class s = n/d;
      if (!(NumericUtil::between(0.0, s, 1.0))) {
        NUM_FAIL ("No point on segment found.");
      }// if
      // compute segment intersection point
      return (tail3D.getR() + s * u).getD();  
    }// evaluate    
    
    std::ostream& operator <<(
        std::ostream& os, const IntersectionSegment& segment){
      os << "IntersectionSegment(" << segment.tail << ", " << segment.head;
      os << ", " << toString(segment.getPredicate());
      os << ")";
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
             this->predicate == segment.predicate;
    }// Operator ==   
/*
9 Struct IntSegCompare

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
      if (segment2->getSegment2D().isLeft(head1.getPoint2D())) return true;
      if (segment1->getSegment2D().isLeft(head2.getPoint2D())) return false; 
      // segment1 is colinear to segment2    
      if (NumericUtil::lower(  head1.getT(), head2.getT())) return false;
      if (NumericUtil::greater(head1.getT(), head2.getT())) return true;
      // head1.getT() == head2.getT(), head1.getW() == head2.getW() 
      if(segment1->getPredicate() < segment2->getPredicate())  return true;
      return false;
    }// IntSegCompare  
/*
10 Class PlaneSweepAccess

*/       
    void PlaneSweepAccess::first(double t1, double t2, 
                                 Point3DContainer& points,
                                 SegmentContainer& segments,
                                 bool pFaceIsCritical){
      NUM_FAIL ("method must override.");
    }// first

    void PlaneSweepAccess::next(double t1, double t2, 
                                Point3DContainer& points,
                                SegmentContainer& segments,
                                bool pFaceIsCritical){
      NUM_FAIL ("method must override.");
    }// next
    
    PlaneSweepAccess::~PlaneSweepAccess(){
    }// Destruktor
    
/*
11 class IntSegContainer

*/   
    IntSegContainer::IntSegContainer(){
    }// Konstruktor
    
    IntSegContainer::IntSegContainer(const IntSegContainer& container){
      set(container);
    }// Konstruktor
    
    IntSegContainer::~IntSegContainer(){  
      std::set<IntersectionSegment*>::iterator iter;
      for (iter = intSegs.begin(); iter != intSegs.end(); ++iter) {
        delete *iter;
      }// for  
    }// Destruktor
    
    void IntSegContainer::set(const IntSegContainer& container){
      std::set<IntersectionSegment*>::iterator iter;
      for (iter = intSegs.begin(); iter != intSegs.end(); ++iter) {
        delete *iter;
      }// form
      this->intSegs = std::set<IntersectionSegment*, IntSegCompare>();
      this->active  = std::list<IntersectionSegment*>();      
      for (iter  = container.intSegs.begin(); 
           iter != container.intSegs.end(); ++iter) {
        this->intSegs.insert(new IntersectionSegment(**iter));
      }// for
    }// set
 
    void IntSegContainer::addIntSeg(const IntersectionSegment& seg){
      intSegs.insert(new IntersectionSegment(seg));
    }// addIntSeg
    
    size_t IntSegContainer::size()const{
      return intSegs.size();
    }// size
  
    std::ostream& operator <<(std::ostream& os, 
                              const IntSegContainer& container){
      return container.print(os,"");
    }// operator 
    
    std::ostream& IntSegContainer::print(std::ostream& os, 
                                         std::string prefix)const{
      os << "IntSegContainer(";
      if (intSegs.empty()) os << "is empty)" << endl;
      else {        
        std::set<IntersectionSegment*>::iterator iter;
        for (iter  = intSegs.begin(); 
             iter != intSegs.end(); ++iter) {
          os << endl << prefix + "  " << *(*iter);
        }// for
        os << endl << prefix << ")" <<endl;
      }// else
      return os;
    }// print
    
    bool IntSegContainer::operator ==(const IntSegContainer& container)const{
      if (this->intSegs.size() != container.intSegs.size()) return false;
      std::set<IntersectionSegment*>::iterator iter1, iter2;
      for (iter1  = this->intSegs.begin(), iter2 =  container.intSegs.begin();
           iter1 != this->intSegs.end() && iter2 != container.intSegs.end();
          ++iter1, ++iter2) {
        if(!(*(*iter1) == *(*iter2))) return false;
      }// for
      return true;
    }// Operator ==  
    
    IntSegContainer& IntSegContainer::operator =(
        const IntSegContainer& container){
      set(container);
      return *this;
    }// Operator = 
    
    bool IntSegContainer::hasMoreSegsToInsert(double t)const{
      if (intSegIter == intSegs.end()) return false;
      IntersectionPoint tail((*intSegIter)->getTail());
      return NumericUtil::nearlyEqual(tail.getT(),t);
    }// hasMoreSegsToInsert
    
    void IntSegContainer::first(double t1, double t2, 
                                Point3DContainer& points,
                                SegmentContainer&  segments,
                                bool pFaceIsCritical){ 
      intSegIter = intSegs.begin();
      next(t1,t2,points,segments,pFaceIsCritical);      
    }// first
    
    void IntSegContainer::next(double t1, double t2, 
                               Point3DContainer& points, 
                               SegmentContainer&  segments,
                               bool pFaceIsCritical){
      list<IntersectionSegment*>::iterator activeIter;
      activeIter = active.begin();
      while (activeIter != active.end()){
        while (activeIter != active.end() && 
              (*activeIter)->isOutOfRange(t1)){
          activeIter = active.erase(activeIter);
        }// while
        if (activeIter == active.end()) break; 
        if (hasMoreSegsToInsert(t1)) {
          IntersectionSegment* newSeg = *intSegIter;
          if (newSeg->isLeftOf(**activeIter)) {
            activeIter = active.insert(activeIter, newSeg);
            intSegIter++;
          }// if 
        }// if 
        activeIter++;
      }// while 
      // Add the tail, if there is one:
      while (hasMoreSegsToInsert(t1)) {
        IntersectionSegment* newSeg = *intSegIter;
        activeIter = active.insert(activeIter, newSeg);
        intSegIter++;
        activeIter = active.end();
      }// while
      // create result segments   
      for (activeIter = active.begin();
           activeIter != active.end();
           activeIter++) {       
        Point3D tail, head;
        IntersectionSegment* segment = *activeIter;
        if (segment->isOrthogonalToTAxis()) {
          tail = segment->getTail().getPoint3D();
          head = segment->getHead().getPoint3D(); 
        }// if
        else {
          tail = segment->evaluate(t1);
          head = segment->evaluate(t2);
        }// else
        size_t p1     = points.add(tail);
        size_t p2     = points.add(head);
        Predicate predicate = segment->getPredicate();
        segments.add(Segment(p1,p2,predicate),pFaceIsCritical);   
      }// for   
    }// next    
/*
12 struct DoubleCompare

*/      
    bool DoubleCompare::operator()(const double& d1, const double& d2) const{ 
        return NumericUtil::lower(d1, d2);
    }// Operator
/*
13 class GlobalTimeValues

*/      
    GlobalTimeValues::GlobalTimeValues(double scale /* = 1 */, 
                                       double orginalStartTime /* = 0 */,
                                       double orginalEndTime /* = 1 */){
      this->orginalStartTime = orginalStartTime;
      this->orginalEndTime   = orginalEndTime;
      this->scale            = scale;
    }// Konstruktor
    
    GlobalTimeValues::GlobalTimeValues(
        const Interval<Instant>& orginalInterval){
      this->orginalStartTime = orginalInterval.start.ToDouble();
      this->orginalEndTime   = orginalInterval.end.ToDouble();
      this->scale            = 1000;
    }// Konstruktor
    
    void GlobalTimeValues::setScaleFactor(double scaleFactor) {
      if (scaleFactor > 0){
        this->scale = scaleFactor;
      }// if
      else NUM_FAIL("Scale factor must be greate as zero"); 
    }// setScale
    
    Interval<Instant> GlobalTimeValues::getOrginalInterval()const{
      return createInterval(this->orginalStartTime, this->orginalEndTime);
    }// getOrginalInterval
    
    Interval<Instant> GlobalTimeValues::getScaledInterval()const{
      return createInterval(0.0, this->scale);
    }// getScalingInterval
    
    double GlobalTimeValues::getOrginalStartTime()const{
      return this->orginalStartTime;
    }// getOrginalStartTime
    
    double GlobalTimeValues::getOrginalEndTime()const{
      return this->orginalEndTime;
    }// getOrginalEndTime
    
    double GlobalTimeValues::getScaledStartTime()const {
      return 0.0;
    }// getScaledStartTime
    
    double GlobalTimeValues::getScaledEndTime() const{
      return this->scale;
    }// getScaledEndTime
    
    Interval<Instant> GlobalTimeValues::createInterval(double start, 
                                                       double end) const{
      Instant starttime(datetime::instanttype);
      Instant endtime(datetime::instanttype);
      starttime.ReadFrom(start);
      endtime.ReadFrom(end);
      return (Interval<Instant>(starttime, endtime, true, false));
    }// createInterval
    
    double GlobalTimeValues::computeOrginalTimeValue(
        double scaledTimeValue)const {
      mpq_class scaledTimeValue_r(scaledTimeValue);
      mpq_class scale_r(scale);
      mpq_class orginalStartTime_r(orginalStartTime);
      mpq_class orginalEndTime_r(orginalEndTime);
          
      mpq_class temp = scaledTimeValue_r/scale_r;  
      mpq_class result = (1.0 - temp) * orginalStartTime_r + 
                         temp * orginalEndTime_r;
      return result.get_d();
    }// computeOrginalTimeValue 
          
    void GlobalTimeValues::addTimeValue(double t){
      if (NumericUtil::greaterOrNearlyEqual(t, 0) &&
          NumericUtil::lowerOrNearlyEqual(t, this->scale)){
        time.insert(t);
      }// if
      else {
        cerr << setprecision(9);
        cerr << *this;
        cerr << setprecision(9);
        cerr << t << endl;
        NUM_FAIL ("Time value don,t be between starttime und endtime");
      }// else  
    }// addTimeValue
    
    size_t GlobalTimeValues::size()const{
      return time.size();
    }// size
         
    std::ostream& operator <<(std::ostream& os, 
                              const GlobalTimeValues& timeValues){
      return timeValues.print(os,"");
    }// Operator <<
    
    std::ostream& GlobalTimeValues::print(std::ostream& os, 
                                          std::string prefix)const{
      std::set<double, DoubleCompare>::const_iterator iter;
      os << "GlobalTimeValues(" << endl;
      os << prefix + "  Orginal start time:= " << this->orginalStartTime;
      os << ", Orginal end time:= " << this->orginalEndTime << "," << endl;
      os << prefix + "  Scaled start time:= 0, Scaled end time:= ";
      os << this->scale << "," <<endl;
      os << prefix + "  Time values (";
      if (time.size() == 0) os << "is empty)" << endl;
      else {       
        for (iter = this->time.begin(); 
            iter != this->time.end(); iter++){
          if (iter != this->time.begin()) os << ", " ;      
          os << *iter;
        }// for
      }// else
      os << ")" << endl;
      os << prefix << ")" << endl;
      return os;
    }// print
    
    bool GlobalTimeValues::operator == (const GlobalTimeValues& other)const{
      if (!(NumericUtil::nearlyEqual(this->orginalStartTime, 
                                     other.orginalStartTime))) return false;
      if (!(NumericUtil::nearlyEqual(this->orginalEndTime, 
                                     other.orginalEndTime))) return false;
      if (!(NumericUtil::nearlyEqual(this->scale, other.scale))) return false;
      if(this->time.size() != other.time.size()) return false;
      std::set<double, DoubleCompare>::iterator iter1,iter2;
      for (iter1  = this->time.begin(),iter2  = other.time.begin(); 
           iter1 != this->time.end(); iter1++,iter2++){
        if(!(NumericUtil::nearlyEqual(*iter1,*iter2))) return false;
      }// for
      return true;
    }// Operator ==
    
     bool GlobalTimeValues::scaledFirst(double& t1, double& t2){
       // min two timevalues requert
       if(time.size() < 2) return false;
       this->timeIter  = time.begin();
       this->t1 = t1   = *timeIter;
       this->timeIter++;
       this->t2 = t2   = *timeIter; 
       this->orginalT1 = computeOrginalTimeValue(this->t1);
       this->orginalT2 = computeOrginalTimeValue(this->t2);
       return true;
     }// scaledFirst
      
     bool GlobalTimeValues::scaledNext(double& t1, double& t2){
       if((++timeIter) != time.end()){
         this->t1 = t1   = this->t2;
         this->orginalT1 = this->orginalT2;
         this->t2 = t2   = *timeIter;
         this->orginalT2 = computeOrginalTimeValue(this->t2);
         return true;
       }// if
       return false;
     }// scaledNext  
     
    bool GlobalTimeValues::orginalFirst(double& t1, double& t2){
      if(time.size() < 2) return false;
      this->timeIter  = time.begin();
      this->t1        = *timeIter;
      this->timeIter++;
      this->t2        = *timeIter; 
      this->orginalT1 = t1 = computeOrginalTimeValue(this->t1);
      this->orginalT2 = t2 = computeOrginalTimeValue(this->t2);
      return true;
    }// orginalFirst
    
    bool GlobalTimeValues::orginalNext(double& t1, double& t2){
      if((++timeIter) != time.end()){
        this->t1        = this->t2;
        this->orginalT1 = t1 = this->orginalT2;
        this->t2        = *timeIter;
        this->orginalT2 = t2 = computeOrginalTimeValue(this->t2);
        return true;
      }// if
      return false;
    }// orginalFirst
/*
14 class MSegment 
 
*/
    MSegment::MSegment(){  
      set(Point3D(0,0,0),Point3D(0,0,0),
          Point3D(1,0,0),Point3D(1,0,0));
    }// Konstruktor
    
    MSegment::MSegment(const MSegment& msegment){
      set(msegment);      
    }// Konstruktor

    MSegment::MSegment(const Segment3D& left, const Segment3D& right){
      set(left.getTail(),left.getHead(),right.getTail(),right.getHead());
    }// Konstruktor

    MSegment::MSegment(const Segment3D& left, 
                       const Segment3D& right,
                       int   faceno,
                       int   cycleno,
                       int   edgeno,
                       bool  leftDomPoint,
                       bool  insideAbove){
      set(left.getTail(),left.getHead(),right.getTail(),right.getHead());
      this->medianHS.attr.faceno  = faceno;
      this->medianHS.attr.cycleno = cycleno;
      this->medianHS.attr.edgeno  = edgeno;
      this->medianHS.SetLeftDomPoint(leftDomPoint);
      this->insideAbove = medianHS.attr.insideAbove = insideAbove;
    }// Konstruktor
    
    MSegment::MSegment(const MSegmentData& mSeg, 
                       const GlobalTimeValues& timeValues){
      Point2D start;
      Point2D end;
      double startTime = timeValues.getScaledStartTime(); 
      double endTime   = timeValues.getScaledEndTime();
      // Fallen die Initialpunkte zusammen
      if (!mSeg.GetPointInitial()) {
        // Start und Endpunkt am initialen Sgement bestimmen
        start = Point2D(mSeg.GetInitialStartX(), mSeg.GetInitialStartY());
        end   = Point2D(mSeg.GetInitialEndX(), mSeg.GetInitialEndY());        
      }// if
      else {
        // Start und Endpunkt an dem finalen Segment bestimmen
        start = Point2D(mSeg.GetFinalStartX(), mSeg.GetFinalStartY());
        end   = Point2D(mSeg.GetFinalEndX(), mSeg.GetFinalEndY());
      }// else
      // Startpunkte festlegen
      if ((start < end) == mSeg.GetInsideAbove()) {                     
        this->leftStart  = Point3D(mSeg.GetInitialStartX(), 
                                   mSeg.GetInitialStartY(),
                                   startTime);
        this->rightStart = Point3D(mSeg.GetInitialEndX(), 
                                   mSeg.GetInitialEndY(),
                                   startTime);
        this->leftEnd    = Point3D(mSeg.GetFinalStartX(), 
                                   mSeg.GetFinalStartY(),
                                   endTime);
        this->rightEnd   = Point3D(mSeg.GetFinalEndX(), 
                                   mSeg.GetFinalEndY(),
                                   endTime);
      }// if
      else {
        this->leftStart  = Point3D(mSeg.GetInitialEndX(), 
                                   mSeg.GetInitialEndY(),
                                       startTime);
        this->rightStart = Point3D(mSeg.GetInitialStartX(), 
                                   mSeg.GetInitialStartY(),
                                   startTime);
        this->leftEnd    = Point3D(mSeg.GetFinalEndX(), 
                                   mSeg.GetFinalEndY(),
                                   endTime);
        this->rightEnd   = Point3D(mSeg.GetFinalStartX(), 
                                   mSeg.GetFinalStartY(),
                                   endTime);        
      }// else 
      // mittleres Halbsegment berechnen
      createMedianHS();
      // Indizes setzen
      medianHS.attr.faceno  = mSeg.GetFaceNo();
      medianHS.attr.edgeno  = mSeg.GetSegmentNo();
      medianHS.attr.cycleno = mSeg.GetCycleNo();
      medianHS.attr.coverageno = -1;
      medianHS.attr.partnerno  = -1;
      boundingRect = getBoundingRec(leftStart);
      boundingRect.Extend(getBoundingRec(leftEnd));
      boundingRect.Extend(getBoundingRec(rightStart));
      boundingRect.Extend(getBoundingRec(rightEnd));       
    }// KonstruktorCreate           

    void MSegment::set(const Point3D leftStart, const Point3D leftEnd,
                       const Point3D rightStart, const Point3D rightEnd){
      this->leftStart   = leftStart;
      this->leftEnd     = leftEnd;
      this->rightStart  = rightStart;
      this->rightEnd    = rightEnd;      
      createMedianHS();
      medianHS.attr.faceno     = -1;
      medianHS.attr.cycleno    = -1;
      medianHS.attr.edgeno     = -1;
      medianHS.attr.coverageno = -1;
      medianHS.attr.partnerno  = -1;
      boundingRect = getBoundingRec(leftStart);
      boundingRect.Extend(getBoundingRec(leftEnd));
      boundingRect.Extend(getBoundingRec(rightStart));
      boundingRect.Extend(getBoundingRec(rightEnd)); 
    }// set
        
    void MSegment::set(const MSegment& mSegment){
      this->leftStart   = mSegment.leftStart;
      this->leftEnd     = mSegment.leftEnd;     
      this->rightStart  = mSegment.rightStart;
      this->rightEnd    = mSegment.rightEnd;
      this->medianHS    = mSegment.medianHS;
      this->insideAbove = mSegment.insideAbove;
      boundingRect = getBoundingRec(leftStart);
      boundingRect.Extend(getBoundingRec(leftEnd));
      boundingRect.Extend(getBoundingRec(rightStart));
      boundingRect.Extend(getBoundingRec(rightEnd)); 
    }// set
    
    void MSegment::createMedianHS(){
      double medianStartX = (this->leftStart.getX() + 
                             this->leftEnd.getX())/2;
      double medianStartY = (this->leftStart.getY() + 
                             this->leftEnd.getY())/2;
      double medianEndX   = (this->rightStart.getX() + 
                             this->rightEnd.getX())/2;
      double medianEndY   = (this->rightStart.getY() + 
                             this->rightEnd.getY())/2;
      Point medianStart(true,medianStartX, medianStartY);
      Point medianEnd  (true,medianEndX,medianEndY);
      medianHS = HalfSegment(true, medianStart, medianEnd);
      insideAbove = medianHS.attr.insideAbove = !(medianStart > medianEnd);
    }// createMedianHS
    
    int MSegment::getFaceNo() const{
      return medianHS.attr.faceno;
    }// getFaceNo
    
    int MSegment::getCycleNo() const{
      return medianHS.attr.cycleno;
    }// getCycleNo
    
    int MSegment::getSegmentNo() const{
      return medianHS.attr.edgeno;
    }// getSegmentNo
    
    bool MSegment::getInsideAbove() const{
      return insideAbove;
    }// getInsideAbove
            
    HalfSegment MSegment::getMedianHS() const{
      return medianHS;
    }// getMedianHS        
        
    Point3D MSegment::getLeftStart() const{
      return leftStart;
    }// getLeftStart
    
    Point3D MSegment::getLeftEnd() const{
      return leftEnd;
    }// getLeftEnd
    
    Point3D MSegment::getRightStart() const{
      return rightStart;
    }// getRightStart
    
    Point3D MSegment::getRightEnd() const{
      return rightEnd;
    }// getRightEnd
    
    Rectangle<2> MSegment::getBoundingRec(const Point3D& point)const{
      double array[2] = {point.getX(),point.getY()};
      return Rectangle<2>(true,array,array);
    }// getBoundingBox 
    
    Rectangle<2> MSegment::getBoundingRec()const{
      return boundingRect;
    }// getBoundingBox 
    
    MSegmentData MSegment::getMSegmentData() const{
       MSegmentData msd(getFaceNo(), getCycleNo(), getSegmentNo(), 
                        getInsideAbove(),
                        leftStart.getX(),leftStart.getY(), 
                        rightStart.getX(),rightStart.getY(), 
                        leftEnd.getX(),leftEnd.getY(),    
                        rightEnd.getX(), rightEnd.getY());        
       msd.SetDegeneratedInitial(DGM_NONE);
       msd.SetDegeneratedFinal(DGM_NONE);
       return msd;
    }// getMSegmentData
    
    bool MSegment::isLeftDomPoint() const{
      return medianHS.IsLeftDomPoint();
    }// isLeftDomPoint
      
    void MSegment::setSegmentNo(int sn){
      medianHS.attr.edgeno = sn;
    }// setSegmentNo
    
    void MSegment::setLeftDomPoint(bool ldp){
       medianHS.SetLeftDomPoint(ldp);
    }// setLeftDomPoint
    
    bool MSegment::lessByMedianHS(const MSegment& other) const {
      return this->medianHS < other.medianHS;
    }// lessByMedianHS
        
    bool MSegment::logicLess(const MSegment& other) const {
      if (isLeftDomPoint() != other.isLeftDomPoint())
        return isLeftDomPoint() > other.isLeftDomPoint();
      return this->medianHS.LogicCompare(other.medianHS) == -1;
    }// logicLess       
    
    void MSegment::copyIndicesFrom(const HalfSegment* hs) {
      medianHS.attr.faceno  = hs->attr.faceno;
      medianHS.attr.cycleno = hs->attr.cycleno;
      medianHS.attr.edgeno  = hs->attr.edgeno;
    }// copyIndicesFrom
    
    std::ostream& MSegment::print(std::ostream& os, std::string prefix)const{
      os << "MSegment(" << endl;
      os << prefix << "  Left:=    " << Segment3D(leftStart, leftEnd) << endl;
      os << prefix << "  Right:=   " << Segment3D(rightStart, rightEnd) << endl;
      os << prefix << "  MedianHS:=" << this->medianHS << endl;
      os << prefix <<")" << endl;
      return os;
    }// print

    std::ostream& operator <<(std::ostream& os, const MSegment& mSegment){
      mSegment.print(os,"");
      return os;
    }// Operator <<
   
    bool MSegment::operator ==(const MSegment& mSegment)const{
      if((this->leftStart   == mSegment.leftStart) &&
         (this->leftEnd     == mSegment.leftEnd) &&
         (this->rightStart  == mSegment.rightStart) &&
         (this->rightEnd    == mSegment.rightEnd) &&
         (this->medianHS    == mSegment.medianHS)&&
         (this->insideAbove == mSegment.insideAbove)) return true;
      return false;
    }// Operator ==
   
    MSegment& MSegment::operator =(const MSegment& mSegment){
      set(mSegment);
      return *this;
    }// Operator = 
/*
15 class CriticalMSegment 
 
*/        
    CriticalMSegment::CriticalMSegment():
        source(UNIT_A),predicate(UNDEFINED){      
    }// Konstruktor
    
    CriticalMSegment::CriticalMSegment(const CriticalMSegment& cmsegment){
      set(cmsegment);
    }// Konstruktor
   
    CriticalMSegment::CriticalMSegment(const Segment3D& left, 
                                       const Segment3D& right, 
                                       SourceFlag source,
                                       Predicate predicate){
      set(left, right, source,predicate);      
    }// Konstruktor
    
    void CriticalMSegment::set(const CriticalMSegment& cmSegment){
      this->left        = cmSegment.left;
      this->right       = cmSegment.right;
      this->source      = cmSegment.source; 
      this->midPoint    = cmSegment.midPoint;
      this->normalVector= cmSegment.normalVector;
      this->predicate   = cmSegment.predicate;
    }// set
    
    void CriticalMSegment::set(const Segment3D& left, const Segment3D& right,
                               SourceFlag source, Predicate predicate){ 
      this->left = left;
      this->right = right;      
      RationalPoint3D initialStart = left.getTail().getR();
      RationalPoint3D initialEnd   = right.getTail().getR();
      RationalPoint3D finalStart   = left.getHead().getR();
      RationalPoint3D finalEnd     = right.getHead().getR();      
      // We compute the normalvector
      if (initialStart != initialEnd) {
        // Cross product of vector 
        this->normalVector = (initialEnd - initialStart) ^ 
                             (finalStart - initialStart);
      }// if
      else { // A == B
        // Cross product of vector :
        this->normalVector = (finalStart - finalEnd) ^ 
                             (initialEnd - finalEnd);
      }// else
      this->normalVector.normalize();
      double x = (left.getTail().getX() + right.getTail().getX() +
                  left.getHead().getX() + right.getHead().getX())/4;
      double y = (left.getTail().getY() + right.getTail().getY() +
                  left.getHead().getY() + right.getHead().getY())/4;
      double z = (left.getTail().getZ() + right.getTail().getZ() +
                  left.getHead().getZ() + right.getHead().getZ())/4;
      this->midPoint =Point3D(x,y,z);
      this->source = source; 
      this->predicate = predicate;
    }// set
    
    Point3D CriticalMSegment::getMidPoint()const{
      return midPoint;
    }// getMidPoint 
    
    Segment3D CriticalMSegment::getLeft() const{
      return left;
    }// getLeft
      
    Segment3D CriticalMSegment::getRight() const{
      return right;
    }// getRight
    
    std::ostream& CriticalMSegment::print(std::ostream& os, 
                                          std::string prefix)const{
      os << "CriticalMSegment(" << endl;
      os << prefix << "  Left:=" <<  this->left << endl;
      os << prefix << "  Right:="<<  this->right << endl;
      if(this->source == UNIT_A) os << prefix << "  Source:= UNIT A"  << endl;
      else                       os << prefix << "  Source:= UNIT B"  << endl;
      os << prefix << "  Predicate:=" << toString(this->predicate) << endl;
      os << prefix << "  MidPoint:=" << this->midPoint << endl;
      os << prefix << "  Normalvector:=" << this->normalVector << endl; 
      os << prefix <<")" << endl;
      return os;
    }// print
    
    std::ostream& operator <<(std::ostream& os, 
                              const CriticalMSegment& cmSegment){
      cmSegment.print(os,"");
      return os;
    }// Operator <<
   
    bool CriticalMSegment::operator ==(const CriticalMSegment& cmSegment)const{
      if((this->left  == cmSegment.left) &&
         (this->right == cmSegment.right) &&       
         (this->source == cmSegment.source)&&
         (this->predicate == cmSegment.predicate)) return true;
      return false;
    }// Operator ==
    
    CriticalMSegment& CriticalMSegment::operator =(
        const CriticalMSegment& cmSegment){
      set(cmSegment);
      return *this;
    }// Operator =            
    
    bool CriticalMSegment::isPartOfUnitA() const{
      return source == UNIT_A;
    }// isPartOfUnitA
      
    bool CriticalMSegment::hasEqualNormalVector(
        const CriticalMSegment& other) const{  
      // Precondition: this is parallel to other.
      // The normal vectors are equal, iff
      // the cosinus of the angle between them is positive:
      return NumericUtil::greater(normalVector * other.normalVector, 0.0);
    }// hasEqualNormalVectors
      
    bool CriticalMSegment::operator <(const CriticalMSegment& cmSegment)const{
      return this->midPoint < cmSegment.midPoint;      
    }// Operator <
    
    Predicate CriticalMSegment::getPredicate() const {
      return this->predicate;      
    }// getPredicate
    
    MSegment CriticalMSegment::getMSegment()const{
      return MSegment(left,right);
    }// getMSegment
/*
16 class ResultUnit 
 
*/
    ResultUnit::ResultUnit(){
      this->orginalStartTime = 0;
      this->orginalEndTime   = 1;      
    }// Konstruktor
    
    ResultUnit::ResultUnit(double orginalStartTime, double orginalEndTime){
      this->orginalStartTime = orginalStartTime;
      this->orginalEndTime   = orginalEndTime;     
    }// Konstruktor
    
    ResultUnit::ResultUnit(const ResultUnit& other){
      set(other);
    }// Konstruktor
    
    void ResultUnit::set(const ResultUnit& other){
      this->mSegments  = std::vector<MSegment>();
      this->mCSegments = std::vector<CriticalMSegment>(); 
      this->orginalStartTime = other.orginalStartTime;
      this->orginalEndTime   = other.orginalEndTime;   
      for (size_t i = 0; i < other.mSegments.size(); i++) {
        this->mSegments.push_back(other.mSegments[i]);
      }// for
      for (size_t i = 0; i < other.mCSegments.size(); i++) {
        this->mCSegments.push_back(other.mCSegments[i]);
      }// for
    }// set
    
    size_t ResultUnit::size(){
      return mSegments.size();
    }// size(
    
    void ResultUnit::addMSegment(MSegment& mSegment, bool completely ){
      if (completely) {
        mSegments.push_back(mSegment);
      }// if
      else {
        mSegment.setSegmentNo(this->mSegments.size()/2);
        mSegment.setLeftDomPoint(true);
        mSegments.push_back(mSegment);
        mSegment.setLeftDomPoint(false);
        mSegments.push_back(mSegment);
      }// else
    }// addMSegment
    
    void ResultUnit::addMSegment(const CriticalMSegment& mCSegment){
      MSegment mSegment = mCSegment.getMSegment();
      addMSegment(mSegment,false);      
    }// addMSegment
    
    void ResultUnit::addCMSegment(const CriticalMSegment& mSegment){
      mCSegments.push_back(mSegment);
    }// addMSegment
    
    Interval<Instant> ResultUnit::getTimeInterval() const {
      Instant start(datetime::instanttype);
      Instant end(datetime::instanttype);
      start.ReadFrom(this->orginalStartTime);
      end.ReadFrom(this->orginalEndTime);
      return Interval<Instant>(start, end, true, false);
    }// getTimeInterval
 
    std::ostream& ResultUnit::print(std::ostream& os, std::string prefix)const{
      os << "ResultUnit(" << endl;
      os << prefix + "  Startime:=" << this->orginalStartTime;
      os << ", EndTime:=" << this->orginalEndTime << endl;
      os << prefix +  "  MSegments (";
      if (this->mSegments.size() == 0) os << "is empty)" << endl;
      else {
        os << endl;
        for (size_t i = 0; i < this->mSegments.size(); i++) {
          os << prefix + "    Index:=" << i << ", "; 
          this->mSegments[i].print(os,prefix + "    ");
        }// for
        os << prefix + "  )" << endl;
      }// else
      os << prefix + "  CriticalMSegment (";  
      if (this->mCSegments.size() == 0) os << "is empty)" << endl;
      else {  
        os << endl;
        for (size_t i = 0; i < this->mCSegments.size(); i++) {
          os << prefix + "    Index:=" << i << ", "; 
          this->mCSegments[i].print(os,prefix + "    ");
        }// for
        os << prefix << "  )"<<endl;
      }// else  
      os << prefix << ")"<<endl;
      return os;
    }// print
    
    std::ostream& operator <<(std::ostream& os, const ResultUnit& unit){
      unit.print(os,"");
      return os;
    }// Operator <<
   
    bool ResultUnit::operator ==(const ResultUnit& other)const{
      if (!(NumericUtil::nearlyEqual(this->orginalStartTime,
                                     other.orginalStartTime) &&
            NumericUtil::nearlyEqual(this->orginalEndTime,
                                     other.orginalEndTime))) {
        cerr << "Start time or end time not equal" << endl;
        return false;
      }// if
      if (this->mSegments.size() != other.mSegments.size()) {
        cerr << "Size of mSegments is differnt" << endl; 
        return false;  
      }// if
      for (size_t i = 0; i < this->mSegments.size(); i++) {
        if (!(this->mSegments[i] == other.mSegments[i])) {
          cerr << "ResultUnit is not equal on Index for MSegment:=";
          cerr << i << endl;
          return false;
        }// if
      }// for
      return true;
    }// Operator ==
   
    ResultUnit& ResultUnit::operator =(const ResultUnit& unit){
      set(unit);
      return *this;
    }// Operator =
    
    bool ResultUnit::less(const MSegment& ms1, const MSegment& ms2) {
      return ms1.lessByMedianHS(ms2);
    }// less

    bool ResultUnit::logicLess(const MSegment& ms1, const MSegment& ms2) {
      return ms1.logicLess(ms2);
    }// logicLess
    
    void ResultUnit::finalize(){
      if(mSegments.size() == 0) return;
      // First, we sort the mSegments of this unit by their 
      // median-halfsegments. Comparison between halfsegments
      // is done by the < operator, implemented in the SpatialAlgebra.
      sort(mSegments.begin(), mSegments.end(), ResultUnit::less);
      // Second, we construct a region from all median-halfsegments
      // of each msegment of this unit:
      Region region(mSegments.size());
      region.StartBulkLoad();
      for (size_t i = 0; i < mSegments.size(); i++) {
        // cout << mSegments[i].getMedianHS() << endl;
        region.Put(i, mSegments[i].getMedianHS());
      }// for
      // Note: Sorting is already done.
      region.EndBulkLoad(false, true, true, true);
      // Third, we retrive the faceNo, cycleNo and edgeNo of
      // each halfsegment from the region, computed in the 
      // Region::EndBulkLoad procedure:
      for (unsigned int i = 0; i < mSegments.size(); i++) {
        HalfSegment halfSegment;
        region.Get(i, halfSegment);
        mSegments[i].copyIndicesFrom(&halfSegment);
      }// for
      // Sort mSegments by faceno, cycleno and segmentno:
      sort(mSegments.begin(), mSegments.end(), ResultUnit::logicLess);
      //  this->Print();
      // Erase the second half of mSegments, 
      // which contains all MSegments with right dominating point:
      mSegments.erase(mSegments.begin() + mSegments.size() / 2, 
                      mSegments.end());
    }// finalize

    URegionEmb* ResultUnit::convertToURegionEmb(
        DbArray<MSegmentData>* segments) const {
      size_t segmentsStartPos = segments->Size();
      URegionEmb* uregion     = new URegionEmb(getTimeInterval(), 
                                               segmentsStartPos);
      Rectangle<2> resultBRec;      
      if(mSegments.size() > 0){      
        resultBRec = mSegments[0].getBoundingRec();
      }// if      
      for(unsigned int i = 0; i < mSegments.size(); i++) {            
         MSegmentData msd  = mSegments[i].getMSegmentData();   
         uregion->PutSegment(segments, i, msd, true);
         Rectangle<2> bRec = mSegments[i].getBoundingRec();
         resultBRec.Extend(bRec);
      }// for
      // Set the bbox           
      double min[3] = { resultBRec.MinD(0),resultBRec.MinD(1), 
        this->orginalStartTime };        
      double max[3] = { resultBRec.MaxD(0),resultBRec.MaxD(1), 
        this->orginalEndTime };   
      uregion->SetBBox(Rectangle<3>(true, min, max));
      return uregion;
    }// convertToURegionEmb
    
    void ResultUnit::copyCriticalMSegmens(const CriticalMSegment& cmSeg, 
                                          SetOp setOp){
      if (setOp == UNION && cmSeg.getPredicate() == OUTER) {
        addMSegment(cmSeg); 
        return;
      }// if
      if (setOp == INTERSECTION && cmSeg.getPredicate() == INNER) {
        addMSegment(cmSeg);  
        return; 
      }// if
      if ( setOp == MINUS && 
          ((cmSeg.isPartOfUnitA() && cmSeg.getPredicate() == OUTER)||
          (!cmSeg.isPartOfUnitA() && cmSeg.getPredicate() == INNER))) {
        addMSegment(cmSeg);  
        return;
      }// if
    }// CopyCriticalMSegmens
    
    void ResultUnit::evaluateCriticalMSegmens(SetOp setOp){
      // Sort by midpoints:
      sort(mCSegments.begin(), mCSegments.end());
      // Existieren kritische Segmente
      if (mCSegments.size() > 0) {
        // über alle Segmente
        size_t i;
        for (i = 0; i < mCSegments.size()-1; i++) {
          CriticalMSegment cmSeg0 = mCSegments[i];
          CriticalMSegment cmSeg1 = mCSegments[i+1];
          // besitze die beiden Segmente den gleichen Mittelpunkt
          if(cmSeg0.getMidPoint() == cmSeg1.getMidPoint()) {
            if (cmSeg0.hasEqualNormalVector(cmSeg1)) {
              addMSegment(cmSeg0);
            }// if
            // zweites Segment berücksichtigen
            i++;
          }// if  
          else {
            // zu diesem Segment gibt es kein zweites Segment
            copyCriticalMSegmens(cmSeg0,setOp);
          }// else          
        }// for
        if (i == mCSegments.size()-1) {
          // Letztes Segment hat auch keinen Partner
          CriticalMSegment cmSeg = mCSegments[i];
          copyCriticalMSegmens(cmSeg,setOp);
        }// if
      }// if
      mCSegments.clear();      
    }// evaluateCriticalMSegmens  
/*
17 class ResultUnitFactory

*/  
    ResultUnitFactory::ResultUnitFactory():pFaceIsCritical(false){
    }// Konstruktor
     
    ResultUnitFactory::ResultUnitFactory(const ResultUnitFactory& other){
      set(other);
    }// Konstruktor
 
    ResultUnitFactory::ResultUnitFactory(
        size_t size,bool isCritical /*= false*/):pFaceIsCritical(isCritical){
      inittialize(size);
    }// Konstruktor    
     
    ResultUnitFactory::ResultUnitFactory(
        Point3DContainer& points,
        GlobalTimeValues &timeValues,
        PlaneSweepAccess &access,
        bool pFaceIsCritical){
      this->pFaceIsCritical = pFaceIsCritical;
      inittialize(timeValues.size());
      double t1,t2;
      if (timeValues.scaledFirst(t1, t2) ) { 
        // Index auf das nächte einzufügende Segment
        size_t n = this->segments.size();
        // Punkte und Segmente für den ersten Slide bestimmen
        access.first(t1, t2, points, this->segments, pFaceIsCritical);
        // Zähler für die Zeitwerte
        size_t k;
        // über alle Zeitwerte laufen
        for (k = 0; k < timeValues.size(); k++) {  
          // über die neu hinzugefügten Segmente laufen
          for (size_t i = n; i < segments.size(); i++) {
            Point3D head, tail;
            Segment segment = segments.get(i);
            head = points.get(segment.getHead());
            tail = points.get(segment.getTail());
            // Ist der Z-Wert von Start- und Endpunkt gleich,
            // dann ahndelt es sich um ein orthogonales Segment
            if (NumericUtil::nearlyEqual(head.getZ(),tail.getZ())) {
              orthogonalEdges[k].push_back(i);
            }// if
            // ansonten handelt sich um ein nict orthogonales Segment
            else {
              // nicht orthogonale Segment, dürfen nicht
              // beim letzten Zeitwert auftreten
              if (k != timeValues.size()-1) {
                nonOrthogonalEdges[k].push_back(i); 
              }// if
              else {
                // assert(false);
                NUM_FAIL("Only ortogonal segments are allowed.");
              }// else
            }// else
          }// for
          //  cout << "k:=" << k << endl;
          //  cout << "t1:=" << t1 << ",t2:="<< t2 << endl;
          //  cout << *this;
          //  cout << timeValues;
          // Zeit für die letzte Auswertung übernehmen
          t1 = t2;
          // im letzten Durchlauf gibt es keinen neuen Startwert
          timeValues.scaledNext(t1, t2);
          // Index auf das nächte einzufügende Segment
          n = this->segments.size();
          // nächsten Slide erfassen
          access.next(t1, t2, points, segments, pFaceIsCritical);
        }// for
      }// if
    }// Konstruktor

    void ResultUnitFactory::inittialize(size_t size){
      this->nonOrthogonalEdges = vector<list<size_t>>(size-1,list<size_t>());
      this->orthogonalEdges    = vector<list<size_t>>(size,list<size_t>());
      this->touchsOnLeftBorder = vector<size_t>(size,0);
    }// inittialize
     
    void ResultUnitFactory::set(vector<list<size_t>>& edges1,
                                 const vector<list<size_t>>& edges2){
      edges1 = vector<list<size_t>>(edges2.size(),list<size_t>());
      list<size_t>::const_iterator edgeIter;
      for (size_t i = 0; i < edges2.size(); i++ ) {
        for (edgeIter  = edges2[i].begin();
            edgeIter != edges2[i].end();
            edgeIter++) {
          edges1[i].push_back(*edgeIter);
        }// for
      }//for
    }// set
     
    void ResultUnitFactory::set(const ResultUnitFactory& other){
      this->pFaceIsCritical = other.pFaceIsCritical;
      this->segments        = other.segments;
      set(this->nonOrthogonalEdges, other.nonOrthogonalEdges);
      set(this->orthogonalEdges, other.orthogonalEdges);
      this->touchsOnLeftBorder = vector<size_t>(
        other.touchsOnLeftBorder.size(),0);
      for (size_t i = 0; i < other.touchsOnLeftBorder.size(); i++) {
        this->touchsOnLeftBorder[i] = other.touchsOnLeftBorder[i];
      }// for
    }// set
    
    ResultUnitFactory& ResultUnitFactory::operator = (
      const ResultUnitFactory& other){    
      set(other);
      return *this;
    }// Operator =
      
    // only for Selftest
    void ResultUnitFactory::addNonOrthogonalEdges(size_t slide, 
                                                  const Segment& segment){
      if (slide < nonOrthogonalEdges.size()) {
        size_t index = segments.add(segment);
        nonOrthogonalEdges[slide].push_back(index);
      }// if
      else NUM_FAIL("Index for slide is out of range.");
    }// addNonOrthogonalEdges
    
    // only for Selftest
    void ResultUnitFactory::addOrthogonalEdges(size_t slide, 
                                               const Segment& segment){
      if (slide < orthogonalEdges.size()) {
        size_t index = segments.add(segment);
        orthogonalEdges[slide].push_back(index);
      }// if
      else NUM_FAIL("Index for slide is out of range.");
    }// addOrthogonalEdge
    
    void ResultUnitFactory::setTouchsOnLeftBorder(size_t slide, size_t value){
      if (slide < touchsOnLeftBorder.size()) touchsOnLeftBorder[slide] = value;
      else NUM_FAIL("Index for slide is out of range.");
    }// setTouch
    
    bool ResultUnitFactory::iSEqual(const vector<list<size_t>>& edges1,
                                     const vector<list<size_t>>& edges2)const{
      if (edges1.size() != edges2.size()) return false;
      list<size_t>::const_iterator edge1Iter,edge2Iter;
      for (size_t i = 0; i < edges1.size(); i++) {
        if (edges1[i].size() != edges2[i].size()) return false;
        size_t j = 0;
        for (edge1Iter  = edges1[i].begin(), 
            edge2Iter  = edges2[i].begin();
            edge1Iter != edges1[i].end();
            edge1Iter++,edge2Iter++){
          if (!(*edge1Iter == *edge2Iter)) {
            return false;
          }// if
          j++;
        }// for
      }// for  
      return true;
    }// compare

    bool ResultUnitFactory::operator == (
        const ResultUnitFactory& other)const{  
      if (!(this->segments == other.segments)) return false;     
      if (!(iSEqual(this->orthogonalEdges,other.orthogonalEdges))) 
        return false;
      if (!(iSEqual(this->nonOrthogonalEdges,other.nonOrthogonalEdges)))
        return false;
      vector<size_t>::const_iterator iter1,iter2;
      if (this->touchsOnLeftBorder.size() != other.touchsOnLeftBorder.size())
        return false;
      for (iter1  = this->touchsOnLeftBorder.begin(), 
          iter2 = other.touchsOnLeftBorder.begin();
          iter1 != this->touchsOnLeftBorder.end(); iter1++, iter2 ++) {
         if (*iter1 != *iter2) return false;
      }// for
      return true;
    }// Operator ==
     
    void ResultUnitFactory::evaluate(){
      list<size_t>::iterator first;
      list<size_t>::reverse_iterator last; 
      for (size_t i = 0; i < nonOrthogonalEdges.size(); i++) {
        evaluate(i);
      }// for
      for (size_t i = 1; i < nonOrthogonalEdges.size(); i++) {
        if (nonOrthogonalEdges[i-1].size() < 2) break;
        first = nonOrthogonalEdges[i-1].begin();
         Predicate predicate = segments.get(*first).getPredicate();
         setSlidePredicates(predicate, i, touchsOnLeftBorder[i]);
       }// for  
       for (int i = nonOrthogonalEdges.size()-2; i >= 0; i--) {
         if (nonOrthogonalEdges[i+1].size() < 2) break;
         first = nonOrthogonalEdges[i+1].begin();
         Predicate predicate = segments.get(*first).getPredicate();
         setSlidePredicates(predicate, i, touchsOnLeftBorder[i+1]);
       }// for  
    }// evaluate
    
    void ResultUnitFactory::getBorderPredicates(Predicate& left, 
                                                Predicate& right)const{
      // Index für die erste und letzte Kante
      size_t first,last;
      // aktuelle Prädikat
      Predicate predicate;
      // Prädikat für die linke kante
      left  = UNDEFINED;
      // Prädikat für die rechte Kante
      right = UNDEFINED;
      // existieren Segmente
      if (segments.size() != 0) {
        // über alle Slides  
        for (size_t i = 0; i < nonOrthogonalEdges.size(); i++) {
          // erste Kante
          first = *nonOrthogonalEdges[i].begin();
          // Bestimme das Prädikat der ersten Kante
          predicate = createPredicate(segments.get(first).getPredicate(),LEFT);
          // Prädikat übernehmen, wenn bisher kein Prädikat festgelegt war
          if (left == UNDEFINED) left = predicate;
          // waren bisher keine Schnitte auf der Kante vermerkt und wurde 
          // jetzt ein abweichendes Prädikat ermittelt, 
          // dann das Pädikat "INTERSECT" setzen
          else if (left != INTERSECT && left != predicate) left = INTERSECT;  
          // letztes Kante
          last = *nonOrthogonalEdges[i].rbegin();
          // Bestimme das Prädikat der letzten Kante
          predicate = createPredicate(segments.get(last).getPredicate(),RIGHT);
          // Prädikat übernehmen, wenn bisher kein Prädikat festgelegt war
          if (right == UNDEFINED) right = predicate;
          // waren bisher keine Schnitte auf der Kante vermerkt und wurde 
          // jetzt ein abweichendes Prädikat ermittelt, 
          // dann das Pädikat "INTERSECT" setzen
          else if (right != INTERSECT && right != predicate) right = INTERSECT;
        }// for
      }// if
    }// getBorderPredicates
       
    void ResultUnitFactory::setSlidePredicates(Predicate predicate,
                                                size_t slide, size_t touch){
       list<size_t>::iterator first, iter;
       if (nonOrthogonalEdges[slide].size() < 2) return;
       first = nonOrthogonalEdges[slide].begin();
       if (predicate != UNDEFINED &&
           segments.get(*first).getPredicate() == UNDEFINED) {
         predicate =createPredicate(predicate, LEFT);
         if (touch%2 == 1) {
           if (predicate == OUTER) predicate = INNER;
           else predicate = OUTER;
         }// if
         for (iter  = nonOrthogonalEdges[slide].begin(); 
              iter != nonOrthogonalEdges[slide].end(); iter++) {
           Predicate oldPredicate = segments.get(*iter).getPredicate();
           if (oldPredicate == UNDEFINED || oldPredicate == NO_INTERSECT) {
               segments.set(*iter,predicate);
           }// if
           // alle Schnitte sind gesetzt
           // geändert für Prädikat "INTERSECT"
           else break;
        }// for
      }// if
    }// setSlidePredicates
       
    void ResultUnitFactory::evaluate(size_t i ){      
      list<size_t>::iterator first, left, right, orthogonal;
      Segment firstSegment, leftSegment, rightSegment, orthogonalSegment;
      // Existieren weniger als zwei Kanten, Funktion verlassen
      if (nonOrthogonalEdges[i].size()< 2) return;
      // Index (Iterator) der erste kante der Slide bestimmen
      first = nonOrthogonalEdges[i].begin();
      // Index (Iterator) auch für die linke und rechte Kante vwerwenden
      right = left = first;  
      // Erste Kante betimmen
      firstSegment = segments.get(*first);
      // Die Schleife solange abarbeiten, bis die rechte die letzte Kante 
      // der Slide ist
      for (right++; right !=  nonOrthogonalEdges[i].end(); 
           right++) {
        // Prädicat vorbelegen
        Predicate predicate = UNDEFINED;
        // linke Kante bestimmen
        leftSegment  = segments.get(*left);
        // rechte Kante bestimmen
        rightSegment = segments.get(*right);
        // Prädikat der Kante als mögliche linke Kante eines
        // PFace auswerten und Ergebnis übernehmen
        checkPredicate(leftSegment,LEFT,predicate); 
        // Prädikat der Kante als mögliche rechte Kante eines
        // PFace auswerten und Ergebnis übernehmen
        checkPredicate(rightSegment,RIGHT,predicate); 
        // ist das Prädikat der linken Kante nicht UNDEFINED,
        // so muss überprüft werden, ob die linke Kante die erste 
        // Kante der Slide berührt. 
        if (left != first && leftSegment.getPredicate() != UNDEFINED) {
          // berühren sich das Ende der ersten und das Ende der linken
          // Kante aber die Anfänge nicht
          if (firstSegment.getTail() == leftSegment.getTail()){
            // Berührung registrieren (Anfang der Slide)
            touchsOnLeftBorder[i]++;
          }// if
          // berühren sich der Anfang der ersten uud der Anfang der 
          // letzten Kante 
          if (firstSegment.getHead() == leftSegment.getHead()) {
            // Berührung registrieren (Ende der Slide)
            touchsOnLeftBorder[i+1]++;
          }// if
        }// if
        // konnte kein Predicate ermittelt werden
        if (predicate == UNDEFINED) {
          // check bottom 
          // Die orthogonalen Kanten auswerten,
          // zuerst die orthogonalen Kanten am Boden
          for (orthogonal  = orthogonalEdges[i].begin(); 
               orthogonal != orthogonalEdges[i].end(); 
               orthogonal ++) {  
            // ortogonale Kante laden
            orthogonalSegment = segments.get(*orthogonal);
            // berühren sich das Ende der linken Kante und das
            // Ende der ortogonalen Kante und das Ende der rechten Kante
            // und das Ende der orthogonalen Kante, dann sollte die Kante
            // ausgwertet werden
            if (leftSegment.getTail() == orthogonalSegment.getTail() && 
                rightSegment.getTail()== orthogonalSegment.getHead()) {
              //  cout << "Bottom" <<endl;
              // orthogonale Kante als rechte Kante für eine PFace 
              // auswerten
              checkPredicate(orthogonalSegment,RIGHT,predicate); 
              // Nach dem das Predicate bestimmt wurde, kann die
              // Schleife verlassen werden
              break;
            }// if
          }// for          
          // check top
          // Die orthogonalen Kanten auswerten,
          // jetzt die oberen Kanten
          for (orthogonal  = orthogonalEdges[i+1].begin(); 
               orthogonal != orthogonalEdges[i+1].end(); 
               orthogonal ++) {
            // orthogonale Kante laden
            orthogonalSegment = segments.get(*orthogonal);
            // berühren sich der Anfang der linken Kante und der Anfang 
            // der orthogonalen Kanten und der Anfang der rechten Kante
            // und der Anfang der orthogonalen Kante, dann sollte die Kante
            // ausgwertet werden
            if (leftSegment.getHead()  == orthogonalSegment.getTail() && 
                rightSegment.getHead() == orthogonalSegment.getHead()) {
              //  cout << "Top" <<endl;
              // orthogonale Kante als linke Kante für eine mögliches 
              // PFace auswerten
              checkPredicate(orthogonalSegment,LEFT,predicate);
              // nachdem ein Predicat bestimmt wurde, kann die Schleife 
              // verlassen werden
              break;
            }// if
          }// for
        }// if
        // Predikat der linken Kante setzen, falls erforderlich
        segments.set(*left, predicate);
        // Predicate der rechten Kante setzen, falls erforderlich
        segments.set(*right, predicate);
        // in der nächsten Runde, ist die ehemals rechte Kante, jetzt die
        // linke Kante
        left = right; 
      }// for
      // Berührungen der orthogonalen Kanten mit der ersten kante auswerten
      if (orthogonalEdges[i].size()!=0) {
        // orthogonale Kante bestimmen 
        orthogonalSegment = segments.get(*orthogonalEdges[i].begin());      
        // berührt das Ende der orthogonale Kante, das Ende
        // das ersten Kante der Slide ???
        if (firstSegment.getTail() == orthogonalSegment.getTail()) {
          // Berührung vermerken
          touchsOnLeftBorder[i]++;
        }// if 
      }// if
      // Falls erforderlich den Slide mit Prädikaten auffüllen 
      // Hierbi wird von letzten Element zum ersten Element iteriert
      list<size_t>::reverse_iterator last, riter;
      // Iteerator auf das letzte Element setzen
      riter = last = nonOrthogonalEdges[i].rbegin();
      // Letzte Kante laden
      rightSegment = segments.get(*last); 
      // Ist die erste Kante auf "UNDEFINED" gesetzt und hat die
      // letzte Kante ein von UNDEFINED abweichendes Prädikat, dann den Slide
      // durchlaufen
      if (firstSegment.getPredicate() == UNDEFINED && 
          rightSegment.getPredicate() != UNDEFINED) {
        // von der letzten zur ersten Kante
        for (riter++; riter != nonOrthogonalEdges[i].rend(); riter++) {
          // rechte Kante laden
          rightSegment= segments.get(*last);
          // linke Kante laden
          leftSegment = segments.get(*riter);
          // ist die linke Kante nicht gesetzt
          if (leftSegment.getPredicate() == UNDEFINED) {
            // Prädikat für die linke Kante aus der rechten Kante
            // bestimmen
            Predicate predicate =
              createPredicate(rightSegment.getPredicate(), LEFT);
            // Die Prädikat "UNDEFINED"
            if (predicate != UNDEFINED && predicate != INTERSECT) {
              // Predikat setzen
              segments.set(*riter, predicate);
            }// if
          }// if
          last++;
        }// for
      }// if
    }// evaluate   
          
    Predicate ResultUnitFactory::createPredicate(const Predicate source,
                                                 const Border border)const{
      Predicate predicate;
      switch (source) {        
        case LEFT_IS_INNER:  if(border == LEFT) predicate = OUTER;
                             else predicate = INNER;
                             break;    
        case RIGHT_IS_INNER: if(border == LEFT) predicate = INNER;
                             else predicate = OUTER;
                             break; 
        case NO_INTERSECT:   predicate = UNDEFINED;
                             break;   
        default:             predicate = source;         
      }// switch
      return predicate;   
    }// createPredicate
         
    void ResultUnitFactory::checkPredicate(const Segment& segment,
                                           const Border border, 
                                           Predicate& result)const{
      Predicate predicate = createPredicate(segment.getPredicate(), border);
      if (predicate == INNER || predicate == OUTER) {
        if (result == UNDEFINED) result = predicate;
        else if (!(result == predicate)&& (!pFaceIsCritical)) {
          cerr << *this;
          cerr << "Segment "<< segment <<endl;
          cerr << "Aktuelle Prädikat " << toString(predicate) << endl;
          cerr << "Zielprädikat " << toString(result) <<endl;
          NUM_FAIL("Different Predicates on edges.");
          // assert(false);
        }// else if 
      }// if
    }// checkPredicate
         
    void ResultUnitFactory::print(std::ostream& os, 
                                   std::string prefix,
                                   vector<list<size_t>> edges)const{
      list<size_t>::const_iterator iter;
      for (size_t i = 0; i < edges.size();i++) {
        os << prefix << "     Index:=" << i << " (";
        for (iter = edges[i].begin(); iter != edges[i].end();) {
          os << *iter; 
          iter++;
          if (iter != edges[i].end()) os << ", ";           
        }// for  
        os << ")" << endl;
      }// for
      os << prefix << "  )" << endl;
    }// print  

    std::ostream& ResultUnitFactory::print(std::ostream& os, 
                                           std::string prefix)const{
      os << "ResultUnitFactory(";
      if (segments.size() == 0) os << "is empty)" << endl;
      else {
        os << endl;
        if(this->pFaceIsCritical){
          os << prefix + "Source PFace is critical" << endl;
        }// if
        else os << prefix + "Source PFace is not critical" <<endl;
        os << prefix + "  ";
        segments.print(os, prefix+"  ");
        os << prefix + "  Non orthogonal edge for MSegments (" << endl;
        print(os, prefix, nonOrthogonalEdges);
        os << prefix + "  Orthogonal edge for MSegments (" << endl;
        print(os, prefix, orthogonalEdges);
        vector<size_t>::const_iterator iter;       
        os << prefix + "  Touch on left border(" << endl;
        for (size_t i = 0; i < touchsOnLeftBorder.size(); i++) {
          os << prefix << "    Index:="<< i << " (";
          os << touchsOnLeftBorder[i] << ")" << endl;
        }// for
        os << prefix << "  )" << endl;
        os << prefix << ")" << endl;
      }// else
       return os;
    }// operator <<
     
    std::ostream& operator <<(std::ostream& os, 
                              const ResultUnitFactory& factory){   
      return factory.print(os,"");
    }// operator 
    
    void ResultUnitFactory::getResultUnit(size_t slide, Predicate predicate,
                                           bool reverse, 
                                           const Point3DContainer& points, 
                                           ResultUnit& unit, State pfState, 
                                           SourceFlag source){ 
      list<size_t>::const_iterator left, right;
      if (slide < nonOrthogonalEdges.size()) {
        if (nonOrthogonalEdges[slide].size() > 1) {
          left = right = nonOrthogonalEdges[slide].begin();
          for (right++; right != nonOrthogonalEdges[slide].end(); right++) {
            Segment leftSegment  = segments.get(*left);
            Segment rightSegment = segments.get(*right);
            Predicate leftPredicate  = 
              createPredicate(leftSegment.getPredicate(),LEFT);
            Predicate rightPredicate = 
              createPredicate(rightSegment.getPredicate(),RIGHT);
            // Prädikat für die linke und rechte Kante müssen gleich sein
            // oder eins der Prädikate muss "INTERSECT" sein
            if ((leftPredicate == rightPredicate)|| 
               ((leftPredicate == INTERSECT && rightPredicate != INTERSECT)||
                (leftPredicate != INTERSECT && rightPredicate == INTERSECT))||
                 pfState == CRITICAL) {
              if ((leftPredicate == predicate)||(rightPredicate == predicate)||
                  pfState == CRITICAL) {
                if (reverse) {
                  Segment temp = leftSegment;
                  leftSegment  = rightSegment;
                  rightSegment = temp;
                }// if
                Point3D leftTail  = points.get(leftSegment.getTail());
                Point3D leftHead  = points.get(leftSegment.getHead());
                Point3D rightTail = points.get(rightSegment.getTail());
                Point3D rightHead = points.get(rightSegment.getHead());
                Segment3D left    = Segment3D(leftTail,leftHead);
                Segment3D right   = Segment3D(rightTail,rightHead);
                if (pfState == CRITICAL) {
                  Predicate p = leftPredicate;
                  if (p == INTERSECT) p = rightPredicate;
                  CriticalMSegment segment(left,right,source,p);
                  unit.addCMSegment(segment);
                }// if
                else {
                  MSegment segment(left, right);                  
                  unit.addMSegment(segment,false);
                }// else 
              }// if
            }// if
            else {
              if (pFaceIsCritical) cerr << "PFace is critical" << endl;
              else cerr << "PFace is not critical" << endl;
              cerr << "gesuchte Predikat:=" ;
              cerr << mregionops3::toString(predicate) <<endl;
              cerr << "Left Segment :=" << leftSegment << endl;
              cerr << "Right Segment:=" << rightSegment<< endl;
              cerr << *this;
              NUM_FAIL ("Predicate on left and right border are different.");
            }// else  
            left = right;
          }// for
        }//if
      }// if
    }// getResultPFace  
    
    Predicate ResultUnitFactory::calculateAreaPredicate(
        const Segment& left, const Segment& right)const{
      Predicate leftP      = left.getPredicate();
      Predicate rightP     = right.getPredicate();
      // Segmente definieren den Bereich als Innen
      if (leftP == RIGHT_IS_INNER || rightP == LEFT_IS_INNER || 
          leftP == INNER || rightP == INNER){
        // existieren Wiedersprüche
        if (leftP == LEFT_IS_INNER || rightP == RIGHT_IS_INNER ||  
            leftP == OUTER || rightP == OUTER){
          cerr << "Left Segment :=" << left << endl;
          cerr << "Right Segment:=" << right<< endl;
          NUM_FAIL ("Predicate on left and right segment are controvert.");
        }// if                
        return INNER;
       }// if
       // Segmente definieren den Bereich als Außen
       // Wiedersprpche sind nicht möglich
       else if (rightP == RIGHT_IS_INNER || leftP == LEFT_IS_INNER || 
                leftP  == OUTER || rightP == OUTER){          
          return OUTER;
       }// else if
       return UNDEFINED;
    }// calculateAreaPredicate
        
    bool ResultUnitFactory::intersects(size_t slide, bool& predicate)const{
      // Anzahl der Einträge bestimmen
      size_t size = this->nonOrthogonalEdges[slide].size();
      if(size < 2) {
        NUM_FAIL ("At least two segments are necessary."); 
      }// if
      // Status "RELEVANT" mit mehr als zwei Segmenten
      if(!pFaceIsCritical && size > 2){
        // Schnitt leigt vor
        predicate = true;
        // Ergebnis konnte ermittelt werden
        return true;
      }// if
      list<size_t>::const_iterator left  = nonOrthogonalEdges[slide].begin();
      list<size_t>::const_iterator right = left;
      right++;
      // Status "RELEVANT" mit genau zwei Segmenten
      if (!pFaceIsCritical && size == 2){
        Predicate result = calculateAreaPredicate(segments.get(*left),
                                                  segments.get(*right));
        // cout << "Prädikate:=" << mregionops3::toString(result) << endl;
        if (result == INNER) {
          predicate = true;
          return true;
        }// if
        else if (result == OUTER) {
          predicate = false;
          return true;
        }// else if
      }// if
      else { 
        // Status "CRITICAL"
        Predicate oldPredicate = UNDEFINED;
        for (;right != nonOrthogonalEdges[slide].end();right++){
          Predicate newPredicate = calculateAreaPredicate(segments.get(*left),
                                                          segments.get(*right));
          // cout << "Old Predicate:=";  
          // cout << mregionops3::toString(oldPredicate) << endl;
          // cout << "New Predicate:=";   
          // cout << mregionops3::toString(newPredicate) << endl;
          // cout << "Left segment.="   << segments.get(*left) << endl;
          // cout << "Right segments:=" << segments.get(*right) << endl;
          if (newPredicate == INNER) {
            predicate = true;
            return true;
          }// if
          // neues Prädikat übernhmen, wenn es gültig ist
          if(newPredicate != UNDEFINED) oldPredicate = newPredicate;
          // rechtes Segment ist jetzt linkes Segment
          left = right;;
        }// for
        if(oldPredicate == OUTER){
            predicate = false;
            return true;
        }// if          
      }// else
      predicate = false;
      return false;
    }// intersects
    
    bool ResultUnitFactory::inside(size_t slide, bool& predicate)const{
      // Anzahl der Einträge bestimmen
      size_t size = this->nonOrthogonalEdges[slide].size();
      if(size < 2) {
        NUM_FAIL ("At least two segments are necessary."); 
      }// if
      // Status "RELEVANT" mit mehr als zwei Segmenten
      if(!pFaceIsCritical && size > 2){
        // Schnitt liegt vor, kann nicht mehr innen liegen
        predicate = false;
        // Ergebnis konnte ermittelt werden
        return true;
      }// if
      list<size_t>::const_iterator left  = nonOrthogonalEdges[slide].begin();
      list<size_t>::const_iterator right = left;
      right++;
      // Status "RELEVANT" mit genau zwei Segmenten
      if (!pFaceIsCritical && size == 2){
        Predicate result = calculateAreaPredicate(segments.get(*left),
                                                  segments.get(*right));
        // cout << "Prädikate:=" << mregionops3::toString(result) << endl;
        if (result == INNER) {
          predicate = true;
          return true;
        }// if
        else if (result == OUTER) {
          predicate = false;
          return true;
        }// else if
      }// if
      else { 
        // Status "CRITICAL"
        Predicate oldPredicate = UNDEFINED;
        for (;right != nonOrthogonalEdges[slide].end();right++){
          Predicate newPredicate = calculateAreaPredicate(segments.get(*left),
                                                          segments.get(*right));
          // cout << "Old Predicate:=";  
          // cout << mregionops3::toString(oldPredicate) << endl;
          // cout << "New Predicate:=";   
          // cout << mregionops3::toString(newPredicate) << endl;
          // cout << "Left segment.="   << segments.get(*left) << endl;
          // cout << "Right segments:=" << segments.get(*right) << endl;
          if (newPredicate == OUTER) {
            predicate = false;
            return true;
          }// if
          // neues Prädikat übernhmen, wenn es gültig ist
          if(newPredicate != UNDEFINED) oldPredicate = newPredicate;
          // rechtes Segment ist jetzt linkes Segment
          left = right;;
        }// for
        if(oldPredicate == INNER){
            predicate = true;
            return true;
        }// if          
      }// else
      predicate = false;
      return false;
    }// inside

    bool ResultUnitFactory::intersects(std::vector<bool>& predicate){
      if(nonOrthogonalEdges.size() != predicate.size()){
        NUM_FAIL ("vector for predicates has a wrong size."); 
      }// if
      for (size_t i = 0; i < nonOrthogonalEdges.size(); i++){
        bool value;
        bool result  = intersects(i, value); 
        predicate[i] = value;
        if(!result)return false;
      }// for
      return true;
    }// intersects
    
    bool ResultUnitFactory::inside(std::vector<bool>& predicate){
      if(nonOrthogonalEdges.size() != predicate.size()){
        NUM_FAIL ("vector for predicates has a wrong size."); 
      }// if
      for (size_t i = 0; i < nonOrthogonalEdges.size(); i++){
        bool value;
        bool result  = inside(i, value); 
        predicate[i] = value;
        if(!result)return false;
      }// for
      return true;
    }// inside
     
/*
18 Class PFace

*/  
    PFace::PFace(size_t left, size_t right, const Point3DContainer& points, 
            const SegmentContainer& segments){
      Segment borderLeft  = segments.get(left);
      Segment borderRight = segments.get(right);
      MSegment::set(points.get(borderLeft.getTail()),
                    points.get(borderLeft.getHead()),
                    points.get(borderRight.getTail()),
                    points.get(borderRight.getHead())); 
      this->left      = left;
      this->right     = right;
      this->state     = UNKNOWN;     
    }// Konstruktor 
    
    PFace::PFace(const MSegmentData& mSeg, 
                 const GlobalTimeValues& timeValues,                 
                 Point3DContainer& points,
                 SegmentContainer& segments){
      
      Point2D start;
      Point2D end;
      double startTime = timeValues.getScaledStartTime(); 
      double endTime   = timeValues.getScaledEndTime();
      // Fallen die Initialpunkte zusammen
      if (!mSeg.GetPointInitial()) {
        // Start und Endpunkt am initialen Sgement bestimmen
        start = Point2D(mSeg.GetInitialStartX(), mSeg.GetInitialStartY());
        end   = Point2D(mSeg.GetInitialEndX(), mSeg.GetInitialEndY());        
      }// if
      else {
        // Start und Endpunkt an dem finalen Segment bestimmen
        start = Point2D(mSeg.GetFinalStartX(), mSeg.GetFinalStartY());
        end   = Point2D(mSeg.GetFinalEndX(), mSeg.GetFinalEndY());
      }// else
      // Startpunkte festlegen
      Point3D leftStart, rightStart, leftEnd, rightEnd;
      if ((start < end) == mSeg.GetInsideAbove()) {                     
        leftStart  = Point3D(mSeg.GetInitialStartX(), 
                             mSeg.GetInitialStartY(),
                             startTime);
        rightStart = Point3D(mSeg.GetInitialEndX(), 
                             mSeg.GetInitialEndY(),
                             startTime);
        leftEnd    = Point3D(mSeg.GetFinalStartX(), 
                             mSeg.GetFinalStartY(),
                             endTime);
        rightEnd   = Point3D(mSeg.GetFinalEndX(), 
                             mSeg.GetFinalEndY(),
                             endTime);
      }// if
      else {
        leftStart  = Point3D(mSeg.GetInitialEndX(), 
                             mSeg.GetInitialEndY(),
                             startTime);
        rightStart = Point3D(mSeg.GetInitialStartX(), 
                             mSeg.GetInitialStartY(),
                             startTime);
        leftEnd    = Point3D(mSeg.GetFinalEndX(), 
                             mSeg.GetFinalEndY(),
                             endTime);
        rightEnd   = Point3D(mSeg.GetFinalStartX(), 
                             mSeg.GetFinalStartY(),
                             endTime);        
      }// else 
      
      size_t iLeftStart = points.add(leftStart);
      size_t iLeftEnd   = points.add(leftEnd);
      size_t iRightStart= points.add(rightStart); 
      size_t iRightEnd  = points.add(rightEnd);
      
      this->leftStart   = points.get(iLeftStart);
      this->leftEnd     = points.get(iLeftEnd);
      this->rightStart  = points.get(iRightStart);
      this->rightEnd    = points.get(iRightEnd);
           
      // mittleres Halbsegment berechnen
      createMedianHS();
      // Indizes setzen
      medianHS.attr.faceno  = mSeg.GetFaceNo();
      medianHS.attr.edgeno  = mSeg.GetSegmentNo();
      medianHS.attr.cycleno = mSeg.GetCycleNo();
      medianHS.attr.coverageno = -1;
      medianHS.attr.partnerno  = -1;
      boundingRect = getBoundingRec(this->leftStart);
      boundingRect.Extend(getBoundingRec(this->leftEnd));
      boundingRect.Extend(getBoundingRec(this->rightStart));
      boundingRect.Extend(getBoundingRec(this->rightEnd)); 
          
      Segment leftSegment(iLeftStart,iLeftEnd, UNDEFINED);
      Segment rightSegment(iRightStart, iRightEnd, UNDEFINED);   
      this->left  = segments.add(leftSegment);
      this->right = segments.add(rightSegment);
      this->state = UNKNOWN;
    }// Konstruktor
       
    
    PFace::PFace(const PFace& pf){
      set(pf);
    }// Konstruktor
    
    void PFace::set(const PFace& pf){
      
      this->leftStart   = pf.leftStart;
      this->leftEnd     = pf.leftEnd;
      this->rightStart  = pf.rightStart;
      this->rightEnd    = pf.rightEnd;
      this->medianHS    = pf.medianHS;
      this->boundingRect= pf.boundingRect;
      this->intSegs     = pf.intSegs; 
      this->factory     = pf.factory; 
      this->state       = pf.state;
      this->left        = pf.left;
      this->right       = pf.right;       
    }// Konstruktor
    
    void PFace::setState(State state){
      this->state = state;
    }// setState      
    
    State PFace::getState() const{
      return state;
    }// getState
    
    bool PFace::existsIntSegs()const{
      return (this->intSegs.size() != 0);
    }// hasIntseg
    
    string PFace::toString(State state){
      switch (state) {
        case UNKNOWN:      return "UNKNOWN";
        case RELEVANT:     return "RELEVANT";
        case CRITICAL:     return "CRITICAL";
        case NOT_RELEVANT: return "NOT_RELEVANT";
        default: return "";
      }// switch
    }// toString
    
    std::ostream& operator <<(std::ostream& os, const PFace& pf){
      return pf.print(os,"");
    }// operator 
    
    std::ostream& PFace::print(std::ostream& os, std::string prefix)const{
      os << "PFace( " << endl; 
      os << prefix + "  left border index:=" << this->left;
      os << ", right border index:=" << this->right << "," << endl;
      os << prefix + "  state:=" << PFace::toString(state) << "," << endl;
      os << prefix + "  left start point:=" << this->leftStart;
      os << " , left end point:="<< this->leftEnd << "," << endl;
      os << prefix + "  right start point:=" << this->rightStart;
      os << " , right end point:=" << this->rightEnd << "," << endl;
      os << prefix + "  MedianHS:=" << this->medianHS << "," << endl;
      os << prefix + "  ";
      this->intSegs.print(os,"  "+prefix);
      os << prefix + "  ";
      this->factory.print(os,"  "+prefix);
      os << prefix + ")" << endl;
      return os;
    }// print

    PFace& PFace::operator =(
        const PFace& pf){
      set(pf);
      return *this;
    }// Operator =
    
    bool PFace::operator ==(const PFace& pf)const{
      if ((this->leftStart == pf.leftStart) &&
          (this->leftEnd == pf.leftEnd) &&
          (this->rightStart == pf.rightStart) &&
          (this->rightEnd == pf.rightEnd) &&
          (this->medianHS == pf.medianHS) &&
          (this->state == pf.state) &&
          (this->boundingRect == pf.boundingRect)&&
          (this->intSegs == pf.intSegs)) return true;
      return false;
    }// Operator == 
    
    void PFace::addIntSeg(const IntersectionSegment& seg){
      this->intSegs.addIntSeg(seg);
    }// addIntSeg
    
    void PFace::addIntSeg(const RationalPlane3D &planeSelf,
                          const RationalPlane3D &planeOther,
                          const RationalSegment3D &intSeg,
                          GlobalTimeValues &timeValues){
      bool result = planeSelf.isLeftSideInner(intSeg,planeOther);
      Predicate predicate    = LEFT_IS_INNER;   
      if (!result) predicate = RIGHT_IS_INNER;
      Segment2D segment = planeSelf.transform(intSeg).getD();
      if (this->state != CRITICAL) this->state = RELEVANT;     
      IntersectionSegment iSeg(intSeg,segment,predicate);
      timeValues.addTimeValue(iSeg.getTail().getT());
      timeValues.addTimeValue(iSeg.getHead().getT());
      addIntSeg(iSeg);
    }// addIntSeg
    
    IntersectionSegment PFace::createBorder( 
        const RationalPlane3D &planeSelf, Border border, Predicate predicate){
      Segment3D segment3D;
      Segment2D segment2D;
      if (border == LEFT) {
        segment3D = Segment3D(this->leftStart,this->leftEnd);
        segment2D = planeSelf.transform(segment3D).getD();
      }// if
      else {
        segment3D = Segment3D(this->rightStart,this->rightEnd);
        segment2D = planeSelf.transform(segment3D).getD();
      }// else
      return IntersectionSegment(segment3D,segment2D,predicate);
    }// createBorder 
    
    // Kernfunktion
    void PFace::addBorder(const RationalPlane3D &plane,
                          GlobalTimeValues &timeValues, 
                          Predicate predicate){
      IntersectionSegment iSeg = createBorder(plane,LEFT,predicate);
      timeValues.addTimeValue(iSeg.getTail().getT());
      timeValues.addTimeValue(iSeg.getHead().getT());
      addIntSeg(iSeg); 
      iSeg = createBorder(plane,RIGHT,predicate);
      timeValues.addTimeValue(iSeg.getTail().getT());
      timeValues.addTimeValue(iSeg.getHead().getT());
      addIntSeg(iSeg); 
    }// addBorder   
    
    // for pFace with intersection
    void PFace::addBorder(GlobalTimeValues &timeValues){
      RationalPlane3D plane(*this);       
      addBorder(plane,timeValues,UNDEFINED);
    }// addBorder 
    
    // for pFace without intersection
    void PFace::addBorder(GlobalTimeValues &timeValues, 
                          const SegmentContainer& segments, 
                          Predicate predicate){
      Predicate leftPredicate  = segments.get(left).getPredicate();
      Predicate rightPredicate = segments.get(right).getPredicate();  

      if (state == UNKNOWN && 
          leftPredicate  != UNDEFINED && 
          rightPredicate != UNDEFINED) {
        if (leftPredicate == predicate || rightPredicate == predicate) {
          state = RELEVANT;
          RationalPlane3D plane(*this);
          addBorder(plane,timeValues,predicate);          
        }// if
        else {
          state = NOT_RELEVANT;          
        }// if
      }// if

      // Ein Pface mit einem Schnitt konnte in einer vorhergehenden 
      // Runde nicht bearbeitet werden. Die Kanten werden mit
      // ihren Predikaten erneut hinzugefügt
      else if ((state == RELEVANT || state == CRITICAL) && 
              (leftPredicate != UNDEFINED || rightPredicate != UNDEFINED)) {
        RationalPlane3D plane(*this);        
        IntersectionSegment iSeg = createBorder(plane,LEFT,leftPredicate);
        addIntSeg(iSeg); 
        iSeg = createBorder(plane,RIGHT,rightPredicate);
        addIntSeg(iSeg); 
      }// if                  
    }// addBorder 
    
    void PFace::setBorderPredicate(SegmentContainer& segments, 
                                   Predicate predicate){
      Predicate leftPredicate  = segments.get(this->left).getPredicate();
      Predicate rightPredicate = segments.get(this->right).getPredicate();      
      if (leftPredicate  == UNDEFINED) segments.set(this->left,predicate);
      if (rightPredicate == UNDEFINED) segments.set(this->right,predicate);
    }// setBorderPredicate   
    
    bool PFace::intersectionOnPlane(PFace& other,
                                    const RationalPlane3D& planeSelf,
                                    GlobalTimeValues &timeValues){
      vector<RationalSegment3D> segments3D;
      vector<RationalSegment2D> segments2D;
      vector<RationalSegment2D> borders;
      // Segment speichern
      segments3D.push_back(RationalSegment3D(other.leftStart,other.leftEnd));
      segments3D.push_back(RationalSegment3D(other.rightStart,other.rightEnd));
      // Punkte transformieren               
      RationalPoint2D leftStart  = planeSelf.transform(other.leftStart);      
      RationalPoint2D leftEnd    = planeSelf.transform(other.leftEnd);       
      RationalPoint2D rightStart = planeSelf.transform(other.rightStart);      
      RationalPoint2D rightEnd   = planeSelf.transform(other.rightEnd); 
      // Segment erstellen und speichern
      segments2D.push_back(RationalSegment2D(leftStart,leftEnd));
      segments2D.push_back(RationalSegment2D(rightStart,rightEnd));
      // Punkte transformieren
      leftStart  = planeSelf.transform(this->leftStart);      
      leftEnd    = planeSelf.transform(this->leftEnd);       
      rightStart = planeSelf.transform(this->rightStart);      
      rightEnd   = planeSelf.transform(this->rightEnd); 
      // Segment erstellen und speichern
      borders.push_back(RationalSegment2D(leftStart,leftEnd));
      borders.push_back(RationalSegment2D(rightStart,rightEnd));
      borders.push_back(RationalSegment2D(leftStart,rightStart));
      borders.push_back(RationalSegment2D(leftEnd,rightEnd));
      // Verschneidung durchführen
      for (size_t i = 0; i < segments2D.size(); i++){
        std::set<RationalPoint2D> points;
        for (size_t j = 0; j < borders.size(); j++){
          RationalPoint2D iPoint;
          if(segments2D[i] == borders[i]){
            points.clear();
            break;
          }// if
          bool result = segments2D[i].intersection(borders[j],iPoint);
          if(result) {            
            points.insert(iPoint);  
          }// if  
        }// for
        if(points.size() > 1 ){    
          std::set<RationalPoint2D>::iterator first,second;
          second = first = points.begin();
          second++;
          mpq_class length  = (segments2D[i].getHead() - 
                               segments2D[i].getTail()).length();
          mpq_class length0 = (*first - segments2D[i].getTail()).length(); 
          mpq_class length1 = (*second - segments2D[i].getTail()).length(); 
          RationalVector3D vector = segments3D[i].getHead() - 
                                    segments3D[i].getTail();
          Point3D tail3D = segments3D[i].getTail() + length0/length*vector;
          Point3D head3D = segments3D[i].getTail() + length1/length*vector;
          // Schnittsegment bestimmen in 2D
          Point2D tail2D = (*first).getD();
          Point2D head2D = (*second).getD();      
          IntersectionSegment iSegment(IntersectionPoint(tail3D,tail2D),
                                       IntersectionPoint(head3D,head2D),
                                       NO_INTERSECT);
          // cout << Segment3D(this->leftStart, this->leftEnd)<< endl;
          // cout << Segment3D(this->rightStart, this->rightEnd)<< endl;
          // cout << iSegment.getSegment3D()<< endl << endl;
          Segment3D segment = iSegment.getSegment3D();
          if(!((segment.getHead() == this->leftEnd && 
                segment.getTail() == this->leftStart) || 
               (segment.getHead() == this->rightEnd && 
                segment.getTail() == this->rightStart))){
            timeValues.addTimeValue(iSegment.getTail().getT());
            timeValues.addTimeValue(iSegment.getHead().getT());
            addIntSeg(iSegment);
          }// if
        }// if
      }// for 
      this->state = CRITICAL;
      return true;
    }// intersectionOnPLane
      
    bool PFace::intersection(PFace& other,GlobalTimeValues &timeValues){
      Rectangle<2> bRec = boundingRect;
      // Boundingbox etwas vergrößern
      bRec.Extend(NumericUtil::eps2);
      // check bounding rectangles
      if (!(this->boundingRect.Intersects(other.boundingRect))) {
         return false; 
      }// if
      // create planes
      RationalPlane3D planeSelf(*this);
      RationalPlane3D planeOther(other);      
      // cout << setprecision(12) << planeSelf << endl;
      // cout << setprecision(12) << planeOther << endl;                  
      // check planes
      if (planeSelf.isParallelTo(planeOther)) {
        if (planeSelf.isCoplanarTo(planeOther)) {   
         intersectionOnPlane(other,planeSelf,timeValues);
         other.intersectionOnPlane(*this,planeOther,timeValues);  
        }// if 
        else { }// else
        return false;
      }// if
      RationalPoint3DExtSet intPointSet;    
      planeSelf.intersection(other, UNIT_A, intPointSet);  
      // We need exactly two intersection points.
      if (intPointSet.size() != 2) return false;    
      planeOther.intersection(*this, UNIT_B, intPointSet);    
      // There is no intersection
      RationalSegment3D intSeg;
      if (!intPointSet.getIntersectionSegment(intSeg)) return false;  
      IntersectionSegment iSeg;
      // create and save result segments
      addIntSeg(planeSelf,planeOther,intSeg,timeValues);
      other.addIntSeg(planeOther,planeSelf,intSeg,timeValues); 
      return true;    
    }// intersection   
    
    void PFace::first(double t1, double t2, Point3DContainer& points,
                       SegmentContainer& segments, bool pFaceIsCritical){ 
      intSegs.first(t1, t2, points, segments, pFaceIsCritical);
    }// first
      
    void PFace::next(double t1, double t2, Point3DContainer& points, 
                     SegmentContainer& segments,bool  pFaceIsCritical){
      intSegs.next(t1, t2, points, segments, pFaceIsCritical); 
    }// next
    
    bool PFace::finalize(Point3DContainer& points, SegmentContainer& segments, 
                         GlobalTimeValues& timeValues){
      // Variable für das rechte und linke Prädikat
      Predicate leftPredicate, rightPredicate;
      // Variable für das Ergebnis
      bool result;
      // Für relevante und Kritrsche PFaces
      if (this->state == RELEVANT || this->state == CRITICAL) {
        bool pFaceIsCritical = false;
        if(this->state == CRITICAL) pFaceIsCritical = true;
        // Ergebnis Factory erstellen
        this->factory = ResultUnitFactory(points, timeValues, *this, 
                                          pFaceIsCritical);
        // Ergebnis Factory auswerten
        this->factory.evaluate();
        // Grenzen des Ergebnisses bestimmen
        this->factory.getBorderPredicates(leftPredicate,rightPredicate); 
        // Wird für die Kanten kein definierter Wert geleifert, ist die 
        // Berechnung nicht erfolgreich geweseniefern 
        if (leftPredicate == UNDEFINED && rightPredicate == UNDEFINED) {
          // Berechnung war nicht erfolgreich 
          result = false;
        }// if
        // wird für beide Kanten ein definierter Wert geliefert,
        // so kann  dieser Wert übernommen werden
        else if (leftPredicate != UNDEFINED && rightPredicate != UNDEFINED) {
          // Wert übernehmen
          segments.set(this->left,leftPredicate);
          segments.set(this->right,rightPredicate); 
          // Berechnung war erfolgreich 
          result =true;
        }// else if
        // für eine der Kanten wird das Prädikat undefiert ermittelt
        else {
          // wird für die linke Kante das Prädikat "INTERSECT" geliefert,
          // dann dieses übernehmen
          if (leftPredicate == INTERSECT) {
            // Segment entsprechend setzen
            segments.set(this->left,leftPredicate);
            // Ergebnis wurde noch nicht vollständig ermittelt.
            // Die Kanteninformation muss an der anderen Kante 
            // hinzugefügt werden
            result = false;                
          }// if
          // wird für die rechte Kante das Prädikat "INTERSECT" geleifert,
          // dan dieses übernehmen
          if (rightPredicate == INTERSECT) {
            // segment entsprechend setzen
            segments.set(this->right,rightPredicate);
            // Ergebnis wurde noch nicht vollständig ermittelt.
            // Die Kanteninformation muss an der anderen Kante 
            // hinzugefügt werden
            result = false;
          }// if
        }// else      
      }// if
      else if (this->state == NOT_RELEVANT) result = true;
      // Für PFaces ohne Schnitte wird das Prädikat von der einen kante
      // zur anderen Kante weiter gereicht
      else {
        // Prädikat der linken Kante bestimmen
        leftPredicate  = segments.get(this->left).getPredicate();
        // Prädikat der rechten Kante bestimmen
        rightPredicate = segments.get(this->right).getPredicate();
        // die Prädikate "INNER" und "OUTER" werden von der linken Kante
        // auf die rechte Kante übernommen, falls erforderlich
        if (leftPredicate == INNER || leftPredicate == OUTER) {
          // Prädikat ändern, wenn dieses "UNDEFINED"
          if (rightPredicate == UNDEFINED) {
            // Prädikat der rechten kante setzen
            segments.set(this->right,leftPredicate);
          }// if
          // Kanteninformation des PFace ist vollständig
          result = true;
        }// if
        // die Prädikate "INNER" und "OUTER" werden von der rechten Kante
        // auf die linke Kante übernommen, falls erforderlich
        if (rightPredicate == INNER || rightPredicate == OUTER) {
          // Prädikat ändern, falls dieses "UNDEFINED" ist
          if (leftPredicate == UNDEFINED) {
            // Prädikat der linken Kante setzen
            segments.set(this->left,rightPredicate);
          }// if
          // Kanteninformation des PFace ist volständig
          result = true;
        }// if
        // Information für das PFace ist bereits vollständig
        result = false;
      }// else 
      return result;
    }// finalize
    
    bool PFace::intersects(Point3DContainer& points, 
                           GlobalTimeValues& timeValues,
                           std::vector<bool>& predicate){
      // Für relevante und Kritrsche PFaces
      if (this->state == RELEVANT || this->state == CRITICAL) {
        bool pFaceIsCritical = false;
        if(this->state == CRITICAL) pFaceIsCritical = true;
        // Ergebnis Factory erstellen
        this->factory = ResultUnitFactory(
          points, timeValues, *this, pFaceIsCritical);
        // Ergbnis vervollständigen
        this->factory.evaluate();
        // Ergebnis Factory auswerten
        return this->factory.intersects(predicate);
      }// if
      return false;
    }// intersects
    
    bool PFace::inside(Point3DContainer& points, 
                       GlobalTimeValues& timeValues,
                       std::vector<bool>& predicate){
      // Für relevante und Kritrsche PFaces
      if (this->state == RELEVANT || this->state == CRITICAL) {
        bool pFaceIsCritical = false;
        if(this->state == CRITICAL) pFaceIsCritical = true;
        // Ergebnis Factory erstellen
        this->factory = ResultUnitFactory(
          points, timeValues, *this, pFaceIsCritical);
        // Ergbnis vervollständigen
        this->factory.evaluate();
        // Ergebnis Factory auswerten
        return this->factory.inside(predicate);
      }// if
      return false;
    }// inside
    
    void PFace::getResultUnit(size_t slide, Predicate predicate,
                              bool reverse, 
                              const Point3DContainer& points, 
                              ResultUnit& unit, SourceFlag source){
      this->factory.getResultUnit(slide,predicate,reverse,points,unit,
                                  this->state,source);
    }// getResultPFace   
/*
19 class FaceCycleInfo

*/      
    FaceCycleInfo::FaceCycleInfo():touch(false){
    }// Konstruktor
      
    void FaceCycleInfo::setFirstIndex(size_t firstIndex){
      this->firstIndex = firstIndex;
    }// setFirstIndex
      
    void FaceCycleInfo::setTouch(){
      this->touch = true;
    }// setTouch
      
    bool FaceCycleInfo::getTouch() const{
      return this->touch;
    }// getTouch
      
    size_t FaceCycleInfo::getFirstIndex() const{
      return this->firstIndex;
    }// getFirstIndex
    
/*
20 class SourceUnit

*/          
    SourceUnit::SourceUnit():pFaceTree(4,8),testRegion(0),
        testRegionDefined(false){      
    }// Konstruktor
    
    SourceUnit::SourceUnit(const SourceUnit& other):pFaceTree(4,8),
        testRegion(0),testRegionDefined(false){
      set(other);
    }// Konstruktor
    
    SourceUnit::~SourceUnit(){
      vector<PFace*>::iterator iter;
      for (iter = pFaces.begin(); iter != pFaces.end(); iter++) {           
        delete *iter;
      }// for
    }// Destruktor
    
    void SourceUnit::set(const SourceUnit& other){
      vector<PFace*>::iterator iter;
      for (iter = pFaces.begin(); iter != pFaces.end(); iter++) {           
        delete *iter;
      }// for
      this->pFaces            = vector<PFace*>();
      this->pFaceTree         = mmrtree::RtreeT<2, size_t>(4,8);
      this->itersectedPFace   = other.itersectedPFace;
      this->segments          = other.segments;
      this->faceCycleInfo     = other.faceCycleInfo;
      this->testRegion        = other.testRegion;
      this->testRegionDefined = other.testRegionDefined;
      for (size_t i = 0; i < other.pFaces.size(); i++) {        
        PFace* pFace = new PFace(PFace(*other.pFaces[i]));
        size_t index = this->pFaces.size();
        this->pFaces.push_back(pFace);
        Rectangle<2> boundigRec = pFace->getBoundingRec();
        this->pFaceTree.insert(boundigRec,index);
      }// for
    }// set     

    void SourceUnit::addPFace(const Segment& leftSegment, 
                              const Segment& rightSegment, 
                              const Point3DContainer& points){    
      size_t left  = this->segments.add(leftSegment);
      size_t right = this->segments.add(rightSegment);
      size_t index = pFaces.size();
      PFace* pFace = new PFace(left,right,points,this->segments);
      pFaces.push_back(pFace);      
      Rectangle<2> boundigRec = pFace->getBoundingRec();
      pFaceTree.insert(boundigRec,index);                                
    }// addPFace
    
    void SourceUnit::addMSegmentData(const MSegmentData& mSeg, 
         const GlobalTimeValues& timeValues,
         Point3DContainer& points){
      size_t index = pFaces.size();
      PFace* pFace = new PFace(mSeg, timeValues, points,this->segments);
      pFaces.push_back(pFace);      
      Rectangle<2> boundigRec = pFace->getBoundingRec();
      pFaceTree.insert(boundigRec,index);  
      createFaceCycleEntry(pFace,index);      
    }// addMSegmentData
        
    bool SourceUnit::isEmpty()const{
      return pFaces.size()==0;
    }// is Empty
    
    bool SourceUnit::intersect(const SourceUnit& other)const{
      Rectangle<2> bRecA = this->pFaceTree.getBBox();
      Rectangle<2> bRecB = other.pFaceTree.getBBox();
      return bRecA.Intersects(bRecB);
    }// intersect
            
    void SourceUnit::addToResultUnit(ResultUnit& result)const{
      for (size_t i = 0; i < pFaces.size(); i ++) {
        MSegment msegment = static_cast<MSegment> (*pFaces[i]);
        result.addMSegment(msegment,false);
      }// for
    }// addToResult
    
    bool SourceUnit::intersection(SourceUnit& other, 
                                  GlobalTimeValues& timeValues){
      bool result =false;
      for (size_t i = 0; i < this->pFaces.size(); i++) {
        PFace* pFaceA = this->pFaces[i];
        Rectangle<2> bRec = (*pFaceA).getBoundingRec();
        // Boundingbox etwas vergrößern
        bRec.Extend(NumericUtil::eps2);
        // Iterator über die gefundenen PFaces erstellen
        std::unique_ptr<mmrtree::RtreeT<2, size_t>::iterator> 
          it(other.pFaceTree.find(bRec)); 
        size_t const* j;  
        while ((j = it->next()) != 0) {
          PFace* pFaceB = other.pFaces[*j];
          pFaceA->intersection(*pFaceB,timeValues);
                 
        //  cout << "A:" << endl << setprecision(12) << *pFaceA;
        //  cout << "B:" << endl << setprecision(12) << *pFaceB;
          
        }// while
        if (pFaceA->existsIntSegs() || pFaceA->getState()==CRITICAL) {
          pFaceA->addBorder(timeValues);
          this->itersectedPFace.push_back(i);
          touchFaceCycleEntry(pFaceA);
        }// if
      }//for 
      for (size_t j = 0; j < other.pFaces.size(); j++) {
        PFace* pFaceB =other.pFaces[j];
        if (pFaceB->existsIntSegs() || pFaceB->getState()==CRITICAL) {
          pFaceB->addBorder(timeValues);
          other.itersectedPFace.push_back(j);
          other.touchFaceCycleEntry(pFaceB);
        }// if
      }// for
      // Für Zyklen ohne Schnitte, jetzt für ein PFace 
      // Bezugsprädikate einfügen 
      checkFaceCycleEntrys(other);
      other.checkFaceCycleEntrys(*this);      
      return result;
    }// intersection
    
    bool SourceUnit::finalize(Point3DContainer& points, 
                              GlobalTimeValues& timeValues, 
                              Predicate predicate,
                               SourceUnit& other){
      // Vektor mit Bool-Werten für den Bearbeitungsstand der PFaces
      vector<bool> ok = vector<bool>(pFaces.size(),false);
      // 
      // zuerst alle PFaces mit Schnitten
      //
      // über alle PFaces, welche an Schnitten beteidigt sind
      for (size_t i = 0; i < this->itersectedPFace.size(); i++) {
        // index eines geschnittenden PFace laden
        size_t index = itersectedPFace[i];
        // wurde das PFace noch nicht behandelt
        
        // cout << *this;     
        
        if (!ok[index]) {
          // Ergebnis ermitteln
          bool result = this->pFaces[index]->finalize(
            points, this->segments,timeValues); 
          // erfolgreiche Bearbeitung vermerken
          ok[index] = result;
        }// if
      }// for       
      //
      // jetzt alle PFaces ohne Schnitte und die Pfaces welche nicht
      // verarbeitet werden konnten, verarbeiten
      // 
      bool finalize;
      size_t j = 0;
      // zwei Durchläufe, falls nach dem ersten Durchlauf noch 
      // kein Ergebnis vorliegt
      
      // cout << *this;      
      
      do {
        // Ergebnis ist korrekt 
        finalize = true;
        // über alle PFaces
        for (size_t i = 0; i < this->pFaces.size(); i++) {
          // wurde das PFace bereits erfolgreich bearbeitet
          if (!ok[i]) {
            // Grenzen des PFace dem Ergebnis hinzufügen bzw. die
            // Grenzen aktualisieren
            this->pFaces[i]->addBorder(timeValues,segments,predicate);
            // Ergebnis ermitteln
            bool result = this->pFaces[i]->finalize(
              points,this->segments,timeValues);
            // hier ist kein weiteres Ergebnis zu erwarten
            if( this->pFaces[i]->getState() == CRITICAL && 
                j == 3 &&  result == false){
              result = true;              
            }// if            
            // war die operation erolgreich
            if (result != true) finalize = false;
            // erfolgreiche Verarbeitung vermerken
            else ok[i] = result;                        
          }// if
        }// for  
        // Anzahl der Durchläufe erhöhen
        j++;
        // zwei Durchläufe werden maximal benötigt. Der dritte
        // Durchlauf läuft ohne Bearbeitung durch
        if (j == 3 ) {  
          for (size_t i = 0; i < this->pFaces.size(); i++) {
            // wurde das PFace bereits erfolgreich bearbeitet
            if (!ok[i] && this->pFaces[i]->getState() != CRITICAL){  
              // liegt es innerhalb der andern SourceUnit            
              if (other.isInside(this->pFaces[i])) {
                this->pFaces[i]->setBorderPredicate(segments,INNER);
              }// if
              else {
                this->pFaces[i]->setBorderPredicate(segments,OUTER);
              }// else
            }// if
          }// for
        }// if
        if(j > 4){
          cerr << " Finalized Pfaces:= ";
          for (size_t i = 0; i< ok.size(); i++) {
             cerr << ok[i];
          }// for
          cerr << endl;
          cerr << *this;
          NUM_FAIL("Finalize for Source Unit is not possible.");
        }// if
      } while (!finalize);
      return true;
    }// finalize
    
    void SourceUnit::intersects(Point3DContainer& points, 
                                GlobalTimeValues& timeValues,
                                SourceUnit& other,
                                std::vector<bool>& predicates){
      // Vektor mit bool-Werten für den Bearbeitungsstand der PFaces
      vector<bool> ok = vector<bool>(pFaces.size(),false);
      // Vektor mit Zwischenergbnissen für das Prädikat "intersects"
      std::vector<bool> resultPredicates(timeValues.size()-1);
      // Vektor mit Endergebnissen für das Prädikat "intersects"
      predicates = std::vector<bool>(timeValues.size()-1,false);
      // Ergebnisstand
      bool result = false;
      // Auswertung aller PFaces mit Schnitten von UnitA
      for (size_t i = 0; i < this->itersectedPFace.size(); i++) {
        // index eines geschnittenden PFace laden
        size_t index = this->itersectedPFace[i];
        // kann das Predicate bestimmt werden 
        if(this->pFaces[index]->intersects(points, timeValues, 
                                           resultPredicates)){
          // Ergebnis übernehmen
          for(size_t i = 0; i < resultPredicates.size(); i++){
            // if(predicates[i]){
            //  cout << "Predicate intersects for slide " << i;
            //  cout << ", true" <<endl;
            // }// if
            // else {
            //   cout << "Predicate intersects for slide " << i;
            //   cout << ", false" <<endl;
            // }// else          
            predicates[i] = predicates[i] || resultPredicates[i];
          }// for
          result = true;
        }// if
        // PFace wurde verarbeitet        
        ok[i] = true;  
      }// for
      // Auswertung aller PFaces mit Schnitten von UnitB
      for (size_t i = 0; i < other.itersectedPFace.size(); i++) {
        // index eines geschnittenden PFace laden
        size_t index = other.itersectedPFace[i];
        // kann das Predicate bestimmt werden 
        if(other.pFaces[index]->intersects(points, timeValues, 
                                           resultPredicates)){
          // Ergebnis übernehmen
          for(size_t i = 0; i < resultPredicates.size(); i++){
            // if(predicates[i]){
            //  cout << "Predicate intersects for slide " << i;
            //  cout << ", true" <<endl;
            // }// if
            // else {
            //   cout << "Predicate intersects for slide " << i;
            //   cout << ", false" <<endl;
            // }// else          
            predicates[i] = predicates[i] || resultPredicates[i];
          }// for
          result = true;
        }// if
        // PFace wurde verarbeitet        
        ok[i] = true;  
      }// for
      // Gibt es ein Ergebnis
      if (result) return;
      // Bisher liegt kein Ergebnis vor
      // Ein weiteres PFace auswerten, da es kein Ergebnis gibt
      for (size_t i = 0; i < this->pFaces.size(); i++) {
        // PFace wurde noch nicht verarbeitet und ist nicht kritisch
        if (!ok[i] && this->pFaces[i]->getState() != CRITICAL){  
          // liegt es innerhalb der andern SourceUnit            
          if (other.isInside(this->pFaces[i])) {
            this->pFaces[i]->setBorderPredicate(segments,INNER);
          }// if
          else {
            this->pFaces[i]->setBorderPredicate(segments,OUTER);
          }// else
          this->pFaces[i]->intersects(points,timeValues,predicates);
          return;
        }// if
      }// for
      // Obwohl alle PFace der SourceUnit verarbeitet wurden, konnte der
      // Status nicht geklärt werden. Beide SourceUnits beschreiben das gleiche
      // Objekt. 
      predicates = std::vector<bool>(timeValues.size()-1,true);
    }// intersects
    
    void SourceUnit::inside(Point3DContainer& points, 
                            GlobalTimeValues& timeValues,
                            SourceUnit& other,
                            std::vector<bool>& predicates){
      // Vektor mit bool-Werten für den Bearbeitungsstand der PFaces
      vector<bool> ok = vector<bool>(pFaces.size(),false);
      // Vektor mit Zwischenergbnissen für das Prädikat "inside"
      std::vector<bool> resultPredicates(timeValues.size()-1);
      // Vektor mit Endergebnissen für das Prädikkat "inside"
      predicates = std::vector<bool>(timeValues.size()-1,true);
      // Ergebnisstand
      bool result = false;
      // Auswertung aller PFaces mit Schnitten
      for (size_t i = 0; i < this->itersectedPFace.size(); i++) {
        // index eines geschnittenden PFace laden
        size_t index = itersectedPFace[i];
        // kann das Predicate bestimmt werden 
        if(this->pFaces[index]->inside(points, timeValues, 
                                       resultPredicates)){
          // Ergebnis übernehmen
          for(size_t i = 0; i < resultPredicates.size(); i++){
            // if(predicates[i]){
            //  cout << "Predicate inside for slide " << i;
            //  cout << ", true" <<endl;
            // }// if
            // else {
            //  cout << "Predicate inside for slide " << i;
            //  cout << ", false" <<endl;
            // }// else          
            predicates[i] = predicates[i] && resultPredicates[i];
          }// for
          result = true;
        }// if
        // PFace wurde verarbeitet        
        ok[i] = true;  
      }// for
      // Gibt es ein Ergebnis
      if (result) return;
      // Bisher liegt kein Ergebnis vor
      // Ein weiteres PFace auswerten, da es kein Ergebnis gibt
      for (size_t i = 0; i < this->pFaces.size(); i++) {
        // PFace wurde noch nicht verarbeitet und ist nicht kritisch
        if (!ok[i] && this->pFaces[i]->getState() != CRITICAL){  
          // liegt es innerhalb der andern SourceUnit            
          if (other.isInside(this->pFaces[i])) {
            this->pFaces[i]->setBorderPredicate(segments,INNER);
          }// if
          else {
            this->pFaces[i]->setBorderPredicate(segments,OUTER);
          }// else
          this->pFaces[i]->inside(points,timeValues,predicates);
          return;
        }// if
      }// for
      // Obwohl alle PFace der SourceUnit verarbeitet wurden, konnte der
      // Status nicht geklärt werden. Beide SourceUnits beschreiben das gleiche
      // Objekt. 
      predicates = std::vector<bool>(timeValues.size()-1,true);
    }// inside
    
        
    void SourceUnit::getResultUnit(size_t slide, Predicate predicate,
                              bool reverse, 
                              const Point3DContainer& points, 
                              ResultUnit& unit, SourceFlag source){
       for (size_t i = 0; i < this->pFaces.size(); i++) {
         this->pFaces[i]->getResultUnit(slide,predicate,reverse,points,unit,
                                        source);
       }// for
    }// getResultPFace

/*
    bool SourceUnit::intersection(SourceUnit& other, GlobalTimeValues& timeValues){
      bool result =false;
      vector<PFace*>::iterator iter;
      for (iter = this->pFaces.begin(); iter != this->pFaces.end(); iter++) {
        PFace* pFaceA = *iter;
        Rectangle<2> bRec = (*pFaceA).getBoundingRec();
        // Boundingbox etwas vergrößern
        bRec.Extend(NumericUtil::eps2);
        RationalPlane3D planeSelf(*pFaceA);
        // Iterator über die gefundenen Dreiecke erstellen
        std::unique_ptr<mmrtree::RtreeT<2, size_t>::iterator> 
          it(other.pFaceTree.find(bRec));      
        size_t const *bRecIndex;
        while((bRecIndex = it->next()) != 0) {
          PFace* pFaceB = other.pFaces[*bRecIndex];
          RationalPlane3D planeOther(*pFaceB);
          // check planes
          if (planeSelf.isParallelTo(planeOther)) {
            if(planeSelf.isCoplanarTo(planeOther)) {
              pFaceA->setState(CRITICAL);
              pFaceB->setState(CRITICAL);           
            }// if 
            break;
          }// if
          RationalPoint3DExtSet intPointSet;
          planeSelf.intersection(*pFaceB, PFACE_A, intPointSet);
          // We need exactly two intersection points.
          if (intPointSet.size() != 2) break; 
          planeOther.intersection(*pFaceA, PFACE_B, intPointSet);  
          // There is no intersection
          RationalSegment3D intSeg;
          if(!intPointSet.getIntersectionSegment(intSeg)) break;  
          IntersectionSegment iSeg;
          // create and save result segments  
          pFaceA->addIntSeg(planeSelf,planeOther,intSeg,timeValues);
          pFaceB->addIntSeg(planeOther,planeSelf,intSeg,timeValues);      
          result = true;
        }// while
        if(pFaceA->existsIntSegs()){
          pFaceA->addBorder(planeSelf, timeValues, UNDEFINED);
        }// if
      }// for
      for (iter = other.pFaces.begin(); iter != other.pFaces.end(); iter++) {
        PFace* pFaceB = *iter;
        if(pFaceB->existsIntSegs()){
          RationalPlane3D planeOther(*pFaceB);
          pFaceB->addBorder(planeOther, timeValues,UNDEFINED);
        }// if
      }// for
      return result;
    }// intersection
*/    

    std::ostream& SourceUnit::print(std::ostream& os, std::string prefix)const{
      os << "SourceUnit (";
      if (segments.size() == 0) os << "is empty)" << endl;
      else {
        os << endl;
        os << prefix + "  ";
        this->segments.print(os,prefix+"  ");
        os << prefix << "  PFaces (" << endl;
        for (size_t i = 0; i < pFaces.size(); i++) {
          os << prefix << "    Index:=" << i << ", " ; 
          this->pFaces[i]->print(os,prefix+"    ");
        }// for
        os << prefix << "  )"<<endl;
        os << prefix << ")"<<endl;
      }// else
      return os;
    }// print
    
    std::ostream& operator << (std::ostream& os, const SourceUnit& unit){
      unit.print(os,"");
      return os;
    }// Operator <<
    
    bool SourceUnit::operator == (const SourceUnit& unit)const{
      if (!(this->segments == unit.segments)) return false;
      if (this->pFaces.size() != unit.pFaces.size()) return false;
      for (size_t i = 0; i < this->pFaces.size(); i++) {
        if (!(*(this->pFaces[i]) == *(unit.pFaces[i]))) return false;
      }// for
      return true;
    }// Operator ==
    
    SourceUnit& SourceUnit::operator =(const SourceUnit& unit){
      set(unit);
      return *this;
    }// Operator =
    
    bool SourceUnit::lessByMedianHS(const PFace* pf1, const PFace *pf2){
      return pf1->lessByMedianHS(static_cast<MSegment>(*pf2));
    }// return
    
    bool SourceUnit::logicLess(const PFace* pf1, const PFace *pf2){
      return pf1->logicLess(static_cast<MSegment>(*pf2));
    }// return
    
    void SourceUnit::reSort(){
      // Alte Einträge im R-Tree löschen
      size_t size = pFaces.size();
      for (size_t i = 0; i < size; i++) {
        // PFace laden
        PFace* pf1 = pFaces[i];
        // Boundingbox bestimmen
        Rectangle<2> boundigRec = pf1->getBoundingRec();
        // Eintrag löschen
        pFaceTree.erase(boundigRec,i);
        // PFace-Eintrage für das erste Halbsegment    
        pf1->setLeftDomPoint(true);
        // Kantennummer setzen
        pf1->setSegmentNo(i);
        // Zurückschreiben
        pFaces[i] = pf1;
        // PFace für das zweite Halbsegment erzeugen
        PFace* pf2 = new PFace(*pf1); 
        // PFace-Eintrage für das zweite Halbsegment    
        pf2->setLeftDomPoint(false);
        // PFace einfügen
        pFaces.push_back(pf2);  
      }// for      
      // PFaces neu sortieren
      sort(pFaces.begin(),pFaces.end(),SourceUnit::lessByMedianHS);
      // Testregion aufbauen
      this->testRegion.StartBulkLoad();
      // über alle PFaces      
      for (size_t i = 0; i < pFaces.size(); i++) {
        // Halbsegment des PFaces zur Testregion hinzufügen
        this->testRegion += pFaces[i]->getMedianHS();
      }// for
      // Note: Sorting is already done.
      this->testRegion.EndBulkLoad(false, true, true, true);
      // Testregion ist jetzt definiert
      this->testRegionDefined = true;
      // Ergebnis aus der Testregion in die PFaces eintragen     
      for (size_t i = 0; i < pFaces.size(); i++) {
        HalfSegment halfSegment;
        this->testRegion.Get(i, halfSegment);
        pFaces[i]->copyIndicesFrom(&halfSegment);
      }// for
      // Sort pFaces by faceno, cycleno and segmentno:
      sort(pFaces.begin(), pFaces.end(), SourceUnit::logicLess);
      // Erase the second half of mSegments, 
      // which contains all MSegments with right dominating point:
      for (size_t i = pFaces.size()/2; i < pFaces.size(); i++) {
        delete pFaces[i];
      }// for
      pFaces.erase(pFaces.begin() + pFaces.size()/2,  pFaces.end());
      // Rebuild R-Tree
      for (size_t i = 0; i < pFaces.size(); i++) {
        PFace* pf1 = pFaces[i];
        // Boundingbox bestimmen
        Rectangle<2> boundigRec = pf1->getBoundingRec();
        // Eintrag löschen
        pFaceTree.insert(boundigRec,i);
        // Indizes des PFace überprüfen
        createFaceCycleEntry(pf1,i);
      }// for       
    }// reSort
    
    void SourceUnit::createFaceCycleEntry(const PFace* pf, size_t index){
      int segmentNo = pf->getSegmentNo();
      int cycleNo   = pf->getCycleNo();
      int faceNo    = pf->getFaceNo();
      if(segmentNo == -1 || cycleNo == -1 || faceNo == -1){
        NUM_FAIL("SegmentNo, CycleNo or FaceNo for a PFace schould not be -1.");
      }// if
      if (segmentNo == 0) {
        while (faceNo >= static_cast<int>(faceCycleInfo.size())) {
          faceCycleInfo.push_back(vector<FaceCycleInfo>());
        }// while
        while (cycleNo >= static_cast<int>(faceCycleInfo[faceNo].size())) {
          faceCycleInfo[faceNo].push_back(FaceCycleInfo());
        }// while
        faceCycleInfo[faceNo][cycleNo].setFirstIndex(index);
      }// if        
    }// checkIndizes
    
    void SourceUnit::touchFaceCycleEntry(const PFace* pf){
      int cycleNo   = pf->getCycleNo();
      int faceNo    = pf->getFaceNo();
      if (cycleNo == -1 || faceNo == -1) return; 
      if (faceNo < static_cast<int>(faceCycleInfo.size())) {
        if (cycleNo < static_cast<int>(faceCycleInfo[faceNo].size())) {
          faceCycleInfo[faceNo][cycleNo].setTouch();
        }// if
      }//if     
    }// touchFaceCycleSegment
    
    void SourceUnit::checkFaceCycleEntrys(SourceUnit& other){
      for (size_t i = 0; i < faceCycleInfo.size(); i++) {
        for (size_t j = 0; j < faceCycleInfo[i].size();j++) {      
          FaceCycleInfo info = faceCycleInfo[i][j];
          if (!info.getTouch()){
            if (!other.testRegionDefined) {
              other.createTestRegion();
            }// if
            PFace* pFace = pFaces[info.getFirstIndex()];
            if (other.isInside(pFace)) {
              pFace->setBorderPredicate(this->segments,INNER);
            }// if
            else {
              pFace->setBorderPredicate(this->segments,OUTER);
            }// else
          }// if
        }// for
      }// for
    }// checkFaceCycleEntrys
    
    void SourceUnit::printFaceCycleEntrys(){       
      for (size_t i = 0; i < faceCycleInfo.size(); i++) {
        for (size_t j = 0; j < faceCycleInfo[i].size();j++) {
          FaceCycleInfo info = faceCycleInfo[i][j];
          cout << "FaceNo:=" << i << ", CycleNo:=" << j;
          cout << ", FirstIndex:=" << info.getFirstIndex();
          if (info.getTouch()) {
            cout << ", Touch:=true"<< endl;
          }// if
          else {
            cout << ", Touch:=false"<< endl;
          }// else
        }// for
      }// for     
    }// printFaceCycleInfo
        
    void SourceUnit::createTestRegion(){
      this->testRegion.StartBulkLoad();
      for (size_t i = 0; i < pFaces.size(); i++) {
        HalfSegment medianHS = pFaces[i]->getMedianHS();
        medianHS.attr.edgeno  = i;
        medianHS.attr.cycleno = -1;
        medianHS.attr.faceno  = -1;        
        medianHS.SetLeftDomPoint(false);
        this->testRegion += medianHS;
        medianHS.SetLeftDomPoint(true);
        this->testRegion += medianHS;
      }// for     
      this->testRegion.EndBulkLoad(true, true, true, true);
      this->testRegionDefined = true;
    }// createTestRegion
    
    bool SourceUnit::isInside(const PFace* pFace) {
      if (!(this->testRegionDefined)) {
        createTestRegion();
      }// if
      HalfSegment halfSegment = pFace->getMedianHS();
      Point left      = halfSegment.GetLeftPoint();
      Point middle    = halfSegment.middlePoint();
      Point right     = halfSegment.GetRightPoint();
      Point nearleft  = Point(true,(left.GetX()+middle.GetX())/2,
                                   (left.GetY()+middle.GetY())/2);
      Point nearright = Point(true,(right.GetX()+middle.GetX())/2,
                                   (right.GetY()+middle.GetY())/2);
      bool result1 = testRegion.Contains(nearleft);
      bool result2 = testRegion.Contains(middle);
      if(result1 == result2){
        return result1;
      }// if
      else {
        bool result3 = testRegion.Contains(nearright);
        if (result1 == result2) return result1;
        else if (result1 == result3) return result1;
        return result2;
      }// else  
    }// isInside  
    
/*
21 Class SourceUnitPair      

*/     
    SourceUnitPair::SourceUnitPair(double orginalStartTime /* = 0*/,
                                   double orginalEndTime   /* = 1 */,
                                   double scale /*= 1*/):
        timeValues(scale, orginalStartTime,orginalEndTime){
    }// Konstruktor
    
    SourceUnitPair::SourceUnitPair(const Interval<Instant>& orginalInterval):
                                   timeValues(orginalInterval){
    }// Konstruktor
    
    void SourceUnitPair::setScaleFactor(double scale){
      // cout << "Scale:=" << scale << endl;
      timeValues.setScaleFactor(scale);
    }// setScaleFactor
          
    void SourceUnitPair::addPFace(SourceFlag flag, Segment3D& leftSegment, 
                                  Segment3D& rightSegment){
      Point3D leftStart = leftSegment.getTail();
      Point3D leftEnd   = leftSegment.getHead();
      Point3D rightStart= rightSegment.getTail();
      Point3D rightEnd  = rightSegment.getHead();
      size_t iLeftStart = points.add(leftStart);
      size_t iLeftEnd   = points.add(leftEnd);
      size_t iRightStart= points.add(rightStart); 
      size_t iRightEnd  = points.add(rightEnd);
      Segment left(iLeftStart, iLeftEnd, UNDEFINED);
      Segment right(iRightStart, iRightEnd, UNDEFINED);      
      if (flag == UNIT_A) {
        unitA.addPFace(left,right,points); 
      }// if
      else {
        unitB.addPFace(left,right,points);
      }// else     
    }// addPFace
      
    void SourceUnitPair::addMSegmentData(const MSegmentData& mSeg, 
                                         SourceFlag flag){
      if (flag == UNIT_A){
        unitA.addMSegmentData(mSeg, this->timeValues, points); 
      }// if
      else {
        unitB.addMSegmentData(mSeg, this->timeValues, points); 
      }// else     
    }// addPFace
    
    std::ostream& SourceUnitPair::print(std::ostream& os, 
                                        std::string prefix)const{
      os << "SourceUnitPair (" << endl;
      os << prefix + "  ";
      this->points.print(os, prefix+ "  ");
      os << prefix + "  ";
      this->timeValues.print(os, prefix +"  ");
      os << prefix + "  UnitA:= ";
      this->unitA.print(os, prefix + "  ");
      os << prefix + "  UnitB:= ";
      this->unitB.print(os, prefix + "  ");
      if (result.size() == 0) os << prefix << "  No result exist" << endl;
      else {
        os << prefix << "  Result (" << endl;
        for (size_t i = 0; i < result.size(); i++) {
          os << prefix + "    ";
          result[i].print(os, prefix + "    ");
        }// for
        os << prefix +"  )" << endl;        
      }// if
      os << prefix + ")" << endl;    
      return os;      
    }// print
    
    std::ostream& operator <<(std::ostream& os, 
                              const SourceUnitPair& unitPair){
      unitPair.print(os,"");
      return os;
    }// Operator <<
    
    bool SourceUnitPair::operate(SetOp setOp){  
      if (unitA.isEmpty()|| unitB.isEmpty() ||
         (!unitA.intersect(unitB))) {
        if (setOp == INTERSECTION) {
          // Result is empty: nothing to do.
          return false;
        }// if
        if (setOp == MINUS) {
          if (unitA.isEmpty()) {
            // Result is empty: nothing to do.
            return false;
          }// if
          // unitB is empty
          else {
            result = vector<ResultUnit>(1,ResultUnit());
            // unitA.createResultUnit(result[0]); 
            result[0] = ResultUnit(timeValues.getOrginalStartTime(),
                                   timeValues.getOrginalEndTime());
            unitA.addToResultUnit(result[0]);            
            result[0].finalize();
            return false;
          }// if
        }// if
        // setOp == UNION
        result = vector<ResultUnit>(1,ResultUnit());
        // unitA.createResultUnit(result[0]);
        result[0] = ResultUnit(timeValues.getOrginalStartTime(),
                               timeValues.getOrginalEndTime());
        unitA.addToResultUnit(result[0]);
        unitB.addToResultUnit(result[0]);
        result[0].finalize();
        return false;
      }// if
      // Intersection
      unitA.intersection(unitB, timeValues);
      // Finalize
      bool inverseB = false;
      Predicate predicateA = OUTER;
      Predicate predicateB = OUTER;
      if (setOp == MINUS){
        inverseB   = true;
        predicateB = INNER;
      }// if
      else if (setOp == INTERSECTION){
        predicateA = INNER;
        predicateB = INNER;
      }// else if        
      // cout << points;
      // cout << timeValues;
      // cout << unitA;
      // cout << unitB;      
      unitA.finalize(points, timeValues, predicateA, unitB);  
      // cout << unitA;       
      unitB.finalize(points, timeValues, predicateB, unitA);      
      // cout << unitB;      
      // get result Units          
      if (timeValues.size() > 1){
        result = vector<ResultUnit>(timeValues.size()-1, ResultUnit());
        double t1,t2;
        size_t i = 0;
        timeValues.orginalFirst(t1,t2);
        do{
          result[i] = ResultUnit(t1,t2);
          unitA.getResultUnit(i,predicateA,false,   points,result[i],UNIT_A);
          unitB.getResultUnit(i,predicateB,inverseB,points,result[i],UNIT_B);
          // cout << result[i] <<endl;
          result[i].evaluateCriticalMSegmens(setOp);
          // cout << result[i] <<endl;
          result[i].finalize();
          i++;
        } while (timeValues.orginalNext(t1, t2));  
      }// if
      return false;      
    }// operate
    
    void SourceUnitPair::createSourceUnit(const Interval<Instant>& interval, 
                          MRegion* mregion,
                          SourceFlag sourceFlag){
      MRegion* temp;
      URegionEmb unitRestrict;
      URegionEmb* unit;
      const DbArray<MSegmentData>* array;     

      Periods intervalAsPeriod(1);
      intervalAsPeriod.Add(interval);        
      temp = new MRegion(1);
      temp->AtPeriods(&intervalAsPeriod, mregion);
      temp->Get(0, unitRestrict);        
      array = temp->GetMSegmentData();      
      unit = new URegionEmb(unitRestrict.timeInterval,
                            unitRestrict.GetStartPos());
      unit->SetSegmentsNum(unitRestrict.GetSegmentsNum());
      unit->SetBBox(unitRestrict.BoundingBox());
      if(sourceFlag == UNIT_A) {
        Rectangle<3> bBox = unitRestrict.BoundingBox();
        setScaleFactor(bBox.MaxD(0) - bBox.MinD(0));
      }// if
      MSegmentData segment;
      for(int i = 0; i < unit->GetSegmentsNum();i++){
        unit->GetSegment(array, i, segment);
        addMSegmentData(segment, sourceFlag);
      }// for  
      delete temp;
      delete unit;        
    }// CreateSourceUnit
    
    void SourceUnitPair::createResultMRegion(MRegion* resMRegion){
      DbArray<MSegmentData>* array = 
        (DbArray<MSegmentData>*)resMRegion->GetFLOB(1);
      for (size_t i = 0; i < result.size(); i++){        
        if (result[i].size()!=0) {
          URegionEmb* ure = result[i].convertToURegionEmb(array);
          resMRegion->Add(*ure);
          delete ure;
        }// if
      }// for
    }// createResultMRegion
    
    size_t SourceUnitPair::countResultUnits()const{
      return result.size();
    }// countResultUnits
     
    ResultUnit SourceUnitPair::getResultUnit(size_t slide)const{
      return result[slide];
    }// getResultUnit 
    
    bool SourceUnitPair::predicate(PredicateOp predicateOp){  
      if (unitA.isEmpty()|| unitB.isEmpty()){
        return false;
      }// if
      // Intersection
      unitA.intersection(unitB, timeValues);
      if (predicateOp == INTERSECTS){ 
        unitA.intersects(points,timeValues,unitB,predicates);
      }// if
      else {
        unitA.inside(points,timeValues,unitB,predicates);
      }// else
      return false;      
    }// predicate    

    void SourceUnitPair::createResultMBool( MBool* resMBool ){
      if (timeValues.size() > 1){
        double t1,t2,t3;
        // cout << " timeValues " << timeValues <<endl;
        this->timeValues.orginalFirst(t1,t2);
        bool value = predicates[0];
        size_t i = 1;
        while (timeValues.orginalNext(t2, t3)){
          // cout << "t1:=" << t1 << endl;
          // cout << "t2:=" << t2 << endl;
          // if (predicates[i]) cout << "value:= true"  << endl;
          // else               cout << "value:= false" << endl; 
          if(value != predicates[i]){
            CcBool predicate(true, value);        
            UBool ubool(timeValues.createInterval(t1,t2), predicate, predicate);
            resMBool->Add(ubool);
            value = predicates[i];
            t1 = t2;         
          }// if
          t2 = t3;
          i++; 
        }// while  
        CcBool predicate(true, value);        
        UBool ubool(timeValues.createInterval(t1,t2), predicate, predicate);
        resMBool->Add(ubool);
      }// if
      else {
        double t1 = timeValues.getOrginalStartTime();
        double t2 = timeValues.getOrginalEndTime();  
        CcBool predicate(true, false);
        UBool ubool(timeValues.createInterval(t1,t2), predicate, predicate);
        resMBool->Add(ubool);
      }// else 
    }// createResultMBool
    
/*
22 class SetOperator

*/
    SetOperator::SetOperator(MRegion* const _mRegionA, 
                             MRegion* const _mRegionB,
                             MRegion* const _mRegionResult):
        mRegionA(_mRegionA), 
        mRegionB(_mRegionB), 
        mRegionResult(_mRegionResult){
    }// Konstruktor

    void SetOperator::operate(SetOp setOp){
      // Beide MRegionen müssen definiert sein
      if (!mRegionA->IsDefined() || !mRegionB->IsDefined()) {
        mRegionResult->SetDefined(false);
        return;
      }// if
      // Compute the RefinementPartition of the two MRegions
      RefinementPartition< MRegion, MRegion, URegionEmb, URegionEmb> 
        rp(*mRegionA, *mRegionB);
      // cout << "RefinementPartition with " << rp.Size() << " units created.";
      // MRegion des Ergebnisses löschen
      mRegionResult->Clear();
      // Speicherbereich der MSegmentData löschen
      ((DbArray<MSegmentData>*)mRegionResult->GetFLOB(1))->clean();
      // Füllvorgang beginnt
      mRegionResult->StartBulkLoad();
      // For each interval of the refinement partition
      for (unsigned int i = 0; i < rp.Size(); i++) {
        Interval<Instant> interval;
        int aPos, bPos;
        bool aIsEmpty, bIsEmpty;
        // liefere den entsprechenden Eintrag
        rp.Get(i, interval, aPos, bPos);         
        SourceUnitPair unitPair(interval);
        // Zeitspanne ist 0, dieser Fall trat beim Test auf
        if(interval.start == interval.end) continue;
        // Bestimmen ob eine der beiden zu erzuegenden Units leer ist  
        aIsEmpty = (aPos == -1);
        bIsEmpty = (bPos == -1);
        // Vereinfachungen suchen     
        if (!aIsEmpty) {
          unitPair.createSourceUnit(interval, mRegionA, UNIT_A);
        }// if
        if (!bIsEmpty) {
          unitPair.createSourceUnit(interval, mRegionB, UNIT_B);
        }// if
        unitPair.operate(setOp);        
        // cout << unitPair;               
        unitPair.createResultMRegion( mRegionResult);        
      }// for
      mRegionResult->EndBulkLoad(false);
    }// operate  
    
    PredicateOperator::PredicateOperator(MRegion* const _mRegionA, 
                                         MRegion* const _mRegionB,
                                         MBool* const  _mBool):
        mRegionA(_mRegionA), 
        mRegionB(_mRegionB), 
        mBool(_mBool){
    }// Konstruktor
    
    void PredicateOperator::operate(PredicateOp predicateOp){
      // Beide MRegionen müssen definiert sein
      if (!mRegionA->IsDefined() || !mRegionB->IsDefined()) {
        mBool->SetDefined(false);
        return;
      }// if
      // Compute the RefinementPartition of the two MRegions
      RefinementPartition< MRegion, MRegion, URegionEmb, URegionEmb> 
        rp(*mRegionA, *mRegionB);
      // cout << "RefinementPartition with " << rp.Size() << " units created.";
      // MBool des Ergebnisses löschen
      mBool->Clear();
      // For each interval of the refinement partition
      for (unsigned int i = 0; i < rp.Size(); i++) {
        Interval<Instant> interval;
        int aPos, bPos;
        bool aIsEmpty, bIsEmpty;
        // liefere den entsprechenden Eintrag
        rp.Get(i, interval, aPos, bPos);         
        SourceUnitPair unitPair(interval);
        // Zeitspanne ist 0, dieser Fall trat beim Test auf
        if(interval.start == interval.end) continue;
        // Bestimmen ob eine der beiden zu erzuegenden Units leer ist  
        aIsEmpty = (aPos == -1);
        bIsEmpty = (bPos == -1);
        // Vereinfachungen suchen     
        if (!aIsEmpty) {
          unitPair.createSourceUnit(interval, mRegionA, UNIT_A);
        }// if
        if (!bIsEmpty) {
          unitPair.createSourceUnit(interval, mRegionB, UNIT_B);
        }// if
        unitPair.predicate(predicateOp);        
        // cout << unitPair;               
        unitPair.createResultMBool(mBool);        
      }// for
    }// operate  

  } // end of namespace mregionops3
} // end of namespace temporalalgebra