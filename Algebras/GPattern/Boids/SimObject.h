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


SimObject.h
INTERFACE: Base class for all object components of the VRRR project
(c) 1996 Christopher Kline <ckline@acm.org>

September 2011 Mahmoud Sakr: The Boids simulator/data-generator is now
available as a SECONDO operator.

*/



// Forward declare SimObject so that the classes that are #included
// below will know about it.
class SimObject;

#ifndef __SIMOBJECT_H
#define __SIMOBJECT_H

#include "Vector.h"
#include<limits>
using namespace std;


class SimObject {

public:

  // -- Query Methods -- 

  virtual double getMass(void) const;
  // Returns the mass of the object in kilograms.
  
  virtual const MathVector &getVelocity(void) const;
  // Returns the current velocity of this object's center of mass
	// (in world coordinates) in m/s.
  
  virtual const MathVector &getPosition(void) const;
  // Returns the current position of this object's center of mass
  // (in world coordinates) in m.

  virtual const MathVector &getDimensions(void) const;
  // Returns the dimensions [x,y,z] = [length, width, height] of
	// this object's minimal bounding box. See the description of the
	// 'dimensions' vector below for more detail.
  
  // -- Modification Methods

  virtual bool update(const double &elapsedSeconds);
  // The simulation will call this method, which updates the object
	// based on its current state, the current elapsed time in
	// seconds, and the external and internal forces acting upon it.
	// A return value of FALSE indicates that some sort of error has
	// occured during the update.
  
  virtual void setDimensions(const MathVector newDimensions);
  // Set the dimensions of this system

  SimObject(void);
  // Default constructor
  
  virtual ~SimObject(void);
  // Destructor

protected:
  
  MathVector velocity;
  // Current velocity of this object's center of mass (in 3-space)
	// in m/s. (assuming +y is up, +z is through the nose, +x is through
  // the left wing)

  MathVector position;
  // Current position of this object's center of mass (in 3-space)
	// in m. (assuming y is the altitude, (x, z) are the 2D coordinates
  // (see the description of the velocity components))

  MathVector dimensions;
  // Vector defining the minimal bounding box for this
	// object. Length is defined as being parallel to the body axis
	// which is aligned with the normal direction of movement. Height
	// in the vertical direction and perpendicular to the Length-Width
	// plane. Width is perpendicular to the Length and Height.
	//
	// dimensions.{x,y,z} = {length, width, height}

  double mass;
  // Mass of this object in kilograms.

  double lastUpdate;
  // Last time this object was updated via update().
  
private:
  
};

// ------------ inline methods --------------------------------------

inline const MathVector &
SimObject::getVelocity(void) const {
  
  return velocity;

}

inline const MathVector &
SimObject::getPosition(void) const {
  
  return position;

}

inline const MathVector &
SimObject::getDimensions(void) const {
  
  return dimensions;

}

inline double
SimObject::getMass(void) const {

  return mass;
  
}


inline bool
SimObject::update(const double &elapsedSeconds) {

  lastUpdate = elapsedSeconds;		

  return true;
  
}

inline void
SimObject::setDimensions(const MathVector newDimensions) {

  dimensions = newDimensions;
  
}


inline
SimObject::SimObject(void) {
  
  position = velocity = dimensions = MathVector(0, 0, 0);
  lastUpdate = 0;
  
}

inline   
SimObject::~SimObject(void) {
  
}

#endif // __SIMOBJECT_H
