
package  viewer.hoese.algebras;

import  java.awt.geom.*;
import  java.awt.*;
import  sj.lang.ListExpr;
import  java.util.*;
import  viewer.*;
import viewer.hoese.*;


/**
 * The displayclass of the line datatype (Rose algebra).
 */
public class Dsplline extends DisplayGraph {
/**
 * The segments of the line datatype are stored in this vector.
 * A single segment ist represented by a Line2D.Double object.
 * @see java.awt.geom.Line2D.Double
 */
  Vector lines;
/** The bounding-box rectangle */
  Rectangle2D.Double bounds;

  /**
   * Scans the representation of the line datatype and constucts the lines Vector.
   * @param v A list of segments
   * @see sj.lang.ListExpr
   * @see <a href="Dspllinesrc.html#ScanValue">Source</a>
  */
  public void ScanValue (ListExpr value) {
    double koord[] = new double[4];
    lines = new Vector(20, 10);
    double x1,y1,x2,y2;
    while (!value.isEmpty()) {
      ListExpr v = value.first();
      //System.out.println(v.writeListExprToString());
      if (v.listLength() != 4) {
        System.out.println("Error: No correct line expression: 4 elements needed");
        value = value.rest();
        err = true;
        return;
      }
      for (int koordindex = 0; koordindex < 4; koordindex++) {
        Double d = LEUtils.readNumeric(v.first());
        if (d == null) {
          err = true;
          return;
        }
        koord[koordindex] = d.doubleValue();
        v = v.rest();
      }
      if (!err) {
        try{
	   x1 = ProjectionManager.getPrjX(koord[0],koord[1]);
	   y1 = ProjectionManager.getPrjY(koord[0],koord[1]);
	   x2 = ProjectionManager.getPrjX(koord[2],koord[3]);
           y2 = ProjectionManager.getPrjY(koord[2],koord[3]);
           Line2D.Double line = new Line2D.Double(x1,y1,x2,y2);
           lines.add(line);
	} catch(Exception e){
	   System.out.println("error in project segment ("+koord[0]+","+koord[1]+")->"+koord[2]+","+koord[3]+")");
	}
      }
      value = value.rest();
    }
  }

  /**
   * Init. the Dsplline instance.
   * @param type The symbol line
   * @param value A list of segments.
   * @param qr queryresult to display output.
   * @see QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dspllinesrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      System.out.println("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": GA(line))"));
      return;
    }
    else
      qr.addEntry(this);
    ListIterator li = lines.listIterator();
    bounds = null;
    while (li.hasNext())
      if (bounds == null)
        bounds = (Rectangle2D.Double)((Line2D.Double)li.next()).getBounds2D();
      else
        bounds = (Rectangle2D.Double)bounds.createUnion(((Line2D.Double)li.next()).getBounds2D());
    RenderObject = bounds;
    // System.out.println(value.writeListExprToString());
  }


  /**
   * @return The boundingbox of this line-object
   * @see <a href="Dspllinesrc.html#getBounds">Source</a>
   */
  public Rectangle2D.Double getBounds () {
    return  bounds;
  }

  /**
   * Tests if a given position is near (10pxs) of this line, by iterating over all segments.
   * @param xpos The x-Position to test.
   * @param ypos The y-Position to test.
   * @param scalex The actual x-zoomfactor
   * @param scaley The actual y-zoomfactor
   * @return true if x-, ypos is contained in this points type
   * @see <a href="Dspllinesrc.html#contains">Source</a>
   */
  public boolean contains (double xpos, double ypos, double scalex, double scaley) {
    if ((bounds.getWidth()*bounds.getHeight()!=0) && (!bounds.intersects(xpos - 5.0*scalex, ypos - 5.0*scaley, 10.0*scalex,
        10.0*scaley)))
      return  false;
    Rectangle2D.Double r = new Rectangle2D.Double(xpos - 5.0*scalex, ypos -
        5.0*scaley, 10.0*scalex, 10.0*scaley);
    //System.out.println(scalex +" " + scaley);
    boolean hit = false;
    ListIterator li = lines.listIterator();
    while (li.hasNext())
      hit |= ((Line2D.Double)li.next()).intersects(r);
    return  hit;
  }

  /**
   * Draws the included segments by creating a Singleline .
   * @param g The graphics context
   * @see <a href="Dspllinesrc.html#draw">Source</a>
   */
  public void draw (Graphics g) {
    ListIterator li = lines.listIterator();
    while (li.hasNext()) {
      Line2D.Double l = (Line2D.Double)li.next();
      new SingleLine(l, this).draw(g);
    }
    drawLabel(g, bounds);
  }

/** Helperclass to construct a single segment */ 
  private class SingleLine extends DisplayGraph {

    /**
     * The constructor of a single segment, that can easily be drawn by the superclass
     * @param     Line2D.Double l A segment.
     * @param     DisplayGraph dg The object to which this new Dsplpoint belongs.
     * @see <a href="Dspllinesrc.html#SingleLine">Source</a>
     */
    public SingleLine (Line2D.Double l, DisplayGraph dg) {
      super();
      RenderObject = l;
      RefLayer = dg.RefLayer;
      selected = dg.getSelected();
      Cat = dg.getCategory();
    }
  }
}



