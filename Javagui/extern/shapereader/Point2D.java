package extern.shapereader;

import sj.lang.ListExpr;

public class Point2D{


/** creates a new point at given position */
public Point2D(double x, double y){
   this.x = x;
   this.y = y;
}

/** creates a new point at (0,0) */
public Point2D(){
  x = 0.0;
  y = 0.0;
}

/** returns true if this equals to P */
public boolean equals(Object P){
  if(!(P instanceof Point2D))
     return false;
  Point2D  PP = (Point2D)P;
  return (float)x==(float)PP.x && (float)y==(float)PP.y;
}

/** move this Point to the given position */
public void moveTo(double x, double y){
  this.x = x;
  this.y = y;
}

/** returns the x position of this point */
public double getX(){
   return x;
}


/** returns the y position of this point */
public double getY(){
   return y;
}

public ListExpr getList(){
  return ListExpr.twoElemList(ListExpr.realAtom((float)x),
                              ListExpr.realAtom((float)y));
}


private double x;
private double y;

}
