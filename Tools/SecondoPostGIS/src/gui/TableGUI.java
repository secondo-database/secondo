/**
 * 
 */
package gui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.util.ArrayList;
import java.util.Vector;

import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.tree.DefaultMutableTreeNode;

/**
 * @author Bill
 *
 */
public class TableGUI {

	public JFrame mTableFrame;
	JPanel panelMainFrame;
	JScrollPane jscrollpaneTable;
	JTable jTable;
	Component mcmainframe;
	
	public TableGUI()
	{
		mTableFrame = new JFrame();
		panelMainFrame = new JPanel(new BorderLayout(5,5));
		
		
		//graphComponent = new mxGraphComponent(null);
	}
	
	public TableGUI(Component c)
	{
		mTableFrame = new JFrame();
		panelMainFrame = new JPanel(new BorderLayout());
	
		mTableFrame.setLocationRelativeTo(c);
		mcmainframe = c;
		//graphComponent = new mxGraphComponent(null);
	}
	
	public void init(Vector<Vector> rowData, Vector<String> vcolumnnames,String strFrameTitle)
	{
				
		jTable = new JTable(rowData,vcolumnnames){public boolean isCellEditable(int x, int y) { return false;}};
		
		jTable.setGridColor(Color.blue);
		jTable.setBackground(Color.yellow);
		
		jTable.setShowHorizontalLines(true);
		jTable.setShowVerticalLines(true);
		jTable.setShowGrid(true);
		
		jTable.setAutoResizeMode(JTable.AUTO_RESIZE_ALL_COLUMNS);
		
		
		jscrollpaneTable = new JScrollPane(jTable);
		
		
		
		mTableFrame.setTitle(strFrameTitle);
		
		panelMainFrame.add( new JLabel(), BorderLayout.PAGE_START );
		panelMainFrame.add( new JLabel(), BorderLayout.WEST);
		panelMainFrame.add( new JLabel(), BorderLayout.EAST);
		panelMainFrame.add( new JLabel(), BorderLayout.PAGE_END);
		panelMainFrame.add(jscrollpaneTable,BorderLayout.CENTER);
		
		panelMainFrame.setBackground(Color.blue);
		
		
		mTableFrame.add(panelMainFrame);
		
		
		//mTableFrame.setSize(600, 600);
		mTableFrame.pack();
		
		
		if(mcmainframe == null)
		{
			Dimension d = mTableFrame.getToolkit().getScreenSize(); 
			 
			this.mTableFrame.setLocation((int) ((d.getWidth() - this.mTableFrame.getWidth()) / 2), (int) ((d.getHeight() - 
					this.mTableFrame.getHeight()) / 2));
		}
		else
			mTableFrame.setLocationRelativeTo(mcmainframe);
		
		mTableFrame.setVisible(true);
		
		
	}
	
	
	
	
	public static void main(String[] args) {
		TableGUI tablegui = new TableGUI();
		
		//tablegui.init();
	}
	
	
	
	
	
	
}
