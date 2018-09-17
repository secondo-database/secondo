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

package mol.datatypes.unit.spatial;

import java.util.Objects;

import mol.datatypes.interval.Period;
import mol.datatypes.spatial.Point;
import mol.datatypes.spatial.util.Rectangle;
import mol.datatypes.time.TimeInstant;
import mol.datatypes.unit.UnitObject;

/**
 * This class represents 'UnitPointConst' objects and is used for 'MovingPoint'
 * objects with a constant location over a certain period of time.
 * 
 * @author Markus Fuessel
 */
public class UnitPointConst extends UnitPoint {

   /**
    * Constant value for a constant 'UnitPoint' object
    */
   private final Point constPoint;

   /**
    * Constructor for an undefined 'UnitPointConst' object
    */
   public UnitPointConst() {
      this.constPoint = new Point();
   }

   /**
    * Constructor for an 'UnitPointConst' object with a maximum Period
    */
   public UnitPointConst(Point point) {
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
   public UnitPointConst(final Period period, final Point point) {

      super(period);
      this.constPoint = Objects.requireNonNull(point, "'point' must not be null");
      setDefined(period.isDefined() && point.isDefined());

   }

   /**
    * Get the projection bounding box for this constant unit point.
    */
   @Override
   public Rectangle getProjectionBoundingBox() {

      return constPoint.getBoundingBox();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.UnitObject#getValue(java.time.Instant)
    */
   @Override
   public Point getValue(final TimeInstant instant) {

      Period period = getPeriod();

      if (isDefined() && period.contains(instant)) {
         return getConstPoint();
      } else {
         return new Point();
      }

   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.UnitObject#atPeriod(mol.datatypes.interval.Period)
    */
   @Override
   public UnitPointConst atPeriod(Period period) {
      Period newPeriod = this.getPeriod().intersection(period);

      if (!newPeriod.isDefined()) {
         return new UnitPointConst();
      }

      return new UnitPointConst(newPeriod, getConstPoint());
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.UnitObject#equalValue(mol.datatypes.unit.UnitObject)
    */
   @Override
   public boolean equalValue(UnitObject<Point> otherUnitObject) {

      if (!(otherUnitObject instanceof UnitPointConst)) {
         return false;
      }

      UnitPointConst otherUnitPointConst = (UnitPointConst) otherUnitObject;

      return constPoint.equals(otherUnitPointConst.getConstPoint());
   }

   /**
    * Getter for the constant value object
    * 
    * @return the value
    */
   protected Point getConstPoint() {
      return constPoint;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.spatial.UnitPoint#getInitial()
    */
   @Override
   public Point getInitial() {
      return getConstPoint();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.spatial.UnitPoint#getFinal()
    */
   @Override
   public Point getFinal() {
      return getConstPoint();
   }
}
