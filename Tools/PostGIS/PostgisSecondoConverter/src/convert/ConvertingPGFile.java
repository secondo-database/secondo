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

public class ConvertingPGFile
  implements IDelimiter
{
  final int iTBLLINE = 0;
  final int iHEADERCOL = 1;
  SecondoTypes msecTypes;
  PostgresTypes mpgTypes;
  private StringBuffer msbTBLName;
  private ArrayList<String> malColumnsNames;
  private ArrayList<String> malColumnsTypes;
  
  public ConvertingPGFile()
  {
    this.msbTBLName = new StringBuffer();
    this.malColumnsNames = new ArrayList();
    this.malColumnsTypes = new ArrayList();
    
    this.msecTypes = new SecondoTypes();
    this.mpgTypes = new PostgresTypes();
  }
  
  public File convertFile2Restore(ArrayList<File> _alfiles)
  {
    PG2SecRestore pg2secres = new PG2SecRestore();
    
    pg2secres.writeDatabaseOpen(new StringBuffer("TEST"));
    pg2secres.writeTYPESOpen();
    pg2secres.writeTYPESClose();
    pg2secres.writeOBJECTSOpen();
    for (int iFile = 0; iFile < _alfiles.size(); iFile++) {
      try
      {
        InputStreamReader reader = new InputStreamReader(new FileInputStream(
          (File)_alfiles.get(iFile)), "UTF-8");
        BufferedReader br = new BufferedReader(reader);
        
        String strLine = null;
        


        int iLineIndex = 0;
        int i = 0;
        
        StringBuffer sbType = new StringBuffer();
        while ((strLine = br.readLine()) != null) {
          if ((iLineIndex != 0) && (iLineIndex != 1))
          {
            String[] strSplitLine = strLine.split(";-;");
            if (strSplitLine.length > 0)
            {
              pg2secres.writeValueBegin();
              for (i = 0; i < strSplitLine.length; i++)
              {
                sbType.delete(0, sbType.length());
                sbType.append((String)this.malColumnsTypes.get(i));
                
                pg2secres
                  .write(this.msecTypes.getSecondoValue(
                  strSplitLine[i], 
                  this.mpgTypes.convertPGType2SecType(sbType)));
                
                pg2secres.write(" ");
              }
              pg2secres.writeValueClose();
            }
          }
          else if (iLineIndex == 0)
          {
            this.msbTBLName.append(strLine);
            
            pg2secres.writeOBJECTOpen(strLine);
            
            iLineIndex++;
          }
          else if (iLineIndex == 1)
          {
            String[] strSplitLine = strLine.split(";");
            if (strSplitLine.length > 0)
            {
              this.malColumnsNames = new ArrayList(
                strSplitLine.length);
              this.malColumnsTypes = new ArrayList(
                strSplitLine.length);
              String strSplitColName = "";
              for (i = 0; i < strSplitLine.length; i++)
              {
                strSplitColName = strSplitLine[i].substring(0, 
                  strSplitLine[i].lastIndexOf(":"));
                strSplitColName = strSplitColName.replaceAll(
                  ":", "");
                this.malColumnsNames.add(strSplitColName);
                this.malColumnsTypes.add(strSplitLine[i]
                  .substring(strSplitLine[i]
                  .lastIndexOf(":") + 1));
              }
              pg2secres.writeHeader(this.malColumnsNames, 
                this.malColumnsTypes);
            }
            iLineIndex++;
          }
        }
        br.close();
        reader.close();
        
        pg2secres.writeOBJECTClose();
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
    pg2secres.writeOBJECTSClose();
    pg2secres.writeDatabaseClose();
    
    return pg2secres.mFile.getAbsoluteFile();
  }
  /* convertFile2RestoreObject
   * in file
   * @return a converted file
   */
  public File convertFile2RestoreObject(File _file)
  {
    PG2SecRestore pg2secres = new PG2SecRestore();
    try
    {
      InputStreamReader reader = new InputStreamReader(new FileInputStream(_file), "UTF-8");
      BufferedReader br = new BufferedReader(reader);
      
      String strLine = null;
      


      int iLineIndex = 0;
      int i = 0;
      StringBuffer sbType = new StringBuffer();
      while ((strLine = br.readLine()) != null) {
        if ((iLineIndex != 0) && (iLineIndex != 1))
        {
        	//System.out.println(" ConvertingPGFile: " +strLine);
        	
          String[] strSplitLine = strLine.split(";-;");
          if (strSplitLine.length > 0)
          {
            pg2secres.writeValueBegin();
            for (i = 0; i < strSplitLine.length; i++)
            {
              sbType.delete(0, sbType.length());
              sbType.append((String)this.malColumnsTypes.get(i));
              
              System.out.println(" ConvertingPGFile: "+sbType.toString());
              
              pg2secres.write(this.msecTypes.getSecondoValue(
                strSplitLine[i], 
              this.mpgTypes.convertPGType2SecType(sbType)));
              
              pg2secres.write(" ");
            }
            pg2secres.writeValueClose();
          }
        }
        else if (iLineIndex == 0)
        {
          this.msbTBLName.append(strLine);
          
          pg2secres.writeOBJECTOpen(strLine);
          
          iLineIndex++;
        }
        else if (iLineIndex == 1)
        {
          String[] strSplitLine = strLine.split(";");
          if (strSplitLine.length > 0)
          {
            this.malColumnsNames = new ArrayList<String>(strSplitLine.length);
            this.malColumnsTypes = new ArrayList<String>(strSplitLine.length);
            String strSplitColName = "";
            for (i = 0; i < strSplitLine.length; i++)
            {
              strSplitColName = strSplitLine[i].substring(0, 
                strSplitLine[i].lastIndexOf(":"));
              strSplitColName = strSplitColName.replaceAll(":", "");
              this.malColumnsNames.add(strSplitColName);
              this.malColumnsTypes.add(strSplitLine[i].substring(strSplitLine[i].lastIndexOf(":") + 1));
              
            }
            pg2secres.writeHeader(this.malColumnsNames, this.malColumnsTypes);
          }
          iLineIndex++;
        }
      }
      br.close();
      reader.close();
      
      pg2secres.writeOBJECTClose();
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
    return pg2secres.mFile.getAbsoluteFile();
  }
}
