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

package mol.datatypes.spatial.util;

import java.util.List;

import mol.datatypes.spatial.Line;
import mol.datatypes.spatial.Point;

/**
 * This utility class represents spatial objects of type 'Cycle'.<br>
 * 
 * @author Markus Fuessel
 */
public class Cycle extends Line {

   /**
    * Constructor for an empty 'Cycle' object.
    * 
    * @param defined
    *           - specifies whether the 'Line' object is to be set as defined
    */
   public Cycle(final boolean defined) {
      super(defined);
   }

   /**
    * Constructor for a 'Cycle' object.<br>
    * The points of the passed list are connected in the order of their occurrence
    * in the list, whereby the last point of the list is connected with the
    * first.<br>
    * To create a valid 'Cycle' object the passed list have to contain three points
    * at least.
    * 
    * @param points
    *           - List of points
    */
   public Cycle(final List<Point> points) {
      super(false);

      if (points.size() >= 3) {
         for (int i = 0; i < points.size() - 1; i++) {
            add(points.get(i), points.get(i + 1));
         }

         add(points.get(points.size() - 1), points.get(0));

         setDefined(true);
      }
   }

   /**
    * Set the 'Cycle' object by passing a list of 'Segment' objects.<br>
    * To create a valid 'Cycle' object the passed list have to contain three
    * segments at least. Already existing segments in this 'Cycle' will be
    * replaced.
    * 
    * @param segments
    *           - List of segments
    */
   public boolean setCycleBySegmentList(final List<Segment> segments) {

      if (segments.size() >= 3) {

         this.clear();

         for (Segment segment : segments) {
            this.add(segment);
         }

         setDefined(true);

         return true;
      }

      return false;

   }

}
