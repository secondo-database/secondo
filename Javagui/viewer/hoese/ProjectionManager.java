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

package viewer.hoese;

import project.*;
import java.awt.geom.*;
import tools.Reporter;


public class ProjectionManager {


   public static boolean project(double x, double y, java.awt.geom.Point2D.Double result){
      return P.project(x,y,result);
   }   


   public static boolean showSettings(){
      return P.showSettings();
   }

   public static String getName(){
      return P.getName();
   }

   public static Projection getVoidProjection(){
      return VP;
   }

   public static Projection getActualProjection(){
      return P;
   }

   public static void setProjection(Projection Prj){
     if(Prj!=null)
       P = Prj;
   }

   public static boolean isReversible(){
      return P.isReversible();
   }

   public static boolean getOrig(double px, double py, java.awt.geom.Point2D.Double result){
      return P.getOrig(px,py,result);
   }
   
   public static boolean estimateOrig(double px,double py, java.awt.geom.Point2D.Double result){
      java.awt.geom.Point2D.Double aPoint = new java.awt.geom.Point2D.Double();
      if(P.isReversible()){
         return  getOrig(px,py,result);
      }
      int maxIterations = 100000;
      double x = 0;
      double y = 0;
      double maxError=0.0000001; // note that is the square root of the actual error
      double minDx = 0.0000000001;
      double minDy = 0.0000000001;
      P.project(x,y,aPoint);
      double error = (px-aPoint.x)*(px-aPoint.x) + (py-aPoint.y)*(py-aPoint.y);
      double lastError=error;
      double dx = 1;
      double dy = 1;
      int iterations = 0;
      boolean changeX = true;
      while( error>maxError   &&
             iterations < maxIterations){
         iterations++;
         if(changeX){
            x = x + dx;
            // compute the new error
            P.project(x,y,aPoint);
            error = (px-aPoint.x)*(px-aPoint.x) + (py-aPoint.y)*(py-aPoint.y);
            if(error < lastError){
                 lastError = error;
                 dx=dx*2;
            } else {
               x = x -dx; // switch to the last value// dont change lastError
               if(dx>0)
                  dx = -dx;
               else{
                  dx = -dx /2;
                  if(dx < minDx)
                      dx = minDx;
               } 
            }
            changeX = false;
         }  else { // changeY
            y = y + dy;
            // compute the new error
            P.project(x,y,aPoint);
            error = (px-aPoint.x)*(px-aPoint.x) + (py-aPoint.y)*(py-aPoint.y);
            if(error < lastError){
                 lastError = error;
                 dy=dy*2;
            } else {
               y = y -dy; // switch to the last value// dont change lastError
               if(dy>0)
                  dy = -dy;
               else{
                  dy = -dy /2;
                  if(dy < minDy)
                     dy = minDy;
               }
            }
            changeX = true;
         }
      }
    if(iterations >= maxIterations){
       Reporter.debug("give up after "+maxIterations+" iterations\n"+
                      "error was " + error +"\n"+
                      "Last values are " + x + " , " + y);
       return false;
    }
    result.x = x;
    result.y = y;
    return true;

   }
 

private static Projection P = new  VoidProjection();

private static Projection VP = new VoidProjection();
private static double EPSILON=0.00001;

}
