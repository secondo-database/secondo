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
import postgis.DatabaseName;
import secondo.ISECTextMessages;
import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;
import appGuiUtil.IPublicTextMessages;
import appGuiUtil.Message;
import appGuiUtil.SetUI;

public class AskForPGDBAndTemplateName
  implements ISECTextMessages, IGlobalParameters, IPublicTextMessages
{
  private String strPostgresDBName;
  private boolean bPressedOK;
  private String strTemplateExistingDB;
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
  JComboBox jComboLinksDB;
  JTextField jtextDBName;
  public final String strNOTEMPLATE = "no template";
  public final String strTEMPLATE0 = "template0";
  public final int iCANCEL = 2;
  public final int iNEU = 1;
  public final int iBESTEHENDE = 0;
  Pattern pSecDBName = Pattern.compile("[A-Z]{1}[A-Z0-9_]{0,14}");
  
  public AskForPGDBAndTemplateName()
  {
    this.strPostgresDBName = "PGStandardName".toLowerCase();
    


    this.jpanelMain = new JPanel(new BorderLayout(5, 5));
    this.jMainFrame = new JFrame("Choose database name");
    this.jpanelRadioButtons = new JPanel(new GridLayout(0, 2, 5, 5));
    this.jpanelCommandButtons = new JPanel(new GridLayout(0, 2, 5, 5));
    

    this.jpanelRight = new JPanel(new GridLayout(0, 2, 5, 5));
    this.jpanelLeft = new JPanel(new GridLayout(0, 2, 5, 5));
    
    this.jpanelRight.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "existing database", 1, 2));
    this.jpanelLeft.setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.BLACK), "new database", 1, 2));
    
    this.jpanelLeft.setToolTipText("All letters must be write lowercase.");
    this.jpanelRight.setToolTipText("");
  }
  
  public void init(LinkedList<DatabaseName> _llDatenbanken, LinkedList<DatabaseName> _llTemplate, String _strDBName)
  {
    String[] strArrDatabases = new String[_llDatenbanken.size()];
    String[] strArrDatabasesTemplate = new String[_llDatenbanken.size() + _llTemplate.size() + 1];
    
    strArrDatabasesTemplate[0] = "no template";
    int iSizeBefore = 0;
    for (int i = 0; i < _llDatenbanken.size(); i++)
    {
      strArrDatabases[i] = ((DatabaseName)_llDatenbanken.get(i)).getSbName().toString();
      strArrDatabasesTemplate[(i + 1)] = ((DatabaseName)_llDatenbanken.get(i)).getSbName().toString();
      iSizeBefore++;
    }
    for (int i = 0; i < _llTemplate.size(); i++)
    {
      iSizeBefore++;
      strArrDatabasesTemplate[iSizeBefore] = ((DatabaseName)_llTemplate.get(i)).getSbName().toString();
    }
    this.bPressedOK = false;
    this.strTemplateExistingDB = "no template";
    this.iNeueDB = 2;
    
    this.jComboRechtsDBs = new JComboBox(strArrDatabases);
    

    this.jComboLinksDB = new JComboBox(strArrDatabasesTemplate);
    this.jtextDBName = new JTextField(_strDBName.toLowerCase());
    this.jtextDBName.setToolTipText("All letters must be write lowercase.");
    
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
    this.jpanelRight.add(new JLabel(""));
    this.jpanelRight.add(this.jComboRechtsDBs);
    
    this.jpanelLeft.add(new JLabel("Enter new database name:"));
    this.jpanelLeft.add(this.jtextDBName);
    this.jpanelLeft.add(new JLabel("Choose an template"));
    this.jpanelLeft.add(this.jComboLinksDB);
    

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
  
  public String getTemplateName()
  {
    return this.strTemplateExistingDB;
  }
  
  public String getDBName()
  {
    return this.strPostgresDBName;
  }
  
  private void setNeueElementeEnable(boolean b)
  {
    this.jComboLinksDB.setEnabled(b);
    this.jtextDBName.setEnabled(b);
    this.jComboRechtsDBs.setEnabled(!b);
  }
  /*
   * This function verify whether the selected database already exist
   * If true the user will be asked whether he wants to overwrite the database or not
   */
  private void pressOK()
  {
    this.bPressedOK = true;
    if (this.jRadioNeueDB.isSelected())
    {
      this.strTemplateExistingDB = this.jComboLinksDB.getSelectedItem().toString();
      this.strPostgresDBName = this.jtextDBName.getText();
      if (this.strPostgresDBName.length() > 0)
      {
        this.strPostgresDBName = this.strPostgresDBName.toLowerCase();
        if (this.strPostgresDBName.intern() == "template0".intern())
        {
          new Message("Databasename template0 is not allowed.");
          return;
        }
        if (this.strPostgresDBName.intern() == this.jComboLinksDB.getSelectedItem().toString().intern())
        {
          new Message("Databasename and templatename are equal. That is not allowed.");
          return;
        }
        for (int i = 0; i < this.jComboRechtsDBs.getItemCount(); i++) {
          if (this.strPostgresDBName.intern() == this.jComboRechtsDBs.getItemAt(i).toString().intern())
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
      this.strPostgresDBName = this.jComboRechtsDBs.getSelectedItem().toString();
      this.mjdi.setVisible(false);
    }
  }
  
  public boolean checkDBName(String _strDBName)
  {
    Matcher m = this.pSecDBName.matcher(_strDBName);
    if (!m.matches())
    {
      LogFileHandler.mlogger.warning("db name is not correct");
      
      new Message("Wrong database name.");
      return false;
    }
    return true;
  }
  
  /*
   * This Actionlistener handels the action selected in the
   * window in which either an available database name or a new one is entered 
   */
  ActionListener al = new ActionListener()
  {
    public void actionPerformed(ActionEvent e)
    {
      if (e.getSource() == AskForPGDBAndTemplateName.this.jRadioNeueDB)
      {
        AskForPGDBAndTemplateName.this.setNeueElementeEnable(true);
      }
      else if (e.getSource() == AskForPGDBAndTemplateName.this.jRadioBestehendeDB)
      {
        AskForPGDBAndTemplateName.this.setNeueElementeEnable(false);
      }
      else if (e.getSource() == AskForPGDBAndTemplateName.this.jButOK)
      {
        AskForPGDBAndTemplateName.this.pressOK();
      }
      else if (e.getSource() == AskForPGDBAndTemplateName.this.jButCancle)
      {
        AskForPGDBAndTemplateName.this.iNeueDB = 2;
        AskForPGDBAndTemplateName.this.mjdi.setVisible(false);
      }
    }
  };
}