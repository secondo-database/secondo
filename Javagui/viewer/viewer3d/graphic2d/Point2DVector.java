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

/****************************
*
*  Author   : Thomas Behr
*  Version  : 1.1
*  Date     : 16.5.2000
*
******************************/

import java.util.Vector;

/**
  * provides a vector to store points
  */
public class Point2DVector {

/** the intern store */
private Vector V;

/** creates a new Point2DVector */
public Point2DVector() { V = new Vector(); }

/** returns a copy from this */
public Point2DVector Duplicate() {
  Point2DVector Copy = new Point2DVector();
  Copy.V = (Vector) V.clone();
  return Copy;
}

/** equalize this to Source */
public void Equalize(Point2DVector Source) { V = (Vector) Source.V.clone(); }

/** returns the number of containing points */
public int getSize() { return V.size(); }

/** add a point to this vector */
public void append(Point2D Pt) { V.add(Pt); }

/** get the point on position i */
public Point2D getPoint2DAt(int i) throws IndexOutOfBoundsException {
   return ((Point2D) V.get(i));
}

/** insert a point on position i */
public void insertAt(int i,Point2D Pt) {
  V.add(i,Pt);
}

/** overwrite the point on position i with Pt */
public void setPoint2DAt(int i, Point2D Pt) {
  V.set(i,Pt);
}

/** deletes all containing points */
public void empty() { V = new Vector(); }

/** check for emptyness */
public boolean isEmpty() { return V.size()==0; }

/** check for equality */
public boolean equals(Point2DVector PK) { return PK.V.equals(V); }


}
