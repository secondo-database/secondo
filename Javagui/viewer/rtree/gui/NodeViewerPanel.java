package viewer.rtree.gui;

import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import javax.swing.*;
import viewer.rtree.*;
import java.util.*;
import viewer.rtree.datatypes.*;
import tools.Reporter;
import java.text.*;
import viewer.*;


/**
 * NodeViewerPanel displays all bounding boxes of the current node and its children.
 * 
 * @author Oliver Feuer
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.9
 * @since 08.03.2010
 */
public class NodeViewerPanel extends JPanel implements NodeDisplay, MouseListener, ActionListener 
{
	// some constants
	private static final int POLL_TIME = 200;

	// the viewer
	private RTreeViewer viewer;
	private DrawingPane drawingPane;
	private JScrollPane scroller;
	
	// tree name
	private String treeName;
	// numer of tree dimensions
	private int noOfDimensions;
	private String nodeText;

	// current node to render
	private Node node;
	private Vector<Node> childNodes;
	private Node selectedNode = null;
	private Tuple selectedTuple = null;
	private int selectedTupleNo = -1;
	// list of rectangles to display
	private LinkedList<Rectangle2D.Double> rects = new LinkedList<Rectangle2D.Double>();
	// list of node ids
	private LinkedList nodeIds = new LinkedList();
	private LinkedList<Tuple> tupleList = new LinkedList<Tuple>();
	private LinkedList<Rectangle2D.Double> tupleBBs = new LinkedList<Rectangle2D.Double>();
	private LinkedList<Node> fatherNodes = new LinkedList<Node>();
	private LinkedList<Rectangle2D.Double> fatherNodeBBs = new LinkedList<Rectangle2D.Double>();
	private Rectangle2D.Double rootMBR = null;
	// list of referenced items to display
	private LinkedList<Drawable> items = new LinkedList<Drawable>();
	// list of node selection monitors
	private LinkedList<NodeDisplay> selectionMonitors = new LinkedList<NodeDisplay>();
	
	// containers for rendering and projection parameters
	private RenderParameters renderParams = new RenderParameters();
	private ProjectionParameters projectionParams = new ProjectionParameters();
	private Dimension drawingSize = new Dimension();
	private double viewScale = 1.0;
	private int drawWidth;
	private int drawHeight;

	// Status bar
	private JPanel statusBar;
	private JTextArea statusMessage;
	private javax.swing.Timer timer;
	
	private boolean displayTupleBBs = true;
	private boolean displayRefs = false;
	private boolean displayChildRefs = false;
	
	// constructor(s)

	/**
	 * Creates a new panel to render node bounding boxes. 
	 */
	public NodeViewerPanel(RTreeViewer viewer)
	{
		this.viewer = viewer;
		
		// set some layout defaults
		setLayout(new BorderLayout());
		setBackground(Color.white);
		setBorder(BorderFactory.createLineBorder(Color.yellow));
		setPreferredSize(new Dimension(1000, 500));
		
		// create viewer
		drawingPane = new DrawingPane();
		drawingPane.setBackground(Color.white);

		// add listener to process mouse click events on rectangles
		drawingPane.addMouseListener(this);

		scroller = new JScrollPane(drawingPane);
		add(scroller, BorderLayout.CENTER);
		
		// create toolbar and statusbar
		nodeText = "\n\n Cursor Position: ";

		// create controls
		this.statusBar = new JPanel();
		this.statusBar.setBorder(BorderFactory.createLineBorder(Color.gray));
		this.statusBar.setPreferredSize(new Dimension(3000, 50));
		this.statusBar.setLayout(new BorderLayout());
		this.statusMessage = new JTextArea();
		this.statusMessage.setBackground(this.statusBar.getBackground());
		this.statusBar.add(this.statusMessage, BorderLayout.LINE_START);

		add(statusBar, BorderLayout.SOUTH);
		
		// create timer to display mouse position
		timer = new javax.swing.Timer(POLL_TIME, this);
		timer.start();
	}
	
	// public methods

	/**
	 * Renders a node's bounding box including all child nodes.
	 * @param n Node to render
	 */
	public void displayNode(Node n)
	{
		this.node = n;
		setNodeText();
		fatherNodes.clear();
		if (node != null )
		{
			this.viewer.setWaitCursor();
			this.selectedNode = node;
			Node parent = node.getParentNode();

			if ((rootMBR==null)&&(parent==null))
			{
				rootMBR = node.getMbr().project(this.projectionParams.getProjectionDimX(),
								this.projectionParams.getProjectionDimY());

				// calculate offsets and scale factor
				calculateOffsets();
				calculateScaleFactor();

				viewScale = 1.0;
				drawWidth = scroller.getWidth()-20;
				drawHeight = scroller.getHeight()-20;
				drawingSize.setSize(drawWidth, drawHeight);
				drawingPane.setPreferredSize(drawingSize);
			}

			while (parent!=null)
			{
				fatherNodes.addFirst(parent);
				parent = parent.getParentNode();
			}

			this.selectedTuple = null;
			this.selectedTupleNo = -1;
		}
		repaint();
	}

	/**
	 * Renders a selected tuple.
	 * @param tuple Tuple to render
	 */
	public void displayTuple(Tuple tuple) 
	{
		this.viewer.setWaitCursor();
		this.selectedTuple = tuple;

		repaint();
	} 

	// stubs of other NodeDisplay members
	public void selectNode(Node node) {};

	/**
	 * Sets rtree attributes.
	 * @param name RTree name
	 * @param noOfDimensions Total number of dimensions
	 */
	public void setRTreeAttributes(String name, int noOfDimensions)
	{
		// clears the currently displayed rtree
		unloadRTree();

		// set default projection
		this.projectionParams.setProjectionDimX(0);
		this.projectionParams.setProjectionDimY(1);

		if ((name != null) && (noOfDimensions > 0))
		{
			// set new tree attributes
			this.treeName = name;
			this.noOfDimensions = noOfDimensions;
		}
	}



	/**
	 * DrawingPane displays all bounding boxes of the current node and its children.
	 * 
	 * @author Oliver Feuer
	 * @version 1.1
	 * @since 08.03.2010
	 */
	private class DrawingPane extends JPanel {
	
		/**
		 * Renders all bounding boxes.
		 * 
		 * Before rendering three issues have to be addressed:
		 * - choose the correct projection
		 * - move the top bounding box' upper left corner to (0, 0), 
		 *   i.e. compute the offset
		 * - compute the correct scale factor to fit the complete rendering into the panel
		 */
		public void paintComponent(Graphics g) 
		{
			super.paintComponent(g);
		
			// return if no node is given
			if (node == null )
			{
				return;
			}
				
			// clear lists
			rects.clear();
			nodeIds.clear();
			tupleBBs.clear();
			fatherNodeBBs.clear();

	 		Graphics2D g2D = (Graphics2D) g;

			// add all father nodes' bounding box rectangles
			for (Node fnode : fatherNodes) 
			{
				fatherNodeBBs.add(fnode.getMbr().project(projectionParams.getProjectionDimX(),
									projectionParams.getProjectionDimY()));
			}

			// apply offsets and scale factor to father rects and calculte indivdual grey color type
			int fNodesize = fatherNodeBBs.size();
			int colNum = 0;
			Color fNodeColor;
			for (int i = 0; i < fNodesize ; i++)
			{
				fatherNodeBBs.get(i).x = fatherNodeBBs.get(i).x + projectionParams.getOffsetX(); 
				fatherNodeBBs.get(i).y = fatherNodeBBs.get(i).y + projectionParams.getOffsetY(); 
				
				fatherNodeBBs.get(i).x = fatherNodeBBs.get(i).x * projectionParams.getScaleFactor(); 
				fatherNodeBBs.get(i).y = fatherNodeBBs.get(i).y * projectionParams.getScaleFactor();  
				fatherNodeBBs.get(i).width = fatherNodeBBs.get(i).width * projectionParams.getScaleFactor();  
				fatherNodeBBs.get(i).height = fatherNodeBBs.get(i).height * projectionParams.getScaleFactor();  

				if (fNodesize>1)
					colNum = 50 + (i*150)/(fNodesize-1);
				else
					colNum = 50;
				fNodeColor = new Color(colNum, colNum, colNum, 255);
				drawRect(fatherNodeBBs.get(i), g2D, renderParams.getSelectedNodeBBLineColor(), fNodeColor);
			}
		
		        // add current node's bounding box rectangle and id to lists
			rects.add(node.getMbr().project(projectionParams.getProjectionDimX(),
					projectionParams.getProjectionDimY()));
			nodeIds.add(new Integer(node.getNodeId()));
		
			// add all child nodes' bounding box rectangles and node ids
			childNodes = node.getChildNodes();
			for (Node child : childNodes) 
			{
				rects.add(child.getMbr().project(projectionParams.getProjectionDimX(),
						projectionParams.getProjectionDimY()));
				nodeIds.add(new Integer(child.getNodeId()));
			}

			// apply offsets and scale factor to child rects
			applyOffsets();
			applyScaleFactor();

			// draw nodes' area first
			drawRect(rects.get(0), g2D, new Color(255, 255, 255, 255), 
						    new Color(255, 255, 255, 255));
			// draw all nodes' bounding boxes,
			// using appropriate colors
			for (int i = 0; i < rects.size(); i++)
			{
				if ((Integer)nodeIds.get(i) == selectedNode.getNodeId())
					drawRect(rects.get(i), g2D,
					renderParams.getSelectedNodeBBLineColor(), 
					renderParams.getSelectedNodeBBFillColor());
				else
					drawRect(rects.get(i), g2D,
					renderParams.getChildNodeBBLineColor(), 
					renderParams.getChildNodeBBFillColor());
			}
		
			// draw all tuples' bounding boxes,
			// using appropriate colors
			int selected = -1;
			if (displayTupleBBs)
			{
				// add all entries' bounding box rectangles
				tupleList = node.getIndexedTuples();
				for (Tuple tuple : tupleList) 
				{
					tupleBBs.add(tuple.getMbr().project(projectionParams.getProjectionDimX(),
							projectionParams.getProjectionDimY()));
				}

				// apply offsets and scale factor to entries' bounding boxes
				for (int i = 0; i < tupleBBs.size(); i++)
				{
					if ((selectedTuple!=null)&&(selectedTuple.getTupleTreeNode()!=null)
					  &&(selectedTuple.getTupleTreeNode()==tupleList.get(i).getTupleTreeNode()))
					{
						selected = i;
						selectedTupleNo = i;
					}

					tupleBBs.get(i).x = tupleBBs.get(i).x + projectionParams.getOffsetX(); 
					tupleBBs.get(i).y = tupleBBs.get(i).y + projectionParams.getOffsetY(); 
				
					tupleBBs.get(i).x = tupleBBs.get(i).x * projectionParams.getScaleFactor(); 
					tupleBBs.get(i).y = tupleBBs.get(i).y * projectionParams.getScaleFactor();  
					tupleBBs.get(i).width = tupleBBs.get(i).width * projectionParams.getScaleFactor();  
					tupleBBs.get(i).height = tupleBBs.get(i).height * projectionParams.getScaleFactor();  

					if (tupleBBs.get(i).width < renderParams.getminBBsize())
					{
						tupleBBs.get(i).x += (tupleBBs.get(i).width - renderParams.getminBBsize())/2;
						tupleBBs.get(i).width = renderParams.getminBBsize();
					}
					if (tupleBBs.get(i).height < renderParams.getminBBsize())
					{
						tupleBBs.get(i).y += (tupleBBs.get(i).height - renderParams.getminBBsize())/2;
						tupleBBs.get(i).height = renderParams.getminBBsize();
					}

					if (i!=selected)
						drawRect(tupleBBs.get(i), g2D,
							renderParams.getEntryTupleBBLineColor(), 
							renderParams.getEntryTupleBBFillColor());
				}
				// draw bounding box of selected tuple on top
				if (selected>-1)
				{
						drawRect(tupleBBs.get(selected), g2D,
							renderParams.getSelectedTupleBBLineColor(), 
							renderParams.getSelectedTupleBBFillColor());
				}
			}
		
			// draw referenced items
			items = node.getIndexedItems();
			NodeStatus nStatus = NodeStatus.CURRENT;
			selected = -1;
			for (int i = 0; i < items.size(); i++)
			{
				nStatus = NodeStatus.CURRENT;
				if ((selectedTuple!=null)
				  &&(selectedTuple.getTupleId()==items.get(i).getTupleId()))
				{
					selected = i;
					nStatus = NodeStatus.SELECTED;
				}
				items.get(i).setProjectionParameters(projectionParams);
				items.get(i).setRenderParameters(renderParams, nStatus);
				items.get(i).draw(g);
			}
			// draw item of selected tuple on top
			if (selected>-1)
			{
				items.get(selected).setProjectionParameters(projectionParams);
				items.get(selected).setRenderParameters(renderParams, NodeStatus.SELECTED);
				items.get(selected).draw(g);
			}
		
			if (displayRefs)
			{

				// draw child node referenced items
				for (Node child : childNodes) 
				{
			
					if (displayChildRefs || child.getNodeId() == selectedNode.getNodeId())
					{
						child.setReferenceParameters(node.getReferenceParameters());
						LinkedList<Drawable> items = child.getIndexedItems();
						for (int i = 0; i < items.size(); i++)
						{
							items.get(i).setProjectionParameters(projectionParams);
							if (child.getNodeId() == selectedNode.getNodeId())
								items.get(i).setRenderParameters(renderParams, NodeStatus.SELECTED);
							else
								items.get(i).setRenderParameters(renderParams, NodeStatus.CHILD);
							items.get(i).draw(g);
						}
					}
				}
			}
			viewer.setDefaultCursor();
		}
	} //end of private class DrawingPane

	
	/**
	 * Adds a selection monitor being notified
	 * if a new node is selected by clicking its bounding box.
	 * @param nodeDisplay SelectionMonitor to add
	 */
	public void addSelectionMonitor(NodeDisplay nodeDisplay) 
	{
		selectionMonitors.add(nodeDisplay);
	}
	
	/**
	 * Handles the mouseClicked event. 
	 */
	public void mouseClicked (MouseEvent e) 
	{
		if (node!=null)
		{
			// find out clicked child node
			Node newlySelectedNode = getSelectedNode(e.getX(), e.getY());
		
			if (newlySelectedNode != null )
			{
				this.viewer.setWaitCursor();
				// check if right button was clicked
				if (e.getButton() == e.BUTTON3)
				{
					notifyMonitorsNodeSelected(selectedNode);
					return;
				}
				else if (this.selectedNode.getNodeId() != newlySelectedNode.getNodeId())
				{
					// set new node id, notify monitors and update canvas
					notifyMonitorsNodeFocussed(newlySelectedNode);
					this.selectedNode = newlySelectedNode;
					repaint();
					return;
				}
			}

			if (node.isLeafNode())
			{
				// check if right button was clicked
				if (e.getButton() == e.BUTTON3)
				{
					notifyMonitorsNodeSelected(selectedNode);
					return;
				}

				// find out clicked tuple
				Tuple newlySelectedTuple = getSelectedTuple(e.getX(), e.getY());
		
				if (newlySelectedTuple != null)
				{
					if ((selectedTuple == null) ||
					   ((selectedTuple != null)&&(selectedTuple.getTupleTreeNode()!=null)
					  &&(selectedTuple.getTupleTreeNode()!=newlySelectedTuple.getTupleTreeNode())))
					{
						this.viewer.setWaitCursor();

						// set new tuple id, notify monitors and update canvas
						this.selectedTuple = newlySelectedTuple;
						notifyMonitorsTupleSelected(selectedTuple);
						repaint();
						return;
					}
				}
			}

			// find out clicked father node
			Node selectedFatherNode = getSelectedFatherNode(e.getX(), e.getY());
		
			if (selectedFatherNode != null)
			{
				notifyMonitorsNodeSelected(selectedFatherNode);
				return;
			}
		}
	}
	
	// stubs of other MouseListener members
	public void mouseEntered (MouseEvent e) {};
	public void mousePressed (MouseEvent e) {};
	public void mouseReleased (MouseEvent e) {};
	public void mouseExited (MouseEvent e) {};
	
	/**
	 * Handles the actionPerformed event. 
	 */
	public void actionPerformed(ActionEvent e) 
	{
		if (e.getSource() == timer)
		{
			PointerInfo info = MouseInfo.getPointerInfo();

			setPointerPositionStatusMessage(info.getLocation());

			repaint();
			this.timer.start();
		}
		
	}

	/**
	 *  Sets the x projection and update drawing area
	 */
	public void setXProjection(int dim) 
	{
		this.projectionParams.setProjectionDimX(dim);
		// update the status bar and resize the viewer area
		setNodeText();
		setRootView();
	}

	/**
	 *  Sets the y projection and update drawing area
	 */
	public void setYProjection(int dim) 
	{
		this.projectionParams.setProjectionDimY(dim);
		// update the status bar and resize the viewer area
		setNodeText();
		setRootView();
	}

	/**
	 *  Switch between the selected projection dims
	 */
	public void switchAxis() 
	{
		// switch projection dims and update drawing area
		int switchProjectionDim = this.projectionParams.getProjectionDimX();
		this.projectionParams.setProjectionDimX(this.projectionParams.getProjectionDimY());
		this.projectionParams.setProjectionDimY(switchProjectionDim);

		// update the status bar and resize the viewer area
		setNodeText();
		setRootView();
	}

	/**
	 *  Sets the viewer scalefactor to show root
	 */
	public void setRootView() 
	{
		calculateScaleFactor();

		viewScale = 1.0;
		drawWidth = scroller.getWidth()-20;
		drawHeight = scroller.getHeight()-20;
		drawingSize.setSize(drawWidth, drawHeight);

		update();
	}

	/**
	 *  Sets the viewer scalefactor show selected node
	 */
	public void setNodeView() 
	{
		calculateScaleFactor();
		double scale = this.projectionParams.getScaleFactor();

		Rectangle2D nodeMBR = node.getMbr().project(this.projectionParams.getProjectionDimX(),
							this.projectionParams.getProjectionDimY());

		double windowWidth = scroller.getWidth() - 20 - 2 * this.projectionParams.getPadding();
		double windowHeight = scroller.getHeight() - 20 - 2 * this.projectionParams.getPadding() 
							- this.projectionParams.getExtraPaddingTop()
							- this.projectionParams.getExtraPaddingBottom();

		double maxX = windowWidth / nodeMBR.getWidth();
		double maxY = windowHeight / nodeMBR.getHeight();

		this.projectionParams.setScaleFactor(Math.min(maxX, maxY));

		viewScale = Math.min(maxX, maxY)/scale;

		drawWidth = scroller.getWidth()-20 ;
		drawHeight = scroller.getHeight()-20;
		drawingSize.setSize(((int) drawWidth*viewScale), ((int) drawHeight*viewScale));

		update();

		// sets the viewport to current selected node
		Rectangle viewRect = new Rectangle(scroller.getViewportBorderBounds());
		int xp = (int)((nodeMBR.getX() + this.projectionParams.getOffsetX())*this.projectionParams.getScaleFactor());
		int yp = (int)((nodeMBR.getY() + this.projectionParams.getOffsetY())*this.projectionParams.getScaleFactor());
		viewRect.setLocation(xp, yp);
		drawingPane.scrollRectToVisible(viewRect);
	}

	/**
	 *  increments the viewer scalefactor to zoom in
	 */
	public void setZoomIn() 
	{
		double scale = this.projectionParams.getScaleFactor();
		scale *= 1.05;
		this.projectionParams.setScaleFactor(scale);

		viewScale *= 1.05;
		drawingSize.setSize(((int) drawWidth*viewScale), ((int) drawHeight*viewScale));

		update();

		// sets the viewport to current selected node
		Rectangle2D nodeMBR = node.getMbr().project(this.projectionParams.getProjectionDimX(),
							this.projectionParams.getProjectionDimY());

		Rectangle viewRect = new Rectangle(scroller.getViewportBorderBounds());
		int xp = (int)((nodeMBR.getX() + this.projectionParams.getOffsetX())*this.projectionParams.getScaleFactor());
		int yp = (int)((nodeMBR.getY() + this.projectionParams.getOffsetY())*this.projectionParams.getScaleFactor());
		viewRect.setLocation(xp, yp);
		drawingPane.scrollRectToVisible(viewRect);
	}

	/**
	 *  decrements the viewer scalefactor to zoom out
	 */
	public void setZoomOut() 
	{
		double scale = this.projectionParams.getScaleFactor();
		scale *= 1/1.05;
		this.projectionParams.setScaleFactor(scale);

		viewScale *= 1/1.05;
		drawingSize.setSize(((int) drawWidth*viewScale), ((int) drawHeight*viewScale));

		update();

		// sets the viewport to current selected node
		Rectangle2D nodeMBR = node.getMbr().project(this.projectionParams.getProjectionDimX(),
							this.projectionParams.getProjectionDimY());

		Rectangle viewRect = new Rectangle(scroller.getViewportBorderBounds());
		int xp = (int)((nodeMBR.getX() + this.projectionParams.getOffsetX())*this.projectionParams.getScaleFactor());
		int yp = (int)((nodeMBR.getY() + this.projectionParams.getOffsetY())*this.projectionParams.getScaleFactor());
		viewRect.setLocation(xp, yp);
		drawingPane.scrollRectToVisible(viewRect);
	}

	/**
	 *  Sets the show tuple-BBs state
	 */
	public void setTupleShow(boolean selected) 
	{
		this.displayTupleBBs = selected;
		if (this.node != null)
			this.viewer.setWaitCursor();
		repaint();
	}

	/**
	 *  Sets the show references state
	 */
	public void setShow(boolean selected) 
	{
		this.displayRefs = selected;
		if (this.node != null)
			this.viewer.setWaitCursor();
		repaint();
	}

	/**
	 *  Sets the show child references state
	 */
	public void setChildShow(boolean selected) 
	{
		this.displayChildRefs = selected;
		if (this.node != null)
			this.viewer.setWaitCursor();
		repaint();
	}


	// private methods

	/**
	 * Updates the scrollable area of the viewer
	 */
	private void update()
	{
		drawingPane.setPreferredSize(drawingSize);
		drawingPane.revalidate();
		drawingPane.repaint();
	}


	/**
	 * Creates status message of displayed node.
	 */
	private void setNodeText()
	{
		nodeText = "\n\n Cursor Position: ";
		if (node != null)
		{
			nodeText = " NodeMBR: " + node.getMbr().toString() + "\n";
			nodeText += " Projection (x"+projectionParams.getProjectionDimX()
					+",x"+projectionParams.getProjectionDimY()+"): ";
			nodeText += node.getMbr().projecttoString(projectionParams.getProjectionDimX(), 
								  projectionParams.getProjectionDimY());
			nodeText += "\n Cursor Position: ";
		}
	}

	/**
	 * Creates the status message of cursor position
	 */
	private void setPointerPositionStatusMessage(java.awt.Point p)
	{
		double x, y;
		String posText = "";
		DecimalFormat dcFormat = new DecimalFormat("#.###");
		
		if (this.isShowing())
		{
			java.awt.Point location = this.getLocationOnScreen();
		
			if (p.x < location.x + this.projectionParams.getPadding() || 
					p.x > location.x + this.getWidth() - this.projectionParams.getPadding() ||
					p.y < location.y + this.projectionParams.getPadding() 
					+ this.projectionParams.getExtraPaddingTop() || 
					p.y > location.y + this.getHeight() - this.projectionParams.getPadding() 
					- this.projectionParams.getExtraPaddingBottom())
			{
				posText = " - / -";
			}
			else
			{
				// apply scaling and offsets
				x = ((p.x - location.x - this.projectionParams.getPadding()) 
						/ (this.projectionParams.getScaleFactor())) 
						- this.projectionParams.getOffsetX();
				y = ((p.y - location.y - this.projectionParams.getPadding() 
						- this.projectionParams.getExtraPaddingTop()) 
						/ (this.projectionParams.getScaleFactor())) 
						- this.projectionParams.getOffsetY();

				posText = " " + dcFormat.format(x) + " / " + dcFormat.format(y);
			}
			posText = nodeText + posText;
			this.statusMessage.setText(posText);
		}
	}


	/**
	 * Draws a rectangle with the given colors. 
	 * @param r Rectangle to draw
	 * @param g Graphic context
	 */
	private void drawRect(Rectangle2D.Double r, Graphics g, 
							Color borderColor, Color fillColor)
	{	
		g.setColor(fillColor);
		g.fillRect((int)r.x + this.projectionParams.getPadding(), 
				(int)r.y + this.projectionParams.getPadding() + this.projectionParams.getExtraPaddingTop(), 
				(int)r.width, (int)r.height);
		g.setColor(borderColor);
		g.drawRect((int)r.x + this.projectionParams.getPadding(), 
				(int)r.y + this.projectionParams.getPadding() + this.projectionParams.getExtraPaddingTop(), 
				(int)r.width, (int)r.height);
	}
	
	/**
	 * Draws a rectangle with the given colors. 
	 * @param r Rectangle to draw
	 * @param g Graphic context
	 */
	private void drawRect(Rectangle2D.Double r, Graphics g, 
							Color fillColor)
	{	
		g.setColor(fillColor);
		g.fillRect((int)r.x + this.projectionParams.getPadding(), 
				(int)r.y + this.projectionParams.getPadding() + this.projectionParams.getExtraPaddingTop(), 
				(int)r.width, (int)r.height);
	}
	
	/**
	 * Calculates x-offset and y-offset from the list of rectangles.
	 */
	private void calculateOffsets()
	{
		double minX, minY;
		
		// set offsets according to first rectangle
		minX = rootMBR.getX();
		minY = rootMBR.getY();

		this.projectionParams.setOffsetX(-minX);
		this.projectionParams.setOffsetY(-minY);
	}
	
	/**
	 * Calculates the scale factor from the list of rectangles.
	 */
	private void calculateScaleFactor()
	{
		double windowWidth, windowHeight, maxX, maxY;
		double scaleFactorX, scaleFactorY;
		
		windowWidth = scroller.getWidth() - 20 - 2 * this.projectionParams.getPadding();
		windowHeight = scroller.getHeight() - 20 - 2 * this.projectionParams.getPadding() 
						- this.projectionParams.getExtraPaddingTop()
						- this.projectionParams.getExtraPaddingBottom();

		maxX = rootMBR.getWidth();
		maxY = rootMBR.getHeight();

		scaleFactorX = windowWidth / maxX;
		scaleFactorY = windowHeight / maxY;

		this.projectionParams.setScaleFactor(Math.min(scaleFactorX, scaleFactorY));
	}
	
	/**
	 * Applies offsets to all rectangles in rects list.
	 */
	private void applyOffsets()
	{
		for ( Rectangle2D.Double rect : rects)
		{
			rect.x = rect.x + this.projectionParams.getOffsetX(); 
			rect.y = rect.y + this.projectionParams.getOffsetY(); 
		}
	}
	
	/**
	 * Applies scale factor to all rectangles in rects list.
	 */
	private void applyScaleFactor()
	{
		for ( Rectangle2D.Double rect : rects)
		{
			rect.x = rect.x * this.projectionParams.getScaleFactor(); 
			rect.y = rect.y * this.projectionParams.getScaleFactor();  
			rect.width = rect.width * this.projectionParams.getScaleFactor();  
			rect.height = rect.height * this.projectionParams.getScaleFactor();  
		}
	}
	
	/**
	 * Finds out the child rectangle which has been clicked onto. 
	 * @param x X-Position
	 * @param y Y-Position
	 * @returns Selected child rectangles' node or null if clicked anywhere else 
	 */
	private Node getSelectedNode(int x, int y)
	{
		if (node.isLeafNode())
		{
			return null;
		}

		Node retValue = null;
		Vector<Node> candidates = new Vector<Node>();
		int selectedIndex = -1;

		// collect all candidate rectangles 
		if (rects.size() > 0)
		{
			// iterate through all rectangles
			for (int i = 1; i < rects.size(); i++)
			{
				if (rects.get(i).contains((double)(x - this.projectionParams.getPadding()), 
						(double)(y - this.projectionParams.getPadding() 
						- this.projectionParams.getExtraPaddingTop())))
				{
					candidates.add(childNodes.elementAt(i - 1));
					if (this.selectedNode != null && 
							this.selectedNode.getNodeId() == childNodes.elementAt(i - 1).getNodeId())
					{
						selectedIndex = candidates.size() - 1;
					}
				}
			}
			
			// check current node
			if (rects.get(0).contains((double)(x - this.projectionParams.getPadding()),
					(double)(y - this.projectionParams.getPadding() 
					- this.projectionParams.getExtraPaddingTop())))
			{
				candidates.add(this.node);
				if (this.selectedNode != null && 
						this.selectedNode.getNodeId() == this.node.getNodeId())
				{
					selectedIndex = candidates.size() - 1;
				}
			}
			
			if (selectedIndex > -1)
			{
				// one of the candidates was selected before, choose successor
				if (selectedIndex == candidates.size() - 1)
				{
					retValue = candidates.elementAt(0);
				}
				else
				{
					retValue = candidates.elementAt(selectedIndex + 1);
				}
			}
			else
			{
				// choose first candidate
				if (candidates.size() > 0)
				{
					retValue = candidates.elementAt(0);
				}
			}
		}
		
		return retValue;
	}
	

	/**
	 * Finds out the tuple bounding box which has been clicked onto. 
	 * @param x X-Position
	 * @param y Y-Position
	 * @returns selected bounding box of tuple or null if clicked anywhere else 
	 */
	private Tuple getSelectedTuple(int x, int y)
	{
		if (!displayTupleBBs)
			return null;

		double xPos = (double)(x - this.projectionParams.getPadding());
		double yPos = (double)(y - this.projectionParams.getPadding() - this.projectionParams.getExtraPaddingTop());
		Tuple retValue = null;
		int selectedIndex = -1;
		int startTuple = 0;

		if ((selectedTuple!=null)&&(tupleBBs.get(selectedTupleNo).contains(xPos, yPos)))
			startTuple = selectedTupleNo+1;

		int listSize = tupleBBs.size();
		for (int i=startTuple; i<2*listSize; i++)
		{
			if (tupleBBs.get(i%listSize).contains(xPos, yPos))
			{
				selectedIndex = (i%listSize);
				break;
			}
		}

		if (selectedIndex>-1)
			retValue = tupleList.get(selectedIndex);

		return retValue;
	}
	
	/**
	 * Finds out the father which has been clicked onto. 
	 * @param x X-Position
	 * @param y Y-Position
	 * @returns selected father node or null if clicked anywhere else 
	 */
	private Node getSelectedFatherNode(int x, int y)
	{
		if (fatherNodeBBs.size()<1)
			return null;

		double xPos = (double)(x - this.projectionParams.getPadding());
		double yPos = (double)(y - this.projectionParams.getPadding() - this.projectionParams.getExtraPaddingTop());
		Node retValue = null;
		int selectedIndex = -1;

		for (int i=fatherNodeBBs.size()-1; i>-1; i--)
		{
			if (fatherNodeBBs.get(i).contains(xPos, yPos))
			{
				selectedIndex = i;
				break;
			}
		}

		// check if click is inside selected node...
		if (rects.get(0).contains(xPos, yPos))
			selectedIndex = -1;

		if (selectedIndex>-1)
			retValue = fatherNodes.get(selectedIndex);

		return retValue;
	}
	
	/**
	 * Notifies all registered selection monitors that 
	 * a new node has been focussed. 
	 * @param node Focussed node
	 */
	private void notifyMonitorsNodeFocussed(Node node)
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
	 * a new node has been selected. 
	 * @param node Selected node
	 */
	private void notifyMonitorsNodeSelected(Node node)
	{
		for (NodeDisplay monitor : selectionMonitors )
		{
			monitor.selectNode(node);
		}
	}
	
	/**
	 * Clears a currently loaded rtree.
	 */
	private void unloadRTree() 
	{
		this.treeName = "";
		this.noOfDimensions = 0;
		this.node = null;
		this.childNodes = null;
		this.selectedNode = null;
		this.selectedTuple = null;
		this.rects.clear();
		this.nodeIds.clear();
		this.tupleList.clear();
		this.tupleBBs.clear();
		this.items.clear();
		rootMBR=null;
		fatherNodes.clear();
		setNodeText();
		drawingSize.setSize(0, 0);
		update();
	}
}
