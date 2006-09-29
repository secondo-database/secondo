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
import tools.Reporter;


/**
 * The displayclass of the Constraint algebras constraint datatype.
 */
public class Dsplconstraint extends DisplayGraph {
/** The internal datatype representation */
  Shape[] geoShapes;
  /**
   * Scans the numeric representation of a point datatype 
   * @param v the numeric value of the x- and y-coordinate
   * @see sj.lang.ListExpr
   * @see <a href="Dsplpointsrc.html#ScanValue">Source</a>
  */ 
  private void ScanValue (ListExpr v) {
    Reporter.writeError("ScanValue gestartet!!");
    
    Vector vGeoObjects = new Vector();
    
    /*for(int i=0; i < 5; i++) 
    try{
    {
      Shape geoObject
      GeneralPath gp = new GeneralPath(GeneralPath.WIND_EVEN_ODD);
      gp.moveTo(0.0f+i*5, 0.0f+i*5);
      gp.lineTo(-20.0f+i*5, 3.0f+i*5);
      gp.lineTo(0.0f+i*5, 10.0f+i*5);
      gp.lineTo(10.0f+i*5, 10.0f+i*5);
      gp.lineTo(12.0f+i*5, 7.0f+i*5);
      gp.lineTo(10.0f+i*5, 0.0f+i*5);
      gp.closePath();           
      vGeoObjects.addElement(geoObject);
    }    
    
    geoShapes = (Shape[])vGeoObjects.toArray(new Shape[0]);   
   
    } catch (Exception e) {
     Reporter.writeError("Fehler in ScanValue: "+e); 
    }
    Reporter.writeError("ScanValue erfolgreich beendet!!");
    */
    /*double koord[] = new double[2];
    if (v.listLength() != 4) {
      Reporter.writeError("Error: No correct rectangle expression: 4 elements needed");
      err = true; 
      return;
    }
    if ((v.first().atomType() != ListExpr.INT_ATOM) || (v.second().atomType()
          != ListExpr.INT_ATOM) || (v.third().atomType() != ListExpr.INT_ATOM)
          || (v.fourth().atomType() != ListExpr.INT_ATOM)) {
      Reporter.writeError("Error: No correct rectangle : 4 INTs needed");
      err = true;
      return;
    }
    if (!err) {
      rect = new Rectangle2D.Double(v.first().intValue(),v.third().intValue(),v.second().intValue()-v.first().intValue(),
        v.fourth().intValue()-v.third().intValue());
    }
    
    //rect = new Rectangle2D.Double(-10.0,-10.0,20.0,20.0);
    
    
    */
    
    boolean correct = true;
    ListExpr symbolicTuplesNL = v;
    ListExpr linConstraintsNL;
    ListExpr oneLinConstraintNL;
    
    if(!v.isAtom())
    {  
      Shape currentShape;
      while(!symbolicTuplesNL.isEmpty())
      {
        linConstraintsNL = symbolicTuplesNL.first();
        symbolicTuplesNL = symbolicTuplesNL.rest();        
        if(!linConstraintsNL.isAtom())
        {
          Shape geoObject;
          
          Vector vLinConstraints = new Vector();        
          int nrOfCEQ = 0;
          int currentIndex = 0;
          int firstIndexCEQ = -1;
          int secondIndexCEQ = -1;
          while(!linConstraintsNL.isEmpty())
          {
            oneLinConstraintNL = linConstraintsNL.first();
            linConstraintsNL = linConstraintsNL.rest();
            if(oneLinConstraintNL.listLength() == 4  &&
                oneLinConstraintNL.first().atomType() == ListExpr.REAL_ATOM &&
                oneLinConstraintNL.second().atomType() == ListExpr.REAL_ATOM &&
                oneLinConstraintNL.third().atomType() == ListExpr.REAL_ATOM &&
                oneLinConstraintNL.fourth().atomType() == ListExpr.SYMBOL_ATOM)
            {
              LinearConstraint2D linConstraint = new LinearConstraint2D(oneLinConstraintNL.first().realValue(), oneLinConstraintNL.second().realValue(), oneLinConstraintNL.third().realValue(), oneLinConstraintNL.fourth().symbolValue());
              vLinConstraints.addElement(linConstraint);
              if(linConstraint.strOp.equals("eq")) 
              {
                nrOfCEQ++; 
                if(firstIndexCEQ==-1) 
                {
                  firstIndexCEQ = currentIndex;
                } 
                else if(secondIndexCEQ==-1)
                {
                  secondIndexCEQ = currentIndex;
                }
                else 
                {
                  // then there are more than 2 EQ-constraints, what is a contradiction to the normalization! 
                  Reporter.writeError("Error: Too many EQ-constraints in symbolic tuple (max 2)!");
                }
              }
            }
            else
            {
              Reporter.writeError("Error: Fehlerhafter Aufbau von linearem Constraint!");
            }            
            currentIndex++;
          }          
          // The number of eq-constraints determines the type of the shape:
          if(nrOfCEQ==0)
          {
            // convex polygon:
            Reporter.writeError("DEBUG: CONVEX POLYGON");
            GeneralPath gp = new GeneralPath(GeneralPath.WIND_EVEN_ODD);
            for(int i=0; i < vLinConstraints.size(); i++)
            {
              Point2D.Double point;
              if(i==0)
              {
                point = GetIntersectionPoint((LinearConstraint2D)vLinConstraints.lastElement(), (LinearConstraint2D)vLinConstraints.firstElement());
                try{
                  if(!ProjectionManager.project(point.x,point.y,aPoint)){
                      Reporter.writeError("error in projection at ("+point.x+", "+point.y+")");
                      err=true;
                      return;
                   }else{
      	              gp.moveTo((float)point.x, (float)point.y);
      	           } 
                }catch(Exception e){
      	           Reporter.writeError("Error in Projection at ("+point.x+","+point.y+")");
      		          err=true;
      		          return;
      	        }                  
              }
              else
              {
                point = GetIntersectionPoint((LinearConstraint2D)vLinConstraints.elementAt(i-1), (LinearConstraint2D)vLinConstraints.elementAt(i));
                try{
                  if(!ProjectionManager.project(point.x,point.y,aPoint)){
                      Reporter.writeError("error in projection at ("+point.x+", "+point.y+")");
                      err=true;
                      return;
                   }else{
      	              gp.lineTo((float)point.x, (float)point.y);
      	           } 
                }catch(Exception e){
      	           Reporter.writeError("Error in Projection at ("+point.x+","+point.y+")");
      		          err=true;
      		          return;
      	        }                          
              }
            }
            gp.closePath();
            geoObject = gp;
            vGeoObjects.addElement(geoObject);
          } 
          else if(nrOfCEQ==1)
          {
            // line segment:
            Reporter.writeError("DEBUG: SEGMENT");
            // the EQ-constraint is the very first
            Point2D.Double pointFirst = GetIntersectionPoint((LinearConstraint2D)vLinConstraints.elementAt(0), (LinearConstraint2D)vLinConstraints.elementAt(1));
            Point2D.Double pointSecond = GetIntersectionPoint((LinearConstraint2D)vLinConstraints.elementAt(0), (LinearConstraint2D)vLinConstraints.elementAt(2));
            GeneralPath gp = new GeneralPath(GeneralPath.WIND_EVEN_ODD);
            try{
              if(!ProjectionManager.project(pointFirst.x,pointFirst.y,aPoint)){
                  Reporter.writeError("error in projection at ("+pointFirst.x+", "+pointFirst.y+")");
                  err=true;
                  return;
               }else{
  	              gp.moveTo((float)pointFirst.x, (float)pointFirst.y);
  	           } 
            }catch(Exception e){
  	           Reporter.writeError("Error in Projection at ("+pointFirst.x+","+pointFirst.y+")");
  		          err=true;
  		          return;
  	        }            
            try{
              if(!ProjectionManager.project(pointSecond.x,pointSecond.y,aPoint)){
                  Reporter.writeError("error in projection at ("+pointSecond.x+", "+pointSecond.y+")");
                  err=true;
                  return;
               }else{
  	              gp.lineTo((float)pointSecond.x, (float)pointSecond.y);
  	           } 
            }catch(Exception e){
  	           Reporter.writeError("Error in Projection at ("+pointSecond.x+","+pointSecond.y+")");
  		          err=true;
  		          return;
  	        }                        
            geoObject = gp;
            vGeoObjects.addElement(geoObject);
          }
          else if(nrOfCEQ==2)
          {
            // point:
            Reporter.writeError("DEBUG: POINT");
            Point2D.Double point = GetIntersectionPoint((LinearConstraint2D)vLinConstraints.elementAt(0), (LinearConstraint2D)vLinConstraints.elementAt(1));
            try{
              if(!ProjectionManager.project(point.x,point.y,aPoint)){
                  Reporter.writeError("error in projection at ("+point.x+", "+point.y+")");
                  err=true;
                  return;
               }else{
                  geoObject = new Rectangle2D.Double(point.x, point.y, 0, 0);            
                  vGeoObjects.addElement(geoObject);  	              
  	           } 
            }catch(Exception e){
  	           Reporter.writeError("Error in Projection at ("+point.x+","+point.y+")");
  		          err=true;
  		          return;
  	        }                        
          }
          else
          {
            Reporter.writeError("Error: nrOfCEQ to big!");
          }                    
        }
        else
        {
          Reporter.writeError("Error: linConstraintsNL is atomic (shoud be a list!)");
        }    
      } // while
      geoShapes = (Shape[])vGeoObjects.toArray(new Shape[0]);   
    }
    else
    {
      Reporter.writeError("Error: instance is atomic (shoud be a list!)");
      geoShapes = null;
    }        
  }
  
  public Shape[] getRenderObjects (AffineTransform at) {
    return geoShapes;
  }
  

  /**
   * Init. the Dsplrectangle instance.
   * @param type The symbol rectangle
   * @param value The 4 INTs  of a rectangle left,right,top,bottom.
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplrectanglesrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    Reporter.writeError("init gestartet!!");
    AttrName = type.symbolValue();
    ispointType = true;
    ScanValue(value);
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": GA(constraint))"));
      return;
    } 
    else 
    {
      qr.addEntry(this);   
      // folgende Zeilen sind nur ein workaround:
      //RenderObject = new Rectangle2D.Double(-10000.0,-10000.0,20000.0,20000.0);                    
      /*Rectangle2D.Double workaraoundBounds = null;      
      if(geoShapes!=null){
        for(int i=0;i<geoShapes.length;i++){
            if(workaraoundBounds==null){
               workaraoundBounds = (Rectangle2D.Double)(geoShapes[i].getBounds2D());
            }else{
               workaraoundBounds.add((Rectangle2D.Double)(geoShapes[i].getBounds2D()));
            }
        }
      }
      GeneralPath gp = new GeneralPath(GeneralPath.WIND_EVEN_ODD);
      gp.moveTo((float)workaraoundBounds.x, (float)workaraoundBounds.y);
      gp.lineTo((float)workaraoundBounds.x, (float)(workaraoundBounds.y+workaraoundBounds.height));
      gp.lineTo((float)(workaraoundBounds.x+workaraoundBounds.width), (float)(workaraoundBounds.y+workaraoundBounds.height));
      gp.lineTo((float)(workaraoundBounds.x+workaraoundBounds.width), (float)workaraoundBounds.y);
      */
      GeneralPath gp = new GeneralPath(GeneralPath.WIND_EVEN_ODD);
      gp.moveTo((float)-10005.0, (float)-10005.0);
      gp.lineTo((float)-10005.0, (float)10005.0);
      gp.lineTo((float)10005.0, (float)10005.0);
      gp.lineTo((float)10005.0, (float)10005.0);
      gp.lineTo((float)10005.0, (float)-10005.0);
      gp.lineTo((float)-10000.0, (float)-10005.0);
      gp.lineTo((float)-10000.0, (float)-10000.0);
      gp.lineTo((float)10000.0, (float)-10000.0);
      gp.lineTo((float)10000.0, (float)10000.0);
      gp.lineTo((float)-10000.0, (float)10000.0);
      gp.lineTo((float)-10000.0, (float)-10000.0);
      RenderObject = gp;
    }
  }
  
  
  Point2D.Double GetIntersectionPoint(final LinearConstraint2D linConFirst, final LinearConstraint2D linConSecond)
  {
    // Input: two linear constraints 
    // prerequisite: not parallel or equal (otherwise they woudn't intersection in one point)
    // Output: intersection point

    boolean blnIsPoint;
    Point2D.Double pIntersection;
    double x, y;
    
    try {
      x = (linConFirst.a2*linConSecond.b-linConSecond.a2*linConFirst.b)/(linConFirst.a1*linConSecond.a2-linConSecond.a1*linConFirst.a2);
      y = (linConFirst.a1*linConSecond.b-linConSecond.a1*linConFirst.b)/(linConFirst.a2*linConSecond.a1-linConSecond.a2*linConFirst.a1);
      pIntersection = new Point2D.Double(x,y);
    }
    catch (Exception e) {
      Reporter.writeError("Error: GetIntersectionPoint can't compute a single intersecting point - invalid input (lin. constraints describes parallel lines)!");               
      pIntersection = new Point2D.Double(0,0);
    }
    return pIntersection; 
  }


  class LinearConstraint2D
  {
    public double a1;
    public double a2;
    public double b;   
    public String strOp;         
    
    LinearConstraint2D() 
    {
      a1 = 0;
      a2 = 0;
      b = 0;
      strOp = new String("eq");      
    }
    
    LinearConstraint2D(double a1, double a2, double b, String strOp)
    {
      this.a1 = a1;
      this.a2 = a2;
      this.b = b;
      this.strOp = strOp;      
    }    
  }


}



