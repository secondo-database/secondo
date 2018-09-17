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

package mol.datatypes.moving;

import java.util.ArrayList;
import java.util.List;

import mol.datatypes.GeneralType;
import mol.datatypes.interval.Period;
import mol.datatypes.intime.Intime;
import mol.datatypes.range.Periods;
import mol.datatypes.time.TimeInstant;
import mol.datatypes.unit.UnitObject;

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
public abstract class MovingObject<T1 extends UnitObject<T2>, T2 extends GeneralType> extends GeneralType {

   /**
    * Periods, 'Range' object over the full set of all periods for which this
    * 'MovingObject' object is defined
    */
   private final Periods periods;

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

   /**
    * Add/append a new unit to this 'MovingObject' object.<br>
    * Only appending is possible, if: <br>
    * - the period of the passed unit is right adjacent with the period of the last
    * unit, or<br>
    * - the period of the passed unit is greater as the period of the last unit
    * <br>
    * <br>
    * If the value of the last unit of this 'MovingObject' is equal to the value of
    * the passed unit and if their periods are adjacent or intersects right, then
    * the period of the last unit will be extended
    * 
    * @param newUnit
    *           - the unit to add/append
    * 
    * @return true if the passed unit was added, false otherwise
    */
   public boolean add(T1 newUnit) {

      if (!this.isDefined() || !newUnit.isDefined()) {
         return false;
      }

      int noUnits = getNoUnits();

      Period newPeriod = newUnit.getPeriod();

      if (noUnits == 0) {

         units.add(newUnit);
         periods.add(newPeriod);

         return true;

      } else {

         T1 lastUnit = getUnit(noUnits - 1);
         Period lastPeriod = lastUnit.getPeriod();

         if ((lastPeriod.rightAdjacent(newPeriod) || lastPeriod.intersectsRight(newPeriod))
               && lastUnit.equalValue(newUnit)) {

            Period newLastPeriod = lastPeriod.merge(newPeriod);
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

   /**
    * Get the number of units
    * 
    * @return number of units
    */
   public int getNoUnits() {
      return units.size();
   }

   /**
    * Get the 'Periods' range object, for which this MovingObject is defined.
    * 
    * @return periods, a range object
    */
   public Periods getPeriods() {
      return periods;
   }

   /**
    * 
    * @return
    */
   public List<T1> getUnits() {
      return units;
   }

   /**
    * Check if the MovingObject exists at the given instant
    * 
    * @param instant
    * 
    * @return true if MovingObject exists, false otherwise
    */
   public boolean present(final TimeInstant instant) {
      return periods.contains(instant);
   }

   /**
    * Get a {@code Intime<T2>} object, consists of the passed instant and the value
    * of this 'MovingObject', which is defined at the passed instant
    * 
    * @param instant
    * 
    * @return {@code Intime<T2>} object or an undefined, empty {@code Intime<T2>}
    *         object if this 'MovingObject' is not defined at the passed instant
    */
   public Intime<T2> atInstant(TimeInstant instant) {

      Intime<T2> intime;

      if (!isDefined() || !instant.isDefined()) {
         intime = new Intime<>();
      } else {
         T2 value = getValue(instant);

         intime = new Intime<>(instant, value);
      }

      return intime;
   }

   /**
    * Get the value of this 'MovingObject', a object of type {@code T2}, which is
    * defined at the passed instant.
    * 
    * @param instant
    * 
    * @return a object of type {@code T2}, if this 'MovingObject' is defined at the
    *         passed, otherwise the returned object will be undefined
    */
   public T2 getValue(final TimeInstant instant) {

      T1 unit = getUnit(instant);

      return unit.getValue(instant);
   }

   /**
    * Get the 'UnitObject' which covers the passed instant of time in this
    * 'MovingObject'
    * 
    * @param instant
    * 
    * @return the 'UnitObject' which covers the passed instant of time or an
    *         undefined, empty 'UnitObject' if the passed instant is not covered by
    *         any 'UnitObject'
    */
   public T1 getUnit(final TimeInstant instant) {

      int unitPos = getUnitPosition(instant);

      return getUnit(unitPos);
   }

   /**
    * Returns the unit at the specified position in this 'MovingObject'.
    * 
    * @param position
    * 
    * @return the unit at the specified position in this 'MovingObject' or an
    *         undefined, empty unit if the passed position does not exists
    */
   public T1 getUnit(final int position) {
      T1 unit;

      if (position >= 0 && position < units.size()) {
         unit = units.get(position);
      } else {
         unit = getUndefinedUnitObject();
      }

      return unit;
   }

   /**
    * Get the position of the unit in the units array which contains the given
    * instant.
    * 
    * @param instant
    * 
    * @return position of the unit in the array, -1 otherwise
    */
   public int getUnitPosition(final TimeInstant instant) {

      if (!isDefined() || !instant.isDefined()) {
         return -1;
      }

      T1 centerUnit;
      Period centerPeriod;
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

   /**
    * Check if this 'MovingObject' is closed, the period of the last unit is right
    * closed.
    * 
    * @return true if period of last unit is right closed, false otherwise
    */
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
