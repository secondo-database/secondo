package mmdb.streamprocessing.parser.tools;

import java.util.HashMap;
import java.util.Map;

import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.functionoperators.ParameterFunction;
import mmdb.streamprocessing.objectnodes.Attr;
import mmdb.streamprocessing.objectnodes.Consume;
import mmdb.streamprocessing.objectnodes.Count;
import mmdb.streamprocessing.objectnodes.aggregation.Average;
import mmdb.streamprocessing.objectnodes.aggregation.Max;
import mmdb.streamprocessing.objectnodes.aggregation.Min;
import mmdb.streamprocessing.objectnodes.aggregation.Sum;
import mmdb.streamprocessing.objectnodes.condition.Contains;
import mmdb.streamprocessing.objectnodes.condition.Equals;
import mmdb.streamprocessing.objectnodes.condition.EqualsGreater;
import mmdb.streamprocessing.objectnodes.condition.EqualsLess;
import mmdb.streamprocessing.objectnodes.condition.Greater;
import mmdb.streamprocessing.objectnodes.condition.Less;
import mmdb.streamprocessing.objectnodes.logic.And;
import mmdb.streamprocessing.objectnodes.logic.Not;
import mmdb.streamprocessing.objectnodes.logic.Or;
import mmdb.streamprocessing.objectnodes.maths.Minus;
import mmdb.streamprocessing.objectnodes.maths.Plus;
import mmdb.streamprocessing.streamoperators.Extend;
import mmdb.streamprocessing.streamoperators.Feed;
import mmdb.streamprocessing.streamoperators.FeedProject;
import mmdb.streamprocessing.streamoperators.Filter;
import mmdb.streamprocessing.streamoperators.Groupby;
import mmdb.streamprocessing.streamoperators.Hashjoin;
import mmdb.streamprocessing.streamoperators.Head;
import mmdb.streamprocessing.streamoperators.Product;
import mmdb.streamprocessing.streamoperators.Project;
import mmdb.streamprocessing.streamoperators.Rename;
import mmdb.streamprocessing.streamoperators.Sort;
import mmdb.streamprocessing.streamoperators.Sortby;
import mmdb.streamprocessing.streamoperators.Tail;

public class OperatorLookup {
	
	private Map<String, Class<? extends Node>> map;
	
	private static OperatorLookup instance = null;

	public static OperatorLookup getInstance() {
		if (instance == null) {
			instance = new OperatorLookup();
		}
		return instance;
	}

	public OperatorLookup() {
		this.map = fillmap();
	}
	
	public Class<? extends Node> lookup(String operatorName) {
		return map.get(operatorName);
	}

	private Map<String, Class<? extends Node>> fillmap() {
		HashMap<String, Class<? extends Node>> operatorMap = new HashMap<String, Class<? extends Node>>();
		// FunctionOperators
		operatorMap.put("fun", ParameterFunction.class);

		// ObjectNodes
		operatorMap.put("avg", Average.class);
		operatorMap.put("max", Max.class);
		operatorMap.put("min", Min.class);
		operatorMap.put("sum", Sum.class);
		operatorMap.put("contains", Contains.class);
		operatorMap.put("=", Equals.class);
		operatorMap.put(">=", EqualsGreater.class);
		operatorMap.put("<=", EqualsLess.class);
		operatorMap.put(">", Greater.class);
		operatorMap.put("<", Less.class);
		operatorMap.put("and", And.class);
		operatorMap.put("not", Not.class);
		operatorMap.put("or", Or.class);
		operatorMap.put("-", Minus.class);
		operatorMap.put("+", Plus.class);
		operatorMap.put("attr", Attr.class);
		operatorMap.put("consume", Consume.class);
		operatorMap.put("count", Count.class);

		// StreamOperators
		operatorMap.put("extend", Extend.class);
		operatorMap.put("feed", Feed.class);
		operatorMap.put("feedproject", FeedProject.class);
		operatorMap.put("filter", Filter.class);
		operatorMap.put("groupby", Groupby.class);
		operatorMap.put("hashjoin", Hashjoin.class);
		operatorMap.put("head", Head.class);
		operatorMap.put("product", Product.class);
		operatorMap.put("project", Project.class);
		operatorMap.put("rename", Rename.class);
		operatorMap.put("sort", Sort.class);
		operatorMap.put("sortby", Sortby.class);
		operatorMap.put("tail", Tail.class);

		return operatorMap;
	}

}
