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

package mol.datatypes.interval;

import mol.datatypes.time.TimeInstant;

/**
 * Concrete class to represent period objects
 * 
 * @author Markus Fuessel
 *
 */
public class Period extends Interval<TimeInstant> {

   /**
    * 
    * @param dateTimeBegin
    * @param dateTimeEnd
    * @param leftClosed
    * @param rightClosed
    */
   public Period(final String dateTimeBegin, final String dateTimeEnd, final boolean leftClosed,
                 final boolean rightClosed) {
      this(new TimeInstant(dateTimeBegin), new TimeInstant(dateTimeEnd), leftClosed, rightClosed);
   }

   /**
    * Constructs an period object
    * 
    * @param lowerBound
    * @param upperBound
    * @param leftClosed
    * @param rightClosed
    */
   public Period(TimeInstant lowerBound, TimeInstant upperBound, boolean leftClosed, boolean rightClosed) {
      super(lowerBound, upperBound, leftClosed, rightClosed);
   }

   /**
    * Copy constructor
    * 
    * @param original
    *           - the original 'Period' object to copy
    */
   private Period(Period original) {
      this(original.getLowerBound(), original.getUpperBound(), original.isLeftClosed(), original.isRightClosed());
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.Interval#clone()
    */
   @Override
   public Period clone() {
      return new Period(this);
   }

}
