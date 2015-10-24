package mmdb.streamprocessing.parser;

import java.util.HashMap;
import java.util.SortedMap;
import java.util.TreeMap;

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
import mmdb.streamprocessing.streamoperators.Symmjoin;
import mmdb.streamprocessing.streamoperators.Tail;

/**
 * A (static only) class that helps finding Operator.
 * 
 * @author Bjoern Clasen
 */
public class OperatorLookup {
	
	/**
	 * Initializes the operator map.
	 */
	static {
		map = initializeMap();
	}

	/**
	 * The map containing all known operator names and classes. It is a sorted
	 * map to keep order in GUI operator-list.
	 */
	private static SortedMap<String, Class<? extends Node>> map;
	
	/**
	 * Private constructor to prevent this class from being instantiated.
	 */
	private OperatorLookup() {
	}

	/**
	 * Lookup an operator.
	 * 
	 * @see HashMap#get(Object)
	 * 
	 * @param operatorName
	 *            the name of the operator to lookup
	 * @return the operator's class or null if the operator is not known.
	 */
	public static Class<? extends Node> lookup(String operatorName) {
		return map.get(operatorName);
	}

	/**
	 * Initializes the map of operators.<br>
	 * <b>!! Put any newly implemented operators in here !!</b><br>
	 * Please put them into the right section.
	 * 
	 * @return the initialized map.
	 */
	private static SortedMap<String, Class<? extends Node>> initializeMap() {
		TreeMap<String, Class<? extends Node>> operatorMap = new TreeMap<String, Class<? extends Node>>();
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
		operatorMap.put("symmjoin", Symmjoin.class);
		operatorMap.put("tail", Tail.class);

		return operatorMap;
	}

	public static String listOperators() {
		StringBuilder sb = new StringBuilder();
		int currentLineLength = 0;
		int maxLineLength = 25;
		sb.append("SUPPORTED OPERATORS:\n");
		for(String key : map.keySet()) {
			sb.append(key + ", ");
			currentLineLength += key.length();
			if (currentLineLength > maxLineLength) {
				sb.append("\n");
				currentLineLength = 0;
			}
		}
		return sb.toString().substring(0, sb.toString().length() - 2);
	}

}
