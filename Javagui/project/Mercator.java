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



public class Mercator implements Projection{

   public double getPrjX(double lambda, double phi) throws InvalidInputException{
     //     if(lambda>180 || lambda<-180)
       // throw new InvalidInputException("lambda out of range");
     return lambda-Lambda_0;
   }

   public double getPrjY(double lambda, double phi) throws InvalidInputException{
     if(phi>90 || phi<-90)
        throw new InvalidInputException("phi out of range");
     if( phi<=(-90+secure_distance))
         // throw new InvalidInputException("phi is to near to a pol");
	phi = -90+secure_distance;
     if( phi>=(90-secure_distance))
        phi = 90-secure_distance;

     double phi_2 = phi*PI/180;
     double y =   Math.log(Math.tan(PI/4 + phi_2/2))/LOG_E;
     return y*180/PI;
   }

   public boolean showSettings(){
     System.out.println("Mercator.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "Mercator";
   }

   public boolean isReversible(){
      return true;
   }

   public double getOrigX(double prjx, double prjy){
      return prjx + Lambda_0; 
   }

   public double getOrigY(double prjx, double prjy) throws InvalidInputException{
       try{
         prjy = prjy*PI/180;
         double res =  2*Math.atan(Math.exp(prjy))-0.5*PI;    
         return res*180/PI;
       } catch(Exception e){
              throw new InvalidInputException("Mercator:getOrigY: prjy out of range");
       }
   }



   private double Lambda_0 = 0;
   private double secure_distance = 1;

}
