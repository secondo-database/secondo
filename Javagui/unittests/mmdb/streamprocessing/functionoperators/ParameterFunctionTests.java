package unittests.mmdb.streamprocessing.functionoperators;

import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.functionoperators.ParameterFunction;
import mmdb.streamprocessing.objectnodes.ConstantNode;

import org.junit.Test;

public class ParameterFunctionTests {

	@Test(expected = TypeException.class)
	public void testWrongParameterNumber() throws TypeException {
		// This tests with absolute nonsense Nodes, just to see if lengths are
		// checked correctly!
		Node operator = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		Node param1 = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		Node param2 = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		Node[] paramArr = { param1, param2 };
		ParameterFunction par = new ParameterFunction(paramArr, operator);
		par.setParamTypes(new AttributeInt());
	}

}
