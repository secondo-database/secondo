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
----

last change 2006-01-23

*/

package viewer.update2;

import sj.lang.ListExpr;
import java.util.Vector;
import tools.Reporter;

/*
This class is the central instance for formatting attributes within the 
UpdateViewer. Here, the conversion from string into nested list and vice
versa is performed. 

*/
public class AttributeFormatter
{
	/**
	 * Returns the formatted String representation.
	 * TODO: format Date, handle unallowed types
	 */
	public static String fromListExprToString(ListExpr pListExpr)
	{
		String result = pListExpr.toString();
		
		// special case TEXT attribute: remove tags
		if (pListExpr.atomType() == ListExpr.TEXT_ATOM)
		{
			int indStart = result.indexOf('>')+1;
			int indEnd = result.lastIndexOf('<');
			result = result.substring(indStart, indEnd);
		}
		if (pListExpr.atomType() == ListExpr.STRING_ATOM)
		{
			int indStart = result.indexOf('\"')+1;
			int indEnd = result.lastIndexOf('\"');
			result = result.substring(indStart, indEnd);
		}
		if (pListExpr.atomType() == ListExpr.INT_ATOM 
			|| pListExpr.atomType() == ListExpr.REAL_ATOM
			|| pListExpr.atomType() == ListExpr.BOOL_ATOM)
		{
			result = result.trim();
		}
					
		return result;
	}
	
	/**
	 * Builds attribute list expression from String
	 * TODO: format Date, handle unallowed types
	 */
	public static ListExpr fromStringToListExpr(String pType, String pValue)
	{		
		try
		{
			if (pType.equals("bool"))
				return ListExpr.boolAtom(pValue.toUpperCase().equals("TRUE")? true : false);
			if (pType.equals("int"))
				return ListExpr.intAtom((pValue.length() == 0) ? 0 : Integer.parseInt(pValue));
			if (pType.equals("real"))
				return ListExpr.realAtom((pValue.length() == 0) ? 0.0 : Float.parseFloat(pValue));
			if (pType.equals("string"))
				return ListExpr.stringAtom(pValue);
			if (pType.equals("text"))
				return ListExpr.textAtom(pValue);
		}
		catch (Exception e)
		{
			return null;
		}
		return null;
	}
	
	
	/** 
     * Checks whether this type can be displayed in UpdateViewer2.
	 * Allowed are atomar types
	 **/
	public static boolean typeAllowed(ListExpr LE)
	{
		if(LE==null) return false;
		return LE.isAtom();
	}
 
} 
