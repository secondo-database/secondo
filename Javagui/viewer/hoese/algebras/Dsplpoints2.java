
package viewer.hoese.algebras;

import sj.lang.ListExpr;
import java.awt.geom.Point2D; 
import java.util.Vector;
import java.awt.geom.Rectangle2D;

public class Dsplpoints2 extends Dsplpoints{


  public void ScanValue (ListExpr value) {

    if(isUndefined(value)){
       err=false;
       defined = false;
       return;
    }
    defined = true;
    double coords[] = new double[2];
    Vector<Point2D.Double>  pointsV = new Vector<Point2D.Double>(20, 20);

    while (!value.isEmpty()) {
      ListExpr v = value.first();
      if(!Dsplpoint2.fillCoordsS(v,coords,true)){
         err = true;
         points = null;
         bounds = null;
         return;
      }
      pointsV.add(new Point2D.Double(coords[0],coords[1]));
      value = value.rest();
    }
    computeBounds(pointsV);
  }


}

