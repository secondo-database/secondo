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
  /** The bounding-box rectangle */
  Rectangle2D.Double bounds;
  /** The shape representing this sequence */
  GeneralPath GP = new GeneralPath();
	/** If only one point is contained in the poinmtsequence,
      we use a representation as a point */
  Point2D.Double point=new Point2D.Double();
  /** flag defining whether the pointsequence contains of a
      single point **/
  boolean single = false;
  /** flag defining whether the pointsequence is empty */
  boolean empty = true;

  
  /** returns true if this sequence contains two or more
      points. **/
  public boolean isLineType(){
    return !single && !empty;
  }

  /** returns true if this seuqnces consist of a single point **/
  public boolean isPointType(){
     return single && !empty;
  }
  /** returns true if this sequence is empty **/
  public boolean isEmpty(){
     return empty;
  }

  /** sets this sequence to be empty **/
  public void reset(){
    GP.reset();
    empty=true;
    single=false; 
  }

  /** Adds a single point to this sequence.
    * The return value is true when the kind of the 
    * representation is changed, e.g. from point to line.
    **/
  public boolean add(double x, double y){
     if(empty){
       point.setLocation(x,y);
       GP.moveTo((float)x,(float)y);
       empty=false;
       single=true;
       bounds = new Rectangle2D.Double();
       bounds.setRect(x,y,0,0);
       return true; // changed from empty to point
     }else{
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
       if(single){
          single=false;
          return true; // changed from point to line
       }
       return false;
     }
  }



 /**
   * Scans the representation of the sequnece datatype .
   * @param v A list of segments
   * @see sj.lang.ListExpr
  */
  public void ScanValue (ListExpr value) {
      err = false;
      GP.reset();
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
    	   if(!ProjectionManager.project(x,y,aPoint)){
            err = true; 
            if(gui.Environment.DEBUG_MODE)
               System.out.println("error in project  point t("+x+" , " +y+")");
            reset();
            return;
				  }else{ 
					   double x1 = aPoint.x;
					   double y1 = aPoint.y;
             add(x1,y1);
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
    if(empty){
       bounds = null;
       qr.addEntry(AttrName + " : empty");
       return;
    }
    qr.addEntry(this);
    bounds = new Rectangle2D.Double();
    if(!single)
       bounds.setRect(GP.getBounds2D());
     else
       bounds.setRect(point.getX(),point.getY(),0,0);
  
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
        (!bounds.intersects(xpos - 5.0*scalex, ypos - 5.0*scaley, 10.0*scalex,
        10.0*scaley)))
      return  false;
    Rectangle2D.Double r = new Rectangle2D.Double(xpos - 5.0*scalex, ypos -
        5.0*scaley, 10.0*scalex, 10.0*scaley);
    return RenderObject.intersects(r);
  }


public Shape getRenderObject (AffineTransform at) {
    if(empty)
      return null;
    if(single){
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



