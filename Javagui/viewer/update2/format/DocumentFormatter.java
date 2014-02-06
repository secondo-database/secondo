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
	private String type;	
	private String aliases;
	private String query;
	private String script;
	private String templateHead;
	private String templateBody;
	private String templateTail;
	private String outputDirectory;
	
	private Map<String,String> aliasMap;	// alias -> relationname
	
	/**
	 * Executes formatQuery, fills templates with query result 
	 * and saves the resulting page(s) to outputDirectory.
	 */
	public abstract boolean format(boolean separatePages) throws Exception;
	
	/**
	 * Returns path names of formatted document files.
	 */
	public abstract List<String> getOutputFiles();
	
	/**
	 * Fills templateHead, templateBody and templateTail with default values.
	 */
	public abstract void createDefaultTemplate(Relation relation);
	
	/**
	 * Returns formatter of class '<pFormatType>Formatter' if that class exists in package viewer.update2.format.
	 */
	public static DocumentFormatter createFormatter(String pFormatType) throws Exception
	{
		String formatterName = "viewer.update2.format." + pFormatType.trim() + "Formatter";
		Object o;
		// try to instatiate specific formatter class
		try
		{
			Class formatterClass = Class.forName(formatterName);
			o = formatterClass.newInstance();
		}
		catch (Exception e)
		{
			throw new Exception("DocumentFormatter \"" + formatterName 
								+ "\" not implemented. DocumentFormatters are implemented for these FormatTypes: " 
								+ getFormatTypes());
		}
		if(!(o instanceof DocumentFormatter))
		{
			throw new Exception("Found class does not extend the abstract class DocumentFormatter. ");
		}
		return (DocumentFormatter)o;
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
	
	
	public String getAliases(){ return this.aliases;	}
	public String getQuery(){ return this.query;	}
	public String getScript(){ return this.script;	}
	public String getTemplateBody(){ return this.templateBody;	}
	public String getTemplateHead(){ return this.templateHead;	}
	public String getTemplateTail(){ return this.templateTail;	}
	public String getType(){	return this.type;	}
	public String getOutputDirectory() { return this.outputDirectory; }
	
	
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
	
	protected Map<String,String> getAliasMap()
	{
		Map<String,String> result = new HashMap<String,String>();
		
		List<String> mappings = LoadDialog.splitList(this.aliases, ";");
		for (String mapStr : mappings)
		{
			mapStr = mapStr.trim();
			if (mapStr != null && mapStr.length() > 0)
			{
				List<String> alias = LoadDialog.splitList(mapStr, " ");
				if (alias.size()==2 && alias.get(0).trim().length() > 0 && alias.get(1).trim().length() > 0)
				{
					result.put(alias.get(1).trim(), alias.get(0).trim());
				}
			}
		}
		return result;
	}
	

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
} 
