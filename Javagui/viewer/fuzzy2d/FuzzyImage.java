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

package viewer.fuzzy2d;

import java.awt.*;
import java.awt.image.*;
import viewer.viewer3d.graphic3d.*;  // we need 3D-points lines and triangles
import gui.idmanager.*;
import viewer.viewer3d.objects.*;


public class FuzzyImage extends BufferedImage{


/** create a new fuzzy images with specific size */
public FuzzyImage(int width, int height){

  super(width,height,TYPE_INT_RGB);
  this.width = width;
  this.height = height;
  ZBuffer = new double[width][height];
  BB.setTo(0,0,width,height);
}


/** keep proportions while resize of objects ? */
public void setProportional(boolean p){
  proportional = p;
}


/** adds a Point to this Image */
public void add(IDPoint3D P){
  if(P!=null) {
    Points.append(P);
    transformAndInsert(P);
  }  
}

/** adds a set of points to this image */
public void add(IDPoint3D[] Pts){
  if(Pts!=null)
     for(int i=0;i<Pts.length;i++)
        add(Pts[i]);
}

/** add all elements of Pts to this images*/
public void add(IDPoint3DVector Pts){
  if(Pts!=null)
     for(int i=0;i<Pts.getSize();i++)
        add(Pts.get(i));
}


/** adds a segment to this image */
public void add(Line3D L){
  if(L!=null) {
    transformAndInsert(L); 
    Lines.append(L);
  }  
}

/** adds a set of segments to this image */
public void add(Line3D[] Lns){
  if(Lns!=null)
     for(int i=0;i<Lns.length;i++)
        add(Lns[i]);
}

/** add all elements of a aLine3DVector */
public void add(Line3DVector Lns){
 if(Lns!=null)
    for(int i=0;i<Lns.getSize();i++)
        add(Lns.get(i));
}


/** adds a triangle to this image */
public void add(Triangle3D T){
  if(T!=null) {
    transformAndInsert(T);
    Triangles.append(T);
  }  
}

/** adds a set of triangles to this image */
public void add(Triangle3D[] Trs){
  if(Trs!=null)
     for(int i=0;i<Trs.length;i++)
         add(Trs[i]);
}

/** add all elements of Trs to this image */
public void add(Triangle3DVector Trs){
   if(Trs!=null)
      for(int i=0;i<Trs.getSize();i++)
          add(Trs.get(i));
}




/** removes all Components from this image */
public void removeAll(){
  Points.empty();
  Lines.empty();
  Triangles.empty();   
  TransformedPoints.empty();
  TransformedLines.empty();
  TransformedTriangles.empty();
}


/** initialized the Zbuffer*/
private void initializeZBuffer(){
  double Min = Double.MIN_VALUE;
  for(int x=0;x<width;x++)
     for(int y=0;y<height;y++)
        ZBuffer[x][y]=Min;
}


/** creates the images with added figures */
public void paint(){
  initializeZBuffer();
  Graphics2D G = (Graphics2D) getGraphics();
  G.setBackground(new Color(255,255,255));
  G.clearRect(0,0,width,height);
  paintTriangles();
  paintLines();
  paintPoints();
}


/* set a colored point on (x,y) if z greater as the current ZBuffer value */
private void set(int x, int y ,double z,int Colorvalue){
  if(x>=0 && y>=0 && x<width && y<height && z>=ZBuffer[x][y]){
     ZBuffer[x][y] = z;  
     setRGB(x,y,Colorvalue);
  }
}






/** paint all containing points 
  */
private void paintPoints(){
  IDPoint3D P;
  for(int i=0;i<TransformedPoints.getSize();i++){
      P = TransformedPoints.get(i);
      paintSinglePoint( (int)P.getX(),(int)P.getY(),P.getZ(),P.getColor().getRGB());
  }
}



/** paint a single Point
  * if you want to paint a point i another form then
  * change this method
  */
private void paintSinglePoint(int x, int y,double z,int color){
   // first set the Point self
   set(x,y,z,color);
   boolean RectanglePoint = false;
   int P2 = PointSize/2;
   int P2_2 = P2*P2;
   for(int i=x-P2;i<=x+P2;i++)
     for(int j=y-P2;j<=y+P2;j++)
        if(RectanglePoint || ((i-x)*(i-x)+(j-y)*(j-y))<P2_2)
           set(i,j,z,color);
}


/** computes a linear approximation between 2 color values
  * if delta<0 (delta>1) then Min (Max) id returned
  */
private int getColor(Color Min,Color Max,double delta){
  int r1 = Min.getRed();
  int g1 = Min.getGreen();
  int b1 = Min.getBlue();
  int r2 = Max.getRed();
  int g2 = Max.getGreen();
  int b2 = Max.getBlue();
  
  
  double r,g,b;
  if(delta<=0){
     r=r1;
     g=g1;
     b=b1;
  } else if(delta>=1){
     r=r2;
     g=g2;
     b=b2;
  } else{
     r = r1 + delta*(r2-r1);
     g = g1 + delta*(g2-g1);
     b = b1 + delta*(b2-b1);
  }   
  return (int) 4278190080L +              // Alpha-Value
         (int) r * 65536 +
         (int) g * 256   +
         (int) b;

}


/** computes a linear approximation  beween Min and Max 
  * if delta<0 (delta>1) then Min (Max) is returned
  */
private double getZ(double Min,double Max, double delta){
  if(delta<=0)
     return Min;
  if(delta>=1)
     return Max;
  return Min+delta*(Max-Min);   
}


/** paints all containing segments */
private void paintLines(){
   for(int i=0;i<TransformedLines.getSize();i++)
      paintSingleLine(TransformedLines.get(i));
}


/** paints a single Line 
  * extend this method to paint 
  * a line with bigger width
  */
private void paintSingleLine(Line3D L){
  Point3D P1 = L.getEP1();
  Point3D P2 = L.getEP2();
  int x1 = (int) P1.getX();
  int y1 = (int) P1.getY();
  double z1 = P1.getZ();
  Color C1 = P1.getColor();

  int x2 = (int) P2.getX();
  int y2 = (int) P2.getY();
  double z2 = P2.getZ();
  Color C2 = P2.getColor();
  double delta;  
  
  int C;
  double Z;
  int x,y;
  
  if ( Math.abs(y2-y1) > Math.abs(x2-x1) ) {    // a steep line
     if(y2<y1){ // swap all values
        Color TmpC = C1;
        C1 = C2;
        C2 = TmpC;
        double  tmpZ = z1;
        z1 = z2;
        z2 =tmpZ;
        int tmpy = y1;
        y1 = y2;
        y2 = tmpy;
        int tmpx = x1;
        x1 = x2;
        x2 = tmpx;
     }
    double deltaX = x2-x1;
    double deltaY = y2-y1;
    for(y=y1; y<=y2;y++) {
         delta= (double)(y-y1)/deltaY;
         x = (int)(x1 + delta*deltaX);
         C = getColor(C1,C2,delta);
         Z = getZ(z1,z2,delta);   
         set(x,y,Z,C);   
     } // for
  } // if
  else {   // flat line
     if(x2<x1){ // swap all values
        Color TmpC = C1;
        C1 = C2;
        C2 = TmpC;
        double  tmpZ = z1;
        z1 = z2;
        z2 =tmpZ;
        int tmpy = y1;
        y1 = y2;
        y2 = tmpy;
        int tmpx = x1;
        x1 = x2;
        x2 = tmpx;
     }
     double deltaX = x2-x1;
     double deltaY = y2-y1;
     for( x = x1;x<=x2;x++) {
        delta = (double)(x-x1)/deltaX;
        y = (int) ( y1 + delta*deltaY);
        C = getColor(C1,C2,delta);
        Z = getZ(z1,z2,delta);
        set(x,y,Z,C);
     } // for
 }// else
}





/** paints all Triangles */
private void paintTriangles(){
  for(int i=0;i<TransformedTriangles.getSize();i++)
     paintSingleTriangle(TransformedTriangles.get(i));
}


/** paint a single Triangle on this image */
private void paintSingleTriangle(Triangle3D T){
  Point3D P1 = T.getCP1(),
          P2 = T.getCP2(),
          P3 = T.getCP3();
  int y1= (int)P1.getY(),
      y2= (int)P2.getY(),
      y3= (int)P3.getY();
  


  
 
  // sort the Points depending from y values    
  int ymin = Math.min(y1,Math.min(y2,y3));
  int ymax = Math.max(y1,Math.max(y2,y3));
  if(ymin==ymax)  // not a triangle
     return;
  
  Point3D TMP;
  // set the 1st Point
  if(y2==ymin){
     TMP=P1;
     P1=P2;
     P2=TMP;
     y1 = (int)P1.getY();
     y2 = (int)P2.getY();
  }else if(y3==ymin) {
     TMP=P3;
     P3=P1;
     P1=TMP;
     y1 = (int)P1.getY();
     y3 = (int)P3.getY();
  }
  
  // set the max Point
  if(y1==ymax){
     TMP=P1;
     P1=P3;
     P3=TMP;
     y1 = (int)P1.getY();
     y3 = (int)P3.getY();
  }else if(y2==ymax){
     TMP=P2;
     P2=P3;
     P3=TMP;
     y2= (int)P2.getY();
     y3= (int)P3.getY();
  }
  

 
  // if we have  a border we paint this lines in black after the inner triangles
  if(!border){
     paintSingleLine(new Line3D(P1,P2));
     paintSingleLine(new Line3D(P2,P3));
     paintSingleLine(new Line3D(P1,P3));
  }   

  
  double x1 = P1.getX(),
         x2 = P2.getX(),
         x3 = P3.getX();
  double z1 = P1.getZ(),
         z2 = P2.getZ(),        
         z3 = P3.getZ();

  int r1 = P1.getR(),
      r2 = P2.getR(),
      r3 = P3.getR(),

      g1 = P1.getG(),
      g2 = P2.getG(),
      g3 = P3.getG(),

      b1 = P1.getB(),
      b2 = P2.getB(),
      b3 = P3.getB();


  int actX1,actX2,actY,actR1,actR2,actG1,actG2,actB1,actB2;

  double actZ1,actZ2;
      

  double delta1,delta2;
  

  
  for(actY=y1+1;actY<y3;actY++){
    delta1 = ((double)(actY-y1))/((double)(y3-y1)); // on line P1->P3
    actX1 = (int)(x1 + delta1*(x3-x1));
    actZ1 = z1 + delta1*(z3-z1);
    actR1 = (int)(r1 + delta1*(r3-r1));
    actG1 = (int)(g1 + delta1*(g3-g1));
    actB1 = (int)(b1 + delta1*(b3-b1));
    if(actY<=y2){                // on line P1->P2 
       delta2 = ((double)(actY-y1))/((double)(y2-y1)); 
       actX2 = (int)(x1+delta2*(x2-x1));
       actZ2 = z1+delta2*(z2-z1);
       actR2 = (int)(r1+delta2*(r2-r1));
       actG2 = (int)(g1+delta2*(g2-g1));
       actB2 = (int)(b1+delta2*(b2-b1));
   
    } else{                  // on Line P2->P3
      delta2 = ((double)(actY-y2))/((double)(y3-y2));
      actX2 = (int)( x2+delta2*(x3-x2));
      actZ2 = z2+delta2*(z3-z2);
      actR2 = (int)(r2+delta2*(r3-r2));
      actG2 = (int)(g2+delta2*(g3-g2));
      actB2 = (int)(b2+delta2*(b3-b2));
    }

    paintHorizontalLine(actY, actX1,actZ1,actR1,actG1,actB1,
                              actX2,actZ2,actR2,actG2,actB2);
   
  }
  

  if(border){
     Point3D P1_1 = P1.duplicate();
     P1_1.setR(0);P1_1.setG(0);P1_1.setB(0);
     Point3D P2_1 = P2.duplicate();
     P2_1.setR(0);P2_1.setG(0);P2_1.setB(0);
     Point3D P3_1 = P3.duplicate();
     P3_1.setR(0);P3_1.setG(0);P3_1.setB(0);
     paintSingleLine(new Line3D(P1_1,P2_1));
     paintSingleLine(new Line3D(P1_1,P3_1));
     paintSingleLine(new Line3D(P2_1,P3_1));
     
  
  
  }
  
  
}


/** paints a horizontal Line */
private void paintHorizontalLine(int y,int x1,double z1,int r1,int g1,int b1,
                                       int x2,double z2,int r2,int g2,int b2){

  if(x2==x1)
    return;

  if(x2<x1){  // swap all values
    int tmpi;
    double tmpd;
    tmpi=x1;
    x1=x2;
    x2=tmpi;
    tmpd=z1;
    z1=z2;
    z2=tmpd;
    tmpi=r1;
    r1=r2;
    r2=tmpi;
    tmpi=g1;
    g1=g2;
    g2=tmpi;
    tmpi=b1;
    b1=b2;
    b2=tmpi;
   }

   double delta;
   int r,g,b;
   int ColorValue;
   double z;
   for(int actX=x1;actX<=x2;actX++){
     delta=((double)(actX-x1))/((double)(x2-x1));
     r=(int)(r1+delta*(r2-r1));
     g=(int)(g1+delta*(g2-g1));
     b=(int)(b1+delta*(b2-b1));
     z=z1+delta*(z2-z1);
     ColorValue = (int) 4278190080L +              // Alpha-Value
                  (int) r * 65536 +
                  (int) g * 256   +
                  (int) b;
     set(actX,y,z,ColorValue);
   }
                                       
}                                       


/** set the BoundingBox of the to drawing world */
public void setBoundingBox(BoundingBox2D BB2){
 double minX=BB2.getMinX(),
     maxX=BB2.getMaxX(),
     minY=BB2.getMinY(),
     maxY=BB2.getMaxY();
 // ensure a minimum size (avoid division by zero)
 if(maxX-minX<10) maxX+=10;
 if(maxY-minY<10) maxY+=10;
 
 if(proportional){ // we must enlarge WorldWidth or Worldheight
    double ScreenRatio = (double)width / height;
    double WorldWidth = maxX-minX;
    double WorldHeight = maxY-minY;
    double WorldRatio = WorldWidth/WorldHeight;
    if(ScreenRatio<WorldRatio)
        WorldHeight = WorldWidth / ScreenRatio;
    if(ScreenRatio>WorldRatio)
        WorldWidth = WorldHeight*ScreenRatio;
    maxY = minY + WorldHeight;
    maxX = minX + WorldWidth;
 }
 BB.setTo(minX,minY,maxX,maxY);
 transformObjects();
}


/** returns the current bounding box */
public BoundingBox2D getBoundingBox(){
  return BB;
}

/** set the BoundingBox of the World ingnoring the z-values */
public void setBoundingBox(BoundingBox3D BB3){
  BB.readFrom(BB3);
  setBoundingBox(BB); // to ensure a minimum size
}


/** converts all Objects from given World-Bounding-Box BB to the Screen */
private void transformObjects(){
  TransformedPoints.empty();
  TransformedLines.empty();
  TransformedTriangles.empty();
  for(int i=0;i<Points.getSize();i++)
     transformAndInsert(Points.get(i));
  for(int i=0;i<Lines.getSize();i++)
     transformAndInsert(Lines.get(i));
  for(int i=0;i<Triangles.getSize();i++)
     transformAndInsert(Triangles.get(i));
}


/** Transforms a point from world into screen coordinates */
private void transform(Point3D P){
  double x = P.getX();
  double y = P.getY();
  double z = P.getZ();
  // first translate to (0,0)
  x = x-BB.getMinX();
  y = y-BB.getMinY();
  // Scale 
  double sx = width/BB.getWidth();
  double sy = height/BB.getHeight();
  if(proportional){          
     sx = Math.min(sx,sy);
     sy = sx;
  }
  x = x*sx;
  // 
  y = height- y*sy;
  P.moveTo(x,y,z);
}


/* returns the world coordinates for the given screen coordinates */
public double[] getWorld(int ScreenX,int ScreenY){
 double[] res = new double[2];
 double sx = BB.getWidth()/width;
 double sy = BB.getHeight()/height;
 if(proportional){
    sx = Math.max(sx,sy);
    sy = sx;
 }
 
 double x = sx*ScreenX+BB.getMinX();
 double y = (height-ScreenY)*sy+BB.getMinY();
 res[0]=x;
 res[1]=y;
 return res; 
}



/** returns the exactness in the world if given in Pixels */
public double getWorldAccuracy(int PixelSize){
   return ((double)PixelSize)* Math.max( BB.getWidth()/width , BB.getHeight()/height);
}


/* if P in in the current World-BoundingBox then
 * transform P a insert the transformed Point into
 * the TransformedPoints vector
 */
private void transformAndInsert(IDPoint3D P){
   TMPBB.readFrom(P.getBoundingBox());
   if(BB.intersect(TMPBB)){
      Point3D P2 = P.duplicate();
      transform(P2);   
      IDPoint3D P3 = new IDPoint3D(P2.getX(),P2.getY(),P2.getZ(),P.getR(),P.getG(),P.getB(),P.getID());
      TransformedPoints.append(P3);
   }   
}


/** if the boundingbox of L intersects the
  * current BoundingBox then is L transformed to screen
  * coordinates and inserted in TransformedLines Vector */
private void transformAndInsert(Line3D L){
  TMPBB.readFrom(L.getBoundingBox());
  if(BB.intersect(TMPBB)){
     ID id = L.getID();
     Point3D P1 = L.getEP1();
     Point3D P2 = L.getEP2();
     transform(P1);
     transform(P2);
     Line3D NL = new Line3D(P1,P2,id);
     TransformedLines.append(NL);
   }  
}


/* if the BoundingBox of T intersects the current bounding box
 * the is T transformed to screen coordinates and 
 * inserted in the Vector TransformedTriangles 
 */
private void transformAndInsert(Triangle3D T){
   TMPBB.readFrom(T.getBoundingBox());
   if(BB.intersect(TMPBB)){
      ID id = T.getID();
      Point3D P1 = T.getCP1().duplicate();
      Point3D P2 = T.getCP2().duplicate();
      Point3D P3 = T.getCP3().duplicate();
      transform(P1);
      transform(P2);
      transform(P3);
      Triangle3D NT = new Triangle3D(P1,P2,P3,id);
      TMPBB.readFrom(NT.getBoundingBox());
      TransformedTriangles.append(NT);
   }   
}

/* remove all figures with given ID */
public void remove(ID id){

  int i=0;
  while(i<Points.getSize()){
     if(Points.get(i).getID().equals(id))
        Points.remove(i);
     else
         i++;
  }
  
  
  i=0;
  while(i<Lines.getSize()){
     if(Lines.get(i).getID().equals(id))
        Lines.remove(i);
     else
         i++;
  }
  
  i=0;
  while(i<Triangles.getSize()){
     if(Triangles.get(i).getID().equals(id))
        Triangles.remove(i);
     else
         i++;
  }
  
  i=0;
  while(i<TransformedPoints.getSize()){
     if(TransformedPoints.get(i).getID().equals(id))
        TransformedPoints.remove(i);
     else
         i++;
  }
    
  i=0;
  while(i<TransformedLines.getSize()){
     if(TransformedLines.get(i).getID().equals(id))
        TransformedLines.remove(i);
     else
         i++;
  }
  
  i=0;
  while(i<TransformedTriangles.getSize()){
     if(TransformedTriangles.get(i).getID().equals(id))
        TransformedTriangles.remove(i);
     else
         i++;
  }
  

}

/** turn the painting of triangleborders on or off */
public void paintBorders(boolean on){
  border = on;
}

/** returns true if border painting enabled */
public boolean isPaintingBorders(){
  return border;
}


/** return the current diameter for painting points (in pixels) */
public int getPointSize(){
  return PointSize;
}

/** set the size for painting points */
public void setPointSize(int Size){
  PointSize = Math.max(1,Size);
}


/** contains the Points in world coordinates */
private IDPoint3DVector Points=new IDPoint3DVector();
/** contains all Lines in world coordinates */
private Line3DVector Lines = new Line3DVector();
/** contsians all triangles in world coordinates */
private Triangle3DVector Triangles = new Triangle3DVector();
/** contains the points to displayed in screen coordinates */
private IDPoint3DVector TransformedPoints = new IDPoint3DVector();
/** contains the lines to displayed in screen coordinates */
private Line3DVector TransformedLines = new Line3DVector();
/** contains triangles in screen coordinates */
private Triangle3DVector TransformedTriangles = new Triangle3DVector();
/** the current bounding box */
private BoundingBox2D BB = new BoundingBox2D();
/** the ZBuffer of this image */
private double[][] ZBuffer;
/** the width of this images */
private int width;
/** the height of this image */
private int height;
/** paint a border for triangles */
private boolean border = false;
/** keep the side ratio */
private boolean proportional = true;

/**this is to avoid to create/destroy many BoundingBoxes */
private static BoundingBox2D TMPBB = new BoundingBox2D(); 


private int PointSize = 15;

}













// $Extended$File$Info$
//
