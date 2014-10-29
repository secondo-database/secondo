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

package mmdb.data.attributes.util;

import java.util.Date;

import mmdb.data.attributes.date.AttributeInstant;
import mmdb.error.convert.ConversionException;
import sj.lang.ListExpr;

/**
 * This class is a container for temporal object types that are used in several
 * attribute types.
 *
 * @author Alexander Castor
 */
public class TemporalObjects {

	/**
	 * This class represents a period.
	 *
	 * @author Alexander Castor
	 */
	public static class Period implements Comparable<Period> {

		/**
		 * The start time.
		 */
		private AttributeInstant startTime;

		/**
		 * The end time.
		 */
		private AttributeInstant endTime;

		/**
		 * Is the period a left closed interval.
		 */
		private boolean leftClosed;

		/**
		 * Is the period a right closed interval.
		 */
		private boolean rightClosed;

		/**
		 * Converts a given nested list to a period instance.
		 * 
		 * @param list
		 *            the list to be converted
		 * @throws ConversionException
		 */
		public void fromList(ListExpr list) throws ConversionException {
			AttributeInstant startTimeInstant = new AttributeInstant();
			startTimeInstant.fromList(list.first());
			setStartTime(startTimeInstant);
			AttributeInstant endTimeInstant = new AttributeInstant();
			endTimeInstant.fromList(list.second());
			setEndTime(endTimeInstant);
			setLeftClosed(list.third().boolValue());
			setRightClosed(list.fourth().boolValue());
		}

		/**
		 * Converts the instant in nested list representation.
		 * 
		 * @return the converted list
		 */
		public ListExpr toList() {
			return ListExpr.fourElemList(getStartTime().toList(), getEndTime().toList(),
					ListExpr.boolAtom(isLeftClosed()), ListExpr.boolAtom(isRightClosed()));
		}

		/*
		 * (non-Javadoc)
		 * 
		 * @see java.lang.Comparable#compareTo(java.lang.Object)
		 */
		@Override
		public int compareTo(Period other) {
			Date otherStartTime = other.getStartTime().getDate();
			int compare = getStartTime().getDate().compareTo(otherStartTime);
			if (compare == 0) {
				if (isLeftClosed() && !other.isLeftClosed()) {
					return 1;
				}
				if (!isLeftClosed() && other.isLeftClosed()) {
					return -1;
				}
			}
			return compare;
		}

		/**
		 * Getter for startTime.
		 * 
		 * @return the startTime
		 */
		public AttributeInstant getStartTime() {
			return startTime;
		}

		/**
		 * Setter for startTime.
		 * 
		 * @param startTime
		 *            the startTime to set
		 */
		public void setStartTime(AttributeInstant startTime) {
			this.startTime = startTime;
		}

		/**
		 * Getter for endTime.
		 * 
		 * @return the endTime
		 */
		public AttributeInstant getEndTime() {
			return endTime;
		}

		/**
		 * Setter for endTime.
		 * 
		 * @param endTime
		 *            the endTime to set
		 */
		public void setEndTime(AttributeInstant endTime) {
			this.endTime = endTime;
		}

		/**
		 * Getter for leftClosed.
		 * 
		 * @return the leftClosed
		 */
		public boolean isLeftClosed() {
			return leftClosed;
		}

		/**
		 * Setter for leftClosed.
		 * 
		 * @param leftClosed
		 *            the leftClosed to set
		 */
		public void setLeftClosed(boolean leftClosed) {
			this.leftClosed = leftClosed;
		}

		/**
		 * Getter for rightClosed.
		 * 
		 * @return the rightClosed
		 */
		public boolean isRightClosed() {
			return rightClosed;
		}

		/**
		 * Setter for rightClosed.
		 * 
		 * @param rightClosed
		 *            the rightClosed to set
		 */
		public void setRightClosed(boolean rightClosed) {
			this.rightClosed = rightClosed;
		}

	}

}
