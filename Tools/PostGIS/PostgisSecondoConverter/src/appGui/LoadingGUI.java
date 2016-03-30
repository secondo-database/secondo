package appGui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.util.logging.Logger;
import javax.swing.ImageIcon;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JWindow;
import secondoPostgisUtil.LogFileHandler;

public class LoadingGUI
{
  private JPanel jpanel;
  private ImageIcon image;
  private JLabel jlabel;
  private JWindow window;
  
  public LoadingGUI()
  {
    this.jpanel = new JPanel(new BorderLayout());
    this.image = new ImageIcon(LoadingGUI.class.getResource("/appLoading.gif"));
    
    this.jlabel = new JLabel(this.image);
    
    this.jpanel.add(this.jlabel, "Center");
    this.jpanel.setBackground(Color.green);
    
    this.window = new JWindow();
    this.window.add(this.jpanel);
    
    this.window.pack();
    
    Dimension d = this.window.getToolkit().getScreenSize();
    
    this.window.setLocation((int)((d.getWidth() - this.window.getWidth()) / 2.0D), 
      (int)((d.getHeight() - this.window.getHeight()) / 2.0D));
    
    this.window.setFocusable(true);
    this.window.setAlwaysOnTop(true);
    this.window.setVisible(true);
    
   LogFileHandler.mlogger.info("show loading gui");
  }
  
  public void closeLoadingWindow()
  {
    LogFileHandler.mlogger.info("close loading gui");
    this.window.setVisible(false);
  }
}
