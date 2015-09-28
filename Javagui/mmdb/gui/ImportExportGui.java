package mmdb.gui;

import gui.ObjectList;
import gui.SecondoObject;

import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.File;
import java.util.List;
import java.util.concurrent.CancellationException;

import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.SwingWorker;
import javax.swing.SwingWorker.StateValue;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;

import mmdb.error.inout.ExportException;
import mmdb.error.inout.ImportException;
import mmdb.service.ObjectExport;
import mmdb.service.ObjectImport;
import tools.Reporter;

/**
 * A class managing any gui components needed while importing or exporting main
 * memory objects.
 * 
 * @author Bjoern Clasen
 */
public class ImportExportGui extends JDialog {

	private static final long serialVersionUID = 5538523860631830202L;

	/**
	 * A value for a filepath to indicate that no file has been selected.
	 */
	public static final String NO_FILE = "NO_FILE";

	/**
	 * The JFileChooser to use for choosing import and export files.
	 */
	private static JFileChooser fileChooser;

	/**
	 * The table containing the object information.
	 */
	private JTable table;

	/**
	 * Buttons for user interaction
	 */
	private JButton buttonOK, buttonCANCEL;

	/**
	 * Progressbar showing progess for currently processed element
	 */
	private JProgressBar progressBar;

	/**
	 * The table's content
	 */
	private String[][] objectHeaders;

	/**
	 * The current SwingWorker
	 */
	private SwingWorker<?, ?> swingWorker;

	/**
	 * Parent component to display relative to
	 */
	private Component component;

	/**
	 * Constructor initializing the static FileChooser if necessary.
	 */
	public ImportExportGui(Component component) {
		this.component = component;
		// FileChooser
		if (ImportExportGui.fileChooser == null) {
			ImportExportGui.fileChooser = new JFileChooser();
			ImportExportGui.fileChooser.setMultiSelectionEnabled(false);
			ImportExportGui.fileChooser
					.setFileSelectionMode(JFileChooser.FILES_ONLY);
			FileNameExtensionFilter filter = new FileNameExtensionFilter(
					"Secondo MM Files", ObjectExport.FILE_EXTENSION,
					ObjectExport.FILE_EXTENSION.toUpperCase());
			ImportExportGui.fileChooser.setFileFilter(filter);
		}
	}

	/**
	 * Processes an export command sent by the menu button.
	 * 
	 * @param sobjects
	 *            The objects selected in the objectlist
	 */
	public void processExportCommand(SecondoObject[] sobjects) {
		if (sobjects.length > 0) {
			String filePath = getExportFilePath();
			if (filePath == ImportExportGui.NO_FILE) {
				return;
			}
			try {
				exportObjects(sobjects, filePath);
			} catch (ExportException e) {
				Reporter.showError(e.getMessage());
			}
		}
	}

	/**
	 * Processes an import command sent by the menu button.
	 * 
	 * @param objectList
	 *            the ObjectList to add results to.
	 */
	public void processImportCommand(ObjectList objectList) {
		String filePath = getImportFilePath();
		if (filePath == ImportExportGui.NO_FILE) {
			return;
		}
		try {
			importObjects(filePath, objectList);
		} catch (ImportException e) {
			Reporter.showError(e.getMessage());
		}
	}

	/**
	 * Processes a {@code setProgress(...)} call from the
	 * {@link ImportExportGui#swingWorker}.<br>
	 * Updates the corresponding table cells and repaints the table.
	 * 
	 * @param processedIndices
	 *            a list of table row indices representing objects whose
	 *            procession has been finished.
	 */
	public void processProgressUpdate(List<Integer> processedIndices) {
		for (int index : processedIndices) {
			this.objectHeaders[index][3] = "Finished!";
			if (index < this.objectHeaders.length - 1) {
				this.objectHeaders[index + 1][3] = "Working...";
			}
		}
		((AbstractTableModel) this.table.getModel()).fireTableDataChanged();
	}

	/**
	 * Exports the given SecondoObjects to a file located under filePath.<br>
	 * Uses the SwingWorker {@link ObjectExport} for background procession.
	 * 
	 * @param sobjects
	 *            the SecondoObjects to export
	 * @param filePath
	 *            the path to export to
	 * @throws ExportException
	 *             if export could not be started
	 */
	private void exportObjects(SecondoObject[] sobjects, String filePath)
			throws ExportException {
		ObjectExport.checkObjectsExportable(sobjects);
		ObjectExport objectExport = new ObjectExport(sobjects, filePath, this);
		this.addPropertyChangeListenerToWorker(objectExport, null);

		// Read headers
		this.objectHeaders = objectExport.getObjectHeaders();
		this.objectHeaders[0][3] = "Working...";

		// Dialog
		this.setTitle("Exporting MMObjects...");
		this.initDialog();

		this.swingWorker = objectExport;
		this.swingWorker.execute();
		this.setVisible(true);
	}

	/**
	 * Imports all SecondoObjects stored in a file located under filePath.<br>
	 * Adds them to the objectList.<br>
	 * Uses the SwingWorker {@link ObjectImport} for background procession.
	 * 
	 * @param filePath
	 *            the path to the file to import
	 * @param objectList
	 *            the ObjectList to add results to
	 * @throws ImportException
	 *             if file was invalid or not readable
	 */
	private void importObjects(String filePath, final ObjectList objectList)
			throws ImportException {
		ObjectImport.checkFile(filePath);
		ObjectImport objectImport = new ObjectImport(filePath, this);
		this.addPropertyChangeListenerToWorker(objectImport, objectList);

		// Read headers
		this.objectHeaders = objectImport.getObjectHeaders();
		this.objectHeaders[0][3] = "Working...";

		// Dialog
		this.setTitle("Importing MMObjects...");
		this.initDialog();

		this.swingWorker = objectImport;
		this.swingWorker.execute();
		this.setVisible(true);
	}

	/**
	 * Adds a PropertyChangeListener to the given SwingWorker.<br>
	 * The PropertyChangeListener will:<br>
	 * - handle progress events (updating JProgressbar)<br>
	 * - handle done state (Cancel/Exception/Success).<br>
	 * ObjectList must be {@code null} for export.
	 * 
	 * @param worker
	 *            the worker to add the PropertyChangeListener to
	 * @param objectList
	 *            for export: {@code null}<br>
	 *            for import: the ObjectList to add imports to.
	 */
	private void addPropertyChangeListenerToWorker(
			final SwingWorker<?, ?> worker, final ObjectList objectList) {
		worker.addPropertyChangeListener(new PropertyChangeListener() {
			public void propertyChange(PropertyChangeEvent evt) {
				if ("progress".equals(evt.getPropertyName())) {
					ImportExportGui.this.progressBar.setValue((Integer) evt
							.getNewValue());
				} else if ("state".equals(evt.getPropertyName())) {
					if ((StateValue) evt.getNewValue() == StateValue.DONE) {
						try {
							if (objectList == null) {
								worker.get();
							} else {
								SecondoObject[] sobjects = (SecondoObject[]) worker
										.get();
								for (SecondoObject sobject : sobjects) {
									objectList.addEntry(sobject);
								}
							}
							ImportExportGui.this.buttonCANCEL.setEnabled(false);
							ImportExportGui.this.buttonOK.setEnabled(true);
						} catch (CancellationException e) {
							JOptionPane
									.showMessageDialog(ImportExportGui.this,
											"Procession has been cancelled!\nAlready processed data is deleted.");
							ImportExportGui.this.setVisible(false);
							ImportExportGui.this.dispose();
						} catch (Exception e) {
							if (e.getCause() instanceof ExportException
									|| e.getCause() instanceof ImportException) {
								JOptionPane
										.showMessageDialog(
												ImportExportGui.this,
												"An Exception occured:\n"
														+ e.getCause()
																.getMessage(),
												"Exception!",
												JOptionPane.ERROR_MESSAGE);
							} else {
								JOptionPane.showMessageDialog(
										ImportExportGui.this,
										"An unknown error occured!",
										"Unexcepted Exception!",
										JOptionPane.ERROR_MESSAGE);
							}
							ImportExportGui.this.setVisible(false);
							ImportExportGui.this.dispose();
						}
					}
				}
			}
		});
	}

	/**
	 * Opens up a Filechooser opendialog to retrieve a filepath to open.
	 * 
	 * @return the filepath selected in the filechooser or {@value #NO_FILE} if
	 *         none was selected.
	 */
	private String getImportFilePath() {
		int retVal = fileChooser.showOpenDialog(null);
		if (retVal == JFileChooser.APPROVE_OPTION) {
			File selectedFile = fileChooser.getSelectedFile();
			if (selectedFile != null) {
				return selectedFile.getAbsolutePath();
			}
		}
		return NO_FILE;
	}

	/**
	 * Opens up a Filechooser davedialog to retrieve a filepath to save to.
	 * 
	 * @return the filepath selected in the filechooser or {@value #NO_FILE} if
	 *         none was selected.
	 */
	private String getExportFilePath() {
		int retVal = fileChooser.showSaveDialog(null);
		if (retVal == JFileChooser.APPROVE_OPTION) {
			File selectedFile = fileChooser.getSelectedFile();
			if (selectedFile != null) {
				return selectedFile.getAbsolutePath();
			}
		}
		return NO_FILE;
	}

	/**
	 * Initializes the dialog and adds necessary components to it.
	 */
	private void initDialog() {
		Container container = this.getContentPane();
		container.setLayout(new BoxLayout(container, BoxLayout.Y_AXIS));
		this.addTablePanel(container);
		this.addProgressPanel(container);
		this.addControlPanel(container);
		this.setLocationRelativeTo(this.component);
		this.setModal(true);
		this.pack();
		this.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
		this.addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				if (ImportExportGui.this.buttonOK.isEnabled()) {
					ImportExportGui.this.handleOK();
				} else {
					ImportExportGui.this.handleCANCEL();
				}
			}
		});
	}

	/**
	 * Handles a "OK" button press: Closes the window.
	 */
	private void handleOK() {
		this.setVisible(false);
		this.dispose();
	}

	/**
	 * Handles a "CANCEL" button press: asks for confirmation and if given
	 * cancels the {@link ImportExportGui#swingWorker}.
	 */
	private void handleCANCEL() {
		int cancel = JOptionPane
				.showConfirmDialog(
						this,
						"Do you really want to cancel?\nAlready processed data will be deleted!",
						"Cancel?", JOptionPane.YES_NO_OPTION);
		if (cancel == JOptionPane.YES_OPTION) {
			if (!this.buttonOK.isEnabled()) {
				this.swingWorker.cancel(true);
			} else {
				JOptionPane.showMessageDialog(this,
						"Procession finished before cancellation!", "Finished",
						JOptionPane.WARNING_MESSAGE);
			}
		}
	}

	/**
	 * Adds the table panel to the container.<br>
	 * Make sure {@link ImportExportGui#objectHeaders} have been initialized
	 * beforehand.
	 * 
	 * @param container
	 *            the container to add the table to.
	 */
	private void addTablePanel(Container container) {
		String[] columnNames = { "NAME", "TYPE", "TUPLES", "STATUS" };
		this.table = new JTable(this.objectHeaders, columnNames);
		this.table.setPreferredScrollableViewportSize(new Dimension(600, 200));
		this.table.setFillsViewportHeight(true);
		DefaultTableCellRenderer centerRenderer = new DefaultTableCellRenderer();
		centerRenderer.setHorizontalAlignment(JLabel.CENTER);
		for (int i = 0; i <= 3; i++) {
			this.table.getColumnModel().getColumn(i)
					.setCellRenderer(centerRenderer);
		}
		((DefaultTableCellRenderer) this.table.getTableHeader()
				.getDefaultRenderer()).setHorizontalAlignment(JLabel.CENTER);
		this.table.setAutoResizeMode(JTable.AUTO_RESIZE_ALL_COLUMNS);
		this.table.getColumn("NAME").setMinWidth(250);
		JScrollPane scrollPane = new JScrollPane(this.table);
		scrollPane.setOpaque(true);
		container.add(scrollPane);
	}

	/**
	 * Adds the JProgressbar to the container.
	 * 
	 * @param container
	 *            the container to add the JProgressbar to.
	 */
	private void addProgressPanel(Container container) {
		this.progressBar = new JProgressBar(0, 100);
		this.progressBar.setStringPainted(true);
		container.add(this.progressBar);
	}

	/**
	 * Adds the control panel to the container.<br>
	 * The control panel contains the buttons for GUI interaction.
	 * 
	 * @param container
	 *            the container to add the control panel to.
	 */
	private void addControlPanel(Container container) {
		JPanel panel = new JPanel(new FlowLayout());
		this.buttonOK = new JButton("OK");
		this.buttonOK.setEnabled(false);
		this.buttonOK.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				ImportExportGui.this.handleOK();
			}
		});
		this.buttonCANCEL = new JButton("CANCEL");
		this.buttonCANCEL.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				ImportExportGui.this.handleCANCEL();
			}
		});
		panel.add(this.buttonOK);
		panel.add(this.buttonCANCEL);
		panel.setMinimumSize(new Dimension(100, 60));
		panel.setMaximumSize(new Dimension(2000, 60));
		panel.setOpaque(true);
		container.add(panel);
	}

}
