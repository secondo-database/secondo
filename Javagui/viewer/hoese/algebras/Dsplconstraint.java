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
  Area areaPolygons;
  Point2D.Double[] arrPoints;
  Rectangle2D.Double bounds;
  GeneralPath GPLines;

  public int numberOfShapes(){
     return 3;
  }

public boolean isPointType(int num){
    return (num==2);
}

  /** returns true because this type is a line **/
  public boolean isLineType(int num){
    return (num==1);
  }

  public Shape getRenderObject(int num,AffineTransform at){
    if(num==0)
    {
        return areaPolygons;
    } else if(num==1){
      return GPLines;
    } else if(num==2){
        Area res = new Area();
        double ps = Cat.getPointSize(renderAttribute,CurrentState.ActualTime);
        double pixx = Math.abs(ps/at.getScaleX());
        double pixy = Math.abs(ps/at.getScaleY());
        for(int i=0; i< arrPoints.length;i++)
        {
          if(Cat.getPointasRect())
          {
	        res.add(new Area(new Rectangle2D.Double(arrPoints[i].x-pixx/2,
	                                        arrPoints[i].y-pixy/2,pixx,pixy)));
          }
          else
          {
            res.add(new Area(new Ellipse2D.Double(arrPoints[i].x-pixx/2,
	                                        arrPoints[i].y-pixy/2,pixx,pixy)));
          }
       }
       return res;
    }
    else
    {
      return null;
    }

  }


  /**
   * Scans the numeric representation of a constraint datatype
  */
  private void ScanValue (ListExpr v) {

    boolean correct = true;
    ListExpr symbolicTuplesNL = v;
    ListExpr linConstraintsNL;
    ListExpr oneLinConstraintNL;

     boolean emptyPolygons = true;
     boolean emptyLines = true;
     boolean emptyPoints = true;
     Vector vecPoints = new Vector();

     areaPolygons = new Area();
     //Line2D.Double newSegment = null;
     GPLines = new GeneralPath(GeneralPath.WIND_EVEN_ODD);

    if(!v.isAtom())
    {
      //Shape currentShape;


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
              Double a1 = LEUtils.readNumeric(oneLinConstraintNL.first());
              Double a2 = LEUtils.readNumeric(oneLinConstraintNL.second());
              Double b = LEUtils.readNumeric(oneLinConstraintNL.third());
              String strOp = oneLinConstraintNL.fourth().symbolValue();

              LinearConstraint2D linConstraint = new LinearConstraint2D(a1.doubleValue(), a2.doubleValue(), b.doubleValue(), strOp);
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
            emptyPolygons = false;
            GeneralPath GPPolygon = new GeneralPath(GeneralPath.WIND_EVEN_ODD);
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
      	              GPPolygon.moveTo((float)point.x, (float)point.y);
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
      	              GPPolygon.lineTo((float)point.x, (float)point.y);
      	           }
                }catch(Exception e){
      	           Reporter.writeError("Error in Projection at ("+point.x+","+point.y+")");
      		          err=true;
      		          return;
      	        }
              }
            }
            GPPolygon.closePath();
            areaPolygons.add(new Area(GPPolygon));
          }
          else if(nrOfCEQ==1)
          {
            // line segment:
            emptyLines = false;
            // the EQ-constraint is the very first
            Point2D.Double pointFirst = GetIntersectionPoint((LinearConstraint2D)vLinConstraints.elementAt(0), (LinearConstraint2D)vLinConstraints.elementAt(1));
            Point2D.Double pointSecond = GetIntersectionPoint((LinearConstraint2D)vLinConstraints.elementAt(0), (LinearConstraint2D)vLinConstraints.elementAt(2));
            try{
              if(!ProjectionManager.project(pointFirst.x,pointFirst.y,aPoint)){
                  Reporter.writeError("error in projection at ("+pointFirst.x+", "+pointFirst.y+")");
                  err=true;
                  return;
               }
               else{
  	              GPLines.moveTo((float)pointFirst.x, (float)pointFirst.y);
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
  	              GPLines.lineTo((float)pointSecond.x, (float)pointSecond.y);
  	           }
            }catch(Exception e){
  	           Reporter.writeError("Error in Projection at ("+pointSecond.x+","+pointSecond.y+")");
  		          err=true;
  		          return;
  	        }
          }
          else if(nrOfCEQ==2)
          {
            // point:
            emptyPoints = false;
            Point2D.Double point = GetIntersectionPoint((LinearConstraint2D)vLinConstraints.elementAt(0), (LinearConstraint2D)vLinConstraints.elementAt(1));
            try{
              if(!ProjectionManager.project(point.x,point.y,aPoint)){
                  Reporter.writeError("error in projection at ("+point.x+", "+point.y+")");
                  err=true;
                  return;
               }else{
                    vecPoints.addElement(point);
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
    }
    else
    {
      Reporter.writeError("Error: instance is atomic (shoud be a list!)");
    }

     if(emptyPolygons)
     {
        areaPolygons = null;
     }
     if(emptyLines)
     {
        GPLines = null;
     }
     arrPoints = (Point2D.Double[])vecPoints.toArray(new Point2D.Double[0]);
  }

  /**
   * Init. the Dsplconstraint instance.
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": GA(constraint))"));
      return;
    }
    else
    {
      qr.addEntry(this);
      bounds = null;
      if(areaPolygons!=null)
      {
        bounds = (Rectangle2D.Double)areaPolygons.getBounds2D();
      }
      if(GPLines!=null)
      {
        Area areaLines = new Area(GPLines.getBounds2D());
        if (bounds == null)
        {
          bounds = (Rectangle2D.Double)areaLines.getBounds2D();
        }
        else
        {
          bounds = (Rectangle2D.Double)bounds.createUnion((Rectangle2D.Double)areaLines.getBounds2D());
        }
      }
      for(int i=0; i< arrPoints.length;i++)
      {
        if (bounds == null)
          bounds = new Rectangle2D.Double(arrPoints[i].x, arrPoints[i].y, 0, 0);
        else
          bounds = (Rectangle2D.Double)bounds.createUnion(new Rectangle2D.Double(arrPoints[i].x, arrPoints[i].y, 0, 0));
      }

    }
  }

  public Rectangle2D.Double getBounds () {
    return bounds;
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
