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

package stlib.operations.projection;

import stlib.datatypes.spatial.Line;
import stlib.interfaces.moving.MovingPointIF;
import stlib.interfaces.spatial.LineIF;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.unit.spatial.UnitPointIF;

/**
 * Class with Trajectory operations on temporal spatial types
 * 
 * @author Markus Fuessel
 */
public class Trajectory {

   /**
    * Computes the the projection of this 'MovingPointIF' object as a 'LineIF'
    * object.
    * 
    * @param mpoint
    * 
    * @return a 'LineIF' object
    */
   public static LineIF execute(final MovingPointIF mpoint) {
      Line line = new Line(true);
      int size = mpoint.getNoUnits();

      for (int i = 0; i < size; i++) {

         UnitPointIF up = mpoint.getUnit(i);

         PointIF p0 = up.getInitial();
         PointIF p1 = up.getFinal();

         if (!p0.almostEqual(p1)) {
            line.add(p0, p1);
         }

      }

      return line;
   }

   /**
    * Private constructor to prevent instances of this class
    */
   private Trajectory() {

   }

}
