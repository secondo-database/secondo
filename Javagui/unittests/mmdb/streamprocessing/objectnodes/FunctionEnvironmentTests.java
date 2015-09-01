package unittests.mmdb.streamprocessing.objectnodes;

import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.FunctionEnvironment;

import org.junit.Test;

public class FunctionEnvironmentTests {

	@Test(expected = TypeException.class)
	public void testFail() throws TypeException {
		FunctionEnvironment fe = new FunctionEnvironment();
		fe.typeCheck();
	}

}
