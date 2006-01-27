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
package viewer.update;
 
import java.awt.event.*;
import java.util.Vector;

import viewer.*;
import sj.lang.*;

/*
This class controls the actionflow of update-operations in the 'UpdateViewer'.

*/
public class ActionController implements ActionListener{
	
	private CommandGenerator commandGenerator;
	private CommandExecuter commandExecuter;
	private UpdateViewer viewer;
	
	
	// The controller is always in one certain state 
	private int state;	
	public final static int INITIAL= 0;
	public final static int LOADED = 1;
	public final static int INSERT = 2;
	public final static int DELETE = 3;
	public final static int UPDATE = 4;
	// The name of the actually loaded and edited relation
	private String relName;
	// The filter-conditions to apply when the relation is loaded
	private String[] filters;
	
	// Indices for the actual relation
	private Vector btreeNames;
	private Vector btreeAttrNames;
	private Vector rtreeNames;
	private Vector rtreeAttrNames;
	
	//initializes the controller
	public ActionController(UpdateViewer viewer){
		this.viewer = viewer;
		commandGenerator = new CommandGenerator(viewer);
		commandExecuter = new CommandExecuter();
		state = INITIAL;
		
	}
	
/*
If any of the possible actions of the viewer was chosen, this method is called and decides 
according to the current state, which action shall be executed and what shall be the next
state.

*/
	public void actionPerformed(ActionEvent e){
		if (e.getActionCommand() == "Load Relation"){
			viewer.showLoadDialog();
			if (viewer.loadCancelled()){
				return;
			}
			if (state != INITIAL){
				viewer.removeRelation();
			}
			if (state == INSERT){
				viewer.removeInsertRelation();
			}
			String loadName = viewer.getLoadName().trim();
			filters = viewer.getFilters();
			loadRelation(loadName, filters); // This method sets the next state
			return;
		}
		if (e.getActionCommand() == "Clear"){
			viewer.removeRelation();
			if (state == INSERT){
				viewer.removeInsertRelation();
			}
			
			state = INITIAL;
			viewer.setSelectionMode(INITIAL);
			relName = null;
			filters = null;
			return;
		}
		if (e.getActionCommand() == "Insert"){
			if (state == INSERT){ // User wants to insert one more tuple
				viewer.takeOverLastEditing(false);
				viewer.addInsertTuple();
				return;
			}
			state = INSERT;
			viewer.setSelectionMode(INSERT);
			viewer.showInsertRelation();
			return;
		}
		if (e.getActionCommand() == "Delete"){
			state = DELETE;
			viewer.setSelectionMode(DELETE);
			return;
		}
		if (e.getActionCommand() == "Update"){
			state = UPDATE;
			viewer.setSelectionMode(UPDATE);
			return;
		}
		if (e.getActionCommand() == "Reset"){
			if(state == INSERT){
				if (! viewer.removeLastInsertTuple()){
					state = LOADED;
					viewer.showOriginalRelation();
					viewer.setSelectionMode(LOADED);
				}
			}
			if(state == UPDATE){
         viewer.resetUpdates();
         state = LOADED;
         viewer.setSelectionMode(LOADED);
      /* undo functionality */
			/*	if (! viewer.resetLastUpdate()){
					state = LOADED;
					viewer.showOriginalRelation();
					viewer.setSelectionMode(LOADED);
				}
       */
       
			}
			if(state == DELETE){
				if (! viewer.resetDeleteSelections()){
					state = LOADED;
					viewer.showOriginalRelation();
					viewer.setSelectionMode(LOADED);
				}
			}
			
			return;
		}
		if (e.getActionCommand() == "Commit"){
			boolean result = false;
			if(state == INSERT){
				result = executeInsert();
			}
			if(state == DELETE){
				result = executeDelete();
			}
			if(state == UPDATE){
				result = executeUpdate();
			}
			if ( result ){
				state = LOADED;
				viewer.setSelectionMode(LOADED);
			}
			return;
		}
		// This point should never be reached
		viewer.showErrorDialog("Command not known");
		
	}
	
/*
Begin transaction

*/
	private boolean beginTransaction(){
		if (commandExecuter.executeCommand("(begin transaction)", SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX)){
			return true;
		}
		else{
			return false;
		}
	}
	
/*
Commit transaction

*/
	private boolean commitTransaction(){
		if (commandExecuter.executeCommand("(commit transaction)", SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX)){
			return true;
		}
		else{
			return false;
		}
	}
	
/*
Abort transaction

*/
	private boolean abortTransaction(){
		if (commandExecuter.executeCommand("(abort transaction)", SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX)){
			return true;
		}
		else{
			return false;
		}
	}
	
/*
Executes all insertcommands generated by the 'CommandGenerator' in one big
transaction. If one of the commands was not succesfully executed the transaction
is aborted.

*/
	private boolean executeInsert(){
		viewer.takeOverLastEditing(false);
		String errorMessage;
    String[] insertCommands=null;
    try{
    		insertCommands = commandGenerator.generateInsert(relName,btreeNames, btreeAttrNames,
				rtreeNames, rtreeAttrNames);
    } catch(InvalidFormatException e){
        String message = e.getMessage()+"\n at position ("+e.row+", "+e.column+")";
        viewer.showErrorDialog(message);
        viewer.insertGoTo(e.row-1,e.column-1);
        return false;
    }
		if(! beginTransaction()){
			errorMessage = commandExecuter.getErrorMessage().toString();
			viewer.showErrorDialog(errorMessage);
			return false;
		}
		for (int i = 0; i < insertCommands.length; i++){
			if(! commandExecuter.executeCommand(insertCommands[i], SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX)){
				errorMessage = commandExecuter.getErrorMessage().toString();
				viewer.showErrorDialog("Error trying to insert a tuple: " + errorMessage);
				if (! abortTransaction()){
					errorMessage = commandExecuter.getErrorMessage().toString();
					viewer.showErrorDialog("Error trying to abort transaction: " + errorMessage);
				}
				return false;
			}
		}
		if(! commitTransaction()){
			errorMessage = commandExecuter.getErrorMessage().toString();
			viewer.showErrorDialog(errorMessage);
			return false;
		}		
		viewer.removeInsertRelation();
		viewer.removeRelation();
		return (loadRelation(relName,filters)); // the new state is set in this method
		
	}
/*	
Executes all deletecommands generated by the 'CommandGenerator' in one big
transaction. If one of the commands was not succesfully executed the transaction
is aborted.

*/
	private boolean executeDelete(){
		String errorMessage;
		String[] deleteCommands = commandGenerator.generateDelete(relName,btreeNames, btreeAttrNames,
				rtreeNames, rtreeAttrNames);;
		if(! beginTransaction()){
			errorMessage = commandExecuter.getErrorMessage().toString();
			viewer.showErrorDialog(errorMessage);
			return false;
		}
		int failures = 0;
		ListExpr result;
		for (int i = 0; i < deleteCommands.length; i++){
			if(! commandExecuter.executeCommand(deleteCommands[i], SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX)){
				errorMessage = commandExecuter.getErrorMessage().toString();
				viewer.showErrorDialog("Error trying to delete a tuple: " + errorMessage);
				if (! abortTransaction()){
					errorMessage = commandExecuter.getErrorMessage().toString();
					viewer.showErrorDialog("Error trying to abort transaction: " + errorMessage);
				}
				return false;
			}
			result = commandExecuter.getResultList();
			if ( result.second().intValue() != 1)
				failures++;
			
		}
		if(! commitTransaction()){
			errorMessage = commandExecuter.getErrorMessage().toString();
			viewer.showErrorDialog(errorMessage);
			return false;
		}
		if (failures > 0){
			if (failures > 1)
				viewer.showErrorDialog("Warning: " + failures + " tuples have already been deleted by a different user!");
			else
				viewer.showErrorDialog("Warning: One tuple has already been deleted by a different user!");
		}
		viewer.removeRelation();		
		return (loadRelation(relName, filters)); // the new state is set in this method
		
	}
/*	
Executes all updatecommands generated by the 'CommandGenerator' in one big
transaction. If one of the commands was not succesfully executed the transaction
is aborted.

*/
	private boolean executeUpdate(){
		viewer.takeOverLastEditing(true); 
		String errorMessage;
    String[] updateCommands;
		try{
        updateCommands = commandGenerator.generateUpdate(relName,btreeNames, btreeAttrNames,
				rtreeNames, rtreeAttrNames);;
    } catch(InvalidFormatException e){
        String message = e.getMessage()+"\n at position ("+e.row+", "+e.column+")";
        viewer.showErrorDialog(message);
        viewer.relGoTo(e.row-1,e.column-1);
        return false;
    }
		if(! beginTransaction()){
			errorMessage = commandExecuter.getErrorMessage().toString();
			viewer.showErrorDialog(errorMessage);
			return false;
		}
		int failures = 0;
		ListExpr result;
		for (int i = 0; i < updateCommands.length; i++){
			if(! commandExecuter.executeCommand(updateCommands[i], SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX)){
				errorMessage = commandExecuter.getErrorMessage().toString();
				viewer.showErrorDialog("Error trying to update a tuple: " + errorMessage);
				if (! abortTransaction()){
					errorMessage = commandExecuter.getErrorMessage().toString();
					viewer.showErrorDialog("Error trying to abort transaction: " + errorMessage);
				}
				return false;
			}
			result = commandExecuter.getResultList();
			if ( result.second().intValue() != 1)
				failures++;
		}
		if(! commitTransaction()){
			errorMessage = commandExecuter.getErrorMessage().toString();
			viewer.showErrorDialog(errorMessage);
			return false;
		}
		if (failures > 0){
			if (failures > 1)
				viewer.showErrorDialog("Warning: " + failures + " tuples that should be"
						+ " updated have already been deleted by a different user!");
			else
				viewer.showErrorDialog("Warning: One tuple that should be updated has "
						+ "already been deleted by a different user!");
		}
		viewer.removeRelation();
		return (loadRelation(relName, filters));// the new state is set in this method
		
	}
	
	
/*	
Tries to get the relation with name 'loadName' and all 'filters' applied from secondo. If successfull
the filtered relation will be shown in the viewer.

*/
	private boolean loadRelation(String loadName, String[] filters){
		StringBuffer command = new StringBuffer("query " + loadName.trim() + " feed ");
		for (int i = 0; i < filters.length; i++){
			if (!filters[i].trim().equals(""))
			command.append(" filter [ " + filters[i] + " ] ");
		}
		command.append(" addid consume ");
		if (commandExecuter.executeCommand(command.toString(),SecondoInterface.EXEC_COMMAND_SOS_SYNTAX)){
			ListExpr relation = commandExecuter.getResultList();
			if (viewer.showNewRelation(relation)){
				state = LOADED;
				viewer.setSelectionMode(LOADED);
				relName = loadName.trim();
				retrieveIndices();
			}
			else{
				state = INITIAL;
				viewer.showErrorDialog("Loaded Object has to be a relation");
				viewer.setSelectionMode(INITIAL);
				relName = null;
			}
			
		}
		else{
			state = INITIAL;
			viewer.setSelectionMode(INITIAL);
			String errorMessage = commandExecuter.getErrorMessage().toString();
			viewer.showErrorDialog(errorMessage);
			relName = null;
		}
		return (relName != null);
	} 

/*
Sends a 'list objects'-command to SECONDO and scans the result for all indices for the actual relation.
To do this it uses the convention that indices have to begin with the relationname with the first
letter in lowercase, following an underscore and then the name of the attribute over which
the index is built

*/
	
	private void retrieveIndices(){
		commandExecuter.executeCommand("(list objects)", SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX);
		ListExpr inquiry = commandExecuter.getResultList();
		ListExpr objectList = inquiry.second().second();
		objectList.first();
		ListExpr rest = objectList.rest();
		ListExpr nextObject;
		String name;
		String attrName;
		ListExpr type;
		String[] attrNames = viewer.getAttrNames();
		btreeNames = new Vector();
		btreeAttrNames = new Vector();
		rtreeNames = new Vector();
		rtreeAttrNames = new Vector();
		while (! rest.isEmpty()){
			nextObject = rest.first();
			type = nextObject.fourth();
			if (!(type.first().isAtom())){
				if ((type.first().first().isAtom())){
					if (type.first().first().symbolValue().equals("btree")){
						name = nextObject.second().symbolValue();
						if (name.indexOf('_') != -1){
							if(name.substring(0,name.indexOf('_')).equalsIgnoreCase(relName)){
								if(name.substring(1,name.indexOf('_')).equals(relName.substring(1))){
									attrName = name.substring(name.indexOf('_') + 1);
									for (int i = 0; i < attrNames.length; i++){
										if (attrName.trim().equals(attrNames[i].trim())){
											btreeNames.add(name);
											btreeAttrNames.add(attrNames[i].trim());
										}
									}
								}
							}
						}
					}
					else {
						if (type.first().first().symbolValue().equals("rtree")){
							name = nextObject.second().symbolValue();
							if (name.indexOf('_') != -1){
								if(name.substring(0,name.indexOf('_')).equalsIgnoreCase(relName)){
									if(name.substring(1,name.indexOf('_')).equals(relName.substring(1))){
										attrName = name.substring(name.indexOf('_') + 1);
										for (int i = 0; i < attrNames.length; i++){
											if (attrName.trim().equals(attrNames[i].trim())){
												rtreeNames.add(name);
												rtreeAttrNames.add(attrNames[i].trim());
											}
										}
									}
								}
							}
						}
					}
				}
			}
			rest = rest.rest();
		}
	}

}
