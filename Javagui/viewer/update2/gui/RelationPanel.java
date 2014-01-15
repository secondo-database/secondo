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

import gui.SecondoObject;
import gui.idmanager.ID;
import gui.idmanager.IDManager;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Frame;
import java.awt.Graphics;
import java.awt.GridLayout;
import java.awt.Image;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.awt.event.InputEvent;
import java.awt.event.InputMethodListener;
import java.awt.event.InputMethodEvent;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.StringTokenizer;
import javax.swing.AbstractAction;
import javax.swing.BoxLayout;
import javax.swing.CellEditor;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.JSplitPane;
import javax.swing.JTable;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.JViewport;
import javax.swing.KeyStroke;
import javax.swing.ListModel;
import javax.swing.ListSelectionModel;
import javax.swing.SwingConstants;
import javax.swing.ToolTipManager;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.MouseInputAdapter;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;
import javax.swing.table.TableColumn;
import javax.swing.table.TableModel;

import project.Projection;
import sj.lang.ListExpr;
import sj.lang.ServerErrorCodes;
import tools.Reporter;

import components.ChangeValueEvent;
import components.ChangeValueListener;
import components.LongScrollBar;

import viewer.update.CommandExecuter;
import viewer.update2.*;

/**
 * Panel that contains a table used to display one relation sequentially,
 * i.e. as a sequence of triples (tuple ID, attribute name, attribute value).
 */
public class RelationPanel extends JPanel implements
    ListSelectionListener,
	PropertyChangeListener
{
	
	private String name;
	
	private Relation relation;
	
	private Relation insertRelation;
	
	// controller listens to action events
	private UpdateViewerController controller;
	
	private JSplitPane splitPane;
	
	// currently loaded relation
	private JTable relTable;
		
	private JScrollPane relScroll;
			
	// components to display the relation in insert mode
	private JTable insertTable;	

	private JScrollPane insertScroll;

	
	private ValueTableCellRenderer tableCellRenderer;
	
	private ValueTableCellEditor tableCellEditor;
	
	// saves the old value of the currently edited cell, when in edit mode
	private String oldEditCellValue;
	
	// search panel
	private JPanel southPanel;
	
	private JPanel searchPanel;

	private JButton search;
	
	private JButton previous;

	private JButton previousFast;
	
	private JButton next;
	
	private JButton nextFast;
	
	private JCheckBox chkCaseSensitive;
		
	private JLabel searchLabel;
	
	private JLabel searchResultLabel;
	
	private JTextField searchField;
	
	// replace panel
	private JPanel replacePanel;
	private JButton replace;
	private JButton replaceAll;
	private JTextField replaceField;
	
		
	/**
	 * Builds a panel to display one relation
	 */
	public RelationPanel(String pRelationName, UpdateViewerController pController) 
	{
		this.name = pRelationName;
		this.setLayout(new BorderLayout());		
		this.controller = pController;
		
		// tables
		this.splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT);
		this.relScroll = new JScrollPane();
		this.relScroll.getVerticalScrollBar().setUnitIncrement(10);
		this.insertScroll = new JScrollPane();
		this.add(this.relScroll, BorderLayout.CENTER);
		this.tableCellRenderer = new ValueTableCellRenderer();
		this.tableCellEditor = new ValueTableCellEditor();
		
		// formatted text view
		
		this.southPanel = new JPanel(new GridLayout(2, 1));

		// search panel
		this.searchPanel = new JPanel();
		this.searchPanel.setLayout(new FlowLayout());
		//this.searchPanel.setPreferredSize(new Dimension(600, 80));
		this.searchField = new JTextField(15);
		this.searchPanel.add(this.searchField);
		this.search = new JButton(UpdateViewerController.CMD_SEARCH);
		this.search.addActionListener(this.controller); 
		this.searchPanel.add(this.search);
		this.chkCaseSensitive = new JCheckBox("case-sensitive");
		this.chkCaseSensitive.addItemListener(this.controller);
		this.searchPanel.add(this.chkCaseSensitive);
		this.previousFast = new JButton(this.createIcon("res/TOFRONT.gif"));
		this.previousFast.addActionListener(this.controller); 
		this.previousFast.setActionCommand(UpdateViewerController.CMD_FIRST);
		this.previousFast.setToolTipText("Go to first search hit in this relation");
		this.searchPanel.add(this.previousFast);
		this.previous = new JButton(this.createIcon("res/REVERSE.gif"));
		this.previous.addActionListener(this.controller); 
		this.previous.setActionCommand(UpdateViewerController.CMD_PREVIOUS);
		this.previous.setToolTipText("Go to previous search hit");
		this.searchPanel.add(this.previous);
		this.next = new JButton(this.createIcon("res/play.gif"));
		this.next.addActionListener(this.controller); 
		this.next.setActionCommand(UpdateViewerController.CMD_NEXT);
		this.next.setToolTipText("Go to next search hit");
		this.searchPanel.add(this.next);
		this.nextFast = new JButton(this.createIcon("res/TOEND.gif"));
		this.nextFast.addActionListener(this.controller); 
		this.nextFast.setActionCommand(UpdateViewerController.CMD_LAST);
		this.nextFast.setToolTipText("Go to last search hit in this relation");
		this.searchPanel.add(this.nextFast);
		this.searchResultLabel = new JLabel();
		this.searchPanel.add(this.searchResultLabel);
		
		// replace panel
		this.replacePanel = new JPanel();
		this.replacePanel.setLayout(new FlowLayout());
		this.replaceField = new JTextField(15);
		this.replacePanel.add(this.replaceField);
		this.replace = new JButton(UpdateViewerController.CMD_REPLACE);
		this.replace.addActionListener(this.controller); 
		this.replace.setToolTipText("Replace current match and go to next");
		this.replacePanel.add(this.replace);
		this.replaceAll = new JButton(UpdateViewerController.CMD_REPLACE_ALL);
		this.replaceAll.addActionListener(this.controller); 
		this.replaceAll.setToolTipText("Replace in all loaded relations");
		this.replacePanel.add(this.replaceAll);
		
		this.southPanel.add(this.searchPanel);
		this.southPanel.add(this.replacePanel);

		this.add(this.southPanel, BorderLayout.SOUTH);

		this.clearSearch();
		this.revalidate();
	}
	
	
	/*
	 * Adds a row set to the insert table, so that a (or another) tuple can be edited. 
	 */
	public void addInsertTuple() throws InvalidRelationException
	{
		((RelationTableModel)this.insertTable.getModel()).addTuple(null);
		this.validate();
		this.repaint();
	}
	
		
	
	/* Adds remove functionality to the insertTable */
	private void addRemoveToInsertTable()
	{
		if(insertTable==null){
			return;
		}
		insertTable.addKeyListener(new KeyAdapter(){
								   public void keyPressed(KeyEvent evt){
								   if( (evt.getKeyCode()==KeyEvent.VK_DELETE)){ 
								   removeInsertSelection();
								   }
								   }
								   });
		
	}
    

    public boolean clearDeletions()
	{
		return this.getTableModel().clearDeletions();
	}
	
	
	/**
	 * Clears search results from search panel and table data.
	 */
	public void clearSearch()
	{
		this.search.setText(UpdateViewerController.CMD_SEARCH);
		this.searchField.setText("");
		this.replaceField.setText("");
		this.searchField.setEnabled(true);
		this.searchResultLabel.setText("");
		this.next.setEnabled(false);
		this.nextFast.setEnabled(false);
		this.previous.setEnabled(false);
		this.previousFast.setEnabled(false);
		
		this.replaceField.setText("");
		
		if (this.relTable != null)
		{
			this.getTableModel().setSearchHits(null);
			this.relTable.revalidate();
			this.relTable.repaint();
		}
		this.repaint();
	}
	
	
 
	public boolean clearUpdateChanges()
	{
		return this.getTableModel().clearChanges();
	}
    
    
	
	/**
	 * Returns the length of the specified String as rendered with current FontMetrics.
	 */
	public int computeLengthByCurrentFont(String pString)
	{
		FontMetrics metrics = ((Component)this).getFontMetrics(this.getFont());
		return metrics.stringWidth(pString);
	}
	
	
	/**
	 * Computes maximum content length of each table column 
	 * and initializes maxContentLengths.
	 * TODO: nicer to have this done while scanning the relation during init phase.
	 */
	private int computeMaxContentLength(int pCol)
	{		
		int max = 0;
		
		if (pCol == 0 || pCol == 1)
		{
			List<String> strings = this.getTableModel().getColumnContent(pCol);
			strings.add(this.getTableModel().getColumnName(pCol));
			
			for (String s : strings)
			{
				int length = this.computeLengthByCurrentFont(s);
				if (length > max)
					max = length;
			}
		}
		
		// add some extra space
		FontMetrics metrics = ((Component)this).getFontMetrics(this.getFont());
		max = max + metrics.getMaxAdvance();
		
		return max;
	}
	
	
	/*
	 * Creates and shows the table that displays the given relation in sequential manner.
	 * Returns true if paramater is valid list expression for relation.
	 */
	public boolean createFromLE(ListExpr pRelationLE, boolean pEditable) 
	{		
		try
		{
			SecondoObject relationSO = new SecondoObject(this.getName(), pRelationLE);
			this.relation = new Relation();
			this.relation.readFromSecondoObject(relationSO);
			
			RelationTableModel rtm = new RelationTableModel(relation, pEditable);
			this.relTable = new JTable(rtm);
			this.relTable.setRowSelectionAllowed(false);
			this.relTable.setColumnSelectionAllowed(false);
			
			// suppress calling renderer every time the mouse is moved 
			ToolTipManager.sharedInstance().unregisterComponent(relTable);
			
			// set column widths
			TableColumn column = this.relTable.getColumnModel().getColumn(0);
			column.setMinWidth(this.computeMaxContentLength(0)); 
			column.setMaxWidth(this.computeMaxContentLength(0)); 
			column = this.relTable.getColumnModel().getColumn(1);
			column.setMinWidth(this.computeMaxContentLength(1));
			column.setMaxWidth(this.computeMaxContentLength(1));
			
			// set cell renderers
			LabelTableCellRenderer lcr = new LabelTableCellRenderer();
			column = this.relTable.getColumnModel().getColumn(0);
			column.setCellRenderer(lcr);
			
			column = this.relTable.getColumnModel().getColumn(1);
			column.setCellRenderer(lcr);
			
			column = this.relTable.getColumnModel().getColumn(2);
			column.setCellRenderer(this.tableCellRenderer);
			column.setCellEditor(this.tableCellEditor);
			
			// set listeners
			this.relTable.addPropertyChangeListener(this);
            
            ListSelectionModel lsm = this.relTable.getSelectionModel();
            lsm.addListSelectionListener(this);
            this.relTable.setSelectionModel(lsm);
			
			this.relScroll.setViewportView(this.relTable);
			
			this.repaint();
			this.validate();
		}
		catch(InvalidRelationException e)
		{
			Reporter.showError(e.getMessage());
			return false;
		}
		return true;
	}
	
	/**
	 * Returns icon with image from given path, resized to specified width and height.
	 */
	public static ImageIcon createIcon(String pPath)
	{
		ImageIcon icon = new ImageIcon(ClassLoader.getSystemResource(pPath));
		icon.setImage(icon.getImage().getScaledInstance(15, 15, Image.SCALE_DEFAULT));
		return icon;
	}	
	
	/**
	 * Deletes marked tuples from the table(model).
	 */
	public void deleteTuples()
	{
		RelationTableModel rtm = this.getTableModel();
		if (rtm != null)
		{
			List<String> deleteIds = new ArrayList<String>(rtm.getDeletions());
			for (String id : deleteIds)
			{
				this.getTableModel().removeTuple(id);
			}
			rtm.clearDeletions();
		}
		this.relTable.revalidate();
		this.revalidate();
		this.repaint();
	}
	
	
	/**
	 * Returns TRUE if checkbox case-sensitive is checked.
	 */
	public boolean getCaseSensitive()
	{
		return this.chkCaseSensitive.isSelected();
	}
	
	/**
	 * Returns index of current search hit, -1 if none exists.
	 */
	public int getCurrentHitIndex()
	{
		return this.getTableModel().getCurrentHitIndex();
	}
	
	
	/*
	 * Returns ids of all tuples that were marked to be deleted.	 	 
	 */
	public List<String> getDeleteTuples()
    {
        List<String> del = this.getTableModel().getDeletions();
        Reporter.debug("RelationPanel.getDeleteTuples: " + del);
		
		return this.getTableModel().getDeletions();
	}
	
	/**
	 * Returns index of current search hit, -1 if none exists.
	 */
	public SearchHit getHit(int pIndex)
	{
		return this.getTableModel().getHit(pIndex);
	}
	
	/**
	 * Returns number of current search hits, -1 if none exists.
	 */
	public int getHitCount()
	{
		return this.getTableModel().getHitCount();
	}
	
	
	/*
	 * Returns values to be inserted.
	 */	
	public List<Tuple> getInsertTuples()
	{
		List<Tuple> tuples = new ArrayList<Tuple>();
		for (int i = 0; i < this.insertRelation.getTupleCount(); i++)
		{
			tuples.add(this.insertRelation.getTupleAt(i));
		}
		//Reporter.debug("RelationPanel.getInsertTuples: no. of insertTuples is " + tuples.size());
		return tuples;
	}
	
	/*
	 * Get relation name.
	 */
	public String getName() {
		return this.name;
	}
	
	/*
	 * Returns the relation data.	 	 
	 */
	public Relation getRelation()
	{
		return this.relation;
	}
	
	/**
	 * Returns RelationTabelModel of currently displayed Table. (Convenience shortcut)
	 */
	private RelationTableModel getTableModel()
	{
		return (RelationTableModel)this.relTable.getModel();
	}
	
	
	/**
	 * Returns text from Replace field.
	 */
	public String getReplacement()
	{
		return this.replaceField.getText();
	}
	
	/**
	 * Returns text from Search field.
	 */
	public String getSearchKey()
	{
		return this.searchField.getText();
	}
	
	
	/**
	 * Returns state of RelationTabelModel.
	 */
	public int getState()
	{
		return this.getTableModel().getState();
	}
	
	
	
	/*
	 * Returns values to be updated.
	 */	
	public Map<Integer, HashMap<String, Change>> getUpdateTuples()
	{
		return this.getTableModel().getChangesForUpdate();
	}
	
	
	/** 
	 * Scrolls table so that specified position within specified row is displayed.
	 * Some distance to border is added for better context viewing.
	 * @param pRow row index
	 * @param pStartPosition position of first within cell
	 **/
	public void goTo(int pRow, int pStartPosition, int pEndPosition)
	{
		//this.relTable.changeSelection(pRow, pCol, false, false);

		Rectangle offset = null;
		
		if (this.getTableModel().getState() == States.UPDATE)
		{
			try
			{
				//if (this.relTable.editCellAt(pRow, RelationTableModel.COL_ATTRVALUE, null))
				if (this.relTable.editCellAt(pRow, RelationTableModel.COL_ATTRVALUE))
                {
                    // get hit position within cell
                    ValueTableCellEditor tc = (ValueTableCellEditor)this.relTable.getCellEditor(pRow, RelationTableModel.COL_ATTRVALUE);
                    offset = tc.getOffset(pStartPosition);
                    //tc.setCaret(pStartPosition, pEndPosition);
                }
				
			} 
			catch(Exception e)
			{
				Reporter.debug(e);
			}
		}
		else
		{
			ValueTableCellRenderer tc = (ValueTableCellRenderer)this.relTable.getCellRenderer(pRow, RelationTableModel.COL_ATTRVALUE);
			offset = tc.getOffset(pStartPosition);
		}
		
		JViewport viewport = (JViewport)relTable.getParent();
		Rectangle viewRect = viewport.getViewRect();

		// get cell position
		Rectangle rect = relTable.getCellRect(pRow, 2, true);		
		
		/*
		// add x-offset within cell
		if (offset != null)
		{
			rect.setLocation(rect.x + offset.x, rect.y + offset.y);			
		}
		else
		{
			Reporter.debug("RelationPanel.goTo: position not found: " + pStartPosition);
		}
		 */
		
		rect.setLocation(rect.x - viewRect.x, rect.y - viewRect.y);
		 
		int centerX = (viewRect.width - rect.width) / 2;
		int centerY = (viewRect.height - rect.height) / 2;
		if (rect.x < centerX) 
		{
			centerX = -centerX;
		}
		if (rect.y < centerY) 
		{
			centerY = -centerY;
		}
		rect.translate(centerX, centerY);
		
		viewport.scrollRectToVisible(rect);
	}
	
	
	
	/** Starts the editing the specified cell within the relation table **/
	public void goToEdit(int pRow, int pCol)
	{
		try
		{
			this.relTable.editCellAt(pRow, pCol, null); 
		} 
		catch(Exception e)
		{
			Reporter.debug(e);
		}
	}
	
	
	/** Starts the editing the specified cell within the insert table **/
	public void goToInsert(int x, int y)
	{
		try
		{
			insertTable.editCellAt(x,y,null);
		}catch(Exception e){
			Reporter.debug(e);
		}
	}
	
	/**
	 * Returns TRUE if last search has found matches in this relation table.
	 */
	public boolean hasSearchHits()
	{
		return this.getTableModel().hasSearchHits();
	}
	
	/**
	 * Inserts the specified tuple into Table(model)
	 */
	public void insertTuple(ListExpr pTuple)
	{
		this.getTableModel().addTuple(pTuple);
		this.relTable.revalidate();
		this.revalidate();
		this.repaint();
	}
	
	/**
	 * Reacts on PropertyChangeEvent indicating that table cells were edited.
	 */
	public void processEditing()
	{
		if (this.relTable.isEditing())
		{
			// Editing started
			// Save old value of currently active editable table cell.
			this.oldEditCellValue = (String)this.tableCellEditor.getCellEditorValue();
			
			Reporter.debug("processEditingStarted: saved old value in cell (" 
						   + this.relTable.getSelectedRow() + ", " + this.relTable.getSelectedColumn() + ")" );		
		}
		else
		{
			// Editing stopped
			int row = this.relTable.getEditingRow();
			int col = this.relTable.getEditingColumn();
			
			String newValue = (String)this.tableCellEditor.getCellEditorValue();
			
			if (!newValue.equals(this.oldEditCellValue))
			{
				RelationTableModel rtm = this.getTableModel();
				
				// update search hits for edited cell
				List<SearchHit> hits = rtm.getHits(row);
				if (hits != null && !hits.isEmpty())
				{
					for (SearchHit h : hits)
					{
						rtm.removeHit(h);
					}
				}
				for (SearchHit newHit : this.retrieveSearchHits(this.getSearchKey(), this.getCaseSensitive(), row))
				{
					rtm.addHit(newHit);
				}
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
				
				Reporter.debug("RelationPanel.processEditingStopped: new value of table cell (" + row + ", " + col + ") is " + newValue) ;			
			}
		}
	}
	
	/**
	 *  Implemention of PropertyChangeListener interface.
	 */
	public void propertyChange(PropertyChangeEvent e)
	{
		//  A table cell has started/stopped editing
		if ("tableCellEditor".equals(e.getPropertyName()))
		{
			this.processEditing();
		}
	}
	
	
	/** Removes all selected rows from the InsertTable. **/
	private void removeInsertSelection(){
		// TODO
		/*
		if(insertTable==null){ // no table available
			return;
		}
		if(insertTable.isEditing()){
			return;
		}
		int[] selectedRows = insertTable.getSelectedRows();
		if( selectedRows.length==0){
			JOptionPane.showMessageDialog(this,"Nothing selected");
			return;
		}
		
		int answer = JOptionPane.showConfirmDialog(this,"All selected rows will be deleted\n Do you want to continue?",
												   "Please Confirm",
												   JOptionPane.YES_NO_OPTION);
		if((answer!=JOptionPane.YES_OPTION)){
			return; 
		}
		
		String[][] newInsertData = new String[insertData.length-selectedRows.length][insertData[0].length];
		// copy all non-removed rows
		int pos = 0; 
		for(int i=0;i < insertData.length;i++){
			if(pos>=selectedRows.length){ // all selected rows are removed already
				newInsertData[i-pos] = insertData[i];
			} else if(i==selectedRows[pos]){ // remove this row
				pos++;
			} else {
				newInsertData[i-pos] = insertData[i];
			}
		}
		
		insertData = newInsertData;
		insertTable = new JTable(insertData, head);
		addRemoveToInsertTable();
		insertScroll.setViewportView(insertTable);
		int lastPos = insertSplit.getDividerLocation();
		insertSplit.setBottomComponent(insertScroll);
		this.add(insertSplit,BorderLayout.CENTER);
		this.validate();
		this.repaint();
		insertSplit.setDividerLocation(lastPos);
		 */
	}
	
	
	/*
	 * Removes the last added tuple from the insert relation.
	 * if it has more than one tuple. 
	 */
	public boolean removeLastInsertTuple()
	{
		int tupleCount = insertRelation.getTupleCount();
		if (tupleCount > 1)
		{
			this.insertRelation.removeTupleByIndex(tupleCount-1);
			this.validate();
			this.repaint();
			return true;
		}
		return false;
	}
	
	
	/**
	 * Replaces the text at the given location by the replacement string and creates a change,
	 * so replacements can be undone.
	 * Replacement is only possible in Update mode.
	 */
	public void replace(SearchHit pHit, String pReplacement)
	{
		//Reporter.debug("RelationPanel.replace: relation=" + this.getName() + ", hit=" + pHit.toString());
		if (this.getState() == States.UPDATE && pHit != null && pReplacement != null)
		{
			int row = pHit.getRowIndex();
			
			RelationTableModel rtm = this.getTableModel();
			if (rtm != null)
			{
				String oldValue = (String)rtm.getValueAt(row, RelationTableModel.COL_ATTRVALUE);
				
				String key = this.getSearchKey();
				if (!this.getCaseSensitive())
				{
					key = "(?i)" + key;
				}
				
				String newValue = oldValue.replaceFirst(key, pReplacement);
				
				//Reporter.debug("RelationPanel.replace: relation=" + this.getName() + ", row=" + row + ", oldvalue=" + oldValue + ", newValue=" + newValue);
				
				// write changed cell value back into table model
				relTable.setValueAt(newValue, row, RelationTableModel.COL_ATTRVALUE);
				
				// create Change for update or undo actions
				int tupleIndex = Integer.valueOf((String)rtm.getValueAt(row, RelationTableModel.COL_TUPLEID));
				int attributeIndex = rtm.rowToAttributeIndex(row);
				String attributeName = this.relation.getAttributeNames().get(attributeIndex);
				String attributeType = this.relation.getAttributeTypes().get(attributeIndex);
				
				Change change = new Change(tupleIndex, attributeIndex, row,
										   attributeName, attributeType, 
										   oldValue, newValue);
				
				rtm.addChange(change);
				
				if (!pReplacement.contains(this.getSearchKey()))
				{
					rtm.removeHit(pHit);
				}
				
				Reporter.debug("RelationPanel.replace: rtm.getValue()=" + rtm.getValueAt(row, RelationTableModel.COL_ATTRVALUE));
				Reporter.debug("RelationPanel.replace: relTable.getValue()=" + relTable.getValueAt(row, RelationTableModel.COL_ATTRVALUE));
				
				this.relTable.revalidate();
				this.revalidate();
				this.repaint();
			}
		}
	}
	
	
	/*
     * Reset for Delete mode:
	 * Clears uncommitted deletions. 
	 */
	public void resetDeleteSelections()
	{
        this.relTable.clearSelection();
        this.getTableModel().clearDeletions();
	}
	
	
	/*
	 * Removes the table with insert tuples.	 	 
	 */
	public void resetInsert() 
	{
		this.remove(this.splitPane);
		this.add(relScroll, BorderLayout.CENTER);
		this.validate();
	}
	
		
	/* 
	 * Reset for Update mode:
	 * Sets table to original data and removes uncommitted changes.
	 */
	public void resetUpdateChanges()
	{
		//Reporter.debug("RelationPanel.resetUpdateChanges");
		
        this.takeOverLastEditing(true);
		
		RelationTableModel rtm = this.getTableModel();
		List<Change> changes = rtm.getChanges();
		// reset table to original data
		for(int i=changes.size()-1; i>=0; i--)
		{
			Change ch = changes.get(i);
			relTable.setValueAt(ch.getOldValue(), ch.getRowIndex(), 2);
			rtm.removeChange(ch);
			Reporter.debug("RelationPanel.resetUpdateChanges: reset and removed change " + ch.toString());
		}
				
		this.relTable.revalidate();
	}
	
	/**
	 * Returns SearchHits (containing row, start and end position within cell) 
     * of all occurences for specified search key.
     * @param pKey search key (java regex pattern)
	 * @param pCaseSensitive
	 */
	public List<SearchHit> retrieveSearchHits(String pKey, boolean pCaseSensitive)
	{
		List<SearchHit> result = new ArrayList<SearchHit>();
		
		if (pKey != null && (pKey.length() > 0 ))
		{
			for (int i = 0; i < this.relTable.getRowCount(); i++)
			{
				result.addAll(this.retrieveSearchHits(pKey, pCaseSensitive, i));
			}
		}

		return result;
	}
		
	/**
	 * Returns hit positions for given key in a single table value cell.
	 */
	public List<SearchHit> retrieveSearchHits(String pKey, boolean pCaseSensitive, int pRow)
	{
		List<SearchHit> result = new ArrayList<SearchHit>();

		String cellValue = (String)this.relTable.getValueAt(pRow, RelationTableModel.COL_ATTRVALUE);
		
		Pattern pattern = pCaseSensitive ? Pattern.compile(pKey) : Pattern.compile(pKey, Pattern.CASE_INSENSITIVE);
		
		Matcher matcher = pattern.matcher(cellValue);
		boolean found = matcher.find();
		
		while (found)
		{
			SearchHit hit = new SearchHit(pRow, matcher.start(), matcher.start() + pKey.length());
			result.add(hit);
			found = matcher.find();
			Reporter.debug("RelationPanel.retrieveSearchHits: match in " + this.getName() + ": " + hit.toString());
		}
		return result;
	}
	
	
	/**
	 * Sets state and changes table properties accordingly.
	 * @param pSelectMode one of the States constants
	 * @see viewer.update2.States
	 */
	public void setState(int pState)
	{
		//Reporter.debug("RelationPanel.setState: oldState of relation " + this.getName() + " is " + this.getTableModel().getState());

		if (this.getTableModel().getState() != States.LOADED_READ_ONLY)
		{
			this.getTableModel().setState(pState);
			
			switch (pState)
			{
				case States.DELETE:
				{
					this.relTable.setRowSelectionAllowed(true);
					this.relTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
					break;
				}
				case States.INSERT:
				{
					((RelationTableModel)this.insertTable.getModel()).setState(pState);
					break;
				}
				case States.UPDATE:
				{
					this.replaceField.setEnabled(true);
					this.replace.setEnabled(true);
					this.replaceAll.setEnabled(true);
					break;
				}
				case States.FORMAT:
				{
					this.replaceField.setEnabled(false);
					this.replace.setEnabled(false);
					this.replaceAll.setEnabled(false);
					break;
				}
				default:
				{
					this.relTable.setRowSelectionAllowed(false);
					this.replaceField.setText("");
					this.replaceField.setEnabled(false);
					this.replace.setEnabled(false);
					this.replaceAll.setEnabled(false);
				}
			}
		}
		//Reporter.debug("RelationPanel.setState: newState of relation " + this.getName() + " is " + this.getTableModel().getState());
	}
	
	/**
	 * 
	 */
	public void setSearchHits(List<SearchHit> pHits)
	{
		this.getTableModel().setSearchHits(pHits);
	}
    
	
	/**
	 * Shows specified search hit (if valid)
	 * scrolls table, sets text and number of current hit in search panel.
	 */
	public boolean showHit(int pIndex)
	{
		Reporter.debug("RelationPanel.showHit: relation=" + this.getName() + ", hit=" + pIndex);
		SearchHit hit = this.getTableModel().getHit(pIndex);
		if (hit != null)
		{
			this.getTableModel().setCurrentHitIndex(pIndex);			
			this.goTo(hit.getRowIndex(), hit.getStart(), hit.getEnd());
			this.searchResultLabel.setText((pIndex+1) + " / " + this.getTableModel().getHitCount());
			return true;
		}
		return false;
	}

	/*
	 * Shows an empty relation that can be edited to contain tuples that shall be inserted.	 
	 */
	public void showInsertTable() throws InvalidRelationException
	{	
		this.insertRelation = this.relation.createEmptyClone();
		this.insertRelation.addTuple(this.insertRelation.createEmptyTuple());
		
		RelationTableModel dtm = new RelationTableModel(this.insertRelation, true);
		this.insertTable = new JTable(dtm);
		
		// set column width and renderers
		TableColumn column = this.insertTable.getColumnModel().getColumn(0);
		column.setMinWidth(this.relTable.getColumnModel().getColumn(0).getMinWidth()); 
		column.setMaxWidth(this.relTable.getColumnModel().getColumn(0).getMaxWidth()); 
		column.setCellRenderer(new LabelTableCellRenderer());
		
		column = this.insertTable.getColumnModel().getColumn(1);
		column.setMinWidth(this.relTable.getColumnModel().getColumn(1).getMinWidth()); 
		column.setMaxWidth(this.relTable.getColumnModel().getColumn(1).getMaxWidth()); 
		column.setCellRenderer(new LabelTableCellRenderer());
		
		column = this.insertTable.getColumnModel().getColumn(2);
		column.setCellRenderer(this.tableCellRenderer);
		column.setCellEditor(this.tableCellEditor);
		
		// replace 
		this.insertScroll.setViewportView(insertTable);		
		this.splitPane.setTopComponent(this.relScroll);	
		this.splitPane.setBottomComponent(this.insertScroll);
		this.splitPane.setResizeWeight(0.5);
		this.remove(relScroll);
		this.add(this.splitPane, BorderLayout.CENTER);
		
		this.revalidate();
		this.repaint();
	}
	
	/**
	 * Sets buttons and number of search results in search panel.
     * Scrolls to first hit.
	 */
	public void showSearchResult(String pKey)
	{
		this.searchField.setText(pKey);
		this.searchField.setEnabled(false);
		this.search.setText(UpdateViewerController.CMD_CLEAR_SEARCH);
		
		if (!this.getTableModel().hasSearchHits())
		{
			this.searchResultLabel.setText("0 / 0");
		}
		else
		{
			int curr = this.getTableModel().getCurrentHitIndex();
			//SearchHit hit = this.getTableModel().getHit(curr);
			
			this.searchResultLabel.setText(curr+1 + " / " + this.getTableModel().getHitCount());
			this.next.setEnabled(true);
			this.nextFast.setEnabled(true);
			this.previous.setEnabled(true);
			this.previousFast.setEnabled(true);
		}
		
		this.relTable.revalidate();
		this.relTable.repaint();
		this.revalidate();
		this.repaint();
	}
	
	
	/*
	 The last cell the user edited before he pressed "commit" is usually not considered
	 because "tableChanged" will only be called, if he pressed "return" or selected a different cell
	 of the JTable before "commiting". Therefore this method takes over the value of the
	 last edited cell that was not taken into consideration yet.	 
	 */
	public void takeOverLastEditing(boolean updateMode)
	{
		if (updateMode)
		{
			if (relTable.isEditing()) 
			{
				int editedRow = relTable.getEditingRow();
				int editedColumn = relTable.getEditingColumn();
				CellEditor editor = relTable.getCellEditor(editedRow, editedColumn);
				editor.stopCellEditing();
			}			
		}
		else
		{
			if (insertTable.isEditing()) 
			{
				int editedRow = this.insertTable.getEditingRow();
				int editedColumn = this.insertTable.getEditingColumn();
				CellEditor editor = this.insertTable.getCellEditor(editedRow, editedColumn);
				editor.stopCellEditing();
			}
		}
	}	
	
	/**
	 * Resets the last selection in Delete mode.
	 * Returns TRUE if after execution there are still deletions left to undo.
	 */
	public boolean undoLastDeleteSelection()
	{
        if (this.getTableModel().removeLastDeletion())
		{
			this.validate();
			this.repaint();            
		}
        
		return this.getTableModel().hasDeletions();
	}
    
    
    /**
	 * Resets the last editited cell in updatemode to its original value.
	 * Returns TRUE if after execution there are still changes left to undo.
	 */
	public boolean undoLastUpdateChange()
	{
		boolean result = false;
		
		this.takeOverLastEditing(true);
		
		Change ch = this.getTableModel().getLastChange();
		
		if (ch != null)
		{
			relTable.setValueAt(ch.getOldValue(), ch.getRowIndex(), 2);
			this.goToEdit(ch.getRowIndex(), 2);
			
			this.getTableModel().removeChange(ch);
			
			if (this.getTableModel().getLastChange() != null)
			{
				result = true;
			}
			
			this.validate();
			this.repaint();
		}
		
		return result;
	}
    
    
    /**
     * Method of interface ListSelectionListener.
     * Reacts to selections in Delete mode.
     */
    public void valueChanged(ListSelectionEvent e)
    {
		//Reporter.debug("RelationPanel.valueChanged: index is " + e.getFirstIndex());
        
        if (this.getTableModel().getState() == States.DELETE)
        {
			int rowIndex;
			
            if (e.getValueIsAdjusting())
            {
                rowIndex = e.getLastIndex();
            }
			else
			{
                rowIndex = e.getFirstIndex();
			}
			
			this.getTableModel().addDeletion(rowIndex);			
			this.relTable.revalidate();			
        }
    }
}

