package viewer.rtree;
import sj.lang.*;

/**
 * Represents an rtree.
 * 
 * @author Oliver Feuer
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @since 20.02.2010
 * @version 1.2
 */
public class RTree {

	private String name;
	private Node rootNode;
	private int treeHeight;
	private int noNodes;
	private int noEntries;
	private boolean rootLoaded = false;
	private boolean heightLoaded = false;
	private boolean nodesLoaded = false;
	private boolean entriesLoaded = false;
	
	// constructors
	
	/**
	 * Creates a new RTree object. 
	 * @param name Name of the rtree
	 */
	public RTree(String name)
	{
		this.name = name;
	}
		
	// public members
	
	/**
	 * Gets the tree's name.
	 * @return Tree name
	 */
	public String getName()
	{
		return name;
	}
	
	/**
	 * Gets the tree's root node.
	 * @return Root node
	 */
	public Node getRootNode()
	{
		if ( !rootLoaded )
		{
			ListExpr result;
			int nodeId;

//System.out.println("RTree: query getRootNode(" + name + ")");
			SecondoManager secondoManager = new SecondoManager();
			result = secondoManager.sendCommand("query getRootNode(" + name + ")", "Get Root Node");
			nodeId = result.second().intValue();
			rootNode = new Node(nodeId, null, this);
			rootLoaded = true;
		}
		return rootNode;
	}
	
	/**
	 * Gets the tree's height.
	 * @return Tree height
	 */
	public int getTreeHeight()
	{
		if ( !heightLoaded )
		{
			ListExpr result;

//System.out.println("RTree: query treeheight(" + name + ")");
			SecondoManager secondoManager = new SecondoManager();
			result = secondoManager.sendCommand("query treeheight(" + name + ")", "Get Tree Height");
			treeHeight = result.second().intValue();
			heightLoaded = true;
		}
		return treeHeight;
	}
	
	public int getNoNodes()
	{	
		if ( !nodesLoaded )
		{
			ListExpr result;

//System.out.println("RTree: query no_nodes(" + name + ")");
			SecondoManager secondoManager = new SecondoManager();
			result = secondoManager.sendCommand("query no_nodes(" + name + ")", "Get no_nodes");
			noNodes = result.second().intValue();
			nodesLoaded = true;
		}
		return noNodes;
	}
	
	public int getNoEntries()
	{
		if ( !entriesLoaded )
		{
			ListExpr result;

//System.out.println("RTree: query no_entries(" + name + ")");
			SecondoManager secondoManager = new SecondoManager();
			result = secondoManager.sendCommand("query no_entries(" + name + ")", "Get no_entries");
			noEntries = result.second().intValue();
			entriesLoaded = true;
		}
		return noEntries;
	}
}
