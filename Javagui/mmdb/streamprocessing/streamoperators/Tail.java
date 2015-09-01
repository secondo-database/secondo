package mmdb.streamprocessing.streamoperators;

import java.util.Iterator;
import java.util.LinkedList;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;

public class Tail implements StreamOperator {

	private Node input1, input2;

	private StreamOperator streamInput;

	private ObjectNode objectInput;

	private MemoryObject outputType;

	private LinkedList<MemoryObject> bufferList;

	private Iterator<MemoryObject> iterator;

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Tail.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Node node2 = NestedListProcessor.nlToNode(params[1], environment);
		return new Tail(node1, node2);
	}

	public Tail(Node input1, Node input2) {
		this.input1 = input1;
		this.input2 = input2;
	}

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

	@Override
	public void open() {
		this.streamInput.open();
		AttributeInt outputLimitAttr = (AttributeInt) this.objectInput
				.getResult();
		if (outputLimitAttr == null) {
			initBuffer(0);
		} else {
			initBuffer(outputLimitAttr.getValue());
		}

	}

	@Override
	public MemoryObject getNext() {
		if (this.iterator.hasNext()) {
			return this.iterator.next();
		} else {
			return null;
		}
	}

	@Override
	public void close() {
		this.streamInput.close();
	}

	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}

	private void initBuffer(int outputLimit) {
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
