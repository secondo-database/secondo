/* 
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
---
*/

package viewer.update2;
 

/*
 * This class contains constants that each name a state of the 'UpdateViewer2'.
 */
public class Filter 
{
	public static final int FILTERTYPE_EQUALS = 0;
	public static final int FILTERTYPE_CONTAINS = 1;
	
	public int type = FILTERTYPE_EQUALS;
	public String attributeName;
	public String attributeValue;
	
	public Filter(int pFilterType, String pAttributeName, String pAttributeValue)
	{
		this.type = pFilterType;
		this.attributeName = pAttributeName;
		this.attributeValue = pAttributeValue;
	}
}
