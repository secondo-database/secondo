package mmdb.streamprocessing.objectnodes.aggregation;

import java.util.List;

import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.streamoperators.StreamOperator;
import mmdb.streamprocessing.tools.HeaderTools;
import mmdb.streamprocessing.tools.TypecheckTools;

public abstract class AggregationOperator implements ObjectNode {

	protected Node input;

	protected StreamOperator streamInput;

	protected String identifier;

	protected int attributeIndex;

	protected MemoryAttribute outputType;

	public AggregationOperator(Node input, String identifier) {
		this.input = input;
		this.identifier = identifier;
	}

	@Override
	public abstract void typeCheck() throws TypeException;

	public void typeCheckForInterface(Class<?> iFace) throws TypeException {
		this.input.typeCheck();

		// Is input a StreamOperator providing tuples?
		TypecheckTools.checkNodeType(this.input, StreamOperator.class,
				this.getClass(), 1);
		this.streamInput = (StreamOperator) input;
		TypecheckTools.checkOutputType(this.streamInput, MemoryTuple.class,
				this.getClass(), 1);

		// Does the attribute have the specified interface?
		List<RelationHeaderItem> inputOutputType = ((MemoryTuple) this.streamInput
				.getOutputType()).getTypecheckInfo();
		HeaderTools.checkAttributeHasIFace(inputOutputType, this.identifier,
				iFace, this.getClass());

		// Is identifier preset?
		this.attributeIndex = HeaderTools.getHeaderIndexForIdentifier(
				inputOutputType, this.identifier, this.getClass());

		this.outputType = this.determineOutputType(inputOutputType);
	}

	@Override
	public abstract MemoryAttribute getResult();

	@Override
	public MemoryAttribute getOutputType() {
		return this.outputType;
	}

	protected MemoryAttribute determineOutputType(
			List<RelationHeaderItem> inputOutputType) throws TypeException {
		try {
			return HeaderTools.getClassForHeaderItem(inputOutputType,
					this.identifier, this.getClass()).newInstance();
		} catch (Exception e) {
			throw new TypeException(String.format(
					"Error in instantiating Attribute: %s", HeaderTools
							.getClassForHeaderItem(inputOutputType,
									this.identifier, this.getClass())));
		}
	}

}
