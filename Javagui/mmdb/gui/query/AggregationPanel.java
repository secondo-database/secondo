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

import java.awt.Color;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.util.Vector;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.DefaultListModel;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.ListSelectionModel;
import javax.swing.ScrollPaneConstants;
import javax.swing.SwingConstants;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

import mmdb.MMDBUserInterfaceController;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryRelation.RelationHeaderItem;
import mmdb.error.memory.MemoryException;
import mmdb.error.query.QueryException;
import mmdb.gui.QueryDialog;
import mmdb.operator.OperationController;
import mmdb.operator.OperationController.AOperator;
import tools.Reporter;

/**
 * The panel representing the 'AGGREGATION' query.
 *
 * @author Alexander Castor
 */
public class AggregationPanel extends AbstractOperationPanel {

	private static final long serialVersionUID = 5679449931695801670L;

	/**
	 * The list of relations from which attributes can be selected.
	 */
	private JList relationList;

	/**
	 * The list of attributes that can be selected.
	 */
	private JList attributeList;

	/**
	 * The list of operators that can be selected.
	 */
	private JComboBox operatorList;

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.gui.operation.AbstractOperationPanel#constructPanel()
	 */
	@Override
	public void constructPanel() {
		this.setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		addDescriptionPanel("CALCULATE AGGREGATIONS OF A CERTAIN ATTRIBUTE FOR ALL TUPLES", this);
		JPanel selectionArea = new JPanel(new GridLayout(1, 2));
		selectionArea.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH, MIDDLE_HEIGHT));
		addRelationPanel(selectionArea);
		JPanel rightArea = new JPanel(new GridLayout(2, 1));
		addOperatorPanel(rightArea);
		addAttributePanel(rightArea);
		selectionArea.add(rightArea);
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
				if (operatorList.getSelectedItem() == null
						|| attributeList.getSelectedValue() == null
						|| relationList.getSelectedValue() == null) {
					Reporter.showInfo("Please select relation, operator and attribute.");
					return;
				}
				MemoryRelation resultRelation = null;
				String selectedRelation = (String)relationList.getSelectedValue();
				AOperator selectedOperator = (AOperator) operatorList.getSelectedItem();
				String selectedAttribute = (String)attributeList.getSelectedValue();
				try {
					resultRelation = queryController.executeQuery(relations.get(selectedRelation),
							selectedOperator, removeType(selectedAttribute));
				} catch (QueryException e) {
					Throwable unexpected = e.getUnexpectedError();
					if (unexpected != null) {
						unexpected.printStackTrace();
						Reporter.showError("Unexpected error occured during aggregation operation.\n"
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
				answer[1] = createObjectName("AGGREGATION", selectedRelation);
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
		relationSelectionPane.setBorder(BorderFactory.createTitledBorder("1) RELATION"));
		relationSelectionPane.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH / 2,
				MIDDLE_HEIGHT));
		area.add(relationSelectionPane);
	}

	/**
	 * Adds the operator panel to the underlying container.
	 * 
	 * @param area
	 *            the underlying container the components are added to
	 */
	private void addOperatorPanel(JPanel area) {
		JPanel operationPanel = new JPanel(new GridLayout(3, 1));
		JPanel dummyPanel1 = new JPanel();
		dummyPanel1.setBackground(Color.WHITE);
		operationPanel.add(dummyPanel1);
		operationPanel.setBorder(BorderFactory.createTitledBorder("2) OPERATOR"));
		operatorList = new JComboBox(AOperator.values());
		((JLabel) operatorList.getRenderer()).setHorizontalAlignment(SwingConstants.CENTER);
		operatorList.setSelectedItem(null);
		operatorList.setPreferredSize(new Dimension(200, 30));
		operatorList.addItemListener(new ItemListener() {
			@Override
			public void itemStateChanged(ItemEvent e) {
				fillAttributeList();
			}
		});
		JPanel operatorPanel = new JPanel(new FlowLayout());
		operatorPanel.setBackground(Color.WHITE);
		operatorPanel.add(operatorList);
		operationPanel.add(operatorPanel);
		JPanel dummyPanel2 = new JPanel();
		dummyPanel2.setBackground(Color.WHITE);
		operationPanel.add(dummyPanel2);
		operationPanel.setBackground(Color.WHITE);
		area.add(operationPanel);
	}

	/**
	 * Adds the attribute selection panel to the underlying container.
	 * 
	 * @param area
	 *            the underlying container the components are added to
	 */
	private void addAttributePanel(JPanel area) {
		attributeList = new JList(new DefaultListModel());
		attributeList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		JScrollPane attributeSelectionPane = new JScrollPane(attributeList,
				ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
				ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
		attributeSelectionPane.setBorder(BorderFactory.createTitledBorder("3) ATTRIBUTE"));
		area.add(attributeSelectionPane);
	}

	/**
	 * Dynamically fills the attribute list depending on the user's relation and
	 * operator selection.
	 */
	private void fillAttributeList() {
		DefaultListModel model = (DefaultListModel) attributeList.getModel();
		model.removeAllElements();
		String selectedRelation = (String)relationList.getSelectedValue();
		AOperator selectedOperator = (AOperator) operatorList.getSelectedItem();
		if (selectedRelation == null || selectedOperator == null) {
			return;
		}
		MemoryRelation relation = relations.get(selectedRelation);
		Class<?> parameterClass = OperationController.getAggMethodParameter(selectedOperator);
		for (RelationHeaderItem item : relation.getHeader()) {
			if (!item.isProjected()) {
				continue;
			}
			if (parameterClass.isAssignableFrom(item.getType())) {
				model.addElement(item.getIdentifier() + " (" + item.getTypeName() + ")");
			}
		}
	}

}
