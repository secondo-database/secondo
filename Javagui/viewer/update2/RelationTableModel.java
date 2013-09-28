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

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;

import gui.idmanager.*;
import gui.SecondoObject;

import sj.lang.ListExpr;


/**
 * This class provides table data displayed in the RelationPanel.
 */

public class RelationTableModel extends AbstractTableModel {

	private String relationName;
	private String[] columnNames = {"TupleId", "Name", "Value"};
	private Object[][] data;
	
	
	public RelationTableModel(ListExpr le){
		// TODO
	}
	
	public RelationTableModel(Relation pRelation){
		// TODO
	}
	
	public String getRelationName(){
		return this.relationName;
	}
	
	public int getColumnCount() {
        return columnNames.length;
    }
	
    public int getRowCount() {
        return data.length;
    }
	
    public String getColumnName(int col) {
        return columnNames[col];
    }
	
    public Object getValueAt(int row, int col) {
        return data[row][col];
    }
	
    public Class getColumnClass(int c) {
        return getValueAt(0, c).getClass();
    }
	
    /*
     * Don't need to implement this method unless your table's
     * editable.
     */
    public boolean isCellEditable(int row, int col) {
        //Note that the data/cell address is constant,
        //no matter where the cell appears onscreen.
        if (col < 2) {
            return false;
        } else {
            return true;
        }
    }
	
    /*
     * Don't need to implement this method unless your table's
     * data can change.
     */
    public void setValueAt(Object value, int row, int col) {
        data[row][col] = value;
        //fireTableCellUpdated(row, col);
    }
    
}
