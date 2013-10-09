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
import java.awt.Frame;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.InputEvent;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.io.File;
import java.io.FileInputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;
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
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.KeyStroke;
import javax.swing.ListModel;
import javax.swing.SwingConstants;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.MouseInputAdapter;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableCellRenderer;
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
public class RelationPanel extends JPanel implements TableModelListener{
	
	private String name;
	
	private Relation relation;
	
	// controller listens to action and table events
	private UpdateViewerController controller;
	
	// components to display the relation in edit mode
	private RelationTableModel relTableModel;

	private JTable relTable;

	private DefaultTableCellRenderer renderer;
	
	private JScrollPane relScroll;
	
	
	// components to display the relation in insert mode
	private JScrollPane insertScroll;

	// shows the relation currently edited
	private JTable insertTable;	

	// contains unsaved changes in chronological order
	private List<Change> changes;
	
	
		
	/**
	 * Builds a panel to display one relation
	 */
	public RelationPanel(String pRelationName, UpdateViewerController pController) 
	{
		this.name = pRelationName;
		this.setLayout(new BorderLayout());		
		this.controller = pController;
		this.renderer = new DefaultTableCellRenderer();
		this.renderer.setBackground(Color.YELLOW);
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
	private void addRemoveToInsertTable(){
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
			
			// set column dimensions
			TableColumn column = this.relTable.getColumnModel().getColumn(0);
			int maxlength = relationTM.getMaxContentLength(0);
			column.setMaxWidth(maxlength*10); 
			column = this.relTable.getColumnModel().getColumn(1);
			maxlength = relationTM.getMaxContentLength(1);
			column.setMaxWidth(maxlength*10); 
				
			this.relScroll = new JScrollPane(relTable);
			this.add(relScroll);
		}
		catch(InvalidRelationException e)
		{
			Reporter.showError(e.getMessage());
		}

		
		/*
		if (LE.listLength() != 2)
			return null;
		else {
			ListExpr type = LE.first();
			ListExpr value = LE.second();
			// analyse type
			if (type.isAtom())
				return null;
			ListExpr maintype = type.first();
			if (type.listLength() != 2
				|| !maintype.isAtom()
				|| maintype.atomType() != ListExpr.SYMBOL_ATOM
				|| !(maintype.symbolValue().equals("rel") | maintype
					 .symbolValue().equals("mrel")))
				return null; // not a relation
			ListExpr tupletype = type.second();
			// analyse Tuple
			ListExpr TupleFirst = tupletype.first();
			if (tupletype.listLength() != 2
				|| !TupleFirst.isAtom()
				|| TupleFirst.atomType() != ListExpr.SYMBOL_ATOM
				|| !(TupleFirst.symbolValue().equals("tuple") | TupleFirst
					 .symbolValue().equals("mtuple")))
				return null; // not a tuple
			ListExpr TupleTypeValue = tupletype.second();
			// the table head
			// Don't count the last attribute which is the tupleidentifier of each tuple
			
			int tupleLength = TupleTypeValue.listLength() - 1;
			String[] head = new String[tupleLength];
			String[] attrTypes = new String[tupleLength];
			for (int i = 0; (i < tupleLength) && result; i++) {
				ListExpr TupleSubType = TupleTypeValue.first();
				if (TupleSubType.listLength() != 2)
					result = false;
				else {
					head[i] = TupleSubType.first().writeListExprToString();
					attrTypes[i] = TupleSubType.second()
					.writeListExprToString();
				}
				TupleTypeValue = TupleTypeValue.rest();
			}
			
			
			if (result) {
				// analyse the values
				ListExpr TupleValue;
				Vector V = new Vector();
				Vector tupleIds = new Vector();
				String[] row;
				ListExpr Elem;
				while (!value.isEmpty()) {
					TupleValue = value.first();
					row = new String[head.length];
					for(int pos=0;pos<head.length; pos++){
						Elem = TupleValue.first();
						LEFormatter LEF = AttributeFormatter.getFormatter(attrTypes[pos]);
						row[pos] = LEF.ListExprToString(Elem);
						TupleValue = TupleValue.rest();
					}
					V.add(row);
					// Get the tupleid of this tuple as the last attribute
					Elem = TupleValue.first();
					if (Elem.isAtom()
						&& Elem.atomType() == ListExpr.STRING_ATOM) {
						tupleIds.add(Elem.textValue());
					} else if ((Elem.isAtom() && Elem.atomType() == ListExpr.TEXT_ATOM)
							   || (!Elem.isAtom() && Elem.listLength() == 1
								   && Elem.first().isAtom() && Elem
								   .first().atomType() == ListExpr.TEXT_ATOM)) {
						if (!Elem.isAtom())
							Elem = Elem.first();
						tupleIds.add(Elem.textValue());
					} else
						tupleIds.add(TupleValue.first().writeListExprToString());
					value = value.rest();
				}
				//Initialize data
				relData = new String[V.size()][head.length];
				originalData = new String[V.size()][head.length];
				
				for (int i = 0; i < V.size(); i++) {
					relData[i] = (String[]) V.get(i);
					for (int j = 0; j < head.length; j++) {
						changedCells[i][j] = false;
						originalData[i][j] = new String(relData[i][j]);
					}
				}
				
				
				// a specialized constructor is needed to mark updated attributes
				// with a different renderer and to make the table only editable
				// in update-mode
				NTable = new JTable(relData, head) {
					public TableCellRenderer getCellRenderer(int row, int column) {
						if (changedCells[row][column]) {
							return renderer;
						} else
							return super.getCellRenderer(row, column);
					}
					
					public boolean isCellEditable(int row, int column) {
						return relEditable;
					}
				};
				
			}
		}
		 */
		
		return result;
	}
	
	
	/*
	 Returns the attribute names of the displayed relation.	 	 
	 */
	public String[] getAttrNames() {
		return this.relation.getAttributeNames();
	}
	
	
	/*
	 Returns the attribute types of the displayed relation.	 	 
	 */
	public String[] getAttrTypes() {
		return this.relation.getAttributeTypes();
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
	 Returns the indices of all tuples that were marked to be deleted.	 	 
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
	
	
	/*
	 Returns the original values of the tuple at position 'index'.	 	 
	public String[] getOriginalTuple(int index) {
		return originalData[index];
	}
	 */
	
	/*
	 Returns the tupleId of the record at position 'index'.
	 
	public String getTupleId(int index){
		return this.relation.tupleIds.get(index);
	}
	 */
	
	/*
	 Returns the values of the updated tuple at position 'index'.	 
	 
	public String[] getUpdateTuple(int index) {
		return relData[index];
	}
	 */
	
	
	/** Starts the editing the specified cell within the insert table **/
	public void insertGoTo(int x, int y){
		// TODO
		/*
		try{
			insertTable.editCellAt(x,y,null);
		}catch(Exception e){
			Reporter.debug(e);
		}
		 */
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
	
	public void relGoTo(int x, int y){
		try{
			relTable.editCellAt(x,y,null); 
		} catch(Exception e){
			Reporter.debug(e);
		}
	}

	
	/*
	 Allows the user to reset delete-selections by pressing 'reset' 
	 
	 */
	public boolean resetDeleteSelections(){
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
	public boolean resetLastUpdate(){
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
	
	public void setTableModel(RelationTableModel pRelTableModel)
	{
		this.relTableModel = pRelTableModel;
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
	public boolean showNewRelation(ListExpr pRelationLE) {
		// TODO analize relations from ListExpr
		
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
	public void showOriginalRelation() {
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
	public void tableChanged(TableModelEvent e) {
		int row = e.getFirstRow();
		int column = e.getColumn();
		int[] lastChanged = {row, column};
		// TODO
		/*
		updatesOrdered.add(lastChanged);
		changedCells[row][column] = true;
		changedRows.add(new Integer(row));
		 */
		this.repaint();
		this.validate();
	}
	
	/*
	 The last cell the user edited before he pressed "commit" is usually not considered
	 because "tableChanged" will only be called, if he pressed "return" or selected a different cell
	 of the JTable before "commiting". Therefore this method takes over the value of the
	 last edited cell that was not taken into consideration yet.
	 
	 */
	public void takeOverLastEditing(boolean updateMode){
		if (updateMode){
			if (relTable.isEditing()) {
				int editedRow = relTable.getEditingRow();
				int editedColumn = relTable.getEditingColumn();
				CellEditor editor = relTable.getCellEditor(editedRow, editedColumn);
				editor.stopCellEditing();
			}
			
		}
		else{
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

