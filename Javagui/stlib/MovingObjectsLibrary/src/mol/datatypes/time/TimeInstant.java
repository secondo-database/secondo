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
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import java.util.Objects;
import java.util.TimeZone;

import mol.datatypes.GeneralType;
import mol.datatypes.features.Orderable;

/**
 * Class for representation of the 'Instant' data type.<br>
 * Internally the 'java.time.Instant' class is used to store the date time
 * value.
 * <p>
 * Stores date time values down to one nanosecond.<br>
 * Objects of this class are immutable.
 * 
 * @author Markus Fuessel
 */
public class TimeInstant extends GeneralType implements Orderable<TimeInstant> {

   /**
    * The minimum supported {@code TimeInstant}, '-1000000000-01-01T00:00Z'.<br>
    * <br>
    * 
    * @see java.time.Instant.MIN
    */
   public static final TimeInstant MIN = new TimeInstant(Instant.MIN);

   /**
    * The maximum supported {@code TimeInstant},
    * '1000000000-12-31T23:59:59.999999999Z'.<br>
    * <br>
    * 
    * @see java.time.Instant.MAX
    */
   public static final TimeInstant MAX = new TimeInstant(Instant.MAX);

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
      defaultTimeZoneId = TimeZone.getDefault().toZoneId();
   }

   /**
    * The Instant value<br>
    * Uses java.time.Instant to store the date time value
    * 
    * @see java.time.Instant
    */
   private final Instant instant;

   /**
    * Setter for the default date time format
    * 
    * @param format
    *           - DateTimeFormatter to use as default
    * 
    * @see java.time.format.DateTimeFormatter
    */
   public static void setDefaultDateTimeFormat(final DateTimeFormatter format) {
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
   public static void setDefaultTimeZoneId(final ZoneId zoneId) {
      defaultTimeZoneId = zoneId;
   }

   /**
    * Simple constructor, creates an undefined 'TimeInstant' object
    */
   public TimeInstant() {
      this.instant = Instant.EPOCH;
   }

   /**
    * Constructor, create an defined 'TimeInstant' object from an Instant value
    * 
    * @param instant
    *           - the instant value
    */
   public TimeInstant(final Instant instant) {
      Objects.requireNonNull(instant, "'instant' must not be null");

      this.instant = instant;
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
   public TimeInstant(final String dateTime, final DateTimeFormatter dateTimeFormat, final ZoneId timeZoneId) {
      Objects.requireNonNull(dateTime, "'dateTime' must not be null");
      Objects.requireNonNull(dateTimeFormat, "'dateTimeFormat' must not be null");
      Objects.requireNonNull(timeZoneId, "'timeZoneId' must not be null");

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
    * Constructor, create an defined 'TimeInstant' object <br>
    * Uses the defaul time zone id to parse the passed date time string with the
    * passed date time format
    * 
    * @param dateTime
    * @param dateTimeFormat
    */
   public TimeInstant(final String dateTime, final String dateTimeFormat) {
      this(dateTime, DateTimeFormatter.ofPattern(dateTimeFormat), defaultTimeZoneId);
   }

   /**
    * Copy constructor
    * 
    * @param original
    *           - the 'TimeInstant' object to copy
    */
   public TimeInstant(final TimeInstant original) {

      this.instant = original.getValue();
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
   public TimeInstant plusMillis(final long millisToAdd) {
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
   public TimeInstant plusNanos(final long nanosToAdd) {
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
   public TimeInstant minusMillis(final long millisToSubtract) {
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
   public TimeInstant minusNanos(final long nanosToSubtract) {
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
   public int compareTo(final TimeInstant otherTimeInstant) {

      return instant.compareTo(otherTimeInstant.getValue());
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
      if (!(obj instanceof TimeInstant)) {
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
   public boolean before(final TimeInstant otherTimeInstant) {

      return (this.compareTo(otherTimeInstant) < 0);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.util.Orderable#after(java.lang.Object)
    */
   @Override
   public boolean after(final TimeInstant otherTimeInstant) {

      return (this.compareTo(otherTimeInstant) > 0);
   }

   /**
    * If other 'TimeInstant' object is adjacent to this.<br>
    * Always returns false because time is considered as a continuous value and
    * there is always another instant between two instants.
    * 
    * @see mol.datatypes.util.Orderable#adjacent(java.lang.Object)
    */
   @Override
   public boolean adjacent(final TimeInstant otherTimeInstant) {

      return false;
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

   /**
    * Converts this TimeInstant to the number of milliseconds from the epoch of
    * 1970-01-01T00:00:00Z.
    * 
    * @return the number of milliseconds since the epoch of 1970-01-01T00:00:00Z
    */
   public long toMilliseconds() {
      return instant.toEpochMilli();
   }

}
