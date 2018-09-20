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

package stlib.interfaces.moving;

import java.util.List;

import stlib.interfaces.GeneralTypeIF;
import stlib.interfaces.intime.IntimeIF;
import stlib.interfaces.range.PeriodsIF;
import stlib.interfaces.time.TimeInstantIF;
import stlib.interfaces.unit.UnitObjectIF;

/**
 * Interface for all 'MovingObject' classes<br>
 * 
 * @author Markus Fuessel
 *
 * @param <T1>
 *           - specifies the type of the 'UnitObjectIF' for this 'MovingObject'
 * @param <T2>
 *           - specifies the base type of the 'UnitObjectIF' and the
 *           'MovingObject'<br>
 *           <br>
 *           both types have to be compatible
 */
public interface MovingObjectIF<T1 extends UnitObjectIF<T2>, T2 extends GeneralTypeIF> extends GeneralTypeIF {

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
   boolean add(T1 newUnit);

   /**
    * Get the number of units
    * 
    * @return number of units
    */
   int getNoUnits();

   /**
    * Get the 'Periods' range object, for which this MovingObject is defined.
    * 
    * @return periods, a range object
    */
   PeriodsIF getPeriods();

   /**
    * 
    * @return
    */
   List<T1> getUnits();

   /**
    * Check if the MovingObject exists at the given instant
    * 
    * @param instant
    * 
    * @return true if MovingObject exists, false otherwise
    */
   boolean present(TimeInstantIF instant);

   /**
    * Get a {@code IntimeIF<T2>} object, consists of the passed instant and the
    * value of this 'MovingObject', which is defined at the passed instant
    * 
    * @param instant
    * 
    * @return {@code IntimeIF<T2>} object or an undefined, empty
    *         {@code IntimeIF<T2>} object if this 'MovingObject' is not defined at
    *         the passed instant
    */
   IntimeIF<T2> atInstant(TimeInstantIF instant);

   /**
    * Get the value of this 'MovingObject', a object of type {@code T2}, which is
    * defined at the passed instant.
    * 
    * @param instant
    * 
    * @return a object of type {@code T2}, if this 'MovingObject' is defined at the
    *         passed, otherwise the returned object will be undefined
    */
   T2 getValue(TimeInstantIF instant);

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
   T1 getUnit(TimeInstantIF instant);

   /**
    * Returns the unit at the specified position in this 'MovingObject'.
    * 
    * @param position
    * 
    * @return the unit at the specified position in this 'MovingObject' or an
    *         undefined, empty unit if the passed position does not exists
    */
   T1 getUnit(int position);

   /**
    * Get the position of the unit in the units array which contains the given
    * instant.
    * 
    * @param instant
    * 
    * @return position of the unit in the array, -1 otherwise
    */
   int getUnitPosition(TimeInstantIF instant);

   /**
    * Check if this 'MovingObject' is closed, the period of the last unit is right
    * closed.
    * 
    * @return true if period of last unit is right closed, false otherwise
    */
   boolean isClosed();

}