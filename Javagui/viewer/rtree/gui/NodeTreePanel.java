package viewer.rtree.gui;

import viewer.rtree.*;
import java.awt.*;
import java.awt.event.*;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import java.util.*;
import tools.Reporter;
import viewer.rtree.secondotypes.*;
import viewer.*;

/**
 * NodeTreePanel displays rtree structure information in a tree view.
 * Child information of a node is read from the database when a node is selected
 * for the first time.
 * 
 * @author Oliver Feuer
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.9
 * @since 08.03.2010
 */
public class NodeTreePanel extends JPanel implements NodeDisplay, TreeSelectionListener, ActionListener, PopupMenuListener
{
	// some constants
	private static final String NO_RTREE_SELECTED = "Choose RTree...";
	private static final String NO_RELATION_SELECTED = "Choose Rel...";
	private static final String NO_ATTRIBUTE_SELECTED = "Choose Attr...";
	private static final String RTREE_OBJECT_TYPE = "rtree,rtree3,rtree4,rtree8";
	private static final String RELATION_OBJECT_TYPE = "rel";

	private static final String UNKNOWN_NODE_ICON = "unknownNodeIcon.gif";
	private static final String LEAF_NODE_ICON = "leafNodeIcon.gif";
	
	// tree to be displayed
	private JTree tree;	
	// underlying tree model
	private DefaultTreeModel treeModel;
	// list of node selection monitors
	private LinkedList<NodeDisplay> selectionMonitors = new LinkedList<NodeDisplay>();
	// currently selected node
	DefaultMutableTreeNode selectedNode = null;
	// name of displayed rtree
	private String rtreeName;	
	
	// object choosers
	private JComboBox rtreeChooser;
	private JComboBox relationChooser;
	private JComboBox attributeChooser;
	private boolean rtreeChooserLoaded = false;
	private boolean relationChooserLoaded = false;
	private boolean attributeChooserLoaded = false;
	
	// reference parameters
	private ReferenceParameters referenceParams = new ReferenceParameters();

	// the viewer
	private RTreeViewer viewer;

	// constructors
	
	/**
	 * Creates a new panel to render a tree view.
	 */
	public NodeTreePanel(RTreeViewer viewer)
	{
		this.viewer = viewer;
		
		// set some layout defaults
		setBackground(Color.white);
		setBorder(BorderFactory.createLineBorder(Color.green));
		setPreferredSize(new Dimension(200,1000));
		setLayout(new BorderLayout());
		
		// create the toolbar
		createToolbar();

		// create and add the tree view panel
		createTreeViewPanel();

                rtreeName = NO_RTREE_SELECTED;
	}
	
	// public methods
	
	/**
	 * Adds a selection monitor being notified
	 * if a new node or tree is selected.
	 * @param nodeDisplay SelectionMonitor to add
	 */
	public void addSelectionMonitor(NodeDisplay nodeDisplay) 
	{
		selectionMonitors.add(nodeDisplay);
	}
	
	/**
	 * Handles the valueChanged event generated when a tree node is selected.
	 */
	public void valueChanged(TreeSelectionEvent e) 
	{
		// get selected node
		DefaultMutableTreeNode node = (DefaultMutableTreeNode)tree.getLastSelectedPathComponent();

		if (node == null)
		{
			return;
		}
		
		this.selectedNode = node;

		// process only nodes, no action on tuple selection
		if (node.getUserObject() instanceof Node)
		{
			Node rtreeNode = (Node) this.selectedNode.getUserObject();
			rtreeNode.setReferenceParameters(this.referenceParams);

			// load child nodes if selected node is currently displayed as a leaf
			if (selectedNode.isLeaf())
			{
				Vector<Node> childNodes = rtreeNode.getChildNodes();
				// load child nodes if there are any...
				if (childNodes.size()>0)
				{
					for ( Node child : childNodes )
					{
						addNode(selectedNode, child);
					}
				}
				// ...or load entries if node is a leaf node!
				else
				{
					for ( Tuple tuple : rtreeNode.getIndexedTuples() )
					{
						addTuple(selectedNode, tuple);
					}
				}
			}

			// notify monitors
			notifyMonitorsNodeSelected(rtreeNode);
		}
		else if (node.getUserObject() instanceof Tuple)
		{
			Tuple tuple = (Tuple) node.getUserObject();
			Node rtreeNode = tuple.getFatherNode();

			// notify monitors
			notifyMonitorsNodeSelected(rtreeNode);
			notifyMonitorsTupleSelected(tuple);
		}
	}
	
	/**
	 * Handles the actionPerformed event. 
	 */
	public void actionPerformed(ActionEvent e) 
	{
		if (e.getSource() == rtreeChooser)
		{
			if (this.rtreeChooser.getSelectedItem() == null) 
			{
				return;
			}

			String selectedItem = (String) this.rtreeChooser.getSelectedItem();		
			if (selectedItem.compareTo(NO_RTREE_SELECTED) == 0) 
			{
				// tree unselected
				viewer.setAttributeMenu(false);
				viewer.setRTreeMenu(0);
				unloadRTree();
       	                        rtreeName = NO_RTREE_SELECTED;
			}
			else 
			{
				// new tree selected
				String[] rtreeNameType = selectedItem.split(":");
                                if (rtreeName.compareTo(rtreeNameType[0])!=0)
				{
					RTree rtree = new RTree(rtreeNameType[0]);
					loadRTree(rtree);
					viewer.setRTreeMenu(((Node)selectedNode.getUserObject()).getMbr().getNoOfDimension());
        	                        rtreeName = rtreeNameType[0];
				}
			}
		}
		else if (e.getSource() == relationChooser)
		{
			if (this.relationChooser.getSelectedItem() == null) 
			{
				return;
			}
		
			this.clearAllReferences();

			String selectedItem = (String) this.relationChooser.getSelectedItem();		
			if (selectedItem.compareTo(NO_RELATION_SELECTED) == 0) 
			{
				// relation unselected
				viewer.setAttributeMenu(false);
				attributeChooser.setSelectedIndex(0);
				this.attributeChooser.setEnabled(false);
			}
			else 
			{
				// relation selected
				repopulateAttributeChooser(selectedItem);
				this.referenceParams.setRelation(selectedItem);
			}
		}
		else if (e.getSource() == attributeChooser)
		{
			if (this.attributeChooser.getSelectedItem() == null) 
			{
				return;
			}
			
			this.clearAllReferences();
			referenceParams.setRelation((String)relationChooser.getSelectedItem());
			viewer.setAttributeMenu(false);
			
			String selectedItem = (String) this.attributeChooser.getSelectedItem();		
			if (selectedItem.compareTo(NO_ATTRIBUTE_SELECTED) != 0) 
			{
				// attribute selected
				String[] attributeNameType = selectedItem.split(":");
				String type = attributeNameType[1].trim();
				if (NodeTreePanel.canDisplay(type))
				{
					viewer.setAttributeMenu(true);
					this.referenceParams.setAttribute(attributeNameType[0].trim());
					this.referenceParams.setType(type);
					if (this.selectedNode != null)
					{
						this.viewer.setWaitCursor();
						if (selectedNode.getUserObject() instanceof Node)
						{
							Node rtreeNode = (Node) this.selectedNode.getUserObject();
							
							// load child entries if selected node is a leaf
							if ((selectedNode.isLeaf())&&(rtreeNode.isLeafNode()))
							{
								for ( Tuple tuple : rtreeNode.getIndexedTuples() )
								{
									addTuple(selectedNode, tuple);
								}
							}
						}
					}
				}
			}
			if ((this.selectedNode != null)
			   &&(selectedNode.getUserObject() instanceof Tuple))
			{
				notifyMonitorsTupleSelected((Tuple)selectedNode.getUserObject());
			}
		}
	}
	
	/**
	 * Handles the popupMenuWillBecomeVisible event. 
	 */
	public void popupMenuWillBecomeVisible(PopupMenuEvent e) 
	{
		if (e.getSource() == rtreeChooser)
		{
			repopulateRTreeChooser();
		}
		else if (e.getSource() == relationChooser)
		{
			repopulateRelationChooser();
		}
	}
	
	// stubs of other popupMenuListener members
	public void popupMenuCanceled(PopupMenuEvent e) {};
	public void popupMenuWillBecomeInvisible(PopupMenuEvent e) {}

	/**
	 * Selects a node in the tree.
	 * @param n Node to select
	 */
	public void selectNode(Node node)
	{
		if (selectedNode.getUserObject() instanceof Tuple)
		{
			selectedNode = (DefaultMutableTreeNode)this.selectedNode.getParent();
		}

		Node actualNode = (Node)this.selectedNode.getUserObject();
		if (node.getNodeId() == actualNode.getNodeId())
		{
			// walk up the tree because of right click
			Object[] pathComponents = this.selectedNode.getPath();
			if (pathComponents.length > 1)
			{
				LinkedList<Object> pathComponentsList = new LinkedList<Object>(Arrays.asList(pathComponents));
				pathComponentsList.remove(pathComponentsList.size() - 1);
				tree.setSelectionPath(new TreePath(pathComponentsList.toArray()));
			}
			return;
		}

		// walk up the tree to search in father nodes
		Node fnode = actualNode.getParentNode();
		Object[] pathComponents = this.selectedNode.getPath();
		LinkedList<Object> pathComponentsList = new LinkedList<Object>(Arrays.asList(pathComponents));

		while ((fnode!=null)&&(pathComponents.length > 1))
		{
			pathComponentsList.remove(pathComponentsList.size() - 1);
			if (node.getNodeId() == fnode.getNodeId())
			{
				tree.setSelectionPath(new TreePath(pathComponentsList.toArray()));
				return;
			}
			fnode = fnode.getParentNode();
		}

		if (!actualNode.isLeafNode())
		{
			// walk down the tree
			pathComponents = this.selectedNode.getPath();
			pathComponentsList = new LinkedList<Object>(Arrays.asList(pathComponents));

			LinkedList<DefaultMutableTreeNode>nodes = new LinkedList<DefaultMutableTreeNode>();
			for (int i = 0; i < pathComponents.length; i++)
				nodes.add((DefaultMutableTreeNode)pathComponents[i]);

			for (Enumeration e = this.selectedNode.children(); e.hasMoreElements();) 
			{
				DefaultMutableTreeNode nextNode = (DefaultMutableTreeNode)e.nextElement();
				if ((Node)nextNode.getUserObject() == node)
				{
					nodes.add(nextNode);
				}
			}
			
			this.viewer.setWaitCursor();
			tree.setSelectionPath(new TreePath(nodes.toArray()));
		}
	}

	/**
	 * Selects a tuple in the tree.
	 * @param tuple Tuple to select
	 */
	public void displayTuple(Tuple tuple) {
		Object[] pathComponents;
		DefaultMutableTreeNode leafNode;

		if (selectedNode.getUserObject() instanceof Node)
		{
			leafNode = this.selectedNode;
		}
		else
		{
			leafNode = (DefaultMutableTreeNode)this.selectedNode.getParent();
		}

		pathComponents = leafNode.getPath();

		LinkedList<DefaultMutableTreeNode> nodes = new LinkedList<DefaultMutableTreeNode>();
		for (int i = 0; i < pathComponents.length; i++)
			nodes.add((DefaultMutableTreeNode)pathComponents[i]);

		for (Enumeration e = leafNode.children(); e.hasMoreElements();) 
		{
			DefaultMutableTreeNode nextNode = (DefaultMutableTreeNode)e.nextElement();
			if ((Tuple)nextNode.getUserObject() == tuple)
			{
				nodes.add(nextNode);
				selectedNode = nextNode;
			}
		}

		this.viewer.setWaitCursor();
		tree.setSelectionPath(new TreePath(nodes.toArray()));
	};


	// stubs of other NodeDisplay members
	public void displayNode(Node n) {};
	public void setRTreeAttributes(String name, int noOfDimensions) {};
	
	/** 
	 * Checks if a given Secondo type is known.
	 * @return True if type is known, otherwise false.
	 */
	public static boolean canDisplay(String type)
	{
		boolean retValue = false;
		type = type.toUpperCase();
		
		for (SecondoType secondoType : SecondoType.values())
		{
			if (secondoType.toString().equals(type))
			{
				retValue = true;
				break;
			}
		}
		return retValue;
	}

	// private methods
	
	/**
	 * Notifies all registered selection monitors that 
	 * a new node has been selected. 
	 * @param node Selected node
	 */
	private void notifyMonitorsNodeSelected(Node node)
	{
		for (NodeDisplay monitor : selectionMonitors )
		{
			monitor.displayNode(node);
		}
	}
	
	/**
	 * Notifies all registered selection monitors that 
	 * a new tuple has been selected. 
	 * @param tuple Selected tuple
	 */
	private void notifyMonitorsTupleSelected(Tuple tuple)
	{
		for (NodeDisplay monitor : selectionMonitors )
		{
			monitor.displayTuple(tuple);
		}
	}
	
	/**
	 * Notifies all registered selection monitors that 
	 * a new tree has been selected. 
	 * @param name Selected tree's name
	 * @param noOfDimensions Number of dimensions
	 */
	private void notifyMonitorsTreeSelected(String name, int noOfDimensions)
	{
		for (NodeDisplay monitor : selectionMonitors )
		{
			monitor.setRTreeAttributes(name, noOfDimensions);
		}
	}

	/**
	 * Creates the toolbar with the tree, relation and attribute choosers.
	 */
	private void createToolbar()
	{
		// create tree chooser
		this.rtreeChooser = new JComboBox();
		this.rtreeChooser.addItem(NO_RTREE_SELECTED);
		this.rtreeChooser.addPopupMenuListener(this);
		this.rtreeChooser.addActionListener(this);
		
		// create relation chooser
		this.relationChooser = new JComboBox();
		this.relationChooser.addItem(NO_RELATION_SELECTED);
		this.relationChooser.addPopupMenuListener(this);
		this.relationChooser.addActionListener(this);
		
		// create attribute chooser
		this.attributeChooser = new JComboBox();
		this.attributeChooser.addItem(NO_ATTRIBUTE_SELECTED);
		this.attributeChooser.setEnabled(false);
		this.attributeChooser.addActionListener(this);
		this.attributeChooser.setRenderer(new ComboBoxRenderer());
		
		// create toolbar and add controls
		JPanel toolbar = new JPanel();
		toolbar.setLayout(new GridLayout(3, 1));
		toolbar.add(this.rtreeChooser);
		toolbar.add(this.relationChooser);
		toolbar.add(this.attributeChooser);
		add(toolbar, BorderLayout.NORTH);
	}
	
	/**
	 * Repopulates the rtree chooser combo box with rtree objects from the
	 * currently opened database. As we don't know when the user opens 
	 * another database this method is always called when the user clicks 
	 * the box (=> popupMenuWillBecomeVisible event).
	 */
	private void repopulateRTreeChooser() 
	{
		// return if no database is open
		SecondoManager manager = new SecondoManager();
		if (!manager.isDatabaseOpen())
		{
			// set items to default
			this.rtreeChooser.removeAllItems();
			this.rtreeChooser.addItem(NO_RTREE_SELECTED);
			return;
		}
		
		// get all objects of rtree type
		Vector<String> rtrees = manager.listObjectsOfType(RTREE_OBJECT_TYPE, true);

		int position=-1;
		int pos=1;
		boolean chooserChanged = false;
		int choosedIndex = rtreeChooser.getSelectedIndex();
		while (pos < rtreeChooser.getItemCount())
		{
			position = rtrees.indexOf((String)rtreeChooser.getItemAt(pos));
			if (position!=-1)
			{
				rtrees.remove(position);
				pos++;
			}
			else
			{
				rtreeChooser.removeItemAt(pos);
				chooserChanged = true;
				if (pos==choosedIndex)
				{
					rtreeChooser.setSelectedIndex(0);
					choosedIndex = 0;
				}
				choosedIndex--;
			}
		}

		for (String rtree : rtrees ) 
		{
			this.rtreeChooser.addItem(rtree);
			chooserChanged = true;
		}

		if (chooserChanged)
		{
			this.rtreeChooser.hidePopup();
			this.rtreeChooser.showPopup();
		}
	}
	
	/**
	 * Repopulates the relation chooser combo box with relation objects from the
	 * currently opened database. As we don't know when the user opens 
	 * another database this method is always called when the user clicks 
	 * the box (=> popupMenuWillBecomeVisible event).
	 */
	private void repopulateRelationChooser() 
	{
		// return if no database is open
		SecondoManager manager = new SecondoManager();
		if (!manager.isDatabaseOpen())
		{
			// set items to default
			this.relationChooser.removeAllItems();
			this.relationChooser.addItem(NO_RELATION_SELECTED);
			return;
		}
		
		// get all objects of relation type
		Vector<String> relations = manager.listObjectsOfType(RELATION_OBJECT_TYPE);

		int position=-1;
		int pos=1;
		boolean chooserChanged = false;
		int choosedIndex = relationChooser.getSelectedIndex();
		while (pos < relationChooser.getItemCount())
		{
			position = relations.indexOf((String)relationChooser.getItemAt(pos));
			if (position!=-1)
			{
				relations.remove(position);
				pos++;
			}
			else
			{
				relationChooser.removeItemAt(pos);
				chooserChanged = true;
				if (pos==choosedIndex)
				{
					relationChooser.setSelectedIndex(0);
					choosedIndex = 0;
				}
				choosedIndex--;
			}
		}

		for (String relation : relations ) 
		{
			this.relationChooser.addItem(relation);
			chooserChanged = true;
		}

		if (chooserChanged)
		{
			this.relationChooser.hidePopup();
			this.relationChooser.showPopup();
		}
	}
	
	/**
	 * Repopulates the attribute chooser combo box with attribute objects from the
	 * given relation.
	 */
	private void repopulateAttributeChooser(String relation)
	{
		// return if no database is open
		SecondoManager manager = new SecondoManager();
		if (!manager.isDatabaseOpen())
		{
			return;
		}
		
		// set items to default
		this.attributeChooser.removeAllItems();
		this.attributeChooser.addItem(NO_ATTRIBUTE_SELECTED);
	
		// get all tuple attributes of selected relation
		Vector<String> attributes = manager.retrieveTupleAttributesOfRelation(relation);
		for (String attribute : attributes ) 
		{
			this.attributeChooser.addItem(attribute);
		}

		this.attributeChooser.setEnabled(true);
	}
	
	/**
	 * Creates and displays the tree view panel.
	 */
	private void createTreeViewPanel() 
	{
		treeModel = new DefaultTreeModel(null);
		tree = new JTree(treeModel);
		tree.setCellRenderer(new CustomCellRenderer());
		tree.addTreeSelectionListener(this);
		tree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
		tree.setShowsRootHandles(true);
		
		JScrollPane treeView = new JScrollPane(tree);
		add(treeView, BorderLayout.CENTER);
	}
	
	/**
	 * Loads an rtree from the database.
	 * Initially, only the root node is displayed in the tree view panel.
	 * @param rtree Rtree to load
	 */
	private void loadRTree(RTree rtree) 
	{
		// clear
		unloadRTree();
		
		// retrieve root node
		Node n = rtree.getRootNode();
		DefaultMutableTreeNode rootNode = new DefaultMutableTreeNode(n);
		this.treeModel.setRoot(rootNode);
		
		// notify monitors that a new tree & node was selected
		notifyMonitorsTreeSelected(rtree.getName(), n.getMbr().getNoOfDimension());

		// select root node and show rtree
		tree.setSelectionRow(0);
	}

	/**
	 * Clears a currently loaded rtree.
	 */
	private void unloadRTree() 
	{
		this.treeModel.setRoot(null);

		clearAllReferences();
		relationChooser.setSelectedIndex(0);
		attributeChooser.setSelectedIndex(0);
		attributeChooser.setEnabled(false);

		// notify monitors that the current tree was deselected
		notifyMonitorsTreeSelected(null, 0);
	}

	/**
	 * Clears all relation references of the currently loaded rtree.
	 */
	private void clearAllReferences()
	{
		this.referenceParams.setRelation("");
		this.referenceParams.setAttribute("");
		this.referenceParams.setType("");

		if (treeModel.getRoot() != null)
		{
			DefaultMutableTreeNode rootNode = (DefaultMutableTreeNode)this.treeModel.getRoot();
			clearNodeReferences(rootNode);
		}
	}

	/**
	 * Clears all relation references of the given node and children recursively.
	 */
	private void clearNodeReferences(DefaultMutableTreeNode node)
	{
		if (node.getUserObject() instanceof Node)
		{
			// this is a node, so clear all references recursively
			Node rtreeNode = (Node) node.getUserObject();
			rtreeNode.clearIndexedItems();
		
			if (node.getChildCount() > 0)
			{
				for (Enumeration e = node.children(); e.hasMoreElements() ;) 
				{
					this.clearNodeReferences((DefaultMutableTreeNode)e.nextElement());
				}
			}
		}
	}
	
	/** 
	 * Adds a new node to the tree.
	 * @param parentNode Node to add to
	 * @param rtreeNode New node to add
	 */
	private void addNode(DefaultMutableTreeNode parentNode, Node rtreeNode) 
	{
		DefaultMutableTreeNode treeNode = new DefaultMutableTreeNode(rtreeNode);
		this.treeModel.insertNodeInto(treeNode, parentNode, parentNode.getChildCount());
	}
	
	/** 
	 * Adds a new tuple to the tree.
	 * @param parentNode Node to add to
	 * @param tuple New tuple to add
	 */
	private void addTuple(DefaultMutableTreeNode parentNode, Tuple tuple) 
	{
		DefaultMutableTreeNode treeNode = new DefaultMutableTreeNode(tuple);
		tuple.setTupleTreeNode(treeNode);
		this.treeModel.insertNodeInto(treeNode, parentNode, parentNode.getChildCount());
	}
	
	/** 
	 * Replaces the default icons for unknown nodes and leaf nodes.
	 * 
	 * @author Christian Oevermann
	 * @version 1.0
	 * @since 20.01.2009
	 */
	private class CustomCellRenderer extends DefaultTreeCellRenderer
	{
		// load image resources
		ImageIcon unknownNodeIcon = new ImageIcon(CustomCellRenderer.class.getResource(UNKNOWN_NODE_ICON));
		ImageIcon leafNodeIcon = new ImageIcon(CustomCellRenderer.class.getResource(LEAF_NODE_ICON));
		
		public Component getTreeCellRendererComponent(JTree tree,
				Object value, boolean sel, boolean expanded, boolean leaf,
				int row, boolean hasFocus)
		{
			super.getTreeCellRendererComponent(tree, value, sel, expanded,
					leaf, row, hasFocus);
			
			Object nodeObj = ((DefaultMutableTreeNode)value).getUserObject();
			
			if (nodeObj instanceof Node)
			{
				// replace icons
				if (	!((Node)nodeObj).childNodesLoaded() &&
					!((Node)nodeObj).nodeInfoLoaded() )
				{
					setIcon(unknownNodeIcon);
				}
				else
				{
					if (((Node)nodeObj).isLeafNode())
					{
						setIcon(leafNodeIcon);
					}
				}
			}
			
			return this;
		}
	}

	/** 
	 * Renders displayable attributes in green.
	 * 
	 * @author Christian Oevermann
	 * @version 1.0
	 * @since 02.02.2010
	 */
	private class ComboBoxRenderer extends JLabel implements ListCellRenderer
	{
		public Component getListCellRendererComponent(JList list,
				Object value, int index, boolean sel, boolean hasFocus)
		{
			setText(value.toString());
			setEnabled(list.isEnabled());
			setFont(list.getFont());
		    setOpaque(true);
		    setMinimumSize(new Dimension(10, 18));
		    setPreferredSize(new Dimension(10, 18));
		    
		    // get type
			String[] attributeNameType = value.toString().split(":");
			String type = "";
			if (attributeNameType.length == 2)
			{
				type = attributeNameType[1].trim();
			}
			
			// choose colors based on selection status and presentability
			if (sel)
			{
				setBackground(list.getSelectionBackground());
				if (NodeTreePanel.canDisplay(type))
				{
					setForeground(new Color(0, 255, 0, 255));
				}
				else
				{
					setForeground(list.getSelectionForeground());
				}
			}
			else
			{
				setBackground(list.getBackground());
				if (NodeTreePanel.canDisplay(type))
				{
					setForeground(new Color(52, 196, 52, 255));
				}
				else
				{
					setForeground(list.getForeground());
				}
			}
			
			return this;
		}
	}
}
