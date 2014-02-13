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
import javax.swing.JOptionPane;

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
	
	public static String FILE_ENDING = "html";

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
		sb.append("</HEAD><BODY>");
		this.templateHead = sb.toString();
		
		// body
		sb = new StringBuilder();
		for (String attributeName : pRelation.getAttributeNames())
		{
			if (!attributeName.toUpperCase().startsWith("TID"))
			{
				sb.append("<p><b><i>").append(attributeName).append(":</i></b>");
				sb.append(" <<").append(attributeName).append(">>");
				sb.append("</br></p>");
			}
		}
		sb.append("<HR/>");
		this.templateBody = sb.toString();
		
		// tail
		sb = new StringBuilder("</BODY></HTML>");
		this.templateTail = sb.toString();
	}
	
	/**
	 * Creates the hidden reference markup for tracing back 
	 */
	public static String createReferenceMarkup(String pRelationName, String pAttrName, String pTupleId)
	{
		StringBuffer sb = new StringBuffer();
		sb.append("<!--");
		sb.append(pRelationName).append(";");
		sb.append(pAttrName).append(";");
		sb.append(pTupleId);
		sb.append("-->");
		return sb.toString();
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
			
			StringBuffer sb = new StringBuffer();
			sb.append(createReferenceMarkup(relationName, attributeName, pTuple.getValueByAttrName(idName)));
			sb.append(pTuple.getValueByAttrName(aliasedName));
			
			page = page.replace("<<" + aliasedName + ">>", sb.toString());
		}
		
		// handle placeholders that could not be filled
		page = page.replaceAll("<<", "&lt;&lt;");
		page = page.replaceAll(">>", " NOT FOUND&gt;&gt;");
		
		//Reporter.debug("HtmlFormatter.fillTemplateBody: page=" + page.substring(0,1000));
		return page;
	}
	
	

	/**
	 * Loads relation by means of given query.
	 * Checks for templates and creates templates if neccessary.
	 * Creates formatted pages from the relation.
	 */
	@Override
	public void format(boolean pSeparatePages, boolean pApplyScript) throws Exception
	{
		// load relation
		long millisStart = System.currentTimeMillis();			
		Relation relation = this.loadRelation();
		long millis = System.currentTimeMillis() - millisStart;
		Reporter.debug("HtmlFormatter.format: relation load time (millis): " + millis);
		
		// check template and create default template if necessary
		if (this.hasTemplates())
		{
			this.createDefaultTemplate(relation);
		}
		
		// create formatted document
		millisStart = System.currentTimeMillis();			
		this.outputPages = new ArrayList<Object>();
		if (pSeparatePages)
		{
			// create separate page for each tuple
			for (int i = 0; i<relation.getTupleCount(); i++)
			{
				StringBuffer sb = new StringBuffer();
				sb.append(this.getTemplateHead());
				sb.append(this.fillTemplateBody(relation.getTupleAt(i)));
				sb.append(this.getTemplateTail());
				this.outputPages.add(sb.toString());
				//Reporter.debug("HtmlFormatter.format: filled page=" + sb.toString().substring(0,100));
			}
		}
		else
		{
			// create document as single page
			StringBuffer sb = new StringBuffer();
			sb.append(this.getTemplateHead());
			for (int i = 0; i<relation.getTupleCount(); i++)
			{
				sb.append(this.fillTemplateBody(relation.getTupleAt(i)));
			}
			sb.append(this.getTemplateTail());
			this.outputPages.add(sb.toString());
			//Reporter.debug("HtmlFormatter.format: filled page=" + sb.toString().substring(0,1000));
		}
		millis = System.currentTimeMillis() - millisStart;
		Reporter.debug("HtmlFormatter.format: create pages time (millis): " + millis);
		
		// save files in output directory
		this.saveOutputPages();
		
		// if document has to be post-processed by a script
		if (pApplyScript)
		{
			// execute the script
			String[] commands = this.readLines(this.getScript());
			this.executeCommands(commands);
			// reload (processed) files from the output directory
			this.readOutputPages();
		}
	}
	
	/**
	 * Returns non-empty lines from the specified file.
	 */
	protected String readFile(File pFile) throws IOException
	{
		FileReader fileReader = new FileReader(pFile);
		BufferedReader bufferedReader = new BufferedReader(fileReader);
		StringBuffer sb = new StringBuffer();
		String line;
		while ((line = bufferedReader.readLine()) != null)
		{
			if (line!=null && !line.isEmpty())
				sb.append(line);
		}
		fileReader.close();		
		return sb.toString();
	}
	
	/**
	 * Returns a list of HTML pages read from the output directory
	 */
	protected void readOutputPages() throws IOException
	{
		this.outputPages.clear();
		File outputDir = new File(this.outputDirectory);
		FileReader fileReader;
		
		for (File file : outputDir.listFiles())
		{
			if (file.isFile() && !file.isHidden() && file.getName().endsWith(this.FILE_ENDING))
			{
				String htmlPage = this.readFile(file);
				this.outputPages.add(htmlPage);
			}
		}
	}
	
	
	
	/**
	 * Saves formatted pages as HTML files. 
	 */
	public void saveOutputPages()
	{
		String baseFileName = JOptionPane.showInputDialog(this.outputPages.size() 
														  + " pages were created and will be saved in "
														  + this.outputDirectory 
														  + ". \nClick Cancel if you do not want to save the output to disk. "
														  + " \nYou may change the base file name: "
														  , "output");
		if (baseFileName != null && !baseFileName.isEmpty())
		{
			long millisStart = System.currentTimeMillis();
			Thread thread = new Thread(new FileSaver(baseFileName));
			thread.run();
			long millis = System.currentTimeMillis() - millisStart;
			Reporter.debug("HtmlFormatter.saveOutputPages: disk writing time (millis): " + millis);
		}
	}
	
	
	class FileSaver implements Runnable
	{
		private String baseName;
		
		FileSaver(String pBaseName)
		{
			this.baseName = pBaseName;
		}
		
		public void run()
		{
			String filename;
			FileWriter fileWriter;
			PrintWriter printWriter;
			String page;
			
			for (int i=0; i<outputPages.size(); i++)
			{
				page = (String)outputPages.get(i);
				filename = getOutputDirectory() + baseName + i + "." + HtmlFormatter.FILE_ENDING;
				try
				{
					fileWriter = new FileWriter(filename);
					printWriter = new PrintWriter(fileWriter);
					printWriter.print((String)page);			
					printWriter.close();
				}
				catch (IOException e)
				{
					Reporter.showError("Error while wrting formatted documents to disk: " + e.getMessage());
					return;
				}
			}
		}
	}
		
} 
