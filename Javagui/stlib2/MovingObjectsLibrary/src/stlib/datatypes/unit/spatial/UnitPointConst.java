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

package stlib.datatypes.unit.spatial;

import java.util.Objects;

import stlib.datatypes.interval.Period;
import stlib.datatypes.spatial.Point;
import stlib.datatypes.unit.UnitObject;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.spatial.util.RectangleIF;
import stlib.interfaces.time.TimeInstantIF;
import stlib.interfaces.unit.UnitObjectConstIF;
import stlib.interfaces.unit.UnitObjectIF;
import stlib.interfaces.unit.spatial.UnitPointIF;

/**
 * This class represents 'UnitPointConst' objects and is used for 'MovingPoint'
 * objects with a constant location over a certain period of time.
 * 
 * @author Markus Fuessel
 */
public class UnitPointConst extends UnitObject<PointIF> implements UnitPointIF, UnitObjectConstIF<PointIF> {

   /**
    * Constant value for a constant 'UnitPoint' object
    */
   private final PointIF constPoint;

   /**
    * Constructor for an undefined 'UnitPointConst' object
    */
   public UnitPointConst() {
      this.constPoint = new Point();
   }

   /**
    * Constructor for an 'UnitPointConst' object with a maximum Period
    */
   public UnitPointConst(PointIF point) {
      super(Period.MAX);
      this.constPoint = point;
      setDefined(point.isDefined());
   }

   /**
    * Constructor to create a UnitPoint object with a constant point value
    * 
    * @param period
    * @param point
    */
   public UnitPointConst(final PeriodIF period, final PointIF point) {

      super(period);
      this.constPoint = Objects.requireNonNull(point, "'point' must not be null");
      setDefined(period.isDefined() && point.isDefined());

   }

   /**
    * Get the projection bounding box for this constant unit point.
    */
   @Override
   public RectangleIF getProjectionBoundingBox() {

      return constPoint.getBoundingBox();
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#getValue(java.time.Instant)
    */
   @Override
   public PointIF getValue(final TimeInstantIF instant) {
      return getValue(instant, false);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#getValue(stlib.interfaces.time.
    * TimeInstantIF, boolean)
    */
   @Override
   public PointIF getValue(final TimeInstantIF instant, final boolean ignoreClosedFlags) {

      PeriodIF period = getPeriod();

      if (isDefined() && period.contains(instant, ignoreClosedFlags)) {
         return getConstPoint();
      } else {
         return new Point();
      }

   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#atPeriod(stlib.interfaces.interval.
    * PeriodIF)
    */
   @Override
   public UnitPointConst atPeriod(PeriodIF period) {
      PeriodIF newPeriod = this.getPeriod().intersection(period);

      if (!newPeriod.isDefined()) {
         return new UnitPointConst();
      }

      return new UnitPointConst(newPeriod, getConstPoint());
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#equalValue(stlib.interfaces.unit.
    * UnitObjectIF)
    */
   @Override
   public boolean equalValue(UnitObjectIF<PointIF> otherUnitObject) {

      if (!(otherUnitObject instanceof UnitObjectConstIF || !otherUnitObject.isDefined())) {
         return false;
      }

      UnitObjectConstIF<?> otherUnitPointConst = (UnitObjectConstIF<?>) otherUnitObject;

      return constPoint.equals(otherUnitPointConst.getValue());
   }

   /**
    * Getter for the constant value object
    * 
    * @return the value
    */
   protected PointIF getConstPoint() {
      return constPoint;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.spatial.UnitPointIF#getInitial()
    */
   @Override
   public PointIF getInitial() {
      return getConstPoint();
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.spatial.UnitPointIF#getFinal()
    */
   @Override
   public PointIF getFinal() {
      return getConstPoint();
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectConstIF#getValue()
    */
   @Override
   public PointIF getValue() {
      return constPoint;
   }

   @Override
   public boolean finalEqualToInitialValue(UnitObjectIF<PointIF> otherUnitObject) {
      return getFinal().almostEqual(otherUnitObject.getInitial());
   }
}
