package com.secondo.webgui.client.mainview;

import java.util.ArrayList;

public class Pattern {

	ArrayList <PatternNode> nodes = new ArrayList <PatternNode>();
	
	public String getPatternValue(){
		String value="";
		for(PatternNode node:nodes){
			value=node.getVariable();
			if(node.getTimeOrLabel().size()!=0){
				for(String eachTimeOrLabel:node.getTimeOrLabel()){
					value=value+" "+eachTimeOrLabel;
				}
			}
			
			if(node.getSequencePattern().size()!=0){
				for(String eachOfSequence:node.getSequencePattern()){
					value=value+" "+eachOfSequence;
				}
			}
			
		}
		return value;
	}
	
	
	/**
	 * @return the nodes
	 */
	public ArrayList<PatternNode> getNodes() {
		return nodes;
	}



	/**
	 * @param nodes the nodes to set
	 */
	public void setNodes(ArrayList<PatternNode> nodes) {
		this.nodes = nodes;
	}



	public class PatternNode{
		
		
		private String variable;
		private ArrayList <String> timeOrLabel;
		private ArrayList <String> sequencePattern;
		/**
		 * @return the timeOrLabel
		 */
		public ArrayList<String> getTimeOrLabel() {
			return timeOrLabel;
		}
		/**
		 * @param timeOrLabel the timeOrLabel to set
		 */
		public void setTimeOrLabel(ArrayList<String> timeOrLabel) {
			this.timeOrLabel = timeOrLabel;
		}
		/**
		 * @return the variable
		 */
		public String getVariable() {
			return variable;
		}
		/**
		 * @param variable the variable to set
		 */
		public void setVariable(String variable) {
			this.variable = variable;
		}
		/**
		 * @return the sequencePattern
		 */
		public ArrayList<String> getSequencePattern() {
			return sequencePattern;
		}
		/**
		 * @param sequencePattern the sequencePattern to set
		 */
		public void setSequencePattern(ArrayList<String> sequencePattern) {
			this.sequencePattern = sequencePattern;
		}
		

		
		
	}
	
	
}
