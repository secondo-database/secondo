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
import java.util.ListIterator;
import java.util.Properties;
import java.util.StringTokenizer;
import java.util.Vector;

import javax.swing.AbstractAction;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
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
	
	private JButton btRemoveRelation;
	
	private JList relations;
	
	// Dialog to show error messages
	private JDialog errorDialog;
	

	public LoadProfilePanel(UpdateViewerController pController) {
		
		controller = pController;
		
		setLayout(new BorderLayout());				
		plButtons = new JPanel();
		
		plButtons.setLayout(new GridLayout(1, 7));
		btAddRelation = new JButton("Add relation");
		btAddRelation.addActionListener(controller);
		plButtons.add(btAddRelation);
		btRemoveRelation = new JButton("Remove relation");
		btRemoveRelation.addActionListener(controller);
		plButtons.add(btRemoveRelation);
		
		add(plButtons, BorderLayout.NORTH);
	}
	
}

