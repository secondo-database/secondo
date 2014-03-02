// SecondoActivity
// (c) Jürgen Ehnes 2012-2013

package eu.ehnes.secondoandroid.activity;

import eu.ehnes.secondoandroid.HandleAssets;
import eu.ehnes.secondoandroid.OwnPath;
import eu.ehnes.secondoandroid.R;
import eu.ehnes.secondoandroid.R.layout;
import eu.ehnes.secondoandroid.R.menu;
import eu.ehnes.secondoandroid.impl.SecondoDba;
import eu.ehnes.secondoandroid.itf.ISecondoDba;
import eu.ehnes.secondoandroid.itf.ISecondoDbaCallback;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager.NameNotFoundException;
import android.view.Menu;
import android.view.View;


public class SecondoActivity extends Activity implements ISecondoDbaCallback {
	//public static starthelper sh;
	public static ISecondoDba secondoDba;
	ProgressDialog progressDialog = null;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		System.out.println("Start des Programms");
		
		// Initialisierung des aktuellen Pfades.
		OwnPath ownpath=OwnPath.getInstance();
		ownpath.setPath(getFilesDir().toString());
		
		System.out.println("Aktueller Pfad: "+ownpath.getPath());
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_secondo);
		
		
		// Die benötigten Dateien werden vom Asset ins Programmverzeichnis entpackt.
		String filesPath=ownpath.getPath();
		String programPath=ownpath.getPath()+"/../";

		HandleAssets handleAssets = HandleAssets.getInstance(getBaseContext());
		
		handleAssets.copyAssets("Config.ini", programPath);
		handleAssets.copyAssets("ProgressConstants.csv",programPath);
		handleAssets.copyAssets("opt",filesPath);
		handleAssets.copyAssets("berlintest",filesPath);
		handleAssets.copyAssets("literature",filesPath);
		handleAssets.copyAssets("constrainttest",filesPath);
		
		// Die Starthelpervariable wird initialisiert
		secondoDba = new SecondoDba();
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
		final String configPath = preferences.getString("configPath", getApplicationContext().getFilesDir().getPath()+"/../Config.ini");

		progressDialog = ProgressDialog.show(this,"Secondo", "Secondo is starting, please be patient ...");
		secondoDba.initializeASync(configPath, this);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.activity_secondo, menu);
		return true;
	}

	
	/**
	 * Opens the settings dialog window
	 *
	 * @param v the View
	 */
	public void call_settings(View v) {
        Intent myIntent = new Intent(v.getContext(), SettingsActivity.class);
        startActivityForResult(myIntent, 0);
	}
	
	/**
	 * Opens the main gui for accessing secondo
	 *
	 * @param v the View
	 */
	public void opengui(View v) {
        Intent myIntent = new Intent(v.getContext(), GuiActivity.class);
        startActivityForResult(myIntent, 0);
	}

	/**
	 * 	call_preferences
	 * 
	 * @param v
	 */
	public void call_preferences(View v) {
		// Settings Dialog aufrufen. Ist momentan noch nicht belegt
		
        Intent myIntent = new Intent(v.getContext(), PreferencesActivity.class);
        startActivityForResult(myIntent, 0);
	}


	/* (non-Javadoc)
	 * @see android.app.Activity#onBackPressed()
	 * 
	 * This method suppresses the back function for the main menu
	 */
	@Override
	public void onBackPressed() {
		//super.onBackPressed();
		return;
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		System.out.println("closedb");
		secondoDba.shutdownSync();
		secondoDba=null;
	}
	
	/**
	 * Opens a new dialog to show the actual memory values
	 *
	 * @param v the View
	 */
	public void showMemory(View v) {
		Intent myIntent = new Intent(v.getContext(), MemoryActivity.class);
		startActivityForResult(myIntent, 0);
		
	}

	@Override
	public void queryCallBack(Object result) {
		System.out.println("queryCallback called");
		
	}

	@Override
	public void initializeCallBack(boolean result) {
		System.out.println("initializeCallback called");
		progressDialog.dismiss();
	

	}
	
}

