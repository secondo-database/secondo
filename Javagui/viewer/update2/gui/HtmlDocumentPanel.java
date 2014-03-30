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
import java.awt.Point;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintStream;
import javax.swing.JEditorPane;
import javax.swing.JTextPane;
//import javax.swing.event.CaretEvent;
//import javax.swing.event.CaretListener;
import javax.swing.text.AttributeSet;
import javax.swing.text.BadLocationException;
import javax.swing.text.Element;
import javax.swing.text.html.HTML;
import javax.swing.text.html.HTMLDocument;
import javax.swing.text.html.HTMLEditorKit;
import javax.swing.text.html.HTMLDocument.Iterator;
import java.util.ArrayList;
import java.util.List;

import tools.Reporter;
import viewer.update2.*;
import viewer.update2.format.HtmlFormatter;


/**
 * Panel that displays a formatted HTML document.
 * 
 */
public class HtmlDocumentPanel extends DocumentPanel
{
	List<JTextPane> panes;
	PositionTracker positionTracker;
	
	public HtmlDocumentPanel()
	{
		super();
		this.panes = new ArrayList<JTextPane>();
		this.positionTracker = null;
	}
	

	public void goTo(RelationPosition pPositionInfo)
	{
		this.currentPosition = pPositionInfo;
		String markup = HtmlFormatter.createReferenceMarkup(pPositionInfo.getRelationName(), 
															pPositionInfo.getAttributeName(),
															pPositionInfo.getTupleId());
		/*
		for(JTextPane pane : this.getComponents())
		{
			// TODO
		}
		 */
	}
	
	

	
	public void load(List<Object> pOutputPages) 
	{		
		if (pOutputPages == null || pOutputPages.isEmpty())
		{
			return;
		}
		
		this.positionTracker = new PositionTracker();
		this.currentPosition = null;
		this.setLayout(new GridLayout(pOutputPages.size(), 1));

		String page;
		JTextPane pane;
		HTMLEditorKit editorKit = new HTMLEditorKit();
		
		for (int i=0; i<pOutputPages.size(); i++)
		{			
			long millisStart = System.currentTimeMillis();		
			
			page = (String)pOutputPages.get(i);	
			
			if(i >= this.panes.size())
			{
				pane = new JTextPane();
				pane.setEditable(false);
				pane.setContentType("text/html");
				pane.setEditorKit(editorKit);
				pane.setSize(new Dimension(800,600));
				pane.setOpaque(true);
				pane.addMouseListener(this.positionTracker);
				
				HTMLDocument htmlDoc = (HTMLDocument)pane.getDocument();
				htmlDoc.putProperty("IgnoreCharsetDirective", Boolean.TRUE);
				//htmlDoc.setPreservesUnknownTags(false);	
				this.panes.add(pane);

				long millis = System.currentTimeMillis() - millisStart;
				Reporter.debug("HtmlDocumentPanel.load: TextPane init time (millis): " + millis);
			}
			
			pane = this.panes.get(i);			
			pane.setText(page);
			this.add(pane);
			
			long millis = System.currentTimeMillis() - millisStart;
			Reporter.debug("HtmlDocumentPanel.load: TextPane.settext() time (millis): " + millis);

		}
	}
	
	
	public RelationPosition trackPosition(Point pMousePosition, Object pComponent)
	{
		RelationPosition result = null;
		
		JTextPane sourcePane = (JTextPane)pComponent;
		
		// determine mouse position within document
		int position = sourcePane.viewToModel(pMousePosition);
		
		// determine enclosing block element where mouse action occurred
		HTMLDocument doc = (HTMLDocument) sourcePane.getDocument();				
		HTMLDocument.BlockElement elem = (HTMLDocument.BlockElement)doc.getParagraphElement(position);
		
		// find comment element within block (if any)
		HTMLDocument.RunElement commentElement = null;
		boolean found = false;
		int childIndex = 0;
		while (!found && childIndex < elem.getElementCount())
		{
			if (elem.getElement(childIndex).getName().equals(HTML.Tag.COMMENT.toString()))
			{
				commentElement = (HTMLDocument.RunElement)elem.getElement(childIndex);
				found = true;
			}					
			childIndex++;
		}
		
		if (found && commentElement.getAttribute(HTML.Attribute.COMMENT)!=null)
		{
			// determine position of comment block end within document
			int startOffset = commentElement.getEndOffset() + 1;
			
			// determine cursor position within text content block
			int offset = position - startOffset;
			
			// if mouse action occured not within text content
			// (e.g. when text element is empty)
			offset = offset<0? 0 : offset;
			
			// read position information from comment attribute of the comment element
			String positionString = (String)commentElement.getAttribute(HTML.Attribute.COMMENT);
			List<String> trackingInfo = LoadDialog.splitList(positionString, ";");
			if (trackingInfo != null && !trackingInfo.isEmpty() && trackingInfo.size()==3)
			{
				result = new RelationPosition(trackingInfo.get(0), 
													   trackingInfo.get(1), 
													   trackingInfo.get(2),
													   offset);
				//Reporter.debug("HtmlDocumentPanel.PositionTracker: RelationPosition=" + currentPosition.toString());
			}
		}	
		
		return result;
	}

	
	
	/**
	 * This class translates the mouse-click position in RelationPosition and stores this position.
	 */
	private class PositionTracker extends MouseAdapter
		{
			public void mousePressed(MouseEvent pEvent)
			{				
				// determine RelationPosition within document
				currentPosition = trackPosition(pEvent.getPoint(), pEvent.getSource());
				
				// Construct new MouseEvent that has this DocumentPanel as source.
				// This is for convenience so that the listener can ask the source
				// for RelationPosition.
				MouseEvent me = new MouseEvent(HtmlDocumentPanel.this, pEvent.getID(), pEvent.getWhen(), pEvent.getModifiers(), 
											   pEvent.getX(), pEvent.getY(), pEvent.getClickCount(), pEvent.isPopupTrigger());
				
				for(MouseListener ml: getMouseListeners())
				{
					ml.mousePressed(me);
				}
			}
		}
}
