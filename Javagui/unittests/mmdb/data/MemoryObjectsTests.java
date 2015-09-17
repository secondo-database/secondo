package unittests.mmdb.data;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import mmdb.data.MemoryObjects;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeText;

import org.junit.Test;

public class MemoryObjectsTests {

	@Test
	public void testGetTypeName() {
		assertEquals("MemoryTuple",
				MemoryObjects.getTypeName(MemoryTuple.class));
		assertNull(MemoryObjects.getTypeName(String.class));
	}

	@Test
	public void testGetTypeClass() {
		assertEquals(AttributeInt.class, MemoryObjects.getTypeClass("int"));
		assertEquals(AttributeText.class, MemoryObjects.getTypeClass("text"));
		assertEquals(MemoryAttribute.class,
				MemoryObjects.getTypeClass("MemoryAttribute"));
		assertEquals(MemoryTuple.class,
				MemoryObjects.getTypeClass("MemoryTuple"));
		assertEquals(MemoryRelation.class,
				MemoryObjects.getTypeClass("MemoryRelation"));
		assertNull(MemoryObjects.getTypeClass("abc"));
	}

}
