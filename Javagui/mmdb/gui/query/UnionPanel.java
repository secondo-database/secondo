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

package mmdb.gui.query;

import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.DefaultListModel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.ListSelectionModel;
import javax.swing.ScrollPaneConstants;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

import mmdb.MMDBUserInterfaceController;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryRelation.RelationHeaderItem;
import mmdb.error.memory.MemoryException;
import mmdb.error.query.QueryException;
import mmdb.gui.QueryDialog;
import tools.Reporter;

/**
 * The panel representing the 'UNION' query.
 *
 * @author Alexander Castor
 */
public class UnionPanel extends AbstractOperationPanel {

	private static final long serialVersionUID = -6679479531695801670L;

	/**
	 * The first list of relations which can be united.
	 */
	private JList firstRelationList;

	/**
	 * The second list of relations which can be united.
	 */
	private JList secondRelationList;

	/**
	 * The list of attributes from the first relation.
	 */
	private JList firstAttributeList;

	/**
	 * The list of attributes from the second relation.
	 */
	private JList secondAttributeList;

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.gui.operation.AbstractOperationPanel#constructPanel()
	 */
	@Override
	public void constructPanel() {
		this.setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		addDescriptionPanel("MERGE THE TUPLES OF TWO GIVEN RELATIONS", this);
		createLists();
		JPanel topArea = new JPanel(new GridLayout(1, 2));
		topArea.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH, MIDDLE_HEIGHT / 2));
		addRelationPanel(firstRelationList, firstAttributeList, "1) FIRST RELATION", topArea);
		addRelationPanel(secondRelationList, secondAttributeList, "2) SECOND RELATION", topArea);
		this.add(topArea);
		JPanel bottomArea = new JPanel(new GridLayout(1, 2));
		bottomArea.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH, MIDDLE_HEIGHT / 2));
		addAttributePanel(firstAttributeList, "ATTRIBUTES OF FIRST RELATION", bottomArea);
		addAttributePanel(secondAttributeList, "ATTRIBUTES OF SECOND RELATION", bottomArea);
		this.add(bottomArea);
		addConvertToListPanel(this);
		addButtonPanel(this);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.gui.query.AbstractOperationPanel#createButtonListener()
	 */
	@Override
	protected ActionListener createButtonListener() {
		return new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent event) {
				String selectedFirstRelation = (String)firstRelationList.getSelectedValue();
				String selectedSecondRelation = (String)secondRelationList.getSelectedValue();
				if (selectedFirstRelation == null || selectedSecondRelation == null) {
					Reporter.showInfo("Please select both relations.");
					return;
				}
				if (!attributesIdentical()) {
					Reporter.showInfo("Attributes of selected relations do not match.");
					return;
				}
				MemoryRelation resultRelation = null;
				try {
					resultRelation = queryController.executeQuery(
							relations.get(selectedFirstRelation),
							relations.get(selectedSecondRelation));
				} catch (QueryException e) {
					Throwable unexpected = e.getUnexpectedError();
					if (unexpected != null) {
						unexpected.printStackTrace();
						Reporter.showError("Unexpected error occured during union operation.\n"
								+ "See console for details.");
					} else {
						Reporter.showWarning(e.getMessage());
					}
					return;
				} catch (MemoryException e) {
					dialog.dispose();
					MMDBUserInterfaceController.getInstance().processMemoryException(e);
				}
				answer[0] = resultRelation;
				answer[1] = createObjectName("UNION", selectedFirstRelation, selectedSecondRelation);
				answer[2] = convert.isSelected();
				dialog.setVisible(false);
				dialog.dispose();
			}

		};
	}

	/**
	 * Initializes the relation and attribute lists.
	 */
	private void createLists() {
		Vector<String> vector = new Vector<String>(relations.keySet());
		firstRelationList = new JList(vector);
		secondRelationList = new JList(vector);
		firstAttributeList = new JList(new DefaultListModel());
		secondAttributeList = new JList(new DefaultListModel());
	}

	/**
	 * Adds a relation selection panel to the underlying container.
	 * 
	 * @param attributeList
	 *            the list of attributes
	 * @param relationList
	 *            the list of relations
	 * @param title
	 *            the component's title
	 * @param area
	 *            the underlying container the components are added to
	 */
	private void addRelationPanel(final JList relationList, final JList attributeList,
			String title, JPanel area) {
		relationList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		relationList.addListSelectionListener(new ListSelectionListener() {
			@Override
			public void valueChanged(ListSelectionEvent e) {
				fillAttributeList((String)relationList.getSelectedValue(), attributeList);
			}
		});
		JScrollPane relationSelectionPane = new JScrollPane(relationList,
				ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
				ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
		relationSelectionPane.setBorder(BorderFactory.createTitledBorder(title));
		relationSelectionPane.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH / 2,
				MIDDLE_HEIGHT / 2));
		area.add(relationSelectionPane);
	}

	/**
	 * Adds an attribute display panel to the underlying container.
	 * 
	 * @param attributeList
	 *            the list of attributes
	 * @param title
	 *            the component's title
	 * @param area
	 *            the underlying container the components are added to
	 */
	private void addAttributePanel(JList attributeList, String title, JPanel area) {
		attributeList.setEnabled(false);
		JScrollPane attributeSelectionPane = new JScrollPane(attributeList,
				ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
				ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
		attributeSelectionPane.setBorder(BorderFactory.createTitledBorder(title));
		attributeSelectionPane.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH / 2,
				MIDDLE_HEIGHT / 2));
		area.add(attributeSelectionPane);
	}

	/**
	 * Dynamically fills a join attribute list depending on the user's relation
	 * selection.
	 * 
	 * @param selectedValue
	 *            the selected relation
	 * @param attributeList
	 *            the list to which the relation's attributes shall be added
	 */
	private void fillAttributeList(String selectedValue, JList attributeList) {
		MemoryRelation relation = relations.get(selectedValue);
		DefaultListModel model = (DefaultListModel) attributeList.getModel();
		model.removeAllElements();
		for (RelationHeaderItem item : relation.getHeader()) {
			if (!item.isProjected()) {
				continue;
			}
			model.addElement(item.getIdentifier() + " (" + item.getTypeName() + ")");
		}
	}

	/**
	 * Checks whether both attribute lists are identical.
	 * 
	 * @return true if both lists are identical, else false
	 */
	private boolean attributesIdentical() {
		DefaultListModel firstModel = (DefaultListModel) firstAttributeList
				.getModel();
		DefaultListModel secondModel = (DefaultListModel) secondAttributeList
				.getModel();
		if (firstModel == null || secondModel == null) {
			return false;
		}
		if (firstModel.size() != secondModel.size()) {
			return false;
		}
		for (int i = 0; i < firstModel.size(); i++) {
			if (!secondModel.contains(firstModel.getElementAt(i))
					|| !firstModel.contains(secondModel.getElementAt(i))) {
				return false;
			}
		}
		return true;
	}

}
