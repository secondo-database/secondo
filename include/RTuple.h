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

May 2007. M. Spiekermann. 

The class ~RTuple~ (Referenced Tuple) manages the calls needed to do a correct
handling of reference counting. This is helpful for complex operator
implementations, which need to manage many pointers of class ~Tuple~ and which
need to keep their values during two REQUEST messages, e.g. refer to the
implementation of ~mergejoin~ for its usage.

*/

#ifndef SEC_RTUPLE_H
#define SEC_RTUPLE_H

#include "Algebras/Relation-C++/RelationAlgebra.h"

class RTuple {

  public:	
  RTuple() : tuple(0) {}

  inline RTuple(Tuple* p) : tuple(p) 
  {
    holdTuple();	  
  }	
  	
  inline RTuple(const RTuple& r) : tuple(r.tuple) 
  {
    holdTuple();	  
  }	  

  inline ~RTuple() 
  {
    releaseTuple();	  
  }	    

  inline RTuple& operator=(const RTuple& r) 
  {
    releaseTuple();	  
    tuple = r.tuple;
    holdTuple();
    return *this;
  }	  

  inline RTuple& operator=(Tuple* t) 
  {
    releaseTuple();	  
    tuple = t;
    holdTuple();
    return *this;
  }	  

/* 
 Note: Function ~setTuple~ decrements the reference counter for the previously
 assigned tuple but does *not* increment it for the new one! 

*/
  inline void setTuple(Tuple* tuple){
    releaseTuple();
    this->tuple=tuple;
  }

  inline void init() { tuple = 0; }


  inline friend bool operator==(const RTuple& r, const void* p) 
  { 
    return r.tuple == p; 
  }

  inline friend bool operator!=(const RTuple& r, const void* p)
  { 
    return r.tuple != p; 
  }

  Tuple* tuple;

  private: 
   inline void releaseTuple() 
   {
     if (tuple) {
       tuple->DeleteIfAllowed();      
     }
   }     

   inline void holdTuple() 
   {
    if (tuple) {
      tuple->IncReference();
    }	    
   }	   
};

#endif
