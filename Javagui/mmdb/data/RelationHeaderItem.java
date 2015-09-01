package mmdb.data;

import mmdb.data.attributes.MemoryAttribute;

/**
 * Inner class representing a relation's header item.
 */
public class RelationHeaderItem {

	/**
	 * The attribute's identifier.
	 */
	private String identifier;

	/**
	 * The attribute's type as String.
	 */
	private String typeName;

	/**
	 * The attribute's type as Class.
	 */
	private Class<? extends MemoryAttribute> type;

	/**
	 * If set to false, the attribute is not visible.
	 */
	private boolean projected;

	/**
	 * Creates a new instance.
	 * 
	 * @param identifier
	 *            the identifier to set
	 * @param typeName
	 *            the typeName to set
	 */
	public RelationHeaderItem(String identifier, String typeName) {
		this.identifier = identifier;
		this.typeName = typeName;
		this.type = MemoryAttribute.getTypeClass(typeName);
		this.projected = true;
	}

	/**
	 * Getter for identifier.
	 * 
	 * @return the identifier
	 */
	public String getIdentifier() {
		return identifier;
	}

	/**
	 * Getter for typeName.
	 * 
	 * @return the typeName
	 */
	public String getTypeName() {
		return typeName;
	}

	/**
	 * Getter for type.
	 * 
	 * @return the type
	 */
	public Class<? extends MemoryAttribute> getType() {
		return type;
	}

	/**
	 * Getter for projected.
	 * 
	 * @return the projected
	 */
	public boolean isProjected() {
		return projected;
	}

	/**
	 * Setter for projected.
	 * 
	 * @param projected
	 *            the projected to set
	 */
	public void setProjected(boolean projected) {
		this.projected = projected;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object obj) {
		if (obj == null)
			return false;
		RelationHeaderItem other = (RelationHeaderItem) obj;
		if (!isProjected() && !other.isProjected()) {
			return true;
		}
		if (!identifier.equals(other.identifier)) {
			return false;
		}
		return true;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((identifier == null) ? 0 : identifier.hashCode());
		return result;
	}

}