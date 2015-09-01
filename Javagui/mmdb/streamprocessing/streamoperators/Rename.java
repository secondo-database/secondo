package mmdb.streamprocessing.streamoperators;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;

public class Rename implements StreamOperator {

	private Node input;

	private StreamOperator streamInput;

	private MemoryTuple outputType;

	private String postfix;

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Rename.class);
		Node node = NestedListProcessor.nlToNode(params[0], environment);
		String identParam = NestedListProcessor.nlToIdentifier(params[1]);
		return new Rename(node, identParam);
	}

	public Rename(Node input1, String postfix) {
		this.input = input1;
		this.postfix = postfix;
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

		this.outputType = this.calculateOutputType();
	}

	@Override
	public void open() {
		this.streamInput.open();
	}

	@Override
	public MemoryTuple getNext() {
		return (MemoryTuple) this.streamInput.getNext();
	}

	@Override
	public void close() {
		this.streamInput.close();
	}

	@Override
	public MemoryTuple getOutputType() {
		return this.outputType;
	}

	private MemoryTuple calculateOutputType() throws TypeException {
		List<RelationHeaderItem> inputType = ((MemoryTuple) this.streamInput
				.getOutputType()).getTypecheckInfo();

		if (this.postfix == null || this.postfix.length() < 1) {
			throw new TypeException(
					"Postfix needs a fixed String, but got null or \"\"");
		}

		List<RelationHeaderItem> outputType = new ArrayList<RelationHeaderItem>();
		for (RelationHeaderItem inputHeaderItem : inputType) {
			String identifier = inputHeaderItem.getIdentifier() + "_"
					+ this.postfix;
			String typeName = inputHeaderItem.getTypeName();
			RelationHeaderItem outputHeaderItem = new RelationHeaderItem(
					identifier, typeName);
			outputType.add(outputHeaderItem);
		}

		return MemoryTuple.createTypecheckInstance(outputType);
	}

}
