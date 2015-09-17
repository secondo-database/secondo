package mmdb.streamprocessing.streamoperators;

import mmdb.data.MemoryTuple;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.ParserTools;
import sj.lang.ListExpr;

/**
 * Simple concatenation of operators feed and project.<br>
 * Resembles the operator in the core.<br>
 * Transforms a MemoryRelation to a stream of MemoryTuples only containing
 * selected columns.
 * 
 * @see mmdb.streamprocessing.streamoperators.Feed
 * @see mmdb.streamprocessing.streamoperators.Project
 * 
 * @author Björn Clasen
 */
public class FeedProject implements StreamOperator {

	/**
	 * The feed operator to delegate to.
	 */
	private Feed feed;

	/**
	 * The project operator to delegate to.
	 */
	private Project project;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, FeedProject.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		String[] identifierParams = NestedListProcessor
				.nlToIdentifierArray(params[1]);
		return new FeedProject(node1, identifierParams);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input
	 *            operator's first parameter
	 * @param identifiers
	 *            operators identifier parameters
	 */
	public FeedProject(Node input, String[] identifiers) {
		this.feed = new Feed(input);
		this.project = new Project(this.feed, identifiers);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		this.feed.typeCheck();
		this.project.typeCheck();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void open() throws MemoryException {
		this.project.open();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryTuple getNext() throws MemoryException {
		return this.project.getNext();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void close() {
		this.project.close();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryTuple getOutputType() {
		return this.project.getOutputType();
	}

}
