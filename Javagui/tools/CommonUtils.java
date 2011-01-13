package tools;

import java.io.*;

/** 
 *  
 **/
public class CommonUtils{

public static String expandVar(String source){
 int pos = 0;
 int index;
 while((index=source.indexOf("$",pos)) >=0){
     if(index<source.length()-1 && source.charAt(index+1)=='('){
          pos = source.indexOf(")",index+1);
          if(pos<0){
             pos = index+1;
          }else{
            String var = source.substring(index+2,pos-index);
            Reporter.writeInfo("expand variable "+ var);
            String repl = System.getenv(var);
            if(repl==null){
              Reporter.writeWarning("environment variable "+var+" used but not defined");
              repl="";
            }
            source = (source.substring(0,index)+repl+source.substring(pos+1)); 
            pos = index+repl.length();
          }
     }else{
         pos++;
     } 
 }
 return source;
}


/** simulates the __PRETTY_FUNCTION__ of C **/

public static String WHERE()
{
    //create exception and write its stack trace to a String
    StringWriter sw =new StringWriter();
    PrintWriter pw =new PrintWriter(sw);
    new Exception("printWhereAmI()").printStackTrace(pw);
    pw.close();
    String exceptionText = sw.toString();

    //skip through first two "at ..."
    for(int i=0;i<2;i++) {
    exceptionText = exceptionText.substring(exceptionText.indexOf("at ",1));
    }
    //clip off remaining stack trace
    return  exceptionText.substring(0,exceptionText.indexOf("at ",1));
}
 

}
