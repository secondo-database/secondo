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

package mmdb.data.attributes.instant;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.date.AttributeInstant;
import mmdb.error.convert.ConversionException;
import sj.lang.ListExpr;

/**
 * Superclass for all instant attributes.
 *
 * @author Alexander Castor
 */
public abstract class InstantAttribute<T extends MemoryAttribute> extends MemoryAttribute {

	/**
	 * The attribute's instant.
	 */
	private AttributeInstant instant;

	/**
	 * The attribute's value.
	 */
	private T value;

	/**
	 * Retrieves an instant object which will be used as value for this
	 * attribute.
	 */
	protected abstract T getInstantObject();

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#fromList(sj.lang.ListExpr)
	 */
	@Override
	public void fromList(ListExpr list) throws ConversionException {
		AttributeInstant instant = new AttributeInstant();
		instant.fromList(list.first());
		setInstant(instant);
		T value = getInstantObject();
		value.fromList(list.second());
		setValue(value);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#toList()
	 */
	@Override
	public ListExpr toList() throws ConversionException {
		return ListExpr.twoElemList(getInstant().toList(), getValue().toList());
	}

	/**
	 * Getter for instant.
	 * 
	 * @return the instant
	 */
	public AttributeInstant getInstant() {
		return instant;
	}

	/**
	 * Setter for instant.
	 * 
	 * @param instant
	 *            the instant to set
	 */
	public void setInstant(AttributeInstant instant) {
		this.instant = instant;
	}

	/**
	 * Getter for value.
	 * 
	 * @return the value
	 */
	public T getValue() {
		return value;
	}

	/**
	 * Setter for value.
	 * 
	 * @param value
	 *            the value to set
	 */
	public void setValue(T value) {
		this.value = value;
	}

}
