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

package  viewer;

import gui.SecondoObject;
import gui.idmanager.ID;
import gui.idmanager.IDManager;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.GridLayout;
import java.awt.Image;

import java.util.ArrayList;
import java.util.List;

import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JTextArea;

import sj.lang.ListExpr;
import tools.Reporter;

import viewer.update2.*;
import viewer.update2.gui.*;


/**
 * This viewer allows editing of multiple relations.
 * It can be used to update, insert and delete tuples.
 * Tuple values are displayed sequentially in order to faciliate
 * viewing and editing of long text values. 
 */
public class UpdateViewer2 extends SecondoViewer {

	private String viewerName = "UpdateViewer2";
	
	private JPanel actionPanel;
			
	private JButton load;
	
	private JButton clear;
	
	private JButton insert;
	
	private JButton delete;
	
	private JButton update;
	
	private JButton reset;
	
	private JButton undo;
	
	private JButton commit;
	
	private JButton format;
	
	
	// Components to display the loaded relation set.
	private JTabbedPane tabbedPane;
	
	private List<RelationPanel> relationPanels;
		
	// the controller decides which action shall be taken next and listens to all buttons
	// for user-input
	private UpdateViewerController controller;
	
	private JTextArea formattedDocument;
		
	
	/*
	 * Constructor.	 
	 */
	public UpdateViewer2() 
	{
		this.controller = new UpdateViewerController(this);
				
		this.setLayout(new BorderLayout());	
		
		// actionpanel
		this.actionPanel = new JPanel();
		this.actionPanel.setLayout(new GridLayout(1, 9));
		this.load = new JButton("Load");
		this.load.addActionListener(this.controller);
		this.load.setToolTipText("Open Load Dialog");
		this.actionPanel.add(load);
		this.clear = new JButton("Clear");
		this.clear.addActionListener(this.controller);
		this.clear.setToolTipText("Remove loaded relations from viewer");
		this.actionPanel.add(clear);
		this.insert = new JButton("Insert");
		this.insert.addActionListener(this.controller);
		this.insert.setToolTipText("Change to Insert Mode");
		this.actionPanel.add(this.insert);
		this.delete = new JButton("Delete");
		this.delete.addActionListener(this.controller);
		this.delete.setToolTipText("Change to Delete Mode");
		this.actionPanel.add(this.delete);
		this.update = new JButton("Update");
		this.update.addActionListener(this.controller);
		this.update.setToolTipText("Change to Update Mode");
		this.actionPanel.add(this.update);
		this.reset = new JButton("Reset");
		this.reset.addActionListener(this.controller);
		this.reset.setToolTipText("Undo all uncommitted changes");
		this.actionPanel.add(this.reset);
		this.undo = new JButton("Undo");
		this.undo.addActionListener(this.controller);
		this.undo.setToolTipText("Undo last uncommited change (cell-wise)");
		this.actionPanel.add(this.undo);
		this.commit = new JButton("Commit");
		this.commit.addActionListener(this.controller); 
		this.commit.setToolTipText("Save changes to database");
		this.actionPanel.add(this.commit);
		this.format = new JButton("Format");
		this.format.addActionListener(this.controller); 
		this.format.setToolTipText("View as formatted document");
		this.actionPanel.add(this.format);		
		this.add(actionPanel, BorderLayout.NORTH);

		// tabbed pane
		this.relationPanels = new ArrayList<RelationPanel>();
		this.tabbedPane = new JTabbedPane(JTabbedPane.TOP, JTabbedPane.SCROLL_TAB_LAYOUT);
		this.add(tabbedPane, BorderLayout.CENTER);

		// formatted text will be invisible initially
		this.formattedDocument = new JTextArea();
		
		this.setSelectionMode(States.INITIAL);
	}
	

	/**
	 * Removes currently shown relation set from this viewer. 
	 * All information about these relations will be lost.	 	 
	 */
	public void clear() 
	{
		this.relationPanels.clear();
		this.tabbedPane.removeAll();
		
		this.validate();
		this.repaint();
	}

	
	/**
	 * Returns the currently active RelationPanel.
	 */
	public RelationPanel getCurrentRelationPanel()
	{
		RelationPanel result = null;
		int index = this.tabbedPane.getSelectedIndex();
		if (index >= 0 && index < this.relationPanels.size())
		{
			result = this.relationPanels.get(index);
		}
		return result;
	}
	

	/**
	 * Returns the RelationPanel with the Relation of the specified name.
	 */
	public RelationPanel getRelationPanel(String pRelName)
	{
		RelationPanel result = null;
		for (RelationPanel relpanel : this.relationPanels)
		{
			if (relpanel.getName().equals(pRelName))
			{
				return relpanel;
			}
		}
		return result;
	}
	
	/**
	 * Returns the RelationPanel at specified index (list index = tab index), null if index invalid.
	 */
	public RelationPanel getRelationPanel(int pIndex)
	{
		if (pIndex >= 0 && pIndex < this.relationPanels.size())
		{
			return this.relationPanels.get(pIndex);
		}
		return null;
	}
	
	/**
	 * Returns all RelationPanels (list order = tab order).
	 */
	public List<RelationPanel> getRelationPanels()
	{
		return this.relationPanels;
	}
	
	/**
	 * Sets RelationPanel at specified index as active tab in tabbed pane.
	 * if index is valid.
	 */
	public void showRelationPanel(int pIndex)
	{
		Reporter.debug("UpdateViewer2.showCurrentRelationPanel: setting index to " + pIndex);
		if (pIndex >= 0 && pIndex < this.relationPanels.size())
		{
			this.tabbedPane.setSelectedIndex(pIndex);
		}
		this.revalidate();
	}
	
	/**
	 * Creates or overwrites RelationPanel data with specified data.
	 */
	public boolean setRelationPanel(String pRelName, ListExpr pRelationLE)
	{
		RelationPanel rp = this.getRelationPanel(pRelName);
		if (rp == null)
		{
			rp = new RelationPanel(pRelName, this.controller);
			this.relationPanels.add(rp);
			
			String tabtitle = rp.getName();
			if (tabtitle.length() > 30)
			{
				tabtitle = tabtitle.substring(0,29) + "...";
			}

			this.tabbedPane.addTab(tabtitle, null, rp, rp.getName());
			this.tabbedPane.setTabComponentAt(this.relationPanels.indexOf(this.getRelationPanel(tabtitle)),
								   new ButtonTabComponent(this.tabbedPane));
		}
		return rp.createFromLE(pRelationLE);
	}
	


	/*
	 * For each mode and state the viewer is in only certain operations and choices are possible.
	 * This method assures only the actually allowed actions can be executed or chosen.	 
	 */
	public void setSelectionMode(int pState)
	{
		switch (pState) 
		{
			case States.INITIAL: 
			{
				insert.setBackground(Color.LIGHT_GRAY);
				delete.setBackground(Color.LIGHT_GRAY);
				update.setBackground(Color.LIGHT_GRAY);
				load.setEnabled(true);
				clear.setEnabled(false);
				insert.setEnabled(false);
				delete.setEnabled(false);
				update.setEnabled(false);
				reset.setEnabled(false);
				undo.setEnabled(false);
				commit.setEnabled(false);
				format.setEnabled(false);
				break;
			}
			case States.LOADED: 
			{
				insert.setBackground(Color.LIGHT_GRAY);
				delete.setBackground(Color.LIGHT_GRAY);
				update.setBackground(Color.LIGHT_GRAY);
				load.setEnabled(true);
				clear.setEnabled(true);
				insert.setEnabled(true);
				update.setEnabled(true);
				delete.setEnabled(true);
				reset.setEnabled(false);
				undo.setEnabled(false);
				commit.setEnabled(false);
				format.setEnabled(true);
				for (RelationPanel rp : this.relationPanels)
				{
					rp.setMode(pState);
				}
				break;
			}
			case States.INSERT: 
			{
				insert.setBackground(Color.YELLOW);
				load.setEnabled(false);
				clear.setEnabled(false);
				insert.setEnabled(false);
				update.setEnabled(false);
				delete.setEnabled(false);
				reset.setEnabled(true);
				undo.setEnabled(true);
				commit.setEnabled(true);
				format.setEnabled(true);
				this.getCurrentRelationPanel().setMode(pState);
				break;
			}
			case States.DELETE: 
			{
				delete.setBackground(Color.YELLOW);
				load.setEnabled(false);
				clear.setEnabled(false);
				insert.setEnabled(false);
				update.setEnabled(false);
				delete.setEnabled(false);
				reset.setEnabled(true);
				undo.setEnabled(true);
				commit.setEnabled(true);
				for (RelationPanel rp : this.relationPanels)
				{
					rp.setMode(pState);
				}
				break;
			}
			case States.UPDATE: 
			{
				update.setBackground(Color.YELLOW);
				load.setEnabled(false);
				clear.setEnabled(false);
				insert.setEnabled(false);
				update.setEnabled(false);
				delete.setEnabled(false);
				reset.setEnabled(true);
				undo.setEnabled(true);
				commit.setEnabled(true);
				format.setEnabled(true);
				for (RelationPanel rp : this.relationPanels)
				{
					rp.setMode(pState);
				}
				break;
			}
			case States.FORMAT: 
			{
				format.setBackground(Color.YELLOW);
				load.setEnabled(true);
				clear.setEnabled(true);
				insert.setEnabled(false);
				update.setEnabled(false);
				delete.setEnabled(false);
				reset.setEnabled(false);
				undo.setEnabled(false);
				commit.setEnabled(false);
				format.setEnabled(true);
				format.setText("Edit View");
				break;
			}
			default:
				break;
		}
	}
	
	
	/**
	 * Displays formatted document created from the laoded relations.
	 */
	public void showFormattedDocument()
	{
		this.remove(tabbedPane);
		this.add(formattedDocument, BorderLayout.CENTER);
		this.validate();
		this.repaint();	
	}
	
	/**
	 * Displays all loaded RelationPanels.
	 */
	public void showRelations()
	{
		/*
		for (RelationPanel rp : this.relationPanels)
		{
			String tabtitle = rp.getName();
			if (tabtitle.length() > 30)
			{
				tabtitle = tabtitle.substring(0,29) + "...";
			}
			tabbedPane.addTab(tabtitle, null, rp, rp.getName());
		}
		 */
		this.remove(formattedDocument);
		this.add(tabbedPane, BorderLayout.CENTER);
		
		this.validate();
		this.repaint();	
	}
	
	

	/*********************************************************
	 * Methods of SecondoViewer
	 *********************************************************/
	
	/**
	 * Method of SecondoViewer:
	 * Get the name of this viewer.
	 * The name is used in the menu of the MainWindow.
	 */
	@Override
	public String getName() 
	{
		return viewerName;
	}
	
	/**
	 * Method of SecondoViewer:
	 * Add new relation panel to display specified relation SecondoObject
	 * is SecondoObject is relation and not yet displayed.
	 */
	@Override
	public boolean addObject(SecondoObject so) 
	{
		if (this.canDisplay(so) && !this.isDisplayed(so))
		{
			this.setRelationPanel(so.getName(), so.toListExpr());
			this.showRelations();
			this.setSelectionMode(States.LOADED);
		}
		return false;
	}
	
	/**
	 * Method of SecondoViewer
	 */
	@Override
	public void removeObject(SecondoObject so) 
	{
		RelationPanel rp = getRelationPanel(so.getName());
		if (rp != null)
		{
			this.relationPanels.remove(rp);
		}
	}
	
	/*
	 * Method of SecondoViewer.
	 * Remove all displayed relations from viewer.
	 */
	@Override
	public void removeAll() 
	{
		this.clear();
	}
		
	/*
	 * Method of SecondoViewer:
	 * Returns true if specified SecondoObject is a relation.	 
	 */
	public boolean canDisplay(SecondoObject so)
	{
		ListExpr le = so.toListExpr();
		Reporter.debug("UpdateViewer2.canDisplay: full ListExpr is " + le.toString());
		
		if (le.listLength() >= 2 && !le.first().isAtom())
		{
			ListExpr type = le.first();
			Reporter.debug("UpdateViewer2.canDisplay: type ListExpr is " + type.toString());
			
			if (type.first().isAtom())
			{
				String objectType = type.first().symbolValue();
				if (objectType.equals("rel")) // || objectType.equals("trel") || objectType.equals("mrel"))
				{
					return true;
				}
			}
		}
		return false;
	}
	
	/*
	 Method of SecondoViewer:
	 Because this viewer shall not display objects others than relations loaded
	 by the viewer itself false is returned.	 
	 */
	@Override
	public boolean isDisplayed(SecondoObject so) 
	{
		RelationPanel rp = getRelationPanel(so.getName());
		if (rp != null)
		{
			return true;
		}
		return false;
	}
	
	/*
	 Method of SecondoViewer:
	 Because this viewer shall not display objects others than relations loaded
	 by the viewer itself false is returned.	 
	 */
	@Override
	public boolean selectObject(SecondoObject so) 
	{
		return false;
	}

	/*
	* Method of SecondoViewer:
	* Because all commands for this viewer can be accessed from the actionPanel
	* no MenuVector is built.	 
	*/
	@Override
	public MenuVector getMenuVector() 
	{
		return null;
	}
	
	/*
	 * Method of SecondoViewer:
	 * Because this viewer shall not display objects others than relations loaded
	 * by the viewer itself 0 is returned.	 
	 */
	@Override
	public double getDisplayQuality(SecondoObject so) 
	{
		if (this.canDisplay(so))
		{
			return 1;
		}
		return 0;
	}
 
}

