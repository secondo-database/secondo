package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import gui.SecondoObject;
import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.streamoperators.Feed;
import mmdb.streamprocessing.streamoperators.Project;

import org.junit.Before;
import org.junit.Test;

import unittests.mmdb.util.TestUtilParser;
import unittests.mmdb.util.TestUtilRelation;

public class ProjectTests {

	MemoryRelation rel = null;
	ObjectNode relNode = null;
	Feed feed = null;
	Project project = null;

	@Before
	public void setUp() {
		rel = TestUtilRelation.getIntStringRelation(5, true, false);
		relNode = ConstantNode.createConstantNode(rel, rel);
		feed = new Feed(relNode);
	}

	@Test
	public void testProjectTuples1() throws TypeException, MemoryException {
		project = new Project(feed, new String[] { "identifierInt" });
		project.typeCheck();

		project.open();
		for (int i = 1; i <= 5; i++) {
			assertEquals(i,
					((AttributeInt) project.getNext().getAttribute(0))
							.getValue());
		}
		project.close();
	}

	@Test
	public void testProjectTuples2() throws TypeException, MemoryException {
		project = new Project(feed, new String[] { "identifierInt",
				"identifierString" });
		project.typeCheck();

		project.open();
		for (int i = 1; i <= 5; i++) {
			MemoryTuple tuple = project.getNext();
			assertEquals(i, ((AttributeInt) tuple.getAttribute(0)).getValue());
			assertEquals("string_" + i,
					((AttributeString) tuple.getAttribute(1)).getValue());
		}
		project.close();
	}

	@Test
	public void testProjectTuplesEmpty() throws TypeException, MemoryException {
		rel = TestUtilRelation.getIntStringRelation(0, true, false);
		relNode = ConstantNode.createConstantNode(rel, rel);
		feed = new Feed(relNode);
		project = new Project(feed, new String[] { "identifierInt",
				"identifierString" });
		project.typeCheck();

		project.open();
		assertNull(project.getNext());
		project.close();
	}

	@Test
	public void testProjectHeader1() throws TypeException {
		project = new Project(feed, new String[] { "identifierInt" });
		project.typeCheck();

		assertEquals(1, ((MemoryTuple) project.getOutputType())
				.getTypecheckInfo().size());
		assertEquals("identifierInt", ((MemoryTuple) project.getOutputType())
				.getTypecheckInfo().get(0).getIdentifier());
	}

	@Test
	public void testProjectHeader2() throws TypeException {
		project = new Project(feed, new String[] { "identifierInt",
				"identifierString" });
		project.typeCheck();

		assertEquals(2, ((MemoryTuple) project.getOutputType())
				.getTypecheckInfo().size());
		assertEquals("identifierInt", ((MemoryTuple) project.getOutputType())
				.getTypecheckInfo().get(0).getIdentifier());
		assertEquals(
				"identifierString",
				((MemoryTuple) project.getOutputType()).getTypecheckInfo()
						.get(1).getIdentifier());
	}

	@Test(expected = TypeException.class)
	public void testProjectHeaderFail1() throws TypeException {
		project = new Project(feed, new String[] { "identifierReal" });
		project.typeCheck();
	}

	@Test(expected = TypeException.class)
	public void testProjectHeaderFail2() throws TypeException {
		project = new Project(feed, new String[] { "identifierInt",
				"identifierInt" });
		project.typeCheck();
	}

	@Test
	public void testQuery() throws Exception {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(1, false,
				false);
		SecondoObject sobject = TestUtilParser.getSecondoObject(rel, "REL");
		String query = "(query (consume (project (feed REL) (identifierInt))))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				TestUtilParser.getList(sobject));
		result.typeCheck();
		MemoryObject mobject = result.getResult();

		MemoryRelation expected = TestUtilRelation.getIntStringRelation(0,
				false, false);
		expected.getHeader().remove(1);
		expected.getTuples().add(new MemoryTuple(new AttributeInt(1)));
		assertEquals(expected, mobject);
	}

}
