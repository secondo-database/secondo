package secondo;

import java.util.Vector;

import javax.swing.JTable;

import sj.lang.ListExpr;

public class MyRelViewer
{
  public JTable createTableFrom(ListExpr LE)
  {
    boolean result = true;
    JTable NTable = null;
    if (LE.listLength() != 2) {
      return null;
    }
    ListExpr type = LE.first();
    ListExpr value = LE.second();
    

    ListExpr maintype = type.first();
    if ((type.listLength() != 2) || (!maintype.isAtom()) || (maintype.atomType() != 5) || (
      (!maintype.symbolValue().equals("rel")) && (!maintype.symbolValue().equals("mrel")) && (!maintype.symbolValue().equals("trel")))) {
      return null;
    }
    ListExpr tupletype = type.second();
    
    ListExpr TupleFirst = tupletype.first();
    if ((tupletype.listLength() != 2) || (!TupleFirst.isAtom()) || 
      (TupleFirst.atomType() != 5) || 
      (!(TupleFirst.symbolValue().equals("tuple") | TupleFirst.symbolValue().equals("mtuple")))) {
      return null;
    }
    ListExpr TupleTypeValue = tupletype.second();
    
    String[] head = new String[TupleTypeValue.listLength()];
    for (int i = 0; (!TupleTypeValue.isEmpty()) && (result); i++)
    {
      ListExpr TupleSubType = TupleTypeValue.first();
      if (TupleSubType.listLength() != 2) {
        result = false;
      } else {
        head[i] = TupleSubType.first().writeListExprToString();
      }
      TupleTypeValue = TupleTypeValue.rest();
    }
    if (result)
    {
      Vector V = new Vector();
      while (!value.isEmpty())
      {
        ListExpr TupleValue = value.first();
        String[] row = new String[head.length];
        int pos = 0;
        while (((pos < head.length ? 1 : 0) & (TupleValue.isEmpty() ? 0 : 1)) != 0)
        {
          ListExpr Elem = TupleValue.first();
          if ((Elem.isAtom()) && (Elem.atomType() == 4))
          {
            row[pos] = Elem.stringValue();
          }
          else if (((Elem.isAtom()) && (Elem.atomType() == 6)) || (
            (!Elem.isAtom()) && (Elem.listLength() == 1) && (Elem.first().isAtom()) && 
            (Elem.first().atomType() == 6)))
          {
            if (!Elem.isAtom()) {
              Elem = Elem.first();
            }
            row[pos] = Elem.textValue();
          }
          else
          {
            row[pos] = TupleValue.first().writeListExprToString();
          }
          pos++;
          TupleValue = TupleValue.rest();
        }
        V.add(row);
        value = value.rest();
      }
      String[][] TableDatas = new String[V.size()][head.length];
      for (int i = 0; i < V.size(); i++) {
        TableDatas[i] = ((String[])V.get(i));
      }
      NTable = new JTable(TableDatas, head)
      {
        private static final long serialVersionUID = 1L;
        
        public boolean isCellEditable(int x, int y)
        {
          return false;
        }
      };
    }
    if (result) {
      return NTable;
    }
    return null;
  }
  
  public boolean canDisplay(MySecondoObject o)
  {
    ListExpr LE = o.toListExpr();
    if (LE.listLength() != 2) {
      return false;
    }
    LE = LE.first();
    if ((LE.isAtom()) || (LE.isEmpty())) {
      return false;
    }
    LE = LE.first();
    if ((LE.isAtom()) && (LE.atomType() == 5) && 
      ((LE.symbolValue().equals("rel") | LE.symbolValue().equals("mrel") | LE.symbolValue().equals("trel")))) {
      return true;
    }
    return false;
  }
}
