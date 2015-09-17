package unittests.mmdb.streamprocessing.parser;

import static org.junit.Assert.assertEquals;
import gui.SecondoObject;
import gui.idmanager.IDManager;

import java.util.ArrayList;

import mmdb.error.streamprocessing.ParsingException;
import mmdb.streamprocessing.parser.Environment;

import org.junit.Test;

import sj.lang.ListExpr;

public class EnvironmentTests {

	@Test
	public void testRemoveResultLabel() {
		assertEquals("abc", Environment.removeResultLabel("R13: abc"));
		assertEquals("R1aa3 sd: abc",
				Environment.removeResultLabel("R1aa3 sd: abc"));
	}

	@Test
	public void testRemoveMMIndicator() {
		assertEquals("abc", Environment.removeMMIndicator("abc [++]"));
		assertEquals("bcd", Environment.removeMMIndicator("bcd [+]"));
		assertEquals("cde [+a+]", Environment.removeMMIndicator("cde [+a+]"));
	}

	@Test
	public void testGetMMIndicator() {
		SecondoObject sobject1 = new SecondoObject(IDManager.getNextID());
		SecondoObject sobject2 = new SecondoObject(IDManager.getNextID());
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("(int 3)");
		sobject2.fromList(listExpr);
		
		assertEquals(" [+]", Environment.getMMIndicator(sobject1));
		assertEquals(" [++]", Environment.getMMIndicator(sobject2));
	}

	@Test(expected = ParsingException.class)
	public void testAddNameTwice() throws ParsingException {
		Environment env = new Environment(new ArrayList<SecondoObject>());
		env = env.addObjectToNewEnvironment("aaa", null);
		env = env.addObjectToNewEnvironment("aaa", null);
	}

}
