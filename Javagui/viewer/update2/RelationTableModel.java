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

import tools.Reporter;

import viewer.relsplit.InvalidRelationException;

/**
 * This class provides table data displayed in the RelationPanel.
 */

public class RelationTableModel extends AbstractTableModel 
{

	private Relation relation;
	private String relationName;
	private String[] columnNames = {"TupleId", "Name", "Value"};
	private String[] attributeNames;
	private int tupleSize;
	private int tupleCount;
	
	
	public RelationTableModel(ListExpr le){
		// TODO
	}
	
	public RelationTableModel(Relation pRelation) throws InvalidRelationException
	{
		if(pRelation==null || !pRelation.isInitialized())
		{
			throw(new InvalidRelationException());
		}
		
		this.relation = pRelation;
		this.relationName = pRelation.getName();
		this.tupleSize = pRelation.getTupleSize();
		this.tupleCount = pRelation.getTupleCount();
		this.attributeNames = pRelation.getAttributeNames();
		
		Reporter.debug(this.toString());
	}
	
	public String getRelationName()
	{
		return this.relationName;
	}
	
	private boolean isSeparator(int pRow)
	{
		return (pRow % (this.tupleSize+1)==0);
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
		// add (empty) extra row as separator between tuples
        return this.tupleCount * (this.tupleSize + 1);
    }
	
    public String getColumnName(int pCol) 
	{
        return columnNames[pCol];
    }
	
    public Object getValueAt(int pRow, int pCol) 
	{
		Reporter.debug("RelationTableModel.getValueAt: " + pRow + ", " + pCol);
		
		String result = "";
		
		// if this is not a Separator Row
		if( !this.isSeparator(pRow)) 
		{
			// get Tuple value
			int seps = pRow / (this.tupleSize+1);
			int attrIndex = (pRow-seps-1) % this.tupleSize;
			SecondoObject soTuple = this.relation.getTupleNo((pRow-seps-1)/this.getColumnCount());
			
			// get value
			switch (pCol)
			{
				case 0: 
					result = soTuple.getID().toString();
					break;
				case 1:
					result = this.attributeNames[attrIndex-1];
					break;
				case 2: 
					// TODO
					String str = soTuple.getName();
					ListExpr leTuple = soTuple.toListExpr();
					str = str.substring(this.relation.toString().length()+2, str.length());
					int lastIndex = str.lastIndexOf("::");
					result = str.substring(0,lastIndex);	
					break;
				default: 
					result = "fehler";
			}
		}
		
		return result;
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
        if (pCol < 3 || this.isSeparator(pRow)) 
		{
            return false;
        } 
		else {
            return true;
        }
    }
	
    /*
     * Don't need to implement this method unless your table's
     * data can change.
     */
    public void setValueAt(Object pValue, int pRow, int pCol) 
	{
		// TODO
		// this.relation.setValueAt(pValue, pRow, pCol);
		//fireTableCellUpdated(row, col);
    }
    
	public String toString()
	{
		StringBuffer sb = new StringBuffer("[RelationTableModel]: ");
		sb.append("columnNames: ").append(columnNames);
		sb.append(", relationName: ").append(relationName);
		sb.append(", attributeNames: ").append(attributeNames);
		sb.append(", tupleCount: ").append(tupleCount);
		sb.append(", tupleSize: ").append(tupleSize);
		return sb.toString();
	}
}
