
package  viewer.hoese.algebras;

import  java.awt.geom.*;
import  java.awt.*;
import  sj.lang.ListExpr;
import  java.util.*;
import  viewer.*;
import viewer.hoese.*;


/**
 * The displayclass of the PointRectangle algebras rectangle datatype.
 */
public class Dsplrectangle extends DisplayGraph {
/** The internal datatype representation */
  Rectangle2D.Double rect;
  /**
   * Scans the numeric representation of a point datatype 
   * @param v the numeric value of the x- and y-coordinate
   * @see sj.lang.ListExpr
   * @see <a href="Dsplpointsrc.html#ScanValue">Source</a>
   */
  private void ScanValue (ListExpr v) {
    double koord[] = new double[2];
    //System.out.println(v.writeListExprToString());
    if (v.listLength() != 4) {
      System.out.println("Error: No correct rectangle expression: 4 elements needed");
      err = true;
      return;
    }
    if ((v.first().atomType() != ListExpr.INT_ATOM) || (v.second().atomType()
          != ListExpr.INT_ATOM) || (v.third().atomType() != ListExpr.INT_ATOM)
          || (v.fourth().atomType() != ListExpr.INT_ATOM)) {
      System.out.println("Error: No correct rectangle : 4 INTs needed");
      err = true;
      return;
    }
    if (!err) {
      rect = new Rectangle2D.Double(v.first().intValue(),v.third().intValue(),v.second().intValue()-v.first().intValue(),
        v.fourth().intValue()-v.third().intValue());
    }
  }

  /**
   * Init. the Dsplrectangle instance.
   * @param type The symbol rectangle
   * @param value The 4 INTs  of a rectangle left,right,top,bottom.
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplrectanglesrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      System.out.println("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": GA(rectangle))"));
      return;
    } 
    else 
      qr.addEntry(this);
      RenderObject=rect;
  }

}



