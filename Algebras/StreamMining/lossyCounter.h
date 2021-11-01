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

*/

#ifndef LOSSY_COUNTER_H
#define LOSSY_COUNTER_H

#include "NestedList.h"
#include "ListUtils.h"
#include "AlgebraTypes.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "counterPair.h"


namespace eschbach {
    template<class T>
    class lossyCounter
{
  public:
  lossyCounter(const double epsilon);
  ~lossyCounter() {}


  //Getter und Setter
  void setDefined(bool value); 
  bool getDefined(); 
  int getEleCounter(); 
  double getEpsilon(); 
  long getCurrentWindowIndex();
  int getWindowSize();
  T getElement(int index);
  std::unordered_map<T, counterPair<T>> getFrequencyList();

  //Auxiliary Functions
  void initialize(const double epsilon);
  //An Element was already present, increase its counter frequency value
  void incrCount(T element);
  //Function that determines how to handle an element
  void addElement(T element);
  //Element was not present and a counter has to be created
  void insertElement(T element);
  //Checks if an element is present within the counter structure
  bool elementPresent(T element);
  //Function to remove counters from the  map that no longer
  //satisfy the threshold
  void reduce();
  //increase our window index
  void updateWindowIndex(); 
  //Return the counters whose frequency exceeds our minimum 
  //support
  std::vector<T> getFrequentElements(double minSupport);
  //Estimate of a single elements count. No operator implementation
  long estimateCount(T element);

  private:
    lossyCounter() {}
    bool defined;
    //allowed error rate
    double epsilon;
    //Amount of elements seen 
    size_t eleCounter;
    //Conceptual bucket/window size
    int windowSize; 
    //Current bucket/window index
    long windowIndex;
    //Structure used for storing the counters
    std::unordered_map<T, counterPair<T>> frequencyList;
  };

}
#endif
