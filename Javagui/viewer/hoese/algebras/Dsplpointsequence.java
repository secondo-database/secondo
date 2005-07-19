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
  GeneralPath GP;
  

public boolean isLineType(){
  return true;
}


 /**
   * Scans the representation of the sequnece datatype .
   * @param v A list of segments
   * @see sj.lang.ListExpr
  */
  public void ScanValue (ListExpr value) {
     boolean first = true;
     err = false;
     GP = new GeneralPath();
      while(!value.isEmpty()){
         ListExpr point = value.first();
         if(point.listLength()!=2){
           err = true;
           return;
         }
         Double X = LEUtils.readNumeric(point.first());
         Double Y = LEUtils.readNumeric(point.second());
         if(X==null || Y == null){
            GP=null;
            err = true;
            return;
         }
         double x = X.doubleValue();
         double y = Y.doubleValue();
    	   if(!ProjectionManager.project(x,y,aPoint)){
            err = true; 
            if(gui.Environment.DEBUG_MODE)
               System.out.println("error in project  point t("+x+" , " +y+")");
            GP=null;
            return;
				  }else{ 
					   double x1 = aPoint.x;
					   double y1 = aPoint.y;
             if(first){
                 GP.moveTo((float)x1,(float)y1);
                 first=false;
              }else{
                  GP.lineTo((float)x1,(float)y1);
              }
          }
          value = value.rest();
    }
    if(first){ // empty sequence
       GP=null;
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
    RenderObject = GP;
    if (err) {
      System.out.println("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": GA(pointsequence))"));
      bounds =null;
      RenderObject=null;
      return;
    }
    else
      qr.addEntry(this);
    if(GP==null)
        bounds = null;
    else{
       bounds = new Rectangle2D.Double();
       bounds.setRect(GP.getBounds2D());
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
    if ((bounds.getWidth()*bounds.getHeight()!=0) && (!bounds.intersects(xpos - 5.0*scalex, ypos - 5.0*scaley, 10.0*scalex,
        10.0*scaley)))
      return  false;
    Rectangle2D.Double r = new Rectangle2D.Double(xpos - 5.0*scalex, ypos -
        5.0*scaley, 10.0*scalex, 10.0*scaley);
    return GP.intersects(r);
  }

}



