package eu.ehnes.secondoandroid.activity;

import java.util.List;

import eu.ehnes.secondoandroid.ProcessQueries;
import eu.ehnes.secondoandroid.R;
import eu.ehnes.secondoandroid.R.id;
import eu.ehnes.secondoandroid.R.layout;
import eu.ehnes.secondoandroid.itf.ISecondoDbaCallback;
import sj.lang.ListExpr;
import android.app.Activity;
import android.app.ProgressDialog;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

/**
 * Diese Klasse bekommt über Intents die Tabelle die abgefragt werden soll, führt diese Abfrage durch und 
 * stellt das Ergebnis in einer weiteren Liste dar. 
 * 
 * @author juergen
 * @version 1.0
 */
public class ListObjectsResultActivity extends Activity implements ISecondoDbaCallback {
	
	ListView ausgabeliste = null;
	TextView ausgabe = null;
	ScrollView sv = null;
	ProgressDialog progressDialog = null;
	
	@Override
	public void onLowMemory() {
		super.onLowMemory();
		System.out.println("Main memory is low");
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_listobjectsresult);
		// Object liste=null;
		ausgabe = (TextView) findViewById(R.id.queryresulttextview);
		ausgabeliste = (ListView) findViewById(R.id.queryresultlistview);
		sv= (ScrollView)findViewById(R.id.scrollView1);

		Bundle extras=getIntent().getExtras();
		if(extras!=null) {
			String ergebnisText=extras.getString("Object");
			progressDialog = ProgressDialog.show(this, "Secondo Query", "Secondo is executing the query. Please wait!");
			try {
				SecondoActivity.secondoDba.queryASync("query "+ergebnisText, this);
			}catch(OutOfMemoryError ome) {
				Toast.makeText(this, "Query not possible, because the system is running out of memory", Toast.LENGTH_LONG).show();

			}
			
			// Lücke
		} else {
			ausgabeliste.setVisibility(View.GONE);
			ausgabe.setText("No Database accessible.");
		}
	}

	@Override
	public void queryCallBack(final Object liste) {
		System.out.println(this.getClass().getName());
		final Activity thisActivity = this;
		
		runOnUiThread(new Runnable() { 
			@Override
			public void run() {
			ListExpr newlist=(ListExpr)liste;

			// Die Darstellung erfolgt je nachdem, ob die Ausgabetypen bekannt sind, als Listview 
			// oder als Textview.
			if(liste!=null) {
				ProcessQueries pq=new ProcessQueries();
				List<String> itemlist=pq.CreateItemList(newlist);
				if(itemlist!=null) {
					ausgabe.setVisibility(View.GONE);
					sv.setVisibility(View.GONE);
					@SuppressWarnings({ "unchecked", "rawtypes" })
					ListAdapter ausgabeAdapter = new ArrayAdapter(thisActivity, R.layout.row, itemlist);
					ausgabeliste.setAdapter(ausgabeAdapter);

				} else	{
					ausgabeliste.setVisibility(View.GONE);
					ausgabe.setText(liste.toString());
				}
			} else {
				ausgabeliste.setVisibility(View.GONE);
				ausgabe.setText(SecondoActivity.secondoDba.errorMessageSync());
			}
			if (progressDialog != null && progressDialog.isShowing()) {
				progressDialog.dismiss();
			}
			}
		});

	}

	@Override
	public void initializeCallBack(boolean result) {
		// nothing to do
		
	}


	

}
