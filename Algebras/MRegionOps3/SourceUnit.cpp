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

#include "SourceUnit.h"

using namespace std;

namespace temporalalgebra { 
  namespace mregionops3 {  
/*
3 class Segment

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

    std::ostream& operator <<(std::ostream& os, const Segment& segment){
      os << "Segment (" << segment.getTail() << ", " << segment.getHead();
      os << ", " << IntersectionSegment::toString(segment.getPredicate()) <<")";
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
4 class ResultPfaceFactory

*/  
    ResultPfaceFactory::ResultPfaceFactory(
        ContainerPoint3D& points,
        GlobalTimeValues &timeValues1,
        IntSegContainer &container1){
      size_t size = timeValues1.size()-1;
      for(size_t i = 0; i < size; i++){
        this->edge.push_back(vector<Segment>());
        this->orthogonal.push_back(vector<Segment>());
      }// for
      list<IntersectionSegment> result,resultOrthogonal;
      list<IntersectionSegment>::iterator iter;
      double t1,t2;
      size_t i = 0;
      if (timeValues1.first(t1)){                     
        container1.first(t1,result,resultOrthogonal);
        while(timeValues1.next(t2)){
          edge[i] = vector<Segment>();
          orthogonal[i] = vector<Segment>(); 
          for(iter = result.begin();
              iter != result.end(); iter++){
            edge[i].push_back(createEdge(points,*iter,t1,t2));
          }// for
          for(iter = resultOrthogonal.begin(); 
              iter != resultOrthogonal.end(); iter++){
            orthogonal[i].push_back(createEdge(points,*iter,t1,t2));
          }// for
          i++;
          t1 = t2;
          container1.next(t1,result,resultOrthogonal);
        }// while
      }// if                          
    }// Konstruktor
    
    Segment ResultPfaceFactory::createEdge(
        ContainerPoint3D& points,
        const IntersectionSegment& segment,double t1, double t2){
      Point3D tail, head;
      if(segment.isOrthogonalToTAxis()){
        tail = segment.getTail().getPoint3D();
        head = segment.getHead().getPoint3D(); 
      }// if
      else {
        tail = segment.evaluate(t1);
        head = segment.evaluate(t2);
      }// else
      size_t p1     = points.add(tail);
      size_t p2     = points.add(head);
      Predicate predicate = segment.getPredicate();
      return Segment(p1,p2,predicate); 
    }// create Edge
    
    
    std::ostream& ResultPfaceFactory::print(std::ostream& os, 
                                            std::string prefix)const{
      os << prefix << "ResultPfaceFactory(" << endl; 
      os << prefix << "  Edge for PFaces(" << endl;
      for(size_t i = 0; i < edge.size(); i++){
        os << prefix << "    Index:="<< i << " (";
        for(size_t j = 0; j < edge[i].size();j++){
          os << edge[i][j];
          if(j < edge[i].size()-1) os << ", ";
        }// for  
        os << ")" << endl;
      }// for
      os << prefix << "  )" << endl;
      os << prefix << "  Orthogonal Edge to t-Axis(" << endl;
      for(size_t i = 0; i < orthogonal.size(); i++){
        os << prefix << "    Index:="<< i << " (";
        for(size_t j = 0; j < orthogonal[i].size();j++){
          os << orthogonal[i][j];
          if(j < orthogonal[i].size()-1) os << ", ";
        }// for  
        os << ")" << endl;
      }// for
      os << prefix << "  )" << endl;
      os << prefix << ")" << endl;
      return os;
    }// operator <<
    
    std::ostream& operator <<(std::ostream& os, 
                              const ResultPfaceFactory& factory){   
      return factory.print(os,"");
    }// operator 
     
    SourceUnit::SourceUnit():pFaceTree(4,8){
    }// Konstruktor
    
    SourceUnit::~SourceUnit(){
      vector<PFace*>::iterator iter;
      for (iter = pFaces.begin(); iter != pFaces.end(); iter++) {   
        delete *iter;
      }// for
    }// Destruktor
      
    void SourceUnit::addPFace(const Point3D& a, const Point3D& b, 
                              const Point3D& c, const Point3D& d){
      PFace* pFace = new PFace(a,b,c,d);
      Rectangle<2> boundigRec = (*pFace).getBoundingRec();
      size_t index = pFaces.size();
      pFaces.push_back(pFace);
      pFaceTree.insert(boundigRec,index);
    }// addPFace
    
    void SourceUnit::addPFace(const PFace& pf){
      PFace* pFace = new PFace(pf);
      Rectangle<2> boundigRec = (*pFace).getBoundingRec();
      size_t index = pFaces.size();
      pFaces.push_back(pFace);
      pFaceTree.insert(boundigRec,index);
    }// addPFace

    bool SourceUnit::intersection(SourceUnit& other, 
                                  GlobalTimeValues& timeValues){
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
          pFaceA->addBorder(planeSelf, timeValues);
        }// if
      }// for
      for (iter = other.pFaces.begin(); iter != other.pFaces.end(); iter++) {
        PFace* pFaceB = *iter;
        if(pFaceB->existsIntSegs()){
          RationalPlane3D planeOther(*pFaceB);
          pFaceB->addBorder(planeOther, timeValues);
        }// if
      }// for
      return result;
    }// intersection
    
    std::ostream& operator <<(std::ostream& os, const SourceUnit& unit){
      vector<PFace*>::const_iterator iter; 
      os << "SourceUnit ("<< endl;
      for (iter = unit.pFaces.begin(); iter != unit.pFaces.end(); iter++) {
        PFace* pf = *iter;
        pf->print(os,"  ");
      }// for
      os <<")" << endl;
      return os;
    }// Operator <<
    
    bool SourceUnit::operator ==(const SourceUnit& unit)const{
      if(this->pFaces.size() != unit.pFaces.size()) return false;
      for(size_t i = 0; i < this->pFaces.size(); i++){
        if(!(*(this->pFaces[i]) == *(unit.pFaces[i]))) return false;
      }// for
      return true;
    }// Operator ==

  } // end of namespace mregionops3
} // end of namespace temporalalgebra