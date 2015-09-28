package unittests.mmdb.suites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import unittests.mmdb.data.MemoryObjectsTests;
import unittests.mmdb.service.ObjectExportTests;
import unittests.mmdb.service.ObjectImportTests;
import unittests.mmdb.streamprocessing.NodesTests;
import unittests.mmdb.streamprocessing.functionoperators.ParameterFunctionTests;
import unittests.mmdb.streamprocessing.objectnodes.AttrTests;
import unittests.mmdb.streamprocessing.objectnodes.ConstantNodeTests;
import unittests.mmdb.streamprocessing.objectnodes.ConsumeTests;
import unittests.mmdb.streamprocessing.objectnodes.CountTests;
import unittests.mmdb.streamprocessing.objectnodes.FunctionEnvironmentTests;
import unittests.mmdb.streamprocessing.objectnodes.aggregation.AverageTests;
import unittests.mmdb.streamprocessing.objectnodes.aggregation.MaxTests;
import unittests.mmdb.streamprocessing.objectnodes.aggregation.MinTests;
import unittests.mmdb.streamprocessing.objectnodes.aggregation.SumTests;
import unittests.mmdb.streamprocessing.objectnodes.condition.ContainsTests;
import unittests.mmdb.streamprocessing.objectnodes.condition.EqualsGreaterTests;
import unittests.mmdb.streamprocessing.objectnodes.condition.EqualsLessTests;
import unittests.mmdb.streamprocessing.objectnodes.condition.EqualsTests;
import unittests.mmdb.streamprocessing.objectnodes.condition.GreaterTests;
import unittests.mmdb.streamprocessing.objectnodes.condition.LessTests;
import unittests.mmdb.streamprocessing.objectnodes.logic.AndTests;
import unittests.mmdb.streamprocessing.objectnodes.logic.NotTests;
import unittests.mmdb.streamprocessing.objectnodes.logic.OrTests;
import unittests.mmdb.streamprocessing.objectnodes.maths.MinusTests;
import unittests.mmdb.streamprocessing.objectnodes.maths.PlusTests;
import unittests.mmdb.streamprocessing.parser.EnvironmentTests;
import unittests.mmdb.streamprocessing.parser.NestedListProcessorTest;
import unittests.mmdb.streamprocessing.parser.ParserControllerTests;
import unittests.mmdb.streamprocessing.streamoperator.ExtendTests;
import unittests.mmdb.streamprocessing.streamoperator.FeedProjectTests;
import unittests.mmdb.streamprocessing.streamoperator.FeedTests;
import unittests.mmdb.streamprocessing.streamoperator.FilterTests;
import unittests.mmdb.streamprocessing.streamoperator.GroupbyTests;
import unittests.mmdb.streamprocessing.streamoperator.HashjoinTests;
import unittests.mmdb.streamprocessing.streamoperator.HeadTests;
import unittests.mmdb.streamprocessing.streamoperator.ProductTests;
import unittests.mmdb.streamprocessing.streamoperator.ProjectTests;
import unittests.mmdb.streamprocessing.streamoperator.RenameTests;
import unittests.mmdb.streamprocessing.streamoperator.SortTests;
import unittests.mmdb.streamprocessing.streamoperator.SortbyTests;
import unittests.mmdb.streamprocessing.streamoperator.SymmjoinTests;
import unittests.mmdb.streamprocessing.streamoperator.TailTests;
import unittests.mmdb.streamprocessing.tools.HeaderToolsTests;
import unittests.mmdb.streamprocessing.tools.ParserToolsTest;
import unittests.mmdb.streamprocessing.tools.TypecheckToolsTests;

@RunWith(Suite.class)
@SuiteClasses({ FeedTests.class, CountTests.class, HeadTests.class,
		TailTests.class, RenameTests.class, ProductTests.class,
		ProjectTests.class, FeedProjectTests.class, ConsumeTests.class,
		SumTests.class, AverageTests.class, MinTests.class, MaxTests.class,
		AttrTests.class, PlusTests.class, MinusTests.class, EqualsTests.class,
		FilterTests.class, NodesTests.class, FunctionEnvironmentTests.class,
		ParameterFunctionTests.class, ConstantNodeTests.class,
		HeaderToolsTests.class, TypecheckToolsTests.class, ExtendTests.class,
		GroupbyTests.class, SortbyTests.class, SortTests.class,
		HashjoinTests.class, NotTests.class, AndTests.class, OrTests.class,
		ContainsTests.class, GreaterTests.class, LessTests.class,
		EqualsGreaterTests.class, EqualsLessTests.class,
		EnvironmentTests.class, ParserToolsTest.class,
		NestedListProcessorTest.class, MemoryObjectsTests.class,
		ObjectExportTests.class, ObjectImportTests.class,
		ParserControllerTests.class, SymmjoinTests.class })
public class TestSuiteStreamProcessing {

}
