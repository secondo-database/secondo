package de.fernunihagen.dna;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import de.fernunihagen.dna.hoese.QueryResult;
import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ListView;
import android.widget.TextView;
import android.content.Context;
import android.content.Intent;

public class TextActivity extends Activity {
	private static final String TAG = "TextActivity";
	private static final String NEWLINE = "\n";
	private QueryResult queryResult;
	private List<QueryResult> queryResults;

	@SuppressWarnings("unchecked")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_text);
		// Show the Up button in the action bar.
		Intent intent = getIntent();
		queryResult = (QueryResult) intent.getSerializableExtra(CommandActivity.EXTRA_RESULT);
		assert(queryResult != null);
		queryResults = (List<QueryResult>) intent.getSerializableExtra(CommandActivity.EXTRA_RESULTS);

		for (QueryResult qr : queryResults) {
			if (qr.isActual()) queryResult = qr;
		}
		
		// if no actual result, then take the first Result
		if (queryResults.size() > 0 && queryResult == null) {
			queryResult = queryResults.get(0);
			queryResult.setActual(true);
			
		}
		
		createAdapter();

	}

	public void updateActivity() {
		QueryResult lastresult = queryResult;

		for (QueryResult qr : queryResults) {
			if (qr.isActual()) queryResult = qr;
		}
		
		if (lastresult.equals(queryResult))
			return;

		createAdapter();
	}


	private void createAdapter() {
		// Search for actual QueryResult
		
		final StableArrayAdapter adapter = new StableArrayAdapter(this,
				R.xml.datatextitem, 
				convertToItem(queryResult.getStrings()), queryResult);
		
		final ListView listview = (ListView) findViewById(R.id.listview);
		listview.setAdapter(adapter);	
		listview.setOnItemClickListener(new AdapterView.OnItemClickListener() {

		      @Override
		      public void onItemClick(AdapterView<?> parent, final View view,
		          int position, long id) {
		        CheckBox checkBox = (CheckBox) view.findViewById(R.id.itemcheck);
		        checkBox.setChecked(!checkBox.isChecked());
		        boolean changeTagMode = false; // If you want to switch immediately to the grafic tab, set to true
				if (changeTagMode) {
		        	switchTabInActivity(2);
		        }
		      }

		    });
	}



	private List<String> convertToItem(List<String> strings) {
		ArrayList<String> items = new ArrayList<String>();
		StringBuilder item = new StringBuilder();
		for (String string : strings) {
			if ("---------".equals(string)) { // getting a new Item
				items.add(item.toString());
				item = new StringBuilder();
			} else {
				item.append(string);
				item.append(NEWLINE);
				}
		}
		String lastItem = item.toString();
		if (!"".equals(lastItem)) { // is the last Item empty?
			items.add(lastItem);
		}
		
		return items;
	}



	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.text, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		return super.onOptionsItemSelected(item);
	}
	
	public void switchTabInActivity(int indexTabToSwitchTo){
        OutputActivity parentActivity;
        parentActivity = (OutputActivity) this.getParent();
        parentActivity.switchTab(indexTabToSwitchTo);
}
	
	static class ItemViewHolder {
	    TextView text;
	    CheckBox checkbox;
	}
	
	
	private class StableArrayAdapter extends ArrayAdapter<String> {

		final private HashMap<String, Integer> mIdMap = new HashMap<String, Integer>();
	    final private QueryResult queryResult;
	    
	    public StableArrayAdapter(Context context, int textViewResourceId,
	        List<String> objects, QueryResult queryResult) {
	      super(context, textViewResourceId, objects);
	      this.queryResult = queryResult;
	      for (int i = 0; i < objects.size(); ++i) {
	        mIdMap.put(objects.get(i), i);
	      }
	    }

	    @Override
	    public long getItemId(int position) {
	      String item = getItem(position);
	      return mIdMap.get(item);
	    }

	    @Override
	    public boolean hasStableIds() {
	      return true;
	    }

	    public View getView(int position, View convertView, ViewGroup parent) {
	        View row = convertView;
	        // Holderpattern: http://www.jmanzano.es/blog/?p=166
	        final ItemViewHolder holder;
	        
	        if (row == null) {
	          holder = new ItemViewHolder();
	          LayoutInflater inflater = getLayoutInflater();
	          row = inflater.inflate(R.xml.datatextitem, parent, false);
		      CheckBox checkBox = (CheckBox) row.findViewById(R.id.itemcheck);
		      checkBox.setChecked(queryResult.getSelected(position));
		      
		      TextView label = (TextView) row.findViewById(R.id.dbServer);
		      holder.checkbox = checkBox;
		      holder.text = label;
		      row.setTag(holder);
		      
		      checkBox.setOnCheckedChangeListener(new OnCheckedChangeListener() {
				
				@Override
				public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
			         Log.w(TAG, "Tag ["+ buttonView.getTag() + "]");
			         queryResult.selectItem((Integer) buttonView.getTag(), isChecked);
			         }					
			});
		      
	        } else {
	        	holder = (ItemViewHolder) convertView.getTag();
	        }


		    String text = getItem(position);

		    holder.text.setText(text);
	        holder.checkbox.setTag(position); // Zuerst die Position setzen
		    boolean selected = queryResult.getSelected(position);
	         Log.w(TAG, "1: setChecked ["+ selected + "]");
	        holder.checkbox.setChecked(selected);
	         Log.w(TAG, "2: setChecked ["+ selected + "]");
	        return (row);
	      }


	  }

}
