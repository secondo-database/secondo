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
import java.awt.Composite;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Frame;
import java.awt.Graphics;
import java.awt.GridLayout;
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

import java.io.File;
import java.io.FileInputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import java.util.StringTokenizer;
import java.util.TreeSet;
import java.util.Vector;

import javax.swing.AbstractAction;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.CellEditor;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButtonMenuItem;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.JSplitPane;
import javax.swing.JTable;
import javax.swing.JTextArea;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.KeyStroke;
import javax.swing.ListModel;
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
public class RelationPanel extends JPanel implements TableModelListener, 
														PropertyChangeListener
{
	
	private String name;
	
	private Relation relation;
	
	// controller listens to action events
	private UpdateViewerController controller;
	
	private JTable relTable;
	
	private JScrollPane relScroll;
		
	
	// components to display the relation in insert mode
	private JScrollPane insertScroll;

	// shows the relation currently edited
	private JTable insertTable;	

	// contains changes in chronological order
	private List<Change> changes;
	
	private ValueTableCellEditor tableCellEditor;
	
	private String oldEditCellValue;
	
	
		
	/**
	 * Builds a panel to display one relation
	 */
	public RelationPanel(String pRelationName, UpdateViewerController pController) 
	{
		this.name = pRelationName;
		this.setLayout(new BorderLayout());		
		this.controller = pController;
		this.changes = new ArrayList<Change>();
	}
	
	
	/*
	 Is called when yet at least one insert-tuple was edited	and another one shall be
	 edited. Takes over the already edited insert-tuples and shows one more empty tuple. 	 
	 */
	public void addInsertTuple() {
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
	public void addInsertRow() {
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
	 * Computes and sets height of column 2
	 * which may contain long text values.
	 */
	private void correctRowHeight() 
	{
		int width = relTable.getColumnModel().getColumn(2).getWidth();
		for (int row = 0; row < relTable.getRowCount(); row++) 
		{
			JTextArea area = new JTextArea();
			area.setLineWrap(true);
			area.setWrapStyleWord(true);
			area.setSize(width, Short.MAX_VALUE);
			area.setText(relTable.getValueAt(row, 2).toString());
			relTable.setRowHeight(row, area.getPreferredSize().height);
		}
	}

	
	
	/**
	 * Creates a JTable from the parameter LE. If LE doesn't represent a relation 'null' will be
	 * returned. The members that represent the original relation-data and the actual relation-data
	 * are initialized. The IDs of the tuples of the relation are stored in a seperate vector	 
	 *
	 */
	public boolean createTableFrom(ListExpr LE) 
	{
		boolean result = true;
		SecondoObject relationSO = new SecondoObject(this.getName(), LE);
		this.relation = new Relation();
		this.relation.readFromSecondoObject(relationSO);
		
		try
		{
			RelationTableModel relationTM = new RelationTableModel(relation);
			this.relTable = new JTable(relationTM);
	
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
			column.setCellRenderer(new ValueTableCellRenderer());
			
			// set cell editor
			this.tableCellEditor = new ValueTableCellEditor();
			column.setCellEditor(tableCellEditor);
			
			// set listeners
			this.relTable.addPropertyChangeListener(this);
			
			/*
			 this.relTable.getModel().addTableModelListener(this);
			
			this.relTable.addComponentListener(new ComponentListener() {
									   public void componentHidden(ComponentEvent e) {}
									   public void componentMoved(ComponentEvent e) {}
									   
									   public void componentResized(ComponentEvent e) {
											   correctRowHeight();
									   }
									   
									   public void componentShown(ComponentEvent e) {
											   correctRowHeight();
									   }
									   });			 
			 */
			
			this.relScroll = new JScrollPane(relTable);
			this.add(relScroll);
		}
		catch(InvalidRelationException e)
		{
			Reporter.showError(e.getMessage());
		}
		
		return result;
	}
	
	
	/*
	 * Returns the relation data.	 	 
	 */
	public Relation getRelation()
	{
		return this.relation;
	}
	
	
	/*
	 * Returns all changes in chronological order.	 
	 */	
	public List<Change> getChanges()
	{
		return this.changes;
	}	
	
	/*
	 * Returns only valid (=last change of attribute) changes mapped by tuple index and attribute name.	 
	 */	
	public Map<Integer, HashMap<String, Change>> getChangedTuples()
	{
		Map<Integer, HashMap<String, Change>> result = new HashMap<Integer, HashMap<String, Change>>();
		
		Integer tid;
		String attrName;
		HashMap<String, Change> map;		
		
		for (Change ch : this.changes)
		{
			Reporter.debug("RelationPanel.getChangedTuples: " + ch.toString());
			tid = ch.getTupleIndex();
			attrName = ch.getAttributeName();
			
			map = result.get(tid);
			if (map == null)
			{
				map = new HashMap<String, Change>();
			}
			map.put(attrName, ch);	
			
			if (!map.isEmpty())
			{
				result.put(tid, map);
			}
		}
		
		for (Integer i : result.keySet())
		{
			for (String s : result.get(i).keySet())
			{
				Reporter.debug("RelationPanel.getChangedTuples result is :" + result.get(i).get(s).toString());
			}
		}
		return result;
	}	 
	
	
	/*
	 For the tuple at position 'index' returns all indices of the attributes that have
	 been changed inside this tuple.	 

	public int[] getChangedAttributes(int index) {
		boolean[] attrs = changedCells[index];
		Vector changedAttrs = new Vector();
		for (int i = 0; i < changedCells[index].length; i++) {
			if (changedCells[index][i]) {
				changedAttrs.add(new Integer(i));
			}
		}
		int[] result = new int[changedAttrs.size()];
		for (int i = 0; i < result.length; i++) {
			result[i] = ((Integer) changedAttrs.get(i)).intValue();
		}
		return result;
	}	 
	 */		
	
	/*
	 * Returns values to be updated.

	public List<SecondoObject> getUpdateValues()
	{
		List<SecondoObject> result = new ArrayList<SecondoObject>();
		
		for (Integer index : this.getUpdateTupleIndices())
		{
			result.add(this.relation.getTupleAt(index));
		}
		
		return result;
	}
	 */	
	
	/*
	 * Returns indices of all values to be updated.

	public List<Integer> getUpdateValueIndices()
	{
		List<Integer> result = new ArrayList<Integer>();
		
		for (Change ch : this.changes)
		{
			if (!result.contains(ch.getIndex()))
			{
				result.add(ch.getIndex());
			}
		}
		
		return result;
	}
	 */
	
	
	
	/*
	 * Returns indices of all tuples that were marked to be deleted.	 	 
	 */
	public int[] getDeleteRows(){
		return relTable.getSelectedRows();
	}
	
	/*
	 Returns the values of all tuples that were marked to be deleted.	 	 

	public String[][] getDeleteTuples() {
		int[] selectedRows = relTable.getSelectedRows();
		String[][] deleteTuples = new String[selectedRows.length][relData[0].length];
		for (int i = 0; i < selectedRows.length; i++) {
			deleteTuples[i] = relData[selectedRows[i]];
		}
		return deleteTuples;
	}
	 */	
	
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
	
	
	/**
	 * Returns RelationTabelModel of currently displayed Table. (Convenience shortcut)
	 */
	private RelationTableModel getTableModel()
	{
		return (RelationTableModel)this.relTable.getModel();
	}
	
	
	/*
	 Returns the original values of the tuple at position 'index'.	 	 
	public String[] getOriginalTuple(int index) {
		return originalData[index];
	}
	 */



	
	
	/** Starts the editing the specified cell within the insert table **/
	public void insertGoTo(int x, int y)
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
	
	/**
	 * Saves old value of currently active editable table cell.
	 */
	private void processEditingStarted()
	{
		this.oldEditCellValue = (String)this.tableCellEditor.getCellEditorValue();
		
		//Reporter.debug("processEditingStarted: old value in cell is " + this.relTable.getSelectedRow() 
		//			   + ", " + this.relTable.getSelectedColumn() + this.oldEditCellValue);		
	}
	
	
	/**
	 * Updates table model with new value and creates a Change
	 * if user actually edited the table cell.
	 */
	private void processEditingStopped()
	{
		int row = this.relTable.getEditingRow();
		int col = this.relTable.getEditingColumn();

		String newValue = (String)this.tableCellEditor.getCellEditorValue();
		
		if (!newValue.equals(this.oldEditCellValue))
		{
			// cell value was changed -> write change back into table model
			this.relTable.setValueAt(newValue, row, col);
			
			// create Change for update or undo actions
			RelationTableModel rtm = (RelationTableModel)this.relTable.getModel();
			int tupleIndex = Integer.valueOf((String)rtm.getValueAt(row,0));
			int attributeIndex = rtm.rowToAttributeIndex(row);
			String attributeName = this.relation.getAttributeNames().get(attributeIndex);
			String attributeType = this.relation.getAttributeTypes().get(attributeIndex);
			
			Change change = new Change(tupleIndex, attributeIndex, 
									   attributeName, attributeType, 
									   this.oldEditCellValue, newValue);
			this.changes.add(change);
			
			//Reporter.debug("processEditingStopped: new value of table cell (" + row + ", " + col + ") is " + newValue) ;			
		}
	}
	
	/**
	*  Implemention of PropertyChangeListener interface.
	*/
	@Override
	public void propertyChange(PropertyChangeEvent e)
	{
		//  A cell has started/stopped editing
		if ("tableCellEditor".equals(e.getPropertyName()))
		{
			if (relTable.isEditing())
			{
				this.processEditingStarted();
			}
			else
			{
				this.processEditingStopped();
			}
		}
	}
	
	/*
	 Removes the relation that could be edited to insert new tuples.	 	 
	 */
	public void removeInsertRelation() {
		// TODO
		/*
		this.remove(insertSplit);
		this.add(relScroll, BorderLayout.CENTER);
		insertData = null;
		insertTable = null;
		insertScroll = null;
		 */
		this.validate();
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
	
	
	/* 
	 * Resets all updates.
	 */
	public boolean resetUpdates(){
		// TODO
		/*
		if(relTable==null)
			return false;
		// reset the curretly editing cell if any
		if(relTable.isEditing()){
			int x = relTable.getEditingRow();
			int y = relTable.getEditingColumn();
			relTable.editingStopped(new ChangeEvent(this));
			relTable.setValueAt(originalData[x][y],x,y);
		} 
		for(int i=0;i<updatesOrdered.size();i++){
			int[] changed = (int[]) updatesOrdered.get(i);
			changedCells[changed[0]][changed[1]] = false;
			relData[changed[0]][changed[1]] = originalData[changed[0]][changed[1]];
		}
		updatesOrdered.clear();
		changedRows.clear();
		relTable.revalidate();
		relTable.repaint();
		*/
		
		return false;
		
	}
	
	
	/** Start edititing the cell at the given position. 
	 *  All errors (wrong arguments, cell is not editable and so on) are
	 *  ignored. 
	 **/
	
	public void relGoTo(int x, int y)
	{
		try{
			relTable.editCellAt(x,y,null); 
		} catch(Exception e){
			Reporter.debug(e);
		}
	}

	
	/*
	 Allows the user to reset delete-selections by pressing 'reset' 
	 
	 */
	public boolean resetDeleteSelections()
	{
		if (relTable.getSelectedRows().length > 0){
			relTable.clearSelection();
			return true;
		}
		else{
			return false;
		}
	}
	
	/*
	 Resets the last editited cell in updatemode to its original value.	 
	 */
	public boolean resetLastUpdate()
	{
		// TODO
		if (!this.changes.isEmpty()) {
					/*
			int[] lastChanged = (int[]) updatesOrdered.lastElement();
			changedCells[lastChanged[0]][lastChanged[1]] = false;
			changedRows.remove(changedRows.size() - 1);
			updatesOrdered.remove(updatesOrdered.size() - 1);
			relData[lastChanged[0]][lastChanged[1]] = originalData[lastChanged[0]][lastChanged[1]];
					 */
			this.repaint();
			this.validate();
			return true;
		}
		else{
			return false;
		}
	}
	

	/*
	 Shows an empty relation that can be edited to contain tuples that shall be inserted.	 
	 */
	public void showInsertRelation() {
		
		// TODO
		/*
		insertData = new String[1][relTable.getColumnCount()];
		for (int i = 0; i < relTable.getColumnCount(); i++) {
			insertData[0][i] = "";
		}
		this.insertTable = new JTable(insertData, head);
		this.addRemoveToInsertTable();
		this.insertScroll = new JScrollPane() {
			public Dimension getPreferredSize() {
				return new Dimension(this.getParent().getWidth(), this
									 .getParent().getHeight() / 5);
			}
		};
		this.insertScroll.setViewportView(insertTable);
		this.insertSplit.setTopComponent(relScroll);
		this.insertSplit.setBottomComponent(insertScroll);
		this.add(insertSplit, BorderLayout.CENTER); 
		this.revalidate();
		this.validate();
		this.repaint();
		insertSplit.revalidate();
		insertSplit.setDividerLocation(0.5);
		 */
	}
	
	
	
	/*
	 * If the paramater 'relation' is really a relation it will be shown in the viewer. 
	 * Otherwise false will be returned.	 
	 */
	public boolean showNewRelation(ListExpr pRelationLE) 
	{
		boolean created = createTableFrom(pRelationLE);
		if (created) 
		{
			relTable.setRowSelectionAllowed(false);
			relTable.setColumnSelectionAllowed(false);
			TableModel model = relTable.getModel();
			model.addTableModelListener(this);
			relScroll.setViewportView(relTable);
			this.repaint();
			this.validate();
		}
		return created;
	}
	
	/*
	 Shows the original relation retrieved from SECONDO. All updates that have not been
	 commited yet will be lost.	 
	 */
	public void showOriginalRelation() 
	{
		// TODO
		/*
		for (int i = 0; i < relData.length; i++) {
			for (int j = 0; j < relData[0].length; j++) {
				changedCells[i][j] = false;
				changedRows.clear();
				relData[i][j] = new String(originalData[i][j]);
			}
		}
		 */
		relTable.clearSelection();
		relTable.invalidate();
		this.repaint();
		this.validate();
	}
	
	
	
	/**
	 * Method of Interface TableModelListener.
	 * If the table was edited this method is called. Registers which cell was edited and
	 * prepares this cell to be marked differently with the next 'repaint'.	 
	 */
	public void tableChanged(TableModelEvent e) 
	{
		// set row heights
			
		int row = e.getFirstRow();
		int column = e.getColumn();
		int[] lastChanged = {row, column};
		
		// TODO
		/*
		updatesOrdered.add(lastChanged);
		changedCells[row][column] = true;
		changedRows.add(new Integer(row));
		 
		//this.correctRowHeight();
		 */
		//this.repaint();
		//this.validate();
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
	
	/*
	 Returns an array with the indices of all updated tuples.	 
	 
	public int[] updatedTuples() {
		int[] updateTuples = new int[changedRows.size()];
		for (int i = 0; i < updateTuples.length; i++) {
			updateTuples[i] = ((Integer) changedRows.get(i)).intValue();
		}
		return updateTuples;
	}
	 */

	
	/**
	 * Resets last modification.
	 */
	public boolean undo(){
		// TODO
		return false;
	}

}

