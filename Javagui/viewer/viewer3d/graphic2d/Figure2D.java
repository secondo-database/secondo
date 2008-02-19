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

import gui.idmanager.*;

/*************************
* Autor : Thomas Behr
* Version 1.1
* Datum : 16.5.2000
**************************/

import java.awt.Graphics;


public abstract class Figure2D {

 /** the id of this figure */
 protected   ID              myID;
 private     double          sort; 
  

 /** check for equality */
 public abstract boolean equals(Figure2D fig2);

 /** returns a copy from this */
 public abstract Figure2D duplicate();

 /** set the id of this */
 public void setID(ID newID) { myID.equalize(newID); }

 /** return the ID */
 public ID getID() { return myID; }

 /** set the sort-value */
 public void setSort(double s) { sort = s; }

 /** get the sort-value */
 public double getSort() { return sort; }


 public abstract void paint(Graphics g, boolean filled, boolean gradient);


} // class 
