package extern.shapereader;

//import sj.lang.ListExpr;
import java.io.*;
import java.util.*;
import sj.lang.*;
import extern.*;

public class ShapeReader implements SecondoImporter{



public ListExpr getList(String FileName){
   Last_Error = "No_Error";
   File F = new File(FileName);
   if(! openFile(F)){
      Last_Error ="error in open file";
      return null;
   }
   String SecondoType = ShapeType.getSecondoName(Head.getShapeType());
   ListExpr TypeList =
        ListExpr.twoElemList( ListExpr.symbolAtom("rel"),
	ListExpr.twoElemList( ListExpr.symbolAtom("tuple"),
	        ListExpr.twoElemList(
		     ListExpr.twoElemList( ListExpr.symbolAtom("recordnumber"),
		                           ListExpr.symbolAtom("int")),
		     ListExpr.twoElemList( ListExpr.symbolAtom("shape"),
		                           ListExpr.symbolAtom(SecondoType)))));

   ListExpr ValueList=null;
   ListExpr Last = null;
   ListExpr LE;
   RecordHeader RH = readRecordHeader();
   Record R = new Record();
   while(RH!=null){
      byte[] Buffer = readNextRecord(RH);
      if(Buffer==null){
        RH=null;
      }else{
          if(!R.readFrom(Buffer)){
            RH =null;
	  } else{
	     LE = R.getValueList();
             LE = ListExpr.twoElemList(ListExpr.intAtom(RH.getRecordNumber()),LE);
             if(ValueList==null){
	        ValueList = ListExpr.oneElemList(LE);
		Last = ValueList;
	     }else{
	       Last = ListExpr.append(Last,LE);
	     }
	     RH = readRecordHeader();
	  }
      }
   }
   closeFile();
   if(ValueList==null)
      ValueList = ListExpr.theEmptyList();
   return ListExpr.twoElemList(TypeList,ValueList);

}


public String getErrorString(){
  return Last_Error;
}



private boolean openFile(File F){
 if(!F.exists()){
    Last_Error="FILE_NOT_FOUND";
    return false;
 }
 else
    try{
       if(F.length()<MIN_FILESIZE){
          Last_Error = "WRONG_FILESIZE";
          return false;
       }
       FIS = new BufferedInputStream(new FileInputStream(F));
       opened = true;
       return readHeader();
    }
    catch(Exception E){
       Last_Error = "i can't open the File";
       return false;
    }
}


private boolean closeFile(){
  try{
     FIS.close();
     opened = false;
     return true;
  }
  catch(Exception e){
    return false;
  }
}




private boolean readHeader(){

   Head = new ShapeHeader();
  try{
   byte[] Buffer = new byte[100];
   int readed = FIS.read(Buffer);
   if(readed!=Buffer.length){
      System.err.println("Shapeheader not correct readed");
      return false;
   }
   if(!Head.readFrom(Buffer)){
      System.err.println("error in Reading ShapeHeader");
      return false;
   }
   }catch(Exception e){
     System.err.println(e);
     Last_Error = "ERROR_IN_READING_FILE";
     return false;
   }
   return true;
}


private RecordHeader readRecordHeader(){
   RecordHeader Res = null;
   try{
     byte[] Buffer = new byte[8];
     FIS.read(Buffer);
     Res = new RecordHeader();
     Res.readFrom(Buffer);
   }catch(Exception e){
     System.err.println(e);
     Last_Error = "ERROR_IN_READING_FILE";
   }
   return Res;
}



private byte[] readNextRecord(RecordHeader RH){
   try{
      byte[] Buffer = new byte[RH.getContentLength()*2];
      if(FIS.read(Buffer)!=Buffer.length){
         System.err.println("Buffer not complete loaded");
         return null;
      }
      return Buffer;
   }catch(Exception e){
       Last_Error = "ERROR_IN_READING_FILE";
       return null;
   }
}

private BufferedInputStream FIS;


private boolean opened= false;
private ShapeHeader Head;

private static long MIN_FILESIZE = 100; // size of shapeheader;

private String Last_Error ="No_Error";




}
