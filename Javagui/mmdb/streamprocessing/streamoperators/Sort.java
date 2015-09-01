package mmdb.streamprocessing.streamoperators;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import mmdb.data.MemoryObject;
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

public class Sort implements StreamOperator {

	private Node input;

	private StreamOperator streamInput;

	private Sortby sortby;

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 1, Sort.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		return new Sort(node1);
	}

	// TODO: Sort non-tuple streams?
	public Sort(Node input) {
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

		Map<String, String> sortAttributes = calcSortAttributes();

		// Delegate to Sortby
		this.sortby = new Sortby(this.streamInput, sortAttributes);
		this.sortby.typeCheck();
	}

	@Override
	public MemoryObject getOutputType() {
		return this.sortby.getOutputType();
	}

	@Override
	public void open() {
		this.sortby.open();
	}

	@Override
	public MemoryObject getNext() {
		return this.sortby.getNext();
	}

	@Override
	public void close() {
		this.sortby.close();
	}

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
