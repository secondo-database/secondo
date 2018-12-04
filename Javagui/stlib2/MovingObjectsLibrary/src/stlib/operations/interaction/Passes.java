//This file is part of SECONDO.

//Copyright (C) 2014, University in Hagen, Department of Computer Science,
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
package stlib.operations.interaction;

import stlib.datatypes.base.BaseBool;
import stlib.interfaces.base.BaseBoolIF;
import stlib.interfaces.moving.MovingBoolIF;
import stlib.interfaces.moving.MovingPointIF;
import stlib.interfaces.spatial.RegionIF;
import stlib.operations.predicates.lifted.InsidePointRegion;

/**
 * Class with Passes operations on temporal types
 * 
 * @author Markus Fuessel
 */
public class Passes {

   /**
    * Check if the MovingBoolIF passes the passed BaseBoolIF value
    * 
    * @param mbool
    *           - the Moving Boolean
    * @param bbool
    *           - the Boolean value
    * 
    * @return true, if the Moving Boolean passes the boolean value, false otherwise
    */
   public static boolean execute(MovingBoolIF mbool, BaseBoolIF bbool) {
      if (mbool == null || bbool == null) {
         return false;
      }

      if (!mbool.isDefined() || !bbool.isDefined()) {
         return false;
      }

      return mbool.getMinValue().equals(bbool) || mbool.getMaxValue().equals(bbool);
   }

   /**
    * Check if the passed MovingPointIF passes the passed RegionIF
    * 
    * @param mpoint
    *           - the Moving Point
    * @param region
    *           - the Region
    * 
    * @return true, if the MovingPointIF passes the passed RegionIF, false
    *         otherwise
    */
   public static boolean execute(MovingPointIF mpoint, RegionIF region) {
      MovingBoolIF mbool = InsidePointRegion.execute(mpoint, region);

      return execute(mbool, new BaseBool(true));
   }

   /**
    * Private constructor to prevent instances of this class
    */
   private Passes() {

   }
}
