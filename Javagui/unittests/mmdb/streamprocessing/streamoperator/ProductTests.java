package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;
import gui.SecondoObject;
import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.Count;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.streamoperators.Feed;
import mmdb.streamprocessing.streamoperators.Product;
import mmdb.streamprocessing.streamoperators.Rename;

import org.junit.Before;
import org.junit.Test;

import unittests.mmdb.util.TestUtilParser;
import unittests.mmdb.util.TestUtilRelation;

public class ProductTests {

	MemoryRelation rel = null;
	ObjectNode relNode = null;
	Feed feed1 = null;
	Feed feed2 = null;
	Rename rename = null;
	Product product = null;

	@Before
	public void setUp() throws TypeException {
		rel = TestUtilRelation.getIntStringRelation(3, true, false);
		relNode = ConstantNode.createConstantNode(rel, rel);
		feed1 = new Feed(relNode);
		feed2 = new Feed(relNode);

		rename = new Rename(feed2, "rel2");
		product = new Product(feed1, rename);
	}

	@Test
	public void testProductHeader() throws TypeException {
		product.typeCheck();
		String[] attributeNames = { "identifierString", "identifierInt",
				"identifierString_rel2", "identifierInt_rel2" };
		for (int i = 0; i < 4; i++) {
			assertEquals(attributeNames[i],
					((MemoryTuple) product.getOutputType()).getTypecheckInfo()
							.get(i).getIdentifier());
		}
	}

	@Test
	public void testProductTuplesCount() throws TypeException, MemoryException {
		Count count = new Count(product);
		count.typeCheck();
		assertEquals(9, ((AttributeInt) count.getResult()).getValue());
	}

	@Test
	public void testProductLeftStreamEmpty() throws TypeException,
			MemoryException {
		// Left stream empty
		MemoryRelation rel1 = TestUtilRelation.getIntStringRelation(0, false,
				false);
		MemoryRelation rel2 = TestUtilRelation.getIntStringRelation(5, false,
				false);
		ObjectNode relNode1 = ConstantNode.createConstantNode(rel1, rel1);
		ObjectNode relNode2 = ConstantNode.createConstantNode(rel2, rel2);
		feed1 = new Feed(relNode1);
		feed2 = new Feed(relNode2);
		rename = new Rename(feed2, "rel2");
		product = new Product(feed1, rename);
		Count count = new Count(product);
		count.typeCheck();
		assertEquals(0, ((AttributeInt) count.getResult()).getValue());
	}

	@Test
	public void testProductRightStreamEmpty() throws TypeException,
			MemoryException {
		MemoryRelation rel1 = TestUtilRelation.getIntStringRelation(5, false,
				false);
		MemoryRelation rel2 = TestUtilRelation.getIntStringRelation(0, false,
				false);
		ObjectNode relNode1 = ConstantNode.createConstantNode(rel1, rel1);
		ObjectNode relNode2 = ConstantNode.createConstantNode(rel2, rel2);
		feed1 = new Feed(relNode1);
		feed2 = new Feed(relNode2);
		rename = new Rename(feed2, "rel2");
		product = new Product(feed1, rename);
		Count count = new Count(product);
		count.typeCheck();

		assertEquals(0, ((AttributeInt) count.getResult()).getValue());
	}

	@Test(expected = TypeException.class)
	public void testProductFail1() throws TypeException {
		product = new Product(feed1, feed2);
		product.typeCheck();
	}

	@Test(expected = TypeException.class)
	public void testProductFail2() throws TypeException {
		MemoryRelation rel2 = TestUtilRelation.getIntStringRelation(5, false,
				true);
		ObjectNode relNode2 = ConstantNode.createConstantNode(rel2, rel2);
		feed2 = new Feed(relNode2);
		product = new Product(feed1, feed2);
		product.typeCheck();
	}

	@Test
	public void testQuery() throws Exception {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(4, false,
				false);
		SecondoObject sobject1 = TestUtilParser.getSecondoObject(rel, "REL1");
		SecondoObject sobject2 = TestUtilParser.getSecondoObject(rel, "REL2");
		String query = "(query (count (product (rename (feed REL1) a) (feed REL2))))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				TestUtilParser.getList(sobject1, sobject2));
		result.typeCheck();
		MemoryObject mobject = result.getResult();

		MemoryObject expected = new AttributeInt(16);
		assertEquals(expected, mobject);
	}

}
