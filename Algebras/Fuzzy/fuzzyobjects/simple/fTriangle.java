package fuzzyobjects.simple;

import fuzzyobjects.basic.*;
import sj.lang.ListExpr;

/**
  * this class provides fuzzy triangles
  * @author Thomas Behr
  */
public class fTriangle implements SimpleObject{

/**
 * creates a new fTriangle
 * no check whether this results a valid triangle
 * @param BP1,BP2,BP3 the location of this Triangle
 * @param Z1,Z2,Z3 the degrees of fuzziness in the cornerpoints
 */
public fTriangle( BasicPoint BP1, double Z1,
                  BasicPoint BP2, double Z2,
                  BasicPoint BP3, double Z3 ) {

fEPoint[] fEPs = new fEPoint[3];
fEPs[0] = new fEPoint(BP1,Z1);
fEPs[1] = new fEPoint(BP2,Z2);
fEPs[2] = new fEPoint(BP3,Z3);

fEPoint.sort(fEPs);

CP_1 = (BasicPoint) fEPs[0].basic();
CP_2 = (BasicPoint) fEPs[1].basic();
CP_3 = (BasicPoint) fEPs[2].basic();
this.Z1 = fEPs[0].getZ();
this.Z2 = fEPs[1].getZ();
this.Z3 = fEPs[2].getZ();
}

/* create a triabgle from 3 fuzzy points */
public fTriangle(fEPoint P1,fEPoint P2,fEPoint P3){
  fEPoint[] fEPs = new fEPoint[3];
  fEPs[0] = P1;
  fEPs[1] = P2;
  fEPs[2] = P3;
  fEPoint.sort(fEPs);
CP_1 = (BasicPoint) fEPs[0].basic();
CP_2 = (BasicPoint) fEPs[1].basic();
CP_3 = (BasicPoint) fEPs[2].basic();
this.Z1 = fEPs[0].getZ();
this.Z2 = fEPs[1].getZ();
this.Z3 = fEPs[2].getZ();


}


/* get the first corner point */
public fEPoint getP1(){
  return new fEPoint(CP_1,Z1);
}

/* gte the second cornerpoint */
public fEPoint getP2(){
  return new fEPoint(CP_2,Z2);
}

/* get the last corner point */
public fEPoint getP3(){
  return new fEPoint(CP_3,Z3);
}


/**
 * creates a new fTriangle
 * whithout check of validity
 * @param x?,y? the location of cornerpoints
 * @param Z?    the degree of fuzziness in a cornerpoint
 */
public fTriangle( int x1, int y1, double Z1,
                  int x2, int y2, double Z2,
                  int x3, int y3, double Z3){
  this( ( new BasicPoint(x1,y1)), Z1,
        ( new BasicPoint(x2,y2)), Z2,
        ( new BasicPoint(x3,y3)), Z3);

}

/**
 * creates a new fTriangle
 * whithout check of validity
 * @param BT the location of this Triangle
 * @param Z? the degree of fuzziness in a cornerPoint
 */
public fTriangle( BasicTriangle BT, double Z1, double Z2, double Z3){
   this (  BT.getCP1(),Z1, BT.getCP2(), Z2, BT.getCP3(),Z3 );
}

/**
 * returns whether this and fT are equal
 */
public boolean equals(fTriangle fT){
 if(fT == null)
    return false;
 else{
    return CP_1.equals(fT.CP_1) & CP_2.equals(fT.CP_2) &
           CP_3.equals(fT.CP_3) & (Z1==fT.Z1)        &
           (Z2 == fT.Z2)      & (Z3==fT.Z3);
 }

}

/** returns a String representing this Triangle */
public String toString(){
  return "FTriangle :"+
          "["+ CP_1+","+Z1+"]["+CP_2+","+Z2+"]["+CP_3+","+Z3+"]";

}

/** returns a copy of this */
public fTriangle copy(){
  return new fTriangle(CP_1.copy(),Z1,
                       CP_2.copy(),Z2,
                       CP_3.copy(),Z3);
} 

/** returns the location of this */
public BasicObject basic(){
 BasicTriangle result = new BasicTriangle(CP_1,CP_2,CP_3);
 return result;
}

/** returns the degree of fuzziness of the smallest point */
public double getZ1(){
   return Z1;
}

/** returns the degree of fuzziness of the middle point */
public double getZ2(){
   return Z2;
}

/** returns the degree of fuzziness of the greatest point */
public double getZ3(){
    return Z3;
}


/** returns the degree of fuzziness on a given Point
  * @param BP the given point
  */
public double Zfkt(BasicPoint BP){
 double result=0.0;
 if (BP.equals(CP_1))
    result = Z1;
 else
   if (BP.equals(CP_2))
      result=Z2;
   else
     if (BP.equals(CP_3))
        result = Z3;
 return result;
}

/** returns the degree of fuzziness on a given point
  * this point must be contained in the pointset of this
  * @param x,y the given Point
  */
public double Zfkt(double x, double y){
// returns the membership-value for this coordinate
// no check : (x,y) on Triangle

if (BasicPoint.isBasicPoint(x,y)){
  BasicPoint BP = new BasicPoint( (int)x, (int) y);
  return Zfkt(BP);
}

// not a BasicPoint

double x1 = CP_1.getX();
double y1 = CP_1.getY();
double z1 = Z1;
double x2 = CP_2.getX();
double y2 = CP_2.getY();
double z2 = Z2;
double x3 = CP_3.getX();
double y3 = CP_3.getY();
double z3 = Z3;

// sort by y-values

double y_max = Math.max(y1,Math.max(y2,y3));
double y_min = Math.min(y1,Math.min(y2,y3));
double y_mid = (y1+y2+y3) - y_max - y_min;

double x_max,x_mid,x_min,z_max,z_mid,z_min;

x_max = x_mid = x_min = z_max=z_mid=z_min=0;

if(y_max==y1){
   x_max=x1;
   z_max=z1;
   if(y_mid==y2){
      x_mid=x2;
      z_mid=z2;
      x_min=x3;
      z_min=z3;
   }
   else{
      x_mid=x3;
      z_mid=z3;
      x_min=x2;
      z_min=z2;
   }
}
else
if(y_max==y2){
  x_max=x2;
  z_max=z2;
  if(y1==y_mid){
    x_mid=x1;
    z_mid=z1;
    x_min=x3;
    z_min=z3;
  }
  else{
    x_mid=x3;
    z_mid=z3;
    x_min=x1;
    z_min=z1;
  }
}
else
if(y_max==y3){
  x_max=x3;
  z_max=z3;
  if(y1==y_mid){
   x_mid=x1;
   z_mid=z1;
   x_min=x2;
   z_min=z2;
  }
  else{
    x_mid=x2;
    z_mid=z2;
    x_min=x1;
    z_min=z1;
  }
}


// compute x and z-Values on Segment (y_min->y_max)
// whith scanline y
double delta1 = (y-y_min)/(y_max-y_min);
double x_sl_1 = (x_min) + delta1*(x_max-x_min);
double z_sl_1 = (z_min) + delta1*(z_max-z_min);



double x_sl_2;
double z_sl_2;
double delta2;

if(y>y_mid) { // scanline intersect line y_mid -> y_max
  delta2 = (y-y_mid)/(y_max-y_mid);
  x_sl_2 = x_mid + delta2*(x_max-x_mid);
  z_sl_2 = z_mid + delta2*(z_max-z_mid);
}
else
 if(y<y_mid) {  // scanline intersect line y_min -> y_mid
  delta2 = (y-y_min)/(y_mid-y_min);
  x_sl_2 = x_min + delta2*(x_mid-x_min);
  z_sl_2 = z_min + delta2*(z_mid-z_min);
 }
else {  // y==y_mid
   delta2 = -1;
   x_sl_2 = x_mid;
   z_sl_2 = z_mid;
}

double delta3 = (x-x_sl_1)/(x_sl_2-x_sl_1);
double result = z_sl_1 + delta3*(z_sl_2-z_sl_1);

return result;

}


/**
  * check whether this is a valid fuzzy Triangle
  * @return true if the underlying BasicTriangles is valid,
  *          all degrees of fuzziness are in [0,1) and
  *          1 degree is greater then 0
  */
public boolean isValid(){
  boolean BasicIsValid = (new BasicTriangle(CP_1,CP_2,CP_3)).isValid();
  return BasicIsValid &&
         0<=Z1 && Z1<=1 && 0<=Z2 && Z2 <=1 && 0<=Z3 && Z3<= 1 &&
         (Z1+Z2+Z3)>0;
}


/**
 * computes the volume &quot;under&quot; this Triangle
 */
public double volume(){

  double Zmin = Math.min(Z1,Math.min(Z2,Z3));
  double Zmax = Math.max(Z1,Math.max(Z2,Z3));
  double Zmid = Z1+Z2+Z3 - Zmin - Zmax;
  double a = fuzzyobjects.Params.a;
  double b = fuzzyobjects.Params.b;
  return  (a*b/4)*Zmin + (( (Zmid-Zmin)+(Zmax-Zmin)/2)*a*b)/6;

}

/**
 * computes the area of this fTriangles viewed as 3D-Triangle
 */
public double area3D(){
   // Bronstein

   double x1 = CP_1.getX();
   double x2 = CP_2.getX();
   double x3 = CP_3.getX();
   double y1 = CP_1.getY();
   double y2 = CP_2.getY();
   double y3 = CP_3.getY();
   double a2 = (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
   double b2 = (x3-x2)*(x3-x2) + (y3-y2)*(y3-y2);
   double c2 = (x1-x3)*(x1-x3) + (y1-y3)*(y1-y3);
   return Math.sqrt((2*a2*b2 + 2*a2*c2 + 2*b2*c2 -a2*a2 -b2*b2 -c2*c2)/2);
}

/** returns the greatest degree of fuzziness in this Triangle */
public double getMaxZ(){
 return Math.max(Z1,Math.max(Z2,Z3));
}


/** returns the smallest degree of fuzziness in the Triangle */
public double getMinZ(){
  return Math.min(Z1,Math.min(Z2,Z3));
}

public int getMinX(){
  return Math.min(CP_1.getX(),Math.min(CP_2.getX(),CP_3.getX()));
}

public int getMinY(){
  return Math.min(CP_1.getY(),Math.min(CP_2.getY(),CP_3.getY()));
}
public int getMaxX(){
  return Math.max(CP_1.getX(),Math.max(CP_2.getX(),CP_3.getX()));
}
public int getMaxY(){
  return Math.max(CP_1.getY(),Math.max(CP_2.getY(),CP_3.getY()));
}

/** returns the ListExpr for this fTriangle */
public ListExpr toListExpr(){
  fEPoint P1 = new fEPoint(CP_1,Z1);
  fEPoint P2 = new fEPoint(CP_2,Z2);
  fEPoint P3 = new fEPoint(CP_3,Z3);
  return ListExpr.threeElemList(P1.toListExpr(),P2.toListExpr(),
                                P3.toListExpr());
}


/** set the values of this fTriangle to LE, if LE represent a valid FTriangle
  * @return true if LE is a valid fTriangle
  */
public boolean readFromListExpr(ListExpr LE){
  if(LE.listLength()!=3)
     return false;
  fEPoint P1 = new fEPoint(0,0,0);
  fEPoint P2 = new fEPoint(0,0,0);
  fEPoint P3 = new fEPoint(0,0,0);
  if(!( P1.readFromListExpr(LE.first()) && P2.readFromListExpr(LE.second())
        && P3.readFromListExpr(LE.third()) ))
     return false;
  BasicPoint BP1 = (BasicPoint) P1.basic();
  BasicPoint BP2 = (BasicPoint) P2.basic();
  BasicPoint BP3 = (BasicPoint) P3.basic();
  if (! (BP1.neightbooring(BP2) && BP1.neightbooring(BP3) &&
         BP2.neightbooring(BP3)))
     return false;

  this.CP_1 = BP1;
  this.CP_2 = BP2;
  this.CP_3 = BP3;
  this.Z1 = P1.getZ();
  this.Z2 = P2.getZ();
  this.Z3 = P3.getZ();
  return true;
}



/** location of a cornerpoint */
private BasicPoint CP_1,CP_2,CP_3;
/** degree of fuzziness of a cornerpoint */
private double Z1,Z2,Z3;

} // class
