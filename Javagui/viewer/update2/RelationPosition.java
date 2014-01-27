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


/**
 * Identifies an editing location within a relation.	 
 */
public class RelationPosition 
{
	private String relationName;
	private String attributeName;
	private String tupleId;
	private int offset; // position within attribute value string

	public RelationPosition(String pRelationName, 
							String pAttributeName,
							String pTupelId, 
							int pOffset) 
	{
		this.relationName = pRelationName;
		this.tupleId = pTupelId;
		this.attributeName = pAttributeName;
		this.offset = pOffset;
	}
	
	public String getAttributeName() { return this.attributeName; }
	public int getOffset() { return this.offset; }
	public String getRelationName() { return this.relationName;	}
	public String getTupleId() { return this.tupleId; }
	
	public String toString()
	{
		StringBuffer sb = new StringBuffer("[viewer.update2.RelationPosition]");
		sb.append(" relationName=").append(this.relationName);
		sb.append(", attributeName=").append(this.attributeName);
		sb.append(", tupelId=").append(this.tupleId);
		sb.append(", offset=").append(this.offset);		
		return sb.toString();
	}
	
}

