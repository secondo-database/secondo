package mmdb.gui;

import gui.ObjectList;
import gui.SecondoObject;

import java.io.File;

import javax.swing.JFileChooser;
import javax.swing.filechooser.FileNameExtensionFilter;

import mmdb.error.inout.ExportException;
import mmdb.error.inout.ImportException;
import mmdb.service.ObjectExport;
import mmdb.service.ObjectImport;
import tools.Reporter;

/**
 * A class managing any gui components needed while importing or exporting main
 * memory objects.
 * 
 * @author Björn Clasen
 */
public class ImportExportGui {

	/**
	 * The singleton instance of ImportExportGui.
	 */
	private static ImportExportGui instance = new ImportExportGui();

	/**
	 * A value for a filepath to indicate that no file has been selected.
	 */
	public static final String NO_FILE = "NO_FILE";

	/**
	 * The JFileChooser to use for choosing import and export files.
	 */
	private JFileChooser fileChooser;

	/**
	 * Private constructor for singleton.
	 */
	private ImportExportGui() {
		fileChooser = new JFileChooser();
		fileChooser.setMultiSelectionEnabled(false);
		fileChooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
		FileNameExtensionFilter filter = new FileNameExtensionFilter(
				"Secondo MM Files", ObjectExport.FILE_EXTENSION,
				ObjectExport.FILE_EXTENSION.toUpperCase());
		fileChooser.setFileFilter(filter);
	}

	/**
	 * Retrieves the single instance of ImportExportGui.
	 * 
	 * @return the singleton
	 */
	public static ImportExportGui getInstance() {
		return instance;
	}

	/**
	 * Processes an export command sent by the menu button.
	 * 
	 * @param sobjects
	 *            The objects selected in the objectlist
	 */
	public void processExportCommand(SecondoObject[] sobjects) {
		if (sobjects.length > 0) {
			String filePath = ImportExportGui.getInstance().getExportFilePath();
			if (filePath == ImportExportGui.NO_FILE) {
				return;
			}
			if (sobjects.length > 0) {
				try {
					long startTime = System.currentTimeMillis();
					ObjectExport.getInstance()
							.exportObjects(sobjects, filePath);
					long duration = System.currentTimeMillis() - startTime;
					System.out.println("Took (ms): " + duration);
				} catch (ExportException e) {
					Reporter.showError(e.getMessage());
				}
			}
		}
	}

	/**
	 * Processes an import command sent by the menu button.
	 * 
	 * @param objectList
	 */
	public void processImportCommand(ObjectList objectList) {
		String filePath = ImportExportGui.getInstance().getImportFilePath();
		if (filePath == ImportExportGui.NO_FILE) {
			return;
		}
		try {
			long startTime = System.currentTimeMillis();
			SecondoObject[] sobjects = ObjectImport.getInstance()
					.importObjects(filePath);
			for (SecondoObject sobject : sobjects) {
				objectList.addEntry(sobject);
			}
			long duration = System.currentTimeMillis() - startTime;
			System.out.println("Took (ms): " + duration);
		} catch (ImportException e) {
			Reporter.showError(e.getMessage());
		}
	}

	/**
	 * Opens up a Filechooser opendialog to retrieve a filepath to open.
	 * 
	 * @return the filepath selected in the filechooser or {@value #NO_FILE} if
	 *         none was selected.
	 */
	public String getImportFilePath() {
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
	public String getExportFilePath() {
		int retVal = fileChooser.showSaveDialog(null);
		if (retVal == JFileChooser.APPROVE_OPTION) {
			File selectedFile = fileChooser.getSelectedFile();
			if (selectedFile != null) {
				return selectedFile.getAbsolutePath();
			}
		}
		return NO_FILE;
	}

}
