package stlib.suites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import stlib.datatypes.base.BaseBoolTest;
import stlib.datatypes.base.BaseIntTest;
import stlib.datatypes.base.BaseRealTest;
import stlib.datatypes.base.BaseStringTest;
import stlib.datatypes.interval.IntervalIntTest;
import stlib.datatypes.interval.PeriodTest;
import stlib.datatypes.intime.IntimeTest;
import stlib.datatypes.range.PeriodsTest;
import stlib.datatypes.range.RangeIntTest;
import stlib.datatypes.time.TimeInstantTest;

/**
 * Suite for collecting all tests of the data modules
 * 
 * @author Markus Fuessel
 *
 */
@RunWith(Suite.class)
@SuiteClasses({ BaseBoolTest.class, BaseIntTest.class, BaseRealTest.class, BaseStringTest.class, IntimeTest.class,
        TimeInstantTest.class, IntervalIntTest.class, PeriodTest.class, PeriodsTest.class, RangeIntTest.class })
public class TestSuiteDatatypes {

}
