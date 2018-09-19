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

import mol.datatypes.interval.Period;
import mol.datatypes.intime.Intime;
import mol.datatypes.moving.MovingBool;
import mol.datatypes.moving.MovingPoint;
import mol.datatypes.moving.MovingRegion;
import mol.datatypes.spatial.Point;
import mol.datatypes.unit.spatial.util.MovableSegmentIF;
import mol.interfaces.GeneralTypeIF;
import mol.interfaces.features.MovableSpatial;
import mol.interfaces.interval.PeriodIF;
import mol.interfaces.intime.IntimeIF;
import mol.interfaces.moving.MovingBoolIF;
import mol.interfaces.moving.MovingPointIF;
import mol.interfaces.moving.MovingRegionIF;
import mol.interfaces.spatial.PointIF;
import mol.interfaces.spatial.RegionIF;
import mol.interfaces.spatial.util.RectangleIF;
import mol.interfaces.time.TimeInstantIF;
import mol.interfaces.unit.UnitObjectIF;
import mol.interfaces.unit.spatial.UnitPointIF;
import mol.interfaces.unit.spatial.UnitRegionIF;
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
   public InsidePointRegion(final PointIF point, final MovingRegionIF mregion) {

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
   public InsidePointRegion(final MovingPointIF mpoint, final RegionIF region) {

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
   public InsidePointRegion(final MovingPointIF mpoint, final MovingRegionIF mregion) {

      setMObject1(mpoint);
      setMObject2(mregion);

   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.operations.predicates.lifted.BaseAlgorithm#additionalChecksSuccessful( )
    */
   @Override
   public boolean additionalChecksSuccessful() {

      RectangleIF bBoxObject1 = ((MovableSpatial) getMobject1()).getProjectionBoundingBox();
      RectangleIF bBoxObject2 = ((MovableSpatial) getMobject2()).getProjectionBoundingBox();

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
   public MovingBoolIF getUnitResult(final UnitObjectIF<? extends GeneralTypeIF> uobject1,
                                     final UnitObjectIF<? extends GeneralTypeIF> uobject2) {

      UnitPointIF upoint = (UnitPointIF) uobject1;
      UnitRegionIF uregion = (UnitRegionIF) uobject2;

      MovingBoolIF mbool = new MovingBool(0);

      PeriodIF period = upoint.getPeriod().intersection(uregion.getPeriod());

      UnitPointIF newUPoint = upoint.atPeriod(period);
      UnitRegionIF newURegion = uregion.atPeriod(period);

      if (newUPoint.isDefined() && newURegion.isDefined()
            && newUPoint.getProjectionBoundingBox().intersects(newURegion.getProjectionBoundingBox())) {

         boolean pointCurrentlyInside = Inside.inside(newUPoint.getInitial(), newURegion.getInitial());

         TimeInstantIF lastInstant = period.getLowerBound();
         boolean lastLeftClosed = period.isLeftClosed();

         List<Intime<PointIF>> intersectionPoints = intersectionPoints(newUPoint, newURegion);

         for (IntimeIF<PointIF> ipoint : intersectionPoints) {
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
   public static List<Intime<PointIF>> intersectionPoints(final UnitPointIF upoint, final UnitRegionIF uregion) {

      List<Intime<PointIF>> intersectionPoints = new ArrayList<>();

      for (MovableSegmentIF msegment : uregion.getMovingSegments()) {
         if (upoint.getProjectionBoundingBox().intersects(msegment.getProjectionBoundingBox())) {
            Intime<PointIF> ipoint = intersectionPoint(upoint, msegment);

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
   public static Intime<PointIF> intersectionPoint(final UnitPointIF upoint, final MovableSegmentIF msegment) {

      PeriodIF timePeriod = upoint.getPeriod();

      PointIF initialUPoint = upoint.getInitial();
      PointIF finalUPoint = upoint.getFinal();

      PointIF msInitialStartPoint = msegment.getInitialStartPoint();
      PointIF msInitialEndPoint = msegment.getInitialEndPoint();

      PointIF msFinalStartPoint = msegment.getFinalStartPoint();
      PointIF msFinalEndPoint = msegment.getFinalEndPoint();

      List<CrossPointScalars> timePositions = GeneralHelper.intersection(new Point(0, 0),
            msInitialStartPoint.minus(initialUPoint).toPoint(), msFinalStartPoint.minus(finalUPoint).toPoint(),
            msInitialEndPoint.minus(initialUPoint).toPoint(), msFinalEndPoint.minus(finalUPoint).toPoint());

      if (!timePositions.isEmpty()) {
         double t = timePositions.get(0).timeScalar;
         Vector2D upDelta = finalUPoint.minus(initialUPoint);

         PointIF crossPoint = initialUPoint.plus(upDelta.scale(t));

         long deltaUPeriod = timePeriod.getDurationInMilliseconds();
         TimeInstantIF initInstant = timePeriod.getLowerBound();

         return new Intime<>(initInstant.plusMillis((long) (deltaUPeriod * t)), crossPoint);
      }

      return new Intime<>();

   }

}
