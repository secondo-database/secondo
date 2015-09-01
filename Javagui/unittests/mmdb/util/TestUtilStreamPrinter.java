package unittests.mmdb.util;

import java.lang.reflect.Method;

import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.streamprocessing.streamoperators.StreamOperator;

public class TestUtilStreamPrinter {

	public static void printTupleStream(StreamOperator stream) throws Exception {

		System.out.println();
		System.out.println("Relation:");
		if (!(stream.getOutputType() instanceof MemoryTuple)) {
			throw new Exception("Can only print Tuple-Streams!");
		}
		System.out.print("  |  ");
		for (RelationHeaderItem headerItem : ((MemoryTuple) stream
				.getOutputType()).getTypecheckInfo()) {
			System.out.print(headerItem.getIdentifier() + "  |  ");
		}
		System.out.println();

		stream.open();
		MemoryTuple tuple;
		while ((tuple = (MemoryTuple) stream.getNext()) != null) {
			System.out.print("  |  ");
			for (MemoryAttribute attribute : tuple.getAttributes()) {
				if (attribute == null) {
					System.out.print("undefined" + "  |  ");
				} else {
					try {
						Method method = attribute.getClass().getMethod(
								"getValue");
						System.out.print(method.invoke(attribute) + "  |  ");
					} catch (Exception e) {
						System.out.print("Fehler: " + e);
					}
				}
			}
			System.out.println();
		}
		stream.close();
	}

}
