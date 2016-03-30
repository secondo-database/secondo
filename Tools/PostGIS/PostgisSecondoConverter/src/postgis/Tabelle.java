package postgis;

public class Tabelle
{
  private StringBuffer sbName;
  private int iRows;
  public static final StringBuffer sbTableRowDelimiter = new StringBuffer(" - ");
  
  public Tabelle()
  {
    this.sbName = new StringBuffer();
    this.iRows = 0;
  }
  
  public StringBuffer getSbName()
  {
    return this.sbName;
  }
  
  public void setSbName(StringBuffer sbName)
  {
    this.sbName.delete(0, this.sbName.length());
    this.sbName.append(sbName);
  }
  
  public int getiRows()
  {
    return this.iRows;
  }
  
  public void setiRows(int iRows)
  {
    this.iRows = iRows;
  }
  
  public StringBuffer getShowText()
  {
    StringBuffer sbReturn = new StringBuffer();
    sbReturn.append(getSbName());
    sbReturn.append(sbTableRowDelimiter);
    sbReturn.append(getiRows());
    return sbReturn;
  }
}
