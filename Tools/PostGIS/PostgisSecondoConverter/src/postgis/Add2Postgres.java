package postgis;


import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.LinkedList;
import java.util.logging.Logger;
import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;
import appGuiUtil.Message;
import appGuiUtil.Warning;

public class Add2Postgres
implements IGlobalParameters, IPGTextMessages
{
final String strALTERTABLE = "ALTER TABLE";
//final String strCREATETABLE = "create table";

public boolean adding(String _strDBName, String _strTemplate, File _fpPGSql, String _strTBL)
{
  ConnectPostgres connPG = new ConnectPostgres(gsbPG_Host, Integer.valueOf(gsbPG_Port.toString()).intValue(), gsbPG_User, gsbPG_Pwd);
  
  LinkedList<DatabaseName> llPGDBs = new LinkedList();
  boolean breturn = true;
  boolean bDBVorhanden = false;
  if (connPG.connect()) {
    try
    {
      llPGDBs = connPG.getDatabaseNames();
      if (llPGDBs.size() > 0)
      {
        for (int i = 0; i < llPGDBs.size(); i++) {
          if (((DatabaseName)llPGDBs.get(i)).getSbName().toString().intern() == _strDBName.intern())
          {
            bDBVorhanden = true;
            LogFileHandler.mlogger.info("find exist database");
            break;
          }
        }
        Statement statement = null;
        if (bDBVorhanden)
        {
          LogFileHandler.mlogger.info("delete exist database");
          statement = connPG.conn.createStatement();
          statement.execute("drop database " + _strDBName + ";");
        }
        statement = connPG.conn.createStatement();
        if (_strTemplate.intern() == "no template".intern()) {
          statement.execute("create database " + _strDBName + ";");
        } else {
          statement.execute("create database " + _strDBName + " template=" + _strTemplate + ";");
        }
      }
    }
    catch (SQLException e)
    {
      breturn = false;
      new Warning(e);
      LogFileHandler.mlogger.warning(e.getMessage());
      try
      {
        if (connPG.conn != null) {
          connPG.conn.close();
        }
      }
      catch (SQLException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
        breturn = false;
      }
    }
    finally
    {
      try
      {
        if (connPG.conn != null) {
          connPG.conn.close();
        }
      }
      catch (SQLException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
        breturn = false;
      }
    }
  }
  if (connPG.connect(new StringBuffer(_strDBName)))
  {
    Statement statement = null;
    InputStreamReader reader = null;
    BufferedReader br = null;
    
    boolean bError = false;
    try
    {
      LinkedList<Tabelle> llTabelle = connPG.getTableNamesFromDB();
      for (int i = 0; i < llTabelle.size(); i++) {
        if (((Tabelle)llTabelle.get(i)).getSbName().toString().intern() == _strTBL.intern())
        {
          statement = connPG.conn.createStatement();
          statement.execute("drop table " + ((Tabelle)llTabelle.get(i)).getSbName().toString());
          LogFileHandler.mlogger.info("drop table");
        }
      }
      reader = new InputStreamReader(new FileInputStream(_fpPGSql), "UTF-8");
      br = new BufferedReader(reader);
      
      String strLine = "";
      
      LogFileHandler.mlogger.info("start copy into new database at postgres from psql file");
      for (;;)
      {
        statement = connPG.conn.createStatement();
        statement.execute(strLine);
        if ((strLine = br.readLine()) == null) {
          break;
        }
      }
    }
    catch (SQLException e)
    {
      
      LogFileHandler.mlogger.warning(e.getMessage());
      breturn = false;
    }
    catch (UnsupportedEncodingException e)
    {
      

      LogFileHandler.mlogger.warning(e.getMessage());
      breturn = false;
    }
    catch (FileNotFoundException e)
    {
      
      LogFileHandler.mlogger.warning(e.getMessage());
      breturn = false;
    }
    catch (IOException e)
    { 
      LogFileHandler.mlogger.warning(e.getMessage());
      breturn = false;
    }
    finally {}
    try
    {
      if (br != null) {
        br.close();
      }
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
    try
    {
      if (reader != null) {
        reader.close();
      }
    }
    catch (IOException e1)
    {
      LogFileHandler.mlogger.severe(e1.getMessage());
    }
    try
    {
      if (connPG.conn != null) {
        connPG.conn.close();
      }
    }
    catch (SQLException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
    if (bError) {
      new Warning("Error in converting process.");
    }
    return breturn;
  }
  return false;
}

public boolean addingDB(String _strDBName, String _strTable, File _fpPGSql)
{
  ConnectPostgres connPG = new ConnectPostgres(gsbPG_Host, Integer.valueOf(gsbPG_Port.toString()).intValue(), gsbPG_User, gsbPG_Pwd);
  
  boolean breturn = true;
  if (connPG.connect(new StringBuffer(_strDBName)))
  {
    Statement statement = null;
    InputStreamReader reader = null;
    BufferedReader br = null;
    
    LinkedList<Tabelle> llTabelle = connPG.getTableNamesFromDB();
    

    boolean bError = false;
    try
    {
      for (int i = 0; i < llTabelle.size(); i++) {
        if (((Tabelle)llTabelle.get(i)).getSbName().toString().intern() == _strTable.intern())
        {
          statement = connPG.conn.createStatement();
          statement.execute("drop table " + ((Tabelle)llTabelle.get(i)).getSbName().toString());
          LogFileHandler.mlogger.info("drop table");
        }
      }
      reader = new InputStreamReader(new FileInputStream(_fpPGSql), "UTF-8");
      br = new BufferedReader(reader);
      
      String strLine = "";
      int i = 0;
      
      LogFileHandler.mlogger.info("start copy into database at postgres from psql file");
      for (;;)
      {
        statement = connPG.conn.createStatement();
        try
        {
          statement.execute(strLine);
          System.out.println( +i +"Value of strLine " +strLine);
        }
        catch (SQLException e)
        {
          LogFileHandler.mlogger.warning(e.getMessage());
          bError = true;
        }
        i++;
        if ((strLine = br.readLine()) == null) {
          break;
        }
      }
    }
    catch (SQLException e)
    {
             
      LogFileHandler.mlogger.severe(e.getMessage());
      breturn = false;
    }
    catch (UnsupportedEncodingException e)
    {

      LogFileHandler.mlogger.severe(e.getMessage());
      breturn = false;
    }
    catch (FileNotFoundException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
      breturn = false;
    }
    catch (IOException e)
    {
      
      LogFileHandler.mlogger.severe(e.getMessage());
      breturn = false;
    }
    finally {}
    try
    {
      if (br != null) {
        br.close();
      }
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
    try
    {
      if (reader != null) {
        reader.close();
      }
    }
    catch (IOException e1)
    {
      LogFileHandler.mlogger.severe(e1.getMessage());
    }
    try
    {
      if (connPG.conn != null) {
        connPG.conn.close();
      }
    }
    catch (SQLException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
    if (bError) {
      new Warning("Error in converting process.");
    }
    return breturn;
  }
  return false;
}
}

