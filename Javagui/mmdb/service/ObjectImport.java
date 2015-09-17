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

import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.error.convert.ConversionException;
import mmdb.error.inout.ImportException;
import mmdb.error.memory.MemoryException;
import mmdb.streamprocessing.parser.Environment;
import sj.lang.ListExpr;

/**
 * A singleton class to handle import of MemoryObjects.
 * 
 * @author Björn Clasen
 */
public class ObjectImport {

	/**
	 * The singleton instance.
	 */
	private static ObjectImport instance = new ObjectImport();

	/**
	 * Privat constructor for singleton.
	 */
	private ObjectImport() {
	}

	/**
	 * Retrieves the singleton instance.
	 * 
	 * @return the singleton.
	 */
	public static ObjectImport getInstance() {
		return instance;
	}

	/**
	 * Imports all objects found in the given export file under filePath.
	 * 
	 * @param filePath
	 *            path to the file to import.
	 * @return an array of SecondoObjects containing the imported MemoryObjects.
	 * @throws ImportException
	 */
	public SecondoObject[] importObjects(String filePath)
			throws ImportException {
		SecondoObject[] resultObjects = null;
		InputStream inputStream = getInputStream(filePath);
		try {
			readFileHeader(inputStream);
			List<String[]> headers = readObjectHeaders(inputStream);
			List<MemoryObject> mobjects = readObjects(inputStream);
			if (headers.size() != mobjects.size()) {
				throw new ImportException(
						"Number of headers and objects does not match!");
			}
			resultObjects = new SecondoObject[headers.size()];
			for (int i = 0; i < headers.size(); i++) {
				SecondoObject sobject = new SecondoObject(IDManager.getNextID());
				sobject.setName(addNameMetadata(headers.get(i)[0]));
				sobject.setMemoryObject(mobjects.get(i));
				resultObjects[i] = sobject;
			}
		} catch (IOException e) {
			throw new ImportException("Problems with reading file! Aborting...");
		} catch (MemoryException e) {
			throw new ImportException("Not enough Memory: " + e.getMessage());
		} finally {
			closeInputStream(inputStream);
		}
		return resultObjects;
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
	private void readFileHeader(InputStream inputStream) throws IOException,
			ImportException {
		String line1 = readLine(inputStream);
		String line2 = readLine(inputStream);
		if (!(line1.equals(ObjectExport.FILE_HEADER) && line2
				.equals(ObjectExport.SEPARATOR))) {
			throw new ImportException("Invalid file header! Aborting...");
		}
	}

	/**
	 * Reads the MemoryObjects from the given InputStream.
	 * 
	 * @param inputStream
	 *            the InputStream to read from.
	 * @return a list of imported MemoryObjects.
	 * @throws ImportException
	 * @throws IOException
	 * @throws MemoryException
	 */
	private List<MemoryObject> readObjects(InputStream inputStream)
			throws ImportException, IOException, MemoryException {
		ArrayList<MemoryObject> retVal = new ArrayList<MemoryObject>();
		String line;
		int objectNumber = 1;
		while ((line = readLine(inputStream)) != null) {
			try {
				if (!ObjectExport.OPEN.equals(line)) {
					retVal.add(readAttribute(line));
					line = readLine(inputStream);
					if (!ObjectExport.SEPARATOR.equals(line)) {
						throw new ImportException(
								"Error in import-file: Seperator expected after Object definition");
					}
				} else {
					retVal.add(readRelation(inputStream));
				}
			} catch (ConversionException e) {
				throw new ImportException("Could not read object "
						+ objectNumber);
			}
			objectNumber++;
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
	 * Reads a MemoryRelation from the given InputStream.
	 * 
	 * @param inputStream
	 *            the InputStream to read from.
	 * @return the MemoryRelation that has been read.
	 * @throws ImportException
	 * @throws IOException
	 * @throws ConversionException
	 * @throws MemoryException
	 */
	private MemoryRelation readRelation(InputStream inputStream)
			throws ImportException, IOException, ConversionException,
			MemoryException {
		StringBuilder relationString = new StringBuilder();
		relationString.append("(");
		String line = "";
		while (!ObjectExport.SEPARATOR.equals((line = readLine(inputStream)))) {
			relationString.append(unescapeString(line));
		}
		ListExpr tupleList = new ListExpr();
		tupleList.readFromString(relationString.toString());
		return (MemoryRelation) ObjectConverter.getInstance()
				.convertListToObject(tupleList);
	}

	/**
	 * Reads the object headers from the given InputStream.
	 * 
	 * @param inputStream
	 *            The InputStream to read from.
	 * @return a list of String-Arrays contains the info from the headers.
	 * @throws ImportException
	 * @throws IOException
	 */
	private List<String[]> readObjectHeaders(InputStream inputStream)
			throws ImportException, IOException {
		ArrayList<String[]> retVal = new ArrayList<String[]>();
		String line;
		while (!(line = readLine(inputStream)).equals(ObjectExport.SEPARATOR)) {
			String curLine = line;
			String[] headerArray;
			if (curLine.substring(0, curLine.indexOf(ObjectExport.COLON))
					.equals(ObjectExport.ATTRIBUTE)) {
				headerArray = new String[1];
				headerArray[0] = curLine.substring(
						curLine.indexOf(ObjectExport.COLON) + 1,
						curLine.length());
			} else if (curLine
					.substring(0, curLine.indexOf(ObjectExport.COLON)).equals(
							ObjectExport.RELATION)) {
				headerArray = new String[2];
				curLine = curLine
						.substring(curLine.indexOf(ObjectExport.COLON) + 1);
				headerArray[0] = curLine.substring(
						curLine.indexOf(ObjectExport.COLON) + 1,
						curLine.length());
				headerArray[1] = curLine.substring(0,
						curLine.indexOf(ObjectExport.COLON));
			} else {
				throw new ImportException("Invalid Header: " + line);
			}
			retVal.add(headerArray);
		}
		return retVal;
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
	 * {@link mmdb.service.ObjectExport#escapeString}.
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
	 * @param inputStream
	 *            The InputStream to read from.
	 * @return the next line (until next newline) in the InputStream.
	 * @throws IOException
	 */
	private String readLine(InputStream inputStream) throws IOException {
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
		checkFile(filePath);
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
	 * @param inputStream
	 *            The (Zip-)InputStream to close.
	 * @throws ImportException
	 */
	private void closeInputStream(InputStream inputStream)
			throws ImportException {
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
	 * Technically checks if file is importable. Checks existance, access and
	 * file extension. Throws {@link ImportException} if it is not importable.
	 * 
	 * @param filePath
	 *            The path to the file to check.
	 * @throws ImportException
	 */
	private void checkFile(String filePath) throws ImportException {
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

}
