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

package viewer.update2;

import gui.SecondoObject;
import gui.idmanager.*;
import java.util.ArrayList;
import java.util.List;
import sj.lang.ListExpr;
import tools.Reporter;
import viewer.update2.InvalidRelationException;

/**
 * Contains name, tpye information and tuples of one relation.
 *
 * @see viewer.update2.model.Tuple
 */
public class Relation
{
	private String name;	
	private RelationTypeInfo relTypeInfo;
	private List<String> tupleIDs;
	private List<Tuple> tuples;
	private List<String> readOnlyAttributes; 
	private boolean initialized;
	
	
	/**
	 * Constructor
	 */
	public Relation()
	{
		tuples = new ArrayList<Tuple>();
		tupleIDs = new ArrayList<String>();
		relTypeInfo = new RelationTypeInfo();
		readOnlyAttributes = new ArrayList<String>();
		initialized = false;
	}
	
	
	/** 
	 * Append new tuple to relation.
	 */
	public void addTuple(Tuple pTuple) throws InvalidRelationException
	{
		if(initialized && pTuple!=null)
		{
			if (!pTuple.getTypeInfo().equals(this.getTypeInfo()))
			{
				throw new InvalidRelationException("Param Tuple type does not match relation's tuple type.");
			}
			else
			{
				this.tuples.add(pTuple);
				
				if (pTuple.getID() != null && pTuple.getID().length() > 0)
				{
					this.tupleIDs.add(pTuple.getID());		
				}
				else
				{
					this.tupleIDs.add(""+tupleIDs.size());		
				}
			}
		}
	}
	
	
	/**
	 * Returns an empty relation of same name and same type as this.
	 * Relation is marked as initialized.
	 */
	public Relation createEmptyClone()
	{
		Relation result = new Relation();
		result.name = this.name;
		result.relTypeInfo = this.relTypeInfo;		
		result.readOnlyAttributes = this.readOnlyAttributes;
		result.initialized = true;
		return result;
	}
	
	
	/**
	 * Returns an empty relation of same name and same type as this.
	 * Relation is marked as initialized.
	 */
	public Tuple createEmptyTuple()
	{
		if (this.initialized)
		{
			try
			{
				return new Tuple(this.relTypeInfo, null);
			}
			catch (InvalidRelationException e)
			{
				// do nothing: as relTypeinfo is taken from this initialized 
				// relation it is guaranteed to be ok.
			}
		}
		return null;
	}
	
	
	/**
	 * Returns list of attribute names.
	 */
	public List<String> getAttributeNames()
	{
		return this.relTypeInfo.getAttributeNames();
	}
	
	/**
	 * Returns list of Attribute types.
	 */
	public List<String> getAttributeTypes()
	{
		return this.relTypeInfo.getAttributeTypes();
	}

	
	/**
	 * Returns relation name.
	 */
	public String getName()
	{
		return this.name;
	}
	
	
	public RelationTypeInfo getTypeInfo()
	{
		return this.relTypeInfo;
	}
	
	
	/** 
	 * Returns tuple at given index. 
	 */
	public Tuple getTupleAt(int pTupleIndex)
	{
		if(!initialized || pTupleIndex<0 || pTupleIndex>=this.tuples.size())
		{
			return null;
		}
		return tuples.get(pTupleIndex);
	}
	
	
	/** 
	 * Returns tuple at given index. 
	 */
	public List<Tuple> getTuplesByFilter(int pFilterType, String pAttrName, String pValue)
	{
		List<Tuple> result = new ArrayList<Tuple>();
		for (Tuple tup : this.tuples)
		{
			switch(pFilterType)
			{
				case Filter.FILTERTYPE_CONTAINS:
				{
					if (tup.getValueByAttrName(pAttrName).contains(pValue))
					{
						result.add(tup);
					}
					break;
				}
				case Filter.FILTERTYPE_EQUALS:
				default:
				{
					if (tup.getValueByAttrName(pAttrName).equals(pValue))
					{
						result.add(tup);
					}
					break;
				}
			}
		}
		return result;
	}
	
	
	/** 
	 * Returns list of all tuples that match all filter conditions. 
	 */
	public List<Tuple> getTuplesByFilter(List<Filter> pFilters)
	{
		List<Tuple> result = new ArrayList<Tuple>();
		Filter filter;
		boolean ok;
		int index;
		
		for (Tuple tup : this.tuples)
		{
			ok = true;
			index = 0;
			
			while (ok && index < pFilters.size())
			{
				filter = pFilters.get(index);
				switch(filter.type)
				{
					case Filter.FILTERTYPE_CONTAINS:
					{
						if (!tup.getValueByAttrName(filter.attributeName).contains(filter.attributeValue))
						{
							ok = false;
						}
						break;
					}
					case Filter.FILTERTYPE_EQUALS:
					{
						if (!tup.getValueByAttrName(filter.attributeName).equals(filter.attributeValue))
						{
							ok = false;
						}
						break;
					}
					default:
						ok = false;
				}
				index++;
			}
			if (ok)
			{
				result.add(tup);
			}
		}
		return result;
	}
	
	
	/** 
	 * Returns the number of tuples,
	 * returns -1 if relation is not initialized.
	 */
	public int getTupleCount()
	{
		if(!initialized)
			return -1;
		else
			return tuples.size();
	}
	
	public int getTupleIndex(String pTupleId)
	{
		return this.tupleIDs.indexOf(pTupleId);
	}
	
	/* 
	 * Returns the number of values in a tuple
	 * if this relation not initialized -1 is returned
	 */
	public int getTupleSize()
	{
		if (!initialized)
			return -1;
		else
			return relTypeInfo.getSize();
	}
	
	
	/**
	 * Returns true if relation name. type info and tuple list were initialized.
	 */
	public boolean isAttributeReadOnly(String pAttributeName)
	{ 
		return this.readOnlyAttributes.contains(pAttributeName);
	}
	
	/**
	 * Returns true if relation name. type info and tuple list were initialized.
	 */
	public boolean isInitialized()
	{ 
		return initialized;
	}
	
	
	
	/**
	 * Returns TRUE if SO contains a relation 
	 */
	public static boolean isRelation(SecondoObject SO)
	{
		return RelationTypeInfo.isRelation(SO.toListExpr());
	}
	
		
	/**
	 * Initializes Relation from a relation SecondoObject.
	 * @return TRUE if initalization was successful.
	 */
	public boolean readFromSecondoObject(SecondoObject SO) throws InvalidRelationException
	{
		name = SO.getName();
		initialized = false;
		ListExpr LE = SO.toListExpr();
		if(LE.listLength()!=2)
		{
			Reporter.writeError("update2.Relation.readFromSecondoObject : wrong list length");
			return false;
		}
		if(!relTypeInfo.readValueFromLE(LE.first()))
		{
			Reporter.writeError("update2.Relation.readFromSecondoObject : wrong type list");
			return false;
		}

		initialized = true;

		if(!this.readValueFromLE(LE.second()))
		{
			Reporter.writeError("update2.Relation.readFromSecondoObject : wrong value list");
			return false;
		}
		
		//Reporter.debug("update2.Relation.readFromSecondoObject: no of tuples is " + this.getTupleCount());

		return true;
	}
		
	
	/** 
	 * Reads the value of this Relation 
	 */
	private boolean readValueFromLE(ListExpr ValueList) throws InvalidRelationException
	{
		//Reporter.debug("update2.Relation.readRelationValueFromLE : LE is " + ValueList);
		
		ListExpr NextTuple;
		ListExpr Rest = ValueList;
		tuples.clear();
		tupleIDs.clear();
		boolean ok = true;
		
		while(Rest.listLength()>0 && ok)
		{
			NextTuple = Rest.first();
			if(NextTuple.listLength()!=this.relTypeInfo.getSize())  // wrong tuplelength
			{
				ok = false;
			}
			else
			{
				Tuple tuple = this.createEmptyTuple();
				tuple.readValueFromLE(NextTuple);
				this.addTuple(tuple);
				//this.addTupleAsLE(NextTuple);
			}
			Rest = Rest.rest();
		}
		
		if(!ok)
		{
			tuples.clear();
			tupleIDs.clear();
		}
		
		return ok;
	}
	

	
	/** 
	 * Removes the tuple by its ID 
	 */
	public void removeTupleByID(String pTupleId)
	{
		if(!initialized || pTupleId == null)
		{
			return;
		}
		
		int ti = this.tupleIDs.indexOf(pTupleId);
		//Reporter.debug("Relation.removeTupleByID: tupleID "+ pTupleId + " has index " + ti);
		
		if(ti >= 0)
		{
			tuples.remove(ti);
			tupleIDs.remove(ti);
		}
	}
	
	
	/** 
	 * Removes the tuple by its index. 
	 */
	public void removeTupleByIndex(int pIndex)
	{
		if(!initialized || pIndex<0 || pIndex>=this.tuples.size())
		{
			return;
		}
		
		//Reporter.debug("Relation.removeTupleByIndex: index "+ pIndex);
		
		tuples.remove(pIndex);
		tupleIDs.remove(pIndex);
	}
	
	
	public void setAttributeReadOnly(String pAttributeName)
	{
		if (this.getAttributeNames().contains(pAttributeName)
			&& !this.readOnlyAttributes.contains(pAttributeName))
		{
			this.readOnlyAttributes.add(pAttributeName);
		}
	}
	
	/** 
	 * Replaces all tuple values on given tuple position. 
	 */
	public void setTupleByIndex(int pTupleIndex, List<String> pTupleValues)
	{
		if(!initialized || pTupleIndex<0 || pTupleIndex>=this.tuples.size())
		{
			return;
		}
		
		Tuple tup = this.tuples.get(pTupleIndex);
		
		for(int i=0;i<this.relTypeInfo.getSize();i++)
		{
			tup.setValueAt(pTupleValues.get(i), i);
		}		
	}
	
	
	/** 
	 * Replaces tuple.
	 */
	public void setTupleByID(Tuple pTuple) throws InvalidRelationException
	{
		if(initialized && pTuple!=null)
		{
			if (!pTuple.getTypeInfo().equals(this.getTypeInfo()))
			{
				throw new InvalidRelationException("Param Tuple type does not match relation's tuple type.");
			}
			else
			{
				int index = this.tupleIDs.indexOf(pTuple.getID());
				if (index >= 0) 
				{
					this.tuples.set(index, pTuple);
				}
			}
		}
	}

	
	/** 
	 * Replaces the value in specified tuple at specified attribute index.
	 */
	public void setValueAt(int pTupleIndex, int pAttributeIndex, String pAttribute)
	{
		if(!initialized || pTupleIndex<0 || pTupleIndex>=this.tuples.size() 
		   || pAttributeIndex<0 || pAttributeIndex>=this.getTupleSize() || pAttribute==null)
		{
			//Reporter.writeError("Relation.setValueAt: returned without setting value!");
			return;
		}
		
		Tuple tup = this.tuples.get(pTupleIndex);
		
		tup.setValueAt(pAttribute, pAttributeIndex);
	}
	
	@Override
	public String toString()
	{
		StringBuffer sb = new StringBuffer("[Relation]: ");
		sb.append(", Name: ").append(this.getName());
		sb.append(", attributeNames: ");
		for (String name : this.getAttributeNames())
		{
			sb.append(name).append(", ");
		}
		sb.append(", attributeTypes: ");
		for (String type : this.getAttributeTypes())
		{
			sb.append(type).append(", ");
		}
		return sb.toString();
	}
	
}
