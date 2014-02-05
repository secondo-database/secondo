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
import java.awt.event.InputMethodListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.Rectangle;
import java.util.List;

import javax.swing.border.Border;
import javax.swing.BorderFactory;
import javax.swing.border.EmptyBorder;
import javax.swing.AbstractCellEditor;
//import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTextArea;
import javax.swing.table.TableCellEditor;
import javax.swing.text.BadLocationException;
import javax.swing.text.DefaultHighlighter;
import javax.swing.text.Highlighter;

import tools.Reporter;

import viewer.update2.*;


/**
 * TableCellEditor for the attribute column of the relation table.
 */
public class ValueTableCellEditor extends AbstractCellEditor implements TableCellEditor
{
	private JTable table;
	private JTextArea textArea;
	private Border borderFocussed;
	private Highlighter hiliter;
	private DefaultHighlighter.DefaultHighlightPainter hilitePainter;
	private DefaultHighlighter.DefaultHighlightPainter hilitePainterCurr;
	private KeyAdapter keyListener;
	
	/**
	 * Constructor
	 */
	public ValueTableCellEditor()
	{			
		this.textArea = new JTextArea();
		this.textArea.setEditable(true);
		this.textArea.setLineWrap(true);
		this.textArea.setWrapStyleWord(true);
		this.textArea.setForeground(Color.BLACK);
		this.borderFocussed = BorderFactory.createLineBorder(Color.BLUE);
		this.hiliter = this.textArea.getHighlighter();
		this.hilitePainter = new DefaultHighlighter.DefaultHighlightPainter(Color.LIGHT_GRAY);
		this.hilitePainterCurr = new DefaultHighlighter.DefaultHighlightPainter(Color.YELLOW);
		this.textArea.addKeyListener(new TableCellKeyAdapter());
	}
	
	
	/**
	 * Returns a component in which an attribute value can be edited.
	 */
	public Component getTableCellEditorComponent(
												   JTable pTable, Object pValue,
												   boolean pSelected,
												   int pRow, int pColumn) 
	{
		RelationTableModel rtm = (RelationTableModel)pTable.getModel();
		this.table = pTable;
		
		// background
        if (pSelected || rtm.isCellChanged(pRow, pColumn))
        {
            this.textArea.setBackground(new Color(210, 230, 255));
        }
        else
        {
            this.textArea.setBackground(Color.WHITE);
        }
		
		
		// border
		this.textArea.setBorder(BorderFactory.createCompoundBorder(this.borderFocussed,
				BorderFactory.createEmptyBorder(1,5,1,1)));

		
		this.textArea.setText(pValue.toString());

		// correct row height according to textarea content
		if (pValue!=null && ((String)pValue).length() > 0)
		{
			int width = pTable.getColumnModel().getColumn(pColumn).getWidth();
			this.textArea.setSize(width, Short.MAX_VALUE);
			//this.textArea.setText(pValue.toString());
			pTable.setRowHeight(pRow, this.textArea.getPreferredSize().height);
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
					if (sh.equals(currHit))
					{
						//this.setCaret(sh.getStart(), sh.getEnd());
						this.hiliter.addHighlight(sh.getStart(), sh.getEnd(), this.hilitePainterCurr);
						//Reporter.debug("ValueTableCellEditor.getTableCellRendererComponent: highlighting CURRENT HIT ");
					}
					else
					{
						this.hiliter.addHighlight(sh.getStart(), sh.getEnd(), this.hilitePainter);
					}
				} 
				catch (Exception e) 
				{
					Reporter.debug("ValueTableCellEditor.getTableCellEditorComponent: highlighting failed ");
				}
				
			}
		}
		
		return this.textArea;
	}
		
	public Object getCellEditorValue()
	{
		return this.textArea.getText();
	}
	
	
	public Rectangle getOffset(int pTextPos)
	{
		try
		{
			//Reporter.debug("ValueTableCellEditor.getOffset: textarea width is " + this.textArea.getPreferredSize().width);
			//Reporter.debug("ValueTableCellEditor.getOffset: value is " + this.textArea.getText());
			return this.textArea.modelToView(pTextPos);
		}
		catch (BadLocationException e)
		{
			Reporter.showError("ValueTableCellEditor.getOffset: BadLocation " + pTextPos +  e.getMessage());
			return null;
		}
	}
	
	public void setCaret(int pStart, int pEnd)
	{
		//Reporter.debug("ValueTableCellEditor.setCaretPosition at position " + pStart);
		
		this.textArea.requestFocusInWindow();
		this.textArea.setCaretPosition(pStart);
		this.textArea.moveCaretPosition(pEnd);
		//this.textArea.getCaret().setSelectionVisible(true);
	}
	
	class TableCellKeyAdapter extends KeyAdapter
	{
		public void keyPressed(KeyEvent event)
		{
			// insert line break on ENTER Key
			if(event.getKeyCode()==KeyEvent.VK_ENTER)
			{
				textArea.replaceSelection(System.getProperty("line.separator"));
			}
			//Reporter.debug("ValueTableCellEditor.keyPressed: selectedRow" + table.getSelectedRow());
			
			table.setRowHeight(table.getSelectedRow(), textArea.getPreferredSize().height);			
		}
	}
	
}

