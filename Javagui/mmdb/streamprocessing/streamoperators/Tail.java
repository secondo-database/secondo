package mmdb.streamprocessing.streamoperators;

import java.util.Iterator;
import java.util.LinkedList;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;
import sj.lang.ListExpr;

/**
 * Operator tail resembling the operator in the core.<br>
 * Only forwards the last X elements of a stream.
 * 
 * @author Björn Clasen
 *
 */
public class Tail implements StreamOperator {

	/**
	 * The operator's parameter Nodes.
	 */
	private Node input1, input2;

	/**
	 * The operator's first parameter as a StreamOperator.
	 */
	private StreamOperator streamInput;

	/**
	 * The operator's second input as an ObjectNode containing the number of
	 * elements to forward.
	 */
	private ObjectNode objectInput;

	/**
	 * The operator's output type.
	 */
	private MemoryObject outputType;

	/**
	 * A list containing the last X elements from the input stream.
	 */
	private LinkedList<MemoryObject> bufferList;

	/**
	 * An iterator over the buffer list.
	 */
	private Iterator<MemoryObject> iterator;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Tail.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Node node2 = NestedListProcessor.nlToNode(params[1], environment);
		return new Tail(node1, node2);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input1
	 *            operator's first parameter
	 * @param input2
	 *            operator's second parameter
	 */
	public Tail(Node input1, Node input2) {
		this.input1 = input1;
		this.input2 = input2;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		this.input1.typeCheck();
		this.input2.typeCheck();

		// Is input1 a StreamOperator?
		TypecheckTools.checkNodeType(this.input1, StreamOperator.class,
				this.getClass(), 1);
		this.streamInput = (StreamOperator) this.input1;

		// Is input1 an ObjectNode providing an integer?
		TypecheckTools.checkNodeType(this.input2, ObjectNode.class,
				this.getClass(), 2);
		this.objectInput = (ObjectNode) this.input2;
		TypecheckTools.checkOutputType(this.objectInput, AttributeInt.class,
				this.getClass(), 2);

		this.outputType = this.streamInput.getOutputType();
	}

	/**
	 * {@inheritDoc}<br>
	 * Initializes the buffer list (and so already completely drains the input
	 * stream).
	 */
	@Override
	public void open() throws MemoryException {
		this.streamInput.open();
		AttributeInt outputLimitAttr = (AttributeInt) this.objectInput
				.getResult();
		if (outputLimitAttr == null) {
			initBuffer(0);
		} else {
			initBuffer(outputLimitAttr.getValue());
		}

	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getNext() {
		if (this.iterator.hasNext()) {
			return this.iterator.next();
		} else {
			return null;
		}
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
	public MemoryObject getOutputType() {
		return this.outputType;
	}

	/**
	 * Initializes the buffer list with the given number of elements.
	 * 
	 * @param outputLimit
	 *            number of elements to store in the buffer list.
	 * @throws MemoryException
	 *             if memory tended to run out during initialization.
	 */
	private void initBuffer(int outputLimit) throws MemoryException {
		this.bufferList = new LinkedList<MemoryObject>();
		MemoryObject object;
		while ((object = (MemoryObject) this.streamInput.getNext()) != null) {
			this.bufferList.add(object);
			if (this.bufferList.size() > outputLimit) {
				this.bufferList.removeFirst();
			}
		}
		this.iterator = this.bufferList.iterator();
	}

}
