package convert;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.logging.Logger;
import secondoPostgisUtil.IDelimiter;
import secondoPostgisUtil.LogFileHandler;

public class ConvertingSECFile
  implements IDelimiter
{
  final int iTBLLINE = 0;
  final int iHEADERCOL = 1;
  SecondoTypes msecTypes;
  PostgresTypes mpgTypes;
  private boolean mbUsePostGIS;
  
  public ConvertingSECFile()
  {
    this.mbUsePostGIS = false;
    
    this.msecTypes = new SecondoTypes();
    this.mpgTypes = new PostgresTypes();
  }
  
  public void setUsePostGis(boolean _busePG)
  {
    this.mbUsePostGIS = _busePG;
  }
  
  public File convertFile2Restore(ArrayList<File> _alfiles)
  {
    int iLineIndex = 0;
    
    Sec2PGStatements sec2PGStatements = new Sec2PGStatements();
    String[] strArrayColNames = null;
    
    String[] strArrayTypes = null;
    for (int iFile = 0; iFile < _alfiles.size(); iFile++) {
      try
      {
        InputStreamReader reader = new InputStreamReader(new FileInputStream((File)_alfiles.get(iFile)), "UTF-8");
        BufferedReader br = new BufferedReader(reader);
        
        String strLine = null;
        int iValueInedex = 0;
        iLineIndex = 0;
        String strTblName = "";
       
        while ((strLine = br.readLine()) != null) {
          if ((iLineIndex != 0) && (iLineIndex != 1))
          {
        	 
            String[] strArrayColValues = strLine.split(";-;");
            for (iValueInedex = 0; iValueInedex < strArrayColValues.length; iValueInedex++) {
              strArrayColValues[iValueInedex] = this.mpgTypes.getPostgresValue(strArrayColValues[iValueInedex], new StringBuffer(strArrayTypes[iValueInedex]));
             // System.out.println(" Gelesen "+strArrayColValues[iValueInedex]);
            }
            sec2PGStatements.writeInsertInto(strTblName, strArrayColValues);
            
          //  System.out.println(" strblName " + strTblName +" strArrayColValues " +strArrayColValues);
          }
          else if (iLineIndex == 0)
          {
            strTblName = strLine;
            
            iLineIndex++;
          }
          else if (iLineIndex == 1)
          {
            String[] strSplitFields = strLine.split(";");
            
            strArrayColNames = new String[strSplitFields.length];
            strArrayTypes = new String[strSplitFields.length];
            for (int i = 0; i < strSplitFields.length; i++)
            {
              String[] tmp = strSplitFields[i].split(":");
              strArrayColNames[i] = tmp[0];
              strArrayTypes[i] = this.msecTypes.convertSecType2PGType(tmp[1], this.mbUsePostGIS);
            }
            sec2PGStatements.writeCreateTbl(strTblName, strArrayColNames, strArrayTypes);
            
            sec2PGStatements.writePreparedStatement(strTblName, strArrayColNames.length);
            
            iLineIndex++;
          }
        }
        br.close();
        reader.close();
      }
      catch (UnsupportedEncodingException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      catch (FileNotFoundException e1)
      {
        LogFileHandler.mlogger.severe(e1.getMessage());
      }
      catch (IOException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
    }
    return sec2PGStatements.mFile;
  }
}
