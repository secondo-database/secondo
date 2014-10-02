package eu.ehnes.secondoandroid;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Switch;

/*
 * In dieser Activity werden die Voreinstellungen verwaltet.
 * Momentan gibt es nur ein Element, nämlich ob die Ausgabe
 * formatiert oder als einfache Textausgabe erfolgen soll.
 * Der Status wird in eine statische Variable geschrieben
 * und überlebt das Programmende nicht.
 */

public class PreferencesActivity extends Activity {
	private Switch formattedOutputSwitch;
	private static boolean formattedOutputState=true;
	
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);

		
		setContentView(R.layout.activity_preferences);
		System.out.println("PreferencesActivity started");

		formattedOutputSwitch=(Switch) findViewById(R.id.switch1);

		if(isFormattedOutputState()) {
			formattedOutputSwitch.setChecked(true);
		}
		else {
			formattedOutputSwitch.setChecked(false);
		}

	}
	
	public void formattingChanged(View v) {
		formattedOutputSwitch=(Switch) findViewById(R.id.switch1);
		
		
		if(formattedOutputSwitch.isChecked()) {
			System.out.println("Aktiviert");
			setFormattedOutputState(true);
			
		} else {
			System.out.println("DeAktiviert");
			setFormattedOutputState(false);
		}
	}

	public static boolean isFormattedOutputState() {
		return formattedOutputState;
	}

	public static void setFormattedOutputState(boolean formattedOutputState) {
		PreferencesActivity.formattedOutputState = formattedOutputState;
	}


}
