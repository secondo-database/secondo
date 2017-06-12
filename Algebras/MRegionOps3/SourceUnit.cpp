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
    
    void Segment::setPredicate(Predicate predicate){
      this->predicate = predicate;
    }// setPredicate

    std::ostream& operator <<(std::ostream& os, const Segment& segment){
      os << "Segment (" << segment.getTail() << ", " << segment.getHead();
      os << ", ";
      os << IntersectionSegment::toString(segment.getPredicate()) <<")";
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
        GlobalTimeValues &timeValues,
        IntSegContainer &container){
      size_t size      = timeValues.size();
      this->edges      = vector<list<Segment>>(size-1,list<Segment>());
      this->orthogonal = vector<list<Segment>>(size,list<Segment>());
      this->touch      = vector<size_t>(size,0);
      list<IntersectionSegment> resultEdges,resultOrthogonal;
      vector<list<Segment>>::iterator edgeIter, orthogonalIter;
      edgeIter         = edges.begin();
      orthogonalIter   = orthogonal.begin();
      double t1,t2;
      if (timeValues.first(t1)){                     
        container.first(t1,resultEdges,resultOrthogonal);
        while(timeValues.next(t2)){
          computeSegments(points, *edgeIter, t1, t2, resultEdges);
          computeSegments(points, *orthogonalIter, t1, t2, resultOrthogonal);
          edgeIter++;
          orthogonalIter++;
          t1 = t2;
          container.next(t1,resultEdges,resultOrthogonal);
        }// while
        computeSegments(points, *orthogonalIter, t1, t2, resultOrthogonal);
      }// if 
    }// Konstruktor
    
    ResultPfaceFactory::ResultPfaceFactory(
        ContainerPoint3D& points,
        GlobalTimeValues &timeValues,
        PFace &pface){
      size_t size      = timeValues.size();
      this->edges      = vector<list<Segment>>(size-1,list<Segment>());
      this->orthogonal = vector<list<Segment>>(size,list<Segment>());
      this->touch      = vector<size_t>(size,0);
      list<IntersectionSegment> resultEdges,resultOrthogonal;
      vector<list<Segment>>::iterator edgeIter, orthogonalIter;
      edgeIter         = edges.begin();
      orthogonalIter   = orthogonal.begin();
      double t1,t2;
      if (timeValues.first(t1)){                     
        pface.first(t1,resultEdges,resultOrthogonal);
        while(timeValues.next(t2)){
          computeSegments(points, *edgeIter, t1, t2, resultEdges);
          computeSegments(points, *orthogonalIter, t1, t2, resultOrthogonal);
          edgeIter++;
          orthogonalIter++;
          t1 = t2;
          pface.next(t1,resultEdges,resultOrthogonal);
        }// while
        computeSegments(points, *orthogonalIter, t1, t2, resultOrthogonal); 
      }// if  
    }// Konstruktor
    
    void ResultPfaceFactory::evaluate(){
      vector<list<Segment>>::iterator edgeIter, orthogonalIter;
      vector<size_t>::iterator touchIter;
      orthogonalIter = orthogonal.begin();
      touchIter      = touch.begin();        
      for( edgeIter  = edges.begin(); edgeIter != edges.end(); edgeIter++){
        evaluate(edgeIter, orthogonalIter, touchIter);
        orthogonalIter++;
        touchIter++;
      }// for
    }// evaluate
      
    void ResultPfaceFactory::evaluate(
        vector<list<Segment>>::iterator edgeIter, 
        vector<list<Segment>>::iterator orthogonalIter,
        vector<size_t>::iterator touchIter){                
      list<Segment>::iterator first, predecessor, successor;
      list<Segment>::iterator segment;
      if(edgeIter->size() < 2) return;
      first = edgeIter->begin();
      successor = predecessor = first;    
      for(successor++; successor != edgeIter->end(); successor++){
        Predicate predicate = UNDEFINED;
        checkPredicate(*predecessor,LEFT,predicate); 
        checkPredicate(*successor,RIGHT,predicate);        
        if(first->getTail() == successor->getTail()) (*touchIter)++;
        if(first->getHead() == successor->getHead()) (*(touchIter + 1))++;
        if(predicate == UNDEFINED){
          // check bottom
          for(segment  = orthogonalIter->begin(); 
              segment != orthogonalIter->end(); 
              segment ++){  
            if(predecessor->getTail() == segment->getTail() && 
               successor->getTail()   == segment->getHead()){
              // cout << "Bottom" <<endl;
              checkPredicate(*segment,RIGHT,predicate); 
              break;
            }// if
          }// for
          // check top
          for(segment  = (orthogonalIter + 1)->begin(); 
              segment != (orthogonalIter + 1)->end(); 
              segment ++){ 
            if(predecessor->getHead() == segment->getTail() && 
               successor->getHead() == segment->getHead()){
              // cout << "Top" <<endl;
             checkPredicate(*segment,LEFT,predicate);
             break;
           }// if
          }// for
        }// if
        setPredicate(*predecessor, predicate);
        setPredicate(*successor, predicate);
        predecessor = successor; 
      }// for
      for(segment  = orthogonalIter->begin(); 
          segment != orthogonalIter->end(); 
          segment ++){ 
        if(first->getTail() == segment->getTail()) (*touchIter)++;
        if(first->getHead() == segment->getTail()) (*(touchIter + 1))++;
      }// for   
    }// evaluate   
    
    void ResultPfaceFactory::setPredicate(Segment& segment, 
                                          Predicate& predicate)const{
       if(segment.getPredicate() == UNDEFINED) 
          segment.setPredicate(predicate);
    }// setPredicate
          
    void ResultPfaceFactory::checkPredicate(const Segment& segment,
                                            const Border border, 
                                            Predicate& result)const{
      Predicate predicate = UNDEFINED;
      switch(segment.getPredicate()){        
        case INSIDE:         predicate = INSIDE;  
                             break;                          
        case OUTSIDE:        predicate = OUTSIDE;
                             break;
        case LEFT_IS_INNER:  if(border == LEFT) predicate = OUTSIDE;
                             else predicate = INSIDE;
                             break;    
        case RIGHT_IS_INNER: if(border == LEFT) predicate = INSIDE;
                             else predicate = OUTSIDE;
                             break;     
        default:             return;           
      }// switch
      if(result == UNDEFINED) result = predicate;
      else if (!(result == predicate)) {
        cout << "Error "  << endl;
      }// else if 
    }// checkPredicate
    
    void ResultPfaceFactory::computeSegments(
        ContainerPoint3D& points, list<Segment>& edges,
        double t1, double t2,
        const list<IntersectionSegment>& segments){
      list<IntersectionSegment>::const_iterator iter; 
      Segment predecessor;
      for(iter = segments.begin(); iter != segments.end(); iter++){
        Segment segment = createEdge(points,*iter,t1,t2);
        if(edges.size() == 0 || checkSegment(predecessor, segment)){
          edges.push_back(segment);
          predecessor = edges.back(); 
        }// if
//        else {
//           if(checkSegment(*predecessor, segment)){
//             edges.push_back(segment);
//             predecessor = edges.end()-1; 
//           }// if
//           // existiert das Segment bereits
//         }// else
      }// for
    }// computeSegments
    
    bool ResultPfaceFactory::checkSegment(Segment& predecessor,
                                          const Segment& segment)const {
      if(segment.getTail() == predecessor.getTail() &&
         segment.getHead() == predecessor.getHead()){
        Predicate predicate1 = predecessor.getPredicate();
        Predicate predicate2 = segment.getPredicate(); 
        if(predicate1 == UNDEFINED){
          if (predicate2 == LEFT_IS_INNER || predicate2 == RIGHT_IS_INNER){
            predecessor.setPredicate(predicate2);
            return false;
          }// if
          if (predicate2 == UNDEFINED) return false;
        }// if
        else if( predicate1 == LEFT_IS_INNER &&
                (predicate2 == LEFT_IS_INNER ||
                 predicate2 == UNDEFINED)) return false;
        else if( predicate1 == RIGHT_IS_INNER &&
                (predicate2 == RIGHT_IS_INNER ||
                 predicate2 == UNDEFINED)) return false;
        else {
          cout << "Error "  << endl;
          return false;
        }// else
      }// if
      return true;       
    }// checkSegment
    
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
      vector<list<Segment>>::const_iterator iter1; 
      list<Segment>::const_iterator iter2;
      for(iter1 = edges.begin(); iter1 != edges.end(); iter1++){
        os << prefix << "    Index:=" << iter1 - edges.begin() << " (";
        for(iter2 = iter1->begin(); iter2 != iter1->end();){
          os << *iter2;
          iter2++;
          if(iter2 == iter1->end()) os << ", ";
        }// for  
        os << ")" << endl;
      }// for
      os << prefix << "  )" << endl;
      for(iter1 = orthogonal.begin(); iter1 != orthogonal.end(); iter1++){
        os << prefix << "    Index:=" << iter1 - orthogonal.begin() << " (";
        for(iter2 = iter1->begin(); iter2 != iter1->end();){
          os << *iter2;
          iter2++;
          if(iter2 == iter1->end()) os << ", ";
        }// for  
        os << ")" << endl;
      }// for
      os << prefix << "  )" << endl;
      vector<size_t>::const_iterator iter3;
      os << prefix << "  Touch on left border(" << endl;
      for(iter3 = touch.begin(); iter3 != touch.end(); iter3++){
        os << prefix << "    Index:="<< iter3 - touch.begin() << " (";
        os << *iter3 << ")" << endl;
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
    
    void SourceUnit::createResultPfaces(ContainerPoint3D& points, 
                                        GlobalTimeValues &timeValues){
      vector<PFace*>::iterator iter; 
      for (iter = this->pFaces.begin(); iter != this->pFaces.end(); iter++) {
         ResultPfaceFactory factory(points,timeValues,**iter);
         // cout << factory;
         factory.evaluate();
         cout << factory;
      }// for
    }// createResultPfaces
    
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