package secondo;



import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.Vector;
import java.util.logging.Logger;

import sj.lang.IntByReference;
import sj.lang.ListExpr;
import secondoPostgisUtil.IDelimiter;
import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;
import appGuiUtil.Message;

public class SendConvertStatementSEC
  implements Runnable, IDelimiter, IGlobalParameters
{
  StringBuffer msbSECCommand;
  StringBuffer msbDBName;
  StringBuffer msbHeader;
  StringBuffer msbQueryCount;
  ConnectSecondo mcsecondo;
  File mtempfile;
  private boolean bError;
  
  public SendConvertStatementSEC()
  {
    this.msbDBName = new StringBuffer();
    this.msbHeader = new StringBuffer();
    this.msbSECCommand = new StringBuffer();
    this.msbQueryCount = new StringBuffer();
    
    this.bError = false;
    try
    {
      this.mcsecondo = new ConnectSecondo(gsbSEC_Host, 
        Integer.valueOf(gsbSEC_Port.toString()).intValue(), gsbSEC_User, 
        gsbSEC_Pwd, 
        Boolean.valueOf(gsbSEC_UseBinaryList.toString()).booleanValue());
    }
    catch (SecurityException e1)
    {
      LogFileHandler.mlogger.severe(e1.getMessage());
      this.bError = true;
    }
    catch (IOException e1)
    {
      LogFileHandler.mlogger.severe(e1.getMessage());
      this.bError = true;
    }
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
  
  public void setParameters(StringBuffer _sbDBName, StringBuffer _sbSQLCommand, StringBuffer _sbHeader, StringBuffer _sbQueryCount)
  {
    this.msbDBName.delete(0, this.msbDBName.length());
    this.msbSECCommand.delete(0, this.msbSECCommand.length());
    this.msbHeader.delete(0, this.msbHeader.length());
    this.msbQueryCount.delete(0, this.msbQueryCount.length());
    
    this.msbDBName.append(_sbDBName);
    this.msbSECCommand.append(_sbSQLCommand);
    this.msbHeader.append(_sbHeader);
    this.msbQueryCount.append(_sbQueryCount);
  }
  
  public void run()
  {
    Writer out = null;
    try
    {
      Thread.sleep(1L);
      if (!this.mcsecondo.connect()) {
        this.bError = true;
      }
      for (;;)
      {
        //return;
        

        this.mcsecondo.sendCommand(new StringBuffer("close database;"));
        
        this.mcsecondo.sendCommand(new StringBuffer("open database " + 
          this.msbDBName.toString() + ";"));
        if (this.mcsecondo.getErrorCode().value != 0)
        {
          new Message(this.mcsecondo.getErrorMessage().toString());
          
          this.mcsecondo.setQueryResults2Null();
          this.mcsecondo.closeConnection();
          this.bError = true;
        }
        else
        {
          this.mcsecondo.setQueryResults2Null();
          if (!this.mcsecondo.isSecondoConnected()) {
            break ; 
          }
          this.mcsecondo.sendCommand(new StringBuffer(this.msbSECCommand.toString() + 
            " count;"));
          if (this.mcsecondo.getErrorCode().value == 0) {
            break;
          }
          new Message(this.mcsecondo.getErrorMessage().toString());
          
          this.mcsecondo.setQueryResults2Null();
          this.mcsecondo.closeConnection();
          this.bError = true;
        }
      }
      int icount = 0;
      if (this.mcsecondo.getResultList().second().atomType() == 1)
      {
        icount = this.mcsecondo.getResultList().second().intValue();
        this.mcsecondo.setQueryResults2Null();
      }
      int iHead = 0;
      double lfAvgTupleSize = 1.0D;
      if (icount > 0)
      {
        this.mcsecondo.sendCommand(new StringBuffer(this.msbSECCommand.toString() + " tuplesize;"));
        if (this.mcsecondo.getErrorCode().value != 0)
        {
          LogFileHandler.mlogger.severe(this.mcsecondo
            .getErrorMessage().toString());
          this.mcsecondo.setQueryResults2Null();
          this.bError = true;
          return;
        }
        if (this.mcsecondo.getResultList().second().atomType() == 2)
        {
          lfAvgTupleSize = 
            this.mcsecondo.getResultList().second().realValue();
          this.mcsecondo.setQueryResults2Null();
        }
      }
      out = new BufferedWriter(new OutputStreamWriter(
        new FileOutputStream(this.mtempfile), "UTF8"));
      
      out.append(this.msbHeader.toString()).append("\r\n");
      
      StringBuffer sbFinalCmd = new StringBuffer();
      
      int iSizeTail = 250;
      while ((lfAvgTupleSize * iSizeTail > 10000000.0D) && (iSizeTail-- > 1)) {}
      int iTail = iSizeTail;
      boolean bStop = false;
      do
      {
        iHead += iSizeTail;
        if (iHead > icount)
        {
          iTail = icount - (iHead - iSizeTail);
          iHead = icount;
          bStop = true;
        }
        sbFinalCmd.delete(0, sbFinalCmd.length());
        sbFinalCmd.append(this.msbSECCommand);
        sbFinalCmd.append("head[");
        sbFinalCmd.append(iHead);
        sbFinalCmd.append("] tail[");
        sbFinalCmd.append(iTail);
        sbFinalCmd.append("] consume;");
        if (this.mcsecondo.sendCommand(sbFinalCmd))
        {
          MySecondoObject secObj = new MySecondoObject("", 
            this.mcsecondo.getResultList());
          
          Vector<String[]> vtmp = secObj.getInVector(this.mcsecondo
            .getResultList());
          for (int i = 0; i < vtmp.size(); i++)
          {
            String[] strTmp = (String[])vtmp.get(i);
            for (int k = 0; k < strTmp.length; k++) {
              if (k < strTmp.length - 1)
              {
                out.append(strTmp[k] + ";-;");
                out.flush();
              }
              else
              {
                out.append(strTmp[k]);
                out.flush();
              }
            }
            out.append("\r\n");
            out.flush();
          }
        }
      } while ((iHead <= icount) && (!bStop));
      out.flush();
      out.close();
      
      this.mcsecondo.sendCommand(new StringBuffer("close database"));
      this.mcsecondo.closeConnection();
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
      this.bError = true;
    }
    catch (InterruptedException irexp)
    {
      try
      {
        if (out != null) {
          out.close();
        }
      }
      catch (IOException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
        this.bError = true;
      }
      if ((this.mcsecondo != null) && (this.mcsecondo.isSecondoConnected()))
      {
        this.mcsecondo.sendCommand(new StringBuffer("close database"));
        this.mcsecondo.closeConnection();
      }
    }
    catch (ThreadDeath td)
    {
      try
      {
        out.close();
      }
      catch (IOException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
      if ((this.mcsecondo != null) && (this.mcsecondo.isSecondoConnected()))
      {
        this.mcsecondo.sendCommand(new StringBuffer("close database"));
        this.mcsecondo.closeConnection();
      }
      this.bError = true;
    }
    finally
    {
      try
      {
        out.close();
      }
      catch (IOException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
      if ((this.mcsecondo != null) && (this.mcsecondo.isSecondoConnected()))
      {
        this.mcsecondo.sendCommand(new StringBuffer("close database"));
        this.mcsecondo.closeConnection();
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
