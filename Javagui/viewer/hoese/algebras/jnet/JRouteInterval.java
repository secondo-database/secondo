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
 * JRouteInterval
 * Describes a part of an route by the route identifier, start pos, end pos and
 * side of the route.
 */
public class JRouteInterval{

   private Integer rid;
   private Double spos;
   private Double epos;
   private JDirection dir;

  public JRouteInterval(ListExpr value)
  {
    if (value.listLength() == 4){
      if(value.first().atomType()==ListExpr.INT_ATOM)
        rid = value.first().intValue();
      else
        rid = -1;
      if (value.second().atomType()==ListExpr.REAL_ATOM)
        spos = value.second().realValue();
      else
        spos = -1.0;
      if (value.third().atomType()==ListExpr.REAL_ATOM)
        epos = value.third().realValue();
      else
        epos = -1.0;
      dir = new JDirection(value.fourth());
    }
    else
    {
      rid = -1;
      spos = -1.0;
      epos = -1.0;
    }
  }

  public String toString(){
    if (rid > -1 && spos > -1.0 && epos > -1.0)
      return "rid " + rid.toString() + " from " + spos.toString() +
             " to " + epos.toString() + " side " + dir.toString();
    else
      return "undefined";
  }

  public boolean contains(RouteLocation rloc, double tolerance){
    return (rid.compareTo(rloc.getRid()) == 0 &&
            (spos <= rloc.getPos() ||
                almostEqual(spos, rloc.getPos(), tolerance)) &&
            (rloc.getPos() <= epos ||
                almostEqual(epos, rloc.getPos(), tolerance)) &&
            dir.compareTo(rloc.getDir(), false) == 0);
  }

  public boolean contains(JRouteInterval rint, double tolerance){
    return (rid.compareTo(rint.getRid()) == 0 &&
            (spos <= rint.getStartPos() ||
                almostEqual(spos, rint.getStartPos(), tolerance)) &&
            (rint.getEndPos() <= epos ||
                almostEqual(epos, rint.getEndPos(), tolerance)) &&
            dir.compareTo(rint.getDir().toString(), false) == 0);
  }

  public boolean completelyInside(JRouteInterval rint, double tolerance){
    return (rid.compareTo(rint.getRid()) == 0 &&
            (rint.getStartPos() <= spos ||
              almostEqual(spos , rint.getStartPos(), tolerance)) &&
            (epos <= rint.getEndPos() ||
              almostEqual(epos, rint.getEndPos(), tolerance)) &&
            dir.compareTo(rint.getDir().toString(), false) == 0);
  }

  public Integer getRid(){
    return rid;
  }

  public Double getStartPos(){
    return spos;
  }

  public Double getEndPos(){
    return epos;
  }

  public JDirection getDir(){
    return dir;
  }

  public double getLength(){
    return Math.abs(epos - spos);
  }

  public RouteLocation getStartRLoc(){
    return new RouteLocation(rid, spos, dir);
  }

  public RouteLocation getEndRLoc(){
    return new RouteLocation(rid, epos, dir);
  }

  private static boolean almostEqual(double a, double b){
    return almostEqual(a,b, 0.00000001);
  }

  private static boolean almostEqual(double a, double b, double tolerance){
    return Math.abs(a-b) < tolerance;
  }


}



