package viewer.hoese.algebras;

import sj.lang.ListExpr;
import viewer.hoese.*;

public class Dsplrational extends DsplGeneric implements DsplSimple{
   // returns a string representing this rational or "ERROR" if
   // the format of the list is wrong
   private String getValueString(ListExpr value){
     int len = value.listLength();
     if(len!=4 && len !=5)
        return "ERROR";
     String result="";
     if(value.listLength()==5){ // with sign
        ListExpr SignList = value.first();
        if(SignList.atomType()!=ListExpr.SYMBOL_ATOM)
           return "ERROR";
        String sign = SignList.symbolValue();
        if(sign.equals("-")) // ignore other values
           result += sign +" ";
           value = value.rest(); // skip the signum
     }
     // check the types
     if( value.first().atomType()!=ListExpr.INT_ATOM ||
         value.second().atomType()!=ListExpr.INT_ATOM ||
         value.fourth().atomType()!=ListExpr.INT_ATOM)
         return "ERROR";
     int intPart = value.first().intValue();
     int numDecimal = value.second().intValue();
     int denomDecimal = value.fourth().intValue();
     result += ""+(denomDecimal*intPart+numDecimal) + " / " + denomDecimal;
     return result;
   }

   public void init(ListExpr type, ListExpr value, QueryResult qr){
     qr.addEntry("rat : " + getValueString(value));
   }
 public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {
     String T = new String(type.symbolValue());
     String V = getValueString(value);
     T=extendString(T,typewidth);
     V=extendString(V,valuewidth);
     qr.addEntry(T + " : " + V);
     return;
  }
}

