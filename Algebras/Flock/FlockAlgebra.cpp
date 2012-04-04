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

[1] Source File of the Flock Algebra

June, 2009 Mahmoud Sakr

[TOC]

1 Overview

This source file essentially contains the necessary implementations for
evaluating the spatiotemporal pattern predicates (STP).

2 Defines and includes

*/
#include "FlockAlgebra.h"
#include "Symbols.h"

namespace FLOCK{

bool
Helpers::string2int(char* digit, int& result)
{
  result = 0;
  while (*digit >= '0' && *digit <='9') {
    result = (result * 10) + (*digit - '0');
    digit++;
  }
  return (*digit == 0); // true if at end of string!
}

string
Helpers::ToString( int number )
{
  ostringstream o;
  o << number ; //<< char(0)
  return o.str();
}

string
Helpers::ToString( double number )
{
  ostringstream o;
  o << number ; //<< char(0)
  return o.str();
}

Flock::Flock(double* _coords, int _coordsCount):pointsCount(0)
{
  this->coordsCount= _coordsCount;
  for(int i=0; i< this->coordsCount; ++i)
    this->coordinates[i]= _coords[i];
  this->SetDefined(true);
}

Flock::Flock(const Flock& arg){
  // copy coordinates to be save from object deletion
  this->SetDefined(arg.IsDefined());
  for (int a = 0; a < arg.coordsCount; ++a){
    this->coordinates[a] = arg.coordinates[a];
  }
  for (int a = 0; a < arg.pointsCount; ++a){
    this->points[a] = arg.points[a];
  }
  this->coordsCount= arg.coordsCount;
  this->pointsCount = arg.pointsCount;
}

Flock::~Flock(){
}

size_t
Flock::HashValue() const
{
  int res=0;
  if(IsDefined())
  {
    for(int pIt = 0; pIt < this->pointsCount; ++pIt)
      res = res ^ points[pIt];
    res= res ^ this->pointsCount;
    res= res ^ this->coordsCount;
    return res;
  }
  return 0;
}

void
Flock::CopyFrom(const Attribute* rhs)
{
  const Flock * arg= dynamic_cast<const Flock*>(rhs);
  this->SetDefined(arg->IsDefined());
  for (int a = 0; a < arg->coordsCount; ++a){
    this->coordinates[a] = arg->coordinates[a];
  }
  for (int a = 0; a < arg->pointsCount; ++a){
    this->points[a] = arg->points[a];
  }
  this->coordsCount= arg->coordsCount;
  this->pointsCount = arg->pointsCount;
}

int
Flock::Compare( const Attribute* rhs ) const
{
  return Attribute::GenericCompare<Flock>( this,
           dynamic_cast<Flock*>(const_cast<Attribute*>(rhs)),
           this->IsDefined(), rhs->IsDefined() );
}

bool
Flock::operator==(const Flock& rhs) const
{
  // this->coordinates playes no role in the equality comparison. In the
  // ReportFlock algorithm, this->coordinates is set to the first point
  // that enters the Flock.
  if (! (this->coordsCount == rhs.coordsCount &&
           this->pointsCount == rhs.pointsCount
        )
     )return false;

  for(int i=0; i<this->pointsCount ; ++i)
    if(this->points[i] != rhs.points[i])
      return false;
  return true;
}

bool
Flock::operator<(const Flock& rhs) const
{
  // We have no formal definition for comparing Flocks. This function is
  // provided only to fulfill the inerface requirements.
  return this->pointsCount < rhs.pointsCount;
}

bool
Flock::IsSubset(const Flock& rhs) const
{
  // this->coordinates playes no role in the equality comparison. In the
  // ReportFlock algorithm, this->coordinates is set to the first point
  // that enters the Flock.
  if (! (this->coordsCount == rhs.coordsCount &&
           this->pointsCount <= rhs.pointsCount
        )
     )return false;
  int l=0, r=0;
  while(l < this->pointsCount && r < rhs.pointsCount)
  {
    if(this->points[l] < rhs.points[r]) return false;
    else if (this->points[l] == rhs.points[r]) {++l; ++r;}
    else ++r;
  }
  return (l == this->pointsCount);
}

int
Flock::Intersection(Flock* arg, Flock* res)
{
  ::std::vector<int> resVec(maxFlockSize);
  ::std::vector<int>::iterator last;
  last= std::set_intersection(this->points, this->points + this->pointsCount,
                        arg->points, arg->points + arg->pointsCount,
                        resVec.begin());

  res->pointsCount = last - resVec.begin();
  std::copy(resVec.begin(), ++last, res->points);
  return res->pointsCount;
}

int
Flock::IntersectionCount(Flock* arg)
{
  ::std::vector<int> resVec(maxFlockSize);
  ::std::vector<int>::iterator last= resVec.begin();
  last= std::set_intersection(this->points, this->points + this->pointsCount,
                        arg->points, arg->points + arg->pointsCount,
                        last);

  return last - resVec.begin();
}

Points* Flock::Flock2Points(Instant& curTime, vector<int>* ids,
       vector<MPoint*>*sourceMPoints)
{
  bool debugme=true;
  Points* res= new Points(this->pointsCount);
  MPoint* curMPoint;
  Point curPoint(0, 0);
  Intime<Point> pointIntime(curTime, curPoint);
  vector<int>::iterator idsIt;
  unsigned int pos;
  for(int i=0; i<this->pointsCount; ++i)
  {
    for(pos=0; pos<ids->size(); ++pos)
      if((*ids)[pos] == this->points[i]) break;
    if(pos == ids->size())
    {
      cerr<<"Error! Flock::GetPoints. "
          "The MPoint with ID= "<< this->points[i] <<" is not found in the "
          "sourceMPoints.";
      return 0;
    }
    curMPoint= (*sourceMPoints)[pos];
    curMPoint->AtInstant(curTime, pointIntime);
    assert(pointIntime.IsDefined());
    (*res) += pointIntime.value;
  }
  return res;
}


ostream&
Flock::Print( ostream &os ) const
{
  if (! this->IsDefined()) return (os << "UnDefined Flock");
  string prnt= "";
  prnt+= "\nFlock size:" + this->pointsCount;
  prnt+= "\nFlock coordinates: ";
  for(int i=0; i< this->coordsCount ; ++i)
    prnt+= FLOCK::Helpers::ToString(this->coordinates[i]) + ", ";

  prnt+= "\nRegistered Flock point ids: " ;
  for (int pointIt=0 ;pointIt < this->pointsCount; ++pointIt)
      prnt+= FLOCK::Helpers::ToString(this->points[pointIt]) + ", ";
  prnt+="\n";

  return (os << prnt);
}

bool
Flock::IsDefined() const
{
  return this->defined;
}

void
Flock::SetDefined(const bool def)
{
  this->defined= def;
}

Attribute* Flock::Clone() const
{
  return new Flock(*this);
}

int
Flock::addPoint(OctreePoint* point){
  if(this->pointsCount >= maxFlockSize )
  {
    cerr<<"Flock size overflow. For efficient implementation we allow for "
        "flock sizes up to"  <<maxFlockSize;
    throw;
  }
  int i= this->pointsCount-1;
  while(i >= 0)
  {
    if(this->points[i] <= point->getIdentifier())
      break;
    i--;
  }
  if(this->points[i] == point->getIdentifier())
    return -1; //point already exists

  std::memmove(this->points + ((i+2) * sizeof(int)),
               this->points + ((i+1) * sizeof(int)),
               (this->pointsCount - i - 1) * sizeof(int));
  this->points[i+1] = point->getIdentifier();
  ++this->pointsCount;
  return 1; //point added successfully
}

int
Flock::addPointID(int point){
  bool debugme=false;
  if(this->pointsCount >= maxFlockSize )
  {
    cerr<<"Flock size overflow. For efficient implementation we allow for "
        "flock sizes up to"  <<maxFlockSize;
    throw;
  }
  int i= this->pointsCount-1;
  while(i >= 0)
  {
    if(this->points[i] <= point)
      break;
    --i;
  }
  if(this->points[i] == point)
    return -1; //point already exists

  if(debugme)
  {
    cout<<"\nBefore Addition \n";
    this->printPoints();
    cout<<"\nAdding pointID \n"<< point;
  }
  int newpos= i+2, // ) * sizeof(int)),
  oldpos = i+1, // * sizeof(int)),
  numbytes= (this->pointsCount - i - 1) * sizeof(int);
  std::memmove(this->points + newpos,
               this->points + oldpos,
               numbytes);
  this->points[i+1] = point;
  ++this->pointsCount;
  if(debugme)
  {
    this->printPoints();
    cout<<"\nAddition done \n";
  }
  return 1; //point added successfully
}

int
Flock::getPoints(int*& res){
  res = this->points;
  return this->pointsCount;
}

void
Flock::printPoints(){
  if (this->points == 0) return;
  printf("registered flock points: %d, ids are ",this->pointsCount);
  for (int pointIt= 0 ;pointIt < this->pointsCount; ++pointIt){
    printf("%d : ", this->points[pointIt]);
  }
  printf("\n");
};

void
Flock::printCoordinates(){
  printf("flock details: %d points",this->pointsCount);
  for (int a=0; a < this->coordsCount; a++)
    printf(", %f", this->coordinates[a]);
  printf("\n");
};

// type name used in Secondo:
const string Flock::BasicType()
{
  return "flock";
}
const bool Flock::checkType(const ListExpr type){
  return listutils::isSymbol(type, BasicType());
}



bool MFlock::CanAdd(Flock* arg)
{
  if(this->GetNoComponents()== 0) return true;
  UFlock lastUnit;
  this->Get(this->GetNoComponents()-1, lastUnit);
  return (lastUnit.constValue.Compare( arg ) == 0);
}
void MFlock::MFlockMergeAdd(UFlock& unit)
{
/*
This is a modified copy from Mappring::MergeAdd. We use this function, instead
of the standard implementation, to overcome some unclear errors in the
ReportFlocks algorithm, that are possibly due to floating point numerical
errors. According to many tests, the final results are correct when we use this
function.

*/
  UFlock lastunit;
  UFlock u1transfer;
  int size = units.Size();
  if ( !unit.IsValid() )
  {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
    << " MFlockMergeAdd(Unit): Unit is invalid:";
    unit.Print(cout); cout << endl;
    assert( false );
  }

  if (size > 0) {
    units.Get( size - 1, u1transfer );
    lastunit = u1transfer;
    if (lastunit.EqualValue(unit) &&
/*
The following condition is modified to be "<=" rather than "==" in the standard
implementation.

*/
        (lastunit.timeInterval.end >= unit.timeInterval.start)) {
      lastunit.timeInterval.end = unit.timeInterval.end;
      lastunit.timeInterval.rc = unit.timeInterval.rc;
      if ( !lastunit.IsValid() )
      {
        cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
        << "\nMFlockMergeAdd(Unit): lastunit is invalid:";
        lastunit.Print(cout); cout << endl;
        assert( false );
      }
      units.Put(size - 1, lastunit);
    }
    else {
      units.Append( unit );
    }
  }
  else {
    units.Append( unit );
  }
}
/*
3 Flock Reporting Functions

*/


void FinalizeLastUnit(::std::vector<MFlock*>* mflocks,
    Instant timeStep, int k)
{
  bool debugme=true;
  ::std::vector<MFlock*>::iterator mflockIt;
  MFlock* mflock;
  UFlock lastUnit;
  UFlock extended(true);
  for(mflockIt= mflocks->begin(); mflockIt!= mflocks->end(); ++mflockIt)
  {
    mflock= *mflockIt;
    if((*mflockIt)->finalized) continue;
    mflock->Get(mflock->GetNoComponents()-1, lastUnit);
    extended.CopyFrom(&lastUnit);
    if(debugme)
    {
       cout<<endl<<"Finalizing MFlock "<<extended.constValue.points[0];
       cout<<endl<<"Last unit before finalization :"; extended.Print(cout);
       cout.flush();
    }
    extended.timeInterval.end += timeStep * (k-1);
    mflock->Put(mflock->GetNoComponents()-1, extended);
    (*mflockIt)->finalized=true;
    if(debugme)
    {
       cout<<endl<<"Last unit after finalization :"; extended.Print(cout);
       cout.flush();
    }
  }
}

::std::vector<MFlock*>*
findFlocks(char* filename, double radius, double tolerance, int flocksize,
    Instant starttime, Instant timeStep, int k, short method)
{
  bool debugme=true;
  bool hasMoreData=true, consumed=false;
  int startCol=0;
  OctreeDatParser* myParser;
  ::std::vector<MFlock*>* mflocks= new ::std::vector<MFlock*>(0);
  ::std::vector<MFlock*>::iterator mflockIt;
  ::std::vector<MFlock*>* mflocksNotUpdated= new ::std::vector<MFlock*>(0);
  ::std::vector<Flock*>* flocks;
  ::std::vector<Flock*>::iterator flockIt;
  Instant curtime(starttime);
  UFlock uflock(true);
  uflock.timeInterval.lc=true;
  uflock.timeInterval.rc=false;
  MFlock* mflock;
  myParser = new OctreeDatParser(filename, startCol, startCol + k);
  hasMoreData= (myParser->activeTrajectoriesCount() >= flocksize);
  while(hasMoreData)
  {
    if(debugme)
    {
      cout<<endl<<"------------------------------------------------";
      cout<<endl<<"New iteration for reporting flocks startCol="<< startCol<<
      ", endCol= "<< startCol+k;
      cout<<endl<<"StartTime= "; curtime.Print(cout);
      cout<<" EndTime= "; (curtime + (timeStep * k)).Print(cout);
      cout<<endl<<"Number of points =" << myParser->getPointCount();
      cout.flush();
    }
    if(myParser->getPointCount() >= flocksize)
    {
      switch(method)
      {
      case _2Dim:
        flocks= findFlocks2Dim(myParser, radius, tolerance, flocksize);
        break;
      case _BruteForce:
        flocks= findFlocks2DimBruteforce(myParser, radius, flocksize);
        break;
      case _Square:
        flocks= findFlocksSkiptreeSquare(myParser, radius,
            tolerance, flocksize);
        break;
      case _Pruning:
        flocks= findFlocksSkiptreeSquareWithPruning(myParser, radius,
            tolerance, flocksize);
        break;
      }
      if(debugme)
      {
        cout<<endl<<"Found "<< flocks->size() << " flocks";
        cout.flush();
      }

      uflock.timeInterval.end= curtime + timeStep;
      uflock.timeInterval.start= curtime;

      for(mflockIt= mflocks->begin(); mflockIt != mflocks->end(); ++mflockIt)
      {
        for(flockIt= flocks->begin(); flockIt != flocks->end(); ++flockIt)
        {
          if((*mflockIt)->CanAdd(*flockIt))
          {
            uflock.constValue.CopyFrom(*flockIt);
            (*mflockIt)->MFlockMergeAdd(uflock);
            (*mflockIt)->finalized= false;
            if(debugme)
            {
              cout<<endl<<"Flock "<< (*flockIt)->points[0]<<" joined MFlock ";
              UFlock tmp1;
              (*mflockIt)->Get(0,tmp1);
              cout<<tmp1.constValue.points[0];
              cout.flush();
            }
            consumed=true;
            break;
          }
        }
        if(consumed)  {flocks->erase(flockIt); consumed=false;}
        else          {mflocksNotUpdated->push_back(*mflockIt);}
      }
/*
Flocks that are not merged with any of the existing MFlocks are used to create
new MFlocks

*/
      for(flockIt= flocks->begin(); flockIt != flocks->end(); ++flockIt)
      {
        uflock.constValue.CopyFrom(*flockIt);
        mflock= new MFlock(0);
        mflock->Add(uflock);
        mflock->finalized= false;
        mflocks->push_back(mflock);
        if(debugme)
        {
          cout<<endl<<"Creating new MFlock ";
          cout<<(*flockIt)->points[0];
          cout.flush();
        }
      }
/*
MFlocks that are not updated (i.e. got no new units), need to be finalized
(i.e. the last unit is prolonged k-1 time steps).

*/

      FinalizeLastUnit(mflocksNotUpdated, timeStep, k);
      for(flockIt=flocks->begin(); flockIt!=flocks->end(); flockIt++)
        delete (*flockIt);
      flocks->clear();
      delete flocks;
      mflocksNotUpdated->clear();
      //need to be tested. The destructors should not be called
    }
    ++startCol;
    curtime += timeStep;
    delete myParser;
    myParser = new OctreeDatParser(filename, startCol, startCol + k);
    hasMoreData= (myParser->activeTrajectoriesCount() >= flocksize);
  }
  delete myParser;
  delete mflocksNotUpdated;
  if(debugme)
    cout<<endl<<"returning from FindFlocks. "<<mflocks->size()<<" flocks found";
  return mflocks;
}

::std::vector<Flock*>*
findFlocks2Dim(OctreeDatParser* myParser,
    double radius, double tolerance, int flocksize){
  bool debugme=false;
  printf("Starting the flock search:\n");
  if(myParser->getPointCount() < flocksize)
    return 0;
  time_t first, second, third, before, after;
  Flock *flock;
  before= time(NULL);
  first = time(NULL);

  OctreeCell* tree = myParser->getOctree();
  second = time(NULL);
  ::std::vector<OctreePoint*>* points = myParser->getPointset();
  ::std::vector<Flock*>* flocks = new ::std::vector<Flock*>();

  ::std::vector<OctreePoint*>::iterator pointIt=points->begin();
  ::std::vector<Flock*>::iterator flockIt;
  int dimensions = (*pointIt)->getDimensions();
  int amount, i, queries=0;
  double *pCoords, *fCoords;
  bool inFlock=false;
  double distance, radiussqrd=radius*radius;

  for(; pointIt!=points->end(); pointIt++){
    pCoords=(*pointIt)->getCoordinates();
    //check against all flocks to make sure this point is not in one already
    for(flockIt=flocks->begin(); flockIt!=flocks->end(); flockIt++){
      fCoords=(*flockIt)->getCoordinates();
      if(debugme)
      {
        cout<<endl<<"pCoords =";
        for(i=0; i<dimensions; ++i)
          cout<< pCoords[i]<< "\t";
        cout<<endl<<"fCoords =";
        for(i=0; i<dimensions; ++i)
          cout<< fCoords[i]<< "\t";
        cout.flush();
      }
      inFlock=true;
      // calculate distance
      for(i=0; i<dimensions; i+=2){
        distance=0;
        distance+=(pCoords[i]-fCoords[i])*(pCoords[i]-fCoords[i]);
        distance+=(pCoords[i+1]-fCoords[i+1])*(pCoords[i+1]-fCoords[i+1]);
        if (distance>radiussqrd){
          inFlock=false;
          break;
        }
      }
      if(inFlock){
        break;
      }
    }
    // if this point is in a flock already, go to the next point
    if(inFlock){
      (*flockIt)->addPoint(*pointIt);
      continue;
    }

    // this point should be in no flock found so far. query it.
    tree->unmarkReported();
    queries++;

    amount = tree->exactRangeQuery2DimSteps(pCoords, radius, tolerance);
    if(debugme)
    {
      cout<<endl<<"Query point: ";
      for(i=0; i<dimensions; i++)
        cout<<pCoords[i]<<"\t";
      cout<<endl<<"Points in its range = "<<amount;
      cout.flush();
    }
    if(amount>=flocksize){
      flock=new Flock(pCoords, dimensions);
      flock->addPoint(*pointIt);
      flocks->push_back(flock);
    }
    //break; // only do the first point plz
  }

  third = time(NULL);

  printf("Flock details (from exact query):\n");

  for(flockIt=flocks->begin(); flockIt!=flocks->end(); flockIt++){
    (*flockIt)->printPoints();

///////////////////////////////////////////////////////////////////////
//// comment in, if you wanna have flock points reported! /////////////
///////////////////////////////////////////////////////////////////////

//    // only report on "unusual" points
//      ::std::vector<OctreePoint*>* fpoints = (*flockIt)->getPoints();
//      pointIt = points->begin();
//      fpoints->clear();
//      printf("checking against all points...\n");
//      fCoords = (*flockIt)->getCoordinates();
//      for(;pointIt != points->end(); pointIt++){
//        pCoords = (*pointIt)->getCoordinates();
//        inFlock=true;
//        // calculate distance
//        for(i=0; i<dimensions; i+=2){
//          distance=0;
//          distance+=(pCoords[i]-fCoords[i])*(pCoords[i]-fCoords[i]);
//          distance+=(pCoords[i+1]-fCoords[i+1])*(pCoords[i+1]-fCoords[i+1]);
//          if (distance>radiussqrd){
//            inFlock=false;
//            break;
//          }
//        }
//        if(inFlock){
//          fpoints->push_back(*pointIt);
//        }
//      }
//    (*flockIt)->printPoints();

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
  }
  printf("Quadtree contains %d points in %d dimensions,\n",
    tree->getPointsContained(), dimensions);
  printf("radius was %f, tolerance was %f.\n", radius, tolerance);
  printf("Found a total of %d flocks.\n", flocks->size());
  printf("Building the quadtree took %d seconds.\n", (int)(second-first));
  printf("Performing %d queries took %d seconds.\n", queries,
      (int)(third-second));

  delete tree;
  after= time(NULL);
  printf("\nProcessing the complete trajectory took %d seconds.\n",
      (int)(after-before));
  return flocks;
};



::std::vector<Flock*>*
findFlocks2DimBruteforce(OctreeDatParser* myParser, double radius,
    int flocksize){
  printf("Starting the flock search:\n");
  if(myParser->getPointCount() < flocksize)
    return 0;
  time_t first, second, third, before, after;
  Flock *flock;
  before= time(NULL);

  first = time(NULL);
  second = time(NULL);
  ::std::vector<OctreePoint*>* points = myParser->getPointset();
  ::std::vector<Flock*>* flocks = new ::std::vector<Flock*>();

  ::std::vector<OctreePoint*>::iterator pointIt=points->begin(), compIt;
  ::std::vector<Flock*>::iterator flockIt;
  int dimensions = (*pointIt)->getDimensions();
  int amount, i, queries=0;
  double *pCoords, *fCoords, *cCoords;
  bool inFlock;
  double distance, radiussqrd=radius*radius;

  for(; pointIt!=points->end(); pointIt++){
    pCoords=(*pointIt)->getCoordinates();
    //check against all flocks to make sure this point is not in one already
    for(flockIt=flocks->begin(); flockIt!=flocks->end(); flockIt++){
      fCoords=(*flockIt)->getCoordinates();
      inFlock=true;
      // calculate distance
      for(i=0; i<dimensions; i+=2){
        distance=0;
        distance+=(pCoords[i]-fCoords[i])*(pCoords[i]-fCoords[i]);
        distance+=(pCoords[i+1]-fCoords[i+1])*(pCoords[i+1]-fCoords[i+1]);
        if (distance>radiussqrd){
          inFlock=false;
          break;
        }
      }
      if(inFlock){
        break;
        inFlock=false;
      }
    }
    // if this point is in a flock already, go to the next point
    if(inFlock){
      continue;
    }

    // this point should be in no flock found so far. Check against all other
    // points
    queries++;
    amount=0;
    for(compIt=points->begin(); compIt!=points->end(); compIt++){
      cCoords=(*compIt)->getCoordinates();
      // calculate distance
      // misusing inFlock here as in reach!
      inFlock=true;
      for(i=0; i<dimensions; i+=2){
        distance=0;
        distance+=(pCoords[i]-cCoords[i])*(pCoords[i]-cCoords[i]);
        distance+=(pCoords[i+1]-cCoords[i+1])*(pCoords[i+1]-cCoords[i+1]);
        if (distance>radiussqrd){
          inFlock=false;
          break;
        }
      }
      if (inFlock) amount++;
    }

    if(amount>=flocksize){
      flock=new Flock(pCoords, dimensions);
      flocks->push_back(flock);
    }
  }

  third = time(NULL);

  printf("Flock details (from brute force):\n");

  for(flockIt=flocks->begin(); flockIt!=flocks->end(); flockIt++){
    (*flockIt)->printPoints();
  }
  printf("Pointset contains %d points in %d dimensions,\n",
    points->size(), dimensions);
  printf("radius was %f.\n", radius);
  printf("Found a total of %d flocks.\n", flocks->size());
  printf("Building the quadtree took %d seconds.\n", (int)(second-first));
  printf("Performing %d queries took %d seconds.\n", queries,
      (int)(third-second));
  after= time(NULL);
  printf("\nProcessing the complete trajectory took %d seconds.\n",
      (int)(after-before));
  return flocks;
};


::std::vector<Flock*>*
findFlocksSkiptreeSquare(OctreeDatParser* myParser, double radius,
    double tolerance, int flocksize){
  printf("Starting the flock search:\n");
  if(myParser->getPointCount() < flocksize)
    return 0;
  time_t first, second, third, before, after;
  Flock *flock;
  before= time(NULL);
  first = time(NULL);

  ::std::map<int, OctreeCell*>* tree = myParser->getSkiptree();
  second = time(NULL);
  ::std::vector<OctreePoint*>* points = myParser->getPointset();
  ::std::vector<Flock*>* flocks = new ::std::vector<Flock*>();

  ::std::vector<OctreePoint*>::iterator pointIt=points->begin();
  ::std::vector<Flock*>::iterator flockIt;
  int dimensions = (*pointIt)->getDimensions();
  int amount, i, queries=0;
  double *pCoords, *fCoords;
  bool inFlock=false;
  double distance, radiussqrd=radius*radius;
  for(; pointIt!=points->end(); pointIt++){
    pCoords=(*pointIt)->getCoordinates();
    //check against all flocks to make sure this point is not in one already
    for(flockIt=flocks->begin(); flockIt!=flocks->end(); flockIt++){
      fCoords=(*flockIt)->getCoordinates();
      inFlock=true;
      // calculate distance
      for(i=0; i<dimensions; i+=2){
        distance=0;
        distance+=(pCoords[i]-fCoords[i])*(pCoords[i]-fCoords[i]);
        distance+=(pCoords[i+1]-fCoords[i+1])*(pCoords[i+1]-fCoords[i+1]);
        if (distance>radiussqrd){
          inFlock=false;
          break;
        }
      }
      if(inFlock){
        (*flockIt)->addPoint(*pointIt);
        break;
        inFlock=false;
      }
    }
    // if this point is in a flock already, go to the next point
    if(inFlock){
      continue;
    }
    // this point should be in no flock found so far. query it.
    (*tree)[0]->unmarkReported();
    queries++;

    amount = (*tree)[0]->boxedRangeQueryCounting(pCoords, radius, tolerance);

    if(amount>=flocksize){
      flock=new Flock(pCoords, dimensions);
      flock->addPoint(*pointIt);
      flocks->push_back(flock);
    }
  }

  third = time(NULL);

  printf("Flock details (from box query):\n");
  for(flockIt=flocks->begin(); flockIt!=flocks->end(); flockIt++){
    (*flockIt)->printPoints();
  }
  printf("Quadtree contains %d points in %d dimensions,\n",
    (*tree)[0]->getPointsContained(), dimensions);
  printf("radius was %f, tolerance was %f.\n", radius, tolerance);
  printf("Found a total of %d flocks.\n", flocks->size());
  printf("Building the quadtree took %d seconds.\n", (int)(second-first));
  printf("Performing %d queries took %d seconds.\n", queries,
      (int)(third-second));
  deleteSkipQuadtree(tree);
  after= time(NULL);
  printf("\nProcessing the complete trajectory took %d seconds.\n",
      (int)(after-before));
  return flocks;
};

::std::vector<Flock*>*
findFlocksSkiptreeSquareWithPruning(OctreeDatParser* myParser,
    double radius, double tolerance, int flocksize){
  printf("Starting the flock search:\n");
  printf("pruning the set of ponts first...");
  if(myParser->getPointCount() < flocksize)
    return 0;
  time_t first, second, third, fourth, fifth, before, after;
  Flock *flock;
  before= time(NULL);
  first = time(NULL);

  ::std::map<int, OctreeCell*>* tree = myParser->getSkiptree4Dim();
  second = time(NULL);
  ::std::vector<OctreePoint*>* points = myParser->getPointset();
  ::std::vector<OctreePoint*>* prunedSet = new vector<OctreePoint*>();
  ::std::vector<Flock*>* flocks = new ::std::vector<Flock*>();

  ::std::vector<OctreePoint*>::iterator pointIt=points->begin();
  ::std::vector<Flock*>::iterator flockIt;
  int dimensions = 4;
  int dimensions2 = (*pointIt)->getDimensions();
  int amount, i, queries=0, queries2=0;
  double *pCoords, *fCoords;
  bool inFlock=false;
  double distance, radiussqrd=radius*radius;
  for(; pointIt!=points->end(); pointIt++){
    pCoords=(*pointIt)->getCoordinates();
    //check against all flocks to make sure this point is not in one already
    for(flockIt=flocks->begin(); flockIt!=flocks->end(); flockIt++){
      fCoords=(*flockIt)->getCoordinates();
      inFlock=true;
      // calculate distance
      for(i=0; i<dimensions; i+=2){
        distance=0;
        distance+=(pCoords[i]-fCoords[i])*(pCoords[i]-fCoords[i]);
        distance+=(pCoords[i+1]-fCoords[i+1])*(pCoords[i+1]-fCoords[i+1]);
        if (distance>radiussqrd){
          inFlock=false;
          break;
        }
      }
      if(inFlock){
        (*flockIt)->addPoint(*pointIt);
        prunedSet->push_back(*pointIt);
        break;
        inFlock=false;
      }
    }
    // if this point is in a flock already, go to the next point
    if(inFlock){
      continue;
    }

    // this point should be in no flock found so far. query it.
    (*tree)[0]->unmarkReported();
    queries++;
    amount = (*tree)[0]->boxedRangeQueryCounting(pCoords, radius, tolerance);
    if(amount>=flocksize){
      flock=new Flock(pCoords, dimensions);
      flock->addPoint(*pointIt);
      prunedSet->push_back(*pointIt);
      flocks->push_back(flock);
    }
  }

  third = time(NULL);
  // clean up
  deleteSkipQuadtree(tree);
  for(flockIt=flocks->begin(); flockIt!=flocks->end(); flockIt++){
    delete *flockIt;
  }
  flocks->clear();
  if (prunedSet->size()>0){
    tree = myParser->getSkiptreeFromSet(prunedSet, dimensions2);
    fourth = time(NULL);
    // get next run started
    fourth = time(NULL);
  inFlock = false; // FH added this one, fragged the first run otherwise!
    pointIt = prunedSet->begin();
    for(; pointIt!=prunedSet->end(); pointIt++){
      pCoords=(*pointIt)->getCoordinates();
      //check against all flocks to make sure this point is not in one already
      for(flockIt=flocks->begin(); flockIt!=flocks->end(); flockIt++){
        fCoords=(*flockIt)->getCoordinates();
        inFlock=true;
        // calculate distance
        for(i=0; i<dimensions2; i+=2){//TW+DM changed dimensions to dimensions2
          distance=0;
          distance+=(pCoords[i]-fCoords[i])*(pCoords[i]-fCoords[i]);
          distance+=(pCoords[i+1]-fCoords[i+1])*(pCoords[i+1]-fCoords[i+1]);
          if (distance>radiussqrd){
            inFlock=false;
            break;
          }
        }
        if(inFlock){
          (*flockIt)->addPoint(*pointIt);
          break;
          inFlock=false;
        }
      }
      // if this point is in a flock already, go to the next point
      if(inFlock){
        continue;
      }
      // this point should be in no flock found so far. query it.
      (*tree)[0]->unmarkReported();
      queries2++;

      amount = (*tree)[0]->boxedRangeQueryCounting(pCoords, radius, tolerance);
      if(amount>=flocksize){
        flock=new Flock(pCoords); // TW+DM changed dimensions to dimensions2
        flock->addPoint(*pointIt);
        flocks->push_back(flock);
      }
    }
  } else {
    fourth = time(NULL);
  }
  fifth = time(NULL);

  printf("Flock details (from box query with pruning):\n");
  for(flockIt=flocks->begin(); flockIt!=flocks->end(); flockIt++){
    (*flockIt)->printPoints();
  }
  printf("Quadtree contains %d points in %d dimensions,\n",
    (*tree)[0]->getPointsContained(), dimensions2);
  printf("radius was %f, tolerance was %f.\n", radius, tolerance);
  printf("Found a total of %d flocks.\n", flocks->size());
  printf("Building the pruning skip quadtree took %d seconds.\n",
      (int)(second-first));
  printf("Performing %d queries for pruning took %d seconds.\n", queries,
      (int)(third-second));
  printf("Building the final skip quadtree took %d seconds.\n",
      (int)(fourth-third));
  printf("Performing %d final queries took %d seconds.\n", queries2,
      (int)(fifth-fourth));
  printf("Complete time after first build of tree was %d.\n",
      (int)(fifth-second));

  deleteSkipQuadtree(tree);
  after= time(NULL);
  printf("\nProcessing the complete trajectory took %d seconds.\n",
      (int)(after-before));
  return flocks;
};



/*
4 Operators

*/

ListExpr ReportFlocksTM( ListExpr typeList )
{
  CHECK_COND(nl->ListLength(typeList) == 8 &&
      nl->IsAtom(nl->First(typeList)) &&
      (nl->SymbolValue(nl->First(typeList))== CcString::BasicType()) &&
      nl->IsAtom(nl->Second(typeList)) &&
      (nl->SymbolValue(nl->Second(typeList))== CcReal::BasicType())&&
      nl->IsAtom(nl->Third(typeList)) &&
      (nl->SymbolValue(nl->Third(typeList))== CcReal::BasicType())&&
      nl->IsAtom(nl->Fourth(typeList)) &&
      (nl->SymbolValue(nl->Fourth(typeList))== CcInt::BasicType())&&
      nl->IsAtom(nl->Fifth(typeList)) &&
      (nl->SymbolValue(nl->Fifth(typeList))== Instant::BasicType())&&
      nl->IsAtom(nl->Sixth(typeList)) &&
      (nl->SymbolValue(nl->Sixth(typeList))== Duration::BasicType())&&
      nl->IsAtom(nl->Nth(7, typeList)) &&
      (nl->SymbolValue(nl->Nth(7, typeList))== CcInt::BasicType())&&
      nl->IsAtom(nl->Nth(8, typeList)) &&
      (nl->SymbolValue(nl->Nth(8, typeList))== CcString::BasicType()),
      "reportflocks operator expects type map error"
      + nl->ToString(typeList))

      return (nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
          nl->SymbolAtom("mflock")));
}


int ReportFlocksVM(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  ::std::vector<MFlock*>* mflocks;
  switch (message)
  {
  case OPEN :{
    string fileName= static_cast<CcString*>(args[0].addr)->GetValue();
    double radius = static_cast<CcReal*>(args[1].addr)->GetRealval();
    double tolerance = static_cast<CcReal*>(args[2].addr)->GetRealval();
    int flockSize = static_cast<CcInt*>(args[3].addr)->GetIntval();
    Instant* startTime = static_cast<Instant*>(args[4].addr);
    Instant* timeStep = static_cast<Instant*>(args[5].addr);
    int k = static_cast<CcInt*>(args[6].addr)->GetIntval();
    string smethod= static_cast<CcString*>(args[7].addr)->GetValue();
    short method;
    if(smethod == "bruteforce") method= _BruteForce;
    else if(smethod == "square") method= _Square;
    else if(smethod == "exact") method= _2Dim;
    else if(smethod == "sqpruning") method= _Pruning;
    mflocks= findFlocks(const_cast<char*>(fileName.c_str()), radius, tolerance,
        flockSize, *startTime, *timeStep, k, method);
    local.setAddr(mflocks);
    return 0;
  }
  case REQUEST :{
    mflocks = static_cast< vector<MFlock*>* >(local.addr);
    MFlock* mflock;
    if (!mflocks->empty())
    {
      mflock= mflocks->back();
      mflocks->pop_back();
      result.setAddr(mflock);
      return YIELD;
    }
    else
    {
      return CANCEL;
    }
  }
  case CLOSE :{
    mflocks = static_cast< vector<MFlock*>* >(local.addr);
    delete mflocks;
    return 0;
  }
  default: return CANCEL;
  }
  return 0;
}



const string ReportFlocksSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>string x real x real x int x instant x duration x int x string -> "
  "stream(mflock)</text--->"
  "<text>reportflocks(fileName, radius, tolerance, flockSize, startTime, "
  "timeStep, flockDuration, method)</text--->"
  "<text>The operator gets its input from the (fileName), reports the flocks "
  "with parameters (radius, tolerance, flockSize, flockDuration), annotating"
  " the first column in the file with (startTime), and moving one (timeStep) "
  "for each further column. The computation uses the technique (method)."
  "</text--->"
  "<text>reportflocks(\"fileName\", 15.0, 16.0, 9, the_instant(.), "
  "create_duration(.), \"exact\")</text--->"
  ") )";

Operator reportflocks(
    "reportflocks",               // name
    ReportFlocksSpec,             // specification
    ReportFlocksVM,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    ReportFlocksTM          // type mapping
);



ListExpr
Flock::Out( ListExpr typeinfo, Word value )
{
  bool debugme=false;
  Flock* flock = static_cast<Flock*>(value.addr);
  if(!flock->IsDefined())
    return nl->SymbolAtom("undef");

  ListExpr lastPoint, points;
  points= lastPoint= nl->TheEmptyList();
  if(flock->pointsCount>0)
  {
    points = lastPoint = nl->OneElemList(nl->IntAtom(flock->points[0]));
    for(int i=1; i< flock->pointsCount; ++i)
      lastPoint = nl->Append(lastPoint,nl->IntAtom(flock->points[i]));
  }


  ListExpr lastCoord, coords;
  coords= lastCoord= nl->TheEmptyList();
  if(flock->coordsCount>0)
  {
    coords = lastCoord = nl->OneElemList(nl->RealAtom(flock->coordinates[0]));
    for(int i=1; i< flock->coordsCount; ++i)
      lastCoord = nl->Append(lastCoord,nl->RealAtom(flock->coordinates[i]));
  }

  ListExpr res= nl->FourElemList(
      nl->IntAtom(flock->coordsCount),
      nl->IntAtom(flock->pointsCount),
      coords,
      points);
  if(debugme)
    cout<< nl->ToString(res);
  return res;
}

Word
Flock::In( ListExpr typeInfo, ListExpr value,
         int errorPos, ListExpr& errorInfo, bool& correct )
{
  bool debugme=false;
  if(nl->IsEqual(value,"undef"))
      return SetWord(Address( 0 ));

  if (
      (! (nl->ListLength(value) == 4))                ||
      (!(nl->IsAtom(nl->First(value)) &&
          nl->AtomType(nl->First(value))==IntType))   ||
      (!(nl->IsAtom(nl->Second(value)) &&
          nl->AtomType(nl->Second(value))==IntType))
     )
  {
    correct= false;
    errorInfo= nl->StringAtom("Invalid flock nested list representation");
    return SetWord(Address(0));
  }
  Word res;
  res.addr= new Flock(true);
  Flock * flock= static_cast<Flock*>(res.addr);

  flock->coordsCount= nl->IntValue(nl->First(value));
  flock->pointsCount= nl->IntValue(nl->Second(value));

  ListExpr coords= nl->Third(value);
  ListExpr points= nl->Fourth(value);

  if(nl->ListLength(points) != flock->pointsCount)
  {
    correct= false;
    errorInfo= nl->StringAtom("Invalid flock nested list representation");
    return SetWord(Address(0));
  }

  if(flock->pointsCount > maxFlockSize)
  {
    correct= false;
    errorInfo= nl->StringAtom(" \nEncountered a flock of size " +
        Helpers::ToString(flock->pointsCount) +
        "For efficient implementation, flocks are limited to " +
        Helpers::ToString(maxFlockSize) );
    return SetWord(Address(0));
  }

  for(int i=0 ; i<flock->pointsCount; ++i)
  {
    flock->points[i]= nl->IntValue(nl->First(points));
    points= nl->Rest(points);
  }

  if(nl->ListLength(coords) != flock->coordsCount)
  {
    correct= false;
    errorInfo= nl->StringAtom("Invalid flock nested list representation");
    return SetWord(Address(0));
  }

  if(flock->coordsCount > maxCoordsSize)
  {
    correct= false;
    errorInfo= nl->StringAtom(" \nEncountered a flock coordinate list of size "
        + Helpers::ToString(flock->coordsCount) +
        "For efficient implementation, coordinates are limited to " +
        Helpers::ToString(maxCoordsSize) );
    return SetWord(Address(0));
  }

  for(int i=0 ; i<flock->coordsCount; ++i)
  {
    flock->coordinates[i]= nl->RealValue(nl->First(coords));
    coords= nl->Rest(coords);
  }

  if(debugme)
  {
    flock->Print(cout); cout<<endl; cout.flush();
  }
  correct= true;
  return res;
}

Word
Flock::Create( const ListExpr typeInfo )
{
  return (SetWord (new Flock()));
}

void
Flock::Delete( const ListExpr typeInfo, Word& w )
{
  delete static_cast<Flock*>(w.addr);
  w.addr= 0;
}

void
Flock::Close( const ListExpr typeInfo, Word& w )
{
  Flock::Delete(typeInfo, w);
}


Word
Flock::Clone( const ListExpr typeInfo, const Word& w )
{
  Flock* arg= static_cast<Flock*>(w.addr);
  Flock* res= static_cast<Flock*>(arg->Clone());
  return SetWord(res);
}

void*
Flock::Cast(void* addr)
{
  return (new (addr) Flock);
}

bool
Flock::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual(type, "flock"));
}

int
Flock::SizeOfObj()
{
  return sizeof(Flock);
}

ListExpr
Flock::Property()
{
  return (nl->TwoElemList(
             nl->FourElemList(nl->StringAtom("Signature"),
                              nl->StringAtom("Example Type List"),
                              nl->StringAtom("List Rep"),
                              nl->StringAtom("Example List")),
             nl->FourElemList(nl->StringAtom("-> FLOCK"),
                              nl->StringAtom("(flock) "),
                              nl->StringAtom("(size dimensions pointsCount "
                                  "coords points)"),
                              nl->TextAtom("(((i1 i2 FALSE FALSE) "
                                  "(2 2 3 (11.1 345.2) (122 524 300))) "
                                  "((i3 i4 TRUE FALSE) "
                                  "(2 2 2 (44.6 117.3) (12 24)))) "))));
}

template <class Alpha, Word (*InFun)( const ListExpr, const ListExpr,
                                      const int, ListExpr&, bool&     )>
Word InConstTemporalUnit2( const ListExpr typeInfo,
               const ListExpr instance,
               const int errorPos,
               ListExpr& errorInfo,
               bool& correct             )
{
  string errmsg;
  if( nl->ListLength( instance ) == 2 )
  {
    //1. deal with the time interval
    ListExpr first = nl->First( instance );

    if( nl->ListLength( first ) == 4 &&
              nl->IsAtom( nl->Third( first ) ) &&
        nl->AtomType( nl->Third( first ) ) == BoolType &&
        nl->IsAtom( nl->Fourth( first ) ) &&
        nl->AtomType( nl->Fourth( first ) ) == BoolType )
    {
      Instant *start =
        (Instant *)InInstant( nl->TheEmptyList(), nl->First( first ),
                              errorPos, errorInfo, correct ).addr;
      if( !correct )
      {
        errmsg = "InConstTemporalUnit2(): Error in first instant.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end =
        (Instant *)InInstant( nl->TheEmptyList(), nl->Second( first ),
                              errorPos, errorInfo, correct ).addr;
      if( !correct )
      {
        errmsg = "InConstTemporalUnit2(): Error in second instant.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        delete end;
        return SetWord( Address(0) );
      }
      // get closedness parameters
      bool lc = nl->BoolValue( nl->Third( first ) );
      bool rc = nl->BoolValue( nl->Fourth( first ) );

      Interval<Instant> tinterval( *start, *end, lc, rc );

      delete start;
      delete end;

      // check, wether interval is well defined
      correct = tinterval.IsValid();
      if ( !correct )
        {
          errmsg = "InConstTemporalUnit2(): Non valid time interval.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
        }
      //2. deal with the alpha value
      Alpha *value = (Alpha *)InFun( nl->TheEmptyList(), nl->Second( instance ),
                                     errorPos, errorInfo, correct ).addr;

      //3. create the class object
      if( correct  )
      {
        ConstTemporalUnit<Alpha> *constunit =
          new ConstTemporalUnit<Alpha>( tinterval, *value );

        if( constunit->IsValid() )
        {
          delete value;
          return SetWord( constunit );
        }
        delete constunit;
      }
      delete value;
    }
  }
  else if ( nl->IsAtom( instance ) &&
            nl->AtomType( instance ) == SymbolType &&
            nl->SymbolValue( instance ) == "undef" )
    {
      ConstTemporalUnit<Alpha> *constunit =
        new ConstTemporalUnit<Alpha>();
      constunit->SetDefined(false);
      constunit->timeInterval=
        Interval<DateTime>(DateTime(instanttype),
                           DateTime(instanttype),true,true);
      correct = true;
      return (SetWord( constunit ));
    }
  errmsg = "InConstTemporalUnit2(): Error in representation.";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  correct = false;
  return SetWord( Address(0) );
}


bool
CheckUFlock( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "uflock" ));
}

bool
MFlock::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "mflock" ));
}


ListExpr
UFlockProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UFLOCK"),
                             nl->StringAtom("(uflock) "),
                             nl->StringAtom("(timeInterval flock) "),
                             nl->TextAtom("((i1 i2 FALSE FALSE) "
                                 "(2 2 3 (11.1 345.2) (122 524 300)))"))));
}

ListExpr
MFlock::Property()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MFLOCK"),
                             nl->StringAtom("(mflock) "),
                             nl->StringAtom("(UFLOCK UFLOCK)"),
                             nl->TextAtom("(((i1 i2 FALSE FALSE) "
                                 "(2 2 3 (11.1 345.2) (122 524 300))) "
                                 "((i3 i4 TRUE FALSE) "
                                 "(2 2 2 (44.6 117.3) (12 24)))) "))));
}


TypeConstructor flockTC(
        "flock",       //name
        Flock::Property, //property function describing signature
        Flock::Out,
        Flock::In,     //Out and In functions
        0, 0,          //SaveToList and RestoreFromList functions
        Flock::Create,
        Flock::Delete, //object creation and deletion
        0, 0,
        Flock::Close,
        Flock::Clone,  //object close and clone
        Flock::Cast,   //cast function
        Flock::SizeOfObj, //sizeof function
        Flock::KindCheck );

TypeConstructor uflockTC(
        "uflock",     //name
        UFlockProperty, //property function describing signature
        OutConstTemporalUnit<Flock, Flock::Out>,
        InConstTemporalUnit2<Flock, Flock::In>, //Out and In functions
        0,                      0,//SaveToList and RestoreFromList functions
        CreateConstTemporalUnit<Flock>,
        DeleteConstTemporalUnit<Flock>, //object creation and deletion
        OpenAttribute<UFlock>,
        SaveAttribute<UFlock>,  // object open and save
        CloseConstTemporalUnit<UFlock>,
        CloneConstTemporalUnit<UFlock>, //object close and clone
        CastConstTemporalUnit<UFlock>,       //cast function
        SizeOfConstTemporalUnit<UFlock>, //sizeof function
        CheckUFlock );                    //kind checking function

TypeConstructor mflockTC(
        "mflock",               //name
        MFlock::Property,   //property function describing signature
        OutMapping<MFlock, UFlock, OutConstTemporalUnit<Flock, Flock::Out> >,
        InMapping<MFlock, UFlock, InConstTemporalUnit2<Flock, Flock::In> >,
    //Out and In functions
        0,
        0,            //SaveToList and RestoreFromList functions
        CreateMapping<MFlock>,
        DeleteMapping<MFlock>,   //object creation and deletion
        OpenAttribute<MFlock>,
        SaveAttribute<MFlock>,           // object open and save
        CloseMapping<MFlock>,
        CloneMapping<MFlock>, //object close and clone
        CastMapping<MFlock>,  //cast function
        SizeOfMapping<MFlock>,  //sizeof function
        MFlock::KindCheck );      //kind checking function



ListExpr RandomMFlockTM(ListExpr args)
{
  //cout<<nl->ToString(args);
  CHECK_COND( nl->ListLength(args) == 2 &&
      nl->IsAtom(nl->First(args)) &&
      (nl->SymbolValue(nl->First(args))== Instant::BasicType()) &&
      nl->IsAtom(nl->Second(args)) &&
      (nl->SymbolValue(nl->Second(args))== CcInt::BasicType()),
  "Operator randommflock expects two parameter.");
  return nl->SymbolAtom("mflock");
}

void CreateRandomMFlock(Instant starttime, int minSize, MFlock& result)
{
  bool debugme=false;
  result.Clear();
  int rnd, ucntr=0, unum, pntcntr=0, pnum;
  Flock* f;
  UFlock unit(true);
  Interval<Instant> intr(starttime, starttime, true, false);

  rnd=rand()%20;  //deciding the number of units in the mflock value
  unum=++rnd;
  while(ucntr++ < unum)
  {
    rnd=rand()%20;
    pnum= rnd + minSize; //deciding the number of points in the flock
    pntcntr=0;
    f=new Flock(0);
    while(pntcntr++ < pnum) //creating the flock
    {
      rnd=rand() % 30000;
      while(f->addPointID(rnd) == -1)
        rnd=rand() % 30000;
    }

    rnd=rand()%50000; //deciding the duration of a unit
    while(rnd<2)
      rnd=rand()%50000;
    intr.end.Set(intr.start.GetYear(), intr.start.GetMonth(),
        intr.start.GetGregDay(), intr.start.GetHour(),intr.start.GetMinute(),
        intr.start.GetSecond(),intr.start.GetMillisecond()+rnd);
    unit.constValue.CopyFrom(f);
    delete f;
    if(debugme)
    {
      cout<<"\nAdding unit number "<<ucntr <<endl;
      unit.constValue.Print(cout); cout.flush();
    }
    unit.timeInterval= intr;
    result.Add(unit);
    if(debugme)
    {
      unit.constValue.Print(cout); cout.flush();
      result.Print(cout); cout.flush();

    }
    intr.start= intr.end;
  }
  if(debugme)
  {
    cout<< "\nCreateMFlock returns \n";
    result.Print(cout);
  }
}

int
RandomMFlockVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme=false;
  result = qp->ResultStorage(s);
  MFlock* res = static_cast<MFlock*>( result.addr);
  DateTime* tstart = (DateTime*) args[0].addr;
  int flocksize = static_cast<CcInt*>(args[1].addr)->GetIntval();
  CreateRandomMFlock(*tstart, flocksize, *res);
  if(debugme)
  {
    cout<<"\nThe random mflock\n "; res->Print(cout);
  }
  return 0;
}

const string RandomMFlockSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> instant x int -> mflock</text--->"
  "<text>randommflock( starttime, flocksize )</text--->"
  "<text>Creates a random mflock value. The operator is used for testing"
  "purposes.</text--->"
  "<text>let mf1 = randommflock(now(), 50)</text--->"
  ") )";


Operator randommflock (
    "randommflock",               // name
    RandomMFlockSpec,             // specification
    RandomMFlockVM,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    RandomMFlockTM          // type mapping
);


ListExpr MFlock2MRegionTM(ListExpr args)
{
  string msg= nl->ToString(args);
  CHECK_COND( nl->ListLength(args) == 3 ,
      "Operator mflock2mregion expects 3 arguments.\nBut got: " + msg + ".");

  msg= nl->ToString(nl->First(args));
  CHECK_COND( listutils::isTupleStream(nl->First(args)) ,
      "Operator mflock2mregion expects stream(tuple(X)) as first argument."
      "\nBut got: " + msg + ".");

  msg= nl->ToString(nl->Second(args));
  CHECK_COND( listutils::isTupleStream(nl->Second(args)) ,
      "Operator mflock2mregion expects stream(tuple(X)) as second argument."
      "\nBut got: " + msg + ".");

  msg= nl->ToString(nl->Third(args));
  CHECK_COND( nl->IsAtom(nl->Third(args)) &&
      nl->SymbolValue(nl->Third(args))== Duration::BasicType(),
          "Operator mflock2mregion expects duration as third "
          "argument.\nBut got: " + msg + ".");

  ListExpr tuple1 = nl->Second(nl->Second(nl->First(args)));
  msg= nl->ToString(tuple1);
  CHECK_COND( nl->ListLength(tuple1) == 2 &&
    nl->IsAtom     (nl->Second(nl->First (tuple1))) &&
    nl->SymbolValue(nl->Second(nl->First (tuple1)))== CcInt::BasicType() &&
    nl->IsAtom     (nl->Second(nl->Second(tuple1))) &&
    nl->SymbolValue(nl->Second(nl->Second(tuple1)))== "mpoint",
        "Operator mflock2mregion expects stream(tuple(int mpoint)) as first "
        "argument.\nBut got: stream(tuple(" + msg + ")).");

  ListExpr tuple2 = nl->Second(nl->Second(nl->Second(args)));
  msg= nl->ToString(tuple2);
  CHECK_COND( nl->ListLength(tuple2) == 1 &&
    nl->IsAtom     (nl->Second(nl->First(tuple2))) &&
    nl->SymbolValue(nl->Second(nl->First(tuple2)))== "mflock",
        "Operator mflock2mregion expects stream(tuple(mflock)) as second "
        "argument.\nBut got: stream(tuple(" + msg + ")).");

  return (nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
      nl->SymbolAtom("movingregion")));
}

void test()
{
  bool debugme=true;
  Points* res= new Points(10);
  Point p(10.0, 10.0);
  for(int i=0; i<20; ++i)
  {
    p.Set(rand()%100 *1.0, rand()%100 *1.0);
    (*res) += p;
    if(debugme)
    {
      res->Print(cerr)<<endl;
    }
  }
}

MRegion*
MFlock::MFlock2MRegion(vector<int>* ids, vector<MPoint*>* sourceMPoints,
      Instant& samplingDuration)
{
  bool debugme=true;
/*
Validating the input

*/
  MRegion* res = new MRegion(0);
  res->SetDefined(false);
  if(!this->IsDefined() || sourceMPoints->size()==0) return res;

/*
Do the real work. The idea is as follows:
foreach uflock in this
  1- Sample uflock according to the samplingDuration (include the start and
  end instants).
  2- From every two consecutive samples, create a URegion

*/
  res->SetDefined(true);
  UFlock curUFlock;
  Flock* curFlock= new Flock(0);
  Instant curTime(instanttype);
  Instant finalTime(instanttype);
  Interval<Instant> unitInterval(curTime, curTime, true, false);
//  Intime<Flock> intimeFlock(curTime, curFlock);
  vector<double> weigths(4);
  weigths[0] = 0.7;            // AreaWeight
  weigths[1] = 0.7;            // OverlapWeight
  weigths[2] = 0.5;            // HausdorffWeight
  weigths[3] = 1.0;            // LinearWeight
  Points* ps;
  Region* reg1=new Region(0), *reg2=new Region(0), *regswap;
  RegionForInterpolation *reginter1, *reginter2, *reginterswap;
  Match *sm;
  mLineRep *lines;
  URegion *resUnit;

  for(int unitIndex=0; unitIndex < this->GetNoComponents(); ++unitIndex)
  {
/*
Computing the left part of the URegion (i.e. the values at the start instant)

*/
    this->Get(unitIndex, curUFlock);
    curTime= curUFlock.timeInterval.start;
    finalTime= curUFlock.timeInterval.end;
    finalTime-= samplingDuration ;
    *curFlock = curUFlock.constValue;
    unitInterval.start= curTime;
    unitInterval.lc= curUFlock.timeInterval.lc;
    ps= curFlock->Flock2Points(curTime, ids, sourceMPoints);
    GrahamScan::convexHull(ps,reg1);
    delete ps;
    reginter1=new RegionInterpol::RegionForInterpolation(reg1);

    while(curTime <= finalTime)
    {
/*
Computing the right part ofthe URegion

*/
      curTime+= samplingDuration;
      ps= curFlock->Flock2Points(curTime, ids, sourceMPoints);
      GrahamScan::convexHull(ps, reg2);
      unitInterval.end= curTime;
      reginter2=new RegionInterpol::RegionForInterpolation(reg2);
      if(debugme)
      {
        cerr<<endl<<"RegionForInter1 faces count "<<reginter1->getNrOfFaces();
        cerr<<endl<<"RegionForInter2 faces count "<<reginter2->getNrOfFaces();
      }
      sm=new OptimalMatch(reginter1, reginter2, weigths);
      lines=new mLineRep(sm);
      resUnit= new URegion(lines->getTriangles(), unitInterval);
      res->Add(*(resUnit->GetEmbedded()));
/*
Copying the right part of this URegion to the left part of the next URegion

*/
      unitInterval.start= unitInterval.end;
      regswap= reg1;
      reg1= reg2;
      reg2= regswap;
      reginterswap= reginter1;
      reginter1= reginter2;
/*
Garbage collection

*/
      delete resUnit;
      delete lines;
//      delete sm;
      //delete regswap;
      delete reginterswap;
      delete ps;

    }
/*
Adding the last instant in the unit

*/
    curTime= finalTime;
    ps= curFlock->Flock2Points(curTime, ids, sourceMPoints);
    GrahamScan::convexHull(ps, reg2);
    unitInterval.end= curTime;
    unitInterval.rc= curUFlock.timeInterval.rc;
    reginter2=new RegionForInterpolation(reg2);
    sm=new OptimalMatch(reginter1, reginter2, weigths);
    lines=new mLineRep(sm);
    resUnit= new URegion(lines->getTriangles(), unitInterval);
    res->Add(*(resUnit->GetEmbedded()));

    delete resUnit;
    delete lines;
//    delete sm;
    delete reg1;
    delete reg2;
    delete reginter1;
    delete reginter2;
    delete ps;
  }
  return res;
}

int
MFlock2MRegionVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme=true;
  result = qp->ResultStorage(s);
  MRegion* res = static_cast<MRegion*>( result.addr);

  MFlock* mflock;
  Tuple * t;
  switch (message)
  {
  case OPEN :{
    qp->Open(args[1].addr);
    return 0;
  }
  case REQUEST :{
    int* idsUnit, count, id;
    std::set<int>* usedIDs=new set<int>();
    std::vector<MPoint*>* mpoints;
    std::vector<MPoint*>::iterator mpointsIt;
    std::vector<int>* ids;
    UFlock uflock;
    Tuple* tuple;
    std::vector<Tuple*>* delBuffer= new std::vector<Tuple*>(0);
    std::vector<Tuple*>::iterator delBufferIt;
    MPoint* mpoint;
    MRegion* tmpres=0;
    Word w;
    qp->Request(args[1].addr, w);
    if(qp->Received(args[1].addr))
    {
      tuple= static_cast<Tuple*>(w.addr);
      mflock= static_cast<MFlock*>(tuple->GetAttribute(0));
      mpoints= new std::vector<MPoint*>();
      ids= new std::vector<int>();
      for(int i=0; i< mflock->GetNoComponents(); ++i)
      {
        mflock->Get(i, uflock);
        count= uflock.constValue.getPoints(idsUnit);
        usedIDs->insert(idsUnit, idsUnit + count);
        //uflock->DeleteIfAllowed();
      }

      qp->Open(args[0].addr);
      qp->Request(args[0].addr, w);
      t= static_cast<Tuple*>(w.addr);
      while(qp->Received(args[0].addr))
      {
        id= dynamic_cast<CcInt*>(t->GetAttribute(0))->GetIntval();
        if(usedIDs->find(id) != usedIDs->end())
        {
          ids->push_back(id);
          mpoint= dynamic_cast<MPoint*>(t->GetAttribute(1));
          mpoints->push_back(mpoint);
          delBuffer->push_back(t);
        }
        else
          t->DeleteIfAllowed();

        qp->Request(args[0].addr, w);
        t= static_cast<Tuple*>(w.addr);
      }
      qp->Close(args[0].addr);
      assert(ids->size()>0);
      res= mflock->MFlock2MRegion(ids, mpoints,
          *static_cast<Instant*>(args[2].addr));
      mflock->DeleteIfAllowed();

      for(delBufferIt= delBuffer->begin(); delBufferIt != delBuffer->end();
        ++delBufferIt)
        (*delBufferIt)->DeleteIfAllowed();
      delBuffer->clear();
      delete delBuffer;
      for(mpointsIt= mpoints->begin(); mpointsIt != mpoints->end(); ++mpointsIt)
        (*mpointsIt)->DeleteIfAllowed();
      mpoints->clear();
      ids->clear();
      delete mpoints;
      delete ids;
      res->CopyFrom(tmpres);
      delete tmpres;
      tuple->DeleteIfAllowed();
      return YIELD;
    }
    else
      return CANCEL;
  }
  case CLOSE :{
    qp->Close(args[1].addr);
    return 0;
  }
  default: return CANCEL;
  }
  return 0;
}

const string MFlock2MRegionSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> stream(tuple(int mpoint)) x stream(tuple(mflock)) x duration -> "
  "stream(movingregion)</text--->"
  "<text>Trains feed addcounter[Cnt, 1] project[Cnt, Trip] Flocks feed "
  "mflocks2mregions[create_duration(0, 10000)]</text--->"
  "<text>Creates mving region representation for the mflocks. The resulting "
  "mregions are the interpolation of the convex hull regions taken at time "
  "intervals of duration at most.</text--->"
  "<text>query Trains feed addcounter[Cnt, 1] project[Cnt, Trip] Flocks feed "
  "mflocks2mregions[create_duration(0, 10000)] consume</text--->"
  ") )";


Operator mflock2mregion (
    "mflock2mregion",               // name
    MFlock2MRegionSpec,             // specification
    MFlock2MRegionVM,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    MFlock2MRegionTM          // type mapping
);

class FlockAlgebra : public Algebra
{
public:
  FlockAlgebra() : Algebra()
  {
    AddTypeConstructor( &flockTC );
    AddTypeConstructor( &uflockTC );
    AddTypeConstructor( &mflockTC );

    flockTC.AssociateKind( Kind::DATA() );

    uflockTC.AssociateKind( Kind::TEMPORAL() );
    uflockTC.AssociateKind( Kind::DATA() );

    mflockTC.AssociateKind( Kind::TEMPORAL() );
    mflockTC.AssociateKind( Kind::DATA() );


    AddOperator(&reportflocks);
    AddOperator(&randommflock);
    AddOperator(&mflock2mregion);
  }
  ~FlockAlgebra() {};
};

};

/*
5 Initialization

*/


extern "C"
Algebra*
InitializeFlockAlgebra( NestedList* nlRef,
    QueryProcessor* qpRef )
    {
  // The C++ scope-operator :: must be used to qualify the full name
  return new FLOCK::FlockAlgebra;
    }
