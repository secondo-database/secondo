
package viewer.hoese.algebras;


import viewer.hoese.*;
import sj.lang.ListExpr;
import java.awt.geom.Point2D;




public class Dsplpoint2 extends Dsplpoint{

  public boolean fillCoords(ListExpr v, double[] coord, boolean useProjection){
     return  Dsplpoint2.fillCoordsS(v,coord,useProjection);
  }

  public static boolean fillCoordsS(ListExpr v, double[] coord, boolean useProjection){
     if(v.listLength()!=3){
       return Dsplpoint.fillCoordsS(v,coord, useProjection);
     }
    for (int koordindex = 0; koordindex < 2; koordindex++) {
      Double d = LEUtils.readNumeric(v.first());
      if (d == null) {
        return false;
      }
      coord[koordindex] = d.doubleValue();
      v = v.rest();
    }
    // now, read the fractional part of the point
    v = v.first();
    if(v.listLength()!=2){
       return false;
    }
    ListExpr fracX = v.first();
    ListExpr fracY = v.second();
    Double fx = LEUtils.readFrac(fracX);
    Double fy = LEUtils.readFrac(fracY);
    if(fx==null || fy==null){
       return false;
    }
    coord[0] += fx;
    coord[1] += fy;
    if(!useProjection){
       return true;
    }
    if(ProjectionManager.project(coord[0],coord[1],aPoint)){
       coord[0] = aPoint.x;
       coord[1] = aPoint.y;
       return true;    
    } else {
       return false;
    }
  }
}
