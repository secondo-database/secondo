package de.fernunihagen.dna.hoese.algebras;

import de.fernunihagen.dna.hoese.DsplGeneric;
import de.fernunihagen.dna.hoese.LEUtils;
import de.fernunihagen.dna.hoese.LabelAttribute;
import de.fernunihagen.dna.hoese.QueryResult;
import de.fernunihagen.dna.hoese.RenderAttribute;
import sj.lang.ListExpr;


public class Dsplreal extends DsplGeneric 
    implements LabelAttribute, RenderAttribute{

// this value is represented
private double value;
private String entry;
private boolean defined;


private String computeValue(ListExpr value){
 if(isUndefined(value)){
    defined = false;
    this.value = 0.0;
    return "undefined";
 } else{
    Double v = LEUtils.readNumeric(value);
    if(v==null){
       this.value =0.0;
       defined = false;
       return "<error>"; 
    } 
    else {
        defined=true;
        this.value = v.doubleValue();
        return ""+this.value;
    }
 }
}

public void init (String name, int nameWidth, int indent,ListExpr type,
                 ListExpr value, QueryResult qr)
{
  String T = name;
  String V = computeValue(value);
  T=extendString(T, nameWidth, indent);
  entry = (T + " : " + V);
  qr.addEntry(this);
}

public String toString(){
  return entry;
}

/** returns the maximum value **/
public double getMaxRenderValue(){
  return value;
}
/** returns the minimum value **/
public double getMinRenderValue(){
  return value;
}
/** returns the current value **/
public double getRenderValue(double time){
  return value;
}
/** returns true if this int is defined **/
public boolean mayBeDefined(){
   return defined;
}
/** returns tre if this integer is defined **/
public boolean isDefined(double time){
  return defined;
}
/** returns the label **/
public String getLabel(double time){
 return defined?""+value:"undefined";
}


}
