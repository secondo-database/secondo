package eu.ehnes.secondoandroid;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.OutputStreamWriter;

import eu.ehnes.secondoandroid.R;
import android.app.Activity;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;

/**
 * The Class SettingsActivity. This class opens a dialog and loads the file config.ini to edit this file.
 * 
 * @author juergen
 * @version 1.0
 */
public class SettingsActivity extends Activity {
	private EditText ausgabe=null;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		String ausgabeText="";
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.activity_settings);
		ausgabe=(EditText) findViewById(R.id.configeditview);
		//programPath=OwnPath.getInstance().getPath()+"/../";

		/** 
		 * Datei einlesen
		 */
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
		String configPath = preferences.getString("configPath", getApplicationContext().getFilesDir().getPath()+"/../Config.ini");

		try {

           //BufferedReader input = new BufferedReader(new FileReader(programPath+"/"+fileName));
            BufferedReader input = new BufferedReader(new FileReader(configPath));
            String inputLine;
	 
	            while ((inputLine = input.readLine()) != null) {
	                ausgabeText = ausgabeText + '\n' + inputLine;
	            }
	            if (ausgabeText.endsWith("\n"))
	                ausgabeText = ausgabeText + "\n";
	 
	            input.close();
		} catch(IOException e) {
			Log.e("FU", "Failed to open Configfile file: " + configPath, e );

		}
		
		// Datei in Textfeld schreiben
		ausgabe.setText(ausgabeText);
		//InputMethodManager imm=(InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
		//imm.hideSoftInputFromWindow(ausgabe.getWindowToken(), 0);
		getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
		
	}
	
	/**
	 * Not sure if needed or not
	 *
	 * @param savedInstanceState the saved instance state
	 */
	protected void onDestroy(Bundle savedInstanceState) {
		finish();
	}
	
	/**
	 * Saves the Config.ini-File from the dialog window to file on SD-Card
	 *
	 * @param v the View
	 */
	public void save(View v) {
		
		// String aus Textfeld auslesen
		String ausgabeText=ausgabe.getText().toString();
		
		//String wieder in Datei schreiben
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
		String configPath = preferences.getString("configPath", getApplicationContext().getFilesDir().getPath()+"/../Config.ini");

		try {
			BufferedWriter output=new BufferedWriter(new OutputStreamWriter(new FileOutputStream(configPath)));
			output.write(ausgabeText,0,ausgabeText.length());
			output.close();
		} catch(IOException e) {
			System.out.println("Konnte Datei nicht schreiben: "+configPath+" "+e);
		}
		
		finish();
		
	}
	
	
	/**
	 * Ignores changes to Config.ini-File and returns to the parent dialog.
	 *
	 * @param v the View
	 */
	public void cancel(View v) {
		// String vernichten
		ausgabe=null;
		finish();
	}
}
