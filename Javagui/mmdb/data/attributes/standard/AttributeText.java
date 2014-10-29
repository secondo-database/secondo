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
import sj.lang.ListExpr;

/**
 * Object representation for database attributes of type 'text'.
 *
 * @author Alexander Castor
 */
public class AttributeText extends MemoryAttribute implements Orderable, Parsable {

	/**
	 * The prefix of the list element
	 */
	private static final String PREFIX = "<text>";

	/**
	 * The suffix of the list element
	 */
	private static final String SUFFIX = "</text--->";

	/**
	 * The attribute's value
	 */
	private String value;

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#fromList(sj.lang.ListExpr)
	 */
	@Override
	public void fromList(ListExpr list) {
		String listValue = list.textValue();
		listValue = listValue.replace(PREFIX, "");
		listValue = listValue.replace(SUFFIX, "");
		setValue(listValue);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.attributes.MemoryAttribute#toList()
	 */
	@Override
	public ListExpr toList() {
		String listValue = PREFIX + getValue() + SUFFIX;
		return ListExpr.textAtom(listValue);
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
		AttributeText attributeOther = (AttributeText) attribute;
		if (getValue().equals(attributeOther.getValue())) {
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
		return getValue().hashCode();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Comparable#compareTo(java.lang.Object)
	 */
	@Override
	public int compareTo(Orderable attribute) {
		String valueOther = ((AttributeText) attribute).getValue();
		return getValue().compareTo(valueOther);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.features.Parseable#parse(java.lang.String)
	 */
	@Override
	public Parsable parse(String text) {
		if (text == null) {
			return null;
		}
		AttributeText result = new AttributeText();
		result.setValue(text);
		return result;
	}

	/**
	 * Getter for value.
	 * 
	 * @return the value
	 */
	public String getValue() {
		return value;
	}

	/**
	 * Setter for value.
	 * 
	 * @param value
	 *            the value to set
	 */
	public void setValue(String value) {
		this.value = value;
	}

}
