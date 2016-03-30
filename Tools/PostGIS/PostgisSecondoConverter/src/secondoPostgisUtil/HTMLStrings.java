package secondoPostgisUtil;

public class HTMLStrings
{
  public StringBuffer generateJTreeServer(StringBuffer sbServer)
  {
    StringBuffer sbTreeServer = new StringBuffer();
    
    sbTreeServer.append("<html><div style=\"color:green\"><b>");
    sbTreeServer.append(sbServer);
    sbTreeServer.append("</b></div></html>");
    return sbTreeServer;
  }
  
  public StringBuffer generateJTreeDatenbank(StringBuffer sbDatenbankname)
  {
    StringBuffer sbTreeDatenbank = new StringBuffer();
    sbTreeDatenbank.append("<html><div style=\"color:green\">");
    sbTreeDatenbank.append(sbDatenbankname);
    sbTreeDatenbank.append("</div></html>");
    return sbTreeDatenbank;
  }
  
  public StringBuffer generateJTreeTabelle(StringBuffer sbTabelle)
  {
    StringBuffer sbTreeTabelle = new StringBuffer();
    sbTreeTabelle.append("<html><div style=\"color:#008B8B\">");
    sbTreeTabelle.append(sbTabelle);
    sbTreeTabelle.append("</div></html>");
    return sbTreeTabelle;
  }
  
  public StringBuffer generateJTreeSpalte(StringBuffer sbSpaltenName, StringBuffer sbSpaltenTyp)
  {
    StringBuffer sbTreeSpalte = new StringBuffer();
    sbTreeSpalte.append("<html><div style=\"color:#4B0082\">");
    sbTreeSpalte.append(sbSpaltenName);
    if (sbSpaltenTyp.length() > 0) {
      sbTreeSpalte.append(":");
    }
    sbTreeSpalte.append("<i>");
    sbTreeSpalte.append(sbSpaltenTyp);
    sbTreeSpalte.append("</i>");
    sbTreeSpalte.append("</div></html>");
    
    return sbTreeSpalte;
  }
  
  public StringBuffer generateJTreeSpaltePrimaryKey(StringBuffer sbSpaltenName, StringBuffer sbSpaltenTyp)
  {
    StringBuffer sbTreeSpalte = new StringBuffer();
    sbTreeSpalte.append("<html><div style=\"color:#4B0082\"><b>");
    sbTreeSpalte.append(sbSpaltenName);
    sbTreeSpalte.append("</b>:");
    sbTreeSpalte.append("<i>");
    sbTreeSpalte.append(sbSpaltenTyp);
    sbTreeSpalte.append("</i>");
    sbTreeSpalte.append("</div></html>");
    
    return sbTreeSpalte;
  }
  
  public StringBuffer generateJTreeSpalteForeignKey(StringBuffer sbSpaltenName, StringBuffer sbSpaltenTyp)
  {
    StringBuffer sbTreeSpalte = new StringBuffer();
    sbTreeSpalte.append("<html><div style=\"color:#4B0082\"><b><u>");
    sbTreeSpalte.append(sbSpaltenName);
    sbTreeSpalte.append("</u></b>:");
    sbTreeSpalte.append("<i>");
    sbTreeSpalte.append(sbSpaltenTyp);
    sbTreeSpalte.append("</i>");
    sbTreeSpalte.append("</div></html>");
    return sbTreeSpalte;
  }
  
  public String strHTMLReplace(String strReplace)
  {
    String strRep = "";
    
    strRep = strReplace;
    strRep = strRep.replace("<html>", "");
    strRep = strRep.replace("</html>", "");
    strRep = strRep.replaceAll("(<div style=\"color:)*", "");
    strRep = strRep.substring(strRep.indexOf(">") + 1, strRep.length());
    strRep = strRep.replaceAll("\">", "");
    strRep = strRep.replace("</div>", "");
    
    strRep = strRep.replace("<i>", "");
    strRep = strRep.replace("</i>", "");
    strRep = strRep.replace("]", "");
    
    return strRep;
  }
}
