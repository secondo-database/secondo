package mmdb.data;

import mmdb.data.attributes.MemoryAttribute;

public abstract class MemoryObjects {

	public static enum MemoryObjectType {

		MemoryRelation(MemoryRelation.class), MemoryTuple(MemoryTuple.class), MemoryAttribute(
				MemoryAttribute.class);

		final Class<? extends MemoryObject> objectClass;

		MemoryObjectType(Class<? extends MemoryObject> objectClass) {
			this.objectClass = objectClass;
		}

	}

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
