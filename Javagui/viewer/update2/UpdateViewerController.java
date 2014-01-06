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
import java.io.File;
import java.io.IOException;
import javax.swing.JCheckBox;
import javax.swing.JOptionPane;
import java.util.ArrayList;
import java.util.List;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import javax.accessibility.AccessibleValue;

import sj.lang.*;

import tools.Reporter;

import viewer.*;
import viewer.relsplit.InvalidRelationException;
import viewer.update2.gui.*;


/*
This class controls the actionflow of update-operations in the 'UpdateViewer2'.

*/
public class UpdateViewerController implements ActionListener, ItemListener
{
	
	private CommandGenerator commandGenerator;
	private CommandExecuter commandExecuter;
	private LoadDialog loadDialog;
	private LoadProfile loadProfile;
	private UpdateViewer2 viewer;
	private int state;	
	private int loadState;	
	private int searchState;
	private boolean caseSensitive;
	
	// Name of relation which contains LoadProfiles
	public final static String LOAD_PROFILE_RELATION = "uv2loadprofiles";
		 		
	/**
	 * Constructor
	 */
	public UpdateViewerController(UpdateViewer2 viewer)
	{
		this.viewer = viewer;
		this.commandGenerator = new CommandGenerator();
		this.commandExecuter = new CommandExecuter();
		this.loadDialog = new LoadDialog(this);
		this.loadProfile = null;
		this.state = States.INITIAL;
		this.loadState = States.INITIAL;
		this.searchState = States.INITIAL;
		this.caseSensitive = false;
	}
	
	/**
	 * Reacts on user actions in UpdateViewer2 and LoadDialog.
	 * Sets the next state.
	 */
	public void actionPerformed(ActionEvent e)
	{
		// UpdateViewer2 window
		if (e.getActionCommand() == "Load")
		{
			this.showLoadDialog();
			return;
		}
		if (e.getActionCommand() == "Clear")
		{
			this.viewer.clear();
			this.setState(States.INITIAL);
			return;
		}
		if (e.getActionCommand() == "Insert")
		{
			this.processCommandInsert();
			this.setState(States.INSERT);
			return;
		}
		if (e.getActionCommand() == "Delete")
		{
			this.setState(States.DELETE);
			return;
		}
		if (e.getActionCommand() == "Update")
		{
			this.setState(States.UPDATE);
			return;
		}
		if (e.getActionCommand() == "Reset")
		{
			this.processCommandReset();
			this.setState(States.LOADED);
			return;
		}
		if (e.getActionCommand() == "Undo")
		{
			this.processCommandUndo();
			return;
		}
		if (e.getActionCommand() == "Commit")
		{
			if (this.processCommandCommit())
			{
				this.setState(States.LOADED);
			}
			return;
		}
		if (e.getActionCommand() == "Format")
		{
			if (this.processCommandFormatMode())
			{
				this.setState(States.FORMAT);
			}
			return;
		}
		if (e.getActionCommand() == "Edit")
		{
			if (this.processCommandEditMode())
			{
				this.setState(States.LOADED);
			}
			return;
		}
		if (e.getActionCommand() == "Close tab")
		{
			if (e.getSource() instanceof AccessibleValue)
			{
				int index = ((AccessibleValue)e.getSource()).getCurrentAccessibleValue().intValue();
				this.viewer.removeRelationPanel(index);
			}
			return;
		}
		
		//
		// LoadDialog		
		//
		if (e.getActionCommand() == "Load selected profile")
		{
			if (this.processCommandLoad())
			{
				state = States.LOADED;
				this.viewer.setSelectionMode(States.LOADED);
				this.loadDialog.setVisible(false);
			}
			return;
		}
			
		if (e.getActionCommand() == "Create new profile")
		{
			String profileName = this.showInputProfileNameDialog();
			if ((profileName != null) && (profileName.length()>0))
			{
				String relName = this.showChooseRelationDialog();
				if ((relName != null) && (relName.length()>0))
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
		
	
		/*
		 * Search		
		 */
		if (e.getActionCommand() == "Search")
		{
			this.processCommandSearch();
			return;
		}
		if (e.getActionCommand() == "Clear search")
		{
			this.processCommandClearSearch();
			return;
		}
		if (e.getActionCommand() == "Next")
		{
			this.processCommandNext();
			return;
			
		}
		if (e.getActionCommand() == "Previous")
		{
			this.processCommandPrevious();
			return;
		}
		if (e.getActionCommand() == "NextFast")
		{
			this.viewer.getCurrentRelationPanel().showHit(this.viewer.getCurrentRelationPanel().getHitCount()-1);
			return;
		}
		if (e.getActionCommand() == "PreviousFast")
		{
			this.viewer.getCurrentRelationPanel().showHit(0);			
			return;
		}
		if (e.getActionCommand() == "Replace")
		{
			this.processCommandReplace();
			return;
		}
		if (e.getActionCommand() == "Replace all")
		{
			this.processCommandReplaceAll();
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
	 * Executes delete commands generated for all relations in one transaction. 
	 * If one of the commands was not succesfully executed the transaction
	 * is aborted.
	 */
	private boolean executeDelete()
	{
		String errorMessage;
		
		if(!this.commandExecuter.beginTransaction())
		{
			errorMessage = this.commandExecuter.getErrorMessage().toString();
			this.showErrorDialog("UpdateViewerController.executeDelete: Error on begin transaction: " 
								 + errorMessage);
			return false;
		}		
		
		// treat all relations with deletions
		for (RelationPanel rp : this.viewer.getRelationPanels())
        { 
			List<String> deleteTupleIds = rp.getDeleteTuples();
			
			if (!deleteTupleIds.isEmpty())
			{
				List<String> commands = this.commandGenerator.generateDelete(rp.getName(),
																			 rp.getRelation().getAttributeNames(),
																			 deleteTupleIds);
				
				//for (String com : commands){Reporter.debug("UpdateViewerController.executeDelete: " + com);}
				
				int failures = 0;
				ListExpr result;
				
				for (String command : commands)
				{
					if(! this.commandExecuter.executeCommand(command, SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX))
					{
						errorMessage = this.commandExecuter.getErrorMessage().toString();
						this.showErrorDialog("UpdateViewerController.executeDelete: Error on executing command " 
											 + command + ": " + errorMessage);
						if (! this.commandExecuter.abortTransaction())
						{
							errorMessage = this.commandExecuter.getErrorMessage().toString();
							this.showErrorDialog("UpdateViewerController.executeDelete: Error on abort transaction: " 
												 + errorMessage);
						}
						return false;
					}
					result = this.commandExecuter.getResultList();
					if ( result.second().intValue() != 1)
						failures++;
				}
				
				if (failures > 0)
				{
					if (failures > 1)
						this.showErrorDialog("Warning: " + failures + " tuples that should be"
											 + " deleted have already been deleted by a different user!");
					else
						this.showErrorDialog("Warning: One tuple that should be deleted has "
											 + "already been deleted by a different user!");
				}
				
				rp.deleteTuples();
			}
			//else	Reporter.debug("UpdateViewerController.executeDelete: no deletions for relation " + rp.getName());
        }
        
		if(!this.commandExecuter.commitTransaction())
		{
			errorMessage = this.commandExecuter.getErrorMessage().toString();
			this.showErrorDialog("UpdateViewerController.executeDelete: Error on commit transaction: " 
								 + errorMessage);
			return false;
		}
		
        return true;
	}
	
	
	/*
	 * Executes all inserts generated for all relations in one big transaction. 
	 * If one of the inserts cannot be executed successfully the transaction
	 * is aborted.	 
	 */
	private boolean executeInsert()
	{
		String errorMessage;
		List<ListExpr> result = new ArrayList<ListExpr>();
		
		if(!this.commandExecuter.beginTransaction())
		{
			errorMessage = this.commandExecuter.getErrorMessage().toString();
			this.showErrorDialog("UpdateViewerController.executeInsert: Error on begin transaction: " 
								 + errorMessage);
			return false;
		}	
		
		RelationPanel rp = this.viewer.getCurrentRelationPanel();
		rp.takeOverLastEditing(false);
         	
		{
			String[] insertCommands = null;

			try
			{
				insertCommands = this.commandGenerator.generateInsert(rp.getName(),
																	  rp.getRelation().getAttributeNames(),
																	  rp.getRelation().getAttributeTypes(),
																	  rp.getInsertTuples());
			} 
			catch(InvalidFormatException e)
			{
				errorMessage = e.getMessage() + "\n at table position (" + e.getRow() + ", " + e.getColumn() + ")";
				if (!this.commandExecuter.abortTransaction())
				{
					errorMessage += this.commandExecuter.getErrorMessage().toString();
					this.showErrorDialog("Error trying to abort transaction: " + errorMessage);
				}
				this.showErrorDialog(errorMessage);
				rp.goToInsert(e.getRow()-1, e.getColumn()-1);
				return false;
			}
			
			for (int i = 0; i < insertCommands.length; i++)
			{
				Reporter.debug("UpdateViewerController.executeInsert: " +insertCommands[i]);
				if(! this.commandExecuter.executeCommand(insertCommands[i], SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX))
				{
					errorMessage = this.commandExecuter.getErrorMessage().toString();
					this.showErrorDialog("Error trying to insert a tuple: " + errorMessage);
					if (!this.commandExecuter.abortTransaction()){
						errorMessage = this.commandExecuter.getErrorMessage().toString();
						this.showErrorDialog("Error trying to abort transaction: " + errorMessage);
					}
					return false;
				}
				else
				{
					ListExpr le = new ListExpr();
					le.setValueTo(this.commandExecuter.getResultList());
					if (!le.isEmpty() && le.listLength() == 2)
					{
						result.add(le.second().first()); 
					}
				}
			}
		}
		
		if(!this.commandExecuter.commitTransaction())
		{
			errorMessage = this.commandExecuter.getErrorMessage().toString();
			this.showErrorDialog("UpdateViewerController.executeInsert: Error on COMMIT TRANSACTION: " 
								 + errorMessage);
			return false;
		}
		
		for (ListExpr le : result)
		{
			Reporter.debug("UpdateViewerController.executeInsert: " + le.toString());
			rp.insertTuple(le);
		}
		rp.resetInsert();
		
		return true;
	}
	
	/*	
	 Executes all updatecommands generated by the 'CommandGenerator' in one big
	 transaction. If one of the commands was not successfully executed the transaction
	 is aborted.	 
	 */
	private boolean executeUpdate()
	{
		for (RelationPanel rp : this.viewer.getRelationPanels())
        {
            rp.takeOverLastEditing(true);
            
            String errorMessage;
            
            List<String> commands;
            
            Map<Integer, HashMap<String, Change>> changesByTupleId = rp.getUpdateTuples();
            
            if (changesByTupleId.isEmpty())
            {
                Reporter.debug("UpdateViewerController.executeUpdate: no changes in relation " + rp.getName());
            }
			
            try
            {
                commands = this.commandGenerator.generateUpdate(rp.getName(),
                                                                      rp.getRelation().getAttributeNames(),
                                                                      changesByTupleId);
                
                for (String com : commands){Reporter.debug("UpdateViewerController.executeUpdate: " + com);}
            }
            catch(InvalidFormatException e)
            {
                String message = e.getMessage()+"\n at position ("+ e.getRow() + ", " + e.getColumn() + ")";
                this.showErrorDialog(message);
                rp.goToEdit(e.getRow() -1, e.getColumn()-1);
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
            
            for (String command : commands)
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
            
            rp.clearUpdateChanges();
        }
        
        return true;
	}
	
	/**
	 * Reacts on Checkbox selection.
	 */
	public void itemStateChanged(ItemEvent e)
	{
		if (e.getItem() instanceof JCheckBox && ((JCheckBox)e.getItem()).getText().equals("case-sensitive"))
		{
			this.caseSensitive = (e.getStateChange() == ItemEvent.SELECTED);
		}
	}
	
	
	/**
	 * Returns true if relation exists in currently open database.
	 */
	private boolean existsInDb(String pObjectName)
	{
		boolean result = false;
		if (commandExecuter.executeCommand("(list objects)", SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX))
		{
			ListExpr inquiry = commandExecuter.getResultList();
			ListExpr objectList = inquiry.second().second();
			objectList.first();
			ListExpr rest = objectList.rest();
			ListExpr nextObject;
			String name;
			
			while (!rest.isEmpty())
			{
				nextObject = rest.first();
				
				name = nextObject.second().symbolValue();
				if (name.equals(pObjectName))
				{
					result = true;
				}
				
				rest = rest.rest();
			}
		}
		else
		{
			return false;
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
			//Reporter.debug("loadRelation: " + relationLE.toString());

			this.viewer.setRelationPanel(pRelName, relationLE, true);
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
	

	
	public void processCommandClearSearch()
	{
		for (RelationPanel rp : this.viewer.getRelationPanels())
		{
			rp.resetSearch();
		}
	}
	
	
	/**
	 *
	 */
	public boolean processCommandCommit()
	{
		boolean result = false;
		
		switch (state)
		{
			case States.INSERT:
			{
				result = this.executeInsert();
				break;
			}
			case States.DELETE:
			{
				result = this.executeDelete();
				break;
			}
			case States.UPDATE:
			{
				result = this.executeUpdate();
				break;
			}
			default: break;
		}
        if (result)
        {
            this.processCommandSearch();
        }
		return result;
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
	 * Show loaded relations in Edit View.
	 * TODO: Set cursor position at relation and tuple according to 
	 * cursor position in formatted document.
	 */
	public boolean processCommandEditMode()
	{
		this.viewer.showRelations(States.LOADED);
		return true;
	}
	
	/**
	 * Shows the loaded relation as formatted document.
	 */
	private boolean processCommandFormatMode()
	{
		// load format file
		//this.generateFormattedPages();
				
		File dir = new File ("../../Desktop/Modulhandbuch/Modulehtml/") ;
		File[] files = dir.listFiles();

		List<String> filenames = new ArrayList<String>();
		for (int i=0; i<files.length; i++)
		{
			File file = files[i];
			if (file.isFile() && !file.isHidden())
			{
				filenames.add(file.getPath());
			}
		}
		
		try
		{
			this.viewer.loadFormattedDocument(filenames, "text/html");
			this.viewer.showRelations(States.FORMAT);
		}
		catch (IOException e)
		{
			this.showErrorDialog(e.getMessage());
			return false;
		}
		return true;
	}
	
	private boolean generateFormattedPages()
	{
		// load format script file
		return false;
	}
	
	
	/**
	 * Shows insert table with one empty tuple if INSERT was pressed for the first time,
	 * else adds an empty tuple to the insert table.
	 */
	private boolean processCommandInsert()
	{
		if (viewer.getCurrentRelationPanel().getState() != States.LOADED_READ_ONLY)
		{
			try
			{
				
				if (this.state != States.INSERT)
				{ 
					viewer.getCurrentRelationPanel().showInsertTable();
				}
				else
				{
					// User wants to insert one more tuple
					viewer.getCurrentRelationPanel().takeOverLastEditing(false);
					viewer.getCurrentRelationPanel().addInsertTuple();
				}
			}
			catch(InvalidRelationException e)
			{
				this.showErrorDialog(e.getMessage());
				return false;
			}
		}
		return true;
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
		
		this.viewer.showRelations(States.LOADED);
		this.viewer.setSelectionMode(States.LOADED);
		this.state = States.LOADED;
		return true;		
	}
	
	
	private boolean processCommandNext()
	{
		int hitIndex = this.viewer.getCurrentRelationPanel().getCurrentHitIndex()+1;
		if (hitIndex < this.viewer.getCurrentRelationPanel().getHitCount())
		{
			this.viewer.getCurrentRelationPanel().showHit(hitIndex);							
		}			
		else
		{
			int rpIndex = this.viewer.getRelationPanels().indexOf(this.viewer.getCurrentRelationPanel())+1;
			boolean found = false;
			while (rpIndex < this.viewer.getRelationPanels().size() && !found)
			{
				if (this.viewer.getRelationPanel(rpIndex).hasSearchHits())
				{
					found = true;
					this.viewer.showRelationPanel(rpIndex);
					this.viewer.getCurrentRelationPanel().showHit(0);			
				}
				rpIndex++;
			}
		}
		return true;
	}
	
	
	private boolean processCommandPrevious()
	{
		int hitIndex = this.viewer.getCurrentRelationPanel().getCurrentHitIndex()-1;
		if (hitIndex >= 0)
		{
			this.viewer.getCurrentRelationPanel().showHit(hitIndex);							
		}
		else
		{
			int rpIndex = this.viewer.getRelationPanels().indexOf(this.viewer.getCurrentRelationPanel())-1;
			boolean found = false;
			while (rpIndex >= 0 && !found)
			{
				if (this.viewer.getRelationPanel(rpIndex).hasSearchHits())
				{
					found = true;
					this.viewer.showRelationPanel(rpIndex);
					this.viewer.getCurrentRelationPanel().showHit(this.viewer.getCurrentRelationPanel().getHitCount()-1);			
				}
				rpIndex--;
			}
		}
		return true;
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
	
	
	private boolean processCommandReplace()
	{
		RelationPanel rp = this.viewer.getCurrentRelationPanel();
		String replacement = rp.getReplacement();
		
		if (replacement != null)
		{
			SearchHit hit = rp.getHit(rp.getCurrentHitIndex());
			rp.replace(hit, replacement);
			this.processCommandNext();
		}
		
		return true;
	}
	
	private boolean processCommandReplaceAll()
	{
		String replacement = this.viewer.getCurrentRelationPanel().getReplacement();
		
		if (replacement != null)
		{
			for (RelationPanel rp : this.viewer.getRelationPanels())
			{
				if (rp.hasSearchHits())
				{
					for (int i = 0; i< rp.getHitCount(); i++)
					{
						rp.replace(rp.getHit(i), replacement);
					}
				}
			}			
		}
		return true;
	}
	
	/**
	 * Resets all uncommitted changes according to current state. 
	 */
	private boolean processCommandReset()
	{
		switch(this.state)
		{
			case States.INSERT:
                for (RelationPanel rp : this.viewer.getRelationPanels())
                {
                    rp.resetInsert();
                }
				break;
			case States.UPDATE:
                for (RelationPanel rp : this.viewer.getRelationPanels())
                {
                    rp.resetUpdateChanges();
                }
				break;
			case States.DELETE:
                for (RelationPanel rp : this.viewer.getRelationPanels())
                {
                    rp.resetDeleteSelections();
                }
				break;
			default:
				break;
		}
		
		return true;
	}
	
	
	/**
	 * Retrieves hits for keyword in search field and displays result in UpdateViewer2.
	 */
	private void processCommandSearch()
	{
		// read keyword from searchfield
		String key = this.viewer.getCurrentRelationPanel().getSearchKey();

		if (key != null && (key.length() > 0))
		{
			if (key.length() < 3)
			{
				this.showErrorDialog("Key length must be 3 at least");
			}
			else
			{
				int first = -1;
				for (RelationPanel rp : this.viewer.getRelationPanels())
				{								
					List<SearchHit> hitlist = rp.retrieveSearchHits(key, this.caseSensitive);
					rp.setSearchHits(hitlist);
					rp.showSearchResult(key);
					if (rp.hasSearchHits())
					{
						first = this.viewer.getRelationPanels().indexOf(rp);
					}
				}
				// go to first RelationPanel with a hit (if there is any)
				if (first >= 0)
				{
					this.viewer.showRelationPanel(first);
				}
			}
		}
	}
	
	/**
     * Undoes last uncommitted change (cell-wise), deletion or insertion.
     */
	public void processCommandUndo()
	{
		if(this.state == States.INSERT)
		{
			this.viewer.getCurrentRelationPanel().removeLastInsertTuple();		
		}
		if(this.state == States.UPDATE)
		{
			this.viewer.getCurrentRelationPanel().undoLastUpdateChange();		
		}
		if(this.state == States.DELETE)
		{
			this.viewer.getCurrentRelationPanel().undoLastDeleteSelection();
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
	
		
	/**
	 * Sets state attribute of this controller and sets selection mode of UpdateViewer2.
	 */
	public void setState(int pState)
	{
		this.state = pState;
		this.viewer.setSelectionMode(pState);
	}
	
	/**
	 * Shows input dialog with list of relations in open database.
	 */
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
			this.setState(States.INITIAL);
			this.showErrorDialog(commandExecuter.getErrorMessage().toString());			
		}
		else
		{
			this.loadDialog.showProfiles();		
			this.loadDialog.setVisible(true);
		}
	}
	
}
