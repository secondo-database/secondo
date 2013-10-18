
package viewer.hoese.algebras;


import viewer.hoese.*;
import sj.lang.ListExpr;
import java.awt.geom.Point2D;




public class Dsplpoint2 extends Dsplpoint{


  protected void ScanValue(ListExpr v){
     if(v.listLength()!=3){
        super.ScanValue(v);
        return;
     }
    double koord[] = new double[2];
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
    // now, read the fractional part of the point
    v = v.first();

    if(v.listLength()!=2){
       err=true;
       label=null;
       return;
    }
    ListExpr fracX = v.first();
    ListExpr fracY = v.second();
    Double fx = LEUtils.readFrac(fracX);
    Double fy = LEUtils.readFrac(fracY);
    if(fx==null || fy==null){
       err=true;
       label=null;
       return;
    }
    koord[0] += fx;
    koord[1] += fy;
    if(ProjectionManager.project(koord[0],koord[1],aPoint)){
          err = false;
          point = new Point2D.Double(aPoint.x,aPoint.y);
          label = "("+format.format(point.getX())+", "+format.format(point.getY())+")";
    }
    else{
       label = null;
       err = true;
    }

  } 


}
