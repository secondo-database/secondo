package postgis; 


import appGui.MainGui;
import appGui.TablePGConvertGUI;

import java.awt.Component;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Properties;
import java.util.Vector;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import secondoPostgisUtil.IGlobalParameters;
import secondoPostgisUtil.LogFileHandler;
import appGuiUtil.Message;

/*
 *This class will called every time a connection to Postgres is needed in the application  
 * 
 */
public class ConnectPostgres
  implements IPGTextMessages, IGlobalParameters
  
  {
    private StringBuffer msbhostName;
    private int miport;
    private StringBuffer msbUser;
    private StringBuffer msbPwd;
    public Connection conn;

    public ConnectPostgres(StringBuffer _msbhostName, int _miport, StringBuffer _msbUser, StringBuffer _msbPwd)
    {
      this.msbhostName = new StringBuffer();
      this.miport = 0;
      this.msbUser = new StringBuffer();
      this.msbPwd = new StringBuffer();
      

      this.msbhostName = _msbhostName;
      this.miport = _miport;
      this.msbUser = _msbUser;
      this.msbPwd = _msbPwd;
      
      
     // initLogger(new StringBuffer(ConnectPostgres.class.getName()), true);
    }
    
    
    
	public boolean connect()
	{
		
		Properties props = new Properties();
		props.setProperty("user",this.msbUser.toString());
		props.setProperty("password",this.msbPwd.toString());
		
		StringBuffer sbURL = new StringBuffer();
		sbURL.append("jdbc:postgresql://");
		sbURL.append(this.msbhostName);
		sbURL.append(":");
		sbURL.append(this.miport);
		sbURL.append("/");
		
		 LogFileHandler.mlogger.info("Try to connect to: " + sbURL.toString());
		
		
		try
		{
			
			Class.forName("org.postgresql.Driver");
			
			//System.out.println("1");
			conn = DriverManager.getConnection(sbURL.toString(), props);
			//System.out.println("2");
			
		}
		catch (ClassNotFoundException e) {
			// TODO Auto-generated catch block
			
			 LogFileHandler.mlogger.warning("Connection Error: "+e.getMessage());
			//this.mlogger.warning("Connection Error: " + e.getMessage());
		}
		catch (SQLException e) {
			// TODO Auto-generated catch block
			
			 LogFileHandler.mlogger.warning("Connection Error: "+e.getMessage());
			
		}
		catch(Exception exp)
		{
			 LogFileHandler.mlogger.warning("Connection Error: "+exp.getMessage());
			
		}
		finally
		{
			try {
				
				if(conn != null && conn.isClosed()==false)
				{
					LogFileHandler.mlogger.info("Connect to Postgres");
					return true;
				}
				else
				{
					LogFileHandler.mlogger.info("No connect to Postgres");
					return false;
				}
					
			} catch (SQLException e) {
				// TODO Auto-generated catch block
				
				e.printStackTrace();
			}
			
		}
		LogFileHandler.mlogger.info("No connect to Postgres");
		return false;
	}
	
    
    
    
    public boolean connect(StringBuffer sbDatabase)
	{
		
		Properties props = new Properties();
		props.setProperty("user",this.msbUser.toString());
		props.setProperty("password",this.msbPwd.toString());
		
		StringBuffer sbURL = new StringBuffer();
		sbURL.append("jdbc:postgresql://");
		sbURL.append(this.msbhostName);
		sbURL.append(":");
		sbURL.append(this.miport);
		sbURL.append("/");
		sbURL.append(sbDatabase);
		
		
		LogFileHandler.mlogger.info("Try to connect to: " + sbURL.toString());
		
		try
		{
			
			Class.forName("org.postgresql.Driver");
			
			//System.out.println("44");
			conn = DriverManager.getConnection(sbURL.toString(), props);
			//System.out.println("55");
			 
			
			
		}
		catch (ClassNotFoundException e) {
			// TODO Auto-generated catch block
			
			LogFileHandler.mlogger.warning("Connection Error: " + e.getMessage());
		}
		catch (SQLException e) {
			// TODO Auto-generated catch block
			LogFileHandler.mlogger.warning("Connection Error: " + e.getMessage());
			
		}
		catch(Exception exp)
		{
			LogFileHandler.mlogger.warning("Exception: " + exp.getMessage());
			
		}
		finally
		{
			try {
				
				if(conn != null && conn.isClosed()==false)
				{
					LogFileHandler.mlogger.info("Connect to Postgres");
					return true;
				}
				else
				{
					LogFileHandler.mlogger.info("No connect to Postgres");
					return false;
				}
					
			} catch (SQLException e) {
				// TODO Auto-generated catch block
				
				e.printStackTrace();
			}
			
		}
		LogFileHandler.mlogger.info("No connect to Postgres");
		return false;
	}
    
      
    
    public LinkedList<DatabaseName> getDatabaseNames()
    {
      LinkedList<DatabaseName> llDatabasenames = new LinkedList<DatabaseName>();
      ResultSet rs = null;
      DatabaseName datenbank = new DatabaseName();
      try
      {
        if ((this.conn != null) && (this.conn.isClosed())) {
          return llDatabasenames;
        }
      }
      catch (SQLException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
        try
        {
          Statement statement = this.conn.createStatement();
          
          rs = statement.executeQuery("SELECT * FROM pg_database where datistemplate = false;");
          while (rs.next())
          {
            datenbank = new DatabaseName();
            datenbank.setSbName(new StringBuffer(rs.getString(1)));
            llDatabasenames.add(datenbank);
          }
          rs.close();
          statement.close();
        }
        catch (SQLException ex)
        {
          LogFileHandler.mlogger.severe(ex.getMessage());
        }
      
      return llDatabasenames;
    }
    
    public LinkedList<DatabaseName> getTemplateNames()
    {
      LinkedList<DatabaseName> llDatabasenames = new LinkedList<DatabaseName>();
      ResultSet rs = null;
      DatabaseName datenbank = new DatabaseName();
      try
      {
        if ((this.conn != null) && (this.conn.isClosed())) {
          return llDatabasenames;
        }
      }
      catch (SQLException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
        try
        {
          Statement statement = this.conn.createStatement();
          
          rs = statement.executeQuery("SELECT * FROM pg_database where datistemplate = true;");
          while (rs.next())
          {
            datenbank = new DatabaseName();
            datenbank.setSbName(new StringBuffer(rs.getString(1)));
            llDatabasenames.add(datenbank);
          }
          rs.close();
          statement.close();
        }
        catch (SQLException ex)
        {
          LogFileHandler.mlogger.severe(ex.getMessage());
        }
      
      return llDatabasenames;
  }
    
    public LinkedList<Tabelle> getTableNamesFromDB()
    {
      ResultSet rs = null;
      LinkedList<Tabelle> llTableNames = new LinkedList<Tabelle>();
      Tabelle tabelle = new Tabelle();
      try
      {
        if ((this.conn != null) && (this.conn.isClosed())) {
          return llTableNames;
        }
      }
      catch (SQLException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
        try
        {
          Statement statement = this.conn.createStatement();
          

          rs = statement.executeQuery("SELECT table_name FROM information_schema.tables WHERE table_schema = 'public';");
          while (rs.next())
          {
            tabelle = new Tabelle();
            tabelle.setSbName(new StringBuffer(rs.getString(1)));
            llTableNames.add(tabelle);
          }
          rs.close();
          statement.close();
        }
        catch (SQLException ex)
        {
          LogFileHandler.mlogger.severe(ex.getMessage());
        }
   
      return llTableNames;
    }
    
    public LinkedList<Spalte> getColumnsNamesFromTable(String strTablename, StringBuffer sbDBName)
    {
      ResultSet rs = null;
      LinkedList<Spalte> llTableNames = new LinkedList();
      Spalte spalte = new Spalte();
      StringBuffer sbColumnName = new StringBuffer();
      StringBuffer sbColumnTyp = new StringBuffer();
      try
      {
        if ((this.conn != null) && (this.conn.isClosed())) {
          return llTableNames;
        }
      }
      catch (SQLException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
        try
        {
          Statement statement = this.conn.createStatement();
          rs = statement.executeQuery("SELECT column_name,data_type,udt_name FROM information_schema.columns WHERE table_name ='" + 
            strTablename + "';");
          while (rs.next())
          {
            spalte = new Spalte();
            sbColumnName.delete(0, sbColumnName.length());
            sbColumnTyp.delete(0, sbColumnTyp.length());
            spalte.setSbName(sbColumnName);
            spalte.setSbTyp(sbColumnTyp);
            
            sbColumnName.append(rs.getString(1));
            sbColumnTyp.append(rs.getString(2));
            if ((sbColumnTyp.toString().contains("USER-DEFINED")) && (rs.getString(3).intern() == "geometry".intern()))
            {
              Statement statementUserDef = this.conn.createStatement();
              
              ResultSet rsUserDef = statementUserDef.executeQuery("SELECT type from geometry_columns where f_table_name = '" + strTablename + "' AND f_geometry_column ='" + 
                sbColumnName.toString() + "';");
              while (rsUserDef.next())
              {
                sbColumnTyp.delete(0, sbColumnTyp.length());
                sbColumnTyp.append(rsUserDef.getString(1));
              }
              statementUserDef.close();
            }
            spalte.setSbName(sbColumnName);
            spalte.setSbTyp(sbColumnTyp);
            
            llTableNames.add(spalte);
          }
          rs.close();
          statement.close();
        }
        catch (SQLException ex)
        {
          LogFileHandler.mlogger.severe(ex.getMessage());
        }
      
      return llTableNames;
    }
    
    public HashSet<String> getPrimaryKeyFromTable(String strTablename)
    {
      ResultSet rs = null;
      
      HashSet<String> hsPrimaryKeys = new HashSet();
      try
      {
        if ((this.conn != null) && (this.conn.isClosed())) {
          return hsPrimaryKeys;
        }
      }
      catch (SQLException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
        try
        {
          Statement statement = this.conn.createStatement();
          

          rs = statement.executeQuery("SELECT pg_attribute.attname, format_type(pg_attribute.atttypid, pg_attribute.atttypmod) FROM pg_index, pg_class, pg_attribute WHERE pg_class.oid = '" + 
          
            strTablename + "'::regclass AND indrelid = pg_class.oid AND pg_attribute.attrelid = " + 
            "pg_class.oid AND pg_attribute.attnum = any(pg_index.indkey) AND indisprimary;");
          while (rs.next()) {
            hsPrimaryKeys.add(rs.getString(1));
          }
          rs.close();
          statement.close();
        }
        catch (SQLException ex)
        {
          LogFileHandler.mlogger.severe(ex.getMessage());
        }
      
      return hsPrimaryKeys;
    }
    
    public HashSet<String> getForeignKeyFromTable(String strTablename)
    {
      ResultSet rs = null;
      HashSet<String> hsForeignKeys = new HashSet();
      try
      {
        if ((this.conn != null) && (this.conn.isClosed())) {
          return hsForeignKeys;
        }
      }
      catch (SQLException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
        try
        {
          Statement statement = this.conn.createStatement();
          


          StringBuffer sbSQL = new StringBuffer();
          
          sbSQL.append("SELECT c.conname, c.conkey[1] AS MyColumnID, c.confkey[1] AS ");
          sbSQL.append("ForeignColumnID, tab1.relname AS MyTable,");
          sbSQL.append("tab2.relname AS ForeignTable, a1.attname AS MyColumn,");
          sbSQL.append("a2.attname AS ForeignColumn ");
          sbSQL.append("FROM pg_constraint c ");
          sbSQL.append("INNER JOIN pg_class tab1 ON c.conrelid = tab1.oid ");
          sbSQL.append("INNER JOIN pg_class tab2 ON c.confrelid = tab2.oid ");
          sbSQL.append("INNER JOIN pg_attribute a1 ON c.conkey[1] = a1.attnum ");
          sbSQL.append("INNER JOIN pg_attribute a2 ON c.confkey[1] = a2.attnum ");
          sbSQL.append("WHERE c.contype = 'f' AND a1.attrelid = tab1.oid AND a2.attrelid = tab2.oid AND tab1.relname ='" + strTablename + "';");
          

          rs = statement.executeQuery(sbSQL.toString());
          StringBuffer sbResult = new StringBuffer();
          while (rs.next()) {
            if (rs.getString(6) != null)
            {
              sbResult.delete(0, sbResult.length());
              
              sbResult.append(rs.getString(6));
              
              hsForeignKeys.add(sbResult.toString());
            }
          }
          rs.close();
          statement.close();
        }
        catch (SQLException ex)
        {
          LogFileHandler.mlogger.severe(ex.getMessage());
        }
      
      return hsForeignKeys;
    }
    
    public ArrayList<String> getForeignRelations()
    {
      ResultSet rs = null;
      ArrayList<String> alForeignRelations = new ArrayList<String>();
      try
      {
        if ((this.conn != null) && (this.conn.isClosed())) {
          return alForeignRelations;
        }
      }
      catch (SQLException e)
      {
        e.printStackTrace();
      }
        try
        {
          Statement statement = this.conn.createStatement();
          
          StringBuffer sbSQL = new StringBuffer();
          sbSQL.append("SELECT c.relname, c2.relname ");
          sbSQL.append("FROM pg_class c JOIN pg_namespace n ON n.oid = c.relnamespace ");
          sbSQL.append("LEFT OUTER JOIN pg_constraint cons ON cons.conrelid = c.oid ");
          sbSQL.append("LEFT OUTER JOIN pg_class c2 ON cons.confrelid = c2.oid ");
          sbSQL.append("LEFT OUTER JOIN pg_namespace n2 ON n2.oid = c2.relnamespace ");
          sbSQL.append("WHERE c.relkind = 'r' AND n.nspname IN ('public') AND ");
          sbSQL.append("(cons.contype = 'f' OR cons.contype IS NULL);");
          
          rs = statement.executeQuery(sbSQL.toString());
          StringBuffer sbResult = new StringBuffer();
          while (rs.next())
          {
            sbResult.delete(0, sbResult.length());
            
            sbResult.append(rs.getString(1));
            sbResult.append(":");
            sbResult.append(rs.getString(2));
            
            alForeignRelations.add(sbResult.toString());
          }
          rs.close();
          statement.close();
        }
        catch (SQLException ex)
        {
          LogFileHandler.mlogger.severe(ex.getMessage());
        }
     
      return alForeignRelations;
    }
    
    public int generateRowsFromTable(String strTablename)
    {
      ResultSet rs = null;
      int iRows = 0;
      try
      {
        if ((this.conn != null) && (this.conn.isClosed())) {
          return iRows;
        }
      }
      catch (SQLException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
        try
        {
          Statement statement = this.conn.createStatement();
          
          StringBuffer sbSQL = new StringBuffer();
          sbSQL.append("select count(*) from ");
          sbSQL.append(strTablename);
          sbSQL.append(";");
          
          rs = statement.executeQuery(sbSQL.toString());
          while (rs.next()) {
            if (rs.getString(1) != null) {
              iRows = rs.getInt(1);
            }
          }
          rs.close();
          statement.close();
        }
        catch (SQLException ex)
        {
          ex.printStackTrace();
        }
      }
      return iRows;
    }
    
    public long generateTableSize(String strTablename)
    {
      ResultSet rs = null;
      long lTableSize = 0L;
      try
      {
        if ((this.conn != null) && (this.conn.isClosed())) {
          return lTableSize;
        }
      }
      catch (SQLException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
        try
        {
          Statement statement = this.conn.createStatement();
          
          StringBuffer sbSQL = new StringBuffer();
          sbSQL.append("select pg_total_relation_size('");
          sbSQL.append(strTablename);
          sbSQL.append("');");
          

          rs = statement.executeQuery(sbSQL.toString());
          while (rs.next()) {
            if (rs.getString(1) != null) {
              lTableSize = rs.getLong(1);
            }
          }
          rs.close();
          statement.close();
        }
        catch (SQLException ex)
        {
          ex.printStackTrace();
        }
      }
      return lTableSize;
    }
    
    public long getAverageRowSize(String strTable)
    {
      if (generateRowsFromTable(strTable) <= 0) {
        return 0L;
      }
      return generateTableSize(strTable) / generateRowsFromTable(strTable);
    }
    
    public ResultSet sendQuery(String strQuery)
    {
      ResultSet rs = null;
      try
      {
        if ((this.conn != null) && (this.conn.isClosed())) {
          return rs;
        }
      }
      catch (SQLException e)
      {
        e.printStackTrace();
      }
        try
        {
          Statement statement = this.conn.createStatement();
          

          rs = statement.executeQuery(strQuery);
          statement.close();
        }
        catch (SQLException ex)
        {
          LogFileHandler.mlogger.severe(ex.getMessage());
        }
      return rs;
    }
    
    public boolean makeExecuteQuerysTo2TableView(String[] sbStatements, Component c, MainGui _mgui)
    {
      ResultSet rs = null;
      try
      {
        if ((this.conn != null) && (this.conn.isClosed())) {
          return false;
        }
      }
      catch (SQLException e)
      {
        LogFileHandler.mlogger.severe(e.getMessage());
      }
        try
        {
          Vector<String> vColumns = new Vector<String>();
          Vector<Vector> vRows = new Vector<>();
          
          Pattern pLimit = Pattern.compile(".+([LIMITlimit]{5}\\s?[\\d]{1,}).*");
          for (int i = 0; i < sbStatements.length; i++)
          {
            vColumns = new Vector();
            vRows = new Vector();
            
            Statement statement = this.conn.createStatement();
            
            StringBuffer sbSQL = new StringBuffer();
            


            Matcher mMatch = pLimit.matcher(sbStatements[i]);
            if (!mMatch.find()) {
              sbStatements[i] = (sbStatements[i] + " LIMIT " + "50");
            }
            sbSQL.append(sbStatements[i]);
            try
            {
              rs = statement.executeQuery(sbSQL.toString());
            }
            catch (SQLException ex)
            {
              LogFileHandler.mlogger.warning("SQL Exception: " + ex.getMessage());
              new Message("Hint: \n" + ex.getMessage());
              continue;
            }
            ResultSetMetaData rsmd = rs.getMetaData();
            for (int y = 1; y <= rsmd.getColumnCount(); y++) {
              vColumns.addElement(rsmd.getColumnLabel(y));
            }
            Vector<String> vEinzelRow = new Vector<String>();
            while (rs.next())
            {
              for (int k = 1; k <= rsmd.getColumnCount(); k++) {
                vEinzelRow.addElement(rs.getString(k));
              }
              vRows.addElement(vEinzelRow);
              vEinzelRow = new Vector();
            }
            rs.close();
            statement.close();
            
            TablePGConvertGUI tablePGgui = new TablePGConvertGUI(_mgui);
            tablePGgui.init(vRows, vColumns, "Postgres select view - " + String.valueOf(vRows.size()) + " rows");
          }
        }
        catch (SQLException ex)
        {
          LogFileHandler.mlogger.warning("SQL Exception: " + ex.getMessage());
          new Message("Hint: \n" + ex.getMessage());
        }
      
      return true;
    }
  }

  
