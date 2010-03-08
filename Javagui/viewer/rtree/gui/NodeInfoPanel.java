package viewer.rtree.gui;

import java.awt.*;
import java.awt.event.MouseEvent;

import javax.swing.*;
import javax.swing.table.*;
import javax.swing.border.*;
import java.text.*;
import viewer.rtree.*;

/**
 * NodeInfoPanel displays detail informationen on a selected node and the selected tree.
 * 
 * @@author Oliver Feuer
 * @@author Benedikt Buer
 * @@author Christian Oevermann
 * @@version 1.4
 * @@since 20.02.2010
 */
public class NodeInfoPanel extends JPanel implements NodeDisplay
{
	private static final long serialVersionUID = -8814226134652849118L;
	
	// panel to display
	private JPanel nodePanel;
	private JPanel tuplePanel;
	// table to display
	private JTable nodeInfoTable;
	// abstract table model
	private NodeInfoTableModel tableModel;
	// tuple info text area
	private JTextArea tupleInfo;

	private RTree tree;
	private int noOfDimensions;
	
	/**
	 * Creates a new panel to display node detail information. 
	 */
	public NodeInfoPanel()
	{
		// set some layout defaults
		setLayout(new BorderLayout());

		// create node info panel
		this.nodePanel = new JPanel();
		this.tableModel = new NodeInfoTableModel();
		this.nodeInfoTable = new NodeInfoTable(tableModel);
		this.nodePanel.add(nodeInfoTable);
		JScrollPane scrollPane1 = new JScrollPane(nodePanel);
		scrollPane1.setBorder(BorderFactory.createLineBorder(Color.red));
		
		// create tuple info panel
		this.tupleInfo = new JTextArea();
		this.tupleInfo.setEditable(false);
		this.tupleInfo.setText("");
		JScrollPane scrollPane2 = new JScrollPane(tupleInfo);
		scrollPane2.setBorder(BorderFactory.createLineBorder(Color.blue));

                JSplitPane splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT, 
                                                      scrollPane1, scrollPane2);
                splitPane.setOneTouchExpandable(true);

		this.add(splitPane, BorderLayout.CENTER);
	}
	
	/**
	 * Displays information on the given node.
	 * @@param n Node to display information on
	 */
	public void displayNode(Node node) 
	{
		tableModel.setNode(node);
		nodeInfoTable.repaint();
		this.tupleInfo.setText("");
	}
	
	/**
	 * Displays information on the given tuple.
	 * @@param t Tuple to display information on
	 */
	public void displayTuple(Tuple tuple) 
	{
		if ( tuple != null)
		{
			this.tupleInfo.setText(tuple.completeToString());
		}
		else
		{
			this.tupleInfo.setText("");
		}
		nodeInfoTable.repaint();
	}
	
	/**
	 * Sets rtree attributes.
	 * @@param name RTree name
	 * @@param noOfDimensions Total number of dimensions
	 */
	public void setRTreeAttributes(String name, int noOfDimensions)
	{
		if ( name == null )
		{
			tableModel.setRTree(null, 0);
			tableModel.setNode(null);
			this.tupleInfo.setText("");
			nodeInfoTable.repaint();
		}
		
		else
		{
			this.tree = new RTree(name);
			this.noOfDimensions = noOfDimensions;
			tableModel.setRTree(tree, this.noOfDimensions);
			nodeInfoTable.repaint();
		}
	}
	
	// stubs of other NodeDisplay members
	public void selectNode(Node node) {};
	
	
	
	/**
	 * The table used to display node detail information.
	 * 
	 * @@author Benedikt Buer
	 * @@version 1.1
	 * @@since 10.2.2010
	 */
	private class NodeInfoTable extends JTable
	{
		private static final long serialVersionUID = 17421624834933038L;

		public NodeInfoTable(AbstractTableModel tableModel)
		{
			super(tableModel);
			
			TableColumn column = this.getColumnModel().getColumn(0);
		    column.setPreferredWidth(110);
			column = this.getColumnModel().getColumn(1);
		    column.setPreferredWidth(140);
		}
		
        public String getToolTipText(MouseEvent e) {
            java.awt.Point p = e.getPoint();
            int rowIndex = rowAtPoint(p);
            int colIndex = columnAtPoint(p);
            int realColumnIndex = convertColumnIndexToModel(colIndex);

            if ((realColumnIndex == 0)||(realColumnIndex == 1))
            {
            	switch (rowIndex )
            	{
				case 0: return "The name of the rtree."; 
				case 1: return "The height of the rtree.";
				case 2: return "The total number of nodes in the rtree.";
				case 3: return "The total number of keys in the rtree.";
				case 4: return "The total number of dimensions.";
				case 5: return "";
				case 6: return "The internal id of the node currently in focus.";  
				case 7: return "The level of the node in the rtree.";
				case 8: return "True if the currently focused node is a root node.";  
				case 9: return "True if the currently focused node is a leaf node.";
				case 10: return "The number of sons of the currently focused node.";
				case 11: return "The number of entries of the currently focused leaf node.";
				case 12: return "The size of the minimal bounding box.";
				case 13: return "The size of the minimal bounding box area that is not overlapped by at least one child node's bounding box.";
				case 14: return "The size of the minimal bounding box area where at least two child node's bounding boxes overlap.";
				case 15: return "The total number of pairwise overlapping son minimal bounding boxes.";
				case 16: return "The average density of son bounding boxes overlapping any given point from this node's minimal bounding box.";
				default: return "-";
            	}
            }
            
            else 
            { 
                return super.getToolTipText(e);
            }
        }

	}
	
	/**
	 * The abstract table model used to display node detail information.
	 * 
	 * @@author Benedikt Buer
	 * @@author Christian Oevermann
	 * @@version 1.1
	 * @@since 26.12.2009
	 */
	private class NodeInfoTableModel extends AbstractTableModel 
	{
		private static final long serialVersionUID = -3848425737904063198L;

		// node to display
		private Node node;
		
		// rtree to display
		private RTree rtree;
		int noOfDimensions;
		

		String[] labels = {"Tree Name", "Tree Height", "No. of Nodes", "No. of Keys", "Dimensions", "",
				"Node Id", "Node Level", "Is Root Node", "Is Leaf Node", "No. of Sons", "No. of Entries", 
				"MBR Size", "MBR Dead", "MBR Overlap", "MBR Overlap No.", "MBR Density"};

		/**
		 * Sets a new node into the table model.
		 * @@param node Node
		 */
		public void setNode(Node node) 
		{
			this.node = node;
			repaint();
		}
		
		/**
		 * Sets a new rtree into the  table model
		 * @@param tree RTree
		 */
		public void setRTree(RTree tree, int noOfDimensions)
		{
			this.rtree = tree;
			this.noOfDimensions = noOfDimensions;
			repaint();
		}
		
		/**
		 * Gets the total columns.
		 */
		public int getColumnCount() 
		{
			return 2;
		}
		
		/**
		 * Gets the total rows.
		 */
		public int getRowCount() 
		{
			return 17;
		}
		
		/**
		 * Gets field contents by row and column.
		 * @@param row Table row
		 * @@param column Table column
		 */
		public Object getValueAt(int row, int column) 
		{	
			DecimalFormat dcFormat1 = new DecimalFormat("#.###");
			DecimalFormat dcFormat2 = new DecimalFormat("#.############");

			if (column == 0)
			{
				return labels[row];
			}

			// No RTree selected
			if (rtree == null )
			{
				if (row==5)
				{
					return "";
				}
				else
				{
					return "-";
				}
			}
				
			// Return values
			else
			{
				switch (row) 
				{
					case 0: return rtree.getName();  
					case 1: return rtree.getTreeHeight();
					case 2: return rtree.getNoNodes();
					case 3: return rtree.getNoEntries();
					case 4: return noOfDimensions;
					case 5: return "";
				}

				// node is undefined
				if (node == null)
				{
					return "-";
				}
				
				// Return node values
				else
				{
					switch (row) 
					{
						case 6: return node.getNodeId();  
						case 7: return node.getNodeLevel();  
						case 8: return node.isRootNode();  
						case 9: return node.isLeafNode();
						case 10: return node.getNoOfSons();
						case 11: return node.getNoOfEntries();
						case 12: return dcFormat1.format(node.getMbrSize());
						case 13: return dcFormat1.format(node.getMbrDead());
						case 14: return dcFormat1.format(node.getMbrOverlap());
						case 15: return node.getMbrOverlapNo();
						case 16: return dcFormat2.format(node.getMbrDensity());
					}
				}
			}
			return "";
		}
		
		private String round(double in)
		{
			double num = in * 10000;
			long longNum = Math.round(num);
			return String.valueOf(longNum / 10000);
		}
	}
}
