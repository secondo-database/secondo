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

import java.awt.Color;
import gui.idmanager.*;
import viewer.viewer3d.graphic2d.*;

/***********************************
*
* Autor   : Thomas Behr
* Version : 1.0
* Datum   : 3.7.2000
*
*************************************/



public class Line3D extends Figure3D{

/** a endpoint */
private   Point3DSequence endpoints;

/** returns a readable representation of this point */
public String toString() {
 return  "Line3d : " + endpoints.getPoint3DAt(0) + ", " + endpoints.getPoint3DAt(1);
 }

/** creates a new line by given endpoints */
public Line3D( Point3DSimple  P1, Point3DSimple P2){
   this(P1,P2,IDManager.getNextID());
}

public Line3D(Point3DSimple P1, Point3DSimple P2,ID aID){
  endpoints = new Point3DSequence();
  endpoints.addPoint(P1);
  endpoints.addPoint(P2);
  myID = IDManager.getNextID();
  myID.equalize(aID);
}


public Line3D(Point3D P1, Point3D P2){
    this(P1.getLocation(),P2.getLocation());

}
public Point3DSimple getEP1(){
   return endpoints.getPoint3DAt(0);
}

public Point3DSimple getEP2(){
   return endpoints.getPoint3DAt(1);
}

/** returns a copy of this */
public Line3D duplicate() {
  return new Line3D(endpoints.getPoint3DAt(0),
                    endpoints.getPoint3DAt(1),myID);
}

public Figure3D copy(){
   return duplicate();
}

/** equalize this to Source */
public void equalize(Line3D Source) {
  endpoints.equalize(Source.endpoints);
  myID.equalize(Source.myID);
}

/** check for equal position and color (not needed ID) */
public boolean equalValues(Line3D D2) {
 return endpoints.equals(D2.endpoints);
}

/** returns the Bounding Box of this Line */
public BoundingBox3D getBoundingBox(){
  BoundingBox3D BB3 = new BoundingBox3D();
  Point3DSimple EP1 = endpoints.getPoint3DAt(0);
  Point3DSimple EP2 = endpoints.getPoint3DAt(1);
  double x1 = EP1.getX(),
         x2 = EP2.getX(),
         y1 = EP1.getY(),
         y2 = EP2.getY(),
         z1 = EP1.getZ(),
         z2 = EP2.getZ();
  BB3.set(Math.min(x1,x2),Math.min(y1,y2),Math.min(z1,z2),
          Math.max(x1,x2),Math.max(y1,y2),Math.max(z1,z2));
  return BB3;
}


/** check for equality with Q2  */
public boolean equals(Line3D Q2) {
  return endpoints.equals(Q2.endpoints) && 
         myID.equals(Q2.myID);
}

/** check for equality of the ID's */
public boolean equalID(Line3D Q2) {
  return  myID.equals(Q2.myID);
}

/** get the projection of this line */
public Figure2D  project(FM3DGraphic fm) {
   Point2DSequence p2s = fm.figureTransformation(endpoints);
   if(p2s.isEmpty()){
      return null;
   }
   Line2D res = new Line2D(p2s.getPoint2DAt(0), p2s.getPoint2DAt(1), myID);
   res.setSort(p2s.getSort());
   return res;
}


} // class Line3D





