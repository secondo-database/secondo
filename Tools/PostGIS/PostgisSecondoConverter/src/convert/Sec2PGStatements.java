package convert;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.logging.Logger;
import secondoPostgisUtil.LogFileHandler;
import secondoPostgisUtil.UtilFunctions;public class Sec2PGStatements
{
	  UtilFunctions myutil = new UtilFunctions();
	  File mFile;
	  StringBuffer sbCRLF = new StringBuffer("\r\n");
	  Writer out;
	  PostgresTypes mpgTypes;
	  private long lpreID = 0L;
	  
	  public Sec2PGStatements()
	  {
	    try
	    {
	      this.mpgTypes = new PostgresTypes();
	      this.mFile = File.createTempFile("pgcommands", ".psql");
	      
	      this.out = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(this.mFile), "UTF8"));
	      
	      this.lpreID = System.currentTimeMillis();
	    }
	    catch (IOException e)
	    {
	      LogFileHandler.mlogger.severe(e.getMessage());
	    }
	  }
	  
	  public void writeCreateDB(String strDBName)
	  {
	    try
	    {
	      this.out.append("create database ");
	      this.out.append(strDBName);
	      this.out.append(";");
	      this.out.append(this.sbCRLF.toString());
	      this.out.flush();
	    }
	    catch (IOException e)
	    {
	      LogFileHandler.mlogger.severe(e.getMessage());
	    }
	  }
	  /*this function is used to delete the database given as parameter
	   * @in Database name
	   */
	  public void writeDropDatabase(String strDBName)
	  {
	    try
	    {
	      this.out.append("drop database ");
	      this.out.append(strDBName);
	      this.out.append(";");
	      this.out.append(this.sbCRLF.toString());
	      this.out.flush();
	    }
	    catch (IOException e)
	    {
	      LogFileHandler.mlogger.severe(e.getMessage());
	    }
	  }
	  /*this function is used to delete the given table of the database given as parameter
	   * @in table name
	   */
	  
	  public void writeDropTable(String strTblName)
	  {
	    try
	    {
	      this.out.append("drop table ");
	      this.out.append(strTblName);
	      this.out.append(";");
	      this.out.append(this.sbCRLF.toString());
	      this.out.flush();
	    }
	    catch (IOException e)
	    {
	      LogFileHandler.mlogger.severe(e.getMessage());
	    }
	  }
	  /*
	   * This function is used for creating a new table  
	   * @parameter in: strTblName, strColName, strTypes
	   * 
	   * */
	  
	  public void writeCreateTbl(String strTblName, String[] strColNames, String[] strTypes)
	  {
	    try
	    {
	      StringBuffer sbPGISExtension = new StringBuffer();
	      
	      this.out.append("create table ");
	      this.out.append(strTblName);
	      this.out.append(" ();");
	      
	      this.out.append(this.sbCRLF.toString());
	      this.out.flush();
	      for (int i = 0; i < strTypes.length; i++) {
	        if (this.mpgTypes.isPostGisType(strTypes[i]))
	        {
	          sbPGISExtension.delete(0, sbPGISExtension.length());
	          
	          sbPGISExtension.append("SELECT AddGeometryColumn('', '");
	          sbPGISExtension.append(strTblName.toLowerCase());
	          sbPGISExtension.append("','");
	          sbPGISExtension.append(strColNames[i]);
	          sbPGISExtension.append("',-1,'");
	          sbPGISExtension.append(strTypes[i]);
	          sbPGISExtension.append("',2);");
	          sbPGISExtension.append(this.sbCRLF.toString());
	          
	          this.out.append(sbPGISExtension.toString());
	          this.out.flush();
	          
	          
	        }
	        else
	        {
	          this.out.append("ALTER TABLE ");
	          this.out.append(strTblName);
	          this.out.append(" ADD COLUMN ");
	          this.out.append(strColNames[i]);
	          this.out.append(" ");
	          this.out.append(strTypes[i]);
	          this.out.append(";");
	          this.out.append(this.sbCRLF.toString());
	          this.out.flush();
	        }
	      }
	    }
	    catch (IOException e)
	    {
	      LogFileHandler.mlogger.severe(e.getMessage());
	    }
	  }
	  
	  public void writePreparedStatement(String _strTbl, int _iAnzCol)
	  {
	    try
	    {
	      this.out.append("prepare pre");
	      this.out.append(_strTbl);
	      this.out.append(String.valueOf(this.lpreID));
	      this.out.append(" as insert into ");
	      this.out.append(_strTbl);
	      this.out.append(" values(");
	      for (int i = 1; i <= _iAnzCol; i++)
	      {
	        this.out.append("$" + Integer.valueOf(i));
	        if (i < _iAnzCol) {
	          this.out.append(",");
	        }
	      }
	      this.out.append(");");
	      
	      this.out.append(this.sbCRLF.toString());
	      this.out.flush();
	    }
	    catch (IOException e)
	    {
	      LogFileHandler.mlogger.severe(e.getMessage());
	    }
	  }
	  
	  public void writeInsertInto(String strTblName, String[] strAColNames, String[] strAColValues)
	  {
	    try
	    {
	      this.out.append("execute pre");
	      this.out.append(strTblName);
	      this.out.append(String.valueOf(this.lpreID));
	      this.out.append(" (");
	      for (int i = 0; i < strAColNames.length; i++)
	      {
	        this.out.append(strAColNames[i]);
	        if (i < strAColNames.length - 1) {
	          this.out.append(",");
	        }
	      }
	      this.out.append(") values(");
	      for (int i = 0; i < strAColValues.length; i++)
	      {
	        this.out.append(strAColValues[i]);
	        if (i < strAColValues.length - 1) {
	          this.out.append(",");
	        }
	      }
	      this.out.append(");");
	      
	      this.out.append(this.sbCRLF.toString());
	      this.out.flush();
	    }
	    catch (IOException e)
	    {
	      LogFileHandler.mlogger.severe(e.getMessage());
	    }
	  }
	  
	  public void writeInsertInto(String strTblName, String[] strAColValues)
	  {
	    try
	    {
	      this.out.append("execute pre");
	      this.out.append(strTblName);
	      this.out.append(String.valueOf(this.lpreID));
	      this.out.append("(");
	      for (int i = 0; i < strAColValues.length; i++)
	      {
	        this.out.append(strAColValues[i]);
	        if (i < strAColValues.length - 1) {
	          this.out.append(",");
	        }
	      }
	      this.out.append(");");
	      
	      this.out.append(this.sbCRLF.toString());
	      this.out.flush();
	    }
	    catch (IOException e)
	    {
	      LogFileHandler.mlogger.severe(e.getMessage());
	    }
	  }
	}




 