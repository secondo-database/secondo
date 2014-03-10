package de.fernunihagen.dna.secondo4android.activity;

import java.util.ArrayList;

import de.fernunihagen.dna.secondo4android.R;
import de.fernunihagen.dna.secondocore.ListOut;
import android.app.ListActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;
import sj.lang.ListExpr;

public class DatabaseOpenActivity extends ListActivity {

	private ListView lv;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_databaseopen);
		
		// Der aktuelle Listview Kontext wird geholt.
		lv=getListView();
		ArrayList<String> item = new ArrayList<String>();
		
		
		ListExpr liste=(ListExpr)SecondoActivity.secondoDba.querySync("list databases");
		if(liste!=null) {
			ListOut lo=new ListOut();
			lo.ListToStringArray(liste);
			//System.out.println(lo.ListToStringArray(liste));
			
			if(lo.size()>2 && lo.getElem(0).equals("inquiry") && lo.getElem(1).equals("databases") ) {
				for(int i=2;i<lo.size();i++) {
					item.add(lo.getElem(i));
				}
			}
		} else {
			// To be implemented
		}

		

		ArrayAdapter<String> fileList = new ArrayAdapter<String>(this, R.layout.row, item);
		
		lv.setOnItemClickListener(new AdapterView.OnItemClickListener() {
			
			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
				Object o=lv.getItemAtPosition(position);
				SecondoActivity.secondoDba.querySync("open database "+o.toString());
		        Toast.makeText(getBaseContext(), "Database opened", Toast.LENGTH_LONG).show();

				finish();
			}
		});
	    setListAdapter(fileList); 
	}
}
