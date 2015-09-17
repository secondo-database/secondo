package unittests.mmdb.streamprocessing.tools;

import mmdb.error.streamprocessing.ParsingException;
import mmdb.streamprocessing.objectnodes.maths.Plus;
import mmdb.streamprocessing.tools.ParserTools;

import org.junit.Test;

import sj.lang.ListExpr;

public class ParserToolsTest {

	@Test
	public void testFullCoverageDummy() {
		@SuppressWarnings("unused")
		ParserTools tools = new ParserTools() {
		};
	}

	@Test(expected = ParsingException.class)
	public void testCheckListElemCountFail() throws ParsingException {
		ListExpr listExpr = new ListExpr();
		ParserTools.checkListElemCount(new ListExpr[] { listExpr }, 2,
				Plus.class);
	}

}
