package mmdb.streamprocessing.objectnodes;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.streamoperators.StreamOperator;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;

public class Consume implements ObjectNode {

	private Node input;

	private StreamOperator streamInput;

	private MemoryRelation outputType;

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 1, Consume.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		return new Consume(node1);
	}

	public Consume(Node input) {
		this.input = input;
	}

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

	@Override
	public MemoryRelation getResult() {
		List<RelationHeaderItem> typecheckInfo = this.outputType
				.getTypecheckInfo();
		MemoryRelation resultRelation = new MemoryRelation(typecheckInfo);
		resultRelation.setTuples(getTuples());
		return resultRelation;
	}

	@Override
	public MemoryRelation getOutputType() {
		return this.outputType;
	}

	private List<MemoryTuple> getTuples() {
		List<MemoryTuple> tuples = new ArrayList<MemoryTuple>();
		this.streamInput.open();
		MemoryTuple tuple;
		while ((tuple = (MemoryTuple) this.streamInput.getNext()) != null) {
			tuples.add(tuple);
		}
		this.streamInput.close();
		return tuples;
	}

}
