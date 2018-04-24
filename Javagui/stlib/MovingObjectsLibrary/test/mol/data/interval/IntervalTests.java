package mol.data.interval;

import static org.junit.Assert.*;

import java.time.Instant;
import java.time.ZoneId;

import org.junit.BeforeClass;
import org.junit.Test;

import mol.util.TestUtilData;

public class IntervalTests {

	private static Period closedPeriod;
	private static Period openPeriod;
	private static Instant instantInside;
	private static Instant instantOutside;
	private static Instant instantOnLowerBound;
	private static Instant instantOnUpperBound;

	private static IntervalInt iint;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		closedPeriod = TestUtilData.getPeriod("05", "10", true, true);
		openPeriod = TestUtilData.getPeriod("05", "10", false, false);

		instantInside = TestUtilData.getInstant("07");
		instantOutside = TestUtilData.getInstant("15");
		instantOnLowerBound = TestUtilData.getInstant("05");
		instantOnUpperBound = TestUtilData.getInstant("10");

		iint = new IntervalInt(3, 5, true, true);
	}

	@Test
	public void testContains_InstantInsidePeriod() {

		assertTrue(closedPeriod.contains(instantInside));
		assertTrue(closedPeriod.contains(instantOnLowerBound));
		assertTrue(closedPeriod.contains(instantOnUpperBound));

	}

	@Test
	public void testContains_InstantOutsidePeriod() {

		assertFalse(closedPeriod.contains(instantOutside));
		assertFalse(openPeriod.contains(instantOnLowerBound));
		assertFalse(openPeriod.contains(instantOnUpperBound));

	}

	@Test
	public void testContains_IntegerInsideInterval() {

		assertTrue(iint.contains(4));

	}

	@Test
	public void testContains_IntegerOutsideInterval() {

		assertFalse(iint.contains(6));

	}

}
