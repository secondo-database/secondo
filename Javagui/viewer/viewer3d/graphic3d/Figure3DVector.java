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

/*****************************
*
*  Autor   : Thomas Behr
*  Version : 1.1
*  Datum   : 16.5.2000
*
*******************************/


class Figure3DVector {

/** the intern store */
 private Vector V;

/** creates a new vector */
 public Figure3DVector() { V = new Vector(); }

/** insert a new figure at end of this vector */
 public void append(Figure3D IPoly) { V.add(IPoly); }
 
/** get the number of containing figures */
 public int getSize() { return V.size(); }


/** get the figure on position i */
 public Figure3D getFigure3DAt(int i) throws NoSuchElementException {
   try {
       return (Figure3D) V.get(i);
       }
   catch (Exception e ) { throw new NoSuchElementException(); }
 }

/** removes all figures from this vector */
 public void empty() { V = new Vector(); }

/** get the position of given figure */
 public int getIndexOf(Figure3D Poly) throws NoSuchElementException {
    return V.indexOf(Poly);
 }

/** remove given figure from this vector */
 public void delete(Figure3D Poly) throws NoSuchElementException {
     V.remove( V.indexOf(Poly));
 }

/** check for emptyness */
 public boolean isEmpty() { return V.size()==0; }


/** check for equality */
public boolean equals(Figure3DVector P) { return V.equals(P.V); }



} // class
