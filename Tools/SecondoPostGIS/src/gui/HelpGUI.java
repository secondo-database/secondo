package gui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.io.FileNotFoundException;
import java.io.IOException;



import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextPane;

public class HelpGUI {

	public JFrame mTableFrame;
	JPanel panelMainFrame;
	JScrollPane jscrollpane;
	JTextArea jTextAreaHelp;
	
	public HelpGUI()
	{
		mTableFrame = new JFrame();
		panelMainFrame = new JPanel(new BorderLayout(5,5));
	}
	
	public void init()
	{
				
		
		
		
		JTextPane textpane = new JTextPane();
		textpane.setContentType("text/html");
		textpane.setEditable(false);
		
		
//		textpane.setText("<html><div style=\"color:blue\"><h1>SecondoPostGIS</h1></div>" +
	//					"<br><b>Daniel Walther</b></html>");
		
		
		
		
		try {
			//textpane.read(new FileReader("HelpHTML"), null);
			textpane.read(HelpGUI.class.getResourceAsStream("/HelpHTML.txt"), null);
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		jscrollpane = new JScrollPane(textpane);
		
		
		
		mTableFrame.setTitle("Help View");
		
		panelMainFrame.add( new JLabel(), BorderLayout.PAGE_START );
		panelMainFrame.add( new JLabel(), BorderLayout.WEST);
		panelMainFrame.add( new JLabel(), BorderLayout.EAST);
		panelMainFrame.add( new JLabel(), BorderLayout.PAGE_END);
		panelMainFrame.add(jscrollpane,BorderLayout.CENTER);
		
		panelMainFrame.setBackground(Color.blue);
		
		
		mTableFrame.add(panelMainFrame);
		
		
		//mTableFrame.setSize(600, 600);
		mTableFrame.pack();
		
		
		
		mTableFrame.setVisible(true);
		
		
	}
	
	
	
	
	public static void main(String[] args) {
		HelpGUI Helpgui = new HelpGUI();
		
		Helpgui.init();
	}
	
	
}
