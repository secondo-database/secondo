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

/************************
  Autor   : Thomas Behr
  Version : 1.0
  Datum   : 3.7.2000
######################## */


import java.awt.image.*;
import java.awt.*;
import gui.idmanager.*;

public class Line2D extends Figure2D{

/** creates a new Line by given Points */
public  Line2D( Point2D P1, Point2D P2, ID aID) {
   x1 = (int) P1.getX();
   y1 = (int) P1.getY();
   c1r = P1.getRed();
   c1g = P1.getGreen();
   c1b = P1.getBlue();
   x2 = (int) P2.getX();
   y2 = (int) P2.getY();
   c2r = P2.getRed();
   c2g = P2.getGreen();
   c2b = P2.getBlue();
   empty = false;
   myID = new ID();
   myID.equalize(aID);
}

public Line2D(Point2D P1, Point2D P2){
   this(P1,P2,IDManager.getNextID());
}


/** returns a copy from this */
public Figure2D duplicate(){
  Line2D C = new Line2D(new Point2D(x1,y1,c1r,c1g,c1b),
                        new Point2D(x2,y2,c2r,c2g,c2b));
  C.myID.equalize(myID);
  return C;
}

/** check for equal position */
public boolean equalsCoordinates(Line2D L){

 return (  x1==L.x1 && y1==L.y1 && x2==L.x2 && y2==L.y2) ||
        (  x1==L.x2 && y1==L.y2 && x2==L.x1 && y2==L.y1);

}

public boolean equals(Figure2D f){
   if(!(f instanceof Line2D)){
      return false;
   }
   Line2D L = (Line2D) f;
   return x1 == L.x1 &&
          x2 == L.x2 &&
          y1 == L.y1 &&
          y2 == L.y2 &&
          c1r == L.c1r && c2r == L.c2r &&
          c1g == L.c1g && c2g == L.c2g &&
          c1b == L.c1b && c2b == L.c2b &&
          myID.equals(L.myID);
}


/**
  * paint this line on img
  * @ param gradient: paint in single color or with gradient
  */
public void paint(Graphics g, boolean filled, boolean Gradient) {

  if( !empty)  {

     if (Gradient)  {
        double delta;
        int deltaCr = c2r-c1r;
        int deltaCg = c2g-c1g;
        int deltaCb = c2b-c1b;
        double deltaX  = x2-x1;
        double deltaY  = y2-y1;
        int ColorValue;
        int x;
        int y;
//        int minY = img.getMinY();
//        int minX = img.getMinX();
//        int maxX = minX + img.getWidth();
//        int maxY = minY + img.getHeight();


        if ( Math.abs(y2-y1) > Math.abs(x2-x1) ) {  // steile Linie

           for(y=Math.min(y2,y1); y<=Math.max(y2,y1);y++) {

              delta= (double)(y-y1)/deltaY;
              x = (int)(x1 + delta*deltaX);
              g.setColor(new Color((int) (c1r+delta*deltaCr),
                                   (int) (c1g+delta*deltaCg),  (int) (c1b+delta*deltaCb)));
              g.fillRect(x,y,1,1);
           } // for
         } // if
         else {   // flat line
            for( x = Math.min(x1,x2);x<=Math.max(x1,x2);x++) {
               delta = (double)(x-x1)/deltaX;
               y = (int) ( y1 + delta*deltaY);

                   ColorValue = (int) 4278190080L +              // Alpha-Value
                            (int) (c1r+delta*deltaCr) * 65536 +
                            (int) (c1g+delta*deltaCg) * 256   +
                            (int) (c1b+delta*deltaCb);
                   g.setColor(new Color(ColorValue));
                   g.fillRect(x,y,1,1);
             } // for
         }// else
     }
     else {   // !Gradient
        Color C = new Color( (c1r+c2r)/2,(c1g+c2g)/2,(c1b+c2b)/2);
        g.setColor(C);
        g.drawLine(x1,y1,x2,y2);
     }// else

  } // if (!empty)

 }  // paint


/** get the x-coordinate of endpoint 1 */
public int getX1(){ return x1;}
/** get the x-coordinate of endpoint 2 */
public int getX2(){ return x2;}
/** get the y-coordinate of endpoint 1 */
public int getY1(){ return y1;}
/** get the y-coordinate of endpoint 2 */
public int getY2(){ return y2;}

/** set the x-coordinate of endpoint 1 */
public void setX1(int X){ x1 = X;}
/** set the x-coordinate of endpoint 2 */
public void setX2(int X){ x2 = X;}
/** set the y-coordinate of endpoint 1 */
public void setY1(int Y){ y1 = Y;}
/** set the y-coordinate of endpoint 2 */
public void setY2(int Y){ y2 = Y;}


/** check for emptyness */
public void setEmpty(boolean e){  // empty by Clipping !
  empty = e;
}


 /** a coordinate */
 private int x1,x2,y1,y2;        // the points
 /** a color-part */
 private int   c1r,c1g,c1b,
               c2r,c2g,c2b;      // rgb-colors of the points

 /** is this a empty line */
 private boolean empty;

} // class Line2D

