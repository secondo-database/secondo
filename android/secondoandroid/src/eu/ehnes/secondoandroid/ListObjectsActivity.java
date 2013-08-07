package eu.ehnes.secondoandroid;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import eu.ehnes.secondoandroid.R;

import sj.lang.ListExpr;
import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.TextView;
import android.widget.Toast;
import android.view.MotionEvent;
import android.view.View.OnTouchListener;

/**
 * @author Jürgen Ehnes
 * @version 1.0
 *
 */
public class ListObjectsActivity extends ListActivity {
	
	private ListView lv;
    private PopupWindow popupWindow;
    private List<String> stringList;
    private Map<String,String> typeList;
    
    /**
     * onCreate
     * @param savedInstanceState Der gespeicherte Zustand der Instanz
     */
	protected void onCreate(Bundle savedInstanceState) {
		boolean fehler=false;
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_listobjects);
		
		lv=getListView();
		ListExpr liste=null;
		
		try {
			liste=(ListExpr)SecondoActivity.sh.query(SecondoCommands.listobjects);
		}
		catch(OutOfMemoryError ome) {
	        Toast.makeText(this, "Query is not successful. System is running out of memory", Toast.LENGTH_LONG).show();
		}
		stringList=new ArrayList<String>(liste.listLength());
		typeList=new HashMap<String,String>(liste.listLength());
		List<String> item = new ArrayList<String>(liste.listLength());
		
		if(liste!=null) {
			 
			ListOut lo=new ListOut();
			
			// ListExpr in Stringarray umwandeln um nicht rekursiv auf der Liste arbeiten zu müssen. 
			lo.ListToStringArray(liste);
			
			// Prüfen ob das Ergebnis der Abfrage eine Objektliste ist. Wenn nicht, wird eine Fehlermeldung ausgegeben.
			if(lo.size()>3 && lo.getElem(0).equals(SecondoCommands.inquiry) && lo.getElem(1).equals(SecondoCommands.objects) && lo.getElem(2).equals("OBJECTS") ) {
				for(int i=2;i<lo.size();i++) {
					// Ein Element nach dem anderen in die Liste ausgeben.
					if(lo.getElem(i).equals("OBJECT")) {
						stringList.add(lo.getElem(i+1));
						StringBuffer sb=new StringBuffer(1000);
						int j=i+5;
						while(j<lo.size()+6 && !lo.getElem(j).equals("OBJECT") && !lo.getElem(j+1).equals("OBJECT")) {
							sb.append(lo.getElem(j)+" : ");
							sb.append(lo.getElem(j+1)+"\n");
							j=j+2;
						}
						typeList.put(lo.getElem(i+1), sb.toString());
						item.add(lo.getElem(i+1)+" : "+lo.getElem(i+3));
						fehler=false;
					}
				}
			}else {
				item.add(SecondoActivity.sh.errorMessage());
				fehler=true;
			}			
		} 
		
		// Der Arrayadapter wird als Vorbereitung für die ausgabe erstellt und bekommt als Paramter die itemliste sowie das Design der Zeile (row)
		ArrayAdapter<String> fileList = new ArrayAdapter<String>(this, R.layout.row, item);

		if(!fehler) {
			LinearLayout ll=(LinearLayout)findViewById(R.id.listobjectslinearlayout);
			
			ll.setOnTouchListener(new OnTouchListener() {
				public boolean onTouch(View v, MotionEvent event) {
					if(popupWindow!=null && popupWindow.isShowing() && (event.getAction() != MotionEvent.ACTION_UP) ) {
						popupWindow.dismiss();
					}

					return false;
				}
			}
					
				
			);
			lv.setOnTouchListener(new OnTouchListener() {
				public boolean onTouch(View v, MotionEvent event) {
					if(popupWindow!=null && popupWindow.isShowing() && (event.getAction() != MotionEvent.ACTION_UP) ) {
						popupWindow.dismiss();
					}

					return false;
				}
			}
					
				
			);
			
			lv.setOnItemClickListener(new AdapterView.OnItemClickListener() {
			
				public void onItemClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
					// Es wurde auf ein Element geklickt. Dieses wird nun ausgewählt und ein neues Fenster mit dem Ergebnis
					// des queries geöffnet. 
					// Die Übergabe des Elements erfolgt über Intents
					if(popupWindow!=null && popupWindow.isShowing()) {
						popupWindow.dismiss();
					} else {
						Object o=stringList.get(position);


						Bundle rucksack = new Bundle();

						rucksack.putString("Object", o.toString());

						//setResult(RESULT_OK,objectString);
						Intent myIntent = new Intent(arg1.getContext(), ListObjectsResultActivity.class);
						myIntent.putExtras(rucksack);
						startActivityForResult(myIntent, 0);
					}
				}
			});
			
			lv.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
				// Wenn lang auf das Display gedrückt wurde, werden die Typen angezeigt.
				public boolean onItemLongClick(AdapterView<?> arg0, View arg1, int position, long arg3) {

					// Wenn das Fenster schon angezeigt wird, wird es ausgeblendet
					if(popupWindow!=null && popupWindow.isShowing()) {
						popupWindow.dismiss();
						popupWindow=null;
					}
					
					// Die Typliste des Elements an der angetippten Position wird generiert
					String types=typeList.get(stringList.get(position));
					
					// Da wir uns nicht mehr in der Funktion onCreate befinden muss der aktuelle 
					// Context neu geholt werden
					Context context=arg1.getContext();
					
					// Die Darstellung wird generiert.
					LayoutInflater layoutInflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
					View popupView = layoutInflater.inflate(R.layout.popup, null);
						
					popupWindow = new PopupWindow(popupView, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
					
					TextView ausgabe = (TextView) popupView.findViewById(R.id.popupTextView);		
					ausgabe.setText(types);
					
					popupWindow.showAsDropDown(arg1);
					
					return true;
				}
			});
			
			
		} else {
			
			}
		
			this.setListAdapter(fileList); 
		
	}
	
	public void onLowMemory() {
		super.onLowMemory();
		System.out.println("Main memory is low");
	}

}
