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
---
*/

package viewer.update2;
 
import java.awt.event.*;
import javax.swing.JOptionPane;
import java.util.ArrayList;
import java.util.List;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import sj.lang.*;

import tools.Reporter;

import viewer.*;
import viewer.update2.gui.*;
import viewer.update.InvalidFormatException;


/*
This class controls the actionflow of update-operations in the 'UpdateViewer2'.

*/
public class UpdateViewerController implements ActionListener, ComponentListener{
	
	private CommandGenerator commandGenerator;
	private CommandExecuter commandExecuter;
	private LoadDialog loadDialog;
	private LoadProfile loadProfile;
	private int state;	
	private UpdateViewer2 viewer;
	
	
	// The controller is always in one certain state 
	public final static int INITIAL= 0;
	public final static int LOADED = 1;
	public final static int INSERT = 2;
	public final static int DELETE = 3;
	public final static int UPDATE = 4;
	
	// Name of relation which contains LoadProfiles
	public final static String LOAD_PROFILE_RELATION = "uv2loadprofiles";
		 		
	//initializes the controller
	public UpdateViewerController(UpdateViewer2 viewer){
		this.viewer = viewer;
		this.commandGenerator = new CommandGenerator();
		this.commandExecuter = new CommandExecuter();
		this.loadDialog = new LoadDialog(this);
		this.loadProfile = null;
		this.state = INITIAL;
		
	}
	
/*
If any of the possible actions of the viewer was chosen, this method is called and decides 
according to the current state, which action shall be executed and what shall be the next
state.

*/
	public void actionPerformed(ActionEvent e)
	{
		// UpdateViewer2-Window
		if (e.getActionCommand() == "Load")
		{
			this.showLoadDialog();
			return;
		}
		if (e.getActionCommand() == "Clear")
		{
			this.viewer.clear();
			if (state == INSERT)
			{
				//viewer.removeInsertRelation();
			}
			
			state = INITIAL;
			viewer.setSelectionMode(INITIAL);
			return;
		}
		if (e.getActionCommand() == "Insert")
		{
			if (state == INSERT)
			{ // User wants to insert one more tuple
				viewer.getCurrentRelationPanel().takeOverLastEditing(false);
				viewer.getCurrentRelationPanel().addInsertTuple();
				return;
			}
			state = INSERT;
			viewer.setSelectionMode(INSERT);
			viewer.getCurrentRelationPanel().showInsertRelation();
			return;
		}
		if (e.getActionCommand() == "Delete")
		{
			state = DELETE;
			viewer.setSelectionMode(DELETE);
			return;
		}
		if (e.getActionCommand() == "Update")
		{
			state = UPDATE;
			viewer.setSelectionMode(UPDATE);
			return;
		}
		if (e.getActionCommand() == "Reset")
		{
			if(state == INSERT)
			{
				if (! viewer.getCurrentRelationPanel().removeLastInsertTuple())
				{
					state = LOADED;
					viewer.getCurrentRelationPanel().showOriginalRelation();
					viewer.setSelectionMode(LOADED);
				}
			}
			if(state == UPDATE)
			{
				viewer.getCurrentRelationPanel().resetUpdates();
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
			if(state == DELETE)
			{
				if (! viewer.getCurrentRelationPanel().resetDeleteSelections())
				{
					state = LOADED;
					viewer.getCurrentRelationPanel().showOriginalRelation();
					viewer.setSelectionMode(LOADED);
				}
			}
			
			return;
		}
		if (e.getActionCommand() == "Commit")
		{
			boolean result = false;
			if(state == INSERT)
			{
				// TODO
			}
			if(state == DELETE)
			{
				result = this.executeDelete(this.viewer.getCurrentRelationPanel().getName());
			}
			if(state == UPDATE)
			{
				result = this.executeUpdate();
			}
			if ( result )
			{
				state = LOADED;
				this.viewer.setSelectionMode(LOADED);
			}
			return;
		}
		
		if (e.getActionCommand() == "Undo")
		{
			if(state == INSERT)
			{
				// TODO
			}
			if(state == UPDATE)
			{
				/* undo functionality */
				/*	if (! viewer.resetLastUpdate()){
				 state = LOADED;
				 viewer.showOriginalRelation();
				 viewer.setSelectionMode(LOADED);
				 }
				 */
				
			}
			if(state == DELETE)
			{
				// TODO
			}
		}	
		
		//
		// LoadDialog		
		//
		if (e.getActionCommand() == "Load selected profile")
		{
			if (this.processCommandLoad())
			{
				this.loadDialog.setVisible(false);
			}
			return;
		}
			
		if (e.getActionCommand() == "Create new profile")
		{
			String profileName = this.showInputProfileNameDialog();
			if (profileName != null && !profileName.isEmpty())
			{
				String relName = this.showChooseRelationDialog();
				if (relName != null && !relName.isEmpty())
				{
					// create, insert and show new load profile
					if (this.processCommandAddRelationProfile(profileName, relName))
					{
						this.loadProfiles();
						this.loadDialog.showProfiles();
					}				}
			}	
			return;
		}
			
		if (e.getActionCommand() == "Delete profile")
		{
			if(this.processCommandDeleteLoadProfile(this.loadDialog.getCurrentLoadProfileName()))
			{
				this.loadProfiles();
				this.loadDialog.showProfiles();
			}
			return;
		}
			
		if (e.getActionCommand() == "Cancel")
		{
			this.loadDialog.setVisible(false);
			return;
		}
			
		if (e.getActionCommand() == "Add relation")
		{
			String relName = this.showChooseRelationDialog();
			if (relName != null && relName.length()!=0)
			{
				if (this.processCommandAddRelationProfile(this.loadDialog.getCurrentLoadProfileName(), relName))
				{
					this.loadProfiles();
					this.loadDialog.showProfiles();
				}
			}
			return;
		}
			
		if (e.getActionCommand() == "Save relation")
		{
			// TODO
			return;
		}
			
		if (e.getActionCommand() == "Remove relation")
		{
			String profileName = this.loadDialog.getCurrentLoadProfileName();
			String relName = loadDialog.getCurrentRelationProfileName();
			
			if (this.processCommandRemoveRelationProfile(this.loadDialog.getCurrentLoadProfileName(), relName))
			{
				this.loadProfiles();
				this.loadDialog.showProfiles();				
			}
			return;
		}
		
		// This point should never be reached
		this.showErrorDialog("Command not known");
	}

	
	/**
	 * Creates an empty relation, that can be queried.
	 * Returns true on success.
	 */
	private boolean createRelationLoadprofiles()
	{
		StringBuffer sb = new StringBuffer("let ");
		sb.append(LOAD_PROFILE_RELATION);
		sb.append(" = [const rel (tuple (");
		sb.append(" [ProfileName: string, RelName: string, FilterExpr: text, ProjectExpr: text, SortExpr: text]");
		sb.append(" )) value ()]");
		return this.commandExecuter.executeCommand(sb.toString(), SecondoInterface.EXEC_COMMAND_SOS_SYNTAX);		
	}
	

	/*	
	 Executes all deletecommands generated by the 'CommandGenerator' in one big
	 transaction. If one of the commands was not succesfully executed the transaction
	 is aborted.
	 
	 */
	private boolean executeDelete(String pRelName)
	{
		// TODO
		/*
		String errorMessage;
		String[] deleteCommands = commandGenerator.generateDelete(pRelName,
																  btreeNames, btreeAttrNames,
																  rtreeNames, rtreeAttrNames);;
		if(! beginTransaction()){
			errorMessage = this.commandExecuter.getErrorMessage().toString();
			this.showErrorDialog(errorMessage);
			return false;
		}
		int failures = 0;
		ListExpr result;
		for (int i = 0; i < deleteCommands.length; i++){
			if(! this.commandExecuter.executeCommand(deleteCommands[i], SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX)){
				errorMessage = this.commandExecuter.getErrorMessage().toString();
				this.showErrorDialog("Error trying to delete a tuple: " + errorMessage);
				if (! abortTransaction()){
					errorMessage = this.commandExecuter.getErrorMessage().toString();
					this.showErrorDialog("Error trying to abort transaction: " + errorMessage);
				}
				return false;
			}
			result = this.commandExecuter.getResultList();
			if ( result.second().intValue() != 1)
				failures++;
			
		}
		if(! commitTransaction()){
			errorMessage = this.commandExecuter.getErrorMessage().toString();
			this.showErrorDialog(errorMessage);
			return false;
		}
		if (failures > 0){
			if (failures > 1){
				this.showErrorDialog("Warning: " + failures + " tuples have already been deleted by a different user!");
			}
			else{
				this.showErrorDialog("Warning: one tuple has already been deleted by a different user!");
			}
		}
		this.viewer.clear();
		return (this.loadRelation(pRelName)); // the new state is set in this method
		*/
		return false;
	}
	
	
	/*
	 Executes all insertcommands generated by the 'CommandGenerator' in one big
	 transaction. If one of the commands was not succesfully executed the transaction
	 is aborted.
	 
	 */
	private boolean executeInsert(String pRelName)
	{
		// TODO
		/*
		this.viewer.getCurrentRelationPanel().takeOverLastEditing(false);
		String errorMessage;
		String[] insertCommands=null;
		try{
    		insertCommands = this.commandGenerator.generateInsert(pRelName,
																  btreeNames, 
																  btreeAttrNames,
																  rtreeNames, 
																  rtreeAttrNames);
		} catch(InvalidFormatException e){
			String message = e.getMessage()+"\n at position ("+e.row+", "+e.column+")";
			this.showErrorDialog(message);
			this.insertGoTo(e.row-1,e.column-1);
			return false;
		}
		if(! beginTransaction()){
			errorMessage = this.commandExecuter.getErrorMessage().toString();
			this.showErrorDialog(errorMessage);
			return false;
		}
		for (int i = 0; i < insertCommands.length; i++){
			if(! this.commandExecuter.executeCommand(insertCommands[i], SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX)){
				errorMessage = this.commandExecuter.getErrorMessage().toString();
				this.showErrorDialog("Error trying to insert a tuple: " + errorMessage);
				if (! abortTransaction()){
					errorMessage = this.commandExecuter.getErrorMessage().toString();
					this.showErrorDialog("Error trying to abort transaction: " + errorMessage);
				}
				return false;
			}
		}
		if(! commitTransaction()){
			errorMessage = this.commandExecuter.getErrorMessage().toString();
			this.showErrorDialog(errorMessage);
			return false;
		}		
		this.viewer.removeInsertRelation();
		this.viewer.clear();
		return (loadRelation(pRelName)); // the new state is set in this method
		*/
		return false;
	}
	
	/*	
	 Executes all updatecommands generated by the 'CommandGenerator' in one big
	 transaction. If one of the commands was not succesfully executed the transaction
	 is aborted.	 
	 */
	private boolean executeUpdate()
	{		
		RelationPanel rp = this.viewer.getCurrentRelationPanel();
				
		rp.takeOverLastEditing(true); 		
		
		String errorMessage;
		
		List<String> updateCommands;
		
		Map<Integer, HashMap<String, Change>> changedTuples = rp.getChangedTuples();
		
		if (changedTuples.isEmpty())
		{
			Reporter.debug("UpdateViewerController.executeUpdate: no changes");
			return false;
		}
			
		try
		{
			updateCommands = this.commandGenerator.generateUpdate(rp.getName(), 
																  rp.getRelation().getAttributeNames(), 
																  changedTuples);
			
			for (String com : updateCommands){Reporter.debug("updatecommand: " + com);}
		} 
		catch(InvalidFormatException e)
		{
			String message = e.getMessage()+"\n at position ("+e.row+", "+e.column+")";
			this.showErrorDialog(message);
			this.viewer.getCurrentRelationPanel().relGoTo(e.row-1,e.column-1);
			return false;
		}
		
		
		if(! this.commandExecuter.beginTransaction())
		{
			errorMessage = this.commandExecuter.getErrorMessage().toString();
			this.showErrorDialog(errorMessage);
			return false;
		}
		
		int failures = 0;
		ListExpr result;
		
		for (String command : updateCommands)
		{
			if(! this.commandExecuter.executeCommand(command, SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX))
			{
				errorMessage = this.commandExecuter.getErrorMessage().toString();
				this.showErrorDialog("Error trying to update a tuple: " + errorMessage);
				if (! this.commandExecuter.abortTransaction())
				{
					errorMessage = this.commandExecuter.getErrorMessage().toString();
					this.showErrorDialog("Error trying to abort transaction: " + errorMessage);
				}
				return false;
			}
			result = this.commandExecuter.getResultList();
			if ( result.second().intValue() != 1)
				failures++;
		}
		
		if(!this.commandExecuter.commitTransaction())
		{
			errorMessage = this.commandExecuter.getErrorMessage().toString();
			this.showErrorDialog(errorMessage);
			return false;
		}
		
		if (failures > 0)
		{
			if (failures > 1)
				this.showErrorDialog("Warning: " + failures + " tuples that should be"
									   + " updated have already been deleted by a different user!");
			else
				this.showErrorDialog("Warning: One tuple that should be updated has "
									   + "already been deleted by a different user!");
		}
		
		//return (loadRelation(pRelName));// the new state is set in this method
		 
		return true;
	}
	
	
	/**
	 * Returns true if relation exists in currently open database.
	 */
	private boolean existsInDb(String pObjectName)
	{
		boolean result = false;
		commandExecuter.executeCommand("(list objects)", SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX);
		ListExpr inquiry = commandExecuter.getResultList();
		ListExpr objectList = inquiry.second().second();
		objectList.first();
		ListExpr rest = objectList.rest();
		ListExpr nextObject;
		String name;

		while (! rest.isEmpty())
		{
			nextObject = rest.first();

			name = nextObject.second().symbolValue();
			if (name.equals(pObjectName))
			{
				result = true;
			}
					
			rest = rest.rest();
		}
		return result;
	}
	
	/*	
	 * Tries to get the LoadProfiles from a relation with name LOAD_PROFILE_RELATION. 
	 * @return true if valid non-empty ListExpression of that relation could be retrieved and evaluated,
	 * else false.
	 */
	private boolean loadProfiles()
	{
		boolean result = false;
		
		ListExpr profilesLE = new ListExpr();
		
		if (!this.existsInDb(LOAD_PROFILE_RELATION))
		{
			this.createRelationLoadprofiles();
		}
		
		try
		{
			if (this.commandExecuter.executeCommand("query uv2loadprofiles", SecondoInterface.EXEC_COMMAND_SOS_SYNTAX)
				&& this.commandExecuter.getErrorCode().value==ServerErrorCodes.NOT_ERROR_CODE)
			{
				profilesLE = this.commandExecuter.getResultList();
				result = this.loadDialog.createLoadProfilesFromLE(profilesLE);
			}
			else if (this.commandExecuter.getErrorCode().value==ServerErrorCodes.NOT_DB_NAME_CODE)
			{

			}
		}
		catch (Exception e)
		{
			this.showErrorDialog(e.getMessage());
		}
		
		return result;
	} 
	
	
	/*	
	 * Tries to get the relation with specified name and all restrictions applied from secondo. 
	 * If successful a Panel to display the relation will be prepared in the viewer.
	 */
	private boolean loadRelation(String pRelName)
	{
		boolean loaded = false;

		StringBuffer command = new StringBuffer("query " + pRelName + " feed ");
		
		if (this.loadProfile != null)
		{
			List<String> filters = this.loadProfile.getFilterExpressions(pRelName);
			List<String> projects = this.loadProfile.getProjectExpressions(pRelName);
			List<String> sorts = this.loadProfile.getSortExpressions(pRelName);
			for (String filter : filters)
			{
				if (!filter.equals(""))
				{
					command.append(" filter [ " + filter + " ] ");
				}
			}
			for (String project : projects)
			{
				if (!project.equals(""))
				{
					command.append(" project [ " + project + " ] ");
				}
			}
			for (String sort : sorts)
			{
				if (!sort.equals(""))
				{
					command.append(" sortBy [ " + sort + " ] ");
				}
			}
		}
		
		command.append(" addid consume ");
		
		Reporter.debug("loadRelation: " + command.toString());
		
		if (commandExecuter.executeCommand(command.toString(),SecondoInterface.EXEC_COMMAND_SOS_SYNTAX))
		{
			ListExpr relationLE = commandExecuter.getResultList();
			Reporter.debug("loadRelation: " + relationLE.toString());

			this.viewer.setRelationPanel(pRelName, relationLE);
			//retrieveIndices(pRelName);
			loaded = true;
		}
		else
		{
			String errorMessage = commandExecuter.getErrorMessage().toString();
			this.showErrorDialog(errorMessage);
		}
		return loaded;
	} 
	
	/**
	 * Inserts new relation profile tuple in relation uv2loadprofiles. 
	 */
	private boolean processCommandAddRelationProfile(String pLoadProfileName, String pRelName)
	{
		StringBuffer sb = new StringBuffer();
		sb.append("query uv2loadprofiles inserttuple[\"");
		sb.append(pLoadProfileName);
		sb.append("\", \"");
		sb.append(pRelName);
		sb.append("\", \'\', \'\', \'\'] consume");
		
		if (commandExecuter.executeCommand(sb.toString(), SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX))
		{
			Reporter.debug("processCommandAddRelationProfile: Executer returned TRUE");
			return true;
		}
		else
		{
			Reporter.debug("processCommandAddRelationProfile: Executer returned FALSE");
			return false;
		}		
	}
	
	/**
	 * Loads all relations of the currently selected LoadProfile from database
	 * and shows them in the viewer.
	 */
	private boolean processCommandLoad()
	{	
		this.viewer.clear();
		this.loadProfile = this.loadDialog.getCurrentLoadProfile();
		String errorMessage = "";
		
		if (loadProfile == null)
		{
			errorMessage = "Please select or create a Load Profile.";
			this.showErrorDialog(errorMessage);
			return false;
		}
				
		for(String relname : this.loadProfile.getRelationNames())
		{
			if (!this.loadRelation(relname))
			{
				errorMessage += "Error while loading relation " + relname + ". ";
			}
		}
				
		if (errorMessage != null && errorMessage.length()!=0)
		{
			this.showErrorDialog(errorMessage);
		}
		
		this.viewer.showRelations();
		this.viewer.setSelectionMode(LOADED);
		this.state = LOADED;
		return true;		
	}
	
	/**
	 * Deletes all entries for specified load profile from relation uv2loadprofiles.
	 */
	private boolean processCommandDeleteLoadProfile(String pLoadProfileName)
	{
		StringBuffer sb = new StringBuffer();
		sb.append("query uv2loadprofiles feed filter[.ProfileName = \"");
		sb.append(pLoadProfileName);
		sb.append("\"] consume feed uv2loadprofiles deletesearch consume");
		
		if (commandExecuter.executeCommand(sb.toString(), SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX))
		{
			return true;
		}
		else
		{
			Reporter.debug(sb.toString());
			Reporter.debug(commandExecuter.getErrorMessage().toString());
			return false;
		}
	}
	
	/**
	 * Deletes the entry for the specified relation profile from relation uv2loadprofiles.
	 */
	private boolean processCommandRemoveRelationProfile(String pLoadProfileName, String pRelName)
	{
		StringBuffer sb = new StringBuffer();
		sb.append("query uv2loadprofiles feed filter[.ProfileName = \"");
		sb.append(pLoadProfileName);
		sb.append("\"] filter[.RelName = \"");
		sb.append(pRelName);
		sb.append("\"] consume feed uv2loadprofiles deletesearch consume");
		
		if (commandExecuter.executeCommand(sb.toString(), SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX))
		{
			return true;
		}
		else
		{
			Reporter.debug(sb.toString());
			Reporter.debug(commandExecuter.getErrorMessage().toString());
			return false;
		}		
	}
	

	
	/**
	 * Returns names of all relations in currently open database.
	 */
	private List<String> retrieveRelationNames()
	{
		List<String> result = new ArrayList<String>();
		commandExecuter.executeCommand("(list objects)", SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX);
		ListExpr inquiry = commandExecuter.getResultList();
		ListExpr objectList = inquiry.second().second();
		objectList.first();
		ListExpr rest = objectList.rest();
		ListExpr nextObject;
		ListExpr type;
		String name;
		
		while (!rest.isEmpty())
		{
			nextObject = rest.first();
			type = nextObject.fourth();
			if (!(type.first().isAtom())){
				if ((type.first().first().isAtom())){
					String objectType = type.first().first().symbolValue();
					if (objectType.equals("rel") || objectType.equals("trel") || objectType.equals("mrel")){
						name = nextObject.second().symbolValue();
						result.add(name);
					}
				}
			}
			rest = rest.rest();
		}
		
		return result;
	}
	
	
	private String showChooseRelationDialog()
	{
		String result = "";
		
		List<String> names = this.retrieveRelationNames();
		
		if (names.isEmpty())
		{
			JOptionPane.showConfirmDialog(null, "Database contains no relations.", "", JOptionPane.OK_OPTION);
		}
		else
		{
			Object selection = JOptionPane.showInputDialog(null,
														   "Choose a relation", "Add relation",
														   JOptionPane.INFORMATION_MESSAGE, null,
														   names.toArray(), names.get(0));
			if (selection!= null)
			{
				result = (String)selection;
			}
		}
		return result;
	}
	
	/**
	 * Shows an input box for specifying the load profile name.
	 */
	private String showInputProfileNameDialog()
	{
		return JOptionPane.showInputDialog("Name the load profile " );
	}
	

	/*
	 * Shows messagebox with the errorText.	 
	 */
	public void showErrorDialog(String errorText)
	{
		Reporter.showError(errorText);
		this.viewer.repaint();
		this.viewer.validate();
	}
	
	
	
	/*
	 * Shows the load dialog.	 
	 */
	public void showLoadDialog() 
	{
		boolean loaded = this.loadProfiles();
		
		if(!loaded)
		{
			this.state = INITIAL;
			this.viewer.setSelectionMode(INITIAL);
			this.showErrorDialog(commandExecuter.getErrorMessage().toString());			
		}
		else
		{
			this.loadDialog.showProfiles();		
			this.loadDialog.setVisible(true);
		}
	}
	
	
	
	/*************************************************
	 * Methods of Interface ComponentListener
	 *************************************************/
    
	/**
	 * Invoked when the components size changes.
	*/
	public void componentResized(ComponentEvent e){
		// TODO
	}	
	
    /**
	 * Invoked when the components position changes.
	 */
	public void componentMoved(ComponentEvent e){
		// TODO
	}
	
	/**
	 * Invoked when the component has been made visible.
	 */
	public void componentShown(ComponentEvent e)
	{
		if (e.getSource() instanceof UpdateViewer2)
		{
			this.viewer.setSelectionMode(INITIAL);

		}
	}	
	
	/**
	 * Invoked when the component has been made invisible. 
	 */
	public void componentHidden(ComponentEvent e){
		// TODO
	}
	
	
	
}
