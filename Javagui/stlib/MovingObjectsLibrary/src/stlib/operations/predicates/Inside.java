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
package stlib.operations.predicates;

import java.util.List;

import stlib.datatypes.spatial.Point;
import stlib.datatypes.spatial.util.Segment;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.spatial.RegionIF;
import stlib.interfaces.spatial.util.HalfsegmentIF;
import stlib.interfaces.spatial.util.SegmentIF;

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
   public static boolean inside(final PointIF point, final RegionIF region) {

      if (!point.isDefined() || !region.isDefined()) {
         return false;
      }

      int intersections = 0;

      SegmentIF plumbline = new Segment(point, new Point(point.getXValue(), Double.MAX_VALUE));

      List<HalfsegmentIF> halfsegments = region.getHalfsegments();

      halfsegments.sort(null);

      for (HalfsegmentIF hs : halfsegments) {
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
