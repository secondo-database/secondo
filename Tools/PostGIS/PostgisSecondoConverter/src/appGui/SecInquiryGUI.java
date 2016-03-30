package appGui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.io.File;
import java.util.logging.Logger;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextPane;
import secondo.ConnectSecondo;
import secondo.ISECTextMessages;
import secondo.MyInquiryViewer;
import secondo.MySecondoObject;
import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;
import appGuiUtil.Message;

public class SecInquiryGUI
  implements IGlobalParameters, ISECTextMessages
{
  public JFrame mTableFrame;
  JPanel panelMainFrame;
  JScrollPane jscrollpane;
  JTextArea jTextAreaHelp;
  File fptmp;
  
  public SecInquiryGUI()
  {
    this.mTableFrame = new JFrame();
    this.panelMainFrame = new JPanel(new BorderLayout(5, 5));
  }
  
  public boolean init(ConnectSecondo conSecondo, StringBuffer sbCMD)
  {
    if (!conSecondo.isSecondoConnected()) {
      conSecondo.connect();
    }
    if (conSecondo.isSecondoConnected())
    {
      if (conSecondo.sendCommand(sbCMD))
      {
        MySecondoObject secObj = new MySecondoObject("", conSecondo.getResultList());
        MyInquiryViewer inqViewer = new MyInquiryViewer();
        
        init(inqViewer.getHTMLCode(secObj));
        
        conSecondo.closeConnection();
        
        return true;
      }
    }
    else {
      new Message("Can not connect to SECONDO database.\nPlease checkconnection parameters.");
    }
    return false;
  }
  
  public void init(StringBuffer sbHTML)
  {
    LogFileHandler.mlogger.info("try to show inquiry gui");
    
    JTextPane textpane = new JTextPane();
    textpane.setContentType("text/html");
    textpane.setEditable(false);
    

    textpane.setText(sbHTML.toString());
    
    this.jscrollpane = new JScrollPane(textpane);
    this.mTableFrame.setTitle("SECONDO Inquiry-Viewer");
    
    this.panelMainFrame.add(new JLabel(), "First");
    this.panelMainFrame.add(new JLabel(), "West");
    this.panelMainFrame.add(new JLabel(), "East");
    this.panelMainFrame.add(new JLabel(), "Last");
    this.panelMainFrame.add(this.jscrollpane, "Center");
    
    this.panelMainFrame.setBackground(Color.blue);
    this.mTableFrame.add(this.panelMainFrame);
    
    this.mTableFrame.pack();
    this.mTableFrame.setIconImage(gimp_S2P);
    this.mTableFrame.setVisible(true);
  }
}
