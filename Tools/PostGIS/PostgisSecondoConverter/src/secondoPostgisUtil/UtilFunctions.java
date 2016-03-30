package secondoPostgisUtil;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.logging.Logger;

public class UtilFunctions
{
  public String firstCharUpperCase(String str)
  {
    return str.substring(0, 1).toUpperCase() + str.substring(1);
  }
  
  public void removeAllTempFiles()
  {
    try
    {
      File fp = File.createTempFile("sec2pg", ".csv");
      
      String strTmpPath = fp.getAbsolutePath().substring(
        0, fp.getAbsolutePath().lastIndexOf(File.separator) + 1);
      
      fp = new File(strTmpPath);
      File[] fpArray = fp.listFiles();
      for (int i = 0; i < fpArray.length; i++) {
        if ((fpArray[i].getName().startsWith("sec2pg")) && (fpArray[i].getName().endsWith(".csv"))) {
          fpArray[i].delete();
        } else if ((fpArray[i].getName().startsWith("seccommands")) && (fpArray[i].getName().endsWith(".sec"))) {
          fpArray[i].delete();
        } else if ((fpArray[i].getName().startsWith("pgcommands")) && (fpArray[i].getName().endsWith(".psql"))) {
          fpArray[i].delete();
        }
      }
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.warning(e.getMessage());
    }
  }
  
  public String[] toStringArray(ArrayList<String> _alStringList)
  {
    String[] _strTmp = new String[_alStringList.size()];
    for (int i = 0; i < _alStringList.size(); i++) {
      _strTmp[i] = ((String)_alStringList.get(i));
    }
    return _strTmp;
  }
  
  public int countChar(String _str, char c)
  {
    int iCount = 0;
    for (int i = 0; i < _str.length(); i++) {
      if (_str.charAt(i) == c) {
        iCount++;
      }
    }
    return iCount;
  }
}
