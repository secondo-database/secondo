
package  viewer.hoese.algebras;

import  java.awt.geom.*;
import  java.awt.*;
import  sj.lang.ListExpr;
import  java.util.*;
import  viewer.*;
import viewer.hoese.*;


/**
 * The displayclass of the region datatype (Rose algebra).
 */
public class Dsplregion extends DisplayGraph {
  /** The internal datastructure of the region datatype */
  Area areas;


  /** convert the Listrepresentation LE to a Area */
  public void ScanValue(ListExpr LE){
     if(LE==null){
       err=true;
       return;
     }
     areas = null;
     // first compute the number of all containing points
     ListExpr TMP  = LE;
     int no = 0;
     while(!TMP.isEmpty()){
         ListExpr Face = TMP.first();
	 TMP = TMP.rest();
         while(!Face.isEmpty()){
	    no = no+Face.first().listLength()+1;
            Face = Face.rest();
	 }
     }

     //System.out.println("Area with +"+no+" points");
     GeneralPath GP = new GeneralPath(GeneralPath.WIND_EVEN_ODD,no);
     while(!LE.isEmpty()){
        ListExpr Face = LE.first();
        LE = LE.rest();
	while(!Face.isEmpty()){
           ListExpr Cycle = Face.first();
	   Face = Face.rest();
           boolean isFirstPoint=true;
	   while(!Cycle.isEmpty()){
	      ListExpr P = Cycle.first();
	      Cycle=Cycle.rest();
	      Double x = LEUtils.readNumeric(P.first());
	      Double y = LEUtils.readNumeric(P.second());
	      if(x!=null && y!=null)
	      try{
	        float x1 = (float)ProjectionManager.getPrjX(x.doubleValue(),y.doubleValue());
	        float y1 = (float)ProjectionManager.getPrjY(x.doubleValue(),y.doubleValue());

	        if(isFirstPoint){
	            GP.moveTo(x1,y1);
		    isFirstPoint=false;
	         }else{
	            GP.lineTo(x1,y1);
	         }
	      } catch(Exception e){
	         System.out.println("Error in Projection at ("+x+","+y+")");
		 err=true;
		 return;
	      }
	      else{
	        err=true;
		return;
	      }
	   }
	   GP.closePath();
	}
     }
     areas= new Area(GP);
  }








  /**
   * Init. the Dsplregion instance.
   * @param type The symbol region
   * @param value A list of polygons /holepolygons
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplregionsrc.html#init">Source</a>
   */
   public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      System.out.println("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": GA(region))"));
      return;
    }
    else
      qr.addEntry(this);
    RenderObject = areas;
    // System.out.println(value.writeListExprToString());
  }


}




