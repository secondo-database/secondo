package DriverSet;

import java.sql.SQLException;

import sj.lang.ListExpr;
import tools.Reporter;

/**
 * 
 * <b> Task of this class </b> <br/>
 * The ResultSet for DatabaseMetaData needs to be built in another way than the one for queries. This
 * class takes that into account.
 */
public class MetaDataRSImpl extends ResultSetImpl {
	
	private String[] DSContainer;
	private String[] ResultHead;
	private String tName;
	private String OneElemStack;
	private String TypeNumber;
	
	public MetaDataRSImpl(ListExpr LE, String[] CHeads) {
		super();
		this.ResultHead = CHeads;
		this.PassedAnswer = LE;
		this.DSPointer = LE;
		this.DSContainer = new String[this.ResultHead.length];
	}
	
	public MetaDataRSImpl(ListExpr LE, String[] CHeads, String tableName) {
		super();
		this.ResultHead = CHeads;
		this.PassedAnswer = LE;
		this.DSPointer = LE;
		this.tName = tableName;
		this.DSContainer = new String[this.ResultHead.length];
	}
	
	private int getHeadNumber(String HName) throws SQLException {
		int i = 0;
		boolean boolResult = false;
		
		while (!boolResult && i<this.ResultHead.length) 
			if (HName.equalsIgnoreCase(this.ResultHead[i]))
				boolResult = true;
			else
				i++;
		if (!boolResult)
			throw new SQLException("invalid ColumnName!");
		return i+1;
		
	}
	
	private void checkConditions(int cNumber) throws SQLException {
		if (cNumber < 0 || cNumber >= ResultHead.length)
			throw new SQLException("invalid ColumnNumber!");
		if (this.DScounter == 0)
			throw new SQLException("no data has been read yet!");
	}
	
	public boolean next() throws SQLException {
		boolean boolResult = false;
		
		// This differentiates between getTables and getColumns since the first one has a ResultSet with
		// 5 columns while the other one has got 18 columns.
		switch (this.ResultHead.length) {
		case 5:
			boolResult = nextTables();
			break;
		case 18:
			boolResult = nextColumns();
		}
		
		return boolResult;
	}
	
	private boolean nextColumns() throws SQLException {
		
		boolean result = false;
		int posBracket;
		int posSpace;
		String DSReader=""; // reads all fields of a dataset
		
		if (this.DScounter == 0) {
			this.DSPointer = this.DSPointer.second().first().first();
			DSReader = this.DSPointer.stringValue();
			DSReader = DSReader.substring(13);
		}
		else
			DSReader = this.OneElemStack;
		
		this.DScounter++;
		if (!DSReader.startsWith(")")) {
			result = true;
			this.DSContainer[2] = this.tName;
			posSpace = DSReader.indexOf(" ");
			
			//DSReader = this.DSPointer.first().first();
			this.DSContainer[3] = DSReader.substring(1, posSpace);
			DSReader = DSReader.substring(posSpace);
			posBracket = DSReader.indexOf(")");
			this.DSContainer[5] = convertInSQLTypes(DSReader.substring(1, posBracket));
			this.DSContainer[4] = this.TypeNumber;
			DSReader = DSReader.substring(posBracket+2);
			this.OneElemStack = DSReader;
		}
		return result;
	}
	
	// The SQL-Types are different from the ones in JAVA ore secondo. Therefore this method converts
	// from secondo-Type into SQL-Type
	private String convertInSQLTypes(String SecType) {
		String result = "";
		
		if (SecType.equalsIgnoreCase("string")) {
			result = "CHAR";
			this.TypeNumber = "1";
		}
		else if(SecType.equalsIgnoreCase("int")) {
			result = "DECIMAL";
			this.TypeNumber = "3";
		}
		else if (SecType.equalsIgnoreCase("real")) {
			result = "REAL";
			this.TypeNumber = "7";
		}
		else if (SecType.equalsIgnoreCase("bool")) {
			result = "BOOLEAN";
			this.TypeNumber = "16";
		}
		
		return result;
	}
	
	private boolean nextTables() throws SQLException {
		
		boolean boolErgebnis = false;
		ListExpr DSReader; // reads all fields of a dataset
		if (this.DScounter == 0) {
			this.DSPointer = this.DSPointer.second();
		}
		else 
			this.DSPointer = this.DSPointer.rest();
		this.DScounter++;
		if (!DSPointer.isEmpty()) {
			boolErgebnis = true;
			DSReader = this.DSPointer.first().first();
			this.DSContainer[2] = this.tName;
			this.DSContainer[3] = DSReader.stringValue();
			this.DSContainer[5] = "TABLE";
		}
		return boolErgebnis;
	}
	
	
	public String getString(int arg0) throws SQLException {
		String StrErgebnis;
		int i = arg0-1;
		this.checkConditions(i);
		StrErgebnis = (String) this.DSContainer[i];
		
		return StrErgebnis;
	}
	
	public String getString(String arg0) throws SQLException {
		return getString(this.getHeadNumber(arg0));
	}
	
	public int getInt(int arg0) throws SQLException {
		int intResult;
		int i = arg0-1;
		this.checkConditions(i);
		intResult = Integer.parseInt(this.DSContainer[i]);
		
		return intResult;
	}
	
	public int getInt(String arg0) throws SQLException {
		return getInt(this.getHeadNumber(arg0));
	}
}
