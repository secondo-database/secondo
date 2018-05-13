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

package mol.datatypes.base;

import mol.datatypes.GeneralType;

/**
 * Class for representation of the 'String' data type
 * 
 * @author Markus Fuessel
 */
public class BaseString extends GeneralType implements Comparable<BaseString> {

	/**
	 * The string value
	 */
	private final String value;

	/**
	 * Simple constructor, creates an undefined 'BaseString' object
	 */
	public BaseString() {
		this.value = "";
		setDefined(false);
	}

	/**
	 * Constructor, create an defined 'BaseString' object
	 * 
	 * @param value
	 *            - the string value
	 */
	public BaseString(final String value) {
		this.value = value;
		setDefined(true);

	}

	/**
	 * Copy constructor
	 * 
	 * @param original
	 *            - the 'BaseString' object to copy
	 */
	public BaseString(final BaseString original) {
		this.value = original.getValue();
		setDefined(original.isDefined());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Comparable#compareTo(java.lang.Object)
	 */
	@Override
	public int compareTo(final BaseString s) {

		return value.compareTo(s.getValue());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		return value.hashCode();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(final Object obj) {

		if (obj == null || !(obj instanceof BaseString)) {
			return false;
		}

		if (this == obj) {
			return true;
		}

		BaseString otherString = (BaseString) obj;

		return compareTo(otherString) == 0;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return "BaseString [value='" + value + "', isDefined()=" + isDefined() + "]";
	}

	/**
	 * Getter for the string value
	 * 
	 * @return the value
	 */
	public String getValue() {
		return value;
	}
}
