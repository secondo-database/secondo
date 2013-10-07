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
import java.awt.Component;
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
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.io.File;
import java.io.FileInputStream;
import java.util.List;
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
import javax.swing.JTable;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.KeyStroke;
import javax.swing.ListModel;
import javax.swing.SwingConstants;
import javax.swing.table.TableModel;
import javax.swing.table.DefaultTableModel;

import project.Projection;
import sj.lang.ListExpr;
import sj.lang.ServerErrorCodes;
import tools.Reporter;

import viewer.update2.*;


/**
 * Panel to display and edit Load Profiles for UpdateViewer2.	 
 */
public class LoadProfilePanel extends JPanel {
	
	private LoadProfile profile;
	
	private UpdateViewerController controller;
	
	private JPanel plButtons;
			
	private JButton btAddRelation;

	private JButton btEditRelation;
	
	private JButton btRemoveRelation;
	
	private JTable tbRelations;
	
	private RelationProfileTableModel mdlRelations;
	
	private JScrollPane scpRelations;
	
	
	/**
	 * Constructor.
	 */
	public LoadProfilePanel(UpdateViewerController pController) 
	{
		this.controller = pController;
		
		// buttons
		this.plButtons = new JPanel();
		this.plButtons.setLayout(new GridLayout(7, 1));
		this.btAddRelation = new JButton("Add relation");
		this.btAddRelation.addActionListener(controller);
		this.plButtons.add(btAddRelation);
		this.btEditRelation = new JButton("Edit relation");
		this.btEditRelation.addActionListener(controller);
		this.plButtons.add(btEditRelation);		
		this.btRemoveRelation = new JButton("Remove relation");
		this.btRemoveRelation.addActionListener(controller);
		this.plButtons.add(btRemoveRelation);
		
		// relation table
		this.tbRelations = new JTable();
		this.scpRelations = new JScrollPane(tbRelations);
		//this.tbRelations.setFillsViewportHeight(true);
		
		// add all components
		this.setLayout(new BorderLayout());				
		this.add(plButtons, BorderLayout.EAST);
		//this.add(tbRelations.getTableHeader(), BorderLayout.PAGE_START);
		this.add(scpRelations, BorderLayout.CENTER);
	}
	
	public String getCurrentRelationProfileName()
	{
		String result = null;
		int rowIndex = tbRelations.getSelectedRow();
		if (this.profile != null && rowIndex > -1)
		{
			result = (String)this.tbRelations.getValueAt(rowIndex, 0);
		}
		return result;
	}
	
	/**
	 * Display the LoadProfile
	 */
	public void showLoadProfile(LoadProfile pLoadProfile)
	{
		if (pLoadProfile != null)
		{
			this.profile = pLoadProfile;
			this.mdlRelations = new RelationProfileTableModel(pLoadProfile);
			this.tbRelations.setModel(mdlRelations);
			
			this.validate();
			this.repaint();
		}
	}
	
}

