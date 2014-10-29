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

package mmdb.data.attributes.standard;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.features.Orderable;
import mmdb.data.features.Parsable;
import mmdb.data.features.Summable;
import sj.lang.ListExpr;

/**
 * Object representation for database attributes of type 'int'.
 *
 * @author Alexander Castor
 */
public class AttributeInt extends MemoryAttribute implements Orderable, Parsable, Summable {

	/**
	 * The attribute's value
	 */
	private int value;

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#fromList(sj.lang.ListExpr)
	 */
	@Override
	public void fromList(ListExpr list) {
		setValue(list.intValue());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#toList()
	 */
	@Override
	public ListExpr toList() {
		return ListExpr.intAtom(getValue());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object attribute) {
		if (attribute == null) {
			return false;
		}
		int valueOther = ((AttributeInt) attribute).getValue();
		if (getValue() == valueOther) {
			return true;
		}
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		return getValue();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Comparable#compareTo(java.lang.Object)
	 */
	@Override
	public int compareTo(Orderable attribute) {
		int valueOther = ((AttributeInt) attribute).getValue();
		return Integer.compare(getValue(), valueOther);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.features.Parseable#parse(java.lang.String)
	 */
	@Override
	public Parsable parse(String text) {
		int value;
		try {
			value = Integer.parseInt(text);
		} catch (NumberFormatException e) {
			return null;
		}
		AttributeInt result = new AttributeInt();
		result.setValue(value);
		return result;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.features.Summable#sum(mmdb.data.features.Summable)
	 */
	@Override
	public Summable sum(Summable other) {
		AttributeInt otherAttribute = (AttributeInt) other;
		AttributeInt result = new AttributeInt();
		int sum = getValue() + otherAttribute.getValue();
		result.setValue(sum);
		return result;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.features.Summable#getValueAsReal()
	 */
	@Override
	public float getValueAsReal() {
		return getValue();
	}

	/**
	 * Getter for value.
	 * 
	 * @return the value
	 */
	public int getValue() {
		return value;
	}

	/**
	 * Setter for value.
	 * 
	 * @param value
	 *            the value to set
	 */
	public void setValue(int value) {
		this.value = value;
	}

}
