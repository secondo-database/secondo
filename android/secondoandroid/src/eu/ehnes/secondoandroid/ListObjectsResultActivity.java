package eu.ehnes.secondoandroid;

import java.util.List;

import eu.ehnes.secondoandroid.R;
import sj.lang.ListExpr;
import android.app.Activity;
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
public class ListObjectsResultActivity extends Activity {
	
	public void onLowMemory() {
		super.onLowMemory();
		System.out.println("Main memory is low");
	}

	protected void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_listobjectsresult);
		Object liste=null;
		TextView ausgabe = (TextView) findViewById(R.id.queryresulttextview);
		ListView ausgabeliste = (ListView) findViewById(R.id.queryresultlistview);
		ScrollView sv= (ScrollView)findViewById(R.id.scrollView1);

		Bundle extras=getIntent().getExtras();
		if(extras!=null) {
			String ergebnisText=extras.getString("Object");
			try {
				liste=SecondoActivity.sh.query("query "+ergebnisText);
			}catch(OutOfMemoryError ome) {
				Toast.makeText(this, "Query not possible, because the system is running out of memory", Toast.LENGTH_LONG).show();

			}
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
					ListAdapter ausgabeAdapter = new ArrayAdapter(this, R.layout.row, itemlist);
					ausgabeliste.setAdapter(ausgabeAdapter);

					} else	{
						ausgabeliste.setVisibility(View.GONE);
						ausgabe.setText(liste.toString());
					}
			} else {
				ausgabeliste.setVisibility(View.GONE);
				ausgabe.setText(SecondoActivity.sh.errorMessage());
			}

		} else {
			ausgabeliste.setVisibility(View.GONE);
			ausgabe.setText("No Database accessible.");
		}
	}


	

}
