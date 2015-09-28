package mmdb.service;

import gui.SecondoObject;
import gui.idmanager.IDManager;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.ZipInputStream;

import javax.swing.SwingWorker;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.error.convert.ConversionException;
import mmdb.error.convert.ConvertToObjectException;
import mmdb.error.inout.ImportException;
import mmdb.error.memory.MemoryException;
import mmdb.gui.ImportExportGui;
import mmdb.streamprocessing.parser.Environment;
import sj.lang.ListExpr;

/**
 * A SwingWorker to handle import of MemoryObjects.
 * 
 * @author Bjoern Clasen
 */
public class ObjectImport extends SwingWorker<SecondoObject[], Integer> {

	/**
	 * The object headers of all imported objects.
	 */
	private String[][] objectHeaders;

	/**
	 * The stream of the input file.
	 */
	private InputStream inputStream;

	/**
	 * Current GUI instance to update progress
	 */
	private ImportExportGui importExportGui;

	/**
	 * Constructor for {@link ObjectImport}.<br>
	 * Opens a stream of the given filePath.
	 * 
	 * @param filePath
	 *            the filePath to import from.
	 * @param importExportGui
	 *            current GUI instance.
	 * @throws ImportException
	 *             if no {@link InputStream} could be created from
	 *             {@code filePath}.
	 */
	public ObjectImport(String filePath, ImportExportGui importExportGui)
			throws ImportException {
		this.inputStream = getInputStream(filePath);
		this.importExportGui = importExportGui;
	}

	/**
	 * Technically checks if file is importable. Checks existence, access and
	 * file extension. Throws {@link ImportException} if it is not importable.
	 * 
	 * @param filePath
	 *            The path to the file to check.
	 * @throws ImportException
	 */
	public static void checkFile(String filePath) throws ImportException {
		File file = new File(filePath);
		if (!file.exists()) {
			throw new ImportException("File to import does not exist!");
		}
		if (!file.canRead()) {
			throw new ImportException("File to import cannot be read!");
		}
		if (!file.getName().toUpperCase()
				.endsWith(ObjectExport.FILE_EXTENSION.toUpperCase())) {
			throw new ImportException("File name does not end on \""
					+ ObjectExport.FILE_EXTENSION + "\"!");
		}
	}

	/**
	 * Returns the object headers read from the {@link ObjectImport#inputStream
	 * InputStream}.
	 * 
	 * @return the object headers as String[][]
	 * @throws ImportException
	 * @see {@link ObjectExport#getObjectHeaders()} for details
	 */
	public String[][] getObjectHeaders() throws ImportException {
		if (this.objectHeaders == null) {
			try {
				this.objectHeaders = readObjectHeaders();
			} catch (IOException e) {
				throw new ImportException(
						"Problems with reading file! Aborting...");
			}
		}
		return this.objectHeaders;
	}

	/**
	 * {@inheritDoc}<br>
	 * <br>
	 * Imports all objects found in the {@link ObjectImport#inputStream
	 * InputStream}.
	 * 
	 * @return an array of SecondoObjects containing the imported MemoryObjects.
	 * @throws ImportException
	 * @throws InterruptedException
	 */
	@Override
	public SecondoObject[] doInBackground() throws ImportException,
			InterruptedException {
		SecondoObject[] resultObjects = null;
		try {
			if (this.objectHeaders == null) {
				this.objectHeaders = readObjectHeaders();
			}
			// readFileHeader(inputStream);
			// List<String[]> headers = readObjectHeaders(inputStream);
			List<MemoryObject> mobjects = readObjects();
			if (this.objectHeaders.length != mobjects.size()) {
				throw new ImportException(
						"Number of headers and objects does not match!");
			}
			resultObjects = new SecondoObject[this.objectHeaders.length];
			for (int i = 0; i < this.objectHeaders.length; i++) {
				SecondoObject sobject = new SecondoObject(IDManager.getNextID());
				sobject.setName(addNameMetadata(this.objectHeaders[i][0]));
				sobject.setMemoryObject(mobjects.get(i));
				resultObjects[i] = sobject;
			}
		} catch (IOException e) {
			throw new ImportException("Problems with reading file! Aborting...");
		} catch (MemoryException e) {
			throw new ImportException("Not enough Memory: " + e.getMessage());
		} finally {
			closeInputStream();
		}
		return resultObjects;
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
	 * cancelled via the GUI.
	 * 
	 * @throws InterruptedException
	 */
	private void failIfInterrupted() throws InterruptedException {
		if (Thread.currentThread().isInterrupted()) {
			throw new InterruptedException("Interrupted while exporting...");
		}
	}

	/**
	 * Reads the export file's file header and throws {@link ImportException} if
	 * it's wrong or not present.
	 * 
	 * @param inputStream
	 *            The InputStream to read from.
	 * @throws IOException
	 * @throws ImportException
	 */
	private void readFileHeader() throws IOException, ImportException {
		String line1 = readLine();
		String line2 = readLine();
		if (!(line1.equals(ObjectExport.FILE_HEADER) && line2
				.equals(ObjectExport.SEPARATOR))) {
			throw new ImportException("Invalid file header! Aborting...");
		}
	}

	/**
	 * Reads the object headers from the given InputStream.
	 * 
	 * @return the object headers as String[][].
	 * @throws ImportException
	 * @throws IOException
	 */
	private String[][] readObjectHeaders() throws ImportException, IOException {
		readFileHeader();
		ArrayList<String[]> retValList = new ArrayList<String[]>();
		String line;
		while (!(line = readLine()).equals(ObjectExport.SEPARATOR)) {
			String curLine = line;
			String[] headerArray = new String[4];
			if (curLine.substring(0, curLine.indexOf(ObjectExport.COLON))
					.equals(ObjectExport.ATTRIBUTE)) {
				headerArray[0] = curLine.substring(
						curLine.indexOf(ObjectExport.COLON) + 1,
						curLine.length());
				headerArray[1] = ObjectExport.ATTRIBUTE;
				headerArray[2] = ObjectExport.NO_TUPLES;
				headerArray[3] = "";
			} else if (curLine
					.substring(0, curLine.indexOf(ObjectExport.COLON)).equals(
							ObjectExport.RELATION)) {
				curLine = curLine
						.substring(curLine.indexOf(ObjectExport.COLON) + 1);
				headerArray[0] = curLine.substring(
						curLine.indexOf(ObjectExport.COLON) + 1,
						curLine.length());
				headerArray[1] = ObjectExport.RELATION;
				String tupleCount = curLine.substring(0,
						curLine.indexOf(ObjectExport.COLON));
				if (!isStringInt(tupleCount)) {
					throw new ImportException(
							"Found invalid value for tuple count: "
									+ tupleCount);
				}
				headerArray[2] = tupleCount;
				headerArray[3] = "";
			} else {
				throw new ImportException("Invalid Header: " + line);
			}
			retValList.add(headerArray);
		}
		String[][] retVal = new String[retValList.size()][4];
		for (int i = 0; i < retValList.size(); i++) {
			retVal[i] = retValList.get(i);
		}
		return retVal;
	}

	/**
	 * Reads the MemoryObjects from the given InputStream.
	 * 
	 * @return a list of imported MemoryObjects.
	 * @throws ImportException
	 * @throws IOException
	 * @throws MemoryException
	 * @throws InterruptedException
	 */
	private List<MemoryObject> readObjects() throws ImportException,
			IOException, MemoryException, InterruptedException {
		ArrayList<MemoryObject> retVal = new ArrayList<MemoryObject>();
		String line;
		int objectIndex = 0;
		while ((line = readLine()) != null) {
			failIfInterrupted();
			setProgress(0);
			try {
				if (!ObjectExport.OPEN.equals(line)) {
					retVal.add(readAttribute(line));
					line = readLine();
					if (!ObjectExport.SEPARATOR.equals(line)) {
						throw new ImportException(
								"Error in import-file: Seperator expected after Object definition");
					}
				} else {
					retVal.add(readRelation(objectIndex));
				}
			} catch (ConversionException e) {
				throw new ImportException("Could not read object "
						+ (objectIndex + 1));
			}
			publish(objectIndex);
			objectIndex++;
			setProgress(100);
		}
		return retVal;
	}

	/**
	 * Reads a MemoryAttribute from a line String.
	 * 
	 * @param line
	 *            the line representing a MemoryAttribute.
	 * @return the MemoryAttribute
	 * @throws ImportException
	 * @throws ConversionException
	 * @throws MemoryException
	 */
	private MemoryObject readAttribute(String line) throws ImportException,
			ConversionException, MemoryException {
		line = unescapeString(line);
		ListExpr attrExpr = new ListExpr();
		attrExpr.readFromString(line);
		return ObjectConverter.getInstance().convertListToObject(attrExpr);
	}

	/**
	 * Reads a MemoryRelation from the {@link ObjectImport#inputStream
	 * InputStream}.
	 *
	 * @param objectIndex
	 *            the MemoryRelation's index in all objects.
	 * @return the MemoryRelation that has been read.
	 * @throws ImportException
	 * @throws IOException
	 * @throws ConversionException
	 * @throws MemoryException
	 * @throws InterruptedException
	 */
	private MemoryRelation readRelation(int objectIndex)
			throws ImportException, IOException, ConversionException,
			MemoryException, InterruptedException {
		int numTuples = Integer.parseInt(this.objectHeaders[objectIndex][2]);
		MemoryRelation resultRelation;

		// Header
		String line = readLine();
		line = unescapeString(line);
		ListExpr headerList = new ListExpr();
		headerList.readFromString(line);
		List<RelationHeaderItem> header = ObjectConverter.getInstance()
				.createHeaderFromList(ListExpr.oneElemList(headerList));
		resultRelation = new MemoryRelation(header);
		line = readLine();
		if (!ObjectExport.OPEN.equals(line)) {
			throw new ImportException("Expected '(' after header!");
		}

		// Tuples
		List<MemoryTuple> tuples = new ArrayList<MemoryTuple>();
		if (numTuples < ObjectExport.PERCENT_MIN) {
			tuples.addAll(readTuples(numTuples, objectIndex, header));
		} else {
			int chunkSize = (int) Math.floor(numTuples / 100);
			for (int i = 0; i < 100; i++) {
				failIfInterrupted();
				tuples.addAll(readTuples(chunkSize, objectIndex, header));
				setProgress(i);
				MemoryWatcher.getInstance().checkMemoryStatus();
			}
			if (100 * chunkSize < numTuples) {
				tuples.addAll(readTuples(numTuples - 100 * chunkSize,
						objectIndex, header));
			}
		}
		if (!ObjectExport.CLOSE.equals(readLine())) {
			throw new ImportException(String.format(
					"Too many tuples in object %d! Header announced %d...",
					objectIndex + 1, numTuples));
		}
		if (!(ObjectExport.CLOSE.equals(readLine()) && (ObjectExport.SEPARATOR
				.equals(readLine())))) {
			throw new ImportException(String.format(
					"End of object %d (relation) is not valid!",
					objectIndex + 1));
		}
		resultRelation.setTuples(tuples);
		return resultRelation;
	}

	/**
	 * Reads a list of {@code amount} tuples from the
	 * {@link ObjectImport#inputStream InputStream}.
	 * 
	 * @param amount
	 *            how many tuples(=lines) to read.
	 * @param objectIndex
	 *            the index of the current relation in all import objects.
	 * @param tmpHeader
	 *            the header of the tuples to read from a temp relation object.
	 * @return all read MemoryTuples in a List
	 * @throws IOException
	 * @throws ImportException
	 * @throws ConvertToObjectException
	 */
	private List<MemoryTuple> readTuples(int amount, int objectIndex,
			List<RelationHeaderItem> tmpHeader) throws IOException,
			ImportException, ConvertToObjectException {
		MemoryRelation tmpRelation = new MemoryRelation(tmpHeader);
		StringBuilder builder = new StringBuilder();
		for (int i = 0; i < amount; i++) {
			String line = readLine();
			line = unescapeString(line);
			if (ObjectExport.CLOSE.equals(line)) {
				throw new ImportException(String.format(
						"Not enough tuples in object %d!", objectIndex + 1));
			}
			builder.append(line);
		}
		ListExpr tuplesExpr = new ListExpr();
		tuplesExpr.readFromString(ObjectExport.OPEN + builder.toString()
				+ ObjectExport.CLOSE);
		while (!tuplesExpr.isEmpty()) {
			tmpRelation.createTupleFromList(tuplesExpr.first());
			tuplesExpr = tuplesExpr.rest();
		}
		return tmpRelation.getTuples();
	}

	/**
	 * Adds the main memory indicator to an object name.
	 * 
	 * @param name
	 *            the original name to extend.
	 * @return the original name with appended main memory indicator.
	 */
	private String addNameMetadata(String name) {
		name = Environment.nextResultLabel() + name;
		name += " [+]";
		return name;
	}

	/**
	 * Unescapes a String read from an export files. So it does the opposite of
	 * {@link ObjectExport#escapeString}.
	 * 
	 * @param string
	 *            the String to unescape.
	 * @return the unescaped String.
	 */
	private String unescapeString(String string) {
		StringBuilder sb = new StringBuilder();
		char[] byteArr = string.toCharArray();
		for (int i = 0; i < byteArr.length; i++) {
			if (byteArr[i] == '\\') {
				if (i == byteArr.length - 1) {
					throw new RuntimeException(
							"Invalid input file! '\\' not tolerated at EOL!");
				}
				i++;
				if (byteArr[i] == 'n') {
					sb.append("\n");
				} else if (byteArr[i] == '\\') {
					sb.append("\\");
				} else {
					throw new RuntimeException(
							"Only '\\n' is allowed in input!");
				}
			} else {
				sb.append(byteArr[i]);
			}
		}
		return sb.toString();
	}

	/**
	 * Reads a line from the given InputStream.
	 * 
	 * @return the next line (until next newline) in the InputStream.
	 * @throws IOException
	 */
	private String readLine() throws IOException {
		StringBuilder sb = new StringBuilder();
		int content;
		while ((content = inputStream.read()) != ObjectExport.NEWLINE_INT) {
			if (content == -1) {
				return null;
			}
			sb.append((char) content);
		}
		return sb.toString().trim();
	}

	/**
	 * Opens a (Zip-)InputStream to the file path.
	 * 
	 * @param filePath
	 *            the file path to read from.
	 * @return the (Zip-)InputStream.
	 * @throws ImportException
	 */
	public InputStream getInputStream(String filePath) throws ImportException {
		try {
			File sourceFile = new File(filePath);
			FileInputStream fis = new FileInputStream(sourceFile);
			ZipInputStream zis = new ZipInputStream(fis);
			zis.getNextEntry();
			return zis;
		} catch (IOException e) {
			throw new ImportException("Could not open File to read!");
		}
	}

	/**
	 * Safely closes the given InputStream.
	 * 
	 * @throws ImportException
	 */
	private void closeInputStream() throws ImportException {
		if (inputStream != null) {
			try {
				((ZipInputStream) inputStream).closeEntry();
				inputStream.close();
			} catch (IOException e) {
				throw new ImportException("Could not close file!");
			}
		}
	}

	/**
	 * Simply determines if a String is a valid Integer.
	 * 
	 * @param string
	 * @return
	 */
	private boolean isStringInt(String string) {
		try {
			Integer.parseInt(string);
		} catch (NumberFormatException e) {
			return false;
		}
		return true;
	}

}
