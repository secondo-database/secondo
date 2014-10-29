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

package mmdb.operator.condition;

import java.util.Collections;
import java.util.Date;
import java.util.List;

import mmdb.data.attributes.date.AttributeInstant;
import mmdb.data.attributes.range.AttributePeriods;
import mmdb.data.attributes.util.TemporalObjects.Period;
import mmdb.data.features.Movable;

/**
 * This class represents the 'PRESENT' operator.
 *
 * @author Alexander Castor
 */
public class OperatorPresent implements ConditionOperator {

	/**
	 * Checks if the moving object is present at the given time.
	 * 
	 * @param movingObject
	 *            the moving object to be checked
	 * @param time
	 *            the time given
	 * @return true if it is present, else false
	 */
	public static boolean operate(Movable movingObject, AttributeInstant instant) {
		List<Period> periods = movingObject.getPeriods();
		Period period = new Period();
		period.setStartTime(instant);
		int index = Math.abs(Collections.binarySearch(periods, period) + 1);
		if (index == 0 || periods.size() == 1) {
			return isInstantInPeriod(instant, periods.get(0));
		}
		if (index == periods.size()) {
			return isInstantInPeriod(instant, periods.get(index - 1));
		}
		return isInstantInPeriod(instant, periods.get(index))
				|| isInstantInPeriod(instant, periods.get(index - 1));
	}

	/**
	 * Checks if the moving object is present at the given period.
	 * 
	 * @param movingObject
	 *            the moving object to be checked
	 * @param time
	 *            the period given
	 * @return true if it is present, else false
	 */
	public static boolean operate(Movable movingObject, AttributePeriods period) {
		List<Period> movablePeriods = movingObject.getPeriods();
		List<Period> attributePeriods = period.getPeriods();
		int c1 = 0;
		int c2 = 0;
		while (c1 < movablePeriods.size() && c2 < attributePeriods.size()) {
			if (isPeriodInPeriod(movablePeriods.get(c1), attributePeriods.get(c2))) {
				return true;
			}
			if (isPeriodBeforePeriod(movablePeriods.get(c1), attributePeriods.get(c2))) {
				c1++;
			} else {
				c2++;
			}
		}
		return false;
	}

	/**
	 * Checks if an instant is inside a period.
	 * 
	 * @param instant
	 *            the instant which might be inside the period
	 * @param period
	 *            the period which might contain the instant
	 * @return true if the instant is in the period, else false
	 */
	private static boolean isInstantInPeriod(AttributeInstant instant, Period period) {
		Date timeToCheck = instant.getDate();
		Date startTime = period.getStartTime().getDate();
		Date endTime = period.getEndTime().getDate();
		if (timeToCheck.before(startTime)) {
			return false;
		}
		if (timeToCheck.after(endTime)) {
			return false;
		}
		if (!period.isLeftClosed() && timeToCheck.equals(startTime)) {
			return false;
		}
		if (!period.isRightClosed() && timeToCheck.equals(endTime)) {
			return false;
		}
		return true;
	}

	/**
	 * Checks if the start time of a period (first argument) is before the start
	 * time of another period (second argument).
	 * 
	 * @param firstPeriod
	 *            the first period
	 * @param secondPeriod
	 *            the second period
	 * @return true if the first period' start time is before the other's, else
	 *         false
	 */
	private static boolean isPeriodBeforePeriod(Period firstPeriod, Period secondPeriod) {
		Date startTimeFirstPeriod = firstPeriod.getStartTime().getDate();
		Date startTimeSecondPeriod = secondPeriod.getStartTime().getDate();
		if (startTimeFirstPeriod.after(startTimeSecondPeriod)) {
			return false;
		}
		if (startTimeFirstPeriod.equals(startTimeSecondPeriod) && !firstPeriod.isLeftClosed()
				&& secondPeriod.isLeftClosed()) {
			return false;
		}
		return true;
	}

	/**
	 * Checks if a period (first argument) is inside another period (second
	 * argument).
	 * 
	 * @param firstPeriod
	 *            the first period
	 * @param secondPeriod
	 *            the second period
	 * @return true if the period is inside the other period, else false
	 */
	private static boolean isPeriodInPeriod(Period firstPeriod, Period secondPeriod) {
		Date startTimeFirstPeriod = firstPeriod.getStartTime().getDate();
		Date endTimeFirstPeriod = firstPeriod.getEndTime().getDate();
		Date startTimeSecondPeriod = secondPeriod.getStartTime().getDate();
		Date endTimeSecondPeriod = secondPeriod.getEndTime().getDate();
		if (startTimeFirstPeriod.before(startTimeSecondPeriod)) {
			return false;
		}
		if (endTimeFirstPeriod.after(endTimeSecondPeriod)) {
			return false;
		}
		if (!secondPeriod.isLeftClosed() && firstPeriod.isLeftClosed()
				&& startTimeFirstPeriod.equals(startTimeSecondPeriod)) {
			return false;
		}
		if (!secondPeriod.isRightClosed() && firstPeriod.isRightClosed()
				&& endTimeFirstPeriod.equals(endTimeSecondPeriod)) {
			return false;
		}
		return true;
	}

}
