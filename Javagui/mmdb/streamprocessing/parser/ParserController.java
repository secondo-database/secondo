package mmdb.streamprocessing.parser;

import gui.CommandPanel;
import gui.ObjectList;
import gui.SecondoObject;
import gui.ViewerControl;
import gui.idmanager.IDManager;

import java.math.RoundingMode;
import java.text.DecimalFormat;
import java.util.List;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.MMDBException;
import mmdb.error.convert.ConvertToListException;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.InvalidQueryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TransformQueryException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.service.ObjectConverter;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import sj.lang.ESInterface;
import sj.lang.IntByReference;
import sj.lang.ListExpr;

/**
 * A singleton class controlling the parse process during query procession.<br>
 * 
 * @author Bjoern Clasen
 */
public class ParserController {

	/**
	 * Boolean indicating if query results are to be directly transformed to
	 * nested list format.
	 */
	private boolean autoconvert = true;

	/**
	 * The current GUI's ObjectList (being injected).
	 */
	private ObjectList objectList;

	/**
	 * The current GUI's CommandPanel (being injected).
	 */
	private CommandPanel commandPanel;

	/**
	 * The current GUI's ViewerContol (being injected).
	 */
	private ViewerControl viewerControl;

	/**
	 * The String prefix of an answer to an invalid query sent to the core for
	 * transformation.
	 */
	private static final String invalidQueryString = "INVALID QUERY ";

	/**
	 * The frame an main memory query is to be embedded in to be transformed by
	 * the core.
	 */
	private static final String q2lQueryString = "query query2list('%s');";

	/**
	 * The singleton instance.
	 */
	private static ParserController instance = new ParserController();

	/**
	 * Private constructor for singleton.
	 */
	private ParserController() {
	}

	/**
	 * Retrieves the singleton instance.
	 * 
	 * @return the singleton.
	 */
	public static ParserController getInstance() {
		return instance;
	}

	/**
	 * Injects the current GUI elements to the ParserController.
	 * 
	 * @param objectList
	 *            the current GUI's ObjectList
	 * @param commandPanel
	 *            the current GUI's CommandPanel
	 * @param viewerControl
	 *            the current GUI's ViewerControl
	 */
	public void injectGuiElements(ObjectList objectList,
			CommandPanel commandPanel, ViewerControl viewerControl) {
		this.objectList = objectList;
		this.commandPanel = commandPanel;
		this.viewerControl = viewerControl;
	}

	/**
	 * Toggles the autoconvert option to directly convert query results to
	 * nested list format.
	 */
	public void processResultAutoconvert() {
		this.autoconvert = !this.autoconvert;
	}

	/**
	 * Processes a command in nested list or standard query format.<br>
	 * Converts the query to nested list format if necessary.<br>
	 * Adds the result to the GUI's {@link ObjectList} and prints occuring
	 * errors to the {@link CommandPanel}.
	 * 
	 * @param command
	 * @param secondoInterface
	 */
	public void processMMDBQuery(String command, ESInterface secondoInterface) {
		long startTime = System.currentTimeMillis();
		long calcTime;
		String commandContent = extractContent(command);

		if (!commandContent.startsWith("(")) {
			try {
				commandContent = translateQueryToNL(commandContent,
						secondoInterface);
			} catch (MMDBException e) {
				handleTranslateError(e);
				return;
			}
		}

		List<SecondoObject> existingObjects = objectList.getAllObjects();
		ObjectNode resultObjectNode;
		MemoryObject resultMemoryObject;
		try {
			resultObjectNode = NestedListProcessor.buildOperatorTree(
					commandContent, existingObjects);
			resultObjectNode.typeCheck();
			resultMemoryObject = resultObjectNode.getResult();
			calcTime = System.currentTimeMillis();
		} catch (MMDBException e) {
			handleGetResultException(e);
			return;
		}

		SecondoObject resultSecondoObject = new SecondoObject(
				IDManager.getNextID());
		if (this.autoconvert || resultMemoryObject == null) {
			try {
				ListExpr resultExpr = null;
				if (isUndefinedResult(resultObjectNode, resultMemoryObject)) {
					resultExpr = getUndefinedResult(resultObjectNode);
				} else {
					resultExpr = ObjectConverter.getInstance()
							.convertObjectToList(resultMemoryObject);
				}
				resultSecondoObject.fromList(resultExpr);
			} catch (ConvertToListException e) {
				commandPanel.SystemArea.append("\n\n  List conversion failed: "
						+ e.getMessage());
				commandPanel.showPrompt();
			}
		}
		resultSecondoObject.setName(Environment.nextResultLabel() + command
				+ Environment.getMMIndicator(resultSecondoObject));
		resultSecondoObject.setMemoryObject(resultMemoryObject);
		objectList.addEntry(resultSecondoObject);
		String time = getTimeMeasureResults(startTime, calcTime,
				resultSecondoObject);
		commandPanel.SystemArea.append("\n\n  Successfully loaded to MMDB! "
				+ time);
		commandPanel.showPrompt();
		// Show result
		if (this.autoconvert) {
			this.viewerControl.showObject(resultSecondoObject);
		}
	}

	/**
	 * Determines if a result represents a valid undefined object:<br>
	 * The MemoryObject itself is {@code null} but it's wrapping ObjectNode
	 * knows its type.
	 * 
	 * @param resultObjectNode
	 *            the ObjectNode wrapping the result MemoryObject.
	 * @param resultMemoryObject
	 *            the MemoryObject itself.
	 * @return true if the result MemoryObject is null and represents a
	 *         MemoryAttribute.
	 */
	private boolean isUndefinedResult(ObjectNode resultObjectNode,
			MemoryObject resultMemoryObject) {
		return resultMemoryObject == null
				&& resultObjectNode.getOutputType() instanceof MemoryAttribute;
	}

	/**
	 * Converts an undefined result to a ListExpr.
	 * 
	 * @param resultObjectNode
	 *            the undefined result's wrapping ObjectNode.
	 * @return the ListExpr representing the undefined MemoryAttribute.
	 */
	private ListExpr getUndefinedResult(ObjectNode resultObjectNode) {
		ListExpr resultExpr;
		if (!this.autoconvert) {
			commandPanel.SystemArea
					.append("\n\n  UNDEFINED attributes are always converted to NL!");
		}
		resultExpr = ObjectConverter.getInstance()
				.convertUndefinedAttributeToList(
						(MemoryAttribute) resultObjectNode.getOutputType());
		return resultExpr;
	}

	/**
	 * Performs calculation and formatting of time measuring results.
	 * 
	 * @param startTime
	 *            the time the query procession started.
	 * @param calcTime
	 *            the time when result calculation was finished.
	 * @param sobject
	 *            the resulting SecondoObject to check if transformation was
	 *            done.
	 * @return a formatted String containing timing information.
	 */
	private String getTimeMeasureResults(long startTime, long calcTime,
			SecondoObject sobject) {
		long completeTime = System.currentTimeMillis();
		long calculation = calcTime - startTime;
		float calculationSeconds = (float) calculation / 1000f;

		DecimalFormat df = new DecimalFormat("#.###");
		df.setRoundingMode(RoundingMode.CEILING);

		if (sobject.toListExpr() == null) {
			return String.format("(%ss)", df.format(calculationSeconds));
		} else {
			long transform = completeTime - calcTime;
			float transformSeconds = (float) transform / 1000f;
			return String.format("(Calc: %ss, Trans: %ss)",
					df.format(calculationSeconds), df.format(transformSeconds));
		}

	}

	/**
	 * Translates a query in SECONDO QUERY LANGUAGE to its nested list
	 * representation.<br>
	 * The SECONDO core performs this step, thus a connection to the core is
	 * vital.<br>
	 * Also a database must be openend in the gui, since the core otherwise does
	 * not execute queries.
	 * 
	 * @param command
	 *            The command in SECONDO QUERS LANGUAGE.
	 * @param secondointerface
	 *            the interface to the core to send queries through.
	 * @return the nested list representation of the query as a String.
	 * @throws InvalidQueryException
	 *             if the query in SECONDO QUERY LANGUAGE format was invalid and
	 *             this not transformed by the core.
	 * @throws TransformQueryException
	 *             if there was a (allegedly technical) error while trying to
	 *             let the core transform the query
	 */
	private String translateQueryToNL(String command,
			ESInterface secondointerface) throws InvalidQueryException,
			TransformQueryException {
		command = command.replace("'", "\\'");
		String query2listCommand = String.format(q2lQueryString, command);

		ListExpr resultList = new ListExpr();
		IntByReference errorCode = new IntByReference(0);
		IntByReference errorPos = new IntByReference(0);
		StringBuffer errorMessage = new StringBuffer();
		secondointerface.secondo(query2listCommand, resultList, errorCode,
				errorPos, errorMessage);

		String resultQuery;
		switch (errorCode.value) {
		case 0:
			resultQuery = resultList.second().textValue();
			if (resultQuery.startsWith(invalidQueryString)) {
				throw new InvalidQueryException(
						resultQuery.substring(invalidQueryString.length()));
			}
			if (resultQuery.endsWith("\n")) {
				resultQuery = resultQuery
						.substring(0, resultQuery.length() - 1);
			}
			return resultQuery;
		case 6:
			throw new TransformQueryException("No database opened?");
		case 80:
			throw new TransformQueryException(
					"Not connected to secondo server?");
		default:
			if ("".equals(errorMessage.toString())) {
				throw new TransformQueryException("Unknown Error!");
			} else {
				throw new TransformQueryException(errorMessage.toString());
			}
		}
	}

	/**
	 * Extracts the command content from a command received directly from the
	 * CommandPanel.<br>
	 * - Removes leading "mmdb "<br>
	 * - Removes trailing ";"<br>
	 * - Trims the remaining query
	 * 
	 * @param command
	 *            The command to extract the mmdb command from
	 * @return the extracted mmdb command.
	 */
	private String extractContent(String command) {
		// Remove "mmdb "
		String commandContent = command.substring(5);

		// Remove ";"
		if (commandContent.endsWith(";")) {
			commandContent = commandContent.substring(0,
					commandContent.length() - 1);
		}

		// Remove blanks
		commandContent = commandContent.trim();

		return commandContent;
	}

	/**
	 * Handles Exceptions thrown during result calculation.<br>
	 * This method keeps the main code easier to read.
	 * 
	 * @param e
	 *            the MMDBException to be handled.
	 */
	private void handleGetResultException(MMDBException e) {
		String errorName;
		if (e instanceof ParsingException) {
			errorName = "NL-Parser";
		} else if (e instanceof TypeException) {
			errorName = "Typecheck";
		} else if (e instanceof MemoryException) {
			errorName = "Memory";
		} else {
			errorName = "Unknown";
		}
		commandPanel.SystemArea.append("\n\n  " + errorName + "-Error: "
				+ e.getMessage());
		commandPanel.showPrompt();
	}

	/**
	 * Handles Exceptions thrown during query translation.<br>
	 * This method keeps the main code easier to read.
	 * 
	 * @param e
	 *            the MMDBException to be handled.
	 */
	private void handleTranslateError(MMDBException e) {
		String generalMessage = "\n\n  ";
		if (e instanceof InvalidQueryException) {
			generalMessage += "Query was invalid:\n";
		} else {
			generalMessage += "Query could not be transformed to nested list format: ";
		}
		commandPanel.SystemArea.append(generalMessage + e.getMessage());
		commandPanel.showPrompt();
	}

}
