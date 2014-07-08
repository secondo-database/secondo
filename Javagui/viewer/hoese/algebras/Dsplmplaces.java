package viewer.hoese.algebras;

import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;
import java.util.*;
import javax.swing.*;
import java.awt.*;
import tools.Reporter;

/**
 * This class displays mplaces objects.
 */
public class Dsplmplaces extends DsplGeneric {

  public void init(String name, int nameWidth, int indent, ListExpr type,
                   ListExpr value, QueryResult qr) {
    String T = name;
    T = extendString(T, nameWidth, indent);
    if (isUndefined(value)) {
      qr.addEntry(T + " : undefined");
    }
    else {
      qr.addEntry(T + " :");
      while (!value.isEmpty()) {
        if (!value.first().isEmpty()) { // unit
          if (value.first().listLength() == 2) { // (interval places)
            Interval iv = LEUtils.readInterval(value.first().first());
            String startStr = SymbolicValues.intervalStartToString(iv);
            String placesStr = SymbolicValues.placesToString(
                                                        value.first().second());
            qr.addEntry(startStr + "    " + placesStr);
            if (!SymbolicValues.areIntervalsAdjacent(iv, value.rest())) {
              qr.addEntry(SymbolicValues.intervalEndToString(iv));
            }          }
        }
        value = value.rest();
      }
    }
  }
}
