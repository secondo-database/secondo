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

//paragraph [1] title: [{\Large \bf ] [}]
//[ae] [\"{a}]
//[ue] [\"{u}]

[1] Implementation of utility data structures for handling 
parameter functions

June 2006 Victor Almeida

This code was taken out the ArrayAlgebra.cpp file implementation given
a namespace problem.

*/
using namespace std;

#include "FunVector.h"
#include "StandardTypes.h"

bool operator<( const FunInfo& f1, const FunInfo& f2 )
{
  return (f1.no < f2.no);
}

std::ostream&
operator<<( std::ostream& os, const FunInfo& f )
{
  os << "function " << f.name << " used " << f.timesUsed
     << " times, used CPU time: " << f.consumedTime << " seconds.";
  return os;
}

/*
1 Class ~FunInfo~

Each object of this class contains a function (given by a Supplier object)
together with some additional information, e.g. an assigned number
(["]function-id["]) and an assigned name.

A function can be requested with given parameters. The system measures, sums up
and prints out the used CPU time of the function. The total number of function
requests is also available.

*/
FunInfo::FunInfo() {}

FunInfo::FunInfo( int No, string Name, Supplier s )
{
  no = No;
  name = Name;
  supplier = s;
  timesUsed = 0;
  consumedTime = 0;
}

double
FunInfo::getTime() {
  return consumedTime;
}

void
FunInfo::request( Word* arguments, int n, Word& funresult, string info = "" )
{
  ArgVectorPointer funargs;
  clock_t c1;
  clock_t c2;
  double timediff;

  funargs = qp->Argument( supplier );

  for (int i=0; i<n; i++) {
    (*funargs)[i] = arguments[i];
  }

  if (info != "") {
    cout << info << ", ";
  }

  cout << "function " << name << ", ";

  c1 = clock();
  qp->Request( supplier, funresult );
  c2 = clock();

  timediff = (double)(c2 - c1) / CLOCKS_PER_SEC;

  timesUsed++;
  consumedTime += timediff;

  cout << "used CPU time: " << timediff << " (" << consumedTime
       << ") seconds." << endl;
}


void
FunInfo::open()
{
  qp->Open(supplier);
}

void
FunInfo::close()
{
  qp->Close(supplier);
}

void
FunInfo::request( Word argument, Word& funresult, string info = "" )
{
  request(&argument, 1, funresult, info);
}

void
FunInfo::request( Word firstArg, Word secondArg, Word& funresult,
                  string info = "" )
{
  Word arguments[2] = { firstArg, secondArg };
  request(arguments, 2, funresult, info);
}

/*
2 Class ~FunVector~

This class uses the class template ["]vector["]. Each object of the class
~FunVector~ contains a vector of ~FunInfo~ objects. The vector is initialized
with a set of functions (given by a Supplier object) and an array of function
names. After initializing the vector, a single function or all functions stored
in the vector may be requested.

The class also provides some useful methods for the implementation of the
switch- and the select algorithm.

*/
void
FunVector::addFunction( string name, Supplier s )
{
  //cout << "size: " << funInfos.size() << endl;
  FunInfo f( funInfos.size()+1, name, s );
  funInfos.push_back(f);
}

void
FunVector::load( Word suppl, Word* funNames )
{
  Supplier funSupplier = (Supplier)suppl.addr;
  Supplier supplier1;
  Supplier supplier2;

  int noOfFuns = qp->GetNoSons(funSupplier);

  for (int i=0; i<noOfFuns; i++) {
    const STRING* name = ((CcString*)funNames[i].addr)->GetStringval();
    //cerr << "Function " << i << "/" << noOfFuns << *name << endl;
    supplier1 = qp->GetSupplier(funSupplier, i);
    supplier2 = qp->GetSupplier(supplier1, 1);
    addFunction(*name, supplier2);
  }
}

void
FunVector::requestFun( int funNo, Word argument, Word& funresult,
                       string info = "" )
{
  funInfos[funNo].request(argument, funresult, info);
}

void
FunVector::requestFun( int funNo, Word firstArgument, Word secondArgument,
                       Word& funresult, string info = "" )
{
  funInfos[funNo].request(firstArgument, secondArgument, funresult, info);
}

void
FunVector::requestAll( Word argument, Word& funresult, string info = "" )
{
  for(int i=0; i<(int)funInfos.size(); i++) {
    requestFun(i, argument, funresult, info);
  }
}

void
FunVector::requestAll( Word firstArgument, Word secondArgument,
                       Word& funresult, string info = "" )
{
  for(int i=0; i<(int)funInfos.size(); i++) {
    requestFun(i, firstArgument, secondArgument, funresult, info);
  }
}

void
FunVector::sendMsgForAll(int msg)
{
  for (size_t i=0; i < funInfos.size(); i++ )
  {
    switch (msg) {
      case OPEN: funInfos[i].open();
      case CLOSE: funInfos[i].close();
      default: assert(false);
    }
  }
}


void
FunVector::reorder()

// Precondition:  All functions between the second and the last element of the
//                vector are ordered by their used CPU time.
// Postcondition: All functions of the vector are ordered by their used CPU
//                time.

{
  int n=funInfos.size();

  if (n > 1) {
    if (funInfos[0].getTime() > funInfos[1].getTime()) {

      // Find the position where to insert the first element, insert the first
      // element at this position and then remove this element from the first
      // position.

      int l = 0;
      int r = n;
      int m;

      do {
        m = (l+r) / 2;

        if (funInfos[m].getTime() <= funInfos[0].getTime()) {
          l = m;
        }
        else {
          r = m;
        }
      }
      while ( (m < (n-1))
              && !((funInfos[m].getTime() <= funInfos[0].getTime())
                   && (funInfos[0].getTime() < funInfos[m+1].getTime())) );

      funInfos.insert( funInfos.begin() + m + 1, funInfos[0] );
      funInfos.erase( funInfos.begin() );
    }
  }
}

int
FunVector::getMin()

// Returns the index of the function with the minimum used CPU time. If there
// are more such functions, the index of the first of these functions is
// returned.

{
  int min=0;

  for (int i=0; i < (int)funInfos.size(); i++) {
    if (funInfos[i].getTime() < funInfos[min].getTime()) {
      min = i;
    }
  }

  return min;
}

void
FunVector::writeSummary()
{
  sort(funInfos.begin(), funInfos.end());

  for (int i=0; i < (int)funInfos.size(); i++) {
    cout << "SUMMARY, " << funInfos[i] << "\n";
  }
}



/*
3 Class ~SwitchAlgorithm~

The switch algorithm is implemented as a sub-class of the class ~FunVector~.

An object of the class ~SwitchAlgorithm~ is initialized like an object of the
class ~FunInfo~. For each requests, the switch algorithm chooses the function
with the (so far) lowest total used CPU time.

*/
void
SwitchAlgorithm::request( Word argument, Word& funresult, string info = "" )
{
  requestFun(0, argument, funresult, info);
  reorder();
}

void
SwitchAlgorithm::request( Word firstArgument, Word secondArgument,
                          Word& funresult, string info = "" )
{
  requestFun(0, firstArgument, secondArgument, funresult, info);
  reorder();
}

/*
4 Class ~SelectAlgorithm~

The select algorithm is implemented as a sub-class of the class ~FunVector~.

An object of the class ~SelectAlgorithm~ is initialized analogous to an object
of the class ~FunInfo~. In addition to a set of functions, the parameter
~testSize~ has to be set to a number greater than zero. For the first
~testSize~ requests, all functions are used for evaluation. After that, the
function with the (so far) lowest total used CPU time is selected for all
further requests. However, this selection will not be changed later on.

*/
SelectAlgorithm::SelectAlgorithm()
{
  testSize = 0;
  selectedFun = -1;
  counter = 0;
}

void
SelectAlgorithm::setTestSize( int n )
{
  testSize = n;
}

void
SelectAlgorithm::request( Word argument, Word& funresult, string info = "" )
{
  if (counter++ < testSize) {
    requestAll(argument, funresult, info);
  }
  else {
    if (selectedFun == -1) {
      selectedFun = getMin();
    }

    requestFun(selectedFun, argument, funresult, info);
  }
}

void
SelectAlgorithm::request( Word firstArgument, Word secondArgument,
                          Word& funresult, string info = "" )
{
  if (counter++ < testSize) {
    requestAll(firstArgument, secondArgument, funresult, info);
  }
  else {
    if (selectedFun == -1) {
      selectedFun = getMin();
    }

    requestFun(selectedFun, firstArgument, secondArgument, funresult, info);
  }
}


