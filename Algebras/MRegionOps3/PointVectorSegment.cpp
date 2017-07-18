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

#include "PointVectorSegment.h"

namespace temporalalgebra{
  namespace mregionops3 {   
    
    std::string toString(Predicate predicate){
      switch(predicate){
        case UNDEFINED: return "UNDEFINED";
        case LEFT_IS_INNER:   return "LEFT_IS_INNER";
        case RIGHT_IS_INNER:  return "RIGHT_IS_INNER";
        case INSIDE: return "INSIDE";
        case OUTSIDE: return "OUTSIDE";
        case INTERSECT: return "INTERSECT";
        default: return "";
      }// switch
    }// toString
/*
3 Class Point3D

*/    
    Point3D::Point3D():x(0),y(0),z(0){
    }// Konstruktor
    
    Point3D::Point3D(const Point3D& point){
      set(point);
    }// Konstruktor
    
    Point3D::Point3D(const RationalPoint3D& point){
      set(point);
    }// Konstruktor

    Point3D::Point3D(double x, double y, double z){
      set(x,y,z);
    }// Konstruktor  
      
    void Point3D::set(const Point3D& point){
      this->x = point.x;
      this->y = point.y;
      this->z = point.z;      
    }// set
    
    void Point3D::set(const RationalPoint3D& point){
      this->x = point.getX().get_d();
      this->y = point.getY().get_d();
      this->z = point.getZ().get_d();      
    }// set
    
    void Point3D::set(double x, double y, double z){
      this->x = x;
      this->y = y;
      this->z = z;
    }// set
    
    double Point3D::getX() const{
      return x;
    }// getX
    
    double Point3D::getY() const{
      return y;
    }// getY
    
    double Point3D::getZ() const{
      return z;    
    }// getZ
    
    RationalPoint3D Point3D::getR() const{
      return RationalPoint3D(x,y,z);  
    }// get
    
    Rectangle<3> Point3D::getBoundingBox()const{
      double array[3] = {x,y,z};
      return Rectangle<3>(true,array,array);
    }// getBoundingBox 
    
    std::ostream& operator <<(std::ostream& os, const Point3D& point){
      os << "Point3D (" << point.x;
      os << ", " << point.y;
      os << ", " << point.z << ")";
      return os; 
    }// operator <<
    
    bool Point3D::operator !=(const Point3D& point) const{
      return !(*this == point);
    }// Operator !=
    
    bool Point3D::operator ==(const Point3D& point) const{
      return NumericUtil::nearlyEqual(this->x, point.x) && 
             NumericUtil::nearlyEqual(this->y, point.y) && 
             NumericUtil::nearlyEqual(this->z, point.z);
    }// Operator ==
    
    Point3D& Point3D::operator =(const Point3D& point){
      set(point);
      return *this;
    }// Operator =
/*
4 Class RationalPoint3D

*/      
    RationalPoint3D::RationalPoint3D():x(0),y(0),z(0){
    }// Konstruktor
    
    RationalPoint3D::RationalPoint3D(const Point3D& point){
      set(point);
    }// Konstruktor
    
    RationalPoint3D::RationalPoint3D(const RationalPoint3D& point){
      set(point);
    }// Konstruktor

    RationalPoint3D::RationalPoint3D(mpq_class x, mpq_class y, mpq_class z){
      set(x,y,z);
    }// Konstruktor  
      
    void RationalPoint3D::set(const Point3D& point){
      this->x = point.getX();
      this->y = point.getY();
      this->z = point.getZ();      
    }// set
    
    void RationalPoint3D::set(const RationalPoint3D& point){
      this->x = point.x;
      this->y = point.y;
      this->z = point.z;      
    }// set
    
    void RationalPoint3D::set(mpq_class x, mpq_class y, mpq_class z){
      this->x = x;
      this->y = y;
      this->z = z;
    }// set
    
    mpq_class RationalPoint3D::getX() const{
      return x;
    }// getX
    
    mpq_class RationalPoint3D::getY() const{
      return y;
    }// getY
    
    mpq_class RationalPoint3D::getZ() const{
      return z;    
    }// getZ
    
    Point3D RationalPoint3D::getD() const{
      return Point3D(x.get_d(),y.get_d(),z.get_d());  
    }// get
    
    std::ostream& operator <<(std::ostream& os, const RationalPoint3D& point){
      os << "RationalPoint3D (" << point.x.get_d();
      os << ", " << point.y.get_d();
      os << ", " << point.z.get_d() << ")";
      return os; 
    }// operator <<
    
    bool RationalPoint3D::operator !=(const RationalPoint3D& point) const{
      return !(*this == point);
    }// Operator !=
    
    bool RationalPoint3D::operator ==(const RationalPoint3D& point) const{
      return NumericUtil::nearlyEqual(this->x, point.x) && 
             NumericUtil::nearlyEqual(this->y, point.y) && 
             NumericUtil::nearlyEqual(this->z, point.z);
    }// Operator ==
    
    RationalPoint3D& RationalPoint3D::operator =(const RationalPoint3D& point){
      set(point);
      return *this;
    }// Operator =
      
    RationalPoint3D RationalPoint3D::operator +(
        const RationalVector3D& vector) const{
      RationalPoint3D point;
      point.x = (this->x + vector.getX());
      point.y = (this->y + vector.getY());
      point.z = (this->z + vector.getZ());
      return point;    
    }// Operator +
    
    RationalPoint3D RationalPoint3D::operator -(
        const RationalVector3D& vector) const{
      RationalPoint3D point;
      point.x = (this->x - vector.getX());
      point.y = (this->y - vector.getY());
      point.z = (this->z - vector.getZ());
      return point;
    }// Operator -
    
    RationalVector3D RationalPoint3D::operator -(
        const RationalPoint3D& point) const{
      return RationalVector3D(this->x - point.x, this->y - point.y, 
                              this->z - point.z);
    }// Operator -
         
    mpq_class RationalPoint3D::distance2(const RationalPoint3D& point)const{
       return (point - *this).length2();      
    }// distance2
      
    double RationalPoint3D::distance(const RationalPoint3D& point)const{
       return (point - *this).length();      
    }// distance     
/*
5 Class RationalVector3D

*/    
    RationalVector3D::RationalVector3D():x(0),y(0),z(0){
    }// Konstruktor
    
    RationalVector3D::RationalVector3D(const RationalVector3D& vector){
      set(vector);
    }// Konstruktor
    
    RationalVector3D::RationalVector3D(const mpq_class& x, const mpq_class& y,
                                       const mpq_class& z){
      set(x,y,z);
    }// Konstruktor
    
    void RationalVector3D::set(const RationalVector3D& vector){
      this->x = vector.x;
      this->y = vector.y;
      this->z = vector.z;
    }// set
    
    void RationalVector3D::set(const mpq_class& x, const mpq_class& y, 
                               const mpq_class& z){
      this->x = x;
      this->y = y;
      this->z = z;
    }// set
    
    mpq_class RationalVector3D::getX() const{
      return x;
    }// getX
    
    mpq_class RationalVector3D::getY() const{
      return y;
    }// GetY
    
    mpq_class RationalVector3D::getZ() const{
      return z;    
    }// getZ
    
    bool RationalVector3D::operator !=(const RationalVector3D& vector) const{
      return !(*this == vector);
    }// Operator !=
    
    bool RationalVector3D::operator ==(const RationalVector3D& vector) const{
      return NumericUtil::nearlyEqual(this->x, vector.x) && 
             NumericUtil::nearlyEqual(this->y, vector.y) && 
             NumericUtil::nearlyEqual(this->z, vector.z);
    }// Operator ==
    
    RationalVector3D& RationalVector3D::operator = (
        const RationalVector3D& vector){
      set(vector);
      return *this;
    }// Operator =  
    
    std::ostream& operator <<(std::ostream& os,const RationalVector3D& vector){
      os << "RationalVector3D ( "<< vector.x.get_d();
      os << ", " << vector.y.get_d();
      os << ", " << vector.z.get_d()<<")";
      return os; 
    }// operator <<

    RationalVector3D operator *(const mpq_class& value, 
                                const RationalVector3D& vector){
      RationalVector3D result;
      result.x = value * vector.x;
      result.y = value * vector.y;
      result.z = value * vector.z;
      return result;
    }// Operator *
    
    RationalVector3D operator *(const RationalVector3D& vector, 
                                const mpq_class& value){
      RationalVector3D result;
      result.x = value * vector.x;
      result.y = value * vector.y;
      result.z = value * vector.z;
      return result;
    }// Operator *  
    
    RationalVector3D RationalVector3D::operator -() const{
      return RationalVector3D(-this->x, -this->y, -this->z);
    }// Operator -

    mpq_class RationalVector3D::operator *(
        const RationalVector3D& vector) const{
      return (this->x * vector.x + this->y * vector.y + this->z * vector.z);
    }// Operator *
       
    RationalVector3D RationalVector3D::operator ^(
        const RationalVector3D& vector) const{
      RationalVector3D result;
      result.x = this->y * vector.z - this->z * vector.y;
      result.y = this->z * vector.x - this->x * vector.z;
      result.z = this->x * vector.y - this->y * vector.x;
      return result;
    }// Operatopr ^ 
       
    void RationalVector3D::normalize(){
      mpq_class len2 = this->x * this->x + 
                       this->y * this->y + 
                       this->z * this->z;                       
      double len = sqrt(len2.get_d());
      if (len != 0.0) {
        x /= len;
        y /= len;
        z /= len;
      }// if
    }// normilize
    
    mpq_class RationalVector3D::length2() const{
      return x*x + y*y + z*z;
    }// length2
    
    double RationalVector3D::length() const{
      return sqrt(length2().get_d());    
    }// length  
/*
6 Class Segment3D

*/      
    Segment3D::Segment3D(){   
    }// Konstruktor    
    
    Segment3D::Segment3D(const Point3D& tail, const Point3D& head){
      this->tail = tail;
      this->head = head; 
    }// Konstruktor
    
    Segment3D::Segment3D(const Segment3D& segment){
      set(segment);
    }// konstruktor
    
    Segment3D::Segment3D(const RationalSegment3D& segment){
      this->tail = segment.getTail();
      this->head = segment.getHead(); 
    }// konstruktor    
    
    void Segment3D::set(const Segment3D& segment){
      this->tail = segment.tail;
      this->head = segment.head; 
    }// konstruktor   

    Point3D Segment3D::getHead() const{
      return this->head;
    }// getHead
    
    Point3D Segment3D::getTail() const{
      return this->tail;
    }// getTail 

    RationalSegment3D Segment3D::getR()const {
      return RationalSegment3D(tail,head);
    }// getR
          
    std::ostream& operator <<(std::ostream& os, const Segment3D& segment){
      os << "Segment3D (" << segment.tail << ", " << segment.head << ")";
      return os; 
    }// operator <<
    
    bool Segment3D::operator ==(const Segment3D& segment) const{
      return (this->head == segment.head && this->tail == segment.tail);
    }// operator == 
    
    Segment3D&  Segment3D::operator =(const Segment3D& segment){
      set(segment);
      return *this;      
    }// Operator =    
/*
7 Class RationalSegment3D

*/
    RationalSegment3D::RationalSegment3D(){
    }// Konstruktor
    
    RationalSegment3D::RationalSegment3D(const RationalSegment3D& segment){
      set(segment);
    }// Konstruktor
    
    RationalSegment3D::RationalSegment3D(const Segment3D& segment){
      this->tail = segment.getTail();
      this->head = segment.getHead();
    }// Kostruktor
    
    RationalSegment3D::RationalSegment3D(const RationalPoint3D& tail, 
                                         const RationalPoint3D& head){
      this->tail = tail;
      this->head = head;
    }// konstruktor

    void RationalSegment3D::set(const RationalSegment3D& segment){
      this->tail = segment.tail;
      this->head = segment.head;
    }// set
    
    RationalPoint3D RationalSegment3D::getHead() const{
      return head;
    }// getHead
    
    RationalPoint3D RationalSegment3D::getTail() const{
      return tail;
    }// getTail
    
    Segment3D RationalSegment3D::getD() const{
      return Segment3D(tail.getD(), head.getD());
    }// get
    
    std::ostream& operator <<(std::ostream& os, 
                              const RationalSegment3D& segment){
       os << "RationalSegment3D (" << segment.tail << ", ";
       os << segment.head << ")";
      return os; 
    }// Operator << 

    bool RationalSegment3D::operator ==(
        const RationalSegment3D& segment) const{
      return (this->head == segment.head && this->tail == segment.tail);
    }// operator == 
    
    RationalSegment3D& RationalSegment3D::operator =(
        const RationalSegment3D& segment){
      set(segment);
      return *this;
    }// operator =
/*
8 Class Point2D

*/      
    Point2D::Point2D():x(0),y(0){
    }// Konstruktor
    
    Point2D::Point2D(const Point2D& point){
      set(point);    
    }// Konstruktor
    
    Point2D::Point2D(const RationalPoint2D& point){
      set(point.getX().get_d(),point.getY().get_d());  
    }// konstruktor
    
    Point2D::Point2D(double x, double y){
      set(x,y);
    }// Konstriktor
    
    void  Point2D::set(const Point2D& point){
      this->x = point.x;
      this->y = point.y;
    }// set
    
    void  Point2D::set(double x, double y){
      this->x = x;
      this->y = y;
    }// set
    
    double  Point2D::getX() const{
      return x;
    }// getX
    
    double  Point2D::getY() const{
      return y;
    }// getY
            
    RationalPoint2D Point2D::getR()const{
      return RationalPoint2D(x,y);
    }// getR
    
    bool Point2D::operator ==(const Point2D& point) const{
      return NumericUtil::nearlyEqual(this->x, point.x) && 
             NumericUtil::nearlyEqual(this->y, point.y);
    }// Operator ==
    
    Point2D&  Point2D::operator =(const Point2D& point){
      set(point);
      return *this;      
    }// Operator =
         
    std::ostream& operator <<(std::ostream& os, const Point2D& point){
      os << "Point2D (" << point.x;
      os << ", " << point.y << ")";
      return os; 
    }// Operator <<
    
    bool Point2D::operator <(const Point2D& point) const {
      if (NumericUtil::lower(x, point.x))
        return true;
      if (NumericUtil::greater(x, point.x))
        return false;           
      return NumericUtil::lower(y, point.y);
    }// Operator <
/*
9 Class RationalPoint2D

*/     
    RationalPoint2D::RationalPoint2D():x(0),y(0){
    }// Konstruktor
    
    RationalPoint2D::RationalPoint2D(const RationalPoint2D& point){
      set(point);
    }// Konstruktor
    
    RationalPoint2D::RationalPoint2D(const Point2D& point){
      set(point.getX(),point.getY());
    }// konstruktor
    
    RationalPoint2D::RationalPoint2D(const mpq_class& x, 
                                     const mpq_class& y){
      set(x,y);
    }// Konstruktor
    
    void RationalPoint2D::set(const RationalPoint2D& point){
      this->x = point.x;
      this->y = point.y;
    }// set
    
    void RationalPoint2D::set(const mpq_class& x, const mpq_class& y){
      this->x = x;
      this->y = y;
    }// set
    
    mpq_class RationalPoint2D::getX() const{
      return x;
    }// getX
    
    mpq_class RationalPoint2D::getY() const{
      return y;
    }// getY
    
    Point2D RationalPoint2D::getD()const{
      return (Point2D(x.get_d(),y.get_d()));
    }// getD

    bool RationalPoint2D::operator ==(const RationalPoint2D& point) const{
      return NumericUtil::nearlyEqual(this->x, point.x) && 
      NumericUtil::nearlyEqual(this->y, point.y);
    }// Operator ==

    RationalPoint2D& RationalPoint2D::operator =(const RationalPoint2D& point){
      set(point);
      return *this;  
    }// Operator =
    
    std::ostream& operator <<(std::ostream& os, const RationalPoint2D& point){
      os << "RationalPoint2D (" << point.x;
      os << ", " << point.y << ")";
      return os; 
    }// Oparator <<
    
    RationalVector2D RationalPoint2D::operator -(
        const RationalPoint2D& point) const{
      return RationalVector2D(x - point.x, y - point.y);
    }// Operator -  
/*
10 Class RationalVector2D

*/     
    RationalVector2D::RationalVector2D():x(0),y(0){
    }// Konstruktor
    
    RationalVector2D::RationalVector2D(const RationalVector2D& vector){
      set(vector);
    }// Konstruktor  
    
    RationalVector2D::RationalVector2D(const mpq_class& x, 
                                       const mpq_class& y){
      set(x,y);
    }// Konstruktor
    
    void RationalVector2D::set(const RationalVector2D& vector){
      this->x = vector.x;
      this->y = vector.y;
    }// set
    
    void RationalVector2D::set(const mpq_class& x, const mpq_class& y){
      this->x = x;
      this->y = y;
    }// set
    
    mpq_class RationalVector2D::getX() const{
      return x;
    }// getX
    
    mpq_class RationalVector2D::getY() const{
      return y;
    }// getY


    bool RationalVector2D::operator ==(const RationalVector2D& point) const{
      return NumericUtil::nearlyEqual(this->x, point.x) && 
      NumericUtil::nearlyEqual(this->y, point.y);
    }// Operator ==

    RationalVector2D& RationalVector2D::operator =(
        const RationalVector2D& vector){
      set(vector);
      return *this;  
    }// Operator =
    
    std::ostream& operator <<(std::ostream& os, 
                              const RationalVector2D& vector){
      os << "RationalVector2D (" << vector.x;
      os << ", " << vector.y << ")";
      return os; 
    }// Oparator <<
    
    void RationalVector2D::normalize(){
      mpq_class len2 = this->x * this->x + 
                       this->y * this->y;                       
      double len = sqrt(len2.get_d());
      if (len != 0.0) {
        x /= len;
        y /= len;
      }// if
    }// normalize
   
     mpq_class RationalVector2D::operator |(
         const RationalVector2D& vector) const{
       return (x * vector.y - y * vector.x);    
     }// operator |
/*
11 Class Segment2D

*/      
    Segment2D::Segment2D(){
    }// Konstruktor
      
    Segment2D::Segment2D(const Point2D& tail, const Point2D& head){
      this->tail = tail;
      this->head = head;
    }// Konstruktor 

    Point2D Segment2D::getHead() const{
      return this->head;
    }// getHead
      
    Point2D Segment2D::getTail() const{
      return this->tail;
    }// getTail      
      
    std::ostream& operator <<(std::ostream& os, const Segment2D& segment){
      os << "Segment2D (" << segment.tail << ", " << segment.head << ")";
      return os; 
    }// Opaerator 
    
     bool Segment2D::operator ==(const Segment2D& segment) const{
      return (this->head == segment.head && this->tail == segment.tail);
    }// operator == 
    
    double Segment2D::whichSide(const Point2D& point)const{
      // This is the fast version:
      // value = (start.x - x) * (end.y - y) - (end.x - x) * (start.y - y);
      // This is slower, but numerical more stable:
      RationalVector2D vector1 = head.getR() - 
                                 tail.getR();
      RationalVector2D vector2 = point.getR() - 
                                 tail.getR();      
      vector1.normalize();
      vector2.normalize();
      mpq_class value = vector1 | vector2;     
      return value.get_d(); 
    }// isLeft
    
    bool Segment2D::isLeft(const Point2D& point)const{
      return NumericUtil::greater(whichSide(point), 0.0); 
    }// isLeft
    
/*
12 Class ContainerPoint3D

*/       
    ContainerPoint3D::ContainerPoint3D():pointsTree(4, 8){
    }// Konstruktor

    ContainerPoint3D::ContainerPoint3D(
       const ContainerPoint3D& other):pointsTree(4, 8){
      set(other);
    }// Konstruktor  
    
    void ContainerPoint3D::set(const ContainerPoint3D& other){
      for(size_t i = 0; i< other.size(); i++){
        add(other.get(i));
      }// for
    }// set
              
    Point3D ContainerPoint3D::get(size_t index)const{
      if(index >= points.size()){
        NUM_FAIL("point index is out of range.");
      }// if  
      return points[index];
    } // get
    
    size_t ContainerPoint3D::add( 
        const Point3D& point){
      Rectangle<3> bbox=point.getBoundingBox(); 
      bbox.Extend(NumericUtil::eps2);
      std::unique_ptr<mmrtree::RtreeT<3, size_t>::iterator> 
        it(pointsTree.find(bbox));
      size_t const* index;
      while((index = it->next()) != 0){
        if(points[*index] == point) return *index;
      }// while
      size_t newIndex = points.size();
      points.push_back(point);
      pointsTree.insert(bbox, newIndex);
      return newIndex;
    }// add
    
    size_t ContainerPoint3D::size()const{
      return points.size();
    } // size

    std::ostream& ContainerPoint3D::print(
        std::ostream& os, std::string prefix)const{ 
      os << prefix << "Container( " << endl;
      for(size_t i = 0; i< points.size(); i++){
        os << prefix << "  Index:="<< i<<", " << points[i] << endl; 
      }// for
      os << prefix << ")" << endl;
      return os;
    }// print
       
    bool ContainerPoint3D::operator==(
        const ContainerPoint3D& other)const{
      if(this->points.size() != other.points.size()) return false;
      for(size_t i = 0; i < points.size(); i++){
        if(other.points[i] != this->points[i]) {
           cout << "Faile on Index:=" << i << ", ";
           cout << this->points[i] << ", ";
           cout << other.points[i] << endl;
          return false;
        }// if
      }// for
      return true;
    }// Operator ==
    
    ContainerPoint3D& ContainerPoint3D::operator=(
        const ContainerPoint3D& points){
      set(points);
      return *this;
    }// Operator =
    
    std::ostream& operator<<(
        std::ostream& os, const ContainerPoint3D& container){
      return container.print(os,"");
    }// Operator << 
/*
13 class Segment

*/      
    Segment::Segment (){
      this->head = 0;
      this->tail = 0;
      this->predicate = UNDEFINED;
    }// Konstruktor    
    
    Segment::Segment (size_t tail, size_t head, Predicate predicate){
      this->head = head;
      this->tail = tail;
      this->predicate = predicate;
    }// Konstruktor
    
    Segment::Segment (const Segment& segment){
      set(segment);
    }// KOnstruktor
    
    void Segment::set(const Segment& segment){
      this->head = segment.head;
      this->tail = segment.tail;
      this->predicate = segment.predicate;
    }// set
    
    size_t Segment::getHead()const{
      return this->head;
    }// getHead
    
    size_t Segment::getTail()const{
      return this->tail;
    }// getTail
      
    Predicate Segment::getPredicate() const{
      return this->predicate;
    }// getPredicate
    
    void Segment::setPredicate(Predicate predicate){
      this->predicate = predicate;
    }// setPredicate

    std::ostream& operator <<(std::ostream& os, const Segment& segment){
      os << "Segment (" << segment.getTail() << ", " << segment.getHead();
      os << ", ";
      os << toString(segment.getPredicate()) <<")";
      return os;
    }// operator <<

    bool Segment::operator ==(const Segment& segment) const{
      if((this->head == segment.head) &&
         (this->tail  == segment.tail) &&
         (this->predicate == segment.predicate)) return true;
      return false;
    }// Operator ==

    Segment& Segment::operator =(const Segment& segment){
      set(segment);
      return *this;
    }// Operator = 
/*
14 class ContainerSegment

*/      
    ContainerSegment::ContainerSegment():
       segmentBuckets(std::vector<std::list<size_t>>(buckets,
                                                     std::list<size_t>())){
    }// Konstruktor
      
    ContainerSegment::ContainerSegment(const ContainerSegment& other):
       segmentBuckets(std::vector<std::list<size_t>>(buckets,
                                                     std::list<size_t>())){
      set(other);
    }// Konstruktor
    
    void ContainerSegment::set(const ContainerSegment& other){
      for(size_t i = 0; i< other.size(); i++){
        add(other.get(i));
      }// for
    }// set
    
    size_t ContainerSegment::size()const {
      return segments.size(); 
    }// size    
    
    size_t ContainerSegment::getHash(const Segment& segment)const{
      return (segment.getTail() + 
             this->buckets * segment.getHead()) % this->buckets;
    }// getHash

    size_t ContainerSegment::add(const Segment& segment){
      size_t hash = getHash(segment);
      std::list<size_t>::const_iterator iterator;
      for(iterator = segmentBuckets[hash].begin(); 
          iterator != segmentBuckets[hash].end(); iterator ++){
        Segment other = segments[*iterator];
        if(other.getTail() == segment.getTail() &&
           other.getHead() == segment.getHead()){
          if(other.getPredicate() == UNDEFINED) {
            segments[*iterator] = segment;
            return *iterator;
          }// if
          else if(other.getPredicate() == segment.getPredicate()){
            return *iterator;
          }// else if
          else if(segment.getPredicate()== UNDEFINED){
            return *iterator;
          }// else if  
          NUM_FAIL("Segments with same points and different predicates.");
        }// if 
      }// for
      size_t index = segments.size();
      segments.push_back(segment);
      segmentBuckets[hash].push_back(index);
      return index;
    }// add
      
    void ContainerSegment::set(size_t index, Predicate predicate){
      if(index < segments.size()){
        if(segments[index].getPredicate() == UNDEFINED){
          segments[index].setPredicate(predicate);
        }// if  
        else if(segments[index].getPredicate() == predicate) return;
        else {
          // cout << "Segment:=" << index << endl;
          // cout << "Old:="<< toString(segments[index].getPredicate())<<endl;
          // cout << "New:="<< toString(predicate)<<endl;          
          NUM_FAIL("Combination from old and new predicate is invalid."); 
        }
      }// if
      else NUM_FAIL("Index is out of range.");        
    }// set
      
    Segment ContainerSegment::get(size_t index)const{
      if(index < segments.size()){
        return segments[index];
      }// if
      else NUM_FAIL("Index is out of range.");  
    }// get
      
    std::ostream& ContainerSegment::print(std::ostream& os, 
                                          std::string prefix)const{
      os << prefix << "ContainerSegment ( " << endl;
      for(size_t i = 0; i < segments.size(); i++){
        os << prefix << "  Index:=" << i << ", " << segments[i] << endl;
      }// for
      //for(size_t i = 0; i < segmentBuckets.size(); i++){
      //  os << prefix << "  Bucket:=" << i << "( ";
      //  std::list<size_t>::const_iterator iterator;
      //  for(iterator  = segmentBuckets[i].begin(); 
      //      iterator != segmentBuckets[i].end();){  
      //    os << *iterator;
      //     iterator++;
      //    if(iterator != segmentBuckets[i].end()) os << ", "; 
      //  }// for
      //  os << ")" << endl;
      //}// for
      os << prefix << ")" << endl;
      return os;
    }// print
      
    std::ostream& operator<<( std::ostream& os,
                              const ContainerSegment& container){
      return container.print(os,"");
    }// Operator <<
      
    bool ContainerSegment::operator == (const ContainerSegment& other)const{
      if(this->segments.size() != other.segments.size()) return false;
      for(size_t i = 0; i < this->segments.size(); i++){
        if(!(other.segments[i] == this->segments[i])) {
          // cout << "Faile on Index:=" << i << ", ";
          // cout << this->segments[i] << ", ";
          // cout << other.segments[i] << endl;
          return false;
        }// if
      }// for
      return true;
    }// Operator ==
      
    ContainerSegment& ContainerSegment::operator = (
        const ContainerSegment& segments){
      set(segments);
      return *this;
    }// Operator =
    
    void ContainerSegment::clear(){
      segments.clear();      
    }// clear

  } // end of namespace mregionops3
} // end of namespace temporalalgebra
