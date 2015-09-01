package unittests.mmdb.streamprocessing.objectnodes;

import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;

import org.junit.Test;

public class ConstantNodeTests {

	@Test(expected = TypeException.class)
	public void testWrappingObjectNodeTypeCheckFail() throws TypeException {
		ObjectNode on = ConstantNode.createConstantNode(null, null);
		on.typeCheck();
	}

	@Test(expected = TypeException.class)
	public void testWrappingObjectNodeTypeCheckFail2() throws TypeException {
		ObjectNode on = ConstantNode.createConstantNode(new AttributeInt(3), null);
		on.typeCheck();
	}

}
