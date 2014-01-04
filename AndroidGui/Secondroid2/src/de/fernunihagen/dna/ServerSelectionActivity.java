package de.fernunihagen.dna;

import java.util.List;

import de.fernunihagen.dna.data.DatabaseServerDto;
import de.fernunihagen.dna.data.SecondoDataSource;
import android.os.Bundle;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;

public class ServerSelectionActivity extends Activity implements
		OnItemClickListener, OnItemLongClickListener {

	private static final String TAG = "de.fernunihagen.dna.ServerSelectionActivity";
	private List<DatabaseServerDto> values;
	private ArrayAdapter<DatabaseServerDto> adapter;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_server_selection);

		final ListView queryList = (ListView) findViewById(R.id.queryList);

		queryList.setItemsCanFocus(false);
		queryList.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
		queryList.setOnItemClickListener(this);
		queryList.setOnItemLongClickListener(this);
		// Open Datenbase and Query History
		SecondoDataSource datasource = new SecondoDataSource(this);
		datasource.open();

		values = datasource.getAllDatabaseServer();
		datasource.close();
		// Use the SimpleCursorAdapter to show the
		// elements in a ListView
		adapter = new ArrayAdapter<DatabaseServerDto>(this,
				R.xml.historytextitem, values);
		queryList.setAdapter(adapter);
	}

	public boolean onItemLongClick(AdapterView<?> arg0, View arg1,
			int position, long id) {
		final ListView queryList = (ListView) findViewById(R.id.queryList);
		DatabaseServerDto queryDto = (DatabaseServerDto) queryList
				.getItemAtPosition(position);

		AlertDialog.Builder adb = new AlertDialog.Builder(
				ServerSelectionActivity.this);
		adb.setTitle(getString(R.string.delete));
		String queryName = queryDto.getDatabaseServer();

		adb.setMessage(String.format(getString(R.string.removeserver),
				queryName));
		// adb.setMessage("Soll der Server '" + queryName
		// + "' gelöscht werden?");
		final int positionToRemove = position;

		adb.setNegativeButton(getString(R.string.no), null);
		adb.setPositiveButton(getString(R.string.yes),
				new AlertDialog.OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						final ListView queryList = (ListView) findViewById(R.id.queryList);

						DatabaseServerDto databaseServerDto = (DatabaseServerDto) queryList
								.getItemAtPosition(positionToRemove);
						values.remove(databaseServerDto);
						adapter.notifyDataSetChanged();
						delete(databaseServerDto);

					}
				});
		adb.show();

		return false;
	}

	private void delete(DatabaseServerDto databaseServerDto) {
		SecondoDataSource ds = new SecondoDataSource(
				this.getApplicationContext());
		ds.open();
		ds.delete(databaseServerDto);
		ds.close();

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.server_selection, menu);
		return true;
	}

	@Override
	public void onItemClick(AdapterView<?> arg0, View arg1, int position,
			long arg3) {

		final ListView queryList = (ListView) findViewById(R.id.queryList);
		DatabaseServerDto databaseServerDto = (DatabaseServerDto) queryList
				.getItemAtPosition(position);
		Log.i(TAG, "onItemClick: index=" + position + " arg3 " + arg3
				+ " selection: " + databaseServerDto.getDatabaseServer());

		Intent returnIntent = new Intent();
		returnIntent.putExtra(SecondroidMainActivity.EXTRA_SERVERPROP,
				databaseServerDto);
		setResult(RESULT_OK, returnIntent);
		finish();
	}

}
