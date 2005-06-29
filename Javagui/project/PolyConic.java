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



public class PolyConic extends ProjectionAdapter{

   public boolean project(double lambda, double phi, java.awt.geom.Point2D.Double result){
       double px = 0;
       double py = 0;
       try{

          if(Math.abs(phi)<5)
              phi=5*signum(phi);
          double phi_2 = phi*PI/180;
          double lambda_2 = (lambda-Lambda_0)*PI/180;
          double E = lambda_2*Math.sin(phi_2);
          double x= cot(phi_2)*Math.sin(E);
          px= x*180/PI;
          double y = (phi-Phi_0)*PI/180+ cot(phi_2)*(1-Math.cos(E));
          py = y*180/PI;
       }catch(Exception e){
          return false;
       }
       result.x = px;
       result.y = py;
       return true;
   }

   public boolean showSettings(){
     System.out.println("Polyconic.showSettings not implemented");
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

   private double Lambda_0 = 0;
   private double Phi_0 = 0;          
   private double secure_distance = 1;

}
