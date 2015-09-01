package mmdb.streamprocessing.streamoperators;

import mmdb.data.MemoryTuple;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.tools.ParserTools;

public class FeedProject implements StreamOperator {

	private Feed feed;

	private Project project;

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, FeedProject.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		String[] identifierParams = NestedListProcessor
				.nlToIdentifierArray(params[1]);
		return new FeedProject(node1, identifierParams);
	}

	public FeedProject(Node input, String[] identifiers) {
		this.feed = new Feed(input);
		this.project = new Project(this.feed, identifiers);
	}

	@Override
	public void typeCheck() throws TypeException {
		this.feed.typeCheck();
		this.project.typeCheck();
	}

	@Override
	public void open() {
		this.project.open();
	}

	@Override
	public MemoryTuple getNext() {
		return this.project.getNext();
	}

	@Override
	public void close() {
		this.project.close();
	}

	@Override
	public MemoryTuple getOutputType() {
		return this.project.getOutputType();
	}

}
