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

package mol.datatypes.range;

import mol.datatypes.interval.Period;
import mol.datatypes.time.TimeInstant;

/**
 * Concrete class for representation of 'Periods' objects. That are 'Range'
 * objects with disjoint and non adjacent 'TimeInstant' intervals.
 * 
 * @author Markus Fuessel
 */
public class Periods extends Range<TimeInstant> {

   /**
    * Simple constructor to create an empty 'Periods' object with the specified
    * initial capacity
    * 
    * @param size
    *           - initial capacity of this 'Periods' object
    */
   public Periods(final int size) {
      super(size);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.range.Range#first()
    */
   @Override
   public Period first() {
      return (Period) super.first();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.range.Range#last()
    */
   @Override
   public Period last() {
      return (Period) super.last();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.rangeset.RangeSet#getInterval(int)
    */
   @Override
   public Period get(final int index) {

      return (Period) super.get(index);

   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.range.Range#getUndefinedValue()
    */
   @Override
   protected TimeInstant getUndefinedObject() {

      return new TimeInstant();
   }

}
