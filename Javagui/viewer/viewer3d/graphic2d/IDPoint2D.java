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

public class IDPoint2D extends Point2D{

// a point with foreign identity

/** creates a point with a foreign inentity */
public IDPoint2D(Point2D P){
    super(P.x_pos,P.y_pos,P.cr,P.cg,P.cb);
}

/** creates a new Point */
public IDPoint2D(double x, double y, int r, int g, int b){
  super(x,y,r,g,b);
}

/** creates a new point */
public IDPoint2D(double x,double y, Color C){
  super(x,y,C);
}

/** returns a copy of this */
public IDPoint2D copy(){
  IDPoint2D C = new IDPoint2D(x_pos,y_pos,cr,cg,cb);
  C.MyID.equalize(MyID);
  return C;
}


/** returns the ID of this point */
public ID getID(){ return MyID;}
/** set the ID of this point */
public void setID(ID newID){ MyID.equalize(newID);}

/** set the diameter for painting */
public void setDiameter(int dia) {
  // set the diameter for paint this point
   this.dia = dia;
}


/** paint this point on img */
public void paint(BufferedImage img){
   Graphics G = img.getGraphics();
   G.setColor(new Color(cr,cg,cb));
   G.fillOval((int)x_pos-dia/2,(int)y_pos-dia/2,dia,dia);
}


/** paint this point if it containing in given clipping area */
public void paint(BufferedImage img,
                   int clipx, int clipy, int clipw,int cliph){
   if ( (x_pos>=clipx)          &&
        (x_pos<=(clipx+clipw))  &&
        (y_pos>=clipy)          &&
        (y_pos<=(clipy+cliph))      )
        paint(img);

 }
    

/** the diameter for painting */
private int dia=13;         // diameter for paint this Point
/** the ID o fthis point */
private ID MyID = IDManager.getNextID();



}


