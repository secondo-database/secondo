package de.fernunihagen.dna;

import java.util.Date;

import de.fernunihagen.dna.data.QueryDto;
import de.fernunihagen.dna.data.SecondoDataSource;
import de.fernunihagen.dna.hoese.QueryResult;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

/**
 * This Activity handles the querys an get the result from QueryAsyncTask
 * 
 * @author Michael Küpper
 * 
 */
public class CommandActivity extends Activity implements OnClickListener {
	private static final String TAG = CommandActivity.class.getName();
	public final static String EXTRA_COMMAND = "de.fernunihagen.dna.secondo.COMMAND";
	public final static String EXTRA_RESULT = "de.fernunihagen.dna.secondo.RESULT";
	public final static String EXTRA_RESULTS = "de.fernunihagen.dna.secondo.RESULTS";
	public final static String EXTRA_DATABASE = "de.fernunihagen.dna.secondo.DATABASE";
	private static final int EXTRA_QUERY = 1;

	protected static final String PROVIDER_NAME = LocationManager.GPS_PROVIDER;
	private Location actLocation;
	private String databaseName;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
		setContentView(R.layout.activity_command);

		setupLocationManager();

		final Intent intent = getIntent();
		databaseName = intent.getStringExtra(DatabasesActivity.EXTRA_MESSAGE);
		TextView textView = (TextView) findViewById(R.id.selectedDatabase);
		textView.setText(databaseName);

		final Button btRun = (Button) findViewById(R.id.runCommand);
		btRun.setOnClickListener(this);
		final Button btHistory = (Button) findViewById(R.id.history);
		btHistory.setOnClickListener(this);
		final ImageButton btLocation = (ImageButton) findViewById(R.id.location);
		btLocation.setOnClickListener(this);

		final Button btShowOutput = (Button) findViewById(R.id.showOutput);
		btShowOutput.setEnabled(false);
		btShowOutput.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				startOutputActivity();
			}
		});

		executeAsyncQuery("open database " + databaseName);
	}

	private void executeAsyncQuery(final String query) {
		QueryAsyncTask openDatabaseQuery = new QueryAsyncTask(this);
		openDatabaseQuery.setContext(this.getApplicationContext());
		SecondoServerService secondoServerService = ServerContext
				.getSecondoServerService();
		secondoServerService.disconnect();
		secondoServerService.setDatabase(databaseName);
		openDatabaseQuery.setSecondoService(secondoServerService);

		openDatabaseQuery.execute(query);
	}

	@Override
	protected void onDestroy() {
		if ("".equals(databaseName))
			return;
		executeAsyncQuery("close database");
		super.onDestroy();
	}

	// @Override
	// protected void onStart() {
	// super.onStart();
	// if ("".equals(databaseName))
	// return;
	// executeAsyncQuery("open database " + databaseName);
	// }

	private void setupLocationManager() {
		// Getting LocationManager object from System Service LOCATION_SERVICE
		LocationManager locationManager = (LocationManager) getSystemService(LOCATION_SERVICE);

		Location location = locationManager.getLastKnownLocation(PROVIDER_NAME);

		LocationListener locationListener = new LocationListener() {
			public void onLocationChanged(Location location) {
				actLocation = location;
				// if (location != null ) {
				// ImageButton button = (ImageButton)
				// findViewById(R.id.location);
				// button.setEnabled(true);
				// }
			}

			@Override
			public void onProviderDisabled(String arg0) {
				Toast.makeText(getApplicationContext(),
						getString(R.string.nogps), Toast.LENGTH_SHORT).show();
				// ImageButton button = (ImageButton)
				// findViewById(R.id.location);
				// button.setEnabled(false);

			}

			@Override
			public void onProviderEnabled(String arg0) {
				Toast.makeText(getApplicationContext(),
						getString(R.string.gps), Toast.LENGTH_SHORT).show();

			}

			@Override
			public void onStatusChanged(String arg0, int arg1, Bundle arg2) {
				// TODO Auto-generated method stub

			}
		};

		if (location != null) {
			locationListener.onLocationChanged(location);
		}

		locationManager.requestLocationUpdates(PROVIDER_NAME, 20000, 0,
				locationListener);
	}

	public void postAsyncTask() {
		boolean intermediateMode = true; // Show the resultwindow
		View resultView = findViewById(R.id.resultText);

		if (intermediateMode) {
			if (resultView.getTag() != null) {
				startOutputActivity();
			}
		}
		
		Button btShowOutput = (Button) findViewById(R.id.showOutput);
		btShowOutput.setEnabled(true);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.command, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle item selection
		TextView commandText = (TextView) findViewById(R.id.commandText);
		switch (item.getItemId()) {
		case R.id.queryKreis:
			commandText
					.setText("query Kreis feed filter[.KName contains \"LK Rosenheim\"] consume");
			return true;
		case R.id.queryLandstrasse:
			commandText
					.setText("query Landstrassen feed filter [.Name contains 'B1:'] consume");
			return true;
		case R.id.queryAutobahn:
			commandText
					.setText("query Autobahn feed filter[.AName contains  \"30\"] consume");
			return true;
		case R.id.queryStadt:
			commandText
					.setText("query Stadt feed filter[.SName contains \"Aa\"] consume");
			return true;
		case R.id.listObjects:
			commandText.setText("list objects");
			return true;
		case R.id.action_settings:
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}

	/**
	 * Verarbeiten des Clickevents auf den Run Button
	 */
	@Override
	public void onClick(View v) {
		TextView commandText = (TextView) findViewById(R.id.commandText);
		switch (v.getId()) {
		case R.id.history:
			Intent i = new Intent(CommandActivity.this,
					QueryHistoryActivity.class);
			i.putExtra(EXTRA_DATABASE, ServerContext.getSecondoServerService()
					.getDatabase());

			startActivityForResult(i, EXTRA_QUERY);
			break;
		case R.id.location:
			// Only if we have a location
			if (actLocation != null) {
				String text = commandText.getText().toString();
				String part1 = text.substring(0,
						commandText.getSelectionStart());
				String part3 = text.substring(commandText.getSelectionEnd());
				commandText.setText(part1
						+ LocationFormatter.format(actLocation) + part3);
			} else {
				Toast.makeText(getApplicationContext(),
						getString(R.string.nolocation), 1).show();

			}

			break;
		case R.id.runCommand:
			String query = commandText.getText().toString();

			if ("".equals(query)) {
				Toast.makeText(getApplicationContext(),
						getString(R.string.noquery), 1).show();
				return;
			}

			// Maybe the query is wrong.
			saveQueryHistory(query);

			Button btShowOutput = (Button) findViewById(R.id.showOutput);
			btShowOutput.setEnabled(false);

			QueryAsyncTask databaseQuery = new QueryAsyncTask(this);
			databaseQuery.setContext(this.getApplicationContext());
			databaseQuery
					.setResultView((EditText) findViewById(R.id.resultText));
			databaseQuery.setSecondoService(ServerContext
					.getSecondoServerService());
			databaseQuery.execute(query);

			break;
		}

	}

	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		if (requestCode == EXTRA_QUERY) {
			if (resultCode == RESULT_OK) {
				TextView commandText = (TextView) findViewById(R.id.commandText);
				commandText.setText(data
						.getStringExtra(CommandActivity.EXTRA_COMMAND));
			}
		}
	}

	private void saveQueryHistory(String query) {
		SecondoDataSource ds = new SecondoDataSource(
				this.getApplicationContext());
		ds.open();
		QueryDto queryDto = new QueryDto();
		queryDto.setDatabase(ServerContext.getSecondoServerService()
				.getDatabase());
		queryDto.setQuery(query);
		queryDto.setStartTime(new Date());
		ds.save(queryDto);
		ds.close();
	}

	private void startOutputActivity() {
		Intent i = new Intent(CommandActivity.this, OutputActivity.class);

		View resultView = findViewById(R.id.resultText);

		QueryResult result = (QueryResult) resultView.getTag();
		i.putExtra(EXTRA_DATABASE, databaseName);

		if (result != null) {
			result.setSelected(true);
			i.putExtra(EXTRA_RESULT, result);
		} else {
			return;
			// i.putExtra(EXTRA_RESULT, new QueryResult());

		}

		startActivity(i);
	}

}
