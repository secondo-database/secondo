
//This file is part of SECONDO.

//Copyright (C) 2004-2007, University in Hagen,
//Faculty of Mathematics and  Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
package tools;

import java.awt.geom.*;
import java.util.Vector;

public class LineSimplification{

/** Don't allow to create an instance of this class. **/
private LineSimplification(){}

/** Douglas Peuker Implementation.
  * @param points: Connected polyline represented a set of points
  * @param isUsed: result array, indicates whether the corresponding point is part
  *                of the resulting line
  * @param epsilon: maximum allowed distance to the original line
  **/

public static boolean DP( Point2D.Double[] points, boolean[] isUsed, double epsilon){
   if(points==null || isUsed==null){
      return false;
   }
   if(points.length != isUsed.length){
      return false;
   }
   if(epsilon <0){
       return false;
   }
   // initialize the used array
   for(int i=0;i<isUsed.length;i++){
       isUsed[i] = false;
   }   
   DPrec(points,isUsed,0,isUsed.length-1,epsilon*epsilon);
   return true;
}

/** recursive implementation of the Douglas Peucker algorithm. **/
private static void DPrec(Point2D.Double[] points, boolean[] isUsed, int min, int max, double epsilon){
    isUsed[min] = true;
    isUsed[max] = true;
    if((max -min)<2){ // no points between min and max
       return;
    }
    
    double maxDist = 0.0;
    int maxIndex = min;
    double x1 = points[min].getX();
    double y1 = points[min].getY();
    double x2 = points[max].getX();
    double y2 = points[max].getY();
    int mid = (max+min)/2;

    for(int i=min+1; i< max ; i++){
       double dist = Line2D.Double.ptSegDistSq(x1,y1,x2,y2,points[i].getX(),points[i].getY());
       if(dist>maxDist){
           maxDist=dist;
           maxIndex=i;
       } else if(dist==maxDist){ // choose the closed one to the mid of the interval
           int d1 = Math.abs(maxIndex-mid);
           int d2 = Math.abs(i-mid);
           if(d2<d1){
              maxDist = dist;
              maxIndex = i;
           }
       }
         
    }
    if(maxDist>epsilon){
      DPrec(points,isUsed,min,maxIndex,epsilon);
      DPrec(points,isUsed,maxIndex,max,epsilon);
    }    
}


/** Adds the simplified segments created by the polyline given in the Points to result.
  * @param thePoints: Vector containing java.awt.geom.Point2D.Double elements building a 
  *                   polyline
  * @param result: will be extended by the simplified segments (java.awt.geom.Line2D.Double)
  *                 of the simplified polyline
  * @param epsilon: maximum distance of the simplified line
  **/
public static void addSegments(Vector thePoints, Vector result, double epsilon){
  if(thePoints==null || result == null){
       return;
  }
  if(thePoints.size()<2){ // cannot be a segment
     return; 
  }

  if(epsilon<0){ // minimum distance is 0
     return;
  }

  int size = thePoints.size();
  Point2D.Double[] points = new Point2D.Double[size];
  // copy the points into the array
  for(int i=0;i<size;i++){
     points[i] = (Point2D.Double) thePoints.get(i);
  }
  boolean[] use = new boolean[size];

  // let compute the used points
  DP(points,use,epsilon);

  // build the segments
  Point2D.Double lastPoint = null;
  Point2D.Double currentPoint = null;
  for(int i=0;i<size;i++){
     if(use[i]){
          currentPoint = points[i];
          if(lastPoint!=null){
             result.add(new Line2D.Double(lastPoint,currentPoint));
          }
          lastPoint = currentPoint;
     }
  }
}


}

