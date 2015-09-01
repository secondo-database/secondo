package mmdb.streamprocessing.parser;

import gui.CommandPanel;
import gui.ObjectList;
import gui.SecondoObject;
import gui.ViewerControl;
import gui.idmanager.IDManager;

import java.util.List;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.convert.ConvertToListException;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.service.ObjectConverter;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import sj.lang.ESInterface;
import sj.lang.IntByReference;
import sj.lang.ListExpr;

public class ParserController {

	private boolean autoconvert = true;

	private static ParserController instance;

	private ObjectList objectList;

	private CommandPanel commandPanel;

	private ViewerControl viewerControl;

	private static final String invalidQueryString = "INVALID QUERY ";

	private static final String q2lErrorString = "Q2L ERROR ";

	private static final String q2lQueryString = "query query2list('%s');";

	private ParserController() {

	}

	public static ParserController getInstance() {
		if (instance == null) {
			instance = new ParserController();
		}
		return instance;
	}

	public void injectGuiElements(ObjectList objectList,
			CommandPanel commandPanel, ViewerControl viewerControl) {
		this.objectList = objectList;
		this.commandPanel = commandPanel;
		this.viewerControl = viewerControl;

	}

	public void processTextAutoconvert() {
		this.autoconvert = !this.autoconvert;
	}

	public void processMMDBQuery(String command, ESInterface secondoInterface) {
		// Remove "mmdb "
		String commandContent = command.substring(5);

		// Remove ";"
		if (commandContent.endsWith(";")) {
			commandContent = commandContent.substring(0,
					commandContent.length() - 1);
		}

		commandContent = commandContent.trim();

		if (!commandContent.startsWith("(")) {
			commandContent = translateQueryToNL(commandContent,
					secondoInterface);
			if (commandContent.startsWith(q2lErrorString)) {
				commandContent = commandContent.substring(10);
				commandPanel.SystemArea
						.append("\n\n  Query could not be transformed to nested list format: "
								+ commandContent);
				commandPanel.showPrompt();
				return;
			}
			if (commandContent.startsWith(invalidQueryString)) {
				commandContent = commandContent.substring(14);
				commandPanel.SystemArea.append("\n\n  Query was invalid:\n"
						+ commandContent);
				commandPanel.showPrompt();
				return;
			}
		}

		List<SecondoObject> existingObjects = objectList.getAllObjects();
		ObjectNode resultObjectNode = null;
		try {
			resultObjectNode = NestedListProcessor.buildOperatorTree(
					commandContent, existingObjects);
		} catch (ParsingException e) {
			commandPanel.SystemArea.append("\n\n  NL-Parser-Error: "
					+ e.getMessage());
			commandPanel.showPrompt();
			return;
		}
		try {
			resultObjectNode.typeCheck();
		} catch (TypeException e) {
			commandPanel.SystemArea.append("\n\n  Typecheck-Error: "
					+ e.getMessage());
			commandPanel.showPrompt();
			return;
		}
		MemoryObject resultMemoryObject = resultObjectNode.getResult();
		SecondoObject resultSecondoObject = new SecondoObject(
				IDManager.getNextID());
		if (this.autoconvert || resultMemoryObject == null) {
			try {
				ListExpr resultExpr = null;
				if (resultMemoryObject == null
						&& resultObjectNode.getOutputType() instanceof MemoryAttribute) {
					commandPanel.SystemArea
							.append("\n\n  UNDEFINED attributes are always converted to NL!");
					resultExpr = ObjectConverter.getInstance()
							.convertUndefinedAttributeToList(
									(MemoryAttribute) resultObjectNode
											.getOutputType());
				} else {
					resultExpr = ObjectConverter.getInstance()
							.convertObjectToList(resultMemoryObject);
				}
				resultSecondoObject.fromList(resultExpr);
				resultSecondoObject.setName(command + " [++]");
			} catch (ConvertToListException e) {
				commandPanel.SystemArea.append("\n\n  List conversion failed: "
						+ e.getMessage());
				commandPanel.showPrompt();
				return;
			} catch (MemoryException e) {
				commandPanel.SystemArea.append("\n\n  No more memory: "
						+ e.getMessage());
				commandPanel.showPrompt();
				return;
			}
		} else {
			resultSecondoObject.setName(command + " [+]");
		}
		resultSecondoObject.setMemoryObject(resultMemoryObject);
		objectList.addEntry(resultSecondoObject);
		if (this.autoconvert) {
			this.viewerControl.showObject(resultSecondoObject);
		}
		commandPanel.SystemArea
				.append("\n\n  Result successfully loaded to MMDB!");
		commandPanel.showPrompt();
	}

	private String translateQueryToNL(String command,
			ESInterface secondointerface) {
		String query2listCommand = String.format(q2lQueryString, command);

		ListExpr resultList = new ListExpr();
		IntByReference errorCode = new IntByReference(0);
		IntByReference errorPos = new IntByReference(0);
		StringBuffer errorMessage = new StringBuffer();
		secondointerface.secondo(query2listCommand, resultList, errorCode,
				errorPos, errorMessage);

		String resultQuery;
		if (errorMessage.length() != 0) {
			resultQuery = q2lErrorString + errorMessage.toString();
		} else {
			resultQuery = resultList.second().textValue();
			if (resultQuery.endsWith("\n")) {
				resultQuery = resultQuery
						.substring(0, resultQuery.length() - 1);
			}
		}
		return resultQuery;
	}

}
