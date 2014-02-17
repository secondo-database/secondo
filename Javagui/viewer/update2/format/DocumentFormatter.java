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

*/

package viewer.update2.format;

import gui.SecondoObject;

import java.awt.Component;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.PrintWriter;

import java.util.ArrayList;
import java.util.List;
import java.util.HashMap;
import java.util.Map;

import sj.lang.*;
import tools.Reporter;

import viewer.update2.gui.LoadDialog;
import viewer.update2.*;

/*
This class is the central instance for formatting documents within the 
UpdateViewer2.  

*/
public abstract class DocumentFormatter 
{
	protected String type;
//	protected String errorMessage;
	protected String aliases;
	protected String query;
	protected String script;
	protected String templateHead;
	protected String templateBody;
	protected String templateTail;
	protected String outputDirectory;
	protected List<Object> outputPages;
	// alias -> relationname
	protected Map<String,String> aliasMap;	

	
	/**
	 * Fills templateHead, templateBody and templateTail with default values.
	 */
	public abstract void createDefaultTemplate(Relation relation);
	
	/**
	 * Executes formatQuery and fills templates with query result.
	 * Resulting output is accessible by method getOutputPages().
	 */
	public abstract void format(boolean separatePages, boolean pApplyScript) throws Exception;
		
	
	/**
	 * Returns formatter of class '<pFormatType>Formatter' if that class exists in package viewer.update2.format.
	 */
	public static DocumentFormatter createFormatter(String pFormatType) throws Exception
	{
		if (!getFormatTypes().contains(pFormatType))
		{
			throw new Exception(pFormatType 
								+ "Formatter is not implemented. DocumentFormatters are available for these FormatTypes: " 
								+ getFormatTypes());
		}
		
		// try to instatiate specific formatter class
		String formatterName = "viewer.update2.format." + pFormatType.trim() + "Formatter";
		Class formatterClass = Class.forName(formatterName);
		Object o = formatterClass.newInstance();
		if(!(o instanceof DocumentFormatter))
		{
			throw new Exception("Found class does not extend abstract class DocumentFormatter. ");
		}
		
		DocumentFormatter formatter = (DocumentFormatter)o;
		formatter.type = pFormatType;
		return formatter;
	}
	
	
	/**
	 * Executes a sequence of commands.
	 * Returns output of all commands as one string.
	 */
	protected String executeCommands(List<String> pCommands, String pWorkDirectory) throws Exception
	{
		File workDirectory = new File(pWorkDirectory);
		StringBuffer commandOutput = new StringBuffer();
		
		for (String cmd : pCommands)
		{
			Process process = Runtime.getRuntime().exec(cmd, null, workDirectory);
			InputStreamReader inputReader;
			
			int returnValue = process.waitFor();
			
			if (returnValue != 0)
			{
				inputReader = new InputStreamReader(process.getErrorStream());
			}
			else
			{
				inputReader = new InputStreamReader(process.getInputStream());
			}
			
			commandOutput.append("\n\"").append(cmd).append("\" terminated ");
			commandOutput.append((returnValue==0)? "ok" : "with error").append(": \n");
			
			// read process (or error) output
			BufferedReader input = new BufferedReader(inputReader);
			String line = null;
			while ((line = input.readLine()) != null) 
			{
				commandOutput.append(line);
				commandOutput.append("\n");
			}

			if (returnValue != 0)
			{
				throw new Exception(commandOutput.toString());
			}
		}

		return commandOutput.toString();
	}
	
	
	/**
	 * Returns a list of the prefixes of currently implemented Formatters.
	 * Each of the returned prefixes can be used as parameter 'FormatType'.
	 */
	public static List<String> getFormatTypes()
	{
		List<String> result = new ArrayList<String>();
		
		File dir = new File ("viewer/update2/format/");
		File[] files = dir.listFiles();
		
		for (int i=0; i<files.length; i++)
		{
			File file = files[i];
			if (file.isFile() && !file.isHidden() 
				&& file.getName().endsWith("Formatter.class") 
				&& !file.getName().contains("Document"))
			{
				String name = file.getPath();
				name = name.substring(file.getParent().length()+1, name.length());
				name = name.substring(0, name.indexOf("Formatter.class"));
				result.add(name);
			}
		}
		return result;
	}
	
	
//	public String getAliases(){ return this.aliases;	}
//	public String getErrorMessage(){ return this.errorMessage;	}
	public String getQuery(){ return this.query;	}
	public String getScript(){ return this.script;	}
	public String getTemplateBody(){ return this.templateBody;	}
	public String getTemplateHead(){ return this.templateHead;	}
	public String getTemplateTail(){ return this.templateTail;	}
	public String getType(){	return this.type;	}
	public String getOutputDirectory() { return this.outputDirectory; }
	
	/**
	 * Returns the formatted document pages that are to be displayed in the appropriate DocumentPanel.
	 */
	public List<Object> getOutputPages() { return this.outputPages; }

	
	/**
	 * Determines the alias part in the specified string 
	 * (i.e. the substring following the last underscore) and returns
	 * the corresponding relation name.
	 */
	protected String getRelationFromAliasedName(String pAliasedAttributeName)
	{
		String result = "";
		int start = pAliasedAttributeName.lastIndexOf("_");
		if (start >= 0)
		{
			String alias = pAliasedAttributeName.substring(start+1, pAliasedAttributeName.length());
			result = this.aliasMap.get(alias);
		}
		//Reporter.debug("DocumentFormatter.getRelationFromAliasedName: relation=" + result);
		return result;
	}
	
	/**
	 * Returns the attribute part in the specified string 
	 * (i.e. substring before the last underscore).
	 */
	protected String getAttributeFromAliasedName(String pAliasedAttributeName)
	{
		String result = "";
		int end = pAliasedAttributeName.lastIndexOf("_");
		if (end >= 0)
		{
			result = pAliasedAttributeName.substring(0, end);
		}
		//Reporter.debug("DocumentFormatter.getAttributeFromAliasedName: attribute=" + result);
		return result;
	}
	
	/**
	 * Returns the name of the ID attribute of the  relation
	 */
	protected String getIdNameFromAliasedName(String pAliasedAttributeName)
	{
		String result = "TID";
		int start = pAliasedAttributeName.lastIndexOf("_");
		if (start >= 0)
		{
			result += pAliasedAttributeName.substring(start, pAliasedAttributeName.length());
		}
		//Reporter.debug("DocumentFormatter.getIdNameFromAliasedName: id=" + result);
		return result;
	}
	
	/**
	 * Returns TRUE if all templates are filled (no validity check!).
	 */
	public boolean hasTemplates()
	{
		return (this.templateBody==null || this.templateBody.isEmpty()
				|| this.templateHead==null || this.templateHead.isEmpty()
				|| this.templateTail==null || this.templateTail.isEmpty());
	}
	
	/**
	 * Returns TRUE if load profile all necessary information
	 * for constructing a valid formatter.
	 */
	public void initialize(String pAliases, String pQuery, String pOutputDirectory, String pScript,  
						   String pTemplateHead, String pTemplateBody, String pTemplateTail)
	throws Exception
	{
		StringBuilder errorMessage = new StringBuilder();
		if (pQuery==null || pQuery.isEmpty())
		{
			errorMessage.append("Please specify query that fetches the document relation. \n");
			Reporter.showError(errorMessage.toString());
		}
		if (pOutputDirectory==null || pOutputDirectory.isEmpty())
		{
			errorMessage.append("Please specify path of output directory. \n");
		}
		if (pAliases==null || pAliases.isEmpty())
		{
			errorMessage.append("Please list each relation name and its alias in the format query. \n\n");
		}
		if (!errorMessage.toString().isEmpty())
		{
			throw new Exception("DocumentFormatter.initialize: " + errorMessage.toString());
		}
		
		this.initializeAliasMap(pAliases);
		this.query = pQuery;
		this.script = pScript;
		this.templateHead = pTemplateHead;
		this.templateBody = pTemplateBody;
		this.templateTail = pTemplateTail;
		this.outputDirectory = pOutputDirectory;
 		if (!this.outputDirectory.endsWith("/"))
		{
			this.outputDirectory += "/";
		}
	}
	
	
	/**
	 * Creates a map of all aliases as keys and the corresponding relations
	 * from the Aliases in a LoadProfile.
	 * alias -> relation name
	 */
	protected void initializeAliasMap(String pAliases) throws Exception
	{
		this.aliasMap = new HashMap<String,String>();
		
		List<String> mappings = LoadDialog.splitList(pAliases, ";");
		for (String mapStr : mappings)
		{
			mapStr = mapStr.trim();
			if (mapStr == null || mapStr.isEmpty())
			{
				this.aliasMap.clear();
				throw new Exception("DocumentFormatter.initializeAliasMap: Invalid alias \"" + mapStr + "\"");
			}
			
			List<String> alias = LoadDialog.splitList(mapStr, " ");
			if (alias.size()!=2 || alias.get(0).trim().isEmpty() || alias.get(1).trim().isEmpty())
			{
				this.aliasMap.clear();
				throw new Exception("DocumentFormatter.initializeAliasMap: Invalid alias \"" + mapStr + "\"");
			}
			this.aliasMap.put(alias.get(1).trim(), alias.get(0).trim());
		}
	}
	
	
	protected Relation loadRelation() throws Exception
	{
		CommandExecuter commandExecuter = new CommandExecuter();
		
		if (!commandExecuter.executeCommand(this.getQuery(), SecondoInterface.EXEC_COMMAND_SOS_SYNTAX))
		{
			throw new Exception("DocumentFormatter: Error while trying to load relation from database: " 
								+ commandExecuter.getErrorMessage().toString());
		}
		
		ListExpr queryResult = new ListExpr();
		queryResult.setValueTo(commandExecuter.getResultList());
		
		if (queryResult == null || queryResult.isEmpty()) 
		{
			throw new Exception("DocumentFormatter: could not load relation from database (query result null or empty).");
		}
		
		SecondoObject relationSO = new SecondoObject("output", queryResult);
		Relation relation = new Relation();
		relation.readFromSecondoObject(relationSO);	
		
		return relation;
	}
	
	/**
	 * Returns non-empty lines from the specified file.
	 */
	protected List<String> readLines(String pFileName) throws IOException
	{
		List<String> result = new ArrayList<String>();
		FileReader fileReader = new FileReader(pFileName);
		BufferedReader bufferedReader = new BufferedReader(fileReader);
		String line;
		while ((line = bufferedReader.readLine()) != null)
		{
			line = line.trim();
			if (line!=null && !line.isEmpty() && !line.startsWith("#"))
			{
				result.add(line);
			}
		}
		fileReader.close();
		return result;
	}
	
	/*
	public void setType(String pFormatType){ this.type = pFormatType; }
	public void setQuery(String pFormatQuery){ this.query = pFormatQuery; }
	public void setScript(String pFormatScript){ this.script = pFormatScript; }
	public void setTemplateBody(String pFormatTemplate){ this.templateBody = pFormatTemplate;	}
	public void setTemplateHead(String pFormatTemplate){ this.templateHead = pFormatTemplate;	}
	public void setTemplateTail(String pFormatTemplate){ this.templateTail = pFormatTemplate;	}
	 
	public void setAliases(String pRelationAliases)
	{ 
		this.aliases = pRelationAliases;
		this.aliasMap = this.getAliasMap();
	}	
	public void setOutputDirectory(String pOutputDirectory)
	{
		this.outputDirectory = pOutputDirectory;
 		if (!this.outputDirectory.endsWith("/"))
		{
			this.outputDirectory += "/";
		}
	}
	*/
	
} 
