package de.fernunihagen.dna.hoese.algebras;

import javamini.awt.Shape;
import javamini.awt.geom.AffineTransform;
import javamini.awt.geom.Ellipse2D;
import javamini.awt.geom.Point2D;
import javamini.awt.geom.Rectangle2D;
import javamini.awt.geom.RectangularShape;
import java.text.DecimalFormat;

import de.fernunihagen.dna.hoese.CurrentState;
import de.fernunihagen.dna.hoese.LEUtils;
import de.fernunihagen.dna.hoese.ProjectionManager;
import de.fernunihagen.dna.hoese.QueryResult;

import javamini.awt.Point;

import sj.lang.ListExpr;
import tools.Reporter;



/**
 * The displayclass of the Rose algebras point datatype.
 */
public class Dsplpoint extends DisplayGraph implements LabelAttribute {
  /** The internal datatype representation */
  Point.Double point;
  DecimalFormat format = new DecimalFormat("#.#####");
  String label = null;
  private RectangularShape shp;

  /**
   * standard constructor.
   * @see <a href="Dsplpointsrc.html#Dsplpoint1">Source</a>
   */
  public Dsplpoint () {
    super();
  }

  /** Returns a short text **/
  public String getLabel(double time){
     return label;
  }


  /**
   * Constructor used by the points datatype
   * @param   Point2D.Double p The position of the new Dsplpoint
   * @param   DisplayGraph dg The object to which this new Dsplpoint belongs.
   * @see <a href="Dsplpointsrc.html#Dsplpoint2">Source</a>
   */
  public Dsplpoint (Point.Double p, DisplayGraph dg) {
    super();
    point = p;
    RefLayer = dg.RefLayer;
    selected = dg.getSelected();
    Cat = dg.getCategory();
  }

  public boolean isPointType(int num){
    return true;
  }

  public int numberOfShapes(){
     return 1;
  }


  /**
   * Creates the internal Object used to draw this point
   * @param at The actual Transformation, used to calculate the correct size.
   * @return Rectangle or Circle Shape
   * @see <a href="Dsplpointsrc.html#getRenderObject">Source</a>
   */
  public Shape getRenderObject (int num, AffineTransform at) {
    if(num!=0){
       return null;
    }
    if(point==null){
        return null;
    }
    Rectangle2D.Double r = getBounds();
    double ps = Cat.getPointSize(renderAttribute,CurrentState.ActualTime);
    double pixy = Math.abs(ps/at.getScaleY());
    double pix = Math.abs(ps/at.getScaleX());
    if (Cat.getPointasRect())
      shp = new Rectangle2D.Double(r.getX()- pix/2, r.getY() - pixy/2, pix, pixy);
    else {
      shp = new Ellipse2D.Double(r.getX()- pix/2, r.getY() - pixy/2, pix, pixy);
    }
    return  shp;
  }

  /**
   * Scans the numeric representation of a point datatype
   * @param v the numeric value of the x- and y-coordinate
   * @see sj.lang.ListExpr
   * @see <a href="Dsplpointsrc.html#ScanValue">Source</a>
   */
  protected void ScanValue (ListExpr v) {
    if(isUndefined(v)){
        err=false;
        point=null;
        label = "-";
        return;
    }
    double koord[] = new double[2];
    if (v.listLength() != 2) {
      Reporter.writeError("Error: No correct point expression: 2 elements needed");
      err = true;
      label =null;
      return;
    }
    for (int koordindex = 0; koordindex < 2; koordindex++) {
      Double d = LEUtils.readNumeric(v.first());
      if (d == null) {
        err = true;
        label = null;
        return;
      }
      koord[koordindex] = d.doubleValue();
      v = v.rest();
    }

    if(ProjectionManager.project(koord[0],koord[1],aPoint)){
          point = new Point2D.Double(aPoint.x,aPoint.y);
          label = "("+format.format(point.getX())+", "+format.format(point.getY())+")";
    }
    else{
       label = null;
       err = true;
    }
  }


  public void init (String name, int nameWidth, int indent,
                    ListExpr type, 
                    ListExpr value,
                    QueryResult qr) {
    AttrName = extendString(name,nameWidth, indent);
    if(isUndefined(value)){
       qr.addEntry(new String("" + AttrName + ": undefined"));
       return;
    }
    ScanValue(value);
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": GA(point))"));
      return;
    }
    else
      qr.addEntry(this);
  }
  /**
   * @return A rectangle with height=0 and width=0
   * @see <a href="Dsplpointsrc.html#getBounds">Source</a>
   */
  public Rectangle2D.Double getBounds () {
    return  new Rectangle2D.Double(point.getX(), point.getY(), 0, 0);
  }

  public Point2D.Double getPoint(){
     return point;
  } 

}