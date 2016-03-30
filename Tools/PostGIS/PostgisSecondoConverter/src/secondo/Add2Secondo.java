package secondo;



import java.io.File;
import java.io.IOException;
import java.util.LinkedList;

import secondoPostgisUtil.IGlobalParameters;
import appGuiUtil.Message;
import appGuiUtil.Warning;

public class Add2Secondo
  implements IGlobalParameters, ISECTextMessages
{
  public boolean addin(String _strDBName, File _fpRestore)
  {
    boolean bReturn = true;
    ConnectSecondo connSecondo = null;
    try
    {
      connSecondo = new ConnectSecondo(gsbSEC_Host, Integer.valueOf(gsbSEC_Port.toString()).intValue(), 
        gsbSEC_User, gsbSEC_Pwd, Boolean.valueOf(gsbSEC_UseBinaryList.toString()).booleanValue());
      if (!connSecondo.connect())
      {
        new Warning("Can not connect to SECONDO database.\nPlease checkconnection parameters.");
        bReturn = false;
      }
      else
      {
        connSecondo.sendCommand(new StringBuffer("restore database " + _strDBName + " from '" + _fpRestore.getAbsolutePath() + "'"));
        if (connSecondo.getErrorCode().value != 0)
        {
          String strTemp = connSecondo.getErrorMessage().toString();
          
          connSecondo.sendCommand(new StringBuffer("close database"));
          if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
            connSecondo.closeConnection();
          }
          new Message(strTemp);
          //break label284;
          if (connSecondo != null)
          {
            connSecondo.sendCommand(new StringBuffer("close database"));
            connSecondo.closeConnection();
          }
          if (!bReturn) {
            new Warning("Error in converting process.");
          }
          return bReturn;
        }
      }
      connSecondo.sendCommand(new StringBuffer("close database;"));
      if (connSecondo.getErrorCode().value != 0)
      {
        String strTemp = connSecondo.getErrorMessage().toString();
        
        connSecondo.sendCommand(new StringBuffer("close database"));
        if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
          connSecondo.closeConnection();
        }
        new Message(strTemp);
      }
    }
    catch (SecurityException e)
    {
      e = e;
      
      e.printStackTrace();
      bReturn = false;
    }
    catch (IOException e)
    {
      e = e;
      
      e.printStackTrace();
      bReturn = false;
    }
    finally {}
    label284:
    if (connSecondo != null)
    {
      connSecondo.sendCommand(new StringBuffer("close database"));
      connSecondo.closeConnection();
    }
    if (!bReturn) {
      new Warning("Error in converting process.");
    }
    return bReturn;
  }
  
  public boolean addin(String _strDBName, String _strObjectname, File _fpRestore)
  {
    boolean bReturn = true;
    ConnectSecondo connSecondo = null;
    try
    {
      connSecondo = new ConnectSecondo(gsbSEC_Host, Integer.valueOf(gsbSEC_Port.toString()).intValue(), 
        gsbSEC_User, gsbSEC_Pwd, Boolean.valueOf(gsbSEC_UseBinaryList.toString()).booleanValue());
      if (!connSecondo.connect())
      {
        new Warning("Can not connect to SECONDO database.\nPlease checkconnection parameters.");
        bReturn = false;
      }
      else
      {
        LinkedList<SecondoObjectInfoClass> lltmp = connSecondo.getObjects(_strDBName);
        
        connSecondo.setQueryResults2Null();
        if (!connSecondo.sendCommand(new StringBuffer("open database " + _strDBName + ";")))
        {
          String strTemp = connSecondo.getErrorMessage().toString();
          
          connSecondo.sendCommand(new StringBuffer("close database"));
          if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
            connSecondo.closeConnection();
          }
          new Message(strTemp);
        }
        else
        {
          for (int i = 0; i < lltmp.size(); i++) {
            if (((SecondoObjectInfoClass)lltmp.get(i)).getStrObjName().toString().intern() == _strObjectname.intern())
            {
              connSecondo.setQueryResults2Null();
              connSecondo.sendCommand(new StringBuffer("delete " + _strObjectname));
            }
          }
          connSecondo.setQueryResults2Null();
          if (!connSecondo.sendCommand(new StringBuffer("restore " + _strObjectname + " from '" + _fpRestore.getAbsolutePath() + "'")))
          {
            String strTemp = connSecondo.getErrorMessage().toString();
            
            connSecondo.sendCommand(new StringBuffer("close database"));
            if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
              connSecondo.closeConnection();
            }
            new Message(strTemp);
          }
          else
          {
            connSecondo.setQueryResults2Null();
            connSecondo.sendCommand(new StringBuffer("close database;"));
            if (connSecondo.getErrorCode().value != 0)
            {
              String strTemp = connSecondo.getErrorMessage().toString();
              
              connSecondo.sendCommand(new StringBuffer("close database"));
              if ((connSecondo != null) && (connSecondo.isSecondoConnected())) {
                connSecondo.closeConnection();
              }
              new Message(strTemp);
            }
          }
        }
      }
    }
    catch (SecurityException e)
    {
      
      
      e.printStackTrace();
      bReturn = false;
    }
    catch (IOException e)
    {
       
      
      e.printStackTrace();
      bReturn = false;
    }
    finally {}
    if (connSecondo != null)
    {
      connSecondo.sendCommand(new StringBuffer("close database"));
      connSecondo.closeConnection();
    }
    if (!bReturn) {
      new Warning("Error in converting process.");
    }
    return bReturn;
  }
}

 