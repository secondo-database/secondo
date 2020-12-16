package  viewer.hoese.algebras;
import java.awt.geom.*;
import java.awt.*;
import viewer.*;
import viewer.hoese.*;
import sj.lang.ListExpr;
import gui.Environment;
import javax.swing.JPanel;
import tools.Reporter;


/**
 * A displayclass for a cupoint
 */
public class Dsplcupointregion extends DisplayTimeGraph implements LabelAttribute{
  private double x1,x2,y1,y2;
  private double radius = -1, dir =0.0, PI = 3.1415926535;
  private Point2D.Double point , p0, p1, p0shifted, p1shifted ;
  private Rectangle2D.Double bounds=null;
  private Interval theInterval;
  private Path2D.Double cylinder;
  private static JPanel EmptyPanel = new JPanel();
  private RectangularShape shp;

  static java.text.DecimalFormat format = new java.text.DecimalFormat("#.#####");
  public boolean isPointType(int num){
    return true;
  }


  public int numberOfShapes(){
    return 1;
  }

  public String getLabel(double time){
    if(Intervals==null){
      return null;
    }
    double t = RefLayer.getActualTime(); // get the current time
    if(!theInterval.isDefinedAt(t)){ // t is outside from the deftime
      point = null;
      return null;
    }

    double t1 = theInterval.getStart();
    double t2 = theInterval.getEnd();
    double Delta = (time-t1)/(t2-t1);
    double e = radius;
    double x = x1+Delta*(x2-x1);
    double y = y1+Delta*(y2-y1);
    return "(("+format.format(x)+", "+ format.format(y)+")"+format.format(e)+")";

  }

  /** Computes the Renderobject at the given point in time */
  public Shape getRenderObject (int num,AffineTransform at) {
    if(num!=0) {
      return null;
    }
    p0 = new Point2D.Double(x1, y1);
    p1 = new Point2D.Double(x2,y2);

    dir = Math.atan2(y2 - y1, x2 - x1) *   Math.PI / 180 ;// /180 ;///PI ;/// 180;
    //p0.Direction(p1, false, geoid) * PI / 180;
    int hSign = p0.getX() < p1.getX() ? 1 :-1;
    int vSign = p0.getY() < p1.getY() ? 1 :-1;
    p0shifted = new Point2D.Double(p0.getX() - hSign * Math.abs(Math.sin(dir)) * radius,p0.getY() - vSign * Math.abs(Math.cos(dir)) * radius);
    p1shifted = new Point2D.Double(p1.getX() + hSign * Math.abs(Math.sin(dir)) * radius,p1.getY() + vSign * Math.abs(Math.cos(dir)) * radius);
    cylinder = new Path2D.Double();
    // cylinder = new Path2D.Double(p0.getX() + hSign * Math.abs(Math.sin(dir)) * radius,p0.getY() + vSign * Math.abs(Math.cos(dir)) * radius);
    //cylinder.moveTo(p0.getX(), p0.getY());
    cylinder.moveTo(p0shifted.getX(), p0shifted.getY());
    cylinder.lineTo(p0.getX() + hSign * Math.abs(Math.sin(dir)) * radius,p0.getY() + vSign * Math.abs(Math.cos(dir)) * radius);
    cylinder.lineTo(p1shifted.getX(), p1shifted.getY());
    cylinder.lineTo(p1.getX() - hSign * Math.abs(Math.sin(dir)) * radius,p1.getY() - vSign * Math.abs(Math.cos(dir)) * radius);

    //cylinder.lineTo(p1.getX(), p1.getY());
    cylinder.closePath();
    //System.out.print("this is t "+t+ " this is t1 "+ t1 + " this is t2 "+ t2 );
    //Reporter.writeError("this is t "+t+ " this is t1 "+ t1 + " this is t2 "+ t2 );
   // point = new Point2D.Double(x, y);

    //double ps = Cat.getPointSize(renderAttribute,CurrentState.ActualTime);
   /* double minMax[] = {
            Math.min(p0shifted.getX(), p1shifted.getX()),
            Math.max(p0shifted.getX(), p1shifted.getX()),
            Math.min(p0shifted.getY(), p1shifted.getY()),
            Math.max(p0shifted.getY(), p1shifted.getY()),
            t1,
            t2}; */


    /*
    cylinder.moveTo(x1,y1);
    cylinder.lineTo(x1 + radius *  Math.cos(45) , y1 + radius  *  Math.sin(90));
    cylinder.lineTo(x2 - radius * Math.cos(45) , y2 + radius *  Math.sin(90));
    cylinder.lineTo(x2 + radius * Math.cos(45) , y2 - radius *  Math.sin(90));
    cylinder.lineTo(x1 + radius *  Math.cos(45) , y1 - radius  *  Math.sin(90));
   */
    return  cylinder;
  }


  /**
   * Scans the representation of a cupoint datatype
   * @param v interval with x1,x2,y1,y2 values
   * @see sj.lang.ListExpr
   */
  private void ScanValue (ListExpr v) {
    err = true;
    if (v.isEmpty())
      return;
    if(v.listLength()!=3) // list must have the form
      //(((interval)(x1 y1 x2 y2))radius)
      return;

    radius = LEUtils.readNumeric(v.third()).doubleValue();
    theInterval = LEUtils.readInterval(v.first());
    if(theInterval==null) // error in reading interval
      return;

    ListExpr StartEnd = v.second(); //cupoint.second();
    if(StartEnd.listLength()!=4) // error in reading start and end point (coordinates)
      return;
    Double X1 = LEUtils.readNumeric(StartEnd.first());
    Double Y1 = LEUtils.readNumeric(StartEnd.second());
    Double X2 = LEUtils.readNumeric(StartEnd.third());
    Double Y2 = LEUtils.readNumeric(StartEnd.fourth());
    if(X1==null || X2==null | Y1==null || Y2==null) // error in reading x,y values
      return;
    x1 = X1.doubleValue();
    x2 = X2.doubleValue();
    y1 = Y1.doubleValue();
    y2 = Y2.doubleValue();
    err = false;
    if(bounds==null)
      bounds = new Rectangle2D.Double(0,0,0,0);
    bounds.setRect(Math.min(x1,x2),Math.min(y1,y2),Math.abs(x1-x2),Math.abs(y1-y2));
    if(!theInterval.isInfinite()){
      TimeBounds = theInterval;
    } else{
      TimeBounds = null;
    }
  }


  public void init (String name, int nameWidth, int indent,
                    ListExpr type,  ListExpr value, QueryResult qr) {
    AttrName = extendString(name, nameWidth, indent);
    ScanValue(value);
    if (err) {
      Reporter.writeError("Dsplcupoint Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": (cupoint))"));
      return;
    }
    else
      qr.addEntry(this);
  }

  /* Returns an empty JPanel at this time */
  public JPanel getTimeRenderer(double d){
    return EmptyPanel;
  }


  /** returns true if the last computet position is in the near of xpos,ypos */
  public boolean contains (double xpos, double ypos, double scalex, double scaley) {
    if(point==null) return false; // no last position
    double x = point.getX();
    double y = point.getY();
    return (Math.abs(x-xpos)<scalex*10) && (Math.abs(y-ypos)<scaley*10);
  }




  /**
   * @return The overall boundingbox of the upoint
   */
  public Rectangle2D.Double getBounds () {
    return  bounds;
  }
}



