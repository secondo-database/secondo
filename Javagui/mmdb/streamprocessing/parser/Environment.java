package mmdb.streamprocessing.parser;

import gui.SecondoObject;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import mmdb.error.streamprocessing.ParsingException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;

/**
 * Class containing all known database objects during the parsing phase.
 * Supports a simple scoping mechanism by artificially making the map of objects
 * unmodifiable.
 * 
 * @author Björn Clasen
 */
public class Environment {

	/**
	 * The last result number that has been published.<br>
	 * Needed for {@link Environment#nextResultLabel()}.
	 */
	private static int resultNum = 0;

	/**
	 * The regular expression for result labels in SecondoObject names.
	 */
	private static final String REULSTLABEL_REGEX = "^R\\d+:\\s.*";

	/**
	 * The memory indicator saying <b>only</b> main memory representation.
	 */
	private static final String MMINDICATOR_MMONLY = " [+]";

	/**
	 * The memory indicator saying main memory <b>and</b> nested list
	 * representation.
	 */
	private static final String MMINDICATOR_BOTH = " [++]";

	/**
	 * The map containing all known database objects.
	 */
	private Map<String, ObjectNode> map;

	/**
	 * Creates a new Environment that contains all MemoryObjects contained in
	 * the given SecondoObjects. Also adds the MemoryObjects under their label
	 * if the SecondoObject's name is labelled.
	 * 
	 * @param existingObjects
	 *            the list of SecondoObjects that are known.
	 */
	public Environment(List<SecondoObject> existingObjects) {
		this.map = new HashMap<String, ObjectNode>();
		for (SecondoObject sobject : existingObjects) {
			if (sobject.getMemoryObject() != null) {
				ObjectNode objNode = ConstantNode.createConstantNode(
						sobject.getMemoryObject(), sobject.getMemoryObject());
				String objectName = sobject.getName();
				objectName = removeMMIndicator(objectName);

				// Labeling for objects
				String resultLabel = getResultLabel(objectName);
				if (!resultLabel.equals("")) {
					this.map.put(resultLabel, objNode);
				}
				this.map.put(objectName, objNode);
			}
		}
	}

	/**
	 * Private constructor to create a new Environment containing the given map.
	 * 
	 * @param map
	 *            the map the new Environment shall contain.
	 */
	private Environment(Map<String, ObjectNode> map) {
		this.map = map;
	}

	/**
	 * Retrieves the object stored unter the given name.<br>
	 * 
	 * @see HashMap#get(Object)
	 * @param name
	 *            the object's name.
	 * @return the ObjectNode of the object under the given name or null if that
	 *         name is unknown.
	 */
	public ObjectNode getEnvironmentObject(String name) {
		return map.get(name);
	}

	/**
	 * <b>!!! OBTAIN THE RETURN VALUE !!!</b><br>
	 * Returns a new Environment containing all formerly known objects and the
	 * newly added object.<br>
	 * This is done for scoping purposes, so the new object is only known in the
	 * callers scope and it's subscopes.
	 * 
	 * @param name
	 *            the name of the new known object.
	 * @param object
	 *            the ObjectNode stored under the given name.
	 * @return <b>A new Environment object</b> containing all formerly known
	 *         objects and the new one.
	 * @throws ParsingException
	 *             if the given name is already in use.
	 */
	public Environment addObjectToNewEnvironment(String name, ObjectNode object)
			throws ParsingException {
		if (map.containsKey(name)) {
			throw new ParsingException("Identifier already in use: " + name);
		}
		Map<String, ObjectNode> newMap = new HashMap<String, ObjectNode>();
		newMap.putAll(map);
		newMap.put(name, object);
		return new Environment(newMap);
	}

	/**
	 * Returns the next result label for SecondoObjects.<br>
	 * Result labels are of the form <i>RXX:</i><br>
	 * <i>XX</i> being the next free/unused integer value.
	 * 
	 * @return the next (unused) result label.
	 */
	public static String nextResultLabel() {
		resultNum++;
		return "R" + resultNum + ": ";
	}

	/**
	 * Returns the result label of the given name if it contains one.
	 * 
	 * @param name
	 *            the name whose result label is to be extracted.
	 * @return the name's result label or an empty String if it has none.
	 */
	public static String getResultLabel(String name) {
		if (name.matches(REULSTLABEL_REGEX)) {
			return name.substring(0, name.indexOf(":"));
		} else {
			return "";
		}
	}

	/**
	 * Removes the result label from the given name it if contains one.
	 * 
	 * @param name
	 *            the name to remove the result label from.
	 * @return the name cleared from any result label.
	 */
	public static String removeResultLabel(String name) {
		if (name.matches(REULSTLABEL_REGEX)) {
			name = name.substring(name.indexOf(": ") + 2);
		}
		return name;
	}

	/**
	 * Removes the memory indicator from a SecondoObject's name.<br>
	 * Memory indicators: <b>" [+]"</b> & <b>" [++]"</b>
	 * 
	 * @param name
	 *            the name which is to be cleared of any memory indicator.
	 * @return the name without any memory indicator.
	 */
	public static String removeMMIndicator(String name) {
		if (name.contains(MMINDICATOR_BOTH)) {
			name = name.substring(0, name.lastIndexOf(MMINDICATOR_BOTH));
		}
		if (name.contains(MMINDICATOR_MMONLY)) {
			name = name.substring(0, name.lastIndexOf(MMINDICATOR_MMONLY));
		}
		return name;
	}

	/**
	 * Creates the memory indicator for the given SecondoObject:<br>
	 * <b>" [+]"</b> if it does only contain a main memory representation.<br>
	 * <b>" [++]"</b> if it contains both a nested list and a main memory
	 * representation.
	 * 
	 * @param sobject
	 *            the SecondoObject to create the memory indicator for.
	 * @return the correct memory indicator for the given SecondoObject (see
	 *         above).
	 */
	public static String getMMIndicator(SecondoObject sobject) {
		return (sobject.toListExpr() == null) ? MMINDICATOR_MMONLY
				: MMINDICATOR_BOTH;
	}

}
