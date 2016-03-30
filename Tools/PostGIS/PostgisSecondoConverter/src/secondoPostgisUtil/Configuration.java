package secondoPostgisUtil;

import java.awt.BorderLayout;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.logging.Logger;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPasswordField;
import appGuiUtil.SetUI;

public class Configuration
  implements IGlobalParameters
{
  private LinkedProperties mprop = null;
  private FileOutputStream mfileOPstream = null;
  private File mfp = null;
  private StringBuffer msbPG_Host;
  private StringBuffer msbPG_Port;
  private StringBuffer msbPG_User;
  private StringBuffer msbPG_Pwd;
  private final String mstrPG_DEFAULT_Host = "127.0.0.1";
  private final String mstrPG_DEFAULT_Port = "5432";
  private final String mstrPG_DEFAULT_User = "postgres";
  private final String mstrPG_DEFAULT_Pwd = "";
  private StringBuffer msbSEC_Host;
  private StringBuffer msbSEC_Port;
  private StringBuffer msbSEC_User;
  private StringBuffer msbSEC_Pwd;
  private StringBuffer msbSEC_UseBinaryList;
  private final String mstrSEC_DEFAULT_Host = "127.0.0.1";
  private final String mstrSEC_DEFAULT_Port = "1234";
  private final String mstrSEC_DEFAULT_User = "";
  private final String mstrSEC_DEFAULT_Pwd = "";
  private ReadWriteDES readwriteDES;
  private final String strKEY = "b2mn10da"; 
  
  public Configuration()
  {
    gsbPG_Host.append("127.0.0.1");
    gsbPG_Port.append("5432");
    gsbPG_User.append("postgres");
    gsbPG_Pwd.append("");
    


    gsbSEC_Host.append("127.0.0.1");
    gsbSEC_Port.append("1234");
    gsbSEC_User.append("");
    gsbSEC_Pwd.append("");
    gsbSEC_UseBinaryList.append(true);
    
    makeFolder();
    if (LogFileHandler.mFH == null) {
      new LogFileHandler();
    }
    this.readwriteDES = new ReadWriteDES();
    

    this.mprop = new LinkedProperties();
    
    this.mfp = new File(globalstrbufferHomeDir.toString());
    
    this.msbPG_Host = new StringBuffer("PG-Host");
    this.msbPG_Port = new StringBuffer("PG-Port");
    this.msbPG_User = new StringBuffer("PG-User");
    this.msbPG_Pwd = new StringBuffer("PG-Pwd");
    this.msbSEC_Host = new StringBuffer("SEC-Host");
    this.msbSEC_Port = new StringBuffer("SEC-Port");
    this.msbSEC_User = new StringBuffer("SEC-User");
    this.msbSEC_Pwd = new StringBuffer("SEC-Pwd");
    this.msbSEC_UseBinaryList = new StringBuffer("SEC-UseBinaryList");
  }
  
  public Configuration(int iValues)
  {
    makeFolder();
    if (LogFileHandler.mFH == null) {
      new LogFileHandler();
    }
    this.readwriteDES = new ReadWriteDES();
    


    this.mprop = new LinkedProperties();
    
    this.mfp = new File(globalstrbufferHomeDir.toString());
    
    this.msbPG_Host = new StringBuffer("PG-Host");
    this.msbPG_Port = new StringBuffer("PG-Port");
    this.msbPG_User = new StringBuffer("PG-User");
    this.msbPG_Pwd = new StringBuffer("PG-Pwd");
    this.msbSEC_Host = new StringBuffer("SEC-Host");
    this.msbSEC_Port = new StringBuffer("SEC-Port");
    this.msbSEC_User = new StringBuffer("SEC-User");
    this.msbSEC_Pwd = new StringBuffer("SEC-Pwd");
    this.msbSEC_UseBinaryList = new StringBuffer("SEC-UseBinaryList");
  }
  
  public boolean makeFolder()
  {
    this.mfp = new File(globalstrbufferHomeDir.toString());
    if (!this.mfp.exists()) {
      return this.mfp.mkdir();
    }
    return true;
  }
  
  public boolean write()
  {
    boolean bReturn = false;
    
    this.mfp = new File(globalstrbufferHomeDir.toString());
    if (!this.mfp.exists()) {
      this.mfp.mkdir();
    }
    this.mfp = new File(globalStringBufConfigFile.toString());
    try
    {
      this.mfileOPstream = new FileOutputStream(this.mfp, false);
      
      this.readwriteDES = new ReadWriteDES();
      
      this.mprop.setProperty(this.msbPG_Host.toString(), gsbPG_Host.toString());
      this.mprop.setProperty(this.msbPG_Port.toString(), gsbPG_Port.toString());
      this.mprop.setProperty(this.msbPG_User.toString(), gsbPG_User.toString());
      this.mprop.setProperty(this.msbPG_Pwd.toString(), this.readwriteDES.encode(gsbPG_Pwd.toString().getBytes(), "a3rg8ukx"));
      
      this.mprop.setProperty(this.msbSEC_Host.toString(), gsbSEC_Host.toString());
      this.mprop.setProperty(this.msbSEC_Port.toString(), gsbSEC_Port.toString());
      this.mprop.setProperty(this.msbSEC_User.toString(), gsbSEC_User.toString());
      this.mprop.setProperty(this.msbSEC_Pwd.toString(), this.readwriteDES.encode(gsbSEC_Pwd.toString().getBytes(), "a3rg8ukx"));
      this.mprop.setProperty(this.msbSEC_UseBinaryList.toString(), gsbSEC_UseBinaryList.toString());
      

      this.mprop.store(this.mfileOPstream, "Configuration-File");
      
      this.mfileOPstream.close();
      bReturn = true;
    }
    catch (FileNotFoundException e)
    { 

      LogFileHandler.mlogger.severe(e.getMessage());
      bReturn = false;
    }
    catch (IOException e)
    {
      
      LogFileHandler.mlogger.severe(e.getMessage());
      bReturn = false;
    }
    finally {}
    if (this.mfileOPstream != null) {
      try
      {
        this.mfileOPstream.close();
      }
      catch (IOException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
    }
    if (bReturn) {
      LogFileHandler.mlogger.info("config written");
    } else {
      LogFileHandler.mlogger.severe("config was not written");
    }
    return bReturn;
  }
  
  public boolean read()
  {
    boolean bReturn = false;
    
    this.mfp = new File(globalStringBufConfigFile.toString());
    if ((!this.mfp.exists()) || (!this.mfp.canRead()))
    {
      insertDialogs();
      if (!write())
      {
        LogFileHandler.mlogger.warning("configuration data cannot be written");
        return false;
      }
    }
    try
    {
      this.mprop.load(new FileInputStream(this.mfp));
      
      gsbPG_Host.delete(0, gsbPG_Host.length());
      gsbPG_Port.delete(0, gsbPG_Port.length());
      gsbPG_User.delete(0, gsbPG_User.length());
      gsbPG_Pwd.delete(0, gsbPG_Pwd.length());
      
      gsbSEC_Host.delete(0, gsbSEC_Host.length());
      gsbSEC_Port.delete(0, gsbSEC_Port.length());
      gsbSEC_User.delete(0, gsbSEC_User.length());
      gsbSEC_Pwd.delete(0, gsbSEC_Pwd.length());
      gsbSEC_UseBinaryList.delete(0, gsbSEC_UseBinaryList.length());
      
      gsbPG_Host.append(this.mprop.getProperty(this.msbPG_Host.toString(), gsbPG_Host.toString()));
      gsbPG_Port.append(this.mprop.getProperty(this.msbPG_Port.toString(), gsbPG_Port.toString()));
      gsbPG_User.append(this.mprop.getProperty(this.msbPG_User.toString(), gsbPG_User.toString()));
      gsbPG_Pwd.append(this.mprop.getProperty(this.msbPG_Pwd.toString(), gsbPG_Pwd.toString()));
      
      this.readwriteDES = new ReadWriteDES();
      
      String strTmpPWD = "";
      if ("".intern() != gsbPG_Pwd.toString().intern())
      {
        strTmpPWD = this.readwriteDES.decode(gsbPG_Pwd.toString(), "a3rg8ukx");
        gsbPG_Pwd.delete(0, gsbPG_Pwd.length());
        gsbPG_Pwd.append(strTmpPWD);
      }
      gsbSEC_Host.append(this.mprop.getProperty(this.msbSEC_Host.toString(), gsbSEC_Host.toString()));
      gsbSEC_Port.append(this.mprop.getProperty(this.msbSEC_Port.toString(), gsbSEC_Port.toString()));
      gsbSEC_User.append(this.mprop.getProperty(this.msbSEC_User.toString(), gsbSEC_User.toString()));
      gsbSEC_Pwd.append(this.mprop.getProperty(this.msbSEC_Pwd.toString(), gsbSEC_Pwd.toString()));
      if ("".intern() != gsbSEC_Pwd.toString().intern())
      {
        strTmpPWD = this.readwriteDES.decode(gsbSEC_Pwd.toString(), "a3rg8ukx");
        gsbSEC_Pwd.delete(0, gsbSEC_Pwd.length());
        gsbSEC_Pwd.append(strTmpPWD);
      }
      gsbSEC_UseBinaryList.append(this.mprop.getProperty(this.msbSEC_UseBinaryList.toString(), gsbSEC_UseBinaryList.toString()));
      if (!gsbPG_Port.toString().matches("^((0-9)+)$"))
      {
        gsbSEC_Port.delete(0, gsbPG_Port.length());
        gsbSEC_Port.append("5432");
      }
      if (!gsbSEC_Port.toString().matches("^((0-9)+)$"))
      {
        gsbSEC_Port.delete(0, gsbSEC_Port.length());
        gsbSEC_Port.append("1234");
      }
      bReturn = true;
    }
    catch (FileNotFoundException e)
    {
      
      LogFileHandler.mlogger.severe(e.getMessage());
      bReturn = false;
    }
    catch (IOException e)
    { 
      LogFileHandler.mlogger.severe(e.getMessage());
      bReturn = false;
    }
    finally {}
    if (bReturn) {
      LogFileHandler.mlogger.info("read config successfull");
    } else {
      LogFileHandler.mlogger.warning("error in reading the config file");
    }
    return bReturn;
  }
  
  public void insertDialogs()
  {
    new SetUI().setUIAndLanguage();
    JPasswordField passwordField = new JPasswordField();
    passwordField.setEchoChar('#');
    JLabel jlabelPasswordText = new JLabel("");
    
    String strReturnValue = (String)JOptionPane.showInputDialog(
      null, 
      "Please enter PostgreSQL/PostGIS host", 
      "Configuration input", 
      3, 
      null, 
      null, 
      gsbPG_Host.toString());
    if ((strReturnValue != null) && (strReturnValue.length() > 0))
    {
      gsbPG_Host.delete(0, gsbPG_Host.length());
      gsbPG_Host.append(strReturnValue);
    }
    strReturnValue = (String)JOptionPane.showInputDialog(
      null, 
      "Please enter PostgreSQL/PostGIS port", 
      "Configuration input", 
      3, 
      null, 
      null, 
      gsbPG_Port.toString());
    if ((strReturnValue != null) && (strReturnValue.length() > 0))
    {
      gsbPG_Port.delete(0, gsbPG_Port.length());
      gsbPG_Port.append(strReturnValue);
    }
    strReturnValue = (String)JOptionPane.showInputDialog(
      null, 
      "Please enter PostgreSQL/PostGIS user", 
      "Configuration input", 
      3, 
      null, 
      null, 
      gsbPG_User.toString());
    if (strReturnValue != null)
    {
      gsbPG_User.delete(0, gsbPG_User.length());
      gsbPG_User.append(strReturnValue);
    }
    JPanel jpanel = new JPanel(new BorderLayout());
    jlabelPasswordText.setText("Please enter PostgreSQL/PostGIS password");
    jpanel.add(jlabelPasswordText, "North");
    jpanel.add(passwordField, "South");
    
    passwordField.setText(gsbPG_Pwd.toString());
    
    int iretPassword = JOptionPane.showConfirmDialog(null, 
      jpanel, 
      "Configuration input", 
      2);
    
    strReturnValue = String.valueOf(passwordField.getPassword());
    passwordField.setText("");
    if ((iretPassword == 0) && (strReturnValue != null))
    {
      gsbPG_Pwd.delete(0, gsbPG_Pwd.length());
      gsbPG_Pwd.append(strReturnValue);
    }
    strReturnValue = (String)JOptionPane.showInputDialog(
      null, 
      "Please enter SECONDO host", 
      "Configuration input", 
      3, 
      null, 
      null, 
      gsbSEC_Host.toString());
    if ((strReturnValue != null) && (strReturnValue.length() > 0))
    {
      gsbSEC_Host.delete(0, gsbSEC_Host.length());
      gsbSEC_Host.append(strReturnValue);
    }
    strReturnValue = (String)JOptionPane.showInputDialog(
      null, 
      "Please enter SECONDO port", 
      "Configuration input", 
      3, 
      null, 
      null, 
      gsbSEC_Port.toString());
    if (strReturnValue != null)
    {
      gsbSEC_Port.delete(0, gsbSEC_Port.length());
      gsbSEC_Port.append(strReturnValue);
    }
    strReturnValue = (String)JOptionPane.showInputDialog(
      null, 
      "Please enter SECONDO user", 
      "Configuration input", 
      3, 
      null, 
      null, 
      gsbSEC_User.toString());
    if (strReturnValue != null)
    {
      gsbSEC_User.delete(0, gsbSEC_User.length());
      gsbSEC_User.append(strReturnValue);
    }
    jpanel = new JPanel(new BorderLayout());
    jlabelPasswordText.setText("Please enter SECONDO password");
    jpanel.add(jlabelPasswordText, "North");
    jpanel.add(passwordField, "South");
    
    passwordField.setText(gsbSEC_Pwd.toString());
    
    iretPassword = JOptionPane.showConfirmDialog(null, 
      jpanel, 
      "Configuration input", 
      2);
    
    strReturnValue = String.valueOf(passwordField.getPassword());
    passwordField.setText("");
    if ((iretPassword == 0) && (strReturnValue != null))
    {
      gsbSEC_Pwd.delete(0, gsbSEC_Pwd.length());
      gsbSEC_Pwd.append(strReturnValue);
    }
  }
}
