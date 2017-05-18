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
#include "Container.tpp"

using namespace std;

namespace temporalalgebra { 
  namespace mregionops3 {
    
        
    template class Container<IntersectionPoint>;
    template std::ostream& operator<<(
      std::ostream& os, const Container<IntersectionPoint>& container); 
   
    
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
              pFaceA->setState(RELEVANT_CRITICAL);
              pFaceB->setState(RELEVANT_CRITICAL);           
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