package extern.binarylist;

import sj.lang.*;
import java.io.*;
import extern.fileio.*;
import extern.*;

public class BinaryList implements SecondoImporter{

public static boolean writeBinaryTo(ListExpr LE,OutputStream OS){
  if(OS==null)
     return false;
  MyDataOutputStream   DOS=null;
  try{
     DOS = new MyDataOutputStream(OS);
     DOS.writeString("bnl");
     DOS.writeShort(1);
     DOS.writeShort(0);
     boolean ok = writeBinaryRec(LE,DOS);
     DOS.close();
     return ok;
   } catch(Exception e){
     e.printStackTrace();
     try{DOS.close();}catch(Exception e1){}
     return false;
   }
}

private static boolean writeBinaryRec(ListExpr LE, MyDataOutputStream OS){
   int AT = LE.atomType();
   byte B = (byte) AT;
   try{
      switch(AT){
          case ListExpr.BOOL_ATOM    : {OS.writeByte(B);
	                  boolean b = LE.boolValue();
			  byte value =(byte) (b?1:0);
			  OS.writeByte(value);
			  return true;
			  }
	  case ListExpr.INT_ATOM     : {OS.writeByte(B);
	                  int value = LE.intValue();
			  OS.writeInt(value);
			  return true;
			  }
	  case ListExpr.REAL_ATOM    :  {OS.writeByte(B);
	                   float value = LE.realValue();
			   OS.writeReal(value);
			   return true;
			   }
	  case ListExpr.STRING_ATOM  :  {OS.writeByte(B);
	                   String value = LE.stringValue();
			   OS.writeInt(value.length());
			   OS.writeString(value);
			   return true;
			   }
	  case ListExpr.SYMBOL_ATOM  :  {OS.writeByte(B);
	                   String value = LE.symbolValue();
			   OS.writeInt(value.length());
			   OS.writeString(value);
			   return true;
			   }
	  case ListExpr.TEXT_ATOM    :  {OS.writeByte(B);
	                   String value = LE.textValue();
			   int L = value.length();
			   OS.writeInt(L);
			   OS.writeString(value);
			   return true;
			  }

	  case ListExpr.NO_ATOM      : { OS.writeByte(B);
	                   int length = LE.listLength();
			   OS.writeInt(length);
                           while(!LE.isEmpty()){
                             if(! writeBinaryRec(LE.first(),OS)) // error in writing sublist
			         return false;
                             LE=LE.rest();
			   }
                           return true;
	                  }
	  default      : return false;
      }
   }
   catch(Exception e){
     return false;
   }

}

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
     return readBinaryFrom(FIS);
     } catch(Exception E){
        LastError = "error in opening file";
        return null;
     }      

}

public ListExpr readBinaryFrom(InputStream In){
 try{
    MyDataInputStream DIN = new MyDataInputStream(In);
    String Sig = DIN.readString(3);
    int major = DIN.readShort();
    int minor = DIN.readShort();
    if(!Sig.equals("bnl") || major!=1 || minor!=0){
      try{DIN.close();}catch(Exception e){}
      System.out.println("wrong signatur or version "+Sig+"  "+major+"."+minor);
      return null;
    }
    ListExpr LE = readBinaryRec(DIN);
    DIN.close();
    return LE;
 }
 catch(Exception e){
   return null;
 }
}




private  ListExpr readBinaryRec(MyDataInputStream in){

try{
  int Type = in.readByte();
  switch(Type){
      case ListExpr.BOOL_ATOM    : {  return ListExpr.boolAtom(in.readBool()); }
	  case ListExpr.INT_ATOM     : {  return ListExpr.intAtom(in.readInt());}
	  case ListExpr.REAL_ATOM    : {  return ListExpr.realAtom(in.readReal());}
	  case ListExpr.STRING_ATOM  : {  int length = in.readInt();
	                                 return ListExpr.stringAtom(in.readString(length));
	                               }  
	  case ListExpr.SYMBOL_ATOM  : {  int length = in.readInt();
	                                 return ListExpr.symbolAtom(in.readString(length));
	                               }
	  case ListExpr.TEXT_ATOM    : { int length = in.readInt();
	                                 return ListExpr.textAtom(in.readString(length));
	                               }

	  case ListExpr.NO_ATOM      : { int length=in.readInt();
	                        	                        
	                        if(length==0)
				                return ListExpr.theEmptyList();
				            ListExpr F = readBinaryRec(in);
				            if(F==null)
				               return null;
				            ListExpr LE = ListExpr.oneElemList(F);
				            ListExpr Last = LE;
				            ListExpr Next = null;
				            for(int i=1;i<length;i++){
				               Next = readBinaryRec(in);
                               if(Next==null)
				                   return null;
				               Last= ListExpr.append(Last,Next);
				            }
				            return LE;
	                       }
	  default      : return null;
  }

}
catch(Exception e){
  return null;
}

}

public String getErrorString(){
   return LastError;
}


private String LastError;

}
