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

[1] Implementation of the MRegionOps3Algebra

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
      // cout << *this << endl;    
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
      if(rightStart.distance(leftStart) > rightEnd.distance(leftEnd)){
        // Cross product of vector ab and ac
        this->normalVector = (rightStart - leftStart) ^ (leftEnd - leftStart);
        // check point d on plane
        if (!NumericUtil::nearlyEqual(distance2ToPlane(rightEnd), 0.0, 
               NumericUtil::eps * NumericUtil::epsRelaxFactor)) {
           cerr << "Distance" << distance2ToPlane(rightEnd).get_d() << endl;
           cerr << *this << endl;
           cerr << pf <<endl;
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
      mpq_class d0 = head.distance2(tail);
      mpq_class d1 = distance2ToPlane(head);
      mpq_class d2 = distance2ToPlane(tail);
      if(d0 > 1) {
        d1 = d1/d0;
        d2 = d2/d0;
      }// if
      // do not evaluate a segment that is parallel to the plane
      if(NumericUtil::nearlyEqual(d1, 0, 
           NumericUtil::eps * NumericUtil::epsRelaxFactor) && 
         NumericUtil::nearlyEqual(d2, 0, 
           NumericUtil::eps * NumericUtil::epsRelaxFactor))return false; 
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
      // cout << result;
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
      for (size_t i = 0, j = 0 ; i < edgesPFace.size();i++) {
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
      if (!NumericUtil::nearlyEqual(distance2ToPlane(point),0.0, 
             NumericUtil::eps * NumericUtil::epsRelaxFactor)) {
        cerr << setprecision(14);
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
    
     void RationalPlane3D::transform(
         const vector<RationalSegment3D>& segment3D,
         vector<RationalSegment2D>& segment2D) const{
       for(size_t i = 0; i < segment3D.size(); i++){
         segment2D.push_back(transform(segment3D[i]));
       }// for 
     }// transform
/*
7 Class IntersectionPoint

*/   
    IntersectionPoint::IntersectionPoint():x(0),y(0),z(0),w(0){
    }// Konstruktor
   
    IntersectionPoint::IntersectionPoint(const IntersectionPoint& point){
      set(point);
    }// Konstruktor
    
    IntersectionPoint::IntersectionPoint(const RationalPoint3D& point3D, 
                                         const RationalPoint2D& point2D){
      if (!NumericUtil::nearlyEqual(point3D.getZ(), point2D.getY())) {
        cerr << "RationalPoint3D:=" << point3D << endl;
        cerr << "RationalPoint2D:=" << point2D << endl;
        NUM_FAIL("Point3D and Point2D don't discribe the same.");
      }// if
      this->x = point3D.getX();
      this->y = point3D.getY();
      this->z = point3D.getZ();
      this->w = point2D.getX();
    }// Konstruktor
    
    IntersectionPoint::IntersectionPoint(mpq_class x, mpq_class y, 
                                         mpq_class z, mpq_class w){
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
    RationalPoint3D IntersectionPoint::getRationalPoint3D() const{
      return RationalPoint3D(x,y,z);
    }// getPoint3D
      
    RationalPoint2D IntersectionPoint::getRationalPoint2D() const{
      return RationalPoint2D(w,z);
    }// getPoint2D
      
    mpq_class IntersectionPoint::getX()const{
      return x;
    }// getX
      
    mpq_class IntersectionPoint::getY()const{
      return y;
    }// getY
      
    mpq_class IntersectionPoint::getZ()const{
      return z;
    }// getZ
      
    mpq_class IntersectionPoint::getW()const{
      return w;        
    }// getW
      
    mpq_class IntersectionPoint::getT()const{
      return z;    
    }// getT
    
    Rectangle<3> IntersectionPoint::getBoundingBox()const{
      double array[3] = {x.get_d(),y.get_d(),z.get_d()};
      return Rectangle<3>(true,array,array);
    }// getBoundingBox 
          
    std::ostream& operator <<(std::ostream& os, 
                              const IntersectionPoint& point){
      os << "IntersectionPoint("; 
      os << point.x.get_d() << ", " << point.y.get_d() << ", ";
      os << point.z.get_d() << ", " << point.w.get_d() << ")";
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
    
    IntersectionSegment::IntersectionSegment(
        const RationalSegment3D& segment3D, 
        const RationalSegment2D& segment2D,
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
      
    RationalSegment3D IntersectionSegment::getRationalSegment3D()const{
      return Segment3D(tail.getRationalPoint3D().getD(),
                       head.getRationalPoint3D());
    }// getSegment3D
    
    RationalSegment2D IntersectionSegment::getRationalSegment2D()const{
      return Segment2D(tail.getRationalPoint2D().getD(),
                       head.getRationalPoint2D());
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
    
    bool IntersectionSegment::isOutOfRange(mpq_class t)const{
      if (NumericUtil::lower(this->head.getT(),t)) return true;
      return NumericUtil::nearlyEqual(this->head.getT(),t);
    }// isOutOfRange
    
    bool IntersectionSegment::isLeftOf(const IntersectionSegment& intSeg)const{
      mpq_class tail1T = this->getTail().getT();
      mpq_class head2T = intSeg.getHead().getT();
      mpq_class tail2T = intSeg.getTail().getT();
      // Precondition: 
      // this->getTail().getT() is inside the interval 
      // [intSeg.getTail().getT(), intSeg.getHead().getT()]
      // and this and intSeg don't intersect in their interior.
      if (NumericUtil::nearlyEqual(tail2T, head2T)){
        mpq_class head1W = this->getHead().getW();
        mpq_class tail1W = this->getTail().getW();
        mpq_class tail2W = intSeg.getTail().getW();
        if (NumericUtil::nearlyEqual(tail1T, tail2T)){
          if( NumericUtil::lower(tail2W, tail1W)) return false; 
          else return true;
        }// if
        else {
          if( NumericUtil::lower(tail2W, head1W)) return false; 
          else return true;
        }// else
      }// if
      if (NumericUtil::lower(tail1T,tail2T) ||
          NumericUtil::greater(tail1T,head2T)) {
        cerr << "this:="   << *this << endl;
        cerr << "other:="  << intSeg << endl;
        cerr << "tail2T:=" << tail2T << endl;
        cerr << "tail1T:=" << tail1T << endl;
        cerr << "head2T:=" << head2T << endl;
        NUM_FAIL ("t must between the t value form tail und haed.");
      }// if
      RationalSegment2D segment2D1 =  this->getRationalSegment2D();
      RationalSegment2D segment2D2 = intSeg.getRationalSegment2D();
      mpq_class sideOfStart = segment2D2.whichSide(segment2D1.getTail());   
      if (sideOfStart >   NumericUtil::eps) return true;
      if (sideOfStart < - NumericUtil::eps) return false;
      mpq_class sideOfEnd = segment2D2.whichSide(segment2D1.getHead());
      return sideOfEnd > NumericUtil::eps;
    }// bool
    
    RationalPoint3D IntersectionSegment::evaluate(mpq_class t) const {
      mpq_class headT = this->getHead().getT();
      mpq_class tailT = this->getTail().getT();
      RationalPoint3D head3D = head.getRationalPoint3D();
      RationalPoint3D tail3D = tail.getRationalPoint3D();
      // Precondition:
      // t is between t on tail and haed
      if(!(NumericUtil::between(tailT, t, headT))){
        cerr << "this:="   << *this << endl;
        cerr << "tailT:=" << tailT.get_d() << endl;
        cerr << "t:="     << t.get_d() << endl;
        cerr << "headT:=" << headT.get_d() << endl;
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
      RationalVector3D u = head3D - tail3D;
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
      return (tail3D + s * u); 
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
      if (segment2->getRationalSegment2D().isLeft(head1.getRationalPoint2D())) 
         return true;
      if (segment1->getRationalSegment2D().isLeft(head2.getRationalPoint2D())) 
         return false; 
      // segment1 is collinear to segment2    
      if (NumericUtil::lower(  head1.getT(), head2.getT())) return false;
      if (NumericUtil::greater(head1.getT(), head2.getT())) return true;
      // head1.getT() == head2.getT(), head1.getW() == head2.getW() 
      if(segment1->getPredicate() < segment2->getPredicate())  return true;
      return false;
    }// IntSegCompare  
/*
10 Class PlaneSweepAccess

*/       
    void PlaneSweepAccess::first(mpq_class t1, mpq_class t2, 
                                 Point3DContainer& points,
                                 SegmentContainer& segments,
                                 bool pFaceIsCritical){
      NUM_FAIL ("method must override.");
    }// first

    void PlaneSweepAccess::next(mpq_class t1, mpq_class t2, 
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
    
    bool IntSegContainer::hasMoreSegsToInsert(mpq_class t)const{
      if (intSegIter == intSegs.end()) return false;
      IntersectionPoint tail((*intSegIter)->getTail());
      return NumericUtil::nearlyEqual(tail.getT(),t);
    }// hasMoreSegsToInsert
    
    void IntSegContainer::first(mpq_class t1, mpq_class t2, 
                                Point3DContainer& points,
                                SegmentContainer&  segments,
                                bool pFaceIsCritical){ 
      intSegIter = intSegs.begin();
      next(t1,t2,points,segments,pFaceIsCritical);      
    }// first
    
    void IntSegContainer::next(mpq_class t1, mpq_class t2, 
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
        RationalPoint3D tail, head;
        IntersectionSegment* segment = *activeIter;
        if (segment->isOrthogonalToTAxis()) {
          tail = segment->getTail().getRationalPoint3D();
          head = segment->getHead().getRationalPoint3D(); 
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
    bool DoubleCompare::operator()(const mpq_class& d1, 
                                   const mpq_class& d2) const{ 
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
    
    Interval<Instant> GlobalTimeValues::createInterval(
        double start, double end, bool lc/*=true*/, bool rc/*=false*/) const{
      Instant starttime(datetime::instanttype);
      Instant endtime(datetime::instanttype);
      starttime.ReadFrom(start);
      endtime.ReadFrom(end);
      return (Interval<Instant>(starttime, endtime, lc, rc));
    }// createInterval
    
    mpq_class GlobalTimeValues::computeOrginalTimeValue(
        mpq_class scaledTimeValue)const {      
      mpq_class temp = scaledTimeValue/scale;  
      mpq_class result = (1.0 - temp) * orginalStartTime + 
                         temp * orginalEndTime;
      return result;
    }// computeOrginalTimeValue 
          
    void GlobalTimeValues::addTimeValue(mpq_class t){
      if (NumericUtil::greaterOrNearlyEqual(t, 0) &&
          NumericUtil::lowerOrNearlyEqual(t, this->scale)){
        time.insert(t);
      }// if
      else {
        cerr << setprecision(9);
        cerr << *this;
        cerr << setprecision(9);
        cerr << t.get_d() << endl;
        NUM_FAIL ("Time value don,t be between starttime und endtime");
      }// else  
    }// addTimeValue
    
    void GlobalTimeValues::addStartAndEndtime(){
      time.insert(0);
      time.insert(this->scale);
    }// addStartAndEndtime
    
    size_t GlobalTimeValues::size()const{
      return time.size();
    }// size
         
    std::ostream& operator <<(std::ostream& os, 
                              const GlobalTimeValues& timeValues){
      return timeValues.print(os,"");
    }// Operator <<
    
    std::ostream& GlobalTimeValues::print(std::ostream& os, 
                                          std::string prefix)const{
      std::set<mpq_class, DoubleCompare>::const_iterator iter;
      os << "GlobalTimeValues(" << endl;
      os << prefix + "  Orginal start time:= ";
      os << this->orginalStartTime;
      os << ", Orginal end time:= ";
      os << this->orginalEndTime << "," << endl;
      os << prefix + "  Scaled start time:= 0, Scaled end time:= ";
      os << this->scale << "," <<endl;
      os << prefix + "  Time values (";
      if (time.size() == 0) os << "is empty)" << endl;
      else {       
        for (iter = this->time.begin(); 
            iter != this->time.end(); iter++){
          if (iter != this->time.begin()) os << ", " ;      
          os << (*iter).get_d();
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
      std::set<mpq_class, DoubleCompare>::iterator iter1,iter2;
      for (iter1  = this->time.begin(),iter2  = other.time.begin(); 
           iter1 != this->time.end(); iter1++,iter2++){
        if(!(NumericUtil::nearlyEqual(*iter1,*iter2))) return false;
      }// for
      return true;
    }// Operator ==
    
     bool GlobalTimeValues::scaledFirst(mpq_class& t1, mpq_class& t2){
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
      
     bool GlobalTimeValues::scaledNext(mpq_class& t1, mpq_class& t2){
       if((++timeIter) != time.end()){
         this->t1 = t1   = this->t2;
         this->orginalT1 = this->orginalT2;
         this->t2 = t2   = *timeIter;
         this->orginalT2 = computeOrginalTimeValue(this->t2);
         return true;
       }// if
       return false;
     }// scaledNext  
     
    bool GlobalTimeValues::orginalFirst(mpq_class& t1, mpq_class& t2){
      if(time.size() < 2) return false;
      this->timeIter  = time.begin();
      this->t1        = *timeIter;
      this->timeIter++;
      this->t2        = *timeIter; 
      this->orginalT1 = t1 = computeOrginalTimeValue(this->t1);
      this->orginalT2 = t2 = computeOrginalTimeValue(this->t2);
      return true;
    }// orginalFirst
    
    bool GlobalTimeValues::orginalNext(mpq_class& t1, mpq_class& t2){
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
14 class PResultFace 
 
*/
    PResultFace::PResultFace(){  
      set(Point3D(0,0,0),Point3D(0,0,0),
          Point3D(1,0,0),Point3D(1,0,0));
    }// Konstruktor
    
    PResultFace::PResultFace(const PResultFace& other){
      set(other);      
    }// Konstruktor

    PResultFace::PResultFace(const Segment3D& left, const Segment3D& right){
      set(left.getTail(),left.getHead(),right.getTail(),right.getHead());
    }// Konstruktor

    PResultFace::PResultFace(const Segment3D& left, 
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
    
    PResultFace::PResultFace(const MSegmentData& mSeg, 
                             const GlobalTimeValues& timeValues){
      Point2D start;
      Point2D end;
      double startTime = timeValues.getScaledStartTime(); 
      double endTime   = timeValues.getScaledEndTime();
      // Fall the initial points together
      if (!mSeg.GetPointInitial()) {
        // Determine startpoint and endpoint on the initial segment
        start = Point2D(mSeg.GetInitialStartX(), mSeg.GetInitialStartY());
        end   = Point2D(mSeg.GetInitialEndX(), mSeg.GetInitialEndY());        
      }// if
      else {
        // Determine startpoint and endpoint on the final segment
        start = Point2D(mSeg.GetFinalStartX(), mSeg.GetFinalStartY());
        end   = Point2D(mSeg.GetFinalEndX(), mSeg.GetFinalEndY());
      }// else
      // Set start points
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
      // Calculate middle halfsegment
      createMedianHS();
      // Set indexes
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

    void PResultFace::set(const Point3D leftStart, const Point3D leftEnd,
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
        
    void PResultFace::set(const PResultFace& prFace){
      this->leftStart   = prFace.leftStart;
      this->leftEnd     = prFace.leftEnd;     
      this->rightStart  = prFace.rightStart;
      this->rightEnd    = prFace.rightEnd;
      this->medianHS    = prFace.medianHS;
      this->insideAbove = prFace.insideAbove;
      boundingRect = getBoundingRec(leftStart);
      boundingRect.Extend(getBoundingRec(leftEnd));
      boundingRect.Extend(getBoundingRec(rightStart));
      boundingRect.Extend(getBoundingRec(rightEnd)); 
    }// set
       
    void PResultFace::createMedianHS(){
      double medianStartX = (this->leftStart.getX() + 
                             this->leftEnd.getX())/2;
      double medianStartY = (this->leftStart.getY() + 
                             this->leftEnd.getY())/2;
      double medianEndX   = (this->rightStart.getX() + 
                             this->rightEnd.getX())/2;
      double medianEndY   = (this->rightStart.getY() + 
                             this->rightEnd.getY())/2;
      medianStartX = medianStartX * medianHSZoom;
      medianStartY = medianStartY * medianHSZoom;
      medianEndX   = medianEndX * medianHSZoom;
      medianEndY   = medianEndY * medianHSZoom;                           
      Point medianStart(true,medianStartX, medianStartY);
      Point medianEnd  (true,medianEndX,medianEndY);
      medianHS = HalfSegment(true, medianStart, medianEnd);
      insideAbove = medianHS.attr.insideAbove = !(medianStart > medianEnd);
    }// createMedianHS
    
    int PResultFace::getFaceNo() const{
      return medianHS.attr.faceno;
    }// getFaceNo
    
    int PResultFace::getCycleNo() const{
      return medianHS.attr.cycleno;
    }// getCycleNo
    
    int PResultFace::getSegmentNo() const{
      return medianHS.attr.edgeno;
    }// getSegmentNo
    
    bool PResultFace::getInsideAbove() const{
      return insideAbove;
    }// getInsideAbove
            
    HalfSegment PResultFace::getMedianHS() const{
      return medianHS;
    }// getMedianHS        
        
    Point3D PResultFace::getLeftStart() const{
      return leftStart;
    }// getLeftStart
    
    Point3D PResultFace::getLeftEnd() const{
      return leftEnd;
    }// getLeftEnd
    
    Point3D PResultFace::getRightStart() const{
      return rightStart;
    }// getRightStart
    
    Point3D PResultFace::getRightEnd() const{
      return rightEnd;
    }// getRightEnd
    
    Rectangle<2> PResultFace::getBoundingRec(const Point3D& point)const{
      double array[2] = {point.getX(),point.getY()};
      return Rectangle<2>(true,array,array);
    }// getBoundingBox 
    
    Rectangle<2> PResultFace::getBoundingRec()const{
      return boundingRect;
    }// getBoundingBox 
    
    MSegmentData PResultFace::getMSegmentData() const{
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
    
    bool PResultFace::isLeftDomPoint() const{
      return medianHS.IsLeftDomPoint();
    }// isLeftDomPoint
      
    void PResultFace::setSegmentNo(int sn){
      medianHS.attr.edgeno = sn;
    }// setSegmentNo
    
    void PResultFace::setLeftDomPoint(bool ldp){
       medianHS.SetLeftDomPoint(ldp);
    }// setLeftDomPoint
    
    bool PResultFace::lessByMedianHS(const PResultFace& other) const {
      return this->medianHS < other.medianHS;
    }// lessByMedianHS
        
    bool PResultFace::logicLess(const PResultFace& other) const {
      if (isLeftDomPoint() != other.isLeftDomPoint())
        return isLeftDomPoint() > other.isLeftDomPoint();
      return this->medianHS.LogicCompare(other.medianHS) == -1;
    }// logicLess       
    
    void PResultFace::copyIndicesFrom(const HalfSegment* hs) {
      medianHS.attr.faceno  = hs->attr.faceno;
      medianHS.attr.cycleno = hs->attr.cycleno;
      medianHS.attr.edgeno  = hs->attr.edgeno;
    }// copyIndicesFrom
    
    std::ostream& PResultFace::print(std::ostream& os, std::string prefix)const{
      os << "PResultFace(" << endl;
      os << prefix << "  Left:=    " << Segment3D(leftStart, leftEnd) << endl;
      os << prefix << "  Right:=   " << Segment3D(rightStart, rightEnd) << endl;
      os << prefix << "  MedianHS:=" << this->medianHS << endl;
      os << prefix <<")" << endl;
      return os;
    }// print

    std::ostream& operator <<(std::ostream& os, const PResultFace& prFace){
      prFace.print(os,"");
      return os;
    }// Operator <<
   
    bool PResultFace::operator ==(const PResultFace& prFace)const{
      if((this->leftStart   == prFace.leftStart) &&
         (this->leftEnd     == prFace.leftEnd) &&
         (this->rightStart  == prFace.rightStart) &&
         (this->rightEnd    == prFace.rightEnd) &&
         (this->medianHS    == prFace.medianHS)&&
         (this->insideAbove == prFace.insideAbove)) return true;
      return false;
    }// Operator ==
    
        
    bool PResultFace::checkBorder( double e)const{
       return(!(leftStart.nearlyEqual(rightStart,e) &&
                leftEnd.nearlyEqual(rightEnd,e)));
    }// checkBorder
    
    void PResultFace::merge(const PResultFace& other){
      if(leftStart == other.rightStart && leftEnd == other.rightEnd){
        leftStart = other.leftStart;
        leftEnd   = other.leftEnd;
      }// if  
      else if (rightStart == other.leftStart && rightEnd == other.leftEnd){  
        rightStart = other.rightStart;
        rightEnd = other.rightEnd;
      }// else if
      else {
        cerr << *this;
        cerr << other;
        NUM_FAIL ("PResultFace are not neighbors");
      }// else 
      createMedianHS();
    }// merge

    PResultFace& PResultFace::operator =(const PResultFace& prFace){
      set(prFace);
      return *this;
    }// Operator = 
/*
15 class CriticalPResultFace 
 
*/        
    CriticalPResultFace::CriticalPResultFace():
        source(UNIT_A),predicate(UNDEFINED){      
    }// Konstruktor
    
    CriticalPResultFace::CriticalPResultFace(
        const CriticalPResultFace& cmsegment){
      set(cmsegment);
    }// Konstruktor
   
    CriticalPResultFace::CriticalPResultFace(const Segment3D& left, 
                                       const Segment3D& right, 
                                       SourceFlag source,
                                       Predicate predicate){
      set(left, right, source,predicate);      
    }// Konstruktor
    
    void CriticalPResultFace::set(const CriticalPResultFace& cprFace){
      this->left        = cprFace.left;
      this->right       = cprFace.right;
      this->source      = cprFace.source; 
      this->midPoint    = cprFace.midPoint;
      this->normalVector= cprFace.normalVector;
      this->predicate   = cprFace.predicate;
    }// set
    
    void CriticalPResultFace::set(const Segment3D& left, const Segment3D& right,
                                  SourceFlag source, Predicate predicate){ 
      this->left = left;
      this->right = right;      
      RationalPoint3D initialStart = left.getTail().getR();
      RationalPoint3D initialEnd   = right.getTail().getR();
      RationalPoint3D finalStart   = left.getHead().getR();
      RationalPoint3D finalEnd     = right.getHead().getR();      
      // We compute the normalvector
      // We compute the normalvector
      if(initialEnd.distance(initialStart) > finalEnd.distance(finalStart)){
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
    
    Point3D CriticalPResultFace::getMidPoint()const{
      return midPoint;
    }// getMidPoint 
    
    Segment3D CriticalPResultFace::getLeft() const{
      return left;
    }// getLeft
      
    Segment3D CriticalPResultFace::getRight() const{
      return right;
    }// getRight
    
    std::ostream& CriticalPResultFace::print(std::ostream& os, 
                                          std::string prefix)const{
      os << "CriticalPResultFace(" << endl;
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
                              const CriticalPResultFace& cprFace){
      cprFace.print(os,"");
      return os;
    }// Operator <<
   
    bool CriticalPResultFace::operator ==(
        const CriticalPResultFace& cprFace)const{
      if((this->left  == cprFace.left) &&
         (this->right == cprFace.right) &&       
         (this->source == cprFace.source)&&
         (this->predicate == cprFace.predicate)) return true;
      return false;
    }// Operator ==
    
    CriticalPResultFace& CriticalPResultFace::operator =(
        const CriticalPResultFace& cprFace){
      set(cprFace);
      return *this;
    }// Operator =            
    
    bool CriticalPResultFace::isPartOfUnitA() const{
      return source == UNIT_A;
    }// isPartOfUnitA
      
    bool CriticalPResultFace::hasEqualNormalVector(
        const CriticalPResultFace& other) const{  
      // Precondition: this is parallel to other.
      // The normal vectors are equal, iff
      // the cosinus of the angle between them is positive:
      return NumericUtil::greater(normalVector * other.normalVector, 0.0);
    }// hasEqualNormalVectors
      
    bool CriticalPResultFace::operator <(
        const CriticalPResultFace& cprFace)const{
      return this->midPoint < cprFace.midPoint;      
    }// Operator <
    
    Predicate CriticalPResultFace::getPredicate() const {
      return this->predicate;      
    }// getPredicate
    
    PResultFace CriticalPResultFace::getPResultFace()const{
      return PResultFace(left,right);
    }// getPResultFace
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
      this->prFaces  = std::vector<PResultFace>();
      this->mCSegments = std::vector<CriticalPResultFace>(); 
      this->orginalStartTime = other.orginalStartTime;
      this->orginalEndTime   = other.orginalEndTime;   
      for (size_t i = 0; i < other.prFaces.size(); i++) {
        this->prFaces.push_back(other.prFaces[i]);
      }// for
      for (size_t i = 0; i < other.mCSegments.size(); i++) {
        this->mCSegments.push_back(other.mCSegments[i]);
      }// for
    }// set
    
    size_t ResultUnit::size(){
      return prFaces.size();
    }// size(
    
    void ResultUnit::addPResultFace(PResultFace& prFace, bool completely ){
      if (prFace.getLeftStart() == prFace.getRightStart() &&   
          prFace.getLeftEnd()   == prFace.getRightEnd()){
        cerr << prFace << endl;
        NUM_FAIL("Error");
      }// if
      if (completely) {
        prFaces.push_back(prFace);
      }// if
      else {
        prFace.setLeftDomPoint(true);
        prFaces.push_back(prFace);
      }// else
    }// addPResultFace
    
    void ResultUnit::addPResultFace(const CriticalPResultFace& mCSegment){
      PResultFace prFace = mCSegment.getPResultFace();
      addPResultFace(prFace,false);    
    }// addPResultFace
    
    void ResultUnit::addCPResultFace(const CriticalPResultFace& prFace){
      mCSegments.push_back(prFace);
    }// addPResultFace
    
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
      os << prefix +  "  PResultFaces (";
      if (this->prFaces.size() == 0) os << "is empty)" << endl;
      else {
        os << endl;
        for (size_t i = 0; i < this->prFaces.size(); i++) {
          os << prefix + "    Index:=" << i << ", "; 
          this->prFaces[i].print(os,prefix + "    ");
        }// for
        os << prefix + "  )" << endl;
      }// else
      os << prefix + "  CriticalPResultFace (";  
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
      if (this->prFaces.size() != other.prFaces.size()) {
        cerr << "Size of prFaces is differnt" << endl; 
        return false;  
      }// if
      for (size_t i = 0; i < this->prFaces.size(); i++) {
        if (!(this->prFaces[i] == other.prFaces[i])) {
          cerr << "ResultUnit is not equal on Index for PResultFace:=";
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
    
    bool ResultUnit::less(const PResultFace& prf1, const PResultFace& prf2) {
      return prf1.lessByMedianHS(prf2);
    }// less

    bool ResultUnit::logicLess(const PResultFace& prf1, 
                               const PResultFace& prf2) {
      return prf1.logicLess(prf2);
    }// logicLess
    
    void ResultUnit::finalize(){
      if(prFaces.size() == 0) return;
      do { 
        size_t size = prFaces.size();
        for(size_t i = 0; i < size; i++){
          PResultFace rFace = prFaces[i];
          rFace.setSegmentNo(i);
          prFaces[i] = rFace; 
          rFace.setLeftDomPoint(false);
          prFaces.push_back(rFace);
        }// for
        // First, we sort the prFaces of this unit by their 
        // median-halfsegments. Comparison between halfsegments
        // is done by the < operator, implemented in the SpatialAlgebra.
        sort(prFaces.begin(), prFaces.end(), ResultUnit::less);
        // Second, we construct a region from all median-halfsegments
        // of each msegment of this unit:
        Region region(prFaces.size());
        region.StartBulkLoad();
        for (size_t i = 0; i < prFaces.size(); i++) {
          // cout << prFaces[i].getMedianHS() << endl;
          region.Put(i, prFaces[i].getMedianHS());
        }// for
        // Note: Sorting is already done.
        region.EndBulkLoad(false, true, true, true);
        // Third, we retrive the faceNo, cycleNo and edgeNo of
        // each halfsegment from the region, computed in the 
        // Region::EndBulkLoad procedure:
        for (unsigned int i = 0; i < prFaces.size(); i++) {
          HalfSegment halfSegment;
          region.Get(i, halfSegment);
          prFaces[i].copyIndicesFrom(&halfSegment);
        }// for
        // Sort prFaces by faceno, cycleno and segmentno
        sort(prFaces.begin(), prFaces.end(), ResultUnit::logicLess);
        //  this->Print();
        // Erase the second half of prFaces, 
        // which contains all PResultFaces with right dominating point
        prFaces.erase(prFaces.begin() + prFaces.size() / 2, 
                        prFaces.end());
      }while(merge(NumericUtil::eps*NumericUtil::epsRelaxFactor));
    }// finalize

    URegionEmb* ResultUnit::convertToURegionEmb(
        DbArray<MSegmentData>* segments) const {
      size_t segmentsStartPos = segments->Size();
      URegionEmb* uregion     = new URegionEmb(getTimeInterval(), 
                                               segmentsStartPos);
      Rectangle<2> resultBRec;      
      if(prFaces.size() > 0){      
        resultBRec = prFaces[0].getBoundingRec();
      }// if      
      for(unsigned int i = 0; i < prFaces.size(); i++) {   
         MSegmentData msd  = prFaces[i].getMSegmentData();   
         uregion->PutSegment(segments, i, msd, true);
         Rectangle<2> bRec = prFaces[i].getBoundingRec();
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
    
    void ResultUnit::copyCriticalMSegmens(const CriticalPResultFace& cmSeg, 
                                          SetOp setOp){
      if (setOp == UNION && cmSeg.getPredicate() == OUTER) {
        addPResultFace(cmSeg); 
        return;
      }// if
      if (setOp == INTERSECTION && cmSeg.getPredicate() == INNER) {
        addPResultFace(cmSeg);  
        return; 
      }// if
      if ( setOp == MINUS && 
          ((cmSeg.isPartOfUnitA() && cmSeg.getPredicate() == OUTER)||
          (!cmSeg.isPartOfUnitA() && cmSeg.getPredicate() == INNER))) {
        addPResultFace(cmSeg);  
        return;
      }// if
    }// CopyCriticalMSegmens
    
    void ResultUnit::evaluateCriticalMSegmens(SetOp setOp){
      // Sort by midpoints:
      sort(mCSegments.begin(), mCSegments.end());
      if (mCSegments.size() > 0) {
        size_t i;
        for (i = 0; i < mCSegments.size()-1; i++) {
          CriticalPResultFace cmSeg0 = mCSegments[i];
          CriticalPResultFace cmSeg1 = mCSegments[i+1];
          if(cmSeg0.getMidPoint() == cmSeg1.getMidPoint()) {
            if (cmSeg0.hasEqualNormalVector(cmSeg1)) {
              addPResultFace(cmSeg0);
            }// if
            i++;
          }// if  
          else {
            copyCriticalMSegmens(cmSeg0,setOp);
          }// else          
        }// for
        if (i == mCSegments.size()-1) {
          CriticalPResultFace cmSeg = mCSegments[i];
          copyCriticalMSegmens(cmSeg,setOp);
        }// if
      }// if
      mCSegments.clear();      
    }// evaluateCriticalMSegmens  
    
    bool ResultUnit::merge(double e){
      bool merge = false;
      for(size_t i = prFaces.size()-2; i != 0; i--){
        if(prFaces.size() < 3){
          NUM_FAIL ("Upps");
        }// if 
        
        PResultFace prFace0 = prFaces[i];
        PResultFace prFace1 = prFaces[i+1];
        if(!prFace1.checkBorder(e)){ 
          prFace0.merge(prFace1);
          prFaces[i]   = prFace0;
          prFaces.erase(prFaces.begin()+i+1);
          merge = true;
        }// if
      }// for 
      return merge;
    }// check
/*
17 class Layer

*/  
    Layer::Layer(bool isCritical /*= false*/){
      this->isCritical = isCritical;
      this->touchBelow = 0;
      this->touchAbove = 0;
    }// Construktor
    
    Layer::Layer(const Layer& other){
      set(other);
    }// Construktor        
    
    void Layer::addOrthSegment(const Segment& segment){
      size_t index = this->segments.add(segment);
      Predicate predicate = segment.getPredicate();
      if (this->nonOrthSegments.size() > 0 && 
          (predicate == RIGHT_IS_INNER || predicate == LEFT_IS_INNER)){
        // the first segment is the left boundary
        Segment leftBoder = this->segments.get(this->nonOrthSegments[0]); 
        if(leftBoder.getHead() == segment.getTail()) this->touchAbove++;
        if(leftBoder.getTail() == segment.getTail()) this->touchBelow++;
      }// if
      this->orthSegments.push_back(index);
    }// addOrthSegment
    
    void Layer::addNonOrthSegment(const Segment& segment){
      size_t index = this->segments.add(segment); 
      Predicate predicate = segment.getPredicate();
      if (this->nonOrthSegments.size() > 0 && 
          (predicate == RIGHT_IS_INNER || predicate == LEFT_IS_INNER)){
        // the first segment is the left boundary
        Segment leftBoder = this->segments.get(this->nonOrthSegments[0]);  
        if(leftBoder.getHead() == segment.getHead()) this->touchAbove++;
        if(leftBoder.getTail() == segment.getTail()) this->touchBelow++;
      }// if
      this->nonOrthSegments.push_back(index);
    }// addNonOrthSegment
    
    void Layer::print(std::ostream& os, const vector<size_t> values)const{
      for( size_t i = 0; i < values.size(); i++){
        os << values[i];
        if(i != values.size() -1) os << ", ";
      }// for
    }// print
    
    std::ostream& Layer::print(std::ostream& os,std::string prefix)const{
      os << "Layer(";
      if (this->segments.size() == 0) os << "is empty)" << endl;
      else {
        os << endl;
        if(this->isCritical){
          os << prefix + "  Source PFace is critical" << endl;
        }// if
        else os << prefix + "  Source PFace is not critical" <<endl;        
        os << prefix + "  ";
        segments.print(os, prefix+"  ");         
        os << prefix + "  Non orthogonal segments("; 
        print(os, this->nonOrthSegments);
        os << ")" << endl;
        os << prefix + "  Orthogonal segments(";
        print(os, this->orthSegments);
        os << ")" << endl;
        os << prefix + "  Touch on the left border below := ";
        os << this->touchBelow << endl;
        os << prefix + "  Touch on the left border above := ";
        os << this->touchAbove << endl;
        os << prefix << ")" << endl;
      }// else
      return os;       
    }// print
    
    std::ostream& operator <<(std::ostream& os, const Layer& layer){
       return layer.print(os,"");      
    }// Operator  <<
        
    Predicate Layer::getBorderPredicate(const Segment& segment, 
                                        Border border)const{
      Predicate predicate = segment.getPredicate();
      if (predicate == LEFT_IS_INNER){
        if (border ==  LEFT) return OUTER;
        else return INNER;                
      }// if
      else if (predicate == RIGHT_IS_INNER){
        if (border ==  LEFT) return INNER;
        else return OUTER;
      }// else if
      else if (predicate == INNER || predicate == OUTER) return predicate;
      else return UNDEFINED;
    }// getBorderPredicate
    
    Predicate Layer::getAreaPredicate(size_t left, 
                                      size_t right,
                                      size_t orthogonal, 
                                      bool orthSegmentExist)const{
      Segment leftSegment  = this->segments.get(left);
      Segment rightSegment = this->segments.get(right); 
      Segment orthSegment  = this->segments.get(orthogonal);
      // cout << "Left:= " << leftSegment << endl;
      // cout << "Right:= " << rightSegment << endl;
      // if( orthSegmentExist) {
      //  cout << "Orthogonal:= " << orthSegment << endl;
      // }// if      
      Predicate result = UNDEFINED;
      Predicate predicateLeft  = getBorderPredicate(leftSegment, LEFT);
      Predicate predicateRight = getBorderPredicate(rightSegment, RIGHT);
      // cout << "Left Predicate:=" << toString(predicateLeft) << endl;
      // cout << "Right Predicate:=" << toString(predicateRight) << endl;      
      if (predicateLeft != UNDEFINED) {
        if (predicateRight != UNDEFINED) {
          if (predicateLeft == predicateRight) {
            result = predicateLeft;
          }// if
          // possibly no predicate can be determined for critical P-Faces
          else if(this->isCritical) {
            result = INTERSECT;
          }// if
          else {
            cerr << *this;
            NUM_FAIL("Different predicates at the edges of a area");
          }// else
        }// else
        // right boundary is undefined
        else result = predicateLeft;
      }// if
      else {
        // right boundary is undefined
        result = predicateRight;
      }// else
      // if necessary, the orthogonal segments must be evaluated
      if (result == UNDEFINED && orthSegmentExist && !this->isCritical){
        result = getBorderPredicate(orthSegment, RIGHT);
      }// if
      // cout << "Predicate:=" << toString(result) << endl;
      // cout << endl;
      return result;
    }// getAreaPredicate  
    
    void Layer::getBorderPredicates(Predicate& left, Predicate& right)const{
      size_t first  = this->nonOrthSegments[0];
      size_t last = this->nonOrthSegments[this->nonOrthSegments.size()-1];      
      left  = getBorderPredicate(this->segments.get(first),LEFT);
      right = getBorderPredicate(this->segments.get(last),RIGHT);
    }// getBorderPredicate
      
    bool Layer::evaluate(){
      // orthogonal segments
      size_t j = 0;
      size_t orthogonal = 0;
      bool orthSegmentExist = false;
      // left border      
      size_t left = this->nonOrthSegments[0];
      size_t right;
      // over all non-orthogonal segments
      for(size_t i = 1; i < this->nonOrthSegments.size(); i++ ){
        // right border
        right =  this->nonOrthSegments[i];
        // do orthogonal segments exist? 
        if( j < orthSegments.size()){
          orthogonal = this->orthSegments[j];
          // does the orthogonal segment touch the left border?
          if (this->segments.get(left).getTail() == 
              this->segments.get(orthogonal).getTail()) {
            orthSegmentExist = true;
            j++;
          }// if
          else orthSegmentExist = false;
        }// if        
        Predicate predicate = getAreaPredicate(left, right,
          orthogonal,orthSegmentExist);
        segments.set(left, predicate);
        segments.set(right, predicate);
        left = right;
      }// for
      // is the predicate of the right segment not undefined but the predicate 
      // of the left segment is set to undefined?
      Predicate leftPredicate, rightPredicate;
      getBorderPredicates(leftPredicate, rightPredicate);
      if(leftPredicate == UNDEFINED && rightPredicate != UNDEFINED){    
        // right border
        right = this->nonOrthSegments[this->nonOrthSegments.size()-1];
        orthogonal = 0;
        for(int i = this->nonOrthSegments.size()-2; i >= 0; i--){
          // left border
          size_t left =  this->nonOrthSegments[i];
          Predicate predicate = getAreaPredicate(left, right,
            orthogonal, orthSegmentExist);
          segments.set(left, predicate);
          segments.set(right, predicate);
          right = left;
        }// for
      }// if
      size_t first   = this->nonOrthSegments[0];
      size_t last    = this->nonOrthSegments[this->nonOrthSegments.size()-1];
      leftPredicate  = this->segments.get(first).getPredicate();
      rightPredicate = this->segments.get(last).getPredicate();
      return (leftPredicate != UNDEFINED && rightPredicate != UNDEFINED);
    }// evaluate
    
    bool Layer::operator ==(const Layer& layer)const{
      if(!(this->segments == layer.segments)) return false;
      if(this->orthSegments.size() != layer.orthSegments.size()) 
        return false;
      for(size_t i = 0; i < this->orthSegments.size(); i++){
        if(this->orthSegments[i] != layer.orthSegments[i]) return false;
      }// for
      if(this->nonOrthSegments.size() != layer.nonOrthSegments.size()) 
        return false;
      for(size_t i = 0; i < this->nonOrthSegments.size(); i++){
        if(this->nonOrthSegments[i] != layer.nonOrthSegments[i]) return false;
      }// for
      if(this->touchAbove != layer.touchAbove) return false;
      if(this->touchBelow != layer.touchBelow) return false;
      if(this->isCritical != layer.isCritical) return false;
      return true;
    }// Operator ==
    
    void Layer::set(const Layer& layer){
      this->segments        = layer.segments;
      this->orthSegments    = layer.orthSegments;
      this->nonOrthSegments = layer.nonOrthSegments;
      this->touchAbove      = layer.touchAbove;
      this->touchBelow      = layer.touchBelow;
      this->isCritical      = layer.isCritical;       
    }// set
    
    Layer& Layer::operator =(const Layer& layer){
      set(layer);
      return *this;
    }// Operator =
    
    Predicate Layer::getPredicateForSuccessor()const{
      if (this->nonOrthSegments.size() < 2) return UNDEFINED;
      size_t first  = this->nonOrthSegments[0];
      size_t second = this->nonOrthSegments[1];            
      Predicate predicate = getAreaPredicate(first, second, first, false);
      if (predicate != UNDEFINED){
        if (touchAbove%2 == 1){
          if (predicate == INNER) return OUTER;
          else return INNER;
        }// if
      }// if
      return predicate;
    }// getSuccessorPredicate
    
    Predicate Layer::getPredicateForPredecessor()const{
     if (this->nonOrthSegments.size() < 2) return UNDEFINED;
      size_t first  = this->nonOrthSegments[0];
      size_t second = this->nonOrthSegments[1];  
      Predicate predicate = getAreaPredicate(first, second, first, false);
      if (predicate != UNDEFINED){
        if (touchBelow%2 == 1){
          if (predicate == INNER) return OUTER;
          else return INNER;
        }// if
      }// if
      return predicate; 
    }// getPredecessorPredicate
    
    void Layer::setPredicateFromSuccessor(Predicate predicate){
      if (predicate == UNDEFINED || this->nonOrthSegments.size() < 2) return;
      if (touchAbove%2 == 1){
        if (predicate == INNER) predicate = OUTER;
        else predicate = INNER;
      }// if
      size_t first   = this->nonOrthSegments[0];
      size_t second = this->nonOrthSegments[1];
      segments.set(first, predicate);
      segments.set(second, predicate);
    }// setPredicateFromPredecessor
    
    void Layer::setPredicateFromPredecessor(Predicate predicate){
      if (predicate == UNDEFINED || this->nonOrthSegments.size() < 2) return;
      if (touchBelow%2 == 1){
        if (predicate == INNER) predicate = OUTER;
        else predicate = INNER;
      }// if
      size_t first  = this->nonOrthSegments[0];
      size_t second = this->nonOrthSegments[1];
      segments.set(first, predicate);
      segments.set(second, predicate);
    }// setPredicateFromPredecessor  
    
    bool Layer::intersects(bool& predicate)const{
      size_t size = this->nonOrthSegments.size();
      if(size < 2) {
        NUM_FAIL ("there must be at least two segments."); 
      }// if      
      // relevantes P-Face with more than two segments
      if (/*!this->isCritical && */ size > 2){
        predicate = true;
        return true;
      }// if
      // relevantes P-Face with exactly two segments
      if (/*!this->isCritical && */size == 2){
        size_t first     = this->nonOrthSegments[0];
        size_t second    = this->nonOrthSegments[1];
        Predicate result = getAreaPredicate(first, second, first, false);
        // cout << "Predicate:=" << toString(result) << endl;
        if ( segments.get(first).getPredicate()  == INTERSECT ||
             segments.get(second).getPredicate() == INTERSECT){
          predicate = true;
          return true;  
        }// if 
        else if (result == INNER) {
          predicate = true;
          return true;
        }// if
        else if (result == OUTER) {
          predicate = false;
          return true;
        }// else if
      }// if     
      predicate = false;
      return false;
    }// intersects
    
    bool Layer::inside(bool& predicate)const{
      size_t size = this->nonOrthSegments.size();
      if(size < 2) {
        NUM_FAIL ("there must be at least two segments."); 
      }// if      
      // relevantes P-Face with more than two segments
      if (!this->isCritical && size > 2){
        predicate = false;
        return true;
      }// if
      // relevantes P-Face with exactly than two segments
      if (!isCritical && size == 2){
        size_t first     = this->nonOrthSegments[0];
        size_t second    = this->nonOrthSegments[1]; 
        Predicate result = getAreaPredicate(first, second, first, false);
        // cout << "Predicate:=" << toString(result) << endl;
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
        //critical P-Face
        Predicate oldPredicate = UNDEFINED;
        for (size_t i = 0; i < size-1; i++){
          Predicate newPredicate = getAreaPredicate(this->nonOrthSegments[i],
            this->nonOrthSegments[i+1],this->nonOrthSegments[i], false);
          // cout << "Old Predicate:=" << toString(oldPredicate) << endl;
          // cout << "New Predicate:=" << toString(newPredicate) << endl;
          // cout << "Left segment.=";
          // cout << this->segments.get(this->nonOrthSegments[i]) << endl;
          // cout << "Right segments:=";
          // cout << this->segments.get(this->nonOrthSegments[i+1]) << endl;
          if (newPredicate == OUTER) {
            predicate = false;
            return true;
          }// if
          if(newPredicate != UNDEFINED) oldPredicate = newPredicate;
        }// for
        if(oldPredicate == INNER){
            predicate = true;
            return true;
        }// if          
      }// else
      predicate = false;
      return false;
    }// inside  
    
    void Layer::getResultUnit(Predicate soughtPredicate, bool reverse, 
        const Point3DContainer& points, ResultUnit& unit, 
        SourceFlag source)const { 
      for(size_t i = 0; i < this->nonOrthSegments.size()-1; i++){
        size_t left  = this->nonOrthSegments[i];
        size_t right = this->nonOrthSegments[i+1];
        Segment leftSegment  = this->segments.get(left);
        Segment rightSegment = this->segments.get(right);
        Predicate predicate  = getAreaPredicate(left, right,
          left, false);
        if (predicate == soughtPredicate || this->isCritical){
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
          if (this->isCritical) {
            CriticalPResultFace segment(left,right,source,predicate);
            unit.addCPResultFace(segment);
          }// if
          else {
            PResultFace segment(left, right); 
            unit.addPResultFace(segment,false);
          }// else 
        }// if
      }// for
    }// getResultUnit  
/*
18 class LayerContainer

*/
    LayerContainer::LayerContainer(size_t size /* = 0 */, 
                                   bool isCritcal /* = false*/){
       this->layers = std::vector<Layer>(size,Layer(isCritcal));
    }// Constructor
    
    LayerContainer::LayerContainer(const LayerContainer& other){
      set(other);
    }// Constructor
    
    LayerContainer::LayerContainer(
        Point3DContainer& points,
        GlobalTimeValues &timeValues,
        PlaneSweepAccess &access,
        bool pFaceIsCritical){ 
      size_t size = timeValues.size()-1;
      this->layers = vector<Layer>(size,Layer(pFaceIsCritical));
      mpq_class t1,t2;
      SegmentContainer segments; 
      if (timeValues.scaledFirst(t1, t2) ) { 
        access.first(t1, t2, points, segments, pFaceIsCritical);
        // over all time periods
        for (size_t i = 0; i < size; i++) { 
          // over all segments       
          for (size_t j = 0; j < segments.size(); j++) {
            Point3D head, tail;
            Segment segment = segments.get(j);
            head = points.get(segment.getHead());
            tail = points.get(segment.getTail());
            if (NumericUtil::nearlyEqual(head.getZ(),tail.getZ())) {
              addOrthSegment(i, segment);
            }// if
            else {
              addNonOrthSegment(i, segment);
            }// else
          }// for
          //  cout << "i:=" << i << endl;
          //  cout << "t1:=" << t1 << ",t2:="<< t2 << endl;
          //  cout << *this;
          //  cout << timeValues;
          t1 = t2;
          timeValues.scaledNext(t1, t2);
          segments.clear();
          access.next(t1, t2, points, segments, pFaceIsCritical);
        }// for
      }// if
    }// Konstruktor
    
    void LayerContainer::set(const LayerContainer& other){
       this->layers = other.layers; 
    }// set
    
    void LayerContainer::addOrthSegment(size_t layer, 
                                        const Segment& segment){
     if (layer < this->layers.size()) {
        this->layers[layer].addOrthSegment(segment);
      }// if
      else NUM_FAIL("Index of layer is out of range.");      
    }// addOrthSegment
    
    void LayerContainer::addNonOrthSegment(size_t layer, 
                                           const Segment& segment){
      if (layer < this->layers.size()) {
        this->layers[layer].addNonOrthSegment(segment);
      }// if
      else NUM_FAIL("Index of layer is out of range.");      
    }// addNonOrthSegment
    
    std::ostream& LayerContainer::print(std::ostream& os,
                                        std::string prefix)const{
      os <<  "LayerContainer(" << endl;
      for(size_t i = 0; i < this->layers.size(); i++){
        os << prefix + "  Index:=" << i << ", ";
        layers[i].print(os,prefix + "  ");       
      }// for
      os << prefix + ")" << endl;
      return os;
    }// print 
    
    std::ostream& operator<<( std::ostream& os,
                              const LayerContainer& container){
      return container.print(os,"");
    }// Operator <<
    
    bool LayerContainer::evaluate(){
      bool status = true;
      vector<bool> evaluated(layers.size(),false);
      // Pass all layers forward
      Predicate predicate = UNDEFINED;      
      for (size_t i = 0; i < layers.size(); i++){
        bool result = layers[i].evaluate();
        // Can a predicate be taken from the predecessor?
        if (!result && predicate != UNDEFINED){
          layers[i].setPredicateFromPredecessor(predicate);
          result = layers[i].evaluate();
          if(!result){
            cerr << *this;
            NUM_FAIL("The processing of the intersections was not successful.");
          }// if
        }// if
        evaluated[i] = result;
        predicate = layers[i].getPredicateForSuccessor();
      }// for 
      // Pass all layers backward
      predicate = UNDEFINED;
      for(int i = layers.size()-1; i >= 0; i--){
        // Can a predicate be taken from the successor?
        if (!evaluated[i] && predicate != UNDEFINED){
          layers[i].setPredicateFromSuccessor(predicate);
          bool result  = layers[i].evaluate();
          evaluated[i] = result;
          if(!result){
            cerr << *this;
            NUM_FAIL("The processing of the intersections was not successful.");
          }// if          
        }// if       
        predicate = this->layers[i].getPredicateForPredecessor();
        status    = status && evaluated[i];
      }// for
      return status;
    }// evaluate
    
    void LayerContainer::getBorderPredicates(Predicate& left, 
                                             Predicate& right)const{
      left  = UNDEFINED;
      right = UNDEFINED;
      if (this->layers.size() > 0){
        layers[0].getBorderPredicates(left,right);
        for( size_t i = 1; i < this->layers.size(); i ++){
          Predicate tmpLeft, tmpRight;
          layers[i].getBorderPredicates(tmpLeft,tmpRight);
          if (right != tmpRight) right = INTERSECT;
          if (left  != tmpLeft)  left  = INTERSECT;
        }// for        
      }// if
    }// getBorderPredicate
    
    bool LayerContainer::operator ==(const LayerContainer& other)const{
      if (this->layers.size() != other.layers.size()) return false;
      for (size_t i = 0 ; i < this->layers.size(); i ++){
        if(!(this->layers[i] == other.layers[i])) return false;          
      }// for
      return true;
    }// Operator ==
    
    LayerContainer& LayerContainer::operator =(const LayerContainer& other){
      set(other);
      return *this;
    }// Operator =
    
    bool LayerContainer::intersects(std::vector<bool>& predicate)const{
      for(size_t i = 0; i < this->layers.size(); i ++){
        bool value;
        bool result  = this->layers[i].intersects(value); 
        predicate[i] = value;
        if(!result)return false;
      }// for
      return true;
    }// intersects
    
    bool LayerContainer::inside(std::vector<bool>& predicate)const{
      for(size_t i = 0; i < this->layers.size(); i ++){
        bool value;
        bool result  = this->layers[i].inside(value); 
        predicate[i] = value;
        if(!result)return false;
      }// for
      return true;
    }// inside
    
    void LayerContainer::getResultUnit(size_t layer, Predicate soughtPredicate,
      bool reverse, const Point3DContainer& points, ResultUnit& unit, 
      SourceFlag source)const { 
      if(layer <  this->layers.size()){
        this->layers[layer].getResultUnit(soughtPredicate, reverse, points, 
                                          unit, source);  
      }// if
      else { 
        cerr << "Index:=" << layer <<endl;
        cerr << *this;
        NUM_FAIL("Index is out of range");
      }// if
    }// getResultUnit     
/*
19 Class PFace

*/  
    PFace::PFace(size_t left, size_t right, const Point3DContainer& points, 
            const SegmentContainer& segments){
      Segment borderLeft  = segments.get(left);
      Segment borderRight = segments.get(right);
      PResultFace::set(points.get(borderLeft.getTail()),
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
      if (!mSeg.GetPointInitial()) {
        start = Point2D(mSeg.GetInitialStartX(), mSeg.GetInitialStartY());
        end   = Point2D(mSeg.GetInitialEndX(), mSeg.GetInitialEndY());        
      }// if
      else {
        start = Point2D(mSeg.GetFinalStartX(), mSeg.GetFinalStartY());
        end   = Point2D(mSeg.GetFinalEndX(), mSeg.GetFinalEndY());
      }// else
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
           
      createMedianHS();
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
      this->layers      = pf.layers;
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
      os << setprecision(12);
      this->intSegs.print(os,"  "+prefix);
      os << prefix + "  ";
      this->layers.print(os,"  "+prefix);
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
      RationalSegment2D segment = planeSelf.transform(intSeg);
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
    
    void PFace::addBorder(const RationalPlane3D &plane,
                          Predicate predicate){
      IntersectionSegment iSeg = createBorder(plane,LEFT,predicate);
      addIntSeg(iSeg); 
      iSeg = createBorder(plane,RIGHT,predicate);
      addIntSeg(iSeg); 
    }// addBorder   
    
    void PFace::addBorder(const SegmentContainer& segments, 
                          Predicate predicate){
      Predicate leftPredicate  = segments.get(left).getPredicate();
      Predicate rightPredicate = segments.get(right).getPredicate();  
      if (state == UNKNOWN && 
          leftPredicate  != UNDEFINED && 
          rightPredicate != UNDEFINED) {
        if (leftPredicate == predicate || rightPredicate == predicate) {
          state = RELEVANT;
          RationalPlane3D plane(*this);
          addBorder(plane,predicate);   
        }// if
        else {
          state = NOT_RELEVANT;          
        }// if
      }// if
      // A P-Face with a intersection cut could not be processed 
      // in a previous round. The edges are re-added with their predicates
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
    
    void PFace::getBorderSegments(bool all, vector<RationalSegment3D>& borders){
      borders.push_back(RationalSegment3D(this->leftStart,this->leftEnd));
      borders.push_back(RationalSegment3D(this->rightStart,this->rightEnd));
      if(all) {
        borders.push_back(RationalSegment3D(this->leftStart,this->rightStart));
        borders.push_back(RationalSegment3D(this->leftEnd,this->rightEnd));
      }// if
    }// getBorderSegments
    
    bool PFace::intersection(const vector<RationalSegment2D>& borders, 
                             const RationalSegment2D& segment, 
                             RationalSegment2D& result){
      std::set<RationalPoint2D> points;
      std::set<RationalPoint2D>::iterator first,second;
      for (size_t i = 0; i < borders.size(); i++){
        RationalPoint2D iPoint;
        if(segment == borders[i]){
          continue;
        }// if
        bool result = segment.intersection(borders[i],iPoint);
        if(result){ 
          // cout << segment << endl;
          // cout << borders[i] << endl;
          // cout << iPoint << endl;
          points.insert(iPoint); 
        }// if
      }// for
      if(points.size() > 1 ){
        second = first = points.begin();
        second++;
        // The intersections points  were sorted in the set "points" 
        // according to the sizes of x, y. 
        // Undo this sorting
        if(first->getY() < second->getY()){
          result = RationalSegment2D(*first,*second);
        }// if
        else {
          result = RationalSegment2D(*second,*first);
        }// else
        return true;
      }// if
      return false;
    }// intersection
    
    RationalSegment3D PFace::map(const RationalSegment3D& orginal3D, 
                         const RationalSegment2D& orginal2D, 
                         const RationalSegment2D& intersection){
      mpq_class length  = (orginal2D.getHead() - 
                           orginal2D.getTail()).length();                      
      mpq_class length0 = (intersection.getTail() - 
                           orginal2D.getTail()).length(); 
      mpq_class length1 = (intersection.getHead() - 
                           orginal2D.getTail()).length(); 
      RationalVector3D vector = orginal3D.getHead() - orginal3D.getTail();
      RationalPoint3D  tail3D = orginal3D.getTail() + length0/length*vector;
      RationalPoint3D  head3D = orginal3D.getTail() + length1/length*vector;
      return RationalSegment3D(tail3D,head3D);
    }// map
    
    bool PFace::intersectionOnPlane(PFace& other,
                                    const RationalPlane3D& planeSelf,
                                    GlobalTimeValues &timeValues){
      vector<RationalSegment3D> self3D;
      vector<RationalSegment2D> self2D;
      vector<RationalSegment3D> other3D;
      vector<RationalSegment2D> other2D;
      // create segments
      other.getBorderSegments(false, other3D); 
      this->getBorderSegments(true, self3D); 
      // transform segments
      planeSelf.transform(other3D,other2D);
      planeSelf.transform(self3D,self2D);
      // calculate the intersegment segments
      for (size_t i = 0; i < other2D.size(); i++){
        RationalSegment2D segment2D;  
        bool result = intersection(self2D, other2D[i], segment2D);
        if (result) {
          RationalSegment3D segment3D = map(other3D[i], other2D[i],segment2D);
          // cout << "Segment2D:=" << segment2D << endl;
          // cout << "Ssegment3D:=" << segment3D << endl;
          if (!(segment3D == self3D[0] || 
                segment3D == self3D[1])){
            IntersectionSegment iSegment(segment3D,segment2D, NO_INTERSECT);
            // cout << "Schnittsegment:=" << iSegment << endl;
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
      bRec.Extend(NumericUtil::eps);
      // No intersection if the bounding boxes do not intersect
      if (!(this->boundingRect.Intersects(other.boundingRect))) {
        return false; 
      }// if
      // create planes
      RationalPlane3D planeSelf(*this);
      RationalPlane3D planeOther(other);                      
      // check planes
      if (planeSelf.isParallelTo(planeOther)) {
        if (planeSelf.isCoplanarTo(planeOther)) {   
          intersectionOnPlane(other,planeSelf,timeValues);
          other.intersectionOnPlane(*this,planeOther,timeValues);  
        }// if 
        return false;
      }// if
      RationalPoint3DExtSet intPointSet;    
      planeSelf.intersection(other, UNIT_A, intPointSet);  
      // We need exactly two intersection points.
      if (intPointSet.size() != 2) return false;    
      planeOther.intersection(*this, UNIT_B, intPointSet);    
      RationalSegment3D intSeg;
      // cout << intPointSet;
      if (!intPointSet.getIntersectionSegment(intSeg)) return false;  
      IntersectionSegment iSeg;
      // create and save result segments
      addIntSeg(planeSelf,planeOther,intSeg,timeValues);
      other.addIntSeg(planeOther,planeSelf,intSeg,timeValues); 
      return true;    
    }// intersection   
    
    void PFace::first(mpq_class t1, mpq_class t2, Point3DContainer& points,
                       SegmentContainer& segments, bool pFaceIsCritical){ 
      intSegs.first(t1, t2, points, segments, pFaceIsCritical);
    }// first
      
    void PFace::next(mpq_class t1, mpq_class t2, Point3DContainer& points, 
                     SegmentContainer& segments,bool  pFaceIsCritical){
      intSegs.next(t1, t2, points, segments, pFaceIsCritical); 
    }// next
    
    bool PFace::finalize(Point3DContainer& points, SegmentContainer& segments, 
                         GlobalTimeValues& timeValues){
      Predicate leftPredicate, rightPredicate;
      bool result;
      if (this->state == RELEVANT || this->state == CRITICAL) {
        bool isCritical = false;
        if(this->state == CRITICAL) isCritical = true;
        this->layers = LayerContainer(points, timeValues, *this, isCritical);
        this->layers.evaluate();
        this->layers.getBorderPredicates(leftPredicate,rightPredicate);
        if (leftPredicate == UNDEFINED && rightPredicate == UNDEFINED && 
            !isCritical) {
          result = false;
        }// if
        else {
          if (leftPredicate != UNDEFINED){
            segments.set(this->left,leftPredicate);
            result = true;
          }// if
          if (rightPredicate != UNDEFINED){
            segments.set(this->right,rightPredicate); 
            result = true;
          }// if
        }// else     
      }// if
      else if (this->state == NOT_RELEVANT) result = true;
      else {
        leftPredicate  = segments.get(this->left).getPredicate();
        rightPredicate = segments.get(this->right).getPredicate();
        if (leftPredicate == INNER || leftPredicate == OUTER) {
          if (rightPredicate == UNDEFINED) {
            segments.set(this->right,leftPredicate);
          }// if
          result = true;
        }// if        
        if (rightPredicate == INNER || rightPredicate == OUTER) {
          if (leftPredicate == UNDEFINED) {
            segments.set(this->left,rightPredicate);
          }// if
          result = true;
        }// if
        result = false;
      }// else 
      return result;
    }// finalize
    
    bool PFace::intersects(Point3DContainer& points, 
                           GlobalTimeValues& timeValues,
                           std::vector<bool>& predicate){
      if (this->state == RELEVANT || this->state == CRITICAL) {
        bool isCritical = false;
        if(this->state == CRITICAL) isCritical = true;
        this->layers = LayerContainer(points, timeValues, *this, isCritical);
        this->layers.evaluate();
        return this->layers.intersects(predicate);     
      }// if
      return false;
    }// intersects
    
    bool PFace::inside(Point3DContainer& points, 
                       GlobalTimeValues& timeValues,
                       std::vector<bool>& predicate){
      if (this->state == RELEVANT || this->state == CRITICAL) {
        bool isCritical = false;
        if(this->state == CRITICAL) isCritical = true;
        this->layers = LayerContainer(points, timeValues, *this, isCritical);
        this->layers.evaluate();
        return this->layers.inside(predicate);
      }// if
      return false;
    }// inside
    
    void PFace::getResultUnit(size_t slide, Predicate predicate,
                              bool reverse, 
                              const Point3DContainer& points, 
                              ResultUnit& unit, SourceFlag source){
      if (this->state == RELEVANT || this->state == CRITICAL) {
        this->layers.getResultUnit(slide,predicate,reverse,points,unit,source);
      }// if  
    }// getResultPFace   
/*
20 class FaceCycleInfo

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
21 class SourceUnit

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
        PResultFace msegment = static_cast<PResultFace> (*pFaces[i]);
        result.addPResultFace(msegment,false);
      }// for
    }// addToResult
    
    void SourceUnit::intersection(SourceUnit& other, 
                                  GlobalTimeValues& timeValues){
      // Over all P-Face of the source.
      for (size_t i = 0; i < this->pFaces.size(); i++) {
        PFace* pFaceA = this->pFaces[i];
        Rectangle<2> bRec = (*pFaceA).getBoundingRec();
        bRec.Extend(NumericUtil::eps);
        // Iterator over all found P-PFace
        std::unique_ptr<mmrtree::RtreeT<2, size_t>::iterator> 
          it(other.pFaceTree.find(bRec)); 
        size_t const* j;  
        while ((j = it->next()) != 0) {
          PFace* pFaceB = other.pFaces[*j];
          pFaceA->intersection(*pFaceB,timeValues); 
          // cout << *pFaceA;
          // cout << *pFaceB;          
        }// while
        if (pFaceA->existsIntSegs() || pFaceA->getState()==CRITICAL) {
          RationalPlane3D plane(*pFaceA);       
          pFaceA->addBorder(plane,UNDEFINED);
          this->itersectedPFace.push_back(i);
          touchFaceCycleEntry(pFaceA);
        }// if
      }//for 
      for (size_t j = 0; j < other.pFaces.size(); j++) {
        PFace* pFaceB =other.pFaces[j];
        if (pFaceB->existsIntSegs() || pFaceB->getState()==CRITICAL) {
          RationalPlane3D plane(*pFaceB);       
          pFaceB->addBorder(plane,UNDEFINED);
          other.itersectedPFace.push_back(j);
          other.touchFaceCycleEntry(pFaceB);
        }// if
      }// for
      // For cycles without cuts, a P-Face with reference predicates 
      // is now inserted.
      checkFaceCycleEntrys(other);
      other.checkFaceCycleEntrys(*this);      
    }// intersection
    
    void SourceUnit::intersectionFast(SourceUnit& other,
                                      GlobalTimeValues& timeValues){
       // Over all P-Face of the source.
      for (size_t i = 0; i < this->pFaces.size(); i++) {
        PFace* pFaceA = this->pFaces[i];
        Rectangle<2> bRec = (*pFaceA).getBoundingRec();
        bRec.Extend(NumericUtil::eps);
        RationalPlane3D planeSelf(*pFaceA);
        // Iterator over all found P-PFace
        std::unique_ptr<mmrtree::RtreeT<2, size_t>::iterator> 
          it(other.pFaceTree.find(bRec)); 
        size_t const* j;  
        while ((j = it->next()) != 0) {
          PFace* pFaceB = other.pFaces[*j];
          // No intersection if the bounding boxes do not intersect
          if (!(pFaceA->getBoundingRec().Intersects(
                pFaceB->getBoundingRec()))) {
            continue; 
          }// if
          RationalPlane3D planeOther(*pFaceB);                      
          // check planes
          if (planeSelf.isParallelTo(planeOther)) {
            if (planeSelf.isCoplanarTo(planeOther)) {   
              pFaceA->intersectionOnPlane(*pFaceB,planeSelf,timeValues);
              pFaceB->intersectionOnPlane(*pFaceA,planeOther,timeValues);  
            }// if 
            continue;
          }// if
          RationalPoint3DExtSet intPointSet;    
          planeSelf.intersection(*pFaceB, UNIT_A, intPointSet);  
          // We need exactly two intersection points.
          if (intPointSet.size() != 2) continue;    
          planeOther.intersection(*pFaceA, UNIT_B, intPointSet);    
          RationalSegment3D intSeg;
          // cout << intPointSet;
          if (!intPointSet.getIntersectionSegment(intSeg)) continue;  
          IntersectionSegment iSeg;
          // create and save result segments
          pFaceA->addIntSeg(planeSelf,planeOther,intSeg,timeValues);
          pFaceB->addIntSeg(planeOther,planeSelf,intSeg,timeValues); 
        }// while
        if (pFaceA->existsIntSegs() || pFaceA->getState()==CRITICAL) {     
          pFaceA->addBorder(planeSelf,UNDEFINED);
          this->itersectedPFace.push_back(i);
          touchFaceCycleEntry(pFaceA);
        }// if
      }//for 
      for (size_t j = 0; j < other.pFaces.size(); j++) {
        PFace* pFaceB =other.pFaces[j];
        if (pFaceB->existsIntSegs() || pFaceB->getState()==CRITICAL) {
          RationalPlane3D planeOther(*pFaceB);       
          pFaceB->addBorder(planeOther,UNDEFINED);
          other.itersectedPFace.push_back(j);
          other.touchFaceCycleEntry(pFaceB);
        }// if
      }// for
      // For cycles without cuts, a P-Face with reference predicates 
      // is now inserted.
      checkFaceCycleEntrys(other);
      other.checkFaceCycleEntrys(*this);  
      // cout << *this;
      // cout << other;
    }// intersectionFast
    
    bool SourceUnit::finalize(Point3DContainer& points, 
                              GlobalTimeValues& timeValues, 
                              Predicate predicate,
                               SourceUnit& other){
      vector<bool> ok = vector<bool>(pFaces.size(),false);
      // first process all P-Face with intersectionsegments
      for (size_t i = 0; i < this->itersectedPFace.size(); i++) {
        size_t index = itersectedPFace[i];
        // cout << *this;     
        if (!ok[index]) {
          ok[index] = this->pFaces[index]->finalize(
            points, this->segments,timeValues); 
        }// if
      }// for      
      // then process all P-Face without cutting segments forward
      bool finalize = true;
      for (size_t i = 0; i < this->pFaces.size(); i++) {
        if (!ok[i]) {
          this->pFaces[i]->addBorder(segments,predicate);
          bool result  = this->pFaces[i]->finalize(
            points, this->segments,timeValues); 
          if (!result) finalize = false;
          else ok[i] = result;
        }// if
      }// for  
      if (finalize) return true;
      // and backward
      finalize = true;
      for (int i = this->pFaces.size()-1; i >= 0; i--) {
        if (!ok[i]) {
          this->pFaces[i]->addBorder(segments,predicate);
          bool result = this->pFaces[i]->finalize(
            points, this->segments,timeValues);  
          if (!result) finalize = false;
          else ok[i] = result;
        }// if
      }// for  
      if (finalize) return true;
      // Determine the position of the p-Face explicitly
      finalize = true;
      for (size_t i = 0; i < this->pFaces.size(); i++) {
        if (!ok[i] && this->pFaces[i]->getState() != CRITICAL){  
          // for non critical P-faces
          if (other.isInside(this->pFaces[i])) {
            this->pFaces[i]->setBorderPredicate(segments,INNER);
          }// if
          else {
            this->pFaces[i]->setBorderPredicate(segments,OUTER);
          }// else
          this->pFaces[i]->addBorder(segments,predicate);
          bool result = this->pFaces[i]->finalize(
          points, this->segments,timeValues); 
          if (!result) finalize = false;
          else ok[i] = result;
        }// if
      }// for
      for (size_t i = 0; i < this->pFaces.size(); i++) {
        if (!ok[i] && this->pFaces[i]->getState() == CRITICAL){  
          // for critical P-faces
          this->pFaces[i]->addBorder(segments,predicate);
          bool result = this->pFaces[i]->finalize(
          points, this->segments,timeValues); 
          if (result) ok[i] = result;
        }// if
      }// for            
      if (finalize) return true;
      cerr << " Finalized Pfaces:= ";
      for (size_t i = 0; i< ok.size(); i++) {
        cerr << ok[i];
      }// for
      cerr << endl;
      cerr << *this;      
      NUM_FAIL("Finalize for Source Unit is not possible.");      
      return false;
    }// finalize
    
    void SourceUnit::intersects(Point3DContainer& points, 
                                GlobalTimeValues& timeValues,
                                SourceUnit& other,
                                std::vector<bool>& predicates){
      vector<bool> ok = vector<bool>(pFaces.size(),false);
      std::vector<bool> resultPredicates(timeValues.size()-1);
      predicates = std::vector<bool>(timeValues.size()-1,false);
      bool result = false;
      for (size_t i = 0; i < this->itersectedPFace.size(); i++) {
        size_t index = this->itersectedPFace[i];
        if(this->pFaces[index]->intersects(points, timeValues, 
                                           resultPredicates)){
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
        ok[i] = true;  
      }// for  
      if (result) return;      
      for (size_t i = 0; i < this->pFaces.size(); i++) {
        if (!ok[i] && this->pFaces[i]->getState() != CRITICAL){             
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
      predicates = std::vector<bool>(timeValues.size()-1,true);
    }// intersects
    
    void SourceUnit::inside(Point3DContainer& points, 
                            GlobalTimeValues& timeValues,
                            SourceUnit& other,
                            std::vector<bool>& predicates){
      vector<bool> ok = vector<bool>(pFaces.size(),false);
      std::vector<bool> resultPredicates(timeValues.size()-1);
      predicates = std::vector<bool>(timeValues.size()-1,true);
      bool result = false;
      for (size_t i = 0; i < this->itersectedPFace.size(); i++) {
        size_t index = itersectedPFace[i];
        if(this->pFaces[index]->inside(points, timeValues, 
                                       resultPredicates)){
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
        ok[i] = true;  
      }// for
      if (result) return;
      for (size_t i = 0; i < this->pFaces.size(); i++) {
        if (!ok[i] && this->pFaces[i]->getState() != CRITICAL){             
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
      return pf1->lessByMedianHS(static_cast<PResultFace>(*pf2));
    }// return
    
    bool SourceUnit::logicLess(const PFace* pf1, const PFace *pf2){
      return pf1->logicLess(static_cast<PResultFace>(*pf2));
    }// return
    
    void SourceUnit::reSort(){
      size_t size = pFaces.size();
      for (size_t i = 0; i < size; i++) {
        PFace* pf1 = pFaces[i];
        Rectangle<2> boundigRec = pf1->getBoundingRec();
        pFaceTree.erase(boundigRec,i);   
        pf1->setLeftDomPoint(true);
        pf1->setSegmentNo(i);
        pFaces[i] = pf1;
        PFace* pf2 = new PFace(*pf1);   
        pf2->setLeftDomPoint(false);
        pFaces.push_back(pf2);  
      }// for      
      sort(pFaces.begin(),pFaces.end(),SourceUnit::lessByMedianHS);
      this->testRegion.StartBulkLoad();  
      for (size_t i = 0; i < pFaces.size(); i++) {
        this->testRegion += pFaces[i]->getMedianHS();
      }// for
      // Note: Sorting is already done.
      this->testRegion.EndBulkLoad(false, true, true, true);
      this->testRegionDefined = true;    
      for (size_t i = 0; i < pFaces.size(); i++) {
        HalfSegment halfSegment;
        this->testRegion.Get(i, halfSegment);
        pFaces[i]->copyIndicesFrom(&halfSegment);
      }// for
      // Sort pFaces by faceno, cycleno and segmentno:
      sort(pFaces.begin(), pFaces.end(), SourceUnit::logicLess);
      // Erase the second half of prFaces, 
      // which contains all PResultFaces with right dominating point:
      for (size_t i = pFaces.size()/2; i < pFaces.size(); i++) {
        delete pFaces[i];
      }// for
      pFaces.erase(pFaces.begin() + pFaces.size()/2,  pFaces.end());
      // Rebuild R-Tree
      for (size_t i = 0; i < pFaces.size(); i++) {
        PFace* pf1 = pFaces[i];
        Rectangle<2> boundigRec = pf1->getBoundingRec();
        pFaceTree.insert(boundigRec,i);
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
22 Class SourceUnitPair      

*/     
    SourceUnitPair::SourceUnitPair(double orginalStartTime /* = 0*/,
                                   double orginalEndTime   /* = 1 */,
                                   double scale /*= 1*/):
        timeValues(scale, orginalStartTime,orginalEndTime){
      timeValues.addStartAndEndtime();    
    }// Konstruktor
    
    SourceUnitPair::SourceUnitPair(const Interval<Instant>& orginalInterval):
                                   timeValues(orginalInterval){
    }// Konstruktor
    
    void SourceUnitPair::setScaleFactor(double scale){
      // cout << "Scale:=" << scale << endl;
      timeValues.setScaleFactor(scale);
      timeValues.addStartAndEndtime();
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
      // cout << "Start Intersection" << endl;
      // unitA.intersectionFast(unitB, timeValues);
      unitA.intersection(unitB, timeValues); 
      // cout << "End Intersection" << endl;
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
      // cout << setprecision(9) << timeValues;
      // cout << unitA;
      // cout << unitB;      
      unitA.finalize(points, timeValues, predicateA, unitB);  
      // cout << "End Finalize A" << endl;
      // cout << unitA;       
      unitB.finalize(points, timeValues, predicateB, unitA);  
      // cout << "End Finalize B" << endl;
      // cout << unitB;      
      // get result Units          
      if (timeValues.size() > 1){
        result = vector<ResultUnit>(timeValues.size()-1, ResultUnit());
        mpq_class t1,t2;
        size_t i = 0;
        timeValues.orginalFirst(t1,t2);
        // cout << "Units " << timeValues.size()-1 << endl;
        do{
          result[i] = ResultUnit(t1.get_d(),t2.get_d());
          unitA.getResultUnit(i,predicateA,false,   points,result[i],UNIT_A);
          unitB.getResultUnit(i,predicateB,inverseB,points,result[i],UNIT_B);
          // cout << result[i] <<endl;
          result[i].evaluateCriticalMSegmens(setOp);
          // cout << result[i] <<endl;          
          // cout << "Start Finalize ResultUnit "<< i << endl;
          result[i].finalize();
          // cout << "End Finalize ResultUnit "<< i << endl;
          i++;
        } while (timeValues.orginalNext(t1, t2));  
      }// if
      // cout << "End Operate" << endl;
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
          Interval<Instant> interval = result[i].getTimeInterval();
          // do not generate degenerate time intervals
          if(interval.start != interval.end){
            URegionEmb* ure = result[i].convertToURegionEmb(array);
            resMRegion->Add(*ure);
            delete ure;
          }// if
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

    void SourceUnitPair::createResultMBool( MBool* resMBool, bool lc, bool rc){
      if ((timeValues.size() > 1) && predicates.size() > 0){
        mpq_class t1,t2,t3;
        // cout << " timeValues " << timeValues <<endl;
        this->timeValues.orginalFirst(t1,t2);
        bool value = predicates[0];
        bool left  = lc;
        size_t i = 1;
        while (timeValues.orginalNext(t2, t3)){
          // cout << "t1:=" << t1 << endl;
          // cout << "t2:=" << t2 << endl;
          // if (predicates[i]) cout << "value:= true"  << endl;
          // else               cout << "value:= false" << endl; 
          if(value != predicates[i]){
            CcBool predicate(true, value);        
            UBool ubool(timeValues.createInterval(t1.get_d(),t2.get_d(),
                                                  left,false),predicate);
            resMBool->Add(ubool);
            left  = true;
            value = predicates[i];
            t1    = t2;         
          }// if
          t2 = t3;
          i++; 
        }// while  
        CcBool predicate(true, value);
        Interval<Instant> interval = timeValues.createInterval(t1.get_d(),
                                                               t2.get_d(),
                                                               true,rc);
        UBool ubool(interval, predicate);
        // do not generate degenerate time intervals
        if(interval.start != interval.end) resMBool->Add(ubool);
      }// if
    }// createResultMBool  
/*
23 class SetOperator

*/
    SetOperator::SetOperator(MRegion* const _mRegionA, 
                             MRegion* const _mRegionB,
                             MRegion* const _mRegionResult):
        mRegionA(_mRegionA), 
        mRegionB(_mRegionB), 
        mRegionResult(_mRegionResult){
    }// Konstruktor

    void SetOperator::operate(SetOp setOp){
      if (!mRegionA->IsDefined() || !mRegionB->IsDefined()) {
        mRegionResult->SetDefined(false);
        return;
      }// if
      // Compute the RefinementPartition of the two MRegions
      RefinementPartition< MRegion, MRegion, URegionEmb, URegionEmb> 
        rp(*mRegionA, *mRegionB);
      // cout << "RefinementPartition with " << rp.Size() << " units created.";
      mRegionResult->Clear();
      ((DbArray<MSegmentData>*)mRegionResult->GetFLOB(1))->clean();
      mRegionResult->StartBulkLoad();
      // For each interval of the refinement partition
      for (unsigned int i = 0; i < rp.Size(); i++) {
        Interval<Instant> interval;
        int aPos, bPos;
        bool aIsEmpty, bIsEmpty;
        rp.Get(i, interval, aPos, bPos);         
        SourceUnitPair unitPair(interval);
        if(interval.start == interval.end) continue;
        aIsEmpty = (aPos == -1);
        bIsEmpty = (bPos == -1);   
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
/*
24 class PredicateOperator

*/      
    PredicateOperator::PredicateOperator(MRegion* const _mRegionA, 
                                         MRegion* const _mRegionB,
                                         MBool* const  _mBool):
        mRegionA(_mRegionA), 
        mRegionB(_mRegionB), 
        mBool(_mBool){
    }// Konstruktor
    
    void PredicateOperator::operate(PredicateOp predicateOp){
      if (!mRegionA->IsDefined() || !mRegionB->IsDefined()) {
        mBool->SetDefined(false);
        return;
      }// if
      // Compute the RefinementPartition of the two MRegions
      RefinementPartition< MRegion, MRegion, URegionEmb, URegionEmb> 
        rp(*mRegionA, *mRegionB);
      // cout << "RefinementPartition with " << rp.Size() << " units created.";
      mBool->Clear();
      // For each interval of the refinement partition
      for (unsigned int i = 0; i < rp.Size(); i++) {
        Interval<Instant> interval;
        int aPos, bPos;
        bool aIsEmpty, bIsEmpty;
        rp.Get(i, interval, aPos, bPos);         
        SourceUnitPair unitPair(interval);
        if(interval.start == interval.end) continue; 
        aIsEmpty = (aPos == -1);
        bIsEmpty = (bPos == -1);    
        if (!aIsEmpty) {
          unitPair.createSourceUnit(interval, mRegionA, UNIT_A);
        }// if
        if (!bIsEmpty) {
          unitPair.createSourceUnit(interval, mRegionB, UNIT_B);
        }// if     
        unitPair.predicate(predicateOp);                       
        unitPair.createResultMBool(mBool,interval.lc,interval.rc);        
      }// for
    }// operate  

  } // end of namespace mregionops3
} // end of namespace temporalalgebra