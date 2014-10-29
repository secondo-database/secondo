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

package mmdb.data.attributes;

import java.util.HashSet;
import java.util.Set;

import mmdb.data.attributes.date.AttributeDate;
import mmdb.data.attributes.date.AttributeInstant;
import mmdb.data.attributes.instant.AttributeIbool;
import mmdb.data.attributes.instant.AttributeIint;
import mmdb.data.attributes.instant.AttributeIpoint;
import mmdb.data.attributes.instant.AttributeIreal;
import mmdb.data.attributes.instant.AttributeIregion;
import mmdb.data.attributes.instant.AttributeIstring;
import mmdb.data.attributes.moving.AttributeMbool;
import mmdb.data.attributes.moving.AttributeMint;
import mmdb.data.attributes.moving.AttributeMpoint;
import mmdb.data.attributes.moving.AttributeMreal;
import mmdb.data.attributes.moving.AttributeMregion;
import mmdb.data.attributes.moving.AttributeMstring;
import mmdb.data.attributes.range.AttributePeriods;
import mmdb.data.attributes.range.AttributeRbool;
import mmdb.data.attributes.range.AttributeRint;
import mmdb.data.attributes.range.AttributeRreal;
import mmdb.data.attributes.range.AttributeRstring;
import mmdb.data.attributes.spatial.AttributeLine;
import mmdb.data.attributes.spatial.AttributePoint;
import mmdb.data.attributes.spatial.AttributePoints;
import mmdb.data.attributes.spatial.AttributeRect;
import mmdb.data.attributes.spatial.AttributeRegion;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.data.attributes.standard.AttributeText;
import mmdb.data.attributes.unit.AttributeUbool;
import mmdb.data.attributes.unit.AttributeUint;
import mmdb.data.attributes.unit.AttributeUpoint;
import mmdb.data.attributes.unit.AttributeUreal;
import mmdb.data.attributes.unit.AttributeUregion;
import mmdb.data.attributes.unit.AttributeUstring;
import mmdb.error.convert.ConversionException;
import sj.lang.ListExpr;

/**
 * Base class for all attribute types. Special treatment for "int" since is it a
 * Java keyword and therefore handled internally as "integer".
 *
 * @author Alexander Castor
 */
public abstract class MemoryAttribute {

	/**
	 * Enum for collecting all attribute types.
	 */
	private static enum AttributeType {
		bool(AttributeBool.class), integer(AttributeInt.class), real(AttributeReal.class), string(
				AttributeString.class), text(AttributeText.class), line(AttributeLine.class), point(
				AttributePoint.class), points(AttributePoints.class), rect(AttributeRect.class), region(
				AttributeRegion.class), date(AttributeDate.class), instant(AttributeInstant.class), ibool(
				AttributeIbool.class), iint(AttributeIint.class), ipoint(AttributeIpoint.class), ireal(
				AttributeIreal.class), iregion(AttributeIregion.class), istring(
				AttributeIstring.class), mbool(AttributeMbool.class), mint(AttributeMint.class), mpoint(
				AttributeMpoint.class), mreal(AttributeMreal.class), mregion(AttributeMregion.class), mstring(
				AttributeMstring.class), periods(AttributePeriods.class), rbool(
				AttributeRbool.class), rint(AttributeRint.class), rreal(AttributeRreal.class), rstring(
				AttributeRstring.class), ubool(AttributeUbool.class), uint(AttributeUint.class), upoint(
				AttributeUpoint.class), ureal(AttributeUreal.class), uregion(AttributeUregion.class), ustring(
				AttributeUstring.class);

		final Class<? extends MemoryAttribute> attributeClass;

		AttributeType(Class<? extends MemoryAttribute> attributeClass) {
			this.attributeClass = attributeClass;
		}
	}

	/**
	 * Converts a given nested list to an instance of the corresponding
	 * attribute type.
	 * 
	 * @param list
	 *            the list to be converted
	 */
	public abstract void fromList(ListExpr list) throws ConversionException;

	/**
	 * Converts the attribute in nested list representation.
	 * 
	 * @return the converted list
	 */
	public abstract ListExpr toList() throws ConversionException;

	/**
	 * Retrieves the type name for a given type class.
	 * 
	 * @param typeClass
	 *            the type class whose type name is being searched
	 * @return the type name if it is found, else null
	 */
	public static String getTypeName(Class<?> typeClass) {
		for (AttributeType type : AttributeType.values()) {
			if (typeClass.isAssignableFrom(type.attributeClass)) {
				if ("integer".equals(type.toString())) {
					return "int";
				} else {
					return type.toString();
				}
			}
		}
		return null;
	}

	/**
	 * Retrieves a set of all type names.
	 * 
	 * @return a set of all type names
	 */
	public static Set<String> getAllTypeNames() {
		Set<String> result = new HashSet<String>();
		for (AttributeType type : AttributeType.values()) {
			if ("integer".equals(type.toString())) {
				result.add("int");
			} else {
				result.add(type.toString());
			}
		}
		return result;
	}

	/**
	 * Retrieves the type class for a given type name.
	 * 
	 * @param typeName
	 *            the type name whose type class is being searched
	 * @return the type class if it is found, else null
	 */
	public static Class<? extends MemoryAttribute> getTypeClass(String typeName) {
		String name = ("int".equals(typeName)) ? "integer" : typeName;
		name = ("integer".equals(typeName)) ? "invalid" : name;
		for (AttributeType type : AttributeType.values()) {
			if (type.toString().equals(name)) {
				return type.attributeClass;
			}
		}
		return null;
	}

	/**
	 * Retrieves a set of all type classes.
	 * 
	 * @return a set of all type names
	 */
	public static Set<Class<? extends MemoryAttribute>> getAllTypeClasses() {
		Set<Class<? extends MemoryAttribute>> result = new HashSet<Class<? extends MemoryAttribute>>();
		for (AttributeType type : AttributeType.values()) {
			result.add(type.attributeClass);
		}
		return result;
	}

}
