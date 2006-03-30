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


public class PolyConic extends ProjectionAdapter{


   public boolean project(double lambda, double phi, java.awt.geom.Point2D.Double result){
       double px = 0;
       double py = 0;
       try{

          lambda = lambda*PI/180;
          phi = phi*PI/180;
          double E = (lambda-Lambda_0)*Math.sin(phi);
          double x = cot(phi)*Math.sin(E);
          double y = (phi-Phi_0)+cot(phi)*(1-Math.cos(E)); 
          px = x;
          py = y;

       }catch(Exception e){
          return false;
       }
       result.x = px;
       result.y = py;
       return true;
   }

   public boolean showSettings(){
     Reporter.writeWarning("Polyconic.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "Polyconic";
   }


   private double cot(double z){
     return Math.cos(z)/Math.sin(z);
   }

   private double signum(double d){
     return d>=0?1:-1;
   }

   public boolean isReversible(){return true;}

   public boolean getOrig(double x , double y, java.awt.geom.Point2D.Double result){
      try{
         double A = Phi_0+y;
         double B = x*x + A*A;
         double phi = A;
         double Delta_Phi= -(A*phi*(Math.tan(phi)+1)-phi-0.5*(phi*phi+B)*Math.tan(phi))   /  ( (phi-A)/Math.tan(phi)-1 );
         int iterations = 10000;

         while(Math.abs(Delta_Phi) > 0.00000000001 && iterations > 0){
           iterations--;
           phi = phi + Delta_Phi;
           Delta_Phi= -(A*(phi*Math.tan(phi)+1)-phi-0.5*(phi*phi+B)*Math.tan(phi))   /  ( (phi-A)/Math.tan(phi)-1 );
         }
         if(iterations==0)
            return false;
         double lambda = Math.asin(x*Math.tan(phi))/Math.sin(phi)+Lambda_0; 
         result.setLocation(lambda*180/PI,phi*180/PI);
         return true;

      }catch(Exception e){
        return false;
      }



   }

   private double Lambda_0 = 0;
   private double Phi_0 = 0;          
   private double secure_distance = 1;

}
