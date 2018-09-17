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

import java.time.temporal.ChronoUnit;

import mol.datatypes.time.TimeInstant;

/**
 * Concrete class to represent 'Period' objects, an 'Interval' object with
 * 'TimeInstant' objects as lower and upper bound.
 * 
 * @author Markus Fuessel
 *
 */
public class Period extends Interval<TimeInstant> {

   /**
    * A period with maximum range, from '-1000000000-01-01T00:00Z' to
    * '1000000000-12-31T23:59:59.999999999Z'
    */
   public static final Period MAX = new Period(TimeInstant.MIN, TimeInstant.MAX, true, true);

   /**
    * Calculate the amount of milliseconds between two passed 'TimeInstant' objects
    * 
    * @return amount of milliseconds
    */
   public static long getDurationInMilliseconds(final TimeInstant startTimeInstant, final TimeInstant endTimeInstant) {

      return ChronoUnit.MILLIS.between(startTimeInstant.getValue(), endTimeInstant.getValue());
   }

   /**
    * Constructor for an undefined 'Period' object
    */
   public Period() {
   }

   /**
    * Constructor for an 'Period' object.
    * 
    * @param lowerBound
    *           TimeInstant
    * @param upperBound
    *           TimeInstant
    * @param leftClosed
    *           boolean
    * @param rightClosed
    *           boolean
    */
   public Period(final TimeInstant lowerBound, final TimeInstant upperBound, final boolean leftClosed,
                 final boolean rightClosed) {
      super(lowerBound, upperBound, leftClosed, rightClosed);
   }

   /**
    * Constructor for an 'Period' object. An 'Interval' object with 'TimeInstant'
    * objects as lower and upper bound. <br>
    * Here the lower and upper bound have to be passed as a date time string. To
    * parse the passed date time string the default date time format and time zone
    * id of the 'TimeInstant' class is used
    * 
    * @param dateTimeBegin
    *           String
    * @param dateTimeEnd
    *           String
    * @param leftClosed
    *           boolean
    * @param rightClosed
    *           boolean
    */
   public Period(final String dateTimeBegin, final String dateTimeEnd, final boolean leftClosed,
                 final boolean rightClosed) {
      this(new TimeInstant(dateTimeBegin), new TimeInstant(dateTimeEnd), leftClosed, rightClosed);
   }

   /**
    * Copy constructor
    * 
    * @param original
    *           - the original 'Period' object to copy
    */
   private Period(final Period original) {
      this(original.getLowerBound(), original.getUpperBound(), original.isLeftClosed(), original.isRightClosed());
      setDefined(original.isDefined());
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.Interval#copy()
    */
   @Override
   public Period copy() {
      return new Period(this);
   }

   /**
    * Return the amount of milliseconds of this 'Period' object between lower bound
    * and upper bound
    * 
    * @return amount of milliseconds
    */
   public long getDurationInMilliseconds() {
      return getDurationInMilliseconds(getLowerBound(), getUpperBound());
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.Interval#merge(mol.datatypes.interval.Interval)
    */
   @Override
   public Period merge(Interval<TimeInstant> otherInterval) {
      return (Period) super.merge(otherInterval);
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.datatypes.interval.Interval#mergeLeft(mol.datatypes.interval.Interval)
    */
   @Override
   public Period mergeLeft(Interval<TimeInstant> otherInterval) {
      return (Period) super.mergeLeft(otherInterval);
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.datatypes.interval.Interval#mergeRight(mol.datatypes.interval.Interval)
    */
   @Override
   public Period mergeRight(Interval<TimeInstant> otherInterval) {
      return (Period) super.mergeRight(otherInterval);
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.datatypes.interval.Interval#intersection(mol.datatypes.interval.Interval)
    */
   @Override
   public Period intersection(Interval<TimeInstant> otherInterval) {
      return (Period) super.intersection(otherInterval);
   }

}
