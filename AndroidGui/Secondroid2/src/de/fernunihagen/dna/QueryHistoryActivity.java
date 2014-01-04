package de.fernunihagen.dna;

import java.util.List;

import de.fernunihagen.dna.data.QueryDto;
import de.fernunihagen.dna.data.SecondoDataSource;
import android.os.Bundle;
import android.app.Activity;
import android.app.AlertDialog;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.AdapterView.OnItemClickListener;
import android.content.DialogInterface;
import android.content.Intent;

/**
 * QueryHistoryActivity shows the list of Querys an allows to select on. The
 * selected Query stored in the ActivityResult
 * 
 * @author Michael Küpper
 * 
 */
public class QueryHistoryActivity extends Activity implements
		OnItemClickListener, OnItemLongClickListener {

	private static final String TAG = "de.fernunihagen.dna.QueryHistoryActivity";
	private List<QueryDto> values;
	private ArrayAdapter<QueryDto> adapter;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_query_history);

		Intent intent = getIntent();
		String databaseName = intent
				.getStringExtra(CommandActivity.EXTRA_DATABASE);

		final ListView queryList = (ListView) findViewById(R.id.queryList);

		queryList.setItemsCanFocus(false);
		queryList.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
		queryList.setOnItemClickListener(this);
		queryList.setOnItemLongClickListener(this);
		// Open Datenbase and Query History
		SecondoDataSource datasource = new SecondoDataSource(this);
		datasource.open();

		values = datasource.getAllByDatabase(databaseName);
		datasource.close();
		// Use the SimpleCursorAdapter to show the
		// elements in a ListView
		adapter = new ArrayAdapter<QueryDto>(this, R.xml.historytextitem,
				values);
		queryList.setAdapter(adapter);
	}

	@Override
	public void onItemClick(AdapterView<?> arg0, View arg1, int position,
			long arg3) {

		final ListView queryList = (ListView) findViewById(R.id.queryList);
		QueryDto queryDto = (QueryDto) queryList.getItemAtPosition(position);
		Log.i(TAG, "onItemClick: index=" + position + " arg3 " + arg3
				+ " selection: " + queryDto.getQuery());

		Intent returnIntent = new Intent();
		returnIntent.putExtra(CommandActivity.EXTRA_COMMAND,
				queryDto.getQuery());
		setResult(RESULT_OK, returnIntent);
		finish();
	}

	public boolean onItemLongClick(AdapterView<?> arg0, View arg1,
			int position, long id) {
		final ListView queryList = (ListView) findViewById(R.id.queryList);
		QueryDto queryDto = (QueryDto) queryList.getItemAtPosition(position);

		AlertDialog.Builder adb = new AlertDialog.Builder(
				QueryHistoryActivity.this);
		adb.setTitle(getString(R.string.delete));
		String queryName = queryDto.getQuery().substring(0,
				Math.min(20, queryDto.getQuery().length()));

		adb.setMessage(String
				.format(getString(R.string.removequery), queryName));
		final int positionToRemove = position;

		adb.setNegativeButton(getString(R.string.no), null);
		adb.setPositiveButton(getString(R.string.yes),
				new AlertDialog.OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						final ListView queryList = (ListView) findViewById(R.id.queryList);

						QueryDto queryDto = (QueryDto) queryList
								.getItemAtPosition(positionToRemove);
						values.remove(queryDto);
						adapter.notifyDataSetChanged();
						delete(queryDto);

					}
				});
		adb.show();

		return false;
	}

	private void delete(QueryDto queryDto) {
		SecondoDataSource ds = new SecondoDataSource(
				this.getApplicationContext());
		ds.open();
		ds.delete(queryDto);
		ds.close();

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.query_history, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {

		return super.onOptionsItemSelected(item);
	}
}
