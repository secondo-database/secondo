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

import java.util.ArrayList;
import java.util.List;

import sj.lang.ListExpr;

import tools.Reporter;

import viewer.relsplit.InvalidRelationException;


/**
 *
 */
public class Tuple
{
	private String id;		
	private List<String> values;
	private RelationTypeInfo typeInfo;
	
	/**
	 * Default Constructor shall not be used.
	 **/
	private Tuple() {}

	
	public Tuple(RelationTypeInfo pTypeInfo, ListExpr pLE) throws InvalidRelationException
	{
		this.typeInfo = pTypeInfo;
		if (pTypeInfo==null || !this.readValueFromLE(pLE))
		{
			throw (new InvalidRelationException());
		}
	}
	
	public boolean readValueFromLE(ListExpr pLE)
	{
		boolean result = false;
		this.values = new ArrayList<String>();
		ListExpr tupleValues = pLE;
		ListExpr nextValue;
		
		// create empty Tuple
		if(pLE == null)
		{
			//Reporter.debug("Tuple.readValueFromLE: create empty Tuple ");
			for(int i=0; i<this.typeInfo.getSize(); i++)
			{				
				values.add("");
			}
			this.id = "";
			result = true;			
		}
		
		// fill Tuple with given values
		else if(tupleValues.listLength() == this.typeInfo.getSize())  // correct tuplelength
		{
			//Reporter.debug("Tuple.readValueFromLE: pLE is " + pLE.toString());
			for(int i=0; i<this.typeInfo.getSize(); i++)
			{
				nextValue = pLE.first();
				//Reporter.debug("Tuple.readValueFromLE: nextValue is " + nextValue.toString());
				
				String val = AttributeFormatter.fromListExprToString(nextValue);
				values.add(val);
				
				int tidindex = this.typeInfo.getTidIndex();
				if (tidindex >= 0 && tidindex == i)
				{
					this.id = val.trim();
				}
				//Reporter.debug("Tuple.readValueFromLE: new value is " + val);
				
				pLE = pLE.rest();
			}
			
			if (this.getID() == null)
			{
				this.id = "";
			}
			
			result = true;
		}
		return result;
	}

	public String getTypeAt(int pIndex)
	{
		if (pIndex >= 0 && pIndex < this.typeInfo.getSize())
		{
			return this.typeInfo.getAttributeType(pIndex);
		}
		return null;
	}
	
	
	public String getValueAt(int pIndex)
	{
		if (pIndex >= 0 && pIndex < this.typeInfo.getSize())
		{
			return values.get(pIndex);
		}
		return null;
	}
	
	
	public void setValueAt(String pValue, int pIndex)
	{
		if (pIndex >= 0 && pIndex < this.typeInfo.getSize())
		{
			this.values.set(pIndex, pValue);
		}
	}
	
	
	
	public String getID()
	{
		return id;
	}
	
	public String toString()
	{
		StringBuffer sb = new StringBuffer("[Tuple]: ");
		sb.append(", ID: ").append(this.getID());
		sb.append(", values: ");
		for (String val : this.values)
		{
			sb.append(val).append(", ");
		}
		return sb.toString();
	}
	
}
