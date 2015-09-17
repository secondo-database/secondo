package mmdb.streamprocessing.objectnodes;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.service.MemoryWatcher;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.streamoperators.StreamOperator;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;
import sj.lang.ListExpr;

/**
 * Operator Consume resembling the core operator.<br>
 * Collects a tuple stream into a MemoryRelation.
 * 
 * @author Björn Clasen
 */
public class Consume implements ObjectNode {

	/**
	 * The operator's parameter Node.
	 */
	private Node input;

	/**
	 * The operators parameter as a StreamOperator.
	 */
	private StreamOperator streamInput;

	/**
	 * The operator's output type.
	 */
	private MemoryRelation outputType;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 1, Consume.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		return new Consume(node1);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input
	 *            operator's parameter
	 */
	public Consume(Node input) {
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

		this.outputType = MemoryRelation
				.createTypecheckInstance(((MemoryTuple) this.streamInput
						.getOutputType()).getTypecheckInfo());
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryRelation getResult() throws MemoryException {
		List<RelationHeaderItem> typecheckInfo = this.outputType
				.getTypecheckInfo();
		MemoryRelation resultRelation = new MemoryRelation(typecheckInfo);
		resultRelation.setTuples(getTuples());
		return resultRelation;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryRelation getOutputType() {
		return this.outputType;
	}

	/**
	 * Collects all tuples of the input stream in a List.
	 * 
	 * @return a list of all tuples in the input stream.
	 * @throws MemoryException
	 *             if during calculation memory tended to run out. Execution is
	 *             then canceled and intermediate results are discarded.
	 */
	private List<MemoryTuple> getTuples() throws MemoryException {
		List<MemoryTuple> tuples = new ArrayList<MemoryTuple>();
		this.streamInput.open();
		MemoryTuple tuple;
		int memorywatch_counter = 0;
		while ((tuple = (MemoryTuple) this.streamInput.getNext()) != null) {
			memorywatch_counter++;
			if (memorywatch_counter % MemoryWatcher.MEMORY_CHECK_FREQUENCY == 0) {
				MemoryWatcher.getInstance().checkMemoryStatus();
			}
			tuples.add(tuple);
		}
		this.streamInput.close();
		return tuples;
	}

}
