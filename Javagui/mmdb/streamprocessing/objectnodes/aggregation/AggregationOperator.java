package mmdb.streamprocessing.objectnodes.aggregation;

import java.util.List;

import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.streamoperators.StreamOperator;
import mmdb.streamprocessing.tools.HeaderTools;
import mmdb.streamprocessing.tools.TypecheckTools;

/**
 * Abstract base class for aggregation operators on tuple streams.
 * 
 * @author Björn Clasen
 *
 */
public abstract class AggregationOperator implements ObjectNode {

	/**
	 * The operator's first parameter's Node.
	 */
	protected Node input;

	/**
	 * The operator's first parameter as a StreamOperator.
	 */
	protected StreamOperator streamInput;

	/**
	 * The operator's second parameter:<br>
	 * the identifier to extract and aggregate.
	 */
	protected String identifier;

	/**
	 * The index of the attribute to aggregate.
	 */
	protected int attributeIndex;

	/**
	 * The operator's output type.
	 */
	protected MemoryAttribute outputType;

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input
	 *            operator's first parameter
	 * @param identifier
	 *            operator's second parameter
	 */
	public AggregationOperator(Node input, String identifier) {
		this.input = input;
		this.identifier = identifier;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public abstract void typeCheck() throws TypeException;

	/**
	 * Implements a Typecheck for a given interface that the aggregation
	 * attribute needs to have.
	 * 
	 * @param iFace
	 *            the interface the aggregation attribute needs to implement.
	 * @throws TypeException
	 *             if any type error is found. Recursive process is completely
	 *             cancelled.
	 */
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

	/**
	 * {@inheritDoc}
	 */
	@Override
	public abstract MemoryAttribute getResult() throws MemoryException;

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryAttribute getOutputType() {
		return this.outputType;
	}

	/**
	 * Determines the present OutputType (subtype of MemoryAttribute).
	 * 
	 * @param inputOutputType
	 *            the OutputType of the stream parameter.
	 * @return detected OutputType.
	 * @throws TypeException
	 *             if OutputType could not be detected (identifier is not
	 *             present).
	 */
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
