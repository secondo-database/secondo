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
  lossyCounter(const float epsilon);
  ~lossyCounter() {}


  //Getter und Setter
  void setDefined(bool value); 
  bool getDefined(); 
  int getEleCounter(); 
  float getEpsilon(); 
  long getCurrentWindowIndex();
  int getWindowSize();
  T getElement(int index);
  std::unordered_map<T, counterPair<T>> getFrequencyList();

  //Auxiliary Functions
  void initialize(const float epsilon);
  void incrCount(T element); 
  void addElement(T element);
  void insertElement(T element);
  bool elementPresent(T element);
  void reduce();
  void updateWindowIndex(); 
  std::vector<T> getFrequentElements(double minSupport);
  long estimateCount(T element);

  private:
    lossyCounter() {}
    bool defined;
    float epsilon; 
    size_t eleCounter; 
    int windowSize; 
    long windowIndex;
    std::unordered_map<T, counterPair<T>> frequencyList;
  };

}
#endif
