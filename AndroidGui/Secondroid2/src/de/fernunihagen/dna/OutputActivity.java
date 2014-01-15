package de.fernunihagen.dna;

import java.util.Vector;

import javamini.awt.geom.Rectangle2D;

import sj.lang.ListExpr;

import de.fernunihagen.dna.hoese.DsplBase;
import de.fernunihagen.dna.hoese.DsplGraph;
import de.fernunihagen.dna.hoese.LEUtils;
import de.fernunihagen.dna.hoese.QueryResult;
import de.fernunihagen.dna.hoese.QueryResultHelper;
import de.fernunihagen.dna.hoese.QueryResultHelper.CoordinateSystem;
import de.fernunihagen.dna.osm.OsmMapActivity;
import android.os.Bundle;
import android.app.Activity;
import android.app.TabActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TabHost;
import android.widget.TabHost.OnTabChangeListener;
import android.widget.TabHost.TabSpec;
import android.content.Intent;

public class OutputActivity extends TabActivity implements OnTabChangeListener {
	private static final String TAG = "OutputActivity";
	private static Vector<QueryResult> queryresults = new Vector<QueryResult>(5);
	private static String database;
	private boolean textTabDirty = false;
	private boolean mapTabDirty = false;
	private boolean cancasTabDirty = false;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_output);
		// Show the Up button in the action bar.

		TabHost tabHost = getTabHost();
		tabHost.setup();
		Intent intent = getIntent();
		QueryResult queryResultExtra = null;

		if (intent.hasExtra(CommandActivity.EXTRA_RESULT))
			queryResultExtra = (QueryResult) intent
					.getSerializableExtra(CommandActivity.EXTRA_RESULT);
		if (intent.hasExtra(CommandActivity.EXTRA_DATABASE)) {
			String actDatabase = (String) intent
					.getSerializableExtra(CommandActivity.EXTRA_DATABASE);
			if (!actDatabase.equals(OutputActivity.database)) {// New Database,
																// so clear the
																// Resultlist
				queryresults.clear();
				OutputActivity.database = actDatabase;
			}
		}

		
		if (!queryresults.contains(queryResultExtra) || queryResultExtra.getStrings().size() == 0) {
			final ListExpr listExpr = queryResultExtra.getResultList();

			if (listExpr == null) {
				Log.e(TAG, "no result");
				return;
			}

			if (listExpr.first() != null) {
				String name = listExpr.first().toString();
				LEUtils.analyse(name, 0, 0, listExpr.first(), listExpr.second(),
						queryResultExtra);
			}

			Log.i(TAG, queryResultExtra.getStrings().toString());
			if (!queryresults.contains(queryResultExtra)) {
				queryresults.add(queryResultExtra);
			}
		} 

		// new result is always the actual result
		QueryResultHelper.setActQueryResult(queryresults, queryResultExtra);

		// Tab for Textoutput
		TabSpec textSpec = tabHost.newTabSpec("Text");
		// setting Tabtitle and Icon
		textSpec.setIndicator("Text",
				getResources().getDrawable(R.drawable.icon_text_tab));
		Intent textIntent = new Intent(this, TextActivity.class);
		textIntent.putExtra(CommandActivity.EXTRA_RESULT, queryResultExtra);
		textIntent.putExtra(CommandActivity.EXTRA_RESULTS,
				OutputActivity.queryresults);
		textSpec.setContent(textIntent);

		// Tab for Results
		TabSpec resultSpec = tabHost.newTabSpec("Results");
		// setting Tabtitle and Icon
		resultSpec.setIndicator(getString(R.string.results), getResources()
				.getDrawable(R.drawable.icon_text_tab));
		Intent resultIntent = new Intent(this, ResultsActivity.class);
		resultIntent.putExtra(CommandActivity.EXTRA_RESULTS,
				OutputActivity.queryresults);
		resultSpec.setContent(resultIntent);

		// Tab for Map
		TabSpec mapSpec = tabHost.newTabSpec("Map");
		mapSpec.setIndicator(getString(R.string.map), getResources()
				.getDrawable(R.drawable.icon_map_tab));
		Intent mapIntent = new Intent(this, OsmMapActivity.class);
		mapIntent.putExtra(CommandActivity.EXTRA_RESULT, queryResultExtra);
		mapIntent.putExtra(CommandActivity.EXTRA_RESULTS,
				OutputActivity.queryresults);
		mapSpec.setContent(mapIntent);

		// Tab for canvas
		TabSpec canvasSpec = tabHost.newTabSpec("Canvas");

		canvasSpec.setIndicator(getString(R.string.grafic), getResources()
				.getDrawable(R.drawable.icon_map_tab));
		Intent canvasIntent = new Intent(this, CanvasActivity.class);
		canvasIntent.putExtra(CommandActivity.EXTRA_RESULT, queryResultExtra);
		canvasIntent.putExtra(CommandActivity.EXTRA_RESULTS,
				OutputActivity.queryresults);
		canvasSpec.setContent(canvasIntent);

		// Adding all TabSpec to TabHost
		tabHost.addTab(resultSpec);
		tabHost.addTab(textSpec);
		tabHost.addTab(canvasSpec);
		tabHost.addTab(mapSpec);
		tabHost.setOnTabChangedListener(this);

		// deactivate one Tab
		// tabHost.getTabWidget().getChildTabViewAt(3).setEnabled(false);
		getTabHost().setCurrentTab(1); // Set the Text Tab as first visible Tab
	}


	public void switchTab(int tab) {
		mapTabDirty = true;
		getTabHost().setCurrentTab(tab);
	}


	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.output, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// switch (item.getItemId()) {
		// case android.R.id.home:
		// // This ID represents the Home or Up button. In the case of this
		// // activity, the Up button is shown. Use NavUtils to allow users
		// // to navigate up one level in the application structure. For
		// // more details, see the Navigation pattern on Android Design:
		// //
		// //
		// http://developer.android.com/design/patterns/navigation.html#up-vs-back
		// //
		// NavUtils.navigateUpFromSameTask(this);
		// return true;
		// }
		return super.onOptionsItemSelected(item);
	}

	@Override
	public void onTabChanged(String name) {
		if ("Text".equals(name)) {
			Log.w(TAG, "onTabChanged TEXT");
			Activity currentActivity = getCurrentActivity();
			if (currentActivity instanceof TextActivity) {
				if (textTabDirty) {
					((TextActivity) currentActivity).updateActivity();
				}
				textTabDirty = false;
			}
		}
		if ("Map".equals(name)) {
			Log.w(TAG, "onTabChanged MAP");
			Activity currentActivity = getCurrentActivity();
			if (currentActivity instanceof OsmMapActivity) {
				if (mapTabDirty) {
					((OsmMapActivity) currentActivity).updateActivity();
				}
				mapTabDirty = false;
			}

		}
		if ("Graphic".equals(name)) {
			Log.w(TAG, "onTabChanged Graphic");
			Activity currentActivity = getCurrentActivity();
			if (currentActivity instanceof CanvasActivity) {
				if (cancasTabDirty) {
					((CanvasActivity) currentActivity).updateActivity();
				}
				mapTabDirty = false;
			}
		}

	}

	public void invalidateTabs() {
		textTabDirty = true;
		mapTabDirty = true;
		cancasTabDirty = true;
	}

}
