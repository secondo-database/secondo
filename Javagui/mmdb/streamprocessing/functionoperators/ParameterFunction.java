package mmdb.streamprocessing.functionoperators;

import mmdb.data.MemoryObject;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.FunctionEnvironment;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.streamoperators.StreamOperator;
import mmdb.streamprocessing.tools.TypecheckTools;

public class ParameterFunction implements Node {

	private Node operator;

	private Node[] parameters;

	private FunctionEnvironment[] functionEnvironments;

	private MemoryObject outputType;

	private Class<? extends Node> operatorType;

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		Node[] parameterNodes = new Node[params.length - 1];
		for (int i = 0; i < params.length - 1; i++) {
			String functionEnvironmentName = NestedListProcessor
					.nlToFunctionEnvironmentName(params[i]);
			parameterNodes[i] = new FunctionEnvironment();

			// Extend environment by the FunctionEnvironment
			environment = environment.addEnvironmentObject(
					functionEnvironmentName,
					(FunctionEnvironment) parameterNodes[i]);
		}
		Node operatorNode = NestedListProcessor.nlToNode(
				params[params.length - 1], environment);
		return new ParameterFunction(parameterNodes, operatorNode);
	}

	public ParameterFunction(Node[] parameters, Node operator) {
		this.parameters = parameters;
		this.operator = operator;
	}

	@Override
	public void typeCheck() throws TypeException {
		for (Node parameter : parameters) {
			parameter.typeCheck();
		}
		this.operator.typeCheck();

		// Types of parameters have been checked in "setParamTypes"
		TypecheckTools.checkMultipleNodeTypes(this.operator, getClass(), 1,
				StreamOperator.class, ObjectNode.class);

		this.operatorType = this.operator.getClass();
		this.outputType = this.operator.getOutputType();
	}

	public Node evaluate(MemoryObject... parameters) {
		for (int i = 0; i < parameters.length; i++) {
			this.functionEnvironments[i].setObject(parameters[i]);
		}
		return this.operator;
	}

	public Class<? extends Node> getOperatorType() {
		return this.operatorType;
	}

	public void setParamTypes(MemoryObject... parameterTypes)
			throws TypeException {
		if (parameterTypes.length != this.parameters.length) {
			throw new TypeException(
					"Wrong number of Parameter-Types given to Function!");
		}

		this.functionEnvironments = new FunctionEnvironment[this.parameters.length];

		for (int i = 0; i < this.parameters.length; i++) {
			TypecheckTools.checkSpecificNodeType(this.parameters[i],
					FunctionEnvironment.class, this.getClass(), i + 2);

			this.functionEnvironments[i] = (FunctionEnvironment) this.parameters[i];

			this.functionEnvironments[i].setOutputType(parameterTypes[i]);
		}
	}

	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}

}
