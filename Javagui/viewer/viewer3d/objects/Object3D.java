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

package viewer.viewer3d.objects;

import viewer.viewer3d.graphic3d.*;
import sj.lang.ListExpr;
import gui.idmanager.*;
import gui.SecondoObject;
import java.awt.*;

public interface Object3D{

/** returns all containing triangles of this object */
public Triangle3DVector getTriangles();
/** returns all containing lines of this object */
public Line3DVector getLines();
/** returns the containing points of this object */
public IDPoint3DVector getPoints();
/** convert SO to a Object3D */
public boolean readFromSecondoObject(SecondoObject SO);
/** return the ID of this Object */
public ID getID();
/** shows a setting dialog of this object */
public void showSettings(Frame F);
/** returns the bounding box */
public BoundingBox3D getBoundingBox();

/** returns true if a vertical line in (x,y) is in the near of this object
  * the maximal distance is given by exactness
  */
public boolean nearByXY(double x,double y,double exactness);

}


