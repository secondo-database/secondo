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
* Version : 1.1
* Datum   : 16.5.2000
*
*************************************/



public class Triangle3D extends Figure3D{


Point3DSequence endpoints; // endpoints of the interior
Point3DSequence line1_2;
Point3DSequence line1_3;
Point3DSequence line2_3;


/** returns a readable representation of this triangle */
public String toString() {
  String S = "Triangle3D ";
  for(int i=0;i<3;i++) { S += endpoints.getPoint3DAt(i); }
  return S;
}


public Point3DSimple getCP1(){
   return endpoints.getPoint3DAt(0);
}

public Point3DSimple getCP2(){
   return endpoints.getPoint3DAt(1);
}
public Point3DSimple getCP3(){
   return endpoints.getPoint3DAt(2);
}

/** creates a new Triangle */
public Triangle3D( Point3DSimple  P1, Point3DSimple P2, Point3DSimple P3,
                   boolean line1_2, boolean line1_3, boolean line2_3, Color borderColor, ID aID){
   endpoints = new Point3DSequence();
   endpoints.addPoint(P1.duplicate());
   endpoints.addPoint(P2.duplicate());
   endpoints.addPoint(P3.duplicate());
   if(line1_2 || line1_3 || line2_3){
      P1 = P1.duplicate();
      P1.setColor(borderColor);
      P2 = P2.duplicate();
      P2.setColor(borderColor);
      P3 = P3.duplicate();
      P3.setColor(borderColor);
   }
   if(line1_2){
      this.line1_2 = new Point3DSequence();
      this.line1_2.addPoint(P1);
      this.line1_2.addPoint(P2);
   } else {
      this.line1_2 = null;
   } 
   if(line1_3){
      this.line1_3 = new Point3DSequence();
      this.line1_3.addPoint(P1);
      this.line1_3.addPoint(P3);
   } else {
      this.line1_3 = null;
   } 
   if(line2_3){
      this.line2_3 = new Point3DSequence();
      this.line2_3.addPoint(P2);
      this.line2_3.addPoint(P3);
   } else {
      this.line2_3 = null;
   }
   myID = new ID();
   myID.equalize(aID); 
}

public Triangle3D(Point3DSimple P1,Point3DSimple P2, Point3DSimple P3,ID aID){
   this(P1,P2,P3,false,false,false,Color.BLACK, aID);
}

public Triangle3D(Point3DSimple P1,Point3DSimple P2, Point3DSimple P3){
   this(P1,P2,P3,false,false,false,Color.BLACK, IDManager.getNextID());
}

/** returns a copy of this triangle */
public Triangle3D duplicate() {
   Color C = null;
   if(line1_2!=null){
      C = line1_2.getPoint3DAt(0).getColor();
   }
   if(line1_3!=null && C==null){
      C = line1_3.getPoint3DAt(0).getColor();
   } 
   if(line2_3!=null && C==null){
      C = line2_3.getPoint3DAt(0).getColor();
   } 
   return new Triangle3D(endpoints.getPoint3DAt(0), 
                  endpoints.getPoint3DAt(1), 
                  endpoints.getPoint3DAt(2),
                  line1_2!=null,
                  line1_3!=null,
                  line2_3!=null,
                  C, myID);
}

public Figure3D copy(){
   return duplicate();
}



/** returns the BoundingBox of this Triangle*/
public BoundingBox3D getBoundingBox(){
  BoundingBox3D BB3 = new BoundingBox3D();
  double xmin,xmax,ymin,ymax,zmin,zmax;
  xmin=xmax=endpoints.getPoint3DAt(0).getX();
  ymin=ymax=endpoints.getPoint3DAt(0).getY();
  zmin=zmax=endpoints.getPoint3DAt(0).getZ();
  for(int i=1;i<3;i++){
      Point3DSimple p = endpoints.getPoint3DAt(i);
      double x = p.getX();
      double y = p.getY();
      double z = p.getZ();
      xmin = xmin>x?x:xmin;
      ymin = ymin>y?y:xmin;
      zmin = zmin>z?z:xmin;
      xmax = xmax<x?x:xmax;
      ymax = ymax<y?y:xmax;
      zmax = zmax<z?z:xmax;
  }
  BB3.set(xmin,ymin,zmin,xmax,ymax,zmax);
  return BB3;
}

/** check for equality */
public boolean equals(Triangle3D Q2) {
  if(! myID.equals(Q2.myID)){
    return false;
  }
  if(!endpoints.equals(endpoints)){
    return false;
  }
  // ignore border at this moment
  return true;
}


/** returns the projection of this triangle */
public Figure2D  project(FM3DGraphic fm) {

   // first, convert the interior
   Point2DSequence interior = fm.figureTransformation(endpoints);
   if(interior.isEmpty()){
      return null;
   }
  
   
 
   Triangle2D res = new Triangle2D(myID);

   res.setSort(interior.getSort());


   Point2D p1 = interior.getPoint2DAt(0);
   for(int i=0;i<interior.getSize()-2;i++){
       Point2D p2 = interior.getPoint2DAt(i+1);
       Point2D p3 = interior.getPoint2DAt(i+2);
       res.insert(new Triangle2DSimple(p1,p2,p3));
   }
   // insert lines 
   if(line1_2!=null){
     Point2DSequence seg = fm.figureTransformation(line1_2);
     if(!seg.isEmpty()){
        res.insert(new Line2D(seg.getPoint2DAt(0), seg.getPoint2DAt(1)));
     }
   } 
   if(line1_3!=null){
     Point2DSequence seg = fm.figureTransformation(line1_3);
     if(!seg.isEmpty()){
        res.insert(new Line2D(seg.getPoint2DAt(0), seg.getPoint2DAt(1)));
     }
   } 
   if(line2_3!=null){
     Point2DSequence seg = fm.figureTransformation(line2_3);
     if(!seg.isEmpty()){
        res.insert(new Line2D(seg.getPoint2DAt(0), seg.getPoint2DAt(1)));
     }
   } 
   return res;
}



} // class Triangle3D













