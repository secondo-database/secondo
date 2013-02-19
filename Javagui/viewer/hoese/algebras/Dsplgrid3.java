package  viewer.hoese.algebras;

import java.awt.geom.*;
import java.awt.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.*;
import viewer.hoese.*;
import tools.Reporter;

/**
 * The displayclass for grid3
 */
public class Dsplgrid3 extends DsplGeneric
{
  public Dsplgrid3()
  {
    super();
  }

  public void init(String name, int nameWidth,  int indent, ListExpr type, ListExpr value, QueryResult qr)
  {
    if(name != null &&
       type != null &&
       value != null &&
       qr != null)
    {
      String extendName = extendString(name, nameWidth, indent);
      String valueString = getValueString(value);

      if(extendName != null &&
         valueString != null)
      {
        qr.addEntry(extendName + " : " + valueString);
      }
    }
  }

  private String getValueString(ListExpr value)
  {
    String valueString = "ERROR";

    if(value != null)
    {
      int values = value.listLength();

      // check length of value list
      if(values == 4)
      {
        if(value.first().isAtom() == true &&
           value.first().atomType() == ListExpr.REAL_ATOM &&
           value.second().isAtom() == true &&
           value.second().atomType() == ListExpr.REAL_ATOM &&
           value.third().isAtom() == true &&
           value.third().atomType() == ListExpr.REAL_ATOM &&
           value.fourth().listLength() == 2 &&
           value.fourth().second().first().isAtom() == true &&
           value.fourth().second().first().atomType() == ListExpr.INT_ATOM &&
           value.fourth().second().second().isAtom() == true &&
           value.fourth().second().second().atomType() == ListExpr.INT_ATOM)
        {
          valueString = "Origin (x = " + value.first().realValue() +
                        ", y = " + value.second().realValue() +
                        "), Length = " + value.third().realValue() +
                        ", Duration = (" +
                        value.fourth().second().first().intValue() +
                        " days, " +
                        value.fourth().second().second().intValue() +
                        " milliseconds)";
        }
      }
    }

    return valueString;
  }
}
