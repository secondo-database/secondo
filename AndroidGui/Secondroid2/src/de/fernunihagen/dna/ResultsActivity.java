package de.fernunihagen.dna;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import de.fernunihagen.dna.hoese.Category;
import de.fernunihagen.dna.hoese.QueryResult;
import de.fernunihagen.dna.hoese.QueryResultHelper;
import android.os.Bundle;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.CompoundButton.OnCheckedChangeListener;

/**
 * Shows the Results in a ListView
 * @author Michael Küpper
 *
 */
public class ResultsActivity extends Activity implements
		OnItemLongClickListener, OnClickListener {

	static final String EXTRA_CATEGORY = "de.fernunihagen.dna.secondo.CATEGORY";
	static final String EXTRA_QUERYRESULT = "de.fernunihagen.dna.secondo.QUERYRESULT";

	private static final int EXTRA_RESULT = 0;
	private int actQueryResult;

	List<QueryResult> queryResults;

	@SuppressWarnings("unchecked")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_results);

		Intent intent = getIntent();
		queryResults = (List<QueryResult>) intent
				.getSerializableExtra(CommandActivity.EXTRA_RESULTS);
		assert (queryResults != null);

		final StableArrayAdapter adapter = new StableArrayAdapter(this,
				R.xml.resulttextitem, convertToItem(queryResults), queryResults);

		final ListView listview = (ListView) findViewById(R.id.resultsview);

		listview.setOnItemClickListener(new AdapterView.OnItemClickListener() {

			@Override
			public void onItemClick(AdapterView<?> parent, final View view,
					int position, long id) {
				CheckBox checkBox = (CheckBox) view
						.findViewById(R.id.itemcheck);
				QueryResultHelper.setActQueryResult(queryResults, queryResults.get((Integer) checkBox.getTag()));
				invalidateTabs();
	        	switchTabInActivity(1); // Goto TextTab
			}

		});

		listview.setOnItemLongClickListener(this);
		listview.setAdapter(adapter);

	}
	public void switchTabInActivity(int indexTabToSwitchTo){
        OutputActivity parentActivity;
        parentActivity = (OutputActivity) this.getParent();
        parentActivity.switchTab(indexTabToSwitchTo);
}

	private List<String> convertToItem(List<QueryResult> queryResults) {
		ArrayList<String> items = new ArrayList<String>();
		for (QueryResult queryResult : queryResults) {
			String string = queryResult.getCommand();
			items.add(string);
		}

		return items;
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.results, menu);

		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle item selection
		switch (item.getItemId()) {
		case R.id.clearResultlist:
			queryResults.clear();
			final StableArrayAdapter adapter = new StableArrayAdapter(this,
					R.xml.resulttextitem, convertToItem(queryResults),
					queryResults);

			final ListView listview = (ListView) findViewById(R.id.resultsview);
			listview.setAdapter(adapter);

			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}

	@Override
	public boolean onItemLongClick(AdapterView<?> arg0, View view,
			int position, long id) {

		CheckBox checkBox = (CheckBox) view.findViewById(R.id.itemcheck);
		actQueryResult = (Integer) checkBox.getTag();
		startCategoryActivity();

		return false;
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.properties:
			actQueryResult = (Integer) v.getTag();
			startCategoryActivity();
			break;
		}
	}

	/**
	 * Start the activity to change the Style Properties
	 */
	private void startCategoryActivity() {
		QueryResult queryResult = queryResults.get(actQueryResult);
		// Create the Preference Dialog
		Intent i = new Intent(ResultsActivity.this, CategoryActivity.class);

		i.putExtra(ResultsActivity.EXTRA_CATEGORY, queryResult.getCategory());
		startActivityForResult(i, EXTRA_RESULT);
	}

	private void invalidateTabs() {
		OutputActivity outputActivity = (OutputActivity) getParent();
		outputActivity.invalidateTabs();
	}

	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		if (requestCode == EXTRA_RESULT) {
			if (resultCode == RESULT_OK) {
				Category cat = (Category) data
						.getSerializableExtra(ResultsActivity.EXTRA_CATEGORY);
				QueryResult queryResult = queryResults.get(actQueryResult);
				queryResult.setCategory(cat);
				invalidateTabs();
			}
		}
	}

	static class ItemViewHolder {
		TextView text;
		CheckBox checkbox;
		ImageButton button;
	}

	private class StableArrayAdapter extends ArrayAdapter<String> {

		private static final String TAG = "StableArrayAdapter";
		final private HashMap<String, Integer> mIdMap = new HashMap<String, Integer>();
		final private List<QueryResult> queryResults;

		public StableArrayAdapter(Context context, int textViewResourceId,
				List<String> objects, List<QueryResult> queryResults) {
			super(context, textViewResourceId, objects);
			this.queryResults = queryResults;
			for (int i = 0; i < objects.size(); ++i) {
				mIdMap.put(objects.get(i), i);
			}
		}

		@Override
		public long getItemId(int position) {
			String item = getItem(position);
			return mIdMap.get(item);
		}

		@Override
		public boolean hasStableIds() {
			return true;
		}

		public View getView(int position, View convertView, ViewGroup parent) {
			View row = convertView;
			// Holderpattern: http://www.jmanzano.es/blog/?p=166
			final ItemViewHolder holder;

			if (row == null) {
				holder = new ItemViewHolder();
				LayoutInflater inflater = getLayoutInflater();
				row = inflater.inflate(R.xml.resulttextitem, parent, false);
				CheckBox checkBox = (CheckBox) row.findViewById(R.id.itemcheck);
				checkBox.setChecked(queryResults.get(position).isSelected());

				ImageButton button = (ImageButton) row
						.findViewById(R.id.properties);
				button.setBackgroundColor(Color.BLACK);
				button.setOnClickListener((ResultsActivity) getContext());
				// There is a problem, when using a ImageButton in ListView and
				// using the onItemCLick Event
				// http://xjaphx.wordpress.com/2011/07/14/listview-doesnt-respond-to-onitemclicklistener/
				button.setFocusable(false);
				TextView label = (TextView) row.findViewById(R.id.dbServer);
				holder.checkbox = checkBox;
				holder.text = label;
				holder.button = button;
				row.setTag(holder);

				checkBox.setOnCheckedChangeListener(new OnCheckedChangeListener() {

					@Override
					public void onCheckedChanged(CompoundButton buttonView,
							boolean isChecked) {
						Log.w(TAG, "Tag [" + buttonView.getTag() + "]");
						queryResults.get((Integer) buttonView.getTag())
								.setSelected(isChecked);
					}
				});

			} else {
				holder = (ItemViewHolder) convertView.getTag();
			}

			String text = getItem(position);

			holder.text.setText(text);
			holder.checkbox.setTag(position); // Zuerst die Position setzen
			holder.button.setTag(position);
			boolean selected = queryResults.get(position).isSelected();
			Log.w(TAG, "1: setChecked [" + selected + "]");
			holder.checkbox.setChecked(selected);
			Log.w(TAG, "2: setChecked [" + selected + "]");
			return (row);
		}
	}
}
