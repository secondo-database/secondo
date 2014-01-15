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
public class DocumentFormatter
{
	
	private String contentType;
	private String formatScript;
	private String formatTemplate;
	private String outputDirectory;
	// Placeholdername -> Field qualifier (Relationname.Attributename)
	private Map<String,FieldMapping> fieldMap;
	private boolean success;
	
	public DocumentFormatter(String pContentType, 
							 String pFormatFields, 
							 String pFormatScript, 
							 String pFormatTemplate, 
							 String pOutputDirectory) throws Exception
	{
		this.formatScript = pFormatScript;
		this.formatTemplate = pFormatTemplate;
		this.outputDirectory = pOutputDirectory;
		this.contentType = pContentType;
		this.fieldMap = new HashMap<String,FieldMapping>();		
		this.readFormatFields(pFormatFields);
		this.success = false;
	}
	
	public void format() throws Exception
	{
		this.success = false;
		List<String> scriptlines;
		
		String template = getFormatTemplateText();
		Reporter.debug("DocumentFormatter.format: template=" + template);

		// read script from file
		FileReader fr = new FileReader(this.formatScript);
		BufferedReader br = new BufferedReader(fr);
		StringBuffer sb = new StringBuffer();
		String zeile = "";
		while( (zeile = br.readLine()) != null )
		{
			zeile = zeile.trim();
			if (zeile.length() > 0 && !zeile.startsWith("#"))
			{
				sb.append(zeile);
			}
		}
		br.close();
		scriptlines = LoadDialog.splitList(sb.toString(), ";");
		Reporter.debug("DocumentFormatter.format: scriptlines=" + scriptlines);
		
		// execute script
		CommandExecuter commandExecuter = new CommandExecuter();
		ListExpr scriptResult = null;
		for(String command : scriptlines)
		{
			if (commandExecuter.executeCommand(command, SecondoInterface.EXEC_COMMAND_SOS_SYNTAX))
			{
				scriptResult = new ListExpr();
				scriptResult.setValueTo(commandExecuter.getResultList());
				//Reporter.debug("loadRelation: resultLE=" + scriptResult.toString());
			}
			else
			{
				String errorMessage = commandExecuter.getErrorMessage().toString();
				Reporter.writeError(errorMessage);
				success = false;
				return;
			}
		}

		// create relation from script result
		if (scriptResult == null || scriptResult.isEmpty()) return;
		
		SecondoObject relationSO = new SecondoObject("output", scriptResult);
		Relation relation = new Relation();
		relation.readFromSecondoObject(relationSO);
		
		// create page for each tuple
		for (int i = 0; i<relation.getTupleCount(); i++)
		{
			Tuple tuple = relation.getTupleAt(i);
			String page = template;
			
			for (String fieldName : this.fieldMap.keySet())
			{
				FieldMapping mapping = fieldMap.get(fieldName);
				StringBuffer sbReplace = new StringBuffer();
				sbReplace.append("<!--###").append(mapping.relationName);
				sbReplace.append(";").append(mapping.attributeName).append("###-->");
				sbReplace.append(tuple.getValueByAttrName(mapping.attributeName));
				page = page.replace("<<" + fieldName + ">>", sbReplace.toString());
			}
			
			FileWriter file = new FileWriter(this.outputDirectory + i + ".html");
			PrintWriter out = new PrintWriter(file);
			out.print(page);
			out.close();
			//Reporter.debug("DocumentFormatter.format: page=" + page);
		}
		
		success = true;
	}
	
	public String getContentType(){	return this.contentType;	}
	public String getFormatScript(){ return this.formatScript;	}
	public String getFormatTemplate(){	return this.formatTemplate;	}
	public String getOutputDirectory(){	return this.outputDirectory;	}
	
	public List<String> getOutputFiles()
	{
		List<String> filenames = new ArrayList<String>();

		if (success) 
		{
			File dir = new File (this.outputDirectory) ;
			File[] files = dir.listFiles();
			
			for (int i=0; i<files.length; i++)
			{
				File file = files[i];
				if (file.isFile() && !file.isHidden())
				{
					filenames.add(file.getPath());
				}
			}
		}
		return filenames;
	}
	
	
	public String getFormatTemplateText() throws IOException
	{
		if (this.formatTemplate == null || this.formatTemplate.length() == 0)
		{
			return this.createDefaultTemplate();
		}
		
		// read template from file
		FileReader fr = new FileReader(this.formatTemplate);
		BufferedReader br = new BufferedReader(fr);
		StringBuffer sb = new StringBuffer();
		String zeile = "";
		while( (zeile = br.readLine()) != null )
		{
			sb.append(zeile);
		}
		br.close();
		
		return sb.toString();
	}
	

	private void readFormatFields(String pFormatFields) throws Exception
	{
		List<String> mappings = LoadDialog.splitList(pFormatFields, ";");
		for (String mapStr : mappings)
		{
			List<String> comps = LoadDialog.splitList(mapStr, ",");
			if (comps.size()!=3)
			{
				throw new Exception("Invalid String for FormatFields");
			}
			else
			{
				fieldMap.put(comps.get(0).trim(), 
							 new FieldMapping(comps.get(0).trim(), comps.get(1).trim(), comps.get(2).trim()));
			}
		}
	}
	
	
	private String createDefaultTemplate()
	{
		StringBuilder sb = new StringBuilder("<!DOCTYPE HTML PUBLIC ");
		sb.append(" \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3c.org/TR/1999/REC-html401-19991224/loose.dtd\">");
		sb.append("<HTML xmlns=\"http://www.w3.org/1999/xhtml\"></HEAD><BODY>");
		for (String placeholder : this.fieldMap.keySet())
		{
			sb.append("<h1>").append(placeholder).append("</h1>");
			sb.append("<p>");
			sb.append("<<").append(placeholder).append(">>");
			sb.append("</p>");
		}
		sb.append("</BODY></HTML>");
		return sb.toString();
	}
	

	class FieldMapping
	{
		public String fieldName;
		public String relationName;
		public String attributeName;
		
		public FieldMapping(String pFieldName, String pRelationName, String pAttributeName)
		{
			this.fieldName = pFieldName;
			this.relationName = pRelationName;
			this.attributeName = pAttributeName;
		}
	}
 
} 
