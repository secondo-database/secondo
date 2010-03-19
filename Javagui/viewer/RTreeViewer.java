package viewer;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

import viewer.MenuVector;
import viewer.SecondoViewer;
import viewer.rtree.*;
import viewer.rtree.datatypes.*;
import gui.SecondoObject;
import viewer.rtree.gui.*;
import sj.lang.ListExpr;

/**
 * Main class of the RTreeViewer.
 * 
 * RTreeViewer mainly consists of an additional menu and three panels: 
 * 
 * NodeTreePanel: Tree view of the tree
 * NodeViewerPanel: Rendering area for the bounding boxes and referenced objects
 * NodeInfoPanel: Detail information on the currently selected tree node
 * 
 * @author Oliver Feuer
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @since 08.03.2010
 * @version 1.3
 */
public class RTreeViewer extends SecondoViewer implements ActionListener, ItemListener 
{
	// the display panels
	private NodeTreePanel nodeTreePanel;
	private NodeInfoPanel nodeInfoPanel;
	private NodeViewerPanel nodeViewerPanel;
	// the menu
	private JMenu xAxis, yAxis, zoom;
	private MenuVector menuVector;
	private JMenuItem xySwitch, rootView, nodeView, zoomIn, zoomOut;
	private JCheckBoxMenuItem tupleBBShow, refShow, childRefShow;
	private LinkedList<JRadioButtonMenuItem> x, y;
	private JMenuItem runscript;

	// constructors
	
	/**
	 * Cretes a new RTreeViewer object.
	 */
	public RTreeViewer() 
	{
		// set layout, create and configure panels
		setLayout(new BorderLayout());
		
		JPanel panel = new JPanel( new BorderLayout() );
		panel.setBorder(BorderFactory.createLineBorder(Color.pink));
		panel.setBackground(Color.blue);
		
		nodeTreePanel = new NodeTreePanel(this);
		nodeViewerPanel = new NodeViewerPanel(this);
		nodeInfoPanel = new NodeInfoPanel();

                JSplitPane splitPaneR = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, 
                                                      nodeViewerPanel, nodeInfoPanel);
                splitPaneR.setOneTouchExpandable(true);
                splitPaneR.setDividerLocation(400);

                JSplitPane splitPaneL = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, 
                                                       nodeTreePanel, splitPaneR);
                splitPaneL.setOneTouchExpandable(true);
                splitPaneL.setDividerLocation(175);

                Dimension minimumSize = new Dimension(50, 50);
                nodeTreePanel.setMinimumSize(minimumSize);
                nodeViewerPanel.setMinimumSize(minimumSize);
                nodeInfoPanel.setMinimumSize(minimumSize);

		panel.add(splitPaneL, BorderLayout.CENTER);

		add(panel, BorderLayout.CENTER);
			
		// ad monitors for inter panel communication
		nodeTreePanel.addSelectionMonitor(nodeViewerPanel);
		nodeTreePanel.addSelectionMonitor(nodeInfoPanel);
		nodeViewerPanel.addSelectionMonitor(nodeInfoPanel);
		nodeViewerPanel.addSelectionMonitor(nodeTreePanel);
		
		// create menu entries and add listeners
		menuVector = new MenuVector();

		JMenu rTreeMenu = new JMenu("RTree");

		xAxis = new JMenu("x-Axis");
		xAxis.setEnabled(false);
		rTreeMenu.add(xAxis);
		x = new LinkedList<JRadioButtonMenuItem>();

		yAxis = new JMenu("y-Axis");
		yAxis.setEnabled(false);
		rTreeMenu.add(yAxis);
		y = new LinkedList<JRadioButtonMenuItem>();

		xySwitch = new JMenuItem("x-y-Switch");
		xySwitch.addActionListener(this);
		xySwitch.setEnabled(false);
		rTreeMenu.add(xySwitch);

		rTreeMenu.addSeparator();

		zoom = new JMenu("Zoom");
		zoom.setEnabled(false);
		rTreeMenu.add(zoom);

		rootView = new JMenuItem("Root View");
		rootView.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R, ActionEvent.ALT_MASK));
		rootView.addActionListener(this);
		rootView.setEnabled(false);
		zoom.add(rootView);

		nodeView = new JMenuItem("Node View");
		nodeView.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N, ActionEvent.ALT_MASK));
		nodeView.addActionListener(this);
		nodeView.setEnabled(false);
		zoom.add(nodeView);

		zoomIn = new JMenuItem("Zoom In");
		zoomIn.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_PLUS, ActionEvent.ALT_MASK));
		zoomIn.addActionListener(this);
		zoomIn.setEnabled(false);
		zoom.add(zoomIn);

		zoomOut = new JMenuItem("Zoom Out");
		zoomOut.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_MINUS, ActionEvent.ALT_MASK));
		zoomOut.addActionListener(this);
		zoomOut.setEnabled(false);
		zoom.add(zoomOut);

		rTreeMenu.addSeparator();

		tupleBBShow = new JCheckBoxMenuItem("Show Key-MBR");
		tupleBBShow.addItemListener(this);
		tupleBBShow.setState(true);
		tupleBBShow.setEnabled(false);
		rTreeMenu.add(tupleBBShow);

		refShow = new JCheckBoxMenuItem("Show references");
		refShow.addItemListener(this);
		refShow.setEnabled(false);
		rTreeMenu.add(refShow);

		childRefShow = new JCheckBoxMenuItem("Show child references");
		childRefShow.addItemListener(this);
		childRefShow.setEnabled(false);
		rTreeMenu.add(childRefShow);

		rTreeMenu.addSeparator();

		JMenu debug = new JMenu("Debug");
		runscript = new JMenuItem("Run Script");
		runscript.addActionListener(this);
		debug.add(runscript);
		rTreeMenu.add(debug);

		menuVector.addMenu(rTreeMenu);
	}

	// public methods

	/**
	 * Handles the actionPerformed event. 
	 * @param e Event detail data
	 */
	public void actionPerformed(ActionEvent e)
	{
		if (e.getSource() == xySwitch)
		{
			nodeViewerPanel.switchAxis();
			int xindex=0;
			for (int i=0; i<x.size(); i++)
				if (x.get(i).isSelected())
				{
					xindex = i;
					break;
				}
			int yindex=0;
			for (int i=0; i<y.size(); i++)
				if (y.get(i).isSelected())
				{
					yindex = i;
					break;
				}
			x.get(yindex).setSelected(true);
			y.get(xindex).setSelected(true);
		}
		if (e.getSource() == rootView)
		{
			nodeViewerPanel.setRootView();
		}
		if (e.getSource() == nodeView)
		{
			nodeViewerPanel.setNodeView();
		}
		if (e.getSource() == zoomIn)
		{
			nodeViewerPanel.setZoomIn();
		}
		if (e.getSource() == zoomOut)
		{
			nodeViewerPanel.setZoomOut();
		}
		if (e.getSource() == runscript)
		{
			runScript();
		}
	}
	
	/**
	 * Handles the ItemEvent. 
	 * @param e Event detail data
	 */
	public void itemStateChanged(ItemEvent e)
	{
		if (e.getItemSelectable() == tupleBBShow)
		{
			nodeViewerPanel.setTupleShow(tupleBBShow.getState());
		}
		else if (e.getItemSelectable() == refShow)
		{
			nodeViewerPanel.setShow(refShow.getState());
		}
		else if (e.getItemSelectable() == childRefShow)
		{
			nodeViewerPanel.setChildShow(childRefShow.getState());
		}
		else if (e.getItemSelectable() == childRefShow)
		{
			nodeViewerPanel.setChildShow(childRefShow.getState());
		}

		else {
			int index = x.indexOf(e.getItemSelectable());
			if (index>-1)
			{
				nodeViewerPanel.setXProjection(index);
			}
			index = y.indexOf(e.getItemSelectable());
			if (index>-1)
			{
				nodeViewerPanel.setYProjection(index);
			}
		}
	}

	/**
	 * Is called by NodeTreePanel if an rtree is set or unset.
	 * @param dim dimension of RTree to set or zero to reset
	 */
	public void setRTreeMenu(int dim)
	{
		xAxis.removeAll();
		yAxis.removeAll();
		xAxis.setEnabled(dim>0);
		yAxis.setEnabled(dim>0);
		xySwitch.setEnabled(dim>0);
		zoom.setEnabled(dim>0);
		rootView.setEnabled(dim>0);
		nodeView.setEnabled(dim>0);
		zoomIn.setEnabled(dim>0);
		zoomOut.setEnabled(dim>0);
		x.clear();
		y.clear();
		tupleBBShow.setEnabled(dim>0);
		if (dim>0)
		{
			JRadioButtonMenuItem rb;
			ButtonGroup xgroup = new ButtonGroup();
			ButtonGroup ygroup = new ButtonGroup();

			for (int i=0; i < dim; i++)
			{
				rb = new JRadioButtonMenuItem("x = x"+i);
				rb.addItemListener(this);
				if (i==0) 
					rb.setSelected(true);
				x.add(rb);
				xAxis.add(rb);
				xgroup.add(rb);

				rb = new JRadioButtonMenuItem("y = x"+i);
				rb.addItemListener(this);
				if (i==1) 
					rb.setSelected(true);
				y.add(rb);
				yAxis.add(rb);
				ygroup.add(rb);
			}
		}
	}
	
	/**
	 * Is called by NodeTreePanel if an attribute is set or unset.
	 * @param enable enables attribute menu items if true, otherwise disables it
	 */
	public void setAttributeMenu(boolean enable)
	{
		refShow.setEnabled(enable);
		childRefShow.setEnabled(enable);
		if (!enable)
		{
			refShow.setState(false);
			childRefShow.setState(false);
		}
	}

	/**
	 * Is called by Secondo to add an object.
	 * Currently not needed, as objects are only added explicitly by the viewer.
	 * @param o A Secondo object
	 */
	public boolean addObject(SecondoObject o) 
	{
		return false;
	}

	/**
	 * Is called by Secondo to check if an object can be displayed.
	 * @param o A Secondo object
	 */
	public boolean canDisplay(SecondoObject o) 
	{	
		return false;
	}

	/**
	 * Is called by Secondo to check if an object is currently displayed.
	 * @param o A Secondo object
	 */
	public boolean isDisplayed(SecondoObject o) 
	{
		return false;
	}

	/**
	 * Is called by Secondo to check how an object can be displayed.
	 * @param o A Secondo object
	 */
	public double getDisplayQuality(SecondoObject so) 
	{
		return 0.0f;
	}

	
	/**
	 * Is called by Secondo to remove all objects.
	 */
	public void removeAll()
	{
	}

	/**
	 * Is called by Secondo to remove an object.
	 * Currently not needed, as objects are only removed explicitly by the viewer.
	 * @param o A Secondo object
	 */
	public void removeObject(SecondoObject o)
	{
	}

	/**
	 * Is called by Secondo to select an object.
	 * @param o A Secondo object
	 */
	public boolean selectObject(SecondoObject O)
	{
		return false;
	}

	/**
	 * Is called by Secondo to display the menu.
	 */
	public MenuVector getMenuVector()
	{
		return menuVector;
	}
	
	/**
	 * Is called by Secondo to get the viewer name.
	 */
	public String getName() {
		return "RTree Viewer";
	}

	/**
	 * Activates test mode.
	 * Currently not implemented. 
	 */
	public void enableTestmode(boolean on)
	{
	}
	
	/**
	 * Sets the cursor to indicate a current activity.
	 */
	public void setWaitCursor()
	{
		Cursor hourglassCursor = new Cursor(Cursor.WAIT_CURSOR);
		setCursor(hourglassCursor);
	}

	/**
	 * Sets the cursor to default.
	 */
	public void setDefaultCursor()
	{
		Cursor normalCursor = new Cursor(Cursor.DEFAULT_CURSOR);
		setCursor(normalCursor);
	}

	// private methods
	
	/**
	 * Enables the user to select and run a script.
	 */
	private void runScript() 
	{
		JFileChooser fileChooser = new JFileChooser();
		fileChooser.showOpenDialog(this);

		// check if file was selected
		if (fileChooser.getSelectedFile() != null)
		{
			String filename = fileChooser.getSelectedFile().getPath();
		
			if (filename != null && filename != "")
			{
				ScriptRunner scriptRunner = new ScriptRunner(this);
				scriptRunner.runScript(filename);
			}
		}
	}
}
