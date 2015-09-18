package mmdb.streamprocessing.functionoperators;

import mmdb.data.MemoryObject;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.FunctionEnvironment;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.streamoperators.StreamOperator;
import mmdb.streamprocessing.tools.TypecheckTools;
import sj.lang.ListExpr;

/**
 * Implementation of a parameter function.<br>
 * A parameter function gets passed elements of a stream by a caller and returns
 * a sub-operator-tree that a caller can evaluate based on the current element.
 * 
 * @author Bjoern Clasen
 */
public class ParameterFunction implements Node {

	/**
	 * The operator that this ParameterFunction returns to the caller.
	 */
	private Node operator;

	/**
	 * The parameters of this ParameterFunction.
	 */
	private Node[] parameters;

	/**
	 * The parameters as FunctionEnvironments.
	 */
	private FunctionEnvironment[] functionEnvironments;

	/**
	 * The OutputType of the {@link ParameterFunction#operator}.
	 */
	private MemoryObject outputType;

	/**
	 * The type of the {@link ParameterFunction#operator}.
	 */
	private Class<? extends Node> operatorType;

	/**
	 * Special fromNL() method:<br>
	 * Appends all FunctionEnvironments with their names to the current
	 * Environment to enable all Nodes in the {@link ParameterFunction#operator}
	 * to access them.
	 * 
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		Node[] parameterNodes = new Node[params.length - 1];
		for (int i = 0; i < params.length - 1; i++) {
			String functionEnvironmentName = NestedListProcessor
					.nlToFunctionEnvironmentName(params[i]);
			parameterNodes[i] = new FunctionEnvironment();

			// Extend environment by the FunctionEnvironment
			environment = environment.addObjectToNewEnvironment(
					functionEnvironmentName,
					(FunctionEnvironment) parameterNodes[i]);
		}
		Node operatorNode = NestedListProcessor.nlToNode(
				params[params.length - 1], environment);
		return new ParameterFunction(parameterNodes, operatorNode);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input
	 *            function's parameters
	 * @param identifier
	 *            function's subtree root Node
	 */
	public ParameterFunction(Node[] parameters, Node operator) {
		this.parameters = parameters;
		this.operator = operator;
	}

	/**
	 * {@inheritDoc}
	 */
	@SuppressWarnings("unchecked")
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

	/**
	 * Passes all given MemoryObjects to the FunctionEnvironments.<br>
	 * Then returns the root Node of the sub-operator-tree.
	 * 
	 * @param parameters
	 *            all current parameter MemoryObjects.
	 * @return the operator that can now be evaluated.
	 */
	public Node evaluate(MemoryObject... parameters) {
		for (int i = 0; i < parameters.length; i++) {
			this.functionEnvironments[i].setObject(parameters[i]);
		}
		return this.operator;
	}

	/**
	 * Retrieves the type of the operator that this ParameterFunction holds.<br>
	 * This is usually StreamOperator or ObjectNode.
	 */
	public Class<? extends Node> getOperatorType() {
		return this.operatorType;
	}

	/**
	 * Sets the types of this ParameterFunction's parameters. These are
	 * represented by TypecheckInstances.
	 * 
	 * @param parameterTypes
	 *            the types of parameters for this ParameterFunction.
	 * @throws TypeException
	 *             if the number of parameters does not fit or if somethings
	 *             wrong with the FunctionEnvironment(s).
	 */
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

	/**
	 * Returns the OutputType of the {@link ParameterFunction#operator}.
	 */
	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}

}
