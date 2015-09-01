package mmdb.streamprocessing.objectnodes;

import mmdb.data.MemoryRelation;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.streamoperators.StreamOperator;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;

public class Count implements ObjectNode {

	private Node input;

	private AttributeInt outputType;

	private InputVariant inputVariant;

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 1, Count.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		return new Count(node1);
	}

	private enum InputVariant {
		STREAM, RELATION
	}

	public Count(Node input) {
		this.input = input;
	}

	@Override
	public void typeCheck() throws TypeException {
		this.input.typeCheck();

		// Is input a StreamOperator or an ObjectNode?
		TypecheckTools.checkMultipleNodeTypes(this.input, this.getClass(), 1,
				StreamOperator.class, ObjectNode.class);

		this.inputVariant = determineInputVariant();

		this.outputType = new AttributeInt();
	}

	@Override
	public MemoryAttribute getResult() {
		switch (this.inputVariant) {
		case STREAM:
			return new AttributeInt(streamCalculation());
		case RELATION:
			int relResult = relationCalculation();
			if (relResult < 0) {
				return null;
			} else {
				return new AttributeInt(relResult);
			}
		default:
			return null;
		}
	}

	@Override
	public AttributeInt getOutputType() {
		return this.outputType;
	}

	private int relationCalculation() {
		ObjectNode objectInput = (ObjectNode) this.input;
		MemoryRelation relation = (MemoryRelation) objectInput.getResult();
		if (relation == null) {
			return -1;
		}

		int count = relation.getTuples().size();
		return count;
	}

	private int streamCalculation() {
		StreamOperator streamInput = (StreamOperator) this.input;
		int count = 0;

		streamInput.open();
		while (streamInput.getNext() != null) {
			count++;
		}
		streamInput.close();
		return count;
	}

	private InputVariant determineInputVariant() throws TypeException {
		if (this.input instanceof StreamOperator) {
			return InputVariant.STREAM;
		} else {
			// Does ObjectNode provide a MemoryRelation?
			TypecheckTools.checkOutputType(this.input, MemoryRelation.class,
					this.getClass(), 1);
			return InputVariant.RELATION;
		}
	}

}
