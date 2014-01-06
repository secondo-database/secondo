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
		
		if (pListExpr.isAtom())
		{
			int atomType = pListExpr.atomType();
			
			switch (atomType)
			{
				case ListExpr.TEXT_ATOM:
				{
					// remove tags
					int indStart = (result.indexOf("<text>") + 6);
					int indEnd = result.lastIndexOf("</text--->");
					result = result.substring(indStart, indEnd);
					break;
				}
				case ListExpr.STRING_ATOM:
				{
					// remove quotes
					int indStart = result.indexOf('\"')+1;
					int indEnd = result.lastIndexOf('\"');
					result = result.substring(indStart, indEnd);
					break;
				}
				case ListExpr.INT_ATOM:
				case ListExpr.REAL_ATOM:
				case ListExpr.BOOL_ATOM:
				{
					// remove whitespace
					result = result.trim();
					break;
				}				
			}
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
			// atom-type attributes
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
			
			// non-atomar attributes
			ListExpr le = new ListExpr();
			le.readFromString("(" + pType + " " + pValue + ")");
			return le;
		}
		catch (Exception e)
		{
			return null;
		}
	}
	
	
	/** 
     * Checks whether this type can be displayed in UpdateViewer2.
	 * Allowed are atomar types
	 **/
	public static boolean typeAllowed(ListExpr LE)
	{
		return true;
		//if(LE==null) return false;
		//return LE.isAtom();
	}
 
} 
