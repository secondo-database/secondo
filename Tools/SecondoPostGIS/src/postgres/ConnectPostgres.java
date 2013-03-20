/**
 * 
 */
package postgres;

import gui.ERMGUI;
import gui.TableGUI;

import java.awt.Color;
import java.awt.Component;
import java.awt.MenuItem;
import java.awt.PopupMenu;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.IOException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Properties;
import java.util.Set;
import java.util.Vector;
import java.util.logging.ConsoleHandler;
import java.util.logging.FileHandler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.sql.Connection;

import javax.swing.JFrame;
import javax.swing.JTree;
import javax.swing.text.html.HTML;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.MutableTreeNode;
import javax.swing.tree.TreeNode;
import javax.swing.tree.TreePath;

import util.HTMLStrings;
import util.IMyLogger;
import utilgui.Meldung;

/**
 * @author Bill
 *
 */
public class ConnectPostgres implements IMyLogger{

	private StringBuffer msbhostName;//="192.168.142.128";
	private int miport;
	private StringBuffer msbUser;
	private StringBuffer msbPwd;
	
	public Connection conn;
	
	public Logger mlogger;
	
	
	/**
	 * @param msbhostName
	 * @param miport
	 * @param msbUser
	 * @param msbPwd
	 */
	public ConnectPostgres(StringBuffer _msbhostName, int _miport,
			StringBuffer _msbUser, StringBuffer _msbPwd) {
		super();
		
		this.msbhostName = new StringBuffer();
		this.miport = 0;
		this.msbUser = new StringBuffer();
		this.msbPwd = new StringBuffer();

		
		this.msbhostName = _msbhostName;
		this.miport = _miport;
		this.msbUser = _msbUser;
		this.msbPwd = _msbPwd;
	
		initLogger(new StringBuffer(ConnectPostgres.class.getName()), true);
		
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
		
		
		this.mlogger.info("Try to connect to: " + sbURL.toString());
		
		try
		{
			
			Class.forName("org.postgresql.Driver");
			
			//System.out.println("1");
			conn = DriverManager.getConnection(sbURL.toString(), props);
			//System.out.println("2");
			 
			
			
		}
		catch (ClassNotFoundException e) {
			// TODO Auto-generated catch block
			
			this.mlogger.warning("Connection Error: " + e.getMessage());
		}
		catch (SQLException e) {
			// TODO Auto-generated catch block
			this.mlogger.warning("Connection Error: " + e.getMessage());
			
		}
		catch(Exception exp)
		{
			this.mlogger.warning("Exception: " + exp.getMessage());
			
		}
		finally
		{
			try {
				
				if(conn != null && conn.isClosed()==false)
				{
					this.mlogger.info("Connect to Postgres");
					return true;
				}
				else
				{
					this.mlogger.info("No connect to Postgres");
					return false;
				}
					
			} catch (SQLException e) {
				// TODO Auto-generated catch block
				
				e.printStackTrace();
			}
			
		}
		this.mlogger.info("No connect to Postgres");
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
		
		
this.mlogger.info("Try to connect to: " + sbURL.toString());
		
		try
		{
			
			Class.forName("org.postgresql.Driver");
			
			//System.out.println("1");
			conn = DriverManager.getConnection(sbURL.toString(), props);
			//System.out.println("2");
			 
			
			
		}
		catch (ClassNotFoundException e) {
			// TODO Auto-generated catch block
			
			this.mlogger.warning("Connection Error: " + e.getMessage());
		}
		catch (SQLException e) {
			// TODO Auto-generated catch block
			this.mlogger.warning("Connection Error: " + e.getMessage());
			
		}
		catch(Exception exp)
		{
			this.mlogger.warning("Exception: " + exp.getMessage());
			
		}
		finally
		{
			try {
				
				if(conn != null && conn.isClosed()==false)
				{
					this.mlogger.info("Connect to Postgres");
					return true;
				}
				else
				{
					this.mlogger.info("No connect to Postgres");
					return false;
				}
					
			} catch (SQLException e) {
				// TODO Auto-generated catch block
				
				e.printStackTrace();
			}
			
		}
		this.mlogger.info("No connect to Postgres");
		return false;
	}
	
	
	public LinkedList<Datenbank> getDatabaseNames()
	{
		LinkedList<Datenbank> llDatabasenames = new LinkedList<Datenbank>();
		ResultSet rs = null;
		Datenbank datenbank = new Datenbank();
		
		try
		{
			if(conn != null && conn.isClosed()==true)
				return llDatabasenames;
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		try {
			Statement statement = conn.createStatement();
			
			rs = statement.executeQuery("SELECT * FROM pg_database where datistemplate = false;");
			
			while (rs.next()) {
				
				datenbank = new Datenbank();
				datenbank.setSbName(new StringBuffer(rs.getString(1)));
				
			   // System.out.println(rs.getString(1));
			 //   System.out.println(rs.getString(13));
				//if(rs.getString(13) == null)
				llDatabasenames.add(datenbank);
			}
			rs.close();
			statement.close();
			
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
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
			if(conn != null && conn.isClosed()==true)
				return llTableNames;
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		try {
			Statement statement = conn.createStatement();
			
			
			rs = statement.executeQuery("SELECT table_name FROM information_schema.tables WHERE table_schema = 'public';");
			
			while (rs.next()) 
			{
			    tabelle = new Tabelle();
			    tabelle.setSbName(new StringBuffer(rs.getString(1)));
			    llTableNames.add(tabelle);
			}
			rs.close();
			statement.close();
			
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		return llTableNames;

	}

	public LinkedList<Spalte> getColumnsNamesFromTable(String strTablename)
	{
		ResultSet rs = null;
		LinkedList<Spalte> llTableNames = new LinkedList<Spalte>();
		Spalte spalte = new Spalte();
		StringBuffer sbColumnName = new StringBuffer();
		StringBuffer sbColumnTyp = new StringBuffer();
		
		try
		{
			if(conn != null && conn.isClosed()==true)
				return llTableNames;
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		try {
			Statement statement = conn.createStatement();
			
			
			rs = statement.executeQuery("SELECT * FROM information_schema.columns WHERE table_name ='" + 
										strTablename + "';");
			
			while (rs.next()) {
			    
				spalte = new Spalte();
				sbColumnName.delete(0, sbColumnName.length());
				sbColumnTyp.delete(0, sbColumnTyp.length());
				spalte.setSbName(sbColumnName);
				spalte.setSbTyp(sbColumnTyp);
				
				sbColumnName.append(rs.getString(4));
				sbColumnTyp.append(rs.getString(8));
				
				spalte.setSbName(sbColumnName);
				spalte.setSbTyp(sbColumnTyp);
				
				//System.out.println(rs.getString(4));
				
				llTableNames.add(spalte);
				
			}
			rs.close();
			statement.close();
			
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		return llTableNames;

	}
	

	public HashSet<String> getPrimaryKeyFromTable(String strTablename)
	{
		ResultSet rs = null;
		//LinkedList<Spalte> llTableNames = new LinkedList<Spalte>();
		HashSet<String> hsPrimaryKeys = new HashSet<String>();
		
		Spalte spalte = new Spalte();
		StringBuffer sbColumnName = new StringBuffer();
		StringBuffer sbColumnTyp = new StringBuffer();
		
		try
		{
			if(conn != null && conn.isClosed()==true)
				return hsPrimaryKeys;
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		try {
			Statement statement = conn.createStatement();
			
			
			rs = statement.executeQuery("SELECT pg_attribute.attname, format_type(pg_attribute.atttypid, " +
					"pg_attribute.atttypmod) FROM pg_index, pg_class, pg_attribute WHERE pg_class.oid = '" +
					strTablename + "'::regclass AND indrelid = pg_class.oid AND pg_attribute.attrelid = " +
					"pg_class.oid AND pg_attribute.attnum = any(pg_index.indkey) AND indisprimary;");
			
			while (rs.next()) {
			    
				//System.out.println(rs.getString(1));
				
				hsPrimaryKeys.add(rs.getString(1));
				
			}
			rs.close();
			statement.close();
			
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		return hsPrimaryKeys;

	}

	public HashSet<String> getForeignKeyFromTable(String strTablename)
	{
		ResultSet rs = null;
		//LinkedList<Spalte> llTableNames = new LinkedList<Spalte>();
		HashSet<String> hsForeignKeys = new HashSet<String>();
		
		
		try
		{
			if(conn != null && conn.isClosed()==true)
				return hsForeignKeys;
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		try {
			Statement statement = conn.createStatement();
			

			
			StringBuffer sbSQL = new StringBuffer();
			sbSQL.append("SELECT c.oid, n.nspname, c.relname, n2.nspname, c2.relname, cons.conname ");
			sbSQL.append("FROM pg_class c JOIN pg_namespace n ON n.oid = c.relnamespace ");
			sbSQL.append("LEFT OUTER JOIN pg_constraint cons ON cons.conrelid = c.oid ");
			sbSQL.append("LEFT OUTER JOIN pg_class c2 ON cons.confrelid = c2.oid ");
			sbSQL.append("LEFT OUTER JOIN pg_namespace n2 ON n2.oid = c2.relnamespace ");
			sbSQL.append("WHERE c.relkind = 'r' AND n.nspname IN ('public') AND ");
			sbSQL.append("(cons.contype = 'f' OR cons.contype IS NULL) AND ");
			sbSQL.append("c.relname='" + strTablename +"';");
			
				
			rs = statement.executeQuery( sbSQL.toString());
			StringBuffer sbResult = new StringBuffer();
			int iFirst_ = 0;
			int iLast_ = 0;
			
			//System.out.println("SQL " + sbSQL.toString());
			//System.out.println("Tabelle " + strTablename);
			while (rs.next()) {

				
				if(rs.getString(6) != null)
			    {
					sbResult.delete(0,sbResult.length());
			    			
				    //System.out.println("Ausgabe: " + rs.getString(6));
				    sbResult.append(rs.getString(6));
				    iFirst_ = sbResult.indexOf("_")+1;
				    iLast_ = sbResult.lastIndexOf("_");
					
				    //System.out.println(sbResult.substring(iFirst_, iLast_));
					
					
					hsForeignKeys.add(sbResult.substring(iFirst_, iLast_));
			    }
			}
			rs.close();
			statement.close();
			
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		return hsForeignKeys;

	}
	

	public ArrayList<String> getForeignRelations()
	{
		ResultSet rs = null;
		//LinkedList<Spalte> llTableNames = new LinkedList<Spalte>();
		ArrayList<String> alForeignRelations = new ArrayList<String>();
		
		
		try
		{
			if(conn != null && conn.isClosed()==true)
				return alForeignRelations;
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		try {
			Statement statement = conn.createStatement();
			
			
			
			StringBuffer sbSQL = new StringBuffer();
			sbSQL.append("SELECT c.relname, c2.relname ");
			sbSQL.append("FROM pg_class c JOIN pg_namespace n ON n.oid = c.relnamespace ");
			sbSQL.append("LEFT OUTER JOIN pg_constraint cons ON cons.conrelid = c.oid ");
			sbSQL.append("LEFT OUTER JOIN pg_class c2 ON cons.confrelid = c2.oid ");
			sbSQL.append("LEFT OUTER JOIN pg_namespace n2 ON n2.oid = c2.relnamespace ");
			sbSQL.append("WHERE c.relkind = 'r' AND n.nspname IN ('public') AND ");
			sbSQL.append("(cons.contype = 'f' OR cons.contype IS NULL);");
			
			//System.out.println(sbSQL);
			rs = statement.executeQuery( sbSQL.toString());
			StringBuffer sbResult = new StringBuffer();
			while (rs.next()) {
				
			    sbResult.delete(0,sbResult.length());
				
			    sbResult.append(rs.getString(1));
			    sbResult.append(":");
			    sbResult.append(rs.getString(2));
			    
			    System.out.println(sbResult);
			    alForeignRelations.add(sbResult.toString());
			    
			}
			rs.close();
			statement.close();
			
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		return alForeignRelations;

		
	}


	public int generateRowsFromTable(String strTablename)
	{
		ResultSet rs = null;
		int iRows=0;
		
		
		try
		{
			if(conn != null && conn.isClosed()==true)
				return iRows;
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		try {
			Statement statement = conn.createStatement();
			

			
			StringBuffer sbSQL = new StringBuffer();
			sbSQL.append("select count(*) from ");
			sbSQL.append(strTablename);
			sbSQL.append(";");
			
				
			rs = statement.executeQuery( sbSQL.toString());
			StringBuffer sbResult = new StringBuffer();
			int iFirst_ = 0;
			int iLast_ = 0;
			
			//.println("SQL " + sbSQL.toString());
			//.println("Tabelle " + strTablename);
			while (rs.next()) {

				
				if(rs.getString(1) != null)
			    {
					iRows = rs.getInt(1);
			    }
			}
			rs.close();
			statement.close();
			
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		return iRows;
		
	}
	
	

	
	

	
	
	public ResultSet sendQuery(String strQuery)
	{
		ResultSet rs = null;
		
		try
		{
			if(conn != null && conn.isClosed()==true)
				return rs;
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		//System.out.println("Hallo1");
		
		try {
			Statement statement = conn.createStatement();
			
			
			
			rs = statement.executeQuery(strQuery);
			/*while (rs.next()) {
			    System.out.print("Column 1 returned ");
			    System.out.println(rs.getString(1));
			}*/
			//rs.close();
			statement.close();
			
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		return rs;
	}
	
	
	public boolean makeExecuteQuerysTo2TableView(String [] sbStatements,Component c)
	{
	
		ResultSet rs = null;
		
		
		try
		{
			if(conn != null && conn.isClosed()==true)
				return false;
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		try {
			
			Vector <String> vColumns = new Vector<String>();
			Vector <Vector> vRows = new Vector<Vector>();
			
			for(int i= 0; i < sbStatements.length; i++)
			{
				vColumns = new Vector<String>();
				vRows = new Vector<Vector>();
				
				Statement statement = conn.createStatement();
				
				StringBuffer sbSQL = new StringBuffer();
				sbSQL.append(sbStatements[i]);
				
				rs = statement.executeQuery( sbSQL.toString());
				
				ResultSetMetaData rsmd = rs.getMetaData();
				
				//Spalten ermitteln und wegschreiben:
				for(int y = 1 ; y <= rsmd.getColumnCount(); y++)
				{
					vColumns.addElement(rsmd.getColumnLabel(y));
					//System.out.println(rsmd.getColumnLabel(y));
				}
				
				
						
				Vector<String> vEinzelRow = new Vector<String>();
				while (rs.next()) {
					
				    
					for(int k=1; k <= rsmd.getColumnCount(); k++)
					{
						vEinzelRow.addElement(rs.getString(k));
					}
				    
					vRows.addElement(vEinzelRow);
					vEinzelRow = new Vector<String>();
				    
				}
				rs.close();
				statement.close();
			
				TableGUI tablegui = new TableGUI();
				tablegui.init(vRows, vColumns, "Select View - " + String.valueOf(vRows.size()) + " rows");
				
				
			}
			
				
		} 
		catch (SQLException e) 
		{
			// TODO Auto-generated catch block
			this.mlogger.warning("SQL Exception: " + e.getMessage());
			new Meldung("Hint: \n" + e.getMessage());
			//e.printStackTrace();
			
		}
		
		
		return true;
		
		
		
		
	}
	
	
	

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub

		ConnectPostgres cPostgres = new ConnectPostgres(new StringBuffer("192.168.178.35"), 5432, new StringBuffer("postgres"), new StringBuffer("123"));
		
		//System.out.println("Hallo");
		if(cPostgres.connect(new StringBuffer("mygis"))==false)
			System.exit(1);
			
	//	System.out.println("Connect");
		
		
		String [] sbTestArr = new String[1];
		sbTestArr[0] = "select * from films1;";
		//cPostgres.makeExecuteQuerysTo2TableView(sbTestArr);
		
		//System.exit(1);
		
		HTMLStrings htmlstrings = new HTMLStrings();
		
		DefaultMutableTreeNode mdmtreenodeRechtsServer = new DefaultMutableTreeNode(htmlstrings.generateJTreeServer(new StringBuffer("Postgres")));
		DefaultMutableTreeNode mdmtreenodeRechtsDatenbank = new DefaultMutableTreeNode();
		DefaultMutableTreeNode mdmtreenodeRechtsTabellen = new DefaultMutableTreeNode();
		DefaultMutableTreeNode mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode();
		
		DefaultMutableTreeNode mdmDatenRechtsServer = new DefaultMutableTreeNode("Postgres");
		DefaultMutableTreeNode mdmDatenRechtsDatenbank = new DefaultMutableTreeNode();
		DefaultMutableTreeNode mdmDatenRechtsTabellen = new DefaultMutableTreeNode();
		DefaultMutableTreeNode mdmDatenRechtsSpalten = new DefaultMutableTreeNode();
		
		
		
		
		
		Tabelle tabelle = new Tabelle();
		Datenbank db = new Datenbank();
		
		
		
		LinkedList<Datenbank> llDatenbanknamen = new LinkedList<Datenbank>();
		LinkedList<Tabelle> llTabellennamen = new LinkedList<Tabelle>();
		LinkedList<Spalte> llSpaltennamen = new LinkedList<Spalte>();
		
		//Datenbank-namen holen
		llDatenbanknamen = cPostgres.getDatabaseNames();	
		
		Iterator<Datenbank> itDatenbanknamen = llDatenbanknamen.iterator();
		
		
		while(itDatenbanknamen.hasNext())
		{
			
			db = itDatenbanknamen.next();
			
			mdmtreenodeRechtsDatenbank = new DefaultMutableTreeNode(htmlstrings.generateJTreeDatenbank(db.getSbName()));
			mdmDatenRechtsDatenbank = new DefaultMutableTreeNode(db.getSbName().toString());
			
			try {
				cPostgres.conn.close();
			} catch (SQLException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			if(cPostgres.connect(db.getSbName())==false)
				System.exit(1);
		
			llTabellennamen.clear();
			llTabellennamen = cPostgres.getTableNamesFromDB();
			Iterator<Tabelle> itTabellennamen = llTabellennamen.iterator();
			
			
				while(itTabellennamen.hasNext())
				{
					tabelle = new Tabelle();
					
					
					
					tabelle = itTabellennamen.next();
					//tabelle.setSbName(new StringBuffer(strTabellenname));
					
					mdmtreenodeRechtsTabellen = new DefaultMutableTreeNode(htmlstrings.generateJTreeTabelle(
												tabelle.getSbName()));
					
					mdmDatenRechtsTabellen = new DefaultMutableTreeNode(tabelle.getSbName().toString());
					
					
					llSpaltennamen.clear();
					llSpaltennamen = cPostgres.getColumnsNamesFromTable(tabelle.getSbName().toString());
					
					//PrimaryKeys bekommen:
					HashSet<String> hsPrimaryKeys = new HashSet<String>();
					hsPrimaryKeys = cPostgres.getPrimaryKeyFromTable(tabelle.getSbName().toString());
					
					
					//Foreign Key bekommen:
					HashSet<String> hsForeignKeys = new HashSet<String>();
					hsForeignKeys = cPostgres.getForeignKeyFromTable(tabelle.getSbName().toString());
					
					
					
					for(int i=0; i < llSpaltennamen.size(); i++)
					{
						//System.out.println("____" + llSpaltennamen.get(i).getSbName());
						
						
						if(hsPrimaryKeys.contains(llSpaltennamen.get(i).getSbName().toString())== true)
						{
							mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode(htmlstrings.generateJTreeSpaltePrimaryKey(
									llSpaltennamen.get(i).getSbName(),llSpaltennamen.get(i).getSbTyp()));
		
						}
						else if(hsForeignKeys.contains(llSpaltennamen.get(i).getSbName().toString())==true)
						{
							mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode(htmlstrings.generateJTreeSpalteForeignKey(
									llSpaltennamen.get(i).getSbName(),llSpaltennamen.get(i).getSbTyp()));
						}
						else
						{
							mdmtreenodeRechtsSpalten = new DefaultMutableTreeNode(htmlstrings.generateJTreeSpalte(
									llSpaltennamen.get(i).getSbName(),llSpaltennamen.get(i).getSbTyp()));
		
						}
							
						mdmDatenRechtsSpalten = new DefaultMutableTreeNode(llSpaltennamen.get(i).getSbName().toString() +":" + llSpaltennamen.get(i).getSbTyp().toString());
						
						mdmtreenodeRechtsTabellen.add(mdmtreenodeRechtsSpalten);
						mdmDatenRechtsTabellen.add(mdmDatenRechtsSpalten);
						
					}
					
					//System.out.println("__"+llSpaltennamen.size());
					
					
					
					mdmtreenodeRechtsDatenbank.add(mdmtreenodeRechtsTabellen);
					mdmDatenRechtsDatenbank.add(mdmDatenRechtsTabellen);
				}
			
			db = new Datenbank();
				
			//jetzt muss das abgespeichert werden
				
			
			
			
			//mdmtreenodeRechtsDatenbank.add(mdmtreenodeRechtsTabellen);
			
			mdmtreenodeRechtsServer.add(mdmtreenodeRechtsDatenbank);
			mdmDatenRechtsServer.add(mdmDatenRechtsDatenbank);
			
			//System.out.println();
		}
		
		ArrayList<String> alRelations = cPostgres.getForeignRelations();
		
		try {
			cPostgres.conn.close();
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		//if != 0 dann kanns weiter gehen
		
		
		JFrame testfrm = new JFrame();
		
		
		
		
		
		
		 DefaultMutableTreeNode dmtnDatenbank = new DefaultMutableTreeNode("Test");
		 dmtnDatenbank.add(mdmtreenodeRechtsServer);
		 /*for (Enumeration e = dmtnDatenbank.children() ; e.hasMoreElements() ;) {
	         
				//System.out.println( mdmtreenodeRechtsServer.getIndex((TreeNode) e.nextElement()));
				 
				// System.out.println(e.nextElement());

	    }*/

		 
		 
		/* for (Enumeration e = dmtnDatenbank.getChildAt(0).getChildAt(1).getChildAt(7).children() ; e.hasMoreElements() ;) {
	         
				//System.out.println( mdmtreenodeRechtsServer.getIndex((TreeNode) e.nextElement()));
				 
				 //System.out.println(e.nextElement());

	    }*/

		 
		 ActionListener l = new ActionListener() {
			
			@Override
			public void actionPerformed(ActionEvent e) {
				// TODO Auto-generated method stub
				
				
			}
		};
		 
		
		 final PopupMenu popup = new PopupMenu();
		 popup.add(new MenuItem("Hallo"));
		 popup.add(new MenuItem("Hallo21"));
		 popup.addActionListener(l);
		 
		 
		
		
		final JTree jtree = new JTree(mdmtreenodeRechtsServer);
		
		jtree.add(popup);
		
		 MouseListener ml = new MouseAdapter() {
			 public void mouseReleased(MouseEvent e)
			 {
				 if(e.getButton() == MouseEvent.BUTTON3)  //rechts klick
				 {
					 //System.out.println("3");
					 popup.show(e.getComponent(), e.getX(), e.getY());
					 
				 }
			 }

			 /*public void mousePressed(MouseEvent e) {
		    	 
		    	 
		         int selRow = jtree.getRowForLocation(e.getX(), e.getY());
		         
		         TreePath selPath = jtree.getPathForLocation(e.getX(), e.getY());
		         System.out.println(selPath.toString());
		         jtree.setSelectionPath(selPath);
		         
		         if(selRow != -1) {
		             if(e.getClickCount() == 1) {
		                 System.out.println("Single");
		             }
		             else if(e.getClickCount() == 2) {
		                 System.out.println("doppel klick");//myDoubleClick(selRow, selPath);
		             }
		         }
		     }*/
			 
			 
			 
		 };
		 
		 
		jtree.addMouseListener(ml);
		
		//jtree.add(new PopupMenu("Hallo"));
		
		
		// JTree jtree = new JTree(dmtnDatenbank);
		
		jtree.setRootVisible(true);
		
		if (jtree.getCellRenderer() instanceof DefaultTreeCellRenderer)
		{
		   /* final DefaultTreeCellRenderer renderer = 
		        (DefaultTreeCellRenderer)(jtree.getCellRenderer());
		    renderer.setBackgroundNonSelectionColor(Color.YELLOW);
		    renderer.setBackgroundSelectionColor(Color.ORANGE);
		    renderer.setTextNonSelectionColor(Color.RED);
		    renderer.setTextSelectionColor(Color.BLUE);
			*/
		}
		else
		{
		    System.err.println("Sorry, no special colors today.");
		}
		
		
		testfrm.add(jtree);
		
		testfrm.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		testfrm.setSize(300, 400);
		testfrm.setVisible(true);
		
		ERMGUI erm = new ERMGUI();
		
		
		
		//erm.init(mdmtreenodeRechtsServer,"mygis",alRelations);
		
		
		
		
		
	}


	@Override
	public void initLogger(StringBuffer _strClassName, boolean _bLogConsole)
	{
		// TODO Auto-generated method stub

		
		this.mlogger = Logger.getLogger(_strClassName.toString());
		
		this.mlogger.setUseParentHandlers(false);
		this.mlogger.setLevel(Level.ALL);
		FileHandler fh = null;
		try 
		{
			//System.out.println("Hallo1");
			fh = new FileHandler(mstrLogDatei, miMaxSize, miCountRotate, mbLogAppend);
			//System.out.println("Hallo2");
		} catch (SecurityException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		this.mlogger.addHandler(fh);
		
		// ConsolenHandler konfigurieren wenn er gewünscht ist
		if(_bLogConsole == true)
		{
			ConsoleHandler ch = new ConsoleHandler();
			ch.setLevel(Level.ALL);
			this.mlogger.addHandler(ch);
		}
		
	}

}
