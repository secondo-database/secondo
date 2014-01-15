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
import javax.swing.JComponent;
import javax.swing.JOptionPane;
import java.util.ArrayList;
import java.util.List;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import javax.accessibility.AccessibleValue;

import sj.lang.*;

import tools.Reporter;

import viewer.UpdateViewer2;
import viewer.update2.gui.*;
import viewer.update2.format.*;


/*
This class controls the actionflow of update-operations in the 'UpdateViewer2'.

*/
public class UpdateViewerController implements ActionListener, ItemListener
{
	
	private CommandGenerator commandGenerator;
	private CommandExecuter commandExecuter;
	private LoadDialog loadDialog;
	private UpdateViewer2 viewer;
	private int state;	
	private int loadState;	
	private boolean searchActive;
	private boolean caseSensitive;
	
	// Name of relation which contain LoadProfiles
	public final static String RELNAME_LOAD_PROFILES_HEAD = "uv2loadprofiles";
	public final static String RELNAME_LOAD_PROFILES_POS = "uv2loadprofilespos";
	
	// button commands in UpdateViewer2 window
	public final static String CMD_LOAD = "Load";
	public final static String CMD_CLEAR = "Clear";
	public final static String CMD_INSERT = "Insert";
	public final static String CMD_UPDATE = "Update";
	public final static String CMD_DELETE = "Delete";
	public final static String CMD_UNDO = "Undo";
	public final static String CMD_RESET = "Reset";
	public final static String CMD_COMMIT = "Commit";
	public final static String CMD_FORMAT = "Format";
	public final static String CMD_EDIT = "Edit";
	public final static String CMD_CLOSE_TAB = "Close tab";
	// button commands in LoadDialog
	public final static String CMD_LOAD_PROFILE = "Load selected profile";
	public final static String CMD_CREATE_PROFILE = "Create new profile";
	public final static String CMD_CREATE_PROFILEPOS = "Add relation";
	public final static String CMD_EDIT_PROFILE = "Edit load profile";
	public final static String CMD_EDIT_PROFILEPOS = "Edit relation restrictions";
	public final static String CMD_REMOVE_PROFILE = "Remove profile";
	public final static String CMD_REMOVE_PROFILEPOS = "Remove relation";
	// button commands in TupleEditor
	public final static String CMD_CANCEL = "Cancel";
	public final static String CMD_SAVE_PROFILE = "Save load profile";
	public final static String CMD_SAVE_PROFILEPOS = "Save relation restrictions";
	// button commands for search/replace
	public final static String CMD_SEARCH = "Search";
	public final static String CMD_CLEAR_SEARCH = "Clear search";
	public final static String CMD_PREVIOUS = "Previous";
	public final static String CMD_NEXT = "Next";
	public final static String CMD_LAST = "Last";
	public final static String CMD_FIRST = "First";
	public final static String CMD_REPLACE = "Replace";
	public final static String CMD_REPLACE_ALL = "Replace all";
	
	/**
	 * Constructor
	 */
	public UpdateViewerController(UpdateViewer2 viewer)
	{
		this.viewer = viewer;
		this.commandGenerator = new CommandGenerator();
		this.commandExecuter = new CommandExecuter();
		this.loadDialog = new LoadDialog(this);
		this.state = States.INITIAL;
		this.loadState = States.INITIAL;
		this.searchActive = false;
		this.caseSensitive = false;
	}
	
	/**
	 * Reacts on user actions in UpdateViewer2 and LoadDialog.
	 * Sets the next state.
	 */
	public void actionPerformed(ActionEvent e)
	{
		// UpdateViewer2 window
		if (e.getActionCommand() == CMD_LOAD)
		{
			this.processCommandLoad();
			return;
		}
		if (e.getActionCommand() == CMD_CLEAR)
		{
			this.viewer.clear();
			this.setState(States.INITIAL);
			return;
		}
		if (e.getActionCommand() == CMD_INSERT)
		{
			this.processCommandInsert();
			this.setState(States.INSERT);
			return;
		}
		if (e.getActionCommand() == CMD_DELETE)
		{
			this.setState(States.DELETE);
			return;
		}
		if (e.getActionCommand() == CMD_UPDATE)
		{
			this.setState(States.UPDATE);
			return;
		}
		if (e.getActionCommand() == CMD_RESET)
		{
			this.processCommandReset();
			this.setState(States.LOADED);
			return;
		}
		if (e.getActionCommand() == CMD_UNDO)
		{
			this.processCommandUndo();
			return;
		}
		if (e.getActionCommand() == CMD_COMMIT)
		{
			if (this.processCommandCommit())
			{
				this.setState(States.LOADED);
			}
			return;
		}
		if (e.getActionCommand() == CMD_FORMAT)
		{
			if (this.processCommandFormatMode())
			{
				this.setState(States.FORMAT);
			}
			return;
		}
		if (e.getActionCommand() == UpdateViewerController.CMD_EDIT)
		{
			if (this.processCommandEditMode())
			{
				this.setState(States.LOADED);
			}
			return;
		}
		if (e.getActionCommand() == CMD_CLOSE_TAB)
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
		if (e.getActionCommand() == CMD_LOAD_PROFILE)
		{
			if (processCommandLoadSelectedProfile())
			{
				state = States.LOADED;
				this.viewer.setSelectionMode(States.LOADED);
				this.loadDialog.setVisible(false);
			}
			return;
		}
			
		if (e.getActionCommand() == CMD_CREATE_PROFILE)
		{
			this.processCommandAddLoadProfile();
			return;
		}
		
		if (e.getActionCommand() == CMD_EDIT_PROFILE)
		{
			String profname = this.loadDialog.getCurrentLoadProfileName();
			this.loadDialog.showEditLoadProfile(profname);
			return;
		}	
		
		if (e.getActionCommand() == CMD_SAVE_PROFILE)
		{
			if(this.processCommandSaveLoadProfile())
			{
				this.loadDialog.closeEditDialog();
			}
			return;
		}
		
			
		if (e.getActionCommand() == CMD_REMOVE_PROFILE)
		{
			String profileName = this.loadDialog.getCurrentLoadProfileName();
			if (this.processCommandRemoveLoadProfile(profileName))
			{
				this.loadDialog.removeLoadProfile(profileName);
			}
			return;
		}
			
		if (e.getActionCommand() == CMD_CREATE_PROFILEPOS)
		{
			this.processCommandAddRelationProfile();
			return;
		}
		
		if (e.getActionCommand() == CMD_EDIT_PROFILEPOS)
		{
			String profname = this.loadDialog.getCurrentLoadProfileName();
			String relname = this.loadDialog.getCurrentRelationProfileName();
			this.loadDialog.showEditRelationProfile(profname, relname);
			return;
		}
		
		if (e.getActionCommand() == CMD_SAVE_PROFILEPOS)
		{
			if (this.processCommandSaveRelationProfile())
			{
				this.loadDialog.closeEditDialog();
			}
			return;
		}
			
		if (e.getActionCommand() == CMD_REMOVE_PROFILEPOS)
		{
			String profileName = this.loadDialog.getCurrentLoadProfileName();
			String relName = this.loadDialog.getCurrentRelationProfileName();
			if (this.processCommandRemoveRelationProfile(profileName, relName))
			{
				this.loadDialog.removeRelationProfile(profileName, relName);
			}
			return;
		}
		
		
		if (e.getActionCommand() == CMD_CANCEL)
		{
			this.loadDialog.closeEditDialog();
			return;
		}
		
	
		//
		// Search		
		//
		if (e.getActionCommand() == CMD_SEARCH)
		{
			this.processCommandSearch();
			return;
		}
		if (e.getActionCommand() == CMD_CLEAR_SEARCH)
		{
			this.processCommandClearSearch();
			return;
		}
		if (e.getActionCommand() == CMD_NEXT)
		{
			this.processCommandNext();
			return;
			
		}
		if (e.getActionCommand() == CMD_PREVIOUS)
		{
			this.processCommandPrevious();
			return;
		}
		if (e.getActionCommand() == CMD_LAST)
		{
			this.viewer.getCurrentRelationPanel().showHit(this.viewer.getCurrentRelationPanel().getHitCount()-1);
			return;
		}
		if (e.getActionCommand() == CMD_FIRST)
		{
			this.viewer.getCurrentRelationPanel().showHit(0);			
			return;
		}
		if (e.getActionCommand() == CMD_REPLACE)
		{
			this.processCommandReplace();
			return;
		}
		if (e.getActionCommand() == CMD_REPLACE_ALL)
		{
			this.processCommandReplaceAll();
			return;
		}
		
		// This point should never be reached
		this.showErrorDialog("Command not known");
	}

	
	/**
	 * Creates empty profile relations, that can be queried.
	 * Returns true on success.
	 */
	private boolean createProfileRelations()
	{	
		List<String> commands = new ArrayList<String>();
		
		StringBuffer sb = new StringBuffer("let ");
		sb.append(RELNAME_LOAD_PROFILES_HEAD);
		sb.append(" = [const rel (tuple (");
		sb.append(" [ProfileName: string, FormatFields: text, ");
		sb.append(" FormatScript: text, FormatTemplate: text, OutputDir: text]");
		sb.append(" )) value ()]");
		commands.add(sb.toString());
		
		sb = new StringBuffer("let ");
		sb.append(RELNAME_LOAD_PROFILES_POS);
		sb.append(" = [const rel (tuple (");
		sb.append(" [ProfileName: string, RelName: string,");
		sb.append(" FilterExpr: text, ProjectExpr: text, SortExpr: text]");
		sb.append(" )) value ()]");
		commands.add(sb.toString());
		
		if (!this.executeBulk(commands))
		{
			return false;
		}
		return true;
	}
	
	/**
	 * Executes given commands as a transaction.
	 * Returns TRUE if transaction was successfully committed.
	 */
	private boolean executeBulk(List<String> pCommands)
	{
		String errorMessage;
		
		if(!this.commandExecuter.beginTransaction())
		{
			errorMessage = this.commandExecuter.getErrorMessage().toString();
			this.showErrorDialog("UpdateViewerController.executeBulk: Error on begin transaction: " 
								 + errorMessage);
			return false;
		}
		
		for (String command : pCommands)
		{
			if(!this.commandExecuter.executeCommand(command, SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX))
			{
				errorMessage = this.commandExecuter.getErrorMessage().toString();
				this.showErrorDialog("UpdateViewerController.executeBulk: Error on executing command " 
									 + command + ": " + errorMessage);
				if (! this.commandExecuter.abortTransaction())
				{
					errorMessage = this.commandExecuter.getErrorMessage().toString();
					this.showErrorDialog("UpdateViewerController.executeBulk: Error on abort transaction: " 
										 + errorMessage);
					return false;
				}
			}
		}
		
		if(!this.commandExecuter.commitTransaction())
		{
			errorMessage = this.commandExecuter.getErrorMessage().toString();
			this.showErrorDialog("UpdateViewerController.executeBulk: Error on commit transaction: " 
								 + errorMessage);
			return false;
		}
		
        return true;
	}
	

	/*	
	 * Executes delete commands generated for all relations in one transaction. 
	 * If one of the commands was not succesfully executed the transaction
	 * is aborted.
	 */
	private boolean executeBulkDelete()
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
				
				for (String com : commands){Reporter.debug("UpdateViewerController.executeDelete: " + com);}
				
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
	private boolean executeBulkInsert()
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
			List<String> commands = null;

			try
			{
				commands = this.commandGenerator.generateInsert(rp.getName(),
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
			
			for (String command : commands)
			{
				ListExpr le = this.executeInsert(command);
				if (le != null)
				{
					result.add(le);
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
	private boolean executeBulkUpdate()
	{
		String errorMessage;

		if(! this.commandExecuter.beginTransaction())
		{
			errorMessage = this.commandExecuter.getErrorMessage().toString();
			this.showErrorDialog(errorMessage);
			return false;
		}
		
		for (RelationPanel rp : this.viewer.getRelationPanels())
        {
            rp.takeOverLastEditing(true);
            
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
        
		if(!this.commandExecuter.commitTransaction())
		{
			errorMessage = this.commandExecuter.getErrorMessage().toString();
			this.showErrorDialog(errorMessage);
			return false;
		}
		
        return true;
	}
	
	/**
	 * Executes one single insert command.
	 * Returns the inserted tuple (with tuple ID)
	 */
	private ListExpr executeInsert(String pCommand)
	{
		Reporter.debug("UpdateViewerController.executeInsert: " + pCommand);
		if(!this.commandExecuter.executeCommand(pCommand, SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX))
		{
			String errorMessage = this.commandExecuter.getErrorMessage().toString();
			this.showErrorDialog("Error trying to insert a tuple: " + errorMessage);
			return null;
		}
		else
		{
			ListExpr le = new ListExpr();
			le.setValueTo(this.commandExecuter.getResultList());
			if (!le.isEmpty() && le.listLength() == 2)
			{
				return le.second().first(); 
			}
			return null;
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
	
		
	/*	
	 * Retrieves the relation with specified name and restrictions. 
	 */
	private ListExpr loadRelation(String pRelName, List<String> pFilter, List<String> pProject, List<String> pSort)
	{
		ListExpr result = null;

		StringBuffer command = new StringBuffer("query " + pRelName + " feed ");
				
		if (pFilter != null)
		{
			for (String filter : pFilter)
			{
				if (!filter.equals(""))
				{
					command.append(" filter [ " + filter + " ] ");
				}
			}
		}
		if (pProject != null)
		{			
			for (String project : pProject)
			{
				if (!project.equals(""))
				{
					command.append(" project [ " + project + " ] ");
				}
			}
		}
		if (pSort != null)
		{	
			for (String sort : pSort)
			{
				if (!sort.equals(""))
				{
					command.append(" sortBy [ " + sort + " ] ");
				}
			}
		}
		
		command.append(" addid consume ");
		//Reporter.debug("loadRelation: command=" + command.toString());
				
		if (commandExecuter.executeCommand(command.toString(),SecondoInterface.EXEC_COMMAND_SOS_SYNTAX))
		{
			result = new ListExpr();
			result.setValueTo(this.commandExecuter.getResultList());
			//Reporter.debug("loadRelation: resultLE=" + result.toString());
		}
		else
		{
			String errorMessage = commandExecuter.getErrorMessage().toString();
			this.showErrorDialog(errorMessage);
		}
		return result;
	} 
	
	
	/**
	 *
	 */
	private boolean processCommandAddLoadProfile()
	{
		String name = this.showInputProfileNameDialog();
		
		if (name == null || name.length() == 0)
		{
			return false;
		}
		
		if (this.loadDialog.getLoadProfile(name) != null)
		{
			this.showErrorDialog("Profile name already exists. Please choose a different name.");
			return false;
		}
		
		this.loadDialog.showEditLoadProfile(name);
		return true;
	}	 	
	
	
	/**
	 * Shows edit dialog for new relation profile. 
	 */
	private boolean processCommandAddRelationProfile()
	{
		String relname = this.showChooseRelationDialog();
		
		if (relname == null || relname.length() == 0)
		{
			return false;
		}
		
		String profname = this.loadDialog.getCurrentLoadProfileName();
		if (this.loadDialog.getRelationProfile(profname, relname) != null)
		{
			this.showErrorDialog("Relation already exists in this load profile. Please choose a different relation.");
			return false;
		}
		
		this.loadDialog.showEditRelationProfile(profname, relname);
		return true;
	}
		

	
	public void processCommandClearSearch()
	{
		this.searchActive = false;
		
		for (RelationPanel rp : this.viewer.getRelationPanels())
		{
			rp.clearSearch();
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
				result = this.executeBulkInsert();
				break;
			}
			case States.DELETE:
			{
				result = this.executeBulkDelete();
				break;
			}
			case States.UPDATE:
			{
				result = this.executeBulkUpdate();
				break;
			}
			default: break;
		}
		if (this.searchActive)
		{
			this.processCommandSearch();
		}
		return result;
	}

	
	/**
	 * Load Dialog action:
	 * Show currently selected load profile in editor.
	 */
	public boolean processCommandEditLoadProfile()
	{
		String loadProfile = this.loadDialog.getCurrentLoadProfileName();
		this.loadDialog.showEditLoadProfile(loadProfile);
		return true;
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
		LoadProfile loadProfile = this.loadDialog.getLoadProfile(this.loadDialog.getCurrentLoadProfileName());
		// check if all neccessary information is in load profile
		String formatFields = loadProfile.getFormatFields();
		String formatScript = loadProfile.getFormatScript();
		String formatTemplate = loadProfile.getFormatTemplate();
		String outputDir = loadProfile.getOutputDirectory();
		StringBuilder errorMessage = new StringBuilder();
		
		if (formatScript==null || formatScript.length() == 0)
		{
			errorMessage.append("Please specify path of file that contains the format script. \n");
		}
		if (outputDir==null || outputDir.length() == 0)
		{
			errorMessage.append("Please specify path of output directory. \n");
		}
		if (formatFields==null || formatFields.length() == 0)
		{
			errorMessage.append("Please specify FormatFields in the load profile. ");
			errorMessage.append("FormatFields is a list of triples [Placeholdername, Relationname, Attributename] ");
			errorMessage.append("that are to be displayed in the formatted document. \n");
			errorMessage.append("Example: \"CHAPTERTITLE,Chapters,Title; SUBTITLE,Chapters,Title2;  \" ");
		}
		if (errorMessage.toString().length() > 0)
		{
			this.showErrorDialog(errorMessage.toString());
			return false;
		}
		
		if (formatTemplate==null || formatTemplate.length() == 0)
		{
			this.showInfoDialog("Template file not specified in load profile. \nWill use default template. ");
		}
		
		// format
		try
		{
			DocumentFormatter formatter = new DocumentFormatter("text/html", 
																formatFields, formatScript, formatTemplate, outputDir);
			formatter.format();
			this.viewer.loadFormattedDocument(formatter.getOutputFiles(), formatter.getContentType());
			this.viewer.showRelations(States.FORMAT);
		}
		catch (Exception e)
		{
			this.showErrorDialog(e.getMessage());
			return false;
		}
		return true;
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
	
	/*
	 * Shows the load dialog.
	 * Loads Profiles from DB.
	 * Creates Profile relations if not yet existent.
	 */
	public void processCommandLoad() 
	{
		boolean result = true;
		
		if (!this.existsInDb(RELNAME_LOAD_PROFILES_HEAD))
		{
			result = this.createProfileRelations();
		}
		
		if (result)
		{
			try
			{
				ListExpr profilesLE = this.loadRelation(RELNAME_LOAD_PROFILES_HEAD, null, null, null);
				ListExpr relationsLE = this.loadRelation(RELNAME_LOAD_PROFILES_POS, null, null, null);
				
				if(profilesLE == null || relationsLE == null)
				{
					this.setState(States.INITIAL);
					this.showErrorDialog(commandExecuter.getErrorMessage().toString());			
				}
				else
				{
					this.loadDialog.showProfiles(profilesLE, relationsLE);		
					this.loadDialog.setVisible(true);
				}
			}
			catch (InvalidRelationException e)
			{
				Reporter.writeError(e.getMessage());
			}
		}
	}
	
	
	/**
	 * Loads all relations of the currently selected LoadProfile from database
	 * and shows them in the viewer.
	 */
	private boolean processCommandLoadSelectedProfile()
	{	
		this.viewer.clear();
		String profileName = this.loadDialog.getCurrentLoadProfileName();
		LoadProfile loadProfile = this.loadDialog.getLoadProfile(profileName);
		String errorMessage = "";
		
		if (loadProfile == null)
		{
			errorMessage = "Please select or create a Load Profile.";
			this.showErrorDialog(errorMessage);
			return false;
		}
				
		for(RelationProfile relprof : loadProfile.getRelationProfiles())
		{
			ListExpr relationLE = this.loadRelation(relprof.getRelationName(), 
													relprof.getFilterExpressions(), 
													relprof.getProjectExpressions(),
													relprof.getSortExpressions());

			if (relationLE == null)
			{
				errorMessage += "Error while loading relation " + relprof.getRelationName() + ". ";
			}
			else
			{
				this.viewer.setRelationPanel(relprof.getRelationName(), relationLE, true);
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
	
	/**
	 *
	 */
	private boolean processCommandNext()
	{
		int hitIndex = this.viewer.getCurrentRelationPanel().getCurrentHitIndex()+1;
		if (hitIndex<0 || hitIndex < this.viewer.getCurrentRelationPanel().getHitCount())
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
	 * Deletes all entries for specified load profile from relation uv2loadprofiles.
	 */
	private boolean processCommandRemoveLoadProfile(String pProfileName)
	{
		String profileName = this.loadDialog.getCurrentLoadProfileName();
		
		StringBuffer sb = new StringBuffer();
		sb.append("query ").append(RELNAME_LOAD_PROFILES_HEAD);
		sb.append(" feed filter[.ProfileName = \"");
		sb.append(pProfileName);
		sb.append("\"] consume feed ");
		sb.append(RELNAME_LOAD_PROFILES_HEAD);
		sb.append(" deletesearch consume");
		
		if (!commandExecuter.executeCommand(sb.toString(), SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX))
		{
			Reporter.debug(sb.toString());
			this.showErrorDialog(commandExecuter.getErrorMessage().toString());
			return false;
		}
		return true;
	}
	
	
	/**
	 * Deletes the entry for the specified relation profile from relation uv2loadprofiles.
	 */
	private boolean processCommandRemoveRelationProfile(String pLoadProfileName, String pRelName)
	{
		StringBuffer sb = new StringBuffer();
		sb.append("query ").append(RELNAME_LOAD_PROFILES_POS);
		sb.append(" feed filter[.ProfileName = \"").append(pLoadProfileName);
		sb.append("\"] filter[.RelName = \"").append(pRelName);
		sb.append("\"] consume feed ");
		sb.append(RELNAME_LOAD_PROFILES_POS);
		sb.append(" deletesearch consume");
		
		if (!commandExecuter.executeCommand(sb.toString(), SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX))
		{
			Reporter.debug(sb.toString());
			this.showErrorDialog(commandExecuter.getErrorMessage().toString());
			return false;
		}
		return true;

	}
	
	
	
	/**
	 * Replaces search key at current SearchHit position and skips to next SearchHit.
	 * Displays confirmation dialog first.
	 */
	private boolean processCommandReplace()
	{
		RelationPanel rp = this.viewer.getCurrentRelationPanel();
		String key = rp.getSearchKey();
		String replacement = rp.getReplacement();
		
		if (replacement == null)
		{
			replacement = "";
		}
		
		if (rp.getCurrentHitIndex() >= 0)
		{
			int option = JOptionPane.showConfirmDialog(rp
													   , "Replace by \"" + replacement + "\"?"
													   , "Replace and skip"
													   , JOptionPane.YES_NO_OPTION);
			if (option == JOptionPane.YES_OPTION)
			{					
				SearchHit hit = rp.getHit(rp.getCurrentHitIndex());
				rp.replace(hit, replacement);
				rp.showSearchResult(key);
				
				if (!rp.showHit(rp.getCurrentHitIndex()))
				{
					this.processCommandNext();
				}
			}
		}
		
		return true;
	}
	
	/**
	 * Replaces search key in all loaded tuples of all (editable) relations.
	 * Displays confirmation dialog first.
	 */
	private boolean processCommandReplaceAll()
	{
		RelationPanel crp = this.viewer.getCurrentRelationPanel();
		String replacement = crp.getReplacement();
		String key = crp.getSearchKey();
		int count = 0;

		StringBuilder msg = new StringBuilder();
		msg.append("Really replace all search matches by \"").append(replacement).append("\" in all loaded relations?");
		if (!this.caseSensitive)
		{
			msg.append("\n\n(Please note: current search mode is CASE-INSENSITIVE.)");
		}
		
		int option = JOptionPane.showConfirmDialog(crp 
												   , msg.toString()
												   , "Replace all"
												   , JOptionPane.YES_NO_OPTION);
		
		if (option == JOptionPane.YES_OPTION)
		{
			for (RelationPanel rp : this.viewer.getRelationPanels())
			{
				while (rp.hasSearchHits())
				{
					rp.replace(rp.getHit(0), replacement);
					count ++;
				}
				rp.showSearchResult(key);
			}			
		}
		
		this.showInfoDialog("Replaced " + count + " occurences.");
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
		
		if (this.searchActive)
		{
			this.processCommandSearch(); 
		}
		
		return true;
	}
	
	
	private boolean insertTuple(String pRelName, Tuple pTuple, ListExpr ioLE)
	{
		if (pTuple == null)
		{
			return false;
		}
		
		try
		{
			List<Tuple> li = new ArrayList<Tuple>();
			li.add(pTuple);
			RelationTypeInfo type = pTuple.getTypeInfo();
			
			List<String> commands = this.commandGenerator.generateInsert(pRelName,
																		 type.getAttributeNames(),
																		 type.getAttributeTypes(),
																		 li);

			ListExpr tupleLE = this.executeInsert(commands.get(0));
			if (tupleLE != null)
			{
				Reporter.debug("insertTuple: resultLE=" + tupleLE.toString());
				ioLE.setValueTo(tupleLE);
			}

		}
		catch (InvalidFormatException e)
		{
			this.showErrorDialog("InvalidFormatException: " + e.getMessage());
			return false;
		}	
		return true;	
	}
	
	
	private boolean updateTuple(String pRelName, List<String> pAttributeNames, 
								Map<Integer,HashMap<String, Change>> pChangesForUpdate)
	{
		try 
		{
			List<String> commands = this.commandGenerator.generateUpdate(pRelName,
																		 pAttributeNames,
																		 pChangesForUpdate);

			if(! this.commandExecuter.executeCommand(commands.get(0), SecondoInterface.EXEC_COMMAND_LISTEXPR_SYNTAX))
			{
				String errorMessage = this.commandExecuter.getErrorMessage().toString();
				this.showErrorDialog("Error trying to update a tuple: " + errorMessage);
				return false;
			}
			ListExpr result = this.commandExecuter.getResultList();
			if ( result.second().intValue() != 1)
			{
				return false;
			}		
		}
		catch (InvalidFormatException e)
		{
			this.showErrorDialog("InvalidFormatException: " + e.getMessage());
			return false;
		}	
		return true;		
	}
	
	/**
	 *
	 */
	private boolean processCommandSaveLoadProfile()
	{
		Tuple tuple = this.loadDialog.getEditTuple();
		if (tuple== null)
		{
			return false;
		}
		
		if (tuple.getID() == null || tuple.getID().length() == 0) // tuple is to be inserted
		{
			ListExpr resultLE = new ListExpr();
			if (!this.insertTuple(RELNAME_LOAD_PROFILES_HEAD, tuple, resultLE))
			{
				return false;
			}
			try
			{
				this.loadDialog.addLoadProfile(resultLE);
			}
			catch (InvalidRelationException e)
			{
				this.showErrorDialog(e.getMessage());
				return false;
			}
		}
		else // tuple is to be updated
		{
			List<String> attributeNames = tuple.getTypeInfo().getAttributeNames();
			Map<Integer, HashMap<String, Change>> changesForUpdate = this.loadDialog.getUpdateTuples();

			if (!this.updateTuple(RELNAME_LOAD_PROFILES_HEAD, attributeNames, changesForUpdate))
			{
				return false;
			}
			try 
			{
				this.loadDialog.updateLoadProfile(tuple);
			}
			catch (InvalidRelationException e) 
			{
				this.showErrorDialog(e.getMessage());
				return false;
			}
		}
		
		return true;		
	}
	
	
	/**
	 *
	 */
	private boolean processCommandSaveRelationProfile()
	{
		Tuple tuple = this.loadDialog.getEditTuple();
		if (tuple== null)
		{
			return false;
		}
		
		if (tuple.getID() == null || tuple.getID().length() == 0) // tuple is to be inserted
		{
			ListExpr resultLE = new ListExpr();
			if (!this.insertTuple(RELNAME_LOAD_PROFILES_POS, tuple, resultLE))
			{
				return false;
			}
			try
			{
				this.loadDialog.addRelationProfile(resultLE);
			}
			catch (InvalidRelationException e)
			{
				this.showErrorDialog(e.getMessage());
				return false;
			}
		}
		else // tuple is to be updated
		{
			List<String> attributeNames = tuple.getTypeInfo().getAttributeNames();
			Map<Integer, HashMap<String, Change>> changesForUpdate = this.loadDialog.getUpdateTuples();
			
			if (!this.updateTuple(RELNAME_LOAD_PROFILES_POS, attributeNames, changesForUpdate))
			{
				return false;
			}
			try
			{
				this.loadDialog.updateRelationProfile(tuple);
			}
			catch (InvalidRelationException e)
			{
				this.showErrorDialog(e.getMessage());
				return false;
			}
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
		
		// retrieve hitlists for all relation panels
		if (key == null || key.length() < 3)
		{
			this.showInfoDialog("Key length must be 3 at least");
		}
		else
		{
			Map<String,List<SearchHit>> map = new HashMap<String,List<SearchHit>>();
			
			for (RelationPanel rp : this.viewer.getRelationPanels())
			{								
				List<SearchHit> hitlist = rp.retrieveSearchHits(key, this.caseSensitive);
				if (!hitlist.isEmpty())
				{
					map.put(rp.getName(), hitlist);
				}
			}
			
			// if none of the relations has hits, display messagebox, else display search results in RelationPanel.
			if (map.isEmpty())
			{
				showErrorDialog("No matches found.");
				this.processCommandClearSearch();
			}
			else
			{
				this.searchActive = true;
				
				int first = -2;
				for (RelationPanel rp : this.viewer.getRelationPanels())
				{								
					List<SearchHit> hitlist = map.get(rp.getName());
					if (hitlist != null)
					{
						rp.setSearchHits(hitlist);
						rp.showSearchResult(key);
						if (first<0)
						{
							first = this.viewer.getRelationPanels().indexOf(rp);
						}
					}
				}
				// go to first hit in first RelationPanel with a hit (if there is any)
				if (first >= 0)
				{
					this.viewer.showRelationPanel(first);
					RelationPanel rp = this.viewer.getCurrentRelationPanel();
					rp.showHit(0);
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
			if (selection != null)
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
	 * Shows messagebox with the error message.	 
	 */
	public void showErrorDialog(String pMessage)
	{
		Reporter.showError(pMessage);
		this.viewer.repaint();
		this.viewer.validate();
	}
	
	/*
	 * Shows messagebox with the info message.	 
	 */
	public void showInfoDialog(String pMessage)
	{
		Reporter.showInfo(pMessage);
		this.viewer.repaint();
		this.viewer.validate();
	}
	
	
}
