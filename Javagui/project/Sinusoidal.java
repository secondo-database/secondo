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



public class Sinusoidal implements Projection{

   public double getPrjX(double lambda, double phi) throws InvalidInputException{
      double phi_1 = phi*PI/180;
      double L_1 = (lambda-Lambda_0)*PI/180;
      double rr = L_1*Math.cos(phi_1);
      return rr*180/PI;
   }

   public double getPrjY(double lambda, double phi) throws InvalidInputException{
     return phi;
   }

   public boolean showSettings(){
     System.out.println("Sinusoisal.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "Sinusoidal";
   }

   public boolean isReversible(){
      return true;
   }

   public double getOrigX(double x, double y){
      x = x*PI/180;
      y = y*PI/180;
      double x_t = Lambda_0 + x / Math.cos(y);
      return 180*x_t/PI;
   }

   public double getOrigY(double x, double y){
      return y;
   }



   private double Lambda_0 = 0;
   private double secure_distance = 1;

}
