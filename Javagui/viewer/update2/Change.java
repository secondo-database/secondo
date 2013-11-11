//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

package  viewer.update2;

import gui.SecondoObject;


/**
 * Information concerning ahange of a single tuple in a relation.	 
 */
public class Change {
	
	private int tupleIndex;
	
	private int attributeIndex;

	private int rowIndex;

	private String attributeName;

	private String attributeType;

	private String oldValue;

	private String newValue;


	/**
	 * Constructor expects tuple index and SecondoObject representing the tuple.
	 */
	public Change(int pTupleIndex, int pAttrIndex, int pRowIndex,
					String pAttrName, String pAttrType,
					String pOldValue, String pNewValue) 
	{
		this.tupleIndex = pTupleIndex;
		this.attributeIndex = pAttrIndex;
		this.rowIndex = pRowIndex;
		this.attributeName = pAttrName;
		this.attributeType = pAttrType;
		this.oldValue = pOldValue;
		this.newValue = pNewValue;
	}
	
	/**
	 * Returns tuple index.
	 */
	public int getTupleIndex()
	{
		return this.tupleIndex;
	}
	
	/**
	 * Returns attribute index.
	 */
	public int getAttributeIndex()
	{
		return this.attributeIndex;
	}
	
	/**
	 * Returns attribute index.
	 */
	public int getRowIndex()
	{
		return this.rowIndex;
	}
	
	/**
	 * Returns attribute name.
	 */
	public String getAttributeName()
	{
		return this.attributeName;
	}
	
	/**
	 * Returns attribute type.
	 */
	public String getAttributeType()
	{
		return this.attributeType;
	}
	
	
	/**
	 * Returns value before change.
	 */
	public String getOldValue()
	{
		return this.oldValue;
	}
	
	/**
	 * Returns value after change.
	 */
	public String getNewValue()
	{
		return this.newValue;
	}
	
	/**
	 * Returns true if given Change has same tuple id and attribute name.
	 */
	public boolean changesSameObject(Change pChange)
	{
		return (this.tupleIndex == pChange.getTupleIndex() 
				&& this.attributeName.equals(pChange.getAttributeName()));
	}
	
	
	public void setNewValue(String pValue)
	{
		this.newValue = pValue;
	}
	
	
	public void setOldValue(String pValue)
	{
		this.oldValue = pValue;
	}
	
	
	public String toString()
	{
		StringBuffer sb = new StringBuffer("[viewer.update2.Change]");
		sb.append(" tupleIndex=").append(this.tupleIndex);
		sb.append(", attributeIndex=").append(this.attributeIndex);
		sb.append(", attributeName=").append(this.attributeName);
		sb.append(", attributeType=").append(this.attributeType);
		sb.append(", oldValue=").append(this.oldValue);
		sb.append(", newValue=").append(this.newValue);
		
		return sb.toString();
	}
	
}

