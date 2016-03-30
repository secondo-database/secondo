package postgis;

public class Spalte
{
  private StringBuffer sbName;
  private StringBuffer sbTyp;
  private boolean bPrimaryKey;
  private boolean bForeignKey;
  
  public Spalte()
  {
    this.sbName = new StringBuffer();
    this.sbTyp = new StringBuffer();
    this.bPrimaryKey = false;
    this.bForeignKey = false;
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
  
  public StringBuffer getSbTyp()
  {
    return this.sbTyp;
  }
  
  public void setSbTyp(StringBuffer sbTyp)
  {
    this.sbTyp.delete(0, this.sbTyp.length());
    this.sbTyp.append(sbTyp);
  }
  
  public boolean isbPrimaryKey()
  {
    return this.bPrimaryKey;
  }
  
  public void setbPrimaryKey(boolean bPrimaryKey)
  {
    this.bPrimaryKey = bPrimaryKey;
  }
  
  public boolean isbForeignKey()
  {
    return this.bForeignKey;
  }
  
  public void setbForeignKey(boolean bForeignKey)
  {
    this.bForeignKey = bForeignKey;
  }
}
