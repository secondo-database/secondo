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

import viewer.relsplit.InvalidRelationException;


/**
 * Contains name, tpye information and tuples of one relation.
 *
 * @see viewer.update2.Tuple
 */
public class Relation
{
	
	private String name;	
	private RelationTypeInfo relTypeInfo;
	private List<String> tupleIDs;
	private List<Tuple> tuples;
	private boolean initialized;
	
	
	/**
	 * Constructor
	 */
	public Relation()
	{
		tuples = new ArrayList<Tuple>();
		tupleIDs = new ArrayList<String>();
		relTypeInfo = new RelationTypeInfo(); 
		initialized = false;
	}
	
	
	/** 
	 * Append new tuple to relation.
	 */
	public void addTuple(ListExpr pTupleLE) throws InvalidRelationException
	{
		Tuple tup = new Tuple(this.relTypeInfo, pTupleLE);
		this.tuples.add(tup);
		this.tupleIDs.add(tup.getID());			
	}
	
	
	/**
	 * Returns an empty relation of same name and same type as this.
	 * Relation is marked as initialized.
	 */
	public Relation createEmptyClone()
	{
		Relation result = new Relation();
		result.relTypeInfo = this.relTypeInfo;		
		result.initialized = true;
		return result;
	}
	
	
	/**
	 * Returns list of attribute names.
	 */
	public List<String> getAttributeNames()
	{
		//Reporter.debug("Relation.getAttributeNames: " + this.relTypeInfo.getAttributeNames());
		return this.relTypeInfo.getAttributeNames();
	}
	
	/**
	 * Returns list of Attribute types.
	 */
	public List<String> getAttributeTypes()
	{
		//Reporter.debug("Relation.getAttributeTypes: " + this.relTypeInfo.getAttributeTypes());
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
	
	
	/*
	public int find(String S,boolean CaseSensitiv,int OffSet)
	{
		boolean found =false;
		int pos = -1;
		String US = S.toUpperCase();
		for(int i=OffSet;i<SecondoObjects.size()&&!found;i++)
		{
			String tmpname = get(i).getName();
			if(CaseSensitiv){
				if (tmpname.indexOf(S)>=0)
				{
					found=true;
					pos=i;
				}
			} else{
				if (tmpname.toUpperCase().indexOf(US)>=0)
				{
					found=true;
					pos=i;
				}
				
			}
		}
		return pos;
	}
	 */
	
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

		if(!this.readValueFromLE(LE.second()))
		{
			Reporter.writeError("update2.Relation.readFromSecondoObject : wrong value list");
			return false;
		}
		
		Reporter.debug("update2.Relation.readFromSecondoObject: no of tuples is " + this.getTupleCount());

		initialized = true;
		return true;
	}
		
	
	/** 
	 * Reads the value of this Relation 
	 */
	private boolean readValueFromLE(ListExpr ValueList) throws InvalidRelationException
	{
		Reporter.debug("update2.Relation.readRelationValueFromLE : LE is " + ValueList);
		
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
				this.addTuple(NextTuple);
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
		Reporter.debug("Relation.removeTupleByID: tupleID "+ pTupleId + " has index " + ti);
		
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
		
		Reporter.debug("Relation.removeTupleByIndex: index "+ pIndex);
		
		tuples.remove(pIndex);
		tupleIDs.remove(pIndex);
	}
	
	
	/** 
	 * Replaces all tuple values on given tuple position. 
	 */
	public void setTupleAt(int pTupleIndex, List<String> pTupleValues)
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
	 * Replaces the value in specified tuple at specified attribute index.
	 */
	public void setValueAt(int pTupleIndex, int pAttributeIndex, String pAttribute)
	{
		if(!initialized || pTupleIndex<0 || pTupleIndex>=this.tuples.size() 
		   || pAttributeIndex<0 || pAttributeIndex>=this.getTupleSize() || pAttribute==null)
		{
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
