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

/*
Class ~Progress~

This class collects useful information and operations for the
handling of progress messages.

*/

class Progress {

public:

  Progress() : ctr(0) {}
  virtual ~Progress() {}

  void setCtr(long value);
  long getCtr();

private:
  long ctr;

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
  
     inline static T* getPtr(void* valuePtr)
     {	     
       return 
	 static_cast<LocalInfo*>( valuePtr )->ptr;
     }  

    T* ptr;  
};	






#endif
