// SecondoActivity
// (c) Jürgen Ehnes 2012-2013

package eu.ehnes.secondoandroid;

import eu.ehnes.secondoandroid.R;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.view.Menu;
import android.view.View;


public class SecondoActivity extends Activity {
	public static starthelper sh;
	
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
		sh=new starthelper();
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
		String configPath = preferences.getString("configPath", getApplicationContext().getFilesDir().getPath()+"/../Config.ini");

		sh.initialize(configPath);

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
		sh.shutdown();
		sh=null;
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
	
}

