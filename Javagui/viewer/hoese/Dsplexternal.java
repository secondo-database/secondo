

package  viewer.hoese;

import  java.io.*;
import  javax.swing.*;
import  sj.lang.ListExpr;
import viewer.HoeseViewer;


/**
 * The general class for all datatypes that a viewed by a external application.
 * An external type must be registered in the configuration-file. A temporal file is created
 * for the data, that need to be decoded from Base64 -format.
 * @author  Thomas Höse
 * @version 0.99 1.1.02
 */
public class Dsplexternal
    implements DsplBase {
  protected String AttrName;
  protected boolean selected;
  private boolean visible = true;
  private String FileType, Command, App;
  private static Launcher LaunchApp = new Launcher();

  /**
   * Constructor
   * @param   String app The application together with any parameters
   * @param   String type The datatype.
   * @see <a href="Dsplexternalsrc.html#Dsplexternal">Source</a>
   */
  public Dsplexternal (String app, String type) {
    FileType = type;
    App = app;
  }

  /**
   * In relations it is neccessary to get the name of the attribute of this datatype instance in
   * a tuple.
   * @return attribute name
   * @see <a href="Dsplexternalsrc.html#getAttrName">Source</a>
   */
  public String getAttrName () {
    return  AttrName;
  }

  /**
   * If this datatype shouldn't be displayed in the default 2D-geographical viewer this
   * method returns the specialized frame, which can do this.
   * @return The instance of the Launcher to execute the external app.
   * @see generic.SecondoFrame
   * @see generic.Launcher
   * @see <a href="Dsplexternalsrc.html#getFrame">Source</a>
   */
  public SecondoFrame getFrame () {
    return  LaunchApp;
  }

  /**
   * 
   * @return A String that holds the application with the temp. file
   * @see <a href="Dsplexternalsrc.html#getCommand">Source</a>
   */
  public String getCommand () {
    return  Command;
  }

  /**
   * The data of exernal-types have Base64-format. So it must be decoded and then written to 
   * a temp. file. This file will be deleted after closing the GBS.
   * @param data The data to write in Base64
   * @return The temporal File
   * @see <a href="Dsplexternalsrc.html#convertDataTofile">Source</a>
   */
  private File convertDataToFile (String data) {
    File f;
    try {
      f = File.createTempFile("GBS", null);
      FileOutputStream fw = new FileOutputStream(f);
      //convert data here
      fw.write(decode(data));
      fw.close();
      return  f;
    } catch (IOException e) {
      return  null;
    }
  }

  /**
   * Builds Commandstring out of App-name and temp. file.
   * @param app The app. String
   * @param f The temp. File
   * @return The application-name +parameters + tempfile-name
   * @see <a href="Dsplexternalsrc.html#buildCmdString">Source</a>
   */
  private String buildCmdString (String app, File f) {
    int i = app.lastIndexOf("%f");
    if (i == -1)
      return  app + " " + f.getAbsolutePath(); 
    else {
      StringBuffer sb = new StringBuffer(app);
      sb.replace(i, i + 2, f.getAbsolutePath());
      return  sb.toString();
    }
  }

  /**
   * Scans the ListExpr for a textatom with the data, then creates a temp. file, calls 
   * convertDataTofile and buildCmdString.
   * @param value The ListExpr with the data.
   * @return null if no error occurs
   * @see <a href="Dsplexternalsrc.html#scanValue">Source</a>
   */
  private String scanValue (ListExpr value) {
    if ((!value.isAtom()) || (value.atomType() != ListExpr.TEXT_ATOM))
      return  "wrong format";
    File filename = convertDataToFile(value.textValue());
    if (filename == null)
      return  "temp. file creation failed";
    filename.deleteOnExit();
    Command = buildCmdString(App, filename);
    return  null;
  }

  /**
   * This method is used to analyse the type and value in NestedList format and build
   * up the intern datastructures for this type. An alphanumeric representation is 
   * neccessary for the displaying this type in the queryresultlist.
   * @param type A symbolatom with the datatype
   * @param value The value of this object a textatom
   * @param qr The queryresultlist to add alphanumeric representation
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplexternalsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    String s = scanValue(value);
    if (s != null)
      qr.addEntry("(external?:" + s + ")"); 
    else 
      qr.addEntry(this);
    return;
  }

  /**
   * Sets the visibility of an object 
   * @param b true=show false=hide
   * @see <a href="Dsplexternalsrc.html#setVisible">Source</a>
   */
  public void setVisible (boolean b) {
    visible = b;
  }

  /**
   * Gets the visibility of an object
   * @return true if visible, false if not
   * @see <a href="Dsplexternalsrc.html#getVisible">Source</a>
   */
  public boolean getVisible () {
    return  visible;
  }

  /**
   * Sets the select status of an object, textual or graphical.
   * @param b true if selected, false if not.
   * @see <a href="Dsplexternalsrc.html#setSelected">Source</a>
   */
  public void setSelected (boolean b) {
    selected = b;
  }

  /**
   * Gets the select status of an object, textual or graphical
   * @return true if selected, false if not
   * @see <a href="Dsplexternalsrc.html#getSelected">Source</a>
   */
  public boolean getSelected () {
    return  selected;
  }

  /**
   * 
   * @return AttrName+datatype
   * @see <a href="Dsplexternalsrc.html#toString">Source</a>
   */
  public String toString () {
    return  AttrName + ":(" + FileType + " Attr.)";
  }
  /**
   * Base64 encoder :Encode some data and return a String.
   * @return A String with the Base64 data
   * @see <a href="Dsplexternalsrc.html#encode">Source</a>
   */
  public final static String encode(byte[] d)
  {
    if (d == null) return null;
    byte data[] = new byte[d.length+2];
    System.arraycopy(d, 0, data, 0, d.length);
    byte dest[] = new byte[(data.length/3)*4];

    // 3-byte to 4-byte conversion
    for (int sidx = 0, didx=0; sidx < d.length; sidx += 3, didx += 4)
    {
      dest[didx]   = (byte) ((data[sidx] >>> 2) & 077);
      dest[didx+1] = (byte) ((data[sidx+1] >>> 4) & 017 |
                  (data[sidx] << 4) & 077);
      dest[didx+2] = (byte) ((data[sidx+2] >>> 6) & 003 |
                  (data[sidx+1] << 2) & 077);
      dest[didx+3] = (byte) (data[sidx+2] & 077);
    }

    // 0-63 to ascii printable conversion
    for (int idx = 0; idx <dest.length; idx++)
    {
      if (dest[idx] < 26)     dest[idx] = (byte)(dest[idx] + 'A');
      else if (dest[idx] < 52)  dest[idx] = (byte)(dest[idx] + 'a' - 26);
      else if (dest[idx] < 62)  dest[idx] = (byte)(dest[idx] + '0' - 52);
      else if (dest[idx] < 63)  dest[idx] = (byte)'+';
      else            dest[idx] = (byte)'/';
    }

    // add padding
    for (int idx = dest.length-1; idx > (d.length*4)/3; idx--)
    {
      dest[idx] = (byte)'=';
    }
    return new String(dest);
  }

  /**
   * Base64 decoder: Decode data and return bytes.
   * @return A byte array with the decoded data.
   * @see <a href="Dsplexternalsrc.html#decode">Source</a>
   */

  public final static byte[] decode(String str)
  {
    if (str == null)  return  null;
    byte data[] = str.getBytes();
    return decode(data);
  }

  /**
   *  Base64 decoder:Decode data and return bytes.  Assumes that the data passed
   *  in is ASCII text.
   */

  public final static byte[] decode(byte[] data)
  {
    int tail = data.length;
    while (data[tail-1] == '=')  tail--;
    byte dest[] = new byte[tail - data.length/4];

    // ascii printable to 0-63 conversion
    for (int idx = 0; idx <data.length; idx++)
    {
      if (data[idx] == '=')    data[idx] = 0;
      else if (data[idx] == '/') data[idx] = 63;
      else if (data[idx] == '+') data[idx] = 62;
      else if (data[idx] >= '0'  &&  data[idx] <= '9')
        data[idx] = (byte)(data[idx] - ('0' - 52));
      else if (data[idx] >= 'a'  &&  data[idx] <= 'z')
        data[idx] = (byte)(data[idx] - ('a' - 26));
      else if (data[idx] >= 'A'  &&  data[idx] <= 'Z')
        data[idx] = (byte)(data[idx] - 'A');
    }

    // 4-byte to 3-byte conversion
    int sidx, didx;
    for (sidx = 0, didx=0; didx < dest.length-2; sidx += 4, didx += 3)
    {
      dest[didx]   = (byte) ( ((data[sidx] << 2) & 255) |
              ((data[sidx+1] >>> 4) & 3) );
      dest[didx+1] = (byte) ( ((data[sidx+1] << 4) & 255) |
              ((data[sidx+2] >>> 2) & 017) );
      dest[didx+2] = (byte) ( ((data[sidx+2] << 6) & 255) |
              (data[sidx+3] & 077) );
    }
    if (didx < dest.length)
    {
      dest[didx]   = (byte) ( ((data[sidx] << 2) & 255) |
              ((data[sidx+1] >>> 4) & 3) );
    }
    if (++didx < dest.length)
    {
      dest[didx]   = (byte) ( ((data[sidx+1] << 4) & 255) |
              ((data[sidx+2] >>> 2) & 017) );
    }
    return dest;
  }

  /**
   *  A simple test that encodes and decodes the first commandline argument.
   * @see <a href="Dsplexternalsrc.html#main">Source</a>
   */
  public static final void main(String args[])
  {
    if (args.length != 1)
    {
      System.out.println("Usage: Base64 string");
      System.exit(0);
    }
    try
    {
      String e = Dsplexternal.encode(args[0].getBytes());
      String d = new String(Dsplexternal.decode(e));
      System.out.println("Input   = '" + args[0] + "'");
      System.out.println("Encoded = '" + e + "'");
      System.out.println("Decoded = '" + d + "'");
    }
    catch (Exception x)
    {
      x.printStackTrace();
    }
  }


}



