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

public class IDPoint3DVector {
 
/** the intern store */
private Vector  V;


/** creates a new vector */
public IDPoint3DVector() {
   V = new Vector();
}

/** returns a copy of this vector */
public IDPoint3DVector Duplicate() {
   IDPoint3DVector Kopie = new IDPoint3DVector();
   Kopie.V = (Vector) V.clone();
   return Kopie;
 }

/** equalize this vector to Source */
public void equalize(IDPoint3DVector Source) {
   V = (Vector) Source.V.clone();
}

/** check for equality with PV */
public boolean equals(IDPoint3DVector PV) {return V.equals(PV.V); }

/** add a new point to this vector */
public void append(IDPoint3D P) {
   V.add(P);
}

/** remove the point on given position */
public void remove(int index) {
   V.remove(index);
}

/** check for emptyness */
public boolean isEmpty() { return V.isEmpty(); }

/** get the number of containing points */
public int getSize() { return V.size(); }

/** get the point on position i */
public IDPoint3D getIDPoint3DAt(int i) throws IndexOutOfBoundsException {
  try {
   return (IDPoint3D) V.get(i);
   }
  catch(Exception e) { throw new IndexOutOfBoundsException(); }
 }

/** shortcut to getIDPoint3DAt **/
public IDPoint3D get(int i){

 return getIDPoint3DAt(i);
}

/** set the point on psotion i */
public void setIDPoint3DAt(IDPoint3D P, int i) throws IndexOutOfBoundsException {
   try {
     V.setElementAt(P,i);
     }
  catch(Exception e) { throw new IndexOutOfBoundsException();}
}


/** removes all points from this vector */
public void removePoints() { V = new Vector(); }

/** removes all points from this vector */
public void empty() { V = new Vector();}

}


