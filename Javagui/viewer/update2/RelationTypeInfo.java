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


public class RelationTypeInfo extends Head{
	
	List<String> attributeNames;
	List<String> attributeTypes;
	
	public boolean readValueFromLE(ListExpr LE)
	{
		boolean result = super.readFromRelTypeLE(LE);
		
		if (result)
		{
			this.attributeNames = new ArrayList<String>();
			this.attributeTypes = new ArrayList<String>();
			for (int i = 0; i < this.getSize(); i++)
			{
				attributeNames.add(this.get(i).Name);
				attributeTypes.add(this.get(i).Type);
			}
		}
		
		return result;
	}
	
	public List<String> getAttributeNames()
	{
		return this.attributeNames;
	}
	
	public List<String> getAttributeTypes()
	{
		return this.attributeTypes;
	}
	
	public String getAttributeName(int index)
	{
		return this.attributeNames.get(index);
	}
	
	public String getAttributeType(int index)
	{
		return this.attributeTypes.get(index);
	}
	
	public int getTidIndex()
	{
		return this.attributeNames.indexOf("TID");
	}
	
	
}

