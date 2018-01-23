

package viewer.hoese.algebras;

import sj.lang.ListExpr;
import java.awt.geom.AffineTransform;
import java.awt.Shape;
import viewer.hoese.QueryResult;
import tools.Reporter;

public class Dsplline2 extends Dsplregion2{

  public Shape getRenderObject(int num,AffineTransform at){
    if(num<1){
       return p;
    } else{
       return null;
    }
  }

  public boolean isLineType(int num){
    return true;
  }


   public void init (String name, int nameWidth, int indent, ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = extendString(name, nameWidth, indent);
    ScanValue(value);
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry((AttrName)+" : Error(line2)");
      return;
    } else if(!defined){
       qr.addEntry((AttrName)+" : undefined");
       return;
    }
    qr.addEntry(this);
  }

}
