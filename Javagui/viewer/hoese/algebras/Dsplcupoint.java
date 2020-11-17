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
public class Dsplcupoint extends DisplayTimeGraph {
  private double x1,x2,y1,y2;
  private double radius = -1;
  private Point2D.Double point;
  private Rectangle2D.Double bounds=null;
  private Interval theInterval;
  private static JPanel EmptyPanel = new JPanel();
  private RectangularShape shp;
  static java.text.DecimalFormat format = new java.text.DecimalFormat("#.#####");
  public boolean isPointType(int num){
     return true;
  }


  public int numberOfShapes(){
      return 1;
  }
/*
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
    double e = epsilon;
    double x = x1+Delta*(x2-x1);
    double y = y1+Delta*(y2-y1);
    return "("+format.format(e)+"("+format.format(x)+", "+ format.format(y)+"))";

  }
*/

  /** Computes the Renderobject at the given point in time */
  public Shape getRenderObject (int num,AffineTransform at) {
    if(num!=0) {
       return null;
    }
    double t = RefLayer.getActualTime(); // get the current time
    if(!theInterval.isDefinedAt(t)){ // t is outside from the deftime
       point = null;
       return null;
    }
    double t1 = theInterval.getStart();
    double t2 = theInterval.getEnd();
    double Delta = (t-t1)/(t2-t1);
    double x = x1+Delta*(x2-x1);
    double y = y1+Delta*(y2-y1);

    point = new Point2D.Double(x, y);
    double ps = Cat.getPointSize(renderAttribute,CurrentState.ActualTime);
    // The higth and width of the circle is related to the radius-value.
    // but pix / pixy denote the complete width / higth
    // of the point-object, they are set to 2*radius.
    double pixy = Math.abs(2* radius);
    double pix = Math.abs(2* radius);
    if (Cat.getPointasRect())
      shp = new Rectangle2D.Double(point.getX()- pix/2, point.getY() - pixy/2, pix, pixy);
    else {
      shp = new Ellipse2D.Double(point.getX()- pix/2, point.getY() - pixy/2, pix, pixy);
    }
    return  shp;
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



