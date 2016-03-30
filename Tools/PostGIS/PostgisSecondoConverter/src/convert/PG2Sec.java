package convert;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.logging.Logger;
import secondoPostgisUtil.LogFileHandler;
import secondoPostgisUtil.UtilFunctions;

public class PG2Sec
{
  PostgresTypes pgTypes;
  UtilFunctions myutil = new UtilFunctions();
  File mFile;
  Writer out;
  
  public PG2Sec()
  {
    this.pgTypes = new PostgresTypes();
    try
    {
      this.mFile = File.createTempFile("seccommands", ".sec");
      
      this.out = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(this.mFile), "UTF8"));
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
  }
  
  public void endWrite()
  {
    try
    {
      this.out.flush();
      this.out.close();
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
  }
  
  public void writeObjectlet(StringBuffer _sbTBL)
  {
    try
    {
      this.out.append("let ");
      this.out.flush();
      if (_sbTBL.length() > 15)
      {
        this.out.append(_sbTBL.toString().substring(0, 15));
        this.out.flush();
      }
      else
      {
        this.out.append(_sbTBL.toString());
        this.out.flush();
      }
      this.out.append(" = ");
      this.out.flush();
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
  }
  
  public void writeHeader(ArrayList<String> _alColNames, ArrayList<String> _alTypeNames)
  {
    StringBuffer sbHeader = new StringBuffer();
    
    sbHeader.append("[const rel(tuple([");
    for (int i = 0; i < _alColNames.size(); i++)
    {
      sbHeader.append(this.myutil.firstCharUpperCase((String)_alColNames.get(i)));
      sbHeader.append(": ");
      

      sbHeader.append(this.pgTypes.convertPGType2SecType((String)_alTypeNames.get(i)));
      
      sbHeader.append(", ");
    }
    sbHeader.deleteCharAt(sbHeader.lastIndexOf(" "));
    sbHeader.deleteCharAt(sbHeader.lastIndexOf(","));
    
    sbHeader.append("])) ");
    try
    {
      this.out.append(sbHeader.toString());
      this.out.flush();
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
  }
  
  public void writeValueBegin()
  {
    write(new StringBuffer("value("));
  }
  
  public void write(StringBuffer _sbToWrite)
  {
    try
    {
      this.out.append(_sbToWrite.toString());
      this.out.flush();
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
  }
  
  public void write(String _strToWrite)
  {
    try
    {
      this.out.append(_strToWrite);
      this.out.flush();
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.severe(e.getMessage());
    }
  }
  
  public void writeValueClose()
  {
    write(new StringBuffer(")]"));
  }
}
