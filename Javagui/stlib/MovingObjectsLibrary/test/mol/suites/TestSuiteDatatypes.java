package mol.suites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import mol.datatypes.base.BaseBoolTest;
import mol.datatypes.base.BaseIntTest;
import mol.datatypes.base.BaseRealTest;
import mol.datatypes.base.BaseStringTest;
import mol.datatypes.interval.IntervalIntTest;
import mol.datatypes.interval.PeriodTest;
import mol.datatypes.intime.IntimeTest;
import mol.datatypes.range.PeriodsTest;
import mol.datatypes.range.RangeIntTest;
import mol.datatypes.time.TimeInstantTest;

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
