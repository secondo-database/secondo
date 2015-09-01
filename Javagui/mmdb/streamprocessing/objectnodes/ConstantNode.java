package mmdb.streamprocessing.objectnodes;

import mmdb.data.MemoryObject;
import mmdb.error.streamprocessing.TypeException;

public class ConstantNode implements ObjectNode {

	private MemoryObject object, typecheckInstance;

	public ConstantNode(MemoryObject object, MemoryObject typecheckInstance) {
		this.object = object;
		this.typecheckInstance = typecheckInstance;
	}

	@Override
	public void typeCheck() throws TypeException {
		if (this.typecheckInstance == null) {
			throw new TypeException("Typecheck Instance for Wrapper was null!");
		}
	}

	@Override
	public MemoryObject getResult() {
		return this.object;
	}

	@Override
	public MemoryObject getOutputType() {
		return this.typecheckInstance;
	}

	public static ObjectNode createConstantNode(MemoryObject object,
			MemoryObject typecheckInstance) {
		return new ConstantNode(object, typecheckInstance);
	}

}
