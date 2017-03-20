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
    
    RationalPoint3D Point3D::get() const{
      return RationalPoint3D(x,y,z);  
    }// get
    
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
4 Class Segment3D

*/      
    Segment3D::Segment3D(){   
    }// Konstruktor
    
    Segment3D::Segment3D(const Point3D& tail, const Point3D& head){
      this->tail = tail;
      this->head = head;    
    }// Konstruktor

    Point3D Segment3D::getHead() const{
      return head;
    }// getHead
    
    Point3D Segment3D::getTail() const{
      return tail;
    }// getTail 
  
    bool Segment3D::isOrthogonalToZAxis() const{
      return NumericUtil::nearlyEqual(tail.getZ(), head.getZ());
    }// isOrthogonalToZAxis
     
    double Segment3D::length() const{
      return (tail.get() - head.get()).length();
    }// length
    
    mpq_class Segment3D::length2() const{
      return (tail.get() - head.get()).length2();
    }// length2
        
    std::ostream& operator <<(std::ostream& os, const Segment3D& segment){
      os << "Segment3D (" << segment.tail << ", " << segment.head << ")";
      return os; 
    }// operator <<
    
    bool Segment3D::operator ==(const Segment3D& segment) const{
      return (this->head == segment.head && this->tail == segment.tail);
    }// operator ==
/*
5 Class RationalPoint3D

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
    
    Point3D RationalPoint3D::get() const{
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
6 Class RationalVector3D

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
 
  } // end of namespace mregionops3
} // end of namespace temporalalgebra
