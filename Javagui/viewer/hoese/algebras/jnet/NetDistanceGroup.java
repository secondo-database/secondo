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
 * NetDistanceGroup
 * Describes network distances and path information of shortest paths in the
 * jnetwork, by source junction id, target junction id, id of next junction and
 * next section on the path and distance from source to target.
 */
public class NetDistanceGroup{

   private Integer source;
   private Integer target;
   private Integer viaJunction;
   private Integer viaSection;
   private Double dist;

  public NetDistanceGroup(ListExpr value)
  {
    if (value.listLength() == 5){
      if(value.first().atomType()==ListExpr.INT_ATOM)
        source = value.first().intValue();
      else
        source = -1;
      if (value.second().atomType()==ListExpr.INT_ATOM)
        target = value.second().intValue();
      else
        target = -1;
      if (value.third().atomType()==ListExpr.INT_ATOM)
        viaJunction = value.third().intValue();
      else
        viaJunction = -1;
      if (value.fourth().atomType()==ListExpr.INT_ATOM)
        viaSection = value.fourth().intValue();
      else
        viaSection = -1;
      if (value.fifth().atomType()==ListExpr.REAL_ATOM)
        dist = value.fifth().realValue();
      else
        dist = -1.0;

    }
    else
    {
      source = -1;
      target = -1;
      viaJunction = -1;
      viaSection = -1;
      dist = -1.0;
    }
  }

  public String toString(){
    if (source > -1 && target > -1 && viaJunction > -1 && viaSection > -1 &&
        dist > -1.0)
      return "from " + source.toString() + " to " + target.toString() +
             " via junction " + viaJunction.toString() + " over section " +
             viaSection.toString() + " network distance " + dist.toString();
    else
      return "undefined";
  }

}



