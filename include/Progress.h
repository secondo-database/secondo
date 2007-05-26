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
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File Progress.h 

April 2007 M. Spiekermann 

2 Overview

This file collects all methods useful for the implementation of progress
messages. This is still work in progress!

*/


#ifndef CLASS_PROGRESS_H
#define CLASS_PROGRESS_H


using namespace std;


class ProgressLocalInfo;

class ProgressInfo
{
public:

  ProgressInfo();

  double Card;		//expected cardinality
  double Size;		//expected total tuple size (including FLOBs)
  double SizeExt;	//expected size of tuple root and extension part 
    			//   (no FLOBs)
  int noAttrs;		//no of attributes
  double *attrSize;	//for each attribute, the complete size
  double *attrSizeExt;	//for each attribute, the root and extension size

  double Time;		//expected time, in millisecond
  double Progress;	//a number between 0 and 1

  double BTime;		//expected time, in millisecond of blocking ops
  double BProgress;	//a number between 0 and 1


  void CopySizes(ProgressInfo p);	//copy the size fields

  void CopySizes(ProgressLocalInfo* pli);  //copy the size fields

  void CopyBlocking(ProgressInfo p);	//copy BTime, BProgress 
					//for non blocking unary op.

  void CopyBlocking(ProgressInfo p1,ProgressInfo p2); //copy BTime, BProgress
					//for non-blocking binary op. (join)

  void Copy(ProgressInfo p);		//copy all fields

};	







class ProgressLocalInfo
{
  
public:

  ProgressLocalInfo(); 

  ~ProgressLocalInfo();

  int returned;        	//current number of tuples returned
  int read;		//no of tuples read from arg stream
  int readFirst;	//no of tuples read from first arg stream
  int readSecond;	//no of tuples read from second argument stream
  int total;          	//total number of tuples in argument relation
  int defaultValue;	//default assumption of result size, needed for 
			//some operators
  int state;		//to keep state info if needed

  bool progressInitialized;

			//the following only defined if progressInitialized
  double Size;		//total tuplesize
  double SizeExt;	//size of root and extension part of tuple
  int noAttrs;		//no of attributes
  double *attrSize;	//full size of each attribute
  double *attrSizeExt;	//size of root and ext. part of each attribute

  void SetJoinSizes( ProgressInfo& p1, ProgressInfo& p2 ) ;

			//set the sizes for a join of first and second argument
			//only done once
};










/*
Class ~Progress~

This class collects useful information and operations for the
handling of progress messages.

*/

class Progress {

public:

  Progress() : ctr(0),
    ctrA(0), ctrB(0), rtrn(0),
    noAtt(0), prInit(0), attSize(0), attSExt(0),
    PtrA(0), PtrB(0)
    {}

  virtual ~Progress() {}

  void setCtr(long value);
  long getCtr();

  long getCtrA();
  void setCtrA(long value);
  void incCtrA();

  long getCtrB();
  void setCtrB(long value);
  void incCtrB();

  long getRtrn();
  void setRtrn(long value);
  void incRtrn();

  bool getPrInit();
  void setPrInit(bool value);

  long getNoAtt();
  void setNoAtt(long value);

  double* getAttSize();
  double* initAttSize(int value);

  double* getAttSExt();
  double* initAttSExt(int value);

  void* getPtrA();
  void setPtrA(void* value);

  void* getPtrB();
  void setPtrB(void* value);


private:
  long ctr;
  long ctrA, ctrB, rtrn, noAtt;
  bool prInit;
  double *attSize, *attSExt;
  void* PtrA;
  void* PtrB;

};

/*
Class ~ProgressWrapper~

This class is useful if operator implementations are encapsulated into their
own classes. By inherting from this class it is possible to access a ~Progress~
object inside the implementation of a special class, e.g. ~SortByLocalInfo~

*/

class ProgressWrapper {

public:	

  ProgressWrapper(Progress* p) : progress(p) {}
  ~ProgressWrapper(){}

protected:

  // the pointer address can only be assigned once, but
  // the object pointed to may be modified.
  Progress* const progress;

};	

/* 
Class ~LocalInfo~

This class inherits from class ~Progress~ and has an additional
pointer to some class ~T~. It is a template class and offers
a function which casts the internal pointer to the appropriate
type.

In operator implementations you may define the local.addr pointer
like this:

----
    local.addr = new LocalInfo<SortByLocalInfo>;
----

Moreover, the ~SortByLocalInfo~ class must inherit from class ~ProgressWrapper~

----
    SortByLocalInfo : protected ProgressWrapper { ... }
----

Refer to the ~sortby~ operator for implementation details.

*/

template<class T>
class LocalInfo : public Progress {

  public:
    LocalInfo() : Progress(), ptr(0) {}
    ~LocalInfo() {}   

    T* ptr;  
};	






#endif
