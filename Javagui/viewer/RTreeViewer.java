package viewer;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

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
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @since 19.12.2009
 * @version 1.1
 */
public class RTreeViewer extends SecondoViewer implements ActionListener 
{
	// the display panels
	private NodeTreePanel nodeTreePanel;
	private NodeInfoPanel nodeInfoPanel;
	private NodeViewerPanel nodeViewerPanel;
	// the menu
	private MenuVector menuVector;

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
		panel.add(nodeTreePanel, BorderLayout.LINE_START);
		
		nodeViewerPanel = new NodeViewerPanel(this);
		panel.add(nodeViewerPanel, BorderLayout.CENTER);
		
		nodeInfoPanel = new NodeInfoPanel();
		panel.add(nodeInfoPanel, BorderLayout.LINE_END);
		
		add(panel, BorderLayout.CENTER);
			
		// ad monitors for inter panel communication
		nodeTreePanel.addSelectionMonitor(nodeViewerPanel);
		nodeTreePanel.addSelectionMonitor(nodeInfoPanel);
		nodeViewerPanel.addSelectionMonitor(nodeInfoPanel);
		nodeViewerPanel.addSelectionMonitor(nodeTreePanel);
		
		// create menu entries and add listener
		menuVector = new MenuVector();
		JMenu settings = new JMenu("RTree");
		JMenu debug = new JMenu("Debug");
		debug.add("Run Script").addActionListener(this);
		settings.add(debug);
		menuVector.addMenu(settings);
	}

	// public methods

	/**
	 * Handles the actionPerformed event. 
	 * @param e Event detail data
	 */
	public void actionPerformed(ActionEvent e)
	{
		runScript();
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
