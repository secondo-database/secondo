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
package stlib.interfaces.time;

import java.time.Instant;

import stlib.interfaces.GeneralTypeIF;
import stlib.interfaces.features.Orderable;

/**
 * Interface that should be provided by objects of type TimeInstant
 * 
 * 
 * @author Markus Fuessel
 */
public interface TimeInstantIF extends GeneralTypeIF, Orderable<TimeInstantIF> {

   /**
    * Create a new 'TimeInstant' object with the amount of milliseconds added to
    * this 'TimeInstant' object
    * 
    * @param millisToAdd
    *           - the milliseconds to add, negativ or positive
    * @return new TimeInstant
    * 
    * @see java.time.Instant#plusMillis(long)
    */
   TimeInstantIF plusMillis(long millisToAdd);

   /**
    * Create a new 'TimeInstant' object with the amount of nanoseconds added to
    * this 'TimeInstant' object
    * 
    * @param nanosToAdd
    *           - the nanoseconds to add, negativ or positive
    * @return new TimeInstant
    * 
    * @see java.time.Instant#plusNanos(long)
    */
   TimeInstantIF plusNanos(long nanosToAdd);

   /**
    * Create a new 'TimeInstant' object with the amount of milliseconds subtracted
    * from this 'TimeInstant' object
    * 
    * @param millisToSubtract
    *           - the milliseconds to subtract, negativ or positive
    * @return new TimeInstant
    * 
    * @see java.time.Instant#minusMillis(long)
    */
   TimeInstantIF minusMillis(long millisToSubtract);

   /**
    * Create a new 'TimeInstant' object with the amount of nanoseconds subtracted
    * from this 'TimeInstant' object
    * 
    * @param nanosToSubtract
    *           - the nanoseconds to subtract, negativ or positive
    * @return new TimeInstant
    * 
    * @see java.time.Instant#minusNanos(long)
    */
   TimeInstantIF minusNanos(long nanosToSubtract);

   /**
    * Getter for the Instant value
    * 
    * @return the value
    */
   Instant getValue();

   /**
    * Converts this TimeInstant to the number of milliseconds from the epoch of
    * 1970-01-01T00:00:00Z.
    * 
    * @return the number of milliseconds since the epoch of 1970-01-01T00:00:00Z
    */
   long toMilliseconds();

}