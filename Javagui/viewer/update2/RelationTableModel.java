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
import java.util.Collections;
import java.util.List;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import gui.idmanager.*;
import gui.SecondoObject;

import sj.lang.ListExpr;

import tools.Reporter;

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
	private int[] maxContentLengths;
	private int state;
	private boolean editable;
	
	// contains changes in chronological order
	private List<Change> changes;
    
    // contains tuple ids for deletion in chronological order
	private List<String> deletions;
	
	// hits from latest search, ordered by tow and startindex
	private List<SearchHit> hitList;
	
	private int currHit;
	
	public static final int COL_TUPLEID = 0;
	public static final int COL_ATTRNAME = 1;
	public static final int COL_ATTRVALUE = 2;
	
		
	/**
	 * Constructor.
	 */
	public RelationTableModel(Relation pRelation, boolean pEditable) throws InvalidRelationException
	{
		if(pRelation==null || !pRelation.isInitialized())
		{
			throw(new InvalidRelationException());
		}
		
		this.relation = pRelation;
		this.relationName = pRelation.getName();
		this.tupleSize = pRelation.getTupleSize();
		this.attributeNames = pRelation.getAttributeNames();
		this.changes = new ArrayList<Change>();
		this.deletions = new ArrayList<String>();
		this.hitList = new ArrayList<SearchHit>();
		this.state = pEditable? States.LOADED : States.LOADED_READ_ONLY;
			 
		Reporter.debug(this.toString());
	}
	
	/**
     * Append specified change to change list for this relation.
     */
	public void addChange(Change pChange)
	{
		if (!this.isReadOnly())
		{
			//Reporter.debug("RelationTableModel.addChange :" + pChange.toString());
			this.changes.add(pChange);
		}
	}

	
    /**
     * Marks the tuple corresponding to the specified row index for deletion.
     * Return true if it was not yet marked else  false.
     */
    public boolean addDeletion(int pRow)
    {
		if (!this.isReadOnly() && !isRowSeparator(pRow) )
		{
			String tupleid = (String)this.getValueAt(pRow, this.COL_TUPLEID);
			if (!this.isRowDeleted(pRow))
			{
				this.deletions.add(tupleid);
				//Reporter.debug("RelationTableModel.addDeletion: marked tuple " + tupleid);
				return true;
			}
			//else Reporter.debug("RelationTableModel.addDeletion: already marked tuple " + tupleid);
		}
        return false;
    }
	
	
	
    /**
	 * Inserts a hit in the hitlist, if not already exists.
     */
    public void addHit(SearchHit pHit)
    {
		if (!this.hitList.contains(pHit))
		{
			this.hitList.add(pHit);
			Collections.sort(hitList);
		}
    }
	
	
	/**
	 * Appends a tuple specified by the ListExpression
     */
    public void addTuple(ListExpr pTuple)
    {
		try
		{
			Tuple tuple = this.relation.createEmptyTuple();
			tuple.readValueFromLE(pTuple);
			this.relation.addTuple(tuple);
			fireTableRowsInserted(0, this.getRowsPerTuple());
		}
		catch (InvalidRelationException e)
		{
			Reporter.writeError("RelationTableModel.addTuple: invalid LE is " + pTuple.toString());
		}
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
    
    
    /**
	 * Removes all deletions. Returns true if any deletions existed.
	 */
	public boolean clearDeletions()
	{
		if (this.hasDeletions())
		{
			this.deletions.clear();
            return true;
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
				//Reporter.debug("RelationPanel.getUpdateTuples result is :" + result.get(i).get(s).toString());
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
		
		for (int i = 0; i < this.relation.getTupleCount() * this.getRowsPerTuple(); i++)
		{
			Object o = this.getValueAt(i, pCol);
			
			if (o== null)
			{
				Reporter.debug("RelationPanel.getColumnContent: null object at tupleindex=" + i + ", columnindex=" + pCol);
			}
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
	
	
	/*
	 * Method of interface AbstractTableModel.
	 */
    public String[] getColumnNames() 
	{
        return columnNames;
    }
	
	
	public int getCurrentHitIndex()
	{
		return this.currHit;
	}
    
    public List<String> getDeletions()
    {
        return this.deletions;
    }
	
	/**
     * Returns SearchHit with specified index in this relation.
     */
	public SearchHit getHit(int pIndex)
	{
		if (pIndex >= 0 && this.hitList != null && pIndex < this.hitList.size())
		{
			return this.hitList.get(pIndex);
		}
		return null;
	}
	
	/**
	 * Returns number of all search hits
	 */
	public int getHitCount()
	{
		if (this.hitList != null)
		{
			return this.hitList.size();
		}
		return 0;
	}
		
	
	/**
	 * Returns all SearchHits for the specified row.
	 */
	public List<SearchHit> getHits(int pRow)
	{
		List<SearchHit> result = new ArrayList<SearchHit>();
		if (this.hasSearchHits())
		{
			for (SearchHit hit : this.hitList)
			{
				if (hit.getRowIndex() == pRow)
				{
					result.add(hit);
				}
			}
		}
		return result;
	}
	
	
	/**
	 * Returns newest change.	 
	 */
	public Change getLastChange()
	{
		if (this.hasChanges())
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
	
	
	public int getRow(String pTupleId, String pAttributeName)
	{
		int tupleIndex = this.relation.getTupleIndex(pTupleId);
		int attributeIndex = this.relation.getAttributeNames().indexOf(pAttributeName);
		int result = tupleIndex*this.getRowsPerTuple() + attributeIndex +1;
		return result;
	}
	
	/*
	 * Method of interface AbstractTableModel.
	 */
    public int getRowCount() 
	{
		// add (empty) extra row as separator between tuples
        return this.relation.getTupleCount() * this.getRowsPerTuple();
    }
	
	
	public int getRowsPerTuple()
	{
		return (this.tupleSize+1);
	}
	

    /**
     * Returns state.
     * Used in TableCellEditor/-Renderer.
     * @see viewer.update2.States
     */
	public int getState()
	{
		return this.state;
	}
	
	
	/*
	 * Method of interface AbstractTableModel.
	 * Returns String representation of Tuple ID for column 1, 
	 * String representation of attribute name for column 2,
	 * formatted String representation of attribute value for column 3.
	 */
    public Object getValueAt(int pRow, int pCol) 
	{
		String result = " ";
		
		// if this is not a Separator Row
		if(!this.isRowSeparator(pRow)) 
		{
			// get Tuple value
			int tupleIndex = this.rowToTupleIndex(pRow);
			int attrIndex = this.rowToAttributeIndex(pRow);
			
			Tuple tuple = this.relation.getTupleAt(tupleIndex);
			
			// get value
			switch (pCol)
			{
				case COL_TUPLEID: 
					result = tuple.getID();
					break;
				case COL_ATTRNAME:
					result = this.attributeNames.get(attrIndex);
					break;
				case COL_ATTRVALUE: 
					result = tuple.getValueAt(attrIndex);
					break;
				default: 
					result = "Fehler";
			}
		}
		//Reporter.debug("RelationTableModel.getValueAt(" + pRow + ", " + pCol + ": " + result);		
		return result;
    }
	
    
    public boolean hasChanges()
    {
        return (this.changes != null && !this.changes.isEmpty());
    }
    
    
    public boolean hasDeletions()
    {
        return (this.deletions != null && !this.deletions.isEmpty());
    }
	
	public boolean hasSearchHits()
    {
        return (this.hitList != null && !this.hitList.isEmpty());
    }
	
    
	
	/*
	 * Method of interface AbstractTableModel.
     * Don't need to implement this method unless your table's
     * editable.
     */
    public boolean isCellEditable(int pRow, int pCol) 
	{
		if (this.state == States.UPDATE || this.state == States.INSERT)
		{
			int attrIndex = this.rowToAttributeIndex(pRow);			
			String attrName = this.relation.getAttributeNames().get(attrIndex);
			
			if (attrIndex != this.relation.getTypeInfo().getTidIndex() 
				&& !this.relation.isAttributeReadOnly(attrName))
			{
				return (pCol == COL_ATTRVALUE && !this.isRowSeparator(pRow));
			}
		}
		return false;
	}
    
    /**
	 * Returns true if there are uncommitted changes for specified table cell.
	 */
	public boolean isCellChanged(int pRow, int pCol)
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
	 * Returns true if relation is read-only (no edit function)
	 */
	public boolean isReadOnly()
	{
		//Reporter.debug("RelationTableModel.isReadOnly: " + (this.state == States.LOADED_READ_ONLY));
		return (this.state == States.LOADED_READ_ONLY);
	}
	
    
    /**
	 * Returns true if the tuple this row belongs to has been marked for deletion.
	 */
	public boolean isRowDeleted(int pRow)
	{
		if (!this.isRowSeparator(pRow))
		{		
			String tupleid = (String)this.getValueAt(pRow, this.COL_TUPLEID);
			
			if (tupleid != null && (tupleid.length() > 0) && this.deletions.contains(tupleid))
			{
				return true;
			}
		}

		return false;
	}
	
    
	/**
	 * Returns true if rowIndex specifies a separator row (empty row between tuples).
	 */
	private boolean isRowSeparator(int pRow)
	{
		return (pRow % this.getRowsPerTuple() == 0);
	}

		
	/**
	 * Removes change at specified index from change history list.	 
	 */
	public Change removeChange(int pIndex)
	{
		//Reporter.debug("RelationTableModel.removeChange :" + pIndex);
		Change result = null;
		
		if (this.hasChanges() && pIndex < this.changes.size())
		{
			return (Change)this.changes.remove(this.changes.size()-1);			
		}		
		return result;
	}
	
	
	/**
	 * Removes specified change and returns TRUE if it existed.
	 */
	public boolean removeChange(Change pChange)
	{
		//Reporter.debug("RelationTableModel.removeChange :" + pChange.toString());
		boolean result = false;
		
		if (this.hasChanges())
		{
			result = this.changes.remove(pChange);			
		}		
		return result;
	}

	/**
	 * Removes specified SearchHit and returns TRUE if it existed.
	 */
	public boolean removeHit(SearchHit pHit)
	{
		//Reporter.debug("RelationTableModel.removeHit :" + pHit.toString());
		boolean result = false;
		
		if (this.hasSearchHits())
		{
			int delIndex = this.hitList.indexOf(pHit);

			result = this.hitList.remove(pHit);		
			
			if (this.currHit > delIndex  || this.currHit >= this.getHitCount())
			{
				this.currHit--;
			}
		}

		return result;
	}
	
    
	/**
	 * Removes last deletion from deletion history list, if any.
     * Returns TRUE if deletion existed.
	 */
	public boolean removeLastDeletion()
	{
		if (this.hasDeletions())
		{
            //Reporter.debug("RelationTableModel.removeDeletion: index is " + (this.deletions.size()-1));

			this.deletions.remove(this.deletions.size()-1);
            return true;
		}
        return false;
	}
	
	/**
	 * Removes Tuple from Relation and notifies table of deleted rows.
	 */
	public void removeTuple(String pTupleId)
	{
		this.relation.removeTupleByID(pTupleId);
		fireTableRowsDeleted(0, this.getRowsPerTuple());
	}
	
	
	public int rowToTupleIndex(int pRow)
	{
		return (pRow / this.getRowsPerTuple());
	}
	
	public int rowToAttributeIndex(int pRow)
	{
		return ((pRow % this.getRowsPerTuple()) - 1);
	}
		
	/**
	 * Builds map for searchhits by row index.
	 * Resets index for currently displayed searchhit.  
	 */
	public void setSearchHits(List<SearchHit> pHitlist)
	{
		if (pHitlist == null || pHitlist.isEmpty())
		{
			this.hitList.clear();
			this.currHit = -1;
		}
		else
		{
			this.hitList = pHitlist;
			this.currHit = 0;
		}
	}
	
    /**
     * Sets the hit index if valid.
     */
	public void setCurrentHitIndex(int pIndex)
	{
		if (pIndex >= 0 && pIndex < this.hitList.size())
		{
			this.currHit = pIndex;
		}
	}
	
	
	/*
	 * Method of interface AbstractTableModel.
     * Change table data.
     */
	@Override
    public void setValueAt(Object pValue, int pRow, int pCol) 
	{
		//Reporter.debug("RelationTableModel.setValueAt: " + pRow + ", " + pCol);
		
		if(!this.isReadOnly() && isCellEditable(pRow, pCol)) 
		{
			int tupleIndex = this.rowToTupleIndex(pRow);
			int attrIndex = this.rowToAttributeIndex(pRow);
									
			this.relation.setValueAt(tupleIndex, attrIndex, (String)pValue);
			
			fireTableCellUpdated(pRow, pCol);
		}
    }

	/**
     * Sets the state.
	 * Does not change state if model was initialized with States.LOADED_READ_ONLY.
	 * @see viewer.update2.States
	 */
	public void setState(int pState)
	{
		if (!this.isReadOnly())
		{
			this.state = pState;
		}
	}
    
	
    
	public String toString()
	{
		StringBuffer sb = new StringBuffer("[RelationTableModel]: ");
		sb.append(", relationName: ").append(relationName);
		sb.append(", tupleCount: ").append(this.relation.getTupleCount());
		sb.append(", tupleSize: ").append(tupleSize);
		sb.append(this.relation.toString());
		return sb.toString();
	}
}
