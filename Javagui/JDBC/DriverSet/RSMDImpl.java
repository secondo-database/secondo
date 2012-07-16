package DriverSet;

import java.sql.ResultSetMetaData;
import java.sql.SQLException;

import SecExceptions.NotSuppDriverException;
import Utilities.ColHead;
import Utilities.WarningContainer;

import java.sql.SQLWarning;

/**
 * 
 * <b> Task of this class </b> <br/>
 * Implementation of the interface ResultSetMetaData
 * It provides metadata of the current resultset 
 */
public class RSMDImpl implements ResultSetMetaData {

	private ColHead[] Head;
	
	public RSMDImpl(ColHead[] hd) {
		this.Head = hd;
	}
	
	public <T> T unwrap(Class<T> iface) throws SQLException {
		throw new NotSuppDriverException("The ResultSetMetaData-method unwrap()");
	}

	public boolean isWrapperFor(Class<?> iface) throws SQLException {
		throw new NotSuppDriverException("The ResultSetMetaData-method isWrapperFor()");
	}

	public int getColumnCount() throws SQLException {
		return this.Head.length;
	}

	public boolean isAutoIncrement(int column) throws SQLException {
		throw new NotSuppDriverException("The ResultSetMetaData-method isAutoIncrement()");
	}

	public boolean isCaseSensitive(int column) throws SQLException {
		return false;
	}

	public boolean isSearchable(int column) throws SQLException {
		return false;
	}

	public boolean isCurrency(int column) throws SQLException {
		return false;
	}

	public int isNullable(int column) throws SQLException {
		return ResultSetMetaData.columnNoNulls;
	}

	public boolean isSigned(int column) throws SQLException {
		throw new NotSuppDriverException("The ResultSetMetaData-method isSigned()");
	}

	public int getColumnDisplaySize(int column) throws SQLException {
		throw new NotSuppDriverException("The ResultSetMetaData-method getColumnDisplaySize()");
	}

	public String getColumnLabel(int column) throws SQLException {
		throw new NotSuppDriverException("The ResultSetMetaData-method getColumnLabel()");
	}

	public String getColumnName(int column) throws SQLException {
		return this.Head[column-1].getName();
	}

	public String getSchemaName(int column) throws SQLException {
		throw new NotSuppDriverException("The ResultSetMetaData-method getSchemaName()");
	}

	public int getPrecision(int column) throws SQLException {
		throw new NotSuppDriverException("The ResultSetMetaData-method getPrecision()");
	}

	public int getScale(int column) throws SQLException {
		throw new NotSuppDriverException("The ResultSetMetaData-method getScale()");
	}

	public String getTableName(int column) throws SQLException {
		String result = "";
		String ColName = this.Head[column-1].getName();
		int posHyphen = ColName.indexOf('_');
		if (posHyphen != -1)
			result = ColName.substring(posHyphen+1);
		else {
			SQLWarning Warning = new SQLWarning("getTableName will just return a result if a qualified column has been evaluated");
			WarningContainer.getInstance().setNextWarning(Warning);
		}
		return result;
	}

	public String getCatalogName(int column) throws SQLException {
		throw new NotSuppDriverException("The ResultSetMetaData-method getCatalogName()");
	}

	public int getColumnType(int column) throws SQLException {
		return this.Head[column-1].getNumType();
	}

	public String getColumnTypeName(int column) throws SQLException {
		return this.Head[column-1].getNamType();
	}

	public boolean isReadOnly(int column) throws SQLException {
		return false;
	}

	public boolean isWritable(int column) throws SQLException {
		return true;
	}

	public boolean isDefinitelyWritable(int column) throws SQLException {
		return true;
	}

	public String getColumnClassName(int column) throws SQLException {
		throw new NotSuppDriverException("The ResultSetMetaData-method getColumnClassName()");
	}

}
