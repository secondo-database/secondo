package mmdb.service;

import gui.SecondoObject;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

import javax.swing.SwingWorker;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.convert.ConversionException;
import mmdb.error.convert.ConvertToListException;
import mmdb.error.inout.ExportException;
import mmdb.gui.ImportExportGui;
import mmdb.streamprocessing.parser.Environment;
import sj.lang.ListExpr;

/**
 * A SwingWorker to handle export of MemoryObjects.
 * 
 * @author Bjoern Clasen
 */
public class ObjectExport extends SwingWorker<Void, Integer> {

	/**
	 * The line that every export file starts with.
	 */
	public static final String FILE_HEADER = "SECONDO MM-Object Export File v1.0";

	/**
	 * The section separator for export files.
	 */
	public static final String SEPARATOR = "--------------------";

	/**
	 * The String an attribute header starts with.
	 */
	public static final String ATTRIBUTE = "ATTR";

	/**
	 * The String a relation header starts with.
	 */
	public static final String RELATION = "REL";

	/**
	 * An opening parenthesis.
	 */
	public static final String OPEN = "(";

	/**
	 * A closing parenthesis.
	 */
	public static final String CLOSE = ")";

	/**
	 * The newline representation in export files.
	 */
	public static final String NEWLINE = "\n";

	/**
	 * A colon.
	 */
	public static final String COLON = ":";

	/**
	 * String for "no tuples"
	 */
	public static final String NO_TUPLES = "---";

	/**
	 * The file extension for export files.
	 */
	public static final String FILE_EXTENSION = "secmm";

	/**
	 * The newline character in int representation.
	 */
	public static final int NEWLINE_INT = 10;

	/**
	 * Minimum number of relation tuples for granular percent calculation
	 */
	public static final int PERCENT_MIN = 1000;

	/**
	 * The SecondoObjects to put out
	 */
	private SecondoObject[] sobjects;

	/**
	 * The path to write export file to
	 */
	private String filePath;

	/**
	 * Current GUI instance to update progress
	 */
	private ImportExportGui importExportGui;

	/**
	 * Calculated object headers
	 */
	private String[][] objectHeaders;

	/**
	 * Constructor for ObjectExport.
	 * 
	 * @param sobjects
	 *            the SecondoObjects to put out.
	 * @param filePath
	 *            the path to write export file to.
	 * @param importExportGui
	 *            reference to current GUI object.
	 */
	public ObjectExport(SecondoObject[] sobjects, String filePath,
			ImportExportGui importExportGui) {
		this.sobjects = sobjects;
		this.filePath = filePath;
		this.importExportGui = importExportGui;
	}

	/**
	 * Checks if all SecondoObjects are exportable. Throws
	 * {@link ExportException} if not.
	 * 
	 * @param sobjects
	 *            The SecondoObjects to check.
	 * @throws ExportException
	 */
	public static void checkObjectsExportable(SecondoObject[] sobjects)
			throws ExportException {
		ArrayList<SecondoObject> noMemory = new ArrayList<SecondoObject>();
		ArrayList<SecondoObject> notAttributeOrRelation = new ArrayList<SecondoObject>();
		for (SecondoObject sobject : sobjects) {
			if (sobject.getMemoryObject() == null) {
				noMemory.add(sobject);
				continue;
			}
			if (!(sobject.getMemoryObject() instanceof MemoryAttribute || sobject
					.getMemoryObject() instanceof MemoryRelation)) {
				notAttributeOrRelation.add(sobject);
			}
		}
		if (!(noMemory.isEmpty() && notAttributeOrRelation.isEmpty())) {
			String errorMessage = "Could not start export.\n";
			if (!noMemory.isEmpty()) {
				errorMessage += "The following objects are not in main memory format:\n";
				for (SecondoObject sobject : noMemory) {
					errorMessage += sobject.getName();
					errorMessage += "\n";
				}
			}
			if (!notAttributeOrRelation.isEmpty()) {
				errorMessage += "\nThe following objects are neither attributes nor relations:\n";
				for (SecondoObject sobject : notAttributeOrRelation) {
					errorMessage += sobject.getName();
					errorMessage += "\n";
				}
			}
			throw new ExportException(errorMessage);
		}
	}

	/**
	 * Returns the object headers for all SecondoObjects to export.
	 * 
	 * @param sobjects
	 *            the SecondoObjects to export.
	 * @return a String array containing for each SecondoObject:<br>
	 *         0 = type ({@link ObjectExport#ATTRIBUTE} /
	 *         {@link ObjectExport#RELATION})<br>
	 *         1 = object's name<br>
	 *         2 = tuple count (or {@link ObjectExport#NO_TUPLES} for
	 *         attributes)<br>
	 *         3 = "" (empty, for progress)
	 */
	public String[][] getObjectHeaders() {
		if (this.objectHeaders == null) {
			this.objectHeaders = createObjectHeaders();
		}
		return this.objectHeaders;
	}

	/**
	 * Creates the object headers for all {@link ObjectExport#sobjects
	 * SecondoObjects}.
	 * 
	 * @return the headers in a String[][].
	 * @see ObjectExport#getObjectHeaders() getObjectHeaders()
	 */
	private String[][] createObjectHeaders() {
		String[][] retVal = new String[sobjects.length][4];
		for (int i = 0; i < sobjects.length; i++) {
			SecondoObject sobject = sobjects[i];
			if (sobject.getMemoryObject() instanceof MemoryAttribute) {
				retVal[i][1] = ATTRIBUTE;
				retVal[i][2] = NO_TUPLES;
				retVal[i][3] = "";
			} else if (sobject.getMemoryObject() instanceof MemoryRelation) {
				int numTuples = ((MemoryRelation) sobject.getMemoryObject())
						.getTuples().size();
				retVal[i][1] = RELATION;
				retVal[i][2] = Integer.toString(numTuples);
				retVal[i][3] = "";
			}
			String objectName = sobject.getName();
			objectName = removeNameMetadata(objectName);
			retVal[i][0] = objectName;
		}
		return retVal;
	}

	/**
	 * {@inheritDoc}<br>
	 * <br>
	 * Exports the {@link ObjectExport#sobjects SecondoObjects} to a file under
	 * {@link ObjectExport#filePath filePath}.
	 * 
	 * @throws ExportException
	 *             if export fails
	 * @throws InterruptedException
	 */
	@Override
	public Void doInBackground() throws ExportException, InterruptedException {
		OutputStream outputStream = getOutputStream(filePath);

		try {
			writeFileHeader(outputStream);

			if (this.objectHeaders == null) {
				this.objectHeaders = createObjectHeaders();
			}
			writeObjectHeaders(outputStream);

			// Print Objects
			for (int i = 0; i < sobjects.length; i++) {
				failIfInterrupted();
				SecondoObject sobject = sobjects[i];
				setProgress(0);
				try {
					if (sobject.getMemoryObject() instanceof MemoryAttribute) {
						writeAttribute(sobject.getMemoryObject(), outputStream);
					} else if (sobject.getMemoryObject() instanceof MemoryRelation) {
						writeRelation(sobject.getMemoryObject(), outputStream);
					}
				} catch (ConversionException e) {
					throw new ExportException(String.format(
							"Could not convert object '%s': %s",
							sobject.getName(), e.getMessage()));
				}
				outputStream.write((SEPARATOR + NEWLINE).getBytes());
				setProgress(100);
				publish(i);
			}
		} catch (IOException e) {
			throw new ExportException("Problems with writing file. Aborting...");
		} finally {
			closeOutputStream(outputStream);
		}
		return null; // Void
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	protected void process(final List<Integer> chunks) {
		this.importExportGui.processProgressUpdate(chunks);
	}

	/**
	 * Throws a {@link InterruptedException} if the procession has been
	 * cancelled via the GUI.<br>
	 * Also deletes the (incomplete, unstable state) export file.
	 * 
	 * @throws InterruptedException
	 */
	private void failIfInterrupted() throws InterruptedException {
		if (Thread.currentThread().isInterrupted()) {
			new File(this.filePath).delete();
			throw new InterruptedException("Interrupted while exporting...");
		}
	}

	/**
	 * Writes the file header to the given OutputStream.
	 * 
	 * @param outputStream
	 *            The OutputStream to write to.
	 * @throws IOException
	 */
	private void writeFileHeader(OutputStream outputStream) throws IOException {
		outputStream.write((FILE_HEADER + NEWLINE).getBytes());
		outputStream.write((SEPARATOR + NEWLINE).getBytes());
	}

	/**
	 * Writes the current {@link ObjectExport#objectHeaders object headers} to
	 * the output stream.
	 * 
	 * @param outputStream
	 *            the stream to write to.
	 * @throws IOException
	 */
	private void writeObjectHeaders(OutputStream outputStream)
			throws IOException {
		for (String[] objectHeader : this.objectHeaders) {
			outputStream.write(objectHeader[1].getBytes());
			outputStream.write(COLON.getBytes());
			if (!objectHeader[2].equals(NO_TUPLES)) {
				outputStream.write((objectHeader[2]).getBytes());
				outputStream.write(COLON.getBytes());
			}
			outputStream.write(objectHeader[0].getBytes());
			outputStream.write(NEWLINE.getBytes());
		}
		outputStream.write((SEPARATOR + NEWLINE).getBytes());
	}

	/**
	 * Writes a MemoryAttribute to the OutputStream.
	 * 
	 * @param memoryObject
	 *            The MemoryAttribute to export.
	 * @param outputStream
	 *            The OutputStream to write to.
	 * @throws IOException
	 * @throws ConversionException
	 */
	private void writeAttribute(MemoryObject memoryObject,
			OutputStream outputStream) throws IOException, ConversionException {
		ListExpr typename = ListExpr.symbolAtom(MemoryAttribute
				.getTypeName(memoryObject.getClass()));
		ListExpr value = ((MemoryAttribute) memoryObject).toList();
		ListExpr resultList = ListExpr.twoElemList(typename, value);
		String output = removeLeadingTrailingNewline(resultList);
		outputStream.write(escapeString(output).getBytes());
		outputStream.write(NEWLINE.getBytes());
	}

	/**
	 * Writes a MemoryRelation to the OutputStream.<br>
	 * Potentially refreshes Progressbar in GUI.
	 * 
	 * @param memoryObject
	 *            The MemoryRelation to export.
	 * @param outputStream
	 *            The OutputStream to write to.
	 * @throws IOException
	 * @throws ConversionException
	 * @throws InterruptedException
	 */
	private void writeRelation(MemoryObject memoryObject,
			OutputStream outputStream) throws ConversionException, IOException,
			InterruptedException {
		MemoryRelation memoryRelation = (MemoryRelation) memoryObject;

		outputStream.write((OPEN + NEWLINE).getBytes());
		// Relation Header
		List<RelationHeaderItem> header = memoryRelation.getHeader();
		ListExpr headerList = ObjectConverter.getInstance().createHeaderList(
				header);
		String headerOutput = removeLeadingTrailingNewline(headerList);
		outputStream.write(escapeString(headerOutput).getBytes());
		outputStream.write(NEWLINE.getBytes());

		// Relation Tuples
		outputStream.write((OPEN + NEWLINE).getBytes());
		List<MemoryTuple> tuples = memoryRelation.getTuples();

		// Progressbar
		int totalTuples = tuples.size();
		int onePercent = (int) Math.floor(totalTuples / 100);

		// Export each tuple
		for (int i = 0; i < tuples.size(); i++) {
			failIfInterrupted();
			MemoryTuple tuple = tuples.get(i);
			ListExpr tupleList = new ListExpr();
			for (int f = 0; f < tuple.getAttributes().size(); f++) {
				try {
					ListExpr attributeList = tuple.getAttribute(f).toList();
					tupleList = ListExpr.concat(tupleList, attributeList);
				} catch (ConversionException e) {
					throw new ConvertToListException(
							"-> Could not create nested list for attribute '"
									+ header.get(f).getIdentifier()
									+ "' of type '"
									+ header.get(f).getTypeName() + "'.");
				}
			}
			String tupleOutput = removeLeadingTrailingNewline(tupleList);
			outputStream.write(escapeString(tupleOutput).getBytes());
			if (i != tuples.size() - 1) {
				outputStream.write(NEWLINE.getBytes());
			}

			// Progressbar
			if (totalTuples >= PERCENT_MIN && i % onePercent == 0) {
				int curPercent = i / onePercent;
				if (curPercent >= 0 && curPercent <= 100) {
					setProgress(curPercent);
				}
			}
		}
		outputStream.write((NEWLINE + CLOSE + NEWLINE).getBytes());
		outputStream.write((CLOSE + NEWLINE).getBytes());
	}

	/**
	 * Removes the main memory indicator from an object name.
	 * 
	 * @param objectName
	 *            The object name to clear from any main memory indicator.
	 * @return The plain object name without an indicator.
	 */
	private static String removeNameMetadata(String objectName) {
		objectName = objectName.trim();
		if (objectName.endsWith(" [++]")) {
			objectName = objectName.substring(0,
					objectName.lastIndexOf(" [++]"));
		}
		if (objectName.endsWith(" [+]")) {
			objectName = objectName
					.substring(0, objectName.lastIndexOf(" [+]"));
		}
		objectName = Environment.removeResultLabel(objectName);
		return objectName;
	}

	/**
	 * Removes leading and trailing newlines from a String. ListExpr tends to
	 * add these.
	 * 
	 * @param listExpr
	 *            the ListExpr to convert to String and then remove newlines.
	 * @return the String representation of the ListExpr without newlines at
	 *         start or end.
	 */
	private static String removeLeadingTrailingNewline(ListExpr listExpr) {
		String outputString = listExpr.toString();
		if (outputString.startsWith(NEWLINE)) {
			outputString = outputString.substring(1);
		}
		if (outputString.endsWith(NEWLINE)) {
			outputString = outputString.substring(0, outputString.length() - 2);
		}
		return outputString;
	}

	/**
	 * Escapes a String for exporting. That means that any characters starting
	 * with '\n' are escaped.
	 * 
	 * @param string
	 *            the String to escape.
	 * @return the escaped String.
	 */
	private static String escapeString(String string) {
		string = string.replace("\\", "\\\\");
		string = string.replace("\n", "\\n");
		string = string.replaceAll(" +", " ");
		return string;
	}

	/**
	 * Creates an OutputStream on the filepath.
	 * 
	 * @param filePath
	 *            the path to the file to write to.
	 * @return the newly created OutputStream
	 * @throws ExportException
	 */
	private OutputStream getOutputStream(String filePath)
			throws ExportException {
		try {
			if (!filePath.toUpperCase().endsWith(
					"." + FILE_EXTENSION.toUpperCase())) {
				filePath += "." + FILE_EXTENSION;
			}
			File destinationFile = new File(filePath);
			FileOutputStream fos = new FileOutputStream(destinationFile);
			ZipOutputStream zout = new ZipOutputStream(fos);
			String contentName = filePath
					.substring(filePath.lastIndexOf(System
							.getProperty("file.separator")) + 1, filePath
							.lastIndexOf("." + FILE_EXTENSION))
					+ ".txt";
			ZipEntry e = new ZipEntry(contentName);
			zout.putNextEntry(e);
			return zout;
		} catch (IOException e) {
			throw new ExportException("Could not open File to write!");
		}
	}

	/**
	 * Closes the OutputStream safely.
	 * 
	 * @param outputStream
	 *            The (Zip-)OutputStream to close.
	 * @throws ExportException
	 */
	private void closeOutputStream(OutputStream outputStream)
			throws ExportException {
		if (outputStream != null) {
			try {
				((ZipOutputStream) outputStream).closeEntry();
				outputStream.close();
			} catch (IOException e) {
				throw new ExportException("Could not close file!");
			}
		}
	}

}
