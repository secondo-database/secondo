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

package  viewer.hoese.algebras;

import  java.awt.geom.*;
import  java.awt.*;
import  viewer.*;
import viewer.hoese.*;
import  sj.lang.ListExpr;
import  java.util.*;
import tools.Reporter;

/**
 * A displayclass for the uregion-type (spatiotemp algebra), 2D with TimePanel
 */
public class Dspluregion extends Dsplmovingregion implements DsplSimple{
  /**
   * Scans the representation of a movingregion datatype
   * @param v A list of start and end intervals with regionmap value
   * @see sj.lang.ListExpr
   */
  public void ScanValue (ListExpr value) {
    if(isUndefined(value)){
      defined=false;
      err=false;
      return;
    }
    defined=true;
    err = true;
    int length = value.listLength();
    RegionMaps = new Vector(length+1);
    Intervals = new Vector(length+1);
    ListExpr unit = value;
    int L = unit.listLength();
    if (L != 5 & L!=2)
        return;

      Interval in = null;
      RegionMap rm = null;

      if(L==5){
         Reporter.writeWarning("Warning: use a deprecated version of"+
                               " external representation of a moving region!");
         in = LEUtils.readInterval(ListExpr.fourElemList(unit.first(),
                                   unit.second(), unit.third(), unit.fourth()));
         rm = readRegionMap(unit.fifth());
      }
      if(L==2){
          in = LEUtils.readInterval(unit.first());
          rm = readRegionMap(unit.second());
      }

      if ((in == null) || (rm == null))
        return;
      Intervals.add(in);
      RegionMaps.add(rm);
    err = false;
  }

}



