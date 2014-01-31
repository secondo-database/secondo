

package viewer.hoese.algebras;

import sj.lang.ListExpr;
import viewer.hoese.*;


/** Class representing a precise value **/

public class Dsplprecise  extends DsplGeneric implements  RenderAttribute, LabelAttribute {

  public static boolean isDigit(char c){
     return c>='0' && c<='9';
  }

  public static int getDigit(char c){
     return (int) (c - '0');
  }

  public static Long getInt(ListExpr e){

     if(e.atomType()==ListExpr.INT_ATOM){
        return new Long(e.intValue());
     }
     if(e.listLength()!=2){
        return null;
     }
     if(  (e.first().atomType()!=ListExpr.INT_ATOM )
        ||(e.second().atomType()!=ListExpr.INT_ATOM)){
       return null;
     }
     long r = (e.first().intValue() << 32) | e.second().intValue();
     return new Long(r);
  }

  public static Double getFrac(ListExpr f){
      if(f.atomType()!=ListExpr.TEXT_ATOM){
         return null;
      }
      String s = f.textValue();
      return getFrac(s);
  }

  public static Double getFrac(String s){
     double num = 0;
     double denom = 0;
     boolean inDenom = false;
     for(int i=0;i<s.length();i++){
        char c = s.charAt(i);
        if(c=='/'){
           inDenom = true;
        } else if(isDigit(c)){
           int d = getDigit(c);
           if(inDenom){
              denom = denom*10 + d;
           } else {
              num = num*10 + d; 
           }
        } else {
           return null;
        }
     }
     if(!inDenom){
       return num;
     }
     if(denom==0){
        return null;
     }
     return num/denom;

  }

  public  static Double getDouble(ListExpr le, boolean scaleIncluded){
     int scale = 1;
     if(scaleIncluded){
         if(le.listLength()!=2){
            return null;
         }
         if(le.first().atomType()!=ListExpr.INT_ATOM){
            return null;
         } 
         scale = le.first().intValue();
         if(scale<=0){
            return null;
         }
         le = le.second();
     }
     if(le.atomType()==ListExpr.INT_ATOM){
        return new Double(le.intValue());
     }


     int len = le.listLength();
     if(len<1 || len>2){
        return null;
     }
     ListExpr grid = le.first();
     Long gd = getInt(le.first());
     if(gd==null){
         return null;
     }
     double res = gd.longValue();
     if(le.listLength()==2){
       Double frac = getFrac(le.second());
       if(frac==null){
         return null;
       }
       res += frac.doubleValue();
     }
     res = res/scale;
     return  new Double(res);
  }




  public double getMaxRenderValue(){
    return value;
  }
  public double getMinRenderValue(){
    return value;
  }
  public double getRenderValue(double time){
    return value;
  }
  public boolean mayBeDefined(){
     return defined;
  }
  public boolean isDefined(double time){
     return defined;
  }

  public String getLabel(double time){
     return label;
  }

  public void init (String name, int nameWidth, int indent, ListExpr type, ListExpr value, QueryResult qr) {
    if(isUndefined(value)){
      defined = false;
      this.value = 0.0;
      label = "undefined";
    } else {
        Double d = getDouble(value,true);
        if(d==null){
          this.value = 0.0;
          label = "error";
          defined = false;
        } else {
           this.value = d.doubleValue();
           label = d.toString();
           defined = true;
        }
    }
    entry = extendString(name,nameWidth, indent) +" : "+ label;
    qr.addEntry(this);
    return;
  }

  public String toString(){
     return entry;
  }


  private double value;
  private String label;
  private String entry;
  private boolean defined;
}



