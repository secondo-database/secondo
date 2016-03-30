package appGui;


import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.util.logging.Logger;
import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JPasswordField;
import javax.swing.JTextField;
import javax.swing.text.AttributeSet;
import javax.swing.text.BadLocationException;
import javax.swing.text.PlainDocument;
import secondoPostgisUtil.Configuration;
import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;

public class ParametersGUI
  implements IGlobalParameters
{
  public JFrame mjframePrameter;
  JPanel jpanelMainFrame;
  JPanel jpanelPostgres;
  JPanel jpanelSecondo;
  JLabel jlabelHost;
  JLabel jLabelPort;
  JLabel jLabelUser;
  JLabel jLabelPWD;
  JTextField jtextPGHost;
  JTextField jtextPGPort;
  JTextField jtextPGUser;
  JPasswordField jtextPGPWD;
  JTextField jtextSECHost;
  JTextField jtextSECPort;
  JTextField jtextSECUser;
  JPasswordField jtextSECPWD;
  JComboBox jcomboBinaryList;
  JLabel jLabelUseBinList;
  JPanel jPanelGemeinsam;
  JPanel jpanelButtons;
  JButton jButtonOK;
  JButton jButtonCancel;
  
  public ParametersGUI()
  {
    this.jpanelMainFrame = new JPanel(new BorderLayout(5, 5));
    
    this.mjframePrameter = new JFrame("Parameter Settings");
   // this.mjframePrameter.setSize(100, 5);
    
    this.jPanelGemeinsam = new JPanel(new BorderLayout(5, 5));
    this.jpanelButtons = new JPanel(new GridLayout(0, 2, 5, 5));
    
    init();
  }
  
  private void init()
  {
    LogFileHandler.mlogger.info("try to show parameter gui");
    
    this.jButtonOK = new JButton("OK");
    this.jButtonCancel = new JButton("Cancel");
    this.jpanelButtons.add(this.jButtonOK);
    this.jpanelButtons.add(this.jButtonCancel);
    
    this.jButtonOK.addActionListener(this.alButtons);
    this.jButtonCancel.addActionListener(this.alButtons);
    
    this.jlabelHost = new JLabel("Host:");
    this.jLabelPort = new JLabel("Port:");
    this.jLabelUser = new JLabel("Username:");
    this.jLabelPWD = new JLabel("Password:");
    this.jLabelUseBinList = new JLabel("Use Binarylist:");
    

    this.jtextPGHost = new JTextField();
    this.jtextPGPort = new JTextField();
    this.jtextPGUser = new JTextField();
    this.jtextPGPWD = new JPasswordField();
    
    this.jtextSECHost = new JTextField();
    this.jtextSECPort = new JTextField();
    this.jtextSECUser = new JTextField();
    this.jtextSECPWD = new JPasswordField();
    

    this.jtextSECPort.setDocument(new PlainDocument()
    {
      private static final long serialVersionUID = 1L;
      
      public void insertString(int offset, String str, AttributeSet as)
      {
        str = str.replaceAll("[^0-9]", "");
        try
        {
          super.insertString(offset, str, as);
        }
        catch (BadLocationException e)
        {
          e.printStackTrace();
        }
      }
    });
    this.jtextPGPort.setDocument(new PlainDocument()
    {
      private static final long serialVersionUID = 1L;
      
      public void insertString(int offset, String str, AttributeSet as)
      {
        str = str.replaceAll("[^0-9]", "");
        try
        {
          super.insertString(offset, str, as);
        }
        catch (BadLocationException e)
        {
          e.printStackTrace();
        }
      }
    });
    this.jtextPGPWD.setEchoChar('#');
    this.jtextPGPWD.addKeyListener(new KeyListener()
    {
      public void keyTyped(KeyEvent arg0) {}
      
      public void keyReleased(KeyEvent arg0)
      {
        StringBuffer sbtmp = new StringBuffer();
        sbtmp.append(ParametersGUI.this.jtextPGPWD.getPassword());
        ParametersGUI.this.jtextPGPWD.setToolTipText(sbtmp.toString());
      }
      
      public void keyPressed(KeyEvent arg0) {}
    });
    this.jtextSECPWD.setEchoChar('#');
    this.jtextSECPWD.addKeyListener(new KeyListener()
    {
      public void keyTyped(KeyEvent arg0) {}
      
      public void keyReleased(KeyEvent arg0)
      {
        StringBuffer sbtmp = new StringBuffer();
        sbtmp.append(ParametersGUI.this.jtextSECPWD.getPassword());
        ParametersGUI.this.jtextSECPWD.setToolTipText(sbtmp.toString());
      }
      
      public void keyPressed(KeyEvent arg0) {}
    });
    Object[] obj = new Object[2];
    obj[0] = "true";
    obj[1] = "false";
    this.jcomboBinaryList = new JComboBox(obj);
    


    this.jpanelSecondo = new JPanel(new GridLayout(0, 2,75,5)); //(0, 2, 5, 5));
    this.jpanelSecondo.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "SECONDO parameters ", 1, 2));
    
    this.jpanelSecondo.add(this.jlabelHost);
    this.jpanelSecondo.add(this.jtextSECHost);
    this.jpanelSecondo.add(this.jLabelPort);
    this.jpanelSecondo.add(this.jtextSECPort);
    this.jpanelSecondo.add(this.jLabelUser);
    this.jpanelSecondo.add(this.jtextSECUser);
    this.jpanelSecondo.add(this.jLabelPWD);
    this.jpanelSecondo.add(this.jtextSECPWD);
    this.jpanelSecondo.add(this.jLabelUseBinList);
    this.jpanelSecondo.add(this.jcomboBinaryList);
    
    this.jpanelPostgres = new JPanel(new GridLayout(0, 2,75,5)); //(0, 2, 5, 5));
    this.jpanelPostgres.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "PostgreSQL parameters ", 1, 2));
    
    this.jlabelHost = new JLabel("Host:");
    this.jLabelPort = new JLabel("Port:");
    this.jLabelUser = new JLabel("Username:");
    this.jLabelPWD = new JLabel("Password:");
    this.jpanelPostgres.add(this.jlabelHost);
    this.jpanelPostgres.add(this.jtextPGHost);
    this.jpanelPostgres.add(this.jLabelPort);
    this.jpanelPostgres.add(this.jtextPGPort);
    this.jpanelPostgres.add(this.jLabelUser);
    this.jpanelPostgres.add(this.jtextPGUser);
    this.jpanelPostgres.add(this.jLabelPWD);
    this.jpanelPostgres.add(this.jtextPGPWD);
    
    this.jPanelGemeinsam.add(this.jpanelSecondo, "North");
    this.jPanelGemeinsam.add(this.jpanelPostgres, "South");
    
    this.jpanelMainFrame.add(this.jPanelGemeinsam, "First");
    this.jpanelMainFrame.add(this.jpanelButtons, "Last");
    

    this.mjframePrameter.add(this.jpanelMainFrame);
  }
  
  public void initView(String _strSECHost, String _strSecPort, String _strSecUser, String _strSecPWD, String _strSECUseBinList, String _strPGHost, String _strPGPort, String _strPGUser, String _strPGPWD)
  {
    this.jtextSECHost.setText(_strSECHost);
    this.jtextSECPort.setText(_strSecPort);
    this.jtextSECUser.setText(_strSecUser);
    this.jtextSECPWD.setText(_strSecPWD);
    this.jtextSECPWD.setToolTipText(_strSecPWD);
    if (Boolean.valueOf(_strSECUseBinList).booleanValue()) {
      this.jcomboBinaryList.setSelectedIndex(0);
    } else {
      this.jcomboBinaryList.setSelectedIndex(1);
    }
    this.jtextPGHost.setText(_strPGHost);
    this.jtextPGPort.setText(_strPGPort);
    this.jtextPGUser.setText(_strPGUser);
    this.jtextPGPWD.setText(_strPGPWD);
    this.jtextPGPWD.setToolTipText(_strPGPWD);
    

    this.mjframePrameter.setIconImage(gimp_S2P);
    this.mjframePrameter.pack();
    this.mjframePrameter.setVisible(true);
  }
  
  private void pressedOK()
  {
    Configuration config = new Configuration();
    

    gsbPG_Host.delete(0, gsbPG_Host.length());
    gsbPG_Port.delete(0, gsbPG_Port.length());
    gsbPG_User.delete(0, gsbPG_User.length());
    gsbPG_Pwd.delete(0, gsbPG_Pwd.length());
    
    gsbSEC_Host.delete(0, gsbSEC_Host.length());
    gsbSEC_Port.delete(0, gsbSEC_Port.length());
    gsbSEC_User.delete(0, gsbSEC_User.length());
    gsbSEC_Pwd.delete(0, gsbSEC_Pwd.length());
    gsbSEC_UseBinaryList.delete(0, gsbSEC_UseBinaryList.length());
    

    gsbPG_Host.append(this.jtextPGHost.getText());
    gsbPG_Port.append(this.jtextPGPort.getText());
    gsbPG_User.append(this.jtextPGUser.getText());
    gsbPG_Pwd.append(this.jtextPGPWD.getPassword());
    


    gsbSEC_Host.append(this.jtextSECHost.getText());
    gsbSEC_Port.append(this.jtextSECPort.getText());
    gsbSEC_User.append(this.jtextSECUser.getText());
    gsbSEC_Pwd.append(this.jtextSECPWD.getPassword());
    gsbSEC_UseBinaryList.append(this.jcomboBinaryList.getSelectedItem().toString());
    
    config.write();
    
    this.mjframePrameter.setVisible(false);
  }
  
  ActionListener alButtons = new ActionListener()
  {
    public void actionPerformed(ActionEvent e)
    {
      if (e.getSource() == ParametersGUI.this.jButtonOK)
      {
        LogFileHandler.mlogger.info("pressed ok button");
        ParametersGUI.this.pressedOK();
      }
      else if (e.getSource() == ParametersGUI.this.jButtonCancel)
      {
        ParametersGUI.this.mjframePrameter.setVisible(false);
      }
    }
  };
}
