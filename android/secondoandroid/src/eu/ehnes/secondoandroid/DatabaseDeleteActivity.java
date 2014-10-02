package eu.ehnes.secondoandroid;

import java.util.ArrayList;
import java.util.List;

import eu.ehnes.secondoandroid.R;

import sj.lang.ListExpr;

import android.app.ListActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

public class DatabaseDeleteActivity extends ListActivity {
	private ListView lv;
	
	
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_databaseopen);
		
		lv=getListView();		// Den aktuellen Listview für die Weiterbearbeitung ermitteln
		
		List<String> item = new ArrayList<String>();
		
		
		Object liste=SecondoActivity.sh.query("list databases");	// Die verfügbaren Datenbanken abfragen 
		if(liste!=null) {
			ListExpr newlist=(ListExpr)liste;	
			ListOut lo=new ListOut();		
			System.out.println(lo.ListToStringArray(newlist));		// Die Liste der Datenbanken in eine ArrayListe umwandeln
			
			// In der folgenden Schleife werden die ersten zwei Elemente ignoriert und bei allen
			// Elementen danach muss es sich um Datenbanknamen handeln. Diese werden in eine Itemliste 
			// geschrieben. 
			if(lo.size()>2 && lo.getElem(0).equals("inquiry") && lo.getElem(1).equals("databases") ) {
				for(int i=2;i<lo.size();i++) {
					item.add(lo.getElem(i));
				}
			}
		} else {
		}
	
		ArrayAdapter<String> fileList = new ArrayAdapter<String>(this, R.layout.row, item);
		
		// Die Liste wird in einem ListView ausgegeben
		lv.setOnItemClickListener(new AdapterView.OnItemClickListener() {
			
			// Wenn ein Eintrag ausgewählt wurde, muss er gelöscht werden
			public void onItemClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
				Object o=lv.getItemAtPosition(position);
				SecondoActivity.sh.query("delete database "+o.toString());
				
		        Toast.makeText(getBaseContext(), "Datenbank deleted", Toast.LENGTH_LONG).show();

		     
				finish();
			}
		});
	    setListAdapter(fileList); 
	}

}
