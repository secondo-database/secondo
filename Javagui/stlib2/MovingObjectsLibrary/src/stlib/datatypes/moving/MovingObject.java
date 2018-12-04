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

package stlib.datatypes.moving;

import java.util.ArrayList;
import java.util.List;

import stlib.datatypes.GeneralType;
import stlib.datatypes.range.Periods;
import stlib.interfaces.GeneralTypeIF;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.moving.MovingObjectIF;
import stlib.interfaces.range.PeriodsIF;
import stlib.interfaces.time.TimeInstantIF;
import stlib.interfaces.unit.UnitObjectIF;

/**
 * Abstract Superclass for all 'MovingObject' subclasses<br>
 * Realizes the mapping of the unit objects to a single object, which represents
 * the real moving object.
 * 
 * @author Markus Fuessel
 *
 * @param <T1>
 *           - specifies the type of the 'UnitObject' in this 'MovingObject'
 * @param <T2>
 *           - specifies the base type of the 'UnitObject' and the
 *           'MovingObject'<br>
 *           <br>
 *           both types have to be compatible
 */
public abstract class MovingObject<T1 extends UnitObjectIF<T2>, T2 extends GeneralTypeIF> extends GeneralType
      implements MovingObjectIF<T1, T2> {

   /**
    * Periods, 'Range' object over the full set of all periods for which this
    * 'MovingObject' object is defined
    */
   private final PeriodsIF periods;

   /**
    * Array of the mapped 'UnitObject' objects
    */
   private final List<T1> units;

   /**
    * Base constructor<br>
    * Initializes the size of the periods and units array by the passed value
    * 
    * @param size
    *           - initial size for the periods and units array
    */
   protected MovingObject(final int size) {
      this.periods = new Periods(size);
      this.units = new ArrayList<>(size);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingObjectIF#add(T1)
    */
   @Override
   public boolean add(T1 newUnit) {

      if (!this.isDefined() || !newUnit.isDefined()) {
         return false;
      }

      int noUnits = getNoUnits();

      PeriodIF newPeriod = newUnit.getPeriod();

      if (noUnits == 0) {

         units.add(newUnit);
         periods.add(newPeriod);

         return true;

      } else {

         T1 lastUnit = getUnit(noUnits - 1);
         PeriodIF lastPeriod = lastUnit.getPeriod();

         if ((lastPeriod.rightAdjacent(newPeriod) || lastPeriod.intersectsRight(newPeriod))
               && lastUnit.equalValue(newUnit)) {

            PeriodIF newLastPeriod = lastPeriod.merge(newPeriod);
            lastUnit.setPeriod(newLastPeriod);

            periods.mergeAdd(newLastPeriod);

            return true;

         } else if (lastPeriod.intersectsOnRightBound(newPeriod) && lastUnit.finalEqualToInitialValue(newUnit)) {

            lastPeriod.setRightClosed(false);

            lastUnit.setPeriod(lastPeriod);

            units.add(newUnit);
            periods.mergeAdd(newPeriod);

            return true;

         } else if (lastPeriod.before(newPeriod)) {

            units.add(newUnit);
            periods.mergeAdd(newPeriod);

            return true;

         } else {
            return false;
         }
      }

   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingObjectIF#getNoUnits()
    */
   @Override
   public int getNoUnits() {
      return units.size();
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingObjectIF#getPeriods()
    */
   @Override
   public PeriodsIF getPeriods() {
      return periods;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingObjectIF#getUnits()
    */
   @Override
   public List<T1> getUnits() {
      return units;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingObjectIF#getValue(stlib.interfaces.time.
    * TimeInstant)
    */
   @Override
   public T2 getValue(final TimeInstantIF instant) {

      T1 unit = getUnit(instant);

      return unit.getValue(instant);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingObjectIF#getUnit(stlib.interfaces.time.
    * TimeInstant)
    */
   @Override
   public T1 getUnit(final TimeInstantIF instant) {

      int unitPos = getUnitPosition(instant);

      return getUnit(unitPos);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingObjectIF#getUnit(int)
    */
   @Override
   public T1 getUnit(final int position) {
      T1 unit;

      if (position >= 0 && position < units.size()) {
         unit = units.get(position);
      } else {
         unit = getUndefinedUnitObject();
      }

      return unit;
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.moving.MovingObjectIF#getUnitPosition(stlib.interfaces.time.
    * TimeInstant)
    */
   @Override
   public int getUnitPosition(final TimeInstantIF instant) {

      if (!isDefined() || !instant.isDefined()) {
         return -1;
      }

      T1 centerUnit;
      PeriodIF centerPeriod;
      int centerPos;

      int firstPos = 0;
      int lastPos = this.units.size() - 1;

      while (firstPos <= lastPos) {

         centerPos = (firstPos + lastPos) / 2;

         centerUnit = this.units.get(centerPos);
         centerPeriod = centerUnit.getPeriod();

         if (centerPeriod.contains(instant)) {
            return centerPos;

         } else if (centerPeriod.after(instant)) {
            lastPos = centerPos - 1;

         } else if (centerPeriod.before(instant)) {
            firstPos = centerPos + 1;
         }

      }

      return -1;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingObjectIF#isClosed()
    */
   @Override
   public boolean isClosed() {

      if (getNoUnits() > 0) {
         return getUnit(getNoUnits() - 1).getPeriod().isRightClosed();
      }

      return false;
   }

   /**
    * Returns an undefined unit object of type {@code <T1>}. <br>
    * Must be implemented by the specific subclass.
    * 
    * @return undefined unit object of type {@code <T1>}
    */
   protected abstract T1 getUndefinedUnitObject();

   /**
    * Returns an undefined object of type {@code <T2>}. <br>
    * Must be implemented by the specific subclass.
    * 
    * @return undefined object of type {@code <T2>}
    */
   protected abstract T2 getUndefinedObject();

}
