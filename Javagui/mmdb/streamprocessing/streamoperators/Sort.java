package mmdb.streamprocessing.streamoperators;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;
import sj.lang.ListExpr;

/**
 * Simple sort operator resembling the core operator.<br>
 * Sorts a stream of MemoryTuples by all their columns in ascending order.<br>
 * Internally uses a delegation to the sortby operator.
 * 
 * @author Bjoern Clasen
 *
 */
public class Sort implements StreamOperator {

	/**
	 * The operator's parameter Node.
	 */
	private Node input;

	/**
	 * The operator's parameter as a StreamOperator.
	 */
	private StreamOperator streamInput;

	/**
	 * The instance of the sortby operator to delegate to.
	 */
	private Sortby sortby;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 1, Sort.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		return new Sort(node1);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input
	 *            operator's parameter
	 */
	public Sort(Node input) {
		this.input = input;
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
		TypecheckTools.checkOutputType(this.streamInput, MemoryTuple.class,
				this.getClass(), 1);

		Map<String, String> sortAttributes = calcSortAttributes();

		// Delegate to Sortby
		this.sortby = new Sortby(this.streamInput, sortAttributes);
		this.sortby.typeCheck();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getOutputType() {
		return this.sortby.getOutputType();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void open() throws MemoryException {
		this.sortby.open();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getNext() {
		return this.sortby.getNext();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void close() {
		this.sortby.close();
	}

	/**
	 * Calculates the sort attributes to pass to the sortby delegate.<br>
	 * Just uses all column names in their original order and adds "asc".
	 * 
	 * @return the parameter map for sortby.
	 */
	private Map<String, String> calcSortAttributes() {
		Map<String, String> sortAttributes = new LinkedHashMap<String, String>();

		List<RelationHeaderItem> inputType = ((MemoryTuple) this.streamInput
				.getOutputType()).getTypecheckInfo();

		for (RelationHeaderItem item : inputType) {
			sortAttributes.put(item.getIdentifier(), "asc");
		}

		return sortAttributes;
	}

}
