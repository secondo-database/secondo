/*
This class repersensts the implementation of nested lists in Java.
The implementation here uses the Java version of the Berkeley DB
for persistence.

*/

package sj.lang;

import java.io.*;
import tools.Base64Decoder;
import  java_cup.runtime.Symbol;
import  sj.lang.JavaListExpr.NLParser;


public class ListExpr{

/*
1 Public part

1.1 Definition of constants decribing atom types


*/

public final static byte NO_ATOM = 0;          // not an atom.
public final static byte INT_ATOM = 1;         // int atom.
public final static byte REAL_ATOM = 2;        // real atom.
public final static byte BOOL_ATOM = 3;        // Bool atom.
public final static byte STRING_ATOM = 4;      // String atom.
public final static byte SYMBOL_ATOM = 5;      // Symbol atom.
public final static byte TEXT_ATOM = 6;        // Text atom


/*
1.2 Constants decribing codings for binary writing of this list

*/

public final static int BIN_LONGLIST = 0;
public final static int BIN_INTEGER  = 1;
public final static int BIN_REAL = 2;
public final static int BIN_BOOLEAN = 3;
public final static int BIN_LONGSTRING = 4;
public final static int BIN_LONGSYMBOL = 5;
public final static int BIN_LONGTEXT = 6;
public final static int BIN_LIST = 10;
public final static int BIN_SHORTLIST = 11;
public final static int BIN_SHORTINT  = 12;
public final static int BIN_BYTE = 13;
public final static int BIN_STRING = 14;
public final static int BIN_SHORTSTRING = 15;
public final static int BIN_SYMBOL= 16;
public final static int BIN_SHORTSYMBOL = 17;
public final static int BIN_TEXT = 18;
public final static int BIN_SHORTTEXT=19;


/*
1.3 Construction of ListExpr

*/

/**
1.3.0 Initialize function

This function must be called before the first ListExpr is created.
Otherwise this function will have no effect.

**/
public static boolean initialize(int cachesize){
   if(dbarray!=null){
      System.err.println("ListExpr initialize already called.");
      System.err.println("The call with cachesize ="+cachesize+" will have no effect.");
      return false;
   }
   if(cachesize < MINCACHESIZE)
       cachesize = MINCACHESIZE;
   System.out.println("initialize Cache with "+cachesize +"elements");
   /* we have make the choice 30 as maxdatasize.
      So all non-string-types are cached. furthermore all string with a 
      length upto 30 characters are also chached. 
   */
   dbarray = new DBArray(cachesize,30); 
   return true;
}

/**
1.3.1 A Constructor

This constructor creates a new empty list.

**/

public ListExpr(){
   this(NO_ATOM,0,0);
}

/** 
1.3.2 Creation of a boolean atom

Creates a new listnode representing a boolean value. 

*/
public static ListExpr boolAtom(boolean value){
   int v = value?1:0;
   return new ListExpr(BOOL_ATOM,v,0);
}

/**
1.3.3 Creation of an integer atom 

The next method creates a new integer atom.

*/
public static ListExpr intAtom(int value){
    return new ListExpr(INT_ATOM,value,0);
}

/**
1.3.4 Creation of a real atom

This method creates a new real atom with the given value.

*/
public static ListExpr realAtom(double value){
    long v = Double.doubleToLongBits(value);
    return new ListExpr(REAL_ATOM,v,0);
}

/*
1.3.5 Creating a string atom

*/
public static ListExpr stringAtom(String value){
    return stringValuedAtom(value,STRING_ATOM);
}

/*
1.3.6 Creating a symbol atom

*/
public static ListExpr symbolAtom(String value){
    return stringValuedAtom(value,SYMBOL_ATOM);
}

/*
1.3.7 Creating a text atom

*/
public static ListExpr textAtom(String value){
    return stringValuedAtom(value,TEXT_ATOM);
}

/*
1.3.8 Creating an empty text atom

*/
public static ListExpr textAtom(){
    return stringValuedAtom("",TEXT_ATOM);
}

/**
1.3.9 Creating an empty list

*/
public static ListExpr theEmptyList(){
  return new ListExpr(); 
}


/*
1.4 Getting values from ListExpr

*/


/** 
1.4.1 Getting the value of a boolean atom.

This method can only called if this node is of 
type BOOL_ATOM.

*/
public boolean boolValue(){
   if(CHECK_PRECONDITIONS){
       if(atomType()!=BOOL_ATOM){
          System.err.println("Error in ListExpr.boolValue: list is not a bool atom");
       }
   }
   return getValue()!=0;
}

/*
1.4.2 Getting the value of an int atom 

*/
public int intValue(){
   if(CHECK_PRECONDITIONS){
       if(atomType()!=INT_ATOM){
          System.err.println("Error in ListExpr.boolValue: list is not a bool atom");
       }
   }
   return (int)getValue();
}

/*
1.4.3 Getting the value of an real atom

*/
public double realValue(){
   if(CHECK_PRECONDITIONS){
       if(atomType()!=REAL_ATOM){
          System.err.println("Error in ListExpr.boolValue: list is not a bool atom");
       }
   }
   return Double.longBitsToDouble(getValue());
}

/*
1.4.4 Getting the value of an string atom

*/
public String stringValue(){
  return stringLikeValue(STRING_ATOM);
}

/*
returns  the value of an symbol atom

*/
public String symbolValue(){
   return stringLikeValue(SYMBOL_ATOM);
}

/*
Getting the value of an text atom 

*/
public String textValue(){
   return stringLikeValue(TEXT_ATOM);
}

/*
Checking of last element

*/
public boolean endOfList(){
  boolean res = atomType()==NO_ATOM && !isEmpty();
  if (!res) return res;
  ListExpr N = new ListExpr(getNext());
  return N.isEmpty();  
}

/*
Getting the type of this node

*/
public int atomType(){
   return data[0];
}

/*
Checks whether this node represents an empty list 

*/
public boolean isEmpty(){
   return (getValue()==0) && (getNext()==0) && (atomType()==NO_ATOM);
}

/*
Checks for atom property of this node 

*/
public boolean isAtom(){
   return atomType()!=NO_ATOM;
}

/** Checks whether this List in in MainMemory 
  */
public boolean isInMemory(){
    return dbarray.isCached(key);
}

/*
The familiar Equals method.

*/
 public boolean equals(Object o){
    if(o==null)
        return false;
    if(!(o instanceof ListExpr))
       return false;
    ListExpr LE = (ListExpr)o;

    if(!equalValues(LE)) // check type and value of the first element
       return false;
    if(atomType()!=NO_ATOM)
       return true;
    long TNkey = getNext();
    long LENkey = LE.getNext();
    while(TNkey!=0  && LENkey!=0){
         ListExpr TN = new ListExpr(TNkey);
         ListExpr LEN = new ListExpr(LENkey);         
         if(!TN.equalValues(LEN)) // differences found
             return false;
         if(TN.atomType()==NO_ATOM){
            TNkey  = TN.getNext();
            LENkey = LEN.getNext();
         } else{
            TNkey=0;
            LENkey=0;
         }
    }
    if(TNkey!=0 || LENkey!=0) // different lengths
       return false;
    return true;
}


/*
1.5 Connecting Lists 

*/

/**
1.5.1 append

This function appends a new node to an existing list. 
The list must be the last element of a non-empty list.

*/
public static ListExpr append(ListExpr lastElement,ListExpr newSon){
    if(DEBUG_MODE){
       if(lastElement==null || newSon==null){
          System.err.println("ListExpr.append called with an null argument");
          if(lastElement==null) System.err.println("lastElement==null");
          if(newSon==null) System.err.println("newSon==null");
          new Throwable().printStackTrace();
       }
    }
    if(CHECK_PRECONDITIONS) {
      if ((!lastElement.endOfList()) || lastElement.isEmpty() || lastElement.isAtom()) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the append() method:");
        System.err.println(" the input argument ~lastElement~ is not the end of a list.");
       }
    }
    ListExpr p = new ListExpr(NO_ATOM,newSon.key,(new ListExpr()).key);
    lastElement.setNext(p.key);
    lastElement.update();
    return p;                
}

 /*
   3.4.3 The cons() method.
   This method construct a new ListExpr which is the union of the argument lists (being
   ~left~ and ~right~ the left and right part respectively) and returns it.
   *Preconditions:*
   ~right~ can not be an Atom.
 */
  public static ListExpr cons (ListExpr left, ListExpr right) {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if(DEBUG_MODE){
       if(left==null || right==null){
           System.err.println("ListExpr.cons called with an null argument");
           if(left==null) System.err.println("Left==null");
           if(right==null) System.err.println("Right==null");
           new Throwable().printStackTrace();
       }
    }
    if (ListExpr.CHECK_PRECONDITIONS) {
      if(right.isAtom()) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the cons() method:");
        System.err.println(" the input argument ~right~ does not fulfil the preconditions.");
      }
    }
    ListExpr result = new ListExpr(NO_ATOM,left.key,right.key);
    return  result;
  }

 /*
   3.4.18 The oneElemList(), twoElemList(), threeElemList(), fourElemList(), fiveElemList()and sixElemList() methods.
   These methods construct a ListExpr with one, two, three, four, five and six elements, respectively.
   */
  public static ListExpr oneElemList (ListExpr elem1) {
    return  cons(elem1, theEmptyList());
  }

  public static ListExpr twoElemList (ListExpr elem1, ListExpr elem2) {
    return  cons(elem1, cons(elem2, theEmptyList()));
  }

  public static ListExpr threeElemList (ListExpr elem1, ListExpr elem2, ListExpr elem3) {
    return  cons(elem1, cons(elem2, cons(elem3, theEmptyList())));
  }

  public static ListExpr fourElemList (ListExpr elem1, ListExpr elem2, ListExpr elem3,
      ListExpr elem4) {
    return  cons(elem1, cons(elem2, cons(elem3, cons(elem4, theEmptyList()))));
  }

  public static ListExpr fiveElemList (ListExpr elem1, ListExpr elem2, ListExpr elem3,
      ListExpr elem4, ListExpr elem5) {
    return  cons(elem1, cons(elem2, cons(elem3, cons(elem4, cons(elem5, theEmptyList())))));
  }

  public static ListExpr sixElemList (ListExpr elem1, ListExpr elem2, ListExpr elem3,
      ListExpr elem4, ListExpr elem5, ListExpr elem6) {
    return  cons(elem1, cons(elem2, cons(elem3, cons(elem4, cons(elem5, cons(elem6,
        theEmptyList()))))));
  }

/*
Accessing Lists

*/ 
 
  public ListExpr first(){
     return nThElement(1);
  } 
  public ListExpr second(){
     return nThElement(2);
  } 
  public ListExpr third(){
     return nThElement(3);
  }

  public ListExpr fourth(){
     return nThElement(4);
  }

  public ListExpr fifth(){
     return nThElement(5);
  }
  public ListExpr sixth(){
     return nThElement(6);
  }

/*
Return the rest of a List. The list can't be an atom and the
list must have an successor.

*/
  public ListExpr rest(){
      if(atomType()!=NO_ATOM)
          return null;
      long n = getNext();
      if(n==0) return null;
      return new ListExpr(n);
  }

/*
   3.4.9 The listLength() method.
   This method returns the number of elements of this ListExpr object.
   WARNING: this methods needs a list transversal searching and therefore
   it spends a time proportional to the length of the list.
   */
  public int listLength () {
    int length = 0;
    ListExpr aux = this;
    // If it is not an atom, it must return the the number of elements.
    if (aux.isAtom()) {
      // if it is an atom, it must return -1.
      return  -1;
    }
    // If it is not an atom.
    while (!aux.isEmpty()) {
      length++;
      aux = new ListExpr(aux.getNext());
    }
    return  length;
  }




/*
Destroying Lists

*/
public void destroy(){
   int at = atomType();
   if(at!=NO_ATOM){
       destroyValue();
       return;
   }
   long next = getNext();
   while (next!=0){
      ListExpr LE = new ListExpr(next);
      next = LE.getNext();
      LE.destroyValue();
   }
   destroyValue(); 
}




/*
2 Private Part

This part is for internal use only.

2.1 Some constants

*/

private static boolean CHECK_PRECONDITIONS=true;
private final static int NO_ERROR_CODE = 0;
private final static int INPUT_STRING_ERROR = -1;
private final static int WRITE_TO_FILE_ERROR = -1;
private final static int READ_FROM_FILE_ERROR = -1;
private final static String BEGIN_TEXT ="<text>";
private final static String END_TEXT ="</text--->";

/*
2.2 Creation of ListExpr

*/
/**
This constructor creates a new ListExpr with the given atomtype,
a given value and the key for the next element in a list.
The new list gets a unsused key.   

**/
private ListExpr(byte atomtype, long value,long next){
   this(atomtype,value,next,getNextKey());
}

/**
This constructor creates a new ListExpr from its arguments.
 
*/
private ListExpr(byte atomtype, long value,long next,long key){
   if(dbarray==null){
      initialize(MINCACHESIZE);
   }
   this.data = getBytes(atomtype,value,next);
   this.key = key;
   dbarray.put(key,data);
}

/**
This constructor creates a new ListExpr from an entry in the database.
If an error occurs, the system will be stopped.

*/
private ListExpr(long key){
    this.key = key;
    this.data = dbarray.get(key);
}


/*
2.2 Modifiing an existing list 

*/
/*
2.2.1 getValue

This method extracts the value from the internal representation of 
the nested list.

*/
private long getValue(){
   long v = 0;
   byte[] b = data;
   for(int i=1;i<9;i++){
      v = (v<<8)+getPositiveInt(b[i]);
   }
   return v; 
} 

/*
2.2.2 setValue

This function puts a new value into the internal
representation.

*/
private void setValue(long v){
   for(int i=1;i<9;i++){
      data[9-i] = (byte)(v&255);
      v = v >> 8;
   }
} 

/*
2.2.3 getNext

Extracts the next information from the internal representation. 

*/
private long getNext(){
   long v = 0;
   for(int i=9;i<17;i++)
      v = v*256+getPositiveInt(data[i]);
   return v; 
} 

/*
2.2.4 setNext

Sets a new next entry for this listExpr.

*/ 
private void setNext(long v){
   for(int i=9;i<17;i++){
      data[17-i] = (byte)(v&255);
      v = v << 8;
   }
} 

/**
  * Checks for euqality of the value
  **/
 private boolean equalValues(ListExpr LE){
    if(LE==null) return false; // this is not null

    int a1 = atomType();
    int a2 = LE.atomType();

    if(a1!=a2) // different types
       return false;

    long v1 = getValue();
    long v2 = getValue();

    if(v1==0  && v2!=0)
       return false;
    if(v1!=0 && v2==0)
       return false;

    if(v1==0 && v2==0)
       return true;
    else{
       // check numric values directly
       if(a1==REAL_ATOM || a1==INT_ATOM){
         return v1==v2; 
       }else if(a1==NO_ATOM){ // check in the depth
          return (new ListExpr(v1)).equals(new ListExpr(v2));
       }else{ // one of the text types
          // for string values we misuse the next pointer as 
          // length
          if(getNext()!=LE.getNext())
             return false;
          // load the appropriate strings and compare them
          String S1 = getText(v1);
          String S2 = getText(v2);
          return S1.compareTo(S2)==0;  
           
       } 
    }
}

/*
2.2.5 update

Synchonizes this value with the values in the dbarray.

*/
private void update(){
   dbarray.put(key,data);
}

/*
2.2.6 Destroying lists

*/
private void destroyValue(){
   int at = atomType();
   switch(at){
      case INT_ATOM :{
            dbarray.delete(key); 
            break;
      }
      case REAL_ATOM :{
            dbarray.delete(key); 
            break;
      }
      case BOOL_ATOM :{
            dbarray.delete(key); 
            break;
      }
      case SYMBOL_ATOM :{
          long v = getValue();
          dbarray.delete(v);
          dbarray.delete(key); 
      }
      case STRING_ATOM :{
          long v = getValue();
          dbarray.delete(v);
          dbarray.delete(key); 
      }
      case TEXT_ATOM :{
          long v = getValue();
          dbarray.delete(v);
          dbarray.delete(key); 
      }
      case NO_ATOM:{
         long v = getValue(); // detroy the value
         (new ListExpr(v)).destroy();
         dbarray.delete(key); // destroy itself
      }
      default: {
          System.err.println("DestroyValue: unknown atomtype detected");
      }
   }
}

/*
This methods returns the nth element from a list.

*/
  private ListExpr nThElement(int n) throws IllegalArgumentException{
      if(n<1) throw new IllegalArgumentException("illegal listElem number " + n);
      int i=1;
      ListExpr tmp = this;
      while(i<n){
        tmp = new ListExpr(tmp.getNext());
        i++;
      }
      return new ListExpr(tmp.getValue());
  } 
//*********************************************************************************************


/** Appends the given text to an existing text atom */
  public void appendText(String text){
     if(CHECK_PRECONDITIONS){
        if(atomType()!=TEXT_ATOM){
           System.err.println("Error in Checking preconditions. ListExpr::appendText");
           System.err.println("List is not a text");
           return;
        }
     }
     if(text==null) return; // nothing to do
     if(text.equals("")) return; // also nothing to do
     long v;
     String  NewText = getText(v=getValue())+text;
     setValue(NewText.length());
     update(); // update this (length)
     try{
        if (! dbarray.put(v,NewText.getBytes("UTF-8"))){ // write new text into the dbarray
             if(DEBUG_MODE){
                System.err.println("Error in writing text to dbarray ");
              }
         }
     }catch(Exception e){
        if(DEBUG_MODE){
            e.printStackTrace();
        }
     }
  }

public void appendText (String text, int startPos, int length) {
     if(CHECK_PRECONDITIONS){
        if(atomType()!=TEXT_ATOM){
           System.err.println("Error in Checking preconditions. ListExpr::appendText");
           System.err.println("List is not a text");
           return;
        }
     }
     if(text==null) return;
     if(text.equals("")) return;
     text = text.substring(startPos,startPos+length);
     appendText(text);
}

public static void setDebugMode(boolean enabled){
   DEBUG_MODE=enabled;
}


public int getTextLength(){
   if(atomType()!=TEXT_ATOM){
      return 0;
   }
   return (int) getNext();
}

public int textLength(){
   return getTextLength();
}


public static String getDirectory(){
    return ParseDir;
}

/** We don't handle text atoms in main memory 
  * when not using them. 
  * This function is only needed for compatibility with
  * the non-persistent version of ListExpr.
  */
public static boolean setMaxInternalTextLength(int length){
   return true;
}

/** We don't handle text atoms in main memory 
  * when not using them. 
  * This function is only needed for compatibility with
  * the non-persistent version of ListExpr.
  */
public static void setTempDir(String Name){ }

/** This function has no effect in this implementation */
public static void usePersistentText(boolean enabled){}


public InputStream decodeText(){
      Base64Decoder BD = new Base64Decoder(new StringReader(textValue()));
      return BD.getInputStream();
}


/** In this version, we ignore length restrictions for String and symbol values 
  */
public static void setMaxStringLength(int len){ }

public void setText(String value){
  if(atomType()!=TEXT_ATOM){
     if(CHECK_PRECONDITIONS){
        System.err.println("precondition failed in ListExpr::setText");
        System.err.println("node is not of type text. ");
     }
     return;
   }
  long v = getValue();
  try{
      byte[] textData = value.getBytes("UTF-8"); 
      if(!dbarray.put(v,textData)){
          if(DEBUG_MODE){
             System.err.println("Error in putting text into dbarray");
          }
      }
      setNext(value.length());
      update(); 
   }catch(Exception e){
      if(DEBUG_MODE){
        e.printStackTrace();
      }
   }
}



public void setValueTo(ListExpr list){
   key=list.key;
   data= list.data;
}


private static ListExpr stringValuedAtom(String value,byte atomType){
 // first create a database entry containing the string
 long SKey = getNextKey();
 try{
    byte[] StringBytes = value.getBytes("UTF-8");
    if(!dbarray.put(SKey,StringBytes)){
        if(DEBUG_MODE){
           System.err.println("Error in putting text into dbarray ");
         }
         return null;
     }
 } catch(Exception e){
    if(DEBUG_MODE)
       e.printStackTrace();
    return null;
 }
 return new ListExpr(atomType,SKey,value.length());
}


private String stringLikeValue(byte atomtype){
  if(atomType()!=atomtype){
     if(CHECK_PRECONDITIONS){
        System.err.println("precondition failed in ListExpr::stringLikeValue");
        System.err.println("List is not of the appropriate type ");
     }
     return null;
  }
  long v = getValue();
  return getText(v);
}

public byte[] textValueAsByteArray(){
  if(atomType()!=TEXT_ATOM){
     if(CHECK_PRECONDITIONS){
        System.err.println("precondition failed in ListExpr::textValueAsByteArray");
        System.err.println("List is not of the appropriate type ");
     }
     return null;
  }
  long v = getValue();
  return getTextBytes(v);
}

/** Returns an InputStream for accessing the content of a text atom.
  * If an error occurs or the list is not an InputSTream, null is
  * returned.
  **/
public InputStream textValueAsInputStream() throws IOException{
  byte[] b = textValueAsByteArray();
  if(b==null) return null;
  return new ByteArrayInputStream(b);
}




/** This function returns the byte representation of the values. 
  */ 
private static byte[] getBytes(byte atomType, long value, long next){
    byte[] result = new byte[17];
    result[0] = atomType;
    long v2 = value;
    for(int i=0;i<8;i++){
        result[8-i]=(byte)(v2&255);
        v2=v2>>8;
    }

    v2=next;
    for(int i=0;i<8;i++){
        result[16-i]=(byte)(v2&255);
        v2=v2>>8;
    }
    return result;
}



public static ListExpr readBinaryFrom(InputStream In){
 try{
    MyDataInputStream DIN=null;
    if(In instanceof MyDataInputStream)
       DIN = (MyDataInputStream) In;
    else
       DIN = new MyDataInputStream(In);
    String Sig = DIN.readString(3);
    int major = DIN.readShort();
    int minor = DIN.readShort();
    if(!Sig.equals("bnl") ){
      System.err.println("wrong signatur or version "+Sig );
      return null;
    }
    if(major!=1 | ( minor!=0 & minor!=1)){
       System.err.println("wrong version numer "+major+"."+minor);
       return null;
    }

    ListExpr LE = readBinaryRec(DIN);
    return LE;
 }
 catch(Exception e){
   if(DEBUG_MODE){
      e.printStackTrace();
   }
   return null;
 }
}


/* returns an integer in range 0..255 */
private static int getPositiveInt(byte b){
  if(b<0)
    return 256+b;
  else
    return b;
}

private static int getPositiveInt(short s){
  if(s<0)
    return 65536+s;
  else
    return s;
}


private static  ListExpr readBinaryRec(MyDataInputStream in){

try{
  int Type = in.readByte();
  switch(Type){
      case BIN_BOOLEAN        : { return boolAtom(in.readBool()); }
      case BIN_BYTE           : { return intAtom(in.readByte());}
      case BIN_SHORTINT       : { return intAtom(in.readShort());}
      case BIN_INTEGER        : { return intAtom(in.readInt());}
      case BIN_REAL           : { return realAtom(in.readReal());}
      case BIN_SHORTSTRING    : { int len = getPositiveInt(in.readByte());
                                  return stringAtom(in.readString(len));
                                }
      case BIN_STRING         : { int len = getPositiveInt(in.readShort());
                                  return stringAtom(in.readString(len));
                                }
      case BIN_LONGSTRING     : { int len = in.readInt();
                                  return ListExpr.stringAtom(in.readString(len));
                                }
      case BIN_SHORTSYMBOL    : {  int len = getPositiveInt(in.readByte());
                                   return ListExpr.symbolAtom(in.readString(len));
                                }
      case BIN_SYMBOL         : {  int len = getPositiveInt(in.readShort());
                                   return ListExpr.symbolAtom(in.readString(len));
                                }
      case BIN_LONGSYMBOL     : {  int len = in.readInt();
                                   return ListExpr.symbolAtom(in.readString(len));
                                }
      case BIN_SHORTTEXT      : { int length = getPositiveInt(in.readByte());
                                  return ListExpr.textAtom(in.readString(length));
                                }
      case BIN_TEXT           : { int length = getPositiveInt(in.readShort());
                                  return ListExpr.textAtom(in.readString(length));
                                }
      case BIN_LONGTEXT       : { int length = in.readInt();
                                  return ListExpr.textAtom(in.readString(length));
                                }

      case BIN_SHORTLIST      : { int length= getPositiveInt(in.readByte());
                                  if(length==0)
                                     return ListExpr.theEmptyList();
                                  ListExpr F = readBinaryRec(in);
                                  if(F==null) // error in reading sublist
                                     return null;
                                  ListExpr LE = ListExpr.oneElemList(F);
                                  ListExpr Last = LE;
                                  ListExpr Next = null;
                                  for(int i=1;i<length;i++){
                                      Next = readBinaryRec(in);
                                      if(Next==null) // error in reading sublist
                                         return null;
                                      Last= ListExpr.append(Last,Next);
                                  }
                                  return LE;
                                }

      case BIN_LIST           : { int length= getPositiveInt(in.readShort());
                                  if(length==0)
                                     return ListExpr.theEmptyList();
                                  ListExpr F = readBinaryRec(in);
                                  if(F==null) // error in reading sublist
                                     return null;
                                  ListExpr LE = ListExpr.oneElemList(F);
                                  ListExpr Last = LE;
                                  ListExpr Next = null;
                                  for(int i=1;i<length;i++){
                                      Next = readBinaryRec(in);
                                      if(Next==null) // error in reading sublist
                                         return null;
                                      Last= ListExpr.append(Last,Next);
                                   }
                                   return LE;
                                }

      case BIN_LONGLIST      : { int length= in.readInt();
                                 if(length==0)
                                    return ListExpr.theEmptyList();
                                 ListExpr F = readBinaryRec(in);
                                 if(F==null) // error in reading sublist
                                    return null;
                                 ListExpr LE = ListExpr.oneElemList(F);
                                 ListExpr Last = LE;
                                 ListExpr Next = null;
                                 for(int i=1;i<length;i++){
                                     Next = readBinaryRec(in);
                                     if(Next==null) // error in reading sublist
                                        return null;
                                     Last= ListExpr.append(Last,Next);
                                 }
                                 return LE;
                               }
    default      :             { if(DEBUG_MODE)
                                    System.out.println("unknow binary list type");
                                 return null;
                               }
  }

}
catch(Exception e){
  if(DEBUG_MODE){
    System.out.println(e);
    e.printStackTrace();
  }
  return null;
}

}
/** stores the Directory of the given filename */
  private void storeDirectory(String FileName){
     if(FileName==null)
        currentDir = null;
     else{
        int index = FileName.lastIndexOf(File.separatorChar);
        if(index<0)
           currentDir = FileName;
        else
           currentDir = FileName.substring(0,index);
     }
  }


public int readFromFile (String fileName) {
    dbarray.printStatistics();
    long startTime = System.currentTimeMillis();
    FileReader inputReader;
    NLParser parser;
    Symbol result;
    //First of all, initializes this ListExpr to the empty list.
    this.setValueTo(ListExpr.theEmptyList());
    try {
      inputReader = new FileReader(fileName);
      storeDirectory((new File(fileName)).getAbsolutePath());
      parser = new NLParser(inputReader);
      result = parser.parse();
      inputReader.close();
      storeDirectory(null);
      if (result == null) {
        // If the parser returns a null value, then an error was detected
        // when parsing the input.
        if (this.DEBUG_MODE) {
          System.err.println("DEBUG MODE: Error in line " + parser.linePos +
              " when parsing input file in ReadFromFile()");
        }
        // It returns the line number where the error was detected.
        return  parser.linePos;
      }
      // else, it sets this ListExpr to the value returned by the parser.
      this.setValueTo((ListExpr)result.value);
    } catch (FileNotFoundException except) {
      // If it could not open the file, it returns READ_FROM_FILE_ERROR (-1).
      if (this.DEBUG_MODE) {
        System.err.println("DEBUG MODE: Error when opening file in readFromFile()");
        except.printStackTrace();
      }
      return  READ_FROM_FILE_ERROR;
    } catch (Exception except) {
      if (this.DEBUG_MODE) {
        System.err.println("DEBUG MODE: EXCEPTION in readFromFile()");
        except.printStackTrace();
      }
      return  INPUT_STRING_ERROR;
    }
    // If no error, returns NO_ERROR_CODE.
    System.out.println(" Reading a nested list from file "+fileName+" has taken "+ (System.currentTimeMillis()-startTime)+" milliseconds");
    dbarray.printStatistics();
    return  this.NO_ERROR_CODE;
  }


 public int readFromString (String chars) {
    StringReader inputReader;
    NLParser parser;
    Symbol result;
    int position;
    //First of all, initializes this ListExpr to the empty list.
    this.setValueTo(ListExpr.theEmptyList());
    inputReader = new StringReader(chars);
    parser = new NLParser(inputReader);
    try {
      result = parser.parse();
      if (result == null) {
        // If the parser returns a null value, then an error was detected
        // when parsing the input.
        if (this.DEBUG_MODE) {
          System.err.println("DEBUG MODE: Error in character " + parser.charPos
              + " when parsing input string in ReadFromString()");
        }
        // It returns the character position where the error was detected.
        return  parser.charPos;
      }
      // else, it sets this ListExpr to the value returned by the parser.
      this.setValueTo((ListExpr)result.value);
    } catch (Exception except) {
      System.out.println("EXCEPTION in readFromString()");
      if(DEBUG_MODE)
         except.printStackTrace();
      return  INPUT_STRING_ERROR;
    } finally {
      inputReader.close();
    }
    return  this.NO_ERROR_CODE;
  }


public boolean writeBinaryTo(OutputStream OS){
  if(OS==null)
     return false;
  MyDataOutputStream   DOS=null;
  try{
     DOS = new MyDataOutputStream(OS);
     DOS.writeString("bnl");
     DOS.writeShort(1);
     DOS.writeShort(1);
     boolean ok = writeBinaryRec(this,DOS);
     DOS.flush();
     return ok;
   } catch(Exception e){
     if(DEBUG_MODE)
        e.printStackTrace();
     try{DOS.close();}catch(Exception e1){}
     return false;
   }
}


private static boolean writeBinaryRec(ListExpr LE, MyDataOutputStream OS){
   byte T = LE.getBinaryType();
   if(T<0){
      if(DEBUG_MODE)
        System.err.println("unknow Listtype in writeBinaryRec");
      return false;
   }

   try{
      switch(T){
          case BIN_BOOLEAN      : { OS.writeByte(T);
                                    boolean b = LE.boolValue();
                                    byte value =(byte) (b?1:0);
                                    OS.writeByte(value);
                                    return true;
                                  }
          case BIN_INTEGER      : { OS.writeByte(T);
                                    int value = LE.intValue();
                                    OS.writeInt(value);
                                    return true;
                                  }
          case BIN_SHORTINT     : { OS.writeByte(T);
                                    short value = (short) LE.intValue();
                                    OS.writeShort(value);
                                    return true;
                                  }
          case BIN_BYTE         : { OS.writeByte(T);
                                    byte value = (byte) LE.intValue();
                                    OS.writeByte(value);
                                    return true;
                                  }

          case BIN_REAL         : { OS.writeByte(T);
                                    double value = LE.realValue();
                                    OS.writeReal(value);
                                    return true;
                                  }
          case BIN_SHORTSTRING  : { OS.writeByte(T);
                                    String value = LE.stringValue();
                                    OS.writeByte((byte)value.length());
                                    OS.writeString(value);
                                    return true;
                                  }
          case BIN_STRING      :  { OS.writeByte(T);
                                    String value = LE.stringValue();
                                    OS.writeShort((short)value.length());
                                    OS.writeString(value);
                                    return true;
                                  }
          case BIN_LONGSTRING  :  { OS.writeByte(T);
                                    String value = LE.stringValue();
                                    OS.writeInt(value.length());
                                    OS.writeString(value);
                                    return true;
                                  }
          case BIN_SHORTSYMBOL :  { OS.writeByte(T);
                                    String value = LE.symbolValue();
                                    OS.writeByte((byte)value.length());
                                    OS.writeString(value);
                                    return true;
                                  }
          case BIN_SYMBOL     :   { OS.writeByte(T);
                                    String value = LE.symbolValue();
                                    OS.writeShort((short)value.length());
                                    OS.writeString(value);
                                    return true;
                                  }

          case BIN_LONGSYMBOL  :  { OS.writeByte(T);
                                    String value = LE.symbolValue();
                                    OS.writeInt(value.length());
                                    OS.writeString(value);
                                    return true;
                                  }

          case BIN_SHORTTEXT   :  { OS.writeByte(T);
                                    String value = LE.textValue();
                                    int L = value.length();
                                    OS.writeByte((byte)L);
                                    OS.writeString(value);
                                    return true;
                                  }
          case BIN_TEXT        :  { OS.writeByte(T);
                                    String value = LE.textValue();
                                    int L = value.length();
                                    OS.writeShort((short)L);
                                    OS.writeString(value);
                                    return true;
                                  }
          case BIN_LONGTEXT   :  { OS.writeByte(T);
                                   String value = LE.textValue();
                                   int L = value.length();
                                   OS.writeInt(L);
                                   OS.writeString(value);
                                   return true;
                                 }
          case BIN_SHORTLIST   : { OS.writeByte(T);
                                   int length = LE.listLength();
                                   OS.writeByte((byte)length);
                                   while(!LE.isEmpty()){
                                       if(! writeBinaryRec(LE.first(),OS)) // error in writing sublist
                                          return false;
                                       LE=LE.rest();
                                   }
                                   return true;
                                 }
          case BIN_LIST        : { OS.writeByte(T);
                                   int length = LE.listLength();
                                   OS.writeShort((short)length);
                                   while(!LE.isEmpty()){
                                       if(! writeBinaryRec(LE.first(),OS)) // error in writing sublist
                                          return false;
                                       LE=LE.rest();
                                   }
                                   return true;
                                 }
          case BIN_LONGLIST    : { OS.writeByte(T);
                                   int length = LE.listLength();
                                   OS.writeInt(length);
                                   while(!LE.isEmpty()){
                                       if(! writeBinaryRec(LE.first(),OS)) // error in writing sublist
                                          return false;
                                       LE=LE.rest();
                                   }
                                   return true;
                                 }
          default              : return false;
      }
   }
   catch(Exception e){
     if(DEBUG_MODE)
        e.printStackTrace();
     return false;
   }
}


private long numberOfNodesWithoutNext(){
  if(atomType()!=NO_ATOM)
       return 1;
  long v = getValue();
  if(v!=0)
     return 1+(new ListExpr(v)).numberOfNodes();
  else
     return 1; 
}

private long  numberOfNodes(){
    long sum = numberOfNodesWithoutNext();
    long next = getNext();
    if(atomType()==NO_ATOM){
				while(next!=0){
					 ListExpr Next = new ListExpr(next);
					 sum += Next.numberOfNodesWithoutNext();
					 if(Next.atomType()==NO_ATOM)
							next = Next.getNext();
					 else
							next = 0;
				}
    }
    return sum; 
} 

private  byte getBinaryType(){
int AT = atomType();
switch(AT){
  case BOOL_ATOM    : return  BIN_BOOLEAN;
  case INT_ATOM     :  { int v = intValue();
                         if(v>=-128 & v<=127)
          return BIN_BYTE;
       if(v>=-32768 & v<=32767)
          return BIN_SHORTINT;
       return BIN_INTEGER;
           }
  case REAL_ATOM    : return BIN_REAL;
  case SYMBOL_ATOM  : { int len = symbolValue().length();
                        if(len<256)
         return BIN_SHORTSYMBOL;
      if(len<65536)
         return BIN_SYMBOL;
      return BIN_LONGSYMBOL;
                      }
  case STRING_ATOM  : {  int len = stringValue().length();
                         if(len<256)
          return BIN_SHORTSTRING;
       if(len<65536)
          return BIN_STRING;
       return BIN_LONGSTRING;
           }
  case TEXT_ATOM    : { int len = textValue().length();
                        if(len<256)
         return BIN_SHORTTEXT;
      if(len<65536)
         return BIN_TEXT;
      return BIN_LONGTEXT;}
  case NO_ATOM       : {int len = listLength();
                        if(len<256)
        return BIN_SHORTLIST;
      if(len<65536)
         return BIN_LIST;
      return BIN_LONGLIST;
      }
  default : return (byte) -1;
}
}

public void writeListExpr () {
    System.out.print(this.writeListExprToString());
    return;
}

public String writeListExprToString () {
    // It initializes the ~chars~ buffer to the empty string.
    StringBuffer chars = new StringBuffer();
    //Appends an starting end of line character.
    chars.append("\n");
    // And calls the auxiliar method.
    writeListExprAppeningToString(this, chars, "");
    return  chars.toString();
}

 private final static void writeListExprAppeningToString (ListExpr list, StringBuffer chars,
      String identation) {
    
    String separator = " ";
    boolean hasSubLists = false;
    identation = identation + "    ";    // It will use an identation bigger in each recursion.
    // If is not an atom appends "(" to the output.
    if (!list.isAtom()) {
      chars.append("(");        // Start of list.
    }
    // while it is not an empty list.
    while (!list.isEmpty()) {
      switch (list.atomType()) {
        case ListExpr.NO_ATOM:
          {
            if ((!hasSubLists) && (!(new ListExpr(list.getValue()).isEmpty()) && 
                (new ListExpr(list.getValue())).atomType() == NO_ATOM)) {
                // If this list contains a sublist it changes the separtor, making
                // it equal to "\n"+~identation~ (this is, increases the
                // identation used).
                hasSubLists = true;
                separator = "\n" + identation;
                // And stats using the new separator between elements right now.
                chars.append(separator);
             }
             // Writes its content, using in it a deeper identation.
             writeListExprAppeningToString((new ListExpr(list.getValue())), chars, identation);
             break;
          }
       case ListExpr.INT_ATOM:
          {
            chars.append(list.intValue());
            return;
          }
       case ListExpr.REAL_ATOM:
          {
            chars.append(list.realValue());
            return;
          }
        case ListExpr.BOOL_ATOM:
          { // It writes the value in uppercase ("TRUE" or "FALSE").
            boolean v = list.boolValue();
            String S = v?"TRUE":"FALSE"; 
            chars.append(S);
            return;
          }
        case ListExpr.STRING_ATOM:
          {
            chars.append("\"" + list.stringValue() + "\"");
            return;
          }
        case ListExpr.SYMBOL_ATOM:
          {
            chars.append(list.symbolValue());
            return;
          }
        case ListExpr.TEXT_ATOM:
          {
            chars.append(BEGIN_TEXT+list.textValue()+END_TEXT);
            return;
          }
      }
      list = new ListExpr(list.getNext());
      if (!list.isEmpty()) {
        // if is not the last element adds the separator string.
        chars.append(separator);
      }
    }
    // if the code reachs this point, it was not an atom, so it appends the end of lists.
    chars.append(")");          // End of list.
    return;
}


public boolean writeTo(OutputStream out,boolean OnlyRationals){
    try{
       Writer W = new OutputStreamWriter(out);
       writeToRec(W,OnlyRationals,"");
       W.write("\n");
       W.flush();
       return true;
    }catch(Exception e){
       return false;
    }
 }


private void writeToRec(Writer out,boolean OnlyRationals,String indent) throws IOException{
     switch(atomType()){
        case INT_ATOM : { out.write(" "+intValue()+" ");
                          break;
                        }
        case REAL_ATOM: { if(!OnlyRationals){
                            out.write(" "+realValue()+" ");
                          }else{
                             double v = realValue();
                             boolean negative = v<0;
                             if(negative)
                                  v=v*-1.0;
                             int intpart = (int) v;
                             double rest = v-intpart;
                             long denominator = 1000000000;
                             long numerator = (long) (rest * (double)denominator);
                             long GCD = gcd(denominator,numerator);
                             denominator = denominator/GCD;
                             numerator = numerator/GCD;

                             if(Math.abs(v-53.9701)<0.0001){
                                 System.out.println("Convert "+ v+" to (rat "+intpart+" "+numerator+" / "+denominator);
                             }

                             out.write(" ( rat ");
                             if(negative) out.write("- ");
                             out.write(intpart+" "+numerator+" / "+denominator+" ) ");
                          }
                          break;
                        }
        case BOOL_ATOM: { boolean b = boolValue();
                          if(b)
                             out.write(" TRUE ");
                          else
                             out.write(" FALSE ");
                          break;
                        }
       case STRING_ATOM: { out.write(" \""+stringValue()+"\" ");
                           break;
                         }
        case SYMBOL_ATOM:{ out.write(" "+symbolValue()+" ");
                           break;
                         }
        case TEXT_ATOM:  { out.write(" "+BEGIN_TEXT+textValue()+END_TEXT+" ");
                           break;
                         }
        case NO_ATOM:    { out.write("\n"+indent+"(");
                           ListExpr tmp  = this;
                           while (!tmp.isEmpty()){
                               tmp.first().writeToRec(out,OnlyRationals,indent+"   ");
                               tmp=tmp.rest();
                           }
                           out.write(")");
                           break;
                         }
        default: System.err.println("unknow AtomType in WriteToRec(..) method found");
     }
 }


/*
 This function returns the greates common denominator of a and b.
*/
 private static long gcd(long a, long b){
   if(a==0 && b==0) // this case should never occur
      return 1;
   long tmp;
   if(a<0) a *= -1;
   if(b<0) b *= -1;
   while(a>0){
     if(b<a){
       tmp = a;
       a = b;
       b = tmp;
     }else{
       b = b % a;
     }
   }
   return b;
}


public int writeToFile(String FileName){

   try{
       FileOutputStream out = new FileOutputStream(FileName);
       if(!writeTo(out,false)){
          return WRITE_TO_FILE_ERROR;
       } else
          return NO_ERROR_CODE;
   }catch(Exception e){
      if(DEBUG_MODE){
         e.printStackTrace();
      }
      return WRITE_TO_FILE_ERROR;
   }
}

public int writeToString(StringBuffer chars){
    writeListExprAppeningToString(this,chars,"");
    return NO_ERROR_CODE;
}


/* The representation */
private byte[] data=null;
private long key=0;

private static boolean DEBUG_MODE=true;
private static String  ParseDir = "";
private static String currentDir;



private static byte[] getTextBytes(long key){
    return dbarray.get(key);
}

private static String getText(long key){
  try{
     return new String(getTextBytes(key),"UTF-8");
  } catch(Exception e){
    e.printStackTrace();
    return null;
  }
}


/* the next unused key */
private static long nextkey=1;

private static long getNextKey(){
    return ++nextkey;
}

private static DBArray dbarray = null;
private static int MINCACHESIZE = 100;


}
