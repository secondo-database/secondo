package de.fernuni_hagen.dna.jwh.secondopositiontransmitter.representation;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

/**
 * Used to handle database access
 */
public class DatabaseManager extends SQLiteOpenHelper {

    public static final String DATABASE_NAME = "secptm.db";
    SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

    public DatabaseManager(Context context) {
        super(context, DATABASE_NAME, null, 1);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL(
                "create table log " +
                        "(id integer primary key autoincrement, message text, time text)"
        );
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        db.execSQL("DROP TABLE IF EXISTS log");
        onCreate(db);
    }

    /**
     * Insert a new Log entry
     * @param date Date to be used, will be formated
     * @param message Message to log
     * @return
     */
    public boolean insertLog(Date date, String message) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues contentValues = new ContentValues();
        contentValues.put("message", message);
        contentValues.put("time", sdf.format(date));
        db.insert("log", null, contentValues);
        return true;
    }

    /**
     * Get all Entries of the Log as ArrayList, orderd by id desc
     * @return
     */
    public ArrayList<String> getAll() {
        ArrayList<String> array_list = new ArrayList<String>();

        SQLiteDatabase db = this.getReadableDatabase();
        Cursor res = db.rawQuery("select * from log order by id desc", null);
        res.moveToFirst();

        while (res.isAfterLast() == false) {
            array_list.add(res.getString(res.getColumnIndex("time")) + " " + res.getString(res.getColumnIndex("message")));
            res.moveToNext();
        }
        return array_list;
    }

    /**
     * Deletes all Entries except the highest "keep" ids
     * @param keep
     */
    public void deleteOld(Integer keep) {
        SQLiteDatabase db = this.getWritableDatabase();
        Log.i(getClass().getSimpleName(), "Deleted:" + db.delete("log", "id not in (select id from log order by id desc limit ?)", new String[]{keep.toString()}));
    }
}
