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

package viewer.viewer3d.graphic2d;

/*************************
*
* Autor   : Thomas Behr
* Version : 1.1   
* Datum   : 16.5.2000
*
***************************/


import java.awt.Color;
import gui.idmanager.*;

/** this class provides 2-dim-figures (points,lines,polygones) */

public class Point2DSequence {

/** the containing points */
private Point2DVector Points;
/** the ID of this figure */
private ID MyID = IDManager.getNextID();

private double sort = 0.0;


public double getSort(){ return sort; }

public void setSort(double sort){ this.sort = sort; }



/** returns a readable representation of this figure */
public String toString() {
  String S = "Point2DSequence ";

  for (int i = 0; i< Points.getSize(); i++)
    { S += Points.getPoint2DAt(i)+ "  "; }

  return S;
}

/** returns the ID of this figure */
public ID getID(){ return MyID;}

/** set the ID of this figure */
public void setID(ID newID){ MyID.equalize(newID); }

/** creates a new empty figure */
public Point2DSequence() {
  Points = new Point2DVector();
}


/** equalize this to Source */
public void equalize(Point2DSequence Source) {
   Points = Source.Points.duplicate();
}

/** returns a copy of this figur */
public Point2DSequence duplicate() {
   Point2DSequence Copy = new Point2DSequence();
   Copy.equalize(this);
   return Copy;
}

/** check for equality */
public boolean equals(Point2DSequence Fig) {
  return Points.equals(Fig.Points);
}

/** add a point to this figure */
public void addPoint(Point2D P) { Points.append(P.duplicate()); }

/** add a point to this figure */
public void addPoint(double X,double Y , Color C) {
  Points.append(new Point2D(X,Y,C));
 }

/** add a point to this figure */
public void addPoint( double X, double Y, 
                      int r , int g , int b ) {
   Points.append(new Point2D(X,Y,r,g,b));
}


/** check for emptyness */
public boolean isEmpty() { return Points.isEmpty(); }

/** returns the number of containing points */
public int getSize() { return Points.getSize(); }

/** returns the point on position i */
public Point2D getPoint2DAt(int i) throws IndexOutOfBoundsException {
   return Points.getPoint2DAt(i);
}

/** set point on position i */
public void setPoint2DAt(Point2D P, int i) throws IndexOutOfBoundsException {
   Points.setPoint2DAt(i,P);
}

/** removes all containing points */
public void removePoints() { Points.empty(); }


}
