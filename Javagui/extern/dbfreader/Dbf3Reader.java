package extern.dbfreader;


import sj.lang.*;
import extern.numericreader.*;
import java.io.*;
import extern.*;



// todo-list
// process memofields


public class Dbf3Reader implements SecondoImporter{


/** returns a ListExpr for given DBase File
    if a error is occurred null is returned
  */
public  ListExpr getList(String FileName){
  ErrorString ="NO_ERROR";
  File F = new File(FileName);
  if(!F.exists()){
     ErrorString="FILE_NOT_EXISTS";
     return null;
  }
  BufferedInputStream BR=null;
  try{
     BR = new BufferedInputStream(new FileInputStream(F));
     byte[] Buffer = new byte[32];
     BR.read(Buffer);
     // read the Main-Header
     DB3MainHeader MH = new DB3MainHeader();
     if(!MH.readFrom(Buffer)){
        try{ BR.close();}catch(Exception e){}
        ErrorString="WRONG_DBASE_III_HEADER";
	return null;
     }

     int FD_Length = MH.getHeaderLength()-32; // all Header-MainHeader
     Buffer = new byte[FD_Length];
     BR.read(Buffer);
     DB3RecordHeader RH = new DB3RecordHeader();
     RH.readFrom(Buffer);

     // read records
     Buffer = new byte[MH.getRecordLength()];

     ListExpr Res=null;
     ListExpr Last=null;
     boolean done = false;
     int no =0;
     while( !done){
         int readed = BR.read(Buffer);
         if (readed<Buffer.length){
	   done=true;
	 }
	 else{
	    DB3Record R = new DB3Record();
	    R.readFrom(Buffer,RH);
            if(!R.isDeleted()){
	       ListExpr N = R.getList();
	       if(Res==null){
	          Res = ListExpr.oneElemList(N);
		  Last = Res;
	       }else{
	          Last = ListExpr.append(Last,N);
	       }
	    } else{

	    }
	 }
     }
     BR.close();
     return ListExpr.twoElemList(RH.getTypeList(),Res);

  }
  catch(Exception e){
     try{
       if(BR!=null)
          BR.close();
     }catch(Exception e2){}
     ErrorString ="ERROR_IN_READING_FILE";
     System.err.println(e);
     e.printStackTrace();
     return null;
  }

}

public void setMaxStringLength(int len){
 if(len>0)
    MAX_STRING_LENGTH = len;
}


public String getErrorString(){
  return ErrorString;
}

private static String ErrorString = "NO_ERROR";

/** replaces the doublequots in S by single qouts */
private String correctString(String S){
  return S.replace('\"','\'');
}



/* the main header contains informations but no
   fielddescriptions */
private class DB3MainHeader{

public boolean readFrom(byte[] Buffer){
   if(Buffer.length!=32){
      return false;
   }
   byte tmpver =   Buffer[0];
   if(tmpver!=3 && tmpver!=131){  // not a dbase 3 file
      return false;
   }
   version = tmpver;
   NumberOfRecords = NumericReader.getIntLittle(Buffer,4);
   HeaderLength = NumericReader.getShortLittle(Buffer,8);
   RecordLength = NumericReader.getShortLittle(Buffer,10);
   return true;
}
public int getVersion(){ return version;}
public boolean hasMemo(){ return version==131;}
public int getRecordNumber(){ return NumberOfRecords;}
public int getHeaderLength(){ return HeaderLength;}
public int getRecordLength(){ return RecordLength;}
private byte version=-1;
private int NumberOfRecords=0;
private int HeaderLength=0;
private int RecordLength=0;
}

/* the FieldDescription for a single Field */
private class DB3FieldDescription{

public boolean readFrom(byte[] Buffer){
  if(Buffer.length!=32)
     return false;
  Name = "";
  for(int i=0;i<11;i++)
     Name +=(char)Buffer[i];
  Type = Buffer[11];
  FieldLength = NumericReader.getInt(Buffer[16]);
  AfterComma =  NumericReader.getInt(Buffer[17]);
  if(Type=='M')
    FieldLength=10;
  else if(Type=='L')
      FieldLength=1;
  else if(Type=='D')
    FieldLength=8;

  Name = Name.trim();
  return true;
}

public ListExpr getTupleList(){
  if(Type=='C')
     if(FieldLength>MAX_STRING_LENGTH)
	 return ListExpr.twoElemList(ListExpr.symbolAtom(Name), ListExpr.symbolAtom("text"));
     else
         return ListExpr.twoElemList(ListExpr.symbolAtom(Name),ListExpr.symbolAtom("string"));
  if(Type=='N')
     if(AfterComma>0)
         return ListExpr.twoElemList(ListExpr.symbolAtom(Name),ListExpr.symbolAtom("real"));
     else
        return ListExpr.twoElemList(ListExpr.symbolAtom(Name),ListExpr.symbolAtom("int"));
  if(Type=='L')
     return ListExpr.twoElemList(ListExpr.symbolAtom(Name),ListExpr.symbolAtom("bool"));
  if(Type=='D')
     return ListExpr.twoElemList(ListExpr.symbolAtom(Name),ListExpr.symbolAtom("string"));
  if(Type=='M')
     return ListExpr.twoElemList(ListExpr.symbolAtom(Name),ListExpr.symbolAtom("string"));
   if(Type=='F')
     return ListExpr.twoElemList(ListExpr.symbolAtom(Name),ListExpr.symbolAtom("real"));
  return null;
}


public String getName(){return Name;}
public void setName(String newName){Name = newName;}
public byte getType(){ return Type;}
public int getFieldLength(){ return FieldLength;}
public int getAfterComma(){ return AfterComma;}
public String toString(){
   return Name+" "+((char)Type)+" "+FieldLength+"."+AfterComma;
}

private String Name;
private byte Type;
private int FieldLength;
private int AfterComma;
}


/* FieldDescriptions for all Fields */
private class DB3RecordHeader{

public boolean readFrom(byte[] Buffer){
  int N = (Buffer.length-1)/32;
  if(N<0) return false;
  FDs = new DB3FieldDescription[N];
  byte[] FD = new byte[32];
  for(int i=0;i<N;i++){
     for(int j=0;j<32;j++)
        FD[j] = Buffer[i*32+j];
     FDs[i] = new DB3FieldDescription();
     FDs[i].readFrom(FD);
  }
  // Because we have found some corupt data, i.e. data which have the 
  // same name used for different attributes, we change the names existing
  // more than one time
  for(int i=1;i<FDs.length;i++){
    String CurrentName = FDs[i].getName();
    int Variant = 0;
    int j=0;
    while(j<i){
       if(FDs[j].getName().equals(CurrentName)){ // name is used
           CurrentName = FDs[i].getName()+"_"+(Variant++); // set new Name
           j = 0; // search from the beginning
       }else{
          j++;
       } 
    }
    // CurrentName now contains a unused name
    FDs[i].setName(CurrentName);
  }
  return true;
}

public String toString(){
  StringBuffer res = new StringBuffer();
  for(int i=0;i<FDs.length;i++)
     res.append(i+": "+FDs[i]+"\n");
  return res.toString();
}


public ListExpr getTypeList(){
   if(FDs==null){
      return null;
   }
   ListExpr TupleList=ListExpr.oneElemList(FDs[0].getTupleList());
   ListExpr Last = TupleList;
   ListExpr AttrList;
   for(int i=1;i<FDs.length;i++){
      AttrList = FDs[i].getTupleList();
      Last = ListExpr.append(Last,AttrList);
   }
   ListExpr RelList = ListExpr.twoElemList(ListExpr.symbolAtom("tuple"),TupleList);
   return ListExpr.twoElemList(ListExpr.symbolAtom("rel"),RelList);
}


public DB3FieldDescription getFieldAt(int index){
  if(index<0 || FDs==null || index>=FDs.length)
     return null;
  return FDs[index];
}

public int getFieldNumber(){
   if(FDs==null)
       return -1;
   else
       return FDs.length;
}
private DB3FieldDescription[] FDs=null;
}


private class DB3Record{

public boolean readFrom(byte[] Buffer,DB3RecordHeader RH){
   int MaxField = RH.getFieldNumber();
   if(MaxField<0)
      return false;
   boolean isDeleted = Buffer[0]=='*';
   ListExpr TMP=null;
   ListExpr Last=null;
   ListExpr Next=null;
   int CurrentPos = 1;  // after delete marker
   DB3FieldDescription FD;
   for(int i=0;i<MaxField;i++){
      FD = RH.getFieldAt(i);
      byte Type = FD.getType();
      int length = FD.getFieldLength();
      Next = ListExpr.theEmptyList();
      if(Type=='C'){
        String S = "";
	for(int k=0;k<length;k++)
	   S += (char)Buffer[CurrentPos+k];
        S = S.trim();
	if(length>MAX_STRING_LENGTH)
	   Next = ListExpr.textAtom(S);
	else
	   Next = ListExpr.stringAtom(correctString(S));
      }

    if(Type=='N' | Type=='F'){
        String S = "";
	for(int k=0;k<length;k++)
	      S += (char)Buffer[CurrentPos+k];

	S = S.trim();
	if(FD.getAfterComma()>0 | Type=='F'){
	  try{
            Next = ListExpr.realAtom( Float.parseFloat(S));
	  } catch(Exception e){
            Next = ListExpr.realAtom(0.0F);
	  }
	}else{
	  try{
	    Next = ListExpr.intAtom(Integer.parseInt(S));
	    }
	  catch(Exception e){
	     Next = ListExpr.intAtom(0);
	  }
	}
      }

      if(Type=='L'){
         byte V = Buffer[CurrentPos];
	 if(V=='j' | V=='J' | V=='T' | V=='t' | V=='Y' | V=='y')
	    Next = ListExpr.boolAtom(true);
	 else
	    Next = ListExpr.boolAtom(false);
      }

      if(Type=='D'){
        String S = "";
	for(int k=0;k<length;k++)
	   S += (char)Buffer[CurrentPos+k];
        Next = ListExpr.stringAtom(correctString(S));
      }

      if(Type=='M'){
         Next = ListExpr.stringAtom("memo");
      }

      CurrentPos += length;
      //System.err.println("add Entry"+Next.writeListExprToString());
      if(TMP==null) {  // the first entry
        TMP = ListExpr.oneElemList(Next);
	Last = TMP;
      }else{
         Last = ListExpr.append(Last,Next);
      }
   }
   LE = TMP;
   return true;
}

public ListExpr getList(){
  return LE;
}

public boolean isDeleted(){
   return isDeleted;
}



private ListExpr LE=null;
private boolean isDeleted=false;

}

private static int MAX_STRING_LENGTH = 48;

}
