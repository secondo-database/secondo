package viewer.viewer3d.graphic2d;

/************************
  Autor   : Thomas Behr
  Version : 1.0
  Datum   : 3.7.2000
######################## */


import java.awt.image.*;
import java.awt.*;
import gui.idmanager.*;

public class Line2D {

/** creates a new Line by given Points */
public  Line2D( Point2D P1, Point2D P2) {
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
   MyID = IDManager.getNextID();
}

/** returns a copy from this */
public Line2D copy(){
  Line2D C = new Line2D(new Point2D(x1,y1,c1r,c1g,c1b),
                        new Point2D(x2,y2,c2r,c2g,c2b));
  C.MyID.equalize(MyID);
  return C;
}

/** get the ID of this */
public ID getID(){ return MyID; }

/** set the ID of this */
public void setID(ID newID){ MyID.equalize(newID);}
   
/** check for equal position */
public boolean equalsCoordinates(Line2D L){

 return (  x1==L.x1 && y1==L.y1 && x2==L.x2 && y2==L.y2) ||
        (  x1==L.x2 && y1==L.y2 && x2==L.x1 && y2==L.y1);

}


/**
  * paint this line on img
  * @ param gradient: paint in single color or with gradient
  */
public void paint(BufferedImage img, boolean Gradient) {

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
        int minY = img.getMinY();
        int minX = img.getMinX();
        int maxX = minX + img.getWidth();
        int maxY = minY + img.getHeight();


        if ( Math.abs(y2-y1) > Math.abs(x2-x1) ) {  // steile Linie

           for(y=Math.min(y2,y1); y<=Math.max(y2,y1);y++) {

              delta= (double)(y-y1)/deltaY;
              x = (int)(x1 + delta*deltaX);

              if( x>=minX & x <maxX & y>=minY & y<maxY) {

                  ColorValue = (int) 4278190080L +              // Alpha-Value
                               (int) (c1r+delta*deltaCr) * 65536 +
                               (int) (c1g+delta*deltaCg) * 256   +
                               (int) (c1b+delta*deltaCb);
                  img.setRGB(x,y,ColorValue);
             }
           } // for
         } // if
         else {   // flat line
            for( x = Math.min(x1,x2);x<=Math.max(x1,x2);x++) {
               delta = (double)(x-x1)/deltaX;
               y = (int) ( y1 + delta*deltaY);

              if( x>=minX & x <maxX & y>=minY & y<maxY) {
                   ColorValue = (int) 4278190080L +              // Alpha-Value
                            (int) (c1r+delta*deltaCr) * 65536 +
                            (int) (c1g+delta*deltaCg) * 256   +
                            (int) (c1b+delta*deltaCb);
                  img.setRGB(x,y,ColorValue);
              }
             } // for
         }// else
     }
     else {   // !Gradient
        Color C = new Color( (c1r+c2r)/2,(c1g+c2g)/2,(c1b+c2b)/2);
        Graphics g = img.getGraphics();
        g.setColor(C);
        g.drawLine(x1,y1,x2,y2);
     }// else

  } // if (!empty)

 }  // paint



/** paint this line if it intersects the given clipping-area */
public void paint(BufferedImage img, boolean Gradient,
                   int clipx, int clipy, int clipw, int cliph) {

boolean paintit=true;
int miny = Math.min(y1,y2);
int maxy = Math.max(y1,y2);
int maxx = Math.max(x1,x2);
int minx = Math.min(x1,x2);

if ( ( miny > (clipy + cliph))   |   // under clippingarea
     ( maxy < clipy         )    |   // over area
     ( minx > (clipx+clipw) )    |   // right of area
     ( maxx < clipx         )       // left of area
    )
    paintit = false;
if (paintit)
    paint(img,Gradient);
}

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
 /** the ID of this line */
 private ID MyID;

} // class Line2D

