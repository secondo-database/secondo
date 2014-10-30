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
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.List;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.SwingWorker;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableModel;

import mmdb.service.MemoryWatcher;
import tools.Reporter;

/**
 * This class represents the memory dialog that opens if a user clicks on "MMDB
 * -> Manage Memory" menu item or when there is an imminent
 * OutOfMemoryException.
 *
 * @author Alexander Castor
 */
public final class MemoryDialog extends JDialog {

	private static final long serialVersionUID = -6526223970354560343L;

	/**
	 * The object statistics used in the table. Each row denotes one secondo
	 * object. Columns are [0] = name, [1] = tuples, [2] = list, [3] = relation,
	 * [4] = indices.
	 */
	private String[][] objectStatistics;

	/**
	 * The table containing the object information.
	 */
	private JTable table;

	/**
	 * The dialog's answer for further processing. ([0] = selected object, [1] =
	 * selected deletion type)
	 */
	private Object[] answer;

	/**
	 * The label containing the total memory.
	 */
	private JLabel memoryTotal = new JLabel();

	/**
	 * The label containing the used memory.
	 */
	private JLabel memoryUsed = new JLabel();

	/**
	 * The label containing the free memory.
	 */
	private JLabel memoryFree = new JLabel();

	/**
	 * Displays the dialog and returns the user input to the caller.
	 * 
	 * @param objectStatistics
	 *            the objects's statistics
	 * @param component
	 *            the parent component for locating the window
	 * @return the user's input
	 */
	public static Object[] showDialog(String[][] objectStatistics, Component component) {
		MemoryDialog dialog = new MemoryDialog(objectStatistics);
		dialog.setLocationRelativeTo(component);
		dialog.setModal(true);
		dialog.pack();
		dialog.setVisible(true);
		return dialog.answer;
	}

	/**
	 * Assembles the dialog's components and initializes members.
	 * 
	 * @param objectStatistics
	 *            the objects's statistics
	 */
	private MemoryDialog(String[][] objectStatistics) {
		this.objectStatistics = objectStatistics;
		this.answer = new Object[2];
		setName("MANAGE MEMORY");
		setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		Container container = this.getContentPane();
		container.setLayout(new BoxLayout(container, BoxLayout.Y_AXIS));
		addStatisticsPanel(container);
		addTablePanel(container);
		addControlPanel(container);
	}

	/**
	 * Adds the statistics area to the dialog.
	 * 
	 * @param container
	 *            the dialog's content pane
	 */
	private void addStatisticsPanel(Container container) {
		JPanel panel = new JPanel(new GridBagLayout());
		GridBagConstraints constraints = new GridBagConstraints();
		constraints.gridx = 0;
		constraints.gridy = 0;
		panel.add(new JLabel("Total: "), constraints);
		constraints.gridx = 1;
		constraints.gridy = 0;
		panel.add(memoryTotal, constraints);
		constraints.gridx = 0;
		constraints.gridy = 1;
		panel.add(new JLabel("Used: "), constraints);
		constraints.gridx = 1;
		constraints.gridy = 1;
		panel.add(memoryUsed, constraints);
		constraints.gridx = 0;
		constraints.gridy = 2;
		panel.add(new JLabel("Free: "), constraints);
		constraints.gridx = 1;
		constraints.gridy = 2;
		panel.add(memoryFree, constraints);
		panel.setBorder(BorderFactory.createTitledBorder("STATISTICS"));
		panel.setMinimumSize(new Dimension(100, 60));
		panel.setMaximumSize(new Dimension(2000, 60));
		panel.setOpaque(true);
		container.add(panel);
		startWorker();
	}

	/**
	 * Starts a swing worker to asynchronously update the memory statistics
	 */
	private void startWorker() {
		SwingWorker<Void, String[]> worker = new SwingWorker<Void, String[]>() {

			@Override
			protected Void doInBackground() throws Exception {
				int counter = 0;
				while (counter < Integer.MAX_VALUE) {
					String[] statistics = MemoryWatcher.getInstance().getMemoryStatistics(counter);
					publish(statistics);
					Thread.sleep(1000);
					counter++;
				}
				return null;
			}

			@Override
			protected void process(List<String[]> statistics) {
				String[] mostRecent = statistics.get(statistics.size() - 1);
				memoryTotal.setText(mostRecent[0]);
				memoryUsed.setText(mostRecent[1]);
				memoryFree.setText(mostRecent[2]);
			}
		};

		worker.execute();
	}

	/**
	 * Adds the table area to the dialog.
	 * 
	 * @param container
	 *            the dialog's content pane
	 */
	private void addTablePanel(Container container) {
		String[] columnNames = { "NAME", "TUPLES", "NESTED-LIST", "RELATION", "INDICES" };
		table = new JTable(objectStatistics, columnNames);
		table.setPreferredScrollableViewportSize(new Dimension(600, 200));
		table.setFillsViewportHeight(true);
		table.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		DefaultTableCellRenderer centerRenderer = new DefaultTableCellRenderer();
		centerRenderer.setHorizontalAlignment(JLabel.CENTER);
		for (int i = 0; i <= 4; i++) {
			table.getColumnModel().getColumn(i).setCellRenderer(centerRenderer);
		}
		((DefaultTableCellRenderer) table.getTableHeader().getDefaultRenderer())
				.setHorizontalAlignment(JLabel.CENTER);
		table.setAutoResizeMode(JTable.AUTO_RESIZE_ALL_COLUMNS);
		table.getColumn("NAME").setMinWidth(250);
		table.changeSelection(0, 0, false, false);
		JScrollPane scrollPane = new JScrollPane(table);
		scrollPane.setBorder(BorderFactory.createTitledBorder("SELECT OBJECT"));
		scrollPane.setOpaque(true);
		container.add(scrollPane);
	}

	/**
	 * Adds the control selection to the dialog including the buttons for
	 * removing elements.
	 * 
	 * @param container
	 *            the dialog's content pane
	 */
	private void addControlPanel(Container container) {
		JPanel panel = new JPanel(new FlowLayout());
		JButton removeObject = new JButton("OBJECT");
		JButton removeNL = new JButton("NESTED-LIST");
		JButton removeMemRelation = new JButton("RELATION");
		JButton removeIndex = new JButton("INDICES");
		addActionListener(removeObject, Command.OBJ);
		addActionListener(removeNL, Command.NES);
		addActionListener(removeMemRelation, Command.REL);
		addActionListener(removeIndex, Command.IDX);
		panel.add(removeObject);
		panel.add(removeNL);
		panel.add(removeMemRelation);
		panel.add(removeIndex);
		panel.setBorder(BorderFactory.createTitledBorder("REMOVE"));
		panel.setMinimumSize(new Dimension(100, 60));
		panel.setMaximumSize(new Dimension(2000, 60));
		panel.setOpaque(true);
		container.add(panel);
	}

	/**
	 * Adds an action listener to the remove button.
	 * 
	 * @param button
	 *            the button that is being clicked
	 * @param command
	 *            the command for the answer
	 */
	private void addActionListener(JButton button, final Command command) {
		button.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				TableModel model = table.getModel();
				int row = table.getSelectedRow();
				if (row == -1) {
					Reporter.showInfo("Please select a row in the table.");
				} else {
					answer[0] = (String) model.getValueAt(row, 0);
					answer[1] = command;
					setVisible(false);
					dispose();
				}
			}
		});
	}
	
	public enum Command {
		OBJ, NES, REL, IDX;
	}

}
