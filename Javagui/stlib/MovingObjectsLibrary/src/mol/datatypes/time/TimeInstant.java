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

package mol.datatypes.time;

import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.time.ZoneOffset;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;

import mol.datatypes.GeneralType;
import mol.datatypes.features.Orderable;

/**
 * Class for representation of the 'instant' data type<br>
 * 
 * @author Markus Fuessel
 */
public class TimeInstant extends GeneralType implements Orderable<TimeInstant> {

   /**
    * Static attribute to store date time format which is used for all instant
    * values by default
    */
   private static DateTimeFormatter defaultDateTimeFormat;

   /**
    * Static attribute to store the time zone id which is used for all instant
    * values by default
    */
   private static ZoneId defaultTimeZoneId;

   /**
    * Initialize the static attributes
    */
   static {
      defaultDateTimeFormat = DateTimeFormatter.ISO_LOCAL_DATE_TIME;
      defaultTimeZoneId = ZoneOffset.UTC;
   }

   /**
    * Setter for the default date time format
    * 
    * @param format
    *           - DateTimeFormatter to use as default
    * 
    * @see java.time.format.DateTimeFormatter
    */
   public static void setDefaultDateTimeFormat(DateTimeFormatter format) {
      defaultDateTimeFormat = format;
   }

   /**
    * Setter for the default zone id
    * 
    * @param zoneId
    *           - ZoneId to use as default
    * 
    * @see java.time.ZoneId
    */
   public static void setDefaultTimeZoneId(ZoneId zoneId) {
      defaultTimeZoneId = zoneId;
   }

   /**
    * Checks if two instants where adjacent
    * <p>
    * Two instants are considered to be adjacent if they differ in one nanosecond
    * 
    * @param instant1
    * @param instant2
    * 
    * @return true - instants are adjacent, false - otherwise
    */
   public static boolean adjacent(Instant instant1, Instant instant2) {

      if (!instant1.equals(instant2)
            && (instant1.plusNanos(1).equals(instant2) || instant2.plusNanos(1).equals(instant1))) {
         return true;
      }

      return false;
   }

   /**
    * Checks if two TimeInstant where adjacent
    * 
    * @param tInstant1
    * @param tInstant2
    * 
    * @return true - TimeInstant are adjacent, false - otherwise
    */
   public static boolean adjacent(TimeInstant tInstant1, TimeInstant tInstant2) {

      return adjacent(tInstant1.getValue(), tInstant2.getValue());
   }

   /**
    * The Instant value<br>
    * Uses java.time.Instant to store the date time value
    * 
    * @see java.time.Instant
    */
   private final Instant instant;

   /**
    * Simple constructor, creates an undefined 'TimeInstant' object
    */
   public TimeInstant() {
      this.instant = Instant.MIN;
      setDefined(false);
   }

   /**
    * Constructor, create an defined 'TimeInstant' object from an Instant value
    * 
    * @param instant
    *           - the instant value
    */
   public TimeInstant(final Instant instant) {
      this.instant = Instant.parse(instant.toString());
      setDefined(true);

   }

   /**
    * Constructor, create an defined 'TimeInstant' object
    * 
    * @param dateTime
    *           - date time string
    * @param dateTimeFormat
    *           - DateTimeFormatter to use to parse the date time string
    * @param timeZoneId
    *           - ZoneId to use for the passed date time string
    */
   public TimeInstant(final String dateTime, final DateTimeFormatter dateTimeFormat, ZoneId timeZoneId) {

      LocalDateTime localDateTime = LocalDateTime.parse(dateTime, dateTimeFormat);
      ZonedDateTime zonedDateTime = localDateTime.atZone(timeZoneId);
      this.instant = zonedDateTime.toInstant();

      setDefined(true);

   }

   /**
    * Constructor, create an defined 'TimeInstant' object <br>
    * Uses the default date time format and time zone id to parse the passed date
    * time string
    * 
    * @param dateTime
    *           - date time string
    */
   public TimeInstant(final String dateTime) {
      this(dateTime, defaultDateTimeFormat, defaultTimeZoneId);
   }

   /**
    * Copy constructor
    * 
    * @param original
    *           - the 'TimeInstant' object to copy
    */
   public TimeInstant(final TimeInstant original) {
      Instant originalInstant = original.getValue();

      this.instant = Instant.parse(originalInstant.toString());
      setDefined(original.isDefined());
   }

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
   public TimeInstant plusMillis(long millisToAdd) {
      Instant newInstant = instant.plusMillis(millisToAdd);
      TimeInstant newTimeInstant = new TimeInstant(newInstant);
      newTimeInstant.setDefined(this.isDefined());

      return newTimeInstant;
   }

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
   public TimeInstant plusNanos(long nanosToAdd) {
      Instant newInstant = instant.plusNanos(nanosToAdd);
      TimeInstant newTimeInstant = new TimeInstant(newInstant);
      newTimeInstant.setDefined(this.isDefined());

      return newTimeInstant;
   }

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
   public TimeInstant minusMillis(long millisToSubtract) {
      Instant newInstant = instant.minusMillis(millisToSubtract);
      TimeInstant newTimeInstant = new TimeInstant(newInstant);
      newTimeInstant.setDefined(this.isDefined());

      return newTimeInstant;
   }

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
   public TimeInstant minusNanos(long nanosToSubtract) {
      Instant newInstant = instant.minusNanos(nanosToSubtract);
      TimeInstant newTimeInstant = new TimeInstant(newInstant);
      newTimeInstant.setDefined(this.isDefined());

      return newTimeInstant;
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Comparable#compareTo(java.lang.Object)
    */
   @Override
   public int compareTo(final TimeInstant t) {

      return instant.compareTo(t.getValue());
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#hashCode()
    */
   @Override
   public int hashCode() {
      return instant.hashCode();
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#equals(java.lang.Object)
    */
   @Override
   public boolean equals(final Object obj) {
      if (obj == null || !(obj instanceof TimeInstant)) {
         return false;
      }

      if (this == obj) {
         return true;
      }

      TimeInstant otherInstant = (TimeInstant) obj;

      return compareTo(otherInstant) == 0;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.util.Orderable#before(java.lang.Object)
    */
   @Override
   public boolean before(TimeInstant otherTimeInstant) {

      return (this.compareTo(otherTimeInstant) < 0);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.util.Orderable#after(java.lang.Object)
    */
   @Override
   public boolean after(TimeInstant otherTimeInstant) {

      return (this.compareTo(otherTimeInstant) > 0);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.util.Orderable#adjacent(java.lang.Object)
    */
   @Override
   public boolean adjacent(TimeInstant other) {

      return TimeInstant.adjacent(this, other);
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#toString()
    */
   @Override
   public String toString() {
      return "TimeInstant [instant=" + instant.atZone(defaultTimeZoneId) + ", isDefined()=" + isDefined() + "]";
   }

   /**
    * Getter for the Instant value
    * 
    * @return the value
    */
   public Instant getValue() {
      return instant;
   }

}
