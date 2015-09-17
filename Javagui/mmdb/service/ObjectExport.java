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

import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.convert.ConversionException;
import mmdb.error.convert.ConvertToListException;
import mmdb.error.inout.ExportException;
import mmdb.streamprocessing.parser.Environment;
import sj.lang.ListExpr;

/**
 * A singleton class to handle export of MemoryObjects.
 * 
 * @author Björn Clasen
 */
public class ObjectExport {

	/**
	 * The singleton instance.
	 */
	private static ObjectExport instance = new ObjectExport();

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
	 * The file extension for export files.
	 */
	public static final String FILE_EXTENSION = "secmm";

	/**
	 * The newline character in int representation.
	 */
	public static final int NEWLINE_INT = 10;

	/**
	 * Privat constructor for singleton.
	 */
	private ObjectExport() {
	}

	/**
	 * Retrieves the singleton instance.
	 * 
	 * @return the singleton.
	 */
	public static ObjectExport getInstance() {
		return instance;
	}

	/**
	 * Expots the given SecondoObjects to a file under filePath.
	 * 
	 * @param sobjects
	 *            All SecondoObjects to be exported.
	 * @param filePath
	 *            The file path of the destination file.
	 * @throws ExportException
	 */
	public void exportObjects(SecondoObject[] sobjects, String filePath)
			throws ExportException {
		checkExportable(sobjects);
		OutputStream outputStream = getOutputStream(filePath);

		try {
			// Print File Header
			writeFileHeader(outputStream);
			outputStream.write((SEPARATOR + NEWLINE).getBytes());

			// Print Object Headers
			for (SecondoObject sobject : sobjects) {
				writeObjectHeader(sobject, outputStream);
			}
			outputStream.write((SEPARATOR + NEWLINE).getBytes());

			// Print Objects
			for (SecondoObject sobject : sobjects) {
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
			}
		} catch (IOException e) {
			throw new ExportException("Problems with writing file. Aborting...");
		} finally {
			closeOutputStream(outputStream);
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
	}

	/**
	 * Checks if all SecondoObjects are exportable. Throws
	 * {@link ExportException} if not.
	 * 
	 * @param sobjects
	 *            The SecondoObjects to check.
	 * @throws ExportException
	 */
	private void checkExportable(SecondoObject[] sobjects)
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
	 * Writes the header for a SecondoObject to the OutputStream.
	 * 
	 * @param sobject
	 *            The SecondoObject to write a header for.
	 * @param outputStream
	 *            The OutputStream to write the header to.
	 * @throws ExportException
	 * @throws IOException
	 */
	private void writeObjectHeader(SecondoObject sobject,
			OutputStream outputStream) throws ExportException, IOException {
		if (sobject.getMemoryObject() instanceof MemoryAttribute) {
			outputStream.write((ATTRIBUTE + COLON).getBytes());
		} else if (sobject.getMemoryObject() instanceof MemoryRelation) {
			int numTuples = ((MemoryRelation) sobject.getMemoryObject())
					.getTuples().size();
			outputStream.write((RELATION + COLON).getBytes());
			outputStream.write((numTuples + COLON).getBytes());
		}
		String objectName = sobject.getName();
		objectName = removeNameMetadata(objectName);
		outputStream.write(objectName.getBytes());
		outputStream.write(NEWLINE.getBytes());
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
	 * Writes a MemoryRelation to the OutputStream.
	 * 
	 * @param memoryObject
	 *            The MemoryRelation to export.
	 * @param outputStream
	 *            The OutputStream to write to.
	 * @throws IOException
	 * @throws ConversionException
	 */
	private void writeRelation(MemoryObject memoryObject,
			OutputStream outputStream) throws ConversionException, IOException {
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
		for (int i = 0; i < tuples.size(); i++) {
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
	private String removeNameMetadata(String objectName) {
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
	private String removeLeadingTrailingNewline(ListExpr listExpr) {
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
	private String escapeString(String string) {
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
			if (!filePath.toUpperCase().endsWith(FILE_EXTENSION.toUpperCase())) {
				filePath += FILE_EXTENSION;
			}
			File destinationFile = new File(filePath);
			FileOutputStream fos = new FileOutputStream(destinationFile);
			ZipOutputStream zout = new ZipOutputStream(fos);
			ZipEntry e = new ZipEntry("content.txt");
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
