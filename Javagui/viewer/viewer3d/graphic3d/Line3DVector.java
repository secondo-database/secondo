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

import java.util.*;

/************************
*
* Autor   : Thomas Behr
* Version : 1.1
* Datum   : 16.5.2000
*
***************************/

/** a vector to store line-objects */
public class Line3DVector {

/** the intern store */
private Vector V = new Vector();

/** add a new Line */
public void append(Line3D L) { V.add(L); }

/** removes all lines from this vector */
public void empty() { V = new Vector(); }

/** check for emptyness */
public boolean isEmpty() { return V.size() == 0; }

/** get the number of containing lines */
public int getSize() { return V.size(); }

/** get the line on position i */
public Line3D getLine3DAt( int i) throws IndexOutOfBoundsException {

  if ( (i<0) || (i>=V.size()) ) throw new IndexOutOfBoundsException();

  return (Line3D) V.get(i); 

}

/** shortcut to getLine3DAt(i) **/
public Line3D get(int i){
   return getLine3DAt(i);
}

/** remove the line on position i */
public void remove(int i) throws IndexOutOfBoundsException {
   if ( (i<0) || (i>=V.size()) ) throw new IndexOutOfBoundsException();  
   V.remove(i);
}


}
