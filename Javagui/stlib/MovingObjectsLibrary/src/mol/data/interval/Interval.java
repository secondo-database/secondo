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

package mol.data.interval;

/**
 * Abstract Superclass for all interval subclasses
 * 
 * @author Markus Fuessel
 *
 * @param <T>
 */
public abstract class Interval<T extends Comparable<T>> {

	/**
	 * Lower bound of the interval
	 */
	private T lowerBound;

	/**
	 * Upper bound of the interval
	 */
	private T upperBound;

	/**
	 * Is the interval left closed
	 */
	private boolean leftClosed;

	/**
	 * Is the interval right closed
	 */
	private boolean rightClosed;

	/**
	 * Verifies if the interval contains the given value
	 * 
	 * @param value
	 * 
	 * @return true if the interval contains the given value, false otherwise
	 */
	public boolean contains(T value) {
		if (value == null) {
			return false;
		}

		if (value.compareTo(lowerBound) < 0 || value.compareTo(upperBound) > 0) {
			return false;
		}

		if ((!leftClosed && value.compareTo(lowerBound) <= 0) || (!rightClosed && value.compareTo(lowerBound) >= 0)) {
			return false;
		}

		return true;
	}

	/**
	 * Get lower bound of the interval
	 * 
	 * @return the lowerBound
	 */
	public T getLowerBound() {
		return lowerBound;
	}

	/**
	 * Set the lower bound of the interval
	 * 
	 * @param lowerBound
	 *            the lowerBound to set
	 */
	public void setLowerBound(T lowerBound) {
		this.lowerBound = lowerBound;
	}

	/**
	 * Get the upper bound of the interval
	 * 
	 * @return the upperBound
	 */
	public T getUpperBound() {
		return upperBound;
	}

	/**
	 * Set the upper bound of the interval
	 * 
	 * @param upperBound
	 *            the upperBound to set
	 */
	public void setUpperBound(T upperBound) {
		this.upperBound = upperBound;
	}

	/**
	 * Is interval left closed
	 * 
	 * @return the leftClosed
	 */
	public boolean isLeftClosed() {
		return leftClosed;
	}

	/**
	 * Sets the interval left closed
	 */
	public void setLeftClosed(boolean leftClosed) {
		this.leftClosed = leftClosed;
	}

	/**
	 * Is interval right closed
	 * 
	 * @return the rightClosed
	 */
	public boolean isRightClosed() {
		return rightClosed;
	}

	/**
	 * Sets the interval right closed
	 */
	public void setRightClosed(boolean rightClosed) {
		this.rightClosed = rightClosed;
	}

}
