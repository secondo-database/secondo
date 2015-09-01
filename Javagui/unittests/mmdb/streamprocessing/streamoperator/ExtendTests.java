package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import java.util.LinkedHashMap;
import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.functionoperators.ParameterFunction;
import mmdb.streamprocessing.objectnodes.Attr;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.FunctionEnvironment;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.maths.Plus;
import mmdb.streamprocessing.streamoperators.Extend;
import mmdb.streamprocessing.streamoperators.Feed;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

public class ExtendTests {

	@Test
	public void testExtendIntString() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(2, false,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);

		// Function1
		ObjectNode attrConst1 = ConstantNode.createConstantNode(
				new AttributeInt(1), new AttributeInt());
		FunctionEnvironment funEnv1 = new FunctionEnvironment();
		Attr attr1 = new Attr(funEnv1, "identifierInt");
		Plus plus1 = new Plus(attr1, attrConst1);
		ParameterFunction fun1 = new ParameterFunction(new Node[] { funEnv1 },
				plus1);

		// Function2
		ObjectNode attrConst2 = ConstantNode.createConstantNode(
				new AttributeString("_NEU"), new AttributeString());
		FunctionEnvironment funEnv2 = new FunctionEnvironment();
		Attr attr2 = new Attr(funEnv2, "identifierString");
		Plus plus2 = new Plus(attr2, attrConst2);
		ParameterFunction fun2 = new ParameterFunction(new Node[] { funEnv2 },
				plus2);

		// Hashmap
		LinkedHashMap<String, Node> map = new LinkedHashMap<String, Node>();
		map.put("plus1", fun1);
		map.put("postfix", fun2);

		// Extend
		Extend extend = new Extend(feed, map);
		extend.typeCheck();

		// Tests
		MemoryTuple tupleInfo = (MemoryTuple) extend.getOutputType();
		List<RelationHeaderItem> list = tupleInfo.getTypecheckInfo();
		assertEquals("identifierInt", list.get(0).getIdentifier());
		assertEquals("identifierString", list.get(1).getIdentifier());
		assertEquals("plus1", list.get(2).getIdentifier());
		assertEquals("postfix", list.get(3).getIdentifier());

		extend.open();
		MemoryTuple tuple = (MemoryTuple) extend.getNext();
		assertEquals(1, ((AttributeInt) tuple.getAttribute(0)).getValue());
		assertEquals("string_1",
				((AttributeString) tuple.getAttribute(1)).getValue());
		assertEquals(2, ((AttributeInt) tuple.getAttribute(2)).getValue());
		assertEquals("string_1_NEU",
				((AttributeString) tuple.getAttribute(3)).getValue());

		tuple = (MemoryTuple) extend.getNext();
		assertEquals("string_2",
				((AttributeString) tuple.getAttribute(1)).getValue());
		assertEquals(3, ((AttributeInt) tuple.getAttribute(2)).getValue());
		assertEquals("string_2_NEU",
				((AttributeString) tuple.getAttribute(3)).getValue());

		tuple = (MemoryTuple) extend.getNext();
		assertNull(tuple);
		extend.close();
	}

	@Test(expected = TypeException.class)
	public void testExtendIdentifierDuplication() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);

		// Function1
		ObjectNode attrConst1 = ConstantNode.createConstantNode(
				new AttributeInt(1), new AttributeInt());
		FunctionEnvironment funEnv1 = new FunctionEnvironment();
		Attr attr1 = new Attr(funEnv1, "identifierInt");
		Plus plus1 = new Plus(attr1, attrConst1);
		ParameterFunction fun1 = new ParameterFunction(new Node[] { funEnv1 },
				plus1);

		// Function2
		ObjectNode attrConst2 = ConstantNode.createConstantNode(
				new AttributeString("_NEU"), new AttributeString());
		FunctionEnvironment funEnv2 = new FunctionEnvironment();
		Attr attr2 = new Attr(funEnv2, "identifierString");
		Plus plus2 = new Plus(attr2, attrConst2);
		ParameterFunction fun2 = new ParameterFunction(new Node[] { funEnv2 },
				plus2);

		// Hashmap
		LinkedHashMap<String, Node> map = new LinkedHashMap<String, Node>();
		map.put("identifierString", fun1);
		map.put("postfix", fun2);

		// Extend
		Extend extend = new Extend(feed, map);
		extend.typeCheck();
	}

}
