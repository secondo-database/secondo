
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

  /**
   * Scans the representation of the region datatype and build up the areas by adding or
   * subtracting(hole) single polygons.
   * @param v A list of polygons/holepolygons
   * @see sj.lang.ListExpr
   * @see <a href="Dsplregionsrc.html#ScanValue">Source</a>
  */
  public void ScanValue (ListExpr value) {
    if (value.isEmpty()) {
      err = true;
      return;
    }
    double koord[] = new double[2];
    //System.out.println(value.writeListExprToString());
    areas = new Area();
    double x,y;
    while (!value.isEmpty()) {                  // value while
      ListExpr face = value.first();
      ////System.out.println(v.writeListExprToString());
      boolean isHole = false;
      //Vector polygons= new Vector(10,10);
      Area area = new Area();
      while (!face.isEmpty()) {                 // face while
        ListExpr poly = face.first();
        //Vector vertices= new Vector(10,10);
        GeneralPath path = new GeneralPath();
        boolean firstpoint = true;
        while (!poly.isEmpty()) {               //poly while
          ListExpr v = poly.first();
          if (v.listLength() != 2) {
            System.out.println("Error: No correct region expression: 2 elements needed");
            err = true;
            return;
          }
          for (int koordindex = 0; koordindex < 2; koordindex++) {
            Double d = LEUtils.readNumeric(v.first());
            if (d == null) {
              err = true;
              return;
            }
            koord[koordindex] = d.doubleValue();
            v = v.rest();
          }                     //end for
          try{
	    x = ProjectionManager.getPrjX(koord[0],koord[1]);
	    y = ProjectionManager.getPrjY(koord[0],koord[1]);
	    if (firstpoint)
               path.moveTo((float)x, (float)y);
            else
               path.lineTo((float)x, (float)y);
	  } catch(Exception e){
	     System.out.println("error in projection: values ("+koord[0]+","+koord[1]+")");
	     err = true;
	     return;

	  }
          //vertices.add(point);
          firstpoint = false;
          poly = poly.rest();
        }       //end poly while
        //polygons.add(vertices);
        Area a = new Area(path);
        if (isHole)
          area.subtract(a); 
        else 
          area.add(a);
        isHole = true;
        face = face.rest();
      }         //end face while
      areas.add(area);
      value = value.rest();
    }           //end value while
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




