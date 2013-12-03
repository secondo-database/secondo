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
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;
import javax.swing.table.TableColumn;
import javax.swing.table.TableModel;
import javax.swing.JViewport;

import project.Projection;
import sj.lang.ListExpr;
import sj.lang.ServerErrorCodes;
import tools.Reporter;

import components.ChangeValueEvent;
import components.ChangeValueListener;
import components.LongScrollBar;

import viewer.relsplit.InvalidRelationException;
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
	
	private JCheckBox chkReplace;
	
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
		this.search = new JButton("Search");
		this.search.addActionListener(this.controller); 
		this.searchPanel.add(this.search);
		this.chkCaseSensitive = new JCheckBox("case-sensitive");
		this.chkCaseSensitive.addItemListener(this.controller);
		this.searchPanel.add(this.chkCaseSensitive);
		this.previousFast = new JButton(this.createIcon("res/TOFRONT.gif"));
		this.previousFast.addActionListener(this.controller); 
		this.previousFast.setActionCommand("PreviousFast");
		this.previousFast.setToolTipText("Go to first search hit in this relation");
		this.searchPanel.add(this.previousFast);
		this.previous = new JButton(this.createIcon("res/REVERSE.gif"));
		this.previous.addActionListener(this.controller); 
		this.previous.setActionCommand("Previous");
		this.previous.setToolTipText("Go to previous search hit");
		this.searchPanel.add(this.previous);
		this.next = new JButton(this.createIcon("res/play.gif"));
		this.next.addActionListener(this.controller); 
		this.next.setActionCommand("Next");
		this.next.setToolTipText("Go to next search hit");
		this.searchPanel.add(this.next);
		this.nextFast = new JButton(this.createIcon("res/TOEND.gif"));
		this.nextFast.addActionListener(this.controller); 
		this.nextFast.setActionCommand("NextFast");
		this.nextFast.setToolTipText("Go to last search hit in this relation");
		this.searchPanel.add(this.nextFast);
		this.searchResultLabel = new JLabel();
		this.searchPanel.add(this.searchResultLabel);
		
		// replace panel
		this.replacePanel = new JPanel();
		this.replacePanel.setLayout(new FlowLayout());
		this.replaceField = new JTextField(15);
		this.replacePanel.add(this.replaceField);
		this.replace = new JButton("Replace");
		this.replace.addActionListener(this.controller); 
		this.replace.setToolTipText("Replace current match and go to next");
		this.replacePanel.add(this.replace);
		this.replaceAll = new JButton("Replace all");
		this.replaceAll.addActionListener(this.controller); 
		this.replaceAll.setToolTipText("Replace in all loaded relations");
		this.replacePanel.add(this.replaceAll);
		
		this.southPanel.add(this.searchPanel);
		this.southPanel.add(this.replacePanel);

		this.add(this.southPanel, BorderLayout.SOUTH);

		this.resetSearch();
		this.revalidate();
	}
	
	
	/*
	 * Adds a row set to the insert table, so that a (or another) tuple can be edited. 
	 */
	public void addInsertTuple() 
	{
		// append empty editable tuple fields
		/*
		this.remove(insertScroll);
		String[][] newInsertData = new String[insertData.length + 1][insertData[0].length];
		 for (int i = 0; i < insertData.length; i++) {
			newInsertData[i] = insertData[i];
		}
		for (int j = 0; j < insertData[0].length; j++) {
			newInsertData[insertData.length][j] = "";
		}
		insertData = newInsertData;
		insertTable = new JTable(insertData, head);
		addRemoveToInsertTable();
		insertScroll.setViewportView(insertTable);
		int lastPos = insertSplit.getDividerLocation();
		//  insertSplit.setTopComponent(relScroll);
		insertSplit.setBottomComponent(insertScroll);
		this.add(insertSplit,BorderLayout.CENTER);
		 */
		this.validate();
		this.repaint();
		//insertSplit.setDividerLocation(lastPos);
	}
	
	
	/*
	 * Add empty row for inserting tuple.
	 */
	public void addInsertRow() 
	{
		// TODO
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
	
	/**
	 * Adjust all row heights according to text value size.

	private void correctRowHeight(int pRow) 
	{
		int width = this.relTable.getColumn(RelationTableModel.COL_ATTRVALUE).getWidth();
		int height = this.computeRowHeight(width, pRow);
		relTable.setRowHeight(pRow, height);

	}
		 */
	
	/**
	 * Adjust row height according to text value size and width of textarea.

	private int computeRowHeight(int pWidth, int pRow) 
	{
		RelationTableModel rtm = this.getTableModel();
		String value = rtm.getValueAt(pRow, RelationTableModel.COL_ATTRVALUE).toString();
		if (value == null || (value.length() == 0)) 
		{
			value = "dummy";
		}
		JTextArea area = new JTextArea();
		area.setLineWrap(true);
		area.setWrapStyleWord(true);
		area.setSize(pWidth, Short.MAX_VALUE);
		area.setText(value);
		return area.getPreferredSize().height;
	}
	 */
	
	/*
	 * Creates and shows the table that displays the given relation in sequential manner.
	 * Returns true if paramater is valid list expression for relation.
	 */
	public boolean createFromLE(ListExpr pRelationLE) 
	{		
		SecondoObject relationSO = new SecondoObject(this.getName(), pRelationLE);
		this.relation = new Relation();
		this.relation.readFromSecondoObject(relationSO);
		
		/*if (this.relScroll != null)
		{
			this.remove(this.relScroll);
		}
		 */
		
		try
		{
			RelationTableModel rtm = new RelationTableModel(relation);
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
			
			//this.relScroll = new JScrollPane(this.relTable);
			//this.add(this.relScroll);			
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
	public int getCurrentHitIndex()
	{
		return this.getTableModel().getCurrentHitIndex();
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
	 Returns the values of all tuples that were edited to be inserted.	 	 

	public String[][] getInsertTuples() {
		return insertData;
	}
	 */	
	
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
	public void goTo(int pRow, int pCol, int pStartPosition, int pEndPosition)
	{
		//this.relTable.changeSelection(pRow, pCol, false, false);

		Rectangle offset = null;
		
		if (this.getTableModel().getState() == States.UPDATE)
		{
			try
			{
				//if (this.relTable.editCellAt(pRow, pCol, null))
				if (this.relTable.editCellAt(pRow, pCol))
                {
                    // get hit position within cell
                    ValueTableCellEditor tc = (ValueTableCellEditor)this.relTable.getCellEditor(pRow, pCol);
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
			ValueTableCellRenderer tc = (ValueTableCellRenderer)this.relTable.getCellRenderer(pRow, pCol);
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
		// TODO
		/*
		try{
			insertTable.editCellAt(x,y,null);
		}catch(Exception e){
			Reporter.debug(e);
		}
		 */
	}
	
	public boolean hasSearchHits()
	{
		return this.getTableModel().hasSearchHits();
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
				
				//Reporter.debug("RelationPanel.processEditingStopped: new value of table cell (" + row + ", " + col + ") is " + newValue) ;			
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
	 Removes the last added tuple from the insert-relation	 
	 */
	public boolean removeLastInsertTuple(){
		
		// TODO
		/*
		if (insertData.length == 1){
			removeInsertRelation();
			return false;
		}
		else {
			String[][] newInsertData = new String[insertData.length - 1][insertData[0].length];
			for (int i = 0; i < insertData.length -1 ; i++) {
				newInsertData[i] = insertData[i];
			}
			insertData = newInsertData;
			insertTable = new JTable(insertData, head);
			insertScroll.setViewportView(insertTable);
			insertSplit.setBottomComponent(insertScroll); 
			this.add(insertSplit,BorderLayout.CENTER);
			insertSplit.revalidate();
			insertSplit.setDividerLocation(0.5); 
			addRemoveToInsertTable();
			//	this.add(insertScroll, BorderLayout.SOUTH);
			this.validate();
			this.repaint();
			return true;
		}
		 */
		return false;
	}
	
	/**
	 * Replaces the text at the given location by the replacement string and creates a change,
	 * so replacements can be undone.
	 * Replacement is only possible in Update mode.
	 */
	public void replace(SearchHit pHit, String pReplacement)
	{
		if (pHit != null)
		{
			int row = pHit.getRowIndex();
			
			RelationTableModel rtm = this.getTableModel();
			String oldValue = (String)rtm.getValueAt(row, RelationTableModel.COL_ATTRVALUE);
			String key = oldValue.substring(pHit.getStart(), pHit.getEnd());
			String newValue = oldValue.replaceFirst(key, pReplacement);
			
			// write changed cell value back into table model
			this.relTable.setValueAt(newValue, row, RelationTableModel.COL_ATTRVALUE);
			
			// create Change for update or undo actions
			int tupleIndex = Integer.valueOf((String)rtm.getValueAt(row, RelationTableModel.COL_TUPLEID));
			int attributeIndex = rtm.rowToAttributeIndex(row);
			String attributeName = this.relation.getAttributeNames().get(attributeIndex);
			String attributeType = this.relation.getAttributeTypes().get(attributeIndex);
			
			Change change = new Change(tupleIndex, attributeIndex, row,
									   attributeName, attributeType, 
									   oldValue, newValue);
			
			rtm.addChange(change);
			
			rtm.removeHit(pHit);
			
			this.showSearchResult(key);
			
			this.relTable.revalidate();
			this.relTable.repaint();
			this.repaint();
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
	
	
	/**
	 * Clears search results from search panel and table data.
	 */
	public void resetSearch()
	{
		this.search.setText("Search");
		this.searchField.setText("");
		this.replaceField.setText("");
		this.searchField.setEnabled(true);
		this.searchResultLabel.setText("");
		this.next.setEnabled(false);
		this.nextFast.setEnabled(false);
		this.previous.setEnabled(false);
		this.previousFast.setEnabled(false);
		this.replace.setEnabled(false);
		this.replaceAll.setEnabled(false);
		this.replaceField.setEnabled(false);

		if (this.relTable != null)
		{
			this.getTableModel().setSearchHits(null);
			this.relTable.revalidate();
		}
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
			Pattern pattern;
			Matcher matcher;
			String cellValue;
			
			for (int i = 0; i < this.relTable.getRowCount(); i++)
			{
				cellValue = (String)this.relTable.getValueAt(i, RelationTableModel.COL_ATTRVALUE);
				
				if (pCaseSensitive)
				{
					pattern = Pattern.compile(pKey);
				}
				else
				{
					pattern = Pattern.compile(pKey, Pattern.CASE_INSENSITIVE);
				}
				
				matcher = pattern.matcher(cellValue);
				boolean found = matcher.find();
				
				while (found)
				{
					SearchHit hit = new SearchHit(i, matcher.start(), matcher.start() + pKey.length());
					result.add(hit);
					found = matcher.find();
					Reporter.debug("RelationPanel.retrieveSearchHits: match in " + this.getName() + ": " + hit.toString());
				}
			}
		}

		return result;
	}
		
	
	/**
	 * Sets state and changes table properties accordingly.
	 * @param pSelectMode one of the States constants
	 * @see viewer.update2.States
	 */
	public void setMode(int pSelectMode)
	{
		this.getTableModel().setState(pSelectMode);
        
        if (pSelectMode == States.DELETE)
        {
            this.relTable.setRowSelectionAllowed(true);
            //this.relTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        }
		if (pSelectMode == States.INSERT)
        {
            this.showInsertTable();
        }
		if (this.getTableModel().getState() == States.UPDATE)
		{
			this.replaceField.setEnabled(true);
			this.replace.setEnabled(true);
			this.replaceAll.setEnabled(true);
		}
        else
        {
            this.relTable.setRowSelectionAllowed(false);
			this.replaceField.setEnabled(false);
			this.replace.setEnabled(false);
			this.replaceAll.setEnabled(false);        
		}
	}
	
	
	public void setSearchHits(List<SearchHit> pHits)
	{
		this.getTableModel().setSearchHits(pHits);
	}
    
	
	/**
	 * Sets buttons and number of search results in search panel.
     * Scrolls to first hit.
	 */
	public void showSearchResult(String pKey)
	{
		this.searchField.setText(pKey);
		this.searchField.setEnabled(false);
		this.search.setText("Clear search");

		if (!this.getTableModel().hasSearchHits())
		{
			this.searchResultLabel.setText("0 / 0");
		}
		else
		{
			int curr = this.getTableModel().getCurrentHitIndex();
			SearchHit hit = this.getTableModel().getHit(curr);
			
			this.searchResultLabel.setText(curr+1 + " / " + this.getTableModel().getHitCount());
			this.next.setEnabled(true);
			this.nextFast.setEnabled(true);
			this.previous.setEnabled(true);
			this.previousFast.setEnabled(true);
			
			this.goTo(hit.getRowIndex(), RelationTableModel.COL_ATTRVALUE, hit.getStart(), hit.getEnd());
		}
	}
	

	/*
	 * Shows an empty relation that can be edited to contain tuples that shall be inserted.	 
	 */
	public void showInsertTable()
	{	
		List<String> attributeNames = this.relation.getAttributeNames();
		String[] colNames = new String[2];
		colNames[0] = this.getTableModel().getColumnName(RelationTableModel.COL_ATTRNAME);
		colNames[1] = this.getTableModel().getColumnName(RelationTableModel.COL_ATTRVALUE);
		
		String[][] insertData = new String[attributeNames.size()][2];
		
		for (int i = 0; i < attributeNames.size(); i++) 
		{
			insertData[i][0] = attributeNames.get(i);			
			insertData[i][1] = "";
		}
		
		this.remove(relScroll);
		this.insertTable = new JTable(insertData, colNames);
		
		// set column width
		TableColumn column = this.insertTable.getColumnModel().getColumn(0);
		column.setMinWidth(this.relTable.getColumnModel().getColumn(1).getMinWidth()); 
		column.setMaxWidth(this.relTable.getColumnModel().getColumn(1).getMaxWidth()); 
		column.setCellRenderer(new LabelTableCellRenderer());

		// set cell renderers
		/*
		 column = this.relTable.getColumnModel().getColumn(0);
		
		column = this.relTable.getColumnModel().getColumn(1);
		column.setCellRenderer(this.tableCellRenderer);
		column.setCellEditor(this.tableCellEditor);
		
		column = this.relTable.getColumnModel().getColumn(2);
		column.setCellRenderer(this.tableCellRenderer);
		column.setCellEditor(this.tableCellEditor);
		*/
		
		this.insertScroll.setViewportView(insertTable);		
		this.splitPane.setTopComponent(this.relScroll);	
		this.splitPane.setBottomComponent(this.insertScroll);
		this.splitPane.setResizeWeight(0.5);
		this.add(this.splitPane, BorderLayout.CENTER);
		
		this.revalidate();
		this.repaint();
	}
	
	
	/**
	 * Shows specified search hit (if valid)
	 * scrolls table, sets text and number of current hit in search panel.
	 */
	public boolean showHit(int pIndex)
	{
		SearchHit hit = this.getTableModel().getHit(pIndex);
		if (hit != null)
		{
			this.getTableModel().setCurrentHitIndex(pIndex);			
			this.goTo(hit.getRowIndex(), RelationTableModel.COL_ATTRVALUE, hit.getStart(), hit.getEnd());
			this.searchResultLabel.setText((pIndex+1) + " / " + this.getTableModel().getHitCount());
			return true;
		}
		return false;
	}
	
	/*
	public void tableChanged(TableModelEvent event) 
	{
		Reporter.debug("RelationPanel.tableChanged: " + event.getFirstRow() + ", " + event.getColumn());
		//this.correctRowHeight(event.getFirstRow());
		
         this.relTable.revalidate();
		this.relTable.repaint();
		this.validate();
		this.repaint();
	}
*/
	
	
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
			// TODO
			/*
			if (insertTable.isEditing()) {
				int editedRow = insertTable.getEditingRow();
				int editedColumn = insertTable.getEditingColumn();
				CellEditor editor = insertTable.getCellEditor(editedRow, editedColumn);
				editor.stopCellEditing();
			}
			*/
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
            if (e.getValueIsAdjusting())
            {
                int rowIndex = e.getLastIndex();
                this.getTableModel().addDeletion(rowIndex);
                
                this.relTable.revalidate();
            }
        }
    }
}

