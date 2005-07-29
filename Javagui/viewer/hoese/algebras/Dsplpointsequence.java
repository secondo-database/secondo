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

import  java.awt.geom.*;
import  java.awt.*;
import  sj.lang.ListExpr;
import  java.util.*;
import  viewer.*;
import viewer.hoese.*;


/**
 * The displayclass of the pointsequnece .
 */
public class Dsplpointsequence extends DisplayGraph {
   /** paint this pointsequence as a line */
   public final static int LINE_MODE=0;
   /** paint this pointsequence as a set of points */
   public final static int POINTS_MODE=1;
   /** fill the shape */
   public static final int AREA_MODE=2;  
  /** The bounding-box rectangle */
  Rectangle2D.Double bounds;
  /** The shape representing this sequence */
  GeneralPath GP = new GeneralPath(GeneralPath.WIND_EVEN_ODD);
  /** because of the reason of precision, we store the points also
      in a vector **/
  Vector points;
  /** drawMode */
  int mode = LINE_MODE;

  /** method changing the paint mode of this sequence */ 
  public void  setPaintMode(int mode){
     if(mode>=0 && mode<=2)
        this.mode = mode;
  }

  
  /** returns true if this sequence
      should be drawn as line **/
  public boolean isLineType(){
    if(mode==POINTS_MODE)
       return false;
     if(points==null)
       return false;
    if(mode==LINE_MODE)
       return points.size()>1;
    // AREA_MODE
    return points.size()==2;
  }

  /** returns true if this seuqnces consist of a single point **/
  public boolean isPointType(){
     if(mode==POINTS_MODE)
        return true;
     if(points==null)
        return true;
     if(mode==LINE_MODE)
        return points.size()>1;
     // AREA_MODE
     return points.size()==2; 
  }
  /** returns true if this sequence is empty **/
  public boolean isEmpty(){
     return points==null || points.size()==0;
  }

  /** sets this sequence to be empty **/
  public void reset(){
    GP.reset();
    points = null; 
  }

  /** Adds a single point to this sequence.
    * The return value is true when the kind of the 
    * representation is changed, e.g. from point to line.
    **/
  public boolean add(double x, double y){
     if(!ProjectionManager.project(x,y,aPoint)){
        err = true;
        return false;
     }
     if(points==null)
        points = new Vector();
     x = aPoint.x;
     y = aPoint.y;
     if(isEmpty()){
       points.add(new Dsplpoint(new Point2D.Double(x,y),this));
       GP.moveTo((float)x,(float)y);
       bounds = new Rectangle2D.Double();
       bounds.setRect(x,y,0,0);
       return true; // changed from empty to point
     }else{
       points.add(new Dsplpoint(new Point2D.Double(x,y),this));
       GP.lineTo((float)x,(float)y);
       double x1 = bounds.getX();
       double y1 = bounds.getY();
       double x2 = x1+bounds.getWidth();
       double y2 = y1+bounds.getHeight();
       x1 = Math.min(x1,x);
       y1 = Math.min(y1,y);
       x2 = Math.max(x2,x);
       y2 = Math.max(y2,y);
       bounds.setRect(x1,y1,x2-x1,y2-y1);
       if(mode==POINTS_MODE)
          return false;
       if(mode==LINE_MODE)
          return points.size()==2; // from point to line
       // AREA_MODE 
          return points.size()==3 || points.size()==2; 
     }
  }



 /**
   * Scans the representation of the sequence datatype .
   * @param v A list of segments
   * @see sj.lang.ListExpr
  */
  public void ScanValue (ListExpr value) {
      err = false;
      GP.reset();
      points = new Vector(value.listLength()+2);
      while(!value.isEmpty()){
         ListExpr apoint = value.first();
         if(apoint.listLength()!=2){
           err = true;
           reset();
           return;
         }
         Double X = LEUtils.readNumeric(apoint.first());
         Double Y = LEUtils.readNumeric(apoint.second());
         if(X==null || Y == null){
            reset();
            err = true;
            return;
         }
         double x = X.doubleValue();
         double y = Y.doubleValue();
         add(x,y);
         if(err){
            if(gui.Environment.DEBUG_MODE)
               System.out.println("error in project  point t("+x+" , " +y+")");
            reset();
            return;
         }
         value = value.rest();
    }
  }

  /**
   * Init. the Dsplpointsequence instance.
   * @param type The symbol line
   * @param value A list of segments.
   * @param qr queryresult to display output.
   * @see QueryResult
   * @see sj.lang.ListExpr
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      System.err.println("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": GA(pointsequence))"));
      bounds =null;
      return;
    }
    else
    if(isEmpty()){
       bounds = null;
       qr.addEntry(AttrName + " : empty");
       return;
    }
    qr.addEntry(this);
    bounds = new Rectangle2D.Double();
    if(points.size()>1)
       bounds.setRect(GP.getBounds2D());
     else{
       Point2D.Double point = ((Dsplpoint)points.get(0)).getPoint();
       bounds.setRect(point.getX(),point.getY(),0,0);
     }
  
  }


  /**
   * @return The boundingbox of this object
   */
  public Rectangle2D.Double getBounds () {
    return  bounds;
  }

  /**
   * Tests if a given position is near (10pxs) of this sequence.
   * @param xpos The x-Position to test.
   * @param ypos The y-Position to test.
   * @param scalex The actual x-zoomfactor
   * @param scaley The actual y-zoomfactor
   * @return true if x-, ypos is contained in this points type
   */
  public boolean contains (double xpos, double ypos, double scalex, double scaley) {
    if (bounds==null) return false; // an empty sequnece
    if ((bounds.getWidth()*bounds.getHeight()!=0) && 
        (!bounds.intersects(xpos - 5.0*scalex, ypos - 5.0*scaley, 10.0*scalex, 10.0*scaley)))
      return  false;
    Rectangle2D.Double r = new Rectangle2D.Double(xpos - 5.0*scalex, ypos - 5.0*scaley, 10.0*scalex, 10.0*scaley);
    if(mode==AREA_MODE)
       return RenderObject.intersects(r);
    if(mode==LINE_MODE)
       return stroke.createStrokedShape(RenderObject).intersects(r);
    // POINT_MODE
    if(points==null)
       return false;
    int len = points.size();
    for(int i=0;i<len;i++){
      Point2D.Double p = ((Dsplpoint) points.get(i)).getPoint();
      if(r.contains(p))
         return true;
    }
    return false;
  }


public Shape getRenderObject (AffineTransform at) {
    if(isEmpty())
      return null;
    if(mode==POINTS_MODE){
       GeneralPath rpoints = new GeneralPath();
       for(int i=0;i<points.size();i++){
            rpoints.append( ((Dsplpoint)points.get(i)).getRenderObject(at),false);
       }
       return rpoints;
    }
    if(points.size()==1){ // only one point
       Rectangle2D.Double r = getBounds();
       double pixy = Math.abs(Cat.getPointSize()/at.getScaleY());
       double pix = Math.abs(Cat.getPointSize()/at.getScaleX());
       if (Cat.getPointasRect())
          RenderObject = new Rectangle2D.Double(r.getX()- pix/2, r.getY() - pixy/2, pix, pixy);
       else {
          RenderObject = new Ellipse2D.Double(r.getX()- pix/2, r.getY() - pixy/2, pix, pixy);
        }
    } else{
     RenderObject = GP;
    }
    return RenderObject;
}




}



