/* 
This file is created automatically. Please don't edit it. 
If changes are requiered at the ListExpr class, make them in the
Original File 'ListExpr.both'
*/
/******************************************************************************

1 Licence

----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----


//paragraph  [10]  title:    [\centerline{\Large \bf] [}]
 [10] ListExpr.java

 March, 1999. Jose Antonio Cotelo Lema.
 ListExpr.java
 Java implementation of the NestedLists tools.
\tableofcontents


0 Warning

This file is not a pure Java file. I.e. this file contains C-preprocessor directives.
The reason is. that this file contains two versions of nested lists. One of them
is main memory based, the other one uses the Berkeley DB Java Edition for persistent
storing of objects. To get a java file use the gcc's preprocessor:

gcc -E -xc -P -C [-DPERSISTENT] ListExpr.both.java >ListExpr.java

*/

package sj.lang;

import java_cup10.runtime.Symbol;
import sj.lang.JavaListExpr.NLParser;
import java.io.*;
import tools.Base64Decoder;
import java_cup10.runtime.*;
import tools.Reporter;
import java.util.Properties;
import java.util.Iterator;
import java.util.Set;
import tools.Environment;

public class ListExpr implements Serializable {

/*
1 Constructors

*/

/** Creates a new empty List */
public ListExpr ()  {

   this.value = null;
   this.type = ListExpr.NO_ATOM;
   this.next = null;



}
/** This method is only for compatibility with the
  * persistent version of nested lists.
  **/
public static boolean initialize(int cachesize){
   return true;
}
/** If the debug mode is set to true, more informations about
  * errors occured are printed to the console
  **/
public static void setDebugMode(boolean enabled){
  DEBUG_MODE = enabled;
}
/** Returns the working directory for parsing lists **/
public static String getDirectory(){
      return CurrentDir;
}
/** Sets the directory where big text atoms are stored
  */
public static void setTempDir(String DirName){
   if(!DirName.endsWith("/"))
      DirName+="/";
   File F = new File(DirName);
   if(F.exists()){
      TEMPDIR = DirName;
   } else{
      if(!F.mkdirs()){
        Reporter.writeError("cannot create directory "+DirName+"for temporal storing of text atoms");
        Reporter.writeError("Directory name remains unchanged. ");
      }else{
        TEMPDIR=DirName;
      }
   }
}
/** Replaces in textStr all occurences of patternOldString by
    patternNewString. The patters is gives as a string NOT by a
    regular expression.
**/
public static String replaceAll(String textStr,
                  String patternOldStr,
                  String patternNewStr)
{
  StringBuffer sstextReplaced = new StringBuffer();
  int lastpos = 0;
  int pos = 0;
  if( patternOldStr.length() == 0 )
  {
    return textStr;
  }
  do {
    lastpos = pos;
    pos = textStr.indexOf(patternOldStr, pos);
    if (pos >= 0)
    {
      sstextReplaced.append(textStr.substring(lastpos,pos));
      sstextReplaced.append(patternNewStr);
      pos += patternOldStr.length();
    }
    else
    {
      sstextReplaced.append(textStr.substring(lastpos, textStr.length()));
    }
  } while ( (pos >=0) && (pos < textStr.length()) );
  return sstextReplaced.toString();
}
/** Replaces all occurences of '\' by '\\' and all included '"' by '\"'.
 **/
private static String transform2Outstring(String theString){
   String s = replaceAll(theString,"\\","\\\\");
   return replaceAll(s,"\"","\\\"");
}
/** Replaces all occurences of '\' by '\\' and all included '</text--->'
    by '\</text--->'.
 **/
private static String transform2Outtext(String theString){
  String s = replaceAll(theString,"\\","\\\\");
  return replaceAll(s,END_TEXT,"\\" + END_TEXT);
    }
  /** If the parameter is true, text atoms with a minimum length of
    * MAX_INTERNAL_TEXT_LENGTH are stored into a file.
    */
public static void usePersistentText(boolean enabled){
     USE_PERSISTENT_TEXT = enabled;
}
  /** Sets the value of the maximum text length handled in main memory
    */
public static boolean setMaxInternalTextLength(int length){
   if(length<1)
      return false;
   MAX_INTERNAL_TEXT_LENGTH = length;
   return true;
}
 /** Sets the maximal length for strings in string atoms.
   */
public static void setMaxStringLength(int len){
    if(len>0)
       MAX_STRING_LENGTH=len;
}
/** Returns the length of a text atom.
  * If this list is not a text atom -1 is returned.
  */
public int getTextLength(){
     if(type!=TEXT_ATOM)
        return -1;
     if(value instanceof File)
         return (int)((File)value).length();
     else
         return ((String)value).length();
}
/** Returns true if the value of this list is
  * in the main memory
  */
public boolean isInMemory(){
    if(type!=TEXT_ATOM)
      return true;
    if(value==null)
       return true;
    if(!(value instanceof File))
       return true;
    return false;
}
/** When this method is called, the value of this node from
  * the nested list is taken from the argument.
  **/
public void setValueTo (ListExpr list) {
  this.type = list.type;
  this.value = list.value;
  this.next = list.next;
}
/**
 * Create a list of pairs representing the (key, value)-pair of a Properties
 * object.
 *
 * @param p
 *            the Properties to read from
 * @return a ListExpr representing the Properties
 */
public static ListExpr fromProperties(Properties p) {
 Set<Object> keys = p.keySet();
 Iterator<Object> i = keys.iterator();
 if (!i.hasNext()) {
  return ListExpr.theEmptyList();
 }
 String keystr = (String) i.next();
 String valuestr = p.getProperty(keystr);
 ListExpr pair = ListExpr.twoElemList(ListExpr.stringAtom(keystr),
   ListExpr.textAtom(valuestr));
 ListExpr nl = ListExpr.oneElemList(pair);
 ListExpr last = nl;
 while (i.hasNext()) {
  keystr = (String) i.next();
  valuestr = p.getProperty(keystr);
  pair = ListExpr.twoElemList(ListExpr.stringAtom(keystr),
    ListExpr.textAtom(valuestr));
  last = ListExpr.append(last, pair);
 }
 return nl;
}
/**
 * Inserts (key, value)-Pairs found in a list ((stringAtom textAtom)*) into
 * a Properties object. Returns false if and only if some problem occurs.
 *
 * @param l
 *            the ListExpr to read from
 * @return the Properties
 */
public static boolean toProperties(ListExpr l, Properties p) {
 if (l.isAtom()) {
  return false;
 }
 boolean ok = true;
 while (!(l.isAtom() || l.isEmpty())) {
  ListExpr pair = l.first();
  l = l.rest();
  if ((pair.listLength() == 2)
    && (pair.first().isAtom() && pair.first().atomType() == ListExpr.STRING_ATOM)
    && (pair.second().isAtom() && pair.second().atomType() == ListExpr.TEXT_ATOM)) {
   String key = pair.first().stringValue();
   String value = pair.second().stringValue();
   p.setProperty(key, value);
  } else {
   ok = false;
  }
 }
 return ok;
}
/** This method returns an empty ListExpr. It will return the same as creating
  * a new ListExpr directly using the class constructor.
  */
public static ListExpr theEmptyList () {
  return new ListExpr();
}
 /** Constructs a new ListExpr which is the union of the argument lists (being
   * ~left~ and ~right~ the left and right part respectively) and returns it.
   * Preconditions:* ~right~ can not be an Atom.
   */
public static ListExpr cons (ListExpr left, ListExpr right) {
  if(DEBUG_MODE){
     if(left==null || right==null){
        Reporter.writeError("ListExpr.cons called with an null argument");
        if(left==null) Reporter.writeError("Left==null");
        if(right==null) Reporter.writeError("Right==null");
        Reporter.printTrace();
     }
  }
 if (CHECK_PRECONDITIONS) {
    if (right.isAtom()) {
       Reporter.writeError("CHECK PRECONDITIONS: Error when calling the cons() method");
       Reporter.writeError(" the input argument ~right~ does not fulfil the preconditions.");
     }
 }
    ListExpr result = new ListExpr();
    result.value = left;
    result.next = right;
    return result;
}
/** Appends newSon to the list ending with lastElement.
  **/
public static ListExpr append (ListExpr lastElement, ListExpr newSon) {
    if(DEBUG_MODE){
       if(lastElement==null || newSon==null){
          Reporter.writeError(" ListExpr.append called with an null argument");
           if(lastElement==null) Reporter.writeError("lastElement==null");
           if(newSon==null) Reporter.writeError("newSon==null");
           Reporter.printTrace();
       }
    }
    if (ListExpr.CHECK_PRECONDITIONS) {
      if ((!lastElement.endOfList()) || lastElement.isEmpty() || lastElement.isAtom()) {
        Reporter.writeError("CHECK PRECONDITIONS: Error when calling the append() method");
        Reporter.writeError(" the input argument astElement~ does not fulfil the preconditions.");
      }
    }
    ListExpr p = new ListExpr();
    p.value = newSon;
    p.next = theEmptyList();
    lastElement.next = p;
    return p;
}
/** Concat newSon to the list starting with headElement.
  **/
public static ListExpr concat(ListExpr headElement, ListExpr newSon) {
    if(DEBUG_MODE){
       if(newSon==null){
          Reporter.writeError(" ListExpr.append called with an null argument");
           if(newSon==null) Reporter.writeError("newSon==null");
           Reporter.printTrace();
       }
    }
    ListExpr result;
    if ((headElement == null) || headElement.isEmpty()){
      result = new ListExpr();
      result.value = newSon;
      result.next = theEmptyList();
    }
    else {
      result = new ListExpr();
      result.value = headElement.first();
      result.next = theEmptyList();
      ListExpr last = result;
      ListExpr rest = headElement.rest();
      while (!rest.isEmpty())
      {
        ListExpr p = rest.first();
        last = append(last, p);
        rest = rest.rest();
      }
      last = append(last, newSon);
    }
    return result;
}
/** For compatibility with the peristent version of nested lists **/
public void destroy() {
}
/** Returns true if this ListExpr object is an empty list, false
  *  otherwise.
  **/
public boolean isEmpty () {
    // If is a NO_ATOM and ~value~ and ~next~ are empty, then it is an empty list.
    if ((this.type == ListExpr.NO_ATOM) && (this.value == null) && (this.next
        == null)) {
      return true;
    }
    // else it is not empty.
    return false;
}
/** Returns true if this ListExpr object is an Atom, false
  * otherwise.
  **/
public boolean isAtom () {
  return atomType()!=NO_ATOM;
}
/** Returns true if this ListExpr object is the end of a list, is
  * not an atom and is not empty. Returns false otherwise.
  **/
public boolean endOfList () {
    if ((this.type == ListExpr.NO_ATOM) && (!this.isEmpty()) && (this.next.isEmpty())) {
      return true;
    }
    return false;
}
/** Returns the number of elements of this ListExpr object.
  * WARNING: this methods needs a list transversal searching and therefore
  * it spends a time proportional to the length of the list.
  **/
public int listLength () {
    int length = 0;
    ListExpr aux = this;
    // If it is not an atom, it must return the the number of elements.
    if (aux.isAtom()) {
      // if it is an atom, it must return -1.
      return -1;
    }
    // If it is not an atom.
    while (!aux.isEmpty()) {
      length++;
      aux = aux.next;
    }
    return length;
}
/** Writes the content of this ListExpr object to the standard
  * output in an indented way.
  **/
public void writeListExpr () {
    Reporter.write(this.writeListExprToString());
    return;
}
/** Writes the value of this listexpr to chars.
  */
public int writeToString (StringBuffer chars) {
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.isAtom()) {
        Reporter.writeError("CHECK PRECONDITIONS: Error when calling the writeToString() method");
        Reporter.writeError("The ListExpr object does not fulfil the preconditions.");
      }
    }
    chars.setLength(0);
    return writeAppeningToString(this, chars);
}
/** This function writes the value of this ListExpr to the given StringBuffer **/
private final static int writeAppeningToString (ListExpr list, StringBuffer chars) {
    int result = 0;
    // If is not an atom appends "(" to the output.
    if (!list.isAtom()) {
      chars.append("("); // Start of list.
    }
    // while it is not an empty list.
    while (!list.isEmpty()) {
      switch (list.atomType()) {
        case ListExpr.NO_ATOM:
          {
            // Writes its content.
            result = writeAppeningToString((ListExpr)list.value, chars);
            if (result != ListExpr.NO_ERROR_CODE) {
              // if an error happened, returns the error.
              return result;
            }
            break;
          }
        case ListExpr.INT_ATOM:
          {
            chars.append(""+list.intValue());
            return 0;
          }
        case ListExpr.REAL_ATOM:
          {
            chars.append(""+list.realValue());
            return 0;
          }
        case ListExpr.BOOL_ATOM:
          { // It writes the value in uppercase ("TRUE" or "FALSE").
            String LS = list.boolValue()?"TRUE":"FALSE";
            chars.append(LS);
            return 0;
          }
        case ListExpr.STRING_ATOM:
          {
            chars.append("\"" +
                transform2Outstring(list.stringValue()) +
                    "\"");
            return 0;
          }
        case ListExpr.SYMBOL_ATOM:
          {
            chars.append(list.symbolValue());
            return 0;
          }
       case ListExpr.TEXT_ATOM:
          {
            chars.append("<text>" + transform2Outtext(list.textValue()) + "</text--->");
            return 0;
          }
      }
      list = list.next;
      if (!list.isEmpty()) {
        // if is not the last element adds an space character as separator.
        chars.append(" ");
      }
    }
    // if the code reachs this point, it was not an atom, so it appends the end of lists.
    chars.append(")"); // End of list.
    return 0; // No error.
  }
public String toString(){
     return writeListExprToString();
}
/** This method returns one string with the content of this ListExpr object
  * in an indented way.
  **/
public String writeListExprToString () {
    // It initializes the ~chars~ buffer to the empty string.
    StringBuffer chars = new StringBuffer();
    //Appends an starting end of line character.
    chars.append("\n");
    // And calls the auxiliar method.
    writeListExprAppeningToString(this, chars, "");
    return chars.toString();
}
/** Returns the first element of a list **/
public ListExpr first(){
  return nThElement(1);
}
/** returns the second element of a list **/
public ListExpr second(){
  return nThElement(2);
}
/** returns the third element of list **/
public ListExpr third(){
   return nThElement(3);
}
/** returns the fourth element of a list **/
public ListExpr fourth(){
   return nThElement(4);
}
/** Returns the fifth element of a list **/
public ListExpr fifth(){
   return nThElement(5);
}
/** Returns the sixth element of a list **/
public ListExpr sixth(){
   return nThElement(6);
}
/** Returns the seventh element of a list **/
public ListExpr seventh(){
  return nThElement(7);
}
/** Returns the eighth element of a list **/
public ListExpr eighth(){
  return nThElement(8);
}
/** Returns the nineth element of a list **/
public ListExpr nineth(){
  return nThElement(9);
}
/** Returns the tenth element of a list **/
public ListExpr tenth(){
  return nThElement(10);
}
/** Returns the twelfth element of a list **/
public ListExpr twelfth(){
  return nThElement(12);
}
/** Returns the rest of this ListExpr object, this is, a ListExpr
  * object what is identical to this ListExpr but without the first element. The
  * result can be an empty list.
  * Preconditions: this ListExpr object can not be an atom and can not be empty.
  **/
public ListExpr rest () {
  if (ListExpr.CHECK_PRECONDITIONS) {
     if (this.isAtom() || this.isEmpty()) {
        Reporter.writeError("CHECK PRECONDITIONS: Error when calling the rest() method");
        Reporter.writeError("the ListExpr object does not fulfil the preconditions.");
      }
    }
    if(atomType()!=NO_ATOM)
          return null;
    return this.next;
}
/** This method reads a ListExpr from the file "fileName" and assigns it to
  * this ListExpr object.
  * It returns 0 if reading was succesful. Otherwise,
  * returns READ\_FROM\_FILE\_ERROR (-1) if the file
  * could not be accessed, or the line number in the file where an error was
  * detected.
  * */
public int readFromFile (String fileName) {
    ListExpr LE = getListExprFromFile(fileName);
    if(LE==null)
        return READ_FROM_FILE_ERROR;
    setValueTo(LE);
    return NO_ERROR_CODE;
}
/** This method returns the ListExpr stored in the file given by fileName.
  * If an error occurs, the resukt will be null.
  **/
public static ListExpr getListExprFromFile(String fileName){
    long startTime=0;
    if(Environment.MEASURE_TIME){
      startTime = System.currentTimeMillis();
    }
    long usedMemory = 0;
    if(Environment.MEASURE_MEMORY){
        usedMemory = Environment.usedMemory();
    }
    Reader inputReader;
    NLParser parser;
    Symbol result;
    try {
      if(Environment.ENCODING!=null){
          inputReader = new InputStreamReader(new FileInputStream(fileName),Environment.ENCODING);
      } else {
          inputReader = new InputStreamReader(new FileInputStream(fileName));
      }
      storeDirectory((new File(fileName)).getAbsolutePath());
      parser = new NLParser(inputReader);
      result = parser.parse();
      inputReader.close();
      storeDirectory(null);
      if (result == null) {
          Reporter.debug(" Error in line " + parser.linePos +
                         " when parsing input file"+fileName +
                         " in Line "+parser.linePos,
                         null);
      }else {
          if(Environment.MEASURE_TIME){
                Reporter.writeInfo("Building nested List from File " + fileName+" has taken "+
                              (System.currentTimeMillis()-startTime) + " milliseconds");
           }
           if(Environment.MEASURE_MEMORY){
               long md = (Environment.usedMemory()-usedMemory);
               Reporter.writeInfo("memory difference:"+Environment.formatMemory(md) );
               Environment.printMemoryUsage();
            }
      }
      return (ListExpr)result.value;
    } catch (FileNotFoundException except) {
        Reporter.debug(except);
        return null;
    } catch (Exception except) {
      Reporter.debug(except);
    }
      return null;
}
 /** Writes this ListExpr object to file "filename". The
   * previous contents of the file will be lost.
   * Returns 0 if writing was sucessful, 1 if the file could not be written
   * properly.
   * Preconditions: this listExpr object must not be an atom.
   */
public int writeToFile (String fileName) {
  storeDirectory((new File(fileName)).getAbsolutePath());
  try{
     OutputStream file = new FileOutputStream(fileName);
     if(writeTo(file,false)){
         storeDirectory(null);
         return NO_ERROR_CODE;
     } else{
        return WRITE_TO_FILE_ERROR;
     }
  } catch(Exception e){
      Reporter.debug(e);
      return WRITE_TO_FILE_ERROR;
  }
}
/** Writes the list to out */
public boolean writeTo(OutputStream out,boolean OnlyRationals){
    try{
       Writer W;
       if(Environment.ENCODING!=null) {
          W = new OutputStreamWriter(out, Environment.ENCODING);
       } else {
          W = new OutputStreamWriter(out);
       }
       writeToRec(W,OnlyRationals,"");
       W.write("\n");
       W.flush();
       return true;
    }catch(Exception e){
       return false;
    }
}
/** Writes this ListExpr object to outputstream os in binary format.
  * Returns true if writing was sucessful, false if the list could not be written
  * properly.
  */
public boolean writeBinaryTo(OutputStream OS){
  if(OS==null)
     return false;
  MyDataOutputStream DOS=null;
  try{
     DOS = new MyDataOutputStream(OS);
     DOS.writeString("bnl");
     DOS.writeShort(1);
     DOS.writeShort(2);
     boolean ok = writeBinaryRec(this,DOS);
     DOS.flush();
     return ok;
   } catch(Exception e){
     Reporter.debug(e);
     try{DOS.close();}catch(Exception e1){}
     return false;
   }
}
/** Supports the writeBinaryTo method **/
private static boolean writeBinaryRec(ListExpr LE, MyDataOutputStream OS){
   byte T = LE.getBinaryType();
   if(T<0){
      Reporter.debug("unknow Listtype in writeBinaryRec");
      return false;
   }
   try{
      switch(T){
          case BIN_BOOLEAN : { OS.writeByte(T);
                                    boolean b = LE.boolValue();
                                    byte value1 =(byte) (b?1:0);
                                    OS.writeByte(value1);
                                    return true;
                                  }
          case BIN_INTEGER : { OS.writeByte(T);
                                    int value1 = LE.intValue();
                                    OS.writeInt(value1);
                                    return true;
                                  }
          case BIN_SHORTINT : { OS.writeByte(T);
                                    short value1 = (short) LE.intValue();
                                    OS.writeShort(value1);
                                    return true;
                                  }
          case BIN_BYTE : { OS.writeByte(T);
                                    byte value1 = (byte) LE.intValue();
                                    OS.writeByte(value1);
                                    return true;
                                  }
          case BIN_REAL : { OS.writeByte(T);
                                    double value1 = LE.realValue();
                                    OS.writeReal(value1);
                                    return true;
                                  }
          case BIN_DOUBLE : { OS.writeByte(T);
                                      double value1 = LE.realValue();
                                      OS.writeDouble(value1);
                                      return true;
                                  }
          case BIN_SHORTSTRING : { OS.writeByte(T);
                                    String value1 = LE.stringValue();
                                    OS.writeByte((byte)value1.length());
                                    OS.writeString(value1);
                                    return true;
                                  }
          case BIN_STRING : { OS.writeByte(T);
                                    String value1 = LE.stringValue();
                                    OS.writeShort((short)value1.length());
                                    OS.writeString(value1);
                                    return true;
                                  }
         case BIN_LONGSTRING : { OS.writeByte(T);
                                    String value1 = LE.stringValue();
                                    OS.writeInt(value1.length());
                                    OS.writeString(value1);
                                    return true;
                                  }
          case BIN_SHORTSYMBOL : { OS.writeByte(T);
                                    String value1 = LE.symbolValue();
                                    OS.writeByte((byte)value1.length());
                                    OS.writeString(value1);
                                    return true;
                                  }
          case BIN_SYMBOL : { OS.writeByte(T);
                                    String value1 = LE.symbolValue();
                                    OS.writeShort((short)value1.length());
                                    OS.writeString(value1);
                                    return true;
                                  }
          case BIN_LONGSYMBOL : { OS.writeByte(T);
                                    String value1 = LE.symbolValue();
                                    OS.writeInt(value1.length());
                                    OS.writeString(value1);
                                    return true;
                                  }
          case BIN_SHORTTEXT : { OS.writeByte(T);
                                    String value1 = LE.textValue();
                                    int L = value1.length();
                                    OS.writeByte((byte)L);
                                    OS.writeString(value1);
                                    return true;
                                  }
          case BIN_TEXT : { OS.writeByte(T);
                                    String value1 = LE.textValue();
                                    int L = value1.length();
                                    OS.writeShort((short)L);
                                    OS.writeString(value1);
                                    return true;
                                  }
          case BIN_LONGTEXT : { OS.writeByte(T);
                                   String value1 = LE.textValue();
                                   int L = value1.length();
                                   OS.writeInt(L);
                                   OS.writeString(value1);
                                   return true;
                                 }
        case BIN_SHORTLIST : { OS.writeByte(T);
                                   int length = LE.listLength();
                                   OS.writeByte((byte)length);
                                   while(!LE.isEmpty()){
                                       if(! writeBinaryRec(LE.first(),OS)) // error in writing sublist
                                          return false;
                                       LE=LE.rest();
                                   }
                                   return true;
                                 }
          case BIN_LIST : { OS.writeByte(T);
                                   int length = LE.listLength();
                                   OS.writeShort((short)length);
                                   while(!LE.isEmpty()){
                                       if(! writeBinaryRec(LE.first(),OS)) // error in writing sublist
                                          return false;
                                       LE=LE.rest();
                                   }
                                   return true;
                                 }
          case BIN_LONGLIST : { OS.writeByte(T);
                                   int length = LE.listLength();
                                   OS.writeInt(length);
                                   while(!LE.isEmpty()){
                                       if(! writeBinaryRec(LE.first(),OS)) // error in writing sublist
                                          return false;
                                       LE=LE.rest();
                                   }
                                   return true;
                                 }
          default : return false;
      }
   }
   catch(Exception e){
     Reporter.debug(e);
     return false;
   }
}
/*
   This method is implemented to support the implementation of the ~writeBinaryTo~
   method.
   */
private byte getBinaryType(){
int AT = atomType();
switch(AT){
  case BOOL_ATOM : return BIN_BOOLEAN;
  case INT_ATOM : { int v = intValue();
                         if(v>=-128 & v<=127)
          return BIN_BYTE;
       if(v>=-32768 & v<=32767)
          return BIN_SHORTINT;
       return BIN_INTEGER;
           }
  case REAL_ATOM : return BIN_DOUBLE;
  case SYMBOL_ATOM : { int len = symbolValue().length();
                        if(len<256)
         return BIN_SHORTSYMBOL;
      if(len<65536)
         return BIN_SYMBOL;
      return BIN_LONGSYMBOL;
                      }
  case STRING_ATOM : { int len = stringValue().length();
                         if(len<256)
          return BIN_SHORTSTRING;
       if(len<65536)
          return BIN_STRING;
       return BIN_LONGSTRING;
           }
  case TEXT_ATOM : { int len = textLength();
                        if(len<256)
         return BIN_SHORTTEXT;
      if(len<65536)
         return BIN_TEXT;
      return BIN_LONGTEXT;}
  case NO_ATOM : {int len = listLength();
                        if(len<256)
        return BIN_SHORTLIST;
      if(len<65536)
         return BIN_LIST;
      return BIN_LONGLIST;
      }
  default : return (byte) -1;
}
}
/** method supporting the writeTo method */
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
                        case STRING_ATOM: { out.write(" \""+transform2Outstring(stringValue())+"\" ");
                           break;
                         }
        case SYMBOL_ATOM:{ out.write(" "+symbolValue()+" ");
                           break;
                         }
        case TEXT_ATOM: { out.write(" "+BEGIN_TEXT+transform2Outtext(textValue()) + END_TEXT + " ");
                           break;
                         }
       case NO_ATOM: { out.write("\n"+indent+"(");
                           ListExpr tmp = this;
                           while (!tmp.isEmpty()){
                               tmp.first().writeToRec(out,OnlyRationals,indent+"   ");
                               tmp=tmp.rest();
                           }
                           out.write(")");
                           break;
                         }
        default: Reporter.writeError("unknow AtomType in WriteToRec(..) method found");
     }
 }
 /*
   3.4.152 The readBinaryFrom() method.
   This method read a ListExpr from inputstream in.
   If an error is occurred nul is returned.
  */
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
      Reporter.writeError("wrong signatur or version "+Sig );
      return null;
    }
    if(major!=1 | ( minor!=0 & minor!=1 & minor!=2)){
       Reporter.writeError("wrong version numer "+major+"."+minor);
       return null;
    }
    ListExpr LE = readBinaryRec(DIN);
    return LE;
 }
 catch(Exception e){
   Reporter.debug(e);
   return null;
 }
}
/** Stores the Directory of the given filename **/
 private static void storeDirectory(String FileName){
    if(FileName==null)
       CurrentDir = null;
    else{
       int index = FileName.lastIndexOf(File.separatorChar);
       if(index<0)
          CurrentDir = FileName;
       else
          CurrentDir = FileName.substring(0,index);
    }
}
/** Writes this list into the stringbuffer using a given initial indentation. */
private final static void writeListExprAppeningToString (ListExpr list, StringBuffer chars,
      String identation) {
    String separator = " ";
    boolean hasSubLists = false;
    identation = identation + "    "; // It will use an identation bigger in each recursion.
    // If is not an atom appends "(" to the output.
    if (!list.isAtom()) {
      chars.append("("); // Start of list.
    }
    // while it is not an empty list.
    while (!list.isEmpty()) {
      switch (list.atomType()) {
        case ListExpr.NO_ATOM:
          {
          if ((!hasSubLists) && (!((ListExpr)list.value).isEmpty()) && (((ListExpr)list.value).type
                == NO_ATOM)) {
                // If this list contains a sublist it changes the separtor, making
                // it equal to "\n"+~identation~ (this is, increases the
                // identation used).
                hasSubLists = true;
                separator = "\n" + identation;
                // And stats using the new separator between elements right now.
                chars.append(separator);
             }
             // Writes its content, using in it a deeper identation.
             writeListExprAppeningToString((ListExpr)list.value,chars,identation);
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
            chars.append("\"" + transform2Outstring(list.stringValue()) + "\"");
            return;
          }
        case ListExpr.SYMBOL_ATOM:
          {
            chars.append(list.symbolValue());
            return;
          }
        case ListExpr.TEXT_ATOM:
          {
            chars.append(BEGIN_TEXT + transform2Outtext(list.textValue()) + END_TEXT);
            return;
          }
      }
     list = (ListExpr) list.next;
      if (!list.isEmpty()) {
        // if is not the last element adds the separator string.
        chars.append(separator);
      }
    }
    // if the code reachs this point, it was not an atom, so it appends the end of lists.
    chars.append(")"); // End of list.
    return;
}
/** Writes this list to a PrintStream without building a big string before **/
public void writeTo(PrintStream out){
  ListExpr list = this;
  writeListExpr(list,out,"");
}
/** Writes list to a PrintStream without building a big string before. */
private final static void writeListExpr (ListExpr list, PrintStream out,
      String identation) {
    String separator = " ";
    boolean hasSubLists = false;
    identation = identation + "    "; // It will use an identation bigger in each recursion.
    // If is not an atom appends "(" to the output.
    if (!list.isAtom()) {
      out.print("("); // Start of list.
    }
    // while it is not an empty list.
    while (!list.isEmpty()) {
      switch (list.atomType()) {
        case ListExpr.NO_ATOM:
          {
          if ((!hasSubLists) && (!((ListExpr)list.value).isEmpty()) && (((ListExpr)list.value).type
                == NO_ATOM)) {
                // If this list contains a sublist it changes the separtor, making
                // it equal to "\n"+~identation~ (this is, increases the
                // identation used).
                hasSubLists = true;
                separator = "\n" + identation;
                // And stats using the new separator between elements right now.
                out.print(separator);
             }
             // Writes its content, using in it a deeper identation.
             writeListExpr((ListExpr)list.value,out,identation);
             break;
          }
     case ListExpr.INT_ATOM:
          {
            out.print(""+list.intValue());
            return;
          }
       case ListExpr.REAL_ATOM:
          {
            out.print(""+list.realValue());
            return;
          }
        case ListExpr.BOOL_ATOM:
          { // It writes the value in uppercase ("TRUE" or "FALSE").
            boolean v = list.boolValue();
            String S = v?"TRUE":"FALSE";
            out.print(S);
            return;
          }
        case ListExpr.STRING_ATOM:
          {
            out.print("\"" + transform2Outstring(list.stringValue()) + "\"");
            return;
          }
        case ListExpr.SYMBOL_ATOM:
          {
            out.print(list.symbolValue());
            return;
          }
        case ListExpr.TEXT_ATOM:
          {
            out.print(BEGIN_TEXT + transform2Outtext(list.textValue()) + END_TEXT);
            return;
          }
      }
     list = (ListExpr) list.next;
      if (!list.isEmpty()) {
        // if is not the last element adds the separator string.
        out.print(separator);
      }
    }
    // if the code reachs this point, it was not an atom, so it appends the end of lists.
    out.print(")"); // End of list.
    return;
}
public static ListExpr oneElemList (ListExpr elem1) {
 return cons(elem1, theEmptyList());
}
public static ListExpr twoElemList (ListExpr elem1, ListExpr elem2) {
 return cons(elem1, cons(elem2, theEmptyList()));
}
public static ListExpr threeElemList (ListExpr elem1, ListExpr elem2, ListExpr elem3) {
 return cons(elem1, cons(elem2, cons(elem3, theEmptyList())));
}
public static ListExpr fourElemList (ListExpr elem1, ListExpr elem2, ListExpr elem3,
  ListExpr elem4) {
 return cons(elem1, cons(elem2, cons(elem3, cons(elem4, theEmptyList()))));
}
public static ListExpr fiveElemList (ListExpr elem1, ListExpr elem2, ListExpr elem3,
  ListExpr elem4, ListExpr elem5) {
 return cons(elem1, cons(elem2, cons(elem3, cons(elem4, cons(elem5, theEmptyList())))));
}
public static ListExpr sixElemList (ListExpr elem1, ListExpr elem2, ListExpr elem3,
  ListExpr elem4, ListExpr elem5, ListExpr elem6) {
 return cons(elem1, cons(elem2, cons(elem3, cons(elem4, cons(elem5, cons(elem6,
   theEmptyList()))))));
}
/**
1.3.2 Creation of a boolean atom

Creates a new listnode representing a boolean value.

*/
public static ListExpr boolAtom(boolean value){
   ListExpr result = new ListExpr();
   result.value = new Boolean(value);
   result.type = ListExpr.BOOL_ATOM;
   return result;
}
/**
1.3.3 Creation of an integer atom

The next method creates a new integer atom.

*/
public static ListExpr intAtom(int value){
    ListExpr result = new ListExpr();
    result.value = new Integer(value);
    result.type = ListExpr.INT_ATOM;
    return result;
}


/**
1.3.4 Creation of a real atom

This method creates a new real atom with the given value.

*/
public static ListExpr realAtom(double value){
    ListExpr result = new ListExpr();
    result.value = new Double(value);
    result.type = ListExpr.REAL_ATOM;
    return result;
}
/*
1.3.5 Creating a string atom

*/
public static ListExpr stringAtom(String value){
   if (ListExpr.CHECK_PRECONDITIONS) {
      if (value.length() > MAX_STRING_LENGTH) {
         Reporter.writeError("CHECK PRECONDITIONS: ");
         Reporter.writeError("   Error when calling the stringAtom() method:");
         Reporter.writeError("       the input string is larger than "+
                              MAX_STRING_LENGTH+" characters.");
         Reporter.writeWarning("   String will be shortened");
       }
   }
   if( value.length() > MAX_STRING_LENGTH){
      value = value.substring(0,MAX_STRING_LENGTH);
   }
   /*
   // former required because double quotes was not alloed
   if(value.indexOf("\"")>0){
       Reporter.writeWarning("Warning: String of a string-atom contains doublequotes");
       Reporter.writeWarning("         replace it by single quotes");
       value = value.replace('\"','\'');
   }*/
     ListExpr result = new ListExpr();
    result.value = value;
    result.type = STRING_ATOM;
    return result;
}
/*
1.3.6 Creating a symbol atom

*/
public static ListExpr symbolAtom(String value){
   if (ListExpr.CHECK_PRECONDITIONS) {
      if (value.length() > MAX_STRING_LENGTH) {
         Reporter.writeError("CHECK PRECONDITIONS: Error when calling the symbolAtom() method");
         Reporter.writeError("The input string is larger than "+
                             MAX_STRING_LENGTH+" characters.");
      }
    }
    ListExpr result = new ListExpr();
    result.value = value;
    result.type = SYMBOL_ATOM;
    return result;
}
/*
1.3.8 Creating an empty text atom

*/
public static ListExpr textAtom(){
    return textAtom("");
}
/*
1.3.7 Creating a text atom

*/
public static ListExpr textAtom(String value){
    ListExpr result = new ListExpr();
    result.type = TEXT_ATOM;
    result.setText(value);
    return result;
}
/** This method sets the content of an exiisting text atom */
 public void setText(String value){
    if (CHECK_PRECONDITIONS) {
      if (atomType() != ListExpr.TEXT_ATOM) {
        Reporter.writeError("CHECK PRECONDITIONS:");
        Reporter.writeError("   Error when calling the setText() method: ");
        Reporter.writeError("   the ListExpr object is not a text atom.");
        return;
      }
    }
    if(value.length()>MAX_INTERNAL_TEXT_LENGTH && USE_PERSISTENT_TEXT){
        // create a new File for this textAtom;
        if(this.value==null || !(this.value instanceof File)){ // create a new File
           File F;
           do{
              F = new File( TEMPDIR+TMPSTART+LastUsedFileNumber);
              LastUsedFileNumber++;
           }while(F.exists());
           this.value=F;
           F.deleteOnExit();
        }
        DataOutputStream out = null;
        try{ // write Text into the File
          out = new DataOutputStream(
                          new BufferedOutputStream(
                              new FileOutputStream((File)this.value)));
          out.write(value.getBytes());
          out.flush();
          out.close();
        }catch(Exception e){
            Reporter.debug(e);
            Reporter.writeError("Error in writing temporal file of text atom");
            Reporter.writeError("Using main memory to store text!");
            this.value = new String(value);
        }
    }else { // main memory based
        this.value = new String(value);
    }
}
/* Appends the text given in the argumnet text from startpos with the given length
 *  to an existing text atom.
 **/
public void appendText (String text, int startPos, int length) {
  // Copies the content of this text atom to a StringBuffer.
  StringBuffer aux = new StringBuffer(textValue());
  // Appends to it the new portion of string.
  aux.append(text.substring(startPos, startPos + length));
  // Stores in value the resulting string.
  setText(aux.toString());
}
/** Appends the given text on an existing text atom **/
public void appendText(String text){
   if(CHECK_PRECONDITIONS){
      if(atomType()!=TEXT_ATOM){
         Reporter.writeError("Error in Checking preconditions. ListExpr::appendText");
         Reporter.writeError("List is not a text");
         return;
      }
   }
   if(text==null) return; // nothing to do
   if(text.equals("")) return; // also nothing to do
    setText(textValue()+text);
}
/** Returns the int value stored in this ListExpr object.
  * Precondition: This ListExpr object must be an intAtom.
  **/
public int intValue () {
   if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.atomType() != INT_ATOM) {
        Reporter.writeError("CHECK PRECONDITIONS: Error when calling the intValue() method");
        Reporter.writeError("The atomtype is not INT_ATOM");
      }
    }
   return ((Integer)this.value).intValue();
}
/** Returns the real value stored in this ListExpr object.
  * Precondition: This ListExpr object must be a realAtom.
  **/
public double realValue () {
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.atomType() != ListExpr.REAL_ATOM) {
        Reporter.writeError("CHECK PRECONDITIONS: Error when calling the realValue() method");
        Reporter.writeError("The ListExpr object does not fulfil the preconditions.");
        return 0.0;
      }
    }
    return ((Double)this.value).doubleValue();
}
/** Returns the boolean value stored in this ListExpr object.
  * Precondition:  This ListExpr object must be a boolAtom.
  **/
public boolean boolValue () {
   if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.atomType() != ListExpr.BOOL_ATOM) {
        Reporter.writeError("CHECK PRECONDITIONS: Error when calling the boolValue() method");
        Reporter.writeError("The ListExpr is not a boolean atom");
      }
    }
    return ((Boolean)this.value).booleanValue();
}
/** Returns the string value stored in this ListExpr object.
  * Precondition:  This ListExpr object must be a stringAtom.
  **/
public String stringValue () {
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.atomType() != ListExpr.STRING_ATOM) {
        Reporter.writeError("CHECK PRECONDITIONS: Error when calling the stringValue() method");
        Reporter.writeError("The List is not a string atom.");
      }
    }
    return (String)this.value;
}
 /** Returns the string value stored in this ListExpr object.
   * Precondition: This ListExpr object must be a symbolAtom.
   */
public String symbolValue () {
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.atomType() != ListExpr.SYMBOL_ATOM) {
         Reporter.writeError("CHECK PRECONDITIONS: Error when calling the symbolValue() method");
         Reporter.writeError("The list is not a symbol");
      }
    }
    return (String)this.value;
}
/** Returns the text stored in this list node **/
public String textValue () {
    if (CHECK_PRECONDITIONS) {
      if (atomType() != TEXT_ATOM) {
        Reporter.writeError("CHECK PRECONDITIONS:");
        Reporter.writeError(" Error when calling the textValue() method:");
        Reporter.writeError(" the ListExpr object is not a text atom.");
      }
    }
    if(value==null || !(value instanceof File)){ // main memory based
       if(DEBUG_MODE && value==null){
          Reporter.writeError("Error in ListExpr: value is null");
          Reporter.printTrace();
       }
       return (String)this.value;
    }else{
       try{
         File F = (File) value;
         int len = (int)F.length();
         byte[] content = new byte[(int)len];
         BufferedInputStream in = new BufferedInputStream(new FileInputStream(F));
         int pos = 0;
         while(pos<len){
             pos += in.read(content,pos,len-pos);
         }
         String res = new String(content);
         in.close();
         return res;
       }catch(Exception e){
          Reporter.debug(e);
          Reporter.writeError("Cannot load a TextAtom value from its temporal file ");
          Reporter.writeError(" empty String is returned");
          return "";
       }
    }
}
/** Returns the length of the text contained in this ListExpr*/
public int textLength(){
   return getTextLength();
}
/** The familiar Equals method.
  */
public boolean equals(Object o){
    if(o==null)
        return false;
    if(!(o instanceof ListExpr))
       return false;
    return equals((ListExpr)o,0.0,true);
}
public boolean equals(ListExpr LE, double epsilon, boolean isAbsolute){
    if(LE==null){
      return false;
    }
    if(!equalValues(LE,epsilon,isAbsolute)){ // check type and value of the first element
       return false;
    }
    if(atomType()!=NO_ATOM){
       return true;
    }
    ListExpr TN = next;
    ListExpr LEN = LE.next;
    while(TN!=null && LEN!=null){
         if(!TN.equalValues(LEN,epsilon,isAbsolute)){ // differences found
             return false;
         }
         if(TN.type==NO_ATOM){
            TN = TN.next;
            LEN=LEN.next;
         } else{
            TN=null;
            LEN=null;
         }
    }
    if(TN!=null || LEN!=null){ // different lengths
       return false;
    }
    return true;
}
private static boolean similar(double a, double b,double epsilon, boolean isAbsolute){
  double diff = a>b? a-b:b-a;
  if(isAbsolute){
     return diff<=epsilon;
  }
  // non absolute
  double epsilon2 = a>0?epsilon*a/100:-epsilon*a/100;
  return diff<=epsilon2;
}
/**
  * Checks for euqlity of the value
  **/
private boolean equalValues(ListExpr LE,double epsilon, boolean isAbsolute){
    if(LE==null) return false; // this is not null
    if(LE.atomType()!=atomType()) // different types
       return false;
    if(value==null && !(LE.value==null))
       return false;
    if(!(value==null) && (LE.value==null))
       return false;
    // special treatment for text atoms
    if(type==TEXT_ATOM){
       String S1 = textValue();
       String S2 = LE.textValue();
       return S1.equals(S2);
    }
    if(value==null && LE.value==null){
       return true;
    }
    else{ // both value members are not null
       if(type!=REAL_ATOM && type!=NO_ATOM){
           boolean res = value.equals(LE.value);
           return res;
       }else if(type==NO_ATOM){
          return ((ListExpr)value).equals((ListExpr)LE.value,epsilon,isAbsolute);
       } else{
           return similar(realValue(),LE.realValue(),epsilon, isAbsolute);
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
      tmp = (ListExpr) tmp.next;
      i++;
   }
   return (ListExpr) tmp.value;
}
/*
   This method is implemented to support the implementation of the ~readBinaryFrom~
   method.
   */
private static ListExpr readBinaryRec(MyDataInputStream in){
try{
  int Type = in.readByte();
  switch(Type){
      case BIN_BOOLEAN : { return boolAtom(in.readBool()); }
      case BIN_BYTE : { return intAtom(in.readByte());}
      case BIN_SHORTINT : { return intAtom(in.readShort());}
      case BIN_INTEGER : { return intAtom(in.readInt());}
      case BIN_REAL : { return realAtom(in.readReal());}
      case BIN_DOUBLE : { return realAtom(in.readDouble());}
      case BIN_SHORTSTRING : { int len = getPositiveInt(in.readByte());
                                  return stringAtom(in.readString(len));
                                }
      case BIN_STRING : { int len = getPositiveInt(in.readShort());
                                  return stringAtom(in.readString(len));
        }
      case BIN_LONGSTRING : { int len = in.readInt();
                             return ListExpr.stringAtom(in.readString(len));
                          }
      case BIN_SHORTSYMBOL : { int len = getPositiveInt(in.readByte());
                             return ListExpr.symbolAtom(in.readString(len));
                          }
      case BIN_SYMBOL : { int len = getPositiveInt(in.readShort());
                             return ListExpr.symbolAtom(in.readString(len));
                          }
      case BIN_LONGSYMBOL : { int len = in.readInt();
                             return ListExpr.symbolAtom(in.readString(len));
                          }
      case BIN_SHORTTEXT : { int length = getPositiveInt(in.readByte());
                                  return ListExpr.textAtom(in.readString(length));
                          }
      case BIN_TEXT : { int length = getPositiveInt(in.readShort());
                                  return ListExpr.textAtom(in.readString(length));
                          }
      case BIN_LONGTEXT : { int length = in.readInt();
                                  return ListExpr.textAtom(in.readString(length));
                          }
     case BIN_SHORTLIST : { int length= getPositiveInt(in.readByte());
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
      case BIN_LIST : { int length= getPositiveInt(in.readShort());
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
     case BIN_LONGLIST : { int length= in.readInt();
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
    default : { Reporter.debug("unknow binary list type");
                     return null;}
  }
}
catch(Exception e){
  Reporter.debug(e);
  return null;
}
}
/** Reads the value of this nested list from a string.
  * On success, this method will return 0.
  */
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
    Reporter.debug("Error in character " + parser.charPos
      + " when parsing input string in ReadFromString()");
   // It returns the character position where the error was detected.
   return parser.charPos;
  }
  // else, it sets this ListExpr to the value returned by the parser.
  this.setValueTo((ListExpr)result.value);
 } catch (Exception except) {
  Reporter.debug(except);
  return INPUT_STRING_ERROR;
 } finally {
  inputReader.close();
 }
 return ListExpr.NO_ERROR_CODE;
}
/** Returns an inputstream from which can readed the
  * decoded data in a text atom. The List must be a text atom and
  *  must contain base64 coded content
  **/
public InputStream decodeText(){
   Base64Decoder BD = new Base64Decoder(new StringReader(textValue()));
   return BD.getInputStream();
}
/** Returns the byte representation of the contained text.
  **/
public byte[] textValueAsByteArray () {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (CHECK_PRECONDITIONS) {
      if (atomType() != TEXT_ATOM) {
        Reporter.writeError("CHECK PRECONDITIONS:");
        Reporter.writeError(" Error when calling the textValueAsByteArray() method:");
        Reporter.writeError(" the ListExpr object is not a text atom.");
      }
    }
    if(value==null || !(value instanceof File)){ // main memory based
           return ((String)this.value).getBytes();
    }else{
       try{
         File F = (File) value;
         int len = (int)F.length();
         byte[] content = new byte[(int)len];
         BufferedInputStream in = new BufferedInputStream(new FileInputStream(F));
         int pos = 0;
         while(pos<len){
             pos += in.read(content,pos,len-pos);
         }
         in.close();
         return content;
       }catch(Exception e){
          Reporter.debug(e);
          Reporter.writeError("Cannot load a TextAtom value from its temporal file ");
          Reporter.writeError(" empty String is returned");
          return new byte[0];
       }
    }
}
/** Returns an InputStream for accessing the content of a text atom.
  * If an error occurs or the list is not an InputSTream, null is
  * returned.
  **/
public InputStream textValueAsInputStream() throws IOException{
  if(atomType()!=TEXT_ATOM){
    if(CHECK_PRECONDITIONS){
       Reporter.writeError("Check preconditions failed");
       Reporter.writeError("  error in textValueAsInputStream");
       Reporter.writeError("  list is not of type text atom ");
    }
    return null;
  }
  if(!(value instanceof File)){
    return new ByteArrayInputStream( ((String)value).getBytes());
  }else{
    return new FileInputStream((File)value);
  }
}
/** Returns the atomType of this node */
public int atomType () {
    return this.type;
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
/** Returns the greates common denominator of a and b.
  **/
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
/*
1 Public fields

1.1 Definition of atom types

*/
public final static byte NO_ATOM = 0; // not an atom.
public final static byte INT_ATOM = 1; // int atom.
public final static byte REAL_ATOM = 2; // real atom.
public final static byte BOOL_ATOM = 3; // Bool atom.
public final static byte STRING_ATOM = 4; // String atom.
public final static byte SYMBOL_ATOM = 5; // Symbol atom.
public final static byte TEXT_ATOM = 6; // Text atom.
/*
1.2 Constants decribing codings for binary writing of this list

*/
public final static int BIN_LONGLIST = 0;
public final static int BIN_INTEGER = 1;
public final static int BIN_REAL = 2;
public final static int BIN_BOOLEAN = 3;
public final static int BIN_LONGSTRING = 4;
public final static int BIN_LONGSYMBOL = 5;
public final static int BIN_LONGTEXT = 6;
public final static int BIN_LIST = 10;
public final static int BIN_SHORTLIST = 11;
public final static int BIN_SHORTINT = 12;
public final static int BIN_BYTE = 13;
public final static int BIN_STRING = 14;
public final static int BIN_SHORTSTRING = 15;
public final static int BIN_SYMBOL= 16;
public final static int BIN_SHORTSYMBOL = 17;
public final static int BIN_TEXT = 18;
public final static int BIN_SHORTTEXT=19;
public final static int BIN_DOUBLE=20;
/*
2 Private Fields

2.1 Some control variables and constants

*/
private static boolean DEBUG_MODE = true;
private static final boolean CHECK_PRECONDITIONS = false;
private static int MAX_STRING_LENGTH = 48;
private final static int NO_ERROR_CODE = 0;
private final static int INPUT_STRING_ERROR = -1;
private final static int WRITE_TO_FILE_ERROR = -1;
private final static int READ_FROM_FILE_ERROR = -1;
private final static String BEGIN_TEXT ="<text>";
private final static String END_TEXT ="</text--->";
private static String CurrentDir=null;
private static final String TMPSTART="TMP_";
private static int LastUsedFileNumber=0;
private static String TEMPDIR =".";
private static int MAX_INTERNAL_TEXT_LENGTH = 256;
private static boolean USE_PERSISTENT_TEXT = false;
/*
2.2 Representation of a single node

*/
private Object value;
private byte type;
private ListExpr next;
} // class ListExpr