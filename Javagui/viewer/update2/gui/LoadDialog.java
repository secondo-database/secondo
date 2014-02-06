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
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.Frame;
import java.awt.GridLayout;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;

import javax.swing.BorderFactory;
import javax.swing.DefaultListModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.JSplitPane;
import javax.swing.JTable;
import javax.swing.ListModel;
import javax.swing.ListSelectionModel;
import javax.swing.table.TableColumn;

import project.Projection;
import sj.lang.ListExpr;
import sj.lang.ServerErrorCodes;
import tools.Reporter;

import viewer.update2.*;
import viewer.update2.gui.*;


/**
 * Dialog to pick relations and specify restrictions.	 
 */
public class LoadDialog extends JDialog implements ListSelectionListener
{
	
	private TupleEditDialog editDialog;

	private UpdateViewerController controller;

	// load profiles and relation profiles
	private List<String> loadProfiles;	
	private Relation relLoadProfiles;
	private Relation relRelProfiles;
	
	// lists
	private DefaultListModel lmProfiles;
	private JList lsProfiles;
	private DefaultListModel lmRelations;
	private JList lsRelations;
	
	// buttons
	private JButton btLoadDirect;
	private JButton btLoadFromProfile;
	private JButton btClose;
	private JButton btNewProfile;
	private JButton btEditProfile;
	private JButton btRemoveProfile;
	private JButton btAddRelation;
	private JButton btEditRelation;
	private JButton btRemoveRelation;

	// have LoadProfiles been loaded from DB? 
	private int state;
	

	/** Constructor */
	public LoadDialog(UpdateViewerController pController)
	{
		this.state = States.INITIAL;
						
		this.controller = pController;
	
		this.setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
		this.setModal(true);
		this.setSize (600,400);
		this.setTitle("Load relations");
		
		// buttons
		this.btLoadFromProfile = new JButton("Load from profile");
		this.btLoadFromProfile.addActionListener(controller);
		this.btLoadFromProfile.setActionCommand(UpdateViewerController.CMD_LOAD_FROM_PROFILE);
		this.btLoadFromProfile.setEnabled(false);
		this.btLoadFromProfile.setToolTipText("Load relation(s) from selected profile");

		this.btLoadDirect = new JButton("Load direct");
		this.btLoadDirect.addActionListener(controller);
		this.btLoadDirect.setActionCommand(UpdateViewerController.CMD_LOAD_DIRECT);
		this.btLoadDirect.setToolTipText("Load a single relation directly without load profile");

		this.btClose = new JButton("Close");
		this.btClose.addActionListener(controller);
		this.btClose.setActionCommand(UpdateViewerController.CMD_CLOSE_LOAD_DIALOG);
		this.btClose.setToolTipText("Hide this window");

		this.btNewProfile = new JButton(UpdateViewerController.CMD_CREATE_PROFILE);
		this.btNewProfile.addActionListener(controller);
		this.btNewProfile.setToolTipText("Create a new load profile");

		this.btEditProfile = new JButton(UpdateViewerController.CMD_EDIT_PROFILE);
		this.btEditProfile.addActionListener(controller);
		this.btEditProfile.setEnabled(false);
		this.btEditProfile.setToolTipText("Edit the selected load profile");

		this.btRemoveProfile = new JButton(UpdateViewerController.CMD_REMOVE_PROFILE);
		this.btRemoveProfile.addActionListener(controller);
		this.btRemoveProfile.setEnabled(false);
		this.btRemoveProfile.setToolTipText("Remove the selected load profile");
		
		this.btAddRelation = new JButton(UpdateViewerController.CMD_CREATE_PROFILEPOS);
		this.btAddRelation.addActionListener(controller);
		this.btAddRelation.setEnabled(false);
		this.btAddRelation.setToolTipText("Add a relation to the current load profile");

		this.btEditRelation = new JButton(UpdateViewerController.CMD_EDIT_PROFILEPOS);
		this.btEditRelation.addActionListener(controller);
		this.btEditRelation.setEnabled(false);
		this.btEditRelation.setToolTipText("Edit restrictions for the selected relation");

		this.btRemoveRelation = new JButton(UpdateViewerController.CMD_REMOVE_PROFILEPOS);
		this.btRemoveRelation.addActionListener(controller);
		this.btRemoveRelation.setEnabled(false);
		this.btRemoveRelation.setToolTipText("Remove the selected relation from the load profile");
		
		// profiles list
		this.lmProfiles = new DefaultListModel();
		this.lsProfiles = new JList();
		this.lsProfiles.setSelectionMode(ListSelectionModel.SINGLE_INTERVAL_SELECTION);
		this.lsProfiles.addListSelectionListener(this);
		this.lsProfiles.setVisibleRowCount(-1);
		
		// relations list
		this.lmRelations = new DefaultListModel();
		this.lsRelations = new JList();
		this.lsRelations.setSelectionMode(ListSelectionModel.SINGLE_INTERVAL_SELECTION);
		this.lsRelations.addListSelectionListener(this);
		this.lsRelations.setVisibleRowCount(-1);
		
		// component arrangement
		JPanel plProfileButtons = new JPanel(new GridLayout(7, 1));
		plProfileButtons.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
		plProfileButtons.add(this.btNewProfile);
		plProfileButtons.add(this.btEditProfile);
		plProfileButtons.add(this.btRemoveProfile);		
		JPanel space = new JPanel();
		space.setPreferredSize(new Dimension(200,10));
		plProfileButtons.add(space);		
		
		JPanel plRelationButtons = new JPanel(new GridLayout(7, 1));
		plRelationButtons.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
		plRelationButtons.add(this.btAddRelation);
		plRelationButtons.add(this.btEditRelation);
		plRelationButtons.add(this.btRemoveRelation);
		JPanel space2 = new JPanel();
		space2.setPreferredSize(new Dimension(200,10));
		plRelationButtons.add(space2);		

		JPanel plGeneralButtons = new JPanel();
		plGeneralButtons.add(this.btLoadFromProfile);
		plGeneralButtons.add(this.btLoadDirect);
		plGeneralButtons.add(this.btClose);
		
		JPanel plProfiles = new JPanel(new BorderLayout());
		plProfiles.setBorder(BorderFactory.createEmptyBorder(5,10,5,5));
		plProfiles.add(new JLabel("Load profiles:"), BorderLayout.NORTH);		
		JScrollPane scpProfiles = new JScrollPane(lsProfiles);
		plProfiles.add(scpProfiles, BorderLayout.CENTER);
		plProfiles.add(plProfileButtons, BorderLayout.EAST);
		
		JPanel plRelations = new JPanel(new BorderLayout());
		plRelations.setBorder(BorderFactory.createEmptyBorder(5,10,5,5));
		plRelations.add(new JLabel("Relations:"), BorderLayout.NORTH);		
		JScrollPane scpRelations = new JScrollPane(lsRelations);
		plRelations.add(scpRelations, BorderLayout.CENTER);
		plRelations.add(plRelationButtons, BorderLayout.EAST);
		
		JPanel listPanel = new JPanel(new GridLayout(2,1));
		listPanel.add(plProfiles);
		listPanel.add(plRelations);
		
		this.getContentPane().setLayout(new BorderLayout());
		this.getContentPane().add(listPanel, BorderLayout.CENTER);
		this.getContentPane().add(plGeneralButtons, BorderLayout.SOUTH);
	}
	
	
	public void addLoadProfile(ListExpr pTupleLE) throws InvalidRelationException
	{
		Tuple tuple = this.relLoadProfiles.createEmptyTuple();
		tuple.readValueFromLE(pTupleLE);
		this.relLoadProfiles.addTuple(tuple);
		String name = tuple.getValueByAttrName("ProfileName");
		this.lmProfiles.addElement(name);
		this.lsProfiles.setSelectedValue(name, true);
	}
	
	public void addRelationProfile(ListExpr pTupleLE) throws InvalidRelationException
	{
		Tuple tuple = this.relRelProfiles.createEmptyTuple();
		tuple.readValueFromLE(pTupleLE);
		this.relRelProfiles.addTuple(tuple);
		String name = tuple.getValueByAttrName("RelName");
		this.lmRelations.addElement(name);
		this.lsRelations.setSelectedValue(name, true);
	}
	
	
	public void updateLoadProfile(Tuple pTuple) throws InvalidRelationException
	{
		this.relLoadProfiles.setTupleByID(pTuple);
		String name = pTuple.getValueByAttrName("ProfileName");
	}
	
	public void updateRelationProfile(Tuple pTuple) throws InvalidRelationException
	{
		this.relRelProfiles.setTupleByID(pTuple);
		String name = pTuple.getValueByAttrName("RelName");
	}
	
	
	public void removeLoadProfile(String pProfileName)
	{
		List<Tuple> tuples = this.relLoadProfiles.getTuplesByFilter(Filter.FILTERTYPE_EQUALS, "ProfileName", pProfileName);
		if (tuples != null && tuples.size()==1)
		{
			Tuple tuple = tuples.get(0);
			this.relLoadProfiles.removeTupleByID(tuple.getID());
			this.lmProfiles.removeElement(pProfileName);		
		}
	}
	
	
	public void removeRelationProfile(String pProfileName, String pRelName)
	{
		List<Filter> filters = new ArrayList<Filter>();
		filters.add(new Filter(Filter.FILTERTYPE_EQUALS, "ProfileName", pProfileName));
		filters.add(new Filter(Filter.FILTERTYPE_EQUALS, "RelName", pRelName));
		List<Tuple> tuples = this.relRelProfiles.getTuplesByFilter(filters);			
		for (Tuple tuple : tuples)
		{
			this.relRelProfiles.removeTupleByID(tuple.getID());
			this.lmRelations.removeElement(pRelName);		
		}
	}
	
	
	public void closeEditDialog()
	{
		this.editDialog.dispose();
		this.editDialog = null;
	}
	
		
	/**
	 *
	 */
	public String getCurrentLoadProfileName()
	{
		Object o = this.lsProfiles.getSelectedValue();
		if (o != null)
		{
			return o.toString();
		}
		return null;
	}
	
	/**
	 *
	 */
	public String getCurrentRelationProfileName()
	{
		Object o = this.lsRelations.getSelectedValue();
		if (o != null)
		{
			return o.toString();
		}
		return null;
	}
	
	public Tuple getEditTuple()
	{
		if (this.editDialog == null)
		{
			return null;
		}
		this.editDialog.takeOverLastEditing();
		return this.editDialog.getEditTuple();
	}
	
	/*
	 * Returns values to be updated.
	 */	
	public Map<Integer, HashMap<String, Change>> getUpdateTuples()
	{
		return this.editDialog.getUpdateTuples();
	}
	
	
	/**
	 * Returns profile positions for specified load profiles
	 */
	public LoadProfile getLoadProfile(String pProfileName)
	{
		LoadProfile result = null;
		List<Filter> filters = new ArrayList<Filter>();
		filters.add(new Filter(Filter.FILTERTYPE_EQUALS, "ProfileName", pProfileName));
		List<Tuple> tuli = this.relLoadProfiles.getTuplesByFilter(filters);
		if (tuli != null && !tuli.isEmpty())
		{
			Tuple tu = tuli.get(0);
			result = new LoadProfile(tu.getValueByAttrName("ProfileName"));
			result.setFormatType(tu.getValueByAttrName("FormatType"));
			result.setFormatAliases(tu.getValueByAttrName("FormatAliases"));
			result.setFormatQuery(tu.getValueByAttrName("FormatQuery"));
			result.setFormatScript(tu.getValueByAttrName("FormatScript"));
			result.setFormatTemplateBody(tu.getValueByAttrName("FormatTemplateBody"));
			result.setFormatTemplateHead(tu.getValueByAttrName("FormatTemplateHead"));
			result.setFormatTemplateTail(tu.getValueByAttrName("FormatTemplateTail"));
			result.setOutputDirectory(tu.getValueByAttrName("OutputDir"));
			for (RelationProfile relprof : this.getRelationProfiles(pProfileName))
			{
				result.addRelationProfile(relprof);
			}
		}
		return result;
	}
	
	
	/**
	 * Returns specified load profile position.
	 */
	public RelationProfile getRelationProfile(String pProfileName, String pRelName)
	{
		RelationProfile result = null;
		List<Filter> filters = new ArrayList<Filter>();
		filters.add(new Filter(Filter.FILTERTYPE_EQUALS, "ProfileName", pProfileName));
		filters.add(new Filter(Filter.FILTERTYPE_EQUALS, "RelName", pRelName));
		List<Tuple> tuli = this.relRelProfiles.getTuplesByFilter(filters);
		if (tuli != null && !tuli.isEmpty())
		{
			Tuple tu = tuli.get(0);
			result = new RelationProfile(tu.getValueByAttrName("ProfileName"), tu.getValueByAttrName("RelName"));
			result.setFilterExpressions(splitList(tu.getValueByAttrName("FilterExpr"), ";"));
			result.setProjectExpressions(splitList(tu.getValueByAttrName("ProjectExpr"), ";"));
			result.setSortExpressions(splitList(tu.getValueByAttrName("SortExpr"), ";"));
		}
		return result;
	}
	
	
	/**
	 * Returns all profile positions for specified load profile
	 */
	public List<RelationProfile> getRelationProfiles(String pProfileName)
	{
		List<RelationProfile> result = new ArrayList<RelationProfile>();
		List<Filter> filters = new ArrayList<Filter>();
		filters.add(new Filter(Filter.FILTERTYPE_EQUALS, "ProfileName", pProfileName));
		List<Tuple> tuli = this.relRelProfiles.getTuplesByFilter(filters);
		if (tuli != null && !tuli.isEmpty())
		{
			for (Tuple tu : tuli)
			{
				RelationProfile profile = new RelationProfile(tu.getValueByAttrName("ProfileName"), tu.getValueByAttrName("RelName"));
				profile.setFilterExpressions(splitList(tu.getValueByAttrName("FilterExpr"), ";"));
				profile.setProjectExpressions(splitList(tu.getValueByAttrName("ProjectExpr"), ";"));
				profile.setSortExpressions(splitList(tu.getValueByAttrName("SortExpr"), ";"));
				result.add(profile);
			}
		}
		return result;
	}

	
	/**
	 * Returns true if LoadProfiles have been loaded from DB.
	 */
	public int getState()
	{
		return this.state;
	}
	
	
	public boolean hasLoadProfiles()
	{
		if (this.state == States.INITIAL)
		{
			return false;
		}
		else
		{
			return this.relLoadProfiles.getTupleCount() > 0;
		}
	}
		
	
	/**
	 * Returns a list of the non-empty substrings in the specified
	 * semicolon-seprated list.
	 */
	public static List<String> splitList(String pSeparatedList, String pSeparator)
	{
		List<String> result = new ArrayList<String>();
		if (pSeparatedList != null && pSeparatedList.length() > 0)
		{
			String[] strings = pSeparatedList.split(pSeparator);
			
			for (String s : strings)
			{
				if (s.length()>0)
				{
					result.add(s);
				}
			}
		}
		//Reporter.debug("splitList(" + pSeparatedList + ", " + pSeparator + ") returned " + result.toString());
		return result;
	}
	
	
	public void showEditLoadProfile(String pProfileName)
	{
		List<Tuple> tuples = this.relLoadProfiles.getTuplesByFilter(Filter.FILTERTYPE_EQUALS, "ProfileName", pProfileName);
		Tuple tuple;
		int state = States.UPDATE;
		
		if (tuples.size() == 0)
		{
			tuple = this.relLoadProfiles.createEmptyTuple();
			tuple.setValueByAttrName(pProfileName, "ProfileName");
			state = States.INSERT;
		}
		else
		{
			tuple = tuples.get(0);
		}
		this.editDialog = new TupleEditDialog(relLoadProfiles, tuple, this.controller, state, 
											  UpdateViewerController.CMD_EDIT_PROFILE);
		this.editDialog.setVisible(true);
	}
	
	
	public void showEditRelationProfile(String pProfileName, String pRelName) 
	{
		List<Filter> filters = new ArrayList<Filter>();
		filters.add(new Filter(Filter.FILTERTYPE_EQUALS, "ProfileName", pProfileName));
		filters.add(new Filter(Filter.FILTERTYPE_EQUALS, "RelName", pRelName));
		List<Tuple> tuples = this.relRelProfiles.getTuplesByFilter(filters);
		Tuple tuple;
		int state = States.UPDATE;
		
		if (tuples.size() == 0)
		{
			tuple = this.relRelProfiles.createEmptyTuple();
			tuple.setValueByAttrName(pProfileName, "ProfileName");
			tuple.setValueByAttrName(pRelName, "RelName");
			state = States.INSERT;
		}
		else
		{
			tuple = tuples.get(0);
		}
		this.editDialog = new TupleEditDialog(this.relRelProfiles, tuple, this.controller, state, 
											  UpdateViewerController.CMD_EDIT_PROFILEPOS);
		this.editDialog.setVisible(true);
	}


	/**
	 * Shows profiles if load profiles are found in database
	 * else shows info message.
	 */
	public void showProfiles(ListExpr pLoadProfilesLE, ListExpr pRelProfilesLE) throws InvalidRelationException
	{
		//Reporter.debug("LoadDialog.showProfiles: START");
		
		SecondoObject profilesSO = new SecondoObject(UpdateViewerController.RELNAME_LOAD_PROFILES_HEAD, pLoadProfilesLE);
		this.relLoadProfiles = new Relation();
		this.relLoadProfiles.readFromSecondoObject(profilesSO);
		this.relLoadProfiles.setAttributeReadOnly("ProfileName");
		
		SecondoObject relationsSO = new SecondoObject(UpdateViewerController.RELNAME_LOAD_PROFILES_POS, pRelProfilesLE);
		this.relRelProfiles = new Relation();
		this.relRelProfiles.readFromSecondoObject(relationsSO);
		this.relRelProfiles.setAttributeReadOnly("ProfileName");
		this.relRelProfiles.setAttributeReadOnly("RelationName");
		
		this.lmProfiles = new DefaultListModel();
		for (int i = 0; i<relLoadProfiles.getTupleCount(); i++)
		{
			Tuple tup = relLoadProfiles.getTupleAt(i);
			this.lmProfiles.addElement(tup.getValueByAttrName("ProfileName"));
		}
		this.lsProfiles.setModel(lmProfiles);
		this.lsProfiles.setSelectedIndex(0);

		this.repaint();
		this.validate();		
		
		if (this.relLoadProfiles == null || this.relLoadProfiles.getTupleCount() == 0)
		{
			Reporter.showInfo("This database contains no load profiles for loading relations. "
							  + "Click NEW LOAD PROFILE to create one.");
		}
		else
		{
			this.state = States.LOADED;
		}
	}
	
	
	/**
	 * Method of Interface ListSelectionListener.
	 * Shows selected LoadProfile in the center panel.
	 */
	public void valueChanged(ListSelectionEvent e)
	{
		if (e.getSource() == this.lsProfiles)
		{
			String profileName = (String)lsProfiles.getSelectedValue();
			
			if (profileName != null)
			{
				this.btLoadFromProfile.setEnabled(true);
				this.btEditProfile.setEnabled(true);
				this.btRemoveProfile.setEnabled(true);
				this.btAddRelation.setEnabled(true);
				
				this.lmRelations = new DefaultListModel();
				for (int i = 0; i<relRelProfiles.getTupleCount(); i++)
				{
					Tuple tup = relRelProfiles.getTupleAt(i);
					if (tup.getValueByAttrName("ProfileName").equals(profileName))
					{
						this.lmRelations.addElement(tup.getValueByAttrName("RelName"));
					}
				}
				this.lsRelations.setModel(lmRelations);
				this.lsRelations.setSelectedIndex(0);
			}
			else
			{
				this.btLoadFromProfile.setEnabled(false);
				this.btEditProfile.setEnabled(false);
				this.btRemoveProfile.setEnabled(false);
				this.btAddRelation.setEnabled(false);
			}
		}
		
		if (e.getSource() == this.lsRelations)
		{
			String relationName = (String)lsRelations.getSelectedValue();
			if (relationName != null)
			{
				this.btEditRelation.setEnabled(true);
				this.btRemoveRelation.setEnabled(true);
			}
			else
			{
				this.btEditRelation.setEnabled(false);
				this.btRemoveRelation.setEnabled(false);
			}
		}		
		this.repaint();
		this.validate();
	}
}

