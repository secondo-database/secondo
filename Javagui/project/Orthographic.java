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



public class Orthographic implements Projection{

   public double getPrjX(double lambda, double phi) throws InvalidInputException{
      double l1 = (lambda-Lambda_0)*PI/180;
      double p1 = phi*PI/180;
      double x= Math.cos(p1)*Math.sin(l1);
      return x*180/PI;
   }

   public double getPrjY(double lambda, double phi) throws InvalidInputException{
       double l1 = (lambda-Lambda_0)*PI/180;
       double p1 = phi*PI/180;
       double y = Math.cos(Phi_1)*Math.sin(p1)-Math.sin(Phi_1)*Math.cos(p1)*Math.cos(l1);
       return y*180/PI;
   }

   public boolean showSettings(){
     System.out.println("Orthographic.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "Orthographic";
   }


   private double Lambda_0 = 0;
   private double Phi_1 = 0*PI/180;
   private double secure_distance = 10;

}
