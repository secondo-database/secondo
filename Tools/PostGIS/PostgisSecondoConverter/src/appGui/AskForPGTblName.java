package appGui;

import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.swing.JOptionPane;
import secondo.ISECTextMessages;
import secondoPostgisUtil.LogFileHandler;
import appGuiUtil.Message;
import appGuiUtil.SetUI;

public class AskForPGTblName
  implements ISECTextMessages
{
  private String strPostgresTblName;
  Pattern pSecTblName = Pattern.compile("[A-Za-z]{1}[a-zA-Z0-9_]{0,47}");
  
  public AskForPGTblName()
  {
    this.strPostgresTblName = "tblname";
  }
  
  public AskForPGTblName(String _strPostgresTblName)
  {
    this.strPostgresTblName = _strPostgresTblName;
  }
  
  public boolean askForPostgresTblName()
  {
    new SetUI().setUIAndLanguage();
    String strDBNameValue = (String)JOptionPane.showInputDialog(
      null, 
      "Your tablename exists. Enter a new name or the table will be overwritten.\nNotice:\nAn identifier is defined by the regular expression:\n[a-z,A-Z]\n([a-z,A-Z]|[0-9]|_)*\n", 
      


      "Input table name", 
      3, 
      null, 
      null, 
      this.strPostgresTblName);
    if ((strDBNameValue != null) && (strDBNameValue.length() > 0))
    {
      this.strPostgresTblName = strDBNameValue;
      
      Matcher m = this.pSecTblName.matcher(this.strPostgresTblName);
      if (!m.matches())
      {
        LogFileHandler.mlogger.warning("wrong name for table");
        new Message("Wrong table name.");
        return false;
      }
    }
    else
    {
      return false;
    }
    return true;
  }
  
  public boolean checkTblName(String _strName)
  {
    Matcher m = this.pSecTblName.matcher(this.strPostgresTblName);
    if (!m.matches())
    {
      LogFileHandler.mlogger.warning("wrong name for table");
      new Message("Wrong table name.");
      return false;
    }
    return true;
  }
  
  public String getStrPostgresTblName()
  {
    return this.strPostgresTblName;
  }
}

