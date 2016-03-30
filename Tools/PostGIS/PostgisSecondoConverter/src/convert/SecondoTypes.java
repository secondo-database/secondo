package convert;

import appGui.TableGUI;

import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Vector;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import postgis.IPostgresTypes;
import secondo.ISecondoTypes;
import secondoPostgisUtil.LogFileHandler;

public class SecondoTypes
implements IPostgresTypes, ISecondoTypes
{
public HashMap<String, StringBuffer> mhmSec2PGTyp;
public HashMap<String, StringBuffer> mhmSec2PostGisTyp;
Pattern pIntervalPG = Pattern.compile("([-0-9]*):([-0-9]*):([-0-9]*)");
Pattern pIntervalPG1 = Pattern.compile("([-0-9]*)  ([-0-9]*):([-0-9]*):([-0-9]*)");
Pattern pDatum = Pattern.compile("[\\d]{4}[-]{1}[\\d]{2}[-]{1}[\\d]{2}");
Pattern pPGISPolygon = Pattern.compile("[(]{1}([-\\d\\s\\.,]{1,})[)]{1}");
Pattern pGISPolygonPoint = Pattern.compile("([-]?[\\d]{1,}[\\.]{1}[\\d]{1,}|[-]?[\\d]{1,})");
Matcher mMatch;

public SecondoTypes()
{
  this.mhmSec2PGTyp = new HashMap();
  this.mhmSec2PostGisTyp = new HashMap();
  
  buildHashMapType();
}

private void buildHashMapType()
{
  this.mhmSec2PGTyp.put(sbSECINT.toString(), sbPGINTEGER);
  this.mhmSec2PGTyp.put(sbSECREAL.toString(), sbPGNUMERIC);
  



  this.mhmSec2PGTyp.put(sbSECBOOL.toString(), sbPGBOOLEAN);
  


  this.mhmSec2PGTyp.put(sbSECTEXT.toString(), sbPGTEXT);
  this.mhmSec2PGTyp.put(sbSECSTRING.toString(), sbPGCHARACTERVARYING48);
  this.mhmSec2PGTyp.put(sbSECLABEL.toString(), sbPGCHARACTERVARYING48);
  


  this.mhmSec2PGTyp.put(sbSECDURATION.toString(), sbPGINTERVAL);
  this.mhmSec2PGTyp.put(sbSECINSTANT.toString(), sbPGTIMESTAMP);
  this.mhmSec2PGTyp.put(sbSECDATE.toString(), sbPGTIMESTAMP);
  


  this.mhmSec2PGTyp.put(sbSECPOINT.toString(), sbPGPOINT);
  this.mhmSec2PGTyp.put(sbSECLINE.toString(), sbPGLSEG);
  this.mhmSec2PGTyp.put(sbSECPATH.toString(), sbPGPATH);
  this.mhmSec2PGTyp.put(sbSECREGION.toString(), sbPGPOLYGON);
  this.mhmSec2PGTyp.put(sbSECRECT.toString(), sbPGBOX);
  


  this.mhmSec2PostGisTyp.put(sbSECREGION.toString(), sbPGISPolygon);
  
  this.mhmSec2PostGisTyp.put(sbSECPOINT.toString(), sbPGISPoint);
  
  this.mhmSec2PostGisTyp.put(sbSECPATH.toString(), sbPGISLineString);
 // this.mhmSec2PostGisTyp.put(sbSECREGION.toString(), sbPGISPolygon);
  this.mhmSec2PostGisTyp.put(sbSECPOINTS.toString(), sbPGISMulitpoint);
  
  this.mhmSec2PostGisTyp.put(sbSECLINE.toString(), sbPGISMultiLineString);
  



  this.mhmSec2PGTyp.put(sbSECUSERDEFINED.toString(), sbPGTEXT);
}

public String convertSecType2PGType(String _strSecType, boolean _bUsePostGISTypes)
{
  StringBuffer sbType = new StringBuffer("USER-DEFINED");
  if (_bUsePostGISTypes) {
    if (this.mhmSec2PostGisTyp.containsKey(_strSecType))
    {
      sbType.delete(0, sbType.length());
      sbType.append((StringBuffer)this.mhmSec2PostGisTyp.get(_strSecType));
      return sbType.toString();
    }
  }
  if (this.mhmSec2PGTyp.containsKey(_strSecType))
  {
    sbType.delete(0, sbType.length());
    sbType.append((StringBuffer)this.mhmSec2PGTyp.get(_strSecType));
  }
  else
  {
    sbType.delete(0, sbType.length());
    sbType.append((StringBuffer)this.mhmSec2PGTyp.get("USER-DEFINED"));
  }
  return sbType.toString();
}

public String getSecondoValueNull(StringBuffer _sbSecType)
{
  if (sbSECINT.toString().intern() == _sbSecType.toString().intern()) {
    return sbSECStandardINT.toString();
  }
  if (sbSECREAL.toString().intern() == _sbSecType.toString().intern()) {
    return sbSECStandardREAL.toString();
  }
  if (sbSECBOOL.toString().intern() == _sbSecType.toString().intern()) {
    return sbSECStandardBOOL.toString();
  }
  if (sbSECSTRING.toString().intern() == _sbSecType.toString().intern()) {
    return sbSECStandardSTRING.toString();
  }
  if (sbSECLABEL.toString().intern() == _sbSecType.toString().intern()) {
    return sbSECStandardSTRING.toString();
  }
  if (sbSECTEXT.toString().intern() == _sbSecType.toString().intern()) {
    return sbSECStandardTEXT.toString();
  }
  if (sbSECDATE.toString().intern() == _sbSecType.toString().intern()) {
    return sbSECStandardDATE.toString();
  }
  if (sbSECINSTANT.toString().intern() == _sbSecType.toString().intern()) {
    return sbSECStandardINSTANT.toString();
  }
  if (sbSECDURATION.toString().intern() == _sbSecType.toString().intern()) {
    return sbSECStandardDURATION.toString();
  }
  if (sbSECPOINT.toString().intern() == _sbSecType.toString().intern()) {
    return sbSECStandardPOINT.toString();
  }
  if (sbSECLINE.toString().intern() == _sbSecType.toString().intern()) {
    return sbSECStandardLINE.toString();
  }
  if (sbSECRECT.toString().intern() == _sbSecType.toString().intern()) {
    return sbSECStandardRECT.toString();
  }
  if (sbSECPATH.toString().intern() == _sbSecType.toString().intern()) {
    return sbSECStandardPATH.toString();
  }
  if (sbSECPOINTS.toString().intern() == _sbSecType.toString().intern()) {
    return sbSECStandardPOINTS.toString();
  }
 // if (sbSECPOLYGON.toString().intern() == _sbSecType.toString().intern()) {
//    return sbSECStandardPOLYGON.toString();
//  }
  if (sbSECREGION.toString().intern() == _sbSecType.toString().intern()) {
    return sbSECStandardREGION.toString();
  }
  return "";
}

public String getSecondoValue(String _strIN, StringBuffer _sbSecType)
{
  try
  {
    if ((_strIN.intern() == "".intern()) || (_strIN.intern() == "null".intern())) {
      return sbSECUNDEFNULL.toString();
    }
    if (sbSECINT.toString().intern() == _sbSecType.toString().intern()) {
      return _strIN.replaceAll("[^0-9+-]", "");
    }
    if (sbSECREAL.toString().intern() == _sbSecType.toString().intern()) {
      return _strIN.replaceAll("[^0-9+-\\.]", "");
    }
    if (sbSECBOOL.toString().intern() == _sbSecType.toString().intern())
    {
      if ((_strIN.equals("TRUE")) || (_strIN.equals("t")) || (_strIN.equals("true")) || (_strIN.equals("y")) || 
        (_strIN.equals("yes")) || (_strIN.equals("on")) || (_strIN.equals("1"))) {
        return "TRUE";
      }
      return "FALSE";
    }
    if (sbSECSTRING.toString().intern() == _sbSecType.toString().intern())
    {
      _strIN = _strIN.replaceAll("\"", "");
      if (_strIN.length() > 48) {
        _strIN = _strIN.substring(0, 48);
      }
      return "\"" + _strIN + "\"";
    }
    if (sbSECTEXT.toString().intern() == _sbSecType.toString().intern()) {
      return "(<text>" + _strIN + "</text--->)";
    }
    if (sbSECDATE.toString().intern() == _sbSecType.toString().intern()) {
      return "\"" + _strIN + "\"";
    }
    if (sbSECINSTANT.toString().intern() == _sbSecType.toString().intern())
    {
      this.mMatch = this.pDatum.matcher(_strIN);
      if (this.mMatch.matches())
      {
        _strIN = _strIN + "-00:00:00";
        
        return "\"" + _strIN + "\"";
      }
      _strIN = _strIN.replace(" ", "-");
      return "\"" + _strIN + "\"";
    }
    if (sbSECDURATION.toString().intern() == _sbSecType.toString().intern())
    {
      StringBuffer sbResult = new StringBuffer();
      if (_strIN.length() == 0) {
        return sbSECUNDEFNULL.toString();
      }
      this.mMatch = this.pIntervalPG.matcher(_strIN);
      if (this.mMatch.matches())
      {
        sbResult.append("(0 ");
        sbResult.append(makeMS(this.mMatch.group(1), this.mMatch.group(2), this.mMatch.group(3)));
        sbResult.append(")");
        
        return sbResult.toString();
      }
      if (_strIN.indexOf('.') != -1) {
        _strIN = _strIN.substring(0, _strIN.indexOf('.'));
      }
      int iValue = Integer.valueOf(_strIN).intValue();
      int irest = iValue % 86400;
      int idays = (iValue - irest) / 86400;
      
      irest *= 1000;
      
      sbResult.append("(");
      sbResult.append(idays);
      sbResult.append(" ");
      sbResult.append(irest);
      sbResult.append(")");
      

      return sbResult.toString();
    }
    if (sbSECPOINT.toString().intern() == _sbSecType.toString().intern())
    {
      _strIN = _strIN.replaceAll(",", " ");
      return _strIN.replaceAll("[A-Za-z]*", "");
    }
    if (sbSECLINE.toString().intern() == _sbSecType.toString().intern())
    {
    	System.out.println(" sbSeCLINE 100 " + _sbSecType.toString());
    	System.out.println(" sbSeCLINE 101 " + _strIN.toString());
    	StringBuffer sbResult = new StringBuffer();
    	 if (_strIN.startsWith("MULTILINESTRING"))  
         {
    		 System.out.println(" 1 csbSeCLINE " + _sbSecType.toString());
           _strIN = _strIN.replace("MULTILINESTRING", "");
           _strIN = _strIN.replace("(", "");
           _strIN = _strIN.replace(")", "");
          
           String[] strSplit = _strIN.split(",");
           
           sbResult.append("(");
           int iPathIndexSecondo = 0;
           for (int i = 0; i < strSplit.length; i++)
           {
        	   iPathIndexSecondo++;
             String[] strSplit1 = strSplit[i].split(" ");
             if(iPathIndexSecondo == 1)
             {
            	 sbResult.append("(");
            	 
             }
             //sbResult.append(" (");
            // sbResult.append(iPathIndexSecondo++);
            
            // sbResult.append(" (");
             sbResult.append(Float.valueOf(strSplit1[0]));
             
             sbResult.append(" ");
             sbResult.append(Float.valueOf(strSplit1[1]));
              
            sbResult.append(" ");
            // sbResult.append("))");
             if(iPathIndexSecondo == 2)
             {
            	 sbResult.append(")");
            	 iPathIndexSecondo = 0;
             }
             if (i < strSplit.length - 1) {
             //  sbResult.append(" 1.0");
             } else {
               sbResult.append(")");
             }
           }
          // System.out.println("1 csbSeCLINE" + sbResult.toString());
          // return sbResult.toString();
         }
    	 else
    	 {
    	
		      _strIN = _strIN.replace("[(", "((");
		      _strIN = _strIN.replace(")]", "))");
		      _strIN = _strIN.replace("),(", " ");
		      //return _strIN.replace(",", " ");
		      String[] strSplit = _strIN.split(",");
		        
		        sbResult.append("(");
		        
		        int iPathIndexSecondo = 0;
		        for (int i = 0; i < strSplit.length; i += 2)
		        {
		        	iPathIndexSecondo++;
		          if(iPathIndexSecondo == 1)
		          {
		        	  sbResult.append(" (");
		          }
		          
		         // sbResult.append(" (");
		          sbResult.append(Float.valueOf(strSplit[i]));
		          sbResult.append(" ");
		          sbResult.append(Float.valueOf(strSplit[(i + 1)]));
		          
		          sbResult.append(" ");
		          if(iPathIndexSecondo == 2)
		          {
		        	  sbResult.append(")");
		        	  iPathIndexSecondo = 0;
		          }
		          
		          //	sbResult.append("))");
		          if (i + 2 < strSplit.length) {
		           // sbResult.append(" 1.0");
		          } else {
		            sbResult.append(")");
		          }
		        }
    	 }
    	 System.out.println("1 csbSeCLINE" + sbResult.toString());
    	 return sbResult.toString();
    }
    if (sbSECRECT.toString().intern() == _sbSecType.toString().intern())
    {
      StringBuffer sbResult = new StringBuffer();
      
      _strIN = _strIN.replace("),(", ",");
      _strIN = _strIN.replace("(", "");
      _strIN = _strIN.replace(")", "");
      
      String[] strINArray = _strIN.split(",");
      if (strINArray.length == 4)
      {
        sbResult.append("(");
        sbResult.append(strINArray[2]);
        sbResult.append(" ");
        sbResult.append(strINArray[0]);
        sbResult.append(" ");
        sbResult.append(strINArray[3]);
        sbResult.append(" ");
        sbResult.append(strINArray[1]);
        sbResult.append(")");
        
        return sbResult.toString();
      }
      return sbSECUNDEFNULL.toString();
    }
    if (sbSECPATH.toString().intern() == _sbSecType.toString().intern())
    {
      StringBuffer sbResult = new StringBuffer();
      if (_strIN.startsWith("LINESTRING"))
      {
        _strIN = _strIN.replace("LINESTRING", "");
        _strIN = _strIN.replace("(", "");
        _strIN = _strIN.replace(")", "");
        
        String[] strSplit = _strIN.split(",");
        
        sbResult.append("(");
        int iPathIndexSecondo = 0;
        for (int i = 0; i < strSplit.length; i++)
        {
          String[] strSplit1 = strSplit[i].split(" ");
          sbResult.append(" (");
          sbResult.append(iPathIndexSecondo++);
          sbResult.append(" (");
          sbResult.append(Float.valueOf(strSplit1[0]));
          sbResult.append(" ");
          sbResult.append(Float.valueOf(strSplit1[1]));
          sbResult.append("))");
          if (i < strSplit.length - 1) {
            sbResult.append(" 1.0");
          } else {
            sbResult.append(")");
          }
        }
      }
      else
      {
        _strIN = _strIN.replace("(", "");
        _strIN = _strIN.replace(")", "");
        _strIN = _strIN.replace("[", "");
        _strIN = _strIN.replace("]", "");
        
        String[] strSplit = _strIN.split(",");
        
        sbResult.append("(");
        
        int iPathIndexSecondo = 0;
        for (int i = 0; i < strSplit.length; i += 2)
        {
          sbResult.append(" (");
          sbResult.append(iPathIndexSecondo++);
          sbResult.append(" (");
          sbResult.append(Float.valueOf(strSplit[i]));
          sbResult.append(" ");
          sbResult.append(Float.valueOf(strSplit[(i + 1)]));
          sbResult.append("))");
          if (i + 2 < strSplit.length) {
            sbResult.append(" 1.0");
          } else {
            sbResult.append(")");
          }
        }
      }
      return sbResult.toString();
    }
    if (sbSECPOINTS.toString().intern() == _sbSecType.toString().intern())
    {
      _strIN = _strIN.replace("MULTIPOINT", "");
      _strIN = _strIN.replaceAll(",", ") (");
      return "(" + _strIN + ")";
    }
   // if (sbSECPOLYGON.toString().intern() == _sbSecType.toString().intern())
    if (sbSECREGION.toString().intern() == _sbSecType.toString().intern())
    {
    	System.out.println(" SecondoType Region " +_sbSecType.toString().intern());
      StringBuffer sbResult = new StringBuffer();
      if (_strIN.startsWith("POLYGON"))
      {
    	  System.out.println(" SecondoType Region in if " +_sbSecType.toString().intern());
        _strIN = _strIN.replace("POLYGON", "");
        _strIN = _strIN.replace("((", "");
        _strIN = _strIN.replace("))", "");
      //  sbResult.append("(");
   // return "(" + _strIN + ")";
        
        String[] strSplit = _strIN.split(",");
        
        sbResult.append("(((");
        for (int i = 0; i < strSplit.length; i++)
        {
          String[] strNeuSplit = strSplit[i].split(" ");
          sbResult.append(" (");
          if (strNeuSplit[0].indexOf('.') != -1) {
           // sbResult.append(Integer.valueOf(strNeuSplit[0].substring(0, strNeuSplit[0].indexOf('.'))));
            sbResult.append(strNeuSplit[0]);
          } else {
            sbResult.append((strNeuSplit[0]));
          }
          sbResult.append(" ");
          if (strNeuSplit[1].indexOf('.') != -1) {
          //  sbResult.append(Integer.valueOf(strNeuSplit[1].substring(0, strNeuSplit[1].indexOf('.'))));
        	  sbResult.append(strNeuSplit[1]);
          } else {
           // sbResult.append(Integer.valueOf(strNeuSplit[1]));
        	  sbResult.append(strNeuSplit[1]);
          }
          sbResult.append(")");
        }
       sbResult.append(")))");    
       }
//      else
//      {
//        _strIN = _strIN.replace("(", "");
//        _strIN = _strIN.replace(")", "");
//        
//        String[] strSplit = _strIN.split(",");
//        
//
//
//        sbResult.append("(");
//        for (int i = 0; i < strSplit.length; i += 2)
//        {
//          sbResult.append(" (");
//          if (strSplit[i].indexOf('.') != -1) {
//            sbResult.append(Integer.valueOf(strSplit[i].substring(0, strSplit[i].indexOf('.'))));
//          } else {
//            sbResult.append(Integer.valueOf(strSplit[i]));
//          }
//          sbResult.append(" ");
//          if (strSplit[(i + 1)].indexOf('.') != -1) {
//            sbResult.append(Integer.valueOf(strSplit[(i + 1)].substring(0, strSplit[(i + 1)].indexOf('.'))));
//          } else {
//            sbResult.append(Integer.valueOf(strSplit[(i + 1)]));
//          }
//          sbResult.append(")");
//        }
//        sbResult.append(")");
     // }
      return sbResult.toString();
   }
    if (sbSECREGION.toString().intern() == _sbSecType.toString().intern())
    {
      StringBuffer sbResult = new StringBuffer();
      


      Matcher m = this.pPGISPolygon.matcher(_strIN);
      int iFindPoly = 0;
      
      sbResult.append("(((");
      Matcher m1;
      for (; m.find(); m1.find())
      {
        if (iFindPoly > 0) {
          sbResult.append(")(");
        }
        iFindPoly++;
        String str = m.group();
        

        m1 = this.pGISPolygonPoint.matcher(str);
        
        int iKlammer = 1;
        //EN
      //  continue;
        if (iKlammer % 2 == 1)
        {
          sbResult.append("(");
          sbResult.append(m1.group());
          sbResult.append(" ");
        }
        else
        {
          sbResult.append(m1.group());
          sbResult.append(")");
        }
        iKlammer++;
      }
      sbResult.append(")))");
      
      return sbResult.toString();
    }
  }
  catch (Exception exp)
  {
    return sbSECUNDEFNULL.toString();
  }
  catch (Error err)
  {
    return sbSECUNDEFNULL.toString();
  }
  return sbSECUNDEFNULL.toString();
}

private StringBuffer generateDuration(String _strIN)
{
  StringBuffer sbRes = new StringBuffer();
  sbRes.append("(");
  if (_strIN.indexOf("days") != -1)
  {
    _strIN = _strIN.replace("days", "");
    if (_strIN.indexOf(":") >= 0)
    {
      String[] strSplit = _strIN.split("  ");
      sbRes.append(strSplit[0]);
      sbRes.append(" ");
      sbRes.append(makeMS(strSplit[1]));
    }
    else
    {
      sbRes.append(_strIN);
      sbRes.append(" ");
      sbRes.append(0);
    }
  }
  else if (_strIN.indexOf("day") != -1)
  {
    _strIN = _strIN.replace("day", "");
    if (_strIN.indexOf(":") >= 0)
    {
      String[] strSplit = _strIN.split("  ");
      sbRes.append(strSplit[0]);
      sbRes.append(" ");
      sbRes.append(makeMS(strSplit[1]));
    }
    else
    {
      sbRes.append(_strIN);
      sbRes.append(" ");
      sbRes.append(0);
    }
  }
  else if (_strIN.indexOf("months") != -1)
  {
    _strIN = _strIN.replace("months", "");
    if (_strIN.indexOf(":") >= 0)
    {
      String[] strSplit = _strIN.split("  ");
      sbRes.append(Integer.valueOf(strSplit[0]).intValue() * 30);
      sbRes.append(" ");
      sbRes.append(makeMS(strSplit[1]));
    }
    else
    {
      _strIN = _strIN.replaceAll(" ", "");
      sbRes.append(Integer.valueOf(_strIN).intValue() * 30);
      sbRes.append(" ");
      sbRes.append(0);
    }
  }
  else if (_strIN.indexOf("month") != -1)
  {
    _strIN = _strIN.replace("month", "");
    if (_strIN.indexOf(":") >= 0)
    {
      String[] strSplit = _strIN.split("  ");
      sbRes.append(Integer.valueOf(strSplit[0]).intValue() * 30);
      sbRes.append(" ");
      sbRes.append(makeMS(strSplit[1]));
    }
    else
    {
      sbRes.append(30);
      sbRes.append(" ");
      sbRes.append(0);
    }
  }
  else if (_strIN.indexOf("years") != -1)
  {
    _strIN = _strIN.replace("years", "");
    if (_strIN.indexOf(":") >= 0)
    {
      String[] strSplit = _strIN.split("  ");
      sbRes.append(Integer.valueOf(strSplit[0]).intValue() * 360);
      sbRes.append(" ");
      sbRes.append(makeMS(strSplit[1]));
    }
    else
    {
      _strIN = _strIN.replaceAll(" ", "");
      sbRes.append(Integer.valueOf(_strIN).intValue() * 360);
      sbRes.append(" ");
      sbRes.append(0);
    }
  }
  else if (_strIN.indexOf("year") != -1)
  {
    _strIN = _strIN.replace("year", "");
    if (_strIN.indexOf(":") >= 0)
    {
      String[] strSplit = _strIN.split("  ");
      sbRes.append(Integer.valueOf(strSplit[0]).intValue() * 360);
      sbRes.append(" ");
      sbRes.append(makeMS(strSplit[1]));
    }
    else
    {
      _strIN = _strIN.replaceAll(" ", "");
      sbRes.append(Integer.valueOf(_strIN).intValue() * 360);
      sbRes.append(" ");
      sbRes.append(0);
    }
  }
  sbRes.append(")");
  if (sbRes.toString().intern() == "()".intern())
  {
    sbRes.delete(0, sbRes.length());
    sbRes.append("(0 0)");
  }
  return sbRes;
}

private String makeMS(String strHH, String strMM, String strSS)
{
  int iHH = 0;
  int iMM = 0;
  int iSS = 0;
  int iMS = 0;
  
  iHH = Integer.valueOf(strHH).intValue();
  iMM = Integer.valueOf(strMM).intValue();
  iSS = Integer.valueOf(strSS).intValue();
  
  iHH = iHH * 60 * 60 * 1000;
  iMM = iMM * 60 * 1000;
  iSS *= 1000;
  
  iMS = iHH + iMM + iSS;
  
  return String.valueOf(iMS);
}

private String makeMS(String str)
{
  String[] strSplit = str.split(":");
  
  int iHH = 0;
  int iMM = 0;
  int iSS = 0;
  int iMS = 0;
  
  iHH = Integer.valueOf(strSplit[0]).intValue();
  iMM = Integer.valueOf(strSplit[1]).intValue();
  iSS = Integer.valueOf(strSplit[2]).intValue();
  
  iHH = iHH * 60 * 60 * 1000;
  iMM = iMM * 60 * 1000;
  iSS *= 1000;
  
  iMS = iHH + iMM + iSS;
  
  return String.valueOf(iMS);
}

public void showSupportedTypes()
{
  LogFileHandler.mlogger.info("try to show supported SECONDO types");
  
  TableGUI tbl = new TableGUI();
  
  Vector<String> vColNames = new Vector();
  vColNames.addElement("SECONDO Type");
  vColNames.addElement("PostgreSQL/PostGIS Type");
  vColNames.addElement("PostgreSQL example value");
  
  Vector<Vector> vrowData = new Vector();
  Vector<String> vrowLine = new Vector();
  PostgresTypes pgTypes = new PostgresTypes();
  for (Map.Entry<String, StringBuffer> entry : this.mhmSec2PGTyp.entrySet())
  {
    vrowLine = new Vector();
    
    vrowLine.addElement((String)entry.getKey());
    vrowLine.addElement(((StringBuffer)entry.getValue()).toString());
    vrowLine.addElement(pgTypes.getPostgresValueNull((StringBuffer)entry.getValue()));
    vrowData.addElement(vrowLine);
  }
  vrowLine = new Vector();
  
  vrowLine.addElement("POSTGIS DATA TYPES");
  vrowLine.addElement("POSTGIS DATA TYPES");
  vrowLine.addElement("");
  vrowData.addElement(vrowLine);
  for (Map.Entry<String, StringBuffer> entry : this.mhmSec2PostGisTyp.entrySet())
  {
    vrowLine = new Vector();
    
    vrowLine.addElement((String)entry.getKey());
    vrowLine.addElement(((StringBuffer)entry.getValue()).toString());
    vrowLine.addElement(pgTypes.getPostgresValueNull((StringBuffer)entry.getValue()));
    vrowData.addElement(vrowLine);
  }
  vrowLine = new Vector();
  
  vrowLine.addElement("MOVING OBJECTS DATA TYPES");
  vrowLine.addElement("MOVING OBJECTS DATA TYPES");
  vrowLine.addElement("");
  vrowData.addElement(vrowLine);
  for (Map.Entry<String, StringBuffer> entry : this.mhmSec2PGTyp.entrySet()) {
    if (canBecomeMO((String)entry.getKey()))
    {
      vrowLine = new Vector();
      
      vrowLine.addElement(Normal2MOType((String)entry.getKey()));
      
      vrowLine.addElement(convertSecType2PGType(((String)entry.getKey()).toString(), false));
      vrowLine.addElement(pgTypes.getPostgresValueNull((StringBuffer)entry.getValue()));
      
      vrowData.addElement(vrowLine);
    }
  }
  for (Map.Entry<String, StringBuffer> entry : this.mhmSec2PostGisTyp.entrySet()) {
    if (canBecomeMO((String)entry.getKey()))
    {
      vrowLine = new Vector();
      
      vrowLine.addElement(Normal2MOType((String)entry.getKey()));
      
      vrowLine.addElement(convertSecType2PGType(((String)entry.getKey()).toString(), true));
      vrowLine.addElement(pgTypes.getPostgresValueNull((StringBuffer)entry.getValue()));
      
      vrowData.addElement(vrowLine);
    }
  }
  tbl.init(vrowData, vColNames, "Types SECONDO to PostgreSQL");
}

public boolean canBecomeMO(String _strType)
{
  if (sbSECBOOL.toString().intern() == _strType.intern()) {
    return true;
  }
  if (sbSECINT.toString().intern() == _strType.intern()) {
    return true;
  }
  if (sbSECLABEL.toString().intern() == _strType.intern()) {
    return true;
  }
  if (sbSECLINE.toString().intern() == _strType.intern()) {
    return true;
  }
  if (sbSECPOINT.toString().intern() == _strType.intern()) {
    return true;
  }
  if (sbSECREAL.toString().intern() == _strType.intern()) {
    return true;
  }
  if (sbSECREGION.toString().intern() == _strType.intern()) {
    return true;
  }
  if (sbSECSTRING.toString().intern() == _strType.intern()) {
    return true;
  }
  return false;
}

public boolean isMOType(String _strType)
{
  if (sbSECMBOOL.toString().intern() == _strType.intern()) {
    return true;
  }
  if (sbSECMINT.toString().intern() == _strType.intern()) {
    return true;
  }
  if (sbSECMLABEL.toString().intern() == _strType.intern()) {
    return true;
  }
  if (sbSECMLINE.toString().intern() == _strType.intern()) {
    return true;
  }
  if (sbSECMPOINT.toString().intern() == _strType.intern()) {
    return true;
  }
  if (sbSECMREAL.toString().intern() == _strType.intern()) {
    return true;
  }
  if (sbSECMREGION.toString().intern() == _strType.intern()) {
    return true;
  }
  if (sbSECMSTRING.toString().intern() == _strType.intern()) {
    return true;
  }
  if(sbSECUPOINT.toString().intern() == _strType.intern()){
	  return true;
  }
  
  return false;
}

public String MOType2NormalType(String _strType)
{
  if (sbSECMBOOL.toString().intern() == _strType.intern()) {
    return sbSECBOOL.toString();
  }
  if (sbSECMINT.toString().intern() == _strType.intern()) {
    return sbSECINT.toString();
  }
  if (sbSECMLABEL.toString().intern() == _strType.intern()) {
    return sbSECLABEL.toString();
  }
  if (sbSECMLINE.toString().intern() == _strType.intern()) {
    return sbSECLINE.toString();
  }
  if (sbSECMPOINT.toString().intern() == _strType.intern()) {
    return sbSECPOINT.toString();
  }
  if (sbSECMREAL.toString().intern() == _strType.intern()) {
    return sbSECREAL.toString();
  }
  if (sbSECMREGION.toString().intern() == _strType.intern()) {
    return sbSECREGION.toString();
  }
  if (sbSECMSTRING.toString().intern() == _strType.intern()) {
    return sbSECSTRING.toString();
  }
  return sbSECTEXT.toString();
}

public String Normal2MOType(String _strType)
{
  if (sbSECBOOL.toString().intern() == _strType.intern()) {
    return sbSECMBOOL.toString();
  }
  if (sbSECINT.toString().intern() == _strType.intern()) {
    return sbSECMINT.toString();
  }
  if (sbSECLABEL.toString().intern() == _strType.intern()) {
    return sbSECMLABEL.toString();
  }
  if (sbSECLINE.toString().intern() == _strType.intern()) {
    return sbSECMLINE.toString();
  }
  if (sbSECPOINT.toString().intern() == _strType.intern()) {
    return sbSECMPOINT.toString();
  }
  if (sbSECREAL.toString().intern() == _strType.intern()) {
    return sbSECMREAL.toString();
  }
  if (sbSECREGION.toString().intern() == _strType.intern()) {
    return sbSECMREGION.toString();
  }
  if (sbSECSTRING.toString().intern() == _strType.intern()) {
    return sbSECMSTRING.toString();
  }
  return sbSECTEXT.toString();
}
}
