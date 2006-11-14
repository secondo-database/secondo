package tools;

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
            String repl = tools.GetEnv.getEnv(var);
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

 

}
