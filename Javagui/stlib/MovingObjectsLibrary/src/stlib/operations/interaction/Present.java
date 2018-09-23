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

import stlib.interfaces.moving.MovingObjectIF;
import stlib.interfaces.time.TimeInstantIF;

/**
 * Class with Present operations on temporal types
 * 
 * @author Markus Fuessel
 */
public class Present {

   /**
    * Check if the passed Moving Object is defined at the passed instant
    * 
    * @param mobject
    *           - the Moving Object
    * @param instant
    *           - the instant
    * 
    * @return true, if the passed Moving Object is defined at the passed instant,
    *         false otherwise
    */
   public static boolean execute(final MovingObjectIF<?, ?> mobject, final TimeInstantIF instant) {
      return mobject.getPeriods().contains(instant);
   }

   /**
    * Private constructor to prevent instances of this class
    */
   private Present() {

   }
}
