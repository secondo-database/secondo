package unittests.mmdb.streamprocessing.parser;

import gui.SecondoObject;

import java.util.ArrayList;

import javax.swing.JOptionPane;

import mmdb.data.MemoryObject;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;

import org.junit.Test;

public class NestedListProcessorTest {

	@Test
	public void testBuildOperatorTreeOptionPane() throws ParsingException,
			TypeException {
		String query = JOptionPane.showInputDialog("Please enter the query!");
		ObjectNode objectNode = NestedListProcessor.buildOperatorTree(query,
				new ArrayList<SecondoObject>());
		objectNode.typeCheck();
		MemoryObject mo = objectNode.getResult();
		System.out.println("BLA");
	}

	@Test
	public void testBuildOperatorTreeString() throws ParsingException,
			TypeException {
		// String query =
		// "(query (consume (filter (feed TestTabelle) (fun (elem TUPLE) (or (= (attr elem identifierInt) 4) (= (attr elem identifierString) \"string_2\"))))))";
		// String query =
		// "(query (consume (hashjoin (feed TestTabelle) (rename (feed TestTabelle) t) identifierInt identifierInt_t)))";
		String query = "(query (consume (groupby (sortby (feed TestTabelle) ((identifierString asc))) (identifierString) ((IntSum (fun (group GROUP) (sum (feed group) identifierInt)))))))";
		ObjectNode objectNode = NestedListProcessor.buildOperatorTree(query,
				new ArrayList<SecondoObject>());
		objectNode.typeCheck();
		MemoryObject mo = objectNode.getResult();
		System.out.println("BLA");
	}

}
