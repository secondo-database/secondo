package viewer.viewer3d.graphic3d;

/********************************
*
* Autor   : Thomas Behr
* Version : 1.1
* Datum   : 3.7.2000
*
*************************************/



class  Clipping {

private static double ZPlane;

// the Clipping-Planes
private final static int  FRONT = 0;
private final static int  TOP = 1;
private final static int  BUTTOM = 2;
private final static int  LEFT = 3;
private final static int  RIGHT = 4;



/** check wether (x,y) is in the uni-pyramid */
private static boolean isIn(double x, double y, double z,int Fct) {
   switch (Fct) {
      case FRONT  :  return z>=ZPlane; 
      case TOP    :  return  x<z;                  
      case BUTTOM :  return  x>=-z;                
      case LEFT   :  return  -z<=y;
      case RIGHT  :  return  y<=z;                
      default     : return false;
   }
}


/** compute lambda for cutpoint of a line with given plane */
private static double computeLambda(double x1, double y1 , double z1,
                     double x2, double y2 , double z2,
                     int Fct) {

     switch  (Fct)   {
       case FRONT   : return  (ZPlane-z1)/(z2-z1);  
       case TOP     : return (x1-z1) / ( ( z2-z1) - (x2-x1));  
       case BUTTOM  : return (-z1-x1) / ( (x2-x1) + (z2-z1));  
       case LEFT    : return (-z1-y1) / ( (y2-y1) + (z2-z1));  
       case RIGHT   : return (y1-z1) / ((z2-z1) - (y2-y1));    
       default      : return -1.0;
     }
}


/** clip a figure on a plane of the uni-pyramide */
private static void clip(Figure3D Figure,int Fct) {
  Figure3D  FHelp;
  Point3D   Pt;
  double    x1,y1,z1,x2,y2,z2;
  int       c1r,c1g,c1b, c2r,c2g,c2b;   //Colors of Points
  int       R,G,B;                     // Colors of Clippoints

  double    X,Y,Z;
  boolean   In1,In2;
  double    Lambda;
  Point3D   Current;
  int       numberOfPoints;

 if (! Figure.isEmpty() ) {

    numberOfPoints = Figure.getSize();

    FHelp = Figure.duplicate();
    Figure.removePoints();

   // take the first Point

    Current = FHelp.getPoint3DAt(0);
    x1  = Current.getX();
    y1  = Current.getY();
    z1  = Current.getZ();
    c1r = Current.getR();
    c1g = Current.getG();
    c1b = Current.getB();


    In1 = isIn(x1,y1,z1,Fct);

    if (In1) { Figure.addPoint(x1,y1,z1,c1r,c1g,c1b); }

    for(int i=1; i<FHelp.getSize(); i++)
       { Current = FHelp.getPoint3DAt(i);
         x2  = Current.getX();
         y2  = Current.getY();
         z2  = Current.getZ();
         c2r = Current.getR();
         c2g = Current.getG();
         c2b = Current.getB();

         In2 = isIn(x2,y2,z2,Fct);

         // distinct cases

         if (In1) {
            if(In2)   {  // both point are in clipping-pyramid
               Figure.addPoint(x2,y2,z2,c2r,c2g,c2b);
               }
            else {
               Lambda = computeLambda(x1,y1,z1,x2,y2,z2,Fct);
               X = x1 + Lambda*(x2-x1);
               Y = y1 + Lambda*(y2-y1);
               Z = z1 + Lambda*(z2-z1);
               R = (int)(c1r + Lambda*(c2r-c1r));
               G = (int)(c1g + Lambda*(c2g-c1g));
               B = (int)(c1b + Lambda*(c2b-c1b));
               Figure.addPoint(X,Y,Z,R,G,B);
            }
            }
         else {  // first point in exterior of pyramid
            if (In2) {
               Lambda = computeLambda(x2,y2,z2,x1,y1,z1,Fct);
               X = x2 + Lambda*(x1-x2);
               Y = y2 + Lambda*(y1-y2);
               Z = z2 + Lambda*(z1-z2);
               R = (int)(c2r + Lambda*(c1r-c2r));
               G = (int)(c2g + Lambda*(c1g-c2g));
               B = (int)(c2b + Lambda*(c1b-c2b));

               Figure.addPoint(X,Y,Z,R,G,B);
               Figure.addPoint(x2,y2,z2,c2r,c2g,c2b);
             }
          } // first if

         x1 = x2;
         y1 = y2;
         z1 = z2;
         c1r = c2r;
         c1g = c2g;
         c1b = c2b;
         In1 = In2;
     }  // for     

     //  the First Point
     if (numberOfPoints>2) {
      Current = FHelp.getPoint3DAt(0);
      x2 = Current.getX();
      y2 = Current.getY();
      z2 = Current.getZ();
      c2r = Current.getR();
      c2g = Current.getG();
      c2b = Current.getB();

      In2 = isIn(x2,y2,z2,Fct);
      
      if (In1 ^ In2 ) {  
        Lambda = computeLambda(x1,y1,z1 , x2,y2,z2,Fct);
        X = x1 + Lambda*(x2-x1);
        Y = y1 + Lambda*(y2-y1);
        Z = z1 + Lambda*(z2-z1);
        R = (int)(c1r + Lambda*(c2r-c1r));
        G = (int)(c1g + Lambda*(c2g-c1g));
        B = (int)(c1b + Lambda*(c2b-c1b));
        Figure.addPoint(X,Y,Z,R,G,B);
      }
    } // numberOfPoints>2
   } // if (Figure.isEmpty() )     

} // Clip




/** clip a figure by given Z_{min} */
public static void clipFigure(Figure3D Fig,double VB) {

 if (VB > 0.0) ZPlane=VB;
 else          ZPlane=0.1;
 

 clip(Fig,FRONT);
 clip(Fig,TOP); 
 clip(Fig,RIGHT);   
 clip(Fig,BUTTOM); 
 clip(Fig,LEFT);

  }


} // Clipping
