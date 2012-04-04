/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

[1] The file is originally provided by Thomas Wolle, and integrated into SECONDO
by Mahmoud Sakr

June, 2009 Mahmoud Sakr

[TOC]

*/

#ifndef OCTREE_ELEMENTS_SOURCE
#define OCTREE_ELEMENTS_SOURCE


#include "OctreeElements.h"
#include "SkipTreeFunctions.h"
//#include "LEDA/numbers/integer.h"
#include <gmpxx.h>
#include <assert.h>
#include <cmath>
#include <map>
#include <vector>
#include <iostream>
#include <iomanip>
//#include <iostream.h>

//using namespace leda;

//#define SHOW_DEBUG 1


class OctreeElement;
class OctreePoint;
class OctreeCell;


inline
double
min(double a, double b){
  if (a <= b)
    return a;
  else
    return b;
}

inline
double
max(double a, double b){
  if (a >= b)
    return a;
  else
    return b;
}

mpf_class* 
hsl2vol(double hsl, int dim){
  assert(dim>0);
  hsl= hsl * 2;
  mpf_class* vol = new mpf_class(dim);
  while(dim>1){
    *vol = *vol * hsl;
    dim--;
  }
  return vol;
}

////////////////////////////////////////////////////////////////////////////////
/////////// Octree Element /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

OctreeElement::OctreeElement(int dim, double* coord)
{
    dimensions= dim; parent=0;
    this->coordinates = new double[dim];
    for (int a = 0; a < this->dimensions; ++a)
      this->coordinates[a] = coord[a];
};

OctreeElement::~OctreeElement(){
    delete[] this->coordinates;
}

int
OctreeElement::getType(){
  return OCTREE_ELEMENT;
};

double* 
OctreeElement::getCoordinates(){
    return this->coordinates;
};

int
OctreeElement::getDimensions(){
    return this->dimensions;
};

OctreeElement*
OctreeElement::getParent(){
    return this->parent;                       
};

void
OctreeElement::setParent(OctreeCell* _parent){
    this->parent = _parent;
}

void
OctreeElement::printCoordinates(){
    ::std::cout << "element coordinates: ";
    for (int a=0; a < this->dimensions; ++a)
        ::std::cout << this->coordinates[a] << ", ";
    ::std::cout << '\n';
}

void
OctreeElement::setCoordinates(double* _coordinates){
    for (int a = 0; a < this->dimensions; ++a){
        this->coordinates[a] = _coordinates[a];
    }
}


////////////////////////////////////////////////////////////////////////////////
/////////// Octree Point ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

OctreePoint::OctreePoint(int dim, double* coord, int ident): 
                             OctreeElement(dim, coord),
                             identifier(ident),
                             pointsReported(0),
                             pointsContained(1){}

OctreeElement*
OctreePoint::getSkipTreeCopy(){
    return this;
};

int
OctreePoint::getType(){
  return OCTREE_POINT;
};

void
OctreePoint::markReported(){
  assert(this->pointsReported==0);
  this->pointsReported=1;
}  

void
OctreePoint::unmarkReported(){
  this->pointsReported=0;
}

bool 
OctreePoint::isReported(){
  return (this->pointsReported>0); 
}

bool
OctreePoint::isContained(double* _boxCoords, double _boxHalfSideLength){
  for (int i=0; i<this->dimensions; i++){
    if(this->coordinates[i] < _boxCoords[i]- _boxHalfSideLength ||
      this->coordinates[i] >= _boxCoords[i] + _boxHalfSideLength) return false;
  }
  return true;
}

inline 
void
OctreePoint::increaseValue(){
  this->pointsContained++;
}

int
OctreePoint::boxedRangeQueryCounting(double* _boxCoords, 
    double _boxHalfSideLength, double _boxHalfSideLengthError){
  int report=this->pointsContained-this->pointsReported;
  if (this->pointsReported>0) return 0;
  if (!this->isContained(_boxCoords, _boxHalfSideLength)) return 0;
//  this->pointsReported=this->pointsContained;
//  report=this->parent->markReported(report);
#ifdef SHOW_DEBUG
  printf("Reporting point %d\n", this->identifier);
#endif
  return this->pointsContained;
  return report;
}

int 
OctreePoint::exactRangeQuery2DimSteps(double* _coords, 
    double _radius, double _radiusError){
  int i, report;
  double distance1, distance2;
  double distance, radiussquared;
#ifdef SHOW_DEBUG
  printf("Querying point %d looking for distance %d\n", this->identifier, 
      _radius);
#endif
  assert(this->dimensions%2==0);
  
  // square the radius once
  radiussquared = _radius * _radius;
  for(i=0; i<this->dimensions; i+=2){
    distance1 = _coords[i]-this->coordinates[i];
    distance2 = _coords[i+1]-this->coordinates[i+1];
    if (distance1>_radius || distance2>_radius){
#ifdef SHOW_DEBUG
      printf("Querying point %d looking for distance %d\n", this->identifier, 
          _radius);
      printf("distance %d too long, ", distance);
      printf(" error in timestep %d, ", i);
      printf("point coordinates: (%d, %d), ", this->coordinates[i], 
          this->coordinates[i+1]);
      printf("circle coordinates: (%d, %d)\n", _coords[i], _coords[i+1]);
#endif      
      return 0;
    }
    distance =(distance1*distance1)+(distance2*distance2);
    if (distance > radiussquared){
#ifdef SHOW_DEBUG
      printf("Querying point %d looking for distance %d\n", this->identifier, 
          _radius);
      printf("distance %d too long, ", distance);
      printf(" error in timestep %d, ", i);
      printf("point coordinates: (%d, %d), ", this->coordinates[i], 
          this->coordinates[i+1]);
      printf("circle coordinates: (%d, %d)\n", _coords[i], _coords[i+1]);
#endif      
      return 0;
    } 
  }
#ifdef SHOW_DEBUG
  printf("Reporting point %d\n", this->identifier);
#endif
  report=this->pointsContained-this->pointsReported;
//  report=this->parent->markReported(report);
  return report;
  
};


////////////////////////////////////////////////////////////////////////////////
/////////// Octree Cell ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


OctreeCell::OctreeCell(int _dimensions, 
    double* _coordinates, double _halfSideLength)
                           : OctreeElement(_dimensions, _coordinates){
    this->halfSideLength = _halfSideLength;
    this->pointsContained=0;
    this->pointsReported=0;
    this->parent=0;
    this->lowerTree=0;
    this->upperTree=0;
    this->content = new ::std::map<unsigned long long, OctreeElement*>();
};

OctreeCell::~OctreeCell(){
    // free the map's contents
#ifdef SHOW_DEBUG
    printf("deleting square...\n");
#endif
    int deletePoints = 0, deleteSquares = 0;
    std::map<unsigned long long, OctreeElement*>::iterator mapIterator;
    for (mapIterator = this->content->begin(); mapIterator!=
      this->content->end(); mapIterator++){
        if (mapIterator->second->getType() == OCTREE_POINT) deletePoints++;
        if (mapIterator->second->getType() == OCTREE_CELL){ 
          deleteSquares++;
          if(mapIterator->second !=0) delete mapIterator->second;
        } 
    }
    this->content->clear();
    assert (this->content->empty());
    delete this->content;
#ifdef SHOW_DEBUG
    printf("found %d points ...\n", deletePoints);
    printf("deleted %d squares ...\n", deleteSquares);
#endif
};

int
OctreeCell::getType(){
  return OCTREE_CELL;
};

void
OctreeCell::deletePointData(){
    ::std::map<unsigned long long, OctreeElement*>::iterator mapIterator;
    ::std::vector<unsigned long long> keys;
#ifdef SHOW_DEBUG
    printf("deleting point data...\n");
#endif
    for (mapIterator = this->content->begin(); mapIterator!=
      this->content->end(); mapIterator++){
        if (mapIterator->second->getType() == OCTREE_POINT){
            keys.push_back(mapIterator->first);
            
        } else if (mapIterator->second->getType() == OCTREE_CELL){ 
            ((OctreeCell*)mapIterator->second)->deletePointData();
        } 
    }  
    ::std::vector<unsigned long long>::iterator vecIterator;
    OctreeElement* tmpPoint;
    for (vecIterator=keys.begin(); vecIterator!=keys.end(); vecIterator++){
      tmpPoint = (*this->content)[*vecIterator];
      
      //this->content->erase(*vecIterator);
      //delete tmpPoint;
    }
};

bool
OctreeCell::covers(OctreeElement* _element){
  // test for coordinates to be inside this cell!
  //XXX:taken out for the pruning step    
  //  assert(this->dimensions == element->getDimensions());
  double* elementCoords = _element->getCoordinates();
  for(int i = 0; i < this->dimensions; i++){
    if( this->coordinates[i] + this->halfSideLength - 
        elementCoords[i] + __epsilon <= 0 ||
        this->coordinates[i] - this->halfSideLength - 
        elementCoords[i] >  __epsilon){
#ifdef SHOW_DEBUG
  printf("does not cover in coordinate %d %4.18f %4.18f %4.18f %4.18f %4.18f\n"
          , i,
          this->coordinates[i],
          this->halfSideLength,
          elementCoords[i],
          this->coordinates[i] + this->halfSideLength - elementCoords[i],
          this->coordinates[i] - this->halfSideLength - elementCoords[i]
      );
#endif
      return false;
    }
  }
  if (_element->getType() == OCTREE_POINT) return true;

  // make sure we don't test for pure elements!
  assert(_element->getType() == OCTREE_CELL);
#ifdef SHOW_DEBUG
  ::std::cout << "testing coverage for cell!" << '\n';    
#endif    
  return (((OctreeCell*)_element)->getHalfSideLength() < this->halfSideLength);
}

void
OctreeCell::printCoordinates(){
  ::std::cout << "element coordinates: ";
  for (int a=0; a < this->dimensions; a++)
    ::std::cout << ::std::setiosflags(::std::ios::fixed) << 
    ::std::setprecision(2) << this->coordinates[a] << ", ";
    ::std::cout << "halfsidelength: " << 
    ::std::setiosflags(::std::ios::fixed) << 
    ::std::setprecision(4) <<this->halfSideLength;
    ::std::cout << " contains: " << this->pointsContained << '\n';
}

OctreeCell*
OctreeCell::largestInterestingSquare(OctreeElement* a, OctreeElement* b){
  
  double* coordsA = a->getCoordinates();
  double* coordsB = b->getCoordinates();
  double* curCoords= new double[this->dimensions];
  double curHSL;
  for(int i=0; i<this->dimensions; i++)
    curCoords[i]=this->coordinates[i];
  curHSL= this->halfSideLength;

  double* nextCoords= new double[this->dimensions];
  double nextHSL;
  bool traverseDown= true;
  double sign1, sign2;
  while(true)
  {
    for(int i = 0; i < this->dimensions; i++)
    {
      sign1 =  (coordsA[i] >= curCoords[i]) ? 1 : -1;
      sign2 =  (coordsB[i] >= curCoords[i]) ? 1 : -1;

      if (sign1 != sign2)
      {
        traverseDown=false;
        break;
      }
      nextHSL= curHSL / 2;
      nextCoords[i] = curCoords[i] + (sign1 * nextHSL);
    }
    if(!traverseDown)
      break;
    double* tmp= curCoords;
    curCoords=nextCoords;
    nextCoords=tmp;
    double tmp1= curHSL;
    curHSL= nextHSL;
    nextHSL=tmp1;
  }
  
  OctreeCell* newCell = new OctreeCell(this->dimensions, curCoords, curHSL);
  newCell->setParent(this);
  
  newCell->insert(a);
  newCell->insert(b);

  // do some cleanups
  delete[] curCoords;
  delete[] nextCoords;

  return newCell;      
}


void
OctreeCell::insert(OctreeElement* _element){
    // make sure we're not doing something wrong here
    if(!this->covers(_element)){
      if(_element->getType()==OCTREE_POINT){
        printf("Failed with point %d.\n", 
          ((OctreePoint*)_element)->getIdentifier());
        this->printCoordinates();
        _element->printCoordinates();
      }
      assert(false);
    }
    
    // increment point counter if this is one
    if (_element->getType() == OCTREE_POINT) this->pointsContained++;
    else if (_element->getType() == OCTREE_CELL) {
      this->pointsContained+=((OctreeCell*)_element)->getPointsContained();
    }
      
    // find id of cell this element belongs in.
    unsigned long long cellid = 0;
    double* elementCoords = _element->getCoordinates();
    for (int a=0; a < this->dimensions; a++){
        if (elementCoords[a] >= this->coordinates[a]) {
            cellid++;
        }
        if (a < this->dimensions -1) cellid <<= 1;
    }
//    cellid >>= 1;   
#ifdef SHOW_DEBUG
    ::std::cout << "Trying to insert element at " << cellid << '\n';    
    ::std::cout << "Type is " << _element->getType() << '\n';    
#endif    
    if (this->content->count(cellid) == 0){
       (*this->content)[cellid] = _element;
       assert(_element!=0);
       _element->setParent(this);
#ifdef SHOW_DEBUG
    ::std::cout << "Inserted into empty field " << cellid << '\n';    
#endif    
       return;
    }
    
    // if already filled, look for type
    if ((*this->content)[cellid]->getType() == OCTREE_CELL){
        if (((OctreeCell*)(*this->content)[cellid])->covers(_element)){
#ifdef SHOW_DEBUG
  ::std::cout << 
    "Trying to insert in underlying interesting square" << '\n';    
#endif    
            ((OctreeCell*)(*this->content)[cellid])->insert(_element);
        } else {
#ifdef SHOW_DEBUG
    ::std::cout << "Trying to find common largest "
        "interesting square (with cell)" << '\n';    
#endif    
            (*this->content)[cellid] = 
                largestInterestingSquare((*this->content)[cellid], _element);
            assert(this->content->count(cellid) > 0);
#ifdef SHOW_DEBUG
    ::std::cout << "Now holding " << this->content->count(cellid) << '\n';    
#endif    
        }
    } else if ((*this->content)[cellid]->getType() == OCTREE_POINT){
#ifdef SHOW_DEBUG
    ::std::cout << "Trying to find common largest "
        "interesting square (with point)" << '\n';    
    ::std::cout << "ID old point: " << 
    ((OctreePoint*)(*this->content)[cellid])->getIdentifier() << '\n';    
    if (_element->getType() == 0)
        ::std::cout << "ID new point: " << 
        ((OctreePoint*)_element)->getIdentifier() << '\n';    
#endif  
        // make sure this is no double point
        double* oldCoords=((*this->content)[cellid])->getCoordinates();
        bool doublePoint=true;
        for (int a=0; a < this->dimensions; ++a){
          if (oldCoords[a]!=elementCoords[a]){
            doublePoint=false;
            break;
          }
        }
        if (doublePoint){
          ((OctreePoint*)(*this->content)[cellid])->increaseValue();
          return;
        }
        // create interesting square (holding both points)
        (*this->content)[cellid] = 
            largestInterestingSquare((*this->content)[cellid], _element);
        assert(this->content->count(cellid) > 0);
#ifdef SHOW_DEBUG
    ::std::cout << "Now holding " << this->content->count(cellid) << '\n';    
#endif    
    }

};

OctreeElement*
OctreeCell::getSkipTreeCopy(){
    OctreeCell* newCell = new OctreeCell(this->dimensions, this->coordinates, 
        this->halfSideLength);
    this->upperTree = newCell;
    newCell->lowerTree = this;
    
    //copy the maps contents!
    std::map<unsigned long long, OctreeElement*>::iterator mapIterator;
    for (mapIterator = this->content->begin(); mapIterator!=
      this->content->end();
        mapIterator++){
        newCell->insert(mapIterator->second->getSkipTreeCopy());
    }

    newCell->pointsContained = this->pointsContained;
    
    return newCell;
}

OctreeElement*
OctreeCell::find(double* _coordinates){
#ifdef SHOW_DEBUG
    ::std::cout << "Searching in ";
    printCoordinates();
#endif    

    // find id of cell we want to take a look at
    int cellid = 0;
    for (int a=0; a < this->dimensions; ++a){
        if (_coordinates[a] >= this->coordinates[a]) {
            cellid++;
        }
        if (a < this->dimensions -1) cellid <<= 1;
    }
//    cellid >>= 1;   
#ifdef SHOW_DEBUG
    ::std::cout << "Trying to find element at " << cellid << '\n';    
#endif    

    if(this->content->count(cellid) == 0){
        //check, if we got upper tree!
        if (this->upperTree != 0){
#ifdef SHOW_DEBUG
    ::std::cout << "Continuing search in upper tree.\n";    
#endif
            return this->upperTree->find(_coordinates);
        } else {
#ifdef SHOW_DEBUG
    ::std::cout << "Returning this, there's no smaller object." << '\n';    
#endif    
            return this;
        }
    } else if((*this->content)[cellid]->getType() == OCTREE_CELL){
#ifdef SHOW_DEBUG
    ::std::cout << "Continuing search in underlying square." << '\n';    
#endif
        return ((OctreeCell*)(*this->content)[cellid])->find(_coordinates);
    } else if((*this->content)[cellid]->getType() == OCTREE_POINT){
        //compare coordinates, to see if we got an exact match!
        double* pointCoords = (*this->content)[cellid]->getCoordinates();
        for(int i=0; i < this->dimensions; ++i){
            if(_coordinates[i] != pointCoords[i]){
#ifdef SHOW_DEBUG
    ::std::cout << "Found unmatching point." << '\n';    
#endif    
                if (this->upperTree != 0){
#ifdef SHOW_DEBUG
    ::std::cout << "Continuing search in upper tree.\n";    
#endif
                    return this->upperTree->find(_coordinates);
                }
#ifdef SHOW_DEBUG
    ::std::cout << "Returning this square (no upper tree).\n";    
#endif
                return this;
            }
        }
#ifdef SHOW_DEBUG
    ::std::cout << "Found matching point with id ";
    ::std::cout <<
    ((OctreePoint*)(*this->content)[cellid])->getIdentifier() <<'\n';    
#endif    
        return (*this->content)[cellid];
    }
    
    // should never be reached
    assert(false);
};

int
OctreeCell::markReported(int amount){ 
  int possible = this->pointsContained - this->pointsReported;
  int fromTop;
  if (this->parent!=0) {
    fromTop = this->parent->markReported(amount);
    if (fromTop < possible) possible = fromTop;
  }
  if (amount>possible) amount=possible;
  assert(this->pointsContained >= this->pointsReported+amount);
  this->pointsReported += amount;
  return amount;
};

void
OctreeCell::unmarkReported(){
//  printf("unmarking cell ...\n");
  if (this->pointsReported == 0) return;
  // cycle through children and unmark their points!
  ::std::map<unsigned long long, OctreeElement*>::iterator mapIterator;
//  printf("%d elements to check on.\n", this->content->size());
  int i=0;
  for (mapIterator=this->content->begin(); mapIterator!=this->content->end();
    mapIterator++){
    if (mapIterator->second==0) {
//    printf("SKIPPING ONE at index %d, %d checked before.\n", 
//      mapIterator->first, i);
      continue;
    }
    i++;
    //assert(mapIterator->second != 0);
    mapIterator->second->unmarkReported();
  }
//  printf("checked on %d\n", i);
//  if (i < this->content->size()) printf("INCONSISTENCY!!!");
  this->pointsReported=0;
  return;
};

int
OctreeCell::boxedRangeQueryCounting(double* _boxCoords, 
    double _boxHalfSideLength, double _boxHalfSideLengthError){
  mpf_class *volBig, *volSmall, *volTmp;  
  int report=0;
  OctreeCell* tmpCell=0, *successor=0;

#ifdef SHOW_DEBUG
  printf("box query in new cell: \n");
  this->printCoordinates();
#endif

  // check, if we can step some skips!
  if (this->halfSideLength > _boxHalfSideLength*2){    
#ifdef SHOW_DEBUG
printf(">>>>> Checking for skips .....");    
#endif
    unsigned long long cellid0 = 0, cellid1 = 0;
    for (int a=0; a < this->dimensions; a++){
      if (_boxCoords[a]-_boxHalfSideLength >= this->coordinates[a]) {
          cellid0++;
      }
      if (_boxCoords[a]+_boxHalfSideLength >= this->coordinates[a]) {
          cellid1++;
      }
      if (a < this->dimensions -1) {
        cellid0 <<= 1;        
        cellid1 <<= 1;        
      }
    }
    if (cellid0==cellid1){
      //check, if there is a cell in that region!
      if (this->content->count(cellid0) > 0){
#ifdef SHOW_DEBUG
printf(" skipping <<<<<<<<<<<<<<\n");
#endif
        return (*this->content)[cellid0]->boxedRangeQueryCounting(_boxCoords,
          _boxHalfSideLength, _boxHalfSideLengthError);
      } else {
        return 0;
      }
    }
#ifdef SHOW_DEBUG
printf(" NOT possible <<<<<<<<<<<<<<\n");
#endif
  }
  volBig = this->getBoxIntersection(_boxCoords, _boxHalfSideLengthError);
  if (volBig == 0) volBig= new mpf_class(0);
  volSmall = this->getBoxIntersection(_boxCoords, _boxHalfSideLength);
  if (volSmall == 0) volSmall= new mpf_class(0);

#ifdef SHOW_DEBUG
  //::std::cout <<
  //"calculated intersections, "<<*volBig<<" and "<<*volSmall<<'\n';
#endif

  // if intersection with inner inner box is null, this cell is out
  if (*volSmall==0){
    delete volSmall;
    delete volBig;
    return 0;
  } else if (*(volTmp=hsl2vol(this->halfSideLength, this->dimensions)) 
      <= *volBig){
  // if intersection with outer box equals the volume, than box is inside
#ifdef SHOW_DEBUG
    ::std::cout << "THIS BOX IS IN!\n";
#endif
    report = pointsContained - pointsReported;
// XXX: killed the marking here!
    //this->pointsReported = this->pointsContained;
    // propagade points reported (report) upwards
    //if (this->parent!=0) report = this->parent->markReported(report);
    // report all so far unreported points!
    delete volBig;
    delete volSmall;
    delete volTmp;
    return report;
  }

#ifdef SHOW_DEBUG
  printf("THIS BOX IS STABBING!\n");
#endif

  // this cell is stabbing!
  if (this->isCritical(_boxCoords, _boxHalfSideLength, volSmall, &successor))
  {   
    // check all underlying squares!

#ifdef SHOW_DEBUG
printf("i am critical! checking underlying squares!\n");
#endif
    ::std::map<unsigned long long, OctreeElement*>::iterator mapIterator;
    for (mapIterator=this->content->begin(); mapIterator!=this->content->end();
      mapIterator++){
        report += mapIterator->second->boxedRangeQueryCounting(_boxCoords, 
          _boxHalfSideLength, _boxHalfSideLengthError);
      }
  } else {
    // search for critical square and report the points we find under that
    // (highly recursive?!)
#ifdef SHOW_DEBUG
printf("i am NOT critical - better look for someone who is!\n");
#endif
    if(this->lowerTree==0){
      tmpCell=successor->findCriticalSquare__up(_boxCoords, _boxHalfSideLength, 
        volSmall);
    } else {
      tmpCell = this->lowerTree->findCriticalSquare__down(_boxCoords, 
        _boxHalfSideLength, volSmall);
    }
    // just in case the underlying square was critical already!
    if (tmpCell!=0){
      report = tmpCell->boxedRangeQueryCounting(_boxCoords, _boxHalfSideLength, 
        _boxHalfSideLengthError);
    } else {
      report = successor->findCriticalSquare__up(_boxCoords, 
          _boxHalfSideLength, volSmall)->boxedRangeQueryCounting(_boxCoords, 
              _boxHalfSideLength, _boxHalfSideLengthError);
    }
  }
  
#ifdef SHOW_DEBUG
printf("returning from box query in lower cell\n\n");
#endif 
  delete volBig;
  delete volSmall;
  return report; 
};

bool 
OctreeCell::isCritical(double* _boxCoords, double _boxHalfSideLength, 
    mpf_class* vol, OctreeCell **successor){
  // if we don't know the intersection, calculate it!
  assert(vol!=0);

//  if (vol==0){
//    vol = this->getBoxIntersection(boxCoords, boxHalfSideLength);
//  }

  // cycle through children an compare their volume
  // check all underlying squares!
  ::std::map<unsigned long long, OctreeElement*>::iterator mapIterator;
  for (mapIterator=this->content->begin(); mapIterator!=this->content->end();
    mapIterator++){
    OctreeElement* test = mapIterator->second;
    if (test->getType() == OCTREE_CELL){
      mpf_class* newVol = 
        ((OctreeCell*)mapIterator->second)->getBoxIntersection(  _boxCoords, 
            _boxHalfSideLength);
      if (newVol==0) {
        continue;
      }
      if (*newVol==*vol) {
        if (successor!=0) *successor = (OctreeCell*)mapIterator->second;
        delete newVol;
        return false;
      }
      delete newVol;
    }
  }
  return true;   
};


mpf_class* 
OctreeCell::getBoxIntersection(double* _boxCoords, double _boxHalfSideLength){
  mpf_class* vol = new mpf_class(1);
  double sideSmall;
  // calculate intersection with box!
  for (int i=0; i<this->dimensions; i++){
    // get the length for each side of the intersecting rectangles
    sideSmall = min(this->coordinates[i] + this->halfSideLength,
      _boxCoords[i] + _boxHalfSideLength) - max(this->coordinates[i] - 
      this->halfSideLength, _boxCoords[i] - _boxHalfSideLength);
    if (sideSmall<=0) {
      delete vol;
      return 0;
    }
    *vol = *vol * sideSmall;
  }
  return vol;
};


OctreeCell*
OctreeCell::findCriticalSquare__down(double* _boxCoords, 
    double _boxHalfSideLength, mpf_class* _vol){
  OctreeCell* successor, *tmpCell=0;

  if (this->isCritical(_boxCoords, _boxHalfSideLength, _vol, &successor)){
    return 0;
  } else {
    if (this->lowerTree!=0){
      tmpCell = this->lowerTree->findCriticalSquare__down(_boxCoords, 
        _boxHalfSideLength, _vol);
    }
    if (tmpCell!=0){
      return tmpCell;
    } else {
      return successor->findCriticalSquare__up(_boxCoords, _boxHalfSideLength, 
        _vol);
    }
  }
}

OctreeCell*
OctreeCell::findCriticalSquare__up(double* _boxCoords, 
    double _boxHalfSideLength,  mpf_class* _vol){
  OctreeCell* successor;
  if (this->isCritical(_boxCoords, _boxHalfSideLength, _vol, &successor)){
    // this one is critical, so work on upper tree, if there is one! otherwise
    // this is the final result
    if (this->upperTree==0) return this;
    return this->upperTree->findCriticalSquare__up(_boxCoords, 
        _boxHalfSideLength, _vol);
  } else {
    return successor->findCriticalSquare__up(_boxCoords, 
        _boxHalfSideLength, _vol);
  }
};
  
int 
OctreeCell::exactRangeQuery2DimSteps(double* _coords, 
    double _radius, double _radiusError){        
  unsigned long long cellid0 = 0, cellid1 = 0;
  int a;
 
  double distsqrd, radiussqrd;
  int i, report=0;
  bool intersects=true, includes=true;
  
 
  // make sure this is somewhat possible
  assert(this->dimensions%2==0);


  // check, if we can step some skips!
  if (this->halfSideLength > _radius*2){    
#ifdef SHOW_DEBUG
printf(">>>>> Checking for skips .....");    
#endif
    for (a=0; a < this->dimensions; a++){
      if (_coords[a]-_radius >= this->coordinates[a]) {
          cellid0++;
      }
      if (_coords[a]+_radius >= this->coordinates[a]) {
          cellid1++;
      }
      if (a < this->dimensions -1) {
        cellid0 <<= 1;        
        cellid1 <<= 1;        
      }
    }
    if (cellid0==cellid1){
      //check, if there is a cell in that region!
      if (this->content->count(cellid0) > 0){
#ifdef SHOW_DEBUG
printf(" skipping <<<<<<<<<<<<<<\n");
#endif
        return (*this->content)[cellid0]->exactRangeQuery2DimSteps(_coords, 
          _radius, _radiusError);
      } else {
        return 0;
      }
    }
#ifdef SHOW_DEBUG
printf(" NOT possible <<<<<<<<<<<<<<\n");
#endif
  }

 
  // check, if box in included in the outer circle (==included)
  radiussqrd = _radiusError*_radiusError;
  for (i=0; i<this->dimensions; i+=2){

    distsqrd = (this->coordinates[i] + halfSideLength - _coords[i])*
      (this->coordinates[i] + halfSideLength - _coords[i])+
      (this->coordinates[i+1] - halfSideLength - _coords[i+1])*
      (this->coordinates[i+1] - halfSideLength - _coords[i+1]);
    if (distsqrd>radiussqrd) includes=false;
    distsqrd = (this->coordinates[i] + halfSideLength - _coords[i])*
      (this->coordinates[i] + halfSideLength - _coords[i])+
      (this->coordinates[i+1] + halfSideLength - _coords[i+1])*
      (this->coordinates[i+1] + halfSideLength - _coords[i+1]);
    if (distsqrd>radiussqrd) includes=false;
    distsqrd = (this->coordinates[i] - halfSideLength - _coords[i])*
      (this->coordinates[i] - halfSideLength - _coords[i])+
      (this->coordinates[i+1] - halfSideLength - _coords[i+1])*
      (this->coordinates[i+1] - halfSideLength - _coords[i+1]);
    if (distsqrd>radiussqrd) includes=false;
    distsqrd = (this->coordinates[i] - halfSideLength - _coords[i])*
      (this->coordinates[i] - halfSideLength - _coords[i])+
      (this->coordinates[i+1] + halfSideLength - _coords[i+1])*
      (this->coordinates[i+1] + halfSideLength - _coords[i+1]);
    if (distsqrd>radiussqrd) includes=false;
    if (includes==false) break;
  }

  // if this square is included return all points
  if (includes){
#ifdef SHOW_DEBUG
    printf("this square is included completely");
#endif
    i = this->pointsContained - this->pointsReported;  
// XXX: killed marking here!      
    //if (this->parent!=0) i = markReported(i);
    //this->pointsReported = this->pointsContained;
    return i;
  }

  
  // check if box is intersecting with the inner circle (==stabbing)
  radiussqrd = _radius*_radius;
  for (i=0; i<this->dimensions; i+=2){

    // get distance from center to center, if center of box is in the circle...
    distsqrd=
      (this->coordinates[i]-_coords[i])*(this->coordinates[i]-_coords[i])+
      (this->coordinates[i+1]-_coords[i+1])*
          (this->coordinates[i+1]-_coords[i+1]);
    if (distsqrd<=(radiussqrd)) continue;
    
    // look for intersections of sides
    // first dimension!
    if (this->coordinates[i]-this->halfSideLength < _coords[i] &&
      this->coordinates[i]+this->halfSideLength > _coords[i]){
      if (this->coordinates[i+1]-halfSideLength-_coords[i+1]<_radius) 
        continue;
      if (_coords[i+1]-(this->coordinates[i+1]+halfSideLength)<_radius) 
        continue;
    }
    // second dimension!
    if (this->coordinates[i+1]-this->halfSideLength < _coords[i+1] &&
      this->coordinates[i+1]+this->halfSideLength > _coords[i+1]){
      if (this->coordinates[i]-halfSideLength-_coords[i]<_radius) continue;
      if (_coords[i]-(this->coordinates[i]+halfSideLength)<_radius) continue;
    }

    distsqrd = (this->coordinates[i] + halfSideLength - _coords[i])*
      (this->coordinates[i] + halfSideLength - _coords[i])+
      (this->coordinates[i+1] - halfSideLength - _coords[i+1])*
      (this->coordinates[i+1] - halfSideLength - _coords[i+1]);
    if (distsqrd>radiussqrd) continue;
    distsqrd = (this->coordinates[i] + halfSideLength - _coords[i])*
      (this->coordinates[i] + halfSideLength - _coords[i])+
      (this->coordinates[i+1] + halfSideLength - _coords[i+1])*
      (this->coordinates[i+1] + halfSideLength - _coords[i+1]);
    if (distsqrd>radiussqrd) continue;
    distsqrd = (this->coordinates[i] - halfSideLength - _coords[i])*
      (this->coordinates[i] - halfSideLength - _coords[i])+
      (this->coordinates[i+1] - halfSideLength - _coords[i+1])*
      (this->coordinates[i+1] - halfSideLength - _coords[i+1]);
    if (distsqrd>radiussqrd) continue;
    distsqrd = (this->coordinates[i] - halfSideLength - _coords[i])*
      (this->coordinates[i] - halfSideLength - _coords[i])+
      (this->coordinates[i+1] + halfSideLength - _coords[i+1])*
      (this->coordinates[i+1] + halfSideLength - _coords[i+1]);
    if (distsqrd>radiussqrd) continue;
        
    intersects=false;
    break;
  }


  // if we have no intersection (and box is not in!), this box is out!
  if (!intersects) {
#ifdef SHOW_DEBUG
    printf("Square does NOT intersect the circle...\n");
#endif
    return 0;
  }  
  
int oldreport;
//  this->printCoordinates();
#ifdef SHOW_DEBUG
  printf("Square does intersect the cirle... checking children\n");
#endif
  // check all children!
  ::std::map<unsigned long long, OctreeElement*>::iterator mapIterator;
  for (mapIterator=this->content->begin(); mapIterator!=this->content->end();
    mapIterator++){
 //     OctreeElement* test = mapIterator->second;

//      if (test==0){
//        printf("\nfound a null pointer ... \n");
//        break;
//      }
  oldreport = report;
  report += mapIterator->second->exactRangeQuery2DimSteps(_coords, _radius,
        _radiusError);
      oldreport = report - oldreport;
  }  
  return report;
}

void
OctreeCell::printAll(){
    std::map<unsigned long long, OctreeElement*>::iterator mapIterator;
    printf("Printing out CELL data:\n");
    this->printCoordinates();
    printf("Holds:\n");
    for(mapIterator=this->content->begin(); mapIterator!=this->content->end(); 
        mapIterator++){
        OctreeElement* test = mapIterator->second;
        if (test->getType()==OCTREE_CELL){
            printf("going down into another cell:\n");
            ((OctreeCell*)test)->printAll();
            printf("going up into another cell:\n");
        } else if (test->getType()==OCTREE_POINT){
            printf("POINT with id %d: ", ((OctreePoint*)test)->getIdentifier());
            test->printCoordinates();
        }
    }


};
  
#endif // OCTREE_ELEMENTS_SOURCE
