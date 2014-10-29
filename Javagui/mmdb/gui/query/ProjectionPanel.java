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
import java.util.List;
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
 * The panel representing the 'PROJECTION' query.
 *
 * @author Alexander Castor
 */
public class ProjectionPanel extends AbstractOperationPanel {

	private static final long serialVersionUID = -6679479531695801670L;

	/**
	 * The list of relations from which attributes can be selected.
	 */
	private JList<String> relationList;

	/**
	 * The list of attributes that can be selected.
	 */
	private JList<String> attributeList;

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.gui.operation.AbstractOperationPanel#constructPanel()
	 */
	@Override
	public void constructPanel() {
		this.setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		addDescriptionPanel("SELECT CERTAIN ATTRIBUTES FROM A GIVEN RELATION", this);
		JPanel selectionArea = new JPanel(new GridLayout(1, 2));
		selectionArea.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH, MIDDLE_HEIGHT));
		addRelationPanel(selectionArea);
		addAttributePanel(selectionArea);
		this.add(selectionArea);
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
				String selectedRelation = relationList.getSelectedValue();
				List<String> selectedAttributes = attributeList.getSelectedValuesList();
				if (selectedRelation == null || selectedAttributes.isEmpty()) {
					Reporter.showInfo("Please select a relation and attributes.");
					return;
				}
				MemoryRelation resultRelation = null;
				try {
					resultRelation = queryController.executeQuery(relations.get(selectedRelation),
							removeTypes(selectedAttributes));
				} catch (QueryException e) {
					Throwable unexpected = e.getUnexpectedError();
					if (unexpected != null) {
						unexpected.printStackTrace();
						Reporter.showError("Unexpected error occured during projection operation.\n"
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
				answer[1] = createObjectName("PROJECTION", selectedRelation);
				answer[2] = convert.isSelected();
				dialog.setVisible(false);
				dialog.dispose();
			}
		};
	}

	/**
	 * Adds the relation selection panel to the underlying container.
	 * 
	 * @param area
	 *            the underlying container the components are added to
	 */
	private void addRelationPanel(JPanel area) {
		Vector<String> vector = new Vector<String>(relations.keySet());
		relationList = new JList<String>(vector);
		relationList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		relationList.addListSelectionListener(new ListSelectionListener() {
			@Override
			public void valueChanged(ListSelectionEvent e) {
				fillAttributeList(relationList.getSelectedValue());
			}
		});
		JScrollPane relationSelectionPane = new JScrollPane(relationList,
				ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
				ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
		relationSelectionPane.setBorder(BorderFactory.createTitledBorder("1) RELATION"));
		relationSelectionPane.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH / 2,
				MIDDLE_HEIGHT));
		area.add(relationSelectionPane);
	}

	/**
	 * Adds the attribute selection panel to the underlying container.
	 * 
	 * @param area
	 *            the underlying container the components are added to
	 */
	private void addAttributePanel(JPanel area) {
		attributeList = new JList<String>(new DefaultListModel<String>());
		attributeList.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
		JScrollPane attributeSelectionPane = new JScrollPane(attributeList,
				ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
				ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
		attributeSelectionPane.setBorder(BorderFactory.createTitledBorder("2) ATTRIBUTES"));
		attributeSelectionPane.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH / 2,
				MIDDLE_HEIGHT));
		area.add(attributeSelectionPane);
	}

	/**
	 * Dynamically fills the attribute list depending on the user's relation
	 * selection.
	 * 
	 * @param selectedValue
	 *            the selected relation
	 */
	private void fillAttributeList(String selectedValue) {
		MemoryRelation relation = relations.get(selectedValue);
		DefaultListModel<String> model = (DefaultListModel<String>) attributeList.getModel();
		model.removeAllElements();
		for (RelationHeaderItem item : relation.getHeader()) {
			if (!item.isProjected()) {
				continue;
			}
			model.addElement(item.getIdentifier() + " (" + item.getTypeName() + ")");
		}
	}

}
