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

import java.io.IOException;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.HashMap;
import java.util.Map;

import sj.lang.*;
import tools.Reporter;

//import viewer.update2.gui.LoadDialog;
import viewer.update2.*;

/*
This class is the central instance for formatting documents within the 
UpdateViewer2.  

*/
public class HtmlFormatter extends DocumentFormatter
{
	
	private List<String> outputFiles = new ArrayList<String>();

	
	/**
	 * Creates default markup for head, body and end of document. 
	 */
	@Override
	public void createDefaultTemplate(Relation pRelation)
	{
		// head
		StringBuilder sb = new StringBuilder("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" ");
		sb.append(" \"http://www.w3c.org/TR/1999/REC-html401-19991224/loose.dtd\">");
		sb.append("<HTML xmlns=\"http://www.w3.org/1999/xhtml\">");
		sb.append("</HEAD><BODY></p>");
		this.setTemplateHead(sb.toString());
		
		// body
		sb = new StringBuilder();
		for (String attributeName : pRelation.getAttributeNames())
		{
			if (!attributeName.toUpperCase().startsWith("TID"))
			{
				sb.append("<b><i>").append(attributeName).append(":</i></b>");
				sb.append("<<").append(attributeName).append(">>");
				sb.append("</br></p>");
			}
		}
		sb.append("</p><HR/>");
		this.setTemplateBody(sb.toString());
		
		// tail
		sb = new StringBuilder("</BODY></HTML>");
		this.setTemplateTail(sb.toString());
	}
	
	
	/**
	 * Returns page with format markup filled with tuple values and hidden markup.
	 */
	private String fillTemplateBody(Tuple pTuple)
	{
		String page = this.getTemplateBody();
		//Reporter.debug("HtmlFormatter.fillTemplateBody: templateBody=" + page.substring(0,1000));
		
		for (String aliasedName : pTuple.getTypeInfo().getAttributeNames())
		{	
			String relationName = this.getRelationFromAliasedName(aliasedName);
			String attributeName = this.getAttributeFromAliasedName(aliasedName);
			String idName = this.getIdNameFromAliasedName(aliasedName);
			
			StringBuffer sbReplace = new StringBuffer();
			// hidden markup for tracing back 
			sbReplace.append("<div id=\"");
			sbReplace.append(relationName).append(";");
			sbReplace.append(attributeName).append(";");
			sbReplace.append(pTuple.getValueByAttrName(idName));
			sbReplace.append("\">");
			sbReplace.append(pTuple.getValueByAttrName(aliasedName));
			sbReplace.append("</div>");
			page = page.replace("<<" + aliasedName + ">>", sbReplace.toString());
		}
		
		page = page.replaceAll("<<", "&lt;&lt;");
		page = page.replaceAll(">>", " NOT FOUND&gt;&gt;");
		
		//Reporter.debug("HtmlFormatter.fillTemplateBody: page=" + page.substring(0,1000));
		return page;
	}
	
	

	/**
	 * Executes the given query.
	 * Formats the relation.
	 * Checks for templates and creates templates if neccessary.
	 * Writes formatted pages to output directory.
	 */
	@Override
	public boolean format(boolean pSeparatePages) throws Exception
	{
		// execute query
		CommandExecuter commandExecuter = new CommandExecuter();

		if (!commandExecuter.executeCommand(this.getQuery(), SecondoInterface.EXEC_COMMAND_SOS_SYNTAX))
		{
			String errorMessage = commandExecuter.getErrorMessage().toString();
			Reporter.writeError(errorMessage);
			return false;
		}
		
		ListExpr queryResult = new ListExpr();
		queryResult.setValueTo(commandExecuter.getResultList());
		
		if (queryResult == null || queryResult.isEmpty()) 
		{
			return false;
		}
		
		// create relation from query result
		SecondoObject relationSO = new SecondoObject("output", queryResult);
		Relation relation = new Relation();
		relation.readFromSecondoObject(relationSO);
				
		// check template and create default template if necessary
		if (this.getTemplateBody()==null || this.getTemplateBody().isEmpty()
			|| this.getTemplateHead()==null || this.getTemplateHead().isEmpty()
			|| this.getTemplateTail()==null || this.getTemplateTail().isEmpty())
		{
			this.createDefaultTemplate(relation);
		}
		
		if (pSeparatePages)
		{
			// create separate page for each tuple
			for (int i = 0; i<relation.getTupleCount(); i++)
			{
				String filename = this.getOutputDirectory() + relation.getName() + i + ".html";
				FileWriter file = new FileWriter(filename);
				PrintWriter out = new PrintWriter(file);
				out.print(this.getTemplateHead());
				String filledPage = this.fillTemplateBody(relation.getTupleAt(i));
				//Reporter.debug("HtmlFormatter.format: filled page=" + filledPage.substring(0,1000));
				out.print(filledPage);
				out.print(this.getTemplateTail());
				out.close();
				this.outputFiles.add(filename);
			}
		}
		else
		{
			// create document as single page
			String filename = this.getOutputDirectory() + relation.getName() + ".html";
			FileWriter file = new FileWriter(filename);
			PrintWriter out = new PrintWriter(file);
			out.print(this.getTemplateHead());			
			for (int i = 0; i<relation.getTupleCount(); i++)
			{
				String filledPage = this.fillTemplateBody(relation.getTupleAt(i));
				//Reporter.debug("HtmlFormatter.format: filled page=" + filledPage.substring(0,1000));
				out.print(filledPage);
			}
			out.print(this.getTemplateTail());			
			out.close();
			this.outputFiles.add(filename);
		}
		
		return true;
	}
	
	
	/**
	 * Returns path names of all files that were produced during format().
	 */
	@Override
	public List<String> getOutputFiles()
	{
		return this.outputFiles;
	}
	
	
	/**
	 * Returns type of the displayed Document.
	 * This is the Classname prefix (as Html in HtmlFormatter).
	 */
	public String getType() { return "Html"; }
	
} 
