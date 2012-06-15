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

//2012, June Simone Jandt

package  viewer.hoese.algebras.jnet;

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;



/**
 * RouteLocation
 * Describes a single position on a route by route id, distance from start and
 * side of the route the position is reachable from.
 */
public class RouteLocation{

   private Integer rid;
   private Double pos;
   private JDirection dir;

  public RouteLocation(ListExpr value)
  {
    if (value.listLength() == 3){
      if(value.first().atomType()==ListExpr.INT_ATOM)
        rid = value.first().intValue();
      else
        rid = -1;
      if (value.second().atomType()==ListExpr.REAL_ATOM)
        pos = value.second().realValue();
      else
        pos = -1.0;
      dir = new JDirection(value.third());
    }
    else {
      rid = -1;
      pos = -1.0;
    }

  }

  public String toString(){
    if (rid > -1 && pos > -1.0)
      return "rid " + rid.toString() + " pos " + pos.toString() +
             " side " + dir.toString();
    else
      return "undefined";
  }

}



