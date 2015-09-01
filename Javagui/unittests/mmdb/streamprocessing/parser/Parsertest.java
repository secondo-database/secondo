package unittests.mmdb.streamprocessing.parser;

import mmdb.streamprocessing.parser.parser;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;

import org.junit.Test;

public class Parsertest {

	@Test
	public void testSTRING() throws Exception {
		String parseString = "(query     (consume        (filter            (feed Personen)            (fun                (streamelem1 STREAMELEM)                (>                    (attr streamelem1 Groesse)                    1.7)))))";
		String parseString2 = "(query			    (consume			        (projectextend			            (feed Personen)			            (ID Name)			            (		                (Groesser			                    (fun			                        (tuple1 TUPLE)			                        (plus			                            (attr tuple1 Groesse)			                            0.5)))))))";
		String parseString3 = "(plus_a)";
		parser parse = new parser(parseString3);
		NestedListNode result = parse.parseToNestedList();
		String fertig = "fertig!"; // Do something for Debug
	}

	@Test
	public void testATOM() throws Exception {
		parser parse = new parser("TRUE");
		parse.parse();
	}

}