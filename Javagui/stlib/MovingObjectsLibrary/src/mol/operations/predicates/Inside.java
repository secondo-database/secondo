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
package mol.operations.predicates;

import java.util.List;

import mol.datatypes.spatial.Point;
import mol.datatypes.spatial.Region;
import mol.datatypes.spatial.util.Halfsegment;
import mol.datatypes.spatial.util.Segment;

/**
 * Class with Inside operations for non temporal types
 * 
 * @author Markus Fuessel
 */
public class Inside {

   /**
    * Verify if the passed 'Point' is inside the passed 'Region'.<br>
    * Get ALL halfsegments from the 'Region' and uses the plumbline algorithm
    * 
    * @param point
    * @param region
    * 
    * @return true if the 'Point' is inside the 'Region', false otherwise
    */
   public static boolean inside(final Point point, final Region region) {

      if (!point.isDefined() || !region.isDefined()) {
         return false;
      }

      int intersections = 0;

      Segment plumbline = new Segment(point, new Point(point.getXValue(), Double.MAX_VALUE));

      List<Halfsegment> halfsegments = region.getHalfsegments();

      halfsegments.sort(null);

      for (Halfsegment hs : halfsegments) {
         if (hs.isLeftDominating()) {

            if (hs.getLeftPoint().getXValue() > point.getXValue()) {
               break;

            } else if (hs.getRightPoint().getXValue() >= point.getXValue() && hs.intersect(plumbline)) {

               intersections++;
            }

         }
      }

      return (intersections % 2) == 1;
   }

   /**
    * Private constructor to prevent instances of this class
    */
   private Inside() {

   }

}
