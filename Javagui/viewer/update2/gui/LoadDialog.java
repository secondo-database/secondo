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
import java.util.HashMap;
import java.util.ListIterator;
import java.util.Map;
import java.util.Properties;
import java.util.StringTokenizer;
import java.util.Vector;

import javax.swing.AbstractAction;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
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
import javax.swing.SwingConstants;

import project.Projection;
import sj.lang.ListExpr;
import sj.lang.ServerErrorCodes;
import tools.Reporter;

import viewer.update2.*;

/**
 * Dialog to pick relations and specify restrictions.	 
 */
public class LoadDialog extends JDialog
{

	// LoadProfiles by their names
	private Map<String, LoadProfile> profiles;
	
	private JList lsProfiles;
	
	// Controller
	private UpdateViewerController controller;
	
	// Buttons
	private JPanel plButtons;
		
	private JButton btLoad;
	
	private JButton btNewProfile;
	
	private JButton btRemoveProfile;
	
	private JButton btSaveProfile;
	
	private JButton btCancel;

		
	// Display/Editor area for current Load Profile
	private LoadProfilePanel plLoadProfile;
	
	// have LoadProfiles been loaded (or at least tried) from DB? 
	private boolean loaded;
	

	/** Constructor */
	public LoadDialog(UpdateViewerController pController) 
	{
		this.loaded = false;
						
		this.profiles = new HashMap<String, LoadProfile>();		

		this.controller = pController;
		
		this.setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
		this.setModal(true);
		this.setSize (600,400);
		
		this.getContentPane().setLayout(new BorderLayout());
		this.plButtons = new JPanel();
		
		this.plButtons.setLayout(new GridLayout(1, 7));
		this.btLoad = new JButton("Load");
		this.btLoad.addActionListener(controller);
		this.plButtons.add(this.btLoad);
		this.btNewProfile = new JButton("New load profile");
		this.btNewProfile.addActionListener(controller);
		this.plButtons.add(this.btNewProfile);
		this.btRemoveProfile = new JButton("Remove load profile");
		this.btRemoveProfile.addActionListener(controller);
		this.plButtons.add(this.btRemoveProfile);
		this.btSaveProfile = new JButton("Save load profile");
		this.btSaveProfile.addActionListener(controller);
		this.plButtons.add(this.btSaveProfile);
		this.btCancel = new JButton("Cancel");
		this.btCancel.addActionListener(controller);
		this.plButtons.add(this.btCancel);
		
		this.lsProfiles = new JList();
		
		this.plLoadProfile = new LoadProfilePanel(this.controller);
		
		this.getContentPane().add(this.lsProfiles, BorderLayout.NORTH);
		this.getContentPane().add(this.plButtons, BorderLayout.SOUTH);
		this.getContentPane().add(this.plLoadProfile, BorderLayout.CENTER);
		
		
		this.addComponentListener(this.controller);
	}
	
	
	/**
	 * Builds LoadProfiles from relation of following type expression:
	 * rel(tuple([ProfileName: string, RelName: string, FilterExpr: text, ProjectExpr: text, SortExpr: text]))
	 */
	public boolean createLoadProfilesFromLE(ListExpr LE)
	{
		boolean result = true;
		
		// check validity of type expression
		if (LE.listLength() != 2)
		{
			result = false;
		}
		else 
		{
			ListExpr type = LE.first();
			ListExpr value = LE.second();
			// check for relation type
			if (type.isAtom())
				result = false;
			ListExpr maintype = type.first();
			if (type.listLength() != 2
				|| !maintype.isAtom()
				|| maintype.atomType() != ListExpr.SYMBOL_ATOM
				|| !(maintype.symbolValue().equals("rel") | maintype
					 .symbolValue().equals("mrel")))
				result = false; // not a relation
			ListExpr tupletype = type.second();
			// check for tuple type
			ListExpr TupleFirst = tupletype.first();
			if (tupletype.listLength() != 2
				|| !TupleFirst.isAtom()
				|| TupleFirst.atomType() != ListExpr.SYMBOL_ATOM
				|| !(TupleFirst.symbolValue().equals("tuple") | TupleFirst
					 .symbolValue().equals("mtuple")))
				result = false; // not a tuple
			ListExpr attributes = tupletype.second();
			// check attributes	
			if (attributes.listLength() != 10){
				result = false;
			}
			else {
				ListExpr attrName = attributes.first();
				ListExpr attrType = attributes.second();
				if (!attrName.isAtom()
					|| attrName.atomType() != ListExpr.SYMBOL_ATOM
					|| !attrName.symbolValue().equals("ProfileName") 
					|| !attrType.isAtom()
					|| attrType.atomType() != ListExpr.SYMBOL_ATOM
					|| !attrType.symbolValue().equals("string") ) {
					result = false;
				}
				else{
					attrName = attributes.third();
					attrType = attributes.fourth();
					if (!attrName.isAtom()
						|| attrName.atomType() != ListExpr.SYMBOL_ATOM
						|| !attrName.symbolValue().equals("RelName") 
						|| !attrType.isAtom()
						|| attrType.atomType() != ListExpr.SYMBOL_ATOM
						|| !attrType.symbolValue().equals("string") ) {
						result = false;
					}
					else{
						attrName = attributes.fifth();
						attrType = attributes.sixth();
						if (!attrName.isAtom()
							|| attrName.atomType() != ListExpr.SYMBOL_ATOM
							|| !attrName.symbolValue().equals("FilterExpr") 
							|| !attrType.isAtom()
							|| attrType.atomType() != ListExpr.SYMBOL_ATOM
							|| !attrType.symbolValue().equals("text") ) {
							result = false;
						}
						else{
							attrName = attributes.seventh();
							attrType = attributes.eighth();
							if (!attrName.isAtom()
								|| attrName.atomType() != ListExpr.SYMBOL_ATOM
								|| !attrName.symbolValue().equals("ProjectExpr") 
								|| !attrType.isAtom()
								|| attrType.atomType() != ListExpr.SYMBOL_ATOM
								|| !attrType.symbolValue().equals("text") ) {
								result = false;
							}
							else{
								attrName = attributes.nineth();
								attrType = attributes.tenth();
								if (!attrName.isAtom()
									|| attrName.atomType() != ListExpr.SYMBOL_ATOM
									|| !attrName.symbolValue().equals("SortExpr") 
									|| !attrType.isAtom()
									|| attrType.atomType() != ListExpr.SYMBOL_ATOM
									|| !attrType.symbolValue().equals("text") ) {
									result = false;
								}
							}
						}
					}
				}
			}
		
			if (result) 
			{
				ListExpr tupleValue;
				
				// analyse the tuple values (each tuple is a RelationProfile)
				while (!value.isEmpty()) 
				{
					tupleValue = value.first();
					
					String profileName = tupleValue.first().stringValue();
					String relName = tupleValue.second().stringValue();
					String filterExpressions = tupleValue.third().textValue();
					String projectExpressions = tupleValue.fourth().textValue();
					String sortExpressions = tupleValue.fifth().textValue();
					
					RelationProfile relProfile = new RelationProfile(relName);
					// TODO set restrictions
					
					// add RelationProfile to its LoadProfile
					LoadProfile lp = this.profiles.get(profileName);
					if (lp == null)
					{
						lp = new LoadProfile(profileName);
					}
					lp.addRelationProfile(relProfile);
					
					this.profiles.put(lp.getName(), lp);
					
					value = value.rest();
				}
			}
		}
		
		this.loaded = result;
		return result;
	}
	
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
	 * Gets LoadProfiles from database (relation "uv2loadprofiles"), if any available.
	 */
	public void initLoadProfiles()
	{
		// TODO
		this.loaded = true;
	}
	
	/**
	 * Returns true if LoadProfiles have been loaded from DB (or at least tried).
	 */
	public boolean isLoaded()
	{
		return this.loaded;
	}
	
	public void showNoProfilesMessage(){
		JTextArea message = new JTextArea("This database contains no load profiles for loading relations. Click NEW LOAD PROFILE to create one.");
		this.plLoadProfile.add(message, BorderLayout.CENTER);
		this.repaint();
		this.validate();
	}
	
}

