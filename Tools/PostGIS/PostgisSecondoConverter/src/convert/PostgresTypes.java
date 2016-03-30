package convert;

 

import appGui.TableGUI;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Vector;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.swing.text.html.HTMLDocument.Iterator;

import postgis.IPostgresTypes;
import secondo.ISecondoTypes;
import secondoPostgisUtil.LogFileHandler;

public class PostgresTypes
  implements IPostgresTypes, ISecondoTypes
{
  public HashMap<String, StringBuffer> mhmPG2SecTyp;
  public HashMap<String, StringBuffer> mhmSec2PostGisTyp;
  Pattern pLine = Pattern.compile("[(].+[(]([-0-9\\.]*)\\s([-0-9\\.]*)\\s([-0-9\\.]*)\\s([-0-9\\.]*).+");
  Pattern pPath = Pattern.compile("[(]{1}[-0-9\\.]*[\\s]{1}[-0-9\\.]*[)]{2}");
  Pattern pPolygon = Pattern.compile("[(]{1}[-0-9]*[\\s]{1}[-0-9]*[)]{1}");
  Pattern pSecRealoderIntZahl = Pattern.compile("[-]?[\\d]+[\\.]{1}[\\d]+|[-]?[\\d]+");
  Pattern pLeerzeichen = Pattern.compile("[\\s]{1}");
  Pattern pRundeKlammerAuf = Pattern.compile("\\(");
  Pattern pRundeKlammerZu = Pattern.compile("\\)");
  Pattern pSecInt = Pattern.compile("[-]?[0-9]+");
  Pattern pDatumEinfach = Pattern.compile("[\"]{1}[\\d]{4}[-]{1}[\\d]{2}[-]{1}[\\d]{2}[\"]{1}");
  Pattern pPolygonStart = Pattern.compile(this.pRundeKlammerAuf.pattern() + "\\s+" + this.pRundeKlammerAuf.pattern() + "[-]?[0-9]+.+");
  Pattern pRegion = Pattern.compile(this.pSecRealoderIntZahl.pattern());
  Pattern pPoint = Pattern.compile(this.pRundeKlammerAuf.pattern() + this.pSecRealoderIntZahl.pattern() + this.pLeerzeichen.pattern() + this.pSecRealoderIntZahl.pattern() + this.pRundeKlammerZu.pattern());
  
  Matcher mMatch;

  
  public PostgresTypes()
  {
    this.mhmPG2SecTyp = new HashMap();
    this.mhmSec2PostGisTyp = new HashMap();
    
    buildHashMapType();
  }
  
  private void buildHashMapType()
  {
    this.mhmPG2SecTyp.put(sbPGINTEGER.toString(), sbSECINT);
    this.mhmPG2SecTyp.put(sbPGNUMERIC.toString(), sbSECREAL);
    this.mhmPG2SecTyp.put(sbPGREAL.toString(), sbSECREAL);
    this.mhmPG2SecTyp.put(sbPGSMALLINT.toString(), sbSECINT);
    this.mhmPG2SecTyp.put(sbPGSERIAL.toString(), sbSECINT);
    this.mhmPG2SecTyp.put(sbPGDOUBLEPRECISION.toString(), sbSECREAL);
    




    this.mhmPG2SecTyp.put(sbPGBOOLEAN.toString(), sbSECBOOL);
    



    this.mhmPG2SecTyp.put(sbPGTEXT.toString(), sbSECTEXT);
    this.mhmPG2SecTyp.put(sbPGCHARACTERVARYING.toString(), sbSECTEXT);
    this.mhmPG2SecTyp.put(sbPGCHARACTER.toString(), sbSECSTRING);
    



    this.mhmPG2SecTyp.put(sbPGDATE.toString(), sbSECINSTANT);
    this.mhmPG2SecTyp.put(sbPGTIMESTAMP.toString(), sbSECINSTANT);
    this.mhmPG2SecTyp.put(sbPGINTERVAL.toString(), sbSECDURATION);
    



    this.mhmPG2SecTyp.put(sbPGPOINT.toString(), sbSECPOINT);
    this.mhmPG2SecTyp.put(sbPGLSEG.toString(), sbSECLINE);
    this.mhmPG2SecTyp.put(sbPGBOX.toString(), sbSECRECT);
    this.mhmPG2SecTyp.put(sbPGPATH.toString(), sbSECPATH);
    this.mhmPG2SecTyp.put(sbPGPOLYGON.toString(), sbSECREGION);
    



    this.mhmPG2SecTyp.put(sbPGISPoint.toString(), sbSECPOINT);
    this.mhmPG2SecTyp.put(sbPGISLineString.toString(), sbSECPATH);
    this.mhmPG2SecTyp.put(sbPGISPolygon.toString(), sbSECREGION);
    this.mhmPG2SecTyp.put(sbPGISMulitpoint.toString(), sbSECPOINTS);
    this.mhmPG2SecTyp.put(sbPGISMultiLineString.toString(), sbSECLINE);
    
    
    this.mhmSec2PostGisTyp.put(sbPGISPoint.toString(), sbSECPOINT);
    this.mhmSec2PostGisTyp.put(sbPGISLineString.toString(), sbSECPATH);
    
    this.mhmSec2PostGisTyp.put(sbPGISPolygon.toString(), sbSECREGION);
    
   //ostGisTyp.put(s.toString(), sbPGISPolygon);
    this.mhmSec2PostGisTyp.put(sbPGISMulitpoint.toString(), sbSECPOINTS);
    
    this.mhmSec2PostGisTyp.put(sbPGISMultiLineString.toString(), sbSECLINE);



    this.mhmPG2SecTyp.put(sbPGUSERDEFINED.toString(), sbSECTEXT);
    this.mhmSec2PostGisTyp.put(sbPGUSERDEFINED.toString(), sbSECTEXT);
  }
  
  public StringBuffer convertPGType2SecType(StringBuffer _sbPGType)
  {
    StringBuffer sbType = new StringBuffer("USER-DEFINED");
    if (this.mhmPG2SecTyp.containsKey(_sbPGType.toString()))
    {
    	System.out.println(" PostgresTypes ConverPGIType2Sec " +_sbPGType.toString());
      sbType.delete(0, sbType.length());
      sbType.append((StringBuffer)this.mhmPG2SecTyp.get(_sbPGType.toString()));
    }
    else if(this.mhmSec2PostGisTyp.containsKey(_sbPGType.toString()))
    {
    	 System.out.println(" PostgresTypes ConverPGIType2Sec " +_sbPGType.toString());
    	sbType.delete(0, sbType.length());
    	sbType.append((StringBuffer)this.mhmSec2PostGisTyp.get(_sbPGType.toString()));
    }
    else
    {
      sbType.delete(0, sbType.length());
      sbType.append((StringBuffer)this.mhmPG2SecTyp.get("USER-DEFINED"));
    }
    return sbType;
  }
  
  public StringBuffer convertPGType2SecType(String _strPGType)
  {
    StringBuffer sbType = new StringBuffer("USER-DEFINED");
    if (this.mhmPG2SecTyp.containsKey(_strPGType))
    {
    	//System.out.println(" PostgresType: PostgVor dem Delete 01");
      sbType.delete(0, sbType.length());
      sbType.append((StringBuffer)this.mhmPG2SecTyp.get(_strPGType));
      System.out.println(" PostgresTypes ConverPGType2Sec " +sbType);
    }
    else if(this.mhmSec2PostGisTyp.containsKey(_strPGType.toString()))
    {
    	sbType.delete(0, sbType.length());
    	sbType.append((StringBuffer)this.mhmSec2PostGisTyp.get(_strPGType.toString()));
    	 System.out.println(" PostgresTypes ConverPGISType2Sec " +sbType);
    }
    else
    {
      sbType.delete(0, sbType.length());
      sbType.append((StringBuffer)this.mhmPG2SecTyp.get("USER-DEFINED"));
    }
    return sbType;
  }
  
  public String getPostgresValueNull(StringBuffer _sbPGType)
  {
    if (sbPGINTEGER.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGStandardINTEGER.toString();
    }
    if (sbPGNUMERIC.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGStandardNUMERIC.toString();
    }
    if (sbPGTEXT.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGStandardTEXT.toString();
    }
    if (sbPGCHARACTERVARYING48.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGStandardCHARACTERVARYING48.toString();
    }
    if (sbPGBOOLEAN.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGStandardBOOLEAN.toString();
    }
    if (sbPGINTERVAL.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGStandardINTERVAL.toString();
    }
    if (sbPGTIMESTAMP.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGStandardTIMESTAMP.toString();
    }
    if (sbPGDATE.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGStandardDATE.toString();
    }
    if (sbPGPOINT.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGStandardPOINT.toString();
    }
    if (sbPGLSEG.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGStandardLSEG.toString();
    }
    if (sbPGPATH.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGStandardPATH.toString();
    }
    if (sbPGPOLYGON.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGStandardPOLYGON.toString();
    }
    if (sbPGBOX.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGStandardBOX.toString();
    }
    if (sbPGISPoint.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGISStandardPoint.toString();
    }
    if (sbPGISLineString.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGISStandardLineString.toString();
    }
    if (sbPGISPolygon.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGISStandardPolygon.toString();
    }
    if (sbPGISMulitpoint.toString().intern() == _sbPGType.toString().intern()) {
      return sbPGISStandardMulitpoint.toString(); 
    }
    if (sbPGISMultiLineString.toString().intern() == _sbPGType.toString().intern()) {
        return sbPGISStandardMultiLineString.toString(); 
      }
    return "";
  }
  
  public String getPostgresValue(String _strIN, StringBuffer _sbPGType)
  {
    try
    {
      if ((_strIN.intern() == sbSECUNDEFINDED.toString().intern()) || 
        (_strIN.intern() == sbSECUNDEFNULL.toString().intern()) || 
        (_strIN.intern() == "".intern())) {
        return sbPGNULL.toString();
      }
      if (sbPGINTEGER.toString().intern() == _sbPGType.toString().intern()) {
        return _strIN;
      }
      if (sbPGREAL.toString().intern() == _sbPGType.toString().intern()) {
        return _strIN;
      }
      if (sbPGNUMERIC.toString().intern() == _sbPGType.toString().intern()) {
        return _strIN;
      }
      if (sbPGTEXT.toString().intern() == _sbPGType.toString().intern())
      {
        _strIN = _strIN.replace("<text>", "");
        _strIN = _strIN.replace("</text--->", "");
        _strIN = _strIN.replaceAll("'", "''");
        return "'" + _strIN + "'";
      }
      if (sbPGCHARACTERVARYING48.toString().intern() == _sbPGType.toString().intern())
      {
        if (_strIN.length() >= 2)
        {
          _strIN = _strIN.replaceFirst("\"", "");
          _strIN = _strIN.substring(0, _strIN.lastIndexOf("\""));
        }
        _strIN = _strIN.replaceAll("'", "''");
        return "'" + _strIN + "'";
      }
      if (sbPGBOOLEAN.toString().intern() == _sbPGType.toString().intern()) {
        return _strIN;
      }
      if (sbPGINTERVAL.toString().intern() == _sbPGType.toString().intern())
      {
        _strIN = _strIN.replace('(', '\'');
        _strIN = _strIN.replace(" ", " days ");
        return _strIN.replace(")", " ms'");
      }
      if (sbPGTIMESTAMP.toString().intern() == _sbPGType.toString().intern())
      {
        Matcher mMatch = this.pDatumEinfach.matcher(_strIN);
        if (mMatch.matches())
        {
          _strIN = _strIN.replace("\"", "");
          

          return "'" + _strIN + " 00:00:00'";
        }
        _strIN = _strIN.replace("\"", "");
        int iLastMinus = _strIN.lastIndexOf("-");
        return "'" + _strIN.substring(0, iLastMinus) + " " + _strIN.substring(iLastMinus + 1) + "'";
      }
      if (sbPGDATE.toString().intern() == _sbPGType.toString().intern())
      {
        _strIN = _strIN.replace("\"", "");
        
        String[] strSplit = _strIN.split("[.]");
        if (strSplit.length == 3) {
          return "'" + strSplit[2] + "-" + strSplit[1] + "-" + strSplit[0] + "'";
        }
        return sbPGNULL.toString();
      }
      if (sbPGPOINT.toString().intern() == _sbPGType.toString().intern())
      {
        _strIN = _strIN.replace(" ", ",");
        return "POINT" + _strIN;
      }
      if (sbPGLSEG.toString().intern() == _sbPGType.toString().intern())
      {
    	
        Matcher mMatch = this.pLine.matcher(_strIN);
        if (mMatch.matches())
        {
          StringBuffer sbResult = new StringBuffer();
          if (mMatch.groupCount() == 4)
          {
            sbResult.append("'[(");
            sbResult.append(mMatch.group(1));
            sbResult.append(",");
            sbResult.append(mMatch.group(2));
            sbResult.append("),(");
            sbResult.append(mMatch.group(3));
            sbResult.append(",");
            sbResult.append(mMatch.group(4));
            sbResult.append(")]'");
            
            return sbResult.toString();
          }
          return sbPGNULL.toString();
        }
        return sbPGNULL.toString();
      }
      if (sbPGPATH.toString().intern() == _sbPGType.toString().intern())
      {
        StringBuffer sbResult = new StringBuffer();
        
        sbResult.append("'(");
        Matcher mMatch = this.pPath.matcher(_strIN);
        while (mMatch.find())
        {
          sbResult.append(_strIN.substring(mMatch.start(), mMatch.end() - 1).replace(' ', ','));
          sbResult.append(",");
        }
        if (sbResult.length() == 2) {
          return sbPGNULL.toString();
        }
        sbResult = new StringBuffer(sbResult.substring(0, sbResult.length() - 1));
        sbResult.append(")'");
        
        return sbResult.toString();
      }
      if (sbPGPOLYGON.toString().intern() == _sbPGType.toString().intern())
      {
        StringBuffer sbResult = new StringBuffer();
        
        sbResult.append("'(");
        
        Matcher mMatch = this.pPolygon.matcher(_strIN);
        while (mMatch.find())
        {
          sbResult.append("(");
          sbResult.append(_strIN.substring(mMatch.start() + 1, mMatch.end() - 1).replace(' ', ','));
          sbResult.append("),");
        }
        if (sbResult.length() == 2) {
          return sbPGNULL.toString();
        }
        sbResult = new StringBuffer(sbResult.substring(0, sbResult.length() - 1));
        sbResult.append(")'");
        
        return sbResult.toString();
      }
      if (sbPGBOX.toString().intern() == _sbPGType.toString().intern())
      {
        StringBuffer sbResult = new StringBuffer();
        _strIN = _strIN.replace("(", "");
        _strIN = _strIN.replace(")", "");
        
        String[] strSplit = _strIN.split(" ");
        if (strSplit.length == 4)
        {
          sbResult.append("'((");
          sbResult.append(strSplit[1]);
          sbResult.append(",");
          sbResult.append(strSplit[3]);
          sbResult.append("),(");
          sbResult.append(strSplit[0]);
          sbResult.append(",");
          sbResult.append(strSplit[2]);
          sbResult.append("))'");
          
          return sbResult.toString();
        }
        return sbPGNULL.toString();
      }
      if (sbPGISPoint.toString().intern() == _sbPGType.toString().intern())
      {
        Matcher mMatch = this.pPoint.matcher(_strIN);
        if (mMatch.find()) {
          return "'POINT" + _strIN + "'";
        }
        return sbPGISStandardPoint.toString();
      }
      if (sbPGISMulitpoint.toString().intern() == _sbPGType.toString().intern())
      {
        Matcher mMatch = this.pSecRealoderIntZahl.matcher(_strIN);
        
        StringBuffer sbResult = new StringBuffer();
        
        sbResult.append("'MULTIPOINT(");
        
        int iKomma = 1;
        while (mMatch.find())
        {
          sbResult.append(mMatch.group());
          if (iKomma % 2 == 0) {
            sbResult.append(",");
          } else {
            sbResult.append(" ");
          }
          iKomma++;
        }
        if (iKomma > 1)
        {
          sbResult.deleteCharAt(sbResult.length() - 1);
          sbResult.append(")'");
          
          return sbResult.toString();
        }
        return sbPGISStandardMulitpoint.toString();
      }
      if (sbPGISLineString.toString().intern() == _sbPGType.toString().intern())
      {
        Matcher mMatch = this.pPath.matcher(_strIN);
        
        StringBuffer sbResult = new StringBuffer();
        sbResult.append("'LINESTRING(");
        
        int iFind = 0;
        while (mMatch.find())
        {
          iFind++;
          sbResult.append(_strIN.substring(mMatch.start(), mMatch.end() - 1).replaceAll("\\(", "").replaceAll("\\)", ""));
          sbResult.append(",");
        }
        if (iFind > 0)
        {
          sbResult.deleteCharAt(sbResult.length() - 1);
          sbResult.append(")'");
          return sbResult.toString();
        }
        return sbPGISStandardLineString.toString();
      }
      if (sbPGISPolygon.toString().intern() == _sbPGType.toString().intern())
      {
        StringBuffer sbResult = new StringBuffer();
        String strValue = "";
        
        Matcher mMatch = this.pPolygonStart.matcher(_strIN);
        if (mMatch.matches())
        {
          mMatch = this.pSecInt.matcher(_strIN);
          
          sbResult.delete(0, sbResult.length());
          
          String strX1 = "";
          String strXn = "";
          String strY1 = "";
          String strYn = "";
          
          sbResult.append("'POLYGON((");
          int iKommaLeerzeichen = 1;
          while (mMatch.find())
          {
            strValue = mMatch.group();
            sbResult.append(strValue);
            if (iKommaLeerzeichen == 1) {
              strX1 = strValue;
            }
            if (iKommaLeerzeichen == 2) {
              strY1 = strValue;
            }
            if (iKommaLeerzeichen % 2 == 1)
            {
              sbResult.append(" ");
              strXn = strValue;
            }
            else
            {
              sbResult.append(",");
              strYn = strValue;
            }
            iKommaLeerzeichen++;
          }
          if ((strX1.intern() != strXn.intern()) || (strY1.intern() != strYn.intern()))
          {
            sbResult.append(strX1);
            sbResult.append(" ");
            sbResult.append(strY1);
            sbResult.append(",");
          }
          if (iKommaLeerzeichen == 1) {
            return sbPGISStandardPolygon.toString();
          }
          sbResult.deleteCharAt(sbResult.length() - 1);
          sbResult.append("))'");
          return sbResult.toString();
        }
        String[] strSplitArrRegion = _strIN.split("\\)\\)");
        
        sbResult.append("'POLYGON(");
        

        int iKommaLeerzeichen = 1;
        for (int i = 0; i < strSplitArrRegion.length; i++)
        {
          if (i >= 1) {
            sbResult.append(",");
          }
          sbResult.append("(");
          
          mMatch = this.pRegion.matcher(strSplitArrRegion[i]);
          
          String strX1 = "";
          String strY1 = "";
          String strXn = "";
          String strYn = "";
          

          iKommaLeerzeichen = 1;
          while (mMatch.find())
          {
            strValue = mMatch.group();
            sbResult.append(strValue);
            if (iKommaLeerzeichen == 1) {
              strX1 = strValue;
            }
            if (iKommaLeerzeichen == 2) {
              strY1 = strValue;
            }
            if (iKommaLeerzeichen % 2 == 1)
            {
              sbResult.append(" ");
              strXn = strValue;
            }
            else
            {
              sbResult.append(",");
              strYn = strValue;
            }
            iKommaLeerzeichen++;
          }
          if ((strX1.intern() != strXn.intern()) || (strY1.intern() != strYn.intern()))
          {
            sbResult.append(strX1);
            sbResult.append(" ");
            sbResult.append(strY1);
            sbResult.append(",");
          }
          sbResult.deleteCharAt(sbResult.length() - 1);
          sbResult.append(")");
        }
        sbResult.append(")'");
        if (iKommaLeerzeichen == 1) {
          return sbPGISStandardPolygon.toString();
        }
        return sbResult.toString();
      }
      
      if (sbPGISMultiLineString.toString().intern() == _sbPGType.toString().intern())
      {
    	  
    	  
    	  Matcher mMatch = this.pLine.matcher(_strIN);
    	  
    	//  System.out.println("Test " +_strIN);
          StringBuffer sbResult = new StringBuffer();
        //  sbResult.append("'MULTILINESTRING(");
    	  
          
          String strValue = "";
    	 
    	  if (mMatch.matches())
          {
    		  strValue = mMatch.group();
            StringBuffer sbResult1 = new StringBuffer();
           sbResult.append("'MULTILINESTRING(");
            
            int i = 0;
         //   System.out.print(" GpCount " +strValue);
//            while (i < strValue.length())
//            {i++;
	           // if(mMatch.groupCount() == 4) 
	            //{
	            	//System.out.print(" GpCount Testons  " );
	            	//sbResult.append("Etien ");
	            	// strValue = mMatch.group();
	            	 int x = strValue.length()-1;
	            	
	            	//This was true
	               while(x > 0)
	               {
            
	            	 
	            	sbResult.append("(");
	            	 
	                sbResult.append(mMatch.group(1)); 
	                sbResult.append(" ");
	              
	                sbResult.append(mMatch.group(2));
	                
	                sbResult.append(", ");
	               
	                sbResult.append(mMatch.group(i+2));
	                sbResult.append(" ");
	                sbResult.append(mMatch.group(i+3));
	               
	                sbResult.append(")");
	                sbResult.append(" ");
	                sbResult.append(",");
	                x--;
	              }
	            i++;
	            System.out.println("Test " +sbResult);
           }
            if(sbResult.length() > 0)
            {
            	sbResult.setLength(sbResult.length() -1);
             sbResult.append(")'");
             System.out.println("Test " +sbResult);
            return sbResult.toString();
            }
 // }		}
            else
    	  return sbPGISMultiLineString.toString();
      }
      }
    catch (Exception exp)
    {
      return sbPGNULL.toString();
    }
    catch (Error err)
    {
      return sbPGNULL.toString();
    }
    return sbPGNULL.toString();
  }
  
  public void showSupportedTypes()
  {
    LogFileHandler.mlogger.info("try to show supported postgres and postgis types");
    TableGUI tbl = new TableGUI();
    
    Vector<String> vColNames = new Vector();
    vColNames.addElement("PostgreSQL/PostGIS Type");
    vColNames.addElement("SECONDO Type");
    vColNames.addElement("SECONDO example value");
    
    Vector<Vector> vrowData = new Vector();
    Vector<String> vrowLine = new Vector();
    
    SecondoTypes secTypes = new SecondoTypes();
    for (Map.Entry<String, StringBuffer> entry : this.mhmPG2SecTyp.entrySet())
    {
      vrowLine = new Vector();
      

      vrowLine.addElement((String)entry.getKey());
      vrowLine.addElement(((StringBuffer)entry.getValue()).toString());
      vrowLine.addElement(secTypes.getSecondoValueNull((StringBuffer)entry.getValue()));
      
      vrowData.addElement(vrowLine);
    }
    tbl.init(vrowData, vColNames, "Types PostgreSQL to SECONDO");
  }
  
  public boolean isPostGisType(String _strType)
  {
    if (sbPGISGeomCollection.toString().intern() == _strType.intern()) {
      return true;
    }
    if (sbPGISLineString.toString().intern() == _strType.intern()) {
      return true;
    }
    if (sbPGISMulitpoint.toString().intern() == _strType.intern()) {
      return true;
    }
    if (sbPGISMultiLineString.toString().intern() == _strType.intern()) {
      return true;
    }
    if (sbPGISMultiPolygon.toString().intern() == _strType.intern()) {
      return true;
    }
    if (sbPGISPoint.toString().intern() == _strType.intern()) {
      return true;
    }
    if (sbPGISPolygon.toString().intern() == _strType.intern()) {
      return true;
    }
    return false;
  }
}
