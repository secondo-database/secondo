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

package project;

import tools.Reporter;

public class Orthographic implements Projection{


   public boolean project(double lambda, double phi,java.awt.geom.Point2D.Double result ){
      double px=0,py=0;
      try{
				double l1 = (lambda-Lambda_0)*PI/180;
				double p1 = phi*PI/180;
				double x= Math.cos(p1)*Math.sin(l1);
				px= x*180/PI;
        double y = Math.cos(Phi_1)*Math.sin(p1)-Math.sin(Phi_1)*Math.cos(p1)*Math.cos(l1);
        py = y*180/PI;
      }catch(Exception e){
         return false;
      }
      result.x = px;
      result.y = py;
      return true;
   }

   public boolean showSettings(){
     Reporter.writeWarning("Orthographic.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "Orthographic";
   }

   public boolean isReversible(){
      return true;
   }

   public boolean getOrig(double x, double y, java.awt.geom.Point2D.Double result){
      double lambda=0;
      double phi=0;
      try{
        x = x*PI/180;
        y = y*PI/180;
        double p = Math.sqrt(x*x+y*y);
        double c = Math.asin(p);
        double x_t =  Lambda_0 + Math.atan((x*Math.sin(c)) / 
                     (p*Math.cos(Phi_1)*Math.cos(c) - 
                      y*Math.sin(Phi_1)*Math.sin(c)  ));
        lambda = x_t*180/PI;
        double y_t = Math.asin( Math.cos(c)*Math.sin(Phi_1)+y*Math.sin(c)*Math.cos(Phi_1)/p);
        phi= 180*y_t/PI;

      }catch(Exception e){
          return false;
      }
      result.x = lambda;
      result.y = phi;
      return true;
   }

   private double Lambda_0 = 0;
   private double Phi_1 = 0*PI/180;
   private double secure_distance = 10;

}
