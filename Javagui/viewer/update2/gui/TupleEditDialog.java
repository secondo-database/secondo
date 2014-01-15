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

import components.ChangeValueEvent;
import components.ChangeValueListener;
import components.LongScrollBar;

import gui.SecondoObject;
import gui.idmanager.ID;
import gui.idmanager.IDManager;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.ArrayList;
import java.util.List;
import java.util.HashMap;
import java.util.Map;

import javax.swing.CellEditor;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.table.TableColumn;

import tools.Reporter;

import viewer.update2.*;


/**
 * Dialog to pick relations and specify restrictions.	 
 */
public class TupleEditDialog extends JDialog implements PropertyChangeListener
{
	private JPanel pnlButtons;
	private JButton btSave;
	private JButton btCancel;
	private JTable table;
	private Relation relation;
	private String oldEditCellValue;
	
	public TupleEditDialog(Relation pRelation, Tuple pEditTuple, ActionListener pActionListener, int pState, String pTitle)
	{
		this.getContentPane().setLayout(new BorderLayout());
		//this.setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
		this.setDefaultCloseOperation(DISPOSE_ON_CLOSE);
		this.setSize(600,400);
		this.setModal(true);
		this.setTitle(pTitle);
		
		// buttons
		this.pnlButtons = new JPanel();
		if (pRelation.getName().equals(UpdateViewerController.RELNAME_LOAD_PROFILES_HEAD))
		{
			this.btSave = new JButton(UpdateViewerController.CMD_SAVE_PROFILE);
		}
		if (pRelation.getName().equals(UpdateViewerController.RELNAME_LOAD_PROFILES_POS))
		{
			this.btSave = new JButton(UpdateViewerController.CMD_SAVE_PROFILEPOS);
		}
		this.btSave.addActionListener(pActionListener);
		this.pnlButtons.add(btSave);
		this.btCancel = new JButton(UpdateViewerController.CMD_CANCEL);
		this.btCancel.addActionListener(pActionListener);
		this.pnlButtons.add(btCancel);
		
		// table
		this.relation = pRelation.createEmptyClone();
		try
		{
			this.relation.addTuple(pEditTuple);
			/*if (this.relation.getName().equals(UpdateViewerController.RELNAME_LOAD_PROFILES_HEAD))
			{
				this.relation.setAttributeReadOnly("ProfileName");
			}
			if (this.relation.getName().equals(UpdateViewerController.RELNAME_LOAD_PROFILES_POS))
			{
				this.relation.setAttributeReadOnly("ProfileName");
				this.relation.setAttributeReadOnly("RelName");
			}*/
			
			RelationTableModel dtm = new RelationTableModel(this.relation, true);
			dtm.setState(pState);

			this.table = new JTable(dtm);
			this.table.addPropertyChangeListener(this);
		}
		catch (InvalidRelationException e)
		{
			// do nothing
			// should always be ok as Tuples were cloned from valid relation
		}

		
		// set column width and renderers
		TableColumn column = table.getColumnModel().getColumn(0);
		column.setMinWidth(20); 
		column.setMaxWidth(20); 
		column.setCellRenderer(new LabelTableCellRenderer());
		
		column = this.table.getColumnModel().getColumn(1);
		column.setMinWidth(100); 
		column.setMaxWidth(100); 
		column.setCellRenderer(new LabelTableCellRenderer());
		
		column = this.table.getColumnModel().getColumn(2);
		column.setCellRenderer(new ValueTableCellRenderer());
		column.setCellEditor(new ValueTableCellEditor());
		
		// 
		this.getContentPane().add(this.table, BorderLayout.CENTER);
		this.getContentPane().add(this.pnlButtons, BorderLayout.SOUTH);
	}
	
	public Tuple getEditTuple()
	{
		Tuple result = null;
		if (this.relation != null && this.relation.getTupleCount()>0)
		{
			result = relation.getTupleAt(0);
		}
		return result;
	}
	
	/*
	 * Returns values to be updated.
	 */	
	public Map<Integer, HashMap<String, Change>> getUpdateTuples()
	{
		return ((RelationTableModel)this.table.getModel()).getChangesForUpdate();
	}
	
	/**
	 *  Implemention of PropertyChangeListener interface.
	 */
	public void propertyChange(PropertyChangeEvent e)
	{
		//  A table cell has started/stopped editing
		if ("tableCellEditor".equals(e.getPropertyName()))
		{
			if (this.table.isEditing())
			{
				// Editing started
				// Save old value of currently active editable table cell.
				this.oldEditCellValue = (String)this.table.getCellEditor().getCellEditorValue();
			}
			else
			{
				// Editing stopped
				int row = this.table.getEditingRow();
				int col = this.table.getEditingColumn();
				
				String newValue = (String)this.table.getCellEditor(row,col).getCellEditorValue();
				
				if (!newValue.equals(this.oldEditCellValue))
				{
					RelationTableModel rtm = (RelationTableModel)this.table.getModel();
					
					// write changed cell value back into table model
					rtm.setValueAt(newValue, row, col);
					
					// create Change for update or undo actions
					int tupleIndex = Integer.valueOf((String)rtm.getValueAt(row,0));
					int attributeIndex = rtm.rowToAttributeIndex(row);
					String attributeName = this.relation.getAttributeNames().get(attributeIndex);
					String attributeType = this.relation.getAttributeTypes().get(attributeIndex);
					
					
					Change change = new Change(tupleIndex, attributeIndex, row,
											   attributeName, attributeType, 
											   this.oldEditCellValue, newValue);
					
					
					rtm.addChange(change);
					
					Reporter.debug("TupleEditDialog.processEditingStopped: new value of table cell (" + row + ", " + col + ") is " + newValue) ;			
				}
			}
		}
	}
	
	/*
	 The last cell the user edited before he pressed "commit" is usually not considered
	 because "tableChanged" will only be called, if he pressed "return" or selected a different cell
	 of the JTable before "commiting". Therefore this method takes over the value of the
	 last edited cell that was not taken into consideration yet.	 
	 */
	public void takeOverLastEditing()
	{
		if (this.table.isEditing()) 
		{
			int editedRow = this.table.getEditingRow();
			int editedColumn = this.table.getEditingColumn();
			CellEditor editor = this.table.getCellEditor(editedRow, editedColumn);
			editor.stopCellEditing();
		}			
	}	
}
