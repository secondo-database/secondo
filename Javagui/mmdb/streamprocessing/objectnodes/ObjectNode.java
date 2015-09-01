package mmdb.streamprocessing.objectnodes;

import mmdb.data.MemoryObject;
import mmdb.streamprocessing.Node;

public interface ObjectNode extends Node {

	public MemoryObject getResult();

}
