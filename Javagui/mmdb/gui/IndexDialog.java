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
import java.awt.FlowLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Map;
import java.util.Vector;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.DefaultComboBoxModel;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.ListSelectionModel;
import javax.swing.ScrollPaneConstants;
import javax.swing.SwingConstants;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryRelation.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.features.Matchable;
import mmdb.data.features.Orderable;
import mmdb.data.indices.MemoryIndex.IndexType;
import mmdb.data.indices.sorted.SortedIndex;
import tools.Reporter;

/**
 * This class represents the index dialog that opens if a user clicks on "MMDB
 * -> Create Index" menu item.
 *
 * @author Alexander Castor
 */
public final class IndexDialog extends JDialog {

	private static final long serialVersionUID = 8967699790432817936L;

	/**
	 * The currently available memory relations.
	 */
	private Map<String, MemoryRelation> relations;

	/**
	 * The list of attributes that can be used for indexing.
	 */
	private JList attributeList;

	/**
	 * The list of relations that can be indexed.
	 */
	private JList relationList;

	/**
	 * The type of index that shall be created.
	 */
	private JComboBox indexSelection;

	/**
	 * The dialog's answer for further processing. ([0] = selected relation, [1]
	 * = selected attribute, [2] = selected index type)
	 */
	private String[] answer;

	/**
	 * Displays the dialog and returns the user input to the caller.
	 * 
	 * @param relations
	 *            the currently available memory relations
	 * @param component
	 *            the parent component for locating the window
	 * @return the user's input
	 */
	public static String[] showDialog(Map<String, MemoryRelation> relations, Component component) {
		IndexDialog dialog = new IndexDialog(relations);
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
	private IndexDialog(Map<String, MemoryRelation> relations) {
		this.relations = relations;
		this.answer = new String[3];
		setName("GENERATE INDEX");
		setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		Container container = this.getContentPane();
		container.setLayout(new BoxLayout(container, BoxLayout.Y_AXIS));
		addRelationPanel(container);
		addAttributePanel(container);
		addControlPanel(container);
	}

	/**
	 * Adds the relation selection area to the dialog.
	 * 
	 * @param container
	 *            the dialog's content pane
	 */
	private void addRelationPanel(Container container) {
		Vector<String> vector = new Vector<String>(relations.keySet());
		relationList = new JList(vector);
		relationList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		relationList.addListSelectionListener(new ListSelectionListener() {
			@Override
			public void valueChanged(ListSelectionEvent e) {
				fillAttributeList();
			}
		});
		JScrollPane relationSelectionPane = new JScrollPane(relationList,
				ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
				ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
		relationSelectionPane.setBorder(BorderFactory.createTitledBorder("SELECT RELATION"));
		relationSelectionPane.setPreferredSize(new Dimension(400, 150));
		relationSelectionPane.setOpaque(true);
		container.add(relationSelectionPane);
	}

	/**
	 * Adds the attribute selection area to the dialog.
	 * 
	 * @param container
	 *            the dialog's content pane
	 */
	private void addAttributePanel(Container container) {
		attributeList = new JList(new DefaultListModel());
		attributeList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		attributeList.addListSelectionListener(new ListSelectionListener() {
			@Override
			public void valueChanged(ListSelectionEvent e) {
				fillIndexSelectionList();
			}
		});
		JScrollPane attributeSelectionPane = new JScrollPane(attributeList,
				ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
				ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
		attributeSelectionPane.setBorder(BorderFactory.createTitledBorder("SELECT ATTRIBUTE"));
		attributeSelectionPane.setPreferredSize(new Dimension(400, 150));
		attributeSelectionPane.setOpaque(true);
		container.add(attributeSelectionPane);
	}

	/**
	 * Adds the control selection to the dialog including the combo box for
	 * selecting an index type and the confirmation button.
	 * 
	 * @param container
	 *            the dialog's content pane
	 */
	private void addControlPanel(Container container) {
		JPanel controlPanel = new JPanel(new GridLayout(2, 1));
		JButton button = new JButton("CREATE");
		button.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				String selectedRelation = (String)relationList.getSelectedValue();
				String selectedAttribute = (String)attributeList.getSelectedValue();
				if (selectedRelation == null || selectedAttribute == null) {
					Reporter.showInfo("Please select a relation and an attribute.");
				} else {
					answer[0] = selectedRelation;
					answer[1] = selectedAttribute.substring(0, selectedAttribute.indexOf("(") - 1);
					answer[2] = indexSelection.getSelectedItem().toString();
					setVisible(false);
					dispose();
				}
			}

		});
		JPanel selectionPanel = new JPanel(new FlowLayout());
		JLabel label = new JLabel("Index Type: ");
		indexSelection = new JComboBox();
		((JLabel) indexSelection.getRenderer()).setHorizontalAlignment(SwingConstants.CENTER);
		indexSelection.setPreferredSize(new Dimension(150, 30));
		selectionPanel.add(label);
		selectionPanel.add(indexSelection);
		controlPanel.add(selectionPanel);
		controlPanel.add(button);
		controlPanel.setOpaque(true);
		container.add(controlPanel);
	}

	/**
	 * Dynamically fills the attribute list depending on the user's relation
	 * selection.
	 * 
	 * @param selectedValue
	 *            the selected relation
	 */
	private void fillAttributeList() {
		DefaultListModel model = (DefaultListModel) attributeList.getModel();
		model.removeAllElements();
		String selectedValue = (String)relationList.getSelectedValue();
		if (selectedValue == null) {
			return;
		}
		MemoryRelation relation = relations.get(selectedValue);
		for (RelationHeaderItem item : relation.getHeader()) {
			if (!item.isProjected()) {
				continue;
			}
			if (Matchable.class.isAssignableFrom(item.getType())
					&& relation.getIndex(item.getIdentifier()) == null) {
				model.addElement(item.getIdentifier() + " (" + item.getTypeName() + ")");
			}
		}
	}

	private void fillIndexSelectionList() {
		DefaultComboBoxModel model = (DefaultComboBoxModel) indexSelection
				.getModel();
		model.removeAllElements();
		String selectedAttribute = (String)attributeList.getSelectedValue();
		if (selectedAttribute == null) {
			return;
		}
		String attributeType = selectedAttribute.substring(selectedAttribute.indexOf("(") + 1,
				selectedAttribute.length() - 1);
		Class<? extends MemoryAttribute> attributeClass = MemoryAttribute
				.getTypeClass(attributeType);
		for (IndexType indexType : IndexType.values()) {
			if (!Orderable.class.isAssignableFrom(attributeClass)
					&& SortedIndex.class.isAssignableFrom(indexType.indexClass)) {
				continue;
			}
			model.addElement(indexType);
		}
	}
}
