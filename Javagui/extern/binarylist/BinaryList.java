package extern.binarylist;

import sj.lang.*;
import java.io.*;
import extern.*;

public class BinaryList implements SecondoImporter{


public ListExpr getList(String FileName){
  File F = new File(FileName);
  if(!F.exists()){
     LastError = "file not found";
     return null;
  }
  LastError="no error";
  try{
     BufferedInputStream FIS = new BufferedInputStream(new FileInputStream(F),4096);
     if(FIS==null)
        LastError = "error in reading file";
     ListExpr Res = ListExpr.readBinaryFrom(FIS);
     try{FIS.close();}catch(Exception e2){}
     return Res;
     } catch(Exception E){
        LastError = "error in opening file";
        return null;
     }
}



public String getErrorString(){
   return LastError;
}


private String LastError;


}
