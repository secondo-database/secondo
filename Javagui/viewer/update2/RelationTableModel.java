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

import viewer.relsplit.InvalidRelationException;

/**
 * This class provides table data displayed in the RelationPanel.
 */

public class RelationTableModel extends AbstractTableModel 
{

	private Relation relation;
	private String relationName;
	private String[] columnNames = {"TupleId", "Name", "Value"};
	private int tuplesize;
	
	
	public RelationTableModel(ListExpr le){
		// TODO
	}
	
	public RelationTableModel(Relation pRelation)
	{
		if(pRelation==null || !pRelation.isInitialized())
		{
			throw(new InvalidRelationException());
		}
		this.relation = pRelation;
		this.relationName = pRelation.getName();
		this.tuplesize = pRelation.getTupleSize();
		//data = new Object[pRelation.getSize() * (pRelation.getTupleSize() +1)][3];
	}
	
	public String getRelationName()
	{
		return this.relationName;
	}
	
	
	/**
	 * Methods of interface AbstractTableModel.
	 *
	 */
	
	public int getColumnCount() 
	{
        return columnNames.length;
    }
	
    public int getRowCount() 
	{
        return data.length;
    }
	
    public String getColumnName(int pCol) 
	{
        return columnNames[pCol];
    }
	
    public Object getValueAt(int pRow, int pCol) 
	{
		String result = "";
		
		// if this is not a Separator Row
		if( ! pRow % (this.tuplesize+1)==0) 
		{
			int seps = pRow / (this.tuplesize+1);
			String S = Rel.get(index-seps-1).getName(); 
			S=S.substring(Rel.toString().length()+2,S.length());
			int lastIndex = S.lastIndexOf("::");
			S = S.substring(0,lastIndex);
			return S;
			
		}
    }
	
    public Class getColumnClass(int pCol) 
	{
        return getValueAt(0, pCol).getClass();
    }
	
    /*
     * Don't need to implement this method unless your table's
     * editable.
     */
    public boolean isCellEditable(int pRow, int pCol) 
	{
        //Note that the data/cell address is constant,
        //no matter where the cell appears onscreen.
        if (pCol < 2) {
            return false;
        } else {
            return true;
        }
    }
	
    /*
     * Don't need to implement this method unless your table's
     * data can change.
     */
    public void setValueAt(Object pValue, int pRow, int pCol) 
	{
        data[pRow][pCol] = pValue;
        //fireTableCellUpdated(row, col);
    }
    
}
