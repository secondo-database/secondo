package  viewer.hoese.algebras;

import java.awt.geom.*;
import java.awt.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.*;
import viewer.hoese.*;
import tools.Reporter;

/**
 * The displayclass for grid2
 */
public class Dsplgrid2 extends DsplGeneric
{
  public Dsplgrid2()
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
      if(values == 3)
      {
        if(value.first().atomType() == ListExpr.REAL_ATOM &&
                value.second().atomType() == ListExpr.REAL_ATOM &&
                value.third().atomType() == ListExpr.REAL_ATOM)
        {
          valueString = "Origin (x = " + value.first().realValue() +
                        ", y = " + value.second().realValue() +
                        "), Length = " + value.third().realValue();
        }
      }
    }

    return valueString;
  }
}
