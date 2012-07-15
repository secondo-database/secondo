package DriverSet;

import java.sql.Connection;
import java.sql.DatabaseMetaData;
import java.sql.ResultSet;
import java.sql.RowIdLifetime;
import java.sql.SQLException;
import SecExceptions.NotCompSQL92Exception;
import SecExceptions.NotSuppMetaDBaseException;
import communication.CommunicationInterface;

/**
 * 
 * <b> Task of this class </b> <br/>
 * Implements the interface DatabaseMetaDataImpl. It provides metainformation about Secondo
 */
public class DatabaseMetaDataImpl implements DatabaseMetaData {

	private CommunicationInterface ComInt;
	private ResultSet RSAnswer;
	
	public DatabaseMetaDataImpl(CommunicationInterface CI) {
		this.ComInt = CI;
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

	public boolean allProceduresAreCallable() throws SQLException {
		return true;
	}

	public boolean allTablesAreSelectable() throws SQLException {
		return true;
	}

	public String getURL() throws SQLException {
		return this.ComInt.getUrl();		
	}

	@Override
	public String getUserName() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public boolean isReadOnly() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean nullsAreSortedHigh() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean nullsAreSortedLow() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean nullsAreSortedAtStart() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean nullsAreSortedAtEnd() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	public String getDatabaseProductName() throws SQLException {
		return "Secondo";
	}

	public String getDatabaseProductVersion() throws SQLException {
		return "3.2.1";
	}

	public String getDriverName() throws SQLException {
		return "secondo";
	}

	public String getDriverVersion() throws SQLException {
		return "1.0";
	}

	public int getDriverMajorVersion() {
		return 1;
	}

	public int getDriverMinorVersion() {
		return 0;
	}

	@Override
	public boolean usesLocalFiles() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean usesLocalFilePerTable() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsMixedCaseIdentifiers() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean storesUpperCaseIdentifiers() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean storesLowerCaseIdentifiers() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean storesMixedCaseIdentifiers() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsMixedCaseQuotedIdentifiers() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean storesUpperCaseQuotedIdentifiers() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean storesLowerCaseQuotedIdentifiers() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean storesMixedCaseQuotedIdentifiers() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	// Quoted identifiers are not supported by secondo
	public String getIdentifierQuoteString() throws SQLException {
		return " ";
	}

	@Override
	public String getSQLKeywords() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	public String getNumericFunctions() throws SQLException {
		System.out.println("This shows a limited choice. More algebra modules can be activated to add more numeric functions!");
		return "addition, substraction, multiplication, division, integer division, modulo operation";
	}

	public String getStringFunctions() throws SQLException {
		System.out.println("This shows a limited choice. More algebra modules can be activated to add string functions!");
		return "";
	}

	@Override
	public String getSystemFunctions() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String getTimeDateFunctions() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String getSearchStringEscape() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	public String getExtraNameCharacters() throws SQLException {
		return "";
	}

	@Override
	public boolean supportsAlterTableWithAddColumn() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsAlterTableWithDropColumn() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsColumnAliasing() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean nullPlusNonNullIsNull() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsConvert() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsConvert(int fromType, int toType)
			throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsTableCorrelationNames() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsDifferentTableCorrelationNames() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsExpressionsInOrderBy() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsOrderByUnrelated() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsGroupBy() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsGroupByUnrelated() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsGroupByBeyondSelect() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsLikeEscapeClause() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsMultipleResultSets() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsMultipleTransactions() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsNonNullableColumns() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsMinimumSQLGrammar() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsCoreSQLGrammar() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsExtendedSQLGrammar() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsANSI92EntryLevelSQL() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsANSI92IntermediateSQL() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsANSI92FullSQL() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsIntegrityEnhancementFacility() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsOuterJoins() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsFullOuterJoins() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsLimitedOuterJoins() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	public String getSchemaTerm() throws SQLException {
		throw new NotSuppMetaDBaseException("Catalogs and Schemas are not supported in secondo. Therefore getSchemaTerm() ");
	}

	@Override
	public String getProcedureTerm() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String getCatalogTerm() throws SQLException {
		throw new NotSuppMetaDBaseException("Catalogs and Schemas are not supported in secondo. Therefore getCatalogTerm() ");
	}

	@Override
	public boolean isCatalogAtStart() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	public String getCatalogSeparator() throws SQLException {
		throw new NotSuppMetaDBaseException("Catalogs and Schemas are not supported in secondo. Therefore getCatalogSeparator() ");
	}

	@Override
	public boolean supportsSchemasInDataManipulation() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsSchemasInProcedureCalls() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsSchemasInTableDefinitions() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsSchemasInIndexDefinitions() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsSchemasInPrivilegeDefinitions() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsCatalogsInDataManipulation() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsCatalogsInProcedureCalls() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsCatalogsInTableDefinitions() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsCatalogsInIndexDefinitions() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsCatalogsInPrivilegeDefinitions() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	//ResultSet cursor can be used to delete a row in the table
	public boolean supportsPositionedDelete() throws SQLException {
		// is not supported by secondo
		return false;
	}

	public boolean supportsPositionedUpdate() throws SQLException {
		// is not supported by secondo
		return false;
	}

	@Override
	public boolean supportsSelectForUpdate() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsStoredProcedures() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsSubqueriesInComparisons() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsSubqueriesInExists() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsSubqueriesInIns() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsSubqueriesInQuantifieds() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsCorrelatedSubqueries() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsUnion() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsUnionAll() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsOpenCursorsAcrossCommit() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsOpenCursorsAcrossRollback() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsOpenStatementsAcrossCommit() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsOpenStatementsAcrossRollback() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	public int getMaxBinaryLiteralLength() throws SQLException {
		throw new NotSuppMetaDBaseException("getMaxBinaryLiteralLength() ");
	}

	// maximum length of string values and symbols is 48 characters
	public int getMaxCharLiteralLength() throws SQLException {
		return 48;
	}

	// maximum length of string values and symbols is 48 characters
	public int getMaxColumnNameLength() throws SQLException {
		return 48;
	}

	// no restrictions in secondo
	public int getMaxColumnsInGroupBy() throws SQLException {
		return 0;
	}

	// An index can just be put on a single column
	public int getMaxColumnsInIndex() throws SQLException {
		System.out.println("WARNING! Indeces are not supported by the JDBC implementation!");
		return 1;
	}

	// no restrictions in secondo
	public int getMaxColumnsInOrderBy() throws SQLException {
		return 0;
	}

	// no restrictions in secondo
	public int getMaxColumnsInSelect() throws SQLException {
		return 0;
	}

	// no restrictions in secondo
	public int getMaxColumnsInTable() throws SQLException {
		return 0;
	}

	public int getMaxConnections() throws SQLException {
		return 100;
	}

	@Override
	public int getMaxCursorNameLength() throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	public int getMaxIndexLength() throws SQLException {
		System.out.println("WARNING! Indeces are not supported by the JDBC implementation!");
		return 0;
	}

	// schemas are not supported by secondo
	public int getMaxSchemaNameLength() throws SQLException {
		throw new NotSuppMetaDBaseException("getMaxSchemaNameLength() ");
	}

	public int getMaxProcedureNameLength() throws SQLException {
		return 48;
	}

	// catalogs are not supported by secondo
	public int getMaxCatalogNameLength() throws SQLException {
		throw new NotSuppMetaDBaseException("getMaxCatalogNameLength() ");
	}

	// no restrictions in secondo
	public int getMaxRowSize() throws SQLException {
		return 0;
	}

	public boolean doesMaxRowSizeIncludeBlobs() throws SQLException {
		throw new NotSuppMetaDBaseException("doesMaxRowSizeIncludeBlobs() ");
	}

	@Override
	public int getMaxStatementLength() throws SQLException {
		// TODO Auto-generated method stub
		return 0;
	}

	// one statement per connection is allowed
	public int getMaxStatements() throws SQLException {
		return 1;
	}

	public int getMaxTableNameLength() throws SQLException {
		return 48;
	}

	// no restrictions in secondo
	public int getMaxTablesInSelect() throws SQLException {
		return 0;
	}

	public int getMaxUserNameLength() throws SQLException {
		return 48;
	}

	// Transactions are not activated per default
	public int getDefaultTransactionIsolation() throws SQLException {
		return Connection.TRANSACTION_NONE;
	}

	@Override
	public boolean supportsTransactions() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsTransactionIsolationLevel(int level)
			throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsDataDefinitionAndDataManipulationTransactions()
			throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsDataManipulationTransactionsOnly()
			throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	public boolean dataDefinitionCausesTransactionCommit() throws SQLException {
		/* transaction will automatically commit */
		return true;
	}

	public boolean dataDefinitionIgnoredInTransactions() throws SQLException {
		// Data definition will be executed
		return false;
	}

	@Override
	public ResultSet getProcedures(String catalog, String schemaPattern,
			String procedureNamePattern) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	// Wie werden Prozeduren realisiert?
	public ResultSet getProcedureColumns(String catalog, String schemaPattern,
			String procedureNamePattern, String columnNamePattern)
			throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	/**
	 *  task of this class </br>
	 *  returns the tables of the given database
	 */
	public ResultSet getTables(String catalog, String schemaPattern,
			String tableNamePattern, String[] types) throws SQLException {
		
		// All parameters have to be null because tableNamePattern are not supported. The type can only
		// be table and catalogs and schemas are generally not supported by secondo.
		if (catalog != null || schemaPattern != null || tableNamePattern != null || types != null)
			throw new NotSuppMetaDBaseException("getTables cannot be restricted. All parameters needs to be null! Therefore getTables() with parameters unequal null");
		
		// The array tableStructure is needed in case the ResultSet is requested by columnname instead
		// of columnnumber
		String[] tableStructure = new String[5];
		tableStructure[0] = "TABLE_CAT";
		tableStructure[1] = "TABLE_SCHEM";
		tableStructure[2] = "TABLE_NAME";
		tableStructure[3] = "TABLE_TYPE";
		tableStructure[4] = "REMARKS";
		
		this.ComInt.executeSecSettings("query getcatalog() filter[ substr(.TypeExpr,1,5) = '(rel '] project[ObjectName] consume");
		this.RSAnswer = new MetaDataRSImpl(this.ComInt.getResult(), tableStructure);
		
		return this.RSAnswer;
	}

	public ResultSet getSchemas() throws SQLException {
		throw new NotSuppMetaDBaseException("Catalogs and Schemas are not supported in secondo. Therefore getSchemas() ");
	}

	public ResultSet getCatalogs() throws SQLException {
		throw new NotSuppMetaDBaseException("Catalogs and Schemas are not supported in secondo. Therefore getCatalogs() ");
	}

	@Override
	public ResultSet getTableTypes() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	/**
	 * task of this method
	 * returns the columns of a given table. The parameter columnNamePattern needs to be null because 
	 * it is not supported. catalog and schema needs to be null because they are generally not supported
	 * by secondo. tableNamePattern needs to contain the complete name of the table because wildcards are
	 * not supported.
	 */
	public ResultSet getColumns(String catalog, String schemaPattern,
			String tableNamePattern, String columnNamePattern)
			throws SQLException {
		if (catalog != null || schemaPattern != null || columnNamePattern != null)
			throw new NotSuppMetaDBaseException("getColumns can just be restricted by tableName. All other parameters needs to be null! Therefore getTables() with these parameters");
		
		if (tableNamePattern == null || tableNamePattern.contains("%") || tableNamePattern.contains("_"))
			throw new SQLException("The use of % or _ in the tablename or a tablename containing null in getColumns() is not allowed.");
		
		// The array tableStructure is needed in case the ResultSet is requested by columnname instead
		// of columnnumber
		String[] tableStructure = new String[18];
		tableStructure[0] = "TABLE_CAT";
		tableStructure[1] = "TABLE_SCHEM";
		tableStructure[2] = "TABLE_NAME";
		tableStructure[3] = "COLUMN_NAME";
		tableStructure[4] = "DATA_TYPE";
		tableStructure[5] = "TYPE_NAME";
		tableStructure[6] = "COLUMN_SIZE";
		tableStructure[7] = "BUFFER_LENGTN";
		tableStructure[8] = "DECIMAL_DIGITS";
		tableStructure[9] = "NUM_PREC_RADIX";
		tableStructure[10] = "NULLABLE";
		tableStructure[11] = "REMARKS";
		tableStructure[12] = "COLUMN_DEF";
		tableStructure[13] = "SQL_DATA_TYPE";
		tableStructure[14] = "SQL_DATETIME_SUB";
		tableStructure[15] = "CHAR_OCTET_LENGTH";
		tableStructure[16] = "ORDINAL_POSITION";
		tableStructure[17] = "IS_NULLABLE";
		
		this.ComInt.executeSecSettings("query getcatalog() filter[ .ObjectName = '"+ tableNamePattern +"'] project[TypeExpr] consume");
		//this.ComInt.executeSecSettings("list objects");
		this.RSAnswer = new MetaDataRSImpl(this.ComInt.getResult(), tableStructure, tableNamePattern);
		// TODO Auto-generated method stub
		return this.RSAnswer;
		//throw new NotSuppMetaDBaseException("getColumns() ");
	}

	public ResultSet getColumnPrivileges(String catalog, String schema,
			String table, String columnNamePattern) throws SQLException {
		throw new NotSuppMetaDBaseException("getColumnPrivileges() ");
	}

	public ResultSet getTablePrivileges(String catalog, String schemaPattern,
			String tableNamePattern) throws SQLException {
		throw new NotSuppMetaDBaseException("getTablePrivileges() ");
	}

	public ResultSet getBestRowIdentifier(String catalog, String schema,
			String table, int scope, boolean nullable) throws SQLException {
		throw new NotSuppMetaDBaseException("getBestRowIdentifier() ");
	}

	@Override
	public ResultSet getVersionColumns(String catalog, String schema,
			String table) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	public ResultSet getPrimaryKeys(String catalog, String schema, String table)
			throws SQLException {
		throw new NotSuppMetaDBaseException("getPrimaryKeys() ");
	}

	public ResultSet getImportedKeys(String catalog, String schema, String table)
			throws SQLException {
		throw new NotSuppMetaDBaseException("getImportedKeys() ");
	}

	// primary and foreign keys are not supported by secondo
	public ResultSet getExportedKeys(String catalog, String schema, String table)
			throws SQLException {
		throw new NotSuppMetaDBaseException("getExportedKeys() ");
	}

	public ResultSet getCrossReference(String parentCatalog,
			String parentSchema, String parentTable, String foreignCatalog,
			String foreignSchema, String foreignTable) throws SQLException {
		throw new NotSuppMetaDBaseException("getCrossReference() ");
	}

	@Override
	public ResultSet getTypeInfo() throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ResultSet getIndexInfo(String catalog, String schema, String table,
			boolean unique, boolean approximate) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public boolean supportsResultSetType(int type) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsResultSetConcurrency(int type, int concurrency)
			throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean ownUpdatesAreVisible(int type) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean ownDeletesAreVisible(int type) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean ownInsertsAreVisible(int type) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean othersUpdatesAreVisible(int type) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean othersDeletesAreVisible(int type) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean othersInsertsAreVisible(int type) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean updatesAreDetected(int type) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	public boolean deletesAreDetected(int type) throws SQLException {
		
		throw new NotCompSQL92Exception("deletesAreDetected()");
	}

	@Override
	public boolean insertsAreDetected(int type) throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsBatchUpdates() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public ResultSet getUDTs(String catalog, String schemaPattern,
			String typeNamePattern, int[] types) throws SQLException {
		// TODO Auto-generated method stub
		return null;
	}

	public Connection getConnection() throws SQLException {
		throw new NotCompSQL92Exception("getConnection()");
	}

	@Override
	public boolean supportsSavepoints() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsNamedParameters() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsMultipleOpenResults() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsGetGeneratedKeys() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public ResultSet getSuperTypes(String catalog, String schemaPattern,
			String typeNamePattern) throws SQLException {
		throw new NotCompSQL92Exception("getSuperTypes()");
	}

	public ResultSet getSuperTables(String catalog, String schemaPattern,
			String tableNamePattern) throws SQLException {
		throw new NotCompSQL92Exception("getSuperTables()");
	}

	public ResultSet getAttributes(String catalog, String schemaPattern,
			String typeNamePattern, String attributeNamePattern)
			throws SQLException {
		throw new NotCompSQL92Exception("getAttributes()");
	}

	@Override
	public boolean supportsResultSetHoldability(int holdability)
			throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	public int getResultSetHoldability() throws SQLException {
		throw new NotCompSQL92Exception("getResultSetHoldablility()");
	}

	public int getDatabaseMajorVersion() throws SQLException {
		throw new NotCompSQL92Exception("getDatabaseMajorVersion()");
	}

	public int getDatabaseMinorVersion() throws SQLException {
		throw new NotCompSQL92Exception("getDatabaseMinorVersion()");
	}

	public int getJDBCMajorVersion() throws SQLException {
		throw new NotCompSQL92Exception("getJDBCMajorVersion()");
	}

	public int getJDBCMinorVersion() throws SQLException {
		throw new NotCompSQL92Exception("getJDBCMinorVersion()");
	}

	public int getSQLStateType() throws SQLException {
		throw new NotCompSQL92Exception("getSQLStateType()");
	}

	@Override
	public boolean locatorsUpdateCopy() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean supportsStatementPooling() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	public RowIdLifetime getRowIdLifetime() throws SQLException {
		throw new NotCompSQL92Exception("getRowIdLifetime()");
	}

	public ResultSet getSchemas(String catalog, String schemaPattern)
			throws SQLException {
		throw new NotSuppMetaDBaseException("Catalogs and Schemas are not supported in secondo. Therefore getSchemas() ");
	}

	@Override
	public boolean supportsStoredFunctionsUsingCallSyntax() throws SQLException {
		// TODO Auto-generated method stub
		return false;
	}

	public boolean autoCommitFailureClosesAllResultSets() throws SQLException {
		throw new NotCompSQL92Exception("autoCommitFailureClosesAllResultSets()");
	}

	public ResultSet getClientInfoProperties() throws SQLException {
		throw new NotCompSQL92Exception("getClientInfoProperties()");
	}

	public ResultSet getFunctions(String catalog, String schemaPattern,
			String functionNamePattern) throws SQLException {
		throw new NotCompSQL92Exception("getFunctions()");
	}

	public ResultSet getFunctionColumns(String catalog, String schemaPattern,
			String functionNamePattern, String columnNamePattern)
			throws SQLException {
		throw new NotCompSQL92Exception("getFunctionColumns()");
	}

}
