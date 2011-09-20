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


NamedObject.c++
CODE: Classes for managing objects in a simulation
(c) 1996 Christopher Kline <ckline@acm.org>

September 2011 Mahmoud Sakr: The Boids simulator/data-generator is now
available as a SECONDO operator.

*/

#ifndef __NAMEDOBJECT_C
#define __NAMEDOBJECT_C

#include "NamedObject.h"
using namespace std;
//-------- Class NamedObject ------------------------------------------------

NamedObject::NamedObject(SimObject &newObject, const char *name) {

  // Grab a pointer to the object so we can return it later when needed
  object = &newObject;

  // Duplicate the name
  identifier = new char[strlen(name)+1];
  strcpy(identifier, name);

  // Set up timeout and update information
  timeout = -1;   // -1 == Never update this object.
  lastUpdate = 0; 

}

NamedObject::~NamedObject(void) {

  delete identifier;
  
}

//------- Class ObjectList ---------------------------------------------------

bool
ObjectList::add(SimObject &objectToAdd, const char *id) {

/*
check here if there's already a object with the same identifier,
and return FALSE if that is the case

*/

  NamedObject *temp = new NamedObject(objectToAdd, id);

  temp->next = root;
  root = temp; 

  return true;
  
}

SimObject *
ObjectList::get(const char *id) const {

  NamedObject *temp = getNO(id);

  if (temp != NULL) {
    return temp->object;
  }
  else {
    return NULL;				// Didn't find it
  }
}

NamedObject *
ObjectList::getNO(const char *id) const {

  NamedObject *temp = root;

  while (temp != NULL) {
    if (strcmp(id, temp->identifier) == 0) 
      return temp;
    else
      temp = temp->next;
  }

  return NULL;				// Didn't find it
  
}

bool
ObjectList::remove(const char *id) {

  NamedObject *previousNode, *thisNode;

  previousNode = thisNode = root;

  while (thisNode != NULL) {
    if (strcmp(id, thisNode->identifier) == 0) { 
      (thisNode == root) ?
          (root = thisNode->next) :
          (previousNode->next = thisNode->next);
      break;
    }
    previousNode = thisNode;
    thisNode = thisNode->next;
  }

  if (thisNode == NULL) {
    return false;			// Didn't find it
  }
  else {
    delete thisNode;
    return true;			// Found and removed it
  }
  
}

void
ObjectList::resetIter(NamedObject **iterator) {

  *iterator = root;

}

SimObject *
ObjectList::iter(NamedObject **iterator) {

  NamedObject *temp = iterNO(iterator);

  if (temp != NULL) {
    return temp->object;
  }
  else {
    return NULL;
  }
}

NamedObject *
ObjectList::iterNO(NamedObject **iterator) {

  if (*iterator != NULL) {
    // Somewhere in the middle of the list of objects
    NamedObject *temp = *iterator;
    *iterator = (*iterator)->next;
    return temp;
  }
  else {
    // No more objects to iterate
      return NULL;
  }
  
}


ObjectList::~ObjectList(void) {

  NamedObject *temp;
  NamedObject *iterator;

  resetIter(&iterator);
  while ((temp = iterNO(&iterator)) != NULL) {
    delete temp;
  }
  
}


ObjectList::ObjectList(void) {


}


#endif __NAMEDOBJECT_C
