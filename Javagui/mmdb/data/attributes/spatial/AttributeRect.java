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

package mmdb.data.attributes.spatial;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.features.Matchable;
import sj.lang.ListExpr;

/**
 * Object representation for database attributes of type 'rect'.
 *
 * @author Alexander Castor
 */
public class AttributeRect extends MemoryAttribute implements Matchable {

	/**
	 * The rectangle's left value
	 */
	private float leftValue;

	/**
	 * The rectangle's right value
	 */
	private float rightValue;

	/**
	 * The rectangle's bottom value
	 */
	private float bottomValue;

	/**
	 * The rectangle's top value
	 */
	private float topValue;

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#fromList(sj.lang.ListExpr)
	 */
	@Override
	public void fromList(ListExpr list) {
		setLeftValue((float) list.first().realValue());
		setRightValue((float) list.second().realValue());
		setBottomValue((float) list.third().realValue());
		setTopValue((float) list.fourth().realValue());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#toList()
	 */
	@Override
	public ListExpr toList() {
		return ListExpr.fourElemList(ListExpr.realAtom(getLeftValue()),
				ListExpr.realAtom(getRightValue()), ListExpr.realAtom(getBottomValue()),
				ListExpr.realAtom(getTopValue()));
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object obj) {
		if (obj == null) {
			return false;
		}
		AttributeRect other = (AttributeRect) obj;
		if (Float.floatToIntBits(bottomValue) != Float.floatToIntBits(other.bottomValue))
			return false;
		if (Float.floatToIntBits(leftValue) != Float.floatToIntBits(other.leftValue))
			return false;
		if (Float.floatToIntBits(rightValue) != Float.floatToIntBits(other.rightValue))
			return false;
		if (Float.floatToIntBits(topValue) != Float.floatToIntBits(other.topValue))
			return false;
		return true;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + Float.floatToIntBits(bottomValue);
		result = prime * result + Float.floatToIntBits(leftValue);
		result = prime * result + Float.floatToIntBits(rightValue);
		result = prime * result + Float.floatToIntBits(topValue);
		return result;
	}

	/**
	 * Getter for leftValue.
	 * 
	 * @return the leftValue
	 */
	public float getLeftValue() {
		return leftValue;
	}

	/**
	 * Setter for leftValue.
	 * 
	 * @param leftValue
	 *            the leftValue to set
	 */
	public void setLeftValue(float leftValue) {
		this.leftValue = leftValue;
	}

	/**
	 * Getter for rightValue.
	 * 
	 * @return the rightValue
	 */
	public float getRightValue() {
		return rightValue;
	}

	/**
	 * Setter for rightValue.
	 * 
	 * @param rightValue
	 *            the rightValue to set
	 */
	public void setRightValue(float rightValue) {
		this.rightValue = rightValue;
	}

	/**
	 * Getter for bottomValue.
	 * 
	 * @return the bottomValue
	 */
	public float getBottomValue() {
		return bottomValue;
	}

	/**
	 * Setter for bottomValue.
	 * 
	 * @param bottomValue
	 *            the bottomValue to set
	 */
	public void setBottomValue(float bottomValue) {
		this.bottomValue = bottomValue;
	}

	/**
	 * Getter for topValue.
	 * 
	 * @return the topValue
	 */
	public float getTopValue() {
		return topValue;
	}

	/**
	 * Setter for topValue.
	 * 
	 * @param topValue
	 *            the topValue to set
	 */
	public void setTopValue(float topValue) {
		this.topValue = topValue;
	}

}
