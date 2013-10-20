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

import javax.swing.border.Border;
import javax.swing.BorderFactory;
import javax.swing.border.EmptyBorder;
import javax.swing.AbstractCellEditor;
import javax.swing.JTable;
import javax.swing.JTextArea;
import javax.swing.table.TableCellEditor;

/**
 * TableCellEditor for the attribute column of the relation table.
 */
public class ValueTableCellEditor extends AbstractCellEditor implements TableCellEditor
{
	private JTextArea textArea;
	private Border borderFocussed;
	
	public ValueTableCellEditor()
	{			
		this.textArea = new JTextArea();
		this.textArea.setEditable(true);
		this.textArea.setLineWrap(true);
		this.textArea.setWrapStyleWord(true);
		this.textArea.setForeground(Color.BLACK);
		
		this.borderFocussed = BorderFactory.createLineBorder(Color.BLUE);
	}
	
	public Object getCellEditorValue()
	{
		return this.textArea.getText();
	}
	
	/**
	 * Returns a component in which an attribute value can be edited.
	 */
	public Component getTableCellEditorComponent(
												   JTable pTable, Object pValue,
												   boolean pSelected,
												   int pRow, int pColumn) 
	{
		//Component c = super.getTableCellRendererComponent(pTable, pValue,
		//												  pSelected, pFocussed, pRow, pColumn);
		
		if (pSelected)
		{
			this.textArea.setBackground(new Color(210, 230, 255));
		} else	{
			this.textArea.setBackground(Color.WHITE);
		}
		
		this.textArea.setBorder(BorderFactory.createMatteBorder(1,5,1,1, this.textArea.getBackground()));

		
		// set text and correct row height according to textarea content
		int width = pTable.getColumnModel().getColumn(2).getWidth();
		this.textArea.setSize(width, Short.MAX_VALUE);
		this.textArea.setText(pValue.toString());
		pTable.setRowHeight(pRow, this.textArea.getPreferredSize().height);

		//this.textArea.setToolTipText("test");
		return this.textArea;
	}
	
	// TODO
	// Method to set highlighting color
	// Method to set font
	
	
}

