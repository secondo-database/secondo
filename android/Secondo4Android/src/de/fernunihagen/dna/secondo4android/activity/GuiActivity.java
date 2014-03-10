package de.fernunihagen.dna.secondo4android.activity;

import de.fernunihagen.dna.secondo4android.R;
import de.fernunihagen.dna.secondocore.AccessDatabase;
import de.fernunihagen.dna.secondocore.itf.ISecondoDbaCallback;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

/**
 * The Class GuiActivity.
 */
public class GuiActivity extends Activity implements ISecondoDbaCallback{
	private String path = null;
	private String filename = null;
	TextView messageLine;
	ProgressDialog progressDialog = null;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_gui);
		messageLine = (TextView) findViewById(R.id.statusline);

	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
	}

	/**
	 * Start the Open Database Dialog.
	 * 
	 * @param v
	 *            the view
	 */
	public void opendatabase(View v) {
		String dbname = AccessDatabase.ActiveDatabase(SecondoActivity.secondoDba);
		if (dbname.length() > 0) {
			Toast.makeText(
					this,
					"Unable to open database, please close other database first.",
					Toast.LENGTH_LONG).show();

		} else {
			Intent myIntent = new Intent(v.getContext(),
					DatabaseOpenActivity.class);
			startActivityForResult(myIntent, 0);
		}
	}

	/**
	 * Start the Delete Database dialog.
	 * 
	 * @param v
	 *            the view
	 */
	public void deletedatabase(View v) {
		String dbname = AccessDatabase.ActiveDatabase(SecondoActivity.secondoDba);
		if (dbname.length() > 0) {
			Toast.makeText(this,
					"Unable to delete database, please close database first.",
					Toast.LENGTH_LONG).show();

		} else {
			Intent myIntent = new Intent(v.getContext(),
					DatabaseDeleteActivity.class);
			startActivityForResult(myIntent, 0);
		}

	}

	/**
	 * Opens a new windows with the objects of the actual open database
	 * 
	 * @param v
	 *            the view
	 */
	public void listobjects(View v) {
		String dbname = AccessDatabase.ActiveDatabase(SecondoActivity.secondoDba);
		if (dbname.length() == 0) {
			Toast.makeText(this,
					"Unable to process query, please open a database first.",
					Toast.LENGTH_LONG).show();

		} else {
			Intent myIntent = new Intent(v.getContext(),
					ListObjectsActivity.class);
			startActivityForResult(myIntent, 0);
		}

	}

	public void restoredatabase(View v) {
		// Dialog für das einlesen von Datenbanken aufrufen
		String dbname = AccessDatabase.ActiveDatabase(SecondoActivity.secondoDba);
		if (dbname.length() > 0) {
			Toast.makeText(this,
					"Restore not possible, please close database first.",
					Toast.LENGTH_LONG).show();

		} else {
			Intent myIntent = new Intent(v.getContext(),
					FileDialogActivity.class);
			startActivityForResult(myIntent, 0);
		}
	}

	/**
	 * Closes the active database. Does not call a subview, closes the database
	 * directly.
	 * 
	 * @param v
	 *            the view
	 */
	public void closedatabase(View v) {
		// Schliessen der Datenbank
		// Fehler werden nicht abgefragt
		String dbname = AccessDatabase.ActiveDatabase(SecondoActivity.secondoDba);
		if (dbname.length() > 0) {
			SecondoActivity.secondoDba.querySync("close database");
			messageLine.setText("No database open");
			Toast.makeText(this, "Database closed", Toast.LENGTH_LONG).show();

		} else {
			Toast.makeText(this, "No open database.", Toast.LENGTH_LONG).show();

		}
	}

	/**
	 * Opens a new dialog windows. In this window you can enter own queries to
	 * acess the database. See also in the QueryDatabaseActivity
	 * 
	 * @param v
	 *            the v
	 */
	public void querydatabase(View v) {
		Intent myIntent = new Intent(v.getContext(),
				QueryDatabaseActivity.class);
		startActivityForResult(myIntent, 0);

	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);

		if (resultCode == RESULT_OK) {

			Bundle rucksack = data.getExtras();
			path = rucksack.getString("resultPath");
			filename = rucksack.getString("resultFile");
			progressDialog = ProgressDialog.show(this, "Secondo", "Secondo is restoring a database from file. Please wait!");

			SecondoActivity.secondoDba.queryASync("restore database " + filename + " from '"
					+ path + "'", this);
		}

		System.out.println("Ergebnispfad: " + path);
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);

		if (hasFocus) {
			String dbname = AccessDatabase.ActiveDatabase(SecondoActivity.secondoDba);
			if (dbname.length() > 0) {
				messageLine.setText("Open database: " + dbname);
			} else {
				messageLine.setText("No database open");
			}

		}
	}

	@Override
	public void queryCallBack(Object result) {
		if (progressDialog != null && progressDialog.isShowing()) {
			progressDialog.dismiss();
		}
	}

	@Override
	public void initializeCallBack(boolean result) {
		// not called
	}

}
