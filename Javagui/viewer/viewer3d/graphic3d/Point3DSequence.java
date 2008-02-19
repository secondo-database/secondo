//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

package viewer.viewer3d.graphic3d;

/*************************
*
* Autor   : Thomas Behr
* Version : 1.1   
* Datum   : 16.5.2000
*
***************************/


import java.awt.Color;
import gui.idmanager.*;
import java.util.Vector;

/** this class provides 3-dim-figures (points,lines,polygones) */

class Point3DSequence {

/** the containing points */
private Vector Points;


/** returns a readable representation of this figure */
public String toString() {
  String S = "Point3DSequence ";

  for (int i = 0; i< Points.size(); i++)
    { S += Points.get(i)+ "  "; }

  return S;
}

/** creates a new empty figure */
public Point3DSequence() {
  Points = new Vector();
}


/** equalize this to Source */
public void equalize(Point3DSequence Source) {
   Points = (Vector) Source.Points.clone();
}

/** returns a copy of this figur */
public Point3DSequence duplicate() {
   Point3DSequence Copy = new Point3DSequence();
   Copy.equalize(this);
   return Copy;
}

/** check for equality */
public boolean equals(Point3DSequence Fig) {
  return Points.equals(Fig.Points);
}

/** add a point to this figure */
public void addPoint(Point3DSimple P) { Points.add(P.duplicate()); }

/** add a point to this figure */
public void addPoint(double X,double Y , double Z, Color C) {
  Points.add(new Point3DSimple(X,Y,Z,C));
 }

/** add a point to this figure */
public void addPoint( double X, double Y, double Z,
                      int r , int g , int b ) {
   Points.add(new Point3DSimple(X,Y,Z,r,g,b));
}

/** computes the centre of gravity of this figure */
public Point3DSimple centreOfGravity() {
  double   x,y,z,n; 
  Point3DSimple  Pt;

  x = 0.0;
  y = 0.0;
  z = 0.0;
  n = 0.0;

   for(int i=0; i<Points.size(); i++) {
        Pt = (Point3DSimple) Points.get(i);
        x = x + Pt.getX();
        y = y + Pt.getY();
        z = z + Pt.getZ();
        n = n + 1.0;
     } // for

   if (n>0) {
     x = x/n;
     y = y/n;
     z = z/n;
   }
     return new Point3DSimple(x,y,z,0,0,0);
     
}

/** computes the distance between P and most far point
  * of this figure
  */
public double distance(Point3DSimple P) {
   // return P.distance(centreOfGravity());
   double dist = 0;
   for(int i=0;i<getSize();i++){
      double d = P.distance(get(i));
      if(d>dist){
         dist = d;
      }
   }
   return dist;
}

/** check for emptyness */
public boolean isEmpty() { return Points.isEmpty(); }

/** returns the number of containing points */
public int getSize() { return Points.size(); }

/** returns the point on position i */
public Point3DSimple getPoint3DAt(int i) throws IndexOutOfBoundsException {
   return (Point3DSimple) Points.get(i);
}

public Point3DSimple get(int i){
   return (Point3DSimple) Points.get(i);

}


/** set point on position i */
public void setPoint3DAt(Point3DSimple P, int i) throws IndexOutOfBoundsException {
   Points.set(i,P);
}

/** removes all containing points */
public void removePoints() { Points.clear(); }


}
