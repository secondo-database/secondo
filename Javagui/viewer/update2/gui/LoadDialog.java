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
import java.awt.event.InputEvent;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.awt.Font;
import java.awt.Frame;
import java.awt.GridLayout;
import java.io.File;
import java.io.FileInputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;
import java.util.Properties;
import java.util.StringTokenizer;
import java.util.Vector;

import javax.swing.AbstractAction;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.DefaultListModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.MouseInputAdapter;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButtonMenuItem;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.JSplitPane;
import javax.swing.JTabbedPane;
import javax.swing.JTextArea;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.KeyStroke;
import javax.swing.ListModel;
import javax.swing.ListSelectionModel;
import javax.swing.SwingConstants;

import project.Projection;
import sj.lang.ListExpr;
import sj.lang.ServerErrorCodes;
import tools.Reporter;

import viewer.update2.*;

/**
 * Dialog to pick relations and specify restrictions.	 
 */
public class LoadDialog extends JDialog implements ListSelectionListener
{

	// LoadProfiles by their names
	private Map<String, LoadProfile> profiles;
	
	private DefaultListModel lmProfiles;
	private JList lsProfiles;
	private JScrollPane scpProfiles;
	
	// Controller
	private UpdateViewerController controller;
	
	// Buttons
	private JPanel plButtons;
		
	private JButton btLoad;
	
	private JButton btNewProfile;
	
	private JButton btRemoveProfile;
		
	private JButton btCancel;

		
	// Display/Editor area for current Load Profile
	private LoadProfilePanel plLoadProfile;
	
	// have LoadProfiles been loaded (or at least tried) from DB? 
	private boolean loaded;
	

	/** Constructor */
	public LoadDialog(UpdateViewerController pController)
	{
		this.loaded = false;
						
		this.controller = pController;
		
		this.addComponentListener(this.controller);

		this.getContentPane().setLayout(new BorderLayout());
		this.setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
		this.setModal(true);
		this.setSize (600,400);
		this.setTitle("Load relations");
		
		// buttons
		this.plButtons = new JPanel();
		this.plButtons.setLayout(new GridLayout(1, 7));
		this.btLoad = new JButton("Load selected profile");
		this.btLoad.addActionListener(controller);
		this.plButtons.add(this.btLoad);
		this.btNewProfile = new JButton("Create new profile");
		this.btNewProfile.addActionListener(controller);
		this.plButtons.add(this.btNewProfile);
		this.btRemoveProfile = new JButton("Delete profile");
		this.btRemoveProfile.addActionListener(controller);
		this.plButtons.add(this.btRemoveProfile);
		this.btCancel = new JButton("Cancel");
		this.btCancel.addActionListener(controller);
		this.plButtons.add(this.btCancel);
		
		// profile selection list
		this.lmProfiles = new DefaultListModel();
		this.lsProfiles = new JList();
		this.lsProfiles.setSelectionMode(ListSelectionModel.SINGLE_INTERVAL_SELECTION);
		//this.lsProfiles.setLayoutOrientation(JList.HORIZONTAL_WRAP);
		this.lsProfiles.addListSelectionListener(this);
		this.lsProfiles.setVisibleRowCount(-1);
		this.scpProfiles = new JScrollPane(lsProfiles);
		this.scpProfiles.setPreferredSize(new Dimension(250, 80));
		
		// profile detail panels
		this.profiles = new HashMap<String, LoadProfile>();		
		this.plLoadProfile = new LoadProfilePanel(this.controller);

		// 
		this.getContentPane().add(this.scpProfiles, BorderLayout.NORTH);
		this.getContentPane().add(this.plButtons, BorderLayout.SOUTH);
		this.getContentPane().add(this.plLoadProfile, BorderLayout.CENTER);
	}
	
	public boolean addProfile(String pProfileName)
	{
		if (profiles.containsKey(pProfileName))
		{
			return false;
		}
		
		this.profiles.put(pProfileName, new LoadProfile(pProfileName));
		
		return true;
	}
	
	/**
	 * Builds LoadProfiles from relation of following type expression:
	 * rel(tuple([ProfileName: string, RelName: string, FilterExpr: text, ProjectExpr: text, SortExpr: text]))
	 */
	public boolean createLoadProfilesFromLE(ListExpr LE)
	{
		// check validity of type expression
		if (LE.listLength() != 2)
		{
			return false;
		}
		//Reporter.debug(LE.toString());

		ListExpr type = LE.first();
		ListExpr value = LE.second();
		
		// check relation type
		if (type.isAtom())
		{
			return false;
		}
		ListExpr maintype = type.first();
		if (type.listLength() != 2
			|| !maintype.isAtom()
			|| maintype.atomType() != ListExpr.SYMBOL_ATOM
			|| !(maintype.symbolValue().equals("rel") | maintype
				 .symbolValue().equals("mrel")))
		{
			return false; // not a relation
		}
		ListExpr tupletype = type.second();
		
		// check tuple type
		ListExpr TupleFirst = tupletype.first();
		if (tupletype.listLength() != 2
			|| !TupleFirst.isAtom()
			|| TupleFirst.atomType() != ListExpr.SYMBOL_ATOM
			|| !(TupleFirst.symbolValue().equals("tuple") | TupleFirst
				 .symbolValue().equals("mtuple")))
			return false; // not a tuple
		ListExpr attributes = tupletype.second();
		
		// check attribute types	
		if (attributes.listLength() != 5){
			return false;
		}
		
		ListExpr attrType = attributes.first();
		ListExpr aName;
		ListExpr aType;
		if(attrType.listLength() != 2)
		{
			return false;
		}
			
		// check attribute ProfileName
		aName = attrType.first();
		aType = attrType.second();
		if (!aName.isAtom()
			|| aName.atomType() != ListExpr.SYMBOL_ATOM
			|| !aName.symbolValue().equals("ProfileName") 
			|| !aType.isAtom()
			|| aType.atomType() != ListExpr.SYMBOL_ATOM
			|| !aType.symbolValue().equals("string") ) {
			return false;
		}
		
		// check attribute RelName
		attrType = attributes.second();
		aName = attrType.first();
		aType = attrType.second();
		if (!aName.isAtom()
			|| aName.atomType() != ListExpr.SYMBOL_ATOM
			|| !aName.symbolValue().equals("RelName") 
			|| !aType.isAtom()
			|| aType.atomType() != ListExpr.SYMBOL_ATOM
			|| !aType.symbolValue().equals("string") ) {
			return false;
		}
		
		// check attribute FilterExpr
		attrType = attributes.third();
		aName = attrType.first();
		aType = attrType.second();
		if (!aName.isAtom()
			|| aName.atomType() != ListExpr.SYMBOL_ATOM
			|| !aName.symbolValue().equals("FilterExpr") 
			|| !aType.isAtom()
			|| aType.atomType() != ListExpr.SYMBOL_ATOM
			|| !aType.symbolValue().equals("text") ) {
			return false;
		}
			
		// check attribute ProjectExpr
		attrType = attributes.fourth();
		aName = attrType.first();
		aType = attrType.second();
		if (!aName.isAtom()
			|| aName.atomType() != ListExpr.SYMBOL_ATOM
			|| !aName.symbolValue().equals("ProjectExpr") 
			|| !aType.isAtom()
			|| aType.atomType() != ListExpr.SYMBOL_ATOM
			|| !aType.symbolValue().equals("text") ) {
			return false;
		}
		

		// check attribute SortExpr
		attrType = attributes.fifth();
		aName = attrType.first();
		aType = attrType.second();
		if (!aName.isAtom()
			|| aName.atomType() != ListExpr.SYMBOL_ATOM
			|| !aName.symbolValue().equals("SortExpr") 
			|| !aType.isAtom()
			|| aType.atomType() != ListExpr.SYMBOL_ATOM
			|| !aType.symbolValue().equals("text") ) {
			return false;
		}
		
		Reporter.debug("Checked type expression for LoadProfile: OK.");
		
		// clear previous values
		this.profiles.clear();
		
		// analyse the tuple values (each tuple is a RelationProfile)
		ListExpr tupleValue;
		
		while (!value.isEmpty()) 
		{
			tupleValue = value.first();
			
			String profileName = tupleValue.first().stringValue();
			String relName = tupleValue.second().stringValue();
			String filterValue = tupleValue.third().textValue();
			String projectValue = tupleValue.fourth().textValue();
			String sortValue = tupleValue.fifth().textValue();
			
			RelationProfile relProfile = new RelationProfile(relName);	
			relProfile.setFilterExpressions(this.parseSeparatedList(filterValue));
			relProfile.setProjectExpressions(this.parseSeparatedList(projectValue));
			relProfile.setSortExpressions(this.parseSeparatedList(sortValue));			
			
			// If there is not yet a LoadProfile of that name,
			// create one and add its Name to the selection list.
			LoadProfile lp = this.profiles.get(profileName);
			if (lp == null)
			{
				lp = new LoadProfile(profileName);
			}
			
			// add RelationProfile to its LoadProfile
			lp.addRelationProfile(relProfile);
			
			//Reporter.debug(profileName + " " + relName);
			
			this.profiles.put(lp.getName(), lp);
					
			value = value.rest();
		}
				
		this.loaded = true;
		return true;
	}
	
	/**
	 * Returns a list of the non-empty substrings in the specified
	 * semicolon-seprated list.
	 */
	private List<String> parseSeparatedList(String pSeparatedList)
	{
		List<String> result = new ArrayList<String>();
		String[] strings = pSeparatedList.split(";");
		
		for (String s : strings)
		{
			if (!s.isEmpty())
			{
				result.add(s);
			}
		}
		return result;
	}
	

	/**
	 *
	 */
	public LoadProfile getCurrentLoadProfile()
	{
		LoadProfile result = null;
		String profileName = (String)this.lsProfiles.getSelectedValue();
		if (profileName != null)
		{
			result = this.profiles.get(profileName);
		}
		return result;
	}
	
	/**
	 *
	 */
	public String getCurrentLoadProfileName()
	{
		return this.lsProfiles.getSelectedValue().toString();
	}
	
	/**
	 *
	 */
	public String getCurrentRelationProfileName()
	{
		return this.plLoadProfile.getCurrentRelationProfileName();
	}
	
	
	
	/**
	 * Returns true if LoadProfiles have been loaded from DB.
	 */
	public boolean isLoaded()
	{
		return this.loaded;
	}
	
	
	/**
	 * Shows profiles if load profiles are found in database
	 * else shows info message.
	 */
	public void showProfiles()
	{
		this.lmProfiles = new DefaultListModel();
		for (String profName : profiles.keySet())
		{
			this.lmProfiles.addElement(profName);
		}
		this.lsProfiles.setModel(lmProfiles);
		this.lsProfiles.setSelectedIndex(0);
		String profileName = (String)lsProfiles.getSelectedValue();
		this.plLoadProfile.showLoadProfile(this.profiles.get(profileName));
		
		this.repaint();
		this.validate();		
		
		if (this.profiles.isEmpty())
		{
			Reporter.showInfo("This database contains no load profiles for loading relations. "
							  + "Click NEW LOAD PROFILE to create one.");
		}
	}
	
	/**
	 * Method of Interface ListSelectionListener.
	 * Shows selected LoadProfile in the center panel.
	 */
	public void valueChanged(ListSelectionEvent e)
	{
		String profileName = (String)lsProfiles.getSelectedValue();
		this.plLoadProfile.showLoadProfile(this.profiles.get(profileName));
		
		this.repaint();
		this.validate();
	}	
}

