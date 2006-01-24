/* 
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

   04-2005, Matthias Zielke


*/
package viewer;


import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.*;
import javax.swing.table.*;
import javax.swing.event.*;
import gui.SecondoObject;
import viewer.update.*;
import sj.lang.*;


/*
This specialized viewer shall only be used for updating relations. Therefore it can not
be called for any object retrieved by commands from the CommandPanel. It asks itself for
relations to be loaded and displays them. Afterwards update-operations can be applied to 
the relation and the changes are finally sent to SECONDO. If succesfull the new updated
relation will be shown, otherwise an errormessage should be displayed.

*/
public class UpdateViewer extends SecondoViewer implements TableModelListener {

	private String viewerName = "UpdateViewer";

	// Needed to build the actionPanel that offers the user possible update-operations
	private JPanel actionPanel;

	private JTextField relField;

	private JButton load;
	
	private JButton clear;

	private JButton insert;

	private JButton delete;

	private JButton update;

	private JButton reset;

	private JButton commit;

	// the controller decides which action shall be taken next and listens to all buttons
	// for user-input
	private ActionController controller;

	// ScrollPanes to show the update-relation or a relation with new tuples to be inserted
	private JScrollPane relScroll;

	private JScrollPane insertScroll;

	// Dialog to show errormessages
	private JDialog errorDialog;
	
	// Dialog to specify relation to be loaded and filters to be applied
	private LoadDialog loadDialog;

	// stores the data actually shown in the viewer
	private String[][] relData;
	
	// stores the data retrieved from secondo for the originally loaded relation
	private String[][] originalData;
	
	// Contains the IDs of all tuples of the actual relation
	private Vector tupleIds;

	// Shows the relation currently edited
	private JTable relTable;

	private String[] head;

	private String[] attrTypes;

	// In updatemode contains true for each updated attribute for each record
	private boolean[][] changedCells;
	// For each updated row an entry with its index is inserted
	private Vector changedRows;
	// Contains all changed Cells in the order they were edited
	private Vector updatesOrdered;
	// Contains the information of the inserted tuples
	private String[][] insertData;
	// Shows the relation to insert new tuples
	private JTable insertTable;
	// Each updated attribute is marked by this renderer
	private DefaultTableCellRenderer renderer;

	private boolean relEditable;

/*
Builds the viewer and sets the intital values

*/
	public UpdateViewer() {
		setLayout(new BorderLayout());		
		controller = new ActionController(this);
//		Build actionpanel
		actionPanel = new JPanel();
		actionPanel.setLayout(new GridLayout(1, 7));
		load = new JButton("Load Relation");
		load.addActionListener(controller);
		actionPanel.add(load);
		clear = new JButton("Clear");
		clear.addActionListener(controller);
		actionPanel.add(clear);
		insert = new JButton("Insert");
		insert.addActionListener(controller);
		actionPanel.add(insert);
		delete = new JButton("Delete");
		delete.addActionListener(controller);
		actionPanel.add(delete);
		update = new JButton("Update");
		update.addActionListener(controller);
		actionPanel.add(update);
		reset = new JButton("Reset");
		reset.addActionListener(controller);
		actionPanel.add(reset);
		commit = new JButton("Commit");
		commit.addActionListener(controller);
		actionPanel.add(commit);
		add(actionPanel, BorderLayout.NORTH);
		renderer = new DefaultTableCellRenderer();
		renderer.setBackground(Color.YELLOW);
		relEditable = false;
		setSelectionMode(ActionController.INITIAL);
	}

/*
Get the name of this viewer.
The name is used in the menu of the MainWindow.

*/
	public String getName() {
		return viewerName;
	}

/*
Because this viewer shall not display objects others than relations loaded
by the viewer itself only false is returned.

*/
	public boolean addObject(SecondoObject o) {
		return false;
	}

/*
Because this viewer shall not display objects others than relations loaded
by the viewer itself no object shall be removed.

*/
	public void removeObject(SecondoObject o) {

	}

/*
Because this viewer shall not display objects others than relations loaded
by the viewer itself no objects shall be removed.

*/
	public void removeAll() {

	}
/*
Because this viewer shall not display objects others than relations loaded
by the viewer itself false is returned.

*/
	public boolean canDisplay(SecondoObject o) {
		return false;
	}

/*
Because this viewer shall not display objects others than relations loaded
by the viewer itself false is returned.

*/
	public boolean isDisplayed(SecondoObject o) {
		return false;
	}
	
/*
Because this viewer shall not display objects others than relations loaded
by the viewer itself false is returned.

*/
	public boolean selectObject(SecondoObject O) {
		return false;
	}

/*
Because all commands for this viewer can easily be accessed from the actionPanel
no MenuVector is built.

*/
	public MenuVector getMenuVector() {
		return null;
	}

/*
Because this viewer shall not display objects others than relations loaded
by the viewer itself 0 is returned.

*/
	public double getDisplayQuality(SecondoObject SO) {
		return 0;
	}

	
/*
Shows a dialog with the errorText.

*/
	public void showErrorDialog(String errorText) {
		JOptionPane.showMessageDialog(this, errorText);
		this.repaint();
		this.validate();
	}

	
/*
If the paramater 'relation' is really a relation it will be shown in the viewer. Otherwise
false will be returned.
  
*/
	public boolean showNewRelation(ListExpr relation) {
		relTable = createTableFrom(relation);
		if (relTable == null) {
			return false;
		}
		relScroll = new JScrollPane();
		add(relScroll, BorderLayout.CENTER);
		relTable.setRowSelectionAllowed(false);
		relTable.setColumnSelectionAllowed(false);
		relEditable = false;
		TableModel model = relTable.getModel();
		model.addTableModelListener(this);
		relScroll.setViewportView(relTable);
		this.repaint();
		this.validate();
		return true;
	}

/*
Shows the original relation retrieved from SECONDO. All updates that have not been
commited yet will be lost.

*/
	public void showOriginalRelation() {
		relTable.setRowSelectionAllowed(false);
		relTable.setColumnSelectionAllowed(false);
		relEditable = false;
		for (int i = 0; i < relData.length; i++) {
			for (int j = 0; j < relData[0].length; j++) {
				changedCells[i][j] = false;
				changedRows.clear();
				relData[i][j] = new String(originalData[i][j]);
			}
		}
		relTable.clearSelection();
		relTable.invalidate();
		this.repaint();
		this.validate();
	}

/*
Removes the relation that could be edited to insert new tuples.	 

*/
	public void removeInsertRelation() {
		this.remove(insertScroll);
		insertData = null;
		insertTable = null;
		insertScroll = null;
		this.validate();
	}
	
/*
Removes the last added tuple from the insert-relation

*/
	public boolean removeLastInsertTuple(){
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
			this.add(insertScroll, BorderLayout.SOUTH);
			this.validate();
			this.repaint();
			return true;
		}
	}

/*
Removes the relation actually shown from this viewer. All information about this relation
will be lost.	 

*/
	public void removeRelation() {
		this.remove(relScroll);
		relData = null;
		relTable = null;
		head = null;
		attrTypes = null;
		changedCells = null;
		changedRows = null;
		originalData = null;
		this.validate();
		this.repaint();
	}

	
/*
For each mode and state the viewer is in only certain operations and choices are possible.
This method assures only the actually allowed actions can be executed or chosen.	 

*/
	public void setSelectionMode(int selectMode) {
		switch (selectMode) {
		case ActionController.INITIAL: {
			insert.setBackground(Color.LIGHT_GRAY);
			delete.setBackground(Color.LIGHT_GRAY);
			update.setBackground(Color.LIGHT_GRAY);
			clear.setEnabled(false);
			insert.setEnabled(false);
			delete.setEnabled(false);
			update.setEnabled(false);
			reset.setEnabled(false);
			commit.setEnabled(false);
			break;
		}
		case ActionController.LOADED: {
			insert.setBackground(Color.LIGHT_GRAY);
			delete.setBackground(Color.LIGHT_GRAY);
			update.setBackground(Color.LIGHT_GRAY);
			clear.setEnabled(true);
			insert.setEnabled(true);
			delete.setEnabled(true);
			update.setEnabled(true);
			reset.setEnabled(false);
			commit.setEnabled(false);
			break;
		}
		case ActionController.INSERT: {
			insert.setBackground(Color.YELLOW);
			delete.setEnabled(false);
			update.setEnabled(false);
			reset.setEnabled(true);
			commit.setEnabled(true);
			relTable.setRowSelectionAllowed(false);
			relTable.setColumnSelectionAllowed(false);
			relEditable = false;
			break;
		}
		case ActionController.DELETE: {
			delete.setBackground(Color.YELLOW);
			insert.setEnabled(false);
			delete.setEnabled(false);
			update.setEnabled(false);
			reset.setEnabled(true);
			commit.setEnabled(true);
			relTable.setRowSelectionAllowed(true);
			relEditable = false;
			break;
		}
		case ActionController.UPDATE: {
			update.setBackground(Color.YELLOW);
			insert.setEnabled(false);
			delete.setEnabled(false);
			update.setEnabled(false);
			reset.setEnabled(true);
			commit.setEnabled(true);
			relTable.setRowSelectionAllowed(false);
			relTable.setColumnSelectionAllowed(false);
			relEditable = true;
			for (int i = 0; i < relData.length; i++) {
				for (int j = 0; j < relData[0].length; j++) {
					changedCells[i][j] = false;
				}
			}
			break;
		}
		default:
			showErrorDialog("The mode: " + selectMode + " is not known");
			break;
		}
	}

/*
Creates a JTable from the parameter LE. If LE doesn't represent a relation 'null' will be
returned. The members that represent the original relation-data and the actual relation-data
are initialized. The IDs of the tuples of the relation are stored in a seperate vector	 

*/
	private JTable createTableFrom(ListExpr LE) {
		boolean result = true;
		JTable NTable = null;
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
			head = new String[tupleLength];
			attrTypes = new String[tupleLength];
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
				tupleIds = new Vector();
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
				changedCells = new boolean[V.size()][head.length];
				changedRows = new Vector();
				updatesOrdered = new Vector();

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

		if (result)
			return NTable;
		else
			return null;
	}

/*
If the table was edited this method is called. Registers which cell was edited and
prepares this cell to be marked differently with the next 'repaint'.	 

*/
	public void tableChanged(TableModelEvent e) {
		int row = e.getFirstRow();
		int column = e.getColumn();
		int[] lastChanged = {row, column};
		updatesOrdered.add(lastChanged);
		changedCells[row][column] = true;
		changedRows.add(new Integer(row));
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
			if (insertTable.isEditing()) {
				int editedRow = insertTable.getEditingRow();
				int editedColumn = insertTable.getEditingColumn();
				CellEditor editor = insertTable.getCellEditor(editedRow, editedColumn);
				editor.stopCellEditing();
			}
			
		}
		
		
	}

/*
Shows an empty relation that can be edited to contain tuples that shall be inserted.	 

*/
	public void showInsertRelation() {
		insertData = new String[1][relTable.getColumnCount()];
		for (int i = 0; i < relTable.getColumnCount(); i++) {
			insertData[0][i] = "";
		}
		insertTable = new JTable(insertData, head);
		insertScroll = new JScrollPane() {
			public Dimension getPreferredSize() {
				return new Dimension(this.getParent().getWidth(), this
						.getParent().getHeight() / 5);
			}
		};
		insertScroll.setViewportView(insertTable);
		this.add(insertScroll, BorderLayout.SOUTH);
		this.validate();
		this.repaint();
	}

/*
Is called when yet at least one insert-tuple was edited	and another one shall be
edited. Takes over the already edited insert-tuples and shows one more empty tuple. 

*/
	public void addInsertTuple() {
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
		insertScroll.setViewportView(insertTable);
		this.add(insertScroll, BorderLayout.SOUTH);
		this.validate();
		this.repaint();

	}

/*
Returns the text inserted by the user into the JTextField. It should be the name of a
relation.	 

*/
	public String getLoadName() {
		return loadDialog.getLoadName();
	}
	
/*
Returns all the filters that shall be applied when the next relation will be loaded	 

*/
	public String[] getFilters() {
		return loadDialog.getFilters();
	}
/*
Returns true if the 'loadDialog' was cancelled

*/	
	public boolean loadCancelled() {
		return loadDialog.loadCancelled();
	}


/*
Returns the types of all attributes of the actually loaded relation.	 

*/
	public String[] getAttrTypes() {
		return attrTypes;
	}

/*
Returns the values of all tuples that were edited to be inserted.	 

*/
	public String[][] getInsertTuples() {
		return insertData;
	}

/*
Returns the values of all tuples that were marked to be deleted.	 

*/
	public String[][] getDeleteTuples() {
		int[] selectedRows = relTable.getSelectedRows();
		String[][] deleteTuples = new String[selectedRows.length][relData[0].length];
		for (int i = 0; i < selectedRows.length; i++) {
			deleteTuples[i] = relData[selectedRows[i]];
		}
		return deleteTuples;
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
Returns the indices of all tuples that were marked to be deleted.	 

*/
	public int[] getDeleteRows(){
		return relTable.getSelectedRows();
	}

/*
Returns the original values of the tuple at position 'index'.	 

*/
	public String[] getOriginalTuple(int index) {
		return originalData[index];
	}
	
/*
Returns the tupleId of the record at position 'index'.

*/
	public String getTupleId(int index){
		return (String)(tupleIds.get(index));
	}

/*
Returns an array with the indices of all updated tuples.	 

*/
	public int[] updatedTuples() {
		int[] updateTuples = new int[changedRows.size()];
		for (int i = 0; i < updateTuples.length; i++) {
			updateTuples[i] = ((Integer) changedRows.get(i)).intValue();
		}
		return updateTuples;
	}

/*
Returns the values of the updated tuple at position 'index'.	 

*/
	public String[] getUpdateTuple(int index) {
		return relData[index];
	}

/*
For the tuple at position 'index' returns all indices of the attributes that have
been changed inside this tuple.	 

*/
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
	
/*
Resets the last editited cell in updatemode to its original value.

*/
	public boolean resetLastUpdate(){
		if (changedRows.size() > 0) {
			int[] lastChanged = (int[]) updatesOrdered.lastElement();
			changedCells[lastChanged[0]][lastChanged[1]] = false;
			changedRows.remove(changedRows.size() - 1);
			updatesOrdered.remove(updatesOrdered.size() - 1);
			relData[lastChanged[0]][lastChanged[1]] = originalData[lastChanged[0]][lastChanged[1]];
			this.repaint();
			this.validate();
			return true;
		}
		else{
			return false;
		}
	}

/* Resets all updates */
public boolean resetUpdates(){
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
   return true;

}
  

/** Start edititing the cell at the given position. 
  *  All errors (wrong arguments, cell is not editable and so on) are
  *  ignored. 
  **/

  public void relGoTo(int x, int y){
    try{
      relTable.editCellAt(x,y,null); 
    } catch(Exception e){
      if(gui.Environment.DEBUG_MODE)
        e.printStackTrace();
    }
  }

  /** Starts the edititing of the specified cell within the insert table **/
  public void insertGoTo(int x, int y){
     try{
       insertTable.editCellAt(x,y,null);
     }catch(Exception e){
       if(gui.Environment.DEBUG_MODE)
          e.printStackTrace();
     }
  }


/*
Returns the names of the attributes of the tuples of the currently loaded relation.	 

*/
	public String[] getAttrNames() {
		return head;
	}

	
/*
Shows a dialog to let the user specifiy which relation he wants to load and which filter-
criteria he wants to apply.

*/
		public void showLoadDialog() {
			if (loadDialog == null)
				loadDialog = new LoadDialog();
			else
				loadDialog.clear();
			loadDialog.setSize(600, 500);
			loadDialog.show();
		}



	
/*
This class implements a dialog in which the user can specify which relation he wants to
load an which filter-criteria he wants to apply.
	 
*/
		private class LoadDialog extends JDialog implements ActionListener {

			private final static String titleString = "Load relation";
			
			//relationPanel
			JPanel relPanel ;
			JLabel relNameLabel;
			JTextField relField;
			
			//filterPanel
			JPanel filterPanel ;
			JPanel addPanel;
			JButton addFilter;
			JScrollPane filterScroll;
			JPanel criteria ;
			Vector removeButtons;
			Vector criteriaPanels;
			Vector criteriaFields;
			
			//commitPanel
			JPanel commitPanel;
			JButton commit;
			JButton cancel;
			
			
			
			private boolean cancelled;

			
	/*
	Builds the visible dialog. 
	*/
			public LoadDialog() {
				//General settings
				this.setModal(true);
				this.setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
				this.getContentPane().setLayout(new BorderLayout());
				this.setTitle(titleString);
				
				//Build relationPanel
				relPanel = new JPanel();
				relNameLabel = new JLabel("relation-name");
				relPanel.add(relNameLabel);
				relField = new JTextField(20);
				relPanel.add(relField);
				this.getContentPane().add(relPanel, BorderLayout.NORTH);
				
				//Build filterPanel
				filterPanel = new JPanel(new BorderLayout());
				addPanel = new JPanel();
				addFilter = new JButton("add filter");
				addFilter.addActionListener(this);
				addPanel.add(addFilter);
				filterPanel.add(addPanel,BorderLayout.NORTH);
				filterScroll = new JScrollPane();
				criteria = new JPanel();
				filterScroll.setViewportView(criteria);
				filterPanel.add(filterScroll, BorderLayout.CENTER);
				this.getContentPane().add(filterPanel, BorderLayout.CENTER);
				
				//Build commitPanel
				commitPanel = new JPanel();
				cancel = new JButton("cancel");
				cancel.addActionListener(this);
				commitPanel.add(cancel);
				commit = new JButton("commit");
				commit.addActionListener(this);
				commitPanel.add(commit);
				this.getContentPane().add(commitPanel, BorderLayout.SOUTH);
				removeButtons = new Vector();
				criteriaPanels = new Vector();
				criteriaFields = new Vector();
				
				
			}

/*
Is called if any of the buttons of the dialog was pressed.

*/	
			public void actionPerformed(ActionEvent e) {
				if (e.getSource() == addFilter) {
					JPanel nextFilterPanel = new JPanel();
					JTextField nextField = new JTextField(20);
					JButton nextRemove = new JButton("remove");
					nextRemove.addActionListener(this);
					nextFilterPanel.add(nextField);
					nextFilterPanel.add(nextRemove);
					criteriaPanels.add(nextFilterPanel);
					removeButtons.add(nextRemove);
					criteriaFields.add(nextField);
					rebuildCriteriaPanel();
				}
				if (e.getActionCommand().equals("remove")){
					for (int i = 0; i < removeButtons.size(); i++){
						if(e.getSource() == removeButtons.get(i)){
							removeButtons.remove(i);
							criteriaFields.remove(i);
							criteriaPanels.remove(i);
							break;
						}
					}
					rebuildCriteriaPanel();
				}
				if (e.getSource() == commit) {
					cancelled = false;
					this.hide();
				}
				if (e.getSource() == cancel) {
					cancelled = true;
					this.hide();
				}
				
			}
			
/*
Rebuilds the panel which contains the filter-conditions and sets an adjusted new
GridLayout to have a better view.

*/
			private void rebuildCriteriaPanel(){
				criteria.removeAll();
				if (criteriaPanels.size() < 8)
					criteria.setLayout(new GridLayout(8,1));
				else
					criteria.setLayout(new GridLayout(criteriaPanels.size(),1));
				for (int i = 0; i < criteriaPanels.size(); i++){
					criteria.add((JPanel)criteriaPanels.get(i));
				}
				this.validate();
				this.repaint();
			}
			
/*
Clears the dialog from all previously edited data.


*/
			public void clear(){
				removeButtons.clear();
				criteriaFields.clear();
				criteriaPanels.clear();
				relField.setText("");
				rebuildCriteriaPanel();
			}
/*
Returns the name of the JTextField for the relation.
				  
*/			
			
			public String getLoadName(){
				return relField.getText().trim();
			}
			
/*
Returns all the filter-conditions of the JTextFields for the filters.
							  
*/	
			public String[] getFilters(){
				String[] filters = new String[criteriaFields.size()];
				for (int i = 0; i < filters.length; i++){
					filters[i] = ((JTextField) criteriaFields.get(i)).getText().trim();
				}
				return filters;
			}
/*
Returns true if this dialog was closed by pressing 'cancel'.
										  
*/				
			public boolean loadCancelled(){
				return cancelled;
			}

		}


}
