package movingregion;

import java.io.*;
/** 
  * This class is meant to store lines and the angle between them. The
  * class itself stores a single point and an angle. The point is the
  * starting point of the line, and the angle is the angle between the two
  * lines that meet at that point. To actually store lines, the
  * <code>LineWA</code> objects must be stored in a list. In such a list,
  * the element after a given element stores the coordinates of the end point
  * of the line.
  *
  * @author Erlend Tøssebro
  */
public class LineWA implements Comparable ,Serializable{
  public int x;          // The x-coordinate of the point
  public int y;          // The y-coordinate of the point
  public double angle;   // The angle between the last and next lines.
  static final long serialVersionUID = 7965461305176575190L;

  /**
    * Constructor without parameters. This constructor initializes all
    * variables to 0.
    */
  public LineWA() {
    x = 0;
    y = 0;
    angle = -1.0;
  }

  /**
    * This constructor initializes the angle to 0.
    *
    * @param xp The x-coordinate of the point to be stored in this object.
    * @param yp The y-coordinate of the point to be stored in this object.
    */
  public LineWA(int xp, int yp) {
    x = xp;
    y = yp;
    angle = -1.0;
  }

  /**
    * This constructor initializes all variables to given values.
    *
    * @param xp The x-coordinate of the point to be stored in this object.
    * @param yp The y-coordinate of the point to be stored in this object.
    * @param a The angle between the last and next lines.
    */
  public LineWA(int xp, int yp, double a) {
    x = xp;
    y = yp;
    angle = a;
  }

  /**
    * This constructor initializes this object to be an exact copy of another
    * <code>LineWA</code> object.
    *
    * @param original The <code>LineWA</code> object to be copied.
    */
  public LineWA(LineWA original) {
    x = original.x;
    y = original.y;
    angle = original.angle;
  }

  /**
    * Compares two LineWA objects. This uses angles for comparison, NOT
    * position. Therefore, the ordering of LineWA is not consistent
    * with equals. This function is an implementation of the
    * <code>Comparable</code> interface.
    *
    * @param o The object that this object should be compated to.
    *
    * @return 1 if the angle of <code>o</code> is less than the angle of 
    *         this object,
    *         0 if they are equal, and -1 if the angle of <code>o</code>
    *         is greater than the angle of this object.
    */
  public int compareTo(Object o) {
    LineWA obj = (LineWA)o;
    if (obj.angle < angle) return(1);
    if (obj.angle == angle) return(0);
    return(-1);
  }

  public int hashCode()
  {
      return((this.x * 4213 + this.y * 133326 + (int)(Math.floor(this.angle * 132451))) % 13542347);
  }
  
  /** 
    * Tests equality of two <code>LineWA</code> objects. These objects are 
    * equal if they have the same position.
    *
    * @param obj The object to be compated to this object.
    *
    * @return TRUE if <code>obj</code> is a <code>LineWA</code> with the same
    *         position as this one and FALSE otherwise.
    */
  public boolean equals(Object obj) {
    LineWA line;
    line = (LineWA)obj;
    if ((line.x == x) && (line.y == y)) {
      return(true);
    } else return(false);
  }
  public String toString()
  {
      return("Line in ("+x+";"+y+") with "+angle+"°");
      //return(+x+";"+y);
  }
}
