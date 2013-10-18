

package viewer.hoese.algebras;

import sj.lang.ListExpr;

public class Dsplline2 extends Dsplline{


  protected boolean fillSegment(ListExpr v, double[] koord, boolean useProjection){
      if (v.listLength() != 2) {
         return super.fillSegment(v,koord,useProjection);
      }
      ListExpr p1 = v.first();
      ListExpr p2 = v.second();
      double[] pcoord = new double[2];
      if(!Dsplpoint2.fillCoordsS(p1,pcoord,useProjection)){
        return false;
      }
      koord[0] =  pcoord[0];
      koord[1] = pcoord[1];
      if(!Dsplpoint2.fillCoordsS(p2,pcoord,useProjection)){
        return false;
      }
      koord[2] = pcoord[0];
      koord[3] = pcoord[1];
      return true;
  }


}
