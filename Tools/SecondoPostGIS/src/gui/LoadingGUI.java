/**
 * 
 */
package gui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;





import javax.swing.ImageIcon;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JWindow;


/**
 * @author Bill
 *
 */
public class LoadingGUI {

	
	private JPanel jpanel;
	private ImageIcon image;
	private JLabel jlabel;
	private JWindow window;
	
	
	
	/**
	 * 
	 */
	public LoadingGUI() {
		super();
	
		this.jpanel = new JPanel(new BorderLayout());
		//InputStream in = Main.class.getResourceAsStream("/Loading.gif");
		this.image = new ImageIcon(LoadingGUI.class.getResource("/Loading.gif"));
		//this.image = new ImageIcon(Main.class.getResource("/Loading.gif"));
		
		this.jlabel = new JLabel(this.image);
		
		this.jpanel.add(this.jlabel,BorderLayout.CENTER);
		this.jpanel.setBackground(Color.green);
		
		this.window = new JWindow();
		this.window.add(this.jpanel);
		
		this.window.pack();
		
		Dimension d = window.getToolkit().getScreenSize(); 
		 
		this.window.setLocation((int) ((d.getWidth() - this.window.getWidth()) / 2), (int) ((d.getHeight() - 
				this.window.getHeight()) / 2));

		this.window.setFocusable(true);
		this.window.setVisible(true);
		
	}


	public void closeLoadindWindow()
	{
		this.window.setVisible(false);
	}
	





	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub

		LoadingGUI loadgui = new LoadingGUI();
		
		
		try {
			Thread.sleep(5000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		loadgui.window.setVisible(false);
		
		
	}

}
