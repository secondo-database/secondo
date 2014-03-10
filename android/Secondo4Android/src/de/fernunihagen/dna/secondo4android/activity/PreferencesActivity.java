package de.fernunihagen.dna.secondo4android.activity;

import de.fernunihagen.dna.secondo4android.R;
import android.app.Activity;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.view.View;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.TextView;

/**
 * In dieser Activity werden die Voreinstellungen verwaltet.
 * Momentan gibt es nur ein Element, nämlich ob die Ausgabe
 * formatiert oder als einfache Textausgabe erfolgen soll.
 * Der Status wird in eine statische Variable geschrieben
 * und überlebt das Programmende nicht.
 * 
 * @author juergen
 * @version 1.0
 */
public class PreferencesActivity extends Activity {
	private Switch formattedOutputSwitch;
	private Switch externalSourceSwitch;
	private EditText externalSourceValue;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		
		setContentView(R.layout.activity_preferences);
		System.out.println("PreferencesActivity started");

		formattedOutputSwitch=(Switch) findViewById(R.id.switch1);
		externalSourceSwitch = (Switch)findViewById(R.id.externalSourceSwitch);
		externalSourceValue = (EditText) findViewById(R.id.externalSourceValue);

		if(isFormattedOutputState()) {
			formattedOutputSwitch.setChecked(true);
		}
		else {
			formattedOutputSwitch.setChecked(false);
		}

	}
	
	/**
	 * Formatting changed.
	 *
	 * @param v the View
	 */
	public void formattingChanged(View v) {
		
		if(formattedOutputSwitch.isChecked()) {
			setFormattedOutputState(true);
			
		} else {
			setFormattedOutputState(false);
		}
	}

	/**
	 * External source changed.
	 *
	 * @param v the v
	 */
	public void externalSourceChanged(View v) {
		externalSourceValue.setEnabled(externalSourceSwitch.isChecked());
	}
	/**
	 * Checks if is formatted output state.
	 *
	 * @return true, if is formatted output state
	 */
	private boolean isFormattedOutputState() {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
		return preferences.getBoolean("formattedOutputState", true);
	}
	
	/**
	 * Sets the formatted output state persistently.
	 *
	 * @param state the new formatted output state
	 */
	private void setFormattedOutputState(boolean state) {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
		SharedPreferences.Editor editor = preferences.edit();
		editor.putBoolean("formattedOutputState", state);
		editor.commit();

	}
	
	@Override
	protected void onPause() {
		TextView configPathValue = (TextView) findViewById(R.id.configPathValue);
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
		SharedPreferences.Editor editor = preferences.edit();
		editor.putString ("configPath", configPathValue.getText().toString());
		editor.putBoolean("externalSourceState", externalSourceSwitch.isChecked());
		if (externalSourceSwitch.isChecked()) {
			editor.putString("externalSourcePath", externalSourceValue.getText().toString());
		}
		editor.commit();
		super.onPause();
	}

	/* (non-Javadoc)
	 * @see android.app.Activity#onResume()
	 */
	@Override
	protected void onResume() {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
		TextView configPathValue = (TextView) findViewById(R.id.configPathValue);
		if (configPathValue != null) {
			configPathValue.setText(preferences.getString("configPath", getApplicationContext().getFilesDir().getPath()+"/../Config.ini"));
		}

		boolean externalSourceStateBoolean = preferences.getBoolean("externalSourceState", false);
		externalSourceSwitch.setChecked(externalSourceStateBoolean);
		externalSourceValue.setEnabled(externalSourceStateBoolean);
		externalSourceValue.setText(preferences.getString("externalSourcePath", ""));

		super.onResume();

	}
}
