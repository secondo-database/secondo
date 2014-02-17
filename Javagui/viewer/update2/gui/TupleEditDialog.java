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
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.ArrayList;
import java.util.List;
import java.util.HashMap;
import java.util.Map;
import java.util.ResourceBundle;

import javax.swing.BorderFactory;
import javax.swing.CellEditor;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JRootPane;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTextArea;
import javax.swing.JWindow;
import javax.swing.SwingUtilities;
import javax.swing.table.TableColumn;

import tools.Reporter;

import viewer.update2.*;


/**
 * Dialog to pick relations and specify restrictions.	 
 */
public class TupleEditDialog extends JDialog implements PropertyChangeListener
{
	private JButton btSave;
	private JButton btCancel;
	private JTable table;
	private Relation relation;
	private String oldEditCellValue;
	private InfoPanel infoPanel;
	
	public TupleEditDialog(Relation pRelation, Tuple pEditTuple, ActionListener pActionListener, int pState, String pTitle)
	{
		this.setDefaultCloseOperation(DISPOSE_ON_CLOSE);
		this.setSize(600,400);
		this.setModal(true);
		this.setTitle(pTitle);
		
		// help information
		this.infoPanel = new InfoPanel(this);
		
		// buttons		
		if (pRelation.getName().equals(UpdateViewerController.RELNAME_LOAD_PROFILES_HEAD))
		{
			this.btSave = new JButton(UpdateViewerController.CMD_SAVE_PROFILE);
		}
		
		if (pRelation.getName().equals(UpdateViewerController.RELNAME_LOAD_PROFILES_POS))
		{
			this.btSave = new JButton(UpdateViewerController.CMD_SAVE_PROFILEPOS);
		}
		
		this.btSave.addActionListener(pActionListener);
		this.btCancel = new JButton(UpdateViewerController.CMD_CANCEL);
		this.btCancel.addActionListener(pActionListener);
		
		// table
		this.relation = pRelation.createEmptyClone();
		try
		{
			this.relation.addTuple(pEditTuple);
			
			RelationTableModel dtm = new RelationTableModel(this.relation, true);
			dtm.setState(pState);
			
			this.table = new JTable(dtm);
			
			// table column width and renderers
			TableColumn column = table.getColumnModel().getColumn(0);
			column.setMinWidth(20); 
			column.setMaxWidth(20); 
			column.setCellRenderer(new LabelTableCellRenderer());
			
			column = this.table.getColumnModel().getColumn(1);
			column.setMinWidth(200); 
			column.setMaxWidth(200); 
			column.setCellRenderer(new LabelTableCellRenderer());
			
			column = this.table.getColumnModel().getColumn(2);
			column.setCellRenderer(new ValueTableCellRenderer());
			column.setCellEditor(new ValueTableCellEditor());
			
			// listener
			this.table.addPropertyChangeListener(this);
			this.table.addMouseListener(new MouseAdapter()
										{
										public void mousePressed(MouseEvent pEvent)	{
										if (SwingUtilities.isRightMouseButton(pEvent))
										showInfo(pEvent.getPoint());
										else 
										hideInfo();
										}
										});
			
			// tooltip
			this.table.setToolTipText("Right-click for help");
		}
		catch (InvalidRelationException e)
		{
			// do nothing
			// should always be ok as Tuples were cloned from valid relation
		}
		
		// table scrolling
		JScrollPane scp = new JScrollPane(table);
		scp.getVerticalScrollBar().setUnitIncrement(10);
		
		// component arrangement	
		JPanel pnlButtons = new JPanel();
		pnlButtons.add(btSave);
		pnlButtons.add(btCancel);
		
		JPanel pnl = new JPanel(new BorderLayout());
		pnl.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));
		pnl.add(scp, BorderLayout.CENTER);
		pnl.add(pnlButtons, BorderLayout.SOUTH);
		this.getContentPane().add(pnl);
	}
	
	/**
	 * Returns the displayed tuple (with its probably changed values)
	 */
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
			this.btSave.setEnabled(true);

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
					int tupleIndex;
					try
					{
						tupleIndex = Integer.valueOf((String)rtm.getValueAt(row,0));
					}
					catch (NumberFormatException ex)
					{
						tupleIndex = -1;
					}
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
	
	private void hideInfo()
	{
		this.infoPanel.setVisible(false);
	}
	
	private void showInfo(Point pPoint)
	{
		int row = this.table.rowAtPoint(pPoint);
		int attrIndex = ((RelationTableModel)this.table.getModel()).rowToAttributeIndex(row);
		String attrName = this.relation.getAttributeNames().get(attrIndex);
		
		this.infoPanel.setTitle(attrName + " (" + this.relation.getAttributeTypes().get(attrIndex) + "):");
		this.infoPanel.setInfo(ResourceBundle.getBundle("viewer.update2.gui.help").getString(this.relation.getName().toLowerCase() 
																		  + "." + attrName.toLowerCase() + ".info"));
		this.infoPanel.setExample(ResourceBundle.getBundle("viewer.update2.gui.help").getString(this.relation.getName().toLowerCase() 
																			 + "." + attrName.toLowerCase() + ".example"));
		
		this.infoPanel.pack();
		this.infoPanel.setLocation(pPoint.x +30, pPoint.y +30);
		this.infoPanel.setVisible(true);
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
	
	
	class InfoPanel extends JDialog
	{
		private JLabel title;
		private JTextArea info;
		private JTextArea example;
		
		InfoPanel(JDialog pOwner)
		{
			super(pOwner);

			this.setSize(300,300);
			
			this.title = new JLabel();
			this.info = new JTextArea();
			this.info.setEditable(false);
			this.info.setLineWrap(true);
			this.info.setWrapStyleWord(true);
			this.example = new JTextArea();
			this.example.setEditable(true);
			this.example.setLineWrap(true);
			this.example.setWrapStyleWord(true);
			
			// component arrangement
			JPanel pnl1 = new JPanel(new BorderLayout());
			pnl1.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
			pnl1.add(this.title, BorderLayout.NORTH);
			JScrollPane scp1 = new JScrollPane(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
			scp1.setPreferredSize(new Dimension(300,100));
			scp1.setBorder(null);
			scp1.setOpaque(false);
			scp1.setViewportView(info);
			pnl1.add(scp1, BorderLayout.CENTER);
			
			JPanel pnl2 = new JPanel(new BorderLayout());
			pnl2.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
			pnl2.add(new JLabel("Example:"), BorderLayout.NORTH);
			JScrollPane scp2 = new JScrollPane(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
			scp2.setPreferredSize(new Dimension(300,100));
			scp2.setViewportView(example);
			pnl2.add(scp2, BorderLayout.CENTER);

			this.getContentPane().setLayout(new GridLayout(2,1));
			this.getContentPane().add(pnl1);
			this.getContentPane().add(pnl2);
		}
	
		public void setTitle(String pTitle) { this.title.setText(pTitle); }
		public void setInfo(String pInfo) { this.info.setText(pInfo); }
		public void setExample(String pExample) { this.example.setText(pExample); }
		
	}
	
}
