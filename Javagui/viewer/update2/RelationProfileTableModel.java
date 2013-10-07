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

package viewer.update2;

import gui.idmanager.*;
import gui.SecondoObject;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;

import java.util.List;

import sj.lang.ListExpr;

import tools.Reporter;


/**
 * This class provides table data displayed in the RelationPanel.
 */

/**
 * TableModel for RelationProfiles
 */
public class RelationProfileTableModel extends AbstractTableModel 
{
	private String[] columnNames = {"Relation", "Filter Expressions", "Project Expressions", "Sort Expressions"};
	private List<RelationProfile> data;
	
	public RelationProfileTableModel(LoadProfile pLoadProfile)
	{
		data = pLoadProfile.getRelations();
	}
	
	public int getColumnCount() 
	{
		return this.columnNames.length;
	}
	
	public int getRowCount() 
	{
		return this.data.size();
	}
	
	public String getColumnName(int pCol) 
	{
		return this.columnNames[pCol];
	}
	
	public Object getValueAt(int pRow, int pCol) 
	{
		String result = "";
		RelationProfile rp = this.data.get(pRow);
		if(rp != null)
		{
			switch (pCol)
			{
				case 0: result = rp.getName(); break;
				case 1: result = rp.getFilterExpressions().toString(); break;
				case 2: result = rp.getProjectExpressions().toString(); break;
				case 3: result = rp.getSortExpressions().toString(); break;
			}
		}
		return result;
	}
	
	public Class getColumnClass(int c) 
	{
		return getValueAt(0, c).getClass();
	}
	
	/*
	 * Don't need to implement this method unless your table's
	 * editable.
	 */
	public boolean isCellEditable(int row, int col) 
	{
		//Note that the data/cell address is constant,
		//no matter where the cell appears onscreen.
		if (col < 1 || col > 3) {
			return false;
		} else {
			return true;
		}
	}
	
	/*
	 * Don't need to implement this method unless your table's
	 * data can change.
	 */
	public void setValueAt(Object value, int row, int col) 
	{
		// TODO
		//this.data[row][col] = value;
		//this.fireTableCellUpdated(row, col);
	}
}

