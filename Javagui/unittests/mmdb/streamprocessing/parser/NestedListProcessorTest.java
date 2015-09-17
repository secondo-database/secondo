package unittests.mmdb.streamprocessing.parser;

import gui.SecondoObject;

import java.util.ArrayList;

import mmdb.error.streamprocessing.ParsingException;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;

import org.junit.Test;

import sj.lang.ListExpr;

public class NestedListProcessorTest {

	// @Test(expected = ParsingException.class)
	// public void testBuildOperatorTreeFail1() throws ParsingException {
	// NestedListProcessor.buildOperatorTree("(abc))",
	// new ArrayList<SecondoObject>());
	// }

	@Test(expected = ParsingException.class)
	public void testBuildOperatorTreeFail2() throws ParsingException {
		NestedListProcessor.buildOperatorTree("(abc)",
				new ArrayList<SecondoObject>());
	}

	@Test(expected = ParsingException.class)
	public void testBuildOperatorTreeFail3() throws ParsingException {
		NestedListProcessor.buildOperatorTree("((query) abc)",
				new ArrayList<SecondoObject>());
	}

	@Test(expected = ParsingException.class)
	public void testBuildOperatorTreeFail4() throws ParsingException {
		NestedListProcessor.buildOperatorTree("(query abc 123)",
				new ArrayList<SecondoObject>());
	}

	@Test(expected = ParsingException.class)
	public void testBuildOperatorTreeFail5() throws ParsingException {
		// No realistic query: fun is not an ObjectNode so the error is provoked
		NestedListProcessor.buildOperatorTree(
				"(query (fun (tuple TUPLE) (+ 1 3)))",
				new ArrayList<SecondoObject>());
	}

	@Test(expected = ParsingException.class)
	public void testNlToIdentifierFail() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("(abc)");
		NestedListProcessor.nlToIdentifier(listExpr);
	}

	@Test(expected = ParsingException.class)
	public void testNlToIdentifierArrayFail1() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("abc");
		NestedListProcessor.nlToIdentifierArray(listExpr);
	}

	@Test(expected = ParsingException.class)
	public void testNlToIdentifierArrayFail2() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("(abc 123 def)");
		NestedListProcessor.nlToIdentifierArray(listExpr);
	}

	@Test(expected = ParsingException.class)
	public void testNlToFunctionEnvironmentNameFail1() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("(abc 123 def)");
		NestedListProcessor.nlToFunctionEnvironmentName(listExpr);
	}

	@Test(expected = ParsingException.class)
	public void testNlToFunctionEnvironmentNameFail2() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("(abc 123)");
		NestedListProcessor.nlToFunctionEnvironmentName(listExpr);
	}

	@Test(expected = ParsingException.class)
	public void testNlToFunctionEnvironmentNameFail3() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("(123 abc)");
		NestedListProcessor.nlToFunctionEnvironmentName(listExpr);
	}

	@Test(expected = ParsingException.class)
	public void testNlToIdentifierPairsFail1() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("((abc bcd cde))");
		NestedListProcessor.nlToIdentifierPairs(listExpr);
	}

	@Test(expected = ParsingException.class)
	public void testNlToIdentifierPairsFail2() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("((abc 123))");
		NestedListProcessor.nlToIdentifierPairs(listExpr);
	}

	@Test(expected = ParsingException.class)
	public void testNlToIdentifierPairsFail3() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("((123 abc))");
		NestedListProcessor.nlToIdentifierPairs(listExpr);
	}

	@Test(expected = ParsingException.class)
	public void testNlToIdentifierNodePairsFail1() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("((123 abc 123))");
		NestedListProcessor.nlToIdentifierNodePairs(listExpr, new Environment(
				new ArrayList<SecondoObject>()));
	}

	@Test(expected = ParsingException.class)
	public void testNlToIdentifierNodePairsFail2() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("((123 abc))");
		NestedListProcessor.nlToIdentifierNodePairs(listExpr, new Environment(
				new ArrayList<SecondoObject>()));
	}

	@Test(expected = ParsingException.class)
	public void testNlToIdentifierNodePairsFail3() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("((abc 123))");
		NestedListProcessor.nlToIdentifierNodePairs(listExpr, new Environment(
				new ArrayList<SecondoObject>()));
	}

	@Test(expected = ParsingException.class)
	public void testAtomToNodeFail() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("abc");
		NestedListProcessor.nlToNode(listExpr, new Environment(
				new ArrayList<SecondoObject>()));
	}

	@Test(expected = ParsingException.class)
	public void testListToNodeFail1() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("((abc) 123)");
		NestedListProcessor.nlToNode(listExpr, new Environment(
				new ArrayList<SecondoObject>()));
	}

	@Test(expected = ParsingException.class)
	public void testListToNodeFail2() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("(plu_s 1 3)");
		NestedListProcessor.nlToNode(listExpr, new Environment(
				new ArrayList<SecondoObject>()));
	}

	@Test(expected = ParsingException.class)
	public void testCallMethodFromNL() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("(+ 1 3 4)");
		NestedListProcessor.nlToNode(listExpr, new Environment(
				new ArrayList<SecondoObject>()));
	}

}
