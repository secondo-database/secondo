/******************************************************************************
 //paragraph	[10]	title:		[\centerline{\Large \bf] [}]
 [10] ListExpr.java

 March, 1999. Jose Antonio Cotelo Lema.
 ListExpr.java
 Java implementation of the NestedLists tools.
 \tableofcontents
 ***************************************************************************** *
 * formatted with JxBeauty (c) johann.langhofer@nextra.at
 */
/*
 1 Overview.
 This is the implementation of the ListExpr class.
 This class implements the Java version of the NestedLists tool, builded fully in Java.
 The original interface was maintained. The main diferences are::
 - In any method where one of the ListExpr parameter can be clearly assumed as the object over which we execute the method, this parameter was removed, and the method codified as an object method (instead of as a class method). For example, we defined the ~isEmpty()~ object method instead of the ~isEmpty(list)~ class method, so the user will use ~list.isEmpty()~. We had defined in this way a more ''Java style'' interface.
 - In some methods returning a String as parameter, it was replaced by a StringBuffer object. This change must be done due to the fact that the content of String objects can not be changed once created, so we con not return in it a diferent content as parameter.
 The ~createTextScan()~, ~endOfText()~, ~destroyTextScan()~ and ~getText()~ method were replaced by a simplier (and less powerfull) method ~textValue()~ which returns all the text atom content. This simplifies an interface between the java code and the original NestedLists tool which otherwise would be very complex. The main drawback is that now, with the ~textValue()~ method, for getting the content of a text atom it must be retrieved in only one steep, needing to store all its content together in memory.
 The following methods were added:
 - ~appendText(String text)~: This overloaded version of the ~appendText()~ method was defined to allow an easier use of this method by Java users.
 - ~textAtom(String value)~: This overloaded version of the ~textAtom()~ method was defined to allow an easier definition of a text atom.
 - ~writeListExprToString()~: This new method was added to allow the users to get in a String the same result that the original method writeListExpr() sends to the standard output. It was added because a Java user usually will want to show it in a text component (e.g. a TextArea object) instead of the standard output.
 - ~setValueTo(ListExpr list)~: This new method was added to allow the user's methods to modify the content of a ListExpr object passed as parameter.
 Technical notes:
 - A non-atom ListExpr is represented as a list of ListExpr nodes, each of them
 storing one element of the list, and finished by the ~emptyList~ node (where
 ~value~ and ~next~ are both ~null~). Each element of this list can be either
 an atom or a non-atom ListExpr.
 - An atom ListExpr is represented as a ListExpr node where the ~next~ field
 is ~null~ and the ~value~ field stores a non ListExpr object. the object
 stored in ~value~ is one of the following types:
 - For an intAtom ListExpr, ~value~ is an Integer object.
 - For a floatAtom ListEpxr, ~value~ is a Float object.
 - For a boolAtom ListExpr, ~value~ is a Boolean object.
 - For a stringAtom, symbolAtom or textAtom ListExpr, ~value~ is a
 String object.

 This class belongs to the package:
 */


package  sj.lang;

/*
 to which belong all the base classes of SecondoJava.
 */
/*
 2 Included files.
 This class uses some external classes and packages what are neded for the correct compiling and execution of this class.
 2.1 External classes.
 The following external classes what are not part of the Java Development Kit are used.
 */
import  java_cup.runtime.Symbol;
import  sj.lang.JavaListExpr.NLParser;
/*
 2.2 External packages.
 The following packages being part of the Java Development Kit are used.
 */

/* From the java.io package */
import java.io.*;
import  tools.Base64Decoder;


/* From the Cup (parser builder) utilities */
import  java_cup.runtime.*;


/*
 3 Class implementation.
 */
/** Java implementation of the NestedLists tools.This class is part of the SecondoJava-User-Interface. See documentation there. */

public class ListExpr extends Object {
  /*
   3.1 Public fields.
   The following public fields are defined and hence can be accessed by the user code.
   */
  // Types of atoms.
  public final static int NO_ATOM = 0;          // not an atom.
  public final static int INT_ATOM = 1;         // int atom.
  public final static int REAL_ATOM = 2;        // real atom.
  public final static int BOOL_ATOM = 3;        // Bool atom.
  public final static int STRING_ATOM = 4;      // String atom.
  public final static int SYMBOL_ATOM = 5;      // Symbol atom.
  public final static int TEXT_ATOM = 6;        // Text atom.

  // Types for binary writing
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
   3.2 Private fields.
   The following private fields are defined, and hence they can be accessed only
   by the code belonging to this class.
   */
  // Defines if the class must show extra information when errors are detected.
  private static boolean DEBUG_MODE = true;
  // Defines if the class must check the preconditions and shows extra information when an input that does not fulfil the preconditions is detected.
  private static final boolean CHECK_PRECONDITIONS = true;
  // defines the maximal length for string atoms
  private static int MAX_STRING_LENGTH = 48;
  // The ~emptyList~ object.
  private final static ListExpr emptyList = new ListExpr();
  // some error codes returned by readFromString, etc...
  private final static int NO_ERROR_CODE = 0;
  private final static int INPUT_STRING_ERROR = -1;
  private final static int WRITE_TO_FILE_ERROR = -1;
  private final static int READ_FROM_FILE_ERROR = -1;
  // Here is stored the content of this ListExpr node.
  private Object value;
  // Here stores the type of the ListExpr.
  private int type;
  // Here is stored a pointer to the following mode of the ListExpr.
  private ListExpr next;
  // tag for begin of text
  private final static String BEGIN_TEXT ="<text>";
  // tag for end of text
  private final static String END_TEXT ="</text--->";
  // the current directory when reading a file
  // should ever be null if not a file is readed
  private static String CurrentDir=null;


  /*
   3.3 Class constructors.
   The following class constructors is defined for constructing new ListExpr nodes. It will return an empty list.
   */
  public ListExpr () {
    // creates a new ListExpr node, being an empty list.
    this.value = null;
    this.type = ListExpr.NO_ATOM;
    this.next = null;
  }

  /*
   3.4 Public methods.
   The following public methods are defined to access to the ListExpr objects, and hence they can be used by the user code.
   */
  /*
    3.4.0 The ~setDebugMode~ method.
    if the debug mode is enabled then additional informations are printed
    when an error is occured. */
  public static void setDebugMode(boolean enabled){
    DEBUG_MODE = enabled;
  }

  /*
    3.4.0 The getDirectory Method
    returns the current working directory.
    if not a file is parsed null is returned
  */
  public static String getDirectory(){
      return CurrentDir;
  }


  /*
    3.4.0 The ~setMaxStringLength~ method.
    set the maximal length for strings in string atoms
    */
  public static void setMaxStringLength(int len){
    if(len>0)
       MAX_STRING_LENGTH=len;
  }


  /*
   3.4.1 The ~setValueTo~ method.
   This new method was added to allow the user's methods to modify the content of a ListExpr object passed as parameter. It gets as parameter a ListExpr ~list~ and modifies the content of this object to be acopy of the ~list~ ListExpr.
   */
  public void setValueTo (ListExpr list) {
    this.type = list.type;
    this.value = list.value;
    this.next = list.next;
    return;
  }

  /*
   3.4.2 The ~theEmptyList()~ method.
   This method returns an empty ListExpr. It will return the same as creating a new ListExpr directly using the class constructor. This public method is provided to offer an interface closer to the original interface of nested lists implemented in Modula2 used by the Secondo server, and to offer an intuitive interface.
   */
  public static ListExpr theEmptyList () {
    return  new ListExpr();
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
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (right.isAtom()) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the cons() method: the input argument ~right~ does not fulfil the preconditions.");
      }
    }
    // It creates the first node for the new list.
    ListExpr result = new ListExpr();
    // it makes ~left~ the value of the first node.
    result.value = left;
    // And sets the first element of ~right~ as the following element of the list.
    result.next = right;
    // As the constructor sets a ListExpr to ~NO_ATOM~ type, ~result~
    // will be a ~NO_ATOM~ type, so we do not need to set the type to it.
    // returns the new ListExpr.
    return  result;
  }

  /*
   3.4.4 The append() method.
   This method creates a new node ~p~ and makes ~newSon~ the value of ~p~. Sets the next element of ~p~ to the empty list. Makes ~p~ the next element of ~lastElement~ and returns a pointer to ~p~. That means that now ~p~ is the last element of the list and ~lastElement~ the second last. ~append~ can now be called with ~p~ as the first argument. In this way one can build a list by a sequence of ~append~ calls.
   Note that there are no restrictions on the element ~newSon~ that is appened; it can also be the empty list.
   *Preconditions:* ~lastElement~ is not the empty list and no atom, but is the last element of a list. That is: ~endOfList(LastElem)~ = ~true~, ~isEmpty(lastElement)~ = ~false~, ~isAtom(lastElement)~ = ~false~.
   */
  public static ListExpr append (ListExpr lastElement, ListExpr newSon) {
    // It creates a new node ~p~.
    ListExpr p = new ListExpr();
    // If CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if ((!lastElement.endOfList()) || lastElement.isEmpty() || lastElement.isAtom()) {
        // If it does not fulfils the preconditions, other ListExpr could result corrupted  when building the new ListExpr. So, it shows an error message.
        System.err.println("CHECK PRECONDITIONS: Error when calling the append() method: the input argument ~lastElement~ does not fulfil the preconditions.");
      }
    }
    p.value = newSon;
    p.next = theEmptyList();
    lastElement.next = p;
    return  p;
  }

  /*
   3.4.5 The destroy() method.
   Actually it does nothing. In the original NestedList code was used to free the space used by a ListExpr. But Java does it automatically when there is no reference to this object, so its use is not needed.
   */
  public void destroy () {
  // It does nothing.
  }

  /*
   3.4.6 The isEmpty() method.
   This method returns true if this ListExpr object is an empty list, false
   otherwise.
   */
  public boolean isEmpty () {
    // If is a NO_ATOM and ~value~ and ~next~ are empty, then it is an empty list.
    if ((this.type == ListExpr.NO_ATOM) && (this.value == null) && (this.next
        == null)) {
      return  true;
    }
    // else it is not empty.
    return  false;
  }

  /*
   3.4.7 The isAtom() method.
   This method returns true if this ListExpr object is an Atom, false
   otherwise.
   */
  public boolean isAtom () {
    return  (this.type != ListExpr.NO_ATOM);
  }

  /*
   3.4.8 The endOfList() method.
   This method returns true if this ListExpr object is the end of a list, is
   not an atom and is not empty. Returns false otherwise.
   */
  public boolean endOfList () {
    if ((this.type == ListExpr.NO_ATOM) && (!this.isEmpty()) && (this.next.isEmpty())) {
      return  true;
    }
    // Else returns false.
    return  false;
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
      aux = aux.next;
    }
    return  length;
  }

  /*
   3.4.10 The writeListExpr() method.
   This method writes the content of this ListExpr object to the standard
   output in an indented way.
   */
  public void writeListExpr () {
    System.out.print(this.writeListExprToString());
    return;
  }

  /*
   3.4.11 The writeListExprToString method.
   This method returns one string with the content of this ListExpr object
   in an indented way.
   This method is defined because is usual in a Java class to show the
   messages in a window, in place of use the standard output. So in this
   cases it would be interesting to write the content of a LisExpr in a
   string in a formatted way. It is *not* defined in the original set
   of NestedLists methods.
   */
  public String writeListExprToString () {
    // It initializes the ~chars~ buffer to the empty string.
    StringBuffer chars = new StringBuffer();
    //Appends an starting end of line character.
    chars.append("\n");
    // And calls the auxiliar method.
    writeListExprAppeningToString(this, chars, "");
    return  chars.toString();
  }



  /** stores the Directory of the given filename */
  private void storeDirectory(String FileName){
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




  /**
   * put your documentation comment here
   * @param list
   * @param chars
   * @param identation
   */
  private final static void writeListExprAppeningToString (ListExpr list, StringBuffer chars,
      String identation) {
    /*
     This method is implemented to improve the performance of the implementation
     of the ~writeListExprtoString~ method. The main improvement is that it passes the buffer when calling itself recursivelly. In this way the recursive calls can work with the same buffer, avoiding using auxiliar buffers in each recursive call.
     The ~identation~ argument is used to define the indendation to use in each recursive call.
     */
    String separator = " ";
    boolean hasSubLists = false;
    identation = identation + "    ";           // It will use an identation bigger in each recursion.
    // If is not an atom appends "(" to the output.
    if (!list.isAtom()) {
      chars.append("(");        // Start of list.
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
            writeListExprAppeningToString((ListExpr)list.value, chars, identation);
            break;
          }
        case ListExpr.INT_ATOM:
          {
            chars.append(((Integer)list.value).toString());
            return;
          }
        case ListExpr.REAL_ATOM:
          {
            chars.append(((Float)list.value).toString());
            return;
          }
        case ListExpr.BOOL_ATOM:
          {                     // It writes the value in uppercase ("TRUE" or "FALSE").
            chars.append(((Boolean)list.value).toString().toUpperCase());
            return;
          }
        case ListExpr.STRING_ATOM:
          {
            chars.append("\"" + ((String)list.value).toString() + "\"");
            return;
          }
        case ListExpr.SYMBOL_ATOM:
          {
            chars.append(((String)list.value).toString());
            return;
          }
        case ListExpr.TEXT_ATOM:
          {
            chars.append(BEGIN_TEXT+((String)list.value).toString()+END_TEXT);
            return;
          }
      }
      list = list.next;
      if (!list.isEmpty()) {
        // if is not the last element adds the separator string.
        chars.append(separator);
      }
    }
    // if the code reachs this point, it was not an atom, so it appends the end of lists.
    chars.append(")");          // End of list.
    return;
  }
  ;             // End writeAppeningToString();

  /*
   3.4.12 The first() method.
   This method returns the first element of this ListExpr object, who can
   be an empty list.
   *Preconditions:* this ListExpr object can not be an atom and can not be
   empty.
   */
  public ListExpr first () {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.isAtom() || this.isEmpty()) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the first() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    return  (ListExpr)this.value;
  }

  /*
   3.4.13 The rest() method.
   This method returns the rest of this ListExpr object, this is, a ListExpr
   object what is identical to this ListExpr but without the first element. The
   result can be an empty list.
   *Preconditions:* this ListExpr object can not be an atom and can not be
   empty.
   */
  public ListExpr rest () {
    ListExpr result = new ListExpr();
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.isAtom() || this.isEmpty()) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the rest() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    return  this.next;
  }

  /*
   3.4.14 The readFromFile() method.
   This method reads a ListExpr from the file "fileName" and assigns it to
   this ListExpr object.
   It returns 0 if reading was succesful. Otherwise, returns READ\_FROM\_FILE\_ERROR (-1) if the file
   could not be accessed, or the line number in the file where an error was
   detected.
   */
  public int readFromFile (String fileName) {
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
    return  this.NO_ERROR_CODE;
  }

  /*
   3.4.15 The writeToFile() method.
   This method writes this ListExpr object to file "filename". The
   previous contents of the file will be lost.
   Returns 0 if writing was sucessful, 1 if the file could not be written
   properly.
   *Preconditions:* this listExpr object must not be an atom.
   */
  public int writeToFile (String fileName) {
    OutputStreamWriter file;
    String result;
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.isAtom()) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the writeToFile() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    try {
      storeDirectory((new File(fileName)).getAbsolutePath());
      file = new OutputStreamWriter(new FileOutputStream(fileName));
      recursiveWriteToFile(this, file, "");
      file.flush();
      file.close();
      storeDirectory(null);
    } catch (SecurityException except) {
      // If an error happened when creating the file, returns WRITE_TO_FILE_ERROR.
      if (this.DEBUG_MODE) {
        System.err.println("DEBUG MODE: Security error when opening output file in writeToFile()");
      }
      return  ListExpr.WRITE_TO_FILE_ERROR;
    } catch (IOException except) {
      // If an error happened when creating  or writing in the file,
      // returns WRITE_TO_FILE_ERROR.
      if (this.DEBUG_MODE) {
        System.err.println("DEBUG MODE: Error when opening output file in writeToFile()");
      }
      return  ListExpr.WRITE_TO_FILE_ERROR;
    }
    return  ListExpr.NO_ERROR_CODE;
  }

  /*
   This method is implemented to support the implementation of the ~writeToFile~
   method.
   */
  private final static void recursiveWriteToFile (ListExpr list, Writer file,
      String identation) throws IOException {
    String separator = " ";
    boolean hasSubLists = false;
    identation = identation + "    ";           // It will use an identation bigger in each recursion.
    // If is not an atom appends "(" to the output.
    if (!list.isAtom()) {
      file.write("(");          // Start of list.
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
              file.write(separator);
            }
            // Writes its content, using in it a deeper identation.
            recursiveWriteToFile((ListExpr)list.value, file, identation);
            break;
          }
        case ListExpr.INT_ATOM:
          {
            file.write(((Integer)list.value).toString());
            return;
          }
        case ListExpr.REAL_ATOM:
          {
            file.write(((Float)list.value).toString());
            return;
          }
        case ListExpr.BOOL_ATOM:
          {                     // It writes the value in uppercase ("TRUE" or "FALSE").
            file.write(((Boolean)list.value).toString().toUpperCase());
            return;
          }
        case ListExpr.STRING_ATOM:
          {
            file.write("\"" + ((String)list.value).toString() + "\"");
            return;
          }
        case ListExpr.SYMBOL_ATOM:
          {
            file.write(((String)list.value).toString());
            return;
          }
        case ListExpr.TEXT_ATOM:
          {
            file.write("<text>" + ((String)list.value).toString() + "</text--->");
            return;
          }
      }
      list = list.next;
      if (!list.isEmpty()) {
        // if is not the last element adds the separator string.
        file.write(separator);
      }
    }
    // if the code reachs this point, it was not an atom, so it appends the end of lists.
    file.write(")");            // End of list.
    return;
  }
  ;             // End writeAppeningToString();




  /*
   3.4.151 The writeBinaryTo() method.
   This method writes this ListExpr object to outputstream os in binary format.
   Returns true if writing was sucessful, false if the list could not be written
   properly.
  */

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


/* This method is implemented to support the implementation of the ~writeBinaryTo~
   method.
   */
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
	  case BIN_INTEGER      : {OS.writeByte(T);
	                           int value = LE.intValue();
			           OS.writeInt(value);
			           return true;
			          }
	  case BIN_SHORTINT     : {OS.writeByte(T);
	                           short value = (short) LE.intValue();
			           OS.writeShort(value);
			           return true;
			          }
	  case BIN_BYTE         : {OS.writeByte(T);
	                            byte value = (byte) LE.intValue();
			            OS.writeByte(value);
			            return true;
			           }

	  case BIN_REAL         :  {OS.writeByte(T);
	                            float value = LE.realValue();
			            OS.writeReal(value);
			            return true;
			           }
	  case BIN_SHORTSTRING  :  {OS.writeByte(T);
	                            String value = LE.stringValue();
			            OS.writeByte((byte)value.length());
			            OS.writeString(value);
			            return true;
			           }
          case BIN_STRING       :  {OS.writeByte(T);
	                            String value = LE.stringValue();
			            OS.writeShort((short)value.length());
			            OS.writeString(value);
			            return true;
			           }
          case BIN_LONGSTRING   :  {OS.writeByte(T);
	                            String value = LE.stringValue();
			            OS.writeInt(value.length());
			            OS.writeString(value);
			            return true;
			           }

	  case BIN_SHORTSYMBOL  :  {OS.writeByte(T);
	                            String value = LE.symbolValue();
			            OS.writeByte((byte)value.length());
			            OS.writeString(value);
			            return true;
			           }
          case BIN_SYMBOL       :  {OS.writeByte(T);
	                            String value = LE.symbolValue();
			            OS.writeShort((short)value.length());
			            OS.writeString(value);
			            return true;
			           }

          case BIN_LONGSYMBOL   :  {OS.writeByte(T);
	                            String value = LE.symbolValue();
			            OS.writeInt(value.length());
			            OS.writeString(value);
			            return true;
			           }

	  case BIN_SHORTTEXT    :  {OS.writeByte(T);
	                            String value = LE.textValue();
			            int L = value.length();
			            OS.writeByte((byte)L);
			            OS.writeString(value);
			            return true;
			            }
          case BIN_TEXT         :  {OS.writeByte(T);
	                            String value = LE.textValue();
			            int L = value.length();
			            OS.writeShort((short)L);
			            OS.writeString(value);
			            return true;
			            }
          case BIN_LONGTEXT    :  {OS.writeByte(T);
	                            String value = LE.textValue();
			            int L = value.length();
			            OS.writeInt(L);
			            OS.writeString(value);
			            return true;
			            }


	  case BIN_SHORTLIST    : { OS.writeByte(T);
	                            int length = LE.listLength();
			            OS.writeByte((byte)length);
                                    while(!LE.isEmpty()){
                                       if(! writeBinaryRec(LE.first(),OS)) // error in writing sublist
			                  return false;
                                       LE=LE.rest();
			            }
                                    return true;
	                           }
          case BIN_LIST         : { OS.writeByte(T);
	                            int length = LE.listLength();
			            OS.writeShort((short)length);
                                    while(!LE.isEmpty()){
                                       if(! writeBinaryRec(LE.first(),OS)) // error in writing sublist
			                  return false;
                                       LE=LE.rest();
			            }
                                    return true;
	                           }
          case BIN_LONGLIST     : { OS.writeByte(T);
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
     if(DEBUG_MODE)
        e.printStackTrace();
     return false;
   }
}


/*
   This method is implemented to support the implementation of the ~writeBinaryTo~
   method.
   */
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


  /*
   3.4.152 The readBinaryFrom() method.
   This method read a ListExpr from inputstream in.
   If an error is occurred nul is returned.
  */
public static ListExpr readBinaryFrom(InputStream In){
 try{
    MyDataInputStream DIN = new MyDataInputStream(In);
    String Sig = DIN.readString(3);
    int major = DIN.readShort();
    int minor = DIN.readShort();
    if(!Sig.equals("bnl") ){
      System.out.println("wrong signatur or version "+Sig );
      return null;
    }
    if(major!=1 | ( minor!=0 & minor!=1)){
       System.out.println("wrong version numer "+major+"."+minor);
       return null;
    }

    ListExpr LE = readBinaryRec(DIN);
    return LE;
 }
 catch(Exception e){
   if(DEBUG_MODE){
      System.out.println(e);
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

/*
   This method is implemented to support the implementation of the ~readBinaryFrom~
   method.
   */
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
      case BIN_LONGSTRING     : {  int len = in.readInt();
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


	  default      : { if(DEBUG_MODE)
	                       System.out.println("unknow binary list type");
	                   return null;}
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






  /*
   3.4.16 The readFromString() method.
   This method is the same as ReadFromFile(), but reads the nested list from
   the string "chars" instead than from a file. Returns 0 if reading was
   succesful,otherwise the position in the string where an error was
   detected.
   A valid list is a sequence of characters described by the following grammar. In it the names between '$<>$'starting with uppercases are symbols of the grammar, the names between '$<>$' starting by lowercases are names of tokens. The manes not enclosed by '$<>$' are constants, this is, are the sequence of characters that should appear.
   ----
   rule 1: <ListExpr> =	'(' <ListExprSeq> ')'
   rule 2:	|	<boolAtom>
   rule 3:	|	<intAtom>
   rule 4:	|	<realAtom>
   rule 5:	|	<stringAtom>
   rule 6:	|	<symbolAtom>
   rule 7:	|	<textAtom>
   rule 8: <ListExprSeq> = <ListExprSeq> <ListExpr>
   rule 9:	|
   The tokens accepted by this scanner follow the following rules:
   <boolAtom> =	{ 'TRUE' | 'FALSE' }
   <intAtom> =	<sign> <number>
   <sign> =	['-']  // This includes the option of having no sign.
   <realAtom> =	<sign> <number> '.' <number>
   |	<sign> <number> '.' <number> <exponent>
   |	<sign> <number> <exponent>
   <stringAtom> =	'"' <characterSeq> '"'
   <symbolAtom> =	<letter> {<letter> | <digit> | <underline>}*
   |	<otherChar> {<otherChar>}*
   <textAtom> =	'<' text '>' {<anyChar>}* '<' '\' text '-' '-' '-' '>'
   ----
   Where:
   ----
   <number> =	<digit> {<digit>}*
   <exponent> =	'E' <sign> <number>
   <characterSeq> =	<stringCharacter> <characterSeq>
   |
   <anyChar> =	<any possible character>
   <letter> =	'A'-'Z'
   |	'a'-'z'
   <digit> =	'0'-'9'
   <stringCharacter> =	<any character except the doble quote '"'
   character>
   <underline> =	<The underline character '_'>
   <otherChar> =	<any character which is neither a <letter> nor a
   <digit> nor a '"', '(' or ')'>
   ----
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

  /*
   3.4.17 The writeToString() method.
   This method writes this list to the StringBuffer object ~chars~. It will
   return 0 if no error happened, an error code otherwise.
   *Preconditions:* this ListExpr object can not be an atom.
   This method doesn't use a String object as type of the ~chars~ parameter due to
   the impossibility to change the data stored in a Java String object once it
   was created. This is why it uses a StringBuffer object as type of the ~chars~
   parameter instead.
   */
  public int writeToString (StringBuffer chars) {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.isAtom()) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the writeToString() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    // It initializes the ~chars~ buffer to the empty string.
    chars.setLength(0);
    // And calls the auxiliar method.
    return  writeAppeningToString(this, chars);
  }

  /**
   * put your documentation comment here
   * @param list
   * @param chars
   * @return
   */
  private final static int writeAppeningToString (ListExpr list, StringBuffer chars) {
    /*
     This method is implemented to improve the performance of the implementation
     of the ~writetoString~ method. The main improvement is that it does not deletes
     the content of the input StringBuffer ~chars~ (it is really needed at start,
     when writeToString() is called by the user), but only appends the text to it.
     In this way the recursive calls can work with the same buffer, avoiding using
     auxiliar buffers in each recursive call.
     */
    int result = 0;
    // If is not an atom appends "(" to the output.
    if (!list.isAtom()) {
      chars.append("(");        // Start of list.
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
              return  result;
            }
            break;
          }
        case ListExpr.INT_ATOM:
          {
            chars.append(((Integer)list.value).toString());
            return  0;
          }
        case ListExpr.REAL_ATOM:
          {
            chars.append(((Float)list.value).toString());
            return  0;
          }
        case ListExpr.BOOL_ATOM:
          {                     // It writes the value in uppercase ("TRUE" or "FALSE").
            chars.append(((Boolean)list.value).toString().toUpperCase());
            return  0;
          }
        case ListExpr.STRING_ATOM:
          {
            chars.append("\"" + ((String)list.value).toString() + "\"");
            return  0;
          }
        case ListExpr.SYMBOL_ATOM:
          {
            chars.append(((String)list.value).toString());
            return  0;
          }
        case ListExpr.TEXT_ATOM:
          {
            chars.append("<text>" + ((String)list.value).toString() + "</text--->");
            return  0;
          }
      }
      list = list.next;
      if (!list.isEmpty()) {
        // if is not the last element adds an space character as separator.
        chars.append(" ");
      }
    }
    // if the code reachs this point, it was not an atom, so it appends the end of lists.
    chars.append(")");          // End of list.
    return  0;                  // No error.
  }
  ;             // End writeAppeningToString();

  /*
   3.4.18 The oneElemList(), twoElemList(), threeElemList(), fourElemList(), fiveElemList()and sixElemList() methods.
   These methods construct a ListExpr with one, two, three, four, five and six elements, respectively.
   */
  public static ListExpr oneElemList (ListExpr elem1) {
    return  cons(elem1, theEmptyList());
  }

  /**
   * put your documentation comment here
   * @param elem1
   * @param elem2
   * @return 
   */
  public static ListExpr twoElemList (ListExpr elem1, ListExpr elem2) {
    return  cons(elem1, cons(elem2, theEmptyList()));
  }

  /**
   * put your documentation comment here
   * @param elem1
   * @param elem2
   * @param elem3
   * @return
   */
  public static ListExpr threeElemList (ListExpr elem1, ListExpr elem2, ListExpr elem3) {
    return  cons(elem1, cons(elem2, cons(elem3, theEmptyList())));
  }

  /**
   * put your documentation comment here
   * @param elem1
   * @param elem2
   * @param elem3
   * @param elem4
   * @return 
   */
  public static ListExpr fourElemList (ListExpr elem1, ListExpr elem2, ListExpr elem3, 
      ListExpr elem4) {
    return  cons(elem1, cons(elem2, cons(elem3, cons(elem4, theEmptyList()))));
  }

  /**
   * put your documentation comment here
   * @param elem1
   * @param elem2
   * @param elem3
   * @param elem4
   * @param elem5
   * @return 
   */
  public static ListExpr fiveElemList (ListExpr elem1, ListExpr elem2, ListExpr elem3, 
      ListExpr elem4, ListExpr elem5) {
    return  cons(elem1, cons(elem2, cons(elem3, cons(elem4, cons(elem5, theEmptyList())))));
  }

  /**
   * put your documentation comment here
   * @param elem1
   * @param elem2
   * @param elem3
   * @param elem4
   * @param elem5
   * @param elem6
   * @return 
   */
  public static ListExpr sixElemList (ListExpr elem1, ListExpr elem2, ListExpr elem3, 
      ListExpr elem4, ListExpr elem5, ListExpr elem6) {
    return  cons(elem1, cons(elem2, cons(elem3, cons(elem4, cons(elem5, cons(elem6, 
        theEmptyList()))))));
  }

  /*
   3.4.19 The second(), third(), fourth(), fifth() and sixth() methods.
   These methods return the second, third, fourth, fifth and sixth  element of this ListExpr object, respectively. The returned ListExpr object can be the empty list.
   *Preconditions:* This ListExpr object must not be an atom and must have at
   least two, three, fourth, five and six elements, respectively.
   The related method ~first()~ was defined above, before the ~rest()~ method, so it is not defined again here.
   */
  public ListExpr second () {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.listLength() < 2) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the second() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    return  (ListExpr)this.next.value;
  }

  /**
   * put your documentation comment here
   * @return 
   */
  public ListExpr third () {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.listLength() < 3) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the third() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    return  (ListExpr)this.next.next.value;
  }

  /**
   * put your documentation comment here
   * @return 
   */
  public ListExpr fourth () {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.listLength() < 4) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the fourth() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    return  (ListExpr)this.next.next.next.value;
  }

  /**
   * put your documentation comment here
   * @return 
   */
  public ListExpr fifth () {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.listLength() < 5) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the fifth() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    return  (ListExpr)this.next.next.next.next.value;
  }

  /**
   * put your documentation comment here
   * @return 
   */
  public ListExpr sixth () {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.listLength() < 6) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the sixth() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    return  (ListExpr)this.next.next.next.next.next.value;
  }

  /*
   3.4.20 The intAtom() method.
   This method creates a new ListExpr object of type intAtom with the
   especified value.
   */
  public static ListExpr intAtom (int value) {
    ListExpr result = new ListExpr();
    result.value = new Integer(value);
    result.type = ListExpr.INT_ATOM;
    return  result;
  }

  /*
   3.5.21 The realAtom() method.
   This method creates a new ListExpr object of type realAtom with the
   especified value.
   */
  public static ListExpr realAtom (float value) {
    ListExpr result = new ListExpr();
    result.value = new Float(value);
    result.type = ListExpr.REAL_ATOM;
    return  result;
  }

  /*
   3.4.22 The boolAtom() method.
   This method creates a new ListExpr object of type boolAtom with the
   especified value.
   */
  public static ListExpr boolAtom (boolean value) {
    ListExpr result = new ListExpr();
    result.value = new Boolean(value);
    result.type = ListExpr.BOOL_ATOM;
    return  result;
  }

  /*
   3.4.23 The stringAtom() method.
   This method creates a new ListExpr object of type stringAtom with the
   especified value.
   */
  public static ListExpr stringAtom (String value) {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (value.length() > MAX_STRING_LENGTH) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the stringAtom() method: the input string is larger than "+
	                    MAX_STRING_LENGTH+" characters.");
      }
    }
    ListExpr result = new ListExpr();
    result.value = value;
    result.type = ListExpr.STRING_ATOM;
    return  result;
  }

  /*
   3.4.24 The symbolAtom() method.
   This method creates a new ListExpr object of type symbolAtom with the
   especified value.
   */
  public static ListExpr symbolAtom (String value) {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (value.length() > MAX_STRING_LENGTH) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the symbolAtom() method: the input string is larger than "+
	                    MAX_STRING_LENGTH+" characters.");
      }
    }
    ListExpr result = new ListExpr();
    result.value = value;
    result.type = ListExpr.SYMBOL_ATOM;
    return  result;
  }

  /*
   3.4.25 The textAtom() method.
   This method is an overloaded method, with two versions:
   -*textAtom()*: This method creates a new ListExpr object as an empty textAtom.
   -*textAtom(String value)*: This method creates a new ListExpr object as a textAtom with the especified value.
   This overloaded version of the textAtom() method is defined to allow an
   easier construction of text atoms. It is NOT defined in the original set
   of NestedLists methods.
   */
  public static ListExpr textAtom () {
    ListExpr result = new ListExpr();
    result.value = new String();
    result.type = ListExpr.TEXT_ATOM;
    return  result;
  }

  /**
   * put your documentation comment here
   * @param value
   * @return 
   */
  public static ListExpr textAtom (String value) {
    ListExpr result = new ListExpr();
    result.value = new String(value);
    result.type = ListExpr.TEXT_ATOM;
    return  result;
  }

  /*
   3.4.26 the appendText() method.
   This method is an overloaded methods, having two versions:
   - *appendText(String text, int startPos, int length)*: This method appends the substring of ~text~ starting at ~startPos~ and with size ~length~ to this ListExpr object.
   *Preconditions:*
   - This ListExpr must be a textAtom.
   - The length of the especified text ~text~ must be at least ~startPos~ + ~length~.
   - *appendText(String text)*: This method appends the especified text ~text~ to this ListExpr object.
   *Preconditions:* This ListExpr must be a textAtom.
   This overloaded appendText() version is defined to allow an easier dealing with text atoms in Java code. It is NOT defined in the original set of NestedLists methods.
   */
  public void appendText (String text, int startPos, int length) {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if ((this.atomType() != ListExpr.TEXT_ATOM) || (text.length() < startPos
          + length)) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the appendText() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    // Copies the content of this text atom to a StringBuffer.
    StringBuffer aux = new StringBuffer((String)this.value);
    // Appends to it the new portion of string.
    aux.append(text.substring(startPos, startPos + length));
    // Stores in value the resulting string.
    this.value = aux.toString();
    return;
  }

  /**
   * put your documentation comment here
   * @param text
   */
  public void appendText (String text) {
    // Copies the content of this text atom to a StringBuffer.
    StringBuffer aux = new StringBuffer((String)this.value);
    // Appends to it the new portion of string.
    aux.append(text);
    // Stores in value the resulting string.
    this.value = aux.toString();
    return;
  }

  /*
   3.4.27 The intValue() method.
   This method returns the int value stored in this ListExpr object.
   *Precondition:* This ListExpr object must be an intAtom.
   */
  public int intValue () {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.atomType() != ListExpr.INT_ATOM) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the intValue() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    return  ((Integer)this.value).intValue();
  }

  /*
   3.4.28 The realValue() method.
   This method returns the real value stored in this ListExpr object.
   *Precondition:* This ListExpr object must be a realAtom.
   */
  public float realValue () {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.atomType() != ListExpr.REAL_ATOM) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the realValue() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    return  ((Float)this.value).floatValue();
  }

  /*
   3.4.29 The boolValue() method.
   This method returns the boolean value stored in this ListExpr object.
   *Precondition:* This ListExpr object must be a boolAtom.
   */
  public boolean boolValue () {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.atomType() != ListExpr.BOOL_ATOM) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the boolValue() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    return  ((Boolean)this.value).booleanValue();
  }

  /*
   3.4.30 The stringValue() method.
   This method returns the string value stored in this ListExpr object.
   *Precondition:* This ListExpr object must be a stringAtom.
   */
  public String stringValue () {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.atomType() != ListExpr.STRING_ATOM) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the stringValue() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    return  (String)this.value;
  }

  /*
   3.4.31 The symbolValue() method.
   This method returns the string value stored in this ListExpr object.
   *Precondition:* This ListExpr object must be a symbolAtom.
   */
  public String symbolValue () {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.atomType() != ListExpr.SYMBOL_ATOM) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the symbolValue() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    return  (String)this.value;
  }

  /*
   3.4.32 The textValue() method.
   This method returns the text value (a String) stored in this ListExpr
   object.
   *Precondition:* This ListExpr object must be a textAtom.
   The methods set createTextScan(), endOfText(), destroyTextScan() and
   getText() defined in the original NestedLists code in Modula are replaced
   here by this simplier (and less powerful) version named textValue().
   It returns all the text atom content.
   This simplifies the interface between the java code and the original NestedLists tool which otherwise would be very complex. The main drawback is that now, with the ~textValue()~ method, for getting the content of a text atom it must be retrieved in only one step, needing to store all its content together in memory.
   One solution to this drawback would be define a getText() method what would accept two parameters ~startPos~ and ~length~ allowing the user define which substring of the text stored into the textAtom he want to retrieve (this solution is not yet implemented).
   */
  public String textValue () {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.atomType() != ListExpr.TEXT_ATOM) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the textValue() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    return  (String)this.value;
  }

  /*
   3.4.33 The textLength() method.
   This method returns the length (in number of characters) of the text
   stored in this ListExpr object.
   *Preconditions:* this ListExpr object must be a textAtom.
   */
  public int textLength () {
    //if CHECK_PRECONDITIONS is set, it checks the preconditions.
    if (ListExpr.CHECK_PRECONDITIONS) {
      if (this.atomType() != ListExpr.TEXT_ATOM) {
        System.err.println("CHECK PRECONDITIONS: Error when calling the textLength() method: the ListExpr object does not fulfil the preconditions.");
      }
    }
    return  ((String)this.value).length();
  }

  /*
   3.4.34 The atomType() method.
   This method returns an int value representing what kind of atom is this
   ListExpr object. The meaning of the returned int is the meaning defined
   with the static fields NO\_ATOM, INT\_ATOM, REAL\_ATOM, ...
   */
  public int atomType () {
    return  this.type;
  }
}



