package mmdb.streamprocessing.objectnodes;

import mmdb.data.MemoryObject;
import mmdb.error.streamprocessing.TypeException;

/**
 * Class representing a ParameterFunction parameter.<br>
 * NO direct representation in the SECONDO core.<br>
 * This class states an environment that is present inside the ParameterFunction
 * and contains the current parameter passed to the ParameterFunction.
 * 
 * @author Bjoern Clasen
 */
public class FunctionEnvironment implements ObjectNode {

	/**
	 * The environment's output type.
	 */
	private MemoryObject outputType;

	/**
	 * The object currently being contained in this environment.
	 */
	private MemoryObject currentObject;

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		if (this.outputType == null) {
			throw new TypeException(
					"Output-Type for FunctionEnvironmet is null!");
		}
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getResult() {
		return this.currentObject;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}
	
	/**
	 * Special method only for FunctionEnvironments.<br>
	 * It sets the OutputType externally.
	 * 
	 * @param outputType
	 *            the OutputType this FunctionEnvironment has.
	 */
	public void setOutputType(MemoryObject outputType) {
		this.outputType = outputType;
	}
	
	/**
	 * Sets the currently contained object.
	 * 
	 * @param object
	 *            the object this FunctionEnvironment temporarily contains.
	 */
	public void setObject(MemoryObject object) {
		this.currentObject = object;
	}

}
