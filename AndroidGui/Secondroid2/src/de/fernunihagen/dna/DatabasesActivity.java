package de.fernunihagen.dna;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

public class DatabasesActivity extends Activity implements OnItemClickListener {
	private static final String TAG = DatabasesActivity.class.getName();
	public final static String EXTRA_MESSAGE = "de.fernunihagen.dna.secondo.MESSAGE";

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.databases);

		Intent intent = getIntent();
		String serverName = intent
				.getStringExtra(SecondroidMainActivity.EXTRA_SERVERNAME);
		String username = intent
				.getStringExtra(SecondroidMainActivity.EXTRA_USERNAME);
		String password = intent
				.getStringExtra(SecondroidMainActivity.EXTRA_PASSWORD);
		String port = intent.getStringExtra(SecondroidMainActivity.EXTRA_PORT);
		String optServerName = intent
				.getStringExtra(SecondroidMainActivity.EXTRA_OPTSERVERNAME);
		String optPort = intent
				.getStringExtra(SecondroidMainActivity.EXTRA_OPTPORT);
		boolean usingOptimizer = intent.getBooleanExtra(
				SecondroidMainActivity.EXTRA_USINGOPTIMIZER, false);

		final ListView listview = (ListView) findViewById(R.id.listview);

		listview.setItemsCanFocus(false);
		listview.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
		listview.setOnItemClickListener(this);

		ConnectAsyncTask databaseQuery = new ConnectAsyncTask();
		databaseQuery.setContext(this.getApplicationContext());
		databaseQuery.execute(serverName, username, password, port,
				optServerName, optPort, usingOptimizer ? "1" : "0");
	}

	public void setAdapter(StableArrayAdapter adapter) {
		final ListView listview = (ListView) findViewById(R.id.listview);

		listview.setAdapter(adapter);
	}

	private class StableArrayAdapter extends ArrayAdapter<String> {

		HashMap<String, Integer> mIdMap = new HashMap<String, Integer>();

		public StableArrayAdapter(Context context, int textViewResourceId,
				List<String> objects) {
			super(context, textViewResourceId, R.id.databaseEntryText, objects);
			for (int i = 0; i < objects.size(); ++i) {
				mIdMap.put(objects.get(i), i);
				Log.i(TAG, "Create Adapter");
			}
		}

		@Override
		public long getItemId(int position) {
			String item = getItem(position);
			Log.i(TAG, "index=" + position + " " + item);
			return mIdMap.get(item);
		}

		@Override
		public boolean hasStableIds() {
			return true;
		}

	}

	@Override
	public void onItemClick(AdapterView<?> arg0, View arg1, int position,
			long arg3) {

		final ListView listview = (ListView) findViewById(R.id.listview);
		String selection = (String) listview.getItemAtPosition(position);
		Log.i(TAG, "onItemClick: index=" + position + " arg3 " + arg3
				+ " selection: " + selection);

		Intent i = new Intent(DatabasesActivity.this, CommandActivity.class);

		i.putExtra(EXTRA_MESSAGE, selection);
		startActivity(i);
	}

	public class ConnectAsyncTask extends
			AsyncTask<String, Integer, List<String>> {
		private Context context;
		private List<String> databases = new ArrayList<String>();

		public void setContext(Context context) {
			this.context = context;
		}

		protected List<String> doInBackground(String... prop) {

			if (ServerContext.getSecondoServerService(prop[0], prop[3],
					prop[1], prop[2], prop[4], prop[5],
					"1".equals(prop[6]) ? true : false) == null) {
				SecondoServerService secondoServerService = new SecondoServerService();
				secondoServerService.setContext(getApplicationContext());
				secondoServerService.connect(prop[0], prop[3], prop[1],
						prop[2], prop[4], prop[5], "1".equals(prop[6]) ? true
								: false);

				if (secondoServerService.isConnected()) {
					ServerContext.setSecondoServerService(secondoServerService);
				} else {
					return null;
				}

			}

			databases = ServerContext.getSecondoServerService().getDatabases();

			Log.i(TAG, "Found databases: " + databases.toString());
			return databases;

		}

		protected void onProgressUpdate(Integer... progress) {
			Log.i(TAG, "onProgressUpdate");
		}

		protected void onPostExecute(List<String> result) {
			if (result == null || result.size() == 0) {
				Toast.makeText(this.context, "keine Datenbanken gefunden!",
						Toast.LENGTH_LONG).show();
				return;
			}

			final StableArrayAdapter adapter = new StableArrayAdapter(context,
					R.layout.database_entry, result);

			setAdapter(adapter);
		}

	}
}
