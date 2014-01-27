//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science,
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package  viewer.update2.gui;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import javax.swing.JEditorPane;
import javax.swing.JTextPane;
//import javax.swing.event.CaretEvent;
//import javax.swing.event.CaretListener;
import javax.swing.text.AttributeSet;
import javax.swing.text.Element;
import javax.swing.text.html.HTML;
import javax.swing.text.html.HTMLDocument;
import javax.swing.text.html.HTMLEditorKit;
import java.util.ArrayList;
import java.util.List;

import tools.Reporter;
import viewer.update2.*;

/**
 * Panel that displays a formatted HTML document.
 * 
 */
public class HtmlDocumentPanel extends DocumentPanel
{
	PositionTracker positionTracker;
	
	public HtmlDocumentPanel()
	{
		super();
		this.positionTracker = null;
	}
	
	public void loadFiles(List<String> pFileNames) throws IOException, FileNotFoundException
	{		
		if (pFileNames == null || pFileNames.isEmpty())
		{
			return;
		}
		
		this.positionTracker = new PositionTracker();
		this.currentPosition = null;

		this.setLayout(new GridLayout(pFileNames.size(), 1));

		for (String path: pFileNames)
		{
			// read formatted file
			FileReader fr = new FileReader(path);
			BufferedReader br = new BufferedReader(fr);
			StringBuffer sb = new StringBuffer();
			String zeile = "";
			while( (zeile = br.readLine()) != null )
			{
				sb.append(zeile);
			}
			br.close();
			String page = sb.toString();
			page = this.clean(page);
			//Reporter.debug("HtmlFormatter.getDocumentDisplay: page=" + page);
			
			JTextPane pane = new JTextPane();
			pane.setEditable(false);
			pane.setContentType("text/html");
			HTMLEditorKit editorKit = new HTMLEditorKit();
			pane.setEditorKit(editorKit);
			pane.setSize(new Dimension(800,600));
			//pane.setMinimumSize(new Dimension(800,600));
			//pane.setMaximumSize(new Dimension(800,600));
			pane.setOpaque(true);
			//pane.addCaretListener(this);
			pane.addMouseListener(this.positionTracker);
			
			
			HTMLDocument htmlDoc = (HTMLDocument)pane.getDocument();
			htmlDoc.putProperty("IgnoreCharsetDirective", Boolean.TRUE);
			//htmlDoc.setPreservesUnknownTags(false);
			
			pane.setText(page);
			this.add(pane);
		}
	}
	
	/**
	 * Removes tags that should not be displayed in EditorPane:
	 * Comments like <!-- this comment -->
	 */
	private String clean(String pPage)
	{
		String result = pPage.replaceAll("<![ \r\n\t]*(--([^-]|[\r\n]|-[^-])*--[ \r\n\t]*)>" , "");
		//String result = pPage.replaceAll("<!--[ ([^-]|[\r\n]|-[^-])]*-->" , "");		
		return result;
	}
	
	
	private class PositionTracker extends MouseAdapter
		{
			public void mousePressed(MouseEvent pEvent)
			{
				JTextPane sourcePane = (JTextPane)pEvent.getSource();
				int position = sourcePane.viewToModel(pEvent.getPoint());
				HTMLDocument doc = (HTMLDocument) sourcePane.getDocument();				
				HTMLDocument.BlockElement elem = (HTMLDocument.BlockElement)doc.getParagraphElement(position).getParentElement();								  
				String elemName = elem.getName();
				if (elemName.equals("div") && elem.getAttribute(HTML.Attribute.ID)!=null)
				{
					String id = elem.getAttribute(HTML.Attribute.ID).toString();
					//Reporter.debug("HtmlDocumentPanel.PositionTracker: found div id=" + id);
					//Reporter.debug("HtmlDocumentPanel.PositionTracker: position=" + position);
					int startOffset = elem.getStartOffset();
					//Reporter.debug("HtmlDocumentPanel.PositionTracker: start offset=" + startOffset);
					int offset = position - startOffset;
					//Reporter.debug("HtmlDocumentPanel.PositionTracker: offset within field=" + offset);
					
					List<String> trackingInfo = LoadDialog.splitList(id, ";");
					if (trackingInfo != null && !trackingInfo.isEmpty() && trackingInfo.size()==3)
					{
						currentPosition = new RelationPosition(trackingInfo.get(0), 
																	trackingInfo.get(1), 
																	trackingInfo.get(2),
																	offset);
						Reporter.debug("HtmlDocumentPanel.PositionTracker: RelationPosition=" + currentPosition.toString());
					}
				}
				else
				{
					currentPosition = null;
				}
				
				for(MouseListener ml: getMouseListeners())
				{
					ml.mousePressed(pEvent);
				}
			}
		}
}
