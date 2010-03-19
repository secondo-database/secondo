package viewer.rtree;

import sj.lang.*;
import tools.Reporter;
import viewer.rtree.datatypes.*;
import java.util.*;

/**
 * Node represents a node from an rtree.
 * A node is identified by its id and the rtree it belongs to.
 * 
 * @author Oliver Feuer
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @since 08.03.2010
 * @version 1.6
 */
public class Node 
{
	// rtree the node belongs to
	RTree rtree;
	// node id
	int nodeId;
	// parent node's id (-1 means it is the root node)
	int parentNodeId = -1;
	// parent node (null means it is the root node)
	Node parentNode = null;
	// node height
	int nodeLevel = 0;

	// some attributes
	BoundingBox mbr;
	int noOfSons = 0;
	int noOfEntries = 0;
	boolean isLeafNode = false;
	boolean isRootNode = false;
	double mbrSize = 0.0;
	double mbrDead = 0.0;
	double mbrOverlap = 0.0;
	int mbrOverlapNo = 0;
	double mbrDensity = 0.0;
	
	// child nodes
	Vector<Node> childNodes = new Vector<Node>();
	
	// references from the indexed relation
	LinkedList tupleIds = new LinkedList();
	ReferenceParameters referenceParams;
	LinkedList<Drawable> indexedItems = new LinkedList<Drawable>();
	LinkedList<Tuple> indexedTuples = new LinkedList<Tuple>();
		
	// lazy loading flags
	boolean nodeInfoLoaded = false;
	boolean isLeafNodeLoaded = false;
	boolean childNodesLoaded = false;
	boolean MBRLoaded = false;
	boolean tupleIdsLoaded = false;
	boolean indexedItemsLoaded = false;

	// constructors
	
	/**
	 * Creates a new Node object.
	 * @param nodeId Node Id
	 * @param parentNode  the parent node
	 * @param rtree RTree this node belongs to
	 */
	public Node(int nodeId, Node parentNode, RTree rtree)
	{
		this.nodeId = nodeId;
		this.rtree = rtree;
		this.parentNode = parentNode;
		if (parentNode!=null)
			this.parentNodeId = parentNode.getNodeId();
		this.nodeLevel = 1;
		Node father = parentNode;
		while (father !=null)
		{
			this.nodeLevel++;
			father = father.getParentNode();
		}

		Interval[] intervals = new Interval[2];
		intervals[0] = new Interval(0.0, 0.1);
		intervals[1] = new Interval(0.0, 0.1);
		mbr = new BoundingBox(intervals);
	}
	
	// public methods
	
	/**
	 * Gets the node display name.
	 */
	public String toString() 
	{
		return "Node: " + nodeId;
	}
	
	/**
	 * Gets the node id.
	 * @return Node id
	 */
	public int getNodeId()
	{
		return this.nodeId;
	}
	
	/**
	 * Gets the node level.
	 * @return level of node in the rtree
	 */
	public int getNodeLevel()
	{
		return this.nodeLevel;
	}
	
	/**
	 * Gets the rtree name.
	 * @return RTree name
	 */
	public RTree getRTree()
	{
		return this.rtree;
	}
	
	
	/**
	 * Indicates if child nodes were loaded.
	 * @return True if child nodes were loaded otherwise false
	 */
	public boolean childNodesLoaded()
	{
		return this.childNodesLoaded;
	}

	/**
	 * Indicates if tuple Ids were loaded.
	 * @return True if tuple Ids were loaded otherwise false
	 */
	public boolean tupleIdsLoaded()
	{
		return this.tupleIdsLoaded;
	}

	/**
	 * Indicates if the node detail information was loaded.
	 * @return True if detail node information was loaded, otherwise false
	 */
	public boolean nodeInfoLoaded()
	{
		return this.nodeInfoLoaded;
	}
	
	/**
	 * Gets the parent node.
	 * @return parent node
	 */
	public Node getParentNode() 
	{
		return this.parentNode;
	}
	
	/**
	 * Gets all child nodes.
	 * @return Child nodes
	 */
	public Vector<Node> getChildNodes() 
	{
		if (!this.childNodesLoaded)
		{
			loadChildNodes();
		}
		
		return this.childNodes;
	}
	
	/**
	 * Gets the node's indexed items.
	 * @return Indexed items
	 */
	public LinkedList<Drawable> getIndexedItems()
	{
		if (!this.indexedItemsLoaded )
		{
			loadIndexedItems();
		}
		
		return this.indexedItems;
	}

	/**
	 * Gets the node's indexed tuples.
	 * @return Indexed tuples
	 */
	public LinkedList<Tuple> getIndexedTuples()
	{
		if (!this.tupleIdsLoaded )
		{
			loadTupleIds();
		}
		
		return this.indexedTuples;
	}

	/**
	 * Clears the node's indexed items.
	 */
	public void clearIndexedItems()
	{
		if (this.indexedItemsLoaded )
		{
			this.indexedItems.clear();
			this.indexedItemsLoaded = false;
		}
		
		return;
	}
	
	/**
	 * Sets the node's bounding box.
	 */
	public void setMbr(BoundingBox mbr)
	{
		this.mbr = mbr;
		this.MBRLoaded = true;
		return;
	}

	/**
	 * Gets the node's bounding box.
	 * @return Bounding box
	 */
	public BoundingBox getMbr()
	{
		if (!this.MBRLoaded )
		{
			loadNodeInfo();
		}
		
		return this.mbr;
	}

	/**
	 * Gets the total child count.
	 * @return Total children
	 */
	public int getNoOfSons()
	{
		if (!this.nodeInfoLoaded )
		{
			loadNodeInfo();
		}
		
		return this.noOfSons;
	}

	/**
	 * Gets the total entries count.
	 * @return Total entries
	 */
	public int getNoOfEntries()
	{
		if (!this.tupleIdsLoaded )
		{
			loadTupleIds();
		}		
		return this.noOfEntries;
	}

	/**
	 * Indicates if this is a leaf node.
	 * @return True if this is a leaf node otherwise false
	 */
	public boolean isLeafNode()
	{
		if (!this.isLeafNodeLoaded )
		{
			loadChildNodes();
		}
		
		return this.isLeafNode;
	}

	/**
	 * Indicates if this is the tree's root node.
	 * @return True if this is the root node otherwise false
	 */
	public boolean isRootNode() {
		if (! nodeInfoLoaded )
		{
			loadNodeInfo();
		}
		
		return isRootNode;
	}

	/**
	 * Gets the bounding box size.
	 * @return Size of bounding box
	 */
	public double getMbrSize()
	{
		if (!this.nodeInfoLoaded )
		{
			loadNodeInfo();
		}
		
		return this.mbrSize;
	}

	/**
	 * Gets the bounding box dead space.
	 * @return Size of bounding box dead space
	 */
	public double getMbrDead()
	{
		if (!this.nodeInfoLoaded )
		{
			loadNodeInfo();
		}
		
		return this.mbrDead;
	}

	/**
	 * Gets the bounding box overlap space.
	 * @return Size of bounding box overlap space
	 */
	public double getMbrOverlap()
	{
		if (!this.nodeInfoLoaded )
		{
			loadNodeInfo();
		}
		
		return this.mbrOverlap;
	}

	/**
	 * Gets the total of overlapping bounding boxes.
	 * @return Total of overlapping bounding boxes
	 */
	public int getMbrOverlapNo()
	{
		if (!this.nodeInfoLoaded )
		{
			loadNodeInfo();
		}
		
		return this.mbrOverlapNo;
	}

	/**
	 * Gets the bounding box density.
	 * @return Bounding box density
	 */
	public double getMbrDensity()
	{
		if (!this.nodeInfoLoaded )
		{
			loadNodeInfo();
		}
		
		return this.mbrDensity;
	}

	/**
	 * Gets the reference parameters.
	 * @return Reference parameters
	 */
	public ReferenceParameters getReferenceParameters()
	{
		return this.referenceParams;
	}

	/**
	 * Sets the reference parameters.
	 * @param referenceParams Reference parameters
	 */
	public void setReferenceParameters(ReferenceParameters referenceParams)
	{
		this.referenceParams = referenceParams;
	}
	
	// private methods

	/**
	 * Loads detail information on the node.
	 */
	private void loadNodeInfo()
	{
		ListExpr result = sendGetNodeInfoCommand();
		
		// rough list structure check
		if (result.isEmpty()) 
		{
			displayUnknownNodeIdError();
		}
		
		if ((result.listLength() != 2) || 
			(result.second().listLength() != 1) ||
			(result.second().first().listLength() != 10))
		{
			Reporter.showError("Incorrect list structure of GetNodeInfo query result.");
			return;
		}
	
		// parse detail information
		ListExpr nodeInfo = result.second().first();
		
		// node id already known, skip it
		nodeInfo = nodeInfo.rest();

		if (!this.MBRLoaded )
		{
			// read MBR with variable dimensions
			ListExpr mbrList = nodeInfo.first();
			int dimensions = mbrList.listLength() / 2;
			Interval[] intervals = new Interval[dimensions];
			int dimensionCounter = 0;
			double left, right;
		
			// read dimensions consecutively
			while (! mbrList.isEmpty())
			{
				left = mbrList.first().realValue();
				right = mbrList.second().realValue();
				intervals[dimensionCounter] = new Interval(left, right);
		
				mbrList = mbrList.rest();
				mbrList = mbrList.rest();
			
				dimensionCounter++;
			}
				
			this.mbr = new BoundingBox(intervals);
			MBRLoaded = true;
		}
		
		// read other node detail information
		nodeInfo = nodeInfo.rest();
		
		this.noOfSons = nodeInfo.first().intValue();
		nodeInfo = nodeInfo.rest();
		
		this.isLeafNode = nodeInfo.first().boolValue();
		nodeInfo = nodeInfo.rest();
		isLeafNodeLoaded = true;
		
		this.isRootNode = nodeInfo.first().boolValue();
		nodeInfo = nodeInfo.rest();
		
		this.mbrSize = nodeInfo.first().realValue();
		nodeInfo = nodeInfo.rest();
		
		this.mbrDead = nodeInfo.first().realValue();
		nodeInfo = nodeInfo.rest();
		
		this.mbrOverlap = nodeInfo.first().realValue();
		nodeInfo = nodeInfo.rest();
		
		this.mbrOverlapNo = nodeInfo.first().intValue();
		nodeInfo = nodeInfo.rest();
		
		this.mbrDensity = nodeInfo.first().realValue();
		
		this.nodeInfoLoaded = true;
	}
	
	/**
	 * Loads the node's child nodes.
	 */
	private void loadChildNodes()
	{
		if ((isLeafNodeLoaded)&&(this.isLeafNode()))
		{
			return;
		}

		ListExpr result = sendGetNodeSonsCommand();
		
		// rough list structure check
		if (result.isEmpty()) 
		{
			displayUnknownNodeIdError();
		}
		
		if ((result.listLength() != 2) || 
			(result.second().listLength() < 1) ||
			(result.second().first().listLength() != 3))
		{
			Reporter.showError("Incorrect list structure of GetNodeSons query result.");
			return;
		}
		
		// parse all child nodes
		ListExpr listOfSons = result.second();
		Node childNode;
		int childNodeId;
		if (listOfSons.first().second().atomType()!=ListExpr.INT_ATOM)
			listOfSons = ListExpr.theEmptyList();
		
		while (! listOfSons.isEmpty() )
		{
			childNodeId = listOfSons.first().second().intValue();
			childNode = new Node(childNodeId, this, rtree);
			this.childNodes.add(childNode);

			// read MBR of childNode with variable dimensions
			ListExpr mbrList = listOfSons.first().third();
			int dimensions = mbrList.listLength() / 2;
			Interval[] intervals = new Interval[dimensions];
			int dimensionCounter = 0;
			double left, right;
		
			// read dimensions consecutively
			while (! mbrList.isEmpty())
			{
				left = mbrList.first().realValue();
				right = mbrList.second().realValue();
				intervals[dimensionCounter] = new Interval(left, right);
		
				mbrList = mbrList.rest();
				mbrList = mbrList.rest();
			
				dimensionCounter++;
			}
			childNode.setMbr(new BoundingBox(intervals));	

			listOfSons = listOfSons.rest();
		}
		
		isLeafNode = childNodes.isEmpty();
		isLeafNodeLoaded = true;
		this.childNodesLoaded = true;
	}
	
	/**
	 * Loads the node's referenced tuple ids.
	 */
	private void loadTupleIds()
	{
		if (!this.isLeafNode )
		{
			// inner nodes don't have indexed items
			this.noOfEntries = 0;
			return;
		}
		
		ListExpr result = sendGetLeafEntriesCommand();
	
		// rough list structure check
		if (result.isEmpty()) 
		{
			displayUnknownNodeIdError();
		}
		
		if ((result.listLength() != 2) || 
			(result.second().listLength() < 1) ||
			(result.second().first().listLength() != 3))
		{
			Reporter.showError("Incorrect list structure of GetLeafEntries query result.");
			return;
		}
		
		// parse tuple ids
		ListExpr listOfTupleIds = result.second();
		int tupleId;
		
		while (! listOfTupleIds.isEmpty() )
		{
			tupleId = listOfTupleIds.first().second().intValue();
			this.tupleIds.add(tupleId);

			// read MBR of entries with variable dimensions
			ListExpr mbrList = listOfTupleIds.first().third();
			int dimensions = mbrList.listLength() / 2;
			Interval[] intervals = new Interval[dimensions];
			int dimensionCounter = 0;
			double left, right;
		
			// read dimensions consecutively
			while (! mbrList.isEmpty())
			{
				left = mbrList.first().realValue();
				right = mbrList.second().realValue();
				intervals[dimensionCounter] = new Interval(left, right);
		
				mbrList = mbrList.rest();
				mbrList = mbrList.rest();
			
				dimensionCounter++;
			}

			this.indexedTuples.add(new Tuple(tupleId, this, new BoundingBox(intervals)));
		
			listOfTupleIds = listOfTupleIds.rest();
		}

		this.noOfEntries = this.indexedTuples.size();
		this.tupleIdsLoaded = (this.noOfEntries>0);
	}
	
	/**
	 * Loads the referenced tuples and populates the indexed items and tuples collections.
	 */
	private void loadIndexedItems()
	{
		// check if a relation is selected
		if (!this.referenceParams.isComplete())
		{
			// no relation data given
			return;
		}
		
		// retrieve tuple ids
		if (!this.tupleIdsLoaded)
		{
			loadTupleIds();
		}
		
		// retrieve indexed items
		if (this.tupleIds.size() > 0)
		{
			for (Tuple tuple: indexedTuples)
			 	tuple.setReferenceParameters(this.referenceParams);
			this.indexedItems = DrawableFactory.GetReferencedItems(this.referenceParams, this.tupleIds);
			this.indexedItemsLoaded = true;
		}
	}
	
	/**
	 * Sends the getNodeInfo command to Secondo.
	 * @return Query result 
	 */
	private ListExpr sendGetNodeInfoCommand() 
	{
		String cmd = "query getNodeInfo(";
		cmd	+= rtree.getName();
		cmd	+= ",";
		cmd	+= nodeId;
		cmd	+= ") consume";
		
		SecondoManager secondo = new SecondoManager();
		return secondo.sendCommand(cmd, "getNodeInfo");
	}
	
	/**
	 * Sends the getLeafEntries command to Secondo.
	 * @return Query result 
	 */
	private ListExpr sendGetLeafEntriesCommand() 
	{
		String cmd	= "query getLeafEntries(";
		cmd += rtree.getName();
		cmd += ",";
		cmd	+= nodeId;
		cmd	+= ") consume";
		
		SecondoManager secondo = new SecondoManager();
		return secondo.sendCommand(cmd, "getLeafEntries");
	}
	
	/**
	 * Sends the getNodeSons command to Secondo.
	 * @return Query result 
	 */
	private ListExpr sendGetNodeSonsCommand()
	{
		String cmd = "query getNodeSons(";
		cmd += rtree.getName();
		cmd += ",";
		cmd	+= nodeId;
		cmd	+= ") consume";

		SecondoManager secondo = new SecondoManager();
		return secondo.sendCommand(cmd, "getNodeSons");
	}

	/**
	 * Displays an error message if node id is incorrect.
	 */
	private void displayUnknownNodeIdError()
	{
		String errormsg = "Node " + this.nodeId + " could not be found in RTree " + rtree.getName();
		Reporter.showError(errormsg);
	}
}
