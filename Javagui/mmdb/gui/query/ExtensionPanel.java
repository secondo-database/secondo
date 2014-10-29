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
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.ListSelectionModel;
import javax.swing.ScrollPaneConstants;
import javax.swing.SwingConstants;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

import mmdb.MMDBUserInterfaceController;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryRelation.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.memory.MemoryException;
import mmdb.error.query.QueryException;
import mmdb.gui.QueryDialog;
import mmdb.operator.OperationController;
import mmdb.operator.OperationController.EOperator;
import tools.Reporter;

/**
 * The panel representing the 'EXTENSION' query.
 *
 * @author Alexander Castor
 */
public class ExtensionPanel extends AbstractOperationPanel {

	private static final long serialVersionUID = -6679479531695801670L;

	/**
	 * The list of relations from which attributes can be selected.
	 */
	private JList<String> relationList;

	/**
	 * The list of operations that can be selected.
	 */
	private JComboBox<EOperator> operatorList;

	/**
	 * The name of the new attribute.
	 */
	private JTextField attributeName;

	/**
	 * The operator's argument types.
	 */
	private List<JTextField> argumentTypes = new ArrayList<JTextField>();

	/**
	 * The operator's argument values.
	 */
	private List<JComboBox<String>> argumentValues = new ArrayList<JComboBox<String>>();

	/**
	 * The number of arguments.
	 */
	private int numberOfArguments = -1;

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.gui.operation.AbstractOperationPanel#constructPanel()
	 */
	@Override
	public void constructPanel() {
		this.setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		addDescriptionPanel("ADD AN ATTRIBUTE TO A RELATION GENERATED THROUGH AN OPERATOR", this);
		JPanel selectionArea = new JPanel(new GridLayout(1, 2));
		selectionArea.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH, MIDDLE_HEIGHT));
		addRelationPanel(selectionArea);
		JPanel rightArea = new JPanel(new GridLayout(2, 1));
		rightArea.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH / 2, MIDDLE_HEIGHT));
		JPanel rightTopArea = new JPanel(new GridLayout(2, 1));
		rightTopArea
				.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH / 2, MIDDLE_HEIGHT / 2));
		createNamePanel(rightTopArea);
		createOperatorPanel(rightTopArea);
		rightArea.add(rightTopArea);
		createArgumentPanel(rightArea);
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
				if (!selectionOk()) {
					Reporter.showInfo("Please select relation, operator, arguments and enter a valid identifier.");
					return;
				}
				MemoryRelation resultRelation = null;
				String selectedRelation = relationList.getSelectedValue();
				EOperator selectedOperator = (EOperator) operatorList.getSelectedItem();
				List<String> arguments = new ArrayList<String>();
				for (int i = 0; i < numberOfArguments; i++) {
					arguments.add((String) argumentValues.get(i).getSelectedItem());
				}
				try {
					resultRelation = queryController.executeQuery(relations.get(selectedRelation),
							selectedOperator, attributeName.getText(), arguments);
				} catch (QueryException e) {
					Throwable unexpected = e.getUnexpectedError();
					if (unexpected != null) {
						unexpected.printStackTrace();
						Reporter.showError("Unexpected error occured during extension operation.\n"
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
				answer[1] = createObjectName("EXTENDED BY", selectedRelation,
						selectedOperator.toString() + " [");
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
	 * Adds the attribute name panel to the underlying container.
	 * 
	 * @param area
	 *            the underlying container the components are added to
	 */
	private void createNamePanel(JPanel area) {
		JPanel attributePanel = new JPanel(new FlowLayout());
		attributePanel.setBorder(BorderFactory.createTitledBorder("2) IDENTIFIER"));
		attributeName = new JTextField();
		attributeName.setPreferredSize(new Dimension(200, 30));
		attributeName.setHorizontalAlignment(JTextField.CENTER);
		attributePanel.add(attributeName);
		attributePanel.setBackground(Color.WHITE);
		area.add(attributePanel);
	}

	/**
	 * Adds the operator panel to the underlying container.
	 * 
	 * @param area
	 *            the underlying container the components are added to
	 */
	private void createOperatorPanel(JPanel area) {
		JPanel operatorPanel = new JPanel(new FlowLayout());
		operatorPanel.setBorder(BorderFactory.createTitledBorder("3) OPERATOR"));
		operatorList = new JComboBox<EOperator>(EOperator.values());
		operatorList.setPreferredSize(new Dimension(200, 30));
		((JLabel) operatorList.getRenderer()).setHorizontalAlignment(SwingConstants.CENTER);
		operatorList.setSelectedItem(null);
		operatorList.addItemListener(new ItemListener() {
			@Override
			public void itemStateChanged(ItemEvent e) {
				adjustArgumentSelections((EOperator) operatorList.getSelectedItem());
			}
		});
		operatorPanel.add(operatorList);
		operatorPanel.setBackground(Color.WHITE);
		area.add(operatorPanel);
	}

	/**
	 * Adds the argument selection panel to the underlying container.
	 * 
	 * @param area
	 *            the underlying container the components are added to
	 */
	private void createArgumentPanel(JPanel area) {
		JPanel argumentPanel = new JPanel(new GridLayout(3, 1));
		argumentPanel.setBorder(BorderFactory.createTitledBorder("4) ARGUMENTS"));
		for (int i = 0; i < 3; i++) {
			JPanel argumentSubPanel = new JPanel(new FlowLayout());
			argumentSubPanel.setBackground(Color.WHITE);
			JTextField textField = new JTextField();
			textField.setEnabled(false);
			textField.setPreferredSize(new Dimension(100, 30));
			textField.setHorizontalAlignment(JTextField.CENTER);
			argumentSubPanel.add(textField);
			argumentTypes.add(textField);
			JComboBox<String> comboBox = new JComboBox<String>();
			comboBox.setPreferredSize(new Dimension(150, 30));
			((JLabel) comboBox.getRenderer()).setHorizontalAlignment(SwingConstants.CENTER);
			argumentSubPanel.add(comboBox);
			argumentValues.add(comboBox);
			argumentPanel.add(argumentSubPanel);
		}
		argumentPanel.setBackground(Color.WHITE);
		hideAllArgumentFields();
		area.add(argumentPanel);
	}

	private void adjustArgumentSelections(EOperator operator) {
		Class<?>[] signature = OperationController.getExtMethodSignature(operator);
		hideAllArgumentFields();
		clearArgumentTypes();
		int counter = 0;
		for (Class<?> argument : signature) {
			argumentTypes.get(counter).setVisible(true);
			argumentTypes.get(counter).setText(MemoryAttribute.getTypeName(argument));
			argumentValues.get(counter).setVisible(true);
			counter++;
		}
		numberOfArguments = counter;
		fillAttributeList();
	}

	/**
	 * Fills the attribute lists depending on the current relation and operator
	 * selection.
	 */
	private void fillAttributeList() {
		if (numberOfArguments < 1 || relationList.isSelectionEmpty()) {
			return;
		}
		clearArgumentValues();
		MemoryRelation relation = relations.get(relationList.getSelectedValue());
		for (int i = 0; i < numberOfArguments; i++) {
			DefaultComboBoxModel<String> model = (DefaultComboBoxModel<String>) argumentValues.get(
					i).getModel();
			String argumentType = argumentTypes.get(i).getText();
			for (RelationHeaderItem item : relation.getHeader()) {
				if (!item.isProjected()) {
					continue;
				}
				if (item.getTypeName().equals(argumentType)) {
					model.addElement(item.getIdentifier());
				}
			}
		}
	}

	/**
	 * Hides all argument fields.
	 */
	private void hideAllArgumentFields() {
		for (JTextField field : argumentTypes) {
			field.setVisible(false);
		}
		for (JComboBox<String> box : argumentValues) {
			box.setVisible(false);
		}
	}

	/**
	 * Clears all argument types.
	 */
	private void clearArgumentTypes() {
		for (JTextField field : argumentTypes) {
			field.setText("");
		}
	}

	/**
	 * Clears all argument values.
	 */
	private void clearArgumentValues() {
		for (JComboBox<String> box : argumentValues) {
			DefaultComboBoxModel<String> model = (DefaultComboBoxModel<String>) box.getModel();
			model.removeAllElements();
		}
	}

	private boolean selectionOk() {
		if (operatorList.getSelectedItem() == null || relationList.getSelectedValue() == null
				|| attributeName == null) {
			return false;
		}
		Pattern pattern = Pattern.compile("[a-z,A-Z]([a-z,A-Z]|[0-9]|_)*");
		Matcher matcher = pattern.matcher(attributeName.getText());
		if (!matcher.matches()) {
			return false;
		}
		MemoryRelation relation = relations.get(relationList.getSelectedValue());
		for (RelationHeaderItem item : relation.getHeader()) {
			if (item.getIdentifier().equals(attributeName.getText())) {
				return false;
			}
		}
		for (int i = 0; i < numberOfArguments; i++) {
			if (argumentValues.get(i).getSelectedItem() == null) {
				return false;
			}
		}
		return true;
	}
}
