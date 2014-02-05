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

import javax.swing.JTable;
import javax.swing.JLabel;
import javax.swing.SwingConstants;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableColumnModel;
import javax.swing.table.TableColumn;

import tools.Reporter;

import viewer.update2.*;


/**
 * TableCellRenderer for the attribute column of the relation table.
 */
public class LabelTableCellRenderer extends DefaultTableCellRenderer
{
	private JLabel label;
	
	public LabelTableCellRenderer()
	{			
		this.label = new JLabel();
		this.label.setForeground(Color.BLACK);
        this.label.setOpaque(true);
		this.label.setHorizontalAlignment(SwingConstants.LEFT);
		this.label.setVerticalAlignment(SwingConstants.TOP);
	}
	
	/**
	 * Returns a label that displays an attribute name or tuple id.
	 */
	public Component getTableCellRendererComponent(
												   JTable pTable, Object pValue,
												   boolean pSelected, boolean pFocussed,
												   int pRow, int pColumn) 
	{
		//Component c = super.getTableCellRendererComponent(pTable, pValue, pSelected, pFocussed, pRow, pColumn);
		
		this.label.setText(pValue.toString());
		//this.label.setToolTipText();
        
        // background
        if (((RelationTableModel)pTable.getModel()).isRowDeleted(pRow))
        {
            this.label.setBackground(new Color(255, 210, 230));
        }
        else
        {
            this.label.setBackground(Color.WHITE);
        }
		
		//Reporter.debug("LabelTableCellRenderer.getTableCellRendererComponent for cell (" + pRow + "," + pColumn + ")");
		
		return this.label;
	}
}

