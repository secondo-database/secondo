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

package mmdb.data.attributes.range;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.convert.ConversionException;
import sj.lang.ListExpr;

/**
 * Superclass for all range attributes.
 *
 * @author Alexander Castor
 */
public abstract class RangeAttribute<T extends MemoryAttribute> extends MemoryAttribute {

	/**
	 * The lower bound of the interval.
	 */
	private T lowerBound;

	/**
	 * The upper bound of the interval.
	 */
	private T upperBound;

	/**
	 * Is the period a left closed interval.
	 */
	private boolean leftClosed;

	/**
	 * Is the period a right closed interval.
	 */
	private boolean rightClosed;

	/**
	 * Injects a value object.
	 */
	protected abstract T getRangeObject();

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#fromList(sj.lang.ListExpr)
	 */
	@Override
	public void fromList(ListExpr list) throws ConversionException {
		T lowerBound = getRangeObject();
		lowerBound.fromList(list.first());
		setLowerBound(lowerBound);
		T upperBound = getRangeObject();
		upperBound.fromList(list.second());
		setUpperBound(upperBound);
		setLeftClosed(list.third().boolValue());
		setRightClosed(list.fourth().boolValue());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#toList()
	 */
	@Override
	public ListExpr toList() throws ConversionException {
		return ListExpr.fourElemList(getLowerBound().toList(), getUpperBound().toList(),
				ListExpr.boolAtom(isLeftClosed()), ListExpr.boolAtom(isRightClosed()));
	}

	/**
	 * Getter for lowerBound.
	 * 
	 * @return the lowerBound
	 */
	public T getLowerBound() {
		return lowerBound;
	}

	/**
	 * Setter for lowerBound.
	 * 
	 * @param lowerBound
	 *            the lowerBound to set
	 */
	public void setLowerBound(T lowerBound) {
		this.lowerBound = lowerBound;
	}

	/**
	 * Getter for upperBound.
	 * 
	 * @return the upperBound
	 */
	public T getUpperBound() {
		return upperBound;
	}

	/**
	 * Setter for upperBound.
	 * 
	 * @param upperBound
	 *            the upperBound to set
	 */
	public void setUpperBound(T upperBound) {
		this.upperBound = upperBound;
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
