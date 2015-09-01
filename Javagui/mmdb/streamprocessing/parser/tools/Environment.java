package mmdb.streamprocessing.parser.tools;

import gui.SecondoObject;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import mmdb.error.streamprocessing.ParsingException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;

public class Environment {

	private Map<String, ObjectNode> map;

	public Environment() {
		this.map = new HashMap<>();
	}

	public Environment(List<SecondoObject> existingObjects) {
		this.map = new HashMap<>();
		for (SecondoObject sobject : existingObjects) {
			if (sobject.getMemoryObject() != null) {
				ObjectNode objNode = ConstantNode.createConstantNode(
						sobject.getMemoryObject(), sobject.getMemoryObject());
				String objectName = sobject.getName();
				String name = "";
				if (objectName.contains(" [++]")) {
					name = sobject.getName().substring(0,
							sobject.getName().indexOf(" [++]"));
				}
				if (objectName.contains(" [+]")) {
					name = sobject.getName().substring(0,
							sobject.getName().indexOf(" [+]"));
				}
				this.map.put(name, objNode);
			}
		}
	}

	private Environment(Map<String, ObjectNode> curMap) {
		this.map = curMap;
	}

	public ObjectNode getEnvironmentObject(String name) {
		return map.get(name);
	}

	public Environment addEnvironmentObject(String name, ObjectNode object)
			throws ParsingException {
		if (map.containsKey(name)) {
			throw new ParsingException("Identifier already in use: " + name);
		}
		Map<String, ObjectNode> newMap = new HashMap<>();
		newMap.putAll(map);
		newMap.put(name, object);
		return new Environment(newMap);
	}

}
