package mmdb.streamprocessing;

import mmdb.data.MemoryObject;
import mmdb.error.streamprocessing.TypeException;

public interface Node {

	public void typeCheck() throws TypeException;

	public MemoryObject getOutputType();

}
