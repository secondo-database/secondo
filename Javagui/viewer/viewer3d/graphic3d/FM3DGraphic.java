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

import viewer.viewer3d.mathe.Matrix;
import viewer.viewer3d.graphic2d.*;

/******************************
*
* Autor   : Thomas Behr
* Version : 1.1
* Datum   : 16.5.2000
*
*******************************/

public class FM3DGraphic{


/**
  * creates a new FM3DGraphic;
  * initialize variables
  */
public FM3DGraphic() {
  // Initialize global variables 
  RotAngle = 0.0;                     // for Rotation
  RotPt1 = new Point3D(0,0,0,0,0,0);
  RotPt2 = new Point3D(0,0,5,0,0,0); 
  RotationMatrix = new Matrix(4,4);
  computeRotationMatrix();

  TranslationMatrix = computeTranslationMatrix(0.0,0.0,0.0);

  Volume.z_min =  0.01;    // the visible part
  Volume.Height = 8.0;
  Volume.Width = 8.0;

  NormalMatrix = new Matrix(4,4);  // Matrix to normalize

  EYE    = new Point3D(0,-1,0,0,0,0);  // values for the visitor
  VRP    = new Point3D(0,0,0,0,0,0); 
  ViewUp = new Point3D(0,0,1,0,0,0);  
  d      = EYE.distance(VRP);

  computeNormalMatrix(); 
  ViewMatrix   = new Matrix(4,4);
  ViewUndo     = new Matrix(4,4);

  computeViewMatrix();

  setWindow(5.0,5.0);
}


/** returns the eye-position of the view */
 public Point3D getEye()    { return EYE.duplicate(); }
/** returns the view-reference-point of the view */
 public Point3D getVRP()    { return VRP.duplicate(); }
/** returns the ViewUp-Point of the view */
 public Point3D getViewUp() { return ViewUp.duplicate(); }

/** check wether the view is valid; */
 public static boolean checkView(double XEye, double YEye, double ZEye,
                                 double XVRP, double YVRP, double ZVRP,
                                 double XVU,  double YVU,  double ZVU){

 // check :  View is ok ?, i.e.
 //          result = true if Eye,VRP and ViewUp not on a line

   double    xdif,ydif,zdif,xges,yges,zges;
  
   xdif = XVRP-XEye;
   ydif = YVRP-YEye;
   zdif = ZVRP-ZEye;

   Point3D Point = new Point3D(0,0,0,0,0,0);

   cross(XVU,YVU,ZVU,xdif,ydif,zdif,Point);

   xges = Point.getX();
   yges = Point.getY();
   zges = Point.getZ();

   double C = absolute(xges,yges,zges);
   return C >0.0;

}


/**
  * set the view
  */
public void setView( double IXE, double IYE , double IZE,
                      double IXVRP, double IYVRP, double IZVRP,
                      double IXVU, double IYVU, double IZVU)

 throws Exception  {

 /* ################################################################
    setzt das Auge des Beobachters auf (IXE,IYE,IZE)
    setzt den ViewReferenzpoit     auf (IXVRP,IYVRP,IZVRP)
    setzt den ViewUpPoint          auf (IXVU,IYVU,IZVU)

    loest bei einer falschen Eingabe (alle Punkte liegen auf einer Linie)
    eine Exception aus.
    ################################################################# */

    
  double    xdif,ydif,zdif,xges,yges,zges;
  boolean   ok;
  
   xdif = IXVRP-IXE;
   ydif = IYVRP-IYE;
   zdif = IZVRP-IZE;

   Point3D Point = new Point3D(0,0,0,0,0,0);

   cross(IXVU,IYVU,IZVU,xdif,ydif,zdif,Point);

   xges = Point.getX();
   yges = Point.getY();
   zges = Point.getZ();

   ok = absolute(xges,yges,zges) >0.0;

   if (ok) {
      EYE    = new Point3D(IXE,IYE,IZE,0,0,0);
      VRP    = new Point3D(IXVRP,IYVRP,IZVRP,0,0,0);
      ViewUp = new Point3D(IXVU,IYVU,IZVU,0,0,0);
      d = EYE.distance(VRP);
      computeNormalMatrix();
      computeViewMatrix();
   }
   else
     { throw new Exception(); }
   
 }


/** set the window of the view */
public void setWindow(double width, double height) {
    Volume.Width  = width;
    Volume.Height = height;
    computeViewMatrix();
}


/** set the frontplane of the view-pyramid */
public void setZPlane(double z_min) {
   // set the frontplane of the view pyramide
    Volume.z_min = z_min;
 }

/** get the with of the window */
public double getWindowWidth() { return Volume.Width; }

/** get the height of the window */
public double getWindowHeight() { return Volume.Height; }

/** get the with of the window */
public double getWindowX() { return Volume.Width;}
/** get the height of the window */
public double getWindowY() { return Volume.Height;}


/** returns the distance between eye and vrp */
public double readDistance() { return d; }


/** set the viewport */
public void setViewport(double x0, double y0, double Width,
                         double Height) {

   ViewPort.x0 = x0;
   ViewPort.y0 = y0;
   ViewPort.Width  = Width;
   ViewPort.Height = Height;
 }


/** get the x-coordinate of refernec-corner-point of the viewport */
public double getViewportX() { return ViewPort.x0; }
/** get the y-coordinate of refernec-corner-point of the viewport */
public double getViewportY() { return ViewPort.y0; }
/** get the with of the viewport */
public double getViewportWidth() { return ViewPort.Width; }
/** get the height of the viewport */
public double getViewportHeight() {return  ViewPort.Height;}


/** set a rotation-axis by 2 points */
public void setRotationAxis(double X1, double Y1, double Z1,
                            double X2, double Y2, double Z2) {

  Point3D Pt1, Pt2;

   Pt1 = new Point3D(X1,Y1,Z1,0,0,0);
   Pt2 = new Point3D(X2,Y2,Z2,0,0,0);   
   if (! Pt1.equals(Pt2) ) {
      RotPt1.equalize(Pt1);
      RotPt2.equalize(Pt2);
      computeRotationMatrix();
   }
}   

/** set the rotation-axis by Pt1 and Pt2 */
public void setRotationAxis(Point3D Pt1, Point3D Pt2) {

   if (! Pt1.equals(Pt2) ) {
      RotPt1.equalize(Pt1);
      RotPt2.equalize(Pt2);
      computeRotationMatrix();
   }
}

/** set the angle of rotation */
public void setRotationAngle(double alpha)  {
  RotAngle = alpha;
  computeRotationMatrix();
}

/** set the values for translation */
public void setTranslation(double XDif, double YDif,double ZDif) {
   TranslationMatrix = computeTranslationMatrix(XDif,YDif,ZDif);
} 

/**
  * transform a figure to 2-dim-representation
  */
public Figure2D figureTransformation(Figure3D Fig) {

 Figure3D   Copy;           // a copy from Input
 Figure2D   Transform;      // the result
 double      X3D,Y3D,Z3D;   // Coordinates of 3d_point
 double      X2D,Y2D;       // Coordinates of 2D-Point
 int         R,G,B;         // Color of Points
 double      Sort;          
 Point3D     Current;

 Copy = Fig.duplicate();  // save the original
 normalizeFigure(Copy);    // main_work
 clipFigure(Copy);        // clip on the View-pyramid
 Sort = getSort(Copy);    // Sort = distance to eye
 Transform = new Figure2D();
 Transform.setSort(Sort);
 Transform.setID(Copy.getID());
 if ( ! Copy.isEmpty() ) {   // empty from clipping ?
       projectFigure(Copy);
       for(int i=0; i< Copy.getSize(); i++) {
         Current = Copy.getPoint3DAt(i);
         X3D = Current.getX();
         Y3D = Current.getY();
         Z3D = Current.getZ();
         R = Current.getR();
         G = Current.getG();
         B = Current.getB();

   //   adapt to Viewport

        X2D =   ((X3D+(Volume.Width/2.0))*ViewPort.Width / Volume.Width)
                 + ViewPort.x0;
      
        Y2D =   ((Y3D+(Volume.Height/2.0))*ViewPort.Height/Volume.Height)
                 + ViewPort.y0;

        Transform.addPoint(new Point2D(X2D,Y2D,R,G,B));
     } // for
   } // if

   return Transform;
} 


/** normalize the figure */
public void normalizeFigure(Figure3D Fig) {
   transformFigure(Fig,NormalMatrix);
}


/** the projection of fig */
public void projectFigure(Figure3D Fig) {
  double  X,Y,Z,h;
  int     R,G,B;
  Point3D Current;

  for (int i=0; i< Fig.getSize(); i++) {
       Current = Fig.getPoint3DAt(i);
       X = Current.getX();
       Y = Current.getY();
       Z = Current.getZ();
       R = Current.getR();
       G = Current.getG();
       B = Current.getB();
       h = -(1.0+Z/d);
       X = X/h;
       Y = Y/h;
       Z = 0.0;
       Fig.setPoint3DAt(new Point3D(X,Y,Z,R,G,B),i);
  }
}  


/** move Fig by given translation */
public void moveFigure(Figure3D Fig)  {
  transformFigure(Fig,TranslationMatrix);
}


/** rotate a point */
public void rotatePoint(Point3D Pt) {
 transformPoint(Pt,RotationMatrix);
}

/** normalize a point */
public void normalizePoint(Point3D Pt) {
  transformPoint(Pt,NormalMatrix);
}

/** translate a point */
public void movePoint(Point3D Pt) {
   transformPoint(Pt,TranslationMatrix);
}

/** distance between VRP and EYE */
private  double   d;      //   (* distance from VRP to  EYE *)
/** the view reference point */
private  Point3D  VRP;
/* the position of the eye */
private  Point3D  EYE;
/** the viewUp-point */
private  Point3D  ViewUp;

/** a class to represent the volume */
private class VolumeC  { public double  Width = 6;
                         public double  Height = 6;
                         public double  z_min  = 6; 
                        } ;

/** the current Volume */
private VolumeC Volume = new VolumeC();

/** a class to represent the ViewPort */
private class  ViewPortC  { public double x0     = 100,
                                          y0     = 100,
                                          Width  = 100,
                                          Height = 100; };

/** the current ViewPort */
private ViewPortC ViewPort = new ViewPortC();

/** a rotation-Matrix */
private Matrix   RotationMatrix;
/** the angle of rotation */
private double   RotAngle;       //    rotationangle
/** a point of the rotation-axis */
private Point3D  RotPt1,RotPt2;  //    rotationaxis
/** the matrix for the translation */
private Matrix   TranslationMatrix;
/** the matrix to normalize */
private Matrix   NormalMatrix;

/** matrix to transform a figure from world to the view-pyramide */
private Matrix   ViewMatrix;  //   Matrices for the view pyramide
/** matrix to transform a figure from view-pyramide to the world */
private Matrix   ViewUndo; 

/** a clipping-object */
private Clipping Cl = new Clipping();

/** compute the sort for the painter-algorithm */
private double getSort(Figure3D Fig) {
  // compute sort of a normalized figure
   double Sort = Fig.distance(new Point3D(0,0,-d,0,0,0));
   return Sort;
}

/** compute the length (0,0,0)-> (X,Y,Z) */
private static double absolute(double X,double Y,double Z) {
     return Math.sqrt(X*X+Y*Y+Z*Z);
}

/** tranform a figure by give matrix */
private void transformFigure(Figure3D Fig, Matrix Mat) {
   double X,Y,Z;
   Point3D Pt;

  for (int i=0; i<Fig.getSize(); i++) {
      Pt = Fig.getPoint3DAt(i);
      transformPoint(Pt,Mat);
      Fig.setPoint3DAt(Pt,i);
  }  
}


/** clip a figure on the uni-pyramid */
private void clipFigure(Figure3D Fl) {

   transformFigure(Fl,ViewMatrix);
   Cl.clipFigure(Fl,Volume.z_min/d);
   transformFigure(Fl,ViewUndo);
}


/** transform a single point by given matrix */
private void transformPoint(Point3D Pt, Matrix Mat)  {

    Matrix  PtMat;     // (* Pt in homogenius coordinates *)
    Matrix  TransMat;   //    transformed Matrix *)
    Point3D TransPt;   // (* transformed Point *)
    int R,G,B;          // Color of Point

   PtMat   = Point2Matrix(Pt);
   R = Pt.getR();
   G = Pt.getG();
   B = Pt.getB();

   TransMat = PtMat.mul(Mat);
   TransPt  = Matrix2Point(TransMat,R,G,B);
   Pt.equalize(TransPt);
}


/** compute the cross-product */
private static void cross( double  x1, double y1, double z1,
                           double  x2, double y2, double z2,
                           Point3D Goal) {


  Goal.setX(y1*z2 - z1*y2);
  Goal.setY(z1*x2 - x1*z2);
  Goal.setZ(x1*y2 - y1*x2);

}

/** normalize a single point */
private void normalize(Point3D Pt) {
  double X,Y,Z, ABS;

  X = Pt.getX();
  Y = Pt.getY();
  Z = Pt.getZ();

  ABS = Math.sqrt(X*X+Y*Y+Z*Z);
   X = X/ABS;
   Y = Y/ABS;
   Z = Z/ABS;
   Pt.moveTo(X,Y,Z);
}


/** computes the matrix for normalize by given view */
private void  computeNormalMatrix() {

  double  x1,y1,z1,x2,y2,z2;   
  double  x3,y3,z3,x4,y4,z4; 
  double  B;                              //  (* absolute *)
  Point3D vzStrich,vxStrich, vyStrich;
  Matrix  Translat,Rotat,Norma;

  // (* fuer die Berechnungen siehe Musterloesung zu Graphische
  //    Datenverarbeitung I KE3 , Aufgabe 4 *)


  x2 = VRP.getX()- EYE.getX();
  y2 = VRP.getY()- EYE.getY();
  z2 = VRP.getZ()- EYE.getZ();

  Point3D Help = new Point3D(x2,y2,z2,0,0,0);
  normalize(Help);
  x2 = Help.getX();
  y2 = Help.getY();
  z2 = Help.getZ();

  vzStrich =  Help;

  x1 = ViewUp.getX();
  y1 = ViewUp.getY();
  z1 = ViewUp.getZ();

  // (* x2 ,y2,z2 sind noch die Koordinaten von vzStrich *)

  Point3D Pt3 = new Point3D(0,0,0,0,0,0);

  cross(x1,y1,z1,x2,y2,z2,Pt3);
  normalize(Pt3);

  x3 = Pt3.getX();
  y3 = Pt3.getY();
  z3 = Pt3.getZ();

  vxStrich = new Point3D(x3,y3,z3,0,0,0);

  //  (* x2 ... ist vzStrich   und x3 ... ist vxStrich  *)

  Point3D Pt4 = new Point3D(0,0,0,0,0,0);

  cross(x2,y2,z2,x3,y3,z3,Pt4);

  x4 = Pt4.getX();
  y4 = Pt4.getY();
  z4 = Pt4.getZ();

  vyStrich = new Point3D(x4,y4,z4,0,0,0);

  Translat = new Matrix(4,4);
  Translat.setValue(0,0, 1.0);
  Translat.setValue(1,1, 1.0);
  Translat.setValue(2,2, 1.0);
  Translat.setValue(3,3, 1.0);
  Translat.setValue(3,0, -VRP.getX() );
  Translat.setValue(3,1, -VRP.getY() );
  Translat.setValue(3,2, -VRP.getZ() );

  // (* Translat verschiebt VRP in den Ursprung *)

  Rotat = new Matrix(4,4);
  Rotat.setValue(0,0, vxStrich.getX());
  Rotat.setValue(0,1, vyStrich.getX()); 
  Rotat.setValue(0,2, vzStrich.getX());

  Rotat.setValue(1,0, vxStrich.getY());
  Rotat.setValue(1,1, vyStrich.getY());
  Rotat.setValue(1,2, vzStrich.getY());

  Rotat.setValue(2,0, vxStrich.getZ());
  Rotat.setValue(2,1, vyStrich.getZ());
  Rotat.setValue(2,2, vzStrich.getZ());

  Rotat.setValue(3,3, 1.0 );

  Norma = Translat.mul(Rotat); 

  NormalMatrix.equalize(Norma);
}


/** compute matrix for rotation by given rotation-axis and angle */
private void computeRotationMatrix() {

Point3D  Help;
Matrix   Transl1,Transl2,Rotation,Part,Total;
double   x,y,z;
int      i,j;
double   W; 


  /* die Prozedur verwendet die Punkte Rotpkt1 und RotPkt2
     (globale Variablen des Moduls) zur Angabe einer Achse und
     RotWinkel zu Angabe des Rotationswinkels */

 // 1. Translation der Achse in den Urspung *)


  x = RotPt1.getX()-RotPt2.getX();
  y = RotPt1.getY()-RotPt2.getY();
  z = RotPt1.getZ()-RotPt2.getZ(); 
 
  Help = new Point3D(x,y,z,0,0,0);  //  (* verschobener RotPkt1 *)
 
  Transl1 = computeTranslationMatrix(-RotPt2.getX(),
                                     -RotPt2.getY(),
                                     -RotPt2.getZ() );

  Transl2 = computeTranslationMatrix( RotPt2.getX(),
                                      RotPt2.getY(),
                                      RotPt2.getZ()   );

  Rotation = PointRotationMatrix(Help,RotAngle); 

  Part   = Transl1.mul(Rotation);
  Total  = Part.mul(Transl2);  

  RotationMatrix.equalize(Total);
}

 // compute elementary Matrices


/** coputes the matrix to translate a figure by given translation */
private Matrix computeTranslationMatrix(double x, double y, double z ) {
  Matrix  Transmat;
  Transmat = new Matrix(4,4);
  Transmat.setValue(0,0, 1.0 );
  Transmat.setValue(1,1, 1.0 );
  Transmat.setValue(2,2, 1.0 );
  Transmat.setValue(3,3, 1.0 );
  Transmat.setValue(3,0, x   );
  Transmat.setValue(3,1, y   );
  Transmat.setValue(3,2, z   );
  return Transmat;
}

/** comptes the matrix to convert the view-pyramid to uni-pyramid */
private void computeViewMatrix() {
 Matrix  M1,M2,M3;
 double  xs,ys,zs;  //  factors of scale
 double  xt,yt,zt;  //  for translation

  // (* die Sichtmatrix formt die gegebene Sichtpyramide in die 
  //    genormte Sichtpyramide um *)

  // (* Die Matrix Sichtzurueck ist die Inverse Dieser Matrix *)

   xt = 0.0;    //   (* Verschiebung von Auge nach 0 *)
   yt = 0.0;            
   zt = d;      //   (* Abstand von Ursprung *)

   xs = 2.0/Volume.Width;  // (* Skalierung zur Normierten Pyramide *)
   ys = 2.0/Volume.Height;
   zs = 1.0/d;

   M1 = computeTranslationMatrix(xt,yt,zt);
   M2 = computeScaleMatrix(xs,ys,zs);
   M3 = M1.mul(M2);

   ViewMatrix.equalize(M3);
   
   zt = -d;
   xs = Volume.Width/2.0;
   ys = Volume.Height/2.0;
   zs = d;

   M1 = computeTranslationMatrix(xt,yt,zt);
   M2 = computeScaleMatrix(xs,ys,zs);
   M3 = M2.mul(M1); 
   ViewUndo.equalize(M3);
}

/** compute a rotation-matrix */
private Matrix PointRotationMatrix(Point3D Pt,double Angle) {
  Matrix   Mat; 
  double   s,t,c,B,x,y,z;
  Point3D  Help;

  Mat  = new Matrix(4,4); 
  Help = new Point3D(0,0,0,0,0,0);
 
  B = Help.distance(Pt);  // (* Laenge des Ortsvektors Pkt *)

  x = Pt.getX()/B;
  y = Pt.getY()/B;
  z = Pt.getZ()/B;  //   (* Normierung *)

  s = Math.sin(Angle);
  c = Math.cos(Angle);
  t = 1.0-Math.cos(Angle);
 
  Mat.setValue(0,0, t*x*x+c     );
  Mat.setValue(0,1, t*x*y + s*z );
  Mat.setValue(0,2, t*x*z - s*y );
  Mat.setValue(0,3,  0.0        );
  Mat.setValue(1,0, t*x*y-s*z   );
  Mat.setValue(1,1, t*y*y + c   );
  Mat.setValue(1,2, t*y*z + s*x );
  Mat.setValue(2,0, t*x*z + s*y );
  Mat.setValue(2,1, t*y*z - s*x );
  Mat.setValue(2,2, t*z*z + c   );
  Mat.setValue(3,3, 1.0         );
  return Mat;
 } 

/** compute a matrix to scale */
private Matrix computeScaleMatrix(double x, double y, double z ) {

  Matrix M ;
  M = new Matrix(4,4);
  M.setValue(0,0,x   );
  M.setValue(1,1,y   );
  M.setValue(2,2,z   );
  M.setValue(3,3,1.0 );
  return M;
}


/** convert a point to a matrix */
private Matrix Point2Matrix(Point3D Pt) {
 Matrix Mat; 
 Mat =  new Matrix(4,4);
 Mat.setValue(0,0,Pt.getX() );
 Mat.setValue(0,1,Pt.getY() );
 Mat.setValue(0,2,Pt.getZ() );
 Mat.setValue(0,3, 1.0       );
 return Mat;
} 

/** convert a matrix to a point */
private Point3D Matrix2Point(Matrix M,int R,int G,int B) {
 Point3D  Pt;
 double   x,y,z,w;

  x = M.getValue(0,0);
  y = M.getValue(0,1);
  z = M.getValue(0,2);
  w = M.getValue(0,3);
  if  (w != 1.0) {
     x = x / w;
     y = y /w;
     z = z / w;
  }

  Pt = new Point3D(x,y,z,R,G,B);
  return Pt;
}
 

} // class



