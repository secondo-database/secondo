package eu.ehnes.secondoandroid.activity;

import eu.ehnes.secondoandroid.R;
import eu.ehnes.secondoandroid.R.id;
import eu.ehnes.secondoandroid.R.layout;
import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

/**
 * The Class MemoryActivity.
 * 
 * @author juergen
 * @version 1.0
 */
public class MemoryActivity extends Activity {
	TextView totalMemoryValue;
	TextView maxMemoryValue;
	TextView freeMemoryValue;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_memory);
		
		totalMemoryValue = (TextView) findViewById(R.id.totalMemoryValue);
		maxMemoryValue = (TextView) findViewById(R.id.maxMemoryValue);
		freeMemoryValue = (TextView) findViewById(R.id.freeMemoryValue);

		showMemory();
	}

	/**
	 * Refresh memory.
	 *
	 * @param v the View 
	 */
	public void refreshMemory(View v) {
		showMemory();
	}
	
	/**
	 * Show memory.
	 */
	public void showMemory() {
		maxMemoryValue.setText(""+Runtime.getRuntime().maxMemory());
		totalMemoryValue.setText(""+Runtime.getRuntime().totalMemory());
		freeMemoryValue.setText(""+Runtime.getRuntime().freeMemory());
	}
}
