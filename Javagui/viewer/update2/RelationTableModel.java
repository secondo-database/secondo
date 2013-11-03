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

import java.util.ArrayList;
import java.util.List;

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
	private List<String> attributeNames;
	private int tupleSize;
	private int tupleCount;
	private int[] maxContentLengths;
	private AttributeFormatter formatter;
		
	/**
	 * Constructor.
	 */
	public RelationTableModel(Relation pRelation) throws InvalidRelationException
	{
		if(pRelation==null || !pRelation.isInitialized())
		{
			throw(new InvalidRelationException());
		}
		
		this.formatter = new AttributeFormatter(); 
		this.relation = pRelation;
		this.relationName = pRelation.getName();
		this.tupleSize = pRelation.getTupleSize();
		this.tupleCount = pRelation.getTupleCount();
		this.attributeNames = pRelation.getAttributeNames();
			 
		Reporter.debug(this.toString());
	}
			
	/**
	 * Method of interface AbstractTableModel.
	 *
	 */
    public Class getColumnClass(int pCol) 
	{
        return getValueAt(0, pCol).getClass();
    }
	
	/**
	 * Returns a List of String representations of all the values in the specified column.
	 */ 
	public List<String> getColumnContent(int pCol)
	{
		List<String> result = new ArrayList<String>();
		
		for (int i = 0; i < this.tupleCount * (tupleSize+1); i++)
		{
			Object o = this.getValueAt(i, pCol);
			result.add(o.toString());
		}
		
		return result;
	}
	
	/*
	 * Method of interface AbstractTableModel.
	 */
	public int getColumnCount() 
	{
        return columnNames.length;
    }
	
	
	/*
	 * Method of interface AbstractTableModel.
	 */
    public String getColumnName(int pCol) 
	{
        return columnNames[pCol];
    }
	

	/**
	 * Returns the relation name.
	 */
	public String getRelationName()
	{
		return this.relationName;
	}
	
	/*
	 * Method of interface AbstractTableModel.
	 */
    public int getRowCount() 
	{
		// add (empty) extra row as separator between tuples
        return this.tupleCount * (this.tupleSize + 1);
    }
	
	/*
	 * Method of interface AbstractTableModel.
	 * Returns String representation of Tuple ID for column 1, 
	 * String representation of attribute name for column 2,
	 * formatted String representation of attribute value for column 3.
	 */
    public Object getValueAt(int pRow, int pCol) 
	{
		//Reporter.debug("RelationTableModel.getValueAt: " + pRow + ", " + pCol);
		
		String result = " ";
		
		// if this is not a Separator Row
		if( !this.isSeparator(pRow)) 
		{
			// get Tuple value
			int tupleIndex = this.rowToTupleIndex(pRow);
			int attrIndex = this.rowToAttributeIndex(pRow);
			
			//SecondoObject soTuple = this.relation.getTupleNo(tupleIndex);
			SecondoObject[] soTuple = this.relation.getTupleAt(tupleIndex);
						
			// get value
			switch (pCol)
			{
				case 0: 
					//result = soTuple.getID().toString();
					SecondoObject last = soTuple[soTuple.length-1];
					result = this.formatter.fromListExprToString(last.toListExpr().second()).trim();
					break;
				case 1:
					result = this.attributeNames.get(attrIndex).trim();
					break;
				case 2: 
					ListExpr rest = soTuple[attrIndex].toListExpr();
					//Reporter.debug("RelationTableModel.getValueAt: " + rest.toString());
					/*for (int i = 0; i<attrIndex; i++)
					{
						rest = rest.rest();
					}*/
					result = this.formatter.fromListExprToString(rest.second()).trim();
					break;
				default: 
					result = "fehler";
			}
		}
		
		return result;
    }
	
	/*
	 * Method of interface AbstractTableModel.
     * Don't need to implement this method unless your table's
     * editable.
     */
    public boolean isCellEditable(int pRow, int pCol) 
	{
        return (pCol == 2 && !this.isSeparator(pRow));
	}
	
	/**
	 * Returns true if rowIndex specifies a separator row (empty row between tuples).
	 */
	private boolean isSeparator(int pRow)
	{
		return (pRow % (this.tupleSize+1)==0);
	}
	
	
    /*
	 * Method of interface AbstractTableModel.
     * Change table data.
	 * TODO set Relation Data
     */
    public void setValueAt(Object pValue, int pRow, int pCol) 
	{
		Reporter.debug("RelationTableModel.setValueAt: " + pRow + ", " + pCol);
				
		if(isCellEditable(pRow, pCol)) 
		{
			// get Tuple value
			int tupleIndex = this.rowToTupleIndex(pRow);
			int attrIndex = this.rowToAttributeIndex(pRow);
						
			ListExpr type = ListExpr.symbolAtom(this.relation.getAttributeTypes().get(attrIndex));
			ListExpr value = ListExpr.textAtom(pValue.toString());
			
			ListExpr le = ListExpr.twoElemList(type, value);
			String name = this.relation.getAttributeNames().get(attrIndex)+"::"+type+"::"+pValue.toString();
			SecondoObject SO = new SecondoObject(name, le);

			this.relation.setSecondoObject(tupleIndex, attrIndex, SO);
		}
		//fireTableCellUpdated(row, col);
    }
	
	public int rowToTupleIndex(int pRow)
	{
		return (pRow / (this.tupleSize+1));
	}
	
	public int rowToAttributeIndex(int pRow)
	{
		return ((pRow % (this.tupleSize+1)) - 1);
	}
	
	
    
	public String toString()
	{
		StringBuffer sb = new StringBuffer("[RelationTableModel]: ");
		sb.append(", relationName: ").append(relationName);
		sb.append(", tupleCount: ").append(tupleCount);
		sb.append(", tupleSize: ").append(tupleSize);
		sb.append(this.relation.toString());
		return sb.toString();
	}
}
