
package  viewer.hoese.algebras;

import  java.awt.geom.*;
import  java.awt.*;
import  sj.lang.ListExpr;
import  java.util.*;
import  viewer.*;
import viewer.hoese.*;


/**
 * The displayclass for rectangles
 */
public class Dsplrect extends DisplayGraph {
/** The internal datatype representation */
  Rectangle2D.Double rect;
  /**
   * Scans the numeric representation of a rectangle
   */
  private void ScanValue (ListExpr v) {
    double koord[] = new double[2];
    //System.out.println(v.writeListExprToString());
    if (v.listLength() != 4) {
      System.err.println("Error: No correct rectangle expression: 4 elements needed");
      err = true;
      return;
    }
    Double X1 = LEUtils.readNumeric(v.first());
    Double X2 = LEUtils.readNumeric(v.second());
    Double Y1 = LEUtils.readNumeric(v.third());
    Double Y2 = LEUtils.readNumeric(v.fourth());
    if(X1==null || X2==null || Y1==null | Y2==null){
       System.err.println("Error: no correct rectangle expression (not a numeric)");
       err =true;
       return;
    }
    double x1 = X1.doubleValue();
    double x2 = X2.doubleValue();
    double y1 = Y1.doubleValue();
    double y2 = Y2.doubleValue();
    double x = Math.min(x1,x2);
    double w = Math.abs(x2-x1);
    double y = Math.min(y1,y2);
    double h = Math.abs(y2-y1);
    rect = new Rectangle2D.Double(x,y,w,h);
  }

  /**
   * Init. the Dsplrect instance.
   * @param type The symbol rect
   * @param value The 4 Numeric  of a rectangle x1 x2 y1 y2
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
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



