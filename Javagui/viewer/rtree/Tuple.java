package viewer.rtree;

import sj.lang.*;
import tools.Reporter;
import viewer.rtree.*;
import viewer.rtree.datatypes.*;
import java.util.*;
import javax.swing.*;
import javax.swing.tree.*;

/**
 * Tuple represents a tuple referenced by a tree node.
 * A tuple is identified by its tuple id.
 * 
 * @author Oliver Feuer
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @since 08.03.2010
 * @version 1.2
 */
public class Tuple 
{
	int tupleId;
	Node fatherLeafNode;
	DefaultMutableTreeNode treeNode;
	int nodeId;
	BoundingBox MBR;
	ReferenceParameters referenceParams;
	boolean tupleLoaded = false;
	ListExpr le;
	
	// constructors

	/**
	 * Creates a new Tuple object.
	 * @param tupleId Tuple Id
	 * @param nodeId Id of the parent node
	 */
	public Tuple(int tupleId, Node node, BoundingBox mbr)
	{
		this.tupleId = tupleId;
		this.fatherLeafNode = node;
		this.MBR = mbr;
		this.treeNode = null;
		this.nodeId = node.getNodeId();
		this.referenceParams = node.getReferenceParameters();
	}
	
	// public methods
	
	/**
	 * Sets the reference parameters.
	 * @param referenceParams Reference parameters
	 */
	public void setReferenceParameters(ReferenceParameters referenceParams)
	{
		this.referenceParams = referenceParams;
	}
	
	/**
	 * Gets the tuple TupleId.
	 */
	public int getTupleId() 
	{
		return tupleId;
	}

	/**
	 * Gets the tuple father LeafNode.
	 */
	public Node getFatherNode() 
	{
		return fatherLeafNode;
	}

	/**
	 * Gets the tuple's bounding box.
	 * @return Bounding box
	 */
	public BoundingBox getMbr()
	{
		return this.MBR;
	}

	/**
	 * Gets the tuple treeNode.
	 */
	public DefaultMutableTreeNode getTupleTreeNode() 
	{
		return treeNode;
	}

	/**
	 * Sets the tuple treeNode.
	 */
	public void setTupleTreeNode(DefaultMutableTreeNode treenode) 
	{
		this.treeNode = treenode;
	}

	/**
	 * Gets the tuple display name.
	 */
	public String toString() 
	{
		return "TId: " + this.tupleId;
	}
	
	/**
	 * Gets the string representation of the complete tuple.
	 */
	public String completeToString() 
	{
		String tuple = "TupleId : " + this.tupleId + "\n";
		
		// check if a relation is selected
		if ((this.referenceParams!=null)
		   &&(this.referenceParams.isComplete()))
		{
			if (!this.tupleLoaded )
			{
				loadTuple();
			}
		
			try
			{
				ListExpr attrList = le.first().second().second();
				ListExpr valList = le.second().first();
				StringBuffer strBuffer = new StringBuffer();

				while (!attrList.isEmpty())
				{
					attrList.first().first().writeToString(strBuffer);
					if (strBuffer.toString().compareTo("id")!=0)
					{
						tuple += strBuffer.toString().replace("_xYz", "");

						valList.first().writeToString(strBuffer);
						tuple += " : " + strBuffer.toString() + "\n";
					}
					attrList = attrList.rest();
					valList = valList.rest();
				}


			}
			catch (Exception ex)
			{
				tuple = "TupleId " + this.tupleId + ":\n\n(unknown)";
			}
		}
		
		return tuple;
	}

	/**
	 * Gets the tuple as list expression.
	 * @return Tuple as list expression
	 */
	public ListExpr getListExpr()
	{
		if (!this.tupleLoaded )
		{
			loadTuple();
		}
		
		return this.le;
	}
	
	// private methods
	
	/**
	 * Loads the tuple from the database.
	 */
	private void loadTuple()
	{
		
		this.le = sendGetTuple();
		this.tupleLoaded = true;
	}
	
	/**
	 * Sends the getTuple command to Secondo.
	 * @return Query result 
	 */
	private ListExpr sendGetTuple()
	{
		StringBuilder cmd = new StringBuilder();
		cmd.append("query ");
		cmd.append(this.referenceParams.getRelation());
		cmd.append(" feed {xYz} addtupleid filter[");
		cmd.append("(.id = [const tid value ");
		cmd.append(String.valueOf(this.tupleId));
		cmd.append("])");
		cmd.append("] consume");
		
		SecondoManager secondo = new SecondoManager();
		this.le = secondo.sendCommand(cmd.toString(), "getTuple");
		
		if (this.le == null)
		{
			this.le = new ListExpr();
		}

		return le;
	}
}
