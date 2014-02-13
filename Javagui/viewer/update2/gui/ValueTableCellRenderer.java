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

import java.awt.Color;
import java.awt.Component;
import java.awt.Insets;
import java.awt.Rectangle;
import java.util.List;

import javax.swing.border.Border;
import javax.swing.BorderFactory;
import javax.swing.border.EmptyBorder;
import javax.swing.JTable;
import javax.swing.JTextArea;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableCellRenderer;
import javax.swing.text.BadLocationException;
import javax.swing.text.DefaultHighlighter;
import javax.swing.text.Highlighter;

import tools.Reporter;

import viewer.update2.*;



/**
 * TableCellRenderer for the attribute column of the relation table.
 */
public class ValueTableCellRenderer extends DefaultTableCellRenderer 
{
	private JTable table;
	private JTextArea textArea;
	private Border borderFocussed;
	private Highlighter hiliter;
	private DefaultHighlighter.DefaultHighlightPainter hilitePainter;
	private DefaultHighlighter.DefaultHighlightPainter hilitePainterCurr;

	/**
	 * Constructor
	 */
	public ValueTableCellRenderer()
	{			
		this.textArea = new JTextArea();
		this.textArea.setEditable(true);
		this.textArea.setLineWrap(true);
		this.textArea.setMargin(new Insets(5,5,5,5));
		this.textArea.setWrapStyleWord(true);
		this.textArea.setForeground(Color.BLACK);
		this.borderFocussed = BorderFactory.createLineBorder(Color.BLUE);
		this.hiliter = this.textArea.getHighlighter();
		this.hilitePainter = new DefaultHighlighter.DefaultHighlightPainter(Color.LIGHT_GRAY);
		this.hilitePainterCurr = new DefaultHighlighter.DefaultHighlightPainter(Color.YELLOW);
	}
	
	
	/**
	 * Returns a component that displays an attribute value.
	 */
	public Component getTableCellRendererComponent(
												   JTable pTable, Object pValue,
												   boolean pSelected, boolean pFocussed,
												   int pRow, int pColumn) 
	{
		RelationTableModel rtm = (RelationTableModel)pTable.getModel();
		int state = rtm.getState();
		this.table = pTable;
		
		// background
		if (rtm.isRowDeleted(pRow))
		{
			this.textArea.setBackground(new Color(255, 210, 230));
		}
		else
		{
			if (state==States.UPDATE && pSelected || rtm.isCellChanged(pRow, pColumn))
			{
				this.textArea.setBackground(new Color(210, 230, 255));
			}
			else
			{
				this.textArea.setBackground(Color.WHITE);
			}
		}
			
		// border
		if (pFocussed && !rtm.isRowDeleted(pRow))
		{
			this.textArea.setBorder(BorderFactory.createCompoundBorder(this.borderFocussed, 
								BorderFactory.createEmptyBorder(1,5,1,1)));
		} 
		else	
		{
			this.textArea.setBorder(BorderFactory.createCompoundBorder(
								BorderFactory.createLineBorder(this.textArea.getBackground()), 
								BorderFactory.createEmptyBorder(1,5,1,1)));
		}
		
		this.textArea.setText(pValue.toString());

		// correct row height according to textarea content
		if (pValue!=null && ((String)pValue).length() > 0)
		{
			int width = pTable.getColumnModel().getColumn(pColumn).getWidth();
			this.textArea.setSize(width, Short.MAX_VALUE);
			pTable.setRowHeight(pRow, this.textArea.getPreferredSize().height);
		}
		else
		{
			// explicitly set row height of empty cells to label cell's height plus border
			// otherwise height of empty table rows will shrink to nearly 0 in linux 
			pTable.setRowHeight(pRow, pTable.getRowHeight()+4);
		}
		
		// render search matches
		hiliter.removeAllHighlights();
		
		if (rtm.hasSearchHits())
		{
			List<SearchHit> hits = rtm.getHits(pRow);
			SearchHit currHit = rtm.getHit(rtm.getCurrentHitIndex());

			for (SearchHit sh : hits) 
			{
				try 
				{
					if (currHit!=null && sh.equals(currHit))
					{
						this.hiliter.addHighlight(sh.getStart(), sh.getEnd(), this.hilitePainterCurr);
					}
					else
					{
						this.hiliter.addHighlight(sh.getStart(), sh.getEnd(), this.hilitePainter);
					}
				}  
				catch (Exception e) 
				{
					Reporter.debug("ValueTableCellRenderer.getTableCellRendererComponent: highlighting failed ");
				}
			}
		}
		
		return this.textArea;
	}	
	
	
	public Rectangle getOffset(int pTextPos) throws BadLocationException
	{
		//Reporter.debug("ValueTableCellEditor.getOffset: textarea width is " + this.textArea.getPreferredSize().width);
		//Reporter.debug("ValueTableCellEditor.getOffset: value is " + this.textArea.getText());
		if (pTextPos<0 || pTextPos>=this.textArea.getText().length())
		{
			return new Rectangle(0,0,0,0);
		}
		return this.textArea.modelToView(pTextPos);
	}
	
	
	public void setCaret(int pStart, int pEnd)
	{
		int start = pStart<0? 0 : pStart;
		start = start<this.textArea.getText().length()? start : this.textArea.getText().length();
		int end = pEnd<0? 0 : pEnd;
		end = end<this.textArea.getText().length()? end : this.textArea.getText().length();
		this.textArea.requestFocusInWindow();
		this.textArea.setCaretPosition(start);
		this.textArea.moveCaretPosition(end);
	}
	
}

