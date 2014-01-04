package de.fernunihagen.dna;

import de.fernunihagen.dna.data.DatabaseServerDto;
import de.fernunihagen.dna.data.SecondoDataSource;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.Toast;

/**
 * Main Secondroid Activity an login screen
 * 
 * @author Michael Küpper
 * 
 */
public class SecondroidMainActivity extends Activity implements
		OnClickListener, OnCheckedChangeListener {
//	private static final String TAG = SecondroidMainActivity.class.getName();
	public final static String EXTRA_SERVERNAME = "de.fernunihagen.dna.secondo.SERVERNAME";
	public static final String EXTRA_USERNAME = "de.fernunihagen.dna.secondo.USERNAME";
	public static final String EXTRA_PASSWORD = "de.fernunihagen.dna.secondo.PASSWORD";
	public static final String EXTRA_PORT = "de.fernunihagen.dna.secondo.PORT";
	public static final String EXTRA_OPTSERVERNAME = "de.fernunihagen.dna.secondo.OPTSERVERNAME";
	public static final String EXTRA_OPTPORT = "de.fernunihagen.dna.secondo.OPTPORT";
	public static final String EXTRA_USINGOPTIMIZER = "de.fernunihagen.dna.secondo.USINGOPTIMIZER";
	public static final String EXTRA_SERVERPROP = "de.fernunihagen.dna.secondo.EXTRA_SERVERPROP";
	private static final int EXTRAS_RESULT = 0;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		Button btLogin = (Button) findViewById(R.id.DoLogin);
		btLogin.setOnClickListener(this);

		CheckBox cbUsingOptimizer = (CheckBox) findViewById(R.id.usingOptimizer);
		cbUsingOptimizer.setOnCheckedChangeListener(this);
		
		loadLastUsedServer();
	}

	/*
	 * loads the last Serverproperties from the SQLiteDatabase
	 */
	private void loadLastUsedServer() {
		SecondoDataSource ds = new SecondoDataSource(
				this.getApplicationContext());
		ds.open();
		DatabaseServerDto databaseServerDto = ds.getLastUsedDatabaseserver();
		if (databaseServerDto != null) {
			showServerProperties(databaseServerDto);
		}

		ds.close();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		menu.add(Menu.NONE, 1, 0, getString(R.string.servers));
		return super.onCreateOptionsMenu(menu);
	}

	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		if (requestCode == EXTRAS_RESULT) {
			if (resultCode == RESULT_OK) {
				DatabaseServerDto databaseServerDto = (DatabaseServerDto) data
						.getSerializableExtra(SecondroidMainActivity.EXTRA_SERVERPROP);
				if (databaseServerDto != null) {
					showServerProperties(databaseServerDto);
				}
			}
		}
	}

	private void showServerProperties(DatabaseServerDto databaseServerDto) {
		EditText server = (EditText) findViewById(R.id.DbServerID);
		EditText usernameView = (EditText) findViewById(R.id.DbUserId);
		EditText passwordView = (EditText) findViewById(R.id.DbPasswordId);
		EditText portView = (EditText) findViewById(R.id.PortID);
		EditText optServerView = (EditText) findViewById(R.id.OptServerID);
		EditText optPortView = (EditText) findViewById(R.id.OptPortID);
		CheckBox usingOptimizerCheckBox = (CheckBox) findViewById(R.id.usingOptimizer);

		server.setText(databaseServerDto.getDatabaseServer());
		usernameView.setText(databaseServerDto.getUserName());
		passwordView.setText(databaseServerDto.getPassword());
		portView.setText(databaseServerDto.getPort().toString());
		optServerView.setText(databaseServerDto.getOptServer());
		optPortView.setText(databaseServerDto.getOptPort()
				.toString());
		usingOptimizerCheckBox.setChecked(databaseServerDto
				.isUsingOptimizer());
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case 0:
			startActivity(new Intent(this, ShowSettingsActivity.class));
			return true;
		case 1:
			startActivityForResult(new Intent(this,
					ServerSelectionActivity.class),
					SecondroidMainActivity.EXTRAS_RESULT);
			return true;
		}

		return false;
	}

	@Override
	public void onClick(View v) {
		EditText server = (EditText) findViewById(R.id.DbServerID);
		EditText usernameView = (EditText) findViewById(R.id.DbUserId);
		EditText passwordView = (EditText) findViewById(R.id.DbPasswordId);
		EditText portView = (EditText) findViewById(R.id.PortID);
		EditText optServerView = (EditText) findViewById(R.id.OptServerID);
		EditText optPortView = (EditText) findViewById(R.id.OptPortID);
		CheckBox usingOptimizerCheckBox = (CheckBox) findViewById(R.id.usingOptimizer);

		String servername = server.getText().toString();
		String username = usernameView.getText().toString();
		String password = passwordView.getText().toString();
		String port = portView.getText().toString();

		String optServername = optServerView.getText().toString();
		String optPort = optPortView.getText().toString();
		boolean usingOptimizer = usingOptimizerCheckBox.isChecked();

		if ("".equals(username))
			username = null;
		if ("".equals(password))
			password = null;

		if (v.getId() == R.id.DoLogin) {
			CheckBox loginProperties = (CheckBox) findViewById(R.id.saveLoginProperties);
			if (loginProperties.isChecked()) {
				saveLoginProperties(servername, username, password, port,
						optServername, optPort, usingOptimizer);
			} else {
				updateLastUsing(servername, username, password, port,
						optServername, optPort, usingOptimizer);
			}
				
			Intent i = new Intent(SecondroidMainActivity.this,
					DatabasesActivity.class);
			i.putExtra(EXTRA_SERVERNAME, servername);
			i.putExtra(EXTRA_USERNAME, username);
			i.putExtra(EXTRA_PASSWORD, password);
			i.putExtra(EXTRA_PORT, port);
			i.putExtra(EXTRA_OPTSERVERNAME, optServername);
			i.putExtra(EXTRA_OPTPORT, optPort);
			i.putExtra(EXTRA_USINGOPTIMIZER, usingOptimizer);

			startActivity(i);
		}

	}

	private void updateLastUsing(String servername, String username,
			String password, String port, String optServer, String optPort,
			boolean usingOptimizer) {
		Integer portnumber = 0;
		try {
			portnumber = Integer.parseInt(port);
		} catch (NumberFormatException numEx) {
			portnumber = 1234; // default
		}
		Integer optPortnumber = 0;
		try {
			optPortnumber = Integer.parseInt(optPort);
		} catch (NumberFormatException numEx) {
			optPortnumber = 1235; // default
		}
		SecondoDataSource ds = new SecondoDataSource(
				this.getApplicationContext());
		ds.open();
		DatabaseServerDto databaseDto = new DatabaseServerDto(servername, username, password, portnumber, optServer, optPortnumber, usingOptimizer);

		try {
			ds.updateLastUsing(databaseDto);
		} catch (Exception ex) {
			Toast.makeText(getApplicationContext(),
					getString(R.string.cannotsaveserver), Toast.LENGTH_SHORT).show();
		}
		ds.close();

	}

	private void saveLoginProperties(String servername, String username,
			String password, String port, String optServer, String optPort,
			boolean usingOptimizer) {
		Integer portnumber = 0;
		try {
			portnumber = Integer.parseInt(port);
		} catch (NumberFormatException numEx) {
			portnumber = 1234; // default
		}
		Integer optPortnumber = 0;
		try {
			optPortnumber = Integer.parseInt(optPort);
		} catch (NumberFormatException numEx) {
			optPortnumber = 1235; // default
		}
		SecondoDataSource ds = new SecondoDataSource(
				this.getApplicationContext());
		ds.open();
		DatabaseServerDto databaseDto = new DatabaseServerDto(servername, username, password, portnumber, optServer, optPortnumber, usingOptimizer);

		try {
			ds.save(databaseDto);
		} catch (Exception ex) {
			Toast.makeText(getApplicationContext(),
					getString(R.string.cannotsaveserver), Toast.LENGTH_SHORT).show();
		}
		ds.close();

	}

	@Override
	public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
		switch (buttonView.getId()) {
		case R.id.usingOptimizer:
			EditText optServerView = (EditText) findViewById(R.id.OptServerID);
			EditText optPortView = (EditText) findViewById(R.id.OptPortID);

			optServerView.setEnabled(isChecked);
			optPortView.setEnabled(isChecked);
			break;

		}

	}

}