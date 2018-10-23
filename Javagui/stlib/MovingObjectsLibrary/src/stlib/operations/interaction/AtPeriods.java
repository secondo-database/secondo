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

package stlib.operations.interaction;

import java.util.ArrayList;
import java.util.List;

import stlib.datatypes.moving.MovingBool;
import stlib.datatypes.moving.MovingPoint;
import stlib.interfaces.GeneralTypeIF;
import stlib.interfaces.base.BaseBoolIF;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.moving.MovingBoolIF;
import stlib.interfaces.moving.MovingObjectIF;
import stlib.interfaces.moving.MovingPointIF;
import stlib.interfaces.range.PeriodsIF;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.unit.UnitBoolIF;
import stlib.interfaces.unit.UnitObjectIF;
import stlib.interfaces.unit.spatial.UnitPointIF;

/**
 * Class with AtPeriods operations on temporal types
 * 
 * @author Markus Fuessel
 */
public class AtPeriods {

   // TODO implementieren, tests

   /**
    * Restrict the passed 'MovingBoolIF' object to the times of the passed
    * 'PeriodsIF' object, a set of time intervals.
    * 
    * @param mbool
    *           - the 'MovingBoolIF' object
    * 
    * @param periods
    *           - the 'PeriodsIF' object
    * 
    * @return a new restricted 'MovingBoolIF' object
    */
   public static MovingBoolIF execute(final MovingBoolIF mbool, final PeriodsIF periods) {
      MovingBool mboolResult;

      if (mbool == null || periods == null) {
         mboolResult = new MovingBool(0);
         mboolResult.setDefined(false);

         return mboolResult;
      }

      if (!mbool.isDefined() || !periods.isDefined()) {
         mboolResult = new MovingBool(0);
         mboolResult.setDefined(false);

         return mboolResult;
      }

      List<UnitObjectIF<BaseBoolIF>> mboolUnits = getRefinementUnits(mbool, periods);
      mboolResult = new MovingBool(mboolUnits.size());

      if (mboolUnits.isEmpty()) {
         mboolResult.setDefined(false);

      } else {

         for (UnitObjectIF<BaseBoolIF> mboolUnit : mboolUnits) {
            mboolResult.add((UnitBoolIF) mboolUnit);
         }
      }

      return mboolResult;
   }

   /**
    * Restrict the passed 'MovingPointIF' object to the times of the passed
    * 'PeriodsIF' object, a set of time intervals.
    * 
    * @param mbool
    *           - the 'MovingPointIF' object
    * 
    * @param periods
    *           - the 'PeriodsIF' object
    * 
    * @return a new restricted 'MovingPointIF' object
    */
   public static MovingPointIF execute(final MovingPointIF mpoint, final PeriodsIF periods) {

      MovingPoint mpointResult;

      if (mpoint == null || periods == null) {
         mpointResult = new MovingPoint(0);
         mpointResult.setDefined(false);

         return mpointResult;
      }

      if (!mpoint.isDefined() || !periods.isDefined()) {
         mpointResult = new MovingPoint(0);
         mpointResult.setDefined(false);

         return mpointResult;
      }

      List<UnitObjectIF<PointIF>> mpointUnits = getRefinementUnits(mpoint, periods);
      mpointResult = new MovingPoint(mpointUnits.size());

      if (mpointUnits.isEmpty()) {
         mpointResult.setDefined(false);

      } else {

         for (UnitObjectIF<PointIF> mpointUnit : mpointUnits) {
            mpointResult.add((UnitPointIF) mpointUnit);
         }
      }

      return mpointResult;

   }

   /**
    * Scans the units of the passed 'MovingObjectIF' object and the periods of the
    * passed 'PeriodsIF' object in parallel. Restricts the units where the period
    * of a unit intersects with a period of the periods object to the intersection
    * time period.
    * 
    * @param mobject
    *           - the 'MovingObjectIF' object
    * 
    * @param periods
    *           - the 'PeriodsIF' object
    * 
    * @return
    */
   private static <T1 extends UnitObjectIF<T2>, T2 extends GeneralTypeIF> List<UnitObjectIF<T2>> getRefinementUnits(final MovingObjectIF<T1, T2> mobject,
                                                                                                       final PeriodsIF periods) {

      int posUnit = 0;
      int posPeriod = 0;

      List<UnitObjectIF<T2>> units = new ArrayList<>();

      UnitObjectIF<T2> uobject = mobject.getUnit(posUnit);
      PeriodIF unitPeriod = uobject.getPeriod();
      PeriodIF period = periods.get(posPeriod);

      while (uobject.isDefined() && period.isDefined()) {

         if (unitPeriod.before(period)) {
            posUnit++;
            uobject = mobject.getUnit(posUnit);
            unitPeriod = uobject.getPeriod();

         } else if (period.before(unitPeriod)) {
            posPeriod++;
            period = periods.get(posPeriod);

         } else {
            UnitObjectIF<T2> newUObject = uobject.atPeriod(period);

            if (newUObject.isDefined()) {
               units.add(newUObject);
            }

            if (unitPeriod.getUpperBound().compareTo(period.getUpperBound()) < 0) {
               posUnit++;
               uobject = mobject.getUnit(posUnit);
               unitPeriod = uobject.getPeriod();

            } else if (period.getUpperBound().compareTo(unitPeriod.getUpperBound()) < 0) {
               posPeriod++;
               period = periods.get(posPeriod);

            } else {
               posUnit++;
               uobject = mobject.getUnit(posUnit);
               unitPeriod = uobject.getPeriod();

               posPeriod++;
               period = periods.get(posPeriod);
            }
         }

      }

      return units;
   }

   /**
    * Private constructor to prevent instances of this class
    */
   private AtPeriods() {

   }
}
