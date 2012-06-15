package DriverSet;

import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import Utilities.ColHead;
import Utilities.WarningContainer;

import java.sql.SQLWarning;

/**
 * 
 * <b> Task of this class </b> <br/>
 * Implementation of the interface ResultSetMetaData
 */
public class RSMDImpl implements ResultSetMetaData {

	private ColHead[] Head;
	
	public RSMDImpl(ColHead[] hd) {
		this.Head = hd;
	}
	
	@Override
	public <T> T unwrap(Class<T> iface) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public boolean isWrapperFor(Class<?> iface) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	public int getColumnCount() throws SQLException {
		return this.Head.length;
	}

	@Override
	public boolean isAutoIncrement(int column) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean isCaseSensitive(int column) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean isSearchable(int column) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean isCurrency(int column) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public int isNullable(int column) throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public boolean isSigned(int column) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public int getColumnDisplaySize(int column) throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public String getColumnLabel(int column) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	public String getColumnName(int column) throws SQLException {
		return this.Head[column-1].getName();
	}

	@Override
	public String getSchemaName(int column) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public int getPrecision(int column) throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int getScale(int column) throws SQLException {
		// TODO Auto-generated method stub
		return 0;
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

	@Override
	public String getCatalogName(int column) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	public int getColumnType(int column) throws SQLException {
		return this.Head[column-1].getNumType();
	}

	public String getColumnTypeName(int column) throws SQLException {
		return this.Head[column-1].getNamType();
	}

	@Override
	public boolean isReadOnly(int column) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean isWritable(int column) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean isDefinitelyWritable(int column) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public String getColumnClassName(int column) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

}
