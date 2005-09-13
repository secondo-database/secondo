import sj.lang.ListExpr;

/**
  * This class provides a simple implementation of a point in  the
  * Euclidean Plane.
**/
public class Point{

/**
  * creates a point at (0,0)
**/
public Point(){
    this(0,0);
}


/**
  * creates a point at the given position
 **/
public Point(double x, double y){
   this.x=x;
   this.y=y;
}

/**
  * moves this Point instance to the given position
 **/
public void moveTo(double x, double y){
   this.x=x;
   this.y=y;
}

/**
  * returns the x-coordinate of this Point
  **/
public double getX(){return x;}

/**
  * returns the y-coordinate of this point
  **/
public double getY(){return y;}


/**
  *  returns the nested list representing this point as String
  **/
public String getListString(){
   return "("+ x+" "+ y+")";
}

/**
  *  returns a String representation of this Point
  **/
public String toString(){
   return "("+ x+","+ y+")";
}

/**
  * reads the value of this intance from LE.
  * if LE is not a valid representation of a point
  * the value of this remains unschnaged an false is returned
 **/
public boolean readFrom(ListExpr LE){
   if(LE.listLength()!=2)
      return false;
   Double X = viewer.hoese.LEUtils.readNumeric(LE.first());
   Double Y = viewer.hoese.LEUtils.readNumeric(LE.second());
   if(X==null || Y==null)
      return false;
   x = X.doubleValue();
   y = Y.doubleValue();
   return true;
}
 // the coordinates of this Point
private double x,y;

}
