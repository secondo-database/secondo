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

#ifndef CONTAINER_TPP
#define CONTAINER_TPP

namespace temporalalgebra {
  namespace mregionops3 {
    
    template <typename T> Container<T>::Container():pointsTree(4, 8){
    }// Konstruktor

    template <typename T> Container<T>::Container(
       const Container<T>& other):pointsTree(4, 8){
      set(other);
    }// Konstruktor
    
    template <typename T> Container<T>::~Container(){
    }// Destruktor
    
    template <typename T> void Container<T>::set(const Container<T>& other){
      for(size_t i = 0; i< other.size(); i++){
        add(other.get(i));
      }// for
    }// set
              
    template <typename T> T Container<T>::get(const size_t index)const{
      if(index >= points.size()){
        NUM_FAIL("point index is out of range.");
      }// if  
      return points[index];
    } // get
    
    template <typename T> size_t Container<T>::add( 
        const T& point){
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
    
    template <typename T> size_t Container<T>::size()const{
      return points.size();
    } // size

    template <typename T> std::ostream& Container<T>::print(
        std::ostream& os, std::string prefix)const{ 
      os << prefix << " Container( " << endl;
      for(size_t i = 0; i< points.size(); i++){
        os << prefix << "    Index:="<< i<<", " << points[i] << endl; 
      }// for
      os << prefix << "  )" << endl;
      return os;
    }// print
       
    template <typename T>bool Container<T>::operator==(
        const Container<T>& other)const{
      if(this->points.size() != other.points.size()) return false;
      for(size_t i = 0; i < points.size(); i++){
        if(other.points[i] == this->points[i]) return false;
      }// for
      return true;
    }// Operator ==
    
    template <typename T> Container<T>& Container<T>::operator=(
        const Container<T>& points){
      set(points);
      return *this;
    }// Operator =
    
    template <typename T> std::ostream& operator<<(
        std::ostream& os, const Container<T>& container){
      return container.print(os,"");
    }// Operator << 
        
  } // end of namespace mregionops3
} // end of namespace temporalalgebra
#endif 
// CONTAINER_TPP