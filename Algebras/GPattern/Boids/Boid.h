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
  of this code in a commercial and/or for\-profit manner must contact the
  author via electronic mail (preferred) or other method to arrange the terms
  of usage. These terms may be as simple as giving the author visible credit
  in the final product.

  There is no warranty or other guarantee of fitness for this software,
  it is provided solely "as is". Bug reports or fixes may be sent
  to the author, who may or may not act on them as he desires.

  If you use this software the author politely requests that you inform him
  via electronic mail.

Boid.h

INTERFACE: Class for objects that move with A-Life boid\-type behavior.

(c) 1996 Christopher Kline <ckline@acm.org>

September 2011 Mahmoud Sakr: The Boids simulator/data-generator is now
available as a SECONDO operator.

*/

#ifndef __BOID_H
#define __BOID_H

#include "NamedObject.h"
#include "Obstacle.h"
#include "DateTime.h"
#include<vector>
using namespace std;

#define FREEMOVER -1
class BoidGenerator;
class Boid;

class Boid : public SimObject {
  
public:
  Boid(MathVector bPosition, MathVector bVelocity, MathVector bDimensions,
      int bType, int boidNumber);
  void InitBoid(
      MathVector bPosition, MathVector bVelocity, MathVector bDimensions,
      int _boidNumber);
/*
Updates this object based on the amount of elapsed time since the last
update, and the previous acceleration and velocity.

*/
  virtual bool
  update(const double &elapsedSeconds, const double sampleIntervalSeconds);


  void SetGenerator(BoidGenerator* BG);

/*
[m/sec] Maximum magnitude of velocity. Default value is 10

*/
  double maxVelocity;
/*
[m/(sec 2)] Maximum magnitude of acceleration as a fraction of
maxVelocity. Default value is 0.5

*/
  double maxAcceleration;
/*
[m] Desired distance from closest neighbor when flying. Default value
is twice the bodylength.

*/
  double cruiseDistance;
/*
[radians] Rotation around body-local z-axis (+roll = counterclockwise).
Default value is 0.

*/
  double roll;
/*
[radians] Rotation around body-local x-axis (+pitch = nose tilting
upward). Default value is 0.

*/
  double pitch;
/*
[radians] Rotation around body-local y-axis (increasing
counterclockwise, 0 is along body-local +z). Default value is 0.

*/
  double yaw;
/*
See description in CalculateVisibilityMatrix() and the comments for the
visMatrix macro for more info.

*/
  static int **visibilityMatrix;

  
protected:
/*
Returns the magnitude of gravitational acceleration in the (0, -1, 0)
direction [m/(sec 2)].

*/
  virtual double getGravAcceleration(void) const;		
/*
Given an accumulator and a value to add to the accumulator, this
method truncates the magnitude of valueToAdd so that, when added to the
accumulator, the magnitude of the accumulator is at most 1.0. It then
adds the truncated value to the accumulator. The value returned is the
magnitude of the accumulator after the addition.

*/
  virtual double accumulate(MathVector &accumulator, MathVector valueToAdd);
/*
Returns how far in front of boid to probe for obstacles. By default,
the probe length scales linearly from 10 times bodylength to 50 times
bodylength as the boid accelerates from 0 m/s to maxVelocity.

*/

  virtual float getProbeLength(const double sampleIntervalSeconds);

/*
Returns the speed the boid would like to travel at when not under any
other influences (i.e., obstacles, flocking desires, etc). The default
value is 1/5 of maxVelocity.

*/
  virtual double desiredCruisingSpeed(void);
/*
Each boid helps maintain a visibility matrix, which is an NxN matrix,
where N is the current number of boids (it is dynamically expanded each
time a new boid is created). Each cell [A,B] represents whether boid A can
see boid B. The contents of the matrix are described further in the
visMatrix macro in boid.c++
The reason for this matrix is to drastically reduce the computational
complexity of determining which boids are visible to the others.

*/
  virtual void calculateVisibilityMatrix(void);
/*
Returns 1 if this boid can see boid b, 0 otherwise.

*/
  virtual int visibleToSelf(Boid *b);
/*
Calculate the roll, pitch, and yaw of this boid based on its
acceleration, velocity, and position. Though position isn't necessary
for most approximations of attitude, it may be useful in some
circumstances.

*/
  virtual void calculateRollPitchYaw(MathVector appliedAcceleration,
				     MathVector currentVelocity,
				     MathVector currentPosition);
/*
Returns a vector which indicates how the boid would like to accelerate
in order to fly level (i.e., with minimal pitch).

*/
  virtual MathVector levelFlight(MathVector AccelSoFar);
/*
Returns a vector which indicates how the boid would like to accelerate
when not under any other influences. Related to desiredCruisingSpeed().

*/
  virtual MathVector wander(void);
/*
Returns a vector which indicates how the boid would like to accelerate
in order to avoid collisions with non-boid obstacles.

*/
  virtual MathVector collisionAvoidance(const double sampleIntervalSeconds);
/*
Called by CollisionAvoidance, this method attempts to avoid a collision
with a specific obstacle, and returns an acceleration vector indicating
how the boid should accelerate to achieve this end.

*/
  virtual MathVector
  resolveCollision(MathVector pointOnObject, MathVector normalToObject,
       const double sampleIntervalSeconds);
/*
Returns a vector which indicates how the boid would like to accelerate
in order to maintain a distance of cruiseDistance from the nearest
visible boid.

*/
  virtual MathVector maintainingCruisingDistance(void);
/*
Returns a vector which indicates how the boid would like to accelerate
in order to fly at approximately the same speed and direction as the
nearby boids.

*/
  virtual MathVector velocityMatching(void);
/*
Returns a vector which indicates how the boid would like to accelerate
in order to be near the center of the flock.

*/
  virtual MathVector flockCentering(void);
/*
This method prioritizes and resolves the acceleration vectors from
CollisionAvoidance(), FlockCentering(), MaintainingCruisingDistance(),
VelocityMatching(), Wander(), and LevelFlight(). It returns the actual
acceleration vector that the boid will apply to its flight in the
current time step.

*/
  virtual MathVector navigator(const double sampleIntervalSeconds);
/*
Returns the type of this boid.

*/
  virtual int getBoidType(void) const;
/*
[m] Length of the boid. By default this value is equal to the z
component of the bDimensions passed to the constructor.

*/
  double bodyLength;
/*
Unique integer identifying the number of this boid. The first boid
created is given boidNumber 1, and the values increase sequentially.

*/
  int boidNumber;
/*
Should this boid flock only with boids of the same boidType, or with
all boids? The default value is FALSE, meaning that this boid will
flock with all boids regardless of type.
Basically, should boids of a feather stick together? :)

*/
  bool flockSelectively;
/*
Identifies the type of boid for selective flocking

*/
  int boidType;

private:
/*
[m/sec] velocity at last update.

*/
  MathVector oldVelocity;
/*
[m/(sec 2)] acceleration requested at last update.

*/
  MathVector acceleration;
/*
Has the boid been updated at least once?

*/
  bool flightflag;
/*
The simulation controller. It is required that every boid has access to the
whole list of boids, which is stored only in the boidGenerator. The Boid
class is a friend of the BoidGenerator class, and has access to its private
members.

*/
  BoidGenerator* Generator;

};

/*
The BoidGenerator class controlling the simulation. The is the only interface
through which a program can use this generator.

*/

class BoidGenerator
{
public:
  friend class Boid;
  BoidGenerator(vector<int>& BoidSizes, vector<double>& Obstacles,
      Instant* SimulationStartTime, Instant* SimulationDuration);
  ~BoidGenerator();
  int  GetNext(int& BoidID, Instant& SampleTime, double& X,double& Y);
private:
/*
The boid generator assumes that the +ve X-Axis goes to the left. In contrast
Secondo assumes it goes right. The following mirroring functions translate
between the two coordinates.

*/
  inline double mirror(double&  xCoord);
  Box* Rect2Box(const pair<pair<double, double>,  pair<double, double> > rect);

  bool initSimulation();
  BoidGenerator();
  bool updateBoids();
  bool makeBoids(void);
  void makeObstacles(vector<double>& Obstacles);
  bool GetCurrent(double& X,double& Y);
  vector<int> boidSizes;
  int freeBoidCount;
  vector<Sphere*> obstacles;
  Sphere* world;
  Instant simulationStart;
  Instant sampleInterval;
  int remainingSamples;
  vector<Boid*> boids;
  unsigned int iterator;
  double elapsedTime;
};
// ------------- inline methods ---------------------------------------------

inline double BoidGenerator::mirror(double&  xCoord)
{
  return (-1 * xCoord);
}

inline Box* BoidGenerator::Rect2Box(
    const pair<pair<double, double>,  pair<double, double> >  rect)
{
/*
The rect format is (<minX, maxX>, <minY, maxY>) where X increases in the
left direction, which is compatible to the boid generator.

*/
  double minX= rect.first.first;
  double maxX= rect.first.second;
  double minY= rect.second.first;
  double maxY= rect.second.second;

  pair<double, double> bottomRight(minX, minY);
  pair<double, double> topLeft(maxX, maxY);
  Box* res= new Box(
   MathVector(topLeft.first, 1000.0, topLeft.second),
   MathVector(bottomRight.first, -1000.0, bottomRight.second));
  return res;
}

inline double
Boid::getGravAcceleration(void) const
{
  return 9.806650;
}

inline int
Boid::getBoidType(void) const
{
    return boidType;
}

inline int
Boid::visibleToSelf(Boid *b)
{
/*
find out if the boid b is within our field of view

*/
  MathVector vectorToObject = b->position - position;
/*
This isn't perfectly accurate, since we're not always facing in
the direction of our velocity, but it's close enough.

*/
  return(AngleBetween(velocity, vectorToObject) <= 1.0471967);
  // pi/3 radians is our FOV
}

inline float
Boid::getProbeLength(const double sampleIntervalSeconds)
{
/*
Mahmoud Sakr thinks that this code is a better way of computing the
ProbeLength. The original code is commented out below. The idea is to relate
the ProbeLength to the object's speed and the update rate. For example, the
boid should start doing something to avoid collisions (5 updates) before it
collides. Which means that the boid sees (max velocity * (1/update rate) * 10)
meters from its position.

*/
  double speed= this->maxVelocity;// Magnitude(this->velocity);
  float probeLengthMeters= 5 * speed * sampleIntervalSeconds;
  return probeLengthMeters;

// float maxScale = 5;
// When we're at maxVelocity, scalefactor = maxScale.
// When our velocity is 0, scalefactor = 1.
// Linearly scale in between.
// float scaleFactor = ((maxScale-1)/maxVelocity) * Magnitude(velocity) + 1;
// return 10*bodyLength*scaleFactor;
}

inline double
Boid::desiredCruisingSpeed(void) {

  return 0.2*maxVelocity;
}

inline MathVector
Boid::levelFlight(MathVector AccelSoFar) {

/*
Determine the vertical acceleration.

*/
  MathVector verticalAcc(0, AccelSoFar.y, 0);
/*
Try to negate it.

*/
  return -verticalAcc;
}

#endif // __BOID_H
