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
import java.util.HashMap;
import java.util.Map;

import gui.idmanager.*;
import gui.SecondoObject;

import sj.lang.ListExpr;

import tools.Reporter;

import viewer.relsplit.InvalidRelationException;
import viewer.update2.*;

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
	private int state;
	
	// contains changes in chronological order
	private List<Change> changes;
	
	// hits from latest search, mapped by rowIndex
	private Map<Integer, List<SearchHit>> searchHits;
	
	public static final int COL_TUPLEID = 0;
	public static final int COL_ATTRNAME = 1;
	public static final int COL_ATTRVALUE = 2;
	
		
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
		this.state = States.LOADED;
		this.changes = new ArrayList<Change>();
			 
		Reporter.debug(this.toString());
	}
	
	
	public void addChange(Change pChange)
	{
		Reporter.debug("RelationTableModel.addChange :" + pChange.toString());
		this.changes.add(pChange);
	}
	
	
	/**
	 * Removes all changes. Returns true if any changes existed.
	 */
	public boolean clearChanges()
	{
		if (this.changes != null && !this.changes.isEmpty())
		{
			this.changes.clear();
		}
		return false;
	}
	
	
	/*
	 * Returns all changes in chronological order.	 
	 */	
	public List<Change> getChanges()
	{
		return this.changes;
	}	

	
	/**
	 * Returns a list of changes, that contain newest value and original value of each changed cell. 
	 */
	public List<Change> getChangesForReset()
	{
		List<Change> result = new ArrayList<Change>();
		Map<Integer, HashMap<String, Change>> updateTuples = new HashMap<Integer, HashMap<String, Change>>();
		
		for (Integer i : updateTuples.keySet())
		{
			for (String s : updateTuples.get(i).keySet())
			{
				result.add(updateTuples.get(i).get(s));
			}
		}
		return result;
	}
	
	
	/*
	 * Returns only valid (=last change of attribute) changes mapped by tuple index and attribute name.
	 * Changes include original values.
	 */	
	public Map<Integer, HashMap<String, Change>> getChangesForUpdate()
	{
		Map<Integer, HashMap<String, Change>> result = new HashMap<Integer, HashMap<String, Change>>();
		
		Integer tid;
		String attrName;
		HashMap<String, Change> mapTuple;		
		
		for (Change ch : this.changes)
		{
			tid = ch.getTupleIndex();
			attrName = ch.getAttributeName();
			
			mapTuple = result.get(tid);
			if (mapTuple == null)
			{
				mapTuple = new HashMap<String, Change>();
			}
			mapTuple.put(attrName, ch);	
			
			if (!mapTuple.isEmpty())
			{
				Change chOld = mapTuple.get(attrName);
				if (chOld == null && ch.changesSameObject(chOld))
				{
					ch.setOldValue(chOld.getOldValue());
				}
				mapTuple.put(attrName, ch);
			}
			
			result.put(tid, mapTuple);
		}
		
		// only debugging
		for (Integer i : result.keySet())
		{
			for (String s : result.get(i).keySet())
			{
				Reporter.debug("RelationPanel.getUpdateTuples result is :" + result.get(i).get(s).toString());
			}
		}
		return result;
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
	 * Returns newest change.	 
	 */
	public Change getLastChange()
	{
		if (this.changes != null && !this.changes.isEmpty()) 
		{
			return this.changes.get(this.changes.size()-1);			
		}
		return null;
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
			
			SecondoObject[] soTuple = this.relation.getTupleAt(tupleIndex);
						
			// get value
			switch (pCol)
			{
				case COL_TUPLEID: 
					SecondoObject last = soTuple[soTuple.length-1];
					result = this.formatter.fromListExprToString(last.toListExpr().second()).trim();
					break;
				case COL_ATTRNAME:
					result = this.attributeNames.get(attrIndex).trim();
					break;
				case COL_ATTRVALUE: 
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
		if (this.state == States.UPDATE)
		{
			return (pCol == COL_ATTRVALUE && !this.isSeparator(pRow));
		}
		return false;
	}
	
	/**
	 * Returns true if rowIndex specifies a separator row (empty row between tuples).
	 */
	private boolean isSeparator(int pRow)
	{
		return (pRow % (this.tupleSize+1)==0);
	}
	
	/**
	 * Returns true if there are uncommitted changes for specified table cell.
	 */
	public boolean isValueChanged(int pRow, int pCol)
	{
		for (Change ch : this.changes)
		{
			if (ch.getRowIndex() == pRow)
			{
				return true;
			}
		}
		return false;
	}
	
		
	/**
	 * Removes change at specified index from change history list.	 
	 */
	public Change removeChange(int pIndex)
	{
		Reporter.debug("RelationTableModel.removeChange :" + pIndex);
		Change result = null;
		
		if (this.changes != null && !this.changes.isEmpty() && pIndex < this.changes.size()) 
		{
			return (Change)this.changes.remove(this.changes.size()-1);			
		}
		
		return result;
	}
	
	
	/**
	 * Removes change at specified index from change history list.	 
	 */
	public boolean removeChange(Change pChange)
	{
		Reporter.debug("RelationTableModel.removeChange :" + pChange.toString());
		boolean result = false;
		
		if (this.changes != null) 
		{
			result = this.changes.remove(pChange);			
		}
		
		return result;
	}
	
	public void setSearchHits(List<SearchHit> pHitlist)
	{
		this.searchHits = new HashMap<Integer, List<SearchHit>>();
		for (SearchHit hit : pHitlist)
		{
			Reporter.debug(hit.toString());

			List<SearchHit> list4row = this.searchHits.get(hit.getRowIndex());
			if (list4row == null)
			{
				list4row = new ArrayList<SearchHit>();
			}
			list4row.add(hit);
			this.searchHits.put(hit.getRowIndex(), list4row);
		}
	}
	
	
	
    /*
	 * Method of interface AbstractTableModel.
     * Change table data.
     */
	@Override
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
			
			//fireTableCellUpdated(pRow, pCol);
		}
    }
	
	public int rowToTupleIndex(int pRow)
	{
		return (pRow / (this.tupleSize+1));
	}
	
	public int rowToAttributeIndex(int pRow)
	{
		return ((pRow % (this.tupleSize+1)) - 1);
	}
	
	/**
	 * 
	 */
	public void setState(int pState)
	{
		this.state = pState;
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
