package unittests.mmdb.streamprocessing.parser;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import gui.SecondoObject;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.streamprocessing.parser.ParserController;

import org.junit.Before;
import org.junit.Test;

import sj.lang.ESInterface;
import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilMocks.CommandPanelMock;
import unittests.mmdb.util.TestUtilMocks.ObjectListMock;
import unittests.mmdb.util.TestUtilMocks.ViewerControlMock;
import unittests.mmdb.util.TestUtilRelation;

public class ParserControllerTests {

	private ObjectListMock objectList;
	private CommandPanelMock commandPanel;
	private ViewerControlMock viewerControl;

	@Before
	public void init() {
		objectList = new ObjectListMock();
		SecondoObject firstObject = new SecondoObject("object_1 [+]", null);
		firstObject.setMemoryObject(TestUtilRelation.getIntStringRelation(1,
				false, false));
		SecondoObject secondObject = new SecondoObject("object_2 [+]", null);
		secondObject.setMemoryObject(TestUtilRelation.getIntStringRelation(0,
				true, false));
		objectList.addEntry(firstObject);
		objectList.addEntry(secondObject);
		commandPanel = new CommandPanelMock();
		viewerControl = new ViewerControlMock();
		ParserController.getInstance().injectGuiElements(objectList,
				commandPanel, viewerControl);
	}

	@Test
	public void testProcessMMDBQuery() {
		ParserController.getInstance().processMMDBQuery("mmdb (query (+ 3 4))",
				new ESInterface());
		assertEquals(3, objectList.getAllObjects().size());
		assertEquals(new AttributeInt(7), objectList.getAllObjects().get(2)
				.getMemoryObject());
		assertTrue(viewerControl.isActualDisplayed(objectList.getAllObjects()
				.get(2)));
	}

	@Test
	public void testProcessMMDBQueryNoAutoconvert() {
		ParserController.getInstance().processResultAutoconvert();
		ParserController.getInstance().processMMDBQuery("mmdb (query (+ 3 4))",
				new ESInterface());
		assertEquals(3, objectList.getAllObjects().size());
		assertEquals(new AttributeInt(7), objectList.getAllObjects().get(2)
				.getMemoryObject());
		assertNull(objectList.getAllObjects().get(2).toListExpr());
		assertFalse(viewerControl.isActualDisplayed(objectList.getAllObjects()
				.get(2)));
	}

	@Test
	public void testProcessMMDBQueryUndefined() {
		ParserController.getInstance().processResultAutoconvert();
		ParserController.getInstance().processMMDBQuery(
				"mmdb (query (max (feed object_2) identifierInt));",
				new ESInterface());
		ListExpr listExpr = new ListExpr();
		listExpr.readFromString("(int undefined)");

		assertEquals(3, objectList.getAllObjects().size());
		assertNull(objectList.getAllObjects().get(2).getMemoryObject());
		assertEquals(listExpr, objectList.getAllObjects().get(2).toListExpr());
		assertFalse(viewerControl.isActualDisplayed(objectList.getAllObjects()
				.get(2)));
	}

	@Test
	public void testProcessMMDBQueryFail() throws Exception {
		ParserController.getInstance().processMMDBQuery("query 3 + 4",
				new ESInterface());
		assertEquals(
				"Sec>\n\n  Query could not be transformed to nested list format: Not connected to secondo server?\nSec>",
				commandPanel.SystemArea.getText());
	}

}
