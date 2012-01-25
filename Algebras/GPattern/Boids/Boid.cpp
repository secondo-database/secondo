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


Boid.c++
Class for objects that move with A-Life boid-type behavior.
(c) 1996 Christopher Kline <ckline@acm.org>

September 2011 Mahmoud Sakr: The Boids simulator/data-generator is now
available as a SECONDO operator.

*/





#ifndef __BOID_C
#define __BOID_C

#include "Boid.h"
#include <stdio.h>
#include <limits>
using namespace std;

/*
Returns the value in the visibility matrix which determines if this boid
can see the "otherBoid". A value of -1 means that otherBoid cannot be
seen; any other value indicates that otherBoid can be seen, and the
value is equal to the otherBoid's boidType.

*/
int **Boid::visibilityMatrix;
#define visMatrix(otherBoid) \
  (Boid::visibilityMatrix[boidNumber-1][(otherBoid)->boidNumber-1])



double
Boid::accumulate(MathVector &accumulator, MathVector valueToAdd) {

  double magnitudeRemaining = 1.0 - Magnitude(accumulator);
  double vMag = Magnitude(valueToAdd);
  double newMag = vMag < magnitudeRemaining ? vMag : magnitudeRemaining;

  accumulator += (Direction(valueToAdd) * newMag); 
  
  return newMag;
}

void Boid::calculateVisibilityMatrix(void) {

  Boid *n;
  int val;

  unsigned int cur= 0;

/*
Foreach boid, if it's not visible to us, put a a -1 in the
matrix. Otherwise put the boidType in the matrix.

*/

  while (cur != this->Generator->boids.size())
  {
    n= this->Generator->boids[cur]; ++cur;
    // Can't see self!
    if (n == this) {
      val = -1;
    }
    else {
      val = (visibleToSelf(n) ? n->boidType : -1);
    }
    visMatrix(n)= val;
  }

}

MathVector
Boid::navigator(const double sampleIntervalSeconds) {

/*
Calculate the visibility matrix so that visibility computations are
much more efficient.

*/
  calculateVisibilityMatrix();

  MathVector vacc(0, 0, 0);  // vector accumulator

  if(this->boidType == FREEMOVER)
  {
    if (accumulate(vacc, wander()) >= 1.0)
      goto MAXACCEL_ATTAINED;
    if (accumulate(vacc, collisionAvoidance(sampleIntervalSeconds)) >= 1.0)
      goto MAXACCEL_ATTAINED;
    if (accumulate(vacc, levelFlight(vacc)) >= 1.0)
      goto MAXACCEL_ATTAINED;
  }
  else
  {
    if (accumulate(vacc, collisionAvoidance(sampleIntervalSeconds)) >= 1.0)
      goto MAXACCEL_ATTAINED;
    if (accumulate(vacc, velocityMatching()) >= 1.0)
      goto MAXACCEL_ATTAINED;
    if (accumulate(vacc, maintainingCruisingDistance()) >= 1.0)
      goto MAXACCEL_ATTAINED;
    if (accumulate(vacc, flockCentering()) >= 1.0)
      goto MAXACCEL_ATTAINED;
    if (accumulate(vacc, wander()) >= 1.0)
      goto MAXACCEL_ATTAINED;
    if (accumulate(vacc, levelFlight(vacc)) >= 1.0)
      goto MAXACCEL_ATTAINED;
  }
  
MAXACCEL_ATTAINED:  // label 
  
/*
IMPORTANT:
Since the FlockCentering, CollisionAvoidance, and VelocityMatching modules
return a vector whose magnitude is a percentage of the maximum acceleration,
we need to convert this to a real magnitude before we hand it off to the Flight
module.
Remember, maxAcceleration is in terms of a fraction (0.0 to 1.0) of maxVelocity

*/

  vacc *= maxAcceleration * maxVelocity; 
  return(vacc);
}

void
Boid::calculateRollPitchYaw(MathVector appliedAcceleration,
			    MathVector currentVelocity,
			    MathVector currentPosition) {

/*
NOTES:

  1) currentPosition (the third argument) is unused, though in a more
  advanced flight model it could make a difference (some flight dynamics
  change depending on altitude, for example)

  2) Dr. Craig Wanke of the Mitre Corporation in McLean, Virginia helped me
  tremendously when working out these equations. Thanks, Craig!
  
  In our simple flight model, roll only a function of lateral
  (centripedal) acceleration and gravitational acceleration. We assume
  that gravitational acceleration will NOT be zero.

  Determine the direction of the lateral acceleration.

*/
  MathVector lateralDir =
      Direction((currentVelocity % appliedAcceleration) % currentVelocity);
  
/*
Set the lateral acceleration's magnitude. The magnitude is the vector
projection of the appliedAcceleration vector onto the direction of the
lateral acceleration).

*/
  double lateralMag = appliedAcceleration * lateralDir;
  
  if (lateralMag == 0)
    roll = 0;  
  else
    roll = -atan2(getGravAcceleration(), lateralMag) + M_PI/2;

  pitch = -atan(currentVelocity.y / 
		sqrt(currentVelocity.z*currentVelocity.z
		     + currentVelocity.x*currentVelocity.x));

  yaw = atan2(currentVelocity.x, currentVelocity.z);
}

MathVector
Boid::wander(void) {

  double distanceFromCruiseSpeed =
    (Magnitude(velocity) - desiredCruisingSpeed()) / maxVelocity;

  double urgency = fabs(distanceFromCruiseSpeed);
  if (urgency > 0.25)
    urgency = 0.25;
  
  return Direction(velocity) * urgency * (distanceFromCruiseSpeed > 0? -1 : 1);
}

MathVector Boid::collisionAvoidance(const double sampleIntervalSeconds)
{
  Obstacle *obs;
  ISectData d;
  MathVector normalToObject(0, 0, 0);
  int objectSeen = 0;
  MathVector pointOnObject;

  // Ignore obstacles that are out of the range of our probe.
  double distanceToObject = getProbeLength(sampleIntervalSeconds);
  double dist=0;
  // Find closest imminent collision with non-boid object
  unsigned int cur=0;
  while (cur != this->Generator->obstacles.size()) {
    obs= this->Generator->obstacles[cur]; ++cur;
    d = obs->DoesRayIntersect(Direction(velocity), position);

    if (d.intersectionflag == 1) {

      // Velocity vector intersects an obstacle
      dist= Magnitude(d.point-position);
      if (dist <= distanceToObject) {
        // found a closer object
        objectSeen = 1;
        distanceToObject = Magnitude(d.point-position);
        normalToObject = d.normal;
        pointOnObject = d.point;
      }
    }
  }

  if (!objectSeen) {
    return MathVector(0,0,0);
  }
  return resolveCollision(pointOnObject, normalToObject, sampleIntervalSeconds);
}
  
MathVector
Boid::resolveCollision(
    MathVector pointOnObject, MathVector normalToObject,
    const double sampleIntervalSeconds)
{
  double distanceToObject = Magnitude(pointOnObject - position);
/*
Make sure the object's normal is pointing towards the boid.
The boid wants to head in the direction of the normal, which
should push it away from the obstacle if the normal is pointing
towards the boid.
  
*/
  if (AngleBetween(velocity, normalToObject) < M_PI/2) 
    normalToObject = - normalToObject;

  double mag = 1 - distanceToObject/getProbeLength(sampleIntervalSeconds);

  // Ignore objects that are farther away than our probe.
  if (mag < 0) mag = 0;

/*
Mahmoud Sakr has changed this. Instead of yielding a vector directed in the
direction of the normal, now it returns a vector in the direction of
(-velocity + normal). This should make the Boids perform a sharper turn when they
reach an obstacle, in an effect which is similar to a ball hitting an obstacle.

return Direction(normalToObject ) * mag;

*/

  return Direction(  normalToObject - velocity ) * mag;
}

MathVector
Boid::maintainingCruisingDistance(void) {

  double distanceToClosestNeighbor = std::numeric_limits<double>::max();
  // DBL_MAX defined in <limits.h>
  int foundClosestNeighbor = 0;
  
  Boid *n, *closestNeighbor;   
  double tempDistance;
  
  unsigned int cur= 0;
  
  while (cur != this->Generator->boids.size()) {
    
    n= this->Generator->boids[cur]; ++cur;
    // Skip boids that we don't need to consider
    if (visMatrix(n) == -1) continue; 
    if (visMatrix(n) != boidType && flockSelectively) continue;
    
    // Find distance from the current boid to self
    tempDistance = Magnitude(n->position - position);
    
    // remember distance to closest boid
    if (tempDistance < distanceToClosestNeighbor) {
      distanceToClosestNeighbor = tempDistance;
      foundClosestNeighbor = 1;
      closestNeighbor = n;
    }

  }
  
  MathVector speedAdjustmentVector(0, 0, 0);

  if (foundClosestNeighbor) {
/*
Have the boid try to remain at least cruiseDistance away from its
nearest neighbor at all times in all directions (i.e., don't violate
your neighbor's "personal space" bounding sphere of radius
cruiseDistance, but stay as close to the neighbor as possible).

*/
    MathVector separationVector = closestNeighbor->getPosition() - position;

     float separateFactor = 0.09;
     float approachFactor = 0.05;

    if (separationVector.y < cruiseDistance) {
      speedAdjustmentVector.y -= separateFactor;
    }
    else if (separationVector.y > cruiseDistance) {
      speedAdjustmentVector.y += approachFactor;
    }

    if (separationVector.x < cruiseDistance) {
      speedAdjustmentVector.x -= separateFactor;
    }
    else if (separationVector.x > cruiseDistance) {
      speedAdjustmentVector.x += approachFactor;
    }
    
    if (separationVector.z < cruiseDistance) {
      speedAdjustmentVector.z -= separateFactor;
    }
    else if (separationVector.z > cruiseDistance) {
      speedAdjustmentVector.z += approachFactor;
    }
    
  }
/*
Otherwise, if we couldn't find a closest boid, speedAdjustmentVector
will have a magnitude of 0 and thus have no effect on navigation.

*/

  return(speedAdjustmentVector);
}


MathVector
Boid::velocityMatching(void) {

  MathVector velocityOfClosestNeighbor(0,0,0);
  double tempDistance;
  double distanceToClosestNeighbor = DBL_MAX;

  Boid *n;   
  
  unsigned int cur= 0;
  
  while (cur != this->Generator->boids.size()) {
    
    n= this->Generator->boids[cur]; ++cur;
    // Skip boids that we don't need to consider
    if (visMatrix(n) == -1) continue; 
    if (visMatrix(n) != boidType && flockSelectively) continue; 
    
    // Find distance from the current boid to self
    tempDistance = Magnitude(n->position - position);
    
    // remember velocity vector of closest boid
    if (tempDistance < distanceToClosestNeighbor) {
      distanceToClosestNeighbor = tempDistance;
      velocityOfClosestNeighbor = n->velocity;
    }
  }
  
/*
If we found a close boid, set the percentage of our acceleration that
we want to use in order to begin flying parallel to its velocity vector.
Otherwise, if we couldn't find a closest boid, velocityOfClosestNeighbor
will have a magnitude of 0 and thus have no effect on navigation.

*/
  if (distanceToClosestNeighbor != DBL_MAX) {
    // return velocity vector of closest boid so we can try to match it
    velocityOfClosestNeighbor.SetMagnitude(0.05);
  }
  
  return(velocityOfClosestNeighbor);
}

MathVector Boid::flockCentering(void)
{
  MathVector t;
  double boids_observed = 0;   // number of boids that were checked 
  MathVector flockcenter(0,0,0);   // approximate center of flock
  
  Boid *n;   
  
  unsigned int cur= 0;

  // Calculate approximate center of flock by averaging the positions of all
  // visible boids that we are flocking with.
  while (cur != this->Generator->boids.size()) {
    n= this->Generator->boids[cur]; ++cur;
    if (visMatrix(n) == -1) continue;
    if (visMatrix(n) != boidType && flockSelectively) continue;

    flockcenter += n->position;
    boids_observed++;
  }
  
  if (boids_observed != 0) { 
    flockcenter /= boids_observed;
    
    // now calculate a vector to head towards center of flock
    t = flockcenter - position;
    
    // and the percentage of maximum acceleration (in decimal)
    // to use when yaw toward center
    t.SetMagnitude(0.1); 
    
  }
  else {
    // Don't see any other birds. 
    t.SetMagnitude(0);
  }
  
  return(t);
}

bool
Boid::update(const double &elapsedSeconds, const double sampleIntervalSeconds)
{
  bool ok =  false;
  if (flightflag == false) {
    flightflag = true;
  }
  else {
    double dt = elapsedSeconds - lastUpdate;

/*
Step 1: Calculate new position and velocity by integrating
    the acceleration vector.
    
Update position based on where we wanted to go, and how long it has
been since we made the decision to go there

*/
    position += (oldVelocity*dt + 0.5*acceleration*dt*dt);
    
    // Set new velocity, which will be in the direction boid is traveling.
    velocity += (acceleration*dt);
    
    // Cap off velocity at maximum allowed value
    if (Magnitude(velocity) > maxVelocity) {
      velocity.SetMagnitude(maxVelocity);
    }
/*
Step 2: Calculate new roll, pitch, and yaw of the boid
which correspond to the changes in position and velocity.
Remember: the boid isn't necessarily oriented in the
direction of the velocity!
(assuming +y is up, +z is through the nose, +x is through the left wing)

*/
    calculateRollPitchYaw(acceleration, oldVelocity, position);
  }    
  
  // remember current velocity 
  oldVelocity = velocity; 

  // remember desired acceleration (the acceleration vector that the
  // Navigator() module specified
  acceleration = navigator(sampleIntervalSeconds);
  
  ok &= SimObject::update(elapsedSeconds); // Do generic object  updating

  return ok;
}

void Boid::SetGenerator(BoidGenerator* BG)
{
  this->Generator= BG;
}

void Boid::InitBoid(
    MathVector bPosition, MathVector bVelocity, MathVector bDimensions,
    int _boidNumber) {
  
  flightflag = false;	      // haven't flown yet

  position = bPosition;
  velocity = bVelocity;
  dimensions = bDimensions; // width, height, length
  mass = 9.07; // 9.07 kg is approx 20 lbs.
  lastUpdate=0;
  oldVelocity= velocity;
  acceleration= MathVector(0,0,0);
  Generator=0;

  maxVelocity = 10.0;
  maxAcceleration = 0.5;
  roll = pitch = yaw = 0;
  boidType = 0;
  cruiseDistance = 2*dimensions.z; // Try to stay at least one bodywidth apart
  flockSelectively = false;

  bodyLength = bDimensions.z;
  boidNumber = _boidNumber;
}		

Boid::Boid(
    MathVector bPosition, MathVector bVelocity, MathVector bDimensions,
    int bType, int boidNumber)
{
  InitBoid(bPosition, bVelocity, bDimensions, boidNumber);
  boidType = bType;
  flockSelectively = true;
}

/*
Class BoidGenerator. This class controls and runs the whole simulation.


*/

BoidGenerator::BoidGenerator(){}
BoidGenerator::BoidGenerator(vector<int>& BoidSizes,
    vector<double>& Obstacles,
    Instant* SimulationStartTime, Instant* SimulationDuration):
    boidSizes(BoidSizes),
    freeBoidCount(BoidSizes[0]),
    simulationStart(*SimulationStartTime),
    sampleInterval(0, 2000, datetime::durationtype),
    remainingSamples(*SimulationDuration / sampleInterval),
    iterator(0),
    elapsedTime(0)
{
  makeObstacles(Obstacles);
  initSimulation();
}

BoidGenerator::~BoidGenerator()
{
  for(unsigned int i=0; i< boids.size(); ++i)
    delete[] Boid::visibilityMatrix[i];
  delete[] Boid::visibilityMatrix;
  for(unsigned int i=0; i< boids.size(); ++i)
    delete boids[i];
  for(unsigned int i=0; i< obstacles.size(); ++i)
    delete obstacles[i];
  //World is already included in obstacles. The previous statement deletes it.
  //delete world;
}

void BoidGenerator::makeObstacles(vector<double>& Obstacles)
{
  MathVector origin(mirror(Obstacles[0]), 0, Obstacles[1]);
  double radius= Obstacles[2];
  this->world= new Sphere(origin, radius);
  for(unsigned int i=3; i<Obstacles.size(); i+=3)
  {
    origin.Set(mirror(Obstacles[i]), 0, Obstacles[i+1]);
    radius= Obstacles[i+2];
    Sphere *obstacle= new Sphere(origin, radius);
    this->obstacles.push_back(obstacle);
  }
  obstacles.push_back(this->world);
}

int BoidGenerator::GetNext(
    int& BoidID, Instant& SampleTime, double& X,double& Y)
{
  if(this->iterator == boids.size())
  {
    if(! updateBoids())
      return -1; //Simulation ended (i.e., the simulation period is completed).
    iterator= 0;
  }
  BoidID= iterator;
  SampleTime.ReadFrom((int64_t)elapsedTime * 1000);
  SampleTime.SetType(datetime::durationtype);
  SampleTime.Add( &simulationStart );
  GetCurrent(X, Y);
  X= mirror(X);
  ++iterator;
  return 0;
}

bool BoidGenerator::GetCurrent(double& X,double& Y)
{
  MathVector pos= boids[iterator]->getPosition();
  X= pos.x;
  Y= pos.z;
  return true;
}

bool BoidGenerator::updateBoids()
{
  if(remainingSamples <= 0) return false;
  // increment elapsed time
  elapsedTime += sampleInterval.millisecondsToNull()/ 1000;

  for(unsigned int i = 0; i < boids.size(); i++)
    boids[i]->update(elapsedTime, sampleInterval.millisecondsToNull()/ 1000);
  --remainingSamples;
  return true;
}

bool BoidGenerator::makeBoids()
{
  bool debugme= false;
  MathVector d(1, .2, .75); // dimensions of boid (RAD, height, length)
  double mv = 20;// maximum magnitude of velocity m/s. This value means 72 km/h
  Boid* myBoid=0;
  int groupSize=0;
  double X, Y;

  for(unsigned int i=1; i< boidSizes.size(); ++i)
  {
    groupSize= boidSizes[i];
    X= (random() % int(world->Radius() * 0.75)) * pow(-1, random());
    Y= (random() % int(world->Radius() * 0.75)) * pow(-1, random());
    MathVector origin= MathVector(mirror(X), 0.0, Y) +  world->Origin();

    X= (random() % int(world->Radius() )) * pow(-1, random());
    Y= (random() % int(world->Radius() )) * pow(-1, random());
    MathVector MainAttitude= MathVector(X, 0.0, Y);
    for(int j=0; j<groupSize; ++j)
    {
      X= (random() % int(world->Radius() * 0.05));
      Y= (random() % int(world->Radius() * 0.05));
      MathVector attitude= MathVector(X, 0.0, Y) +  MainAttitude;
      MathVector v  = Direction(attitude) * (mv)/4.0;

      X= (random() % int(min(world->Radius() / 100, 200.0)));
      Y= (random() % int(min(world->Radius() / 100, 200.0)));
      MathVector pos= MathVector(mirror(X), 0.0, Y) + origin;

      myBoid = new Boid(pos, v, d, i, boids.size() + 1);
      if(debugme) cerr << "\nConstructing boid " << boids.size() + 1 << endl;
      // Set mac acceleration, and max velocity.
      myBoid->maxAcceleration = 0.65;
      //allowing more maneuver than the default 0.5
      myBoid->maxVelocity = mv;
      myBoid->SetGenerator(this);
      boids.push_back(myBoid);
    }
  }

  for(int i=0; i< this->freeBoidCount; ++i)
  {
    X= (random() % int(world->Radius() )) * pow(-1, random());
    Y= (random() % int(world->Radius() )) * pow(-1, random());
    MathVector attitude= MathVector(X, 0.0, Y);
    MathVector v  = Direction(attitude) * (mv)/4.0;

    X= (random() % int(world->Radius() * 0.75)) * pow(-1, random());
    Y= (random() % int(world->Radius() * 0.75)) * pow(-1, random());
    MathVector pos= MathVector(mirror(X), 0.0, Y) +  world->Origin();

    myBoid = new Boid(pos, v, d, -1, boids.size() + 1);
    if(debugme) cerr << "\nConstructing boid " << boids.size() + 1 << endl;
    // Set mac acceleration, and max velocity.
    myBoid->maxAcceleration = 0.65;
    //allowing more maneuver than the default 0.5
    myBoid->maxVelocity = mv;
    myBoid->SetGenerator(this);
    boids.push_back(myBoid);
  }

  return true;
}

bool BoidGenerator::initSimulation()
{
  if (! makeBoids())
  {
    cerr << "\nmakeBoids() returned an error\n";
    return false;
  }

  // Allocate the visibility matrix.
  unsigned int boidCount= this->boids.size();
  Boid::visibilityMatrix = new int*[boidCount];
  for (unsigned int i = 0; i < boidCount; i++)
    Boid::visibilityMatrix[i] = new int[boidCount];

  return true;
}



#endif // __BOID_C 
