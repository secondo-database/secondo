package postgis;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;
import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.logging.Logger;
import secondoPostgisUtil.IDelimiter;
import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;
import appGuiUtil.Warning;

public class SendConvertStatementsPG
  implements Runnable, IDelimiter, IGlobalParameters
{
  StringBuffer msbSQLCommand;
  StringBuffer msbDBName;
  StringBuffer msbHeader;
  StringBuffer msbOrder;
  StringBuffer msbTable;
  StringBuffer msbWhereCondition;
  ConnectPostgres mcpostgres;
  private boolean bError;
  File mtempfile;
  
  public SendConvertStatementsPG()
  {
    this.msbSQLCommand = new StringBuffer();
    this.msbDBName = new StringBuffer();
    this.msbHeader = new StringBuffer();
    this.msbOrder = new StringBuffer();
    this.msbTable = new StringBuffer();
    this.msbWhereCondition = new StringBuffer();
    
    this.bError = false;
    
    this.mcpostgres = new ConnectPostgres(gsbPG_Host, Integer.valueOf(gsbPG_Port.toString()).intValue(), gsbPG_User, gsbPG_Pwd);
    try
    {
      this.mtempfile = File.createTempFile("sec2pg", ".csv");
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
      this.bError = true;
    }
  }
  
  public void setParameters(StringBuffer _sbDBName, StringBuffer _sbTbl, StringBuffer _sbwhere, StringBuffer _sbSQLCommand, StringBuffer _sbHeader, StringBuffer _sbOrder)
  {
    this.msbDBName.delete(0, this.msbDBName.length());
    this.msbSQLCommand.delete(0, this.msbSQLCommand.length());
    this.msbHeader.delete(0, this.msbHeader.length());
    this.msbOrder.delete(0, this.msbOrder.length());
    this.msbTable.delete(0, this.msbTable.length());
    this.msbWhereCondition.delete(0, this.msbWhereCondition.length());
    
    this.msbDBName.append(_sbDBName);
    this.msbSQLCommand.append(_sbSQLCommand);
    this.msbHeader.append(_sbHeader);
    this.msbOrder.append(_sbOrder);
    this.msbTable.append(_sbTbl);
    this.msbWhereCondition.append(_sbwhere);
  }
  
  public void run()
  {
    StringBuffer sbFinalCommand = new StringBuffer();
    Statement statement = null;
    Writer out = null;
    try
    {
      Thread.sleep(1L);
      if ((this.mcpostgres.connect(this.msbDBName)) && (this.mcpostgres != null) && (!this.mcpostgres.conn.isClosed()))
      {
        String strTblName = this.msbSQLCommand.toString().substring(this.msbSQLCommand.lastIndexOf("from"), this.msbSQLCommand.toString().length());
        
        strTblName = this.msbTable.toString();
        
        long lCount = 0L;
        long lCountWhere = 0L;
        long lTblSize = 0L;
        

        statement = this.mcpostgres.conn.createStatement();
        ResultSet rs = statement.executeQuery("select count(*) from " + strTblName + " " + this.msbWhereCondition.toString());
        while (rs.next()) {
          lCountWhere = rs.getLong(1);
        }
        statement = this.mcpostgres.conn.createStatement();
        
        rs = statement.executeQuery("select count(*) from " + strTblName + ";");
        while (rs.next()) {
          lCount = rs.getLong(1);
        }
        statement = this.mcpostgres.conn.createStatement();
        rs = statement.executeQuery("select  pg_relation_size('" + strTblName + "')::numeric AS Size_inByte;");
        while (rs.next()) {
          lTblSize = rs.getLong(1);
        }
        int i = 1;
        int iOffset = 0;
        int iLimit = 250;
        
        int iSizeRow = 0;
        if (lCount == 0L) {
          iSizeRow = 0;
        }
        if (lCount > 1L) {
          iSizeRow = (int)(lTblSize / lCount);
        }
        while ((iSizeRow * iLimit > 10000000) && (iLimit-- > 1)) {}
        sbFinalCommand = new StringBuffer(this.msbDBName.toString().length());
        StringBuffer sbLIMIT = new StringBuffer("LIMIT " + String.valueOf(iLimit) + " ");
        StringBuffer sbOrderBy = new StringBuffer("ORDER BY " + this.msbOrder.toString() + " ASC NULLS FIRST ");
        
        out = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(this.mtempfile), "UTF8"));
        

        out.append(this.msbHeader).append("\r\n");
        out.flush();
        while (iOffset <= lCountWhere)
        {
          sbFinalCommand.delete(0, sbFinalCommand.length());
          
          sbFinalCommand.append(this.msbSQLCommand.toString());
          sbFinalCommand.append(" ");
          sbFinalCommand.append(this.msbWhereCondition.toString());
          sbFinalCommand.append(" ");
          sbFinalCommand.append(sbOrderBy.toString());
          sbFinalCommand.append(sbLIMIT.toString());
          sbFinalCommand.append("OFFSET ");
          sbFinalCommand.append(iOffset);
          sbFinalCommand.append(";");
          


          rs = statement.executeQuery(sbFinalCommand.toString());
          ResultSetMetaData meta = rs.getMetaData();
          int iColumns = meta.getColumnCount();
          while (rs.next())
          {
            for (i = 1; i <= iColumns; i++) {
              if (i == iColumns) {
                out.append(rs.getString(i));
              } else {
                out.append(rs.getString(i)).append(";-;");
              }
            }
            out.append("\r\n");
            out.flush();
          }
          iOffset += iLimit;
        }
        statement.close();
        out.close();
        out.close();
      }
    }
    catch (SQLException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
      this.bError = true;
      new Warning(sbFinalCommand.toString() + "\n" + e.getMessage());
      try
      {
        if (statement != null) {
          statement.close();
        }
      }
      catch (SQLException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if (out != null) {
          out.close();
        }
      }
      catch (IOException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if ((this.mcpostgres != null) && (this.mcpostgres.conn != null) && (!this.mcpostgres.conn.isClosed())) {
          this.mcpostgres.conn.close();
        }
      }
      catch (SQLException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
    }
    catch (UnsupportedEncodingException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
      this.bError = true;
      try
      {
        if (statement != null) {
          statement.close();
        }
      }
      catch (SQLException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if (out != null) {
          out.close();
        }
      }
      catch (IOException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if ((this.mcpostgres != null) && (this.mcpostgres.conn != null) && (!this.mcpostgres.conn.isClosed())) {
          this.mcpostgres.conn.close();
        }
      }
      catch (SQLException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
    }
    catch (FileNotFoundException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
      this.bError = true;
      try
      {
        if (statement != null) {
          statement.close();
        }
      }
      catch (SQLException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if (out != null) {
          out.close();
        }
      }
      catch (IOException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if ((this.mcpostgres != null) && (this.mcpostgres.conn != null) && (!this.mcpostgres.conn.isClosed())) {
          this.mcpostgres.conn.close();
        }
      }
      catch (SQLException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
      this.bError = true;
      try
      {
        if (statement != null) {
          statement.close();
        }
      }
      catch (SQLException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if (out != null) {
          out.close();
        }
      }
      catch (IOException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if ((this.mcpostgres != null) && (this.mcpostgres.conn != null) && (!this.mcpostgres.conn.isClosed())) {
          this.mcpostgres.conn.close();
        }
      }
      catch (SQLException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
    }
    catch (InterruptedException i)
    {
      try
      {
        if (statement != null) {
          statement.close();
        }
      }
      catch (SQLException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if (out != null) {
          out.close();
        }
      }
      catch (IOException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if ((this.mcpostgres != null) && (this.mcpostgres.conn != null) && (!this.mcpostgres.conn.isClosed())) {
          this.mcpostgres.conn.close();
        }
      }
      catch (SQLException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
      this.bError = true;
      try
      {
        if (statement != null) {
          statement.close();
        }
      }
      catch (SQLException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if (out != null) {
          out.close();
        }
      }
      catch (IOException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if ((this.mcpostgres != null) && (this.mcpostgres.conn != null) && (!this.mcpostgres.conn.isClosed())) {
          this.mcpostgres.conn.close();
        }
      }
      catch (SQLException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
    }
    catch (ThreadDeath td)
    {
      try
      {
        if (statement != null) {
          statement.close();
        }
      }
      catch (SQLException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if (out != null) {
          out.close();
        }
      }
      catch (IOException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if ((this.mcpostgres != null) && (this.mcpostgres.conn != null) && (!this.mcpostgres.conn.isClosed())) {
          this.mcpostgres.conn.close();
        }
      }
      catch (SQLException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
      this.bError = true;
      try
      {
        if (statement != null) {
          statement.close();
        }
      }
      catch (SQLException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if (out != null) {
          out.close();
        }
      }
      catch (IOException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if ((this.mcpostgres != null) && (this.mcpostgres.conn != null) && (!this.mcpostgres.conn.isClosed())) {
          this.mcpostgres.conn.close();
        }
      }
      catch (SQLException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
    }
    finally
    {
      try
      {
        if (statement != null) {
          statement.close();
        }
      }
      catch (SQLException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if (out != null) {
          out.close();
        }
      }
      catch (IOException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      try
      {
        if ((this.mcpostgres != null) && (this.mcpostgres.conn != null) && (!this.mcpostgres.conn.isClosed())) {
          this.mcpostgres.conn.close();
        }
      }
      catch (SQLException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
    }
  }
  
  public File getMtempfile()
  {
    return this.mtempfile;
  }
  
  public boolean wasError()
  {
    return this.bError;
  }
}
