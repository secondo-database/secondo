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

package extern.shapereader;

import sj.lang.ListExpr;

public class Point2D{


/** creates a new point at given position */
public Point2D(double x, double y){
   this.x = x;
   this.y = y;
}

/** creates a new point at (0,0) */
public Point2D(){
  x = 0.0;
  y = 0.0;
}

/** returns true if this equals to P */
public boolean equals(Object P){
  if(!(P instanceof Point2D))
     return false;
  Point2D  PP = (Point2D)P;
  return (float)x==(float)PP.x && (float)y==(float)PP.y;
}

/** move this Point to the given position */
public void moveTo(double x, double y){
  this.x = x;
  this.y = y;
}

/** returns the x position of this point */
public double getX(){
   return x;
}


/** returns the y position of this point */
public double getY(){
   return y;
}

public ListExpr getList(){
  return ListExpr.twoElemList(ListExpr.realAtom(x),
                              ListExpr.realAtom(y));
}

public String toString(){
   return "("+x+" , "+y+")";
}

private double x;
private double y;

}
