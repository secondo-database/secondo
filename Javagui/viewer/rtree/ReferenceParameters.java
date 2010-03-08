package viewer.rtree;

/**
 * ReferenceParameters contains detail information about 
 * the entities indexed by an rtree.
 * 
 * @author Christian Oevermann
 * @since 25.01.2010
 * @version 1.0
 */
public class ReferenceParameters { 
	
	// referenced relation
	private String relation;
	// referenced attribute
	private String attribute;
	// referenced type
	private String type;
	
	// constructors
	
	/**
	 * Creates a new ReferenceParameters object.
	 */
	public ReferenceParameters() 
	{
		this.relation = "";
		this.attribute = "";
		this.type = "";
	}

	/**
	 * Creates a new ReferenceParameters object.
	 * @param relation Referenced relation
	 * @param attribute Referenced attribute
	 * @param type Referenced type
	 */
	public ReferenceParameters(String relation, String attribute, String type) 
	{
		this.relation = relation;
		this.attribute = attribute;
		this.type = type;
	}
	
	// public members
	
	/**
	 * Gets the referenced relation.
	 * @return Referenced relation
	 */
	public String getRelation() 
	{
		return this.relation;
	}
	
	/**
	 * Sets the referenced relation.
	 * @param relation Referenced relation
	 */
	public void setRelation(String relation) 
	{
		this.relation = relation;
	}
	
	/**
	 * Gets the referenced attribute.
	 * @return Referenced attribute
	 */
	public String getAttribute() 
	{
		return this.attribute;
	}
	
	/**
	 * Sets the referenced attribute.
	 * @param attribute Referenced attribute
	 */
	public void setAttribute(String attribute) 
	{
		this.attribute = attribute;
	}
	
	/**
	 * Gets the referenced type.
	 * @return Referenced type
	 */
	public String getType() 
	{
		return this.type;
	}
	
	/**
	 * Sets the referenced type.
	 * @param type Referenced type
	 */
	public void setType(String type) 
	{
		this.type = type;
	}
	
	/**
	 * Clears all parameters.
	 */
	public void clear() 
	{
		this.relation = "";
		this.attribute = "";
		this.type = "";
	}
	
	/**
	 * Indicates if all parameters are set.
	 * @Return True if all parameters are set, otherwise false
	 */
	public boolean isComplete() 
	{
		return (this.relation != "" &&
			this.attribute != "" &&
			this.type != "");
	}
}
