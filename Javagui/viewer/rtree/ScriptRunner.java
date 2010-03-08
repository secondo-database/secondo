package viewer.rtree;

import gui.SecondoObject;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import sj.lang.ListExpr;
import tools.Reporter;
import viewer.SecondoViewer;

/**
 * ScriptRunner can be used to run Secondo commands from a script file.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 18.12.2009
 */
public class ScriptRunner {

	SecondoViewer viewer;
	SecondoManager secondoManager;
	
	// constructors
	
	/**
	 * Creates a new ScriptRunner object.
	 * @param viewer Viewer to receive script commands
	 */
	public ScriptRunner(SecondoViewer viewer) 
	{
		this.viewer = viewer;
		secondoManager = new SecondoManager();
	}
	
	// public methods
	
	/**
	 * Runs a script.
	 * @param filename The complete path and name of the script file
	 */
	public void runScript(String filename) 
	{
		BufferedReader reader;
	
		try 
		{
			reader = new BufferedReader( new FileReader( filename) );
			
			// read all lines
			int lineNo = 1;
			String cmd = reader.readLine();
			while ( cmd != null)
			{
				// remark or empty line
				if (!isCommand(cmd))
				{
					lineNo++;
					cmd = reader.readLine();
					continue;
				}
				
				
				// submit results to viewer
				if (isAddComand(cmd))
				{
					String addCmd = retrieveCmdFromAdd(cmd);
					ListExpr result = secondoManager.sendCommand(addCmd, "Line: " + lineNo);
					SecondoObject so = new SecondoObject("Result line" + lineNo, result);
					
					if (viewer.canDisplay(so))
					{
						viewer.addObject(so);
					}
				}
				else 
				{
					// command without results
					secondoManager.sendCommand(cmd, "Zeile Nr: " + lineNo);
				}
				
				
				lineNo++;
				cmd = reader.readLine();
			}
			reader.close();
		}
		
		catch (FileNotFoundException e)
		{
			String errorMessage = "Error opening script file. ";
			errorMessage = "File " + filename + " does not exist.";	
			Reporter.showError(errorMessage);
		}	
		
		catch (IOException e) 
		{
			String errorMessage = "Error reading script file. ";
			errorMessage = "File " + filename + " could not be read from.";	
			Reporter.showError(errorMessage);			
		}
	}
	
	// private methods
	
	/**
	 * Checks if a line contains a command or remark.
	 * @param line Line to check
	 * @return True if the line contains a command, otherwise false.
	 */
	private boolean isCommand(String line) 
	{
		if ((line.compareTo("") == 0) || (line.charAt(0) == '#'))
		{
			return false;
		}
		
		return true;
	}
	
	
	/**
	 * Checks if a line contains an 'add' command.
	 * @param line Line to check
	 * @return True if the line contains an 'add' command, otherwise false.
	 */
	private boolean isAddComand(String cmd) 
	{
		if (cmd.startsWith("add")) 
		{
			return true;
		}
		
		return false;
	}

	
	/**
	 * Parses the Secondo command from an add command.
	 * @param addCmd Comand to parse
	 * @return Secondo command
	 */
	private String retrieveCmdFromAdd(String addCmd)
	{
		return (String) addCmd.subSequence(3, addCmd.length());
	}
}
