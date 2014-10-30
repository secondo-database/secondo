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

import gui.CommandPanel;

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

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JOptionPane;
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
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.features.Parsable;
import mmdb.error.memory.MemoryException;
import mmdb.error.query.QueryException;
import mmdb.gui.QueryDialog;
import mmdb.operator.OperationController;
import mmdb.operator.OperationController.COperator;
import sj.lang.ListExpr;
import tools.Reporter;

/**
 * The panel representing the 'SELECTION' query.
 *
 * @author Alexander Castor
 */
public class SelectionPanel extends AbstractOperationPanel {

	private static final long serialVersionUID = -6679479531695801670L;

	/**
	 * The list of relations from which attributes can be selected.
	 */
	private JList relationList;

	/**
	 * The list of operations that can be selected.
	 */
	private JComboBox operatorList;

	/**
	 * The list of attributes that can be selected.
	 */
	private JComboBox attributeList;

	/**
	 * The button which opens an input dialog for loading an attribute from
	 * text.
	 */
	private JButton fromTextButton;

	/**
	 * The button which opens an input dialog for loading an attribute from
	 * database.
	 */
	private JButton fromObjectButton;

	/**
	 * The selected value in text representation.
	 */
	private JTextField valueField;;

	/**
	 * The selected value type.
	 */
	private JComboBox valueTypeSelection;

	/**
	 * The selected value as object.
	 */
	private MemoryAttribute selectedValue;

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.gui.operation.AbstractOperationPanel#constructPanel()
	 */
	@Override
	public void constructPanel() {
		this.setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		addDescriptionPanel("SELECT CERTAIN TUPLES FROM A GIVEN RELATION DEPENDING ON A CONDITION",
				this);
		JPanel selectionArea = new JPanel(new GridLayout(1, 2));
		selectionArea.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH, MIDDLE_HEIGHT));
		addRelationPanel(selectionArea);
		JPanel rightArea = new JPanel(new GridLayout(2, 1));
		rightArea.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH / 2, MIDDLE_HEIGHT));
		JPanel rightTopArea = new JPanel(new GridLayout(2, 1));
		rightTopArea
				.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH / 2, MIDDLE_HEIGHT / 2));
		createOperatorPanel(rightTopArea);
		createAttributePanel(rightTopArea);
		rightArea.add(rightTopArea);
		createValuePanel(rightArea);
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
						|| attributeList.getSelectedItem() == null
						|| relationList.getSelectedValue() == null || selectedValue == null) {
					Reporter.showInfo("Please select relation, operator, attribute and value.");
					return;
				}
				MemoryRelation resultRelation = null;
				String selectedRelation = (String)relationList.getSelectedValue();
				COperator selectedOperator = (COperator) operatorList.getSelectedItem();
				String selectedAttribute = (String) attributeList.getSelectedItem();
				try {
					resultRelation = queryController.executeQuery(relations.get(selectedRelation),
							selectedOperator, removeType(selectedAttribute), selectedValue);
				} catch (QueryException e) {
					Throwable unexpected = e.getUnexpectedError();
					if (unexpected != null) {
						unexpected.printStackTrace();
						Reporter.showError("Unexpected error occured during selection operation.\n"
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
				answer[1] = createObjectName("SELECTION", selectedRelation);
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
				fillCondAttributeList((String)relationList.getSelectedValue(),
						(COperator) operatorList.getSelectedItem(), attributeList);
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
	private void createOperatorPanel(JPanel area) {
		JPanel operationPanel = new JPanel(new FlowLayout());
		operationPanel.setBorder(BorderFactory.createTitledBorder("2) OPERATOR"));
		operatorList = new JComboBox(COperator.values());
		((JLabel) operatorList.getRenderer()).setHorizontalAlignment(SwingConstants.CENTER);
		operatorList.setSelectedItem(null);
		operatorList.setPreferredSize(new Dimension(200, 30));
		operatorList.addItemListener(new ItemListener() {
			@Override
			public void itemStateChanged(ItemEvent e) {
				fillCondAttributeList((String)relationList.getSelectedValue(),
						(COperator) operatorList.getSelectedItem(), attributeList);
			}
		});
		operationPanel.add(operatorList);
		operationPanel.setBackground(Color.WHITE);
		area.add(operationPanel);
	}

	/**
	 * Adds the attribute panel to the underlying container.
	 * 
	 * @param area
	 *            the underlying container the components are added to
	 */
	private void createAttributePanel(JPanel area) {
		JPanel attributePanel = new JPanel(new FlowLayout());
		attributePanel.setBorder(BorderFactory.createTitledBorder("3) ATTRIBUTE"));
		attributeList = new JComboBox(new DefaultComboBoxModel());
		((JLabel) attributeList.getRenderer()).setHorizontalAlignment(SwingConstants.CENTER);
		attributeList.setPreferredSize(new Dimension(200, 30));
		attributeList.addItemListener(new ItemListener() {
			@Override
			public void itemStateChanged(ItemEvent e) {
				fillValueTypeSelectionList((String) attributeList.getSelectedItem(),
						(COperator) operatorList.getSelectedItem());
			}
		});
		attributePanel.add(attributeList);
		attributePanel.setBackground(Color.WHITE);
		area.add(attributePanel);
	}

	/**
	 * Adds the value panel to the underlying container.
	 * 
	 * @param area
	 *            the underlying container the components are added to
	 */
	private void createValuePanel(JPanel area) {
		JPanel valuePanel = new JPanel(new GridLayout(3, 1));
		valuePanel.setBorder(BorderFactory.createTitledBorder("4) VALUE"));
		JPanel valueTypePanel = new JPanel(new FlowLayout());
		valueTypeSelection = new JComboBox(new DefaultComboBoxModel());
		((JLabel) valueTypeSelection.getRenderer()).setHorizontalAlignment(SwingConstants.CENTER);
		valueTypeSelection.setPreferredSize(new Dimension(200, 30));
		addValueTypeSelectionListener();
		valueTypePanel.setBackground(Color.WHITE);
		valueTypePanel.add(valueTypeSelection);
		valuePanel.add(valueTypePanel);
		JPanel buttonArea = new JPanel(new FlowLayout());
		fromTextButton = new JButton("FROM TEXT");
		fromObjectButton = new JButton("FROM OBJECT");
		fromTextButton.setPreferredSize(new Dimension(120, 30));
		fromObjectButton.setPreferredSize(new Dimension(120, 30));
		fromTextButton.setEnabled(false);
		fromObjectButton.setEnabled(false);
		addFromTextListener();
		addFromObjectListener();
		buttonArea.add(fromTextButton);
		buttonArea.add(fromObjectButton);
		buttonArea.setBackground(Color.WHITE);
		valuePanel.add(buttonArea);
		JPanel displayPanel = new JPanel(new FlowLayout());
		valueField = new JTextField();
		valueField.setEnabled(false);
		valueField.setPreferredSize(new Dimension(200, 30));
		valueField.setHorizontalAlignment(JTextField.CENTER);
		displayPanel.add(valueField);
		displayPanel.setBackground(Color.WHITE);
		valuePanel.add(displayPanel);
		valuePanel.setBackground(Color.WHITE);
		area.add(valuePanel);
	}

	/**
	 * Adds a listener to the fromText button.
	 */
	private void addFromTextListener() {
		fromTextButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent event) {
				String input = JOptionPane.showInputDialog(dialog,
						"Please insert the attribute's value:", "VALUE SELECTION",
						JOptionPane.QUESTION_MESSAGE);
				if (input != null) {
					try {
						Class<? extends MemoryAttribute> attributeClass = MemoryAttribute
								.getTypeClass((String) valueTypeSelection.getSelectedItem());
						Parsable attribute = (Parsable) attributeClass.newInstance();
						selectedValue = (MemoryAttribute) attribute.parse(input);
						if (selectedValue == null) {
							throw new Exception();
						}
						valueField.setText("TEXT: " + input);
					} catch (Exception e) {
						Reporter.showWarning("Could not parse value from text.");
					}
				}
			}
		});
	}

	/**
	 * Adds a listener to the fromObject button.
	 */
	private void addFromObjectListener() {
		fromObjectButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent event) {
				String input = JOptionPane.showInputDialog(dialog,
						"Please insert an object's name which will be loaded from the database.",
						"VALUE SELECTION", JOptionPane.QUESTION_MESSAGE);
				if (input != null) {
					try {
						CommandPanel panel = MMDBUserInterfaceController.getInstance()
								.getCommandPanel();
						ListExpr queryResult = panel.getCommandResult("query " + input);
						String attributeType = (String) valueTypeSelection.getSelectedItem();
						Class<? extends MemoryAttribute> attributeClass = MemoryAttribute
								.getTypeClass(attributeType);
						MemoryAttribute attribute = attributeClass.newInstance();
						attribute.fromList(queryResult.second());
						selectedValue = attribute;
						valueField.setText("OBJECT: " + input);
					} catch (Exception e) {
						Reporter.showWarning("Could not create value object.");
					}
				}
			}
		});
	}

	/**
	 * Adds a listener to the valueTypeSelection combobox.
	 */
	private void addValueTypeSelectionListener() {
		valueTypeSelection.addItemListener(new ItemListener() {
			@Override
			public void itemStateChanged(ItemEvent e) {
				selectedValue = null;
				valueField.setText("");
				fromTextButton.setEnabled(false);
				fromObjectButton.setEnabled(false);
				String selectedAttribute = (String) valueTypeSelection.getSelectedItem();
				if (selectedAttribute == null) {
					return;
				}
				if (isAttributeParsable(selectedAttribute)) {
					fromTextButton.setEnabled(true);
				}
				fromObjectButton.setEnabled(true);
			}
		});
	}

	/**
	 * Dynamically fills the value type selection list depending on the selected
	 * attribute and operator.
	 * 
	 * @param selectedAttribute
	 *            the selected attribute
	 * @param selectedOperator
	 *            the selected operator
	 */
	private void fillValueTypeSelectionList(String selectedAttribute, COperator selectedOperator) {
		DefaultComboBoxModel model = (DefaultComboBoxModel) valueTypeSelection
				.getModel();
		model.removeAllElements();
		if (selectedAttribute == null || selectedOperator == null) {
			return;
		}
		Class<?> attributeValueClass = MemoryAttribute
				.getTypeClass(removeIdentifier(selectedAttribute));
		List<Class<?>> valueClasses = OperationController.getCondValueClasses(selectedOperator,
				attributeValueClass);
		List<String> insertedElements = new ArrayList<String>();
		for (Class<?> valueClass : valueClasses) {
			for (Class<? extends MemoryAttribute> attributeClass : MemoryAttribute
					.getAllTypeClasses()) {
				if (valueClass.isAssignableFrom(attributeClass)) {
					String element = MemoryAttribute.getTypeName(attributeClass);
					String attributeType = removeIdentifier((String) attributeList
							.getSelectedItem());
					if (selectedOperator.parameterTypesEqual && !element.equals(attributeType)) {
						continue;
					}
					if (!insertedElements.contains(element)) {
						model.addElement(element);
						insertedElements.add(element);
					}
				}
			}
		}
		if (!insertedElements.isEmpty()) {
			valueTypeSelection.setSelectedIndex(0);
		}
	}

	/**
	 * Checks whether a given attribute is assignable to Parsable.
	 * 
	 * @param selectedAttribute
	 *            the attribute to check
	 * @return true if the attribute is parsable, else false
	 */
	private boolean isAttributeParsable(String selectedAttribute) {
		Class<? extends MemoryAttribute> attributeClass = MemoryAttribute
				.getTypeClass(selectedAttribute);
		if (attributeClass != null && Parsable.class.isAssignableFrom(attributeClass)) {
			return true;
		}
		return false;
	}

}
