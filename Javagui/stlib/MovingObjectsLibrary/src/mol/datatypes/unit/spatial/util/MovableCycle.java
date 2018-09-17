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
package mol.datatypes.unit.spatial.util;

import java.util.ArrayList;
import java.util.List;

import mol.datatypes.interval.Period;
import mol.datatypes.spatial.util.Cycle;
import mol.datatypes.spatial.util.Rectangle;
import mol.datatypes.spatial.util.Segment;
import mol.datatypes.time.TimeInstant;

/**
 * This class represents 'MovableCycle' objects, which are generally used to
 * build 'UnitRegion' objects
 * 
 * @author Markus Fuessel
 */
public class MovableCycle {

   /**
    * List of 'MovableSegment' of this MovableCycle
    */
   private final List<MovableSegment> movingSegments;

   /**
    * Constructor for a 'MovableCycle'
    * 
    * @param movingSegments
    */
   public MovableCycle(final List<MovableSegment> movingSegments) {
      this.movingSegments = new ArrayList<>(movingSegments);
   }

   /**
    * This method returns a 'Cycle' which is valid at the passed time instant
    * within the period of the movement of this 'MovableCycle'.<br>
    * If the 'Cycle' degenerates at the passed time instant, the returned 'Cycle'
    * will be undefined.
    * 
    * @param movementPeriod
    *           - the period in which the movement of this cycle is defined
    * @param instant
    *           - time instant
    * 
    * @return a defined 'Cycle' which is valid at the passed instant, otherwise the
    *         returned 'Cycle' is undefined
    */
   public Cycle getValue(final Period movementPeriod, final TimeInstant instant) {
      List<Segment> segments = new ArrayList<>();

      for (MovableSegment movableSegment : movingSegments) {
         Segment currentSegment = movableSegment.getValue(movementPeriod, instant);
         if (!currentSegment.isAlmostAPoint()) {
            segments.add(currentSegment);
         }
      }

      Cycle cycle = new Cycle(false);

      cycle.setCycleBySegmentList(segments);

      return cycle;
   }

   /**
    * Get the initial 'Cycle' object at the beginning of the movement
    * 
    * @return the initial 'Cycle' object
    */
   public Cycle getInitial() {

      List<Segment> segments = new ArrayList<>();

      for (MovableSegment movableSegment : movingSegments) {
         Segment currentSegment = movableSegment.getInitial();
         if (!currentSegment.isAlmostAPoint()) {
            segments.add(currentSegment);
         }
      }

      Cycle cycle = new Cycle(false);

      cycle.setCycleBySegmentList(segments);

      return cycle;
   }

   /**
    * Get the final 'Cycle' object at the end of the movement
    * 
    * @return the final 'Cycle' object
    */
   public Cycle getFinal() {
      List<Segment> segments = new ArrayList<>();

      for (MovableSegment movableSegment : movingSegments) {
         Segment currentSegment = movableSegment.getFinal();
         if (!currentSegment.isAlmostAPoint()) {
            segments.add(currentSegment);
         }
      }

      Cycle cycle = new Cycle(false);

      cycle.setCycleBySegmentList(segments);

      return cycle;
   }

   /**
    * Get all 'MovableSegment' objects of this object
    * 
    * @return list of 'MovableSegment' objects
    */
   public List<MovableSegment> getMovingSegments() {
      return new ArrayList<>(movingSegments);
   }

   /**
    * Get the projection bounding box of this 'MovableCycle' object
    * 
    * @return a 'Rectangle' object
    */
   public Rectangle getProjectionBoundingBox() {

      Rectangle initialBB = getInitial().getBoundingBox();
      Rectangle finalBB = getFinal().getBoundingBox();

      return initialBB.merge(finalBB);
   }

}
