package viewer.viewer3d.graphic2d;

/****************************
*
*  Author   : Thomas Behr
*  Version  : 1.1
*  Date     : 16.5.2000
*
******************************/

import java.util.Vector;

/**
  * provides a vector to store points
  */
public class Point2DVector {

/** the intern store */
private Vector V;

/** creates a new Point2DVector */
public Point2DVector() { V = new Vector(); }

/** returns a copy from this */
public Point2DVector Duplicate() {
  Point2DVector Copy = new Point2DVector();
  Copy.V = (Vector) V.clone();
  return Copy;
}

/** equalize this to Source */
public void Equalize(Point2DVector Source) { V = (Vector) Source.V.clone(); }

/** returns the number of containing points */
public int getSize() { return V.size(); }

/** add a point to this vector */
public void append(Point2D Pt) { V.add(Pt); }

/** get the point on position i */
public Point2D getPoint2DAt(int i) throws IndexOutOfBoundsException {
   return ((Point2D) V.get(i));
}

/** insert a point on position i */
public void insertAt(int i,Point2D Pt) {
  V.add(i,Pt);
}

/** overwrite the point on position i with Pt */
public void setPoint2DAt(int i, Point2D Pt) {
  V.set(i,Pt);
}

/** deletes all containing points */
public void empty() { V = new Vector(); }

/** check for emptyness */
public boolean isEmpty() { return V.size()==0; }

/** check for equality */
public boolean equals(Point2DVector PK) { return PK.V.equals(V); }


}
