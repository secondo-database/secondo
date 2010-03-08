package viewer.rtree;

import java.util.*;

import sj.lang.ListExpr;
import tools.Reporter;

/**
 * NestedListParser is a helper class to parse list expressions.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 17.12.2009
 */
public class NestedListParser {

	// public methods
	
	/**
	 * Retrieves a list of objects from the database.
	 * @param le List expression
	 * @param types Comma separated list of types (without any blanks inside)
	 * @return List of objects of the given types
	 */
	public Vector<String> getObjectsOfType(ListExpr le, String types)
	{
		return getObjectsOfType(le, types, false);
	}
	
	/**
	 * Retrieves a list of objects from the database.
	 * @param le List expression
	 * @param types Comma separated list of types (without any blanks inside)
	 * @param addTypeToName Indicates if ': <typeName>' is to be added to the object name.
	 * @return List of objects of the given types
	 */
	public Vector<String> getObjectsOfType(ListExpr le, String types, boolean addTypeToName)
	{
		String typesToRetrieve = "," + types.trim() + ',';
		
		// "list objects" structure
		// (inquiry (objects (OBJECTS ...))))
		// (OBJECTS (OBJECT xxx) (OBJECT xxx) (OBJECT xxx) ...)
		
		// check (inquiry (objects ...)) 
		if (! ((le.listLength() == 2) && (isSymbolNamed(le.first(), "inquiry"))))
		{
			getObjectsOfTypeError();
			return null;
		}
		
		// check (objects (OBJECTS ...))
		if (!((le.second().listLength() == 2) && 
			   ( isSymbolNamed(le.second().first(), "objects"))))
		{
			getObjectsOfTypeError();
			return null;
		}		
		
		// check (OBJECTS (OBJECT xxx) (OBJECT xxx) (OBJECT xxx) ...)
		ListExpr listOfObjects = le.second().second();
		if (!((listOfObjects.listLength() >= 2) && 
			   (isSymbolNamed(listOfObjects.first(), "OBJECTS"))))
		{
			getObjectsOfTypeError();
			return null;
		}			
		
		// iterate through all objects
		// remember objects of correct types
		Vector<String> objectNames = new Vector<String>();
		
		listOfObjects = listOfObjects.rest();
		while (!listOfObjects.isEmpty()) 
		{
			ListExpr aObject = listOfObjects.first();
			
			// check (OBJECT <name> () (typename ...))
			if (!((aObject.listLength() == 4) && 
				   ( isSymbolNamed(aObject.first(), "OBJECT")) &&
				   ( isSymbol(aObject.second()) )))
			{
				getObjectsOfTypeError();
				return null;
			}

			// check (typename ...)
			if (aObject.fourth().listLength() != 1)
			{
				getObjectsOfTypeError();
				return null;
			}

			ListExpr typeList = aObject.fourth();
			String actType = "";
						
			if (isSymbol(typeList.first()))
			{
				actType = typeList.first().stringValue();
			}
			
			else if ( isSymbol( typeList.first().first()))
			{
				actType = typeList.first().first().stringValue();
			}
			else 
			{
				getObjectsOfTypeError();
				return null;
			}
			
			// compare object types
			// remember object if type matches
			if (typesToRetrieve.indexOf(actType) != -1)
			{
				if (addTypeToName)
					objectNames.add( aObject.second().stringValue() + ": " + actType);
				else
					objectNames.add( aObject.second().stringValue());
			}

			listOfObjects = listOfObjects.rest();
		}
		
		return objectNames;
	}
	
	/**
	 * Extracts a list of attributes from the given list expression.
	 * @param le List expression
	 * @return List of attributes
	 */
	public Vector<String> extractTupleAttributes(ListExpr le)
	{
		Vector<String> attributes = new Vector<String>();
		
		// check (inquiry (objects ...)) 
		if (! ((le.listLength() == 2) && ( isSymbolNamed(le.first().first(), "rel"))))
		{
			getObjectsOfTypeError();
			return null;
		}

		le = le.first().second();
		
		if (! ((le.listLength() == 2) && (isSymbolNamed(le.first(), "tuple"))))
		{
			getObjectsOfTypeError();
			return null;
		}
		
		le = le.second();
		if (le.listLength() > 0)
		{
			// parse all attributes
			while (!le.isEmpty() )
			{
				attributes.add(le.first().first().stringValue() + ": " + le.first().second().stringValue());
				le = le.rest();
			}
		}
		return attributes;
	}
	
	/**
	 * Display an error message.
	 */
	private void getObjectsOfTypeError()
	{
		String errorMessage = "Nested List Parser Error:\n";
		errorMessage += "Error parsing list expression.";

		Reporter.showError(errorMessage);
	}
	
	/**
	 * Checks if the given list expression represents a symbol.
	 * @param le List expression
	 * @return True if the list expression represents a symbol, otherwise false.
	 */
	private boolean isSymbol(ListExpr le)
	{
		if (le == null)
		{
			return false;
		}
		
		if ((le.isAtom()) && (le.atomType() == ListExpr.SYMBOL_ATOM))
		{
			return true;
		}
		
		return false;
	}
	
	
	/**
	 * Checks if the given list expression represents a symbol with the given name.
	 * @param le List expression
	 * @param symbolName Symbol name
	 * @return True if the list expression represents a symbol with the given name, otherwise false.
	 */
	private boolean isSymbolNamed(ListExpr le, String symbolName)
	{
		if (le == null)
		{
			return false;
		}
		
		if ((le.isAtom()) && 
				  (le.atomType() == ListExpr.SYMBOL_ATOM) &&
				  (le.stringValue().compareTo(symbolName ) == 0))
		{
			return true;
		}
		
		return false;
	}

	/**
	 * Checks if the given list expression represents a given string.
	 * @param le List expression
	 * @param symbolName String
	 * @return True if the list expression represents the given string, otherwise false.
	 */
	private boolean isStringNamed(ListExpr le, String stringName)
	{
		if ( le == null)
		{
			return false;
		}
		
		if ((le.isAtom()) && 
				  (le.atomType() == ListExpr.STRING_ATOM) &&
				  (le.stringValue().compareTo( stringName ) == 0))
		{
			return true;
		}
		
		return false;
	}
}