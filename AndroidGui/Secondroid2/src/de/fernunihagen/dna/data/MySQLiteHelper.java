package de.fernunihagen.dna.data;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class MySQLiteHelper extends SQLiteOpenHelper {
	
	private static final int DATABASE_VERSION = 7;
	private static final String DATABASE_NAME = "Secondroid.db";

	public MySQLiteHelper(Context context) {
		    super(context, DATABASE_NAME, null, DATABASE_VERSION);
		  }
	@Override
	public void onCreate(SQLiteDatabase database) {
		 database.execSQL(QueryDao.createTable());
		 database.execSQL(DatabaseServerDao.createTable());
	}

	public void onCreateVersion6(SQLiteDatabase database) {
		 database.execSQL(DatabaseServerDao.createTable());
	}

	@Override
	public void onUpgrade(SQLiteDatabase database, int oldVersion, int newVersion) {
		 Log.w(MySQLiteHelper.class.getName(),
			        "Upgrading database from version " + oldVersion + " to "
			            + newVersion + ", which will destroy all old data");
		 if (oldVersion == 5 || oldVersion == 6) {
			 database.execSQL(DatabaseServerDao.removeTable());			 
			 onCreateVersion6(database);
		 } else {			 
			 database.execSQL(QueryDao.removeTable());
			 database.execSQL(DatabaseServerDao.removeTable());
			 onCreate(database);
		 }

	}

}
