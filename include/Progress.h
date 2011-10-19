/*
----
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

//paragraph [23]  table3columns:  [\begin{quote}\begin{tabular}{lll}] [\end{tabular}\end{quote}]
//[--------]  [\hline]
//characters  [1] verbatim: [\verb|]  [|]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [$\leq$]
//[#]  [$\neq$]
//[tilde] [\verb|~|]

1 Header File Progress.h

April 2007 M. Spiekermann

2 Overview

This file collects all methods useful for the implementation of progress
messages. This is still work in progress!

*/


#ifndef CLASS_PROGRESS_H
#define CLASS_PROGRESS_H

#include <iostream>
#include <map>


using namespace std;


/*
The following are thresholds used in filter and join operators to determine when to switch into warm state (i.e., trust the selectivity observation). They denote the number of matching tuples that must have been found.


*/

const int enoughSuccessesSelection = 50;
const int enoughSuccessesJoin = 50;

/*
If switched on, suitable operators just use the progress value of their predecessor, if there is no blocking operator before them.

*/

const bool pipelinedProgress = true;




class ProgressLocalInfo;

class ProgressInfo
{
public:

  ProgressInfo();

  double Card;    //expected cardinality
  double Size;    //expected total tuple size (including FLOBs)
  double SizeExt;  //expected size of tuple root and extension part
          //   (no FLOBs)
  int noAttrs;    //no of attributes
  double *attrSize;  // array: for each attribute, the complete size
                    // stream sources should allocate the memory only once
                    // and delete it in CLOSEPROGRESS. Other operators
                    // should just copy the pointer and pass it on, unless they
                    // have impact on the size table. Then they should work
                    // like a stream source operator.
  double *attrSizeExt;  // for each attribute, the root and extension size
                        // also see ~attrSize~
  bool sizesChanged;  //true if sizes have been recomputed in this request

  double Time;    //expected time, in millisecond
  double Progress;  //a number between 0 and 1

  double BTime;    //expected time, in millisecond of blocking ops
  double BProgress;  //a number between 0 and 1


  void CopySizes(const ProgressInfo& p);  //copy the size fields

  void CopySizes(const ProgressLocalInfo* pli);  //copy the size fields

  void CopyBlocking(const ProgressInfo& p);  //copy BTime, BProgress
          //for non blocking unary op.

  void CopyBlocking(const ProgressInfo& p1,
                    const ProgressInfo& p2); //copy BTime, BProgress
          //for non-blocking binary op. (join)

  void Copy(const ProgressInfo& p);    //copy all fields


  ostream& Print(ostream& out) const;

  bool checkRanges() const;

};


ostream& operator<<(ostream& out, const ProgressInfo& pi);



class ProgressLocalInfo
{

public:

  ProgressLocalInfo();

  ~ProgressLocalInfo();

  int returned;     //current number of tuples returned
  int read;         //no of tuples read from arg stream
  int readFirst;    //no of tuples read from first arg stream
  int readSecond;   //no of tuples read from second argument stream
  int total;        //total number of tuples in argument relation
  int defaultValue; //default assumption of result size, needed for
                    //some operators
  int state;        //to keep state info if needed
  int memoryFirst,
      memorySecond;  //size of buffers for first and second argument

  void* firstLocalInfo;  //pointers to localinfos of first and second arg
  void* secondLocalInfo;

  bool sizesInitialized;//size fields only defined if sizesInitialized;
                        //initialized means data structures are allocated
                        //and fields are filled
  bool   sizesChanged;  //sizes were recomputed in last call
  double Size;          //total tuplesize
  double SizeExt;        //size of root and extension part of tuple
  int    noAttrs;        //no of attributes
  double *attrSize;      //full size of each attribute
  double *attrSizeExt;  //size of root and ext. part of each attribute

  void SetJoinSizes( ProgressInfo& p1, ProgressInfo& p2 ) ;

      //set the sizes for a join of first and second argument
      //only done when sizes are initialized or have changed
};




/*
Class ~ProgressWrapper~

This class is useful if operator implementations are encapsulated into their
own classes. By inheriting from this class, it is possible to access a ~ProgressLocalInfo~
object inside the implementation of a special class, e.g. ~SortByLocalInfo~


*/

class ProgressWrapper {

public:

  ProgressWrapper(ProgressLocalInfo* p) : progress(p) {}
  ~ProgressWrapper(){}

protected:

  // the pointer address can only be assigned once, but
  // the object pointed to may be modified.
  ProgressLocalInfo* const progress;

};



/*
Class ~LocalInfo~

This class inherits from class ~ProgressLocalinfo~ and has an additional
pointer to some class ~T~.

In operator implementations you may define the local.addr pointer
like this:

----  local.addr = new LocalInfo<SortByLocalInfo>;
----

Moreover, the ~SortByLocalInfo~ class must inherit from class ~ProgressWrapper~

----  SortByLocalInfo : protected ProgressWrapper { ... }
----

Refer to the ~sortby~ operator for implementation details.


*/

template<class T>
class LocalInfo : public ProgressLocalInfo {

  public:
    LocalInfo() : ProgressLocalInfo(), ptr(0) {}
    ~LocalInfo() { if (ptr) delete ptr; }

    inline void deletePtr() { if (ptr) {delete ptr; ptr = 0; } }

    T* ptr;
};


/*
Class ~ProgressConstants~

This class manages constants used by operators in progress estimation.

*/
class ProgressConstants{
  public:

/*
~readConstants~

This function reads constants from a csv file having the following format

Algebra, Operator, ConstantName, ConstantValue, ;Meaning, Meaning1

for each line. Lines starting with '[#]' are ignored (comment).

*/  
     static bool readConstants(const std::string& filename);

     static double getValue(const string& AlgName,
                              const string& OpName,
                              const string& ConstantName);


  private:
     ProgressConstants(); // pure static class don't allow to 
                          // create an instance

     static map<string,double> values;


};







#endif
