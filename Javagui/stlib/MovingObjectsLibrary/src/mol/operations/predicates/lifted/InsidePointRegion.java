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

package mol.operations.predicates.lifted;

import java.util.ArrayList;
import java.util.List;

import mol.datatypes.GeneralType;
import mol.datatypes.features.Spatial;
import mol.datatypes.interval.Period;
import mol.datatypes.intime.Intime;
import mol.datatypes.moving.MovingBool;
import mol.datatypes.moving.MovingPoint;
import mol.datatypes.moving.MovingRegion;
import mol.datatypes.spatial.Point;
import mol.datatypes.spatial.Region;
import mol.datatypes.spatial.util.Rectangle;
import mol.datatypes.time.TimeInstant;
import mol.datatypes.unit.UnitObject;
import mol.datatypes.unit.spatial.UnitPoint;
import mol.datatypes.unit.spatial.UnitRegion;
import mol.datatypes.unit.spatial.util.MovableSegment;
import mol.operations.predicates.Inside;
import mol.util.CrossPointScalars;
import mol.util.GeneralHelper;
import mol.util.Vector2D;

/**
 * This class realizes the template method pattern of the abstract class
 * 'BaseAlgorithm' for the inside operation using 'Point' and 'Region'
 * objects<br>
 * 
 * @author Markus Fuessel
 */
public class InsidePointRegion extends BaseAlgorithm {

   /**
    * Constructor for operation 'Point' inside 'MovingRegion'
    * 
    * @param point
    *           - the 'Point'
    * @param mregion
    *           - the 'MovingRegion'
    */
   public InsidePointRegion(final Point point, final MovingRegion mregion) {

      setMObject1(new MovingPoint(point));
      setMObject2(mregion);

   }

   /**
    * Constructor for operation 'MovingPoint' inside 'Region'
    * 
    * @param mpoint
    *           - the 'MovingPoint'
    * @param region
    *           - the 'Region'
    */
   public InsidePointRegion(final MovingPoint mpoint, final Region region) {

      setMObject1(mpoint);
      setMObject2(new MovingRegion(region));

   }

   /**
    * Constructor for operation 'MovingPoint' inside 'MovingRegion'
    * 
    * @param mpoint
    *           - the 'MovingPoint'
    * @param mregion
    *           - the 'MovingRegion'
    */
   public InsidePointRegion(final MovingPoint mpoint, final MovingRegion mregion) {

      setMObject1(mpoint);
      setMObject2(mregion);

   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.operations.predicates.lifted.BaseAlgorithm#additionalChecksSuccessful(
    * )
    */
   @Override
   public boolean additionalChecksSuccessful() {

      Rectangle bBoxObject1 = ((Spatial) getMobject1()).getBoundingBox();
      Rectangle bBoxObject2 = ((Spatial) getMobject2()).getBoundingBox();

      return bBoxObject1.intersects(bBoxObject2);
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.operations.predicates.lifted.BaseAlgorithm#unitFunction(mol.datatypes.
    * unit.UnitObject, mol.datatypes.unit.UnitObject)
    */
   @Override
   public MovingBool getUnitResult(final UnitObject<? extends GeneralType> uobject1,
                                  final UnitObject<? extends GeneralType> uobject2) {
      // TODO testen

      UnitPoint upoint = (UnitPoint) uobject1;
      UnitRegion uregion = (UnitRegion) uobject2;

      MovingBool mbool = new MovingBool(0);

      Period period = upoint.getPeriod().intersection(uregion.getPeriod());

      UnitPoint newUPoint = upoint.atPeriod(period);
      UnitRegion newURegion = uregion.atPeriod(period);

      if (newUPoint.isDefined() && newURegion.isDefined()
            && newUPoint.getProjectionBoundingBox().intersects(newURegion.getProjectionBoundingBox())) {

         boolean pointCurrentlyInside = Inside.inside(newUPoint.getInitial(), newURegion.getInitial());

         TimeInstant lastInstant = period.getLowerBound();
         boolean lastLeftClosed = period.isLeftClosed();

         List<Intime<Point>> intersectionPoints = intersectionPoints(newUPoint, newURegion);

         for (Intime<Point> ipoint : intersectionPoints) {
            Period boolPeriod = new Period(lastInstant, ipoint.getInstant(), lastLeftClosed, false);

            mbool.add(boolPeriod, pointCurrentlyInside);

            lastInstant = ipoint.getInstant();
            lastLeftClosed = false;
            pointCurrentlyInside = !pointCurrentlyInside;
         }

         Period boolPeriod = new Period(lastInstant, period.getUpperBound(), lastLeftClosed, period.isRightClosed());

         mbool.add(boolPeriod, pointCurrentlyInside);
      }

      return mbool;
   }

   /**
    * Get the intersections as a list of {@code Intime<Point>}. <br>
    * <br>
    * Preconditions:<br>
    * - 'UnitPoint' and 'UnitRegion' have the same defined time period
    * 
    * @param upoint
    * @param uregion
    * @return a sorted list of {@code Intime<Point>}
    */
   public static List<Intime<Point>> intersectionPoints(final UnitPoint upoint, final UnitRegion uregion) {

      List<Intime<Point>> intersectionPoints = new ArrayList<>();

      for (MovableSegment msegment : uregion.getMovingSegments()) {
         if (upoint.getProjectionBoundingBox().intersects(msegment.getProjectionBoundingBox())) {
            Intime<Point> ipoint = intersectionPoint(upoint, msegment);

            if (ipoint.isDefined()) {
               intersectionPoints.add(ipoint);
            }
         }
      }

      intersectionPoints.sort(null);

      return intersectionPoints;
   }

   /**
    * Get the intersection as {@code Intime<Point>}. <br>
    * Due to the assumption of a linear movement of both objects, there should be
    * at most one point of intersection <br>
    * Preconditions:<br>
    * - 'UnitPoint' and 'MovableSegment' have the same defined time period<br>
    * - both objects move linearly
    * 
    * @param upoint
    * @param msegment
    *           - a 'MovableSegment'
    * @return a {@code Intime<Point>}
    */
   public static Intime<Point> intersectionPoint(final UnitPoint upoint, final MovableSegment msegment) {

      Period timePeriod = upoint.getPeriod();

      Point initialUPoint = upoint.getInitial();
      Point finalUPoint = upoint.getFinal();

      Point msInitialStartPoint = msegment.getInitialStartPoint();
      Point msInitialEndPoint = msegment.getInitialEndPoint();

      Point msFinalStartPoint = msegment.getFinalStartPoint();
      Point msFinalEndPoint = msegment.getFinalEndPoint();

      List<CrossPointScalars> timePositions = GeneralHelper.intersection(new Point(0, 0),
            msInitialStartPoint.minus(initialUPoint).toPoint(), msFinalStartPoint.minus(finalUPoint).toPoint(),
            msInitialEndPoint.minus(initialUPoint).toPoint(), msFinalEndPoint.minus(finalUPoint).toPoint());

      if (!timePositions.isEmpty()) {
         double t = timePositions.get(0).timeScalar;
         Vector2D upDelta = finalUPoint.minus(initialUPoint);

         Point crossPoint = initialUPoint.plus(upDelta.scale(t));

         long deltaUPeriod = timePeriod.getDurationInMilliseconds();
         TimeInstant initInstant = timePeriod.getLowerBound();

         return new Intime<>(initInstant.plusMillis((long) (deltaUPeriod * t)), crossPoint);
      }

      return new Intime<>();

   }

}
