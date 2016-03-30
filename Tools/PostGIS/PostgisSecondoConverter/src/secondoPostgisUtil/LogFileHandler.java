package secondoPostgisUtil;



import java.io.File;
import java.io.IOException;
import java.util.logging.FileHandler;
import java.util.logging.Level;
import java.util.logging.Logger;

public class LogFileHandler
{
  private int miMaxSize = 10485760;
  private int miCountRotate = 10;
  public static volatile FileHandler mFH;
  public static volatile Logger mlogger;
  
  public LogFileHandler()
  {
    try
    {
      mFH = new FileHandler(System.getProperty("user.home") + File.separatorChar + ".Etie" + File.separatorChar + ".Etie.log", 
        this.miMaxSize, this.miCountRotate, true);
      
      mlogger = Logger.getLogger("SEcondoPostgis-Log");
      mlogger.setUseParentHandlers(true);
      mlogger.setLevel(Level.ALL);
      mlogger.addHandler(mFH);
    }
    catch (SecurityException e)
    {
      e.printStackTrace();
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
  }
}
