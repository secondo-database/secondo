package viewer.rtree;

import gui.MainWindow;
import sj.lang.*;
import tools.Reporter;
import java.util.*;


/**
 * SecondoManager provides an interface to run Secondo commands.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 18.12.2009
 */
public class SecondoManager {

	UpdateInterface updateInterface;
	
	// constructors
	
	/**
	 * Creates a new Secondo object.
	 */
	public SecondoManager() 
	{
		updateInterface = MainWindow.getUpdateInterface();
	}
		
	// public methods
	
	/**
	 * Checks if a database is currently open.
	 * @return True if a database is currently open, otherwise false.
	 */
	public boolean isDatabaseOpen() 
	{
		String listObjects = "list objects";
		ListExpr result = sendCommand(listObjects, "Is Database Open", false);
				
		if ( result == null)
		{
			return false;
		}
		
		return true;	
	}
	
	/**
	 * Opens a database. 
	 * @param database Database name
	 */
	public void openDatabase(String database)
	{	
		String cmd = "open database " + database;
		sendCommand( cmd, "Open Database");
	}
	
	/**
	 * Executes the 'list objects' command.
	 * @return Result list expression
	 */
	public ListExpr listObjects() 
	{
		String cmd = "list objects";
		return sendCommand( cmd, "List Objects");
	}
	
	/**
	 * Retrieves a list of objects of the given type.
	 * @param type Type
	 * @return List of object names
	 */
	public Vector<String> listObjectsOfType(String type)
	{
		ListExpr allObjects = listObjects();
		NestedListParser parser = new NestedListParser();
		
		return parser.getObjectsOfType(allObjects, type);
	}
	
	/**
	 * Retrieves a list of objects of the given type.
	 * @param type Type
	 * @param addTypeToName Determines whether ': <type>' is to be added to the name
	 * @return List of object names
	 */
	public Vector<String> listObjectsOfType(String type, boolean addTypeToName)
	{
		ListExpr allObjects = listObjects();
		NestedListParser parser = new NestedListParser();
		
		return parser.getObjectsOfType(allObjects, type, true);
	}

	/**
	 * Retrieves the tuple attributes of the given relation.
	 * @param relation Relation name
	 * @return List of attribute names
	 */
	public Vector<String> retrieveTupleAttributesOfRelation(String relation)
	{
		Vector<String> returnValue = new Vector<String>();
		String cmd = "query " + relation + " feed head[0] consume";
		ListExpr rel = sendCommand(cmd, "Get Relation Head");
		
		NestedListParser parser = new NestedListParser();
		
		return parser.extractTupleAttributes(rel);
	}
	
	/**
	 * Sends a command to Secondo.
	 * @param cmd Command text
	 * @param description Description that will be displayed in case of errors
	 * @return Result list expression
	 */	
	public ListExpr sendCommand(String cmd, String description)
	{
		return sendCommand(cmd, description, true);
	}
	
	/**
	 * Sendet einen Befehl an Secondo
	 * @param cmd Der Befehl. Syntax siehe Secondo User Manual
	 * @param description Kurze Beschreibung. Wird bei Fehlern ausgeben.
	 * @param showError Determines whether error are to be displayed
	 * @return Das Ergebniss des Queries, bei Fehlern null
	 */
	public ListExpr sendCommand(String cmd, String description, boolean showError) 
	{
		if (!updateInterface.isInitialized()) 
		{
			if (showError) 
			{ 	
				showNotInitializedError(); 
			}
			return null;
		}
	
		
		ListExpr resultList = new ListExpr();
		IntByReference errorCode = new IntByReference();
		IntByReference errorPos = new IntByReference();
		StringBuffer errorMessage = new StringBuffer();
		
		updateInterface.secondo(cmd, resultList, errorCode, errorPos, errorMessage);
			
		if ((errorCode.value != 0))
		{
			if (showError) 
			{
				showError(cmd, description, errorCode, errorPos, errorMessage);
			}
			
			return null;
		}

		return resultList;
	}
	
	// private methods
	
	/**
	 * Displays a default error message in case the interface is not initialized.
	 */
	private void showNotInitializedError()
	{
		String errorInfo = "Update interface not initialized.\n";
		errorInfo += "Please check the connection to the Secondo Server.";
		
		Reporter.showError(errorInfo);
	}
	
	/**
	 * Displays the given error message.
	 * @param cmd Command that failed
	 * @param description Command description
	 * @param errorCode Error code returned by Secondo
	 * @param errorPos Error position in the command text
	 * @param errorMessage Error text returned by Secondo
	 */
	private void showError(String cmd, String description, IntByReference errorCode,
								IntByReference errorPos, StringBuffer errorMessage)
	{
		String errorInfo = "Error in " + description + "\n";
		errorInfo += "Secondo Command: " + cmd + "\n";
		errorInfo += "Error Code: " + errorCode.value + "\n";
		errorInfo += "Error Position: " + errorPos.value + "\n"; 
		errorInfo += "Error Message: " + errorMessage + "\n";

		Reporter.showError(errorInfo);
	}
}
