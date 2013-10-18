//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package  viewer.hoese.algebras;

import java.awt.geom.*;
import java.awt.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.*;
import viewer.hoese.*;
import tools.Reporter;
import java.text.DecimalFormat;


/**
 * The displayclass of the Rose algebras point datatype.
 */
public class Dsplpoint extends DisplayGraph implements LabelAttribute {
  /** The internal datatype representation */
  Point2D.Double point;
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
  public Dsplpoint (Point2D.Double p, DisplayGraph dg) {
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
  public Shape getRenderObject (int num,AffineTransform at) {
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

  public static boolean fillCoordsS(ListExpr v, double[] coord, boolean useProjection){
    if (v.listLength() != 2) {
      Reporter.writeError("Error: No correct point expression: 2 elements needed");
      return false;
    }
    for (int koordindex = 0; koordindex < 2; koordindex++) {
      Double d = LEUtils.readNumeric(v.first());
      if (d == null) {
        return false;
      }
      coord[koordindex] = d.doubleValue();
      v = v.rest();
    }
    if(!useProjection){
        return true;
    }

    if(ProjectionManager.project(coord[0],coord[1],aPoint)){
          coord[0] = aPoint.x;
          coord[1] = aPoint.y;
          return true;
    }
    else{
         return false;
    }
  }
  
  public boolean fillCoords(ListExpr v, double[] coord, boolean useProjection){
      return   Dsplpoint.fillCoordsS(v,coord,useProjection);
  }

  protected void ScanValue (ListExpr v) {
    if(isUndefined(v)){
        err=false;
        point=null;
        label = "-";
        return;
    }
    double koord[] = new double[2];
    if(!fillCoords(v,koord,true)){
      err = true;
      label = null;
      point = null;
    } else {
      err=false;
      point = new Point2D.Double(koord[0],koord[1]);
      label = "("+format.format(point.getX())+", "+format.format(point.getY())+")";
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



