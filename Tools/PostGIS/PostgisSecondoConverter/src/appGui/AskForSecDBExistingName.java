package appGui;


import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.LinkedList;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.swing.BorderFactory;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JTextField;
import secondo.ISECTextMessages;
import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;
import appGuiUtil.IPublicTextMessages;
import appGuiUtil.Message;
import appGuiUtil.SetUI;

public class AskForSecDBExistingName
  implements ISECTextMessages, IGlobalParameters, IPublicTextMessages
{
  private String strSecDBName;
  private boolean bPressedOK;
  private int iNeueDB;
  JDialog mjdi;
  JFrame jMainFrame;
  JPanel jpanelRadioButtons;
  JRadioButton jRadioNeueDB;
  JRadioButton jRadioBestehendeDB;
  ButtonGroup radiobuttonGroup;
  JButton jButOK;
  JButton jButCancle;
  JPanel jpanelCommandButtons;
  JPanel jpanelMain;
  JPanel jpanelLeft;
  JPanel jpanelRight;
  JComboBox jComboRechtsDBs;
  JTextField jtextDBName;
  public final int iCANCEL = 2;
  public final int iNEU = 1;
  public final int iBESTEHENDE = 0;
  Pattern pSecDBName = Pattern.compile("[A-Z]{1}[A-Z0-9_]{0,14}");
  
  public AskForSecDBExistingName()
  {
    this.strSecDBName = "secstandard".toUpperCase();
    
    this.jpanelMain = new JPanel(new BorderLayout(5, 5));
    this.jMainFrame = new JFrame("Choose database name");
    this.jpanelRadioButtons = new JPanel(new GridLayout(0, 2, 5, 5));
    this.jpanelCommandButtons = new JPanel(new GridLayout(0, 2, 5, 5));
    

    this.jpanelRight = new JPanel(new GridLayout(0, 2, 5, 5));
    this.jpanelLeft = new JPanel(new GridLayout(0, 2, 5, 5));
    
    this.jpanelRight.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "existing database", 1, 2));
    this.jpanelLeft.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "new database", 1, 2));
    
    this.jpanelLeft.setToolTipText("All letters must be written lowercase.");
    this.jpanelRight.setToolTipText("");
  }
  
  public void init(LinkedList<String> _llDatenbanken, String _strDBName)
  {
    String[] strArrDatabases = new String[_llDatenbanken.size()];
    for (int i = 0; i < _llDatenbanken.size(); i++) {
      strArrDatabases[i] = ((String)_llDatenbanken.get(i)).toString();
    }
    this.bPressedOK = false;
    
    this.iNeueDB = 2;
    
    this.jComboRechtsDBs = new JComboBox(strArrDatabases);
    

    this.jtextDBName = new JTextField(_strDBName.toUpperCase());
    this.jtextDBName.setToolTipText("All letters must be written lowercase.");
    
    this.jButOK = new JButton("OK");
    this.jButCancle = new JButton("Cancel");
    
    this.jButOK.addActionListener(this.al);
    this.jButCancle.addActionListener(this.al);
    
    this.jRadioNeueDB = new JRadioButton("new database");
    this.jRadioNeueDB.setSelected(true);
    setNeueElementeEnable(true);
    this.jRadioNeueDB.addActionListener(this.al);
    
    this.jRadioBestehendeDB = new JRadioButton("exisiting database");
    this.jRadioBestehendeDB.addActionListener(this.al);
    
    this.radiobuttonGroup = new ButtonGroup();
    this.radiobuttonGroup.add(this.jRadioNeueDB);
    this.radiobuttonGroup.add(this.jRadioBestehendeDB);
    

    this.jpanelRadioButtons.add(this.jRadioNeueDB);
    this.jpanelRadioButtons.add(this.jRadioBestehendeDB);
    

    this.jpanelRight.add(new JLabel("Choose an existing database:"));
    this.jpanelRight.add(this.jComboRechtsDBs);
    
    this.jpanelLeft.add(new JLabel("Enter new database name:"));
    this.jpanelLeft.add(this.jtextDBName);
    

    this.jpanelCommandButtons.add(this.jButOK);
    this.jpanelCommandButtons.add(this.jButCancle);
    
    this.jpanelMain.add(this.jpanelRadioButtons, "First");
    this.jpanelMain.add(this.jpanelLeft, "West");
    this.jpanelMain.add(this.jpanelRight, "East");
    this.jpanelMain.add(this.jpanelCommandButtons, "Last");
    

    this.jMainFrame.add(this.jpanelMain);
    
    this.mjdi = new JDialog(this.jMainFrame, true);
    
    this.mjdi.setIconImage(gimp_S2P);
    this.mjdi.add(this.jpanelMain);
    this.mjdi.setTitle("Choose database name");
    this.mjdi.pack();
    this.mjdi.setVisible(true);
  }
  
  public int getButtonStatus()
  {
    LogFileHandler.mlogger.info("Button-Status: " + Integer.valueOf(this.iNeueDB));
    return this.iNeueDB;
  }
  
  public String getDBName()
  {
    return this.strSecDBName;
  }
  
  private void setNeueElementeEnable(boolean b)
  {
    this.jtextDBName.setEnabled(b);
    this.jComboRechtsDBs.setEnabled(!b);
  }
  
  private void pressOK()
  {
    this.bPressedOK = true;
    if (this.jRadioNeueDB.isSelected())
    {
      this.strSecDBName = this.jtextDBName.getText();
      if (this.strSecDBName.length() > 0)
      {
        this.strSecDBName = this.strSecDBName.toUpperCase();
        
        Matcher m = this.pSecDBName.matcher(this.strSecDBName);
        if (!m.matches())
        {
          LogFileHandler.mlogger.warning("wrong db name");
          new Message("Wrong database name.");
          return;
        }
        for (int i = 0; i < this.jComboRechtsDBs.getItemCount(); i++) {
          if (this.strSecDBName.intern() == this.jComboRechtsDBs.getItemAt(i).toString().intern())
          {
            new SetUI().setUIAndLanguage();
            
            int n = JOptionPane.showConfirmDialog(
              null, 
              "Your database name exists. Do you want to overwrite?", 
              "Question", 
              0);
            if (n == 1) {
              return;
            }
          }
        }
        this.iNeueDB = 1;
        
        this.mjdi.setVisible(false);
      }
      else
      {
        new Message("Please insert a valid database name.");
      }
    }
    else
    {
      this.iNeueDB = 0;
      this.strSecDBName = this.jComboRechtsDBs.getSelectedItem().toString();
      this.mjdi.setVisible(false);
    }
  }
  
  public boolean checkDBName(String _strDBName)
  {
    Matcher m = this.pSecDBName.matcher(this.strSecDBName);
    if (!m.matches())
    {
      LogFileHandler.mlogger.warning("wrong db name");
      new Message("Wrong database name.");
      return false;
    }
    return true;
  }
  
  ActionListener al = new ActionListener()
  {
    public void actionPerformed(ActionEvent e)
    {
      if (e.getSource() == AskForSecDBExistingName.this.jRadioNeueDB)
      {
        AskForSecDBExistingName.this.setNeueElementeEnable(true);
      }
      else if (e.getSource() == AskForSecDBExistingName.this.jRadioBestehendeDB)
      {
        AskForSecDBExistingName.this.setNeueElementeEnable(false);
      }
      else if (e.getSource() == AskForSecDBExistingName.this.jButOK)
      {
        AskForSecDBExistingName.this.pressOK();
      }
      else if (e.getSource() == AskForSecDBExistingName.this.jButCancle)
      {
        AskForSecDBExistingName.this.iNeueDB = 2;
        AskForSecDBExistingName.this.mjdi.setVisible(false);
      }
    }
  };
}
