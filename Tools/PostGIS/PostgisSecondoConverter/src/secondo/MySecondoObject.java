package secondo;

import java.util.Vector;

import javax.swing.JTable;

import sj.lang.ListExpr;

public class MySecondoObject
{
  private String Name = "";
  private ListExpr value = null;
  StringBuffer errorMessage = new StringBuffer();
  
  public MySecondoObject(String name, ListExpr list)
  {
    this.Name = name;
    this.value = list;
  }
  
  public String getName()
  {
    return this.Name;
  }
  
  public String toString()
  {
    return this.Name;
  }
  
  public void setName(String Name)
  {
    this.Name = Name;
  }
  
  public boolean fromList(ListExpr value)
  {
    this.value = value;
    return true;
  }
  
  public ListExpr toListExpr()
  {
    return this.value;
  }
  
  public Vector<String[]> getInVector(ListExpr LE)
  {
    boolean result = true;
    JTable NTable = null;
    
    Vector<String[]> V = new Vector();
    if (LE.listLength() != 2) {
      return V;
    }
    ListExpr type = LE.first();
    ListExpr value = LE.second();
    

    ListExpr maintype = type.first();
    if ((type.listLength() != 2) || (!maintype.isAtom()) || (maintype.atomType() != 5) || (
      (!maintype.symbolValue().equals("rel")) && (!maintype.symbolValue().equals("mrel")) && (!maintype.symbolValue().equals("trel")))) {
      return V;
    }
    ListExpr tupletype = type.second();
    
    ListExpr TupleFirst = tupletype.first();
    if ((tupletype.listLength() != 2) || (!TupleFirst.isAtom()) || 
      (TupleFirst.atomType() != 5) || 
      (!(TupleFirst.symbolValue().equals("tuple") | TupleFirst.symbolValue().equals("mtuple")))) {
      return V;
    }
    ListExpr TupleTypeValue = tupletype.second();
    
    String[] head = new String[TupleTypeValue.listLength()];
    if (result) {
      while (!value.isEmpty())
      {
        ListExpr TupleValue = value.first();
        String[] row = new String[head.length];
        int pos = 0;
        while (((pos < head.length ? 1 : 0) & (TupleValue.isEmpty() ? 0 : 1)) != 0)
        {
          ListExpr Elem = TupleValue.first();
          
          row[pos] = TupleValue.first().writeListExprToString();
          row[pos] = row[pos].replace("\r", "");
          row[pos] = row[pos].replace("\n", "");
          pos++;
          TupleValue = TupleValue.rest();
        }
        V.add(row);
        value = value.rest();
      }
    }
    return V;
  }
}
