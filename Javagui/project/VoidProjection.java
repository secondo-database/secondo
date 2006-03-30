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

public class VoidProjection implements Projection{

   public boolean project(double lambda, double phi, java.awt.geom.Point2D.Double result){
     result.x = lambda;
     result.y = phi;
     return true;
   }

   public boolean showSettings(){
     Reporter.writeWarning("VoidProjection.showSettings not implemented");
     return true;
   }

   public String getName(){
     return "VoidProjection";
   }

   public boolean isReversible(){
      return true;
   }

   public boolean getOrig(double x, double y, java.awt.geom.Point2D.Double result){
       result.x = x;
       result.y = y;
       return true;
   }
}
