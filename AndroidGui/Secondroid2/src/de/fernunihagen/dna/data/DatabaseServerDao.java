package de.fernunihagen.dna.data;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

public class DatabaseServerDao {
	private SQLiteDatabase database;
	
	static final String TABLENAME = "databaseserver";

	private static final String TAG = "de.fernunihagen.dna.data.DatabaseServerDao";
	  private String[] allColumns = { "id",
		      "databaseserver",
		      "username",
		      "password",
		      "port",
		      "optserver",
		      "optport",
		      "usingoptimizer",		      
		      "lastusing",		      
		      };

//	  private String[] queryColumns = { "id",
//		      "databaseserver",
//		      "username"
//			  };

	  public static String createTable() {
		return "create table "
			      + TABLENAME + 
			      " (id integer primary key autoincrement, " + 
			      "  databaseserver text not null," +
			      "  username text null," +
			      "  password int null," +
			      "  port int null," +
			      "  optserver text not null," +
			      "  optport int null," +
			      "  usingoptimizer int null," +
			      "  lastusing int null" +
			      ");";
	}
	
	public static String removeTable() {
		return "DROP TABLE IF EXISTS " + TABLENAME + ";";
	}
	
	public DatabaseServerDao(SQLiteDatabase database) {
		this.database = database;
	}
	
	/**
	 * Save the DatabaseserverDto object If an equal Object exists in the
	 * Database, update this object
	 * 
	 * @param databaseServerDto
	 * @return
	 * @throws Exception Datenbankfehler
	 */
	public DatabaseServerDto save(DatabaseServerDto databaseServerDto) throws Exception {
		DatabaseServerDto foundQueryDto = null;
		try {
			foundQueryDto = getByDatabaseServer(
					databaseServerDto.getDatabaseServer(),
					databaseServerDto.getUserName());
			if (foundQueryDto == null) {
				ContentValues values = new ContentValues();
				values.put("databaseserver",
						databaseServerDto.getDatabaseServer());
				values.put("username", databaseServerDto.getUserName());
				values.put("password", databaseServerDto.getPassword());
				values.put("port", databaseServerDto.getPort());
				values.put("optserver",
						databaseServerDto.getOptServer());
				values.put("optport", databaseServerDto.getOptPort());
				values.put("usingoptimizer", databaseServerDto.isUsingOptimizer() ? 1 : 0);
				values.put("lastusing", new Date().getTime());
				long insertId = database.insert(TABLENAME, null, values);

				Cursor cursor = database.query(TABLENAME, allColumns,
						allColumns[0] + " = " + insertId, null, null, null,
						null);
				cursor.moveToFirst();
				foundQueryDto = cursorToDatabaseServerDto(cursor);
				cursor.close();
			} else {
				// Update current Object
				ContentValues values = new ContentValues();
				values.put("databaseserver", foundQueryDto.getDatabaseServer());
				values.put("username", foundQueryDto.getUserName());
				values.put("password", foundQueryDto.getPassword());
				values.put("port", foundQueryDto.getPort());
				values.put("optserver",
						databaseServerDto.getOptServer());
				values.put("optport", databaseServerDto.getOptPort());
				values.put("usingoptimizer", databaseServerDto.isUsingOptimizer() ? 1 : 0);
				values.put("lastusing", new Date().getTime());
				database.update(TABLENAME, values,
						"id = " + foundQueryDto.getId(), null);
			}
		} catch (Exception ex) {
			Log.e(TAG, "Speichern der Datenbankeinstellungen fehlgeschlagen", ex);
			throw ex;
		}
		return foundQueryDto;
	}
	
	public List<DatabaseServerDto> getAll() {
		Cursor cursor = database.query(TABLENAME, allColumns, null, null, null, null, null);

	    return createListFromCursor(cursor);
	}

	
	public DatabaseServerDto getByDatabaseServer(String databaseserver, String username) {
		
		Cursor cursor;
		if (username == null) {
			cursor = database.query(TABLENAME, allColumns, "databaseserver = ? and username is null", 
					new String[] {databaseserver}, 
					null, null, null);
		} else {
			cursor = database.query(TABLENAME, allColumns, "databaseserver = ? and username = ?", 
					new String[] {databaseserver, username},
					null, null, null);

		}

		if (cursor.getCount() == 0) {
			return null; // No Result
		}
		cursor.moveToFirst();
	    return cursorToDatabaseServerDto(cursor);
	}


	public List<DatabaseServerDto> getAllByDatabaseServer(String databaseServer) {
		Cursor cursor = database.query(TABLENAME, allColumns, "databaseserver = '" + databaseServer + "'", null, null, null, null);
	    return createListFromCursor(cursor);
	}

	private DatabaseServerDto cursorToDatabaseServerDto(Cursor cursor) {
		DatabaseServerDto databaseServerDto = new DatabaseServerDto();
		databaseServerDto.setId(cursor.getInt(0));
		databaseServerDto.setDatabaseServer(cursor.getString(1));
		databaseServerDto.setUserName(cursor.getString(2));
		databaseServerDto.setPassword(cursor.getString(3));
		databaseServerDto.setPort(cursor.getInt(4));
		databaseServerDto.setOptServer(cursor.getString(5));
		databaseServerDto.setOptPort(cursor.getInt(6));
		databaseServerDto.setUsingOptimizer(cursor.getInt(7) == 1);
		
		return databaseServerDto;
		}


	private List<DatabaseServerDto> createListFromCursor(Cursor cursor) {
		List<DatabaseServerDto> databseServerDtos = new ArrayList<DatabaseServerDto>();

		cursor.moveToFirst();
	    while (!cursor.isAfterLast()) {
	    	DatabaseServerDto databaseServerDto = cursorToDatabaseServerDto(cursor);
	      databseServerDtos.add(databaseServerDto);
	      cursor.moveToNext();
	    }
	    
	    // Make sure to close the cursor
	    cursor.close();
	    return databseServerDtos;
	}

	public void delete(DatabaseServerDto databaseServerDto) {
		database.delete(TABLENAME, "id = ?", new String[] {databaseServerDto.getId().toString()});
	}
	
	public void updateLastUsing(DatabaseServerDto databaseServerDto) {
		ContentValues values = new ContentValues();

		values.put("lastusing", new Date().getTime());

		database.update(TABLENAME, values, "id = ?", new String[] {databaseServerDto.getId().toString()});
		
	}

	/**
	 * Get the last used databaseserver
	 * @return last used databaseserver properties
	 */
	public DatabaseServerDto getLastUsedDatabaseserver() {
		Cursor cursor = database.query(TABLENAME, allColumns, null, null, null, null, "lastusing" +  " desc");

		if (cursor.getCount() == 0) {
			return null; // No Result
		}
		cursor.moveToFirst();
	    return cursorToDatabaseServerDto(cursor);
	}


}
