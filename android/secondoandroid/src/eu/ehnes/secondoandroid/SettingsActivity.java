package eu.ehnes.secondoandroid;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.OutputStreamWriter;

import eu.ehnes.secondoandroid.R;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;

public class SettingsActivity extends Activity {
	private final static String fileName="Config.ini";
	private EditText ausgabe=null;
	private String programPath=null;
	
	
	protected void onCreate(Bundle savedInstanceState) {
		String ausgabeText="";
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.activity_settings);
		ausgabe=(EditText) findViewById(R.id.configeditview);
		programPath=OwnPath.getInstance().getPath()+"/../";

		/** 
		 * Datei einlesen
		 */
		try {
           BufferedReader input = new BufferedReader(new FileReader(programPath+"/"+fileName));
            String inputLine;
	 
	            while ((inputLine = input.readLine()) != null) {
	                ausgabeText = ausgabeText + '\n' + inputLine;
	            }
	            if (ausgabeText.endsWith("\n"))
	                ausgabeText = ausgabeText + "\n";
	 
	            input.close();
		} catch(IOException e) {
			Log.e("FU", "Failed to open Configfile file: " + programPath + "/"+fileName, e );

		}
		
		// Datei in Textfeld schreiben
		ausgabe.setText(ausgabeText);
		//InputMethodManager imm=(InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
		//imm.hideSoftInputFromWindow(ausgabe.getWindowToken(), 0);
		getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
		
	}
	
	protected void onDestroy(Bundle savedInstanceState) {
		finish();
	}
	
	public void save(View v) {
		
		// String aus Textfeld auslesen
		String ausgabeText=ausgabe.getText().toString();
		
		//String wieder in Datei schreiben
		try {
			BufferedWriter output=new BufferedWriter(new OutputStreamWriter(new FileOutputStream(programPath+"/"+fileName)));
			output.write(ausgabeText,0,ausgabeText.length());
			output.close();
		} catch(IOException e) {
			System.out.println("Konnte Datei nicht schreiben: "+programPath+"/"+fileName+" "+e);
		}
		
		finish();
		
	}
	
	
	public void cancel(View v) {
		// String vernichten
		ausgabe=null;
		finish();
	}
}
