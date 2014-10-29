//This file is part of SECONDO.

//Copyright (C) 2014, University in Hagen, Department of Computer Science,
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

package mmdb.gui;

import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.util.Map;

import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JTabbedPane;

import mmdb.data.MemoryRelation;
import mmdb.gui.query.AbstractOperationPanel;
import mmdb.gui.query.AbstractOperationPanel.QueryPanel;
import mmdb.query.AbstractQueryController;
import tools.Reporter;

/**
 * This class represents the query dialog that opens if a user clicks on "MMDB
 * -> Execute Query" menu item.
 *
 * @author Alexander Castor
 */
public class QueryDialog extends JDialog {

	private static final long serialVersionUID = -8187982735534460524L;

	/**
	 * The dialog's width
	 */
	public static final int DIALOG_WIDTH = 650;

	/**
	 * The dialog's height
	 */
	public static final int DIALOG_HEIGHT = 450;

	/**
	 * The dialog's answer for further processing. ([0] = result relation, [1] =
	 * object name, [2] = convert result to nested list).
	 */
	private Object[] answer;

	/**
	 * Displays the dialog and returns the user input and result relation to the
	 * caller.
	 * 
	 * @param relations
	 *            the currently available memory relations
	 * @param component
	 *            the parent component for locating the window
	 * @return the user's input and the result relation
	 */
	public static Object[] showDialog(Map<String, MemoryRelation> relations, Component component) {
		QueryDialog dialog = new QueryDialog(relations);
		dialog.setLocationRelativeTo(component);
		dialog.setModal(true);
		dialog.pack();
		dialog.setVisible(true);
		return dialog.answer;
	}

	/**
	 * Assembles the dialog's components and initializes members.
	 * 
	 * @param relations
	 *            the currently available memory relations
	 */
	public QueryDialog(Map<String, MemoryRelation> relations) {
		this.answer = new Object[3];
		setName("EXECUTE QUERY");
		setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		Container container = this.getContentPane();
		container.setLayout(new GridLayout(1, 1));
		JTabbedPane tabbedPane = new JTabbedPane();
		addPanelsToPane(tabbedPane, relations);
		tabbedPane.setPreferredSize(new Dimension(DIALOG_WIDTH, DIALOG_HEIGHT));
		container.add(tabbedPane);
	}

	/**
	 * Instantiates all panels and adds them to the tabbed pane.
	 * 
	 * @param tabbedPane
	 *            the tabbed pane the panels are added to
	 */
	private void addPanelsToPane(JTabbedPane tabbedPane, Map<String, MemoryRelation> relations) {
		for (int i = 1; i <= QueryPanel.values().length; i++) {
			for (QueryPanel entry : QueryPanel.values()) {
				if (entry.position == i) {
					try {
						AbstractOperationPanel panel = entry.panelClass.newInstance();
						AbstractQueryController controller = entry.controllerClass.newInstance();
						panel.injectMembers(answer, relations, this, controller);
						panel.constructPanel();
						tabbedPane.addTab(entry.name(), panel);
					} catch (Throwable e) {
						Reporter.showWarning("Could not instantiate operation panel '"
								+ entry.toString() + "'.");
					}
					break;
				}
			}
		}
	}

}
