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
  Datum   : 16.5.2000
######################## */


import java.awt.image.*;
import java.awt.*;
import gui.idmanager.*;


public class Triangle2D {

/** creates a new Tringle2D with given Points */
public  Triangle2D( Point2D P1, Point2D P2, Point2D P3) {
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

   x3 = (int) P3.getX();
   y3 = (int) P3.getY();
   c3r = P3.getRed();
   c3g = P3.getGreen();
   c3b = P3.getBlue();
   swap();
   TriangleID = IDManager.getNextID();
 }

/** returns the ID of this triangle */
public ID   getID(){ return TriangleID; }

/** set the ID of this triangle */
public void setID(ID newID) { TriangleID.equalize(newID);}

/** returns a readable representation of this triangle */
public String toString(){
  return "T2D ("+x1+","+y1+");("+x2+","+y2+");("+x3+","+y3+")";
}

/** check for equal position with T*/
public boolean equalsCoordinates(Triangle2D T){
 boolean ok = true;
 for(int i=0;i<3;i++){
    if (PointsX[i]!=T.PointsX[i] | PointsY[i]!=T.PointsY[i])
      ok = false;
 }
 
 return ok;
}

/** return the x-coordinate from cornerpoint 1 */
public int getX1(){ return x1;}
/** return the x-coordinate from cornerpoint 2 */
public int getX2(){ return x2;}
/** return the x-coordinate from cornerpoint 3 */
public int getX3(){ return x3;}
/** return the y-coordinate from cornerpoint 1 */
public int getY1(){ return y1;}
/** return the y-coordinate from cornerpoint 2 */
public int getY2(){ return y2;}
/** return the y-coordinate from cornerpoint 3 */
public int getY3(){ return y3;}


/** set the x-coordinate from cornerpoint 1 */
public void setX1(int X){ x1 = X; swap();}
/** set the x-coordinate from cornerpoint 2 */
public void setX2(int X){ x2 = X; swap();}
/** set the x-coordinate from cornerpoint 3 */
public void setX3(int X){ x3 = X; swap();}
/** set the y-coordinate from cornerpoint 1 */
public void setY1(int Y){ y1 = Y; swap();}
/** set the y-coordinate from cornerpoint 2 */
public void setY2(int Y){ y2 = Y; swap();}
/** set the y-coordinate from cornerpoint 3 */
public void setY3(int Y){ y3 = Y; swap();}

/** set the 3 y-valus of this point */
public void setYValues(int y1, int y2, int y3){
  this.y1 = y1;
  this.y2 = y2;
  this.y3 = y3;
  swap();
}

/**
  * paint this triangle on img
  * @param img : the image to paint
  * @param border : paint a border or not
  * @param fill: paint the interior
  * @param gradient: paint with a single color or with a gradient
  */
public void paint(BufferedImage img,boolean border,
                  boolean fill, boolean gradient) {

  if (fill) {
    if (gradient) {
        int minY = Math.min(y1,Math.min(y2,y3));
        int maxY = Math.max(y1,Math.max(y2,y3));
        int Xmin = img.getMinX();   // the Clipping-Area
        int Xmax = Xmin + img.getWidth();
        int Ymin = img.getMinY();
        int Ymax = img.getHeight()+Ymin;

        double Delta;
        int l1x,l2x,l1r,l1g,l1b,l2r,l2g,l2b;

        int ColorValue;

        /* paint the 3 cornerpoints */

        if( x1>=Xmin & x1 <Xmax & y1>=Ymin & y1<Ymax) {
           ColorValue =  (int) 4278190080L +                       // Alpha-Wert
                         (c1r)*65536 + (c1g)*256 + (c1b);
           img.setRGB(x1,y1,ColorValue);
        }
        if( x2>=Xmin & x2<Xmax & y2>=Ymin & y2<Ymax){
           ColorValue =  (int) 4278190080L +                       // Alpha-Wert
                       (c2r)*65536 + (c2g)*256 + (c2b);
           img.setRGB(x2,y2,ColorValue);
        }

        if( x3>=Xmin & x3 <Xmax & y3>=Ymin & y3<Ymax){
           ColorValue =  (int) 4278190080L +                       // Alpha-Wert
                       (c3r)*65536 + (c3g)*256 + (c3b);
           img.setRGB(x3,y3,ColorValue);
        }

        /* paint horizontal lines */

       if ( PointsY[0] == PointsY[1] )
           line(img, PointsX[0],PointsX[1],PointsY[0],
              ColorR[0],ColorG[0],ColorB[0],
              ColorR[1],ColorG[1],ColorB[1] );

       if (PointsY[1] == PointsY[2])
           line(img, PointsX[1],PointsX[2],PointsY[1],
               ColorR[1],ColorG[1],ColorB[1],
               ColorR[2],ColorG[2],ColorB[2]);    
  
       for (int y = minY; y < maxY; y++){   // for all rows
         if (y<PointsY[1])  {
            Delta = (double)(y-PointsY[0])/(double)(PointsY[1]-PointsY[0]);
            l1x = (int)(PointsX[0] + Delta*(PointsX[1]-PointsX[0]));
            l1r = (int)( ColorR[0] + Delta*(ColorR[1]-ColorR[0]));
            l1g = (int)( ColorG[0] + Delta*(ColorG[1]-ColorG[0]));
            l1b = (int)( ColorB[0] + Delta*(ColorB[1]-ColorB[0]));
         }
         else {
            Delta = (double)(y-PointsY[1])/(double)(PointsY[2]-PointsY[1]);
            l1x = (int)(PointsX[1] + Delta*(PointsX[2]-PointsX[1]));
            l1r = (int)( ColorR[1] + Delta*(ColorR[2]-ColorR[1]));
            l1g = (int)( ColorG[1] + Delta*(ColorG[2]-ColorG[1]));
            l1b = (int)( ColorB[1] + Delta*(ColorB[2]-ColorB[1]));
         } // else

         Delta = (double) (y-PointsY[0]) / (double)(PointsY[2]-PointsY[0]);
         l2x = (int)(PointsX[0] + Delta*(PointsX[2]-PointsX[0]));
         l2r = (int)(ColorR[0] + Delta*(ColorR[2]-ColorR[0]));
         l2g = (int)(ColorG[0] + Delta*(ColorG[2]-ColorG[0]));
         l2b = (int)(ColorB[0] + Delta*(ColorB[2]-ColorB[0]));         

         line( img , l1x,l2x,y,l1r,l1g,l1b,l2r,l2g,l2b);

      } // for
   } // if Verlauf
   else {
       java.awt.Polygon Poly = new java.awt.Polygon(PointsX,PointsY,3);
       java.awt.Graphics g = img.getGraphics();
       int red = (c1r+c2r+c3r)/3;
       int green = (c1g+c2g+c3g)/3;
       int blue = (c1b+c2b+c3b)/3;
       g.setColor(new Color( (c1r+c2r+c3r)/3 , (c1g+c2g+c3g)/3 ,
                             (c1b+c2b+c3b)/3 ));
       g.fillPolygon(Poly);
   } // else !Gradient
  } // if (fill)
  
   if (border) {
    java.awt.Polygon Poly = new java.awt.Polygon(PointsX,PointsY,3);
    java.awt.Graphics g = img.getGraphics();
    g.setColor(Color.black);
    g.drawPolygon(Poly);
   }
 }  // paint


/**
 * paint this triangle if it intersects the given clipping-area
 */
 public void paint(BufferedImage img,boolean border,
                   boolean fill, boolean Gradient,
                   int clipx,int clipy, int clipw, int cliph) {

 // invokes paint if a Point in the clipping area
 boolean paintit = true;

 int miny = PointsY[0];
 int maxy = PointsY[2];
 int minx = PointsX[0];
 int maxx = PointsX[0];
 for(int i=1;i<3;i++) {
   if (PointsX[i]<minx) minx=PointsX[i];
   if (PointsX[i]>maxx) maxx=PointsX[i];
 }


 if ( ( miny > (clipy+cliph)       )    |   // over clippingarea
      ( maxy < (clipy))                 |          // under area
      ( minx > (clipx+clipw) )          |   // right of area
      ( maxx < clipx         )              // left of area
     )
     paintit = false;

 if (paintit)
     paint(img,border,fill,Gradient);
}


/** sort the points by y-values */
private void swap() {

 // sort points by y-values
   
  int temp;
  PointsX[0] = x1; PointsY[0] = y1;
  ColorR[0] = c1r; ColorG[0] = c1g; ColorB[0]=c1b;
  PointsX[1] = x2; PointsY[1] = y2;
  ColorR[1] = c2r; ColorG[1] = c2g; ColorB[1]= c2b;
  PointsX[2] = x3; PointsY[2] = y3;
  ColorR[2] = c3r; ColorG[2] = c3g; ColorB[2]= c3b;

  for(int i=0;i<PointsY.length;i++)
    for(int j=i+1;j<PointsY.length; j++)
     if (PointsY[i]>PointsY[j]) {
        temp = PointsY[i];          // swap y_field
        PointsY[i] =PointsY[j];
        PointsY[j] = temp;
        temp = PointsX[i];          // swap X_Field
        PointsX[i] = PointsX[j];
        PointsX[j] = temp;
        temp = ColorR[i];           // swap Red
        ColorR[i] = ColorR[j];
        ColorR[j] = temp;
        temp = ColorG[i];           // swap Green
        ColorG[i] = ColorG[j];
        ColorG[j] = temp;
        temp = ColorB[i];           // swap Blue
        ColorB[i] = ColorB[j];
        ColorB[j] = temp;
     } // if

 }



/** paint a scanline of this triangle on img */
private void line(BufferedImage img,
                   int x1 , int x2, int y,
                   int c1r , int c1g, int c1b,
                   int c2r , int c2g, int c2b )  {

 double delta;
 double deltaCr = c2r-c1r,
        deltaCg = c2g-c1g,
        deltaCb = c2b-c1b;
  int   ColorValue;

  int minX = img.getMinX();   // Clipping-Area
  int minY = img.getMinY();
  int maxX = minX+img.getWidth();
  int maxY = minY+img.getHeight();

  // paint the endpoints

     if( x1>=minX & x1 <maxX & y>minY & y<maxY){
        ColorValue =  (int) 4278190080L +                       // Alpha-Value
                    (c1r) * 65536 +
                    (c1g) * 256   +
                    (c1b);
        img.setRGB(x1,y,ColorValue);
    }
    if( x2>=minX & x2 <maxX & y>=minY & y<maxY){
       ColorValue =  (int) 4278190080L +                       // Alpha-Value
                   (c2r) * 65536 +
                   (c2g) * 256   +
                   (c2b);
       img.setRGB(x2,y,ColorValue);
     }

  if (x2>x1) 
     for(int x=x1; x<x2 ;x++) {
         delta = (double)(x-x1) / (double) (x2-x1);
         if( x>=minX & x <maxX & y>=minY & y<maxY){
             ColorValue =
                         (int) 4278190080L +                       // Alpha-Value
                         (int) (c1r+delta*deltaCr) * 65536 +
                         (int) (c1g+delta*deltaCg) * 256   +
                         (int) (c1b+delta*deltaCb);
             img.setRGB(x,y,ColorValue);
         }
      } // for
  else
    for(int x=x1; x>x2; x--) {
         delta = (double)(x-x1) / (double) (x2-x1);
         if( x>=minX & x <maxX & y>=minY & y<maxY){
            ColorValue =  (int) 4278190080L +                       // Alpha-Value
                        (int) (c1r+delta*deltaCr) * 65536 +
                        (int) (c1g+delta*deltaCg) * 256   +
                        (int) (c1b+delta*deltaCb);
            img.setRGB(x,y,ColorValue);
         }

      } // for
  
  
   
 } // line


 /** returns the distnace from (x1,y1) to (x2,y2) */
  private double distance(double x1, double y1, double x2, double y2) {
     double dx = (x2-x1)*(x2-x1);
     double dy = (y2-y1)*(y2-y1);
     return Math.sqrt(dx+dy);
  }


/** returns a copy from this */
 public Triangle2D copy(){
   Triangle2D C = new Triangle2D(new Point2D(x1,y1,c1r,c1g,c1b),
                      new Point2D(x2,y2,c2r,c2g,c2b),
                      new Point2D(x3,y3,c3r,c3g,c3b));
   C.TriangleID.equalize(TriangleID);
   return C;
 }

 /** a coordinate of a cornerpoint */
 private int x1,x2,x3,y1,y2,y3;   // the Points

 /** a colorpart of a cornerpoint */
 private int   c1r,c1g,c1b,
               c2r,c2g,c2b,      //  rgb- values
               c3r,c3g,c3b;      //  of the 3 Colors


 /** array of sorted points */
 private int[]  PointsX = new int[3];  // arrays of sorted points
 private int[]  PointsY = new int[3];  // ..[3] containts max y-value
 private int[]  ColorR  = new int[3];  // and colors
 private int[]  ColorG  = new int[3];
 private int[]  ColorB  = new int[3];

 /** the ID */
 private ID  TriangleID;


} // class Triangle2D

