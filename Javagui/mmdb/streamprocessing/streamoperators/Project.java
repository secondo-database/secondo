package mmdb.streamprocessing.streamoperators;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.tools.HeaderTools;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;

public class Project implements StreamOperator {

	private Node input;

	private StreamOperator streamInput;

	private String[] identifiers;

	private Map<Integer, Integer> columnPositions;

	private MemoryTuple outputType;

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Project.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		String[] identifierParams = NestedListProcessor
				.nlToIdentifierArray(params[1]);
		return new Project(node1, identifierParams);
	}

	public Project(Node input, String[] identifiers) {
		this.input = input;
		this.identifiers = identifiers;
	}

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

	@Override
	public void open() {
		this.streamInput.open();
	}

	@Override
	public MemoryTuple getNext() {
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

	@Override
	public void close() {
		this.streamInput.close();
	}

	@Override
	public MemoryTuple getOutputType() {
		return this.outputType;
	}

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
