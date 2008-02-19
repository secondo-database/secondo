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
import java.awt.*;
import java.awt.image.*;

public class IDPoint2D extends Figure2D{

 protected double x_pos;
 protected double y_pos;
 protected int cr,cg,cb;    // Colorvalues for this Point

 public boolean equals(Figure2D f){
    if(! (f instanceof IDPoint2D)){
        return false;
    }
    IDPoint2D p = (IDPoint2D) f;
    return myID.equals(p.myID) &&
           x_pos == p.x_pos &&
           y_pos == p.y_pos &&
           cr == p.cr && cg == p.cg && cb == p.cb;
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

 public String toString() {
   return "[( " + x_pos + " , "+ y_pos + "),("+cr+","+cg+","+cb+")]"; }

/** creates a new Point */
public IDPoint2D(double x, double y, int r, int g, int b){
  this.x_pos = x;
  this.y_pos = y;
  this.cr = r;
  this.cg = g;
  this.cb = b;
}

/** creates a new point */
public IDPoint2D(double x,double y, Color C){
   this(x,y,C.getRed(),C.getGreen(),C.getBlue());
}

public IDPoint2D(Point2D p, ID aID){
   this(p.getX(),p.getY(),p.getRed(),p.getGreen(),p.getBlue());
   myID = new ID();
   myID.equalize(aID);

}


/** returns a copy of this */
public Figure2D duplicate(){
  IDPoint2D C = new IDPoint2D(x_pos,y_pos,cr,cg,cb);
  C.myID.equalize(myID);
  return C;
}

/** set the diameter for painting */
public void setDiameter(int dia) {
  // set the diameter for paint this point
   this.dia = dia;
}


/** paint this point on img */
public void paint(Graphics g, boolean filled, boolean gradient){
   g.setColor(new Color(cr,cg,cb));
   g.fillOval((int)x_pos-dia/2,(int)y_pos-dia/2,dia,dia);
}


/** the diameter for painting */
private int dia=13;         // diameter for paint this Point

}


