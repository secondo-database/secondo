package secondo;

/*
 *List of the messages which will be used in the whole application  
 * 
 *
 */

public abstract interface ISECTextMessages
{
  public static final String gstrSECTEXT_WRONGDB = "Wrong database name.";
  public static final String gstrSECTEXT_WRONGTBL = "Wrong table name.";
  public static final String gstrSECTEXT_NOTCONNECT = "Can not connect to SECONDO database.\nPlease checkconnection parameters.";
  public static final String gstrSECTEXT_SECPROGBARTITLE = "SECONDO to PostgreSQL Converting";
  public static final String gstrSECTEXT_SECPROGBARBORDER = "Converting to ";
  public static final String gstrSECTEXT_TOOLTIP_TOOLBARCOPY2 = "Open selection dialog to copy to PostgreSQL db";
  public static final String gstrSECTEXT_TOOLTIP_TOOLBARRECONNECT = "Reconnet to SECONDO-Server";
  public static final String gstrSECTEXT_ERRCONVERTING = "Error in converting process.";
  public static final String gstrSECTEXT_GUITREEOBJECT = "OBJECTS";
  public static final String gstrSECTEXT_GUITREETYPES = "TYPES";
  public static final String gstrSECTEXT_TOOLTIP_INSERTSQLWHERECONDITION = "insert a SECONDO filter condition for your statement";
  public static final String gstrSECTEXT_TOOLTIP_INSERTNEWOBJECTNAME = "name for the new PostgreSQL table";
  public static final String gstrSECTEXT_TOOLTIP_BUTTONNAME_COPY_TO_OTHER_DB = "copy to PostgreSQL database";
  public static final String gstrSECTEXT_QUERYINFO = "Your query can not be analysed \nand may need a lot of time.\nDo you really want to continue?\nYour query was:";
  public static final String gstrSECTEXT_ERRMOTEXT = "Enter object name. Object name can not be same as an existing object name or can not use an operator name.";
  public static final String gstrSECTEXT_ERRMOGROUPBY = "Enter a group by definition.";
  public static final String gstrSECTEXT_ERRMONEWCOLUMNS = "No columns were selected.";
  public static final String gstrSECTEXT_ERRMOCOLEXISTS = "Please choose another attribute name.";
  public static final String gstrSECTEXT_ERRMOGROUPBYDOPPELT = "You can not use a column at groupby definition and select the same column at existing column.";
  public static final String gstrSECTEXT_ERRINSTANTCOL = "No instant attribute available.";
}
