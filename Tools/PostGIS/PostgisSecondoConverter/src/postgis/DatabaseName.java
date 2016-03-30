package postgis;

public class DatabaseName
{
  private StringBuffer sbName;
  
  public DatabaseName()
  {
    this.sbName = new StringBuffer();
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
}
