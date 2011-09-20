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

This file was originally written by Christopher John Kline, under the copy
right statement below. Mahomud Sakr, September 2011, has made the necessary
changes to make it available as a SECONDO operator.

  Copyright (C) 1996, Christopher John Kline
  Electronic mail: ckline@acm.org

  This software may be freely copied, modified, and redistributed
  for academic purposes and by not-for-profit organizations, provided that
  this copyright notice is preserved on all copies, and that the source
  code is included or notice is given informing the end-user that the source
  code is publicly available under the terms described here.

  Persons or organizations wishing to use this code or any modified version
  of this code in a commercial and/or for-profit manner must contact the
  author via electronic mail (preferred) or other method to arrange the terms
  of usage. These terms may be as simple as giving the author visible credit
  in the final product.

  There is no warranty or other guarantee of fitness for this software,
  it is provided solely "as is". Bug reports or fixes may be sent
  to the author, who may or may not act on them as he desires.

  If you use this software the author politely requests that you inform him
  via electronic mail.


NamedObject.h
INTERFACE: Class for managing objects in a simulation
(c) 1996 Christopher Kline <ckline@acm.org>

September 2011 Mahmoud Sakr: The Boids simulator/data-generator is now
available as a SECONDO operator.

*/


#ifndef __NAMEDOBJECT_H
#define __NAMEDOBJECT_H

#include "SimObject.h"
#include <string.h>
using namespace std;
#ifndef DBL_MAX
#define DBL_MAX numeric_limits<double>::max()
#endif
//-------- Class NamedObject -------------------------------------------------

class NamedObject {

  friend class ObjectList;
  // The ObjectList class needs access to this class
  
public:

  SimObject *object;
  // The actual object.
  char *identifier;
  // A unique name which identifies this object.

  double timeout;
  // Desired time (in seconds) between calls to this object's update() method.
	// This is NOT guaranteed, more time may elapse between
  // updates depending on system load and other factors.

  double lastUpdate;
  // Last time this object was updated via update().

  NamedObject(SimObject &newObject, const char *name);
  // Constructor

protected:

  virtual ~NamedObject(void);
  // Destructor
  
private:

  NamedObject *next;
  // A pointer to the next item in the list of named objects.

};

//------------- Class ObjectList --------------------------------------------

class ObjectList {

public:

  virtual bool add(SimObject &objectToAdd, const char *id);
  // Adds the given named object to the object list. It is illegal to
	// add two objects with the same identifier to the same object list.

  virtual bool remove(const char *id) ;
  // Search the object list and delete the object whose
	// identifier matches the argument. Returns FALSE if
	// no matching object could be found.

  virtual SimObject *get(const char *id) const;
  // Search the object list and return the object whose
	// identifier matches the argument. Returns NULL if
	// no matching object could be found.

  virtual void resetIter(NamedObject **iterator);
  // Resets the iterator to the first element of the list of objects.
  
  virtual SimObject *iter(NamedObject **iterator);
  // The current object in the list is returned and the
	// iterator is incremented to the next object.
  //
  // Returns NULL if there are no more objects to iterate, or
  // if the argument is null.
  //
  // NOTE: it is DANGEROUS to modify the list between iterations, so
  // be sure to reset the iterator after using add() or remove().
  //
  // Sample useage follows:
  //
  // NamedObject *i;
  // SimObject *temp;
  //
  // resetIter(&i);   // reset iterator
  // while ((temp = iter(&i)) != NULL) {
  //      do stuff with temp
  // }

  virtual NamedObject *getNO(const char *id) const;
  // Analogous to get(), but returns the encapsulating NamedObject.
  virtual NamedObject *iterNO(NamedObject **iterator);
  // Analogous to iter(), but returns the encapsulating NamedObject.
	
  ObjectList(void);
  // Constructor
  virtual ~ObjectList(void);
  // Destructor
						      
protected:
  
private:

  NamedObject *root;
  // Pointer to first element in the list of named objects.
};


#endif // #ifndef __NAMEDOBJECT_H
