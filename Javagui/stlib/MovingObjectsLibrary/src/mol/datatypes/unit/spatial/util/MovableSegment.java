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

import mol.datatypes.interval.Period;
import mol.datatypes.spatial.Point;
import mol.datatypes.spatial.util.Rectangle;
import mol.datatypes.spatial.util.Segment;
import mol.datatypes.time.TimeInstant;

/**
 * This class represents 'MovableSegment' objects, which are generally used to
 * build 'UnitRegion' objects
 * 
 * @author Markus Fuessel
 *
 */
public class MovableSegment {

   /**
    * The start point of this segment at the beginning of the movement
    */
   private final Point initialStartPoint;

   /**
    * The end point of this segment at the beginning of the movement
    */
   private final Point initialEndPoint;

   /**
    * The start point of this segment at the end of the movement
    */
   private final Point finalStartPoint;

   /**
    * The end point of this segment at the end of the movement
    */
   private final Point finalEndPoint;

   /**
    * Constructor for a 'MovableSegment'
    * 
    * @param initialStartPoint
    * @param initialEndPoint
    * @param finalStartPoint
    * @param finalEndPoint
    */
   public MovableSegment(final Point initialStartPoint, final Point initialEndPoint, final Point finalStartPoint,
                         final Point finalEndPoint) {

      this.initialStartPoint = initialStartPoint;
      this.initialEndPoint = initialEndPoint;

      this.finalStartPoint = finalStartPoint;
      this.finalEndPoint = finalEndPoint;

   }

   /**
    * Constructor for a "static" 'MovableSegment'
    * 
    * @param segment
    */
   public MovableSegment(final Segment segment) {
      this(segment.getLeftPoint(), segment.getRightPoint(), segment.getLeftPoint(), segment.getRightPoint());
   }

   /**
    * This method returns a 'Segment' which is valid at the passed time instant
    * within the period of the movement of this 'MovableSegment'
    * 
    * @param movementPeriod
    *           - the period in which the movement of this segment is defined
    * @param instant
    *           - time instant
    * 
    * @return a defined 'Segment' which is valid at the passed instant, otherwise
    *         the returned 'Segment' is undefined
    */
   public Segment getValue(final Period movementPeriod, final TimeInstant instant) {

      if (movementPeriod.contains(instant, true)) {
         if (instant.equals(movementPeriod.getLowerBound())) {
            return getInitial();

         } else if (instant.equals(movementPeriod.getUpperBound())) {
            return getFinal();

         } else {
            long durationMilli = movementPeriod.getDurationInMilliseconds();
            long durationCurrent = Period.getDurationInMilliseconds(movementPeriod.getLowerBound(), instant);

            double currentStartX = initialStartPoint.getXValue()
                  + (Point.getDeltaX(initialStartPoint, finalStartPoint) / durationMilli) * (durationCurrent);
            double currentStartY = initialStartPoint.getYValue()
                  + (Point.getDeltaY(initialStartPoint, finalStartPoint) / durationMilli) * (durationCurrent);

            double currentEndX = initialEndPoint.getXValue()
                  + (Point.getDeltaX(initialEndPoint, finalEndPoint) / durationMilli) * (durationCurrent);
            double currentEndY = initialEndPoint.getYValue()
                  + (Point.getDeltaY(initialEndPoint, finalEndPoint) / durationMilli) * (durationCurrent);

            Point currentStartPoint = new Point(currentStartX, currentStartY);
            Point currentEndPoint = new Point(currentEndX, currentEndY);

            return new Segment(currentStartPoint, currentEndPoint);
         }
      } else {
         return new Segment();
      }
   }

   /**
    * Get the initial 'Segment' object at the beginning of the movement
    * 
    * @return the initial 'Segment' object
    */
   public Segment getInitial() {
      return new Segment(initialStartPoint, initialEndPoint);
   }

   /**
    * Get the final 'Segment' object at the end of the movement
    * 
    * @return the final 'Segment' object
    */
   public Segment getFinal() {
      return new Segment(finalStartPoint, finalEndPoint);
   }

   /**
    * Get the start point at the beginning of the movement
    * 
    * @return the initial start point, a 'Point' object
    */
   public Point getInitialStartPoint() {
      return initialStartPoint;
   }

   /**
    * Get the start point at the end of the movement
    * 
    * @return the final start point, a 'Point' object
    */
   public Point getFinalStartPoint() {
      return finalStartPoint;
   }

   /**
    * Get the end point at the beginning of the movement
    * 
    * @return the initial end point, a 'Point' object
    */
   public Point getInitialEndPoint() {
      return initialEndPoint;
   }

   /**
    * Get the end point at the end of the movement
    * 
    * @return the final end point, a 'Point' object
    */
   public Point getFinalEndPoint() {
      return finalEndPoint;
   }

   /**
    * Getter for the projection bounding box of this 'MovableSegment' object
    * <p>
    * Combines the bounding box of the initial segment with the bounding box of the
    * final segment
    */
   public Rectangle getProjectionBoundingBox() {

      Rectangle initialSegmentBB = getInitial().getBoundingBox();
      Rectangle finalSegmentBB = getFinal().getBoundingBox();

      return initialSegmentBB.merge(finalSegmentBB);
   }

}
