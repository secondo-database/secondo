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
import viewer.viewer3d.graphic2d.Point2D;

/*******************
* author  : Thomas Behr
* version : 1.1
* date    : 16.5.2000
* content : class Point3D
********************/


public class Point3D {

/** coordinate of this point */
protected double  x,y,z;
/** color-part of this point */
protected int     cr,cg,cb;     // rgb-values of color


/** creates a new point */
public Point3D(double x, double y, double z, int r , int g, int b) {
  this.x = x;
  this.y = y;
  this.z = z;
  cr = r;
  cg = g;
  cb = b;
}

/** creates a new point */
public Point3D( double x, double y, double z, Color C) {
  this.x = x;
  this.y = y;
  this.z = z;
  cr = C.getRed();
  cg = C.getGreen();
  cb = C.getBlue();
}

/** check for valid color-values */
public boolean isValid(){
 // test : color is ok ?
 return cr>=0 & cr <256 & cg>=0 & cg<256 & cb>=0 & cb<256;

}

/** returns the x-coordinate of this point*/
public double getX() { return x; }

/** returns the y-coordinate of this point*/
public double getY() {return y; }

/** returns the z-coordinate of this point*/
public double getZ() {return z; }

/** returns the red-part of this point */
public int getR() { return cr; }
/** returns the green-part of this point */
public int getG() { return cg; }
/** returns the blue-part of this point */
public int getB() { return cb; }
/** returns the color of this point */
public Color getColor() { return new Color(cr,cg,cb); }

/** set the color of this point */
public void setColor(Color C) {
   cr = C.getRed();
   cg = C.getGreen();
   cb = C.getBlue();
}

/** set the color of this point */
public void setColor(int r,int g, int b){
  cr=r;
  cg=g;
  cb=b;
}


/** set the red-part of this point */
public void setR(int r) { cr = r; }
/** set the green-part of this point */
public void setG(int g) { cg = g;}
/** set the blue-part of this point */
public void setB(int b) { cb = b; }

/** set the x-coordinate of this point */
public void setX(double x) { this.x = x;}
/** set the y-coordinate of this point */
public void setY(double y) { this.y = y;}
/** set the z-coordinate of this point */
public void setZ(double z) { this.z = z;}

/** set the position of this point */
public void moveTo(double x, double y, double z) {
  this.x = x;
  this.y = y;
  this.z = z;
}

/** returns a readable representation of this point */
public String toString() {
  return ( "[( "+ x + ", "+ y + ", "+z+"),("+cr+","+cg+","+cb+")]"  );
}

/** returns a copy of this point */
public Point3D duplicate() {
   return new Point3D(x,y,z,cr,cg,cb); }

/** equalize this to P */
public void equalize(Point3D P) {
   x = P.x;
   y = P.y;
   z = P.z;
   cr = P.cr;
   cg = P.cg;
   cb = P.cb;
}

/** check for equality with P */
public boolean equals(Point3D P) {
  return ( (x==P.x) && (y==P.y) && (z==P.z) &&
           (cr==P.cr) && (cg==P.cg) && (cb==P.cb));
 }




/** computes the distance between this point and  P */
public double distance(Point3D P) {
 double d =  (x-P.x)*(x-P.x) +
             (y-P.y)*(y-P.y) +
             (z-P.z)*(z-P.z);
 return Math.sqrt(d);
}

/** returns the projection of this point */
public Point2D project() {
  return new Point2D(x,y,cr,cg,cb);
}



}
