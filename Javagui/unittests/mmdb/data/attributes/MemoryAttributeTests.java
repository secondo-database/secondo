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

package unittests.mmdb.data.attributes;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.util.Set;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.data.attributes.standard.AttributeText;

import org.junit.Test;

/**
 * Tests for class "MemoryAttribute".
 *
 * @author Alexander Castor
 */
public class MemoryAttributeTests {

	@Test
	public void testGetTypeNameValid() {
		Class<?> typeClass = AttributeInt.class;
		String actual = MemoryAttribute.getTypeName(typeClass);
		assertEquals("int", actual);
	}

	@Test
	public void testGetTypeNameInvalid() {
		Class<?> typeClass = String.class;
		String actual = MemoryAttribute.getTypeName(typeClass);
		assertNull(actual);
	}

	@Test
	public void testGetAllTypeNames() {
		Set<String> types = MemoryAttribute.getAllTypeNames();
		assertTrue(types.contains("bool"));
		assertTrue(types.contains("int"));
		assertTrue(types.contains("real"));
		assertTrue(types.contains("string"));
		assertTrue(types.contains("text"));
	}

	@Test
	public void testGetTypeClassValid() {
		Class<? extends MemoryAttribute> typeClass = MemoryAttribute.getTypeClass("int");
		assertNotNull(typeClass);
	}

	@Test
	public void testGetTypeClassInvalid() {
		Class<? extends MemoryAttribute> typeClass = MemoryAttribute.getTypeClass("integer");
		assertNull(typeClass);
	}

	@Test
	public void testGetAllTypeClasses() {
		Set<Class<? extends MemoryAttribute>> types = MemoryAttribute.getAllTypeClasses();
		assertTrue(types.contains(AttributeBool.class));
		assertTrue(types.contains(AttributeInt.class));
		assertTrue(types.contains(AttributeReal.class));
		assertTrue(types.contains(AttributeString.class));
		assertTrue(types.contains(AttributeText.class));
	}

}
