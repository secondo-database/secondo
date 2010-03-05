package viewer.rtree.gui;

import viewer.rtree.Node;
import viewer.rtree.Tuple;

/**
 * This interface must be implemented by all GUI classes
 * being able to display nodes in any way.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.4
 * @since 27.12.2009
 */
public interface NodeDisplay {
	
	/**
	 * Displays the given node.
	 * @param n Node to display
	 */
	public void displayNode(Node n);

	/**
	 * Displays the given tuple.
	 * @param t Tuple to display
	 */
	public void displayTuple(Tuple t);

	/**
	 * Selects the given node.
	 * @param n Node to select
	 */
	public void selectNode(Node n);

	/**
	 * Sets rtree attributes.
	 * @param name RTree name
	 * @param noOfDimensions Total number of dimensions
	 */
	public void setRTreeAttributes(String name, int noOfDimensions);
}
