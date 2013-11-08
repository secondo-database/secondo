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
	 */
	public String fromListExprToString(ListExpr pListExpr)
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
		
		// TODO
		// format Date
			
		return result;
	}
	
/*
	public String ListExprToString(ListExpr LE)
	{
		String result = "kein Format gefunden für Typ ";
		
		if(LE == null || LE.listLength() != 2 || 
		    LE.first().atomType != ListExpr.SYMBOL_ATOM)
		{
			
		}
		if(LE.first().atomType())
		{
			result = LE.stringValue();
		}
		}
		
        return result;
	}
	
	public ListExpr StringToListExpr(String type, String value)
	{
		
		if(value.length()>tools.Environment.MAX_STRING_LENGTH)
			return null;
		if(value.indexOf("\"")>=0)
			return null;
		return ListExpr.stringAtom(value);
	}
	
	/** Checks whether this is a simple type. 
	 * Other types are not allowed withing the current implementation
	 * of the updateviewer.
	 **/
	public boolean typeAllowed(ListExpr LE){
		if(LE==null) return false;
		return LE.isAtom(); // we allow all atomar types
	}
	
	
	


 
} 
