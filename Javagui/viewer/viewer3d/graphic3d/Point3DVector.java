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
* Autor   : Thomas Behr
* Version : 1.1
* Datum   : 16.5.2000
**************************/


import java.util.Vector;

/** a vector containing Point3D-objects */
public class Point3DVector {
 
/** the intern store */
private Vector  V;


/** creates a new container */
public Point3DVector() {
   V = new Vector();
}


/** returns a copy of this vector */
public Point3DVector duplicate() {
   Point3DVector Kopie = new Point3DVector();
   Kopie.V = (Vector) V.clone();
   return Kopie;
 }

/** equlaize this vector to Source */
public void equalize(Point3DVector Source) {
   V = (Vector) Source.V.clone();
}

/** check for equality with PV */
public boolean equals(Point3DVector PV) {return V.equals(PV.V); }


/** add a new point to end of this vector */
public void append(Point3D P) {
   V.add(P);
}

/** remove the point on position index */
public void remove(int index) {
   V.remove(index);
}


/** check for emptyness */
public boolean isEmpty() { return V.isEmpty(); }

/** returns the number of containing points */
public int getSize() { return V.size(); }

/** returns the point on position i */
public Point3D getPoint3DAt(int i) throws IndexOutOfBoundsException {
  try {
   return (Point3D) V.get(i);
   }
  catch(Exception e) { throw new IndexOutOfBoundsException(); }
 }

/** a shortcut to getPoint3DAt **/
public Point3D get(int i){
   return getPoint3DAt(i);
}


/** set the point on position i */
public void setPoint3DAt(Point3D P, int i) throws IndexOutOfBoundsException {
   try {
     V.setElementAt(P,i);
     }
  catch(Exception e) { throw new IndexOutOfBoundsException();}
}


/** removes all containing points */
public void removePoints() { V = new Vector(); }

/** removes all containing points */
public void empty() { V = new Vector();}

}


