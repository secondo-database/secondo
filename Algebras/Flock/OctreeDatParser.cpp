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

#ifndef OCTREE_PARSER_SOURCE
#define OCTREE_PARSER_SOURCE


#include <fstream>
#include <assert.h>
#include <iostream>
#include <vector>
#include <map>
#include <cstdlib>
#include <cmath>

#include "OctreeDatParser.h"
#include "OctreeElements.h"

//#define SHOW_DEBUG 1

OctreeDatParser::OctreeDatParser(char* inputFileName, int startCol, int endCol)
{
  this->activeTrajectories=0;
#ifdef SHOW_DEBUG
    ::std::cout << "About to parse the file: " << inputFileName << '\n';
    ::std::cout << "Start Column: " << startCol << "\tEnd Column: " << endCol 
    << '\n';
#endif 
  // set up the vars
  this->tmpArray = new char[16];
  int index = 0;
  this->timesteps = endCol-startCol;
  this->inputFile.open(inputFileName, ios::in);

  // parse the general tree specs
  while (inputFile.get(this->tmpArray[index])){
    if (this->tmpArray[index]=='\n' || this->tmpArray[index]==':'){
      this->tmpArray[index]='\0';
#ifdef SHOW_DEBUG
    ::std::cout << "found string: " << this->tmpArray << '\n';    
#endif    
      if (strcmp(this->tmpArray, "dimensions") == 0) {
#ifdef SHOW_DEBUG
    ::std::cout << "setting dimensions in parser" << '\n';    
#endif    
        this->dimensions = this->getNextNumber();
      } else if (strcmp(this->tmpArray, "extent") == 0) {
#ifdef SHOW_DEBUG
    ::std::cout << "setting extent in parser" << '\n';    
#endif  
        this->extent= new double[this->dimensions * 2];
        for(int i=0; i< this->dimensions * 2 ; ++i)
          this->extent[i] = this->getNextDouble();
//      }  
//      else if (strcmp(this->tmpArray, "timesteps") == 0) {
//#ifdef SHOW_DEBUG
//    ::std::cout << "setting timesteps in parser" << '\n';    
//#endif    
 //       this->timesteps = this->getNextNumber();
      } else if (strcmp(this->tmpArray, "details") == 0) {
#ifdef SHOW_DEBUG
    ::std::cout << "turning to details in parser" << '\n';    
#endif    
        assert(this->dimensions > 0);
        assert(this->timesteps > 0);
        break;
      }
      index=-1;
    }
    index++;
  }

  this->pointData = new vector<OctreePoint*>();
#ifdef SHOW_DEBUG
    ::std::cout << "parsing point information" << '\n';    
#endif
  index=0;
  // parse the point information and insert to the tree!
  while (inputFile.get(this->tmpArray[index])){
    if (this->tmpArray[index]=='\n' || this->tmpArray[index]==':'){
      this->tmpArray[index]='\0';
#ifdef SHOW_DEBUG
    ::std::cout << "found string: " << this->tmpArray << '\n';    
#endif    
      if (strcmp(this->tmpArray, "obj") == 0) {
#ifdef SHOW_DEBUG
    ::std::cout << "adding point to list" << '\n';    
#endif    
        OctreePoint* tmp = this->createPoint(startCol, endCol);
        if(tmp != 0) this->pointData->push_back(tmp);
#ifdef SHOW_DEBUG
    ::std::cout << "Coordinates: " ; tmp->printCoordinates();    
#endif    
      }
      index=-1;
    }
  index++;
  }
  
#ifdef SHOW_DEBUG
  printf("pointData contains %d points.\n", pointData->size());
#endif
  this->inputFile.close();
};

OctreeDatParser::OctreeDatParser(char* inputFileName){

#ifdef SHOW_DEBUG
    ::std::cout << "About to parse the file: " << inputFileName << '\n';    
#endif 
  // set up the vars
  this->tmpArray = new char[16];
  int index = 0;
  
  this->inputFile.open(inputFileName, ios::in);

  // parse the general tree specs
  while (inputFile.get(this->tmpArray[index])){
    if (this->tmpArray[index]=='\n' || this->tmpArray[index]==':'){
      this->tmpArray[index]='\0';
#ifdef SHOW_DEBUG
    ::std::cout << "found string: " << this->tmpArray << '\n';    
#endif    
      if (strcmp(this->tmpArray, "dimensions") == 0) {
#ifdef SHOW_DEBUG
    ::std::cout << "setting dimensions in parser" << '\n';    
#endif    
        this->dimensions = this->getNextNumber();
      } else if (strcmp(this->tmpArray, "extent") == 0) {
#ifdef SHOW_DEBUG
    ::std::cout << "setting extent in parser" << '\n';    
#endif  
        this->extent= new double[this->dimensions * 2];
        for(int i=0; i< this->dimensions * 2 ; ++i)
          this->extent[i] = this->getNextDouble();
      }  else if (strcmp(this->tmpArray, "timesteps") == 0) {
#ifdef SHOW_DEBUG
    ::std::cout << "setting timesteps in parser" << '\n';    
#endif    
        this->timesteps = this->getNextNumber();
      } else if (strcmp(this->tmpArray, "details") == 0) {
#ifdef SHOW_DEBUG
    ::std::cout << "turning to details in parser" << '\n';    
#endif    
        assert(this->dimensions > 0);
        assert(this->timesteps > 0);
        break;
      }
      index=-1;
    }
    index++;
  }

  this->pointData = new vector<OctreePoint*>();
#ifdef SHOW_DEBUG
    ::std::cout << "parsing point information" << '\n';    
#endif
  index=0;
  // parse the point information and insert to the tree!
  while (inputFile.get(this->tmpArray[index])){
    if (this->tmpArray[index]=='\n' || this->tmpArray[index]==':'){
      this->tmpArray[index]='\0';
#ifdef SHOW_DEBUG
    ::std::cout << "found string: " << this->tmpArray << '\n';    
#endif    
      if (strcmp(this->tmpArray, "obj") == 0) {
#ifdef SHOW_DEBUG
    ::std::cout << "adding point to list" << '\n';    
#endif    
        OctreePoint* tmp = this->createPoint();
        this->pointData->push_back(tmp);
#ifdef SHOW_DEBUG
    ::std::cout << "Coordinates: " ; tmp->printCoordinates();    
#endif    
      }
      index=-1;
    }
  index++;
  }
  
#ifdef SHOW_DEBUG
  printf("pointData contains %d points.\n", pointData->size());
#endif
  this->inputFile.close();
};

OctreeDatParser::~OctreeDatParser(){
  // delete all points in the pointData set
  ::std::vector<OctreePoint*>::iterator pointIterator;
  for(pointIterator=this->pointData->begin(); pointIterator!=
    this->pointData->end(); pointIterator++){    
    OctreePoint* point = *pointIterator;
    delete point;
  }
  delete this->pointData;
  delete[] this->tmpArray;
  delete[] this->extent;
};


::std::vector<OctreePoint*>* 
OctreeDatParser::getPointset(){
  return this->pointData;
}



::std::map<int, OctreeCell*>* 
OctreeDatParser::getSkiptree4Dim(){
  int tmpTimesteps = this->timesteps;
  this->timesteps = 4;
  ::std::map<int, OctreeCell*>* skiptree = 
    this->getSkiptree();
  this->timesteps = tmpTimesteps;
  return skiptree;
}

::std::map<int, OctreeCell*>* 
OctreeDatParser::getSkiptreeFromSet(::std::vector<OctreePoint*>* pointSet, 
  int pointDimensions){
  ::std::vector<OctreePoint*>* tmpPointData = this->pointData;
  this->pointData = pointSet;
  ::std::map<int, OctreeCell*>* skiptree = 
    this->getSkiptree();
  this->pointData = tmpPointData;
  return skiptree;
}  

double OctreeDatParser::calcHalfSideLength()
{
  double max=0, diff=0;
  for(int i= 0; i<this->dimensions; ++i)
  {
    diff= abs(extent[i] - extent[i+this->dimensions]);
    max= (diff > max) ? diff: max;
  }
  return max;
}

void OctreeDatParser::calcSkipTreeCoords(double* res)
{
  for(int i= 0; i<this->timesteps; ++i)
  {
    for(int k= 0; k< this->dimensions; ++k)
      res[k + i* this->dimensions]= 
        (this->extent[k] + this->extent[k + this->dimensions])/2;
  }
}

::std::map<int, OctreeCell*>* 
OctreeDatParser::getSkiptree(){
  
  assert(this->pointData->size() > 0);
  
  // init the tree data
  double* treeCoords = new double[this->dimensions*this->timesteps];
  double halfSideLength = this->calcHalfSideLength();
  this->calcSkipTreeCoords(treeCoords);

  // init random numbers.
  srand(time(NULL));
  ::std::map<int, ::std::vector<OctreePoint*>*> skipTreeSets;
  vector<OctreePoint*>::iterator pointIterator;
  
  int nextIndex = 0;
  
  for(pointIterator=this->pointData->begin(); pointIterator!=
    this->pointData->end(); pointIterator++){    
    for (int index=0;; index++){
      if(rand()%2 == 0){
        // init all new sets, if there were none!
        while(nextIndex <= index){
          ::std::vector<OctreePoint*> *testvec = 
            new ::std::vector<OctreePoint*>();
           skipTreeSets[nextIndex++] = testvec;
        }
        skipTreeSets[index]->push_back(*pointIterator);
        break;
      }
    }
  }

#ifdef SHOW_DEBUG
  printf("Number of sets is %d, numbers in each set are:\n", nextIndex);
  for(int i=0; i<nextIndex; i++)
    printf("Set %d contains %d points\n", i, skipTreeSets[i]->size());
#endif

#ifdef SHOW_DEBUG
    ::std::cout << "initializing octrees" << '\n';    
#endif 
    

  // init the last tree
  ::std::map<int, OctreeCell*>* treeMap = new ::std::map<int, OctreeCell*>();
  (*treeMap)[nextIndex-1] = new OctreeCell(this->dimensions*this->timesteps, 
    treeCoords, halfSideLength);

  // insert points from last set
  for(pointIterator=skipTreeSets[nextIndex-1]->begin(); 
    pointIterator!=skipTreeSets[nextIndex-1]->end(); pointIterator++){
    (*treeMap)[nextIndex-1]->insert(*pointIterator);
  }
#ifdef SHOW_DEBUG
    printf("Inserted %d points into skiptree no. %d\n", 
      (*treeMap)[nextIndex-1]->getPointsContained(), nextIndex-1);
#endif    
  
  for(int i=nextIndex-2; i>=0; i--){
    (*treeMap)[i] = (OctreeCell*)(*treeMap)[i+1]->getSkipTreeCopy();

    OctreeCell* tmpCell = (*treeMap)[i];
    for(pointIterator=skipTreeSets[i]->begin(); pointIterator!=
      skipTreeSets[i]->end(); pointIterator++){
      tmpCell->insert(*pointIterator);
    }
    //delete the list here
    delete skipTreeSets[i];
#ifdef SHOW_DEBUG
    printf("Inserted %d points into skiptree no. %d\n", 
      tmpCell->getPointsContained(), i);
#endif    
  }
  // do some cleanups
  delete[] treeCoords;
    
#ifdef SHOW_DEBUG
    ::std::cout << "handing over new octree" << '\n';    
#endif  
  return treeMap;
};


OctreePoint* 
OctreeDatParser::createPoint(){
  OctreePoint *newPoint;
  double *pointCoords = new double[this->dimensions*this->timesteps];
  int ident = getNextNumber();

  for (int index=0; index < this->dimensions*this->timesteps; index++){
    pointCoords[index] = this->getNextDouble();
  }
  
  newPoint = new OctreePoint(this->dimensions*this->timesteps, 
    pointCoords, ident);
  delete[] pointCoords;
  return newPoint;
};

OctreePoint* 
OctreeDatParser::createPoint(int startCol, int endCol){
  //bool debugme=true;
  OctreePoint *newPoint=0;
  int ident = getNextNumber();
  int cnt = getNextNumber();
  if(cnt < endCol +1 ) return newPoint;
  this->activeTrajectories++;
  //skip the coordinates before startCol 
  double tmp;
  bool defined;
  for (int index=0; index < this->dimensions * startCol; index++)  
    this->getNextCoord(tmp);

  //read the specified part of the trajectory  
  assert( (endCol-startCol) > 0);  
  double *pointCoords = new double[(endCol-startCol)* this->dimensions];  
  for (int index=0; index < this->dimensions * (endCol - startCol ) ; ++index)
  {
    defined = this->getNextCoord(tmp);
    if(defined) pointCoords[index]= tmp; else break;
  }  
  if(defined)
    newPoint = new OctreePoint(this->dimensions * this->timesteps, 
        pointCoords, ident);  
  delete[] pointCoords;
  return newPoint;
};

OctreeCell*
OctreeDatParser::getOctree(){
  // init the tree data
  double* treeCoords = new double[this->dimensions*this->timesteps];
  double halfSideLength = this->calcHalfSideLength();
  this->calcSkipTreeCoords(treeCoords);
  
  // init the tree
  OctreeCell* octree = new OctreeCell(this->dimensions*this->timesteps, 
    treeCoords, halfSideLength);
    
  // insert the points
  ::vector<OctreePoint*>::iterator pointIterator = this->pointData->begin();
  for(; pointIterator!=this->pointData->end(); pointIterator++){
    octree->insert(*pointIterator);
  }
  
  delete[] treeCoords;
  return octree;
};


int
OctreeDatParser::getNextNumber(){
  char* tmp = new char[20];
  int index=0;
  
  while (inputFile.get(tmp[index]))
  {
    if (tmp[index]=='\n' || tmp[index]==':')
      break;
    ++index;
  }
    
  tmp[index]='\0';
  int res = atoi (tmp);
  delete[] tmp;
  return res;
}

bool
OctreeDatParser::getNextCoord(double& res){
  char* tmp = new char[20];
  int index=0; 
  while (inputFile.get(tmp[index]))
  {
    if (tmp[index]=='\n' || tmp[index]==':')
      break;
    ++index;
      
  }
  tmp[index]='\0';
  if(strcmp(tmp, "undef") == 0 )
  {
    delete[] tmp;
    return false;
  }
  res= atof(tmp);
  delete[] tmp;
  return true;
}

double
OctreeDatParser::getNextDouble(){
  char* tmp = new char[20];
  int index=0;
  
  while (inputFile.get(tmp[index]))
  {
    if (tmp[index]=='\n' || tmp[index]==':')
      break;
    ++index;
      
  }
    
  tmp[index]='\0';
  double res= atof(tmp);
  delete[] tmp;
  return res;
}

int 
OctreeDatParser::activeTrajectoriesCount()
{
  return this->activeTrajectories;
}


#endif // OCTREE_PARSER_SOURCE
