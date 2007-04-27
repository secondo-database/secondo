package viewer.chess;

import gui.MainWindow;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.ListSelectionModel;
import javax.swing.border.Border;
import javax.swing.border.TitledBorder;
import tools.Reporter;

public class QueryCreateDialog extends JDialog implements ActionListener {
	private Border compound2 = BorderFactory.createCompoundBorder(
			BorderFactory.createLineBorder(Color.black),
			BorderFactory.createMatteBorder(5, 5, 5, 5, Color.WHITE));
	
	private JTextField selField,fromField;
	private JTextArea queryArea;
	private JList condList;
	private Vector sqlConditions;
	private Vector secConditions;
	private Vector conditions;
  private gui.ViewerControl VC;
	
	public QueryCreateDialog(Vector queryCond, Vector secCond, gui.ViewerControl VC) {
		super();
    this.VC = VC;
		this.setTitle("Create Query");
		this.sqlConditions = queryCond;
		this.secConditions = secCond;
		this.conditions = new Vector(sqlConditions);
		for (int i = 0; i < secConditions.size(); i++) 
			conditions.add(secConditions.get(i));
		Box centerBox = new Box(BoxLayout.Y_AXIS);
		
//	create selection 
		//JPanel selPanel = new JPanel();
		Box labelBox = new Box(BoxLayout.Y_AXIS);
		Box fieldBox = new Box(BoxLayout.Y_AXIS);
		Box aBox = new Box(BoxLayout.X_AXIS);
		selField = new JTextField(20);
		selField.setHorizontalAlignment(JTextField.RIGHT);
		selField.setEnabled(true);
		selField.setBorder(compound2);
		selField.setText("*");
		selField.setActionCommand("selField");
		selField.addActionListener(this);
		
// create from
		//JPanel fromPanel = new JPanel();
		fromField = new JTextField(20);
		fromField.setHorizontalAlignment(JTextField.RIGHT);
		fromField.setEnabled(true);
		fromField.setBorder(compound2);
		fromField.setActionCommand("fromField");
		fromField.addActionListener(this);
		labelBox.add(Box.createVerticalStrut(10));
		labelBox.add(new JLabel("sql select "));
		labelBox.add(Box.createVerticalStrut(15));
		labelBox.add(new JLabel("from        "));
		labelBox.add(Box.createVerticalStrut(5));
		fieldBox.add(Box.createVerticalStrut(5));
		fieldBox.add(selField);
		fieldBox.add(Box.createVerticalStrut(5));
		fieldBox.add(fromField);
		aBox.add(Box.createHorizontalStrut(3));
		aBox.add(labelBox);
		aBox.add(Box.createHorizontalStrut(5));
		aBox.add(fieldBox);
		aBox.add(Box.createHorizontalStrut(3));
		centerBox.add(aBox);
		
// create where
		JPanel wherePanel = new JPanel();
		wherePanel.add(new JLabel("where     "));
		condList = new JList(conditions);
		condList.setVisibleRowCount(6);
		condList.setFixedCellWidth(220);
		condList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		JScrollPane scrollPane = new JScrollPane(condList);
		//scrollPane.setBorder(BorderFactory.createMatteBorder(5, 5, 5, 5, getBackground()));
		wherePanel.add(scrollPane);
		GridLayout grid = new GridLayout(3,0);
		grid.setHgap(3);
		grid.setVgap(3);
		JPanel buttonBox = new JPanel(grid);
		JButton editButton = new JButton("edit");
		editButton.setToolTipText("edit the selected condition");
		editButton.setBorder(ChessObject.compound5);
		editButton.addActionListener(this);
		editButton.setEnabled(true);
		buttonBox.add(editButton);
		//buttonBox.add(Box.createHorizontalStrut(5));
		JButton removeButton = new JButton("remove");
		removeButton.setToolTipText("removes the selected condition");
		removeButton.setBorder(ChessObject.compound5);
		removeButton.addActionListener(this);
		removeButton.setEnabled(true);
		buttonBox.add(removeButton);
		//buttonBox.add(Box.createHorizontalStrut(5));
		JButton addButton = new JButton("add");
		addButton.setToolTipText("add a condition");
		addButton.setBorder(ChessObject.compound5);
		addButton.addActionListener(this);
		addButton.setEnabled(true);
		buttonBox.add(addButton);
		wherePanel.add(buttonBox);
		centerBox.add(wherePanel);
		
// create preview
		queryArea = new JTextArea();
		queryArea.setLineWrap(true);
		queryArea.setText(createQuery());
		queryArea.setRows(10);
		queryArea.setColumns(25);
		queryArea.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
		JScrollPane scroll = new JScrollPane();
		
		scroll.setBorder(BorderFactory.createCompoundBorder(
				BorderFactory.createTitledBorder(
				BorderFactory.createLineBorder(Color.BLACK), 
				"Query-Preview",
				TitledBorder.CENTER,TitledBorder.TOP),
				BorderFactory.createEmptyBorder(5,5,5,5)));
		scroll.setViewportView(queryArea);
		centerBox.add(scroll);
		centerBox.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
		
//	Create the JOptionPane
		JPanel optionPanel = new JPanel();
		JButton copyButton = new JButton(" Copy ");
		JButton clearButton = new JButton(" Clear ");
		JButton closeButton = new JButton(" Close ");
		copyButton.setToolTipText("copy selection to console");
		clearButton.setToolTipText("clear all from list");
		closeButton.setToolTipText("close this dialog");		
		copyButton.setBorder(ChessObject.compound5);
		clearButton.setBorder(ChessObject.compound5);
		closeButton.setBorder(ChessObject.compound5);
		optionPanel.add(copyButton);
		optionPanel.add(clearButton);
		optionPanel.add(closeButton);
		copyButton.addActionListener(this);
		closeButton.addActionListener(this);
		clearButton.addActionListener(this);
	
// put all together
		this.getContentPane().setLayout(new BorderLayout());
		this.getContentPane().add(centerBox, BorderLayout.CENTER);
		this.getContentPane().add(optionPanel, BorderLayout.SOUTH);
		this.pack();
	}
	
	
	private String createQuery() {
		//condList.setListData(conditions);
		StringBuffer myQuery = new StringBuffer();
		if (!sqlConditions.isEmpty() || secConditions.isEmpty()) {
			myQuery.append("sql select ");
			myQuery.append(selField.getText());
			myQuery.append(" from ");
			myQuery.append(fromField.getText());
			myQuery.append(" where ");
			myQuery.append("[");
			for (int i =0; i< sqlConditions.size();i++) {
				myQuery.append(sqlConditions.get(i));
				myQuery.append(",");
			}
			// remove last comma
			myQuery.deleteCharAt(myQuery.length()-1);
			myQuery.append("].");
		}
		if (!secConditions.isEmpty()) {
			myQuery.append("query ");
			myQuery.append(fromField.getText());
			if (!selField.getText().equals("*")) {
				myQuery.append(" project[");
				myQuery.append(selField.getText());
				myQuery.append("]");
			}
			myQuery.append(" feed ");
			for (int i =0; i< secConditions.size();i++) {
				myQuery.append(secConditions.get(i));
				myQuery.append(" feed ");
			}
			//remove last feed
			myQuery.delete(myQuery.length()-6,myQuery.length()-1);
			myQuery.append("consume");	
		}		
		return myQuery.toString();
	}

	public void actionPerformed(ActionEvent arg0) {
		if (arg0.getActionCommand().equals(" Copy ")) {
			//MainWindow.ComPanel.appendText(queryArea.getText());
      VC.execUserCommand(queryArea.getText());
			this.setVisible(false);
		}
		if (arg0.getActionCommand().equals(" Clear ")) {
			selField.setText("*");
			fromField.setText("");
			sqlConditions.clear();
			secConditions.clear();
			this.conditions = new Vector(sqlConditions);
			for (int i = 0; i < secConditions.size(); i++) 
				conditions.add(secConditions.get(i));
			condList.setListData(conditions);
		}
		if (arg0.getActionCommand().equals(" Close ")) {
			this.setVisible(false);
		}
		if (arg0.getActionCommand().equals("selField")) {
			// ?? check if exists
		}
		if (arg0.getActionCommand().equals("fromField")) {
			// ?? check if object exists
		}
		
		if (arg0.getActionCommand().equals("edit")) {
			if (!condList.isSelectionEmpty()) {
				int idx = condList.getSelectedIndex();
				System.out.println("Size of cond "+sqlConditions.size());
				String oldStr = (String)sqlConditions.get(idx);
				String newStr = JOptionPane.showInputDialog(this,
            		"Edit this Expression :", oldStr);
				if (newStr.length()==0) newStr = oldStr;
				if (idx < sqlConditions.size()) {
					sqlConditions.remove(idx);
					sqlConditions.add(idx, newStr);
				}	
				else {
					secConditions.remove(sqlConditions.size()+idx);
					secConditions.add(sqlConditions.size()+idx, newStr);
				}
				this.conditions = new Vector(sqlConditions);
				for (int i = 0; i < secConditions.size(); i++) 
					conditions.add(secConditions.get(i));
				condList.setListData(conditions);
				condList.setSelectedIndex(idx);
				condList.repaint();
			}
			else Reporter.showWarning("No object selected !");
		}
		
		if (arg0.getActionCommand().equals("remove")) {
			if (!condList.isSelectionEmpty()) {
				int idx = condList.getSelectedIndex();	
				if (idx < sqlConditions.size()) {
					sqlConditions.remove(idx);
				}	
				else {
					secConditions.remove(idx-sqlConditions.size());
				}
				this.conditions = new Vector(sqlConditions);
				for (int i = 0; i < secConditions.size(); i++) 
					conditions.add(secConditions.get(i));
				condList.setListData(conditions);
				condList.repaint();
			}		
			else Reporter.showWarning("No object selected !");
		}
		if (arg0.getActionCommand().equals("add")) {
			
			String newStr = JOptionPane.showInputDialog(this,
            		"Insert your Expression :", "");
			
			if (newStr.length()>0) {
				StringBuffer buf = new StringBuffer(newStr);
				boolean what = buf.indexOf("query")!=-1 || buf.indexOf("filter")!=-1 ||
								buf.indexOf("consume")!=-1 || buf.indexOf("feed")!=-1;
				if (!what )
				sqlConditions.addElement(newStr);
				else 
				secConditions.addElement(newStr);	
				this.conditions = new Vector(sqlConditions);
				for (int i = 0; i < secConditions.size(); i++) 
					conditions.add(secConditions.get(i));
				condList.setListData(conditions);
				if (!what) 
					condList.setSelectedIndex(sqlConditions.size()-1);
				else
					condList.setSelectedIndex(secConditions.size()-1);
			}
		}
		queryArea.setText(createQuery());
	}
	
	public void setVisible(boolean visible) {
		this.conditions = new Vector(sqlConditions);
		for (int i = 0; i < secConditions.size(); i++) 
			conditions.add(secConditions.get(i));
		condList.setListData(conditions);
		queryArea.setText(createQuery());
		//System.out.println("condList size"+ conditions.size());
		super.setVisible(visible);
	}

}
