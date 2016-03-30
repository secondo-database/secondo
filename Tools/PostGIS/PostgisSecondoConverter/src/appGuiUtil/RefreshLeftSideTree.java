package appGuiUtil;

import appGui.MainGui;



import java.util.logging.Logger;

import secondo.ISECTextMessages;
import secondoPostgisUtil.LogFileHandler;

public class RefreshLeftSideTree extends Thread implements ISECTextMessages
{
  private MainGui mgui;
  
  public RefreshLeftSideTree(MainGui mgui2)
  {
    this.mgui = mgui2;
  }
  
  public void run()
  {
    super.run();
    
    LogFileHandler.mlogger.info("refresh left side");
    try
    {
      Thread thread = new Thread(this.mgui.getRunInitLinks());
      thread.start();
      thread.join();
    }
    catch (InterruptedException e1)
    {
      e1.printStackTrace();
    }
    finally
    {
      if (!this.mgui.getInitLinks())
      {
        LogFileHandler.mlogger.severe("no connection to left side");
        new Warning("The connection to SECONDO database cannot be established."
          		+ "Please check whether the connection's parameters for Secondo are correct.");
      }
    }
  }
}
