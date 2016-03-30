package secondo;

import java.util.ArrayList;
import secondoPostgisUtil.IMySEC2XXXINFO;

public class SecondoObjectInfoClass
  implements IMySEC2XXXINFO
{
  private String strObjName;
  private int iCount;
  private ArrayList<StringBuffer> alColNames;
  private ArrayList<StringBuffer> alColTypes;
  
  public SecondoObjectInfoClass()
  {
    this.strObjName = "";
    this.iCount = -1;
    this.alColNames = new ArrayList();
    this.alColTypes = new ArrayList();
  }
  
  public String getStrObjName()
  {
    return this.strObjName;
  }
  
  public void setStrObjName(String strObjName)
  {
    this.strObjName = strObjName;
  }
  
  public int getiCount()
  {
    return this.iCount;
  }
  
  public void setiCount(int iCount)
  {
    this.iCount = iCount;
  }
  
  public void setColNames(ArrayList<StringBuffer> _alColumns)
  {
    this.alColNames = new ArrayList(_alColumns);
  }
  
  public void setColTypes(ArrayList<StringBuffer> _alTypes)
  {
    this.alColTypes = new ArrayList(_alTypes);
  }
  
  public int sizeTypes()
  {
    return this.alColTypes.size();
  }
  
  public int sizeNames()
  {
    return this.alColNames.size();
  }
  
  public StringBuffer getColName(int i)
  {
    return (StringBuffer)this.alColNames.get(i);
  }
  
  public StringBuffer getColTypes(int i)
  {
    return (StringBuffer)this.alColTypes.get(i);
  }
  
  public ArrayList<StringBuffer> getTypes()
  {
    return this.alColTypes;
  }
  
  public ArrayList<StringBuffer> getNames()
  {
    return this.alColNames;
  }
}
