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

import stlib.datatypes.intime.Intime;
import stlib.interfaces.GeneralTypeIF;
import stlib.interfaces.intime.IntimeIF;
import stlib.interfaces.moving.MovingObjectIF;
import stlib.interfaces.time.TimeInstantIF;

/**
 * Class with 'Final' operations on temporal spatial types
 * 
 * @author Markus Fuessel
 */
public class Final {

   /**
    * Get the last defined value at the last defined instant of the passed
    * 'MovingObject'
    * 
    * @param mobject
    *           - the 'MovingObject'
    * 
    * @return a 'IntimeIF' object
    */
   public static <T extends GeneralTypeIF> IntimeIF<T> execute(final MovingObjectIF<?, T> mobject) {
      IntimeIF<T> intime;

      if (mobject == null || !mobject.isDefined() || mobject.getNoUnits() == 0) {
         intime = new Intime<>();
      } else {
         int lastUnitPos = mobject.getNoUnits() - 1;

         T value = mobject.getUnit(lastUnitPos).getFinal();
         TimeInstantIF instant = mobject.getUnit(lastUnitPos).getPeriod().getUpperBound();

         intime = new Intime<>(instant, value);
      }

      return intime;
   }

   /**
    * Private constructor to prevent instances of this class
    */
   private Final() {

   }

}
