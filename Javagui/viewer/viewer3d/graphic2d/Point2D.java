package viewer.viewer3d.graphic2d;

import java.awt.*;
import java.awt.image.*;
import gui.idmanager.*;

/***************************
*
* Autor   : Thomas Behr
* Version : 1.1
* Datum   : 16.5.2000
*
****************************/

/** this class provides a colorized Point-Object*/
public class Point2D {

 protected double x_pos;
 protected double y_pos;
 protected int cr,cg,cb;    // Colorvalues for this Point


 public String toString() {
   return "[( " + x_pos + " , "+ y_pos + "),("+cr+","+cg+","+cb+")]"; }


 /** creates a new Point from gives values */
 public Point2D(double x, double y,int r,int g,int b) {
    x_pos=x;
    y_pos = y;
    cr=r;
    cg = g;
    cb = b;
 }

 /** creates a new Point from given values */
 public Point2D(double x, double y, Color C) {
     x_pos = x;
     y_pos = y;
     cr = C.getRed();
     cg = C.getGreen();
     cb = C.getBlue();
 }

 
 /** equalize this to source */
 public void equalize(Point2D Source) {
    x_pos = Source.x_pos;
    y_pos = Source.y_pos;
    cr = Source.cr;
    cg = Source.cg;
    cb = Source.cb;
 }

 /** returns a copy from this */
 public Point2D duplicate() {
   return new Point2D(x_pos,y_pos,cr,cg,cb);
 }

 /** check for equality */
 public boolean equals(Point2D Pt) {
   return (x_pos == Pt.x_pos) && (y_pos ==Pt.y_pos) &&
          (cr == Pt.cr) && (cg==Pt.cg) && (cb==Pt.cg); }

 /** check for equal position */
 public boolean equalsCoordinates(Point2D P){
   return x_pos==P.x_pos && y_pos == P.y_pos;

 }


 /** returns the x-coordinate of this point*/
 public double getX() { return x_pos; }

 /** returns the y-coordinate of this point*/
 public double getY() { return y_pos; }

 /** set the x-coordinate */
 public void setX(double x) { x_pos = x; }

 /** set the y-coordinate */
 public void setY(double y) { y_pos = y; }

 /** set the x and the y-coordinate */
 public void moveTo(double x, double y) {
   x_pos = x;
   y_pos = y;
 }

 /** get the red-part of the color of this point */
 public int getRed() { return cr; }
 /** get the green-part of the color of this point */ 
 public int getGreen() { return cg; }
 /** get the blue-part of the color of this point */
 public int getBlue() { return cb; }


 /** returns the distance to Pt2 */
 public double distance(Point2D Pt2) {
   double x,y,d;
   x = x_pos-Pt2.x_pos;
   y = y_pos-Pt2.y_pos;
   d = Math.sqrt(x*x + y*y);
   return d;
 }



} // class Point2D
