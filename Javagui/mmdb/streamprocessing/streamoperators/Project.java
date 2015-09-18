package mmdb.streamprocessing.streamoperators;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
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
 * Operator project resembling the operator in the core.<br>
 * Removes all but the selected columns from a stream of MemoryTuples.<br>
 * Represents SQL SELECT statement.
 * 
 * @author Bjoern Clasen
 *
 */
public class Project implements StreamOperator {

	/**
	 * The operator's parameter Node.
	 */
	private Node input;

	/**
	 * The operators first parameter as a StreamOperator.
	 */
	private StreamOperator streamInput;

	/**
	 * The operator's identifier parameters to filter.
	 */
	private String[] identifiers;

	/**
	 * The positions of the identifiers in the MemoryTuples' headers.
	 */
	private Map<Integer, Integer> columnPositions;

	/**
	 * The operator's output type.
	 */
	private MemoryTuple outputType;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Project.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		String[] identifierParams = NestedListProcessor
				.nlToIdentifierArray(params[1]);
		return new Project(node1, identifierParams);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input
	 *            operator's first parameter
	 * @param identifiers
	 *            operator's identifier parameters
	 */
	public Project(Node input, String[] identifiers) {
		this.input = input;
		this.identifiers = identifiers;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		this.input.typeCheck();

		// Is input a StreamOperator providing Tuples?
		TypecheckTools.checkNodeType(this.input, StreamOperator.class,
				this.getClass(), 1);
		this.streamInput = (StreamOperator) this.input;
		TypecheckTools.checkOutputType(this.input, MemoryTuple.class,
				this.getClass(), 1);

		HeaderTools.checkIdentifiersPresent(((MemoryTuple) this.streamInput
				.getOutputType()).getTypecheckInfo(), this.getClass(),
				identifiers);

		this.checkIdentifierDuplicates();

		this.outputType = MemoryTuple.createTypecheckInstance(this
				.calculateOutputType());
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void open() throws MemoryException {
		this.streamInput.open();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryTuple getNext() throws MemoryException {
		MemoryTuple inputTuple = (MemoryTuple) this.streamInput.getNext();
		if (inputTuple == null) {
			return null;
		}

		MemoryTuple outputTuple = new MemoryTuple();
		for (int i = 0; i < this.identifiers.length; i++) {
			int position = columnPositions.get(i);
			outputTuple.addAttribute(inputTuple.getAttribute(position));
		}

		return outputTuple;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void close() {
		this.streamInput.close();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryTuple getOutputType() {
		return this.outputType;
	}

	/**
	 * Calculates the header of the output MemoryTuples by only adding selected
	 * columns from the input header.
	 * 
	 * @return
	 */
	private List<RelationHeaderItem> calculateOutputType() {
		this.initPositions();
		List<RelationHeaderItem> oldHeader = ((MemoryTuple) this.streamInput
				.getOutputType()).getTypecheckInfo();
		List<RelationHeaderItem> newHeader = new ArrayList<RelationHeaderItem>();
		for (int i = 0; i < this.identifiers.length; i++) {
			int position = columnPositions.get(i);
			newHeader.add(new RelationHeaderItem(oldHeader.get(position)
					.getIdentifier(), oldHeader.get(position).getTypeName()));
		}
		return newHeader;
	}

	/**
	 * Determines the positions of the parameter identifiers in the incoming
	 * MemoryTuples and stores the result in {@link Project#columnPositions}.
	 */
	private void initPositions() {
		this.columnPositions = new HashMap<Integer, Integer>();
		List<RelationHeaderItem> oldHeader = ((MemoryTuple) this.streamInput
				.getOutputType()).getTypecheckInfo();
		for (int i = 0; i < this.identifiers.length; i++) {
			for (int f = 0; f < oldHeader.size(); f++) {
				if (this.identifiers[i]
						.equals(oldHeader.get(f).getIdentifier())) {
					this.columnPositions.put(i, f);
				}
			}
		}
	}

	/**
	 * Checks if there were duplications in the identifier parameters.
	 * 
	 * @throws TypeException
	 *             if there were duplications.
	 */
	private void checkIdentifierDuplicates() throws TypeException {
		Set<String> identifierSet = new HashSet<String>();
		for (String identifier : this.identifiers) {
			if (!identifierSet.add(identifier)) {
				throw new TypeException("Identifier duplication on: "
						+ identifier);
			}
		}
	}

}
