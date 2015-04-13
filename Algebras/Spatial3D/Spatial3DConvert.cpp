/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[NP] [\newpage]
//[ue] [\"u]
//[e] [\'e]

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
 
 01590 Fachpraktikum "Erweiterbare Datenbanksysteme" 
 WS 2014 / 2015

Jens Breit, Joachim Dechow, Daniel Fuchs, Simon Jacobi, G[ue]nther Milosits, 
Daijun Nagamine, Hans-Joachim Klauke.

Betreuer: Dr. Thomas Behr, Fabio Vald[e]s


[1] Implementation of a Spatial3D algebra: 
region2volume, region2surface, mregion2volume

[TOC]

[NP]

1 Includes and Defines

*/

#include <cmath>
#include <list>
#include <algorithm>    // std::min
#include <time.h>      
#include "Spatial3D.h"
#include "Spatial3DConvert.h"
#include "Spatial3DSetOps.h"
#include "RelationAlgebra.h"
#include "Triangulate.h"
#include "../Spatial/SpatialAlgebra.h"
#include "../MovingRegion/MovingRegionAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace std;

namespace spatial3DConvert {

/*
1 Some auxiliary functions for the creations:

1.1 ~det~

Calculate the determinant of a matrix with the three points as columns.
 
*/
double det(Point& a, Point& b, Point& c){
  long double r = ((long double)a.GetX() * (long double)b.GetY() 
    + (long double)b.GetX() * (long double)c.GetY() 
    + (long double)c.GetX() * (long double)a.GetY()) 
    - ((long double)c.GetX() * (long double)b.GetY() 
    + (long double)b.GetX() * (long double)a.GetY() 
    + (long double)a.GetX() * (long double)c.GetY());
  return(double)r;
}

/*
1.2 ~pointIsBetween~

Verify wether a point lies on the line between two points.
d is the value of the determinant.
 
*/
bool pointIsBetween(double d, Point& a, Point& b, Point& c){
  double epsilon = 0.0000000001;
  if(abs(d) < epsilon && 
    ((a.GetX() < max(b.GetX(),c.GetX()) && 
    (a.GetX() > min(b.GetX(),c.GetX()))) || (a.GetY() < max(b.GetY(),c.GetY())
    && (a.GetY() > min(b.GetY(),c.GetY()))))){
    return true;
  }
  return false;
}


/*
1.3 ~intersect~

Calculates wether the two segents a-b and c-d intersect each other:
 
*/
bool intersect(Point& a, Point& b, Point& c, Point& d){
  //1. They are the same:
  if((AlmostEqual(a,c) && AlmostEqual(b,d)) || 
    (AlmostEqual(a,d) && AlmostEqual(b,c))){
    return true;
  }

  //2. One segment is left of the other
  double minX1 = min(a.GetX(),b.GetX());
  double maxX1 = max(a.GetX(),b.GetX());
  double minX2 = min(c.GetX(),d.GetX());
  double maxX2 = max(c.GetX(),d.GetX()); 
  if((maxX1 < minX2) || (minX1 > maxX2)){
    return false;
  }

  //3. One segment is above the other
  double minY1 = min(a.GetY(),b.GetY());
  double maxY1 = max(a.GetY(),b.GetY());
  double minY2 = min(c.GetY(),d.GetY());
  double maxY2 = max(c.GetY(),d.GetY()); 
  if((maxY1 < minY2) || (minY1 > maxY2)){
    return false;
  }

  //If the points are on different sides of the segment,
  //the dets have different signs.
  //If all dets are 0 then the segments are in one line, 
  //and because of 2. and 3. they overlap.
  double s1 = det(a,c,d);
  double s2 = det(b,c,d);
  double s3 = det(c,a,b);
  double s4 = det(d,a,b);

  double epsilon = 0.0000000001;

  if(
  (abs(s1) < epsilon && abs(s2) < epsilon  && 
  abs(s3) < epsilon  && abs(s4) < epsilon ) ||
  (s1 < -epsilon && s2 > epsilon && s3 < -epsilon && s4 > epsilon) ||
  (s1 < -epsilon && s2 > epsilon && s3 > epsilon && s4 < -epsilon) ||
  (s1 > epsilon && s2 < -epsilon && s3 < -epsilon && s4 > epsilon) ||
  (s1 > epsilon && s2 < -epsilon && s3 > epsilon && s4 < -epsilon)){
    return true;
  }
  
  if(
  pointIsBetween(s1,a,c,d) ||
  pointIsBetween(s2,b,c,d) ||
  pointIsBetween(s3,c,a,b) ||
  pointIsBetween(s4,d,a,b)){
    return true;
  }
  return false;
}


/*
1.4 ~segmentIntersectPolygon~

Calculates wether the segent intersect the polygon:
 
*/
bool segmentIntersectPolygon(Point& p1,Point& p2, list<Point>& list){
  std::list<Point>::iterator it = list.begin();
  Point l1 = (*it);
  Point l2 = l1; 
  ++it;
  while (it != list.end()){
    l2 = l1; 
    l1 = (*it);
    if(intersect(p1, p2, l1, l2)){
      return true;
    }
    ++it;
  }
return false;
}

/*
1.5 ~pointIsLeftOf~

Point x is left of the segment a-b

*/
bool pointIsLeftOf(Point& x, Point& a, Point& b){
  double minY = min(a.GetY(),b.GetY());
  double maxY = max(a.GetY(),b.GetY());
  if(x.GetY() < minY || x.GetY() > maxY){
    return false;
  }

  double maxX = max(a.GetX(),b.GetX());
  if(x.GetX() > maxX){
    return false;
  }

  long double q = ((long double)x.GetY() - (long double)a.GetY()) / 
                  ((long double)b.GetY() - (long double)a.GetY()); 
  long double ab = (long double)a.GetX() + 
                    q * ((long double)b.GetX() - (long double)a.GetX());
  //endpoints only count once
  if(x.GetX() < (double)ab && q != 1.0){
    return true;
  }
  return false;
}

/*
1.6 ~pointIsInsideOfThePolygon~

When a point is inside a polygon, 
then there is a uneven number of segments left of the point. 
 
*/
bool pointIsInsideOfThePolygon(Point& p, list<Point>& list){
  int count = 0;
  std::list<Point>::iterator it = list.begin();
  Point l1 = (*it);
  Point l2 = l1; 
  ++it;
  while (it != list.end()){
    l2 = l1; 
    l1 = (*it);
    if(pointIsLeftOf(p,l1,l2)){
      count++;
    }
    ++it;
  }
  if(count % 2 == 0){
    return false;
  }
  return true;
}
  
/*
1.7 ~pointsAreDiagonal~

The segment between two points of the polygon is a diagonal if it 
does not intercect other parts of the polygon and is inside of the polygon.
 
*/
bool pointsAreDiagonal(Point& p1,Point& p2, list<Point>& list){
  if(AlmostEqual(p1,p2)){
    return false;
  }
  if(segmentIntersectPolygon(p1,p2,list)){
    return false;
  }

  Point p(true, 0.5 * (p1.GetX() + p2.GetX()), 0.5 * (p1.GetY() + p2.GetY()));
  if(pointIsInsideOfThePolygon(p,list)){
    return true;
  }
  return false;
}

/*
1.9 ~cutPolygonOnPoints~

The polygon will be spitted into two polygons
 
*/
void cutPolygonOnPoints(std::list<Point>::iterator it1, 
                        std::list<Point>::iterator it2, 
                        std::list<Point>& list, std::list<Point>& newList){
  Point first = *it1;
  Point last = *it2;

  ++it1;
  std::list<Point>::iterator it;
//  while((*it1) != (*it2)){
  while((it1 != it2)){
    newList.push_back(*it1);
    it = it1;
    ++it1;
    list.erase(it);
  }
  newList.push_front(first);
  newList.push_back(last);
  newList.push_back(first);
}
  
/*
1.10 ~listIsClockwise~

The points in the list run clockwise around the polygon.
 
*/
bool listIsClockwise(list<Point>& list){
  //Gauss's area formula
  std::list<Point>::iterator it1 = list.begin();
  std::list<Point>::iterator it2 = it1;
  if(it1 != list.end()){
    ++it1;
  }
  
  double A = 0;
  while(it1 != list.end()){
    Point p1 = *it1;
    Point p2 = *it2;
    A += p1.GetX() * p2.GetY() - p1.GetY() * p2.GetX();
    it2 = it1;
    ++it1;
  }
  return A > 0;
}

/*
1.11 ~addTriangle~

Add a triangle to the TriangleContainer
 
*/
void addTriangle(TriangleContainer& res, Point p1, 
                Point p2,Point p3, bool bottom, double heigth){
  //bottom means that the triangle is clockwise oriented.
  //Gauss's area formula
  double A = (p1.GetX() * p2.GetY()) + 
            (p2.GetX() * p3.GetY()) + 
            (p3.GetX() * p1.GetY()) - 
            (p2.GetX() * p1.GetY()) - 
            (p3.GetX() * p2.GetY()) - 
            (p1.GetX() * p3.GetY());  

  if((bottom && A < 0) || (!bottom && A > 0)){
    res.add(Triangle(Point3d(p1.GetX(),p1.GetY(),heigth),
                    Point3d(p2.GetX(),p2.GetY(),heigth),
                    Point3d(p3.GetX(),p3.GetY(),heigth)));
  }else{
    res.add(Triangle(Point3d(p1.GetX(),p1.GetY(),heigth),
                    Point3d(p3.GetX(),p3.GetY(),heigth),
                    Point3d(p2.GetX(),p2.GetY(),heigth)));
  }
}

/*
1.12 ~triangulation~

Every polygon with more than tree points, has a diagonal with two corner 
pointsthat lies totaly inside the polygon. 
So it is possible to split the polygon on that points, 
~bottom~ defines whether all Triangles are clockwise orientated (true),
~heigth~ defines the z Coordinate of the points of the triangles.
 
*/
void triangulation(TriangleContainer& res, list<Point>& list, 
                  double heigth1, bool clockwise1,
                   double heigth2, bool clockwise2){
  
  if(list.size() < 4){
    res.SetDefined(false);
    return;
  }

  //put the list on a stack  
  stack< std::list<Point> > stack;
  stack.push(list);
  //do while there is a list on the stack  
  while(!stack.empty()){
    //take the first list of the stack
    std::list<Point> actualList = stack.top();
    stack.pop();
    if(actualList.size() < 4){
      cout << "The list should always have more than 3 elements!" << endl;
    }
    //the list is a triangle
    if(actualList.size() == 4){
      std::vector<Point> v;
      for (std::list<Point>::iterator it = actualList.begin(); 
          it != actualList.end(); ++it){
        v.push_back(*it);
      }
      addTriangle(res, v[0], v[1], v[2], clockwise1, heigth1);
      if(heigth2 > 0.0){
        addTriangle(res, v[0], v[1], v[2], clockwise2, heigth2);
      }
    }else{
    //the list will be splited at a diagonal  
      bool found = false;
      for (std::list<Point>::iterator it1 = actualList.begin(); 
          (it1 != actualList.end() && !found); ++it1){
        Point p1 = *it1;
        std::list<Point>::iterator it2 = it1;
        //try every possible segment
        //there must be at least one diagonal 
        while((it2 != actualList.end()) && !found){
          Point p2 = *it2;
          //Find the diagonal
          if(pointsAreDiagonal(p1, p2, actualList)){
            found = true; 
            std::list<Point> newList;
            //cut the polygon
            cutPolygonOnPoints(it1, it2, actualList, newList);
            if(actualList.size() < 4 ||newList.size() < 4){
              cout << "List should always have more than 3 elements!" << endl;
              return;
            }
            //Put the two new lists on the stack  
            stack.push(newList);
            stack.push(actualList);
            it2 == actualList.end();
            break;
          }
          if(it2 != actualList.end()){
            ++it2;
          }
        }
      }
      if(found == false){
        cout << "The triangulation does not work!" << endl;
        std::list<Point>::iterator ita = actualList.begin();
        while(ita != actualList.end()){
          cout << "Punkt X:" << (*ita).GetX() << endl;
          cout << "Punkt Y:" << (*ita).GetY() << endl << endl;
          ++ita;
        }
      }
    }
  }
}


/*
1.12 ~triangulation~

Alternative triangulation using the class HPolygon 
from algebra TransportationMode
 
*/
void triangulation2(TriangleContainer& res, vector<list<Point> >& lists,
                   double heigth1, bool clockwise1,
                   double heigth2, bool clockwise2){

  int ncontours = lists.size();
  int cntr[ncontours];
  vector<double> vertices_x;
  vector<double> vertices_y;
  
  for(int i = 0; i < ncontours;i++){
    list<Point> list = lists[i];
    cntr[i] = list.size();
    std::list<Point>::iterator it = list.begin();
    while(it != list.end()){
      Point p = *it;
      vertices_x.push_back(p.GetX());
      vertices_y.push_back(p.GetY());
      ++it;
    }
  }
  
  Copied_from_Algebra_TransportationMode::HPolygon poly;
  poly.Init2(ncontours, cntr, vertices_x,vertices_y);
cout<< "start Triangulation." << endl;
  poly.Triangulate();
cout<< "end Triangulation." << endl;
    
  for (unsigned int i=0; i< poly.mtabCell.size(); ++i){
    if(heigth2 == 0.0){
      addTriangle(res, 
        Point(true,poly.mtabPnt[poly.mtabCell[i].Index(0)].X(),
              poly.mtabPnt[poly.mtabCell[i].Index(0)].Y()), 
        Point(true,poly.mtabPnt[poly.mtabCell[i].Index(1)].X(),
              poly.mtabPnt[poly.mtabCell[i].Index(1)].Y()), 
        Point(true,poly.mtabPnt[poly.mtabCell[i].Index(2)].X(),
              poly.mtabPnt[poly.mtabCell[i].Index(2)].Y()), 
        clockwise1, heigth1);
    }else{
      addTriangle(res, 
        Point(true,poly.mtabPnt[poly.mtabCell[i].Index(0)].X(),
              poly.mtabPnt[poly.mtabCell[i].Index(0)].Y()), 
        Point(true,poly.mtabPnt[poly.mtabCell[i].Index(1)].X(),
              poly.mtabPnt[poly.mtabCell[i].Index(1)].Y()), 
        Point(true,poly.mtabPnt[poly.mtabCell[i].Index(2)].X(),
              poly.mtabPnt[poly.mtabCell[i].Index(2)].Y()), 
        clockwise1, heigth1);
      addTriangle(res, 
        Point(true,poly.mtabPnt[poly.mtabCell[i].Index(0)].X(),
              poly.mtabPnt[poly.mtabCell[i].Index(0)].Y()), 
        Point(true,poly.mtabPnt[poly.mtabCell[i].Index(1)].X(),
              poly.mtabPnt[poly.mtabCell[i].Index(1)].Y()), 
        Point(true,poly.mtabPnt[poly.mtabCell[i].Index(2)].X(),
              poly.mtabPnt[poly.mtabCell[i].Index(2)].Y()), 
        clockwise2, heigth2);
    }
  }
cout<< "leave Triangulation." << endl;
}

/*
1.13 ~addSides~

Adds the triang on the sides of the volume.
 
*/
void addSides(TriangleContainer& res, list<Point>& list, 
              double heigth, bool listIsClockwise){
  std::list<Point>::iterator it1 = list.begin();
  std::list<Point>::iterator it2 = it1;
  if(it1 != list.end()){
    ++it1;
  }
  
  while(it1 != list.end()){
    Point p1 = *it1;
    Point p2 = *it2;
cout<< "P1 x: " << p1.GetX() << endl;
cout<< "P1 y: " << p1.GetY() << endl;
cout<< "P2 x: " << p2.GetX() << endl;
cout<< "P2 y: " << p2.GetY() << endl;
    if(listIsClockwise){
      res.add(Triangle(Point3d(p2.GetX(),p2.GetY(),heigth),
                      Point3d(p1.GetX(),p1.GetY(),heigth),
                      Point3d(p1.GetX(),p1.GetY(),0)));
      res.add(Triangle(Point3d(p2.GetX(),p2.GetY(),heigth),
                      Point3d(p1.GetX(),p1.GetY(),0),
                      Point3d(p2.GetX(),p2.GetY(),0)));
    }else{
      res.add(Triangle(Point3d(p2.GetX(),p2.GetY(),heigth),
                      Point3d(p1.GetX(),p1.GetY(),0),
                      Point3d(p1.GetX(),p1.GetY(),heigth)));
      res.add(Triangle(Point3d(p2.GetX(),p2.GetY(),heigth),
                      Point3d(p2.GetX(),p2.GetY(),0),
                      Point3d(p1.GetX(),p1.GetY(),0)));
    }
    it2 = it1;
    ++it1;
  }      
}   

/*
1.14 ~triangulationToSurface~

The result of the triangulation is added to a surface.
 
*/
void triangulationToSurface(TriangleContainer& res, list<Point>& list){
  res.startBulkLoad();
  
  vector<std::list<Point> > lists;
  lists.push_back(list);
  triangulation2(res, lists, 0.0,false,0.0, false);
//  triangulation(res, list, 0.0,false,0.0, false);
  res.endBulkLoad(NO_REPAIR);
}

/*
1.14 ~triangulationToSurface~

The result of the triangulation is added to a surface.
With the use of HPolygon.
 
*/
void triangulationToSurface(TriangleContainer& res, 
                            vector<list<Point> >& lists){
  triangulation2(res, lists, 0.0,false,0.0, false);
}

/*
1.15 ~triangulationToVolume~

The result of the triangulation is added to a volume.
 
*/
void triangulationToVolume(TriangleContainer& res, list<Point>& list, 
                  double heigth){
  res.startBulkLoad();
  triangulation(res, list, 0.0,true, heigth, false);
  bool clockwise = listIsClockwise(list);
  
  addSides(res, list, heigth, clockwise);  
  res.endBulkLoad(NO_REPAIR);
}

/*
1.15 ~triangulationToVolume~

The result of the triangulation is added to a volume.
 
*/
void triangulationToVolume(TriangleContainer& res, vector<list<Point> >& lists, 
                  double heigth){
  triangulation2(res, lists, 0.0,true, heigth, false);
cout<< "Start add sides." << endl;  
  if(lists.size() > 0){
    bool clockwise = listIsClockwise(lists[0]);
cout<< "aussen" << endl;  
    addSides(res, lists[0], heigth, clockwise);
    for(unsigned int i = 1;i < lists.size(); i++){
cout<< i << endl;  
      clockwise = listIsClockwise(lists[i]);
      addSides(res, lists[i], heigth, !clockwise);
    }
  }
cout<< "End add sides." << endl;  
}

/*
1.16 ~region2list~

Builds a list of points that represent a polygon 
of the outer circle of the region.
The first and the last point of the list have to be the same.
  
For the region we assume that the edgeno are ordered in the cycle
the first  havesegment has the number of the face and 
the number of the other cycle
we assume hat every region has only one face and that 
the first halfsegment is part of the border of the face.
 
*/
void region2list(Region* r,list<Point>& list){
  int size = r->Size(); 

  if(size < 3){
    cout << "The region should have more than 2 elements!" << endl;
    return;
  }
  HalfSegment hs;
  HalfSegment hs2;
  Point rp;
  Point lp;
  Point rp2;
  Point lp2;
  Point firstpoint;
  Point nextpoint;
  
  //find first
  r->Get(0,hs);
  int faceno = hs.attr.faceno;
  int cycleno = hs.attr.cycleno;
  int edgeno = hs.attr.edgeno;
  edgeno++;

  int oldedgeno;
  lp = hs.GetLeftPoint();
  rp = hs.GetRightPoint();
  
  //find second
  //because we don't know whether the left or the right point is the beginning
  for (int i=1; i < size;i++){
    r->Get(i,hs2);
    if((hs2.attr.faceno == faceno) && 
      (hs2.attr.cycleno == cycleno) && 
      (hs2.attr.edgeno == edgeno)){
      lp2 = hs2.GetLeftPoint();
      rp2 = hs2.GetRightPoint();
      if(AlmostEqual(lp2,lp)){
        list.push_back(rp);
        list.push_back(lp);
        list.push_back(rp2);
        nextpoint = rp2;
        firstpoint = rp;
      } else if (AlmostEqual(lp2,rp)){
        list.push_back(lp);
        list.push_back(rp);
        list.push_back(rp2);
        nextpoint = rp2;
        firstpoint = lp;
      } else if (AlmostEqual(rp2,lp)){
        list.push_back(rp);
        list.push_back(lp);
        list.push_back(lp2);
        nextpoint = lp2;
        firstpoint = rp;
      } else if (AlmostEqual(rp2,rp)){
        list.push_back(lp);
        list.push_back(rp);
        list.push_back(lp2);
        nextpoint = lp2;
        firstpoint = lp;
      }
      edgeno++;
      break;
    }
  }
        
  //find next
  while(!AlmostEqual(nextpoint,firstpoint)){
    oldedgeno = edgeno;
    for (int i=0; i < size;i++){
      r->Get(i,hs);
      if((hs.attr.faceno == faceno) && 
        (hs.attr.cycleno == cycleno) && 
        (hs.attr.edgeno == edgeno)){
        lp = hs.GetLeftPoint();
        rp = hs.GetRightPoint();
        if(AlmostEqual(lp,nextpoint)){
          list.push_back(rp);
          nextpoint = rp;
        } else if (AlmostEqual(rp,nextpoint)){
          list.push_back(lp);
          nextpoint = lp;
        } else {
          break;
        }
        edgeno++;
        break;
      }
    }
    if(oldedgeno == edgeno){
      break;
    }
  }
}

/*
1.17 ~getURegionEmbFaceCount~

Returns the number of faces of an object with type URegionEmb.
 
*/
int getURegionEmbFaceCount(MRegion* mr, URegionEmb& ur){
  unsigned int  count = ur.GetSegmentsNum();
  int pos = ur.GetStartPos();
  DbArray<MSegmentData>* segments = 
          (DbArray<MSegmentData>*)mr->GetFLOB(1);

  int maxIndex = -1;
  for (unsigned int  i = 0; i < count; i++) {
    MSegmentData segment;
    segments->Get(pos + i, segment);
    maxIndex = max(maxIndex, (int)segment.GetFaceNo());
  }
  return maxIndex + 1;
}

/*
1.18 ~getURegionEmbFaceCount~

Returns the number of holes in an face of an object with type URegionEmb.
 
*/
int  getURegionEmbCycleCount(MRegion* mr, URegionEmb& ur,unsigned int faceNo){
  unsigned int  count = ur.GetSegmentsNum();
  int pos = ur.GetStartPos();
  DbArray<MSegmentData>* segments = 
          (DbArray<MSegmentData>*)mr->GetFLOB(1);

  int  maxIndex = -1;
  for (unsigned int  i = 0; i < count; i++) {
    MSegmentData segment;
    segments->Get(pos + i, segment);
    if(segment.GetFaceNo() == faceNo){
      maxIndex = max(maxIndex, (int)segment.GetCycleNo());
    }
  }
  return maxIndex + 1;
}

/*
1.19 ~uregion2listInitial~ and uregion2listFinal

Builds 2 lists of points that represent a polygon 
of the outer circle of the mregion at the begin and the end of the interval.
The first and the last point of the list have to be the same.
  
For the region we assume that the edgeno are ordered in the cycle.
We assume hat every region has only one face and that 
the face has cycle number 0. Cycle with other numbers represent holes.
 
*/
void uregion2listInitial(
  MRegion* mr, 
  URegionEmb& ur,
  list<Point>& listInitial, 
  unsigned int faceNo, 
  unsigned int cycleNo){
  
  unsigned int  count = ur.GetSegmentsNum();
  int pos = ur.GetStartPos();
  DbArray<MSegmentData>* segments = 
          (DbArray<MSegmentData>*)mr->GetFLOB(1);

  unsigned int segmentno = 0;
  unsigned int startsegment = 0;

  while(true){
    for (unsigned int  i = 0; i < count; i++) {
      startsegment = segmentno;
      MSegmentData segment;
      segments->Get(pos + i, segment);
      if(segment.GetFaceNo() == faceNo && 
        segment.GetCycleNo() == cycleNo && 
        segment.GetSegmentNo() == segmentno){
        if(segmentno == 0){
          listInitial.push_back(
            Point(true,segment.GetInitialStartX(),segment.GetInitialStartY()));
        }
        if(!segment.GetPointInitial()){
          listInitial.push_back(
            Point(true,segment.GetInitialEndX(),segment.GetInitialEndY()));
        }
        segmentno++;
      }
    }
    if((segmentno == count) || (startsegment == segmentno)){
      return;
    }
  }
}

void uregion2listFinal(
  MRegion* mr, 
  URegionEmb& ur,
  list<Point>& listFinal, 
  unsigned int faceNo, 
  unsigned int cycleNo){
  
  unsigned int  count = ur.GetSegmentsNum();
  int pos = ur.GetStartPos();
  DbArray<MSegmentData>* segments = 
          (DbArray<MSegmentData>*)mr->GetFLOB(1);

  unsigned int segmentno = 0;
  unsigned int startsegment = 0;

  while(true){
    for (unsigned int  i = 0; i < count; i++) {
      startsegment = segmentno;
      MSegmentData segment;
      segments->Get(pos + i, segment);
      if(segment.GetFaceNo() == faceNo && 
        segment.GetCycleNo() == cycleNo && 
        segment.GetSegmentNo() == segmentno){
        if(segmentno == 0){
          listFinal.push_back(
            Point(true,segment.GetFinalStartX(),segment.GetFinalStartY()));
        }
        if(!segment.GetPointFinal()){
          listFinal.push_back(
            Point(true,segment.GetFinalEndX(),segment.GetFinalEndY()));
        }
        segmentno++;
      }
    }
    if((segmentno == count) || (startsegment == segmentno)){
      return;
    }
  }
} 

/*
1.20 ~addMSides~

Adds the triangle on the sides of the volume build from a MRegion object.
 
*/ 
void addMSides(
  MRegion* mr, 
  URegionEmb& ur,
  TriangleContainer& res, 
  double heigthInitial, 
  double heigthFinal, 
  bool listIsClockwise, 
  unsigned int faceNo, 
  unsigned int cycleNo){
  
  unsigned int  count = ur.GetSegmentsNum();
  int pos = ur.GetStartPos();
  DbArray<MSegmentData>* segments = 
          (DbArray<MSegmentData>*)mr->GetFLOB(1);

  unsigned int segmentno = 0;
  unsigned int startsegment = 0;

  while(true){
    for (unsigned int  i = 0; i < count; i++) {
      startsegment = segmentno;
      MSegmentData segment;
      segments->Get(pos + i, segment);
      if(segment.GetFaceNo() == faceNo && 
        segment.GetCycleNo() == cycleNo && 
        segment.GetSegmentNo() == segmentno){
        //build triangle for sides
        if(!segment.GetPointInitial()){
          if(listIsClockwise){
            res.add(Triangle(
              Point3d(segment.GetFinalStartX(),
                      segment.GetFinalStartY(),
                      heigthFinal),
              Point3d(segment.GetInitialEndX(),
                      segment.GetInitialEndY(),
                      heigthInitial),
              Point3d(segment.GetInitialStartX(),
                      segment.GetInitialStartY(),
                      heigthInitial)));
          }else{
            res.add(Triangle(
              Point3d(segment.GetFinalStartX(),
                      segment.GetFinalStartY(),
                      heigthFinal),
              Point3d(segment.GetInitialStartX(),
                      segment.GetInitialStartY(),
                      heigthInitial),
              Point3d(segment.GetInitialEndX(),
                      segment.GetInitialEndY(),
                      heigthInitial)));
          }
        }
        if(!segment.GetPointFinal()){
          if(listIsClockwise){
            res.add(Triangle(
              Point3d(segment.GetFinalStartX(),
                      segment.GetFinalStartY(),
                      heigthFinal),
              Point3d(segment.GetFinalEndX(),
                      segment.GetFinalEndY(),
                      heigthFinal),
              Point3d(segment.GetInitialEndX(),
                      segment.GetInitialEndY(),
                      heigthInitial)));
          }else{
            res.add(Triangle(
              Point3d(segment.GetFinalStartX(),
                      segment.GetFinalStartY(),
                      heigthFinal),
              Point3d(segment.GetInitialEndX(),
                      segment.GetInitialEndY(),
                      heigthInitial),
              Point3d(segment.GetFinalEndX(),
                      segment.GetFinalEndY(),
                      heigthFinal)));
          }
        }
        segmentno++;
      }
    }
    if((segmentno == count) || (startsegment == segmentno)){
      return;
    }
  } 
}

/*
1.21 ~region2surface~

Main auxiliary function for the operator ~region2surface~.
 
*/ 
void region2surfaceOld(Region* r, Surface3d& res){

  //A region could have n faces
  vector<Region*> components;
  r->Components(components );
  //Every region in components has now only 1 face
  std::vector<Region*>::iterator it = components.begin();
  Surface3d* faces = new Surface3d(1);
  faces->clear();
  faces->startBulkLoad();
  Surface3d* holes = new Surface3d(1);
  holes->clear();
  holes->startBulkLoad();
  while (it != components.end()){
    std::list<Point> list;
    region2list((*it),list);
    Surface3d* face = new Surface3d(1);
    triangulationToSurface(*face, list);
    //Every face could have n holes
    Region* rholes = new Region(1);
    (*it)->getHoles(*rholes); 
    vector<Region*> holecomponents;
    rholes->Components(holecomponents);
    std::vector<Region*>::iterator ith = holecomponents.begin();
    while (ith != holecomponents.end()){
      std::list<Point> listh;
      region2list((*ith),listh);
      Surface3d* hole = new Surface3d(1);
      triangulationToSurface(*hole, listh);
      int sizeh = hole->size();
      for(int i = 0; i < sizeh; i++){
        holes->add(hole->get(i));
      }
      delete hole;
      ++ith;
    }
    for(std::vector<Region*>::iterator ithc = holecomponents.begin();
        ithc != holecomponents.end();++ithc){
      delete (*ithc);
    }
    delete rholes;
    int size = face->size();
    for(int i = 0; i < size; i++){
      faces->add(face->get(i));
    }
    delete face;    
    ++it;
  }
  for(std::vector<Region*>::iterator itc = components.begin();
    itc != components.end();++itc){
    delete (*itc);
  }
  holes->endBulkLoad(NO_REPAIR);
  faces->endBulkLoad(NO_REPAIR);

  //ToDo: wenn Jens und Simon mit performSetOperation
  // fertig sind aktivieren. Bis dahin geht es nur ohne Loecher!
  bool performSetOperationWorks = false;
  if(holes->size() > 0 && performSetOperationWorks){
    spatial3DSetOps::performSetOperation (
      spatial3DSetOps::set_minus, *faces, *holes, res);
  }else{
    res.clear();
    res.startBulkLoad();
    int size = faces->size();
    for(int i = 0; i < size; i++){
      res.add(faces->get(i));
    }
    res.endBulkLoad(NO_REPAIR);
  }
  delete holes;    
  delete faces;    
}

/*
1.21 ~region2surface~

Main auxiliary function for the operator ~region2surface~.
 
*/ 
void region2surface(Region* r, Surface3d& res){

  res.clear();
  res.startBulkLoad();
  //A region could have n faces
  vector<Region*> components;
  r->Components(components );
  //Every region in components has now only 1 face
  std::vector<Region*>::iterator it = components.begin();
  Surface3d* faces = new Surface3d(1);
  faces->clear();
  faces->startBulkLoad();
  Surface3d* holes = new Surface3d(1);
  holes->clear();
  holes->startBulkLoad();
  int count = 1;
  while (it != components.end()){
    vector<std::list<Point> > lists;
    std::list<Point> list;
    region2list((*it),list);
    lists.push_back(list);
    //Every face could have n holes
    Region* rholes = new Region(1);
    (*it)->getHoles(*rholes); 
    vector<Region*> holecomponents;
    rholes->Components(holecomponents);
    std::vector<Region*>::iterator ith = holecomponents.begin();
    while (ith != holecomponents.end()){
      std::list<Point> listh;
      region2list((*ith),listh);
      lists.push_back(listh);
      ++ith;
    }
    triangulationToSurface(res, lists);
    cout << "End Triangulation: " << count++ << endl;
    for(std::vector<Region*>::iterator ithc = holecomponents.begin();
        ithc != holecomponents.end();++ithc){
      delete (*ithc);
    }
    delete rholes;
    ++it;
  }
  for(std::vector<Region*>::iterator itc = components.begin();
    itc != components.end();++itc){
    delete (*itc);
  }

  res.endBulkLoad(NO_REPAIR);
}

/*
1.22 ~region2volume~

Main auxiliary function for the operator ~region2volume~.
 
*/ 
void region2volumeOld(Region* r,double h, Volume3d& res){

  //A region could have n faces
  vector<Region*> components;
  r->Components(components );
  //Every region in components has now only 1 face
  std::vector<Region*>::iterator it = components.begin();
  Volume3d* faces = new Volume3d(1);
  faces->clear();
  faces->startBulkLoad();
  Volume3d* holes = new Volume3d(1);
  holes->clear();
  holes->startBulkLoad();
  while (it != components.end()){
    std::list<Point> list;
    region2list((*it),list);
    Volume3d* face = new Volume3d(1);
    triangulationToVolume(*face, list, h);
    //Every face could have n holes
    Region* rholes = new Region(1);
    (*it)->getHoles(*rholes); 
    vector<Region*> holecomponents;
    rholes->Components(holecomponents);
    std::vector<Region*>::iterator ith = holecomponents.begin();
    while (ith != holecomponents.end()){
      std::list<Point> listh;
      region2list((*ith),listh);
      Volume3d* hole = new Volume3d(1);
      triangulationToVolume(*hole, listh, h);
      int sizeh = hole->size();
      for(int i = 0; i < sizeh; i++){
        holes->add(hole->get(i));
      }
      delete hole;
      ++ith;
    }
    for(std::vector<Region*>::iterator ithc = holecomponents.begin();
        ithc != holecomponents.end();++ithc){
      delete (*ithc);
    }
    delete rholes;
    int size = face->size();
    for(int i = 0; i < size; i++){
      faces->add(face->get(i));
    }
    delete face;    
    ++it;
  }
  for(std::vector<Region*>::iterator itc = components.begin();
    itc != components.end();++itc){
    delete (*itc);
  }
  holes->endBulkLoad(NO_REPAIR);
  faces->endBulkLoad(NO_REPAIR);

  //ToDo: wenn Jens und Simon mit performSetOperation
  // fertig sind aktivieren. Bis dahin geht es nur ohne Loecher!
  bool performSetOperationWorks = false;
  if(holes->size() > 0 && performSetOperationWorks){
    spatial3DSetOps::performSetOperation (
      spatial3DSetOps::set_minus, *faces, *holes, res);
  }else{
    res.clear();
    res.startBulkLoad();
    int size = faces->size();
    for(int i = 0; i < size; i++){
      res.add(faces->get(i));
    }
    res.endBulkLoad(NO_REPAIR);
  }
  delete holes;    
  delete faces;    
}


/*
1.22 ~region2volume~

Main auxiliary function for the operator ~region2volume~.
 
*/ 
void region2volume(Region* r,double h, Volume3d& res){
  res.clear();
  res.startBulkLoad();

  //A region could have n faces
  vector<Region*> components;
  r->Components(components );
  //Every region in components has now only 1 face
  std::vector<Region*>::iterator it = components.begin();
  Surface3d* faces = new Surface3d(1);
  faces->clear();
  faces->startBulkLoad();
  Surface3d* holes = new Surface3d(1);
  holes->clear();
  holes->startBulkLoad();
  while (it != components.end()){
    std::list<Point> list;
    region2list((*it),list);
    vector<std::list<Point> > lists;
    lists.push_back(list);
    //Every face could have n holes
    Region* rholes = new Region(1);
    (*it)->getHoles(*rholes); 
    vector<Region*> holecomponents;
    rholes->Components(holecomponents);
    std::vector<Region*>::iterator ith = holecomponents.begin();
    while (ith != holecomponents.end()){
      std::list<Point> listh;
      region2list((*ith),listh);
      lists.push_back(listh);
      ++ith;
    }
    triangulationToVolume(res, lists, h);
    for(std::vector<Region*>::iterator ithc = holecomponents.begin();
        ithc != holecomponents.end();++ithc){
      delete (*ithc);
    }
    delete rholes;
    ++it;
  }
  for(std::vector<Region*>::iterator itc = components.begin();
    itc != components.end();++itc){
    delete (*itc);
  }

  res.endBulkLoad(NO_REPAIR);
}

/*
1.23 ~mregion2volume~

Main auxiliary function for the operator ~mregion2volume~.
 
*/ 
void mregion2volumeOld(MRegion* mr,double h, Volume3d& res){
  
  Volume3d* faces = new Volume3d(1);
  faces->clear();
  faces->startBulkLoad();
  Volume3d* holes = new Volume3d(1);
  holes->clear();
  holes->startBulkLoad();
  
  int noComponents = mr->GetNoComponents();
  double startHeigth = 0.0; 
  
  for(int componentNo = 0; componentNo <  noComponents; componentNo++){
    URegionEmb ur;
    mr->Get(componentNo,ur);

    Instant duration = ur.getTimeInterval().end - ur.getTimeInterval().start;
    double heigth = h * ((double)duration.GetDay() +
                    (double)duration.GetAllMilliSeconds() / (double)86400000);  

    int noFaces = getURegionEmbFaceCount(mr, ur);
    for(int faceNo = 0; faceNo <  noFaces; faceNo++){
      //the face 
      list<Point> listInitial;
      uregion2listInitial(mr, ur,listInitial, faceNo, 0);
      bool clockwise = listIsClockwise(listInitial);
      list<Point> listFinal;
      uregion2listFinal(mr, ur,listFinal, faceNo, 0);
      addMSides(mr, ur,*faces, startHeigth, 
                startHeigth + heigth, clockwise, faceNo, 0);
      if(componentNo == 0){
        triangulation(*faces, listInitial, startHeigth, true, 0.0, false);
      }
      if(componentNo == (noComponents - 1)){
        triangulation(*faces, listFinal, startHeigth + heigth, false,
                      0.0, false);
      }
      //the holes
      int noCycles = getURegionEmbCycleCount(mr, ur, faceNo);
      for(int cycleNo = 1; cycleNo <  noCycles; cycleNo++){
        list<Point> listHoleInitial;
        uregion2listInitial(mr, ur,listHoleInitial, faceNo, cycleNo);
        bool clockwise = listIsClockwise(listHoleInitial);
        list<Point> listHoleFinal;
        uregion2listFinal(mr, ur,listHoleFinal, faceNo, cycleNo);
        addMSides(mr, ur,*holes, startHeigth, startHeigth + heigth, 
                  clockwise, faceNo, cycleNo);
        if(componentNo == 0){
          triangulation(*holes, listHoleInitial, startHeigth, true, 
                        0.0, false);
        }
        if(componentNo == (noComponents - 1)){
          triangulation(*holes, listHoleFinal, startHeigth + heigth, false,
                        0.0, false);
        }
      }
      startHeigth += heigth;
    }
  }  

  holes->endBulkLoad(NO_REPAIR);
  faces->endBulkLoad(NO_REPAIR);


  //ToDo: wenn Jens und Simon mit performSetOperation
  // fertig sind aktivieren. Bis dahin geht es nur ohne Loecher!
  bool performSetOperationWorks = false;
  if(holes->size() > 0 && performSetOperationWorks){
    spatial3DSetOps::performSetOperation (
      spatial3DSetOps::set_minus, *faces, *holes, res);
  }else{
    res.clear();
    res.startBulkLoad();
    int size = faces->size();
    for(int i = 0; i < size; i++){
      res.add(faces->get(i));
    }
    res.endBulkLoad(NO_REPAIR);
  }
  delete holes;    
  delete faces;    
}

/*
1.23 ~mregion2volume~

Main auxiliary function for the operator ~mregion2volume~.
 
*/ 
void mregion2volume(MRegion* mr,double h, Volume3d& res){
  
  res.clear();
  res.startBulkLoad();  
  int noComponents = mr->GetNoComponents();
  double startHeigth = 0.0; 
  for(int componentNo = 0; componentNo <  noComponents; componentNo++){
    URegionEmb ur;
    mr->Get(componentNo,ur);

    vector<std::list<Point> > initialLists;
    vector<std::list<Point> > finalLists;
    Instant duration = ur.getTimeInterval().end - ur.getTimeInterval().start;
    double heigth = h * ((double)duration.GetDay() +
                    (double)duration.GetAllMilliSeconds() / (double)86400000);  

    int noFaces = getURegionEmbFaceCount(mr, ur);
    for(int faceNo = 0; faceNo <  noFaces; faceNo++){
      //the face 
      list<Point> listInitial;
      uregion2listInitial(mr, ur,listInitial, faceNo, 0);
      bool clockwise = listIsClockwise(listInitial);
      list<Point> listFinal;
      uregion2listFinal(mr, ur,listFinal, faceNo, 0);
      addMSides(mr, ur,res, startHeigth, 
                startHeigth + heigth, clockwise, faceNo, 0);
      initialLists.push_back(listInitial);
      finalLists.push_back(listFinal);
      //the holes
      int noCycles = getURegionEmbCycleCount(mr, ur, faceNo);
      for(int cycleNo = 1; cycleNo <  noCycles; cycleNo++){
        list<Point> listHoleInitial;
        uregion2listInitial(mr, ur,listHoleInitial, faceNo, cycleNo);
        bool clockwise = listIsClockwise(listHoleInitial);
        list<Point> listHoleFinal;
        uregion2listFinal(mr, ur,listHoleFinal, faceNo, cycleNo);
        addMSides(mr, ur,res, startHeigth, startHeigth + heigth, 
                  !clockwise, faceNo, cycleNo);
        initialLists.push_back(listHoleInitial);
        finalLists.push_back(listHoleFinal);
      }
      if(componentNo == 0){
        triangulation2(res, initialLists, startHeigth, true, 0.0, false);
      }
      if(componentNo == (noComponents - 1)){
        triangulation2(res, finalLists, startHeigth + heigth, false,
                      0.0, false);
      }
      startHeigth += heigth;
    }
  }  
  res.endBulkLoad(NO_REPAIR);
}

/*
10 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

10.1 Type mapping functions

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

10.1.1 Type mapping function Spatial3DRegion2SurfaceMap.

This type mapping function is used for the ~region2surface~ operator.

*/
ListExpr Spatial3DRegion2SurfaceMap( ListExpr args )
{
  if ( nl->ListLength( args ) != 1 )
  { ErrorReporter::ReportError("wrong number of arguments (1 expected)");
    return nl->TypeError();
  }
  ListExpr arg1 = nl->First(args);

  if(nl->AtomType(arg1)!=SymbolType){
    ErrorReporter::ReportError("region expected");
    return nl->TypeError();
  }
  string st = nl->SymbolValue(arg1);
  if( st!=Region::BasicType()){
    ErrorReporter::ReportError("region expected");
    return nl->TypeError();
  }

  st = Surface3d::BasicType();
  return nl->SymbolAtom(st);
}
  
/*  
10.1.2 Type mapping function Spatial3DRegion2VolumeMap

This type mapping function is used for the ~region2volume~ 
operator.

*/
ListExpr Spatial3DRegion2VolumeMap( ListExpr args )
{
  if ( nl->ListLength( args ) != 2 )
  { ErrorReporter::ReportError("wrong number of arguments (2 expected)");
    return nl->TypeError();
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if(nl->AtomType(arg1)!=SymbolType){
    ErrorReporter::ReportError("region x real expected");
    return nl->TypeError();
  }
  string st = nl->SymbolValue(arg1);
  if( st!=Region::BasicType()){
    ErrorReporter::ReportError("region x real expected");
    return nl->TypeError();
  }

  if(!nl->IsEqual(arg2,CcReal::BasicType())){
    ErrorReporter::ReportError("region x real expected");
    return nl->TypeError();
  }    

  st = Volume3d::BasicType();
  return nl->SymbolAtom(st);
}
  
  
/*  
10.1.3 Type mapping function Spatial3DMRegion2VolumeMap

This type mapping function is used for the ~mregion2volume~ 
operator.

*/
ListExpr Spatial3DMRegion2VolumeMap( ListExpr args )
{
  if ( nl->ListLength( args ) != 2 )
  { ErrorReporter::ReportError("wrong number of arguments (2 expected)");
    return nl->TypeError();
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if(nl->AtomType(arg1)!=SymbolType){
    ErrorReporter::ReportError("mregion x real expected");
    return nl->TypeError();
  }
  string st = nl->SymbolValue(arg1);
  if( st!=MRegion::BasicType()){
    ErrorReporter::ReportError("mregion x real expected");
    return nl->TypeError();
  }

  if(!nl->IsEqual(arg2,CcReal::BasicType())){
    ErrorReporter::ReportError("mregion x real expected");
    return nl->TypeError();
  }    

  st = Volume3d::BasicType();
  return nl->SymbolAtom(st);
}
  
/*
10.4 Value mapping functions
  
10.4.1 Value mapping functions of operator ~createCube~

*/
int Spatial3DRegion2Surface( Word* args, Word& result, int message,
                    Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  Surface3d* res = static_cast<Surface3d*>(result.addr);
  Region* r = static_cast<Region*>(args[0].addr);
  if(!r->IsDefined()){
      res->SetDefined(false);
      return 0;
  }
  region2surface(r,*res);
  return 0;
}
  
int Spatial3DRegion2Volume( Word* args, Word& result, int message,
                    Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  Volume3d* res = static_cast<Volume3d*>(result.addr);
  CcReal* ch = static_cast<CcReal*>(args[1].addr);
  Region* r = static_cast<Region*>(args[0].addr);
  
  if(!r->IsDefined() || !ch->IsDefined()){
      res->SetDefined(false);
      return 0;
  }
  
  double h = ch->GetRealval();
  region2volume(r,h,*res);
  return 0;
}
  
int Spatial3DMRegion2Volume( Word* args, Word& result, int message,
                    Word& local, Supplier s ){
  result = qp->ResultStorage(s);
  Volume3d* res = static_cast<Volume3d*>(result.addr);
  CcReal* ch = static_cast<CcReal*>(args[1].addr);
  MRegion* r = static_cast<MRegion*>(args[0].addr);
  
  if(!r->IsDefined() || !ch->IsDefined()){
      res->SetDefined(false);
      return 0;
  }
  
  double h = ch->GetRealval();
  mregion2volume(r,h,*res);
  return 0;
}

/*
10.5 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

10.5.2 Definition of specification strings

*/

const string Spatial3DSpecRegion2Surface  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(region) -> surface3d</text--->"
  "<text> region2surface(r)</text--->"
  "<text> creates a surface3d from a region</text--->"
  "<text> query region2surface(region)</text--->"
  ") )";

const string Spatial3DSpecRegion2Volume  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(region) -> volume3d</text--->"
  "<text> region2volume(r,h)</text--->"
  "<text> creates a volume3d with heigth h from a region</text--->"
  "<text> query region2volume(region,h)</text--->"
  ") )";

const string Spatial3DSpecMRegion2Volume  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mregion) -> volume3d</text--->"
  "<text> mregion2volume(r,h)</text--->"
  "<text> creates a volume3d from a mregion</text--->"
  "<text> query mregion2volume(mregion,h)</text--->"
  ") )";

/*
10.5.3 Definition of the operators

*/
 
  Operator* getRegion2SurfacePtr(){
    return new Operator(
    "region2surface",
    Spatial3DSpecRegion2Surface,
    Spatial3DRegion2Surface,
    Operator::SimpleSelect,
    Spatial3DRegion2SurfaceMap
   );
  }
 
  Operator* getRegion2VolumePtr(){
    return new Operator(
    "region2volume",
    Spatial3DSpecRegion2Volume,
    Spatial3DRegion2Volume,
    Operator::SimpleSelect,
    Spatial3DRegion2VolumeMap
   );
  }
  
  Operator* getMRegion2VolumePtr(){
    return new Operator(
    "mregion2volume",
    Spatial3DSpecMRegion2Volume,
    Spatial3DMRegion2Volume,
    Operator::SimpleSelect,
    Spatial3DMRegion2VolumeMap
   );
  }
}
  
