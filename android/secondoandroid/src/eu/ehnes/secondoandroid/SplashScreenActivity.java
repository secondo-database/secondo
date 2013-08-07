package eu.ehnes.secondoandroid;

import eu.ehnes.secondoandroid.R;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

public class SplashScreenActivity extends Activity {
	private TextView messageLine;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_splashscreen);
        
        
		
	}
	public void startSecondo(View v) {
        Intent myIntent = new Intent(v.getContext(), SecondoActivity.class);
        startActivityForResult(myIntent, 0);
		
        messageLine=(TextView) findViewById(R.id.textView1);
        messageLine.setText("Secondo is starting, please be patient...");
        PreferencesActivity.setFormattedOutputState(true);
		ProgressDialog.show(this, "Secondo", "Secondo is starting, please be patient ...");
	}
	
}
