
package viewer.hoese.algebras;
import java.awt.geom.*;
import java.awt.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.*;
import viewer.hoese.*;
import tools.Reporter;
import javax.swing.JPanel;
import java.text.DecimalFormat;

/**
 * The displayclass of the cpoint  of TemporalAlgebra.
 */

public class Dsplcpoint extends DisplayGraph {
    /** The internal datatype representation */
    Point2D.Double point;
    double radius= 0.0;
    private double x1,y1;
    private RectangularShape circle, shp;
    DecimalFormat format = new DecimalFormat();
    String label = null;
    private static JPanel EmptyPanel = new JPanel();
    /** Standard Constructor **/
    public Dsplcpoint () { super();}

    /** Return  a short text **/
    public String getLabel(double time) {return label;}

    /**
     * Constructor used by points datatype
     * @param Point2D.Double p The position of the new Dsplcpoint
     * @param DisplayGraph dg The object to which this new Dsplcpint belongs
     *
     * **/
    public Dsplcpoint (Point2D.Double p, DisplayGraph dg) {
        super();
        point = p;
        RefLayer = dg.RefLayer;
        selected = dg.getSelected();
        Cat = dg.getCategory();
    }

    protected void ScanValue (ListExpr v) {
        if (isUndefined(v)) {
            err = false;
            point = null;
            return;
        }
        if (v.listLength() != 2) // list must have the form ((x y) r)
            return;
        radius = LEUtils.readNumeric(v.second()).doubleValue();
        ListExpr coord = v.first();
        if (coord.listLength() != 2) // error in reading start and end point (Coordinates)
            return;
        Double X1 = LEUtils.readNumeric(coord.first());
        Double Y1 = LEUtils.readNumeric(coord.second());
        if (X1 == null || Y1 == null) // error in reading x,y values
            return;
        x1 = X1.doubleValue();
        y1 = Y1.doubleValue();
        err = false;
        Point2D.Double p1 = new Point2D.Double();
        if (ProjectionManager.project(x1, y1, p1)) {
            x1 = p1.getX();
            y1 = p1.getY();
            double diameter = radius * 2;
            if (Cat.getPointasRect())
                shp = new Rectangle2D.Double(x1 - radius, y1 - radius, diameter, diameter);
            else {
                shp = new Ellipse2D.Double(x1 - radius, y1 - radius, diameter, diameter);
            }
        }
    }
    public int numberOfShapes(){
        return 1;
    }
    /** Show the point **/
    public Shape getRenderObject (int num,AffineTransform at) {
        if (num<1){
            return shp;
        }
            return null;
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
            qr.addEntry(new String("(" + AttrName + ": (cpoint))"));
            return;
        }
        else
            qr.addEntry(this);
    }
    public JPanel getTimeRenderer(double d){
        return EmptyPanel;
    }
}