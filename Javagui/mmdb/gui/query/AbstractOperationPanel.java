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
import java.awt.Font;
import java.awt.GridLayout;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import javax.swing.BorderFactory;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingConstants;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryRelation.RelationHeaderItem;
import mmdb.gui.QueryDialog;
import mmdb.operator.OperationController;
import mmdb.operator.OperationController.COperator;
import mmdb.query.AbstractQueryController;
import mmdb.query.AggregationController;
import mmdb.query.ExtensionController;
import mmdb.query.JoinController;
import mmdb.query.ProjectionController;
import mmdb.query.SelectionController;
import mmdb.query.UnionController;

/**
 * Superclass for all operation panels that are included in the query dialog.
 *
 * @author Alexander Castor
 */
public abstract class AbstractOperationPanel extends JPanel {

	private static final long serialVersionUID = 5428757906271438415L;

	/**
	 * The height of the panel's standard component (description, convert,
	 * button).
	 */
	private static final int STANDARD_COMPONENT_HEIGHT = 40;

	/**
	 * The height for the middle area
	 */
	protected static final int MIDDLE_HEIGHT = QueryDialog.DIALOG_HEIGHT - 3
			* STANDARD_COMPONENT_HEIGHT;

	/**
	 * The controller which is responsible for executing the query.
	 */
	protected AbstractQueryController queryController;

	/**
	 * The dialog's answer for further processing. ([0] = result relation, [1] =
	 * object name, [2] = convert result to nested list).
	 */
	protected Object[] answer;

	/**
	 * Check box that indicates whether the result relation shall be converted
	 * to a nested list.
	 */
	protected JCheckBox convert;

	/**
	 * The currently available memory relations.
	 */
	protected Map<String, MemoryRelation> relations;

	/**
	 * Reference to the parent dialog, needed for disposing the window in the
	 * button's action listeners.
	 */
	protected JDialog dialog;
	
	/**
	 * The previously selected value from the cond attribute list.
	 */
	private String previousValueSelection;

	/**
	 * Creates the UI elements contained in the panel.
	 */
	public abstract void constructPanel();

	/**
	 * Creates an action listener for the execute button.
	 * 
	 * @return the button's action listener
	 */
	protected abstract ActionListener createButtonListener();

	/**
	 * Adds the description to the top of the panel.
	 * 
	 * @param text
	 *            the description's text
	 * @param area
	 *            the underlying container the components are added to
	 */
	protected void addDescriptionPanel(String text, JPanel area) {
		JPanel descArea = new JPanel(new GridLayout(1, 1));
		descArea.setBorder(BorderFactory.createTitledBorder(""));
		descArea.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH, STANDARD_COMPONENT_HEIGHT));
		descArea.setMinimumSize(new Dimension(QueryDialog.DIALOG_WIDTH, STANDARD_COMPONENT_HEIGHT));
		descArea.setMaximumSize(new Dimension(2000, STANDARD_COMPONENT_HEIGHT));
		JLabel description = new JLabel(text, SwingConstants.CENTER);
		Font newFont = new Font(description.getFont().getName(), Font.BOLD, description.getFont()
				.getSize());
		description.setFont(newFont);
		description.setForeground(Color.WHITE);
		descArea.setBackground(Color.GRAY);
		descArea.add(description);
		area.add(descArea);
	}

	/**
	 * Adds the convert to list panel to the bottom of the panel.
	 * 
	 * @param area
	 *            the underlying container the components are added to
	 */
	protected void addConvertToListPanel(JPanel area) {
		JPanel convertArea = new JPanel(new FlowLayout());
		JLabel text = new JLabel("CONVERT RESULT RELATION AUTOMATICALLY TO NESTED LIST?");
		convert = new JCheckBox();
		convert.setSelected(false);
		convertArea.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH,
				STANDARD_COMPONENT_HEIGHT));
		convertArea.setMinimumSize(new Dimension(QueryDialog.DIALOG_WIDTH,
				STANDARD_COMPONENT_HEIGHT));
		convertArea.setMaximumSize(new Dimension(2000, STANDARD_COMPONENT_HEIGHT));
		Font newFont = new Font(text.getFont().getName(), Font.BOLD + Font.ITALIC, 12);
		text.setFont(newFont);
		text.setForeground(Color.GRAY);
		convertArea.setAlignmentX(CENTER_ALIGNMENT);
		convertArea.setAlignmentY(CENTER_ALIGNMENT);
		convertArea.add(text);
		convertArea.add(convert);
		area.add(convertArea);
	}

	/**
	 * Adds the button to the bottom of the panel.
	 * 
	 * @param area
	 *            the underlying container the components are added to
	 */
	protected void addButtonPanel(JPanel area) {
		JPanel buttonArea = new JPanel(new GridLayout(1, 1));
		JButton button = new JButton("EXECUTE");
		buttonArea.setPreferredSize(new Dimension(QueryDialog.DIALOG_WIDTH,
				STANDARD_COMPONENT_HEIGHT));
		buttonArea
				.setMinimumSize(new Dimension(QueryDialog.DIALOG_WIDTH, STANDARD_COMPONENT_HEIGHT));
		buttonArea.setMaximumSize(new Dimension(2000, STANDARD_COMPONENT_HEIGHT));
		button.addActionListener(createButtonListener());
		button.setOpaque(true);
		Font newFont = new Font(button.getFont().getName(), Font.BOLD, 18);
		button.setFont(newFont);
		button.setBackground(Color.GRAY);
		buttonArea.add(button);
		area.add(buttonArea);
	}

	/**
	 * Dynamically fills an attribute list for a conditional operator depending
	 * on the user's relation and operator selection.
	 * 
	 * @param selectedRelation
	 *            the selected relation
	 * @param operator
	 *            the selected operator
	 * @param attributeList
	 *            the list to which the relation's attributes shall be added
	 */
	protected void fillCondAttributeList(String selectedRelation, COperator operator,
			JComboBox attributeList) {
		if(attributeList.getSelectedItem() != null) {
			previousValueSelection = (String) attributeList.getSelectedItem();
		}
		DefaultComboBoxModel model = (DefaultComboBoxModel) attributeList
				.getModel();
		model.removeAllElements();
		if (selectedRelation == null || operator == null) {
			return;
		}
		MemoryRelation relation = relations.get(selectedRelation);
		List<Class<?>> argumentClasses = OperationController.getCondAttributeClasses(operator);
		List<String> insertedElements = new ArrayList<String>();
		for (RelationHeaderItem item : relation.getHeader()) {
			if (!item.isProjected()) {
				continue;
			}
			for (Class<?> argumentClass : argumentClasses) {
				if (argumentClass.isAssignableFrom(item.getType())) {
					String element = item.getIdentifier() + " (" + item.getTypeName() + ")";
					if (!insertedElements.contains(element)) {
						model.addElement(element);
						insertedElements.add(element);
					}

				}
			}
		}
		int previousIndex = model.getIndexOf(previousValueSelection);
		if (!insertedElements.isEmpty()) {
			if(previousIndex > 0) {
				attributeList.setSelectedIndex(previousIndex);
			}
			else {
				attributeList.setSelectedIndex(0);
			}
		}
	}

	/**
	 * Creates the object name for the result relation to be displayed in the
	 * object explorer.
	 * 
	 * @param operationName
	 *            the operation's name
	 * @param selectedRelation
	 *            the first relation's name
	 * @return the object name for the result relation
	 */
	protected String createObjectName(String operationName, String selectedRelation) {
		int indexOfBrackets = selectedRelation.indexOf("[");
		String relationName;
		if (selectedRelation.contains(";")) {
			relationName = selectedRelation.substring(0, indexOfBrackets - 2);
		} else {
			relationName = selectedRelation.substring(0, indexOfBrackets - 1);
		}
		return operationName + " ON " + relationName;
	}

	/**
	 * Creates the object name for the result relation to be displayed in the
	 * object explorer.
	 * 
	 * @param operationName
	 *            the operation's name
	 * @param selectedFirstRelation
	 *            the first relation's name
	 * @param selectedSecondRelation
	 *            the second relation's name
	 * @return the object name for the result relation
	 */
	protected String createObjectName(String operationName, String selectedFirstRelation,
			String selectedSecondRelation) {
		int indexOfBracketsForFirstRelation = selectedFirstRelation.indexOf("[");
		int indexOfBracketsForSecondRelation = selectedSecondRelation.indexOf("[");
		String firstRelationName;
		if (selectedFirstRelation.contains(";")) {
			firstRelationName = selectedFirstRelation.substring(0,
					indexOfBracketsForFirstRelation - 2);
		} else {
			firstRelationName = selectedFirstRelation.substring(0,
					indexOfBracketsForFirstRelation - 1);
		}
		String secondRelationName;
		if (selectedSecondRelation.contains(";")) {
			secondRelationName = selectedSecondRelation.substring(0,
					indexOfBracketsForSecondRelation - 2);
		} else {
			secondRelationName = selectedSecondRelation.substring(0,
					indexOfBracketsForSecondRelation - 1);
		}
		return firstRelationName + " " + operationName + " " + secondRelationName;
	}

	/**
	 * Removes the type name from the attribute name.
	 * 
	 * @param attribute
	 *            the attribute which is selected
	 * @return string containing only the identifier
	 */
	protected String removeType(String attribute) {
		return attribute.substring(0, attribute.indexOf("(") - 1);
	}

	/**
	 * Removes the identifier from the attribute name.
	 * 
	 * @param attribute
	 *            the attribute which is selected
	 * @return string containing only the type
	 */
	protected String removeIdentifier(String attribute) {
		return attribute.substring(attribute.indexOf("(") + 1, attribute.length() - 1);
	}

	/**
	 * Removes all type names from the attribute list.
	 * 
	 * @param attributeList
	 *            the attribute list which is selected
	 * @return an attribute list containing only the identifiers
	 */
	protected List<String> removeTypes(List<String> attributeList) {
		List<String> cleanAttributeList = new ArrayList<String>();
		for (String attribute : attributeList) {
			cleanAttributeList.add(removeType(attribute));
		}
		return cleanAttributeList;
	}

	/**
	 * Method for injecting member references.
	 * 
	 * @param answer
	 *            the dialog's answer
	 * @param relations
	 *            the available relations
	 */
	public void injectMembers(Object[] answer, Map<String, MemoryRelation> relations,
			JDialog dialog, AbstractQueryController controller) {
		this.answer = answer;
		this.relations = relations;
		this.dialog = dialog;
		this.queryController = controller;
	}

	/**
	 * Enum for collecting all query panels.
	 */
	public static enum QueryPanel {
		SELECTION(1, SelectionPanel.class, SelectionController.class), PROJECTION(2,
				ProjectionPanel.class, ProjectionController.class), EXTENSION(3,
				ExtensionPanel.class, ExtensionController.class), UNION(4, UnionPanel.class,
				UnionController.class), JOIN(5, JoinPanel.class, JoinController.class), AGGREGATION(
				6, AggregationPanel.class, AggregationController.class);

		QueryPanel(int position, Class<? extends AbstractOperationPanel> panelClass,
				Class<? extends AbstractQueryController> controllerClass) {
			this.position = position;
			this.panelClass = panelClass;
			this.controllerClass = controllerClass;
		}

		public final int position;
		public final Class<? extends AbstractOperationPanel> panelClass;
		public final Class<? extends AbstractQueryController> controllerClass;
	}

}
