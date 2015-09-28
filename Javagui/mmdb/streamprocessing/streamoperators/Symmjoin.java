package mmdb.streamprocessing.streamoperators;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentLinkedQueue;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.StreamStateException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.functionoperators.ParameterFunction;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;
import sj.lang.ListExpr;

/**
 * The Symmjoin operator resembling the operator in the core.<br>
 * Like the core operator it takes two tuple streams and a function that maps
 * two tuples (one from the right and one from the left stream) to a boolean
 * value. It continuously builds up internal tuple buffers. When a new tuple
 * comes in, it is joined with all the yet collected tuples from the other
 * stream and stored in its buffer. This way result tuples can be calculated
 * before entire streams have been cached.
 * 
 * @author Bjoern Clasen
 *
 */
public class Symmjoin implements StreamOperator {

	/**
	 * The operator's parameter Nodes.
	 */
	private Node input1, input2, input3;

	/**
	 * The operator's first two parameters as StreamOperators.
	 */
	private StreamOperator streamInput1, streamInput2;

	/**
	 * The operator's third parameter as a ParameterFunction.
	 */
	private ParameterFunction functionInput;

	/**
	 * The operator's output type.
	 */
	private MemoryTuple outputType;

	/**
	 * The buffers for both streams.
	 */
	private List<MemoryTuple> leftBuffer, rightBuffer;

	/**
	 * A temporary queue to store tuples that have already been joined but not
	 * put out.
	 */
	private ConcurrentLinkedQueue<MemoryTuple> tupleStore;

	/**
	 * Simple boolean variable storing where the last tuple was pulled
	 */
	private boolean lastLeft = false;

	/**
	 * Simple boolean storing whether streams have ended
	 */
	private boolean leftEnded = false, rightEnded = false;

	/**
	 * Enum as return value for join[Left/Right]Tuple()
	 */
	private enum Success {
		RESULTS, NO_RESULTS, STREAM_ENDED
	}

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 3, Symmjoin.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Node node2 = NestedListProcessor.nlToNode(params[1], environment);
		Node node3 = NestedListProcessor.nlToNode(params[2], environment);
		return new Symmjoin(node1, node2, node3);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input1
	 *            operator's first parameter
	 * @param input2
	 *            operator's second parameter
	 * @param input3
	 *            operator's third parameter
	 */
	public Symmjoin(Node input1, Node input2, Node input3) {
		this.input1 = input1;
		this.input2 = input2;
		this.input3 = input3;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		this.input1.typeCheck();
		this.input2.typeCheck();

		// Is input1 a StreamOperator providing MemoryTuples?
		TypecheckTools.checkNodeType(this.input1, StreamOperator.class,
				this.getClass(), 1);
		this.streamInput1 = (StreamOperator) this.input1;
		TypecheckTools.checkOutputType(this.streamInput1, MemoryTuple.class,
				this.getClass(), 1);

		// Is input2 a StreamOperator providing MemoryTuples?
		TypecheckTools.checkNodeType(this.input2, StreamOperator.class,
				this.getClass(), 2);
		this.streamInput2 = (StreamOperator) this.input2;
		TypecheckTools.checkOutputType(this.streamInput2, MemoryTuple.class,
				this.getClass(), 2);

		// Is input3 a ParamterFunction?
		TypecheckTools.checkNodeType(this.input3, ParameterFunction.class,
				this.getClass(), 3);
		this.functionInput = (ParameterFunction) this.input3;

		// Typecheck ParameterFunction
		this.functionInput.setParamTypes(this.streamInput1.getOutputType(),
				this.streamInput2.getOutputType());
		this.functionInput.typeCheck(); // !!! Important !!!

		// Does the ParameterFunction provide an ObjectNode providing an
		// AttributeBool?
		TypecheckTools.checkFunctionOperatorType(this.functionInput,
				ObjectNode.class, this.getClass(), 3);
		TypecheckTools.checkFunctionOutputType(this.functionInput,
				AttributeBool.class, this.getClass(), 3);

		// Any identifier collisions?
		checkIdentifierCollisions();

		this.outputType = calcOutputType();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void open() throws MemoryException {
		// Open stream inputs
		this.streamInput1.open();
		this.streamInput2.open();

		// Initialize fields
		this.leftBuffer = new ArrayList<MemoryTuple>();
		this.rightBuffer = new ArrayList<MemoryTuple>();
		this.tupleStore = new ConcurrentLinkedQueue<MemoryTuple>();

		// Get first Tuple from right stream
		MemoryTuple firstTuple = (MemoryTuple) this.streamInput2.getNext();
		if (firstTuple != null) {
			this.rightBuffer.add(firstTuple);
		}
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getNext() throws MemoryException {
		if (this.tupleStore == null) {
			throw new StreamStateException(
					"Stream was accessed while being closed!");
		}

		// If store is not empty just put out first element in store
		if (!this.tupleStore.isEmpty()) {
			return this.tupleStore.poll();
		} else { // Fill store with next match(es)
			// If on open() no tuple could be fetched from right stream, it's
			// buffer is empty. There are no results.
			if (this.rightBuffer.isEmpty()) {
				return null;
			}

			// Check if both streams have ended
			if (this.leftEnded && this.rightEnded) {
				return null;
			}

			// Check if either side has ended
			if (this.leftEnded) {
				// Was left stream empty from the start?
				if (this.leftBuffer.isEmpty()) {
					return null;
				}
				// Try joining right tuples until results have been produced or
				// right stream also ends
				Success success = Success.NO_RESULTS;
				while (success == Success.NO_RESULTS) {
					success = joinRightTuple();
				}
				if (success == Success.RESULTS) {
					return this.tupleStore.poll();
				} else {
					return null;
				}
			}
			if (this.rightEnded) {
				// Try joining left tuples until results have been produced or
				// left stream also ends
				Success success = Success.NO_RESULTS;
				while (success == Success.NO_RESULTS) {
					success = joinLeftTuple();
				}
				if (success == Success.RESULTS) {
					return this.tupleStore.poll();
				} else {
					return null;
				}
			}

			// Join normally: Fetch left and right tuples alternating until
			// results have been produced
			Success success = Success.NO_RESULTS;
			while (success == Success.NO_RESULTS) {
				if (this.lastLeft) {
					success = joinRightTuple();
					this.lastLeft = false;
				} else {
					success = joinLeftTuple();
					this.lastLeft = true;
				}
			}
			if (success == Success.RESULTS) {
				return this.tupleStore.poll();
			} else {
				// One stream has ended. To prevent code duplications we just
				// recursively call getNext() which will use the remaining
				// stream to get results
				return getNext();
			}
		}
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void close() {
		this.streamInput1.close();
		this.streamInput2.close();
		this.rightBuffer = null;
		this.leftBuffer = null;
		this.tupleStore = null;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}

	/**
	 * Try to join a tuple coming from the left stream with all yet available
	 * tuples from the right stream. Potentially sets {@link Symmjoin#leftEnded}
	 * .
	 * 
	 * @return NO_RESULTS if there were no results with the next left Tuple,<br>
	 *         RESULTS if there were, or <br>
	 *         STREAM_ENDED if there was no more element in the left stream.
	 * @throws MemoryException
	 *             if during joining memory tended to run out.
	 */
	private Success joinLeftTuple() throws MemoryException {
		MemoryTuple leftTuple = (MemoryTuple) this.streamInput1.getNext();

		// Check if stream ended
		if (leftTuple == null) {
			this.leftEnded = true;
			return Success.STREAM_ENDED;
		}
		this.leftBuffer.add(leftTuple);

		// Try joining with all yet available tuples from right stream
		Success retVal = Success.NO_RESULTS;
		for (MemoryTuple rightTuple : this.rightBuffer) {
			ObjectNode functionResult = (ObjectNode) this.functionInput
					.evaluate(leftTuple, rightTuple);
			AttributeBool resultAttribute = (AttributeBool) functionResult
					.getResult();
			if (resultAttribute != null && resultAttribute.isValue()) {
				addJoinedTuplesToStore(leftTuple, rightTuple);
				retVal = Success.RESULTS;
			}
		}
		return retVal;
	}

	/**
	 * Try to join a tuple coming from the right stream with all yet available
	 * tuples from the left stream. Potentially sets {@link Symmjoin#rightEnded}
	 * .
	 * 
	 * @return NO_RESULTS if there were no results with the next right Tuple,<br>
	 *         RESULTS if there were, or <br>
	 *         STREAM_ENDED if there was no more element in the right stream.
	 * @throws MemoryException
	 *             if during joining memory tended to run out.
	 */
	private Success joinRightTuple() throws MemoryException {
		MemoryTuple rightTuple = (MemoryTuple) this.streamInput2.getNext();

		// Check if stream ended
		if (rightTuple == null) {
			this.rightEnded = true;
			return Success.STREAM_ENDED;
		}
		this.rightBuffer.add(rightTuple);

		// Try joining with all yet available tuples from left stream
		Success retVal = Success.NO_RESULTS;
		for (MemoryTuple leftTuple : this.leftBuffer) {
			ObjectNode functionResult = (ObjectNode) this.functionInput
					.evaluate(leftTuple, rightTuple);
			AttributeBool resultAttribute = (AttributeBool) functionResult
					.getResult();
			if (resultAttribute != null && resultAttribute.isValue()) {
				addJoinedTuplesToStore(leftTuple, rightTuple);
				retVal = Success.RESULTS;
			}
		}
		return retVal;
	}

	/**
	 * Check if there would be any identifier collisions in the output tuples.
	 * 
	 * @throws TypeException
	 *             if any collisions would occur.
	 */
	private void checkIdentifierCollisions() throws TypeException {
		List<RelationHeaderItem> header1 = ((MemoryTuple) this.streamInput1
				.getOutputType()).getTypecheckInfo();
		List<RelationHeaderItem> header2 = ((MemoryTuple) this.streamInput2
				.getOutputType()).getTypecheckInfo();

		for (RelationHeaderItem item1 : header1) {
			for (RelationHeaderItem item2 : header2) {
				if (item1.getIdentifier().equals(item2.getIdentifier())) {
					throw new TypeException(
							"%s: identifier collision on identifier: %s", this
									.getClass().getSimpleName(),
							item1.getIdentifier());
				}
			}
		}
	}

	/**
	 * Determines the OutputType by adding all columns of both input streams.
	 * 
	 * @return the determined OutputType.
	 */
	private MemoryTuple calcOutputType() {
		List<RelationHeaderItem> header1 = ((MemoryTuple) this.streamInput1
				.getOutputType()).getTypecheckInfo();
		List<RelationHeaderItem> header2 = ((MemoryTuple) this.streamInput2
				.getOutputType()).getTypecheckInfo();

		List<RelationHeaderItem> outputHeader = new ArrayList<RelationHeaderItem>();
		outputHeader.addAll(header1);
		outputHeader.addAll(header2);
		return MemoryTuple.createTypecheckInstance(outputHeader);
	}

	/**
	 * Joins two tuples and adds the result tuple to {@link Symmjoin#tupleStore}
	 * .
	 * 
	 * @param leftTuple
	 * @param rightTuple
	 */
	private void addJoinedTuplesToStore(MemoryTuple leftTuple,
			MemoryTuple rightTuple) {
		MemoryTuple newTuple = new MemoryTuple();
		for (MemoryAttribute attribute : leftTuple.getAttributes()) {
			newTuple.addAttribute(attribute);
		}
		for (MemoryAttribute attribute : rightTuple.getAttributes()) {
			newTuple.addAttribute(attribute);
		}
		this.tupleStore.add(newTuple);
	}

}
