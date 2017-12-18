

package viewer.hoese.algebras;

import sj.lang.ListExpr;

// currently just an alias for text

public class Dspldocument extends Dspltext{

protected void computeType(ListExpr type, String Text) {
 if(type.listLength()!=2){
    super.computeType(type,Text);
    return;
 } 
 type = type.second();
 if(type.atomType()!=ListExpr.SYMBOL_ATOM){
    super.computeType(type,Text);
    return;
 }
 String t = type.symbolValue();
 if(t.equals("pdf")){
   Type = PDF_TYPE;
   return;
 } 
 // handle other type here
 super.computeType(type,Text);

}

}


