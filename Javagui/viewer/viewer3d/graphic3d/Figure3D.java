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

/** this class provides 3-dim-figures (points,lines,polygones) */

class Figure3D {

/** the containing points */
private Point3DVector Points;
/** the ID of this figure */
private ID MyID = IDManager.getNextID();


/** returns a readable representation of this figure */
public String toString() {
  String S = "Figure3D ";

  for (int i = 0; i< Points.getSize(); i++)
    { S += Points.getPoint3DAt(i)+ "  "; }

  return S;
}

/** returns the ID of this figure */
public ID getID(){ return MyID;}

/** set the ID of this figure */
public void setID(ID newID){ MyID.equalize(newID); }

/** creates a new empty figure */
public Figure3D() {
  Points = new Point3DVector();
}


/** equalize this to Source */
public void equalize(Figure3D Source) {
   Points = Source.Points.duplicate();
}

/** returns a copy of this figur */
public Figure3D duplicate() {
   Figure3D Copy = new Figure3D();
   Copy.equalize(this);
   return Copy;
}

/** check for equality */
public boolean equals(Figure3D Fig) {
  return Points.equals(Fig.Points);
}

/** add a point to this figure */
public void addPoint(Point3D P) { Points.append(P.duplicate()); }

/** add a point to this figure */
public void addPoint(double X,double Y , double Z, Color C) {
  Points.append(new Point3D(X,Y,Z,C));
 }

/** add a point to this figure */
public void addPoint( double X, double Y, double Z,
                      int r , int g , int b ) {
   Points.append(new Point3D(X,Y,Z,r,g,b));
}

/** computes the centre of gravity of this figure */
public Point3D centreOfGravity() {
  double   x,y,z,n; 
  Point3D  Pt;

  x = 0.0;
  y = 0.0;
  z = 0.0;
  n = 0.0;

   for(int i=0; i<Points.getSize(); i++) {
        Pt = Points.getPoint3DAt(i);
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
     return new Point3D(x,y,z,0,0,0);
     
}

/** computes the distance between P and the centre of gravity
  * of this figure
  */
public double distance(Point3D P) {
   return P.distance(centreOfGravity());
}

/** check for emptyness */
public boolean isEmpty() { return Points.isEmpty(); }

/** returns the number of containing points */
public int getSize() { return Points.getSize(); }

/** returns the point on position i */
public Point3D getPoint3DAt(int i) throws IndexOutOfBoundsException {
   return Points.getPoint3DAt(i);
}

/** set point on position i */
public void setPoint3DAt(Point3D P, int i) throws IndexOutOfBoundsException {
   Points.setPoint3DAt(P,i);
}

/** removes all containing points */
public void removePoints() { Points.removePoints(); }


}
