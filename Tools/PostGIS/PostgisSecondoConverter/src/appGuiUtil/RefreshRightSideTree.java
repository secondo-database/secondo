package appGuiUtil;

import appGui.MainGui;
import java.util.logging.Logger;
import postgis.IPGTextMessages;
import secondoPostgisUtil.LogFileHandler;

public class RefreshRightSideTree
  extends Thread
  implements IPGTextMessages
{
  private MainGui mgui;
  
  public RefreshRightSideTree(MainGui _mgui)
  {
    this.mgui = _mgui;
  }
  
  public void run()
  {
    super.run();
    LogFileHandler.mlogger.info("refresh right side");
    try
    {
      Thread th = new Thread(this.mgui.getRunInitRechts());
      th.start();
      th.join();
    }
    catch (InterruptedException e1)
    {
      e1.printStackTrace();
    }
    finally
    {
      if (!this.mgui.getInitRechts())
      {
        LogFileHandler.mlogger.severe("no connection to right side");
        new Warning("Can not connect to PostgreSQL/ PostGIS database.\nPlease check connection parameter.");
      }
    }
  }
}
