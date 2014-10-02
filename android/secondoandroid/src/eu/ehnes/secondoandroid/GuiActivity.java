package eu.ehnes.secondoandroid;

import eu.ehnes.secondoandroid.R;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

public class GuiActivity extends Activity {
	private String path=null;
	private String filename=null;
	TextView messageLine;
	
	protected void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_gui);
	       messageLine=(TextView) findViewById(R.id.statusline);
		
			
	}
	
	protected void onDestroy() {
		super.onDestroy();
	}
	
	// Öffnen einer 
	public void opendatabase(View v) {
		String dbname=AccessDatabase.ActiveDatabase();
		if(dbname.length()>0)  {
	        Toast.makeText(this, "Unable to open database, please close other database first.", Toast.LENGTH_LONG).show();
    
		} else	{
			Intent myIntent = new Intent(v.getContext(), DatabaseOpenActivity.class);
			startActivityForResult(myIntent, 0);
		}		
	}
	public void deletedatabase(View v) {
		String dbname=AccessDatabase.ActiveDatabase();
		if(dbname.length()>0)  {
	        Toast.makeText(this, "Unable to delete database, please close database first.", Toast.LENGTH_LONG).show();
    
		} else	{
			Intent myIntent = new Intent(v.getContext(), DatabaseDeleteActivity.class);
			startActivityForResult(myIntent, 0);
		}
		
	}
	public void listobjects(View v) {
		String dbname=AccessDatabase.ActiveDatabase();
		if(dbname.length()==0)  {
	        Toast.makeText(this, "Unable to process query, please open a database first.", Toast.LENGTH_LONG).show();
    
		} else	{
			Intent myIntent = new Intent(v.getContext(), ListObjectsActivity.class);
			startActivityForResult(myIntent, 0);
		}
		
	}
	
	public void restoredatabase(View v) {
		// Dialog für das einlesen von Datenbanken aufrufen
		String dbname=AccessDatabase.ActiveDatabase();
		if(dbname.length()>0)  {
	        Toast.makeText(this, "Restore not possible, please close database first.", Toast.LENGTH_LONG).show();
    
		} else	{
			Intent myIntent = new Intent(v.getContext(), FileDialogActivity.class);
			startActivityForResult(myIntent, 0);
		}
	}
	public void closedatabase(View v) {
		// Schliessen der Datenbank
		// Fehler werden nicht abgefragt
		String dbname=AccessDatabase.ActiveDatabase();
		if(dbname.length()>0)  {
		       SecondoActivity.sh.query("close database");
		       messageLine.setText("No database open");
		       Toast.makeText(this, "Database closed", Toast.LENGTH_LONG).show();

		} else {
	        Toast.makeText(this, "No open database.", Toast.LENGTH_LONG).show();

		}


	}
	public void querydatabase(View v) {
		Intent myIntent = new Intent(v.getContext(), QueryDatabaseActivity.class);
		startActivityForResult(myIntent, 0);
		
	}
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		// TODO Auto-generated method stub
		super.onActivityResult(requestCode, resultCode, data);
		
		if(resultCode == RESULT_OK) {
			
			Bundle rucksack = data.getExtras();
			path=rucksack.getString("resultPath");
			filename=rucksack.getString("resultFile");
	        SecondoActivity.sh.query("restore database "+filename+" from '"+path+"'");			
		}
		
		System.out.println("Ergebnispfad: "+path);
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		// TODO Auto-generated method stub
		super.onWindowFocusChanged(hasFocus);
		
		if(hasFocus) {
			String dbname=AccessDatabase.ActiveDatabase();
			if(dbname.length()>0)  {
			       messageLine.setText("Open database: "+dbname);
			} else {
			       messageLine.setText("No database open");
			}
			
		}
	}

	
	
}
