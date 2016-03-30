package postgis;

public abstract interface IPGTextMessages
{
  public static final String gstrPGTEXT_HINT = "Hint: \n";
  public static final String gstrPGTEXT_NOTCONNECT = "Can not connect to PostgreSQL/ PostGIS database.\nPlease check connection parameter.";
  public static final String gstrPGTEXT_ERRTYPECHOICE = "Error in converting process. Please check your type choice.";
  public static final String gstrPGTEXT_ERRCONVERTING = "Error in converting process.";
  public static final String gstrPGTEXT_PGPROGBARTITLE = "PostgreSQL to SECONDO Converting";
  public static final String gstrPGTEXT_PGPROGBARBORDER = "Converting to ";
  public static final String gstrPGTEXT_CONVERTING2MO = "Converting to moving objects";
  public static final String gstrPGTEXT_CONVERTING2MOTOOLTIP = "This button converts your data into moving objects.";
  public static final String gstrPGTEXT_PGCHOOSETEMPLATE = "Please choose a template where PostGIS extension exists if you need it.";
  public static final String gstrPGTEXT_PGTEMPLATEFRAME = "Choose Template";
  public static final String gstrPGTEXT_TOOLTIP_TOOLBARCOPY2 = "Open selection dialog to copy to SECONDO db";
  public static final String gstrPGTEXT_TOOLTIP_TOOLBARRECONNECT = "Reconnect the PostgreSQL/ PostGIS-Server";
  public static final String gstrPGTEXT_TOOLTIP_INSERTSQLWHERECONDITION = "insert a PostgreSQL where-condition for your statement: start with where";
  public static final String gstrPGTEXT_TOOLTIP_INSERTNEWOBJECTNAME = "name for the new SECONDO object";
  public static final String gstrPGTEXT_TOOLTIP_BUTTONNAME_COPY_TO_OTHER_DB = "copy to SECONDO database";
}
