/*
 
 see Lines.h for documentation
 
 \tableofcontents
 
*/
#include "RegionInterpolator.h"

using namespace std;

namespace RegionInterpol
{  
/*

1 LineWA

1.1 Constructors and Destructor

*/
 LineWA :: LineWA() 
{
    x = -1.0;
    y = -1.0;
    angle = -1.0;
}

LineWA :: LineWA(double xp, double yp) 
{
    x = xp;
    y = yp;
   angle = -1.0;
}

LineWA :: LineWA(double xp, double yp, double a) 
{
    x = xp;
    y = yp;
    angle = a;
}

LineWA :: LineWA(LineWA* original) 
{
    x = original->getX();
    y = original->getY();
    angle = original->getAngle();
}
   
LineWA :: LineWA(CHLine *line)
{
      x = line->getX();
      y = line->getY();
      angle = line->getAngle();
}
   
LineWA :: ~LineWA()
{
} 
/*

1.1 Get functions

1.1.1 getX()

*/  
double LineWA :: getX()
{
   return(x);
}
/*

1.1.1 getY()

*/  
double LineWA :: getY()
{
   return(y);
}
/*

1.1.1 getAngle()

*/    
double LineWA :: getAngle()
{
   return(angle);
}
/*

1.1 Set functions

1.1.1 setX()

*/
void LineWA :: setX(double x)
{
   this->x = x;
}
/*

1.1.1 setY()

*/  
void LineWA :: setY(double y)
{
   this->y = y;
}
/*

1.1.1 setAngle()

*/    
void LineWA :: setAngle(double angle)
{
   this->angle = angle;
}
/*

1.1 Public Methods

1.1.1 compareTo()

*/   
int LineWA :: compareTo(LineWA*   other) 
{    
    if (other->getAngle() < angle) return(1);
    if (other->getAngle() == angle) return(0);
   return(-1);
}
/*

1.1.1 equals()
 
*/  
bool LineWA :: equals(LineWA* other) 
{    
    if ((AlmostEqual(other->getX() , x)) && (AlmostEqual(other->getY() , y))) 
    {
      return(true);
    } 
    else return(false);
}
/*

1.1 Operators

1.1.1 $<<$

*/   
ostream& operator << (ostream& s, LineWA line)
{
   s << "(" << setw(6) << line.getX() << "; " << setw(6)
   << line.getY() << ") " << fixed << setw(7) << line.getAngle() * 180 / M_PI 
   << "Degree" << endl;
   return s;
}
/*

1.1.1 $<$
 
*/
bool operator < (const LineWA &l1,const  LineWA &l2)

{
  if (l1.angle < l2.angle) return true;
  return false;
}

//virtual void LineWA::operator = (LineWA &arg)
//{
//  x = arg.getX();
//  y = arg.getY();
//  angle = arg.getAngle();
//}
/*
 
1 LineDist

1.1 Constructors and Destructor

*/  
LineDist :: LineDist() 
{
    x = -1.0;
    y = -1.0;
   distance = numeric_limits<double>::quiet_NaN();
}

LineDist :: LineDist(double xp, double yp) 
{
    x = xp;
    y = yp;
    distance = numeric_limits<double> :: quiet_NaN();
}

LineDist :: LineDist(LineWA* original, double distance) 
{
    x = original->getX();
    y = original->getY();
    this->distance = distance;
}

LineDist :: ~LineDist()
{
}
/*

1.1 Get functions

1.1.1 getX()

*/    
 double LineDist :: getX()
{
   return(x);
}
/*

1.1.1 getY()
 
*/  
double LineDist :: getY()
{
   return(y);
}
/*

1.1.1 getDistance()
 
*/    
double LineDist :: getDistance()
{
   return(distance);
}
/*

1.1 Set functions

1.1.1 setX()

*/
void LineDist :: setX(double x)
{
   this->x = x;
}
/*

1.1.1 setY()
 
*/  
void LineDist :: setY(double y)
{
   this->y = y;
}
/*

1.1.1 setDistance()
 
*/    
void LineDist :: setDistance(double distance)
{
   this->distance = distance;
}
/*

1.1 Public Methods

1.1.1 compareTo()

*/   
int LineDist :: compareTo(LineDist*  other) 
{    
    if (other->getDistance() < distance) return(1);
    if (other->getDistance() == distance) return(0);
    return(-1);
}
/*

1.1 Operators

1.1.1 $<<$

*/  
ostream& operator << (ostream& s, LineDist line)
{
   s << "(" << setw(6) << line.getX() << "; " << setw(6)
   << line.getY() << ") " << fixed << setw(7) << line.getDistance() << endl;
   return s;
}
/*

1.1.1 $<$
  
*/
bool operator < (const LineDist &l1, const  LineDist &l2)
{
  if (l1.distance < l2.distance) return true;
  return false;
}
/*
 
1 CHLine

1.1 Constructors and Destructor

*/        
CHLine :: CHLine()
{
   LineWA();
   child = NULL;
}
    
CHLine :: CHLine(double x, double y) : LineWA(x, y)
{
   child = NULL;
}
    
CHLine :: CHLine(LineWA* newLine) : LineWA(newLine)
{     
   child = NULL;
}
/*

1.1 Get functions

1.1.1 getChild()

*/   
ConvexHullTreeNode* CHLine :: getChild()
{
   return(child);
}
/*

1.1 Set functions

1.1.1 setChild()

*/ 
void CHLine :: setChild(ConvexHullTreeNode* child)
{
   this->child = child;
}

//virtual void operator = (CHLine &arg)
//{
//  setX( arg.getX());
//  setY = arg.getY();
//  setAngle = arg.getAngle();
//  setChild(arg.getChild());
//}
/*

1.1 Operators

1.1.1 $<<$

*/   
ostream& operator << (ostream& s, CHLine line)
{
   s << "(" << setw(6) << line.getX() << "; " << setw(6)
   << line.getY() << ") " << fixed << setw(7) << line.getAngle() * 180 / M_PI 
   << "Degree" << endl;
   return s;
}

/*
 
1 PointWNL

1.1 Constructors and Destructor
 
*/
PointWNL :: PointWNL() : LineWA()
{
   t = 0;
}

PointWNL :: PointWNL(double x, double y, int t) : LineWA(x, y)
{
   this->t = t;   
}

PointWNL :: ~PointWNL()
{
}

PointWNL :: PointWNL(LineWA* line, int time) : LineWA(line)
{
   this->t = time;
}
/*

1.1 Get functions

1.1.1 getT()

*/
int PointWNL :: getT()
{
   return(t);
}
/*

1.1 Public Methods

1.1.1 equals()

*/
bool PointWNL :: equals(PointWNL *other)
{
   return(AlmostEqual(this->getX(), other->getX()) && 
      AlmostEqual(this->getY(), other->getY()) && t == other->getT());
}
/*

1.1 Operators

1.1.1 $<<$

*/
ostream& operator << (ostream& s, PointWNL p)
{
   s << "(" << setw(6) << p.getX() << "; " << setw(6) << p.getY() 
   << "; " << p.getT() << ") " << endl;
   return s;
}

}
   

