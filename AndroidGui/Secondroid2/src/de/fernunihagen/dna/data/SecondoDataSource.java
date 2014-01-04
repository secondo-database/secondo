package de.fernunihagen.dna.data;

import java.util.List;

import android.content.Context;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;

/**
 * database access wrapper
 * @author Michael Küpper
 *
 */
public class SecondoDataSource {

	// Database fields
	private SQLiteDatabase database;
	private MySQLiteHelper dbHelper;

	public SecondoDataSource(Context context) {
		dbHelper = new MySQLiteHelper(context);
	}

	public void open() throws SQLException {
		database = dbHelper.getWritableDatabase();
	}

	public void close() {
		dbHelper.close();
	}

	public QueryDto save(QueryDto queryDto) {
		QueryDao queryDao = new QueryDao(database);
		return queryDao.save(queryDto);
	}

	public void delete(QueryDto queryDto) {
		QueryDao queryDao = new QueryDao(database);
		queryDao.delete(queryDto);
	}

	public void delete(DatabaseServerDto databaseServerDto) {
		DatabaseServerDao databaseServerDao = new DatabaseServerDao(database);
		databaseServerDao.delete(databaseServerDto);
	
	}
	
	public List<QueryDto> getAll() {
		QueryDao queryDao = new QueryDao(database);
		return queryDao.getAll();
	}

	public List<QueryDto> getAllByDatabase(String databaseName) {
		QueryDao queryDao = new QueryDao(database);
		return queryDao.getAllByDatabase(databaseName);
	}

	public DatabaseServerDto save(DatabaseServerDto databaseServerDto) throws Exception {
		DatabaseServerDao databaseServerDao = new DatabaseServerDao(database);
		return databaseServerDao.save(databaseServerDto);
	}


	public void updateLastUsing(DatabaseServerDto databaseServerDto) throws Exception {
		DatabaseServerDao databaseServerDao = new DatabaseServerDao(database);
		databaseServerDao.updateLastUsing(databaseServerDto);
	}

	public List<DatabaseServerDto> getAllDatabaseServer() {
		DatabaseServerDao databaseServerDao = new DatabaseServerDao(database);
		return databaseServerDao.getAll();
	}

	public List<DatabaseServerDto> getAllByDatabaseserver(String databaseServer, String username) {
		DatabaseServerDao databaseServerDao = new DatabaseServerDao(database);
		return databaseServerDao.getAllByDatabaseServer(databaseServer);
	}

	public DatabaseServerDto getLastUsedDatabaseserver() {
		DatabaseServerDao databaseServerDao = new DatabaseServerDao(database);
		return databaseServerDao.getLastUsedDatabaseserver();
		
	}


}
