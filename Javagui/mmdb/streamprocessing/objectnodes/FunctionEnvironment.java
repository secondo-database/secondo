package mmdb.streamprocessing.objectnodes;

import mmdb.data.MemoryObject;
import mmdb.error.streamprocessing.TypeException;

public class FunctionEnvironment implements ObjectNode {
	
	private MemoryObject outputType;
	
	private MemoryObject currentObject;

	@Override
	public void typeCheck() throws TypeException {
		if (this.outputType == null) {
			throw new TypeException(
					"Output-Type for FunctionEnvironmet is null!");
		}
	}

	@Override
	public MemoryObject getResult() {
		return this.currentObject;
	}

	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}
	
	public void setOutputType(MemoryObject outputType) {
		this.outputType = outputType;
	}
	
	public void setObject(MemoryObject object) {
		this.currentObject = object;
	}

}
