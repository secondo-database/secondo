package mmdb.streamprocessing.streamoperators;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.functionoperators.ParameterFunction;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;

public class Filter implements StreamOperator {

	private Node input1, input2;

	private StreamOperator streamInput;

	private ParameterFunction functionInput;

	private MemoryObject outputType;

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Filter.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Node node2 = NestedListProcessor.nlToNode(params[1], environment);
		return new Filter(node1, node2);
	}

	public Filter(Node input1, Node input2) {
		this.input1 = input1;
		this.input2 = input2;
	}

	@Override
	public void typeCheck() throws TypeException {
		this.input1.typeCheck();

		// Is input1 a StreamOperator?
		TypecheckTools.checkNodeType(this.input1, StreamOperator.class,
				this.getClass(), 1);
		this.streamInput = (StreamOperator) this.input1;

		// Is input2 a ParamterFunction?
		TypecheckTools.checkNodeType(this.input2, ParameterFunction.class,
				this.getClass(), 2);
		this.functionInput = (ParameterFunction) this.input2;

		// Typecheck ParameterFunction
		this.functionInput.setParamTypes(this.streamInput.getOutputType());
		this.functionInput.typeCheck(); // !!! Important !!!

		// Does the ParameterFunction provide an ObjectNode providing an
		// AttributeBool?
		TypecheckTools.checkFunctionOperatorType(this.functionInput,
				ObjectNode.class, this.getClass(), 2);
		TypecheckTools.checkFunctionOutputType(this.functionInput,
				AttributeBool.class, this.getClass(), 2);

		this.outputType = this.streamInput.getOutputType();
	}

	@Override
	public void open() {
		this.streamInput.open();
	}

	@Override
	public MemoryObject getNext() {
		MemoryObject inputObject = null;
		boolean matchesCondition = false;
		while (!matchesCondition
				&& (inputObject = this.streamInput.getNext()) != null) {
			ObjectNode functionResult = (ObjectNode) this.functionInput
					.evaluate(inputObject);
			AttributeBool resultAttribute = (AttributeBool) functionResult
					.getResult();
			if (resultAttribute == null) {
				matchesCondition = false;
			} else {
				matchesCondition = resultAttribute.isValue();
			}
		}
		return inputObject;
	}

	@Override
	public void close() {
		this.streamInput.close();
	}

	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}

}
