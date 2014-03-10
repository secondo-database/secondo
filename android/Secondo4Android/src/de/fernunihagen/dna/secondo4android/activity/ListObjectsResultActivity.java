package de.fernunihagen.dna.secondo4android.activity;

import java.util.List;

import de.fernunihagen.dna.secondo4android.R;
import de.fernunihagen.dna.secondocore.ProcessQueries;
import de.fernunihagen.dna.secondocore.itf.ISecondoDbaCallback;
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
	
//	ListView ausgabeliste = null;
//	TextView ausgabe = null;
//	ScrollView sv = null;
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
		TextView ausgabe = (TextView) findViewById(R.id.queryresulttextview);
		ListView ausgabeliste = (ListView) findViewById(R.id.queryresultlistview);
		

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
		final Activity thisActivity = this;

		runOnUiThread(new Runnable() { 
			@Override
			public void run() {
				ListExpr newlist=(ListExpr)liste;

				ScrollView sv= (ScrollView)findViewById(R.id.scrollView1);
				ListView ausgabeliste = (ListView) findViewById(R.id.queryresultlistview);
				TextView ausgabe = (TextView) findViewById(R.id.queryresulttextview);

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
						String teststring = liste.toString();
						System.out.println(teststring);
						//String teststring = "Hallo Welt";
						ausgabe.setText(teststring);
					}
				} else {
					ausgabeliste.setVisibility(View.GONE);
					ausgabe.setText(SecondoActivity.secondoDba.errorMessageSync());
				}
			}
		});
		if (progressDialog != null && progressDialog.isShowing()) {
			progressDialog.dismiss();
			progressDialog = null;
		}

	}

	@Override
	public void initializeCallBack(boolean result) {
		// nothing to do
		
	}


	

}
