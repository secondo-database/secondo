package convert;

import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import secondoPostgisUtil.UtilFunctions;

public class PG2SecRestore
  extends PG2Sec
{
  StringBuffer sbCRLF;
  
  public PG2SecRestore()
  {
    this.sbCRLF = new StringBuffer("\r\n");
  }
  
  public void writeDatabaseOpen(StringBuffer sbDBName)
  {
    try
    {
      this.out.append("(DATABASE ");
      this.out.append(sbDBName.toString());
      this.out.append(this.sbCRLF.toString());
      this.out.flush();
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
  }
  
  public void writeDatabaseClose()
  {
    try
    {
      this.out.append(")");
      this.out.append(this.sbCRLF.toString());
      this.out.flush();
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
  }
  
  public void writeTYPESOpen()
  {
    try
    {
      this.out.append("\t(TYPES");
      this.out.append(this.sbCRLF.toString());
      this.out.flush();
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
  }
  
  public void writeTYPESClose()
  {
    try
    {
      this.out.append("\t)");
      this.out.append(this.sbCRLF.toString());
      this.out.flush();
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
  }
  
  public void writeOBJECTSOpen()
  {
    try
    {
      this.out.append("\t(OBJECTS");
      this.out.append(this.sbCRLF.toString());
      this.out.flush();
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
  }
  
  public void writeOBJECTSClose()
  {
    try
    {
      this.out.append("\t)");
      this.out.append(this.sbCRLF.toString());
      this.out.flush();
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
  }
  
  public void writeOBJECTOpen(String strOBJName)
  {
    try
    {
      this.out.append("\t\t(OBJECT ");
      this.out.append(strOBJName);
      this.out.append(this.sbCRLF.toString());
      this.out.append("\t\t()\r\n\t\t(rel");
      this.out.append(this.sbCRLF.toString());
      this.out.append("\t\t\t(tuple");
      this.out.append(this.sbCRLF.toString());
      this.out.append("\t\t(");
      this.out.append(this.sbCRLF.toString());
      this.out.flush();
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
  }
  
  public void writeOBJECTClose()
  {
    try
    {
      this.out.append("))");
      this.out.append(this.sbCRLF.toString());
      this.out.flush();
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
  }
  
  public void writeHeader(ArrayList<String> _alColNames, ArrayList<String> _alTypeNames)
  {
    StringBuffer sbHeader = new StringBuffer();
    for (int i = 0; i < _alColNames.size(); i++)
    {
      sbHeader.append("(");
      
      sbHeader.append(this.myutil.firstCharUpperCase((String)_alColNames.get(i)));
      sbHeader.append(" ");
      


      sbHeader.append(this.pgTypes.convertPGType2SecType((String)_alTypeNames.get(i)));
      
      sbHeader.append(")");
      sbHeader.append(this.sbCRLF);
    }
    try
    {
      this.out.append("\t\t\t");
      this.out.append(sbHeader.toString());
      this.out.append("\t\t\t)))");
      this.out.append(this.sbCRLF.toString());
      this.out.append("\t\t(");
      this.out.append(this.sbCRLF.toString());
      this.out.flush();
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
  }
  
  public void writeValueBegin()
  {
    try
    {
      this.out.append("\t\t\t(");
      this.out.flush();
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
  }
  
  public void write(StringBuffer _sbToWrite)
  {
    super.write(_sbToWrite);
  }
  
  public void write(String _strToWrite)
  {
    super.write(_strToWrite);
  }
  
  public void writeValueClose()
  {
    try
    {
      this.out.append(")");
      this.out.append(this.sbCRLF);
      this.out.flush();
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
  }
}
