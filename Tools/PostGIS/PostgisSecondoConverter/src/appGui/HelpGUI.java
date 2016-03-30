package appGui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.logging.Logger;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextPane;
import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;

public class HelpGUI
  implements IGlobalParameters
{
  public JFrame mTableFrame;
  JPanel panelMainFrame;
  JScrollPane jscrollpane;
  JTextArea jTextAreaHelp;
  JTextPane textpane;
  
  public HelpGUI()
  {
    this.mTableFrame = new JFrame();
    this.panelMainFrame = new JPanel(new BorderLayout(5, 5));
    this.panelMainFrame.setBackground(Color.cyan); //.blue);
    this.textpane = new JTextPane();
  }
  
  public void init()
  {
    LogFileHandler.mlogger.info("show HelpGUI");
    
    this.textpane.setContentType("text/html");
    this.textpane.setEditable(false);
    try
    {
      this.textpane.read(HelpGUI.class.getResourceAsStream("/HelpHTML.txt"), null);
    }
    catch (FileNotFoundException e)
    {
      e.printStackTrace();
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
    this.jscrollpane = new JScrollPane(this.textpane);
    
    this.mTableFrame.setTitle("About View");
    
    this.panelMainFrame.add(new JLabel(), "First");
    this.panelMainFrame.add(new JLabel(), "West");
    this.panelMainFrame.add(new JLabel(), "East");
    this.panelMainFrame.add(new JLabel(), "Last");
    this.panelMainFrame.add(this.jscrollpane, "Center");
    
    this.mTableFrame.add(this.panelMainFrame);
    

    this.mTableFrame.pack();
    this.mTableFrame.setIconImage(gimp_S2P);
    this.mTableFrame.setVisible(true);
  }
}
