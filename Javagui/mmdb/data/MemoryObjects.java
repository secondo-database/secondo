package mmdb.data;

import mmdb.data.attributes.MemoryAttribute;

/**
 * Class to store and manage all subclasses of MemoryObject.
 * 
 * @author Bjoern Clasen
 */
public abstract class MemoryObjects {

	/**
	 * Enum for collecting all MemoryObject types.
	 */
	public static enum MemoryObjectType {

		MemoryRelation(MemoryRelation.class), MemoryTuple(MemoryTuple.class), MemoryAttribute(
				MemoryAttribute.class);

		final Class<? extends MemoryObject> objectClass;

		MemoryObjectType(Class<? extends MemoryObject> objectClass) {
			this.objectClass = objectClass;
		}

	}

	/**
	 * Retrieves the type name for a given type class.
	 * 
	 * @param typeClass
	 *            the type class whose type name is being searched
	 * @return the type name if it is found, else null
	 */
	public static String getTypeName(Class<?> typeClass) {
		// Delegate to MemoryAttribute
		if (MemoryAttribute.class.isAssignableFrom(typeClass)) {
			return MemoryAttribute.getTypeName(typeClass);
		}

		for (MemoryObjectType type : MemoryObjectType.values()) {
			if (type.objectClass.isAssignableFrom(typeClass)) {
				return type.toString();
			}
		}
		return null;
	}

	/**
	 * Retrieves the type class for a given type name.
	 * 
	 * @param typeName
	 *            the type name whose type class is being searched
	 * @return the type class if it is found, else null
	 */
	public static Class<? extends MemoryObject> getTypeClass(String typeName) {
		// Delegate to MemoryAttribute
		if (MemoryAttribute.getAllTypeNames().contains(typeName)) {
			return MemoryAttribute.getTypeClass(typeName);
		}

		for (MemoryObjectType type : MemoryObjectType.values()) {
			if (type.toString().equals(typeName)) {
				return type.objectClass;
			}
		}
		return null;
	}

}
