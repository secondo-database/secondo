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
import mol.datatypes.spatial.util.Segment;
import mol.interfaces.interval.PeriodIF;
import mol.interfaces.spatial.PointIF;
import mol.interfaces.spatial.util.RectangleIF;
import mol.interfaces.spatial.util.SegmentIF;
import mol.interfaces.time.TimeInstantIF;

/**
 * This class represents 'MovableSegment' objects, which are generally used to
 * build 'UnitRegion' objects
 * 
 * @author Markus Fuessel
 *
 */
public class MovableSegment implements MovableSegmentIF {

   /**
    * The start point of this segment at the beginning of the movement
    */
   private final PointIF initialStartPoint;

   /**
    * The end point of this segment at the beginning of the movement
    */
   private final PointIF initialEndPoint;

   /**
    * The start point of this segment at the end of the movement
    */
   private final PointIF finalStartPoint;

   /**
    * The end point of this segment at the end of the movement
    */
   private final PointIF finalEndPoint;

   /**
    * Constructor for a 'MovableSegment'
    * 
    * @param initialStartPoint
    * @param initialEndPoint
    * @param finalStartPoint
    * @param finalEndPoint
    */
   public MovableSegment(final PointIF initialStartPoint, final PointIF initialEndPoint, final PointIF finalStartPoint,
                         final PointIF finalEndPoint) {

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
   public MovableSegment(final SegmentIF segment) {
      this(segment.getLeftPoint(), segment.getRightPoint(), segment.getLeftPoint(), segment.getRightPoint());
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.datatypes.unit.spatial.util.MovableSegmentIF#getValue(mol.interfaces.
    * interval.PeriodIF, mol.interfaces.time.TimeInstantIF)
    */
   @Override
   public SegmentIF getValue(final PeriodIF movementPeriod, final TimeInstantIF instant) {

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

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.spatial.util.MovableSegmentIF#getInitial()
    */
   @Override
   public SegmentIF getInitial() {
      return new Segment(initialStartPoint, initialEndPoint);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.spatial.util.MovableSegmentIF#getFinal()
    */
   @Override
   public SegmentIF getFinal() {
      return new Segment(finalStartPoint, finalEndPoint);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.spatial.util.MovableSegmentIF#getInitialStartPoint()
    */
   @Override
   public PointIF getInitialStartPoint() {
      return initialStartPoint;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.spatial.util.MovableSegmentIF#getFinalStartPoint()
    */
   @Override
   public PointIF getFinalStartPoint() {
      return finalStartPoint;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.spatial.util.MovableSegmentIF#getInitialEndPoint()
    */
   @Override
   public PointIF getInitialEndPoint() {
      return initialEndPoint;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.spatial.util.MovableSegmentIF#getFinalEndPoint()
    */
   @Override
   public PointIF getFinalEndPoint() {
      return finalEndPoint;
   }

   /**
    * Getter for the projection bounding box of this 'MovableSegmentIF' object
    * <p>
    * Combines the bounding box of the initial segment with the bounding box of the
    * final segment
    */
   @Override
   public RectangleIF getProjectionBoundingBox() {

      RectangleIF initialSegmentBB = getInitial().getBoundingBox();
      RectangleIF finalSegmentBB = getFinal().getBoundingBox();

      return initialSegmentBB.merge(finalSegmentBB);
   }

}
