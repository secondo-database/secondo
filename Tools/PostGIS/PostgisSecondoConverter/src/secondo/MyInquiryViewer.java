package secondo;

import sj.lang.ListExpr;

public class MyInquiryViewer
{
  private static final String DATABASES = "databases";
  private static final String CONSTRUCTORS = "constructors";
  private static final String OPERATORS = "operators";
  private static final String ALGEBRAS = "algebras";
  private static final String ALGEBRA = "algebra";
  private static final String TYPES = "types";
  private static final String OBJECTS = "objects";
  private String HeaderColor = "silver";
  private String CellColor = "white";
  
  private String getStringValue(ListExpr atom)
  {
    int at = atom.atomType();
    String res = "";
    switch (at)
    {
    case 0: 
      return "";
    case 1: 
      return "You have " +atom.intValue(); 
    case 3: 
      return atom.boolValue() ? "TRUE" : "FALSE";
    case 2: 
      return "You have " +atom.realValue();
    case 4: 
      res = atom.stringValue(); break;
    case 6: 
      res = atom.textValue(); break;
    case 5: 
      res = atom.symbolValue(); break;
    default: 
      return "";
    }
    res = replaceAll("&", res, "&amp");
    res = replaceAll("<", res, "&lt;");
    res = replaceAll(">", res, "&gt;");
    return res;
  }
  
  private static String replaceAll(String what, String where, String ByWhat)
  {
    StringBuffer res = new StringBuffer();
    int lastpos = 0;
    int len = what.length();
    int index = where.indexOf(what, lastpos);
    while (index >= 0)
    {
      if (index > 0) {
        res.append(where.substring(lastpos, index));
      }
      res.append(ByWhat);
      lastpos = index + len;
      index = where.indexOf(what, lastpos);
    }
    res.append(where.substring(lastpos));
    return res.toString();
  }
  
  private String formatEntry(ListExpr LE)
  {
    if (LE.listLength() != 3) {
      return "";
    }
    ListExpr Name = LE.first();
    ListExpr Properties = LE.second();
    ListExpr Values = LE.third();
    Properties.listLength();Values.listLength();
    



    String res = " <tr><td class=\"opname\" colspan=\"2\">" + 
      Name.symbolValue() + "</td></tr>\n";
    while (((Properties.isEmpty() ? 0 : 1) & (Values.isEmpty() ? 0 : 1)) != 0)
    {
      res = 
      

        res + "   <tr><td class=\"prop\">" + getStringValue(Properties.first()) + "</td>" + "<td class=\"value\">" + getStringValue(Values.first()) + "</td></tr>\n";
      Properties = Properties.rest();
      Values = Values.rest();
    }
    while (!Properties.isEmpty())
    {
      res = 
      
        res + "   <tr><td class=\"prop\">" + getStringValue(Properties.first()) + "</td>" + "<td>   </td></tr>\n";
      Properties = Properties.rest();
    }
    while (!Values.isEmpty())
    {
      res = 
      
        res + "   <tr><td> </td>" + "<td class=\"value\">" + getStringValue(Values.first()) + "</td></tr>\n";
      Values = Values.rest();
    }
    return res;
  }
  
  private String getHTMLHead()
  {
    StringBuffer res = new StringBuffer();
    res.append("<html>\n");
    res.append("<head>\n");
    res.append("<title> inquiry viewer </title>\n");
    res.append("<style type=\"text/css\">\n");
    res.append("<!--\n");
    res.append("td.opname { background-color:" + this.HeaderColor + 
      "; font-family:monospace;" + 
      "font-weight:bold; " + 
      "color:green; font-size:x-large;}\n");
    res.append("td.prop  {background-color:" + this.CellColor + 
      "; font-family:monospace; font-weight:bold; color:blue}\n");
    res.append("td.value {background-color:" + this.CellColor + 
      "; font-family:monospace; color:black;}\n");
    res.append("-->\n");
    res.append("</style>\n");
    res.append("</head>\n");
    return res.toString();
  }
  
  private String getHTMLCode_Constructors(ListExpr ValueList)
  {
    StringBuffer res = new StringBuffer();
    if (ValueList.isEmpty()) {
      return "no type constructors are defined <br>";
    }
    res.append("<table border=\"2\">\n");
    while (!ValueList.isEmpty())
    {
      res.append(formatEntry(ValueList.first()));
      ValueList = ValueList.rest();
    }
    res.append("</table>\n");
    return res.toString();
  }
  
  private String getHTMLCode_Operators(ListExpr ValueList)
  {
    if (ValueList.isEmpty()) {
      return "no operators are defined <br>";
    }
    return getHTMLCode_Constructors(ValueList);
  }
  
  private String getHTMLCode_Databases(ListExpr Value)
  {
    if (Value.isEmpty()) {
      return "no database exists <br>";
    }
    StringBuffer res = new StringBuffer();
    res.append("<ul>\n");
    while (!Value.isEmpty())
    {
      res.append("<li> " + Value.first().symbolValue() + " </li>");
      Value = Value.rest();
    }
    res.append("</ul>");
    return res.toString();
  }
  
  private String getHTMLCode_Objects(ListExpr Value)
  {
    ListExpr tmp = Value.rest();
    if (tmp.isEmpty()) {
      return "no existing objects";
    }
    StringBuffer res = new StringBuffer();
    res.append("<h2> Objects - short list </h2>\n ");
    res.append("<ul>\n");
    while (!tmp.isEmpty())
    {
      res.append("  <li>" + tmp.first().second().symbolValue() + " </li> \n");
      tmp = tmp.rest();
    }
    res.append("</ul><br><hr><br>");
    res.append("<h2> Objects - full list </h2>\n");
    res.append("<pre>\n" + Value.rest().writeListExprToString() + "</pre>");
    return res.toString();
  }
  
  private String getHTMLCode_Types(ListExpr Value)
  {
    ListExpr tmp = Value.rest();
    if (tmp.isEmpty()) {
      return "no existing type";
    }
    StringBuffer res = new StringBuffer();
    res.append("<h2> Types - short list </h2>\n ");
    res.append("<ul>\n");
    while (!tmp.isEmpty())
    {
      res.append("  <li>" + tmp.first().second().symbolValue() + " </li> \n");
      tmp = tmp.rest();
    }
    res.append("</ul><br><hr><br>");
    res.append("<h2> Types - full list </h2>\n");
    res.append("<pre>\n" + Value.rest().writeListExprToString() + "</pre>");
    return res.toString();
  }
  
  private String getHTMLCode_Algebras(ListExpr Value)
  {
    if (Value.isEmpty()) {
      return "no algebra is included <br> please check your Secondo installation <br>";
    }
    return getHTMLCode_Databases(Value);
  }
  
  private String getHTMLCode_Algebra(ListExpr Value)
  {
    StringBuffer res = new StringBuffer();
    res.append("<h1> Algebra " + Value.first().symbolValue() + " </h1>\n");
    res.append("<h2> type constructors of algebra: " + 
      Value.first().symbolValue() + " </h2>\n");
    res.append(getHTMLCode_Constructors(Value.second().first()));
    res.append("<br>\n<h2> operators of algebra: " + 
      Value.first().symbolValue() + "</h2>\n");
    res.append(getHTMLCode_Operators(Value.second().second()));
    return res.toString();
  }
  
  private String getHTMLCode(ListExpr VL)
  {
    StringBuffer Text = new StringBuffer();
    Text.append(getHTMLHead());
    Text.append("<body>\n");
    String inquiryType = VL.first().symbolValue();
    if (inquiryType.equals("databases"))
    {
      Text.append("<h1> Databases </h1>\n");
      Text.append(getHTMLCode_Algebras(VL.second()));
    }
    else if (inquiryType.equals("algebras"))
    {
      Text.append("<h1> Algebras </h1>\n");
      Text.append(getHTMLCode_Algebras(VL.second()));
    }
    else if (inquiryType.equals("constructors"))
    {
      Text.append("<h1> Type Constructors </h1>\n");
      Text.append(getHTMLCode_Constructors(VL.second()));
    }
    else if (inquiryType.equals("operators"))
    {
      Text.append("<h1> Operators </h1>\n");
      Text.append(getHTMLCode_Operators(VL.second()));
    }
    else if (inquiryType.equals("algebra"))
    {
      Text.append(getHTMLCode_Algebra(VL.second()));
    }
    else if (inquiryType.equals("objects"))
    {
      Text.append("<h1> Objects </h1>\n");
      Text.append(getHTMLCode_Objects(VL.second()));
    }
    else if (inquiryType.equals("types"))
    {
      Text.append("<h1> Types </h1>\n");
      Text.append(getHTMLCode_Types(VL.second()));
    }
    Text.append("\n</body>\n</html>\n");
    return Text.toString();
  }
  
  public StringBuffer getHTMLCode(MySecondoObject o)
  {
    if (!canDisplay(o)) {
      return new StringBuffer("");
    }
    ListExpr VL = o.toListExpr().second();
    
    return new StringBuffer(getHTMLCode(VL));
  }
  
  public boolean canDisplay(MySecondoObject o)
  {
    ListExpr LE = o.toListExpr();
    if (LE.listLength() != 2) {
      return false;
    }
    if ((LE.first().atomType() != 5) || 
      (!LE.first().symbolValue().equals("inquiry"))) {
      return false;
    }
    ListExpr VL = LE.second();
    if (VL.listLength() != 2) {
      return false;
    }
    ListExpr SubTypeList = VL.first();
    if (SubTypeList.atomType() != 5) {
      return false;
    }
    String SubType = SubTypeList.symbolValue();
    if ((SubType.equals("databases")) || (SubType.equals("constructors")) || 
      (SubType.equals("operators")) || (SubType.equals("algebra")) || 
      (SubType.equals("algebras")) || (SubType.equals("objects")) || 
      (SubType.equals("types"))) {
      return true;
    }
    return false;
  }
}
