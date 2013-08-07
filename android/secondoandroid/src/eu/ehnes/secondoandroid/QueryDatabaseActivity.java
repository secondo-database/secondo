package eu.ehnes.secondoandroid;

import java.util.List;

import sj.lang.ListExpr;

import android.app.Activity;
import android.app.ProgressDialog;
import android.os.Bundle;
import android.os.SystemClock;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.ScrollView;
import android.widget.TextView;


/**
 * Abfrage per Kommandozeile
 * 
 * @author juergen
 *
 */
public class QueryDatabaseActivity extends Activity {

	private EditText eingabe;
	private TextView ausgabe;
	private History history;
	
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_querydatabase);
		
		history=History.getInstance();
		
		eingabe = (EditText) findViewById(R.id.querystring);
		ausgabe = (TextView) findViewById(R.id.queryresultstring);
		System.out.println("onCreate");
	}
	
	/**
	 * Wenn auf den Send Requestbutton gedrückt wurde, wird hier die Abfrage ausgeführt und das Ergebnis als Rawtext ausgegeben.
	 * 
	 * @param v
	 */
	public void send_request(View v) {
		ProgressDialog dialog = ProgressDialog.show(this, "Secondo", "Query in Progress, please wait");
		
		SystemClock.sleep(1000);
				

		ScrollView sv= (ScrollView)findViewById(R.id.queryscrollview);
		ListView ausgabeliste = (ListView) findViewById(R.id.queryresultlistview2);

		System.out.println("send_request");
		eingabe= (EditText) findViewById(R.id.querystring);
		
		// Der Nullstring kommt nicht in die History
		if(eingabe.length()>0)
		{
			String eingabetext=eingabe.getText().toString().replace('"', '\'');
			history.add(eingabe.getText().toString());
			
			Object liste=SecondoActivity.sh.query(eingabetext);
			if(liste!=null) {
				eingabe.setText("");
				eingabe.setHint("Last entered command:\n\n"+eingabetext);

				ProcessQueries pq=new ProcessQueries();
				List<String> itemlist=pq.CreateItemList((ListExpr)liste);
				if(PreferencesActivity.isFormattedOutputState() && itemlist!=null) {
					ausgabe.setVisibility(View.INVISIBLE);
					sv.setVisibility(View.GONE);
					ausgabeliste.setVisibility(View.VISIBLE);

					@SuppressWarnings({ "unchecked", "rawtypes" })
					ListAdapter ausgabeAdapter = new ArrayAdapter(this, R.layout.row, itemlist);
					ausgabeliste.setAdapter(ausgabeAdapter);
				}
				else {
					sv.setVisibility(View.VISIBLE);
					ausgabeliste.setVisibility(View.GONE);
					ausgabe.setVisibility(View.VISIBLE);

					ausgabe.setText(liste.toString());
				}

			} else {
				sv.setVisibility(View.VISIBLE);
				ausgabeliste.setVisibility(View.GONE);
				ausgabe.setVisibility(View.VISIBLE);
				eingabe.setText("");
				ausgabe.setText("Error: "+SecondoActivity.sh.errorMessage());

			}
		}
		dialog.dismiss();
	}
	
	/**
	 * Der Last Command Button wurde gedrückt und das letzte Kommando aus der History geholt.
	 * 
	 * @param v
	 */
	public void last_command(View v) {
		eingabe.setText(history.last());

	}

	public void next_command(View v) {
		eingabe.setText(history.next());

	}
	public void clear_command(View v) {
		eingabe.setText("");

	}

}
