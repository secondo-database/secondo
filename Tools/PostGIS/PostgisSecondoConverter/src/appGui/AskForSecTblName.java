package appGui;

import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.swing.JOptionPane;
import secondo.ISECTextMessages;
import secondo.UtilSecFunction;
import secondoPostgisUtil.LogFileHandler;
import appGuiUtil.Message;
import appGuiUtil.SetUI;
public class AskForSecTblName
implements ISECTextMessages
{
	private String strSecondoTblName;
	private Pattern pSecTblName = Pattern.compile("[A-Za-z]{1}[a-zA-Z0-9_]{0,47}");
	
	public AskForSecTblName()
	{
	  this.strSecondoTblName = "tblname";
	}
	
	public AskForSecTblName(String strSecondoTblName)
	{
	  this.strSecondoTblName = strSecondoTblName;
	}
	
	public boolean askForSecondoTblName()
	{
	  new SetUI().setUIAndLanguage();
	  String strDBNameValue = (String)JOptionPane.showInputDialog(
		    null, 
		    "Your objectname exists. An existing object will be overwritten. Please enter a new object name.Notice:"
		    + "\nAn identifier is defined by the regular expression:\n[a-z,A-Z]\n([a-z,A-Z]|[0-9]|_)"
		    + "*\nwith a maximal length of 48 characters,"
		    + "\ne.g. lineitem, employee, cities_pop but not _x_ or 10times.", 
		        "Input table name", 
		    3, 
		    null, 
		    null, 
		    this.strSecondoTblName);
	  if ((strDBNameValue != null) && (strDBNameValue.length() > 0))
	  {
	    this.strSecondoTblName = strDBNameValue;
	    Matcher m = this.pSecTblName.matcher(this.strSecondoTblName);
	    if ((!m.matches()) || (new UtilSecFunction().isOperator(this.strSecondoTblName)))
	    {
	      LogFileHandler.mlogger.warning("wrong object name");
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
	
	public String getStrSecondoTblName()
	{
	  return this.strSecondoTblName;
	}
	
	public boolean checkTblName(String _strTblName)
	{
	  if ((_strTblName != null) && (_strTblName.length() > 0))
	  {
	    Matcher m = this.pSecTblName.matcher(_strTblName);
	    if ((!m.matches()) || (new UtilSecFunction().isOperator(_strTblName)))
	    {
	      LogFileHandler.mlogger.warning("wrong object name");
	      new Message("Wrong table name.");
	      return false;
	    }
	    return true;
	  }
	  new Message("Wrong table name.");
	  return false;
	}
}
