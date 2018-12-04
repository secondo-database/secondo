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
package stlib.datatypes.unit.spatial.util;

import java.util.ArrayList;
import java.util.List;

import stlib.datatypes.spatial.util.Cycle;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.spatial.util.CycleIF;
import stlib.interfaces.spatial.util.RectangleIF;
import stlib.interfaces.spatial.util.SegmentIF;
import stlib.interfaces.time.TimeInstantIF;
import stlib.interfaces.unit.spatial.util.MovableCycleIF;
import stlib.interfaces.unit.spatial.util.MovableSegmentIF;

/**
 * This class represents 'MovableCycle' objects, which are generally used to
 * build 'UnitRegion' objects
 * 
 * @author Markus Fuessel
 */
public class MovableCycle implements MovableCycleIF {

   /**
    * List of 'MovableSegmentIF' of this MovableCycle
    */
   private final List<MovableSegmentIF> movingSegments;

   /**
    * Constructor for a 'MovableCycle'
    * 
    * @param movingSegments
    */
   public MovableCycle(final List<MovableSegmentIF> movingSegments) {
      this.movingSegments = new ArrayList<>(movingSegments);
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.unit.spatial.util.MovableCycleIF#getValue(stlib.interfaces.
    * interval.PeriodIF, stlib.interfaces.time.TimeInstantIF)
    */
   @Override
   public CycleIF getValue(final PeriodIF movementPeriod, final TimeInstantIF instant) {
      List<SegmentIF> segments = new ArrayList<>();

      for (MovableSegmentIF movableSegment : movingSegments) {
         SegmentIF currentSegment = movableSegment.getValue(movementPeriod, instant);
         if (!currentSegment.isAlmostAPoint()) {
            segments.add(currentSegment);
         }
      }

      CycleIF cycle = new Cycle(false);

      cycle.setCycleBySegmentList(segments);

      return cycle;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.spatial.util.MovableCycleIF#getInitial()
    */
   @Override
   public CycleIF getInitial() {

      List<SegmentIF> segments = new ArrayList<>();

      for (MovableSegmentIF movableSegment : movingSegments) {
         SegmentIF currentSegment = movableSegment.getInitial();
         if (!currentSegment.isAlmostAPoint()) {
            segments.add(currentSegment);
         }
      }

      CycleIF cycle = new Cycle(false);

      cycle.setCycleBySegmentList(segments);

      return cycle;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.spatial.util.MovableCycleIF#getFinal()
    */
   @Override
   public CycleIF getFinal() {
      List<SegmentIF> segments = new ArrayList<>();

      for (MovableSegmentIF movableSegment : movingSegments) {
         SegmentIF currentSegment = movableSegment.getFinal();
         if (!currentSegment.isAlmostAPoint()) {
            segments.add(currentSegment);
         }
      }

      CycleIF cycle = new Cycle(false);

      cycle.setCycleBySegmentList(segments);

      return cycle;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.spatial.util.MovableCycleIF#getMovingSegments()
    */
   @Override
   public List<MovableSegmentIF> getMovingSegments() {
      return new ArrayList<>(movingSegments);
   }

   /**
    * Get the projection bounding box of this 'MovableCycle' object
    * 
    * @return a 'Rectangle' object
    */
   public RectangleIF getProjectionBoundingBox() {

      RectangleIF initialBB = getInitial().getBoundingBox();
      RectangleIF finalBB = getFinal().getBoundingBox();

      return initialBB.merge(finalBB);
   }

}
