package de.fernunihagen.dna.data;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

/**
 * DataAccessObject for the Query Datatable
 * @author Michael Küpper
 *
 */
public class QueryDao {
	private SQLiteDatabase database;
	
	static final String TABLENAME = "Query";
	  private String[] allColumns = { "id",
		      "database",
		      "query",
		      "starttime",
		      "duration"
		      };

	  private String[] queryColumns = { "id",
		      "query"
			  };

	  public static String createTable() {
		return "create table "
			      + TABLENAME + 
			      " (id integer primary key autoincrement, " + 
			      "  database text not null," +
			      "  query text not null," +
			      "  starttime int not null," +
			      "  duration int null" +
			      ");";
	}
	
	public static String removeTable() {
		return "DROP TABLE IF EXISTS " + TABLENAME + ";";
	}
	
	public QueryDao(SQLiteDatabase database) {
		this.database = database;
	}
	
	
	public QueryDto save(QueryDto queryDto) {
		QueryDto foundQueryDto = getByQuery(queryDto.getDatabase(), queryDto.getQuery());
		if (foundQueryDto == null) {					    
			ContentValues values = new ContentValues();
		    values.put("database", queryDto.getDatabase());
		    values.put("query", queryDto.getQuery());
		    values.put("starttime", queryDto.getStartTime().getTime());
		    values.put("duration", queryDto.getDuration());
		    long insertId = database.insert(TABLENAME, null,
		        values);
			
	    Cursor cursor = database.query(TABLENAME,
	        allColumns, allColumns[0] + " = " + insertId, null,
	        null, null, null);
	    cursor.moveToFirst();
	    foundQueryDto = cursorToQueryDto(cursor);
	    cursor.close();
		} else {
			// Update 
			ContentValues values = new ContentValues();
		    values.put("database", foundQueryDto.getDatabase());
		    values.put("query", foundQueryDto.getQuery());
		    values.put("starttime", foundQueryDto.getStartTime().getTime());
		    values.put("duration", foundQueryDto.getDuration());
			database.update(TABLENAME, values, "id = " + foundQueryDto.getId(), null );
		}
	    return foundQueryDto;
	}
	

	public void delete(QueryDto queryDto) {

		database.delete(TABLENAME, "id = ?", new String[] {queryDto.getId().toString()});
		return;
	}

	public List<QueryDto> getAll() {
		Cursor cursor = database.query(TABLENAME, allColumns, null, null, null, null, null);

	    return createListFromCursor(cursor);
	}

	
	public QueryDto getByQuery(String databaseName, String query) {
		String[] selection = new String[2];
		selection[0] = databaseName;
		selection[1] = query;
		
		Cursor cursor = database.query(TABLENAME, allColumns, "database = ? and query = ?", selection, null, null, null);

		if (cursor.getCount() == 0) {
			return null; // No Result
		}
		cursor.moveToFirst();
	    return cursorToQueryDto(cursor);
	}


	public List<QueryDto> getAllByDatabase(String databaseName) {
		Cursor cursor = database.query(TABLENAME, allColumns, "database = '" + databaseName + "'", null, null, null, null);
	    return createListFromCursor(cursor);
	}

	private QueryDto cursorToQueryDto(Cursor cursor) {
		QueryDto queryDto = new QueryDto();
		queryDto.setId(cursor.getInt(0));
		queryDto.setDatabase(cursor.getString(1));
		queryDto.setQuery(cursor.getString(2));
		queryDto.setStartTime(new Date(cursor.getLong(3)));
		queryDto.setDuration(cursor.getInt(4));
		return queryDto;
		}


	private List<QueryDto> createListFromCursor(Cursor cursor) {
		List<QueryDto> queryDtos = new ArrayList<QueryDto>();

		cursor.moveToFirst();
	    while (!cursor.isAfterLast()) {
	      QueryDto queryDto = cursorToQueryDto(cursor);
	      queryDtos.add(queryDto);
	      cursor.moveToNext();
	    }
	    
	    // Make sure to close the cursor
	    cursor.close();
	    return queryDtos;
	}

}
