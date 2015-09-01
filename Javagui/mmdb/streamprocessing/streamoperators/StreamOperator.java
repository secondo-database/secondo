package mmdb.streamprocessing.streamoperators;

import mmdb.data.MemoryObject;
import mmdb.streamprocessing.Node;

public interface StreamOperator extends Node {

	public void open();

	public MemoryObject getNext();

	public void close();

}
