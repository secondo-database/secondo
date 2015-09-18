package mmdb.streamprocessing.objectnodes;

import java.util.List;

import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.HeaderTools;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;
import sj.lang.ListExpr;

/**
 * Operator attr resembling the core operator.<br>
 * Extracts a single MemoryAttribute by its name from a MemoryTuple.
 * 
 * @author Bjoern Clasen
 */
public class Attr implements ObjectNode {

	/**
	 * The operator's parameter Node.
	 */
	private Node input;

	/**
	 * The operators first parameter as an ObjectNode.
	 */
	private ObjectNode objectInput;

	/**
	 * The identifier of the MemoryAttribute to extract.
	 */
	private String identifier;

	/**
	 * The index of the MemoryAttribute in the incoming MemoryTuples.
	 */
	private int identifierIndex;

	/**
	 * The operator's output type.
	 */
	private MemoryAttribute outputType;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Attr.class);
		Node node = NestedListProcessor.nlToNode(params[0], environment);
		String identParam = NestedListProcessor.nlToIdentifier(params[1]);
		return new Attr(node, identParam);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input
	 *            operator's first parameter
	 * @param identifier
	 *            operator's second parameter
	 */
	public Attr(Node input, String identifier) {
		this.input = input;
		this.identifier = identifier;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		this.input.typeCheck();

		// Is input an ObjectNode providing a Tuple?
		TypecheckTools.checkNodeType(this.input, ObjectNode.class,
				this.getClass(), 1);
		this.objectInput = (ObjectNode) input;
		TypecheckTools.checkOutputType(this.objectInput, MemoryTuple.class,
				this.getClass(), 1);

		this.identifierIndex = HeaderTools.getHeaderIndexForIdentifier(
				((MemoryTuple) this.objectInput.getOutputType())
						.getTypecheckInfo(), this.identifier, this.getClass());

		this.outputType = determineOutputType();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryAttribute getResult() throws MemoryException {
		MemoryTuple tuple = (MemoryTuple) this.objectInput.getResult();
		if (tuple == null) {
			return null;
		}

		return tuple.getAttribute(this.identifierIndex);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryAttribute getOutputType() {
		return outputType;
	}

	/**
	 * Determines attr's OutputType.<br>
	 * Creates the MemoryAttributes TypecheckInstance via Reflection.
	 * 
	 * @return a TypecheckInstance of the MemoryAttribute under
	 *         {@link Attr#identifierIndex}
	 * @throws TypeException
	 *             if the MemoryAttribute could not be instantiated via
	 *             Reflection.
	 */
	private MemoryAttribute determineOutputType() throws TypeException {
		List<RelationHeaderItem> typecheckInfo = ((MemoryTuple) this.objectInput
				.getOutputType()).getTypecheckInfo();
		try {
			// Create a new (Typecheck-)Instance of the MemoryAttribute via
			// Reflection
			return typecheckInfo.get(this.identifierIndex).getType()
					.newInstance();
		} catch (Exception e) {
			throw new TypeException(String.format(
					"Error in instantiating Attribute: %s",
					typecheckInfo.get(this.identifierIndex).getType()));
		}
	}

}
