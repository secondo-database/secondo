package DriverSet;

import java.io.InputStream;
import java.io.Reader;
import java.math.BigDecimal;
import java.net.URL;
import java.sql.Array;
import java.sql.Blob;
import java.sql.Clob;
import java.sql.Date;
import java.sql.NClob;
import java.sql.Ref;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.RowId;
import java.sql.SQLException;
import java.sql.SQLWarning;
import java.sql.SQLXML;
import java.sql.Statement;
import java.sql.Time;
import java.sql.Timestamp;
import java.util.Calendar;
import java.util.Map;

import tools.Reporter;
import sj.lang.ListExpr;
import Ext_Tools.*;
import Utilities.ColHead;
import Utilities.WarningContainer;
import java.util.Vector;
import java.util.Iterator;

/**
 * 
 * <b> Task of this class </b> <br/>
 * Implements the interface ResultSet. It represents the database output of an sql-query
 */
public class ResultSetImpl implements ResultSet {

	protected ListExpr PassedAnswer;
	protected ListExpr DSPointer; // points on the current dataset
	private ColHead[] ResultHead; // contains the names and types of all colums in the resultset
	protected int DScounter; //counts according to next which dataset the cursor is pointing to
	private Object[] DSContainer; // contains all data of the current dataset
	private Vector<ShadowQualifier> ShList; // contains the Qualifiers
	private int QualCounter; // counts the number of Qualifiers that have already been considered
	private boolean lastResultNull;
	private SQLWarning Warning;
	
	
	// for MetaDataRSImpl an empty constructor is needed
	public ResultSetImpl() {
		this.DScounter = 0;
		this.Warning = WarningContainer.getInstance();
	}
	
	public ResultSetImpl(ListExpr LE) {
		this();
		this.PassedAnswer = LE;
		this.DSPointer = LE;
		this.getHead();
		this.DSContainer = new Object[this.ResultHead.length];
		this.Warning = WarningContainer.getInstance();
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * transforms the columnname into the columnnumber
	 * @param HName columnname
	 * @return columnnumber
	 */
	
	private int getHeadNumber(String HName) throws SQLException {
		int i = 0;
		this.QualCounter = 0;
		boolean result;
		String PassedName = HName;
		int posPeriod;
		
		// the following 7 lines take into account that passedName can be qualified by the tablename
		posPeriod = PassedName.indexOf('.');
		if (posPeriod != -1) {
			String part1, part2;
			part1 = PassedName.substring(0, posPeriod);
			part2 = PassedName.substring(posPeriod+1);
			PassedName = part2 + "_" + part1;
		}
		do {
			while (i<this.ResultHead.length && !ResultHead[i].getName().equalsIgnoreCase(PassedName))
				i++;
			result = i < this.ResultHead.length; // HName has not been found in ResultHead if i==ResultHead.length
			if (!result && this.ShList != null ) { // Shadowlist exists
				PassedName = this.getQualifiedColumnName(HName); // get next qualified name or alias
				if (!PassedName.equalsIgnoreCase("")) // if no qualifier fits "" is passed
					i = 0;     // at this stage i is always > 0. So if set to 0 it means a qualified name 
							   // or an alias could be found. Then all the column heads have to be looked
							   // through again (that is i==0).
			}
		}
		while (!result && i == 0);
		if(!result)
			throw new SQLException("invalid ColumnName!");
		return i+1;
		
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * It gets the qualified name or the alias of the column
	 * @param HeadName
	 * @return
	 */
	private String getQualifiedColumnName(String HeadName) {
		Iterator<ShadowQualifier> it = this.ShList.iterator();; 
		ShadowQualifier qu;
		int i = 0;
		String result="";
		
		while (it.hasNext() && i < this.QualCounter) { // it starts in the QualifierList where it last 
													   // stopped in case there is more than one qualifier
													   // with the same attribute name (e.g. tentest:no and tentytest:no)
			qu = it.next();
			i++;
		}
		
		
		while (it.hasNext() && result.equalsIgnoreCase("")) {
			qu = it.next();
			this.QualCounter++;
			if (HeadName.equalsIgnoreCase(qu.getAtt())) {
				String asexpr=qu.getAsExpr();
				if (asexpr != "")
					result = asexpr;
				else
					result = HeadName + "_" + qu.getQuali();
			}
		}
		
		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * checks weather the columnNumber is correct and that a dataset has been read
	 * @param cNumber ColumnNumber
	 * @throws SQLException
	 */
	
	private boolean checkConditions(int cNumber) throws SQLException {
		boolean result = true;
		
		if (cNumber < 0 || cNumber >= ResultHead.length)
			throw new SQLException("invalid ColumnNumber!");
		if (this.DScounter == 0)
			throw new SQLException("no data has been read yet!");
		if (this.DSContainer[cNumber] == null) 
			result = false;
		this.lastResultNull = !result;
		
		return result;
	}
	
	public void setShadowList(Vector<ShadowQualifier> SList) {
		this.ShList = SList;
	}
	
	@Override
	public boolean isWrapperFor(Class<?> arg0) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public <T> T unwrap(Class<T> arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public boolean absolute(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void afterLast() throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void beforeFirst() throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void cancelRowUpdates() throws SQLException {
		// TODO Auto-generated method stub

	}

	public void clearWarnings() throws SQLException {
		this.Warning = WarningContainer.ClearWarning();
	}

	@Override
	public void close() throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void deleteRow() throws SQLException {
		// TODO Auto-generated method stub

	}

	public int findColumn(String arg0) throws SQLException {
		return getHeadNumber(arg0);
	}

	@Override
	public boolean first() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public Array getArray(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Array getArray(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public InputStream getAsciiStream(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public InputStream getAsciiStream(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public BigDecimal getBigDecimal(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public BigDecimal getBigDecimal(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public BigDecimal getBigDecimal(int arg0, int arg1) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public BigDecimal getBigDecimal(String arg0, int arg1) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public InputStream getBinaryStream(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public InputStream getBinaryStream(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Blob getBlob(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Blob getBlob(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	public boolean getBoolean(int arg0) throws SQLException {
		Boolean BooErgebnis;
		int i = arg0-1;
		if (!this.checkConditions(i))	// checkConditions is false if DSContainer contains null
			return false;
		try {
			BooErgebnis = (Boolean) this.DSContainer[i];
		}
		catch (ClassCastException e) {
			throw new SQLException("Field does not contain a boolean value! \n"+ e.toString());
		}
		return BooErgebnis.booleanValue();
	}

	public boolean getBoolean(String arg0) throws SQLException {
		return getBoolean(this.getHeadNumber(arg0));
	}

	@Override
	public byte getByte(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public byte getByte(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public byte[] getBytes(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public byte[] getBytes(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Reader getCharacterStream(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Reader getCharacterStream(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Clob getClob(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Clob getClob(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public int getConcurrency() throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public String getCursorName() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Date getDate(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Date getDate(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Date getDate(int arg0, Calendar arg1) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Date getDate(String arg0, Calendar arg1) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	public double getDouble(int arg0) throws SQLException {
		Double douResult;
		int i = arg0-1;
		if (!this.checkConditions(i))	// checkConditions is false if DSContainer contains null
			return 0;
		try {
			douResult = (Double) this.DSContainer[i];
		}
		catch (ClassCastException e) {
			throw new SQLException("Field does not contain a double value! \n"+ e.toString());
		}

		return douResult.doubleValue();
	}

	public double getDouble(String arg0) throws SQLException {
		return getDouble(this.getHeadNumber(arg0));
	}

	@Override
	public int getFetchDirection() throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int getFetchSize() throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	public float getFloat(int arg0) throws SQLException {
		Float floResult;
		int i = arg0-1;
		if (!this.checkConditions(i))	// checkConditions is false if DSContainer contains null
			return 0;
		try {
			floResult = (Float) this.DSContainer[i];
		}
		catch (ClassCastException e) {
			throw new SQLException("Field does not contain a float value! \n"+ e.toString());
		}

		return floResult.floatValue();
	}

	public float getFloat(String arg0) throws SQLException {
		return getFloat(this.getHeadNumber(arg0));
	}

	@Override
	public int getHoldability() throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	
	public int getInt(int arg0) throws SQLException {
		int i = arg0-1;
		if (!this.checkConditions(i))	// checkConditions is false if DSContainer contains null
			return 0;
		Integer IntErgebnis;
		try {
			IntErgebnis = (Integer) this.DSContainer[i];
		}
		catch (ClassCastException e) {
			throw new SQLException("Field does not contain an integer! \n"+ e.toString());
		}
		
		return IntErgebnis.intValue();
	}

	public int getInt(String arg0) throws SQLException {
		return getInt(getHeadNumber(arg0));
	}

	@Override
	public long getLong(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public long getLong(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	public ResultSetMetaData getMetaData() throws SQLException {
		return new RSMDImpl(this.ResultHead);
	}

	@Override
	public Reader getNCharacterStream(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Reader getNCharacterStream(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public NClob getNClob(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public NClob getNClob(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String getNString(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String getNString(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Object getObject(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Object getObject(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Object getObject(int arg0, Map<String, Class<?>> arg1)
			throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Object getObject(String arg0, Map<String, Class<?>> arg1)
			throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Ref getRef(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Ref getRef(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public int getRow() throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public RowId getRowId(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public RowId getRowId(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SQLXML getSQLXML(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SQLXML getSQLXML(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public short getShort(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public short getShort(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public Statement getStatement() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	public String getString(int arg0) throws SQLException {
		String StrErgebnis;
		int i = arg0-1;
		if (!this.checkConditions(i))	// checkConditions is false if DSContainer contains null
			return null;
		StrErgebnis = (String) this.DSContainer[i];
		
		return StrErgebnis;
	}

	public String getString(String arg0) throws SQLException {
		return getString(this.getHeadNumber(arg0));
	}

	@Override
	public Time getTime(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Time getTime(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Time getTime(int arg0, Calendar arg1) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Time getTime(String arg0, Calendar arg1) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Timestamp getTimestamp(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Timestamp getTimestamp(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Timestamp getTimestamp(int arg0, Calendar arg1) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Timestamp getTimestamp(String arg0, Calendar arg1)
			throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public int getType() throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public URL getURL(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public URL getURL(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public InputStream getUnicodeStream(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public InputStream getUnicodeStream(String arg0) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	public SQLWarning getWarnings() throws SQLException {
		return this.Warning.getNextWarning();
	}

	@Override
	public void insertRow() throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public boolean isAfterLast() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean isBeforeFirst() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean isClosed() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean isFirst() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean isLast() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean last() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void moveToCurrentRow() throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void moveToInsertRow() throws SQLException {
		// TODO Auto-generated method stub

	}

	/**
	 * reads the next line of a database output.
	 */
	public boolean next() throws SQLException {
		
		boolean boolResult = false;
		int typeOfValue = 0;
		
		ListExpr DSReader; // reads all fields of a dataset
		if (this.DScounter == 0)
			if (this.ResultHead[0].getName() == "")  // aggregat queries like select count(*)... and select min(...
				this.DSPointer = this.DSPointer.rest();
			else
				this.DSPointer = this.DSPointer.second();
		else 
			this.DSPointer = this.DSPointer.rest();
		this.DScounter++;
		if (!DSPointer.isEmpty()) {
			boolResult = true;
			DSReader = this.DSPointer.first();
			for (int i = 0; i < this.DSContainer.length; i++) {
				if (this.ResultHead[0].getName() != "")
					typeOfValue = DSReader.first().atomType();
				if (typeOfValue == 5)
					this.DSContainer[i] = null;
				else {
					switch (this.ResultHead[i].getNumType()) {
					case 4:
						if (this.ResultHead[0].getName() == "") 		// aggregat queries like select count(*)... and select min(...
							this.DSContainer[i] = DSReader.intValue();
						else
							this.DSContainer[i] = DSReader.first().intValue();
						break;
					case 8:
						this.DSContainer[i] = DSReader.first().realValue();
						break;
					case 12:
						this.DSContainer[i] = DSReader.first().stringValue();
						break;
					case -7:
						this.DSContainer[i] = DSReader.first().boolValue();
						break;
					default:
						Reporter.reportInfo("Unbekanntes Format", true);
						break;
					}
				}
				DSReader = DSReader.rest();
			}
		}
		
		return boolResult;
	}

	@Override
	public boolean previous() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void refreshRow() throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public boolean relative(int arg0) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean rowDeleted() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean rowInserted() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean rowUpdated() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void setFetchDirection(int arg0) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void setFetchSize(int arg0) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateArray(int arg0, Array arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateArray(String arg0, Array arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateAsciiStream(int arg0, InputStream arg1)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateAsciiStream(String arg0, InputStream arg1)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateAsciiStream(int arg0, InputStream arg1, int arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateAsciiStream(String arg0, InputStream arg1, int arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateAsciiStream(int arg0, InputStream arg1, long arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateAsciiStream(String arg0, InputStream arg1, long arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBigDecimal(int arg0, BigDecimal arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBigDecimal(String arg0, BigDecimal arg1)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBinaryStream(int arg0, InputStream arg1)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBinaryStream(String arg0, InputStream arg1)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBinaryStream(int arg0, InputStream arg1, int arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBinaryStream(String arg0, InputStream arg1, int arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBinaryStream(int arg0, InputStream arg1, long arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBinaryStream(String arg0, InputStream arg1, long arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBlob(int arg0, Blob arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBlob(String arg0, Blob arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBlob(int arg0, InputStream arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBlob(String arg0, InputStream arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBlob(int arg0, InputStream arg1, long arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBlob(String arg0, InputStream arg1, long arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBoolean(int arg0, boolean arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBoolean(String arg0, boolean arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateByte(int arg0, byte arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateByte(String arg0, byte arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBytes(int arg0, byte[] arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateBytes(String arg0, byte[] arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateCharacterStream(int arg0, Reader arg1)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateCharacterStream(String arg0, Reader arg1)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateCharacterStream(int arg0, Reader arg1, int arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateCharacterStream(String arg0, Reader arg1, int arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateCharacterStream(int arg0, Reader arg1, long arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateCharacterStream(String arg0, Reader arg1, long arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateClob(int arg0, Clob arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateClob(String arg0, Clob arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateClob(int arg0, Reader arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateClob(String arg0, Reader arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateClob(int arg0, Reader arg1, long arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateClob(String arg0, Reader arg1, long arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateDate(int arg0, Date arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateDate(String arg0, Date arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateDouble(int arg0, double arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateDouble(String arg0, double arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateFloat(int arg0, float arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateFloat(String arg0, float arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateInt(int arg0, int arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateInt(String arg0, int arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateLong(int arg0, long arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateLong(String arg0, long arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateNCharacterStream(int arg0, Reader arg1)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateNCharacterStream(String arg0, Reader arg1)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateNCharacterStream(int arg0, Reader arg1, long arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateNCharacterStream(String arg0, Reader arg1, long arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateNClob(int arg0, NClob arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateNClob(String arg0, NClob arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateNClob(int arg0, Reader arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateNClob(String arg0, Reader arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateNClob(int arg0, Reader arg1, long arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateNClob(String arg0, Reader arg1, long arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateNString(int arg0, String arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateNString(String arg0, String arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateNull(int arg0) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateNull(String arg0) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateObject(int arg0, Object arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateObject(String arg0, Object arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateObject(int arg0, Object arg1, int arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateObject(String arg0, Object arg1, int arg2)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateRef(int arg0, Ref arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateRef(String arg0, Ref arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateRow() throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateRowId(int arg0, RowId arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateRowId(String arg0, RowId arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateSQLXML(int arg0, SQLXML arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateSQLXML(String arg0, SQLXML arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateShort(int arg0, short arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateShort(String arg0, short arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateString(int arg0, String arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateString(String arg0, String arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateTime(int arg0, Time arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateTime(String arg0, Time arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateTimestamp(int arg0, Timestamp arg1) throws SQLException {
		// TODO Auto-generated method stub

	}

	@Override
	public void updateTimestamp(String arg0, Timestamp arg1)
			throws SQLException {
		// TODO Auto-generated method stub

	}

	public boolean wasNull() throws SQLException {
		return this.lastResultNull;
	}
	
	private void getHead() {
		ListExpr Ant;
		Ant = this.PassedAnswer.first();
		if (Ant.rest() == null) {  // in case select count(*)... or select min(no)... and so on has been used
			this.ResultHead = new ColHead[1];    
			this.ResultHead[0] = new ColHead("", "int");
		}
		else {
			Ant = Ant.second();
			Ant = Ant.second();
			fillArr(Ant,0);
		}
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * recursive method to fill ResultHead with the column-types of the answer
	 * @param Ant
	 * @param i
	 */
	private void fillArr(ListExpr Ant, int i) {
		if( Ant.isEmpty()) 
			this.ResultHead = new ColHead[i];
		else {
			fillArr(Ant.rest(), i+1);
			ListExpr Follow = Ant.first();
			String Na = Follow.first().stringValue();
			Follow = Follow.second();
			String NT = Follow.stringValue();
			ColHead Sp = new ColHead(Na, NT);
			this.ResultHead[i] = Sp;
		}
	}
	

}
